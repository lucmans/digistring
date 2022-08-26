#include "program.h"

#include "graphics.h"
#include "results_file.h"
#include "performance.h"
#include "quit.h"
#include "error.h"

#include "config/cli_args.h"
#include "config/audio.h"
#include "config/transcription.h"
#include "config/graphics.h"
#include "config/results_file.h"
#include "config/synth.h"

#include "estimators/estimators.h"
#include "sample_getter/sample_getters.h"

#include <SDL2/SDL.h>

#include <iomanip>  // std::setw()
#include <chrono>
#include <thread>  // sleep
#include <utility>  // std::move()
#include <cmath>  // std::round()
#include <cstring>  // memcpy()


Program::Program(Graphics *const _g, SDL_AudioDeviceID *const _in, SDL_AudioDeviceID *const _out) : graphics(_g) {
    // As early as possible, so it is very likely enough time has passed to render next frame when main loop is running
    prev_frame = std::chrono::steady_clock::now();

    in_dev = _in;
    out_dev = _out;

    // We let the estimator create the input buffer for optimal size and better alignment
    input_buffer = NULL;
    input_buffer_n_samples = -1;
    // estimator = new Tuned(input_buffer, input_buffer_n_samples);
    estimator = new HighRes(input_buffer, input_buffer_n_samples);
    // estimator = new BasicFourier(input_buffer, input_buffer_n_samples);
    if(input_buffer == NULL) {
        error("Estimator did not create an input buffer");
        exit(EXIT_FAILURE);
    }
    if(input_buffer_n_samples == -1) {
        error("Estimator did not set number of samples in input buffer");
        exit(EXIT_FAILURE);
    }

    switch(cli_args.audio_input_method) {
        case SampleGetters::wave_generator:
            sample_getter = new WaveGenerator(input_buffer_n_samples, cli_args.generate_sine_freq);
            break;

        case SampleGetters::note_generator:
            sample_getter = new NoteGenerator(input_buffer_n_samples, cli_args.generate_note_note);
            break;

        case SampleGetters::audio_file:
            sample_getter = new AudioFile(input_buffer_n_samples, cli_args.play_file_name);
            break;

        case SampleGetters::audio_in:
            sample_getter = new AudioIn(input_buffer_n_samples, in_dev);
            break;

        default:
            error("No entry in switch to construct given SampleGetters type");
            exit(EXIT_FAILURE);
    }
    if(sample_getter->is_audio_recording_device()) {
        if(cli_args.do_slowdown) {
            error("Can't slowdown if SampleGetter is an audio recording device (SampleGetter is blocking)");
            exit(EXIT_FAILURE);
        }
        if(cli_args.sync_with_audio) {
            warning("You should not sync with audio during real-time usage for optimal performance; disabling syncing...");
            cli_args.sync_with_audio = false;
        }
    }
    else {
        if constexpr(DO_OVERLAP_NONBLOCK) {
            error("Can't perform nonblocking overlap on samples getters which do not block (audio recording devices)");
            hint("Enable normal overlapping instead in config/transcription.h or choose a different sample getter");
            exit(EXIT_FAILURE);
        }
    }

    synth_buffer = nullptr;
    if(cli_args.synth) {
        synth_buffer_n_samples = input_buffer_n_samples;
        try {
            synth_buffer = new float[synth_buffer_n_samples];
        }
        catch(const std::bad_alloc &e) {
            error("Failed to allocate synth_buffer (" + STR(e.what()) + ")");
            hint("Try using an Estimator with a smaller input buffer size or don't synthesize sound");
            exit(EXIT_FAILURE);
        }
        synth = synth_factory(cli_args.synth_type);
    }
    volume = cli_args.volume;

    if(cli_args.stereo_split) {
        playback_buffer_n_samples = input_buffer_n_samples;
        try {
            playback_buffer = new float[playback_buffer_n_samples];
        }
        catch(const std::bad_alloc &e) {
            error("Failed to allocate playback_buffer (" + STR(e.what()) + ")");
            hint("Try using an Estimator with a smaller input buffer size or don't playback sound");
            exit(EXIT_FAILURE);
        }
    }

    mouse_clicked = false;

    if(cli_args.output_file)
        results_file = new ResultsFile(cli_args.output_filename);

    note_change_time = std::chrono::duration<double>(NOTE_TIME + 1);
}

Program::~Program() {
    if(cli_args.output_file)
        delete results_file;

    if(cli_args.synth) {
        if(synth_buffer != nullptr)  // Check, as Program may be destroyed while synth_buffer is reallocated
            delete[] synth_buffer;
        delete synth;
    }

    if(cli_args.stereo_split) {
        if(playback_buffer != nullptr)  // Check, as Program may be destroyed while synth_buffer is reallocated
            delete[] playback_buffer;
    }

    delete sample_getter;
    delete estimator;
}


void Program::main_loop() {
    // Unpause audio devices so that samples are collected/played
    if(cli_args.audio_input_method == SampleGetters::audio_in)
        SDL_PauseAudioDevice(*in_dev, 0);

    if(cli_args.playback || cli_args.synth)
        SDL_PauseAudioDevice(*out_dev, 0);

    if(cli_args.output_file) {
        write_result_header();
        results_file->start_array("note events");  // Stopped after while loop, so only note events can be pushed from now on
    }

    Performance perf;

    const std::chrono::steady_clock::time_point start_estimation_loop = std::chrono::steady_clock::now();
    unsigned long processed_samples = 0;
    while(!poll_quit()) {
        perf.clear_time_points();
        perf.push_time_point("Start");

        // Check for keyboard/mouse/window/OS events
        handle_sdl_events();
        if(poll_quit())
            break;
        perf.push_time_point("Handled SDL events");

        // Read a frame
        // new_samples is not const, as slowdown may alter it
        int new_samples = sample_getter->get_frame(input_buffer, input_buffer_n_samples);  // TODO: Don't block on this call (or still handle SDL events during)
        processed_samples += new_samples;
        perf.push_time_point("Got samples");

        // Play input_buffer back to the user before it is altered by estimator->perform()
        if(cli_args.playback)
            playback_audio(new_samples);

        // Send frame to estimator
        NoteEvents estimated_events;
        estimator->perform(input_buffer, estimated_events);
        perf.push_time_point("Pitch estimated");

        // If less than input_buffer_n_samples new samples are retrieved, only the NoteEvents regarding the first 'new_samples' samples are relevant, as the rest is "overwritten" in the next cycle
        if(new_samples < input_buffer_n_samples)
            adjust_events(estimated_events, input_buffer_n_samples, new_samples);
        // DEBUG: Sanity check
        else if(new_samples > input_buffer_n_samples) {
            error("Received too many samples from SampleGetter");
            exit(EXIT_FAILURE);
        }

        // Write estimation to output file (before applying slowdown)
        if(cli_args.output_file)
            write_results(estimated_events, new_samples);

        if(cli_args.do_slowdown)
            slowdown(estimated_events, new_samples);

        // Arg parser disallows both cli_args.playback and cli_args.synth to be true
        if(cli_args.synth) {
            synthesize_audio(estimated_events, new_samples);
            perf.push_time_point("Synthesized audio");
        }

        if(cli_args.stereo_split)
            play_split_audio(new_samples);

        // Print note estimation to CLI
        // print_results(estimated_events);

        // Graphics
        if constexpr(!HEADLESS) {
            const bool new_frame = update_graphics(estimated_events);
            if(new_frame)
                perf.push_time_point("Frame rendered");
        }

        // Print performance information to CLI
        if(cli_args.output_performance)
            std::cout << perf << std::endl;

        // Always has to be done when using audio out to prevent program running faster than audio
        // Otherwise, when no audio out is used and cli_args.sync_with_audio is true, it will simulate this behavior
        sync_with_audio(new_samples);


        // Arpeggiator easter egg
        if constexpr(ENABLE_ARPEGGIATOR)
            arpeggiate();
    }
    const std::chrono::steady_clock::time_point stop_estimation_loop = std::chrono::steady_clock::now();
    const std::chrono::duration<double> estimation_loop_time = stop_estimation_loop - start_estimation_loop;
    info("Pitch estimation time: " + STR(estimation_loop_time.count()) + " s");
    if(!cli_args.sync_with_audio && !cli_args.playback && !cli_args.synth && !sample_getter->is_audio_recording_device()) {
        info("Processed samples time: " + STR((double)processed_samples / (double)SAMPLE_RATE) + " s");
        info("Estimator was at least " + STR(((double)processed_samples / (double)SAMPLE_RATE) / estimation_loop_time.count()) + " times real-time");
    }

    if(cli_args.output_file) {
        // Write silent note event to explicitly stop the last note
        write_results(NoteEvents(), 0);

        // End note events array
        results_file->stop_array();
    }
}


void Program::resize(const int w, const int h) {
    graphics->resize_window(w, h);
}


void Program::playback_audio(const int new_samples) {
    if constexpr(PRINT_AUDIO_UNDERRUNS)
        if(SDL_GetQueuedAudioSize(*out_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8) == 0)
            warning("Audio underrun; no audio left to play");

    if(cli_args.stereo_split) {
        if(new_samples > playback_buffer_n_samples) {
            delete[] playback_buffer;
            playback_buffer = nullptr;  // In case Program is destroyed between delete[] and new float[]

            playback_buffer_n_samples = new_samples;
            try {
                debug("Reallocating playback buffer to accommodate for slowdown");
                playback_buffer = new float[playback_buffer_n_samples];
            }
            catch(const std::bad_alloc &e) {
                error("Failed to allocate new (larger) playback_buffer (" + STR(e.what()) + ")");
                hint("Slowdown factor may be too large, which causes the need for large reallocated synth buffers");
                exit(EXIT_FAILURE);
            }
        }

        memcpy(playback_buffer, input_buffer + (input_buffer_n_samples - new_samples), new_samples * sizeof(float));
    }
    else {
        if(SDL_QueueAudio(*out_dev, input_buffer + (input_buffer_n_samples - new_samples), new_samples * sizeof(float))) {
            error("Failed to queue audio for playback\nSDL error: " + STR(SDL_GetError()));
            exit(EXIT_FAILURE);
        }
    }

    // Waiting till samples are played is done in sync_with_audio()
}


void Program::write_result_header() {
    results_file->write_int("Sample rate (Hz)", SAMPLE_RATE);
    results_file->write_int("Input buffer size (samples)", input_buffer_n_samples);
    results_file->write_double("Input buffer time (ms)", ((double)input_buffer_n_samples * 1000.0) / (double)SAMPLE_RATE);
    results_file->write_double("Fourier bin size (Hz)", (double)SAMPLE_RATE / (double)input_buffer_n_samples);

    if(DO_OVERLAP)
        results_file->write_double("Overlap ratio", OVERLAP_RATIO);

    if(DO_OVERLAP_NONBLOCK) {
        results_file->write_int("Minimum non-blocking overlap ratio", MIN_NONBLOCK_OVERLAP_RATIO);
        results_file->write_int("Maximum non-blocking overlap ratio", MAX_NONBLOCK_OVERLAP_RATIO);
    }
}

void Program::write_results(const NoteEvents &note_events, const int new_samples) {
    results_file->start_dict();

    const int start_frame_samples = sample_getter->get_played_samples() - new_samples;
    const double start_frame_time = (double)start_frame_samples / (double)SAMPLE_RATE;

    const int n_notes = note_events.size();
    if(n_notes == 0) {
        if constexpr(WRITE_SILENCE) {
            results_file->write_int("note_start (samples)", start_frame_samples);
            results_file->write_double("note_start (seconds)", start_frame_time);
            results_file->write_null("note_duration (samples)");
            results_file->write_null("note_duration (seconds)");
            results_file->write_null("note");
            results_file->write_null("frequency");
            results_file->write_null("amplitude");
            results_file->write_null("error");
            results_file->write_null("midi_number");
        }
    }

    for(const auto &note_event : note_events) {
        const std::string note = note_to_string_ascii(note_event.note);
        results_file->write_int("note_start (samples)", start_frame_samples + note_event.offset);
        results_file->write_double("note_start (seconds)", start_frame_time + ((double)note_event.offset / (double)SAMPLE_RATE));
        results_file->write_int("note_duration (samples)", note_event.length);
        results_file->write_double("note_duration (seconds)", (double)note_event.length / (double)SAMPLE_RATE);
        results_file->write_string("note", note);
        results_file->write_double("frequency", note_event.note.freq);
        results_file->write_double("amplitude", note_event.note.amp);
        results_file->write_double("error", note_event.note.error);
        results_file->write_int("midi_number", note_event.note.midi_number);
    }

    results_file->stop_dict();
}


void Program::print_results(const NoteEvents &note_events) const {
    if(note_events.size() == 0)
        return;

    if(note_events.size() > 1) {
        warning("Polyphonic result printing is not yet supported");  // TODO: Support
        return;
    }

    std::cout << note_events[0].note << "  (" << note_events[0].note.freq << " Hz, " << note_events[0].note.amp << " dB, " << note_events[0].note.error << " cent)" << "     \r" << std::flush;
}


bool Program::update_graphics(const NoteEvents &note_events) {
    // Get the pointer to the graphics object here, as it is only valid till next call to Estimator::perform()
    const EstimatorGraphics *const estimator_graphics = estimator->get_estimator_graphics();

    // Limit FPS
    frame_time = std::chrono::steady_clock::now() - prev_frame;
    if(frame_time.count() < 1000.0 / MAX_FPS)
        return false;
    prev_frame = std::chrono::steady_clock::now();

    // Set render data and render frame
    graphics->set_clicked((mouse_clicked ? mouse_x : -1), mouse_y);

    if(cli_args.audio_input_method == SampleGetters::audio_in)
        graphics->set_queued_samples(SDL_GetQueuedAudioSize(*in_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8));
    else if(cli_args.audio_input_method == SampleGetters::audio_file)
        graphics->set_file_played_time(sample_getter->get_played_time());  // TODO: Subtract new_samples(_time) from playtime?

    const int n_notes = note_events.size();
    if(n_notes == 0)
        graphics->render_frame(nullptr, estimator_graphics);
    else if(n_notes == 1)
        graphics->render_frame(&note_events[0].note, estimator_graphics);
    else  // n_notes > 1
        warning("Polyphonic graphics not yet supported");  // TODO: Support

    return true;
}


/*static*/ void Program::adjust_events(NoteEvents &events, const int n_frame_samples, const int new_samples) {
    /*                  OLD                NEW
     *    +-----------------------------+-------+
     *  1 |               |------>      |       |
     *  2 |                        |------->    |
     *  3 |                             | |---> |
     *    +-------------------------------------+
     *  These are the three different cases when adjusting events, see image above
     *  The first case, we can simply discard
     *  In the second case, we have to shorten the length by "old_samples - event.offset" and set offset to 0
     *  In the third case, we copy the length and set the offset to "event.offset - old_samples"
     */
    const int old_samples = n_frame_samples - new_samples;

    // Make second NoteEvents and move events, so that case 1 event deletion doesn't interfere with the loop
    NoteEvents adjusted;
    for(const auto &event : events) {
        // Case 1
        if(event.offset + event.length < old_samples)
            continue;

        // Case 2
        // Else if below only filters for case 2, as case 1 is filtered by previous if
        else if(event.offset < old_samples) {
            const int offset_before_new = old_samples - event.offset;
            const int new_length = event.length - offset_before_new;
            const int new_offset = 0;
            adjusted.push_back(NoteEvent(std::move(event.note), new_length, new_offset));
        }

        // Case 3
        else {
            const int new_offset = event.offset - old_samples;
            const int new_length = event.length;
            adjusted.push_back(NoteEvent(std::move(event.note), new_length, new_offset));
        }
    }
    events = std::move(adjusted);
}


/*static*/ void Program::slowdown(NoteEvents &events, int &new_samples) {
    for(auto &event : events) {
        event.offset = std::round((double)event.offset * cli_args.slowdown_factor);
        event.length = std::round((double)event.length * cli_args.slowdown_factor);
    }

    new_samples = std::round((double)new_samples * cli_args.slowdown_factor);
}


void Program::synthesize_audio(const NoteEvents &notes, const int new_samples) {
    if constexpr(PRINT_AUDIO_UNDERRUNS)
        if(SDL_GetQueuedAudioSize(*out_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8) == 0)
            warning("Audio underrun; no audio left to play");

    // new_samples may be larger due to artificial slowdown; overlapping input buffers may shorten it however
    if(cli_args.do_slowdown) {
        if(new_samples > synth_buffer_n_samples) {
            delete[] synth_buffer;
            synth_buffer = nullptr;  // In case Program is destroyed between delete[] and new float[]

            synth_buffer_n_samples = new_samples;
            try {
                debug("Reallocating synth buffer to accommodate for slowdown");
                synth_buffer = new float[synth_buffer_n_samples];
            }
            catch(const std::bad_alloc &e) {
                error("Failed to allocate new (larger) synth_buffer (" + STR(e.what()) + ")");
                hint("Slowdown factor may be too large, which causes the need for large reallocated synth buffers");
                exit(EXIT_FAILURE);
            }
        }
    }

    synth->synthesize(notes, synth_buffer, new_samples, volume);

    if(!cli_args.stereo_split) {
        if(SDL_QueueAudio(*out_dev, synth_buffer, new_samples * sizeof(float))) {
            error("Failed to queue audio for playback\nSDL error: " + STR(SDL_GetError()));
            exit(EXIT_FAILURE);
        }
    }

    // Waiting till samples are played is done in sync_with_audio()
}


void Program::play_split_audio(const int new_samples) {
    int played = 0;

    float buf[SAMPLES_PER_BUFFER * 2];
    while(played < new_samples) {
        const int block = std::min(new_samples - played, (int)SAMPLES_PER_BUFFER);
        for(int i = 0; i < block; i++) {
            if(cli_args.stereo_split_playback_left) {
                buf[(i * 2) + 0] = playback_buffer[played + i];
                buf[(i * 2) + 1] = synth_buffer[played + i];
            }
            else {
                buf[(i * 2) + 0] = synth_buffer[played + i];
                buf[(i * 2) + 1] = playback_buffer[played + i];
            }
        }

        played += block;

        if(SDL_QueueAudio(*out_dev, buf, block * 2 * sizeof(float))) {
            error("Failed to queue audio for playback\nSDL error: " + STR(SDL_GetError()));
            exit(EXIT_FAILURE);
        }
    }
}


void Program::sync_with_audio(const int new_samples) {
    // If audio out it used, we can make sure the out buffer isn't filled faster than it plays
    if(cli_args.playback || cli_args.synth) {
        // DEBUG: If the while() below works correctly, the out buffer should never be filled faster than it is played
        if(cli_args.audio_input_method == SampleGetters::audio_in && SDL_GetQueuedAudioSize(*out_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8) > (unsigned int)input_buffer_n_samples * 1.9) {
            warning("Audio overrun (too much audio to play); clearing buffer...");
            SDL_ClearQueuedAudio(*out_dev);
        }

        // Wait till one frame is left in systems audio out buffer (needed when fetching samples is faster than playing)
        while(SDL_GetQueuedAudioSize(*out_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8) >= (unsigned int)input_buffer_n_samples && !poll_quit())
            handle_sdl_events();
    }

    // Otherwise, we have to time the number of used samples ourselves to enforce a virtual sample out rate
    else if(cli_args.sync_with_audio) {
        static std::chrono::steady_clock::time_point last_call = std::chrono::steady_clock::now();

        // First call will wait too long, as "last_call" is just set, so return immediately
        // When blocking on audio output, we also return instantly the first call as we wait till one full buffer is left in audio out (which also causes output latency)
        static bool first_call = true;
        if(first_call) {
            first_call = false;
            return;
        }

        // Given that "new_samples" samples were retrieved, we need to pause for this duration in total (since last call)
        while(std::chrono::duration<double>(std::chrono::steady_clock::now() - last_call).count() < (double)new_samples / (double)SAMPLE_RATE && !poll_quit())
            handle_sdl_events();

        last_call = std::chrono::steady_clock::now();
    }
}


void Program::arpeggiate() {
    note_change_time = std::chrono::steady_clock::now() - prev_note_change;
    if(note_change_time.count() > NOTE_TIME) {
        if(!plus_held_down && minus_held_down)
            sample_getter->pitch_down();
        else if(plus_held_down && !minus_held_down)
            sample_getter->pitch_up();

        prev_note_change = std::chrono::steady_clock::now();
    }
}


void Program::handle_sdl_events() {
    // DEBUG
    static int lag = 0;

    SDL_Event e;
    while(SDL_PollEvent(&e) && !poll_quit()) {
        switch(e.type) {
            case SDL_QUIT:
            case SDL_APP_TERMINATING:
                set_quit();
                break;

            case SDL_KEYDOWN:
                switch(e.key.keysym.sym) {
                    case SDLK_q:
                    case SDLK_ESCAPE:
                        set_quit();
                        break;

                    case SDLK_MINUS:
                        if constexpr(ENABLE_ARPEGGIATOR)
                            minus_held_down = true;
                        else
                            sample_getter->pitch_down();
                        break;

                    case SDLK_EQUALS:
                        if constexpr(ENABLE_ARPEGGIATOR)
                            plus_held_down = true;
                        else
                            sample_getter->pitch_up();
                        break;

                    case SDLK_r:
                        graphics->reset_max_recorded_value();
                        if(cli_args.synth)
                            synth->reset_max_amp();
                        break;

                    case SDLK_i:
                        graphics->toggle_show_info();
                        break;

                    case SDLK_t:
                        if(cli_args.playback)
                            SDL_ClearQueuedAudio(*out_dev);
                        break;

                    case SDLK_y:
                        if(cli_args.playback)
                            SDL_ClearQueuedAudio(*in_dev);
                        break;

                    case SDLK_p:
                        estimator->next_plot_type();
                        break;

                    case SDLK_LEFTBRACKET:
                        graphics->add_max_display_frequency(-D_MAX_DISPLAYED_FREQUENCY);
                        break;

                    case SDLK_RIGHTBRACKET:
                        graphics->add_max_display_frequency(D_MAX_DISPLAYED_FREQUENCY);
                        break;

                    case SDLK_SEMICOLON:
                        volume -= D_SYNTH_VOLUME;
                        if(volume < 0.0)
                            volume = 0.0;
                        info("Set synth volume to " + STR(volume));
                        break;

                    case SDLK_QUOTE:
                        volume += D_SYNTH_VOLUME;
                        if(volume > 1.0)
                            volume = 1.0;
                        info("Set synth volume to " + STR(volume));
                        break;

                    case SDLK_COMMA:
                        graphics->zoom(0.5);
                        break;

                    case SDLK_PERIOD:
                        graphics->zoom(2.0);
                        break;

                    // case SDLK_f:
                    //     graphics->toggle_freeze_graph();
                    //     break;

                    // DEBUG
                    case SDLK_s:
                        debug("Creating lag spike");
                        lag += 250;
                        break;
                }
                break;

            case SDL_KEYUP:
                if constexpr(ENABLE_ARPEGGIATOR) {
                    switch(e.key.keysym.sym) {
                        case SDLK_MINUS:
                            minus_held_down = false;
                            break;

                        case SDLK_EQUALS:
                            plus_held_down = false;
                            break;
                    }
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if(e.button.button == SDL_BUTTON_LEFT) {
                    mouse_clicked = true;
                    mouse_x = e.button.x;
                    mouse_y = e.button.y;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if(e.button.button == SDL_BUTTON_LEFT)
                    mouse_clicked = false;
                break;

            case SDL_MOUSEMOTION:
                if(e.motion.state & SDL_BUTTON_LMASK) {
                    mouse_clicked = true;
                    mouse_x = e.motion.x;
                    mouse_y = e.motion.y;
                }
                break;

            case SDL_MOUSEWHEEL:
                // if(cli_args.play_file) {
                if(sample_getter->get_type() == SampleGetters::audio_file) {
                    if(cli_args.output_file) {
                        static bool warning_printed = false;
                        if(!warning_printed) {
                            warning("Can't seek audio while writing results to a file");
                            warning_printed = true;
                        }
                        break;
                    }

                    AudioFile *const audio_in_cast = dynamic_cast<AudioFile *>(sample_getter);
                    if(e.wheel.y > 0)
                        audio_in_cast->seek((int)(SECONDS_PER_SCROLL * SAMPLE_RATE * e.wheel.y));
                    else if(e.wheel.y < 0)
                        audio_in_cast->seek((int)(SECONDS_PER_SCROLL * SAMPLE_RATE * e.wheel.y));
                }
                break;

            case SDL_WINDOWEVENT:
                switch(e.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                        set_quit();
                        break;

                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        resize(e.window.data1, e.window.data2);
                        break;

                    case SDL_WINDOWEVENT_ENTER:
                        int x, y;
                        if(SDL_GetMouseState(&x, &y) & SDL_BUTTON_LMASK) {
                            mouse_clicked = true;
                            mouse_x = x;
                            mouse_y = y;
                        }
                        else
                            mouse_clicked = false;
                        break;

                    case SDL_WINDOWEVENT_LEAVE:
                        mouse_clicked = false;
                        break;
                }
                break;

            case SDL_RENDER_TARGETS_RESET:
            case SDL_RENDER_DEVICE_RESET:
                warning("Graphics had a mishap");
                break;

            // default:
            //     debug("Unhandled event of type " + STR(e.type));
            //     break;
        }
    }

    // DEBUG
    if(lag != 0) {  // Lag might have been increased by in handle_sdl_events()
        debug("Lagging for " + STR(lag) + " ms");
        std::this_thread::sleep_for(std::chrono::milliseconds(lag));
        lag = 0;
    }
}

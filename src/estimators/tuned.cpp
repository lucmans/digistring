
#include "tuned.h"

#include "../error.h"


Tuned::Tuned(float *const input_buffer) {
    error("Tuned class not yet implemented!");

    input_buffer[0] = 0.0;  // prevent warning
}

Tuned::~Tuned() {

}


Estimators Tuned::get_type() const {
    return Estimators::tuned;
}


// TODO
/*static*/ float *Tuned::create_input_buffer(int &buffer_size) {
    error("Tuned class not yet implemented!");
    buffer_size = -1;
    return nullptr;

    // float *input_buffer = (float*)fftwf_malloc(FRAME_SIZE * sizeof(float));
    // if(input_buffer == NULL) {
    //     error("Failed to malloc input buffer");
    //     exit(EXIT_FAILURE);
    // }

    // buffer_size = FRAME_SIZE;
    // return input_buffer;
}

float *Tuned::_create_input_buffer(int &buffer_size) const {
    return Tuned::create_input_buffer(buffer_size);
}

// Implemented by superclass
// void HighRes::free_input_buffer(float *const input_buffer) const {
//     fftwf_free(input_buffer);
// }


void Tuned::perform(float *const input_buffer) {
    // TODO
    warning("Tuned 'perform' not yet implemented");

    input_buffer[0] = 0.0;  // prevent warning
}

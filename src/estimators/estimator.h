#ifndef DIGISTRING_ESTIMATORS_ESTIMATOR_H
#define DIGISTRING_ESTIMATORS_ESTIMATOR_H


#include "note.h"
#include "estimator_graphics/spectrum.h"

#include <SDL2/SDL.h>

#include <map>
#include <string>
#include <vector>


/* When adding a new estimator, don't forget to include the file in estimators.h */
// Different estimator algorithms types
enum class Estimators {
    highres, tuned
};

// For printing enum
// If a string isn't present in EstimatorString for every type, random crashes may happen
const std::map<const Estimators, const std::string> EstimatorString = {{Estimators::highres, "highres"},
                                                                       {Estimators::tuned, "tuned"}};

class EstimatorGraphics;  // Declared below Estimator class in this file
class Estimator {
    public:
        Estimator();
        virtual ~Estimator();

        // This function should only return its type as named in Estimators
        virtual Estimators get_type() const = 0;

        // Functions for retrieving graphics related data
        double get_max_norm() const;

        // The estimator graphics pointer is valid till the next perform() call
        const EstimatorGraphics *get_estimator_graphics();
        void next_plot_type();

        // Actually performs the estimation
        virtual void perform(float *const input_buffer, NoteEvents &note_events) = 0;


    protected:
        // Graphics output related variables, so only available without headless mode
        // These variables are set during perform, so relate to the last perform call
        EstimatorGraphics *estimator_graphics;
        double max_norm;
};


struct GraphicsData {
    double max_display_frequency;
    double max_recorded_value;
};

class EstimatorGraphics {
    public:
        EstimatorGraphics() : cur_plot(0) {};
        virtual ~EstimatorGraphics() {};

        void next_plot() {cur_plot++;};
        virtual void render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data) const = 0;


    protected:
        // Mutable, so Estimator can return a const EstimatorGraphics pointer, so that the data used for graphics can only be altered by an Estimator
        // cur_plot should only be changed in the const member function render() to set it back to 0 if value is invalid
        mutable int cur_plot;
};


#endif  // DIGISTRING_ESTIMATORS_ESTIMATOR_H

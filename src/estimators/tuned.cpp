
#include "tuned.h"

#include "../error.h"


Tuned::Tuned() {

}

Tuned::~Tuned() {

}


Estimators Tuned::get_type() const {
    return Estimators::tuned;
}


float *Tuned::get_input_buffer() const {
    return in;
}

float *Tuned::get_input_buffer(int &buffer_size) const {
    // TODO
    buffer_size = -1;

    return in;
}


void Tuned::perform() {
    // TODO
    info("Not yet implemented");
}

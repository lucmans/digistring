
#include "sample_getter.h"

#include "../config.h"


SampleGetter::SampleGetter() {
    played_samples = 0;
}

SampleGetter::~SampleGetter() {

}


unsigned long SampleGetter::get_played_samples() const {
    return played_samples;
}

double SampleGetter::get_played_time() const {
    return (double)played_samples / (double)SAMPLE_RATE;
}

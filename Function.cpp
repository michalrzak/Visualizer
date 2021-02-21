#include "Function.h"
#include <cmath>

Function::Function(double sample_rate): sample_rate{sample_rate} {current_sample = 0;}

double Function::next_sample() {
    //temporary with set function
    current_sample+=sample_rate;
    return std::sin(current_sample*current_sample/2.3);
    //return current_sample*current_sample;
}

double Function::get_sample_rate() {
    return sample_rate;
}

void Function::set_sample_rate(double sample_rate) {
    this->sample_rate = sample_rate;
}

void Function::set_current_sample(double new_sample) {
    current_sample = new_sample;
}

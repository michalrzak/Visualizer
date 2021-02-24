#include "Function.h"
#include <cmath>
#include <iostream>

Function::Function(double sample_rate): sample_rate{sample_rate} {current_sample = 0;}

double Function::next_sample() {
    //temporary with set function
    current_sample+=sample_rate;
    //return std::sin(current_sample*current_sample);
    //return current_sample*current_sample;
    //return std::sin(current_sample);
    //return 1/current_sample;
    if (!std::tan(current_sample))
        std::cout << "I was 0\n";
    return std::tan(current_sample);
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

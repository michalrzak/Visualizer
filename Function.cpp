#include "Function.h"
#include <cmath>
#include <iostream>

Function::Function(double sample_rate): sample_rate{sample_rate} {current_sample = 0;}

/*Function::Function(double sample_rate, std::string function): sample_rate {sammple_rate}, function {function} {
    current_sample = 0;
    
    std::string num1;
    bool dot {false};
    for (char ele : function) {
        if (ele >= '0' && ele <= '9')
            num1 += ele;
        else if (ele == '.' && !dot) {
            dot = true;
            num1 += ele;
        }
        
        else if ()
    }
}*/



double Function::next_sample() {
    //temporary with set function
    current_sample+=sample_rate;
    //return std::sin(current_sample*current_sample);
    //return current_sample*current_sample;
    //return std::sin(current_sample);
    //return 1/current_sample;
    //return std::tan(current_sample);
    return std::sin(current_sample) * std::sin(current_sample);
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

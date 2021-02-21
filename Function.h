#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>

//TODO: Rename this
class Function {
    
    std::string function; //funcito; unused for now
    double sample_rate;
    double current_sample;
    
public:
    Function(double sample_rate);
    
    double next_sample();
    
    void set_sample_rate(double);
    double get_sample_rate();
    
    void set_current_sample(double);
};

#endif

#ifndef SIMPLEPOLGENERATOR_HPP
#define SIMPLEPOLGENERATOR_HPP

#include <vector>
#include <fstream>
#include "bounds.hpp"
#include "statistics.hpp"


class SimplePolGenerator
{
    unsigned long num_of_coefficients;
    std::vector<long> divisors;
    std::vector<long> avalues;
    std::vector<long> svalues;
    long discriminant;
    unsigned int fixed;
    unsigned int locked;
    BoundGenerator *bounds;
    
    bool next(unsigned int level);
    bool init_value(unsigned int level);
    bool set_value(unsigned int level, long value);
    bool increase_value(unsigned int level);
    
public:
    SimplePolGenerator(std::vector<long> const &coefficients,
        unsigned long const degree, std::vector<long> const &divisors,
        long const discriminant, Statistics *stats);
    ~SimplePolGenerator();
    bool init();
    bool configure_next();
    void get_coefficients(long container[]);
};

#endif // POLGENERATOR_HPP 

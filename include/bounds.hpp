#ifndef BOUNDS_HPP
#define BOUNDS_HPP
 
#include <utility>
#include <vector>
#include <fstream>
#include "gmp.h"
#include "statistics.hpp"

class BoundGenerator
{
private:
    std::vector<long> avalues;
    std::vector<long> svalues;
    std::vector<long> alower;
    std::vector<long> aupper;
    std::vector<long> slower;
    std::vector<long> supper;
    std::vector<long> lagrange_bounds;
    std::vector<long> lagrange_bounds_neg;
    std::vector<long> coeff_sums;
    std::vector<long> alt_coeff_sums;
    long a1old;
    long anold;
    long discriminant;
    std::vector<long> divisors;
    double t2_d;
    std::vector<long> eval_bounds;
    long bound_last;
    Statistics *stats;

    void compute_lagrange_bounds();
    double bound_constant();
    void calc_t2();
    void update_bounds(unsigned int level);

public:
    BoundGenerator(long discriminant, std::vector<long> &divisors,
        Statistics *stats);
    ~BoundGenerator();
    void set_values(unsigned int level, long avalue, long svalue);
    long get_alower(unsigned int level);
    long get_aupper(unsigned int level);
    long get_slower(unsigned int level);
    long get_supper(unsigned int level);
};
 
#endif // BOUNDS_HPP
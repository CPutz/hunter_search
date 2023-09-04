#ifndef POLGENERATOR_HPP
#define POLGENERATOR_HPP

#include <vector>
#include <queue>
#include <fstream>
#include "gmp.h"
#include "congruencelookup.hpp"
#include "congruencegenerator.hpp"
#include "bounds.hpp"
#include "hornertree.hpp"
#include "statistics.hpp"

class PolGenerator
{
    mpz_t tmp_z; // gmp integer used for intermediate calculations
    std::vector<long> divisors; // divisor for each a_k (A_k)
    std::vector<std::vector<long>> divisors_gcd;
    std::vector<long> avalues; // a_k values
    std::vector<long> svalues; //s_k for k = 0,1,..,DEGREE-3,-2,-1
    long discriminant; // the desired discriminant
    unsigned int locked; // number of locked coefficients
    unsigned long prod;
    std::vector<long> crt_coeffs;
    HornerTree *disctree;
    std::vector<CongruenceLookup *> congruencelookup;
    std::vector<CongruenceGenerator *> congruencegenerators;
    std::vector<std::vector<long>> moduli;
    std::queue<long> last_values;
    BoundGenerator *bounds;
    Statistics *stats;

    void eval_disc(unsigned int level);
    bool next(unsigned int level);
    bool set_value(unsigned int level, long value);
    bool init_value(unsigned int level);
    bool increase_value(unsigned int level);
    
public:
    PolGenerator(std::vector<long> const &divisors,
        long const discriminant, std::fstream &treestream,
        std::vector<std::vector<std::vector<long>>> const &congruences,
        std::vector<std::vector<long>> const &moduli,
        std::vector<long> const &primes, Statistics *stats);
    ~PolGenerator();
    bool init(std::vector<long> const &coefficients);
    bool configure_next();
    void get_coefficients(long container[]);
    bool disc_is_square();
};

#endif // POLGENERATOR_HPP 

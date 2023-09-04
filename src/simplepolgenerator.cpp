/*
A simplified version of PolGenerator. It generates tuples for the first
k coefficients (where k is called num_of_coefficients in this class). During generation,
it does not take congruences or the value of the discriminant up to squares
into account.
*/

#include <algorithm>
#include <vector>
#include <iostream>
#include <math.h>
#include "simplepolgenerator.hpp"
#include "mathextra.hpp"
#include "order.hpp"

/*
 * Initialize simplified polynomial generator
 */
SimplePolGenerator::SimplePolGenerator(std::vector<long> const &fixedcoefficients,
    unsigned long const num_of_coefficients, std::vector<long> const &divisors,
    long const discriminant, Statistics *stats) {

    this->num_of_coefficients = num_of_coefficients;
    this->avalues = std::vector<long>(DEGREE+1);
    this->svalues = std::vector<long>(DEGREE+1);
    this->divisors = divisors;
    this->discriminant = discriminant;
    this->bounds = new BoundGenerator(this->discriminant, this->divisors, stats);

    // Initialize the fixed coefficients
    svalues[0] = DEGREE;
    avalues[0] = 1;
    this->fixed = fixedcoefficients.size();
    for (int i = 1; i <= this->fixed; i++) {
        set_value(i, fixedcoefficients[i-1]);
    }
}

SimplePolGenerator::~SimplePolGenerator() { }

/*
 * Initialize generator to starting values
 */
bool SimplePolGenerator::init() {
    bool res = true;
    for (unsigned int i = this->fixed + 1; i <= this->num_of_coefficients && res; i++) {
        res = res && init_value(i);
    }

    return res;
}

/*
 * Initializes a_{get_degree(level)} using the bounds on a_{get_degree(level)}
 */
bool SimplePolGenerator::set_value(unsigned int level, long value) {
    bool res = true;
    if (get_degree(level) == DEGREE) {
        long m = mod(-value, divisors[level]);
        avalues[level] = value + m;

        res = avalues[level] == value;
    } else {
        unsigned int deg = get_degree(level);
        long sum = 0;
        // Compute the sum from Newton's identities
        for (unsigned int i = 1; i < deg; i++) {
            sum -= avalues[get_index(i)] * svalues[get_index(deg-i)];
        }

        long m = deg * divisors[level];
        svalues[level] = value + mod(sum - value, m);
        avalues[level] = (sum - svalues[level]) / deg;

        res = svalues[level] == value;
    }

    this->bounds->set_values(level, avalues[level], svalues[level]);

    return res && (avalues[level] % divisors[level] == 0);
}

/*
 * Initialize a_{get_degree(level)} to its minimual value
 */
bool SimplePolGenerator::init_value(unsigned int level) {
    if (get_degree(level) == DEGREE) {
        set_value(level, this->bounds->get_alower(level));
        return avalues[level] <= this->bounds->get_aupper(level);
    } else {
        set_value(level, this->bounds->get_slower(level));
        return svalues[level] <= this->bounds->get_supper(level);
    }
}

/*
 * Increase the value of a_{get_degree(level)} by the minimal amount
 */
bool SimplePolGenerator::increase_value(unsigned int level) {
    if (get_degree(level) == DEGREE) {
        avalues[level] += divisors[level];
        // Skip a_{DEGREE} = 0
        if (avalues[level] == 0) {
            avalues[level] += divisors[level];
        }
        if (avalues[level] > this->bounds->get_aupper(level)) {
            return false;
        }
    } else {
        unsigned int deg = get_degree(level);
        svalues[level] += deg * divisors[level];
        if (svalues[level] > this->bounds->get_supper(level)) {
            return false;
        }
        avalues[level] -= divisors[level];
    }

    this->bounds->set_values(level, avalues[level], svalues[level]);
    return true;
}

/*
 * Recursively generate the next configuration of coefficients.
 * Returns false if no new configuration exists.
 */
bool SimplePolGenerator::next(unsigned int level) {
    if (!increase_value(level)) {
        //if we reached the last non-locked level we are done
        if (level - 1 <= this->fixed) {
            return false;
        }
        if (!next(level - 1) || !init_value(level)) {
            return false;
        }
    }

    return true;
}

/*
 * Generate the next configuration of coefficients.
 * Returns false if no new configuration exists.
 */
bool SimplePolGenerator::configure_next() {
    return next(this->num_of_coefficients);
}

/*
 * Stores the first num_of_coefficients coefficients.
 * They are stored using the internal ordering of coefficients (a_1, a_n, a_2, ...)
 */
void SimplePolGenerator::get_coefficients(long container[]) {
    for (int i = 1; i <= this->num_of_coefficients; i++) {
        if (get_degree(i) == DEGREE) {
            container[i-1] = avalues[i];
        } else {
            container[i-1] = svalues[i];
        }
    }
}


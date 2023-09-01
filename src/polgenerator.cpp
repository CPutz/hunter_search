/*
This file defines a polynomial generator which performs the Hunter search enumeration.
During generation, it takes congruences on the coefficients into account (which can be
passed to the generator on initialization). Moreover, it checks whether the value of
the discriminant is correct up to squares.
*/

#include <algorithm>
#include <vector>
#include <iostream>
#include <iterator>
#include <math.h>
#include "polgenerator.hpp"
#include "mathextra.hpp"
#include "congruencegenerator.hpp"
#include "order.hpp"

PolGenerator::PolGenerator(std::vector<long> const &fixedcoefficients,
        std::vector<long> const &divisors,
        long const discriminant, std::fstream &treestream,
        std::vector<std::vector<std::vector<long>>> const &congruences,
        std::vector<std::vector<long>> const &moduli,
        std::vector<long> const &filter_primes, Statistics *stats) {

    this->avalues = std::vector<long>(DEGREE+1);
    this->svalues = std::vector<long>(DEGREE+1);
    this->divisors = divisors;
    this->moduli = std::vector<std::vector<long>>(moduli);
    this->discriminant = discriminant;
    this->disctree = new HornerTree(treestream);
    this->fixed = DEGREE - this->disctree->get_depth() + 1;
    this->bounds = new BoundGenerator(this->discriminant, this->divisors, stats);
    this->stats = stats;

    // Initialize the fixed coefficients
    std::copy(fixedcoefficients.begin(), fixedcoefficients.end(), svalues.begin());
    this->fixedsize = fixedcoefficients.size();
    svalues[0] = DEGREE;
    avalues[0] = 1;
    for (int i = 1; i < this->fixedsize; i++) {
        set_value(i, fixedcoefficients[i]);
    }
    mpz_init(this->tmp_z);

    // Create congruence lookup trees from congruence lists
    for (int i = 0; i < moduli.size(); i++) {
        this->congruencelookup.push_back(new CongruenceLookup(congruences[i], moduli[i]));
    }

    // Create congruence lookup trees for extra filtered primes
    for (int p : filter_primes) {
        int num_coefficients = DEGREE - this->fixed + 1;
        std::vector<long> moduli_p(num_coefficients);
        for (unsigned int i = 0; i < num_coefficients; i++) {
            moduli_p[i] = p;
        }
        this->moduli.push_back(moduli_p);
        this->congruencegenerators.push_back(
            new CongruenceGenerator(treestream, p));
        this->congruencelookup.push_back(nullptr);
    }

    for (std::vector<long> moduli_p : this->moduli) {
        long mod_lcm = 1;
        for (long m : moduli_p) {
            mod_lcm = lcm(mod_lcm, m);
        }

        std::vector<long> gcds = std::vector<long>();
        for (int i = 0; i < moduli_p.size(); i++) {
            long div = divisors[i+this->fixed];
            long g = gcd(div / g, mod_lcm);
            long gd = 1;
            do {
                gd *= g;
                g = gcd(div / gd, mod_lcm);
            } while (g != 1);
            gcds.push_back(gd);
        }
        this->divisors_gcd.push_back(gcds);
    }

    this->prod = 1;
    for (unsigned int i = 0; i < this->moduli.size(); i++) {
        this->prod *= this->moduli[i].back() * this->divisors_gcd[i].back();
    }

    // Create Chinese remainder theorem coefficients
    for (unsigned int i = 0; i < this->moduli.size(); i++) {
        long mi = this->moduli[i].back() * divisors_gcd[i].back();
        long c = divisors_gcd[i].back();
        for (unsigned int j = 0; j < this->moduli.size(); j++) {
            if (j != i) {
                long mj = this->moduli[j].back() * divisors_gcd[j].back();
                c *= mj * inverse_mod(mj, mi);
            }
        }
        this->crt_coeffs.push_back(mod(c,this->prod));
    }
}

PolGenerator::~PolGenerator() {
    delete disctree;
    //delete crttree;

    mpz_clear(tmp_z);

    for (auto gen : congruencegenerators) {
        delete gen;
    }
    for (auto tree : congruencelookup) {
        delete tree;
    }
}

/*
 * Evaluates the disciminant for a_i where i = get_degree(level)
 */
void PolGenerator::eval_disc(unsigned int level) {
    this->stats->start_timer_all_stats(ST_EVAL);
    mpz_set_si(tmp_z, avalues[level]);
    this->disctree->eval(DEGREE - level, tmp_z);
    this->stats->stop_timer_all_stats(ST_EVAL);
}

/*
 * Initialize generator to starting values
 */
bool PolGenerator::init(std::vector<long> const &coefficients) {
    this->stats->start_timer_all_stats(ST_INIT);
    this->locked = this->fixedsize + coefficients.size();
    for (unsigned int i = this->fixed; i < this->locked; i++) {
        if (!set_value(i, coefficients[i-this->fixed])) {
            std::cout << "this should not happen: " << i << std::endl;
            this->stats->stop_timer_all_stats(ST_INIT);
            return false;
        }
    }

    for (unsigned int i = locked; i < DEGREE; i++) {
        init_value(i);
    }


    //initialize all congruence lookup trees
    int index_shift = moduli.size() - congruencegenerators.size();
    for (unsigned int i = 0; i < this->congruencelookup.size(); i++) {
        //first create the tree for extra filtered primes
        if (i >= index_shift) {
            std::vector<long> locked_coeffs = std::vector<long>();
            for (unsigned int i = this->fixed; i < this->locked; i++) {
                locked_coeffs.push_back(avalues[i]);
            }
            CongruenceGenerator *gen = this->congruencegenerators[i - index_shift];
            if (this->congruencelookup[i]) {
                delete this->congruencelookup[i];
            }
            std::vector<std::vector<long>> con = gen->generate_congruences(locked_coeffs);
            this->congruencelookup[i] = new CongruenceLookup(con, moduli[i]);
        }

        this->congruencelookup[i]->reset();
        //walk to the position given by the locked coefficients
        for (unsigned int j = this->fixed; j < this->locked; j++) {
            if (!this->congruencelookup[i]->walk(
                    avalues[j] / divisors_gcd[i][j-this->fixed])) {
                //cancel if no combination exists for the locked coefficients mod p
                this->stats->stop_timer_all_stats(ST_INIT);
                return false;
            }
        }

        for (unsigned int j = this->locked; j < DEGREE; j++) {
            this->congruencelookup[i]->walk(
                avalues[j] / divisors_gcd[i][j-this->fixed]);
        }
    }

    //evalulate discriminant for the locked coefficients
    for (unsigned int i = this->fixed; i <= DEGREE-2; i++) {
        eval_disc(i);
    }

    this->last_values = std::queue<long>();
    this->stats->stop_timer_all_stats(ST_INIT);

    //find first configuration of a1,...,a_{n-1} such that there are
    //possible values for a_n
    return configure_next();
}

/*
 * Sets the value of s_i (0 < i < n) or a_n using to a given value.
 * Here i = get_index(level).
 * If the value of s_i or a_n is impossible, then they are set to the
 * next lowest values. This may happen if a_i must be divisible by some
 * positive divisor, or because the Newton relations must hold.
 * Returns true if the value set in the end is exactly equal to the
 * input value which was given.
 */
bool PolGenerator::set_value(unsigned int level, long value) {
    bool res = true;
    if (get_degree(level) == DEGREE) {
        long m = mod(-value, divisors[level]);
        avalues[level] = value + m;
        if (avalues[level] == 0) {
            avalues[level] += divisors[level];
        }

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
 * Initializes s_i and a_i (0 < i < n) or a_n using the bounds on s_i and a_n.
 * Here i = get_index(level).
 * Returns true if the generated values for s_i and a_i satisfy all bounds
 * (it is possible that s_i and a_i cannot both satisfy their bounds
 * given the previous a_1,...,a_i).
 */
bool PolGenerator::init_value(unsigned int level) {
    int deg = get_degree(level);
    if (deg == DEGREE) {
        set_value(level, this->bounds->get_alower(level));
        return avalues[level] <= this->bounds->get_aupper(level);
    } else {
        set_value(level, this->bounds->get_slower(level));
        // Correct a_level if its value does not satisfy the upper bound
        if (avalues[level] > this->bounds->get_aupper(level)) {
            long diff = avalues[level] - this->bounds->get_aupper(level);
            long step = (long)ceil((double)diff / divisors[level]);
            avalues[level] -= step * divisors[level];
            svalues[level] += step * divisors[level] * deg;
        }
        return svalues[level] <= this->bounds->get_supper(level);
    }
}

/*
 * Increases the values of s_i (0 < i < n) or a_n using by the minimal amount.
 * Here i = get_index(level). The corresponding value of a_i is updated accordingly.
 * Returns true if s_i and a_i satisfy their bounds after updating
 */
bool PolGenerator::increase_value(unsigned int level) {
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
        avalues[level] -= divisors[level];
        if (svalues[level] > this->bounds->get_supper(level) ||
            avalues[level] < this->bounds->get_alower(level)) {
            return false;
        }
    }

    this->bounds->set_values(level, avalues[level], svalues[level]);
    
    return true;
}

/*
 * Generates the next configuration of a_{get_degree(1)},...,a_{get_degree(level)}
 */
bool PolGenerator::next(unsigned int level) {
    bool cancelled;

    do {
        cancelled = false;
        //set all congruence lookup trees one level back
        for (CongruenceLookup *t : this->congruencelookup) {
            t->back();
        }

        if (!increase_value(level)) {
            //if we reached the last non-locked level we are done
            if (level <= locked) {
                return false;
            }

            do {
                if (!next(level-1)) {
                    return false;
                }
            } while (!init_value(level));
        }

        //for almost every a_{level} where level <= DEGREE-2, there exists possibilities
        //for (a_{level},...,a_DEGREE), so we already evaluate the discriminant at this
        //point
        if (level <= DEGREE-2) {
            eval_disc(level);
        }

        //walk one level deeper in all congruence lookup trees
        for (int i = 0; i < moduli.size(); i++) {
            if (cancelled) {
                this->congruencelookup[i]->walk_dummy();
            } else {
                cancelled |= !this->congruencelookup[i]->walk(
                    avalues[level] / divisors_gcd[i][level-this->fixed]);
            }
        }
    } while (cancelled);
    return true;
}

/*
 * Generates the next polynomial.
 * Returns false once all polynomials have been enumerated.
 */
bool PolGenerator::configure_next() {
    while (last_values.size() == 0 && next(DEGREE-1)) {
        this->stats->start_timer_all_stats(ST_CRT);
        long low = bounds->get_alower(get_index(DEGREE-1));
        long upp = bounds->get_aupper(get_index(DEGREE-1));

        if (low <= upp) {
            // Apply Chinese remainder theorem
            this->last_values.push(0);
            for (unsigned int i = 0; i < this->congruencelookup.size(); i++) {
                unsigned int size = this->last_values.size();
                for (unsigned int j = 0; j < size; j++) {
                    long t = this->last_values.front();
                    this->last_values.pop();
                    for (long c : *this->congruencelookup[i]->get_values()) {
                        this->last_values.push(t + this->crt_coeffs[i] * c);
                    }
                }
            }

            // Convert values modulo this->prod to global values
            unsigned int size = this->last_values.size();
            for (unsigned int i = 0; i < size; i++) {
                long c = this->last_values.front();
                this->last_values.pop();
                for (int v = low + mod(c - low, this->prod); v < upp; v += this->prod) {
                    this->last_values.push(v);
                }
            }

            if (last_values.size() > 0) {
                eval_disc(DEGREE-1);
            }
        }

        this->stats->stop_timer_all_stats(ST_CRT);
    }
    

    if (last_values.size() > 0) {
        avalues[DEGREE] = last_values.front();
        eval_disc(DEGREE);
        last_values.pop();
        return true;
    }
    return false;
}

/*
 * Stores the coefficients of the current polynomial coefficients in
 * descending order of degree
 */
void PolGenerator::get_coefficients(long container[]) {
    for (int i = 0; i <= DEGREE; i++) {
        container[i] = avalues[get_index(i)];
    }
}

/*
 * Returns if the discriminant of the current polynomial is a square
 * times the desired discriminant for the number field
 */
bool PolGenerator::disc_is_square() {
    this->disctree->get_val(tmp_z);

    //if the discriminant is 0 we discard the polynomial
    if (mpz_sgn(tmp_z) == 0) {
        return 0;
    }

    return mpz_perfect_square_p(tmp_z);
}

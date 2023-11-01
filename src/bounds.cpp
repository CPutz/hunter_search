/*
This file defines a BoundGenerator class which can compute all bounds which are
used during the Hunter search.
*/

#include <cmath>
#include <sstream>
#include <string>
#include <iostream>
#include <limits>
#include "bounds.hpp"
#include "order.hpp"

BoundGenerator::BoundGenerator(long discriminant, std::vector<long> &divisors,
    Statistics *stats) {

    this->discriminant = discriminant;
    this->divisors = divisors;
    this->stats = stats;

    this->lagrange_bounds = std::vector<long>(DEGREE+1);
    this->lagrange_bounds_neg = std::vector<long>(2+1);
    this->eval_bounds = std::vector<long>(3);
    this->avalues = std::vector<long>(DEGREE+1);
    this->svalues = std::vector<long>(DEGREE+1);
    this->alower = std::vector<long>(DEGREE+1);
    this->aupper = std::vector<long>(DEGREE+1);
    this->slower = std::vector<long>(DEGREE+1);
    this->supper = std::vector<long>(DEGREE+1);
    this->coeff_sums = std::vector<long>(DEGREE);
    this->alt_coeff_sums = std::vector<long>(DEGREE);
    this->a1old = std::numeric_limits<long>::max();
    this->anold = std::numeric_limits<long>::max();

    this->avalues[0] = 1;
    this->svalues[0] = DEGREE;
    for (unsigned int i = 1; i <= DEGREE; i++) {
        this->avalues[i] = std::numeric_limits<long>::max();
        this->svalues[i] = std::numeric_limits<long>::max();
    }

    // Init bounds for a_1 and s_1
    this->alower[get_index(1)] = 0;
    this->aupper[get_index(1)] =  DEGREE / 2;
    this->slower[get_index(1)] = -DEGREE / 2;
    this->supper[get_index(1)] = 0;

    this->coeff_sums[0] = 1;
    this->alt_coeff_sums[0] = DEGREE % 2 == 0 ? 1 : -1; //(-1)^DEGREE
}
 
BoundGenerator::~BoundGenerator() { }

/*
 * Compute bounds from Lagrange multipliers, given a_1 and a_n
 */
void BoundGenerator::compute_lagrange_bounds() {
    // Update Lagrange bounds if a_1 or a_n changed
    if (this->a1old != this->avalues[get_index(1)] ||
        this->anold != this->avalues[get_index(DEGREE)]) {

        this->a1old = this->avalues[get_index(1)];
        this->anold = this->avalues[get_index(DEGREE)];
 
        double eps = 0.00001;
        double an = std::abs((double)(avalues[get_index(DEGREE)])); 
 
#if DEGREE == 8 //optimization for n = 8
    double r = this->t2_d / sqrt(sqrt(an));
#else
    double r = this->t2_d / pow(an, 2.0 / DEGREE);
#endif

        std::vector<double> roots = std::vector<double>();
        //compute roots
        for (int m = 1; m < DEGREE; m++) {
            double z = pow(m / r, 1.0 / (DEGREE - m));
            //Newton iteration
            double delta = 0;
            do {
                double Rz = m * pow(z, (double)m - DEGREE) + (DEGREE - m) * pow(z, (double)m);
                double Rdz = (m - DEGREE) * m * (pow(z, (double)m - DEGREE - 1) - pow(z, (double)m - 1));
                delta = (Rz - r) / Rdz;
                z = z - delta;
            } while (std::abs(delta) > eps);
            roots.push_back(z);
        }

        this->lagrange_bounds[0] = DEGREE;
        this->lagrange_bounds[1] = abs(this->svalues[get_index(1)]);
        this->lagrange_bounds[2] = floor(this->t2_d);
        // Lagrange bounds for s_3,...,s_{n-1}
        for (int k = 3; k < DEGREE; k++) {
            double tk = 0;
            for (int m = 1; m < DEGREE; m++) {
                double zm = roots[m-1];
                double t = m * pow(zm, (double)k * (m - DEGREE) / 2.0) + (DEGREE - m) * pow(zm, (double)k * m / 2.0);
                if (tk < t) {
                    tk = t;
                }
            }
            tk *= pow(an, (double)k / DEGREE);
            this->lagrange_bounds[k] = floor(tk);
        }

        this->lagrange_bounds_neg[0] = DEGREE;
        // Lagrange bounds for s_{-1}a_n and s_{-2}a_n^2
        for (int k = -1; k >= -2; k--) {
            double tk = 0;
            for (int m = 1; m < DEGREE; m++) {
                double zm = roots[m-1];
                double t = m * pow(zm, (double)k * (m - DEGREE) / 2.0) + (DEGREE - m) * pow(zm, (double)k * m / 2.0);
                if (tk < t) {
                    tk = t;
                }
            }
            tk *= pow(an, -k) * pow(an, (double)k / DEGREE);
            this->lagrange_bounds_neg[-k] = floor(tk);
        }
    }
}
 
/*
 * Returns the part Hunter's bound which is independent of a_1
 */
double BoundGenerator::bound_constant() {
    double bc;
    const double pi = 3.1415926535;

#if DEGREE == 5
    bc = pow(4 * std::abs((double)this->discriminant) / DEGREE, 1.0 / (DEGREE - 1));
#elif DEGREE = 6
    bc = pow(8 * std::abs((double)this->discriminant) / DEGREE, 1.0 / (DEGREE - 1));
#elif DEGREE == 7
    bc = pow(64 / 3.0 * std::abs((double)this->discriminant) / DEGREE, 1.0 / (DEGREE - 1));
#elif DEGREE == 8
    bc = pow(64 * std::abs((double)this->discriminant) / DEGREE, 1.0 / (DEGREE - 1));
#elif DEGREE == 10
    bc = 1.0 / pi * pow(108056025 / 8.0 * std::abs((double)this->discriminant) / DEGREE, 1.0 / (DEGREE - 1));
#elif DEGREE == 11
    bc = 1.0 / pi * pow(530841600 * std::abs((double)this->discriminant) / DEGREE, 1.0 / (DEGREE - 1));
#else
    #error Bound constant not implemented for this degree
#endif

    return bc;
}
 
/*
 * Recalculates the value of t_2 (depending on a_1)
 */
void BoundGenerator::calc_t2() {
    long a1 = avalues[get_index(1)];
    double bc = this->bound_constant();
    this->t2_d = ((double)a1 * a1) / DEGREE + bc;
}

/*
 * Get the lower bound for a_i where i is the index corresponding to level
 */
long BoundGenerator::get_alower(unsigned int level) {
    return this->alower[level];
}

/*
 * Get the upper bound for a_i where i is the index corresponding to level
 */
long BoundGenerator::get_aupper(unsigned int level) {
    return this->aupper[level];
}

/*
 * Get the lower bound for s_i where i is the index corresponding to level
 */
long BoundGenerator::get_slower(unsigned int level) {
    return this->slower[level];
}

/*
 * Get the upper bound for s_i where i is the index corresponding to level
 */
long BoundGenerator::get_supper(unsigned int level) {
    return this->supper[level];
}

/*
 * Update the values of a_i and s_i and update all relevant bounds
 */
void BoundGenerator::set_values(unsigned int level, long avalue, long svalue) {
    this->coeff_sums[level] = this->coeff_sums[level-1] + avalue;
    long sgn = (DEGREE - get_degree(level)) % 2 == 0 ? 1 : -1;
    this->alt_coeff_sums[level] = this->alt_coeff_sums[level-1] + sgn * avalue;

    if (get_degree(level) == 1 && avalues[level] != avalue) {
        this->avalues[level] = avalue;
        this->svalues[level] = svalue;
        calc_t2();
    } else {
        this->avalues[level] = avalue;
        this->svalues[level] = svalue;
    }

    if (get_degree(level) == DEGREE) {
        this->stats->start_timer_all_stats(ST_LAGRANGE);
        this->compute_lagrange_bounds();
        this->stats->stop_timer_all_stats(ST_LAGRANGE);
    }

    //this->stats->start_timer_all_stats(ST_BOUNDS);
    update_bounds(level);
    //this->stats->stop_timer_all_stats(ST_BOUNDS);
}

/*
 * Update all bounds after a_{get_index(level)} changed
 */
void BoundGenerator::update_bounds(unsigned int level) {
    long max = std::numeric_limits<long>::max();
    long min = std::numeric_limits<long>::min();
    const long n = DEGREE;
    unsigned int deg = get_degree(level);
 
    if (deg == 1) { // Update bounds for a_n
        for (int k = -1; k <= 1; k++) {
            long b = (long)floor(pow((n * k * k - 2 * k * this->svalues[get_index(1)] + this->t2_d) / n, (double)n / 2));
            this->eval_bounds[k+1] = b;
        }
        //|a_n| <= (t_2/n)^(n/2)
        this->alower[get_index(n)] = -this->eval_bounds[0+1];
        this->aupper[get_index(n)] =  this->eval_bounds[0+1];

        //|a_{n-1}| <= n * (t_2/(n-1))^((n-1)/2) and |a_{n-1}| = a_n * s_{-1}
        this->bound_last = std::min(this->lagrange_bounds_neg[1], (long)floor(n * pow(this->t2_d / (n-1), (double)(n-1) / 2)));
    } else if (deg == n) { // Update bounds for s_2,...,s_{n-1}
        for (unsigned int d = 2; d <= n-1; d++) {
            long upp = this->lagrange_bounds[d];
            this->slower[get_index(d)] = -upp;
            this->supper[get_index(d)] =  upp;
            this->alower[get_index(d)] = min;
            this->aupper[get_index(d)] = max;
        }
 
        //(2/n)s_1^2 - t_2 <= s_2 <= t_2
        this->slower[get_index(2)] = (long)ceil(2.0 / n * this->svalues[get_index(1)] - this->t2_d);
        
        //if s_1 = 0 then take s_3 >= 0
        if (this->svalues[get_index(1)] == 0) {
            this->slower[get_index(3)] = 0;
        }
 
        //2a_{n-2}a_n >= -(a_n)^2*s_{-2}
        long an = avalues[get_index(n)];
        long b = this->lagrange_bounds_neg[2]; // Bound on |(a_n)^2*s_{-2}|
        if (an > 0) {
            this->alower[get_index(n-2)] = (long)ceil(-(double)b / (2 * an));
        } else {
            this->aupper[get_index(n-2)] = (long)floor(-(double)b / (2 * an));
        }
    } else if (deg == 3) { // Update bounds for s_4
        //s_4 >= 2s_3^2 / (t_2 + s_2) - (t_2 - s_2)^2
        double diff = t2_d - this->svalues[get_index(2)];
        double b = (double)2 * this->svalues[get_index(3)] * this->svalues[get_index(3)] / (this->t2_d + this->svalues[get_index(2)]) - diff * diff;
        this->slower[get_index(4)] = std::max(this->slower[get_index(4)], (long)ceil(b));
    } else if (deg == n-2) { // Update bounds for a_{n-1}
        //Bounds for a_{n-1} that does not depend on a_{n-2}
        //|a_{n-1}| <= n * (t2/(n-1))^((n-1)/2) and |a_{n-1}| = a_n * s_{-1}
        long alow = -this->bound_last;
        long aupp =  this->bound_last;
        
        /*long newton_sum = 0;
        for (unsigned int i = 1; i < n-1; i++) {
            newton_sum -= avalues[get_index(i)] * svalues[get_index(n-1-i)];
        }
        //bound on s_{n-1}
        alow = (long)ceil( (float)(newton_sum + this->slower[get_index(n-1)]) / (n - 1));
        aupp = (long)floor((float)(newton_sum + this->supper[get_index(n-1)]) / (n - 1));*/

        long newton_sum = 0;
        for (unsigned int i = 1; i < n-1; i++) {
            newton_sum += avalues[get_index(i)] * svalues[get_index(n-1-i)];
        }
        //bound on s_{n-1}
#if (DEGREE - 1) % 2 == 0
        alow = (long)ceil( (float)(-newton_sum - this->supper[get_index(n-1)]) / (n - 1));
        aupp = (long)floor((float)(-newton_sum - this->slower[get_index(n-1)]) / (n - 1));
#else
        alow = (long)ceil( (float)(newton_sum + this->slower[get_index(n-1)]) / (n - 1));
        aupp = (long)floor((float)(newton_sum + this->supper[get_index(n-1)]) / (n - 1));
#endif

        //a_{n-1}^2 = 2a_{n-2}a_n + (a_n)^2s_{-2}
        long bm2 = this->lagrange_bounds_neg[2];
        long b = (long)floor(sqrt((double)std::max((long)0, 2 * avalues[get_index(n-2)] * avalues[get_index(n)] + bm2)));
        alow = std::max(alow, -b);
        aupp = std::min(aupp,  b);

        //|a_n + a_{n-1} + ... + a_0| <= ((n - 2s_1 + t_2) / n)^(n/2)
        long sum = this->coeff_sums[level];
        b = this->eval_bounds[2]; //((n - 2s_1 + t_2) / n)^(n/2)
        alow = std::max(alow, -sum - b);
        aupp = std::min(aupp, -sum + b);

        //|a_n - a_{n-1} + ... + (-1)^n a_0| <= ((n + 2s_1 + t_2) / n)^(n/2)
        long alt_sum = this->alt_coeff_sums[level];
        b = this->eval_bounds[0]; //((n + 2s_1 + t_2) / n)^(n/2)
        alow = std::max(alow, alt_sum - b);
        aupp = std::min(aupp, alt_sum + b);

        //general code for alternating coeff sums
        /*for (long k = -1; k <= 1; k++) {
            if (k != 0) {
                sum = avalues[get_index(n)];
                long sgn = k * k;
                for (int i = n-2; i >= 0; i--) {
                    sum += sgn * avalues[get_index(i)];
                    sgn *= k;
                }
 
                b = this->eval_bounds[k+1];
                if (k > 0) {
                    alow = std::max(alow, (long)ceil( (double)(-sum - b) / k));
                    aupp = std::min(aupp, (long)floor((double)(-sum + b) / k));
                } else {
                    alow = std::max(alow, (long)floor((double)(-sum + b) / k));
                    aupp = std::min(aupp, (long)ceil( (double)(-sum - b) / k));
                }
            }
        }*/

        this->alower[get_index(n-1)] = alow;
        this->aupper[get_index(n-1)] = aupp;
    }
}

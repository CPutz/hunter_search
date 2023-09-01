/*
This files defines a Statistics class which collects statistics during runtime.
It also handles sending and receiving statistics to and from other threads.
*/

#include <algorithm>
#include <limits>
#include <iomanip>
#include "statistics.hpp"
#include "mpi.h"


Statistics::Statistics() {
    this->timers = std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>(ST_NUM_STATS);
    this->reset();
}

/*
 * Reset all statistics
 */
void Statistics::reset() {
    for (unsigned int i = 0; i < ST_NUM_STATS; i++) {
        this->totals[i] = 0;
        this->mins[i] = std::numeric_limits<long>::max();
        this->maxs[i] = std::numeric_limits<long>::min();
        this->counts[i] = 0;
    }
}

/*
 * Start a timer for a statistic type
 */
void Statistics::start_timer(StatType t) {
    this->timers[t] = std::chrono::high_resolution_clock::now();
}

/*
 * Stop a timer for a statistic type and collect the data
 */
void Statistics::stop_timer(StatType t) {
    auto time = std::chrono::high_resolution_clock::now();
    //duration in microseconds
    auto duration = this->duration_to_us(time - this->timers[t]);
    
    this->totals[t] += duration;
    this->mins[t] = (duration < this->mins[t]) ? duration : this->mins[t];
    this->maxs[t] = (duration > this->maxs[t]) ? duration : this->maxs[t];
    this->counts[t]++;
}

/*
 * Converts a high_resolution_clock duration to microseconds
 */
long Statistics::duration_to_us(const std::chrono::high_resolution_clock::duration &duration) {
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}


// MPI functions

/*
 * Send all statistics to the MPI thread with the given rank
 */
void Statistics::send(int const rank, int const tag) {
    MPI_Send(this->totals, ST_NUM_STATS, MPI_LONG, rank, tag, MPI_COMM_WORLD);
    MPI_Send(this->mins,   ST_NUM_STATS, MPI_LONG, rank, tag, MPI_COMM_WORLD);
    MPI_Send(this->maxs,   ST_NUM_STATS, MPI_LONG, rank, tag, MPI_COMM_WORLD);
    MPI_Send(this->counts, ST_NUM_STATS, MPI_LONG, rank, tag, MPI_COMM_WORLD);
}

/*
 * Receive statistics from the MPI thread with the given rank
 */
void Statistics::receive(int const rank, int const tag) {
    long new_totals[ST_NUM_STATS];
    long new_mins[ST_NUM_STATS];
    long new_maxs[ST_NUM_STATS];
    long new_counts[ST_NUM_STATS];

    MPI_Recv(new_totals, ST_NUM_STATS, MPI_LONG, rank, tag, MPI_COMM_WORLD, nullptr);
    MPI_Recv(new_mins,   ST_NUM_STATS, MPI_LONG, rank, tag, MPI_COMM_WORLD, nullptr);
    MPI_Recv(new_maxs,   ST_NUM_STATS, MPI_LONG, rank, tag, MPI_COMM_WORLD, nullptr);
    MPI_Recv(new_counts, ST_NUM_STATS, MPI_LONG, rank, tag, MPI_COMM_WORLD, nullptr);

    for (int i = 0; i < ST_NUM_STATS; i++) {
        this->totals[i] += new_totals[i];
        this->counts[i] += new_counts[i];
        //We only have to update mins and maxs if this event happened
        if (new_counts[i] > 0) {
            this->mins[i] = (new_mins[i] < this->mins[i]) ? new_mins[i] : this->mins[i];
            this->maxs[i] = (new_maxs[i] > this->maxs[i]) ? new_maxs[i] : this->maxs[i];
        }
    }
}


// Printing

/*
 * Print a single line in the statistics section
 */
void Statistics::print_statistic_line(std::ostream &output, std::string name, int part) {
    long count = this->counts[part];
    long total = this->totals[part];
    long min = this->mins[part];
    long max = this->maxs[part];

    output.flags(std::ios::left);
    output << std::setw(16) << name << std::setw(16) << count << std::setw(20) << total << std::setw(20);
    if (count > 0) {
        output << min << std::setw(20) << max << std::setw(16) << total / count;
    } else {
        output << "N/A" << std::setw(20) << "N/A" << std::setw(16) << "N/A";
    }
    output << std::endl;
}

/*
 * Print all statistics
 */
void Statistics::print(std::ostream &output) {
    output << "Total elapsed time: " << this->totals[ST_TOTAL] << "us" << std::endl;
    output << "Server\t\t\tAmount\t\t\tTotal Time (us)\t\tFastest Time (us)\tSlowest Time (us)\tApprox Avg Time (us)" << std::endl;
    this->print_statistic_line(output, "Idle:",            ST_WAIT_SERVER);
    this->print_statistic_line(output, "Generate:",        ST_GENERATE_SERVER);
    output << std::endl;
    output << "Workers\t\t\tAmount\t\t\tTotal Time (us)\t\tFastest Time (us)\tSlowest Time (us)\tApprox Avg Time (us)" << std::endl;
    this->print_statistic_line(output, "Workers:",         ST_WORKER);
    this->print_statistic_line(output, "Read input:",      ST_INPUT_READ);
    this->print_statistic_line(output, "Idle:",            ST_WAIT);
    this->print_statistic_line(output, "Work units:",      ST_UNIT);
    this->print_statistic_line(output, "Init: ",           ST_INIT);
    this->print_statistic_line(output, "Generate:",        ST_GENERATE);
    this->print_statistic_line(output, "Last coeff.:",     ST_CRT);
    this->print_statistic_line(output, "Disc. eval.:",     ST_EVAL);
    //this->print_statistic_line(output, "Bounds:",          ST_BOUNDS);
    this->print_statistic_line(output, "Lagrange:",        ST_LAGRANGE);
    this->print_statistic_line(output, "Send results:",    ST_SEND);
    this->print_statistic_line(output, "Disc. sq. test:",  ST_SQUARE_TEST);
    this->print_statistic_line(output, "Saved:",           ST_SAVE);
}


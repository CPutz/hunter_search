#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <set>
#include "mpi.h"
#include "bounds.hpp"
#include "polgenerator.hpp"
#include "simplepolgenerator.hpp"
#include "congruences.hpp"
#include "statistics.hpp"
#include "mathextra.hpp"
#include "order.hpp"
#include "print.hpp"


// MPI process info
static int const mpi_root = 0;
int mpi_index; // The index of this thread
int world_size; // The total number of mpi threads
enum MPI_TAGS {
    WORK_REQUEST_TAG,
    WORK_UNIT_TAG,
    RESULT_COLLECT_TAG,
    STATS_UPDATE_TAG,
    TERMINATION_REQUEST_TAG,
    TERMINATION_SUCCESFULL_TAG
};

// Output
std::string outputfile = "output.txt"; // Default output file
std::fstream outputstream;

// Input
std::string treefile = "input/tree.txt"; // Default disc. tree input file
std::fstream treestream;
std::vector<std::string> congruencefiles;
std::vector<std::vector<long>> moduli;
std::vector<std::vector<std::vector<long>>> congruences;
long discriminant;
int locked = 0;
std::vector<long> fixedcoefficients;
std::vector<long> filter_primes;
std::vector<long> divisors;

// Results and statistics
std::vector<std::vector<long>> results;
Statistics stats;

// Workers info
unsigned long worklimit = 0;
bool hasworklimit = false;


/*
 * Process a single work-unit
 */
void process_work_unit(PolGenerator &generator, std::vector<long> const &combination) {
    long coefficients[DEGREE+1];
    bool polAvailable = generator.init(combination);

    while (polAvailable) {
        // Do the square test
        stats.start_timer_all_stats(ST_SQUARE_TEST);
        bool is_square = generator.disc_is_square();
        stats.stop_timer_all_stats(ST_SQUARE_TEST);

        if (is_square) {
            // Extract the current coefficients from the generator
            generator.get_coefficients(coefficients);

            // Save polynomial to the result
            stats.start_timer(ST_SAVE);
            std::vector<long> result;
            for (unsigned int i = 0; i <= DEGREE; i++) {
                result.push_back(coefficients[i]);
            }
            results.push_back(result);
            stats.stop_timer(ST_SAVE);
        }

        // Configure the next coefficients in generator
        // Last call will fail, but still be timed and counted
        stats.start_timer_all_stats(ST_GENERATE);
        polAvailable = generator.configure_next();
        stats.stop_timer_all_stats(ST_GENERATE);
    }

    if (!results.empty()) {
        // Send results to root if any
        stats.start_timer(ST_SEND);
        for (auto result : results) {
            MPI_Send(result.data(), result.size(), MPI_LONG, mpi_root, RESULT_COLLECT_TAG, MPI_COMM_WORLD);
        }
        results.clear();
        stats.stop_timer(ST_SEND);
    }
}

/*
 * Receive and process work units for worker nodes
 */
void worker_thread_loop(std::vector<std::vector<long>> const &moduli) {
    long fixed = fixedcoefficients.size();
    std::vector<long> combination(fixed + locked);
    MPI_Status status;

    stats.start_timer(ST_WORKER);

    // Open tree file stream
    std::fstream treestream;
    treestream.open(treefile);
    if (!treestream.is_open()) {
        std::cout << "Could not open file " << treefile << ", exiting" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Create polynomial generator
    PolGenerator generator(divisors, discriminant, treestream,
        congruences, moduli, filter_primes, &stats);
    treestream.close();

    // Destroy the congruences data
    std::vector<std::vector<std::vector<long>>>().swap(congruences);

    while (true) {
        MPI_Send(nullptr, 0, MPI_INT, mpi_root, WORK_REQUEST_TAG, MPI_COMM_WORLD);
        // Wait for request of root
        stats.start_timer_all_stats(ST_WAIT);
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        stats.stop_timer_all_stats(ST_WAIT);
        if (status.MPI_TAG == TERMINATION_REQUEST_TAG) {
            // Root indicated everything is done
            MPI_Recv(nullptr, 0, MPI_INT, status.MPI_SOURCE, TERMINATION_REQUEST_TAG, MPI_COMM_WORLD, &status);
            break;
        } else {
            // Receive the work-units and process them
            MPI_Recv(combination.data(), fixed + locked, MPI_LONG, status.MPI_SOURCE, WORK_UNIT_TAG, MPI_COMM_WORLD, &status);
            stats.start_timer_all_stats(ST_UNIT);
            process_work_unit(generator, combination);
            stats.stop_timer_all_stats(ST_UNIT);
            
            // Send progress delta to root
            stats.send(mpi_root, STATS_UPDATE_TAG);
            stats.reset();
        }
    }
    stats.stop_timer(ST_WORKER);

    // Send last statistics
    stats.send(mpi_root, STATS_UPDATE_TAG);
    MPI_Send(nullptr, 0, MPI_INT, mpi_root, TERMINATION_SUCCESFULL_TAG, MPI_COMM_WORLD);
}

/*
 * Divide work among workers, send and collect results
 */
void main_thread_loop() {
    long fixed = fixedcoefficients.size();
    long coefficients[fixed + locked];
    std::vector<long> combination(fixed + locked);
    int terminated = 0;
    MPI_Status status;
    unsigned long count = 0;

    long result[DEGREE+1];

    SimplePolGenerator generator(fixedcoefficients, fixed + locked, divisors, discriminant, &stats);

    bool finished = !generator.init();

    while (true) {
        if (terminated >= world_size - 1) {
            // Every worker has terminated
            break;
        }
        //Wait for a message from any worker
        stats.start_timer_all_stats(ST_WAIT_SERVER);
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        stats.stop_timer_all_stats(ST_WAIT_SERVER);
        if (status.MPI_TAG == WORK_REQUEST_TAG) {
            MPI_Recv(nullptr, 0, MPI_INT, status.MPI_SOURCE, WORK_REQUEST_TAG, MPI_COMM_WORLD, &status);
            if (finished) {
                MPI_Send(nullptr, 0, MPI_INT, status.MPI_SOURCE, TERMINATION_REQUEST_TAG, MPI_COMM_WORLD);
            } else {
                generator.get_coefficients(coefficients);
                std::copy(coefficients, coefficients + fixed + locked, combination.begin());
                MPI_Send(combination.data(), fixed + locked, MPI_LONG, status.MPI_SOURCE, WORK_UNIT_TAG, MPI_COMM_WORLD);
                count++;
                stats.start_timer_all_stats(ST_GENERATE_SERVER);
                finished = !generator.configure_next() || (hasworklimit && count >= worklimit);
                stats.stop_timer_all_stats(ST_GENERATE_SERVER);
            }
        } else if (status.MPI_TAG == RESULT_COLLECT_TAG) {
            // Save the result from a worker
            MPI_Recv(result, DEGREE+1, MPI_LONG, status.MPI_SOURCE, RESULT_COLLECT_TAG, MPI_COMM_WORLD, &status);
            print_result(outputstream, result, DEGREE+1);
        } else if (status.MPI_TAG == STATS_UPDATE_TAG) {
            stats.receive(status.MPI_SOURCE, STATS_UPDATE_TAG);
        } else if (status.MPI_TAG == TERMINATION_SUCCESFULL_TAG) {
            MPI_Recv(nullptr, 0, MPI_INT, status.MPI_SOURCE, TERMINATION_SUCCESFULL_TAG, MPI_COMM_WORLD, &status);
            terminated += 1;
        }
    }
}

/*
 * Parse commandline arguments
 */
void init_arg(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        int ni = i + 1;
        std::string cmdarg = argv[i];
        if (cmdarg == "-limit") {
            // Control when generator stops
            if (ni >= argc) {
                std::cout << "-limit <amount> : set worklimit or -1 for unlimited [default = -1]" << std::endl;
                std::exit(1);
            }
            std::istringstream iss(argv[ni]);
            if (!(iss >> worklimit)) {
                std::cout << "-limit <amount> : set worklimit [default = unlimited]" << std::endl;
                std::exit(1);
            }
            hasworklimit = true;
            i = ni;
        } else if (cmdarg == "-output") {
            if (ni >= argc) {
                std::cout << "-output <name> : use given filename for output [default = output.txt]" << std::endl;
                std::exit(1);
            }
            outputfile = argv[ni];
            i = ni;
        } else if (cmdarg == "-input") {
            // Add congruence-files
            if (ni >= argc) {
                std::cout << "-input <file...> : use given comma-separated congruence-files, at least 1 file is required" << std::endl;
                std::exit(1);
            }
            std::stringstream ss(argv[ni]);
            std::string file;
            while (std::getline(ss, file, ',')) {
                congruencefiles.push_back(file);
            }
            i = ni;
        } else if (cmdarg == "-coeffs") {
            if (ni >= argc) {
                std::cout << "-coeffs <c...> : set fixed coefficients" << std::endl;
                std::exit(1);
            }
            std::stringstream ss(argv[ni]);
            long c;
            while (ss >> c) {
                fixedcoefficients.push_back(c);
                ss.ignore(1, ',');
            }
            i = ni;
        } else if (cmdarg == "-divisors") {
            if (ni >= argc) {
                std::cout << "-divisors <c...> : set fixed divisors" << std::endl;
                std::exit(1);
            }
            std::stringstream ss(argv[ni]);
            long d;
            std::vector<long> tmp;
            while (ss >> d) {
                tmp.push_back(d);
                ss.ignore(1, ',');
            }
            if (tmp.size() != DEGREE) {
                std::cout << "number of divisors (" << tmp.size() << ") must be equal to the DEGREE (" << DEGREE << ")" << std::endl;
                std::exit(1);
            }
            //reorder the divisors
            divisors.push_back(1); //the first (of the leading coefficient) divisor is always one
            for (unsigned int j = 1; j <= DEGREE; j++) {
                divisors.push_back(tmp[get_degree(j)-1]);
            }
            i = ni;
        } else if (cmdarg == "-discriminant") {
            if (ni >= argc) {
                std::cout << "-discriminant <amount> : set discriminant" << std::endl;
                std::exit(1);
            }
            std::stringstream ss(argv[ni]);
            ss >> discriminant;
            i = ni;
        } else if (cmdarg == "-tree") {
            if (ni >= argc) {
                std::cout << "-tree <name> : use given filename for discriminant tree [default = input/tree.txt]" << std::endl;
                std::exit(1);
            }
            treefile = argv[ni];
            i = ni;
        } else if (cmdarg == "-locked") {
            if (ni >= argc) {
                std::cout << "-locked <amount> : set number of locked coefficients [default = 1]" << std::endl;
                std::exit(1);
            }
            std::stringstream ss(argv[ni]);
            ss >> locked;
            i = ni; 
        } else if (cmdarg == "-filter") {
            if (ni >= argc) {
                std::cout << "-filter <c...> : set primes used for filtering" << std::endl;
                std::exit(1);
            }
            std::stringstream ss(argv[ni]);
            long p;
            while (ss >> p) {
                filter_primes.push_back(p);
                ss.ignore(1, ',');
            }
            i = ni;
        } else {
            // Invalid commandline argument given, report and exit
            std::cout << "Unknown argument " << argv[i] << std::endl;
            exit(1);
        }
    }
}

/*
 * Read the congruence input files
 */
void read_input() {
    // Read congruences-files
    moduli = std::vector<std::vector<long>>(congruencefiles.size());
    congruences = std::vector<std::vector<std::vector<long>>>(congruencefiles.size());
    std::fstream congruencestream;
    congruencestream.exceptions(std::fstream::failbit);
    for (unsigned int i = 0; i < congruencefiles.size(); i++) {
        try {
            congruencestream.open(congruencefiles[i]);
        } catch (const std::exception& e) {
            std::cout << "Opening congruence file '" << congruencefiles[i] << "' failed, it either doesn't exist or is not accessible." << std::endl;
            exit(1);
        }
        read_congruences(congruencestream, congruences[i], moduli[i]);
        congruencestream.close();
    }   
}

int main(int argc, char **argv) {
    int provided;
    // Make sure MPI_THREAD_FUNNELED is supported, meaning only the main thread may make MPI calls
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    if (provided < MPI_THREAD_FUNNELED) {
        std::cout << "MPI_THREAD_FUNNELED level not provided, aborting" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_index);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Parse program arguments
    init_arg(argc, argv);

    // Read input
    if (mpi_index != mpi_root) {
        stats.start_timer(ST_INPUT_READ);
        read_input();
        stats.stop_timer(ST_INPUT_READ);
    }

    // Write start info into outputfile
    if (mpi_index == mpi_root) {
        outputstream.open(outputfile, std::ios::app | std::ios::in);
        if (!outputstream.is_open()) {
            std::cout << "Could not open file " << outputfile << ", exiting" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        print_start(outputstream, world_size, argc, argv);
    }

    // Start computation
    MPI_Barrier(MPI_COMM_WORLD);
    if (mpi_index == mpi_root) {
        stats.start_timer(ST_TOTAL);
        main_thread_loop();
    } else {
        worker_thread_loop(moduli);
    }

    // End computation
    MPI_Barrier(MPI_COMM_WORLD);
    if (mpi_index == mpi_root) {
        stats.stop_timer(ST_TOTAL);
        stats.print(outputstream);
        outputstream.close();
    }

    // Cleanup
    MPI_Finalize();

    return 0;
}

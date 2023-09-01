/*
This file defines functions for printing results of the Hunter search
as well as some additional information.
*/

#include <iostream>
#include <iomanip>
#include <chrono>
#include "print.hpp"

std::string const MARK_ARG = "A";
std::string const MARK_INFO = "I";
std::string const MARK_RESULT = "R";

/*
 * Print an array of longs as a list
 */
void print_array_long(std::ostream &output, long arr[], int size) {
    output << "[";
    for (int i = 0; i < size; i++) {
         output << arr[i];
         if (i < (size - 1)) {
             output << ", ";
         }
    }
    output << "]";
}

/*
 * Print a marker with a mark and a time stamp
 */
void print_marker(std::ostream &output, std::string mark) {
    std::chrono::time_point<std::chrono::system_clock> timepoint = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(timepoint);
    output << "[" << mark << " " << std::put_time(std::localtime(&time), "%c") << "] ";
}

/*
 * Print a result
 */
void print_result(std::ostream &output, long result[], int terms) {
    print_marker(output, MARK_RESULT);
    print_array_long(output, result, terms);
    output << std::endl;
}

/*
 * Print the start of the output file
 */
void print_start(std::ostream &output, int worldSize, int argc, char **argv) {
    print_marker(output, MARK_INFO);
    output << "Program launch: " << worldSize << " workers" << std::endl;
    print_marker(output, MARK_ARG);
    for (int i = 0; i < argc; i++) {
        output << argv[i] << " ";
    }
    output << std::endl;
}
/*
This files defines a function to read a list of congruences
*/

#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>
#include "congruences.hpp"
#include "order.hpp"

bool read_congruences(std::istream &input,
    std::vector<std::vector<long>> &congruences, std::vector<long> &moduli) {

    std::vector<long> congruence;
    std::string line;
    std::getline(input, line);
    std::stringstream ss(line);

    // Read moduli
    long c;
    ss.ignore(1, ',');
    std::vector<long> tmp = std::vector<long>();
    while (ss >> c) {
        tmp.push_back(c);
        ss.ignore(1, ',');
    }

    // Determine reordering of coefficients
    int length = tmp.size();
    std::vector<long> degrees = std::vector<long>();
    for (unsigned int index = DEGREE; index > DEGREE - length; index--) {
        degrees.push_back(get_degree(index));
    }
    sort(degrees.begin(), degrees.end());
    for (int i = 0; i < length; i++) {
        degrees[i] = get_index(degrees[i]);
    }
    std::vector<long> indices(length);
    int start = DEGREE - length + 1;
    for (int index = start; index <= DEGREE; index++) {
        indices[index - start] = std::find(degrees.begin(), degrees.end(), index) - degrees.begin();
    }

    // Create moduli
    for (unsigned int i = 0; i < length; i++) {
        moduli.push_back(tmp[indices[i]]);
    }

    // Skip a line
    std::getline(input, line);

    // Read congruences until a line "]" appears
    for (std::getline(input, line); line != "]"; std::getline(input, line)) {
        std::stringstream ss(line);
        ss.ignore(1, '<');
        std::vector<long> congruence = std::vector<long>();
        std::vector<long> tmp = std::vector<long>();
        while (ss >> c) {
            tmp.push_back(c);
            ss.ignore(1, ',');
        }

        for (unsigned int i = 0; i < length; i++) {
            congruence.push_back(tmp[indices[i]]);
        }
        congruences.push_back(congruence);
    }

    return !congruences.empty();
}
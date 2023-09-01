/*
This files defines a CongruenceGenerator class which can congruence lists
with the property that the evaluated Horner expression will be a square modulo m.
The first k variables of the Horner expression can be fixed so that only
congruences with a certain prefix will be included.
*/

#include "congruencegenerator.hpp"
#include "mathextra.hpp"
#include <algorithm>
#include <iostream>

/*
 * Initialize a CongruenceGenerator with Horner tree read from the file stream fs.
 * The depth is the number of coefficients which will not be fixed.
 */
CongruenceGenerator::CongruenceGenerator(std::fstream &fs, long modulus) {
	this->tree = new HornerModTree(fs, modulus);
	int depth = this->tree->get_depth();
	this->moduli = std::vector<long>(depth);
	for (unsigned int i = 0; i < depth; i++) {
		this->moduli[i] = modulus;
	}

	//Compute squares (can contain duplicates if modulus is not prime)
	std::vector<long> squares = std::vector<long>();
	for (unsigned int i = 0; i <= modulus / 2; i++) {
		squares.push_back((i * i) % modulus);
	}

	//Create squares lookup table
	this->is_square = std::vector<bool>(modulus);
	for (unsigned int i = 0; i < modulus; i++) {
		this->is_square[i] = 
			std::find(squares.begin(), squares.end(), i) != squares.end();
	}
}

CongruenceGenerator::~CongruenceGenerator() {
	delete this->tree;
}

/*
 * Clear all results to prepare for new calculations
 */
void CongruenceGenerator::clear() {
	this->res = std::vector<std::vector<long>>();
	this->avalues = std::vector<long>();
}

/*
 * Create the list of congruences for which the evaluated Horner tree is a
 * square modulo the modulus, without any fixed coefficients
 */
std::vector<std::vector<long>> &CongruenceGenerator::generate_congruences() {
	return generate_congruences(std::vector<long>());
}

/*
 * Create the list of congruences for which the evaluated Horner tree is a
 * square modulo the modulus, with the first number of coefficients fixed
 */
std::vector<std::vector<long>> &CongruenceGenerator::generate_congruences(
	std::vector<long> fixedcoefficients) {

	this->res = std::vector<std::vector<long>>();
	this->avalues = std::vector<long>();
	//init avalues and the discriminant tree
	for (unsigned int i = 0; i < fixedcoefficients.size(); i++) {
		long c = mod(fixedcoefficients[i], moduli[i]);
		this->avalues.push_back(c);
		this->tree->eval(this->moduli.size() - 1 - i, c);
	}

	generate(this->moduli.size() - 1 - fixedcoefficients.size());
	return this->res;
}

/*
 * Internal function for recursively generating the congruences.
 */
void CongruenceGenerator::generate(int level) {
	if (level == -1) {
		if (this->is_square[this->tree->get_val()]) {
			this->res.push_back(this->avalues);
		}
	} else {
		for (long c = 0; c < this->moduli[this->moduli.size()-level-1]; c++) {
			this->tree->eval(level, c);
			this->avalues.push_back(c);
			generate(level - 1);
			this->avalues.pop_back();
		}
	}
}

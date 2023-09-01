#ifndef CONGRUENCEGENERATOR_HPP
#define CONGRUENCEGENERATOR_HPP

#include "hornermodtree.hpp"
#include <vector>
#include <fstream>

class CongruenceGenerator {
private:
	HornerModTree *tree;
	std::vector<std::vector<long>> res;
	std::vector<long> avalues;
	std::vector<long> moduli;
	std::vector<bool> is_square;
	void generate(int level);

public:
	CongruenceGenerator(std::fstream &fs, long modulus);
	~CongruenceGenerator();
	std::vector<std::vector<long>> &generate_congruences();
	std::vector<std::vector<long>> &generate_congruences(std::vector<long> fixedcoefficients);
	void clear();
};

#endif // CONGRUENCEGENERATOR_HPP
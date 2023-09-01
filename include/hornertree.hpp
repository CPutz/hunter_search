#ifndef HORNERTREE_HPP
#define HORNERTREE_HPP

#include <vector>
#include <fstream>
#include "gmp.h"


class HornerTreeNode {
private:
	mpz_t coeff; //the value of the node
	std::vector<HornerTreeNode *> children;

public:
	HornerTreeNode(mpz_t val);
	HornerTreeNode(std::vector<HornerTreeNode *> children);
	~HornerTreeNode();
	void get_coeff(mpz_t rop);
	void add_coeff(mpz_t val);
	bool is_leaf();
	void eval(mpz_t val);
	void optimize();
	void print();
};


class HornerTree {
private:
	mpz_t tmp_z;
	HornerTreeNode *head;
	std::vector<std::vector<HornerTreeNode *>> nodes;
	HornerTreeNode* tree_read(int level, std::istringstream &ss);

public:
	HornerTree(HornerTreeNode *head,
		std::vector<std::vector<HornerTreeNode *>> nodes);
	HornerTree(std::fstream &fs);
	~HornerTree();
	void get_val(mpz_t rop);
	void eval(int level, mpz_t val);
	void print(int level);
	int get_depth();
};


#endif // HORNERTREE_HPP

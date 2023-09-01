#ifndef HORNERMODTREE_HPP
#define HORNERMODTREE_HPP

#include <vector>
#include <fstream>


class HornerModTreeNode {
private:
	long coeff;
	long modulus;
	std::vector<HornerModTreeNode *> children;

public:
	HornerModTreeNode(long coeff, long modulus);
	HornerModTreeNode(std::vector<HornerModTreeNode *> children, long modulus);
	~HornerModTreeNode();
	long get_coeff();
	bool is_leaf();
	void eval(long val);
	void optimize();
};


class HornerModTree {
private:
	long modulus;
	HornerModTreeNode *head;
	std::vector<std::vector<HornerModTreeNode *>> nodes;
	HornerModTreeNode* tree_read(int level, std::istringstream &ss);

public:
	HornerModTree(std::fstream &fs, long modulus);
	~HornerModTree();
	long get_val();
	void eval(int level, long val);
	int get_depth();
};

#endif // HORNERMODTREE_HPP

/*
This file defines a tree data structure for evaluating multivariate polynomial
expressions using a multivariate Horner scheme.
*/

#include "hornertree.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

/*
 * Initialize a leaf with a fixed value
 */
HornerTreeNode::HornerTreeNode(mpz_t val) { 
	mpz_init_set(this->coeff, val);
}

/*
 * Initialize a node with children
 */
HornerTreeNode::HornerTreeNode(std::vector<HornerTreeNode *> children) {
	this->children = children;
	mpz_init(this->coeff);
}

HornerTreeNode::~HornerTreeNode() {
	mpz_clear(this->coeff);

	for (HornerTreeNode *c : this->children) {
		delete c;
	}
}

/*
 * Set rop to the value of this node
 */
void HornerTreeNode::get_coeff(mpz_t rop) {
	mpz_set(rop, this->coeff);
}

/*
 * Add the value of this node to rop
 */
void HornerTreeNode::add_coeff(mpz_t rop) {
	mpz_add(rop, rop, this->coeff);
}

/*
 * A node is a leaf if it has no children
 */
bool HornerTreeNode::is_leaf() {
	return this->children.size() == 0;
}

/*
 * Calculate and update the value of this node when we evaluate it at value val,
 * using the values of its children and the Horner scheme relation
 */
void HornerTreeNode::eval(mpz_t val) {
	std::vector<HornerTreeNode *>::iterator it = this->children.begin();
	(*it)->get_coeff(this->coeff);

	//Horner scheme on children
	while (++it != this->children.end()) {
		HornerTreeNode *c = *it;
		mpz_mul(this->coeff, this->coeff, val);
		if (c) {
			c->add_coeff(this->coeff);
		}
	}
}

/*
 * Recusively cut constant paths in the current subtree
 */
void HornerTreeNode::optimize() {
	//recurse on the children
	for (HornerTreeNode *c : this->children) {
		if (c) {
			c->optimize();
		}
	}

	//if this node has one child, and that child is a leaf, then we can remove
	//the child and give the current node its value
	if (this->children.size() == 1 && this->children[0]->is_leaf()) {
		//copy value from child to parent
		this->children[0]->get_coeff(this->coeff);
		//remove child
		for (HornerTreeNode *c : this->children) {
			delete c;
		}
		//this->children.clear();
		this->children = std::vector<HornerTreeNode *>();
	}
}

/*
 * Print the value of the current node
 */
void HornerTreeNode::print() {
	gmp_printf("%Zd\n", this->coeff);
}

/*
 * Create a Horner tree from a file stream
 */
HornerTree::HornerTree(std::fstream &fs) {
	mpz_init(tmp_z);
	//go to start of file
	fs.clear();
	fs.seekg(0, std::ios::beg);

	std::string line;
	std::getline(fs, line);
	std::istringstream ss(line);
	this->head = tree_read(0, ss);
	//this->head->optimize();
}

/*
 * Recursively construct a Horner tree from a file stream
 */
HornerTreeNode* HornerTree::tree_read(int level, std::istringstream &ss) {
	HornerTreeNode *n;
	char c;
	ss.get(c);

	if (this->nodes.size() < level + 1) {
		this->nodes.push_back(std::vector<HornerTreeNode*>());
	}

	//we go one level down in the tree
	if (ss.peek() == '[') {
		std::vector<HornerTreeNode*> children;
		//construct the children until a ']' appears, meaning we go one level up
		do {
			children.push_back(tree_read(level + 1, ss));
		} while (ss.peek() != ']');
		ss.get(c);
		n = new HornerTreeNode(children);
		this->nodes[level].push_back(n);
	}
	//we read the the current value and create a leaf with that value
	else { 
		std::string v;
		std::getline(ss, v, ']');
		//if v is empty then we create an empty leaf
		if (v.empty()) {
			n = nullptr;
		} else {
			//convert the string v to a number in base 10
			mpz_set_str(tmp_z, v.c_str(), 10);
			n = new HornerTreeNode(tmp_z);
			//do not add leaves, because we do not have to update them
			//this->nodes[level].push_back(n)
		}
	}

	return n;
}

HornerTree::~HornerTree() {
	//recursively deletes all its children
	delete head;
	mpz_clear(tmp_z);
}

/*
 * Get the value of the root node
 */
void HornerTree::get_val(mpz_t rop) {
	this->head->get_coeff(rop);
}

/*
 * Evaluates the Horner expression for the level-th variable with value val.
 * For correct results, the variables should be evaluated in descending order
 * (highest level first)
 */
void HornerTree::eval(int level, mpz_t val) {
	//evaluate all nodes at depth level
	for (HornerTreeNode *n : this->nodes[level]) {
		n->eval(val);
	}
}

/*
 * Print the values of all nodes at depth level
 */
void HornerTree::print(int level) {
	for (HornerTreeNode *n : this->nodes[level]) {
		n->print();
	}
}

/*
 * The maximal depth of the tree
 */
int HornerTree::get_depth() {
	return this->nodes.size() - 1;
}

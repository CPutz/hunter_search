/*
This file defines a tree data structure for evaluating multivariate polynomial
expressions modulo m using a multivariate Horner scheme.
*/

#include "hornermodtree.hpp"
#include "mathextra.hpp"
#include <sstream>
#include <iostream>

/*
 * Initialize a leaf with a fixed value
 */
HornerModTreeNode::HornerModTreeNode(long val, long modulus) { 
	this->coeff = mod(val, modulus);
	this->modulus = modulus;
}

/*
 * Initialize a node with children
 */
HornerModTreeNode::HornerModTreeNode(std::vector<HornerModTreeNode *> children,
	long modulus) {

	this->children = children;
	this->modulus = modulus;
}

HornerModTreeNode::~HornerModTreeNode() {
	for (HornerModTreeNode *c : this->children) {
		delete c;
	}
}

/*
 * The value of this node
 */
long HornerModTreeNode::get_coeff() {
	return this->coeff;
}

/*
 * A node is a leaf if it has no children
 */
bool HornerModTreeNode::is_leaf() {
	return this->children.size() == 0;
}

/*
 * Calculate and update the value of this node when we evaluate it at value val,
 * using the values of its children and the Horner scheme relation
 */
void HornerModTreeNode::eval(long val) {
	std::vector<HornerModTreeNode *>::iterator it = this->children.begin();
	this->coeff = (*it)->get_coeff();

	//use Horner scheme on children
	while (++it != this->children.end()) {
		HornerModTreeNode *c = *it;
		this->coeff *= val;
		if (c != nullptr) {
			this->coeff += c->get_coeff();
		}
		this->coeff = mod(this->coeff, this->modulus);
	}
}

/*
 * Recusively cut constant paths in the current subtree
 */
void HornerModTreeNode::optimize() {
	bool was_leaf = this->is_leaf();

	//recurse on the children
	for (HornerModTreeNode *c : this->children) {
		if (c) {
			c->optimize();
		}
	}

	//if the highest degree coefficient of this node is 0, remove that child
	//(it is unnecessary)
	while (this->children.size() > 0 && (!this->children[0] ||
				(this->children[0]->is_leaf() &&
				 this->children[0]->get_coeff() == 0))) {
		if (this->children[0]) {
			delete this->children[0];
		}
		this->children.erase(this->children.begin());
	}

	
	//the node was no leaf before optimization, but is a leaf after optimization
	//in that case, set its value to 0
	if (!was_leaf && this->is_leaf()) {
		this->coeff = 0;
	}
	//if this node has one child, and that child is a leaf, then we can remove
	//the child and give the current node its value
	else if (this->children.size() == 1 && this->children[0]->is_leaf()) {
		//copy value from child to parent
		this->coeff = this->children[0]->get_coeff();
		//remove child
		for (HornerModTreeNode *c : this->children) {
			delete c;
		}
		//this->children.clear();
		this->children = std::vector<HornerModTreeNode *>();
	}
}

/*
 * Create a Horner tree from a file stream
 */
HornerModTree::HornerModTree(std::fstream &fs, long modulus) {
	//go to start of file
	fs.clear();
	fs.seekg(0, std::ios::beg);
	
	this->modulus = modulus;
	std::string line;
	std::getline(fs, line);
	std::istringstream ss(line);
	this->head = tree_read(0, ss);
	this->head->optimize();
	for (unsigned int i = 0; i < this->nodes.size(); i++) {
		//remove nullptrs from node list
		auto it = this->nodes[i].begin();
		while(it != this->nodes[i].end()) {
			if (!(*it) || (*it)->is_leaf()) {
				it = this->nodes[i].erase(it);
			} else {
				it++;
			}
		}
	}
}

/*
 * Recursively construct a Horner tree from a file stream
 */
HornerModTreeNode* HornerModTree::tree_read(int level, std::istringstream &ss) {
	HornerModTreeNode *n;
	char c;
	ss.get(c);

	if (this->nodes.size() < level + 1) {
		this->nodes.push_back(std::vector<HornerModTreeNode*>());
	}

	if (ss.peek() == '[') {
		std::vector<HornerModTreeNode*> children;
		do {
			children.push_back(tree_read(level + 1, ss));
		} while (ss.peek() != ']');
		ss.get(c);
		n = new HornerModTreeNode(children, this->modulus);
		this->nodes[level].push_back(n);
	} else {
		std::string v;
		std::getline(ss, v, ']');
		if (!v.empty()) {
			long c;
			std::istringstream(v) >> c;
			n = new HornerModTreeNode(c, this->modulus);
		} else {
			n = nullptr;
		}
		//do not add leaves, because we do not have to update them
		//this->nodes[level].push_back(n);
	}

	return n;
}

HornerModTree::~HornerModTree() {
	delete head;
}

/*
 * Get the value of the root node
 */
long HornerModTree::get_val() {
	return this->head->get_coeff();
}

/*
 * Evaluates the Horner expression for the level-th variable with value val.
 * For correct results, the variables should be evaluated in descending order
 * (highest level first)
 */
void HornerModTree::eval(int level, long val) {
	//evaluate all nodes at the current level
	for (HornerModTreeNode *n : this->nodes[level]) {
		n->eval(val);
	}
}

/*
 * The maximal depth of the tree
 */
int HornerModTree::get_depth() {
	return this->nodes.size() - 1;
}
/*
This file defines a data structure for looking up of congruence data.
*/

#include "congruencelookup.hpp"
#include "mathextra.hpp"
#include <iostream>

/*
 * Create a node
 */
CongruenceLookupNode::CongruenceLookupNode(CongruenceLookupNode *parent, int size) {
	this->children = std::vector<CongruenceLookupNode *>(size);
	this->parent = parent;
}

/*
 * Create a leaf
 */
CongruenceLookupNode::CongruenceLookupNode(CongruenceLookupNode *parent) { 
	this->parent = parent;
}

CongruenceLookupNode::~CongruenceLookupNode() {
	for (CongruenceLookupNode *c : this->children) {
		delete c;
	}
}

/*
 * Get the values attached to this node
 */
std::vector<long> &CongruenceLookupNode::get_values() {
	return this->values;
}

/*
 * Get the c-th child of this node
 */
CongruenceLookupNode *CongruenceLookupNode::next(long c) {
	return this->children[mod(c, this->children.size())];
}

/*
 * Get the parent of this node
 */
CongruenceLookupNode *CongruenceLookupNode::prev() {
	return this->parent;
}

/*
 * Create a leaf child with index c
 */
CongruenceLookupNode *CongruenceLookupNode::set(long c) {
	this->children[c] = new CongruenceLookupNode(this);
	return this->children[c];
}

/*
 * Create a node child with index c
 */
CongruenceLookupNode *CongruenceLookupNode::set(long c, long m) {
	this->children[c] = new CongruenceLookupNode(this, m);
	return this->children[c];
}

/*
 * Add the value c to a node/leaf
 */
void CongruenceLookupNode::add(long c) {
	this->values.push_back(c);
}

/*
 * Print the values attached to this node
 */
void CongruenceLookupNode::print() {
	for (unsigned int c = 0; c < this->children.size(); c++) {
		if (this->children[c]) {
			std::cout << c << " ";
		}
	}
	std::cout << std::endl;
}

/*
 * Creates a search tree from a list of congruences
 */
CongruenceLookup::CongruenceLookup(std::vector<std::vector<long>> const &congruences,
	std::vector<long> const &moduli) {
	this->moduli = moduli;
	this->origin = new CongruenceLookupNode(nullptr, moduli[0]);
	for (std::vector<long> congruence : congruences) {
		add_node(congruence);
	}
	reset();
}

CongruenceLookup::~CongruenceLookup() {
	delete this->origin;
}

/*
 * Add a congruence to the lookup tree
 */
void CongruenceLookup::add_node(std::vector<long> &congruence) {
	CongruenceLookupNode *current;
	current = this->origin;
	for (unsigned int i = 0; i < this->moduli.size()-1; i++) {
		long c = congruence[i];
		CongruenceLookupNode *next = current->next(c);
		if (!next) {
			next = current->set(c, this->moduli[i+1]);
		}
		current = next;
	}
	current->add(congruence[this->moduli.size()-1]);
}

/*
 * Traverse one level deeper with value c.
 * Returns true if a congruence exists with the current starting sequence
 * followed by value c. Returns false otherwise.
 */
bool CongruenceLookup::walk(long c) {
	if (this->overflow == 0) {
		CongruenceLookupNode *next = this->marker->next(c);
		if (next) {
			this->marker = next;
			return true;
		}
	}
	this->overflow++;
	return false;
}

/*
 * Fake traverse one level deeper in the tree.
 */
void CongruenceLookup::walk_dummy() {
	this->overflow++;
}

/*
 * Traverse one level up in the tree.
 */
void CongruenceLookup::back() {
	if (this->overflow > 0) {
		this->overflow--;
	} else {
		this->marker = this->marker->prev();
	}
}

/*
 * Reset the lookup tree back to the root node.
 */
void CongruenceLookup::reset() {
	this->marker = this->origin;
	this->overflow = 0;
}

/*
 * Get the values at the current position in the lookup tree.
 */
std::vector<long> *CongruenceLookup::get_values() {
	return &this->marker->get_values();
}

/*
 * Print the current marker node in the tree.
 */
void CongruenceLookup::print() {
	std::cout << "tree print: ";
	if (this->overflow > 0) {
		std::cout << "overflow " << overflow << std::endl;
	} else {
		this->marker->print();
	}
}
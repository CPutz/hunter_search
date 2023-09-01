#ifndef SEARCHTREE_HPP
#define SEARCHTREE_HPP

#include <vector>


class CongruenceLookupNode {
private:
	std::vector<CongruenceLookupNode *> children;
	CongruenceLookupNode *parent;
	std::vector<long> values;

public:
	CongruenceLookupNode(CongruenceLookupNode *parent, int size);
	CongruenceLookupNode(CongruenceLookupNode *parent);
	~CongruenceLookupNode();
	std::vector<long> &get_values();
	CongruenceLookupNode *next(long c);
	CongruenceLookupNode *prev();
	CongruenceLookupNode *set(long c);
	CongruenceLookupNode *set(long c, long m);
	void add(long c);
	void print();
};


class CongruenceLookup {
private:
	CongruenceLookupNode *origin;
	CongruenceLookupNode *marker;
	int overflow;
	std::vector<long> moduli;
	void add_node(std::vector<long> &congruence);

public:
	CongruenceLookup(std::vector<std::vector<long>> const &congruences,
		std::vector<long> const &modulos);
	~CongruenceLookup();
	bool walk(long c);
	void walk_dummy();
	void back();
	void reset();
	std::vector<long> *get_values();
	void print();
};


#endif // SEARCHTREE_HPP

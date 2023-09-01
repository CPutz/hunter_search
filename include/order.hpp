/*
This file defines two bijections {0,1,...,n} -> {0,1,...,n} which convert
between the indices for which the variables a_1,...,a_n are saved in memory
([a_1,a_2,...,a_n]), and the way they are enumerated ([a_1,a_n,a_2,...,a_{n-1})}])
*/

#ifndef ORDER_HPP
#define ORDER_HPP

constexpr unsigned int get_index(unsigned int degree) {
	return degree <= 1 ? degree : (degree == DEGREE ? 2 : degree + 1);
};
constexpr unsigned int get_degree(unsigned int index) {
	return index <= 1 ? index : (index == 2 ? DEGREE : index - 1);
};

#endif // ORDER_HPP
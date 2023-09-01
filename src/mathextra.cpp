/*
This files defines some miscellaneous math functions
*/

#include "mathextra.hpp"

/*
 * The class of a modulo m, represented as a number between 0 and m-1
 */
long mod(long const a, long const m) {
    long r = a % m;
    return r < 0 ? r + m : r;
}

/*
 * The greatest common divisor of a and b
 */
long gcd(long const a, long const b) {
	if (b == 0) {
		return a;
	}
   	return gcd(b, mod(a, b));
}

/*
 * The lowest common multiple of a and b
 */
long lcm(long const a, long const b) {
	return (a * b) / gcd(a, b);
}

/*
 * The multiplicative inverse of a modulo m
 */
long inverse_mod(long const a, long const m) {
	// extended Euclidean algorithm
	long x = m;
	long y = mod(a,m);
	long cx = 0;
	long cy = 1;

	while (y != 1) {
		long r = mod(x, y);
		long d = (x - r) / y;
		x = y;
		y = r;
		long old_cx = cx;
		cx = cy;
		cy = old_cx - d * cy;
	}

	return mod(cy, m);
}
#ifndef _POLYNOMIAL_H
#define _POLYNOMIAL_H

#include <stdint.h>

/**
 * Given an array of coefficients representing a polynomial
 * in a 2^8 Galois field and an x value, returns the value
 * obtained when evaluating the polynomial.
 *
 * @param coefficients The coefficients in reverse order, so that
 *                     coefficients[0] is the coefficient for the
 *                     highest degree and coefficients[n - 1] is the
 *                     one for X^0, i.e. the constant term of the
 *                     polynomial
 * @param n The number of coefficients
 * @param x The value for which to evaluate the polynomial
 * @return The value of the polynomial calculated for the given x
 */
u_int8_t evaluate_polynomial(u_int8_t* coefficients, unsigned int n, u_int8_t x);

#endif

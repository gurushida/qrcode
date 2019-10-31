#ifndef _POLYNOMIAL_H
#define _POLYNOMIAL_H

#include <stdint.h>

/**
 * This structure represents a polynomial
 * where the coefficients are elements of the 2^8
 * Galois field used by QR codes. By convention,
 * the coefficients are in reverse order, i.e.
 * coefficients[0] is the coefficient for X^(n_coefficients - 1)
 * while coefficients[n_coefficients - 1] is the constant term
 * of the polynomial.
 */
struct gf_polynomial {
    u_int8_t* coefficients;
    unsigned int n_coefficients;
};


/**
 * Given an array of coefficients representing a polynomial
 * in a 2^8 Galois field and an x value, returns the value
 * obtained when evaluating the polynomial.
 *
 * @param p The polynomial to evaluate
 * @param x The value for which to evaluate the polynomial
 * @return The value of the polynomial calculated for the given x
 */
u_int8_t evaluate_polynomial(struct gf_polynomial* p, u_int8_t x);


/**
 * Returns the value of the coefficient for the given degree in the given
 * polynomial or 0 if the given degree is greater than the degree of the
 * polynomial.
 */
u_int8_t get_coefficient(struct gf_polynomial* a, unsigned int degree);


/**
 * Sets the given coefficient value for the given polynomial at the given degree.
 * Returns 1 on success or 0 if the given degree is beyond the size of the coefficient
 * array.
 */
int set_coefficient(struct gf_polynomial* a, unsigned int degree, u_int8_t coefficient);


/**
 * Returns the highest value i > 0 so that the coefficient for X^i is not not 0
 * or 0 if the given polynomial is reduced to a constant.
 */
unsigned int get_degree(struct gf_polynomial* a);


/**
 * Allocates and returns a polynomial corresponding to a+b or NULL
 * in case of memory allocation error.
 */
struct gf_polynomial* add_polynomials(struct gf_polynomial* a, struct gf_polynomial* b);


/**
 * Allocates and returns a polynomial corresponding to a.b or NULL
 * in case of memory allocation error.
 */
struct gf_polynomial* multiply_polynomials(struct gf_polynomial* a, struct gf_polynomial* b);


/**
 * Divides a by b and places the quotient and remainder.
 * @return 1 on success
 *         0 if a's degree is lower than b's degree (which means
 *           that the quotient is 0 and b is the remainder)
 *        -1 if b only contains zeroes
 *        -2 in case of memory allocation error.
 */
int divide_polynomials(struct gf_polynomial* a, struct gf_polynomial* b,
                        struct gf_polynomial* *quotient, struct gf_polynomial* *remainder);


/**
 * Returns 1 if a and b have same degree and coefficients.
 */
int equal_polynomials(struct gf_polynomial* a, struct gf_polynomial* b);


/**
 * Allocates a polynomial for the given number of coefficients.
 *
 * @param n_coefficients The number of coefficients
 * @param coefficients If not null, the given array will be copied into the
 *                     newly created polynomial; otherwise, the coefficients are
 *                     initalized to 0
 * @return The polynomial or NULL in case of memory allocation error
 */
struct gf_polynomial* new_gf_polynomial(unsigned int n_coefficients, u_int8_t* coefficients);


/**
 * Frees the memory associated to the given polynomial.
 */
void free_gf_polynomial(struct gf_polynomial* p);

#endif

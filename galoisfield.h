#ifndef _GALOISFIELD_H
#define _GALOISFIELD_H

#include <stdint.h>

/**
 * QR codes use Reed-Solomon codes for error correction that are based on
 * a 2^8 Galois field with the prime polynomial P = X^8 + X^4 + X^3 + X^2 + 1
 *
 * A 2^8 Galois field is a mathematical construction that can represent
 * 256 elements as polynoms with binary coefficients, so that for each
 * byte abcdefgh, we can represent it as:
 *
 * a.X^7 + b.X^6 + c.X^5 + d.X^4 + e.X^3 + f.X^2 + g.X^1 + h
 *
 * All operations done between elements are done modulo the prime
 * polynomial P.
 *
 * There are two things in particular that make such a Galois field very
 * interesting for code correction calculation. First, since the coefficients
 * are binary, addition and subtraction are the same. For example,
 *
 * (X^3 + X^2) + (X^2 + 1) = X^3 + 1
 *
 * because 2.X^2 is the same as 0.X^2 so we can remove the X^2 terms.
 * Since we have (0 + 0) = 0, (0 + 1) = (1 + 0) = 1 and (1 + 1) = 0,
 * this operation can be efficiently implemented with a bitwise XOR operation.
 *
 * The second very interesting property is that any element except the zero one
 * (where all coefficents are zero) can be represented as a power of a generator
 * element alpha that is one of the elements of the Galois field (note that
 * there may be more than one generator depending on the Galois field).
 * The number of elements that can be represented as a power of the generator
 * alpha is called the order of the generator and is noted |alpha|. For
 * a 2^8 Galois, |alpha| = 255 (all values except 0).
 *
 * That means that for any x > 0, there is an m so that x = (alpha^m).
 * This is very interesting because it makes it easy to multiply values:
 *
 * x.y = (alpha^m) . (alpha^n) = alpha^(m + n)
 *
 * For a 2^8 Galois field, it is easy to precompute all the powers and
 * logarithms so that given an m we get the x so that x = (alpha^m) and that
 * given (alpha^m), we can find the corresponding x. As a consequence, given
 * x.y = alpha^(m + n), it is easy to find the element z that corresponds
 * to x.y with:
 *
 * x.y = alpha^(m + n) = alpha^(log(x) + log(y))
 *
 * However, in order to calculate a power of alpha, the exponent must be
 * lower than |alpha| so we apply a modulo and obtain:
 *
 * x.y = alpha^((log(x) + log(y)) % |alpha|)
 *     = alpha^((log(x) + log(y)) % 255)
 *
 * It is also easy to compute the inverse of a number x = alpha^m:
 *
 * 1/x = 1/(alpha^m) = alpha^(-m) = alpha^(|alpha| - m) = alpha^(255 - log(x))
 *
 * As it happens, the prime polynomial used for QR codes has alpha = 2 as
 * a generator element which makes operations easier as powers of 2 can be
 * computed with bit shifts.
 *
 * Given that a 2^8 Galois field can represent 256 values, let's represent
 * all the elements as unsigned 8-bit values.
 */
u_int8_t gf_add_or_subtract(u_int8_t a, u_int8_t b);
u_int8_t gf_multiply(u_int8_t a, u_int8_t b);
u_int8_t gf_inverse(u_int8_t a);

#endif

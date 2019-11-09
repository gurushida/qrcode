#ifndef _REEDSOLOMON_H
#define _REEDSOLOMON_H

#include "blocks.h"
#include "polynomial.h"


/**
 * The principle of the Reed-Solomon codes is to treat each data block
 * of k codewords as a polynomial of degree k - 1 where the coefficients
 * are the codewords interpreted as values of a 2^8 Galois field, like this:
 *
 * A(X) = r_(k-1) . X^(k-1) + ... + r_1.X + r_0
 *
 * For a given value of k, there is a corresponding value t that is the error
 * correction capacity, i.e. the number of errors that can be detected and
 * corrected.
 *
 * We define a generator polynomial G like this:
 *
 * G(X) = (X - alpha)(X - alpha^2)...(X - alpha^2t)
 *
 * where alpha the generator element of the Galois field. With it we can obtain
 * the control polynomial B which is the remainder of the Euclidian division of
 * A.X^2t by G. The coefficients of B are the error correction codewords that will
 * be used to detect errors in A. Since B is of a degree lower than 2t, we can obtain
 * the final message to be transmitted like this:
 *
 * R = A.X^2t + B
 *
 * which happens to be exactly the structure of a QR code block.
 *
 *
 * Given a block of data+error correction codewords, this function checks
 * whether the block contains errors or, and if so, corrects them in place
 * so that the data codewords at beginning of the codeword array are
 * the correct codewords to be used to decode the QR code.
 *
 * The implementation is a port of the one in the zxing project.
 *
 * @param b The single block to decode
 * @return On success, a value n>=0 representing the number of errors
 *         that were corrected; -1 if the block could not be decoded
 *         because there were too many errors; -2 if we failed because
 *         of a memory allocation error
 */
int error_correction(struct block* b);


/**
 * Each block of M codewords is treated a polynomial like:
 *
 * R(X) = r_(M-1) . X^(M-1) + r_(M-2) . X^(M-2) + ... + r_1 . X + r_0
 *
 * where the values R=(r_(M-1), r_(M-2), ... , r_0) represents the codewords.
 * In order to verify if the block contains errors, we need to calculate
 * N syndromes where N is 2*t (twice the error correction capacity), where
 * each syndrome S_i is equal to R(alpha^i) where alpha is the generator
 * element of the Galois field. If all syndromes are 0s, then there is no
 * error to be corrected. Otherwise there will be more work to do to figure
 * out how many errors there are and where they are.
 *
 * @param message A polynomial representing the message we want to decode
 * @param syndromes A polynomial structure where to store the syndromes
 * @return The number of non zero syndromes
 */
unsigned int calculate_syndromes(struct gf_polynomial* message, struct gf_polynomial* syndromes);


/**
 * Given the syndromes, this function uses the Euclidian algorithm to
 * obtain the error locator polynomial (often referred to as sigma) and
 * the error evaluator polynomial (often referred to as omega).
 *
 * @param syndromes The syndrome polynomial
 * @param n_error_correction_codewords The number of non-data codewords in the message
 * @param sigma Where to store the error locator polynomial
 * @param omega Where to store the error evalutator polynomial
 * @return 1 in case of success
 *         0 in case of internal error
 *        -1 in case of memory allocation error
 */
int calculate_sigma_omega(struct gf_polynomial* syndromes, unsigned int n_error_correction_codewords,
                            struct gf_polynomial* *sigma, struct gf_polynomial* *omega);


/**
 * Given a sigma calculated for a message with errors, this function
 * returns all the values alpha^i so that (1/alpha^i) is a root of sigma.
 * As it turns out, each such alpha^i root indicates that there is an
 * error at the codeword corresponding to the degree i.
 *
 * For instance, with a message like A_BC_E_G...ZZZ with errors
 * at positions 1, 4 and 6, the error positions would correspond
 * to the monomials X^24, X^21 and and X^19, so the error locations
 * returned by the function would be alpha^24, alpha^21 and alpha^19
 *
 * @param sigma The error evaluator polynomial whose degree is the number
 *              of errors
 * @param locations An array of size degree(sigma) where to put the alpha^i
 *                  values
 * @return 1 on success or 0 if the number of roots does not match not match the
 *         expected number of errors which indicates that we cannot correct
 *         the errors
 */
int find_error_locations(struct gf_polynomial* sigma, u_int8_t* locations);


/**
 * Given the error evaluator polynomial and the error positions,
 * this function calculates the error magnitudes, i.e. for each error
 * position alpha^i calculates the value m so that adding m.X^i to the
 * message polynomial will correct the error.
 *
 * @param omega The error evaluator polynomial
 * @param n_errors The size of the error_locations and error_magnitudes arrays
 * @param error_locations The error position given as alpha^i where i is the degree
 *                        of the error in the message polynomial
 * @param error_magnitudes Where to store the error magnitudes
 */
void find_error_magnitudes(struct gf_polynomial* omega, unsigned int n_errors,
                            u_int8_t* error_locations, u_int8_t* error_magnitudes);


/**
 * Given a list of blocks, this function performs error detection/correction
 * on them and if all blocks can be decoded correctly, aggregates the data
 * bytes in a message buffer.
 *
 * @param blocks The (data+error) blocks from the QR code
 * @param message On success, *message will be allocated and filled with
 *                the correct data codewords
 * @return n > 0 on success, where n is the size of the message array
 *         0 if a block cannot be successfully decoded
 *        -1 in case of memory allocation error
 */
int get_message_bytes(struct blocks* blocks, u_int8_t* *message);

#endif

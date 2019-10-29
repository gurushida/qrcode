#ifndef _REEDSOLOMON_H
#define _REEDSOLOMON_H

#include "blocks.h"


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
 * @param b The single block to decode
 * @return On success, a value n>=0 representing the number of errors
 *         that were corrected; -1 if the block could not be decoded
 *         because there were too many errors
 */
int error_correction(struct block* b);


#endif

#ifndef _CODEWORDMASK_H
#define _CODEWORDMASK_H

#include "bitmatrix.h"
#include "errors.h"

/**
 * Given a a QR code size given in modules, this function
 * return a size x size matrix where all the cells corresponding
 * to data modules are set to 0 while all the other cells
 * (finder patterns, timing patterns, alignment patterns,
 * format information, version information) are set to 1.
 *
 * This matrix will be used to identify the modules to ignore
 * when scanning the QR code matrix for codewords.
 *
 * @param size The number of modules of one side of the QR code
 * @param mask Where to store the result
 * @return SUCCESS on success
 *         DECODING_ERROR if the given size is not a legal QR code size
 *         MEMORY_ERROR in case of memory allocation error
 */
int get_codeword_mask(unsigned int size, struct bit_matrix* *mask);

#endif

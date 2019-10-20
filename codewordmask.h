#ifndef _CODEWORDMASK_H
#define _CODEWORDMASK_H

#include "bitmatrix.h"

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
 * Returns NULL on memory error or if the given size is not
 * a legal QR code size.
 */
struct bit_matrix* get_codeword_mask(unsigned int size);

#endif

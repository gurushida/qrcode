#ifndef _CODEWORDS_H
#define _CODEWORDS_H

#include <stdint.h>
#include "bitmatrix.h"


/**
 * Given a module matrix, codeword mask matrix and the mask
 * pattern to apply to data modules, this function returns
 * an array containing all the 8-bit codewords contained in
 * the QR code represented bu the module matrix.
 *
 * @param modules The QR code bit matrix
 * @param codeword_mask A matrix where 1s represent function modules
 *                      that should be ignored when scanning for data
 *                      modules
 * @param mask_pattern A value between 0 and 7 that represent the mask
 *                     pattern to be applied to data modules
 * @param codewords The address where to store the codeword array that
 *                  will be dynamically allocated or NULL on error
 * @return n > 0 the number of decoded codewords on success, i.e. the size
 *               size of the codeword array
 *         0 if the matrix sizes are different or not valid QR code sizes,
 *           or if the mask_pattern value is not between 0 and 7
 *        -1 on memory allocation error
 */
int get_codewords(struct bit_matrix* modules,
                struct bit_matrix* codeword_mask,
                u_int8_t mask_pattern,
                u_int8_t* *codewords);

#endif

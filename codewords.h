#ifndef _CODEWORDS_H
#define _CODEWORDS_H

#include <stdint.h>
#include "bitmatrix.h"


/**
 * In order to convert modules into 8-bit words, the matrix is divided
 * into 2-module wide columns (we ignore the module column that contains
 * the vertical timing pattern as it does not contain any data module).
 * Starting from the bottom right corner, we scan the modules by going
 * upwards following this pattern:
 *
 *   01
 *   23
 *   45
 *   67
 *
 * where 7 is the most signicant bit of the current codeword. When we
 * reach the top of the column, we move one colum to the left and scan
 * downwards with the pattern:
 *
 *   67
 *   45
 *   23
 *   01
 *
 * As the exploration goes, the scanning will encounter non-data modules
 * that should be ignored. In such a case, we keep following the snake
 * scanning pattern and only take into account the data modules. For
 * instance, if we are scanning downwards and suddenly encounter the
 * corner of an alignment pattern, we will get something like this:
 *
 *   67
 *   45
 *   3*****
 *   2*****
 *   1*****
 *   0*****
 *   7*****
 *   56
 *   34
 *   12
 *    0
 *
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

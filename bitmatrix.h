#ifndef _BITMATRIX_H
#define _BITMATRIX_H

#include <stdint.h>
#include "errors.h"
#include "logs.h"


#define WHITE 0
#define BLACK 1


struct bit_matrix {
    unsigned int width;
    unsigned int height;
    u_int8_t* matrix;
};


/**
 * Allocates and returns a bit matrix of the given size where
 * all the bits are initialized to 0, i.e. white.
 * Returns NULL in case of memory allocation error.
 */
struct bit_matrix* create_bit_matrix(unsigned int width, unsigned int height);


/**
 * Frees the memory associated to the given bit matrix.
 */
void free_bit_matrix(struct bit_matrix* bm);


/**
 * Returns 1 if the pixel at the given coordinates is black; 0 otherwise.
 * Terminates the program if the coordinates are out of bounds.
 */
u_int8_t is_black(struct bit_matrix* bm, unsigned int x, unsigned int y);


/**
 * Sets the given value normalized to 1 (black) or 0 (white) at the given coordinates
 * or terminates the program if the coordinates are out of bounds.
 */
void set_color(struct bit_matrix* bm, u_int8_t value, unsigned int x, unsigned int y);


/**
 * Given a null-terminated array of null-terminated strings containing
 * either '*' or ' ', this function will create the corresponding bit matrix
 * by converting '*' to 1 and ' ' to 0. This is meant for test purposes.
 *
 * @param data A null-terminated string array where all strings must have the
 *             same length and only contain '*' or ' '
 * @param bm Where to store the result
 * @return SUCCESS on success
 *         DECODING_ERROR if the given array violates the criteria
 *         MEMORY_ERROR in case of memory allocation error
 */
int create_from_string(const char* data[], struct bit_matrix* *bm);


/**
 * Debug prints the matrix.
 */
void print_matrix(LogLevel level, struct bit_matrix* matrix);

#endif

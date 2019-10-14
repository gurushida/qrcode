#ifndef _BITMATRIX_H
#define _BITMATRIX_H

#include <stdint.h>

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

#endif

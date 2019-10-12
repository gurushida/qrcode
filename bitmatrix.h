#include <stdint.h>

struct bit_matrix {
    unsigned int width;
    unsigned int height;
    u_int8_t* matrix;
};


/**
 * Allocates and returns a bit matrix of the given size where
 * all the bits are initialized to 0, i.e. black.
 * Returns NULL in case of memory allocation error.
 */
struct bit_matrix* create_bit_matrix(unsigned int width, unsigned int height);


/**
 * Frees the memory associated to the given bit matrix.
 */
void free_bit_matrix(struct bit_matrix* bm);


/**
 * Returns 0 (black) or 1 (white) depending on the value of the bit at the given
 * coordinates or terminates the program if the coordinates are out of bounds.
 */
u_int8_t get_bit(struct bit_matrix* bm, unsigned int x, unsigned int y);


/**
 * Sets the given value normalized to 0 (black) or 1 (white) at the given coordinates
 * or terminates the program if the coordinates are out of bounds.
 */
void set_bit(struct bit_matrix* bm, u_int8_t value, unsigned int x, unsigned int y);

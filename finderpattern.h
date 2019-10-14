#ifndef _FINDERPATTERN_H
#define _FINDERPATTERN_H

#include "bitmatrix.h"

/**
 * This structure represents a possible location of a finder pattern,
 * i.e. one of the 2 big squares in the corners of a QR code.
 */
struct finder_pattern {
    // x,y represents the position of the finder pattern's center
    // given in pixels from the top left corner of the bit matrix
    float x;
    float y;

    // Approximate size in pixels of one module, i.e. one of the
    // squares that compose a QR code
    float module_size;
};


/**
 * This structure is used to maintain a list of possible positions.
 */
struct finder_pattern_list {
    // A possible position
    struct finder_pattern pattern;

    // The number of times the possible position has been matched.
    // The higher the count, the higher the chances the position
    // is actually the center of a finder pattern
    int count;

    struct finder_pattern_list* next;
};


/**
 * Given a bit matrix, returns a list of potential positions for
 * the finder patterns by looking for black/white/black/white/black
 * modules with 1:1:3:1:1 ratios.
 *
 * This function assumes that the edges of the QR code(s) are
 * reasonably parallel to the edges of the image.
 */
struct finder_pattern_list* find_potential_centers(struct bit_matrix* bm);


/**
 * Frees all the memory associated to the given list.
 */
void free_finder_pattern_list(struct finder_pattern_list* list);

#endif

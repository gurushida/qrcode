#ifndef _FINDERPATTERN_H
#define _FINDERPATTERN_H

#include "bitmatrix.h"
#include "errors.h"


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
 * the finder/alignment patterns by looking for black/white/black/white/black
 * modules with 1:1:3:1:1 or 1:1:1:1:1 ratios.
 *
 * This function assumes that the edges of the QR code(s) are
 * reasonably parallel to the edges of the image.
 *
 * @param bm The binary matrix to explore
 * @param search_finder_pattern If non zero, the function looks for finder patterns,
 *                              i.e. patterns with 1:1:3:1:1 ratios; if zero, it looks
 *                              for alignment patterns that have 1:1:1:1:1 ratios
 * @param list Where to store the result
 * @return SUCCESS on success
 *         DECODING_ERROR if no center can be found
 *         MEMORY_ERROR in case of memory allocation error
 */
int find_potential_centers(struct bit_matrix* bm, int search_finder_pattern, struct finder_pattern_list* *list);


/**
 * Frees all the memory associated to the given list.
 */
void free_finder_pattern_list(struct finder_pattern_list* list);


/**
 * Returns the number of elements in the given list.
 */
unsigned int get_list_size(struct finder_pattern_list* list);

#endif

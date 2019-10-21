#include <stdlib.h>
#include "bitmatrix.h"


/**
 * A QR code has a version between 1 and 40. For each version above 1,
 * there are alignment patterns defined by the following array that encodes
 * the alignement pattern positions as follows:
 *
 * - code[x] describes the patterns of version (x + 1) as a zero-terminated list
 *   of positions
 * - given a list (x, y, ....), each combination of coordinates
 *   such as (x,x), (x,y), (y,x) and so on describes the center
 *   position of an alignement pattern, except if the coordinates are inside one
 *   of the 3 finder patterns
 */
static u_int8_t alignment_patterns[40][8] = {
    { 0 },
    { 6, 18, 0 },
    { 6, 22, 0 },
    { 6, 26, 0 },
    { 6, 30, 0 },
    { 6, 34, 0 },
    { 6, 22, 38, 0 },
    { 6, 24, 42, 0 },
    { 6, 26, 46, 0 },
    { 6, 28, 50, 0 },
    { 6, 30, 54, 0 },
    { 6, 32, 58, 0 },
    { 6, 34, 62, 0 },
    { 6, 26, 46, 66, 0 },
    { 6, 26, 48, 70, 0 },
    { 6, 26, 50, 74, 0 },
    { 6, 30, 54, 78, 0 },
    { 6, 30, 56, 82, 0 },
    { 6, 30, 58, 86, 0 },
    { 6, 34, 62, 90, 0 },
    { 6, 28, 50, 72, 94, 0 },
    { 6, 26, 50, 74, 98, 0 },
    { 6, 30, 54, 78, 102, 0 },
    { 6, 28, 54, 80, 106, 0 },
    { 6, 32, 58, 84, 110, 0 },
    { 6, 30, 58, 86, 114, 0 },
    { 6, 34, 62, 90, 118, 0 },
    { 6, 26, 50, 74, 98, 122, 0 },
    { 6, 30, 54, 78, 102, 126, 0 },
    { 6, 26, 52, 78, 104, 130, 0 },
    { 6, 30, 56, 82, 108, 134, 0 },
    { 6, 34, 60, 86, 112, 138, 0 },
    { 6, 30, 58, 86, 114, 142, 0 },
    { 6, 34, 62, 90, 118, 146, 0 },
    { 6, 30, 54, 78, 102, 126, 150, 0 },
    { 6, 24, 50, 76, 106, 128, 154, 0 },
    { 6, 28, 54, 80, 106, 132, 158, 0 },
    { 6, 32, 58, 84, 110, 136, 162, 0 },
    { 6, 26, 54, 82, 110, 138, 166, 0 },
    { 6, 30, 58, 86, 114, 142, 170, 0 }
};


struct bit_matrix* get_codeword_mask(unsigned int size) {
    if (size < 21
        || size > 177
        || (size % 4) != 1) {
            return NULL;
    }

    struct bit_matrix* bm = create_bit_matrix(size, size);
    if (bm == NULL) {
        return NULL;
    }

    // For the top left finder pattern, we include the white border
    // and the format information modules
    for (unsigned int y = 0 ; y < 9 ; y++) {
        for (unsigned int x = 0 ; x < 9 ; x++) {
            set_color(bm, BLACK, x, y);
        }
    }

    // For the bottom left finder pattern, we include the white border
    // and the format modules on the right
    for (unsigned int y = size - 8 ; y < size ; y++) {
        for (unsigned int x = 0 ; x < 9 ; x++) {
            set_color(bm, BLACK, x, y);
        }
    }

    // For the top right finder pattern, we include the white border
    // and the format modules under it
    for (unsigned int y = 0 ; y < 9 ; y++) {
        for (unsigned int x = size - 8 ; x < size ; x++) {
            set_color(bm, BLACK, x, y);
        }
    }

    // Horizontal timing pattern
    for (unsigned int x = 8 ; x < size - 8 ; x++) {
        set_color(bm, BLACK, x, 6);
    }

    // Vertical timing pattern
    for (unsigned int y = 8 ; y < size - 8 ; y++) {
        set_color(bm, BLACK, 6, y);
    }

    // Version information modules above the bottom left finder pattern
    for (unsigned int y = size - 11 ; y < size - 8 ; y++) {
        for (unsigned int x = 0 ; x < 6 ; x++) {
            set_color(bm, BLACK, x, y);
        }
    }

    // Version information modules on the left of the top right finder pattern
    for (unsigned int y = 0 ; y < 6 ; y++) {
        for (unsigned int x = size - 11 ; x < size - 8 ; x++) {
            set_color(bm, BLACK, x, y);
        }
    }

    u_int8_t version = (size - 17) / 4;
    u_int8_t* pos = alignment_patterns[version - 1];

    for (unsigned int i = 0 ; pos[i] != 0 ; i++) {
        for (unsigned int j = 0 ; pos[j] != 0 ; j++) {
            unsigned int x = pos[i];
            unsigned int y = pos[j];
            if (x == 6 && y == 6) {
                // Inside the top left finder pattern
                continue;
            }
            if (x == 6 && y == size - 7) {
                // Inside the bottom left finder pattern
                continue;
            }
            if (x == size - 7 && y == 6) {
                // Inside the top right finder pattern
                continue;
            }

            for (unsigned int ap_y = y - 1 ; ap_y <= y + 1 ; ap_y++) {
                for (unsigned int ap_x = x - 1 ; ap_x <= x + 1 ; ap_x++) {
                    set_color(bm, BLACK, ap_x, ap_y);
                }
            }
        }
    }

    return bm;
}

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "qrcodefinder.h"


static float get_distance(float x1, float y1, float x2, float y2) {
    return sqrtf((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}


/**
 * Returns the estimated dimension of the potential QR code, i.e.
 * the number of modules on the side of the QR code.
 *
 * QR codes are designed in such a way that the dimension has
 * to be a number of the form 4x+1. If we cannot find such a value,
 * we return -1. Otherwise, we return the dimension.
 */
static int get_dimension(struct finder_pattern bottom_left,
                            struct finder_pattern top_left,
                            struct finder_pattern top_right,
                            float module_size) {
    float distance1 = get_distance(bottom_left.x, bottom_left.y, top_left.x, top_left.y);
    float distance2 = get_distance(top_left.x, top_left.y, top_right.x, top_right.y);
    // Since the distances are between the centers of the finder patterns, we need to add 7
    // to get the actual size from one corner to another
    unsigned int dimension = 7 + (unsigned int)((distance1 + distance2) / (2.0f * module_size));

    // If the dimension ok or off by one, we adjust it if needed
    // and return it. If not, we give up
    switch (dimension % 4) {
        case 0: return dimension + 1;
        case 1: return dimension;
        case 2: return dimension - 1;
        default: return -1;
    }
}


/**
 * Returns the subregion of the given image defined by the
 * given bounds.
 */
static struct bit_matrix* create_search_area(struct bit_matrix* image,
                                unsigned int minX, unsigned int minY,
                                unsigned int maxX, unsigned int maxY) {
    if (minX > maxX || minY > maxY) {
        fprintf(stderr, "Internal error\n");
        exit(1);
    }
    struct bit_matrix* m = create_bit_matrix(maxX + 1 - minX, maxY + 1 - minY);
    if (m == NULL) {
        return NULL;
    }

    for (unsigned int y = minY ; y <= maxY ; y++) {
        for (unsigned int x = minX ; x <= maxX ; x++) {
            set_color(m, is_black(image, x, y) ? BLACK : WHITE, x - minX, y - minY);
        }
    }

    return m;
}


/**
 * Each QR code with a dimension > 21 has at least one alignment pattern like this:
 *
 *   #######           #######
 *   #     #           #     #
 *   # ### #           # ### #
 *   # ### #           # ### #
 *   # ### #           # ### #
 *   #     #           #     #
 *   #######           #######
 *                     |
 *                     |
 *                     |
 *                     |
 *                     |
 *                     |
 *                     |
 *                     |
 *                     | 
 *                   #####
 *                   # | #
 *   #######---------#-# #
 *   #     #         #   #
 *   # ### #         #####
 *   # ### #
 *   # ### #
 *   #     #
 *   #######
 *
 * This function looks for this pattern and uses it to predict the coordinates
 * of where the bottom right finder pattern would be if there was one.
 * If the dimension is 21 or if no pattern can be found, the position is
 * just guessed.
 *
 * Returns 1 if a guess was made or 0 if the guess is out of the bounds of the image
 * which indicates that the given finder patterns are not a good QR code candidate.
 */
static int find_bottom_right_finder_pattern(struct finder_pattern bottom_left,
                            struct finder_pattern top_left,
                            struct finder_pattern top_right,
                            struct bit_matrix* image,
                            float module_size,
                            int dimension,
                            float *bottom_right_x, float *bottom_right_y) {

    *bottom_right_x = bottom_left.x + (top_right.x - top_left.x);
    *bottom_right_y = top_right.y + (bottom_left.y - top_left.y);
    if (*bottom_right_x < 0 || *bottom_right_y < 0
        || *bottom_right_x >= image->width || *bottom_right_y >= image->height) {
        return 0;
    }

    if (dimension == 21) {
        return 1;
    }

    // Let's see if we can find an alignment pattern. Starting from
    // the center of the virtual bottom right finder pattern,
    // the center of the alignment center is 3 modules along the diagonal
    // to the top left finder pattern.
    float modules_between_finder_patterns = dimension - 7.0;
    float ratio  = (modules_between_finder_patterns - 3.0) / modules_between_finder_patterns;
    float alignment_x = top_left.x + ratio * (*bottom_right_x - top_left.x);
    float alignment_y = top_left.y + ratio * (*bottom_right_y - top_left.y);

    // Let's look around this position for the black/white/black/white/black pattern
    // with 1:1:1:1:1 ratios. We do this by apply the pattern detection process
    // to a small matrix centered around the potential position;
    unsigned int minX = (unsigned int)fmax(0, alignment_x - 3 * module_size);
    unsigned int maxX = (unsigned int)fmin(image->width - 1, alignment_x + 3 * module_size);
    unsigned int minY = (unsigned int)fmax(0, alignment_y - 3 * module_size);
    unsigned int maxY = (unsigned int)fmin(image->height - 1, alignment_y + 3 * module_size);

    struct bit_matrix* search_area = create_search_area(image, minX, minY, maxX, maxY);
    struct finder_pattern_list* candidates = find_potential_centers(search_area, 0);
    free_bit_matrix(search_area);

    if (candidates != NULL) {
        // We don't expect to find more than one pattern in such a small search area.
        // Let's recalculate the coordinates of the virtual bottom right finder pattern
        // based on the top left one and on the pattern we just found
        *bottom_right_x = top_left.x + (alignment_x - top_left.x) / ratio;
        *bottom_right_y = top_left.y + (alignment_y - top_left.y) / ratio;
        free_finder_pattern_list(candidates);
    }

    return 1;
}


/**
 * Given 2 points P1 and P2 and a the distance between them given in modules,
 * this function converts a position given in modules from p1 into the coordinates
 * of a point.
 */
static void interpolate(float x1, float y1, float x2, float y2, unsigned int distance_in_modules, int pos,
                        float *result_x, float *result_y) {
    *result_x = x1 + (pos / (float)distance_in_modules) * (x2 - x1);
    *result_y = y1 + (pos / (float)distance_in_modules) * (y2 - y1);
}


/**
 * Given the positions of the 4 finder patterns' centers (3 real ones + virtual bottom right one),
 * the original image and the dimension, this function populate the given QR code structure from
 * the original binary image. If part of the QR code is outside the image, we assume arbitrarily that
 * the missing modules are white.
 */
static void populate_qr_code(struct qr_code* code, struct bit_matrix* image, int dimension,
                            struct finder_pattern bottom_left,
                            struct finder_pattern top_left,
                            struct finder_pattern top_right,
                            float bottom_right_x, float bottom_right_y) {
    // We have a QR code defined by 4 patterns whose centers are each 4 modules into the QR code
    // like this:
    //
    // +-----------------------------------+
    // +                                   +
    // +   B                           C   +
    // +                                   +
    // +                                   +
    // +                                   +
    // +                                   +
    // +                                   +
    // +                                   +
    // +                                   +
    // +                                   +
    // +                                   +
    // +   A                           D   +
    // +                                   +
    // +-----------------------------------+
    //
    // For each row y, we interpolate 2 points, one point P_left on BA
    // and another point P_right on CD. Then, for each column x, we
    // interpolate a point M between P_left and P_right.
    // The coordinates of M are the center of the module (x,y)
    // in the original image. We then just need to look at the color of
    // this pixel and use it as the value for the module (x,y) in the
    // QR code

    // Let's consider that the center module of the top left finder pattern
    // is at 0,0
    for (int y = -3 ; y < (dimension - 3) ; y++) {
        float p_left_x, p_left_y;
        float p_right_x, p_right_y;

        interpolate(top_left.x, top_left.y, bottom_left.x, bottom_left.y,
                    dimension - 7, y, &p_left_x, &p_left_y);
        interpolate(top_right.x, top_right.y, bottom_right_x, bottom_right_y,
                   dimension - 7, y, &p_right_x, &p_right_y);

        for (int x = -3 ; x < (dimension - 3) ; x++) {
            float m_x, m_y;
            interpolate(p_left_x, p_left_y, p_right_x, p_right_y,
                    dimension - 7, x, &m_x, &m_y);

            // In case we reach a position outside the image, let's
            // default to white
            int _mx = (int)m_x;
            int _my = (int)m_y;
            int outside = _mx < 0 || _mx >= image->width || _my < 0 || _my >= image->height;

            int black = outside ? 0 : is_black(image, (int)m_x, (int)m_y);
            set_color(code->modules, black ? BLACK : WHITE, x + 3, y + 3);

            // If M is on a corner, let's update the QR code bounds
            if (y == -3) {
                if (x == -3) {
                    code->top_left_x = (int)m_x;
                    code->top_left_y = (int)m_y;
                } else if (x == dimension - 4) {
                    code->top_right_x = (int)m_x;
                    code->top_right_y = (int)m_y;
                }
            } else if (y == dimension - 4) {
                if (x == -3) {
                    code->bottom_left_x = (int)m_x;
                    code->bottom_left_y = (int)m_y;
                } else if (x == dimension - 4) {
                    code->bottom_right_x = (int)m_x;
                    code->bottom_right_y = (int)m_y;
                }
            }
        }
    }
}



struct qr_code* get_qr_code(struct finder_pattern bottom_left,
                            struct finder_pattern top_left,
                            struct finder_pattern top_right,
                            struct bit_matrix* image) {
    float module_size = (bottom_left.module_size + top_left.module_size + top_right.module_size) / 3.0f;
    int dimension = get_dimension(bottom_left, top_left, top_right, module_size);
    if (-1 == dimension) {
        return NULL;
    }

    float x, y;
    if (!find_bottom_right_finder_pattern(bottom_left, top_left, top_right, image, module_size, dimension, &x, &y)) {
        return NULL;
    }

    struct qr_code* code = (struct qr_code*)malloc(sizeof(struct qr_code));
    if (code == NULL) {
        return NULL;
    }
    code->modules = create_bit_matrix(dimension, dimension);
    if (code->modules == NULL) {
        free(code);
        return NULL;
    }

    populate_qr_code(code, image, dimension, bottom_left, top_left, top_right, x, y);
    return code;
}


void free_qr_code(struct qr_code* code) {
    free_bit_matrix(code->modules);
    free(code);
}

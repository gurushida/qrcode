#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmatrix.h"
#include "finderpattern.h"


static int check_potential_center(struct bit_matrix* bm, unsigned int pixel_counts[],
                            unsigned int x, unsigned int y, struct finder_pattern_list* *list);


struct finder_pattern_list* find_potential_centers(struct bit_matrix* bm) {
    unsigned int maxY = bm->height;
    unsigned int maxX = bm->width;

    struct finder_pattern_list* list = NULL;
    unsigned int pixel_counts[5];

    for (unsigned int y = 0 ; y < maxY ; y++) {
        memset(pixel_counts, 0, 5 * sizeof(int));

        int current_state = 0;
        for (unsigned int x = 0 ; x < maxX ; x++) {
            if (is_black(bm, x, y)) {
                if ((current_state % 2) == 1) {
                    // If we are currently counting white pixels, we move on
                    // to counting black pixels
                    current_state++;
                }
                pixel_counts[current_state]++;
            }
            else { // We have a white pixel

                if ((current_state % 2) == 0) {
                    // If we are currently counting black pixels

                    if (current_state == 4) {
                        // We have now found a b/w/b/w/b pattern.
                        // We need to check if it looks like a finder pattern
                        if (check_potential_center(bm, pixel_counts, x, y, &list)) {
                            // Let's reset the counts before continuing to look for more
                            current_state = 0;
                            memset(pixel_counts, 0, 5 * sizeof(int));
                        } else {
                            // Let's shift the pixel counts by 2
                            pixel_counts[0] = pixel_counts[2];
                            pixel_counts[1] = pixel_counts[3];
                            pixel_counts[2] = pixel_counts[4];
                            pixel_counts[3] = 1; // The white pixel we just found
                            pixel_counts[4] = 0;
                            current_state = 3;
                        }
                    } else {
                        current_state++;
                        pixel_counts[current_state]++;
                    }
                } else {
                    pixel_counts[current_state]++;
                }
            }
        }
        // A valid match may be ended by the right edge of the image rather than a white pixel
        check_potential_center(bm, pixel_counts, maxX, y, &list);
    }

    return list;
}


struct finder_pattern_list* create_finder_pattern_list(float x, float y, float module_size) {
    struct finder_pattern_list* list = (struct finder_pattern_list*)malloc(sizeof(struct finder_pattern_list));
    if (list == NULL) {
        return NULL;
    }
    list->pattern.x = x;
    list->pattern.y = y;
    list->pattern.module_size = module_size;
    list->count = 1;
    list->next = NULL;
    return list;
}


void free_finder_pattern_list(struct finder_pattern_list* list) {
    struct finder_pattern_list* next;
    while (list != NULL) {
        next = list->next;
        free(list);
        list = next;
    }
}


/**
 * Returns 1 if the given black and white ratios are close enough
 * to 1:1:3:1:1; 0 otherwise.
 */
static int proper_ratios(unsigned int pixel_counts[]) {
    unsigned int total_pixels = 0;
    for (int i = 0 ; i < 5 ; i++) {
        if (pixel_counts[i] == 0) {
            return 0;
        }
        total_pixels += pixel_counts[i];
    }

    if (total_pixels < 7) {
        return 0;
    }

    float module_size = total_pixels / 7.0f;
    // We allow less than 50% difference between expected and actual values
    float max_variance = module_size / 2.0f;

    return fabs(module_size - pixel_counts[0]) < max_variance
        && fabs(module_size - pixel_counts[1]) < max_variance
        && fabs(3.0f * module_size - pixel_counts[2]) < (3.0f * max_variance)
        && fabs(module_size - pixel_counts[3]) < max_variance
        && fabs(module_size - pixel_counts[4]) < max_variance;
}


/**
 * Returns the estimated coordinate of the center of the sequence.
 */
static float get_center(unsigned int pixel_counts[], int end) {
    return (end - pixel_counts[4] - pixel_counts[3]) - pixel_counts[2] / 2.0f;
}


/**
 * Given a centerX position calculated on the given row, this function will
 * try to confirm the horizontal match by looking for a vertical match at the
 * centerX position. Returns 1 if the match is confirmed; 0 otherwise.
 *
 * @param bm The binary image
 * @param centerX The x position we are trying to confirm
 * @param row The row where the given x position was found as a candidate
 * @param max_pixels_per_module A limit to be used as a fail fast when looking for the
 *                              vertical pixel counts
 * @param total_pixels The total number of pixels that made the horizontal match
 * @param centerY Where to store the position of the vertical center in case of success
 */
static int check_vertically(struct bit_matrix* bm, int centerX, int row, unsigned max_pixels_per_module,
                    unsigned int total_pixels, float *centerY) {
    unsigned int pixel_counts[5] = { 0, 0, 0, 0, 0 };

    unsigned int y = row;
    while (y > 0 && is_black(bm, centerX, y)) {
        pixel_counts[2]++;
        y--;
    }
    if (y == 0) {
        return 0;
    }

    while (y > 0 && !is_black(bm, centerX, y)) {
        if (++pixel_counts[1] > max_pixels_per_module) {
            return 0;
        }
        y--;
    }
    if (y == 0) {
        return 0;
    }

    while (y >= 0 && is_black(bm, centerX, y)) {
        if (++pixel_counts[0] > max_pixels_per_module) {
            return 0;
        }
        y--;
    }

    y = row + 1;
    while (y < bm->height && is_black(bm, centerX, y)) {
        pixel_counts[2]++;
        y++;
    }
    if (y == bm->height) {
        return 0;
    }

    while (y < bm->height && !is_black(bm, centerX, y)) {
        if (++pixel_counts[3] > max_pixels_per_module) {
            return 0;
        }
        y++;
    }
    if (y == bm->height) {
        return 0;
    }

    while (y < bm->height && is_black(bm, centerX, y)) {
        if (++pixel_counts[4] > max_pixels_per_module) {
            return 0;
        }
        y++;
    }

    // We now have our vertical pixel counts. Let's check if the ratios are good
    if (!proper_ratios(pixel_counts)) {
        return 0;
    }

    unsigned int vertical_total_pixels = pixel_counts[0] + pixel_counts[1] + pixel_counts[2] +
                                pixel_counts[3] + pixel_counts[4];
    if (5 * (unsigned int)abs((int)vertical_total_pixels - (int)total_pixels) >= 2 * total_pixels) {
        return 0;
    }

    *centerY = get_center(pixel_counts, y);
    return 1;
}


/**
 * Given a centerY position calculated on the given colmun, this function will
 * try to confirm the vertical match by looking for an horizontal match at the
 * centerY position. Returns 1 if the match is confirmed; 0 otherwise.
 *
 * @param bm The binary image
 * @param centerY The y position we are trying to confirm
 * @param column The colmun where the given y position was found as a candidate
 * @param max_pixels_per_module A limit to be used as a fail fast when looking for the
 *                              horizontal pixel counts
 * @param total_pixels The total number of pixels that made the vertical match
 * @param centerX Where to store the position of the horizontal center in case of success
 */
static int check_horizontally(struct bit_matrix* bm, int centerY, int column, unsigned max_pixels_per_module,
                    unsigned int total_pixels, float *centerX) {
    unsigned int pixel_counts[5] = { 0, 0, 0, 0, 0 };

    unsigned int x = column;
    while (x > 0 && is_black(bm, x, centerY)) {
        pixel_counts[2]++;
        x--;
    }
    if (x == 0) {
        return 0;
    }

    while (x > 0 && !is_black(bm, x, centerY)) {
        if (++pixel_counts[1] > max_pixels_per_module) {
            return 0;
        }
        x--;
    }
    if (x == 0) {
        return 0;
    }

    while (x >= 0 && is_black(bm, x, centerY)) {
        if (++pixel_counts[0] > max_pixels_per_module) {
            return 0;
        }
        x--;
    }

    x = column + 1;
    while (x < bm->width && is_black(bm, x, centerY)) {
        pixel_counts[2]++;
        x++;
    }
    if (x == bm->width) {
        return 0;
    }

    while (x < bm->width && !is_black(bm, x, centerY)) {
        if (++pixel_counts[3] > max_pixels_per_module) {
            return 0;
        }
        x++;
    }
    if (x == bm->width) {
        return 0;
    }

    while (x < bm->width && is_black(bm, x, centerY)) {
        if (++pixel_counts[4] > max_pixels_per_module) {
            return 0;
        }
        x++;
    }

    // We now have our horizontal pixel counts. Let's check if the ratios are good
    if (!proper_ratios(pixel_counts)) {
        return 0;
    }

    unsigned int horizontal_total_pixels = pixel_counts[0] + pixel_counts[1] + pixel_counts[2] +
                                pixel_counts[3] + pixel_counts[4];
    if (5 * (unsigned int)abs((int)horizontal_total_pixels - (int)total_pixels) >= 2 * total_pixels) {
        return 0;
    }

    *centerX = get_center(pixel_counts, x);
    return 1;
}


/**
 * Returns 1 the given patterns have approximately the same center and module size; 0 otherwise.
 */
static int pattern_close_enough(struct finder_pattern_list* list, float centerX, float centerY, float estimated_module_size) {
    if ((fabs(list->pattern.x - centerX) <= estimated_module_size) && (fabs(list->pattern.y - centerY) <= estimated_module_size)) {
        float size_diff = fabs(list->pattern.module_size - estimated_module_size);
        return size_diff <= 1.0f || size_diff <= list->pattern.module_size;
    }
    return 0;
}


static void combine_patterns(struct finder_pattern_list* list, float centerX, float centerY, float estimated_module_size) {
    list->pattern.x = (list->count * list->pattern.x + centerX) / (list->count + 1);
    list->pattern.y = (list->count * list->pattern.y + centerY) / (list->count + 1);
    list->pattern.module_size = (list->count * list->pattern.module_size + estimated_module_size) / (list->count + 1);
    (list->count)++;
}


/**
 * Adds the given match to the list. If the list already contains a match
 * that is close enough to the new one, the existing match is updated.
 * Return 1 on success, 0 on memory allocation error.
 */
static int handle_potential_center(struct finder_pattern_list* *list, float centerX, float centerY, float estimated_module_size) {
    struct finder_pattern_list* tmp = (*list);
    while (tmp != NULL) {
        if (pattern_close_enough(tmp, centerX, centerY, estimated_module_size)) {
            combine_patterns(tmp, centerX, centerY, estimated_module_size);
            return 1;
        }

        tmp = tmp->next;
    }

    // We haven't found any item in the list close enough to our match.
    // Let's add it
    tmp = create_finder_pattern_list(centerX, centerY, estimated_module_size);
    if (tmp == NULL) {
        return 0;
    }
    tmp->next = (*list);
    (*list) = tmp;
    return 1;
}


/**
 * If the given module counts found at the given x,y position corresponds
 * indeed to a potential finder pattern center, returns 1 and add this match
 * to the given list. Returns 0 otherwise.
 *
 * @param bm The binary image
 * @param module_counts The black:white:black:white:black pixel counts that we wan
 *                      to check
 * @param xEnd The x coordinate of the first white pixel after the candidate sequence
 * @param y The row where the sequence was found
 * @param list The list where to add matches
 */
static int check_potential_center(struct bit_matrix* bm, unsigned int pixel_counts[],
                            unsigned int xEnd, unsigned int y, struct finder_pattern_list* *list) {

    if (!proper_ratios(pixel_counts)) {
        return 0;
    }

    int total_pixels = pixel_counts[0] + pixel_counts[1] + pixel_counts[2] +
                        pixel_counts[3] + pixel_counts[4];
    float centerX = get_center(pixel_counts, xEnd);
    float centerY;
    if (!check_vertically(bm, (unsigned int)centerX, y, pixel_counts[2], total_pixels, &centerY)) {
        return 0;
    }
    if (!check_horizontally(bm, (unsigned int)centerY, (int)centerX, pixel_counts[2], total_pixels, &centerX)) {
        return 0;
    }

    float estimated_module_size = total_pixels / 7.0f;
    return handle_potential_center(list, centerX, centerY, estimated_module_size);
}

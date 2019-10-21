#include <stdio.h>
#include <stdlib.h>
#include "codewords.h"


/**
 * Given a position x,y and a direction (upwards or downwards),
 * this function updates x and y so that they point to the next data module.
 * *upwards and *right will also be updated.
 *
 * @param x The address of x
 * @param y The address of y
 * @param codeword_mask The codeword mask that indicates while modules
 *                      to take into account
 * @param upwards Contains 1 if we are currently moving upwards, 0 if downwards
 * @param right Contains 1 if the given x,y position is on the right side of
 *             the column that is currently scanned, 0 otherwise
 */
static void move_to_next_data_module(unsigned int *x, unsigned int *y, struct bit_matrix* codeword_mask,
                                    uint8_t *upwards, uint8_t *right) {
    do {
        if (*right) {
            // When we are on the right side, the next candidate is always on the left side
            (*x)--;
            (*right) = 0;
        }
        else if (*upwards) {
            (*right) = 1;
            if ((*y) > 0) {
                // If we can move up, let's do it
                (*x)++;
                (*y)--;
            } else {
                // We have reached the top of the matrix, we need to
                // continue downwards one column to the left
                (*upwards) = 0;
                (*x)--;
                if ((*x) == 6) {
                    // The column 6 must be ignored entirely as it cannot contain
                    // any data module because of the left finder patterns and the vertical
                    // timing line
                    (*x)--;
                }
            }
        }
        else {
            // Moving downwards
            (*right) = 1;
            if ((*y) < codeword_mask->height - 1) {
                // We can move down
                (*x)++;
                (*y)++;
            } else {
                // We have reached to bottom of the matrix, let's continue
                // upwards one column to the left
                (*upwards) = 1;
                (*x)--;
                if ((*x) == 6) {
                    // Same reason as above
                    (*x)--;
                }
            }
        }
    } while (is_black(codeword_mask, *x, *y));
}


/**
 * Given a position (x,y) this function returns the value of the bit at
 * this position in the matrix after applying the given mask pattern.
 */
static int get_data_bit(struct bit_matrix* modules, unsigned x ,unsigned y, u_int8_t mask_pattern) {
    int raw_bit = is_black(modules, x, y);

    // In the ISO specification, the mask patterns are defined with i as the
    // row and j as the column, so let's use the same variables here
    unsigned int i = y;
    unsigned int j = x;

    int mask_bit;

    switch (mask_pattern) {
        case 0: {
            mask_bit = ((i + j) % 2) == 0;
            break;
        }
        case 1: {
            mask_bit = (i % 2) == 0;
            break;
        }
        case 2: {
            mask_bit = (j % 3) == 0;
            break;
        }
        case 3: {
            mask_bit = ((i + j) % 3) == 0;
            break;
        }
        case 4: {
            mask_bit = (((i / 2) + (j / 3)) % 2) == 0;
            break;
        }
        case 5: {
            mask_bit = ((i * j) % 2) + ((i * j) % 3) == 0;
            break;
        }
        case 6: {
            mask_bit = (((i * j) % 2 + (i * j) % 3) % 2) == 0;
            break;
        }
        case 7: {
            mask_bit = ((i * j) % 3 + i + j) % 2 == 0;
            break;
        }
        default: {
            fprintf(stderr, "Illegal mask pattern %d\n", mask_pattern);
            exit(1);
        }
    }

    return raw_bit ^ mask_bit;
}


int get_codewords(struct bit_matrix* modules,
                struct bit_matrix* codeword_mask,
                u_int8_t mask_pattern,
                u_int8_t* *codewords) {
    *codewords = NULL;
    if (modules->width != modules->height
        || modules->width != codeword_mask->width
        || modules->width != codeword_mask->height
        || (modules->width % 4) != 1
        || mask_pattern >= 8) {
            return 0;
        }

    unsigned int size = modules->width;

    // Let's count the white modules in the codeword mask,
    // i.e. the number of modules that can be part of
    // decoded codewords
    int n = 0;
    for (unsigned int y = 0 ; y < size ; y++) {
        for (unsigned int x = 0 ; x < size ; x++) {
            if (!is_black(codeword_mask, x, y)) {
                n++;
            }
        }
    }

    // Let's divide by 8 to get the number of codewords
    n = n / 8;
    (*codewords) = (u_int8_t*)malloc(n * sizeof(u_int8_t));
    if (*codewords == NULL) {
        return -1;
    }

    unsigned int x = size - 1;
    unsigned int y = size - 1;
    u_int8_t upwards = 1;
    u_int8_t right = 1;

    for (int i = 0 ; i < n ; i++) {
        int bit_pos = 7;
        u_int8_t codeword = 0;
        do {
            int bit = get_data_bit(modules, x ,y, mask_pattern);
            codeword = codeword | (bit << bit_pos);

            move_to_next_data_module(&x, &y, codeword_mask, &upwards, &right);
            bit_pos--;
        } while (bit_pos >= 0);
        (*codewords)[i] = codeword;
    }

    return n;
}


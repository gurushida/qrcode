#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "binarize.h"

static unsigned int BLOCK_SIZE = 8;
static unsigned int MIN_DYNAMIC_RANGE = 24;

/**
 * Returns a matrix giving for each RGB pixel an 8-bit luminance value.
 */
static u_int8_t* calculate_luminances(struct rgb_image* img) {
    unsigned int size = img->width * img->height;
    u_int8_t* luminances = (u_int8_t*)malloc(size * sizeof(u_int8_t));
    if (luminances == NULL) {
        return NULL;
    }
    int j = 0;
    unsigned int size_rgb_buffer = size * 3;
    for (unsigned int offset = 0 ; offset < size_rgb_buffer ; offset += 3) {
        u_int8_t red = img->buffer[offset];
        u_int8_t green = img->buffer[offset + 1];
        u_int8_t blue = img->buffer[offset + 2];
        // The human eye perceives green approximately twice as much
        // as red and blue when it comes to brightness
        luminances[j++] = (u_int8_t) ((red + green * 2 + blue) >> 2);
    }

    return luminances;
}


/**
 * For each block of 8x8 pixels, this function calculates a
 * threshold value representing the limit between black and white.
 */
static u_int8_t* calculate_black_points(u_int8_t* luminances,
                unsigned int subWidth, unsigned int subHeight,
                unsigned int width, unsigned int height) {

    unsigned int size = subWidth * subHeight;
    u_int8_t* black_points = (u_int8_t*)malloc(size * sizeof(u_int8_t));
    if (black_points == NULL) {
        return NULL;
    }

    for (unsigned int y = 0 ; y < subHeight ; y++) {
        for (unsigned int x = 0 ; x < subWidth ; x++) {
            unsigned int sum = 0;
            unsigned int min = 0xFF;
            unsigned int max = 0;

            unsigned int maxX = (x + 1) * BLOCK_SIZE;
            if (maxX > width) {
                maxX = width;
            }
            unsigned int maxY = (y + 1) * BLOCK_SIZE;
            if (maxY > height) {
                maxY = height;
            }
            unsigned int n = 0;
            for (unsigned int yy = y * BLOCK_SIZE ; yy < maxY ; yy++) {
                for (unsigned int xx = x * BLOCK_SIZE ; xx < maxX ; xx++) {
                    u_int8_t pixel = luminances[yy * width + xx];

                    sum += pixel;
                    n++;

                    if (pixel < min) {
                        min = pixel;
                    }
                    if (pixel > max) {
                        max = pixel;
                    }
                }
            }

            unsigned int average = sum / n;
            if ((max - min) <= MIN_DYNAMIC_RANGE) {
                average = min / 2;
                if (y > 0 && x > 0) {
                    unsigned int averageNeighborBlackPoint =
                        (black_points[(y - 1) * subWidth + x]
                        + (2 * black_points[y * subWidth + x - 1])
                        + black_points[(y - 1) * subWidth +x - 1]) / 4;

                    if (min < averageNeighborBlackPoint) {
                        average = averageNeighborBlackPoint;
                    }
                }
            }

            black_points[y * subWidth + x] = average;
        }
    }

    return black_points;
}


static int cap(int value, int max) {
    return value < 2 ? 2 : (value < max ? value : max);
}


static void threshold_block(u_int8_t* luminances,
    unsigned int x,
    unsigned int y,
    unsigned int threshold,
    struct bit_matrix* bm) {

    unsigned int maxX = x + BLOCK_SIZE;
    if (maxX > bm->width) {
        maxX = bm->width;
    }
    unsigned int maxY = y + BLOCK_SIZE;
    if (maxY > bm->height) {
        maxY = bm->height;
    }
    for (unsigned int yy = y ; yy < maxY ; yy++) {
        for (unsigned int xx = x ; xx < maxX ; xx++) {
            if (luminances[yy * bm->width + xx] <= threshold) {
                set_color(bm, BLACK, xx, yy);
            }
        }
    }
}


static void calculate_threshold_for_blocks(u_int8_t* luminances,
    unsigned int subWidth, unsigned int subHeight, unsigned int width, unsigned int height,
    u_int8_t* black_points, struct bit_matrix* bm) {

    unsigned int maxYOffset = height - BLOCK_SIZE;
    unsigned int maxXOffset = width - BLOCK_SIZE;
    for (unsigned int y = 0 ; y < subHeight ; y++) {
        unsigned int yoffset = y * BLOCK_SIZE;
        if (yoffset > maxYOffset) {
            yoffset = maxYOffset;
        }
        unsigned int top = cap(y, subHeight - 3);
        for (unsigned int x = 0 ; x < subWidth ; x++) {
            unsigned int xoffset = x * BLOCK_SIZE;
            if (xoffset > maxXOffset) {
                xoffset = maxXOffset;
            }
            unsigned int left = cap(x, subWidth - 3);
            unsigned int sum = 0;
            for (int z = -2 ; z <= 2 ; z++) {
                u_int8_t* black_row = &(black_points[(top + z) * subWidth]);
                sum += black_row[left - 2] + black_row[left - 1] + black_row[left] + black_row[left + 1] + black_row[left + 2];
            }
            unsigned int average = sum / 25;
            threshold_block(luminances, xoffset, yoffset, average, bm);
        }
    }
}


/**
 * This function creates a bit matrix from the given image
 * using the same implementation as in the HybridBinarizer in
 * the zxing project.
 *
 * https://github.com/zxing/zxing/blob/master/core/src/main/java/com/google/zxing/common/HybridBinarizer.java
 */
struct bit_matrix* binarize(struct rgb_image* img) {
    u_int8_t* luminances = calculate_luminances(img);
    if (luminances == NULL) {
        return NULL;
    }

    unsigned int subWidth = (img->width / BLOCK_SIZE);
    if ((img->width % BLOCK_SIZE) != 0) {
        subWidth++;
    }
    unsigned int subHeight = (img->height / BLOCK_SIZE);
    if ((img->height % BLOCK_SIZE) != 0) {
        subHeight++;
    }

    u_int8_t* black_points = calculate_black_points(luminances, subWidth, subHeight, img->width, img->height);
    if (black_points == NULL) {
        free(luminances);
        return NULL;
    }

    struct bit_matrix* bm = create_bit_matrix(img->width, img->height);
    calculate_threshold_for_blocks(luminances, subWidth, subHeight, img->width, img->height, black_points, bm);

    free(luminances);
    free(black_points);
    return bm;
}

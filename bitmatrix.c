#include <stdio.h>
#include <stdlib.h>
#include "bitmatrix.h"


struct bit_matrix* create_bit_matrix(unsigned int width, unsigned int height) {
    struct bit_matrix* bm = (struct bit_matrix*)malloc(sizeof(struct bit_matrix));
    if (bm == NULL) {
        return NULL;
    }
    bm->width = width;
    bm->height = height;
    int n_bytes = ((width * height) / 8) + ((width * height) % 8 != 0);
    bm->matrix = (u_int8_t*)calloc(n_bytes, sizeof(u_int8_t));
    if (bm->matrix == NULL) {
        free(bm);
        return NULL;
    }
    return bm;
}


void free_bit_matrix(struct bit_matrix* bm) {
    free(bm->matrix);
    free(bm);
}


u_int8_t is_black(struct bit_matrix* bm, unsigned int x, unsigned int y) {
    if (x >= bm->width || y >= bm->height) {
        fprintf(stderr, "Invalid access in is_black %d,%d while dimensions = %dx%d\n", x, y, bm->width, bm->height);
        exit(1);
    }
    int pos = y * bm->width + x;
    return (bm->matrix[pos / 8] & (1 << (pos % 8))) != 0;
}


void set_color(struct bit_matrix* bm, u_int8_t value, unsigned int x, unsigned int y) {
    if (x >= bm->width || y >= bm->height) {
        fprintf(stderr, "Invalid access in set_color %d,%d  while dimensions = %dx%d\n", x, y, bm->width, bm->height);
        exit(1);
    }
    int pos = y * bm->width + x;
    if (value) {
        bm->matrix[pos / 8] |= (1 << (pos % 8));
    } else {
        bm->matrix[pos / 8] &= (0xFF - (1 << (pos % 8)));
    }
}


int create_from_string(const char* data[], struct bit_matrix* *bm) {
    unsigned width = 0;
    unsigned int height = 0;
    while (data[height] != NULL) {
        const char* line = data[height];
        unsigned int len = 0;
        while (line[len] != '\0') {
            if (line[len] != '*' && line[len] != ' ') {
                return DECODING_ERROR;
            }
            len++;
        }
        if (len == 0) {
            return DECODING_ERROR;
        }
        if (width == 0) {
            width = len;
        } else if (width != len) {
            return DECODING_ERROR;
        }

        height++;
    }

    *bm = create_bit_matrix(width, height);
    if ((*bm) == NULL) {
        return MEMORY_ERROR;
    }

    for (unsigned int y = 0 ; y < height ; y++) {
        for (unsigned int x = 0 ; x < width ; x++) {
            if (data[y][x] == '*') {
                set_color(*bm, BLACK, x, y);
            }
        }
    }

    return SUCCESS;
}


void print_matrix(LogLevel level, struct bit_matrix* matrix) {
    print_log(level, "%d x %d:\n", matrix->width, matrix->height);
    for (unsigned int y = 0 ; y < matrix->height ; y++) {
        for (unsigned int x = 0 ; x < matrix->width ; x++) {
            print_log(level, "%c", is_black(matrix, x, y) ? '*' : ' ');
        }
        print_log(level, "\n");
    }
}

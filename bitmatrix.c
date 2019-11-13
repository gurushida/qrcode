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


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
    bm->matrix = (u_int8_t*)calloc(width * height, sizeof(u_int8_t));
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
        fprintf(stderr, "Invalid access in get_bit\n");
        exit(1);
    }
    return bm->matrix[y * bm->width + x] & 1;
}


void set_color(struct bit_matrix* bm, u_int8_t value, unsigned int x, unsigned int y) {
    if (x >= bm->width || y >= bm->height) {
        fprintf(stderr, "Invalid access in set_bit\n");
        exit(1);
    }
    bm->matrix[y * bm->width + x] = value & 1;
}


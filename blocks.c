#include <stdlib.h>
#include <string.h>
#include "blocks.h"


/**
 * For each (version,error correction level) combination, this
 * array gives the block layout as a 0 terminated array made of
 * sequences x,y,z,t where x is a number of blocks, y is the total
 * number of codewords for each of those block, z is the number
 * of data codewords among them (so that y - z gives the number of
 * error codewords).
 *
 * t is the error correction capacity. Most of the time, it will
 * be the same as z/2. It will diverge when the block has a non zero
 * p value.
 */
static unsigned int block_descriptions[40][4][9]= {
    {           /* Version 1 */
        /* L */ { 1, 26, 19, 2,   0 },
        /* M */ { 1, 26, 16, 4,   0 },
        /* Q */ { 1, 26, 13, 6,   0 },
        /* H */ { 1, 26,  9, 8,   0 },
    },
    {           /* Version 2 */
        /* L */ { 1, 44, 34,  4,   0 },
        /* M */ { 1, 44, 28,  8,   0 },
        /* Q */ { 1, 44, 22, 11,   0 },
        /* H */ { 1, 44, 16, 14,   0 },
    },
    {           /* Version 3 */
        /* L */ { 1, 70, 55,  7,   0 },
        /* M */ { 1, 70, 44, 13,   0 },
        /* Q */ { 2, 35, 17,  9,   0 },
        /* H */ { 2, 35, 13, 11,   0 },
    },
    {           /* Version 4 */
        /* L */ { 1, 100, 80, 10,   0 },
        /* M */ { 2,  50, 32,  9,   0 },
        /* Q */ { 2,  50, 24, 13,   0 },
        /* H */ { 4,  25,  9,  8,   0 },
    },
    {           /* Version 5 */
        /* L */ { 1, 134, 108, 13,   0 },
        /* M */ { 2,  67,  43, 12,   0 },
        /* Q */ { 2,  33,  15,  9,   2, 34, 16,  9,   0 },
        /* H */ { 2,  33,  11, 11,   2, 34, 12, 11,   0 },
    },
    {           /* Version 6 */
        /* L */ { 2, 86, 68,  9,   0 },
        /* M */ { 4, 43, 27,  8,   0 },
        /* Q */ { 4, 43, 19, 12,   0 },
        /* H */ { 4, 43, 15, 14,   0 },
    },
    {           /* Version 7 */
        /* L */ { 2, 98, 78, 10,   0 },
        /* M */ { 4, 49, 31,  9,   0 },
        /* Q */ { 2, 32, 14,  9,   4, 33, 15,  9,   0 },
        /* H */ { 4, 39, 13, 13,   1, 40, 14, 13,   0 },
    },
    {           /* Version 8 */
        /* L */ { 2, 121, 97, 12,   0 },
        /* M */ { 2,  60, 38, 11,   2, 61, 39, 11,   0 },
        /* Q */ { 4,  40, 18, 11,   2, 41, 19, 11,   0 },
        /* H */ { 4,  40, 14, 13,   2, 41, 15, 13,   0 },
    },
    {           /* Version 9 */
        /* L */ { 2, 146, 116, 15,   0 },
        /* M */ { 3,  58,  36, 11,   2, 59, 37, 11,   0 },
        /* Q */ { 4,  36,  16, 10,   4, 37, 17, 10,   0 },
        /* H */ { 4,  36,  12, 12,   4, 37, 13, 12,   0 },
    },
    {           /* Version 10 */
        /* L */ { 2, 86, 68,  9,   2, 87, 69,  9,   0 },
        /* M */ { 4, 69, 43, 13,   1, 70, 44, 13,   0 },
        /* Q */ { 6, 43, 19, 12,   2, 44, 20, 12,   0 },
        /* H */ { 6, 43, 15, 14,   2, 44, 16, 14,   0 },
    },
    {           /* Version 11 */
        /* L */ { 4, 101, 81, 10,   0 },
        /* M */ { 1,  80, 50, 15,   4, 81, 51, 15,   0 },
        /* Q */ { 4,  50, 22, 14,   4, 51, 23, 14,   0 },
        /* H */ { 3,  36, 12, 12,   8, 37, 13, 12,   0 },
    },
    {           /* Version 12 */
        /* L */ { 2, 116, 92, 12,   2, 117, 93, 12,   0 },
        /* M */ { 6,  58, 36, 11,   2,  59, 37, 11,   0 },
        /* Q */ { 4,  46, 20, 13,   6,  47, 21, 13,   0 },
        /* H */ { 7,  42, 14, 14,   4,  43, 15, 14,   0 },
    },
    {           /* Version 13 */
        /* L */ {  4, 133, 107, 13,   0 },
        /* M */ {  8,  59,  37, 11,   1, 60, 38, 11,   0 },
        /* Q */ {  8,  44,  20, 12,   4, 45, 21, 12,   0 },
        /* H */ { 12,  33,  11, 11,   4, 34, 12, 11,   0 },
    },
    {           /* Version 14 */
        /* L */ {  3, 145, 115, 15,   1, 146, 116, 15,   0 },
        /* M */ {  4,  64,  40, 12,   5,  65,  41, 12,   0 },
        /* Q */ { 11,  36,  16, 10,   5,  37,  17, 10,   0 },
        /* H */ { 11,  36,  12, 12,   5,  37,  13, 12,   0 },
    },
    {           /* Version 15 */
        /* L */ {  5, 109, 87, 11,   1, 110, 88, 11,   0 },
        /* M */ {  5,  65, 41, 12,   5,  66, 42, 12,   0 },
        /* Q */ {  5,  54, 24, 15,   7,  55, 25, 15,   0 },
        /* H */ { 11,  36, 12, 12,   7,  37, 13, 12,   0 },
    },
    {           /* Version 16 */
        /* L */ {  5, 122, 98, 12,    1, 123, 99, 12,   0 },
        /* M */ {  7,  73, 45, 14,    3,  74, 46, 14,   0 },
        /* Q */ { 15,  43, 19, 12,    2,  44, 20, 12,   0 },
        /* H */ {  3,  45, 15, 15,   13,  46, 16, 15,   0 },
    },
    {           /* Version 17 */
        /* L */ {  1, 135, 107, 14,    5, 136, 108, 14,   0 },
        /* M */ { 10,  74,  46, 14,    1,  75,  47, 14,   0 },
        /* Q */ {  1,  50,  22, 14,   15,  51,  23, 14,   0 },
        /* H */ {  2,  42,  14, 14,   17,  43,  15, 14,   0 },
    },
    {           /* Version 18 */
        /* L */ {  5, 150, 120, 15,    1, 151, 121, 15,   0 },
        /* M */ {  9,  69,  43, 13,    4,  70,  44, 13,   0 },
        /* Q */ { 17,  50,  22, 14,    1,  51,  23, 14,   0 },
        /* H */ {  2,  42,  14, 14,   19,  43,  15, 14,   0 },
    },
    {           /* Version 19 */
        /* L */ {  3, 141, 113, 14,    4, 142, 114, 14,   0 },
        /* M */ {  3,  70,  44, 13,   11,  71,  45, 13,   0 },
        /* Q */ { 17,  47,  21, 13,    4,  48,  22, 13,   0 },
        /* H */ {  9,  39,  13, 13,   16,  40,  14, 13,   0 },
    },
    {           /* Version 20 */
        /* L */ {  3, 135, 107, 14,    5, 136, 108, 14,   0 },
        /* M */ {  3,  67,  41, 13,   13,  68,  42, 13,   0 },
        /* Q */ { 15,  54,  24, 15,    5,  55,  25, 15,   0 },
        /* H */ { 15,  43,  15, 14,   10,  44,  16, 14,   0 },
    },
    {           /* Version 21 */
        /* L */ {  4, 144, 116, 14,   4, 145, 117, 14,   0 },
        /* M */ { 17,  68,  42, 13,   0 },
        /* Q */ { 17,  50,  22, 14,   6,  51,  23, 14,   0 },
        /* H */ { 19,  46,  16, 15,   6,  47,  17, 15,   0 },
    },
    {           /* Version 22 */
        /* L */ {  2, 139, 111, 14,    7, 140, 112, 14,   0 },
        /* M */ { 17,  74,  46, 14,    0 },
        /* Q */ {  7,  54,  24, 15,   16,  55,  25, 15,   0 },
        /* H */ { 34,  37,  13, 12,    0 },
    },
    {           /* Version 23 */
        /* L */ {  4, 151, 121, 15,    5, 152, 122, 15,   0 },
        /* M */ {  4,  75,  47, 14,   14,  76,  48, 14,   0 },
        /* Q */ { 11,  54,  24, 15,   14,  55,  25, 15,   0 },
        /* H */ { 16,  45,  15, 15,   14,  46,  16, 15,   0 },
    },
    {           /* Version 24 */
        /* L */ {  6, 147, 117, 15,    4, 148, 118, 15,   0 },
        /* M */ {  6,  73,  45, 14,   14,  74,  46, 14,   0 },
        /* Q */ { 11,  54,  24, 15,   16,  55,  25, 15,   0 },
        /* H */ { 30,  46,  16, 15,    2,  47,  17, 15,   0 },
    },
    {           /* Version 25 */
        /* L */ {  8, 132, 106, 13,    4, 133, 107, 13,   0 },
        /* M */ {  8,  75,  47, 14,   13,  76,  48, 14,   0 },
        /* Q */ {  7,  54,  24, 15,   22,  55,  25, 15,   0 },
        /* H */ { 22,  45,  15, 15,   13,  46,  16, 15,   0 },
    },
    {           /* Version 26 */
        /* L */ { 10, 142, 114, 14,   2, 143, 115, 14,   0 },
        /* M */ { 19,  74,  46, 14,   4,  75,  47, 14,   0 },
        /* Q */ { 28,  50,  22, 14,   6,  51,  23, 14,   0 },
        /* H */ { 33,  46,  16, 15,   4,  47,  17, 15,   0 },
    },
    {           /* Version 27 */
        /* L */ {  8, 152, 122, 15,    4, 153, 123, 15,   0 },
        /* M */ { 22,  73,  45, 14,    3,  74,  46, 14,   0 },
        /* Q */ {  8,  53,  23, 15,   26,  54,  24, 15,   0 },
        /* H */ { 12,  45,  15, 15,   28,  46,  16, 15,   0 },
    },
    {           /* Version 28 */
        /* L */ {  3, 147, 117, 15,   10, 148, 118, 15,   0 },
        /* M */ {  3,  73,  45, 14,   23,  74,  46, 14,   0 },
        /* Q */ {  4,  54,  24, 15,   31,  55,  25, 15,   0 },
        /* H */ { 11,  45,  15, 15,   31,  46,  16, 15,   0 },
    },
    {           /* Version 29 */
        /* L */ {  7, 146, 116, 15,    7, 147, 117, 15,   0 },
        /* M */ { 21,  73,  45, 14,    7,  74,  46, 14,   0 },
        /* Q */ {  1,  53,  23, 15,   37,  54,  24, 15,   0 },
        /* H */ { 19,  45,  15, 15,   26,  46,  16, 15,   0 },
    },
    {           /* Version 30 */
        /* L */ {  5, 145, 115, 15,   10, 146, 116, 15,   0 },
        /* M */ { 19,  75,  47, 14,   10,  76,  48, 14,   0 },
        /* Q */ { 15,  54,  24, 15,   25,  55,  25, 15,   0 },
        /* H */ { 23,  45,  15, 15,   25,  46,  16, 15,   0 },
    },
    {           /* Version 31 */
        /* L */ { 13, 145, 115, 15,    3, 146, 116, 15,   0 },
        /* M */ {  2,  74,  46, 14,   29,  75,  47, 14,   0 },
        /* Q */ { 42,  54,  24, 15,    1,  55,  25, 15,   0 },
        /* H */ { 23,  45,  15, 15,   28,  46,  16, 15,   0 },
    },
    {           /* Version 32 */
        /* L */ { 17, 145, 115, 15,    0 },
        /* M */ { 10,  74,  46, 14,   23,  75,  47, 14,   0 },
        /* Q */ { 10,  54,  24, 15,   35,  55,  25, 15,   0 },
        /* H */ { 19,  45,  15, 15,   35,  46,  16, 15,   0 },
    },
    {           /* Version 33 */
        /* L */ { 17, 145, 115, 15,    1, 146, 116, 15,   0 },
        /* M */ { 14,  74,  46, 14,   21,  75,  47, 14,   0 },
        /* Q */ { 29,  54,  24, 15,   19,  55,  25, 15,   0 },
        /* H */ { 11,  45,  15, 15,   46,  46,  16, 15,   0 },
    },
    {           /* Version 34 */
        /* L */ { 13, 145, 115, 15,    6, 146, 116, 15,   0 },
        /* M */ { 14,  74,  46, 14,   23,  75,  47, 14,   0 },
        /* Q */ { 44,  54,  24, 15,    7,  55,  25, 15,   0 },
        /* H */ { 59,  46,  16, 15,    1,  47,  17, 15,   0 },
    },
    {           /* Version 35 */
        /* L */ { 12, 151, 121, 15,    7, 152, 122, 15,   0 },
        /* M */ { 12,  75,  47, 14,   26,  76,  48, 14,   0 },
        /* Q */ { 39,  54,  24, 15,   14,  55,  25, 15,   0 },
        /* H */ { 22,  45,  15, 15,   41,  46,  16, 15,   0 },
    },
    {           /* Version 36 */
        /* L */ {  6, 151, 121, 15,   14, 152, 122, 15,   0 },
        /* M */ {  6,  75,  47, 14,   34,  76,  48, 14,   0 },
        /* Q */ { 46,  54,  24, 15,   10,  55,  25, 15,   0 },
        /* H */ {  2,  45,  15, 15,   64,  46,  16, 15,   0 },
    },
    {           /* Version 37 */
        /* L */ { 17, 152, 122, 15,    4, 153, 123, 15,   0 },
        /* M */ { 29,  74,  46, 14,   14,  75,  47, 14,   0 },
        /* Q */ { 49,  54,  24, 15,   10,  55,  25, 15,   0 },
        /* H */ { 24,  45,  15, 15,   46,  46,  16, 15,   0 },
    },
    {           /* Version 38 */
        /* L */ {  4, 152, 122, 15,   18, 153, 123, 15,   0 },
        /* M */ { 13,  74,  46, 14,   32,  75,  47, 14,   0 },
        /* Q */ { 48,  54,  24, 15,   14,  55,  25, 15,   0 },
        /* H */ { 42,  45,  15, 15,   32,  46,  16, 15,   0 },
    },
    {           /* Version 39 */
        /* L */ { 20, 147, 117, 15,    4, 148, 118, 15,   0 },
        /* M */ { 40,  75,  47, 14,    7,  76,  48, 14,   0 },
        /* Q */ { 43,  54,  24, 15,   22,  55,  25, 15,   0 },
        /* H */ { 10,  45,  15, 15,   67,  46,  16, 15,   0 },
    },
    {           /* Version 40 */
        /* L */ { 19, 148, 118, 15,    6, 149, 119, 15,   0 },
        /* M */ { 18,  75,  47, 14,   31,  76,  48, 14,   0 },
        /* Q */ { 34,  54,  24, 15,   34,  55,  25, 15,   0 },
        /* H */ { 20,  45,  15, 15,   61,  46,  16, 15,   0 },
    },
};


static unsigned int get_p(int version, ErrorCorrectionLevel ec_level) {
    if (version == 1) {
        if (ec_level == LOW) return 3;
        if (ec_level == MEDIUM) return 2;
        return 1;
    }
    if (version == 2 && ec_level == LOW) {
        return 2;
    }

    if (version == 3 && ec_level == LOW) {
        return 1;
    }

    return 0;
}

struct blocks* get_blocks(u_int8_t* codewords, int version, ErrorCorrectionLevel ec_level) {
    if (version < 1 || version > 40 || ec_level < 0 || ec_level > 3) {
        return NULL;
    }

    struct blocks* blocks = (struct blocks*)malloc(sizeof(struct blocks));
    if (blocks == NULL) {
        return NULL;
    }

    blocks->n_blocks = 0;
    unsigned int* description = block_descriptions[version - 1][ec_level];

    unsigned total_data_codewords = 0;
    unsigned total_error_codewords = 0;

    int i = 0;
    while (description[i] != 0) {
        unsigned int n_blocks = description[i];
        unsigned int n_data_codewords = description[i + 2];
        unsigned int n_error_codewords = description[i + 1] - n_data_codewords;
        blocks->n_blocks += n_blocks;
        total_data_codewords += n_blocks * n_data_codewords;
        total_error_codewords += n_blocks * n_error_codewords;
        i += 4;
    }

    // This array will be used when populating the codeword arrays
    // to remember how full each array is at any time
    unsigned int* counters = (unsigned int*)calloc(blocks->n_blocks, sizeof(unsigned int));
    if (counters == NULL) {
        free(blocks);
        return NULL;
    }

    // Allocating with calloc guarantees that all the pointers in the blocks
    // will be NULL, which will make it easier to cleanup if any allocation fails
    // as invoking free() on a NULL pointer is a no-op
    blocks->block = (struct block*)calloc(blocks->n_blocks, sizeof(struct block));
    if (blocks->block == NULL) {
        free(counters);
        free(blocks);
        return NULL;
    }

    // Let's add p to each block
    unsigned int p = get_p(version, ec_level);
    for (unsigned int i = 0 ; i < blocks->n_blocks ; i++) {
        blocks->block[i].p = p;
    }

    // Let's allocate all the codeword arrays
    i = 0;
    int current_block = 0;
    int fail = 0;
    while (!fail && description[i] != 0) {
        unsigned int n = description[i];
        for (unsigned int j = 0 ; j < n ; j++) {
            unsigned int n_data = description[i + 2];
            unsigned int n_error = description[i + 1] - n_data;
            blocks->block[current_block].error_correction_capacity = description[i + 3];
            blocks->block[current_block].n_data_codewords = n_data;
            blocks->block[current_block].n_error_correction_codewords = n_error;
            blocks->block[current_block].codewords = (u_int8_t*)malloc((n_data + n_error) * sizeof(u_int8_t));
            if (blocks->block[current_block].codewords == NULL) {
                fail = 1;
                break;
            }
            current_block++;
        }
        i += 4;
    }

    // If any memory allocation failed, let's free everything and return
    if (fail) {
        for (unsigned int i = 0 ; i < blocks->n_blocks ; i++) {
            free(blocks->block[i].codewords);
        }
        free(blocks->block);
        free(blocks);
        free(counters);
        return NULL;
    }

    // We now have allocated all the space to store the codewords.
    // It is time to populate the codeword arrays

    unsigned int pos = 0;
    int current_block_index = 0;
    while (pos < total_data_codewords) {
        // Let's read one data codeword from the array
        u_int8_t codeword = codewords[pos++];

        // Now let's find where to put it by looking for the next data block that is not already full
        while (counters[current_block_index] == blocks->block[current_block_index].n_data_codewords) {
            current_block_index = (current_block_index + 1) % blocks->n_blocks;
        }

        // Let's add the data codeword
        blocks->block[current_block_index].codewords[counters[current_block_index]] = codeword;
        counters[current_block_index]++;
        current_block_index = (current_block_index + 1) % blocks->n_blocks;
    }

    // Let's do the same for the error codewords, after resetting the counter array
    memset(counters, 0, blocks->n_blocks * sizeof(unsigned int));

    current_block_index = 0;
    unsigned total_codewords = total_data_codewords + total_error_codewords;
    while (pos < total_codewords) {
        // Let's read one error codeword from the array
        u_int8_t codeword = codewords[pos++];

        // Now let's find where to put it by looking for the next data block that is not already full
        while (counters[current_block_index] == blocks->block[current_block_index].n_error_correction_codewords) {
            current_block_index = (current_block_index + 1) % blocks->n_blocks;
        }

        // Let's add the error correction codeword
        blocks->block[current_block_index].codewords[blocks->block[current_block_index].n_data_codewords + counters[current_block_index]] = codeword;
        counters[current_block_index]++;
        current_block_index = (current_block_index + 1) % blocks->n_blocks;
    }

    return blocks;
}


void free_blocks(struct blocks* blocks) {
    for (unsigned int i = 0 ; i < blocks->n_blocks ; i++) {
        free(blocks->block[i].codewords);
    }
    free(blocks->block);
    free(blocks);
}


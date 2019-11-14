#ifndef _BLOCKS_H
#define _BLOCKS_H

#include <stdint.h>

#include "errors.h"
#include "formatinformation.h"

#define MAX_ERROR_CORRECTION_CAPACITY 15

/**
 * Information is encoded into a QR code by splitting the data codewords
 * into blocks and to add for each block some error correction codewords.
 * Data and error correction codewords are interleaved in the QR so this
 * structure is used to represent a de-interleaved block.
 */
struct block {
    // The array containing all the codewords of this block.
    // The first n_data_codewords are the data codewords
    // and the remaining n_error_correction_codewords ones
    // and the ones for error detection/correction
    u_int8_t* codewords;

    unsigned int n_data_codewords;
    unsigned int n_error_correction_codewords;
};


/**
 * This structure represents an array of blocks representing
 * all the data contained in a QR code.
 */
struct blocks {
    struct block* block;
    unsigned int n_blocks;
};


/**
 * For each (version, error correction level) combination, the data
 * is divided into N data blocks for which there are N corresponding
 * error correction blocks that are as organized as follows:
 *
 * +--------------------------------+--------------------------------------------+
 * | codewords of the N data blocks | codewords of the N error correction blocks |
 * +--------------------------------+--------------------------------------------+
 *
 * The sizes of the blocks may vary. For instance, for a version 5 QR code with
 * high error correction level, we have N = 4, the data blocks are made of
 * 2 data blocks of 11 codewords followed by 2 blocks of 12 codewords, and
 * the error correction blocks consist of 4 blocks of 22 codewords. This gives
 * a total of 134 codewords which the total number of codewords in a version 5 QR
 * code.
 *
 * Inside the data and error correction zones, the codewords are interleaved by taking
 * one codeword from each block in turn until there are no more blocks.
 *
 * In our example of a version 5 QR code with high error correction level, our
 * 4 data blocks D1, D2, D3 and D4 will look like this:
 *
 * D1[1] D1[2] .................... D1[11]
 * D2[1] D2[2] .................... D2[11]
 * D3[1] D3[2] .................... D3[11] D3[12]
 * D4[1] D4[2] .................... D4[11] D4[12]
 *
 * The interleaving process will then produce the following sequence:
 *
 * D1[1] D2[1] D3[1] D4[1] D1[2] D2[2] D3[2] D4[2] ... D1[11] D2[11] D3[11] D4[11] D3[12] D4[12]
 *
 * The same will happen to the error blocks E1, E2, E3 and E4, and therefore the final result will be:
 *
 * +---------------------------------------------------------------------------------------------------+
 * | D1[1] D2[1] D3[1] D4[1] D1[2] ... D3[12] D4[12] E1[1] E2[1] E3[1] ... E1[22] E2[22] E3[22] E4[22] |
 * +---------------------------------------------------------------------------------------------------+
 *
 *
 * Given an array containing all data and error correction codewords all
 * interleaved, this function de-interleaves the data into
 * data+error correction blocks.
 *
 * @param codewords The array of codewords obtained by scanning the QR code
 * @param version The QR code version between 1 and 40
 * @param ec_level The error correction level that, in combination with the version,
 *                 determines how the blocks are organized
 * @param blocks Where to store the result
 * @return SUCCESS on success
 *         DECODING_ERROR if any of the parameter has an invalid value
 *         MEMORY_ERROR in case of memory allocation error
 */
int get_blocks(u_int8_t* codewords, int version, ErrorCorrectionLevel ec_level, struct blocks* *blocks);


/**
 * Frees all the memory associated to the given blocks.
 */
void free_blocks(struct blocks* blocks);

#endif

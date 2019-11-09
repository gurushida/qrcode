#ifndef _BITSTREAM_H
#define _BITSTREAM_H

#include <stdint.h>

struct bitstream {
    // The source data
    uint8_t* bytes;
    unsigned int n_bytes;

    // Index of the byte where the read cursor is 
    unsigned int current_byte_pos;
    // Number of bits already read from the current byte
    unsigned int n_read_bits;
};


/**
 * Allocates a bitstream with a byte array of the given size,
 * initialized with zeroes. Returns the bitstream or NULL in case
 * of memory error.
 */
struct bitstream* new_bitstream(unsigned int n_bytes);


/**
 * Frees the memory associated with the given bitstream.
 */
void free_bitstream(struct bitstream* stream);


/**
 * Returns the number of bits that can still be read from the stream.
 */
unsigned int remaining_bits(struct bitstream* stream);


/**
 * Reads n bits from the stream and moves the read cursor in the stream.
 *
 * @param stream The stream to read from
 * @param n The number of bits to read. Must be between 1 and 32 and no
 *          more than the nubmer of available bits in the stream.
 *          The caller is responsible for checking the available bits
 *          before calling this function.
 * @return The value corresponding to the n bits that were read. Returns 0 and
 *         don't move the read cursor if n is not a
 */
u_int32_t read_bits(struct bitstream* stream, unsigned int n);

#endif

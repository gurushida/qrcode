#ifndef _BYTEBUFFER_H
#define _BYTEBUFFER_H

#include <stdint.h>

/**
 * This structure represents a growing byte buffer.
 */
struct bytebuffer {
    u_int8_t* bytes;
    unsigned int capacity;
    unsigned int n_bytes;
};


/**
 * Allocates a buffer of 32 bytes.
 * Returns NULL in case of memory allocation error.
 */
struct bytebuffer* new_bytebuffer();


/**
 * Frees the memory associated with this byte buffer.
 */
void free_bytebuffer(struct bytebuffer* buffer);


/**
 * Appends the given value to the given buffer,
 * enlarging it if necessary.
 * Returns 1 on success or 0 in case of memory allocation
 * error.
 */
int write_byte(struct bytebuffer* buffer, uint8_t value);


/**
 * Writes to the given buffer the bytes corresponding to the utf8
 * encoding of the given unicode character.
 *
 * @return 1 on success
 *         0 in case of memory allocation error
 *        -1 if the given value is not in a the range [0 - 0x10FFFF]
 */
int write_unicode_as_utf8(struct bytebuffer* buffer, u_int32_t value);

#endif

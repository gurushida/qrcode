#ifndef _BITSTREAMDECODER_H
#define _BITSTREAMDECODER_H

#include <stdint.h>
#include "bitstream.h"

/**
 * The data in a QR code is encoded in multiple segments that
 * can have their own modes, where a mode is a way to encode data.
 *
 * Given a bitstream containing the encoded codewords, this function
 * decodes it to expose the data that was encoded in the QR code
 * as a null-terminated utf8 string.
 *
 * @param stream The bitstream to decode
 * @param version The QR code version (between 1 and 40)
 * @param decoded Where to store the decoded data. This will be dynamically alloced and filled
 *                with the data on success
 * @return n = strlen(*decoded) >= 0 on success
 *        -1 on memory allocation error
 *        -2 on decoding error or if the given version is invalid
 */
int decode_bitstream(struct bitstream* stream, unsigned int version, u_int8_t* *decoded);

#endif

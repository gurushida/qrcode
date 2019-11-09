#ifndef _BITSTREAMDECODER_H
#define _BITSTREAMDECODER_H

#include <stdint.h>

/**
 * Given a byte array contained the encoded codewords, this function
 * decodes these bytes to expose the data that was encoded in the QR
 * code.
 *
 * @param bytes The bytes to decode
 * @param n_bytes The size of the byte array
 * @param version The QR code version (between 1 and 40)
 * @param decoded Where to store the decoded data. This will be dynamically alloced and filled
 *                with the data on success
 * @return n > 0 on success where n will be the size of the allocated 'decoded' array
 *         0 on decoding error or if the given version is invalid
 *        -1 on memory allocation error
 */
int decode_bitstream(u_int8_t* bytes, unsigned int n_bytes, unsigned int version, u_int8_t* *decoded);

#endif

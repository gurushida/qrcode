#ifndef _BITSTREAMDECODER_H
#define _BITSTREAMDECODER_H

#include <stdint.h>
#include "bitstream.h"
#include "bytebuffer.h"
#include "eci.h"
#include "errors.h"

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
 * @return n >= 0 on success, where n is the number of decoded bytes placed in the
 *           decoded array. If none of the decoded byte is 0, then n is the same as
 *           strlen(*decoded)
 *         MEMORY_ERROR on memory allocation error
 *         DECODING_ERROR on decoding error or if the given version is invalid
 */
int decode_bitstream(struct bitstream* stream, unsigned int version, u_int8_t* *decoded);


/**
 * Decodes count bytes from the given stream and adds
 * the corresponding data as utf8 in the given buffer.
 *
 * @return SUCCESS on success
 *         DECODING_ERROR on decoding failure
 *         MEMORY_ERROR on memory allocation error
 */
int decode_byte_segment(struct bitstream* stream, unsigned int count, EciMode eci_mode, struct bytebuffer* buffer);


/**
 * In FNC1 mode, '%' characters in alphanumeric character sequences must be
 * transformed as follows:
 * - the sequence '%%' must be converted to a single '%' character
 * - single '%' characters must be converted to the 0x1D value used as
 *   a FNC1 separator
 *
 * This function does such decoding on the given buffer, from the given start position
 * to the end of the buffer. As a consequence, the n_bytes field may become smaller.
 *
 * @param buffer The buffer to decode
 * @param start The position where to start decoding
 */
void decode_percents_in_FNC1_mode(struct bytebuffer* buffer, unsigned int start);

#endif

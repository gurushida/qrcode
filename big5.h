#ifndef _BIG5_H
#define _BIG5_H

#include <stdint.h>
#include "bitstream.h"
#include "bytebuffer.h"


/**
 * Decodes count bytes from the given stream interpreted as
 * a Big5-encoded data.
 *
 * This is a direct implementation of
 * https://encoding.spec.whatwg.org/#big5-decoder
 * 
 * Returns 1 on success
 *         0 in case of decoding failure
 *        -1 in case of memory allocation error
 */
int decode_big5_segment(struct bitstream* stream, unsigned int count, struct bytebuffer* buffer);

void print_big5();

#endif

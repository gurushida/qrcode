#ifndef _GB18030_H
#define _GB18030_H

#include <stdint.h>
#include "bitstream.h"
#include "bytebuffer.h"


/**
 * Decodes count characters from the given stream interpreted as
 * a GB18030-encoded data.
 *
 * This is a direct implementation of
 * https://encoding.spec.whatwg.org/#ref-for-gb18030-first
 * 
 * Returns 1 on success
 *         0 in case of decoding failure
 *        -1 in case of memory allocation error
 */
int decode_gb18030_segment(struct bitstream* stream, unsigned int count, struct bytebuffer* buffer);

#endif

#ifndef _GB18030_H
#define _GB18030_H

#include <stdint.h>
#include "bitstream.h"
#include "bytebuffer.h"
#include "errors.h"


/**
 * Decodes count bytes from the given stream interpreted as
 * a GB18030-encoded data.
 *
 * This is a direct implementation of
 * https://encoding.spec.whatwg.org/#ref-for-gb18030-first
 * 
 * Returns SUCCESS on success
 *         DECODING_ERROR in case of decoding failure
 *         MEMORY_ERROR in case of memory allocation error
 */
int decode_gb18030_segment(struct bitstream* stream, unsigned int count, struct bytebuffer* buffer);

#endif

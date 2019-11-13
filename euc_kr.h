#ifndef _EUC_KR_H
#define _EUC_KR_H

#include <stdint.h>
#include "bitstream.h"
#include "bytebuffer.h"
#include "errors.h"


/**
 * Decodes count bytes from the given stream interpreted as
 * a EUC-KR-encoded data.
 *
 * This is a direct implementation of
 * https://encoding.spec.whatwg.org/#euc-kr-decoder
 * 
 * Returns SUCCESS on success
 *         DECODING_ERROR in case of decoding failure
 *         MEMORY_ERROR in case of memory allocation error
 */
int decode_euc_kr_segment(struct bitstream* stream, unsigned int count, struct bytebuffer* buffer);

#endif

#ifndef _EUC_KR_H
#define _EUC_KR_H

#include <stdint.h>
#include "bitstream.h"
#include "bytebuffer.h"


/**
 * Decodes count bytes from the given stream interpreted as
 * a EUC-KR-encoded data.
 *
 * This is a direct implementation of
 * https://encoding.spec.whatwg.org/#euc-kr-decoder
 * 
 * Returns 1 on success
 *         0 in case of decoding failure
 *        -1 in case of memory allocation error
 */
int decode_euc_kr_segment(struct bitstream* stream, unsigned int count, struct bytebuffer* buffer);

#endif

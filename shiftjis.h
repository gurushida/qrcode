#ifndef _SHIFTJIS_H
#define _SHIFTJIS_H


#include <stdint.h>

/**
 * Given a Shift JIS value, returns the corresponding unicode character
 * or 0 if the given value cannot be decoded.
 */
u_int32_t from_SJIS(u_int32_t value);

#endif

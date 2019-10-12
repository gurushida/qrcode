#ifndef _BINARIZE_H
#define _BINARIZE_H

#include "bitmatrix.h"
#include "rgbimage.h"

/**
 * Given an RGB image, returns the bit matrix obtained when
 * converting it to 2-bit black and white.
 */
struct bit_matrix* binarize(struct rgb_image* img);

#endif

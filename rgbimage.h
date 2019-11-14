#ifndef _RGBIMAGE_H
#define _RGBIMAGE_H

#include <stdint.h>
#include "errors.h"

struct rgb_image {
    unsigned int width;
    unsigned int height;
    u_int8_t* buffer;
};


/**
 * Loads the given png file.
 *
 * @param filename The path to the png file to load
 * @param image Where to store the result
 * @return SUCCESS on success
 *         DECODING_ERROR if the image cannot be loaded
 *         MEMORY_ERROR in case of memory allocation error
 */
int load_rgb_image(const char* filename, struct rgb_image* *image);


/**
 * Frees the memory associated to the given image.
 */
void free_rgb_image(struct rgb_image* img);

#endif

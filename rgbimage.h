#include <stdint.h>

struct rgb_image {
    unsigned int width;
    unsigned int height;
    u_int8_t* buffer;
};


/**
 * Loads the given png file and returns the corresponding
 * RGB image.
 * Returns NULL if the given image cannot be loaded.
 */
struct rgb_image* load_rgb_image(char* filename);


/**
 * Frees the memory associated to the given image.
 */
void free_rgb_image(struct rgb_image* img);

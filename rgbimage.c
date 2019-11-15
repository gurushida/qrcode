#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rgbimage.h"


int load_rgb_image(const char* filename, struct rgb_image* *rgb_image) {
    struct rgb_image* img = (struct rgb_image*)malloc(sizeof(struct rgb_image*));
    if (img == NULL) {
        return MEMORY_ERROR;
    }
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_file(&image, filename)) {
        free(img);
        return CANNOT_LOAD_IMAGE;
    }

    image.format = PNG_FORMAT_RGB;

    img->width = image.width;
    img->height = image.height;
    img->buffer = malloc(PNG_IMAGE_SIZE(image));
    if (img->buffer == NULL) {
        free(img);
        return MEMORY_ERROR;
    }
    png_color background;
    background.red = 255;
    background.green = 255;
    background.blue = 255;
    if (!png_image_finish_read(&image, &background, img->buffer, 0, NULL)) {
        free(img->buffer);
        free(img);
        return CANNOT_LOAD_IMAGE;
    }

    (*rgb_image) = img;
    return SUCCESS;
}


void free_rgb_image(struct rgb_image* img) {
    free(img->buffer);
    free(img);
}

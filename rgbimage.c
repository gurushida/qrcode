#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rgbimage.h"


struct rgb_image* load_rgb_image(char* filename) {
    struct rgb_image* img = (struct rgb_image*)malloc(sizeof(struct rgb_image*));
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_file(&image, filename)) {
        fprintf(stderr, "Cannot read image from '%s'\n", filename);
        return NULL;
    }

    image.format = PNG_FORMAT_RGB;

    img->width = image.width;
    img->height = image.height;
    img->buffer = malloc(PNG_IMAGE_SIZE(image));
    if (img->buffer == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        free(img);
        return NULL;
    }
    png_color background;
    background.red = 255;
    background.green = 255;
    background.blue = 255;
    if (!png_image_finish_read(&image, &background, img->buffer, 0, NULL)) {
        free(img->buffer);
        free(img);
        fprintf(stderr, "Cannot read image from '%s'\n", filename);
        return NULL;
    }

    return img;
}


void free_rgb_image(struct rgb_image* img) {
    free(img->buffer);
    free(img);
}

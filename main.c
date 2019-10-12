/**
 * Copyright (C) 2019 - gurushida
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "binarize.h"
#include "rgbimage.h"


int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("Usage: qrcode PNG\n");
        printf("\n");
        printf("Given a png image, tries to locate a QR code in it. On success, prints the decoded content.\n");
        printf("\n");
        return 1;
    }

    struct rgb_image* img = load_rgb_image(argv[1]);
    if (img == NULL) {
        return 1;
    }

    struct bit_matrix* bm = binarize(img);
    if (bm == NULL) {
        return 1;
    }

    return 0;
}

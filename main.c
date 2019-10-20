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
#include "finderpattern.h"
#include "finderpatterngroup.h"
#include "formatinformation.h"
#include "qrcodefinder.h"
#include "rgbimage.h"
#include "versioninformation.h"


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

    struct finder_pattern_list* list = find_potential_centers(bm, 1);
    struct finder_pattern_group_list* groups = find_groups(list);

    struct finder_pattern_group_list* tmp = groups;
    while (tmp != NULL) {
        printf("---------------------------------------------\n");
        printf("B: %.2f %.2f  C: %.2f %.2f\n", tmp->top_left.x, tmp->top_left.y, tmp->top_right.x, tmp->top_right.y);
        printf("A: %.2f %.2f\n", tmp->bottom_left.x, tmp->bottom_left.y);

        struct qr_code* code = get_qr_code(tmp->bottom_left, tmp->top_left, tmp->top_right, bm);
        if (code == NULL) {
            printf("Could not find code :(\n");
        } else {
            printf("Found code:\n");
            for (unsigned int y = 0 ; y < code->modules->height ; y++) {
                for (unsigned int x = 0 ; x < code->modules->width ; x++) {
                    printf("%c", is_black(code->modules, x, y) ? '*' : ' ');
                }
                printf("\n");
            }
            printf("B: %d,%d     C: %d,%d\n", code->top_left_x, code->top_left_y, code->top_right_x, code->top_right_y);
            printf("A: %d,%d     D: %d,%d\n", code->bottom_left_x, code->bottom_left_y, code->bottom_right_x, code->bottom_right_y);

            ErrorCorrectionLevel ec;
            uint8_t mask_pattern;
            if (1 == get_formation_information(code->modules, &ec, &mask_pattern)) {
                printf("Error correction level = %d, mask pattern = %d\n", ec, mask_pattern);

                uint8_t version;
                int ret = get_version_information(code->modules, &version);
                if (1 == ret) {
                    printf("version %d confirmed\n", version);
                } else if (ret == -1) {
                    printf("Could not decode version information\n");
                } else if (ret == -2) {
                    printf("Expcted version %d but got %d\n", (bm->width - 17) / 4, version);
                }
            }

            free_qr_code(code);
        }
        printf("\n");
        tmp = tmp->next;
    }

    free_finder_pattern_list(list);
    free_finder_pattern_group_list(groups);
    free_bit_matrix(bm);
    free_rgb_image(img);
    return 0;
}

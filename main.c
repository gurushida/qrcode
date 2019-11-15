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
#include "qrcode.h"


int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("Usage: qrcode PNG\n");
        printf("\n");
        printf("Given a png image, tries to locate QR codes in it. On success,\n");
        printf("prints on the standard output an html page that shows the matches\n");
        printf("in the image as red rectangles. Hovering a rectangle with the mouse\n");
        printf("will show the decoded message associated with the QR code.\n");
        printf("\n");
        return 1;
    }

    struct qr_code_match_list* matches;
    if (SUCCESS != find_qr_codes(argv[1], &matches)) {
        return 1;
    }
    printf("<html>\n");
    printf("<head></head>\n");
    printf("<body>\n");
    printf("<div style='position:absolute; top:0px; left:0px'>\n");
    printf("<img src='%s'>\n", argv[1]);
    printf("<div>\n");

    struct qr_code_match_list* tmp = matches;
    while (tmp != NULL) {
        // Let's find the rectangle that is the bounding box
        int min_x = tmp->bottom_left_x;
        if (min_x > tmp->top_left_x) min_x = tmp->top_left_x;
        if (min_x > tmp->top_right_x) min_x = tmp->top_right_x;
        if (min_x > tmp->bottom_right_x) min_x = tmp->bottom_right_x;

        int max_x = tmp->bottom_left_x;
        if (max_x < tmp->top_left_x) max_x = tmp->top_left_x;
        if (max_x < tmp->top_right_x) max_x = tmp->top_right_x;
        if (max_x < tmp->bottom_right_x) max_x = tmp->bottom_right_x;

        int min_y = tmp->bottom_left_y;
        if (min_y > tmp->top_left_y) min_y = tmp->top_left_y;
        if (min_y > tmp->top_right_y) min_y = tmp->top_right_y;
        if (min_y > tmp->bottom_right_y) min_y = tmp->bottom_right_y;

        int max_y = tmp->bottom_left_y;
        if (max_y < tmp->top_left_y) max_y = tmp->top_left_y;
        if (max_y < tmp->top_right_y) max_y = tmp->top_right_y;
        if (max_y < tmp->bottom_right_y) max_y = tmp->bottom_right_y;

        int width = (max_x - min_x);
        int height = (max_y - min_y);
        printf("<div style='position:absolute; top:%dpx; left:%dpx'>\n", min_y, min_x);
        printf("  <svg xmlns='http://www.w3.org/2000/svg' version='1.1' width='%dpx' height='%dpx'>\n", width, height);
        printf("    <rect width='%d' height='%d' style='stroke: red; stroke-width: 4; fill: none'/>\n", width, height);
        printf("    <title>");
        if (contains_binary_data(tmp->message)) {
            printf("Binary message:");
            for (unsigned int j = 0 ; j < tmp->message->n_bytes ; j++) {
                printf(" %02x", tmp->message->bytes[j]);
            }
        } else {
            printf("Text message: ");
            for (unsigned int j = 0 ; j < tmp->message->n_bytes ; j++) {
                u_int8_t c = tmp->message->bytes[j];
                if (c == '<') {
                    printf("&lt;");
                } else if (c == '>') {
                    printf("&gt;");
                } else {
                    printf("%c", c);
                }
            }
        }
        printf("\n");
        printf("</title>\n");
        printf("  </svg>\n");
        printf("</div>\n");

        tmp = tmp->next;
    }

    free_qr_code_match_list(matches);
    printf("</body>\n");
    printf("<html>\n");

    return 0;
}

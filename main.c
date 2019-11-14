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
        printf("Given a png image, tries to locate QR codes in it. On success, prints the decoded content.\n");
        printf("\n");
        return 1;
    }

    struct qr_code_match_list* matches;
    find_qr_codes(argv[1], &matches);
    struct qr_code_match_list* tmp = matches;
    int i = 1;
    while (tmp != NULL) {
        if (contains_binary_data(tmp->message)) {
            printf("Binary message #%d:", i++);
            for (unsigned int j = 0 ; j < tmp->message->n_bytes ; j++) {
                printf(" %02x", tmp->message->bytes[j]);
            }
        } else {
            printf("Text message #%d: %s", i++, tmp->message->bytes);
        }
        printf("\n");
        tmp = tmp->next;
    }

    free_qr_code_match_list(matches);

    return 0;
}

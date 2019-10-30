#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "reedsolomon.h"


/**
 * These examples come from:
 * https://en.wikiversity.org/wiki/Reed%E2%80%93Solomon_codes_for_coders
 */

/**
 * This is a 16 bytes message, suitable for a version 1 QR code with medium
 * error correction.
 */
static u_int8_t test_data[] = {
    0x40, 0xd2, 0x75, 0x47, 0x76, 0x17, 0x32, 0x06, 0x27, 0x26, 0x96, 0xc6, 0xc6, 0x96, 0x70, 0xec
};

/**
 * This corresponds to test_data + the 10 error codewords obtained when applying
 * medium error correction level.
 */
static u_int8_t test_block[] = {
    // data
    0x40, 0xd2, 0x75, 0x47, 0x76, 0x17, 0x32, 0x06, 0x27, 0x26, 0x96, 0xc6, 0xc6, 0x96, 0x70, 0xec,
    // error correction
    0xbc, 0x2a, 0x90, 0x13, 0x6b, 0xaf, 0xef, 0xfd, 0x4b, 0xe0
};

static char* decoded_text = "'Twas brillig";


/**
 * Verifies that the syndromes are all 0 when there is no error.
 */
static int test_reed_solomon1() {
    struct gf_polynomial* message = new_gf_polynomial(26, test_block);
    struct gf_polynomial* syndromes = new_gf_polynomial(10, NULL);

    int result = calculate_syndromes(message, syndromes);
    free_gf_polynomial(message);
    free_gf_polynomial(syndromes);
    return result == 0;
}


/**
 * Verifies that the syndromes are not all 0 when there is an error.
 */
static int test_reed_solomon2() {
    struct gf_polynomial* message = new_gf_polynomial(26, test_block);
    message->coefficients[0] = 0;
    struct gf_polynomial* syndromes = new_gf_polynomial(10, NULL);

    int result = calculate_syndromes(message, syndromes);
    free_gf_polynomial(message);
    free_gf_polynomial(syndromes);
    return result != 0;
}


typedef int (*test)();

int main() {
    printf("Running tests...\n");
    test tests[] = {
        test_reed_solomon1,
        test_reed_solomon2,
        NULL
    };
    int total = 0;
    int failed = 0;
    for (int i = 0 ; tests[i] != NULL ; i++) {
        total++;
        printf("---------------- test #%d ----------------\n", i);
        if (!(tests[i]())) {
            failed++;
        }
    }

    if (failed) {
        printf("%d/%d test%s failed\n\n", failed, total, (failed > 1) ? "s" : "");
        exit(1);
    } else {
        printf("%d/%d test%s successful\n\n", total, total, (total > 1) ? "s" : "");
        exit(0);
    }
}

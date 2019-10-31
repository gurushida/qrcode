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


static void print(char* name, struct gf_polynomial* p) {
    printf("%s (d=%d):", name, get_degree(p));
    for (unsigned int i = 0 ; i <= get_degree(p) ; i++) {
        printf(" %d", get_coefficient(p, i));
    }
    printf("\n");
}


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


static int test_get_degree() {
    struct gf_polynomial* p = new_gf_polynomial(5, (uint8_t[]){ 0, 0, 0, 0, 0 });
    int ok = 1;
    ok = ok && (0 == get_degree(p));

    p->coefficients[2] = 1;
    ok = ok && (2 == get_degree(p));

    p->coefficients[1] = 1;
    ok = ok && (3 == get_degree(p));

    free_gf_polynomial(p);
    return ok;
}


static int test_get_coefficient() {
    struct gf_polynomial* a = new_gf_polynomial(7, (uint8_t[]){ 0, 1, 1, 0, 1, 1, 1 });
    int ok = 1 == get_coefficient(a, 0)
        && 1 == get_coefficient(a, 1)
        && 1 == get_coefficient(a, 2)
        && 0 == get_coefficient(a, 3)
        && 1 == get_coefficient(a, 4)
        && 1 == get_coefficient(a, 5)
        && 0 == get_coefficient(a, 6);
    free_gf_polynomial(a);
    return ok;
}


static int test_set_coefficient() {
    struct gf_polynomial* a = new_gf_polynomial(7, (uint8_t[]){ 0, 1, 1, 0, 1, 1, 1 });
    int ok = 1 == get_coefficient(a, 1);
    set_coefficient(a, 1, 0);
    ok = ok && 0 == get_coefficient(a, 1);
    free_gf_polynomial(a);
    return ok;
}


static int test_equal_polynomials() {
    struct gf_polynomial* a = new_gf_polynomial(8, (uint8_t[]){ 0, 0, 1, 1, 0, 1, 1, 1 });
    struct gf_polynomial* b = new_gf_polynomial(6, (uint8_t[]){ 1, 1, 0, 1, 1, 1 });
    int ok = equal_polynomials(a, b);
    free_gf_polynomial(a);
    free_gf_polynomial(b);
    return ok;
}


static int test_add_polynomials() {
    struct gf_polynomial* a = new_gf_polynomial(7, (uint8_t[]){       1, 1, 0, 0, 0, 1, 1 });
    struct gf_polynomial* b = new_gf_polynomial(9, (uint8_t[]){ 0, 0, 0, 1, 1, 0, 0, 1, 0 });
    struct gf_polynomial* expected = new_gf_polynomial(7, (uint8_t[]){ 1, 0, 1, 0, 0, 0, 1 });
    struct gf_polynomial* c = add_polynomials(a, b);
    int ok = equal_polynomials(expected, c);

    free_gf_polynomial(c);
    c = add_polynomials(b, a);
    ok = ok && equal_polynomials(expected, c);

    free_gf_polynomial(a);
    free_gf_polynomial(b);
    free_gf_polynomial(c);
    free_gf_polynomial(expected);
    return ok;
}


static int test_add_polynomials2() {
    struct gf_polynomial* a = new_gf_polynomial(7, (uint8_t[]){           1, 1, 0, 0, 0, 1, 1 });
    struct gf_polynomial* b = new_gf_polynomial(8, (uint8_t[]){        1, 1, 0, 0, 0, 1, 1, 0 });
    struct gf_polynomial* expected = new_gf_polynomial(8, (uint8_t[]){ 1, 0, 1, 0, 0, 1, 0, 1 });
    struct gf_polynomial* c = add_polynomials(a, b);

    int ok = equal_polynomials(expected, c);

    free_gf_polynomial(c);
    c = add_polynomials(b, a);
    ok = ok && equal_polynomials(expected, c);

    free_gf_polynomial(a);
    free_gf_polynomial(b);
    free_gf_polynomial(c);
    free_gf_polynomial(expected);
    return ok;
}


/**
 * Verifies that given a polynomial A, we have A.X+A = A.(X+1)
 */
static int test_multiply_polynomials() {
    struct gf_polynomial* a = new_gf_polynomial(7, (uint8_t[]){ 1, 1, 0, 0, 0, 1, 1 });
    struct gf_polynomial* x = new_gf_polynomial(2, (uint8_t[]){ 1, 0 });
    struct gf_polynomial* x_1 = new_gf_polynomial(2, (uint8_t[]){ 1, 1 });
    struct gf_polynomial* ax = multiply_polynomials(a, x);
    struct gf_polynomial* ax_a1 = add_polynomials(ax, a);
    struct gf_polynomial* ax_a2 = multiply_polynomials(a, x_1);
    struct gf_polynomial* expected = new_gf_polynomial(8, (uint8_t[]){ 1, 0, 1, 0, 0, 1, 0, 1 });

    int ok = equal_polynomials(expected, ax_a1);
    ok = ok && equal_polynomials(expected, ax_a2);

    free_gf_polynomial(a);
    free_gf_polynomial(x);
    free_gf_polynomial(x_1);
    free_gf_polynomial(ax);
    free_gf_polynomial(ax_a1);
    free_gf_polynomial(ax_a2);
    free_gf_polynomial(expected);
    return ok;
}


/**
 * Verifies that dividing a polynomial equal to A.(X+1) by
 * (X+1) given A as the quotient and 0 as the remainder.
 */
static int test_divide_polynomials() {
    struct gf_polynomial* a = new_gf_polynomial(8, (uint8_t[]){ 1, 0, 1, 0, 0, 1, 0, 1 });
    struct gf_polynomial* b = new_gf_polynomial(2, (uint8_t[]){ 1, 1 });
    struct gf_polynomial* expected_q = new_gf_polynomial(7, (uint8_t[]){ 1, 1, 0, 0, 0, 1, 1 });
    struct gf_polynomial* expected_r = new_gf_polynomial(1, (uint8_t[]){ 0 });

    struct gf_polynomial* quotient;
    struct gf_polynomial* remainder;
    int res = divide_polynomials(a, b, &quotient, &remainder);
    int ok = res == 1
        && equal_polynomials(expected_q, quotient)
        && equal_polynomials(expected_r, remainder);

    free_gf_polynomial(a);
    free_gf_polynomial(b);
    free_gf_polynomial(expected_q);
    free_gf_polynomial(expected_r);
    free_gf_polynomial(quotient);
    free_gf_polynomial(remainder);
    return ok;
}


/**
 * Verifies that dividing a polynomial equal to A.(X+1)+1 by
 * (X+1) given A as the quotient and 1 as the remainder.
 */
static int test_divide_polynomials2() {
    struct gf_polynomial* a = new_gf_polynomial(8, (uint8_t[]){ 1, 0, 1, 0, 0, 1, 0, 0 });
    struct gf_polynomial* b = new_gf_polynomial(2, (uint8_t[]){ 1, 1 });
    struct gf_polynomial* expected_q = new_gf_polynomial(7, (uint8_t[]){ 1, 1, 0, 0, 0, 1, 1 });
    struct gf_polynomial* expected_r = new_gf_polynomial(1, (uint8_t[]){ 1 });

    struct gf_polynomial* quotient;
    struct gf_polynomial* remainder;
    int res = divide_polynomials(a, b, &quotient, &remainder);

    int ok = res == 1
        && equal_polynomials(expected_q, quotient)
        && equal_polynomials(expected_r, remainder);

    free_gf_polynomial(a);
    free_gf_polynomial(b);
    free_gf_polynomial(expected_q);
    free_gf_polynomial(expected_r);
    free_gf_polynomial(quotient);
    free_gf_polynomial(remainder);
    return ok;
}


typedef int (*test)();

int main() {
    printf("Running tests...\n");
    test tests[] = {
        test_reed_solomon1,
        test_reed_solomon2,
        test_get_degree,
        test_get_coefficient,
        test_set_coefficient,
        test_equal_polynomials,
        test_add_polynomials,
        test_add_polynomials2,
        test_multiply_polynomials,
        test_divide_polynomials,
        test_divide_polynomials2,
        NULL
    };
    int total = 0;
    int failed = 0;
    for (int i = 0 ; tests[i] != NULL ; i++) {
        total++;
        printf("---------------- test #%d ----------------\n", i);
        if (!(tests[i]())) {
            failed++;
            printf("============ test #%d failed! ============\n", i);
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

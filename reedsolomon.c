#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "galoisfield.h"
#include "logs.h"
#include "polynomial.h"
#include "reedsolomon.h"


unsigned int calculate_syndromes(struct gf_polynomial* message, struct gf_polynomial* syndromes) {
    unsigned int n = 0;
    unsigned int n_syndromes = syndromes->n_coefficients;
    for (unsigned int i = 0 ; i < n_syndromes ; i++) {
        u_int8_t x = gf_power(i);
        syndromes->coefficients[n_syndromes - 1 - i] = evaluate_polynomial(message, x);
        if (syndromes->coefficients[i]) {
            n++;
        }
    }
    return n;
}


/**
 * Returns a polynomial with all coefficients to zero or null in case
 * of memory allocation error.
 */
static struct gf_polynomial* zero() {
    return get_monomial(0, 0);
}


/**
 * Returns a polynomial with equal to the constant term 1 or null in case
 * of memory allocation error.
 */
static struct gf_polynomial* one() {
    return get_monomial(0, 1);
}


int calculate_sigma_omega(struct gf_polynomial* syndromes, unsigned int n_error_correction_codewords,
                            struct gf_polynomial* *sigma, struct gf_polynomial* *omega) {

    struct gf_polynomial* syndromes_copy = new_gf_polynomial(syndromes->n_coefficients, syndromes->coefficients);
    if (syndromes_copy == NULL) {
        return MEMORY_ERROR;
    }
    struct gf_polynomial* monomial = get_monomial(n_error_correction_codewords, 1);
    if (monomial == NULL) {
        free_gf_polynomial(syndromes_copy);
        return MEMORY_ERROR;
    }
    set_coefficient(monomial, n_error_correction_codewords, 1);

    struct gf_polynomial* rLast;
    struct gf_polynomial* r;
    struct gf_polynomial* tLast = zero();
    if (tLast == NULL) {
        free_gf_polynomial(syndromes_copy);
        free_gf_polynomial(monomial);
        return MEMORY_ERROR;
    }
    struct gf_polynomial* t = one();
    if (t == NULL) {
        free_gf_polynomial(syndromes_copy);
        free_gf_polynomial(monomial);
        free_gf_polynomial(tLast);
        return MEMORY_ERROR;
    }

    // Let's make sure that rLast is the one with the highest degree
    if (get_degree(syndromes_copy) < n_error_correction_codewords) {
        rLast = monomial;
        r = syndromes_copy;
    } else {
        rLast = syndromes_copy;
        r = monomial;
    }

    // Now let's divide rLast by r
    while (get_degree(r) >= n_error_correction_codewords / 2) {
        struct gf_polynomial* rLastLast = rLast;
        struct gf_polynomial* tLastLast = tLast;
        rLast = r;
        tLast = t;

        if (is_zero_polynomial(rLast)) {
            free_gf_polynomial(r);
            free_gf_polynomial(t);
            free_gf_polynomial(rLast);
            free_gf_polynomial(tLast);
            return DECODING_ERROR;
        }

        r = rLastLast;
        struct gf_polynomial* q = zero();
        if (q == NULL) {
            free_gf_polynomial(r);
            free_gf_polynomial(t);
            free_gf_polynomial(rLast);
            free_gf_polynomial(tLast);
            return MEMORY_ERROR;
        }

        u_int8_t denominator_leading_term = get_coefficient(rLast, get_degree(rLast));
        u_int8_t dlt_inverse = gf_inverse(denominator_leading_term);

        while (get_degree(r) >= get_degree(rLast) && !is_zero_polynomial(r)) {
            int degree_diff = get_degree(r) - get_degree(rLast);
            u_int8_t scale = gf_multiply(get_coefficient(r, get_degree(r)), dlt_inverse);

            struct gf_polynomial* scale_monomial = get_monomial(degree_diff, scale);
            if (scale_monomial == NULL) {
                free_gf_polynomial(r);
                free_gf_polynomial(t);
                free_gf_polynomial(rLast);
                free_gf_polynomial(tLast);
                return MEMORY_ERROR;
            }
            struct gf_polynomial* new_q = add_polynomials(q, scale_monomial);
            if (new_q == NULL) {
                free_gf_polynomial(r);
                free_gf_polynomial(t);
                free_gf_polynomial(rLast);
                free_gf_polynomial(tLast);
                free_gf_polynomial(scale_monomial);
                return MEMORY_ERROR;
            }
            free_gf_polynomial(q);
            q = new_q;

            struct gf_polynomial* tmp = multiply_polynomials(rLast, scale_monomial);
            if (tmp == NULL) {
                free_gf_polynomial(r);
                free_gf_polynomial(t);
                free_gf_polynomial(rLast);
                free_gf_polynomial(tLast);
                free_gf_polynomial(scale_monomial);
                return MEMORY_ERROR;
            }
            struct gf_polynomial* new_r = add_polynomials(r, tmp);
            if (new_r == NULL) {
                free_gf_polynomial(r);
                free_gf_polynomial(t);
                free_gf_polynomial(rLast);
                free_gf_polynomial(tLast);
                free_gf_polynomial(scale_monomial);
                free_gf_polynomial(tmp);
                return MEMORY_ERROR;
            }
            r = new_r;
            free_gf_polynomial(tmp);
            free_gf_polynomial(scale_monomial);
        }

        struct gf_polynomial* q_x_tLast = multiply_polynomials(q, tLast);
        if (q_x_tLast == NULL) {
            free_gf_polynomial(r);
            free_gf_polynomial(t);
            free_gf_polynomial(rLast);
            free_gf_polynomial(tLast);
            free_gf_polynomial(q);
            return MEMORY_ERROR;
        }
        struct gf_polynomial* new_t = add_polynomials(q_x_tLast, tLastLast);
        if (new_t == NULL) {
            free_gf_polynomial(r);
            free_gf_polynomial(t);
            free_gf_polynomial(rLast);
            free_gf_polynomial(tLast);
            free_gf_polynomial(q_x_tLast);
            free_gf_polynomial(q);
            return MEMORY_ERROR;
        }
        t = new_t;
        free_gf_polynomial(q_x_tLast);
        free_gf_polynomial(q);

        if (get_degree(r) >= get_degree(rLast)) {
            free_gf_polynomial(r);
            free_gf_polynomial(t);
            free_gf_polynomial(rLast);
            free_gf_polynomial(tLast);
            return DECODING_ERROR;
        }
    }

    u_int8_t sigma_tilde_at_0 = get_coefficient(t, 0);
    if (sigma_tilde_at_0 == 0) {
        free_gf_polynomial(r);
        free_gf_polynomial(t);
        free_gf_polynomial(rLast);
        free_gf_polynomial(tLast);
        return DECODING_ERROR;
    }
    u_int8_t inverse = gf_inverse(sigma_tilde_at_0);
    multiply_by_scalar(t, inverse);
    multiply_by_scalar(r, inverse);
    *sigma = t;
    *omega = r;

    free_gf_polynomial(rLast);
    free_gf_polynomial(tLast);
    return SUCCESS;
}


int find_error_locations(struct gf_polynomial* sigma, u_int8_t* locations) {
    unsigned int n_errors = get_degree(sigma);
    unsigned int n = 0;
    for (unsigned int i = 1 ; i <= 255 && n < n_errors; i++) {
        if (evaluate_polynomial(sigma, i) == 0) {
            locations[n++] = gf_inverse(i);
        }
    }
    return (n == n_errors) ? SUCCESS : DECODING_ERROR;
}


void find_error_magnitudes(struct gf_polynomial* omega, unsigned int n_errors,
                            u_int8_t* error_locations, u_int8_t* error_magnitudes) {
    for (unsigned int i = 0 ; i < n_errors ; i++) {
        u_int8_t xi_inverse = gf_inverse(error_locations[i]);
        u_int8_t denominator = 1;
        for (unsigned int j = 0 ; j < n_errors ; j++) {
            if (i != j) {
                denominator = gf_multiply(denominator,
                                            gf_add_or_subtract(1, gf_multiply(error_locations[j], xi_inverse)));
            }
        }
        error_magnitudes[i] = gf_multiply(evaluate_polynomial(omega, xi_inverse), gf_inverse(denominator));
    }
}


int error_correction(struct block* b) {
    struct gf_polynomial message;
    message.coefficients = b->codewords;
    message.n_coefficients = b->n_data_codewords + b->n_error_correction_codewords;

    gory("%d data codewords:\n", b->n_data_codewords);
    print_bytes(GORY, b->codewords, b->n_data_codewords);
    gory("\n%d error detection/correction codewords:\n", b->n_error_correction_codewords);
    print_bytes(GORY, b->codewords + b->n_data_codewords, b->n_error_correction_codewords);


    struct gf_polynomial* syndromes = new_gf_polynomial(b->n_error_correction_codewords, NULL);
    if (syndromes == NULL) {
        return MEMORY_ERROR;
    }
    if (0 == calculate_syndromes(&message, syndromes)) {
        free_gf_polynomial(syndromes);
        return 0;
    }
    poly_print(GORY, "\nSyndromes", syndromes);

    struct gf_polynomial* sigma;
    struct gf_polynomial* omega;

    int res = calculate_sigma_omega(syndromes, b->n_error_correction_codewords, &sigma, &omega);
    if (res != SUCCESS) {
        gory("Cannot calculate sigma and omega polynomials\n");
        free_gf_polynomial(syndromes);
        return res;
    }

    poly_print(GORY, "\nsigma", sigma);
    poly_print(GORY, "\nomega", omega);

    unsigned int n_errors = get_degree(sigma);
    gory("\n%d error%s detected\n", n_errors, (n_errors > 1) ? "s" : "");

    u_int8_t* error_locations = (u_int8_t*)malloc(n_errors * sizeof(u_int8_t));
    if (error_locations == NULL) {
        free_gf_polynomial(syndromes);
        free_gf_polynomial(sigma);
        free_gf_polynomial(omega);
        return MEMORY_ERROR;
    }

    res = find_error_locations(sigma, error_locations);
    if (res == DECODING_ERROR) {
        gory("Cannot find error locations\n");
        free_gf_polynomial(syndromes);
        free_gf_polynomial(sigma);
        free_gf_polynomial(omega);
        free(error_locations);
        return DECODING_ERROR;
    }

    gory("\nError are at bytes:");
    int total = b->n_data_codewords + b->n_error_correction_codewords;
    for (unsigned int i = 0 ; i < n_errors ; i++) {
        gory(" %d", total - 1 - gf_log(error_locations[i]));
    }
    gory("\n");

    u_int8_t* error_magnitudes = (u_int8_t*)malloc(n_errors * sizeof(u_int8_t));
    if (error_magnitudes == NULL) {
        free_gf_polynomial(syndromes);
        free_gf_polynomial(sigma);
        free_gf_polynomial(omega);
        free(error_locations);
        return MEMORY_ERROR;
    }
    find_error_magnitudes(omega, n_errors, error_locations, error_magnitudes);

    // Finally, let's apply the corrections
    for (unsigned int i = 0 ; i < n_errors ; i++) {
        unsigned int pos = message.n_coefficients - 1 - gf_log(error_locations[i]);
        u_int8_t bad = b->codewords[pos];
        b->codewords[pos] = gf_add_or_subtract(b->codewords[pos], error_magnitudes[i]);
        gory("Correcting codeword #%d from %02x to %02x\n", pos, bad, b->codewords[pos]);
    }

    free_gf_polynomial(syndromes);
    free_gf_polynomial(sigma);
    free_gf_polynomial(omega);
    free(error_locations);
    free(error_magnitudes);

    gory("\nByte sequence after error correction:\n");
    print_bytes(GORY, b->codewords, b->n_data_codewords);
    gory("\n");
    return n_errors;
}


int get_message_bitstream(struct blocks* blocks, struct bitstream* *bitstream) {
    unsigned int n = 0;
    for (unsigned int i = 0 ; i < blocks->n_blocks ; i++) {
        gory("\nApplying error detection/correction to block %d/%d...\n", (i + 1), blocks->n_blocks);
        n += blocks->block[i].n_data_codewords;
        int res = error_correction(&(blocks->block[i]));
        if (res < 0) {
            return res;
        }
        if (res > 0) {
            info("Fixed %d errors in block %d/%d\n", res, (i + 1), blocks->n_blocks);
        } else {
            info("No errors in block %d/%d\n", (i + 1), blocks->n_blocks);
        }
    }

    (*bitstream) = new_bitstream(n);
    if ((*bitstream) == NULL) {
        return MEMORY_ERROR;
    }

    unsigned int pos = 0;
    for (unsigned int i = 0 ; i < blocks->n_blocks ; i++) {
        int n_bytes = blocks->block[i].n_data_codewords * sizeof(u_int8_t);
        memcpy((*bitstream)->bytes + pos, blocks->block[i].codewords, n_bytes);
        pos += n_bytes;
    }

    gory("\nAll blocks successfully parsed into %d bytes:\n", (*bitstream)->n_bytes);
    print_bytes(GORY, (*bitstream)->bytes, (*bitstream)->n_bytes);
    return SUCCESS;
}

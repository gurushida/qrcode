#include <stdlib.h>
#include "galoisfield.h"
#include "polynomial.h"
#include "reedsolomon.h"


unsigned int calculate_syndromes(struct gf_polynomial* message, struct gf_polynomial* syndromes) {
    unsigned int n = 0;
    unsigned int n_syndromes = syndromes->n_coefficients;
    for (unsigned int i = 0 ; i < n_syndromes ; i++) {
        u_int8_t x = gf_power(i);
        syndromes->coefficients[i] = evaluate_polynomial(message, x);
        if (syndromes->coefficients[i]) {
            n++;
        }
    }
    return n;
}


int error_correction(struct block* b) {
    struct gf_polynomial message;
    message.coefficients = b->codewords;
    message.n_coefficients = b->n_data_codewords + b->n_error_correction_codewords;

    struct gf_polynomial* syndromes = new_gf_polynomial(b->n_error_correction_codewords, NULL);
    if (syndromes == NULL) {
        return -2;
    }
    if (0 == calculate_syndromes(&message, syndromes)) {
        free_gf_polynomial(syndromes);
        return 0;
    }

    free_gf_polynomial(syndromes);

    return -1;
}

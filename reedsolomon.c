#include "galoisfield.h"
#include "polynomial.h"
#include "reedsolomon.h"


/**
 * Each block of M codewords is treated a polynomial like:
 *
 * R(X) = r_(M-1) . X^(M-1) + r_(M-2) . X^(M-2) + ... + r_1 . X + r_0
 *
 * where the values R=(r_(M-1), r_(M-2), ... , r_0) represents the codewords.
 * In order to verify if the block contains errors, we need to calculate
 * N syndromes where N is 2*t (twice the error correction capacity), where
 * each syndrome S_i is equal to R(alpha^i) where alpha is the generator
 * element of the Galois field. If all syndromes are 0s, then there is no
 * error to be corrected. Otherwise there will be more work to do to figure
 * out how many errors there are and where they are.
 *
 * @param b The block on which to calculate the syndromes
 * @param syndromes An array large enough to store all the syndromes
 * @return The number of non zero syndromes
 */
static int calculate_syndromes(struct block* b, u_int8_t* syndromes) {
    int n = 0;
    unsigned int n_syndromes = b->error_correction_capacity * 2;
    for (unsigned int i = 0 ; i < n_syndromes ; i++) {
        u_int8_t x = gf_power(i);
        syndromes[i] = evaluate_polynomial(b->codewords, b->n_data_codewords + b->n_error_correction_codewords, x);
        if (syndromes[i]) {
            n++;
        }
    }
    return n;
}


int error_correction(struct block* b) {
    u_int8_t syndromes[MAX_ERROR_CORRECTION_CAPACITY * 2];
    if (0 == calculate_syndromes(b, syndromes)) {
        return 0;
    }
    // TODO error correction
    return -1;
}

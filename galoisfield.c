#include "galoisfield.h"

// The prime polynomial used to define the 2^8 Galois field
// used by QR codes.
// Note that this is a 9-bit value !
static u_int16_t PRIME_POLYNOMIAL = ((1<<8) + (1<<4) + (1<<3) + (1<<2) + 1);

// Powers of the generator value
static u_int8_t powers[256];

// Logarithms
static u_int8_t logarithms[256];

static int initialized = 0;


/**
 * Precomputes all the powers and logarithms.
 */
static void initialize() {
    // Let's start with the polynomial 1 which is represented
    // by the integer value 1 and corresponds to alpha^0 where
    // alpha is the generator element which happens to be X for
    // QR codes.
    // Note that element needs to be more than 8 bits as some operations
    // will overflow 8 bit values, hence the 16 bit variable.
    u_int16_t element = 1;

    for (int i = 0 ; i < 256 ; i++) {
        // The initial value is 1 and all subsequent values will be
        // lower than 256 as explained below, so it is always a safe
        // operation to assign element to an unsigned 8 bit value
        powers[i] = element;

        // In order to enumerate the powers of the X polynomial,
        // we need to multiply each value X^j by X to find the next
        // one X^(j+1). However, multiplying a polynomial like
        // X^5 + X^3 + 1 by X will give X^6 + X^4 + X which
        // is the same as shifting the bits representing the first
        // polynomial one bit to the left. In our example,
        // X^5 + X^3 + 1 would be represented in binary by
        // 00101001 and multiplying it by X would give 01010010
        element = (element<<1);

        if (element >= 256) {
            // In a Galois field, all operations must be done
            // modulo P where P is the prime polynomial which, by
            // definition, has a degree greater by 1 than the
            // highest value of the field. As a consequence,
            // if we have a value element >= 256, we have to take
            // the value (element modulo P).
            //
            // However, since the generator is X, we know that
            // multiplying by X a value < 256 can only give a value
            // >= 256 of the same degree as P because in order to
            // get a higher degree, we should have multiplied by
            // something with a bigger degree than X like for instance
            // (X^3 + 1).
            //
            // This means that we are in a situation where we have:
            //
            // element = 1 . P + R
            //
            // where R is the remainder of (element / P), in other words,
            // (element modulo P) which is what we are looking and whose
            // value is (element - P).
            //
            // Note that after this modulo, the value is guaranteed to be < 256
            element = (element ^ PRIME_POLYNOMIAL);
        }
    }

    // Once we have all the powers of the generator,
    // it is trivial to fill up the logarithm table
    for (int i = 0 ; i < 256 ; i++) {
        logarithms[powers[i]] = i;
    }

    initialized = 1;
}

u_int8_t gf_power(u_int8_t n) {
    if (!initialized) {
        initialize();
    }
    return powers[n];
}


static u_int8_t gf_log(u_int8_t a) {
    if (!initialized) {
        initialize();
    }
    return logarithms[a];
}


u_int8_t gf_add_or_subtract(u_int8_t a, u_int8_t b) {
    return a ^ b;
}


u_int8_t gf_multiply(u_int8_t a, u_int8_t b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    return gf_power((gf_log(a) + gf_log(b)) % 255);
}


uint8_t gf_inverse(uint8_t a) {
    return gf_power(255 - gf_log(a));
}


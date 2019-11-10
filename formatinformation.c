#include "formatinformation.h"

// The table C.1 in the In ISO/IEC 18004:2006 Annex C enumerates the
// valid 15-bit sequences that correspond to the possible 5-bit values
// encoding the format information, after XOR-masking with 101010000010010.
// The reason for masking with this value is to make it impossible to
// have only zeroes in the 15-bit string.
//
// This array lists these values so that code[x] is the 15-bit masked
// value encoding the 5-bit value x
static u_int16_t code[] = {
    0x5412,
    0x5125,
    0x5E7C,
    0x5B4B,
    0x45F9,
    0x40CE,
    0x4F97,
    0x4AA0,
    0x77C4,
    0x72F3,
    0x7DAA,
    0x789D,
    0x662F,
    0x6318,
    0x6C41,
    0x6976,
    0x1689,
    0x13BE,
    0x1CE7,
    0x19D0,
    0x0762,
    0x0255,
    0x0D0C,
    0x083B,
    0x355F,
    0x3068,
    0x3F31,
    0x3A06,
    0x24B4,
    0x2183,
    0x2EDA,
    0x2BED
};


/**
 * Compares the number of bits that differ between the
 * 15 lowest bits of a and b.
 */
static int count_different_bits(u_int16_t a, u_int16_t b) {
    int diff = 0;
    for (int i = 0 ; i < 15 ; i++) {
        if ((a & 1) != (b & 1)) {
            diff++;
        }
        a >>= 1;
        b >>= 1;
    }
    return diff;
}


int get_format_information(struct bit_matrix* bm, ErrorCorrectionLevel *ec, u_int8_t *mask_pattern) {
    if (bm->width != bm->height
        || bm->width < 21
        || bm->width > 177
        || (bm->width % 4) != 1) {
            return 0;
    }

    uint16_t formatInfo1 =
          is_black(bm, 0, 8) << 14   // A
        | is_black(bm, 1, 8) << 13   // B
        | is_black(bm, 2, 8) << 12   // C
        | is_black(bm, 3, 8) << 11   // D
        | is_black(bm, 4, 8) << 10   // E
        | is_black(bm, 5, 8) << 9    // F
        | is_black(bm, 7, 8) << 8    // G
        | is_black(bm, 8, 8) << 7    // H
        | is_black(bm, 8, 7) << 6    // I
        | is_black(bm, 8, 5) << 5    // J
        | is_black(bm, 8, 4) << 4    // K
        | is_black(bm, 8, 3) << 3    // L
        | is_black(bm, 8, 2) << 2    // M
        | is_black(bm, 8, 1) << 1    // N
        | is_black(bm, 8, 0);        // O

    uint16_t formatInfo2 =
          is_black(bm, 8, bm->height - 1) << 14   // A
        | is_black(bm, 8, bm->height - 2) << 13   // B
        | is_black(bm, 8, bm->height - 3) << 12   // C
        | is_black(bm, 8, bm->height - 4) << 11   // D
        | is_black(bm, 8, bm->height - 5) << 10   // E
        | is_black(bm, 8, bm->height - 6) << 9    // F
        | is_black(bm, 8, bm->height - 7) << 8    // G
        | is_black(bm, bm->width - 8 , 8) << 7    // H
        | is_black(bm, bm->width - 7 , 8) << 6    // I
        | is_black(bm, bm->width - 6 , 8) << 5    // J
        | is_black(bm, bm->width - 5 , 8) << 4    // K
        | is_black(bm, bm->width - 4 , 8) << 3    // L
        | is_black(bm, bm->width - 3 , 8) << 2    // M
        | is_black(bm, bm->width - 2 , 8) << 1    // N
        | is_black(bm, bm->width - 1 , 8);        // O

    // In order to find which 5-bit value is the one we want,
    // we compare our 2 15-bit values to each of the 32 valid
    // sequences. The design of the Bose-Chaudhuri-Hocquenghem
    // (15,5) code used here allows for the detection of 3
    // wrong bits. Therefore, If we find a sequence that has at
    // most 3 bits different from either of our 15-bit values,
    // we have found our winner
    int bestBitDifference = 32;
    int bestValue = -1;

    for (int i = 0 ; i < 32 ; i++) {
        if (code[i] == formatInfo1 || code [i] == formatInfo2) {
            // If we have a perfect match, let's stop right there
            bestBitDifference = 0;
            bestValue = i;
            break;
        }

        int bitDifference1 = count_different_bits(formatInfo1, code[i]);
        if (bitDifference1 < bestBitDifference) {
            bestBitDifference = bitDifference1;
            bestValue = i;
        }

        int bitDifference2 = count_different_bits(formatInfo2, code[i]);
        if (bitDifference2 < bestBitDifference) {
            bestBitDifference = bitDifference2;
            bestValue = i;
        }
    }

    if (bestBitDifference > 3) {
        return -1;
    }
    int bit_value = ((bestValue >> 3) & 3);
    switch (bit_value) {
        case 1: *ec = LOW; break;
        case 0: *ec = MEDIUM; break;
        case 3: *ec = QUARTILE; break;
        case 2: *ec = HIGH; break;
    }
    *mask_pattern = bestValue & 7;

    return 1;
}

#include "versioninformation.h"

/**
 * This array contains the valid 18-bit sequences described in
 * table D.1 in annex D of ISO/IEC 18004:2006 so that
 * code[x] is the sequence corresponding the version number (x + 7).
 */
u_int32_t code[] = {
    0x07C94,
    0x085BC,
    0x09A99,
    0x0A4D3,
    0x0BBF6,
    0x0C762,
    0x0D847,
    0x0E60D,
    0x0F928,
    0x10B78,
    0x1145D,
    0x12A17,
    0x13532,
    0x149A6,
    0x15683,
    0x168C9,
    0x177EC,
    0x18EC4,
    0x191E1,
    0x1AFAB,
    0x1B08E,
    0x1CC1A,
    0x1D33F,
    0x1ED75,
    0x1F250,
    0x209D5,
    0x216F0,
    0x228BA,
    0x2379F,
    0x24B0B,
    0x2542E,
    0x26A64,
    0x27541,
    0x28C69
};


/**
 * Compares the number of bits that differ between the
 * 18 lowest bits of a and b.
 */
static int count_different_bits(u_int16_t a, u_int16_t b) {
    int diff = 0;
    for (int i = 0 ; i < 18 ; i++) {
        if ((a & 1) != (b & 1)) {
            diff++;
        }
        a >>= 1;
        b >>= 1;
    }
    return diff;
}


int get_version_information(struct bit_matrix* bm, u_int8_t *version_info) {
    if (bm->width != bm->height
        || bm->width < 21
        || bm->width > 177
        || (bm->width % 4) != 1) {
            return 0;
    }

    // Version * 4 + 17 = size in modules
    *version_info = (bm->width - 17) / 4;

    if (bm->width < 45) {
        return 1;
    }

    u_int32_t versionInfo1 =
              is_black(bm, 5, bm->height - 9) << 17    // A
            | is_black(bm, 5, bm->height - 10) << 16   // B
            | is_black(bm, 5, bm->height - 11) << 15   // C
            | is_black(bm, 4, bm->height - 9) << 14    // D
            | is_black(bm, 4, bm->height - 10) << 13   // E
            | is_black(bm, 4, bm->height - 11) << 12   // F
            | is_black(bm, 3, bm->height - 9) << 11    // G
            | is_black(bm, 3, bm->height - 10) << 10   // H
            | is_black(bm, 3, bm->height - 11) << 9    // I
            | is_black(bm, 2, bm->height - 9) << 8     // J
            | is_black(bm, 2, bm->height - 10) << 7    // K
            | is_black(bm, 2, bm->height - 11) << 6    // L
            | is_black(bm, 1, bm->height - 9) << 5     // M
            | is_black(bm, 1, bm->height - 10) << 4    // N
            | is_black(bm, 1, bm->height - 11) << 3    // O
            | is_black(bm, 0, bm->height - 9) << 2     // P
            | is_black(bm, 0, bm->height - 10) << 1    // Q
            | is_black(bm, 0, bm->height - 11);        // R

    u_int32_t versionInfo2 =
              is_black(bm, bm->width - 9, 5) << 17     // A
            | is_black(bm, bm->width - 10, 5) << 16    // B
            | is_black(bm, bm->width - 11, 5) << 15    // C
            | is_black(bm, bm->width - 9, 4) << 14     // D
            | is_black(bm, bm->width - 10, 4) << 13    // E
            | is_black(bm, bm->width - 11, 4) << 12    // F
            | is_black(bm, bm->width - 9, 3) << 11     // G
            | is_black(bm, bm->width - 10, 3) << 10    // H
            | is_black(bm, bm->width - 11, 3) << 9     // I
            | is_black(bm, bm->width - 9, 2) << 8      // J
            | is_black(bm, bm->width - 10, 2) << 7     // K
            | is_black(bm, bm->width - 11, 2) << 6     // L
            | is_black(bm, bm->width - 9, 1) << 5      // M
            | is_black(bm, bm->width - 10, 1) << 4     // N
            | is_black(bm, bm->width - 11, 1) << 3     // O
            | is_black(bm, bm->width - 9, 0) << 2      // P
            | is_black(bm, bm->width - 10, 0) << 1     // Q
            | is_black(bm, bm->width - 11, 0);         // R

    // Like for the format information, we have a tolerance for up to
    // 3 errors, so we scan the legal values to look for the best match
    int bestBitDifference = 32;
    int bestValue = -1;

    for (int i = 0 ; i < 34 ; i++) {
        if (code[i] == versionInfo1 || code [i] == versionInfo2) {
            // If we have a perfect match, let's stop right there
            bestBitDifference = 0;
            bestValue = i + 7;
            break;
        }

        int bitDifference1 = count_different_bits(versionInfo1, code[i]);
        if (bitDifference1 < bestBitDifference) {
            bestBitDifference = bitDifference1;
            bestValue = i + 7;
        }

        int bitDifference2 = count_different_bits(versionInfo2, code[i]);
        if (bitDifference2 < bestBitDifference) {
            bestBitDifference = bitDifference2;
            bestValue = i + 7;
        }
    }

    if (bestBitDifference > 3) {
        return -1;
    }

    if (bestValue != (*version_info)) {
        (*version_info) = bestValue;
        return -2;
    }

    return 1;
}

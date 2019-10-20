#ifndef _FORMATINFORMATION_H
#define _FORMATINFORMATION_H

#include <stdint.h>
#include "bitmatrix.h"

/**
 * The error correction level defines the ratio between
 * storage capacity and error tolerance. The lower the
 * error correction, the more information can be stored
 * in the QR code.
 */
typedef enum {
    LOW = 3,         // 7% of codewords can be restored
    MEDIUM = 2,      // 15%
    QUARTILE = 1,    // 25%
    HIGH = 0         // 30%
} ErrorCorrectionLevel;


/**
 * The format information of a QR code consists of a 2-bits
 * value representing the error correction level and a 3-bits
 * value representing the mask pattern used to ensure that the
 * modules look random enough.
 *
 * These 5 bits are completed with 10 bits for error correction.
 * Each code contains 2 copies of these 15 bits as follows:
 *
 * AB         = error correction level
 * CDE        = mask pattern
 * FGHIJKLMNO = bits used to correct errors on the ABCDE bits
 *
 *   ####### O         #######
 *   #     # N         #     #
 *   # ### # M         # ### #
 *   # ### # L         # ### #
 *   # ### # K         # ### #
 *   #     # J         #     #
 *   #######           #######
 *           I
 *   ABCDEF GH        HIJKLMNO
 *
 *
 *
 *
 *   ####### G
 *   #     # F
 *   # ### # E
 *   # ### # D
 *   # ### # C
 *   #     # B
 *   ####### A
 *
 * Given a bit matrix representing a QR code, this function extracts the
 * the error correction level and the mask pattern used in the QR code.
 *
 * @param bm A bit matrix representing a QR code
 * @param ec The address here to store the error correction level which is
 *           a value between 0 and 3
 * @param mask_pattern The address to store the mask pattern which is a value
 *                     between 0 and 7
 *
 * @return 1 on success
 *         0 if the given bit matrix has a dimension incompatible with a QR code
 *        -1 if it was not possible to decode the format information
 */
int get_formation_information(struct bit_matrix* bm, ErrorCorrectionLevel *ec, uint8_t *mask_pattern);


#endif

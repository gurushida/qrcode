#ifndef _VERSIONINFORMATION_H
#define _VERSIONINFORMATION_H

#include <stdint.h>
#include "bitmatrix.h"
#include "errors.h"


/**
 * QR codes with a dimension >= 45 include 2 copies of the version
 * information made of 6 bits ABCDEF of data and 12 bits GHIJKLMNOPQR
 * for error correction stored as a follows:
 *
 *   #######         RQP  #######
 *   #     #         ONM  #     #
 *   # ### #         LKJ  # ### #
 *   # ### #         IHG  # ### #
 *   # ### #         FED  # ### #
 *   #     #         CBA  #     #
 *   #######              #######
 *
 *
 *
 *
 *   ROLIFC
 *   QNKHEB
 *   PMJGDA
 *
 *   #######
 *   #     #
 *   # ### #
 *   # ### #
 *   # ### #
 *   #     #
 *   #######
 *
 *
 * @param bm A bit matrix representing a QR code
 * @param version_info The address where to store a value between 1 and 40
 * @return SUCCESS on success; the version is stored in (*version_info)
 *         DECODING_ERROR if the given bit matrix has a dimension incompatible with a QR code
 *           or if it was not possible to decode the version information)
 */
int get_version_information(struct bit_matrix* bm, u_int8_t *version_info);


#endif

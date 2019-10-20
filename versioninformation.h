#ifndef _VERSIONINFORMATION_H
#define _VERSIONINFORMATION_H

#include <stdint.h>
#include "bitmatrix.h"

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
 * @return 1 on success; the version is stored in (*version_info)
 *         0 if the given bit matrix has a dimension incompatible with a QR code;
 *           (*version_info) is not modified
 *        -1 if it was not possible to decode the version information;
 *           (*version_info) contains the expected version number calculated from
 *           the size of the QR code
 *        -2 if the decoded version information is not in agreement with the
 *           version number deduced from the QR code size. In such a case,
 *           the decoded version number will be stored in (*version_info)
 */
int get_version_information(struct bit_matrix* bm, u_int8_t *version_info);


#endif

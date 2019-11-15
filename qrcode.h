#ifndef _QRCODE_H
#define _QRCODE_H

#include "bitmatrix.h"
#include "bytebuffer.h"
#include "finderpattern.h"
#include "logs.h"


/**
 * This structure represents a list of matches in an image.
 */
struct qr_code_match_list {
    // The decoded message. Unless the data contains some bytes
    // with a value <= 0x1F that would likely mean that the data
    // is binary, the byte buffer can be used as a '\0' terminated
    // UTF8 string. Note that message->n_bytes contains the number
    // of bytes that were decoded from the QR code, without taking
    // into account the extra '\0' added at the end to turn the data
    // into a C string (provided that there is no null byte in the
    // data)
    struct bytebuffer* message;

    // The coordinates of the QR code corners in the original image
    int bottom_left_x, bottom_left_y;
    int top_left_x, top_left_y;
    int top_right_x, top_right_y;
    int bottom_right_x, bottom_right_y;

    // The next match
    struct qr_code_match_list* next;
};


/**
 * Given a png file, this function analyzes it to try to find
 * QR codes.
 *
 * @param png The path to the .png file to analyze
 * @param list Where to store the results, if any
 * @param potential_finder_patterns If not NULL, this is where will be
 *                                  stored all the positions of the potential
 *                                  finder patterns that were identified in the
 *                                  image
 * @return SUCCESS if at least one QR code is found
 *         DECODING_ERROR if no QR code is found in the image
 *         MEMORY_ERROR in case of memory allocation error
 *         CANNOT_LOAD_IMAGE if the image cannot be loaded
 */
int find_qr_codes(const char* png, struct qr_code_match_list* *list,
                struct finder_pattern_list* *potential_finder_patterns);


/**
 * Given a bit matrix that is supposed to represent a QR code (i.e. the
 * matrix is a square one where each cell represents a module), this
 * functions tries to decode it.
 *
 * @param matrix The matrix that is supposed to represent the QR code
 * @param code Where to store the result or NULL if the decoding fails
 * @return SUCCESS on success
 *         DECODING_ERROR if the matrix does not represent a QR that can be decoded
 *         MEMORY_ERROR in case of memory allocation error
 */
int find_qr_code(struct bit_matrix* matrix, struct bytebuffer* *code);


/**
 * Frees all the memory associated to the given list.
 */
void free_qr_code_match_list(struct qr_code_match_list* list);


#endif

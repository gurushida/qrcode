#include <stdio.h>
#include <stdlib.h>
#include "binarize.h"
#include "bitstream.h"
#include "bitstreamdecoder.h"
#include "blocks.h"
#include "codewords.h"
#include "codewordmask.h"
#include "finderpattern.h"
#include "finderpatterngroup.h"
#include "formatinformation.h"
#include "logs.h"
#include "qrcode.h"
#include "qrcodefinder.h"
#include "reedsolomon.h"
#include "rgbimage.h"
#include "versioninformation.h"


/**
 * Debug prints the matrix.
 */
static void print_matrix(struct bit_matrix* matrix) {
    info("%d x %d:\n", matrix->width, matrix->height);
    for (unsigned int y = 0 ; y < matrix->height ; y++) {
        for (unsigned int x = 0 ; x < matrix->width ; x++) {
            info("%c", is_black(matrix, x, y) ? '*' : ' ');
        }
        info("\n");
    }
}


int find_qr_codes(const char* png, struct qr_code_match_list* *match_list) {
    (*match_list) = NULL;

    // First, let's load the png image as an RGB image
    struct rgb_image* img;
    int res = load_rgb_image(png, &img);
    if (res == CANNOT_LOAD_IMAGE) {
        return res;
    }
    if (res == MEMORY_ERROR) {
        return res;
    }

    // Now let's convert the image into a black and white matrix. In
    // real life, a QR code may be scanned with shadows so that in the
    // worse case scenario, the same shade of gray could represent a
    // white module in some part of the image and a black module somewhere
    // else. To avoid such problems, the conversion to black and white is
    // done using some local luminance calculation rules
    struct bit_matrix* bm = binarize(img);
    free_rgb_image(img);
    if (bm == NULL) {
        return MEMORY_ERROR;
    }

    // 3 of the corners of a QR code have the same regular shape. They
    // are called finder patterns and they are meant to be used by
    // decoders to understand that there is a QR code to decoded
    // as well as to indicate the orientation/deformation of the code.
    // What we do now is look for these finder patterns. Depending on
    // how robust you want the QR code scanning to be, this is where
    // you would look for rotations and deformations. This implementation
    // does not try to do anything fancy and only looks for the finder
    // patterns with the assumption that they are kind of parallel to
    // the sides of the image
    struct finder_pattern_list* list;
    res = find_potential_centers(bm, 1, &list);
    if (res != SUCCESS) {
        if (res == DECODING_ERROR) {
            info("Could not find any finder pattern center\n");
        }
        free_bit_matrix(bm);
        return res;
    }

    // Now we have a list of all the finder patterns that may be
    // corners of a QR code. Since an image may contain multiple
    // QR codes or partial QR codes, we need to figure out which
    // triplets of finder patterns are candidates for being the
    // corners of a QR code. During this analysis, we also figure
    // out for each triplet X,Y,Z which finder pattern corresponds
    // to which corner. If we assume that the QR code is oriented
    // with its finder patterns A, B and C like this:
    //
    // B--------C
    // |
    // |
    // |
    // A
    //
    // this means figuring which finder pattern is A, B and C.
    struct finder_pattern_group_list* groups;
    res = find_groups(list, &groups);
    free_finder_pattern_list(list);
    if (res != SUCCESS) {
        info("Could not find any center group\n");
        free_bit_matrix(bm);
        return res;
    }

    // For each triplet of finder patterns, let's try to find a QR code and to analyze it
    struct finder_pattern_group_list* tmp = groups;
    int memory_error = 0;
    while (tmp != NULL && !memory_error) {
        struct qr_code* code;
        switch(get_qr_code(tmp->bottom_left, tmp->top_left, tmp->top_right, bm, &code)) {
            case MEMORY_ERROR: {
                memory_error = 1;
                break;
            }
            case DECODING_ERROR: {
                // Cannot decode the QR code ? Nothing to do
                break;
            }
            case SUCCESS: {
                // We have a QR code matrix, let's try to decode it
                info("Found a potential code ");
                print_matrix(code->modules);

                struct bytebuffer* message;
                res = find_qr_code(code->modules, &message);
                free_qr_code(code);

                if (res == MEMORY_ERROR) {
                    memory_error = 1;
                }
                else if (res == SUCCESS) {
                    // We have a match, let's add it to the result list
                    struct qr_code_match_list* match = (struct qr_code_match_list*)malloc(sizeof(struct qr_code_match_list));
                    if (match == NULL) {
                        memory_error = 1;
                        free_bytebuffer(message);
                    } else {
                        match->message = message;
                        match->bottom_left_x = code->bottom_left_x;
                        match->bottom_left_y = code->bottom_left_y;
                        match->top_left_x = code->top_left_x;
                        match->top_left_y = code->top_left_y;
                        match->top_right_x = code->top_right_x;
                        match->top_right_y = code->top_right_y;
                        match->bottom_right_x = code->bottom_right_x;
                        match->bottom_right_y = code->bottom_right_y;
                        match->next = (*match_list);
                        (*match_list) = match;
                    }
                }
                break;
            }
        }

        tmp = tmp->next;
    }

    free_bit_matrix(bm);
    free_finder_pattern_group_list(groups);
    if (memory_error) {
        free_qr_code_match_list(*match_list);
        (*match_list) = NULL;
        return MEMORY_ERROR;
    }

    return (*match_list) != NULL ? SUCCESS : DECODING_ERROR;
}


int find_qr_code(struct bit_matrix* matrix, struct bytebuffer* *code) {
    ErrorCorrectionLevel ec;
    uint8_t mask_pattern;
    int res;

    // First, let's get the format information which consists of the
    // error correction level used for the QR code and a code representing
    // the XOR masking pattern that was used when encoding the data to make sure
    // that the data modules looked random enough not to confuse QR code decoders
    // (like for instance all the data modules being of the same color)
    if (SUCCESS != (res = get_format_information(matrix, &ec, &mask_pattern))) {
        info("Cannot find format information\n");
        return res;
    }

    switch (ec) {
        case LOW: info("Error correction level = LOW\n"); break;
        case MEDIUM: info("Error correction level = MEDIUM\n"); break;
        case QUARTILE: info("Error correction level = QUARTILE\n"); break;
        case HIGH: info("Error correction level = HIGH\n"); break;
    }

    info("XOR mask pattern number = %d\n", mask_pattern);

    // Then let's get the version information N which indicates that the QR code
    // is a N x N module matrix
    uint8_t version;
    res = get_version_information(matrix, &version);
    if (res != SUCCESS) {
        info("Cannot find QR version\n");
        return res;
    }

    info("QR version = %d\n", version);

    // Let's create the mask that will indicate which modules
    // are data modules (as opposed to non-data modules like
    // the ones used to encode format and version for instance)
    struct bit_matrix* codeword_mask;
    res = get_codeword_mask(matrix->width, &codeword_mask);
    if (res != SUCCESS) {
        return res;
    }

    // Now that we have the XOR masking pattern and the mask that
    // indicates which modules are data modules, we can scan
    // these data modules in the snake-fashion used by QR codes
    // and XOR them with the masking pattern to get the bitstream
    // representing the data to be decoded
    u_int8_t* codewords;
    int n_codewords = get_codewords(matrix, codeword_mask, mask_pattern, &codewords);
    free_bit_matrix(codeword_mask);
    if (n_codewords < 0) {
        if (n_codewords == DECODING_ERROR) {
            fprintf(stderr, "Illegal arguments passed to get_codewords()\n");
            exit(1);
        }
        return n_codewords;
    }

    // For error correction purposes, the original data is split
    // in blocks and for each block some error correction bytes
    // are produced. Moreover, the data blocks and the error
    // correction blocks are interleaved to be more tolerant
    // to physical alterations of QR codes like having a bit teared
    // off, some part covered by dirt or even intentional
    // alterations like adding a logo onto a code for artistic/branding
    // purposes and relying on the error correction to make it decodable
    // anyway. So the next step is to de-interleave the bytes into
    // proper data+error correction blocks
    struct blocks* blocks;
    res = get_blocks(codewords, version, ec, &blocks);
    free(codewords);
    if (res != SUCCESS) {
        if (res == DECODING_ERROR) {
            fprintf(stderr, "Illegal arguments passed to get_blocks()\n");
            exit(1);
        }
        return res;
    }

    // Now comes the error correction math magic. Each
    // data+error correction block will be checked for errors
    // and if there are not too many errors, we will be able to
    // fix them. Using this mechanism, we now try to extract
    // the original data that was encoded into each block
    // and re-assemble the bytes that were stored into the QR code
    struct bitstream* bitstream;
    res = get_message_bitstream(blocks, &bitstream);
    int n = blocks->n_blocks;
    free_blocks(blocks);
    if (res != SUCCESS) {
        if (res == DECODING_ERROR) {
            info("Failed to decode bistream blocks. The data may be too corrupted.\n", n);
        }
        return res;
    }

    // Now we have the bytes that were encoded into the QR code.
    // It is time to decode these bytes to figure out what
    // original message was encoded. Why, do you ask ? Well,
    // some QR codes may be used only to encode digits in which
    // case using one byte per digit would waste a lot of space.
    // Or the encoded message could be in Japanese which happens
    // to be the native language of the engineers who designed QR
    // codes and so having UTF-8 encoding may take more space
    // than a more ad hoc encoding like Shift JIS. Anyway, the idea
    // is to make it possible for QR code encoders to split the message
    // into segments that may be encoded differently, so that the
    // encoder is free to use the segments that minimize the amount
    // of bytes needed to encode the original message. So, the last
    // step is to take the error-free bytes and to extract from them
    // the original data
    u_int8_t* message;
    res = decode_bitstream(bitstream, version, &message);
    free_bitstream(bitstream);
    if (res < 0) {
        if (res == DECODING_ERROR) {
            info("Failed to decode message\n", n);
        }
        return res;
    }

    (*code) = (struct bytebuffer*)malloc(sizeof(struct bytebuffer));
    if ((*code) == NULL) {
        return MEMORY_ERROR;
    }
    (*code)->bytes = message;
    (*code)->n_bytes = res;


    if (contains_text_data(*code)) {
        info("Decoded text message:\n%s\n", (*code)->bytes);
    } else {
        info("Decoded binary message:\n");
        print_bytes(INFO, (*code)->bytes, (*code)->n_bytes);
    }
    info("\n");

    return SUCCESS;
}


void free_qr_code_match_list(struct qr_code_match_list* list) {
    while (list != NULL) {
        struct qr_code_match_list* next = list->next;
        free_bytebuffer(list->message);
        free(list);
        list = next;
    }
}


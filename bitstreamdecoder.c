#include <stdlib.h>
#include "bitstream.h"
#include "bitstreamdecoder.h"
#include "bytebuffer.h"
#include "eci.h"

// Here are the possible mode values
#define TERMINATOR 0
#define NUMERIC 1
#define ALPHANUMERIC 2
#define STRUCTURED_APPEND 3
#define BYTE 4
#define FNC1_FIRST 5
#define ECI 7
#define KANJI 8
#define FNC1_SECOND 9


/**
 * Decodes count bytes from the given stream and adds
 * the corresponding data as utf8 in the given buffer.
 *
 * @return 1 on success
 *         0 on decoding failure
 *        -1 on memory allocation error
 */
static int decode_byte_segment(struct bitstream* stream, unsigned int count, EciMode eci_mode, struct bytebuffer* buffer) {
    if (8 * count > remaining_bits(stream)) {
        return 0;
    }
    for (unsigned int i = 0 ; i < count ; i++) {
        u_int8_t value = read_bits(stream, 8);

        // TODO: decode bytes using the ECI mode

        if (!write_byte(buffer, value)) {
            return -1;
        }
    }

    return 1;
}


/**
 * Decodes count alphanumeric characters from the given stream
 * and adds the corresponding data as utf8 in the given buffer.
 *
 * @return 1 on success
 *         0 on failure
 *        -1 on memory allocation error
 */
static int decode_alphanumeric_segment(struct bitstream* stream, unsigned int count, int fnc1_mode, struct bytebuffer* buffer) {

    return 0;
}


/**
 * The QR code specification says that the number of bits to be read
 * for a segment is not the same for all modes/versions so this functions
 * tells how many bits should be read for a given combination.
 */
static unsigned int get_character_count_bit(u_int8_t mode, unsigned int version) {
    switch(mode) {
        case NUMERIC: {
            if (version <= 9) {
                return 10;
            } else if (version <= 26) {
                return 12;
            } else {
                return 14;
            }
        }
        case ALPHANUMERIC: {
            if (version <= 9) {
                return 9;
            } else if (version <= 26) {
                return 11;
            } else {
                return 13;
            }
        }
        case BYTE: {
            if (version <= 9) {
                return 8;
            } else if (version <= 26) {
                return 16;
            } else {
                return 16;
            }
        }
        case KANJI: {
            if (version <= 9) {
                return 8;
            } else if (version <= 26) {
                return 10;
            } else {
                return 12;
            }
        }
        default: return -1;
    }
}


int decode_bitstream(struct bitstream* stream, unsigned int version, u_int8_t* *decoded) {
    if (version < 1 || version > 40) {
        return -2;
    }
    
    struct bytebuffer* buffer = new_bytebuffer();
    if (buffer == NULL) {
        return -1;
    }

    u_int8_t mode;
    int fnc1_mode = 0;
    EciMode eci_mode = ISO8859_1;
    do {
        if (remaining_bits(stream) < 4) {
            mode = TERMINATOR;
        } else {
            mode = read_bits(stream, 4);
        }

        printf("mode = %d\n", mode);
        switch(mode) {
            case TERMINATOR: break;
            case FNC1_FIRST:
            case FNC1_SECOND: {
                fnc1_mode = 1;
                break;
            }
            case STRUCTURED_APPEND: {
                // TODO
                return -2;
            }
            case ECI: {
                // This mode indicates that we need to read an ECI value
                // representing the new charset encoding to be used from now on
                int n = read_eci_designator(stream);
                if (n != -1) {
                    int new_eci_mode = get_eci_mode(n);
                    if (new_eci_mode != -1) {
                        eci_mode = new_eci_mode;
                        break;
                    }
                }
                free_bytebuffer(buffer);
                return -2;
            }
            case NUMERIC:
            case ALPHANUMERIC:
            case BYTE:
            case KANJI: {
                // Let's handle the non-special modes. They have in common
                // to expose the number of characters to read after the mode
                // in a way that depends on the mode/version combination
                u_int32_t count = read_bits(stream, get_character_count_bit(mode, version));
                printf("count %d\n", count);
                switch(mode) {
                    case NUMERIC: {
                        // TODO
                        return -2;
                    }
                    case ALPHANUMERIC: {
                        int res = decode_alphanumeric_segment(stream, count, fnc1_mode, buffer);
                        if (res == 0 || res == -1) {
                            free_bytebuffer(buffer);
                            return res == -1 ? -1 : -2;
                        }
                        break;
                    }
                    case BYTE: {
                        int res = decode_byte_segment(stream, count, eci_mode, buffer);
                        if (res == 0 || res == -1) {
                            free_bytebuffer(buffer);
                            return res == -1 ? -1 : -2;
                        }
                        break;
                    }
                    case KANJI: {
                        // TODO
                        return -2;
                    }
                }
                break;
            }
            default: {
                // Unknown mode
                return -2;
            }
        }
        printf("remaining bits %d\n", remaining_bits(stream));
    } while (mode != TERMINATOR);

    // Let's turn the buffer into a null-terminated string
    if (!write_byte(buffer, 0)) {
        free_bytebuffer(buffer);
        return -1;
    }
    unsigned int n = buffer->n_bytes;

    // Let's steal the byte array from the byte buffer
    *decoded = buffer->bytes;
    buffer->bytes = NULL;
    free_bytebuffer(buffer);

    return n;
}

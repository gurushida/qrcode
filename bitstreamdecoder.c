#include <stdlib.h>
#include "bitstream.h"
#include "bitstreamdecoder.h"
#include "bytebuffer.h"
#include "eci.h"
#include "gb18030.h"
#include "shiftjis.h"

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

#define ALPHANUMERIC_CHARS "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:"


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

    if (eci_mode == GB18030) {
        return decode_gb18030_segment(stream, count, buffer);
    }

    for (unsigned int i = 0 ; i < count ; i++) {
        u_int8_t value = read_bits(stream, 8);

        if (eci_mode == UTF8) {
            // If we have utf8, we can copy the raw bytes as is
            if (!write_byte(buffer, value)) {
                return -1;
            }
        } if (eci_mode == UnicodeBigUnmarked) {
            // For UTF16 Big Endian, we need 2 bytes
            if (remaining_bits(stream) < 8) {
                return 0;
            }
            // If we have utf8, we can copy the raw bytes as is
            if (!write_byte(buffer, value)) {
                return -1;
            }
            value = read_bits(stream, 8);
            if (!write_byte(buffer, value)) {
                return -1;
            }
        } else if (eci_mode == SJIS) {
            if (value <= 0x7F) {
                // This is a one byte ascii value
                if (!write_byte(buffer, value)) {
                    return -1;
                }
            } else {
                // We have a 2-byte value
                if (remaining_bits(stream) < 8) {
                    return -1;
                }
                u_int8_t value2 = read_bits(stream, 8);
                u_int32_t unicode = from_SJIS((value << 8) | value2);
                if (!write_unicode_as_utf8(buffer, unicode)) {
                    return -1;
                }
            }
        } else {
            // If we have a supported one byte charset, let's write
            // the utf8 representation of the character
            u_int32_t unicode;
            switch(eci_mode) {
                case ISO8859_1: unicode = from_iso8859_1(value); break;
                case ISO8859_2: unicode = from_iso8859_2(value); break;
                case ISO8859_3: unicode = from_iso8859_3(value); break;
                case ISO8859_4: unicode = from_iso8859_4(value); break;
                case ISO8859_5: unicode = from_iso8859_5(value); break;
                case ISO8859_6: unicode = from_iso8859_6(value); break;
                case ISO8859_7: unicode = from_iso8859_7(value); break;
                case ISO8859_8: unicode = from_iso8859_8(value); break;
                case ISO8859_9: unicode = from_iso8859_9(value); break;
                case ISO8859_10: unicode = from_iso8859_10(value); break;
                case ISO8859_11: unicode = from_iso8859_11(value); break;
                case ISO8859_13: unicode = from_iso8859_13(value); break;
                case ISO8859_14: unicode = from_iso8859_14(value); break;
                case ISO8859_15: unicode = from_iso8859_15(value); break;
                case ISO8859_16: unicode = from_iso8859_16(value); break;
                case Cp437: unicode = from_Cp437(value); break;
                case Cp1250: unicode = from_Cp1250(value); break;
                case Cp1251: unicode = from_Cp1251(value); break;
                case Cp1252: unicode = from_Cp1252(value); break;
                case Cp1256: unicode = from_Cp1256(value); break;
                case ASCII: unicode = from_ascii(value); break;
                default: return 0;
            }
            if (!write_unicode_as_utf8(buffer, unicode)) {
                return -1;
            }
        }
    }

    return 1;
}


void decode_percents_in_FNC1_mode(struct bytebuffer* buffer, unsigned int start) {
    unsigned int end_pos = buffer->n_bytes;
    unsigned int read_pos = start;
    unsigned int write_pos = start;

    while (read_pos < end_pos) {
        u_int8_t v = buffer->bytes[read_pos++];
        if (v != '%') {
            buffer->bytes[write_pos++] = v;
        } else {
            // We need to decode a '%'
            if (read_pos == end_pos) {
                // '%' at the end of the buffer; let's consider it a single '%'
                buffer->bytes[write_pos++] = 0x1D;
            } else {
                v = buffer->bytes[read_pos++];
                if (v == '%') {
                    // We have a '%%' sequence
                    buffer->bytes[write_pos++] = '%';
                } else {
                    // We have '%' followed by something else;
                    buffer->bytes[write_pos++] = 0x1D;
                    buffer->bytes[write_pos++] = v;
                }
            }
        }
    }
    // When we are done, the write position is the new buffer size
    buffer->n_bytes = write_pos;
}


/**
 * Alphanumeric characters are encoded as 11-bit pairs plus maybe a
 * single 6-bit value at the end. Each value is converted into a character
 * with the ALPHANUMERIC_CHARS lookup table.
 *
 * Decodes count alphanumeric characters from the given stream
 * and adds the corresponding data as utf8 in the given buffer.
 *
 * @return 1 on success
 *         0 on failure
 *        -1 on memory allocation error
 */
static int decode_alphanumeric_segment(struct bitstream* stream, unsigned int count, int fnc1_mode, struct bytebuffer* buffer) {
    int start = buffer->n_bytes;

    while (count > 1) {
        if (remaining_bits(stream) < 11) {
            return 0;
        }

        u_int32_t value = read_bits(stream, 11);
        if ((value / 45) >= 45) {
            return 0;
        }
        u_int8_t first = ALPHANUMERIC_CHARS[value / 45];
        u_int8_t second = ALPHANUMERIC_CHARS[value % 45];
        if (-1 == write_byte(buffer, first)
            || -1 == write_byte(buffer, second)) {
                return -1;
        }
        count -= 2;
    }

    if (count == 1) {
        // There is a single character at the end of the segment
        if (remaining_bits(stream) < 6) {
            return 0;
        }
        u_int32_t value = read_bits(stream, 6);
        if (value >= 45) {
            return 0;
        }
        if (-1 == write_byte(buffer, ALPHANUMERIC_CHARS[value])) {
            return -1;
        }
    }

    if (fnc1_mode) {
        // If we are in FNC1 mode, there is more to do
        decode_percents_in_FNC1_mode(buffer, start);
    }

    return 1;
}


/**
 * Numeric characters are encoded as 10-bit triplets plus maybe a
 * 7-bit pair or a 4-bit single value at the end.
 * converted into a characterwith the ALPHANUMERIC_CHARS lookup table.
 *
 * Decodes count numeric characters from the given stream
 * and adds the corresponding data as utf8 in the given buffer.
 *
 * @return 1 on success
 *         0 on failure
 *        -1 on memory allocation error
 */
static int decode_numeric_segment(struct bitstream* stream, unsigned int count, struct bytebuffer* buffer) {
    while (count >= 3) {
        if (remaining_bits(stream) < 10) {
            return 0;
        }

        u_int32_t value = read_bits(stream, 10);
        if (value >= 1000) {
            return 0;
        }
        u_int8_t first = '0' + (value / 100);
        u_int8_t second = '0' + ((value / 10) % 10);
        u_int8_t third = '0' + (value % 10);
        if (-1 == write_byte(buffer, first)
            || -1 == write_byte(buffer, second)
            || -1 == write_byte(buffer, third)) {
                return -1;
        }
        count -= 3;
    }

    if (count == 1) {
        // There is a single character at the end of the segment
        if (remaining_bits(stream) < 4) {
            return 0;
        }
        u_int32_t value = read_bits(stream, 4);
        if (value >= 10) {
            return 0;
        }

        if (-1 == write_byte(buffer, '0' + value)) {
            return -1;
        }
    } else if (count == 2) {
        // There are 2 characters at the end
        if (remaining_bits(stream) < 7) {
            return 0;
        }
        u_int32_t value = read_bits(stream, 7);
        if (value >= 100) {
            return 0;
        }
        u_int8_t first = '0' + (value / 10);
        u_int8_t second = '0' + (value % 10);

        if (-1 == write_byte(buffer, first)
            || -1 == write_byte(buffer, second)) {
                return -1;
        }
    }

    return 1;
}


/**
 * Kanji characters are 2-bytes Shift JIS X 0208 values that are either
 * in the range from 0x8140 to 0x9FFC or in the range 0xE040 to 0xEBBF.
 *
 * Each Kanji character WXYZ is encoded as a 13-bit value as
 * follows:
 *
 * 1) Subtract 0x8140 or 0xC140 to obtain a new value ABCD.
 *    The value to subtract depend on the range the value belongs to.
 * 2) Use (AB * 0xC0 + CD) as a 13-bit value
 *
 * Decodes count Kanji characters from the given stream
 * and adds the corresponding data as utf8 in the given buffer.
 *
 * @return 1 on success
 *         0 on failure
 *        -1 on memory allocation error
 */
static int decode_kanji_segment(struct bitstream* stream, unsigned int count, struct bytebuffer* buffer) {
    if (count * 13 > remaining_bits(stream)) {
        return 0;
    }
    while (count > 0) {
        // Get the raw 13-bit value
        u_int32_t value = read_bits(stream, 13);
        // Unpack it as a 2-byte value
        value = ((value / 0xC0) << 8) | (value % 0xC0);
        // Adjust the value to its original range
        if (value < 0x1F00) {
            value += 0x8140;
        } else {
            value += 0xC140;
        }
        count--;
        uint32_t unicode = from_SJIS(value);
        int res = write_unicode_as_utf8(buffer, unicode);
        if (res == 0) {
            return -1;
        }
        if (res == -1) {
            return 0;
        }
    }
    return 1;
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

        switch(mode) {
            case TERMINATOR: break;
            case FNC1_FIRST:
            case FNC1_SECOND: {
                // These modes act as a flag to turn on a special behaviour
                // when decoding alphanumeric segments. Once on, the flag
                // cannot be turned off
                fnc1_mode = 1;
                break;
            }
            case STRUCTURED_APPEND: {
                // The structured append mode is meant to indicate that
                // a QR code is part of a series of QR codes that should be
                // put together to reassemble the full original message.
                // We will ignore this feature, so let's just skip the 16
                // bits describing the symbol sequence and the parity data
                if (remaining_bits(stream) < 16) {
                    free_bytebuffer(buffer);
                    return -2;
                }
                read_bits(stream, 16);
                break;
            }
            case ECI: {
                // This mode indicates that we need to read an ECI value
                // representing the new charset encoding to be used from now on
                int n = read_eci_designator(stream);
                if (n != -1) {
                    int new_eci_mode = get_eci_mode(n);
                    if (new_eci_mode != -1) {
                        if (can_decode(new_eci_mode)) {
                            eci_mode = new_eci_mode;
                            break;
                        } else {
                            printf("Unsupported eci mode %d\n", new_eci_mode);
                        }
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
                switch(mode) {
                    case NUMERIC: {
                        int res = decode_numeric_segment(stream, count, buffer);
                        if (res == 0 || res == -1) {
                            free_bytebuffer(buffer);
                            return res == -1 ? -1 : -2;
                        }
                        break;
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
                        int res = decode_kanji_segment(stream, count, buffer);
                        if (res == 0 || res == -1) {
                            free_bytebuffer(buffer);
                            return res == -1 ? -1 : -2;
                        }
                        break;
                    }
                }
                break;
            }
            default: {
                // Unknown mode
                free_bytebuffer(buffer);
                return -2;
            }
        }
    } while (mode != TERMINATOR);

    unsigned int n = buffer->n_bytes;

    // Let's turn the buffer into a null-terminated string
    if (!write_byte(buffer, 0)) {
        free_bytebuffer(buffer);
        return -1;
    }

    // Let's steal the byte array from the byte buffer
    *decoded = buffer->bytes;
    buffer->bytes = NULL;
    free_bytebuffer(buffer);

    return n;
}

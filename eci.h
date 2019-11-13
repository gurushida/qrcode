#ifndef _ECI_H
#define _ECI_H

#include "bitstream.h"
#include "errors.h"


/**
 * This enum lists all the possible ECI modes that
 * can be used to encode text in a segment inside a
 * QR code's bitstream.
 */
typedef enum {
    Cp437,
    ISO8859_1,
    ISO8859_2,
    ISO8859_3,
    ISO8859_4,
    ISO8859_5,
    ISO8859_6,
    ISO8859_7,
    ISO8859_8,
    ISO8859_9,
    ISO8859_10,
    ISO8859_11,
    // Not a mistake that there is no ISO-8859-12
    ISO8859_13,
    ISO8859_14,
    ISO8859_15,
    ISO8859_16,
    SJIS,
    Cp1250,
    Cp1251,
    Cp1252,
    Cp1256,
    UnicodeBigUnmarked,
    UTF8,
    ASCII,
    Big5,
    GB18030,
    EUC_KR
} EciMode;


/**
 * Given an integer representing an ECI mode, returns
 * the ECI mode or DECODING_ERROR if no ECI mode is found.
 */
int get_eci_mode(unsigned int value);


/**
 * The value representing the ECI mode can be encoded in 1, 2 or 3 bytes
 * like this:
 *
 * 1 byte   0bbbbbbb
 * 2 bytes  10bbbbbb bbbbbbbb
 * 3 bytes  110bbbbb bbbbbbbb bbbbbbbb
 *
 * This function tries to read such a value from the given stream.
 *
 * @param stream The bitstream to read from
 * @return n >= 0 in case of success
 *        DECODING_ERROR if the stream does not contain enough bits or if the
 *                       first byte we read starts with 111
 */
int read_eci_designator(struct bitstream* stream);


/**
 * Returns the unicode value corresponding to the given byte.
 */
u_int32_t from_iso8859_1(u_int8_t c);
u_int32_t from_iso8859_2(u_int8_t c);
u_int32_t from_iso8859_3(u_int8_t c);
u_int32_t from_iso8859_4(u_int8_t c);
u_int32_t from_iso8859_5(u_int8_t c);
u_int32_t from_iso8859_6(u_int8_t c);
u_int32_t from_iso8859_7(u_int8_t c);
u_int32_t from_iso8859_8(u_int8_t c);
u_int32_t from_iso8859_9(u_int8_t c);
u_int32_t from_iso8859_10(u_int8_t c);
u_int32_t from_iso8859_11(u_int8_t c);
u_int32_t from_iso8859_13(u_int8_t c);
u_int32_t from_iso8859_14(u_int8_t c);
u_int32_t from_iso8859_15(u_int8_t c);
u_int32_t from_iso8859_16(u_int8_t c);
u_int32_t from_Cp437(u_int8_t c);
u_int32_t from_Cp1250(u_int8_t c);
u_int32_t from_Cp1251(u_int8_t c);
u_int32_t from_Cp1252(u_int8_t c);
u_int32_t from_Cp1256(u_int8_t c);
u_int32_t from_ascii(u_int8_t c);

#endif

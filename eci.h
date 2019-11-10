#ifndef _ECI_H
#define _ECI_H

#include "bitstream.h"

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
    ISO8859_12,
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
 * the ECI mode or -1 if no ECI mode is found.
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
 * @return n >=0 in case of success
 *        -1 if the stream does not contain enough bits or if the
 *        first byte we read starts with 111
 */
int read_eci_designator(struct bitstream* stream);

#endif

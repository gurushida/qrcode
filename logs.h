#ifndef _LOGS_H
#define _LOGS_H

typedef enum {
    // Does not print anything on stderr
    NO_LOGS = 0,

    // Only prints hard errors like memory allocation issues
    // and failures to loade images
    ERRORS = 1,

    // Prints errors as well as info messages about what is
    // found during the QR code analyis
    INFO = 2,

    // Same as INFO with extra logging for some who wants
    // juicy details about how things work, like error correction
    GORY = 3
} LogLevel;


/**
 * @param level The log level to use. Will use ERRORS if
 *              the given value is not a valid LogLevel value
 */
void set_log_level(LogLevel level);


/**
 * Emits a debug log message to stderr, if the log level allows it.
 */
void info(const char* fmt, ...);


/**
 * Emits an error message to stderr, if the log level allows it.
 */
void error(const char* fmt, ...);


/**
 * Emits an advanced log that is likely to have, if the log level allows it.
 */
void gory(const char* fmt, ...);


/**
 * Sames as info(), error(), gory() based on the
 * given log level. Defaults to ERRORS if the given value
 * is not a valid log level.
 */
void print_log(LogLevel level, const char* fmt, ...);


/**
 * Prints the given bytes in hexdecimal, 32 bytes per line.
 */
void print_bytes(LogLevel level, u_int8_t* bytes, unsigned int n_bytes);


#endif

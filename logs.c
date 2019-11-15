#include <stdarg.h>
#include <stdio.h>
#include "logs.h"


static LogLevel LOG_LEVEL = ERRORS;


void set_log_level(LogLevel level) {
    switch(level) {
        case NO_LOGS:
        case ERRORS:
        case INFO:
        case GORY: LOG_LEVEL = level; break;
        default: LOG_LEVEL = ERRORS; break;
    }
}


void info(const char* fmt, ...) {
    if (LOG_LEVEL < INFO) return;

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}


void error(const char* fmt, ...) {
    if (LOG_LEVEL < ERRORS) return;

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}


void gory(const char* fmt, ...) {
    if (LOG_LEVEL < GORY) return;

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}


void print_log(LogLevel level_to_use, const char* fmt, ...) {
    switch (level_to_use) {
        case NO_LOGS: return;
        case INFO: if (LOG_LEVEL < INFO) return; break;
        case GORY: if (LOG_LEVEL < GORY) return; break;
        default:  if (LOG_LEVEL < ERRORS) return; break;
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}


void print_bytes(LogLevel level, u_int8_t* bytes, unsigned int n_bytes) {
    for (unsigned int i = 0 ; i < n_bytes ; i++) {
        if (i > 0 && (i % 32) == 0) {
            print_log(level, "\n");
        }
        print_log(level, "%02x ", bytes[i]);
    }
    print_log(level, "\n");
}


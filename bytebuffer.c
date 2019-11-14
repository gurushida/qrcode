#include <stdlib.h>
#include "bytebuffer.h"

struct bytebuffer* new_bytebuffer() {
    struct bytebuffer* b = (struct bytebuffer*)malloc(sizeof(struct bytebuffer));
    if (b == NULL) {
        return NULL;
    }
    b->bytes = (u_int8_t*)malloc(32);
    if (b->bytes == NULL) {
        free(b);
        return NULL;
    }
    b->capacity = 32;
    b->n_bytes = 0;

    return b;
}


void free_bytebuffer(struct bytebuffer* buffer) {
    free(buffer->bytes);
    free(buffer);
}


int write_byte(struct bytebuffer* buffer, uint8_t value) {
    if (buffer->n_bytes == buffer->capacity) {
        u_int8_t* tmp = (u_int8_t*)realloc(buffer->bytes, 2 * buffer->capacity);
        if (tmp == NULL) {
            return MEMORY_ERROR;
        }
        buffer->bytes = tmp;
        buffer->capacity = 2 * buffer->capacity;
    }

    buffer->bytes[buffer->n_bytes++] = value;
    return SUCCESS;
}


int write_unicode_as_utf8(struct bytebuffer* buffer, u_int32_t value) {
    if (value <= 0x7F) {
        return write_byte(buffer, value);
    }

    if (value <= 0x7FF) {
        u_int8_t a = (128 + 64) | (value >> 6);
        u_int8_t b = 128 | (value & 63);
        return write_byte(buffer, a) && write_byte(buffer, b);
    }

    if (value <= 0xFFFF) {
        u_int8_t a = (128 + 64 + 32) | (value >> 12);
        u_int8_t b = 128 | ((value >> 6) & 63);
        u_int8_t c = 128 | (value & 63);
        return write_byte(buffer, a) && write_byte(buffer, b) && write_byte(buffer, c);
    }

    if (value <= 0x10FFFF) {
        u_int8_t a = (128 + 64 + 32 + 16) | (value >> 18);
        u_int8_t b = 128 | ((value >> 12) & 63);
        u_int8_t c = 128 | ((value >> 6) & 63);
        u_int8_t d = 128 | (value & 63);
        return write_byte(buffer, a) && write_byte(buffer, b) && write_byte(buffer, c) && write_byte(buffer, d);
    }
    return DECODING_ERROR;
}


int contains_binary_data(struct bytebuffer* buffer) {
    for (unsigned int i = 0 ; i < buffer->n_bytes ; i++) {
        u_int8_t c = buffer->bytes[i];
        if (c <= 0x1F && c != '\t' && c != '\r' && c != '\n') {
            return 1;
        }
    }
    return 0;
}


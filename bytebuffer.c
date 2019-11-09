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
            return 0;
        }
        buffer->bytes = tmp;
        buffer->capacity = 2 * buffer->capacity;
    }

    buffer->bytes[buffer->n_bytes++] = value;
    return 1;
}

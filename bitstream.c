#include <stdlib.h>
#include "bitstream.h"

struct bitstream* new_bitstream(unsigned int n_bytes) {
    struct bitstream* s = (struct bitstream*)calloc(1, sizeof(struct bitstream));
    if (s == NULL) {
        return NULL;
    }
    s->n_bytes = n_bytes;
    s->bytes = (u_int8_t*)calloc(n_bytes, 1);
    if (s->bytes == NULL) {
        free(s);
        return NULL;
    }

    return s;
}


void free_bitstream(struct bitstream* stream) {
    free(stream->bytes);
    free(stream);
}


unsigned int remaining_bits(struct bitstream* stream) {
    return 8 * (stream->n_bytes - stream->current_byte_pos) - stream->n_read_bits;
}


u_int32_t read_bits(struct bitstream* stream, unsigned int n) {
    if (n < 1 || n > 32 || n > remaining_bits) {
        return 0;
    }
    u_int32_t result = 0;

    while (n > 0) {
        unsigned available_bits_in_current_byte = 8 - stream->n_read_bits;
        unsigned int n_to_read = n <= available_bits_in_current_byte ? n : available_bits_in_current_byte;
        u_int8_t current_byte = stream->bytes[stream->current_byte_pos];

        // We want to get the AB bits in a byte XXABXXXX, so let's start by nullifying
        // the upper bits to get 00ABXXXX with a left shift followed by a right shift 
        u_int8_t bits = ((current_byte << stream->n_read_bits) & 0xFF) >> stream->n_read_bits;
        // Now let's right shift again to get 000000AB
        bits = bits >> (8 - stream->n_read_bits - n_to_read);
        // And finally compose this with the current result
        result = (result << n_to_read) | bits;

        // Let's update the read cursor
        stream->n_read_bits += n_to_read;
        if (stream->n_read_bits == 8 && stream->current_byte_pos != (stream->n_bytes - 1)) {
            stream->current_byte_pos++;
            stream->n_read_bits = 0;
        }


        n -= n_to_read;
    }

    return result;
}


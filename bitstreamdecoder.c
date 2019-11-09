
#include "bitstream.h"
#include "bitstreamdecoder.h"

int decode_bitstream(struct bitstream* stream, unsigned int version, u_int8_t* *decoded) {
    if (version < 1 || version > 40) {
        return 0;
    }
    
    // TODO

    return 0;
}


#include "bitstreamdecoder.h"

int decode_bitstream(u_int8_t* bytes, unsigned int n_bytes, unsigned int version, u_int8_t* *decoded) {
    if (version < 1 || version > 40) {
        return 0;
    }

    // TODO
    
    return 1;
}

#include <stdlib.h>
#include "blocks.h"



struct blocks* get_blocks(u_int8_t* codewords, int version, ErrorCorrectionLevel ec_level) {
    // TODO
    return NULL;
}


void free_blocks(struct blocks* blocks) {
    for (unsigned int i = 0 ; i < blocks->n_blocks ; i++) {
        free(blocks->block[i].data_codewords);
        free(blocks->block[i].error_correction_codewords);
    }
    free(blocks->block);
    free(blocks);
}

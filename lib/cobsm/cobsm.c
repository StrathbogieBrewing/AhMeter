#include "cobsm.h"

uint8_t cobsm_encode(uint8_t buffer[], uint8_t size) {
    const uint8_t nul_index = size + 1;
    uint8_t zero_offset = size;
    buffer[nul_index] = 0;
    buffer[zero_offset] = nul_index;
    uint8_t buffer_index = size - 1;

    do {
        if (buffer[buffer_index] == 0U) {
            buffer[zero_offset] = size - buffer_index;
            zero_offset = buffer_index;
            buffer[zero_offset] = nul_index;
        }
    } while (buffer_index--);

    return size + 2;
}

uint8_t cobsm_decode(uint8_t buffer[], uint8_t size) {
    const uint8_t nul_index = size - 1;
    uint8_t zero_offset = 0;
    uint8_t zero_index = 0;

    while (zero_index != nul_index - zero_offset - 1) {
        zero_index = nul_index - zero_offset - 1;
        if(zero_index >= nul_index){
            return 0;
        }
        if (buffer[zero_index] == nul_index) {
            buffer[zero_index] = 0;
            break;
        } else {
            zero_offset = buffer[zero_index];
            buffer[zero_index] = 0;
        }
    }
    
    return size - 2;
}
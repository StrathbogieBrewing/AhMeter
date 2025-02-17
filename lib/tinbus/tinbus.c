#include "tinbus.h"
#include "crc8.h"

bool tinbus_build_frame(buffer_t *frame, uint16_t id, buffer_t *data) {

    if (data->length > TINBUS_MAX_DATA_LENGTH) {
        return false;
    }
    if (frame->max_length < TINBUS_FRAME_LENGTH(data->length)) {
        return false;
    }
    frame->length = TINBUS_FRAME_LENGTH(data->length);

    id &= 0x0FFF;
    uint8_t byte = (id >> 4U);
    uint8_t crc = crc8(0, byte);
    frame->data[0] = byte;

    byte = (id << 4U) | data->length;
    crc = crc8(crc, byte);
    frame->data[1] = byte;

    for (uint8_t index = 0; index < data->length; index++) {
        byte = data->data[index];
        crc = crc8(crc, byte);
        frame->data[TINBUS_HEADER_LENGTH + index] = byte;
    }
    frame->data[TINBUS_HEADER_LENGTH + data->length] = crc;

    return true;
}

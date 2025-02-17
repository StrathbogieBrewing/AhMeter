#ifndef TINBUS_H
#define TINBUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "buffer.h"

#define TINBUS_HEADER_LENGTH 2
#define TINBUS_MAX_DATA_LENGTH 15
#define TINBUS_CRC_LENGTH 1
#define TINBUS_MAX_FRAME_LENGTH (TINBUS_HEADER_LENGTH + TINBUS_MAX_DATA_LENGTH + TINBUS_CRC_LENGTH)

#define TINBUS_FRAME_LENGTH(data_length) (TINBUS_HEADER_LENGTH + data_length + TINBUS_CRC_LENGTH)

bool tinbus_build_frame(buffer_t *frame, uint16_t id, buffer_t *data);

#ifdef __cplusplus
}
#endif

#endif /* TINBUS_H */

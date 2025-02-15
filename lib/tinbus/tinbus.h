#ifndef TINBUS_H
#define TINBUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    TINBUS_PRIORITY_HIGH = 0,
    TINBUS_PRIORITY_MEDIUM,
    TINBUS_PRIORITY_LOW,
} tinbus_priority_t;

typedef enum {
    TINBUS_DATA_FRAME = 0,
    TINBUS_RTR_FRAME,
} tinbus_frame_t;

uint32_t get_tinbus_ext_id(uint32_t id, tinbus_priority_t priority, tinbus_frame_t frame_type);

#ifdef __cplusplus
}
#endif

#endif /* TINBUS_H */

#include "tinbus.h"

#define TINBUS_BIT_SSR 20UL
#define TINBUS_BIT_IDE 19UL
#define TINBUS_BIT_RTR 0UL

uint32_t get_tinbus_ext_id(uint32_t id, tinbus_priority_t priority, tinbus_frame_t frame_type) {
    uint32_t msg_id = (1UL << TINBUS_BIT_SSR) | (1UL << TINBUS_BIT_IDE);
    msg_id |= (uint32_t)frame_type;
    msg_id |= (uint32_t)priority << 30UL;
    msg_id |= (id & 0x0003FFFF) << 1UL;
    msg_id |= (id & 0x1FFC0000) << 3UL;
    return msg_id;
}
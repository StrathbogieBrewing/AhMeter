#ifndef CRC8_H
#define CRC8_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint8_t crc8(uint8_t crc, uint8_t data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CRC8_H
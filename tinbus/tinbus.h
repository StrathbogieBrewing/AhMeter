
#ifndef TINBUS_H
#define TINBUS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum {
    TINBUS_RX_NO_DATA = 0,
    TINBUS_RX_DATA_READY,
    TINBUS_RX_FRAME_READY,
    TINBUS_RX_ERROR,
} tinbus_rx_status_t;

typedef struct tinbus_rx_t {
    uint8_t data;
    tinbus_rx_status_t status;
} tinbus_rx_t;

void tinbus_init(void);
tinbus_rx_t tinbus_rx(void);

#ifdef __cplusplus
}
#endif

#endif /* TINBUS_H */




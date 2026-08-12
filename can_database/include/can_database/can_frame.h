#pragma once
#include <stdint.h>

typedef enum {
    CAN_BUS_POWERTRAIN = 0,
    CAN_BUS_SENSOR,
    CAN_BUS_DASHBOARD,
    CAN_BUS_COUNT
} CanBusId_t;

typedef struct {
    uint32_t id;
    uint8_t  data[8];
    uint8_t  dlc;
} CanFrame_t;

typedef struct {
    void (*send)(const CanFrame_t *frame);
} CanDb_BusDriver_t;

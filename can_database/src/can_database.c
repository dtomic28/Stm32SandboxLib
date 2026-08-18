#include "can_database/can_database.h"
#include "can_database_internal.h"
#include <stddef.h>

/* --- Bus registry --- */

typedef struct {
    CanDb_BusDriver_t driver;
    uint8_t           valid;
} BusEntry_t;

static BusEntry_t bus_registry[CANDB_MAX_BUSES];

void CanDb_RegisterBus(CanBusId_t bus, const CanDb_BusDriver_t *driver) {
    if (bus < CAN_BUS_COUNT) {
        bus_registry[bus].driver = *driver;
        bus_registry[bus].valid  = 1;
    }
}

void CanDb_Send(CanBusId_t bus, const CanFrame_t *frame) {
    if (bus < CAN_BUS_COUNT && bus_registry[bus].valid && bus_registry[bus].driver.send) {
        bus_registry[bus].driver.send(frame);
    }
}

/* --- Subscriber registry --- */

typedef struct {
    uint32_t id;
    void (*callback)(const CanFrame_t *);
} SubscriberEntry_t;

static SubscriberEntry_t subscriber_registry[CANDB_MAX_HANDLERS];
static uint8_t           subscriber_count = 0;

void CanDb_Subscribe(uint32_t id, void (*callback)(const CanFrame_t *)) {
    if (subscriber_count < CANDB_MAX_HANDLERS) {
        subscriber_registry[subscriber_count].id       = id;
        subscriber_registry[subscriber_count].callback = callback;
        subscriber_count++;
    }
}

void CanDb_OnRawFrameReceived(CanBusId_t bus, const CanFrame_t *frame) {
    (void)bus;
    for (uint8_t i = 0; i < subscriber_count; i++) {
        if (subscriber_registry[i].id == frame->id) {
            subscriber_registry[i].callback(frame);
            return;
        }
    }
}

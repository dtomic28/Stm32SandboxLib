#include "can_database/can_database.h"
#include "can_database/can_codec.h"
#include "can_database/can_ids.h"
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

static void CanDb_Send(CanBusId_t bus, CanFrame_t *frame) {
    if (bus < CAN_BUS_COUNT && bus_registry[bus].valid && bus_registry[bus].driver.send) {
        bus_registry[bus].driver.send(frame);
    }
}

/* --- Handler registry --- */

typedef struct {
    uint32_t id;
    void (*handler)(const CanFrame_t *);
} HandlerEntry_t;

static HandlerEntry_t handler_registry[CANDB_MAX_HANDLERS];
static uint8_t        handler_count = 0;

void CanDb_RegisterHandler(uint32_t id, void (*handler)(const CanFrame_t *)) {
    if (handler_count < CANDB_MAX_HANDLERS) {
        handler_registry[handler_count].id      = id;
        handler_registry[handler_count].handler = handler;
        handler_count++;
    }
}

void CanDb_Dispatch(CanBusId_t bus, const CanFrame_t *frame) {
    (void)bus;
    for (uint8_t i = 0; i < handler_count; i++) {
        if (handler_registry[i].id == frame->id) {
            handler_registry[i].handler(frame);
            return;
        }
    }
}

/* --- Publish --- */

void CanDb_WheelSpeeds_Publish(uint16_t fl, uint16_t fr, uint16_t rl, uint16_t rr) {
    CanFrame_t frame = { .id = CAN_ID_WHEEL_SPEEDS, .dlc = 8 };
    CanCodec_WheelSpeeds_Encode(fl, fr, rl, rr, frame.data);
    CanDb_Send(CAN_BUS_WHEEL_SPEEDS, &frame);
}

void CanDb_PowertrainCtrl_Publish(uint8_t throttle_pct, uint8_t brake_pct, uint16_t torque_limit, uint8_t flags) {
    CanFrame_t frame = { .id = CAN_ID_POWERTRAIN_CTRL, .dlc = 8 };
    CanCodec_PowertrainCtrl_Encode(throttle_pct, brake_pct, torque_limit, flags, frame.data);
    CanDb_Send(CAN_BUS_POWERTRAIN_CTRL, &frame);
}

void CanDb_BatteryStatus_Publish(uint16_t voltage_mv, int16_t current_ma, uint8_t soc_pct, int8_t temp_max, uint8_t fault_flags) {
    CanFrame_t frame = { .id = CAN_ID_BATTERY_STATUS, .dlc = 8 };
    CanCodec_BatteryStatus_Encode(voltage_mv, current_ma, soc_pct, temp_max, fault_flags, frame.data);
    CanDb_Send(CAN_BUS_BATTERY_STATUS, &frame);
}

void CanDb_MotorStatus_Publish(uint16_t rpm, int16_t torque_actual, int8_t temp_motor, int8_t temp_inverter, uint16_t fault_flags) {
    CanFrame_t frame = { .id = CAN_ID_MOTOR_STATUS, .dlc = 8 };
    CanCodec_MotorStatus_Encode(rpm, torque_actual, temp_motor, temp_inverter, fault_flags, frame.data);
    CanDb_Send(CAN_BUS_MOTOR_STATUS, &frame);
}

void CanDb_IMUAccel_Publish(int16_t accel_x, int16_t accel_y, int16_t accel_z) {
    CanFrame_t frame = { .id = CAN_ID_IMU_ACCEL, .dlc = 8 };
    CanCodec_IMUAccel_Encode(accel_x, accel_y, accel_z, frame.data);
    CanDb_Send(CAN_BUS_IMU_ACCEL, &frame);
}

void CanDb_DashboardCtrl_Publish(uint8_t button_flags, uint8_t rotary_1, uint8_t rotary_2, uint8_t led_flags) {
    CanFrame_t frame = { .id = CAN_ID_DASHBOARD_CTRL, .dlc = 8 };
    CanCodec_DashboardCtrl_Encode(button_flags, rotary_1, rotary_2, led_flags, frame.data);
    CanDb_Send(CAN_BUS_DASHBOARD_CTRL, &frame);
}

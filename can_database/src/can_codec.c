#include "can_database/can_codec.h"
#include <string.h>

/* All encode/decode functions follow the same pattern:
   encode → fill struct, memcpy into data[]
   decode → memcpy from data[], return struct by value  */

void CanCodec_WheelSpeeds_Encode(uint16_t fl, uint16_t fr, uint16_t rl, uint16_t rr, uint8_t data[8]) {
    CanMsg_WheelSpeeds_t msg = { .fl = fl, .fr = fr, .rl = rl, .rr = rr };
    memcpy(data, &msg, sizeof(msg));
}

CanMsg_WheelSpeeds_t CanCodec_WheelSpeeds_Decode(const uint8_t data[8]) {
    CanMsg_WheelSpeeds_t msg;
    memcpy(&msg, data, sizeof(msg));
    return msg;
}

void CanCodec_PowertrainCtrl_Encode(uint8_t throttle_pct, uint8_t brake_pct, uint16_t torque_limit, uint8_t flags, uint8_t data[8]) {
    CanMsg_PowertrainCtrl_t msg = {
        .throttle_pct = throttle_pct,
        .brake_pct    = brake_pct,
        .torque_limit = torque_limit,
        .flags        = flags,
        .reserved     = {0}
    };
    memcpy(data, &msg, sizeof(msg));
}

CanMsg_PowertrainCtrl_t CanCodec_PowertrainCtrl_Decode(const uint8_t data[8]) {
    CanMsg_PowertrainCtrl_t msg;
    memcpy(&msg, data, sizeof(msg));
    return msg;
}

void CanCodec_BatteryStatus_Encode(uint16_t voltage_mv, int16_t current_ma, uint8_t soc_pct, int8_t temp_max, uint8_t fault_flags, uint8_t data[8]) {
    CanMsg_BatteryStatus_t msg = {
        .voltage_mv  = voltage_mv,
        .current_ma  = current_ma,
        .soc_pct     = soc_pct,
        .temp_max    = temp_max,
        .fault_flags = fault_flags,
        .reserved    = 0
    };
    memcpy(data, &msg, sizeof(msg));
}

CanMsg_BatteryStatus_t CanCodec_BatteryStatus_Decode(const uint8_t data[8]) {
    CanMsg_BatteryStatus_t msg;
    memcpy(&msg, data, sizeof(msg));
    return msg;
}

void CanCodec_MotorStatus_Encode(uint16_t rpm, int16_t torque_actual, int8_t temp_motor, int8_t temp_inverter, uint16_t fault_flags, uint8_t data[8]) {
    CanMsg_MotorStatus_t msg = {
        .rpm           = rpm,
        .torque_actual = torque_actual,
        .temp_motor    = temp_motor,
        .temp_inverter = temp_inverter,
        .fault_flags   = fault_flags
    };
    memcpy(data, &msg, sizeof(msg));
}

CanMsg_MotorStatus_t CanCodec_MotorStatus_Decode(const uint8_t data[8]) {
    CanMsg_MotorStatus_t msg;
    memcpy(&msg, data, sizeof(msg));
    return msg;
}

void CanCodec_IMUAccel_Encode(int16_t accel_x, int16_t accel_y, int16_t accel_z, uint8_t data[8]) {
    CanMsg_IMUAccel_t msg = { .accel_x = accel_x, .accel_y = accel_y, .accel_z = accel_z, .reserved = 0 };
    memcpy(data, &msg, sizeof(msg));
}

CanMsg_IMUAccel_t CanCodec_IMUAccel_Decode(const uint8_t data[8]) {
    CanMsg_IMUAccel_t msg;
    memcpy(&msg, data, sizeof(msg));
    return msg;
}

void CanCodec_DashboardCtrl_Encode(uint8_t button_flags, uint8_t rotary_1, uint8_t rotary_2, uint8_t led_flags, uint8_t data[8]) {
    CanMsg_DashboardCtrl_t msg = {
        .button_flags = button_flags,
        .rotary_1     = rotary_1,
        .rotary_2     = rotary_2,
        .led_flags    = led_flags,
        .reserved     = {0}
    };
    memcpy(data, &msg, sizeof(msg));
}

CanMsg_DashboardCtrl_t CanCodec_DashboardCtrl_Decode(const uint8_t data[8]) {
    CanMsg_DashboardCtrl_t msg;
    memcpy(&msg, data, sizeof(msg));
    return msg;
}

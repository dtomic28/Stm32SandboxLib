#include <gtest/gtest.h>

extern "C" {
#include "can_database/can_codec.h"
}

/* --- WheelSpeeds --- */

TEST(CanCodec_WheelSpeeds, RoundTrip) {
    uint8_t data[8] = {0};
    CanCodec_WheelSpeeds_Encode(1000, 1001, 999, 1002, data);
    CanMsg_WheelSpeeds_t msg = CanCodec_WheelSpeeds_Decode(data);
    EXPECT_EQ(msg.fl, 1000);
    EXPECT_EQ(msg.fr, 1001);
    EXPECT_EQ(msg.rl, 999);
    EXPECT_EQ(msg.rr, 1002);
}

TEST(CanCodec_WheelSpeeds, ZeroValues) {
    uint8_t data[8] = {0xFF};
    CanCodec_WheelSpeeds_Encode(0, 0, 0, 0, data);
    CanMsg_WheelSpeeds_t msg = CanCodec_WheelSpeeds_Decode(data);
    EXPECT_EQ(msg.fl, 0);
    EXPECT_EQ(msg.fr, 0);
    EXPECT_EQ(msg.rl, 0);
    EXPECT_EQ(msg.rr, 0);
}

TEST(CanCodec_WheelSpeeds, MaxValues) {
    uint8_t data[8] = {0};
    CanCodec_WheelSpeeds_Encode(UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, data);
    CanMsg_WheelSpeeds_t msg = CanCodec_WheelSpeeds_Decode(data);
    EXPECT_EQ(msg.fl, UINT16_MAX);
    EXPECT_EQ(msg.fr, UINT16_MAX);
    EXPECT_EQ(msg.rl, UINT16_MAX);
    EXPECT_EQ(msg.rr, UINT16_MAX);
}

TEST(CanCodec_WheelSpeeds, ByteLayout) {
    uint8_t data[8] = {0};
    CanCodec_WheelSpeeds_Encode(0x0102, 0x0304, 0x0506, 0x0708, data);
    /* little-endian: fl=0x0102 → [0x02, 0x01] */
    EXPECT_EQ(data[0], 0x02); EXPECT_EQ(data[1], 0x01);
    EXPECT_EQ(data[2], 0x04); EXPECT_EQ(data[3], 0x03);
    EXPECT_EQ(data[4], 0x06); EXPECT_EQ(data[5], 0x05);
    EXPECT_EQ(data[6], 0x08); EXPECT_EQ(data[7], 0x07);
}

/* --- PowertrainCtrl --- */

TEST(CanCodec_PowertrainCtrl, RoundTrip) {
    uint8_t data[8] = {0};
    CanCodec_PowertrainCtrl_Encode(150, 20, 3000, 0x03, data);
    CanMsg_PowertrainCtrl_t msg = CanCodec_PowertrainCtrl_Decode(data);
    EXPECT_EQ(msg.throttle_pct, 150);
    EXPECT_EQ(msg.brake_pct,    20);
    EXPECT_EQ(msg.torque_limit, 3000);
    EXPECT_EQ(msg.flags,        0x03);
}

TEST(CanCodec_PowertrainCtrl, ReservedBytesZeroed) {
    uint8_t data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    CanCodec_PowertrainCtrl_Encode(0, 0, 0, 0, data);
    EXPECT_EQ(data[5], 0x00);
    EXPECT_EQ(data[6], 0x00);
    EXPECT_EQ(data[7], 0x00);
}

TEST(CanCodec_PowertrainCtrl, ZeroValues) {
    uint8_t data[8] = {0};
    CanCodec_PowertrainCtrl_Encode(0, 0, 0, 0, data);
    CanMsg_PowertrainCtrl_t msg = CanCodec_PowertrainCtrl_Decode(data);
    EXPECT_EQ(msg.throttle_pct, 0);
    EXPECT_EQ(msg.brake_pct,    0);
    EXPECT_EQ(msg.torque_limit, 0);
    EXPECT_EQ(msg.flags,        0);
}

/* --- BatteryStatus --- */

TEST(CanCodec_BatteryStatus, RoundTrip) {
    uint8_t data[8] = {0};
    CanCodec_BatteryStatus_Encode(48000, -5000, 85, 42, 0x01, data);
    CanMsg_BatteryStatus_t msg = CanCodec_BatteryStatus_Decode(data);
    EXPECT_EQ(msg.voltage_mv,  48000);
    EXPECT_EQ(msg.current_ma,  -5000);
    EXPECT_EQ(msg.soc_pct,     85);
    EXPECT_EQ(msg.temp_max,    42);
    EXPECT_EQ(msg.fault_flags, 0x01);
}

TEST(CanCodec_BatteryStatus, NegativeCurrent) {
    uint8_t data[8] = {0};
    CanCodec_BatteryStatus_Encode(0, INT16_MIN, 0, 0, 0, data);
    CanMsg_BatteryStatus_t msg = CanCodec_BatteryStatus_Decode(data);
    EXPECT_EQ(msg.current_ma, INT16_MIN);
}

TEST(CanCodec_BatteryStatus, NegativeTemp) {
    uint8_t data[8] = {0};
    CanCodec_BatteryStatus_Encode(0, 0, 0, -40, 0, data);
    CanMsg_BatteryStatus_t msg = CanCodec_BatteryStatus_Decode(data);
    EXPECT_EQ(msg.temp_max, -40);
}

/* --- MotorStatus --- */

TEST(CanCodec_MotorStatus, RoundTrip) {
    uint8_t data[8] = {0};
    CanCodec_MotorStatus_Encode(8000, 1500, 85, 72, 0x0000, data);
    CanMsg_MotorStatus_t msg = CanCodec_MotorStatus_Decode(data);
    EXPECT_EQ(msg.rpm,           8000);
    EXPECT_EQ(msg.torque_actual, 1500);
    EXPECT_EQ(msg.temp_motor,    85);
    EXPECT_EQ(msg.temp_inverter, 72);
    EXPECT_EQ(msg.fault_flags,   0x0000);
}

TEST(CanCodec_MotorStatus, NegativeTorque) {
    uint8_t data[8] = {0};
    CanCodec_MotorStatus_Encode(0, -500, 0, 0, 0, data);
    CanMsg_MotorStatus_t msg = CanCodec_MotorStatus_Decode(data);
    EXPECT_EQ(msg.torque_actual, -500);
}

TEST(CanCodec_MotorStatus, FaultFlags) {
    uint8_t data[8] = {0};
    CanCodec_MotorStatus_Encode(0, 0, 0, 0, 0xDEAD, data);
    CanMsg_MotorStatus_t msg = CanCodec_MotorStatus_Decode(data);
    EXPECT_EQ(msg.fault_flags, 0xDEAD);
}

/* --- IMUAccel --- */

TEST(CanCodec_IMUAccel, RoundTrip) {
    uint8_t data[8] = {0};
    CanCodec_IMUAccel_Encode(100, -200, 980, data);
    CanMsg_IMUAccel_t msg = CanCodec_IMUAccel_Decode(data);
    EXPECT_EQ(msg.accel_x, 100);
    EXPECT_EQ(msg.accel_y, -200);
    EXPECT_EQ(msg.accel_z, 980);
}

TEST(CanCodec_IMUAccel, AllAxesNegative) {
    uint8_t data[8] = {0};
    CanCodec_IMUAccel_Encode(-1000, -1000, -1000, data);
    CanMsg_IMUAccel_t msg = CanCodec_IMUAccel_Decode(data);
    EXPECT_EQ(msg.accel_x, -1000);
    EXPECT_EQ(msg.accel_y, -1000);
    EXPECT_EQ(msg.accel_z, -1000);
}

/* --- DashboardCtrl --- */

TEST(CanCodec_DashboardCtrl, RoundTrip) {
    uint8_t data[8] = {0};
    CanCodec_DashboardCtrl_Encode(0b00000101, 3, 7, 0b00001111, data);
    CanMsg_DashboardCtrl_t msg = CanCodec_DashboardCtrl_Decode(data);
    EXPECT_EQ(msg.button_flags, 0b00000101);
    EXPECT_EQ(msg.rotary_1,     3);
    EXPECT_EQ(msg.rotary_2,     7);
    EXPECT_EQ(msg.led_flags,    0b00001111);
}

TEST(CanCodec_DashboardCtrl, ReservedBytesZeroed) {
    uint8_t data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    CanCodec_DashboardCtrl_Encode(0, 0, 0, 0, data);
    EXPECT_EQ(data[4], 0x00);
    EXPECT_EQ(data[5], 0x00);
    EXPECT_EQ(data[6], 0x00);
    EXPECT_EQ(data[7], 0x00);
}

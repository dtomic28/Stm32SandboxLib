/**
 * @file example.c
 * @brief Full workflow example using a mock CAN port.
 *
 * Demonstrates how to wire up the CAN database on any platform by
 * implementing the CanDb_BusDriver_t interface. Here the "send"
 * function just prints to stdout — on a real target it calls the
 * HAL or RTOS queue instead.
 *
 * Build and run on any host machine (no hardware needed).
 */

#include <stdio.h>
#include <string.h>

#include "can_database/can_database.h"
#include "can_database/can_codec.h"
#include "can_database/can_ids.h"

/* ------------------------------------------------------------------ */
/* Mock CAN port                                                        */
/* ------------------------------------------------------------------ */

static void MockCAN_Send(const CanFrame_t *frame) {
    printf("  [TX] ID=0x%03X DLC=%d  DATA=", frame->id, frame->dlc);
    for (int i = 0; i < frame->dlc; i++) {
        printf("%02X ", frame->data[i]);
    }
    printf("\n");
}

static const CanDb_BusDriver_t mock_driver = { .send = MockCAN_Send };

/* ------------------------------------------------------------------ */
/* RX handlers                                                          */
/* ------------------------------------------------------------------ */

static void OnWheelSpeeds(const CanFrame_t *frame) {
    CanMsg_WheelSpeeds_t msg = CanCodec_WheelSpeeds_Decode(frame->data);
    printf("  [RX] WheelSpeeds  FL=%.1f  FR=%.1f  RL=%.1f  RR=%.1f RPM\n",
           msg.fl / 10.0f, msg.fr / 10.0f,
           msg.rl / 10.0f, msg.rr / 10.0f);
}

static void OnPowertrainCtrl(const CanFrame_t *frame) {
    CanMsg_PowertrainCtrl_t msg = CanCodec_PowertrainCtrl_Decode(frame->data);
    printf("  [RX] PowertrainCtrl  Throttle=%.1f%%  Brake=%.1f%%  TorqueLimit=%.1fNm  Flags=0x%02X\n",
           msg.throttle_pct / 2.0f, msg.brake_pct / 2.0f,
           msg.torque_limit / 10.0f, msg.flags);
}

static void OnBatteryStatus(const CanFrame_t *frame) {
    CanMsg_BatteryStatus_t msg = CanCodec_BatteryStatus_Decode(frame->data);
    printf("  [RX] BatteryStatus  %.3fV  %+.3fA  SOC=%d%%  TempMax=%d°C  Faults=0x%02X\n",
           msg.voltage_mv / 1000.0f, msg.current_ma / 1000.0f,
           msg.soc_pct, msg.temp_max, msg.fault_flags);
}

static void OnMotorStatus(const CanFrame_t *frame) {
    CanMsg_MotorStatus_t msg = CanCodec_MotorStatus_Decode(frame->data);
    printf("  [RX] MotorStatus  RPM=%d  Torque=%.1fNm  TempMotor=%d°C  TempInv=%d°C  Faults=0x%04X\n",
           msg.rpm, msg.torque_actual / 10.0f,
           msg.temp_motor, msg.temp_inverter, msg.fault_flags);
}

static void OnIMUAccel(const CanFrame_t *frame) {
    CanMsg_IMUAccel_t msg = CanCodec_IMUAccel_Decode(frame->data);
    printf("  [RX] IMUAccel  X=%dmg  Y=%dmg  Z=%dmg\n",
           msg.accel_x, msg.accel_y, msg.accel_z);
}

static void OnDashboardCtrl(const CanFrame_t *frame) {
    CanMsg_DashboardCtrl_t msg = CanCodec_DashboardCtrl_Decode(frame->data);
    printf("  [RX] DashboardCtrl  Buttons=0x%02X  Rot1=%d  Rot2=%d  LEDs=0x%02X\n",
           msg.button_flags, msg.rotary_1, msg.rotary_2, msg.led_flags);
}

/* ------------------------------------------------------------------ */
/* Main                                                                 */
/* ------------------------------------------------------------------ */

int main(void) {

    /* 1. Register buses — one mock driver handles all buses here */
    CanDb_RegisterBus(CAN_BUS_POWERTRAIN, &mock_driver);
    CanDb_RegisterBus(CAN_BUS_SENSOR,     &mock_driver);
    CanDb_RegisterBus(CAN_BUS_DASHBOARD,  &mock_driver);

    /* 2. Register RX handlers */
    CanDb_RegisterHandler(CAN_ID_WHEEL_SPEEDS,    OnWheelSpeeds);
    CanDb_RegisterHandler(CAN_ID_POWERTRAIN_CTRL, OnPowertrainCtrl);
    CanDb_RegisterHandler(CAN_ID_BATTERY_STATUS,  OnBatteryStatus);
    CanDb_RegisterHandler(CAN_ID_MOTOR_STATUS,    OnMotorStatus);
    CanDb_RegisterHandler(CAN_ID_IMU_ACCEL,       OnIMUAccel);
    CanDb_RegisterHandler(CAN_ID_DASHBOARD_CTRL,  OnDashboardCtrl);

    /* 3. Publish — internally encodes and calls MockCAN_Send */
    printf("=== TX (publish) ===\n");
    CanDb_WheelSpeeds_Publish(1200, 1205, 1198, 1201);
    CanDb_PowertrainCtrl_Publish(160, 0, 2500, 0x01);
    CanDb_BatteryStatus_Publish(48200, -12500, 83, 38, 0x00);
    CanDb_MotorStatus_Publish(7800, 1450, 82, 69, 0x0000);
    CanDb_IMUAccel_Publish(120, -30, 980);
    CanDb_DashboardCtrl_Publish(0x05, 3, 7, 0x0F);

    /* 4. Simulate RX — craft frames manually and dispatch */
    printf("\n=== RX (dispatch) ===\n");

    CanFrame_t frame;

    frame.id = CAN_ID_WHEEL_SPEEDS; frame.dlc = 8;
    CanCodec_WheelSpeeds_Encode(1200, 1205, 1198, 1201, frame.data);
    CanDb_Dispatch(CAN_BUS_SENSOR, &frame);

    frame.id = CAN_ID_POWERTRAIN_CTRL; frame.dlc = 8;
    CanCodec_PowertrainCtrl_Encode(160, 0, 2500, 0x01, frame.data);
    CanDb_Dispatch(CAN_BUS_POWERTRAIN, &frame);

    frame.id = CAN_ID_BATTERY_STATUS; frame.dlc = 8;
    CanCodec_BatteryStatus_Encode(48200, -12500, 83, 38, 0x00, frame.data);
    CanDb_Dispatch(CAN_BUS_POWERTRAIN, &frame);

    frame.id = CAN_ID_MOTOR_STATUS; frame.dlc = 8;
    CanCodec_MotorStatus_Encode(7800, 1450, 82, 69, 0x0000, frame.data);
    CanDb_Dispatch(CAN_BUS_POWERTRAIN, &frame);

    frame.id = CAN_ID_IMU_ACCEL; frame.dlc = 8;
    CanCodec_IMUAccel_Encode(120, -30, 980, frame.data);
    CanDb_Dispatch(CAN_BUS_SENSOR, &frame);

    frame.id = CAN_ID_DASHBOARD_CTRL; frame.dlc = 8;
    CanCodec_DashboardCtrl_Encode(0x05, 3, 7, 0x0F, frame.data);
    CanDb_Dispatch(CAN_BUS_DASHBOARD, &frame);

    /* 5. Unknown ID — silently dropped */
    printf("\n=== Unknown ID (should be dropped silently) ===\n");
    frame.id = 0xDEAD; frame.dlc = 8;
    memset(frame.data, 0xAB, 8);
    CanDb_Dispatch(CAN_BUS_SENSOR, &frame);
    printf("  (no output — correct)\n");

    return 0;
}

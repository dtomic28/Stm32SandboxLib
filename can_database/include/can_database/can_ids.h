#pragma once
#include "can_frame.h"

/* Wheel speeds — all four corners */
#define CAN_ID_WHEEL_SPEEDS         0x100U
#define CAN_BUS_WHEEL_SPEEDS        CAN_BUS_SENSOR

/* Driver inputs — throttle, brake, torque request */
#define CAN_ID_POWERTRAIN_CTRL      0x200U
#define CAN_BUS_POWERTRAIN_CTRL     CAN_BUS_POWERTRAIN

/* Accumulator status */
#define CAN_ID_BATTERY_STATUS       0x300U
#define CAN_BUS_BATTERY_STATUS      CAN_BUS_POWERTRAIN

/* Motor and inverter status */
#define CAN_ID_MOTOR_STATUS         0x400U
#define CAN_BUS_MOTOR_STATUS        CAN_BUS_POWERTRAIN

/* IMU linear acceleration */
#define CAN_ID_IMU_ACCEL            0x500U
#define CAN_BUS_IMU_ACCEL           CAN_BUS_SENSOR

/* Dashboard inputs and LED control */
#define CAN_ID_DASHBOARD_CTRL       0x600U
#define CAN_BUS_DASHBOARD_CTRL      CAN_BUS_DASHBOARD

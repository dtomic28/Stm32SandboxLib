#pragma once
#include <stdint.h>
#include <assert.h>

#pragma pack(push, 1)

/* Wheel speeds — 0.1 RPM resolution per corner */
typedef struct {
    uint16_t fl;  /* front left  [RPM * 10] */
    uint16_t fr;  /* front right [RPM * 10] */
    uint16_t rl;  /* rear left   [RPM * 10] */
    uint16_t rr;  /* rear right  [RPM * 10] */
} CanMsg_WheelSpeeds_t;

/* Driver inputs — throttle/brake in 0.5% steps, torque limit in 0.1 Nm */
typedef struct {
    uint8_t  throttle_pct;  /* [0-200] = 0-100% in 0.5% steps */
    uint8_t  brake_pct;     /* [0-200] = 0-100% in 0.5% steps */
    uint16_t torque_limit;  /* [Nm * 10] */
    uint8_t  flags;         /* bit0: regen en, bit1: launch ctrl en */
    uint8_t  reserved[3];
} CanMsg_PowertrainCtrl_t;

/* Accumulator status */
typedef struct {
    uint16_t voltage_mv;   /* [mV] */
    int16_t  current_ma;   /* [mA] signed — negative = charging */
    uint8_t  soc_pct;      /* [0-100] */
    int8_t   temp_max;     /* [°C] hottest cell */
    uint8_t  fault_flags;
    uint8_t  reserved;
} CanMsg_BatteryStatus_t;

/* Motor and inverter status */
typedef struct {
    uint16_t rpm;
    int16_t  torque_actual;  /* [Nm * 10] signed */
    int8_t   temp_motor;     /* [°C] */
    int8_t   temp_inverter;  /* [°C] */
    uint16_t fault_flags;
} CanMsg_MotorStatus_t;

/* IMU linear acceleration */
typedef struct {
    int16_t  accel_x;  /* [mg] */
    int16_t  accel_y;  /* [mg] */
    int16_t  accel_z;  /* [mg] */
    uint16_t reserved;
} CanMsg_IMUAccel_t;

/* Dashboard — physical inputs and LED state */
typedef struct {
    uint8_t button_flags;
    uint8_t rotary_1;
    uint8_t rotary_2;
    uint8_t led_flags;
    uint8_t reserved[4];
} CanMsg_DashboardCtrl_t;

#pragma pack(pop)

static_assert(sizeof(CanMsg_WheelSpeeds_t)    <= 8, "CanMsg_WheelSpeeds_t exceeds 8 bytes");
static_assert(sizeof(CanMsg_PowertrainCtrl_t) <= 8, "CanMsg_PowertrainCtrl_t exceeds 8 bytes");
static_assert(sizeof(CanMsg_BatteryStatus_t)  <= 8, "CanMsg_BatteryStatus_t exceeds 8 bytes");
static_assert(sizeof(CanMsg_MotorStatus_t)    <= 8, "CanMsg_MotorStatus_t exceeds 8 bytes");
static_assert(sizeof(CanMsg_IMUAccel_t)       <= 8, "CanMsg_IMUAccel_t exceeds 8 bytes");
static_assert(sizeof(CanMsg_DashboardCtrl_t)  <= 8, "CanMsg_DashboardCtrl_t exceeds 8 bytes");

#pragma once
#include <stdint.h>
#include "can_messages.h"

/**
 * @file can_codec.h
 * @brief Pure CAN message encode/decode functions.
 *
 * This layer is completely stateless — no global variables, no side effects.
 * Each function either packs signal values into an 8-byte CAN payload or
 * unpacks a received payload back into a typed struct.
 *
 * @note All values use scaled integers. No floats on the wire.
 *       See @ref can_messages.h for scaling factors per signal.
 *
 * @section codec_usage Direct usage (without the database runtime)
 * @code
 *   uint8_t data[8];
 *   CanCodec_WheelSpeeds_Encode(1200, 1201, 1198, 1199, data);
 *   // transmit data[8] on your CAN peripheral
 *
 *   // on the receiving side:
 *   CanMsg_WheelSpeeds_t msg = CanCodec_WheelSpeeds_Decode(data);
 *   float fl_rpm = msg.fl / 10.0f;
 * @endcode
 */

/** @brief Encode wheel speeds into @p data. Values in RPM * 10. */
void CanCodec_WheelSpeeds_Encode(uint16_t fl, uint16_t fr, uint16_t rl, uint16_t rr, uint8_t data[8]);

/** @brief Decode wheel speeds from @p data. Returns struct by value. */
CanMsg_WheelSpeeds_t CanCodec_WheelSpeeds_Decode(const uint8_t data[8]);

/** @brief Encode driver inputs. Throttle/brake in 0-200 (0.5% steps), torque in Nm * 10. */
void CanCodec_PowertrainCtrl_Encode(uint8_t throttle_pct, uint8_t brake_pct, uint16_t torque_limit, uint8_t flags, uint8_t data[8]);

/** @brief Decode driver inputs from @p data. Returns struct by value. */
CanMsg_PowertrainCtrl_t CanCodec_PowertrainCtrl_Decode(const uint8_t data[8]);

/** @brief Encode accumulator status. Voltage in mV, current in mA (signed), temp in °C. */
void CanCodec_BatteryStatus_Encode(uint16_t voltage_mv, int16_t current_ma, uint8_t soc_pct, int8_t temp_max, uint8_t fault_flags, uint8_t data[8]);

/** @brief Decode accumulator status from @p data. Returns struct by value. */
CanMsg_BatteryStatus_t CanCodec_BatteryStatus_Decode(const uint8_t data[8]);

/** @brief Encode motor/inverter status. Torque in Nm * 10 (signed), temps in °C. */
void CanCodec_MotorStatus_Encode(uint16_t rpm, int16_t torque_actual, int8_t temp_motor, int8_t temp_inverter, uint16_t fault_flags, uint8_t data[8]);

/** @brief Decode motor/inverter status from @p data. Returns struct by value. */
CanMsg_MotorStatus_t CanCodec_MotorStatus_Decode(const uint8_t data[8]);

/** @brief Encode IMU linear acceleration in milli-g. */
void CanCodec_IMUAccel_Encode(int16_t accel_x, int16_t accel_y, int16_t accel_z, uint8_t data[8]);

/** @brief Decode IMU linear acceleration from @p data. Returns struct by value. */
CanMsg_IMUAccel_t CanCodec_IMUAccel_Decode(const uint8_t data[8]);

/** @brief Encode dashboard button and LED state. */
void CanCodec_DashboardCtrl_Encode(uint8_t button_flags, uint8_t rotary_1, uint8_t rotary_2, uint8_t led_flags, uint8_t data[8]);

/** @brief Decode dashboard state from @p data. Returns struct by value. */
CanMsg_DashboardCtrl_t CanCodec_DashboardCtrl_Decode(const uint8_t data[8]);

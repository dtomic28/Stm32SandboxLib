#pragma once
#include "can_frame.h"
#include "can_messages.h"

/**
 * @file can_database.h
 * @brief CAN message database — runtime registration, dispatch, and publish API.
 *
 * @section workflow Workflow
 *
 * The library is split into two layers:
 *
 *  - **Codec** (`can_codec.h`) — pure, stateless encode/decode functions.
 *    No side effects. Safe to call from any context. Use directly when you
 *    need fine-grained control over frame construction.
 *
 *  - **Database** (this file) — runtime layer. Owns the bus registry and
 *    the RX handler table. Ties codec + transport together via registered
 *    platform drivers.
 *
 * @section usage Typical usage
 *
 * @subsection init 1. Register buses (once at startup)
 * @code
 *   static void MyCAN1_Send(const CanFrame_t *frame) {
 *       HAL_CAN_AddTxMessage(&hcan1, ...);
 *   }
 *   static const CanDb_BusDriver_t can1_driver = { .send = MyCAN1_Send };
 *
 *   CanDb_RegisterBus(CAN_BUS_POWERTRAIN, &can1_driver);
 *   CanDb_RegisterBus(CAN_BUS_SENSOR,     &can2_driver);
 * @endcode
 *
 * @subsection rx 2. Register RX handlers (once at startup)
 * @code
 *   void OnWheelSpeeds(const CanFrame_t *frame) {
 *       CanMsg_WheelSpeeds_t msg = CanCodec_WheelSpeeds_Decode(frame->data);
 *       // use msg.fl, msg.fr, msg.rl, msg.rr
 *   }
 *
 *   CanDb_RegisterHandler(CAN_ID_WHEEL_SPEEDS, OnWheelSpeeds);
 * @endcode
 *
 * @subsection dispatch 3. Dispatch from your RX interrupt
 * @code
 *   void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
 *       CAN_RxHeaderTypeDef header;
 *       CanFrame_t frame;
 *       HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &header, frame.data);
 *       frame.id  = header.StdId;
 *       frame.dlc = header.DLC;
 *       CanDb_Dispatch(CAN_BUS_SENSOR, &frame);
 *   }
 * @endcode
 *
 * @subsection tx 4. Publish messages
 * @code
 *   CanDb_WheelSpeeds_Publish(1200, 1201, 1198, 1199);
 * @endcode
 */

#ifndef CANDB_MAX_BUSES
/** @brief Maximum number of registered CAN buses. Override via compiler define. */
#define CANDB_MAX_BUSES     4
#endif

#ifndef CANDB_MAX_HANDLERS
/** @brief Maximum number of registered RX handlers. Override via compiler define. */
#define CANDB_MAX_HANDLERS  32
#endif

/* --- Runtime API --- */

/**
 * @brief Register a platform CAN bus driver.
 *
 * Must be called once per bus before any publish or dispatch calls.
 * The driver struct must remain valid for the lifetime of the program.
 *
 * @param bus    Bus identifier (see @ref CanBusId_t).
 * @param driver Pointer to a driver struct containing the send callback.
 */
void CanDb_RegisterBus(CanBusId_t bus, const CanDb_BusDriver_t *driver);

/**
 * @brief Register a handler for a specific CAN message ID.
 *
 * When @ref CanDb_Dispatch receives a frame with a matching ID, this
 * handler is called. Only one handler per ID is supported — the first
 * registration wins.
 *
 * @param id      CAN message ID to listen for (see @ref can_ids.h).
 * @param handler Callback invoked with the raw frame on match.
 */
void CanDb_RegisterHandler(uint32_t id, void (*handler)(const CanFrame_t *));

/**
 * @brief Dispatch a received CAN frame to its registered handler.
 *
 * Call this from your platform RX interrupt or task. The frame ID is
 * looked up in the handler registry and the matching handler is called
 * synchronously. Frames with no registered handler are silently dropped.
 *
 * @param bus   Bus the frame was received on.
 * @param frame Pointer to the received frame.
 */
void CanDb_Dispatch(CanBusId_t bus, const CanFrame_t *frame);

/* --- Publish API --- */

/** @brief Publish wheel speeds. Values in RPM * 10 (0.1 RPM resolution). */
void CanDb_WheelSpeeds_Publish(uint16_t fl, uint16_t fr, uint16_t rl, uint16_t rr);

/** @brief Publish driver inputs. Throttle/brake in 0-200 (0.5% steps), torque limit in Nm * 10. */
void CanDb_PowertrainCtrl_Publish(uint8_t throttle_pct, uint8_t brake_pct, uint16_t torque_limit, uint8_t flags);

/** @brief Publish accumulator status. Voltage in mV, current in mA (signed), temp in °C. */
void CanDb_BatteryStatus_Publish(uint16_t voltage_mv, int16_t current_ma, uint8_t soc_pct, int8_t temp_max, uint8_t fault_flags);

/** @brief Publish motor and inverter status. RPM, torque in Nm * 10 (signed), temps in °C. */
void CanDb_MotorStatus_Publish(uint16_t rpm, int16_t torque_actual, int8_t temp_motor, int8_t temp_inverter, uint16_t fault_flags);

/** @brief Publish IMU linear acceleration. Values in milli-g. */
void CanDb_IMUAccel_Publish(int16_t accel_x, int16_t accel_y, int16_t accel_z);

/** @brief Publish dashboard button and LED state. */
void CanDb_DashboardCtrl_Publish(uint8_t button_flags, uint8_t rotary_1, uint8_t rotary_2, uint8_t led_flags);

#pragma once
#include "can_frame.h"
#include "can_database/can_messages.h"

/**
 * @file can_database.h
 * @brief CAN message database — runtime registration, publish, and subscribe API.
 *
 * @section workflow Workflow
 *
 * The library is split into layers:
 *
 *  - **Codec** (`can_codec.h`, generated) — pure, stateless encode/decode
 *    functions. No side effects. Safe to call from any context.
 *
 *  - **Database** (this file, hand-written) — runtime layer. Owns the bus
 *    registry and the subscriber table. Ties codec + transport together
 *    via registered platform drivers.
 *
 *  - **Publish** (`can_publish.h`, generated) — one `CanDb_X_Publish`
 *    function per message: encodes and sends on that message's bus.
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
 * @subsection rx 2. Subscribe to messages (once at startup)
 * @code
 *   void OnWheelSpeeds(const CanFrame_t *frame) {
 *       CanMsg_WheelSpeeds_t msg = CanCodec_WheelSpeeds_Decode(frame->data);
 *       // use msg.fl, msg.fr, msg.rl, msg.rr
 *   }
 *
 *   CanDb_Subscribe(CAN_ID_WHEEL_SPEEDS, OnWheelSpeeds);
 * @endcode
 *
 * @subsection dispatch 3. Feed raw frames in from your RX interrupt
 * @code
 *   void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
 *       CAN_RxHeaderTypeDef header;
 *       CanFrame_t frame;
 *       HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &header, frame.data);
 *       frame.id  = header.StdId;
 *       frame.dlc = header.DLC;
 *       CanDb_OnRawFrameReceived(CAN_BUS_SENSOR, &frame);
 *   }
 * @endcode
 *
 * @subsection tx 4. Publish messages
 * @code
 *   CanMsg_WheelSpeeds_t msg = { .fl = 120.0f, .fr = 120.5f, .rl = 119.8f, .rr = 119.9f };
 *   CanDb_WheelSpeeds_Publish(&msg);
 * @endcode
 */

#ifndef CANDB_MAX_BUSES
/** @brief Maximum number of registered CAN buses. Override via compiler define. */
#define CANDB_MAX_BUSES     4
#endif

#ifndef CANDB_MAX_HANDLERS
/** @brief Maximum number of subscribed message IDs. Override via compiler define. */
#define CANDB_MAX_HANDLERS  32
#endif

/**
 * @brief Register a platform CAN bus driver.
 *
 * Must be called once per bus before any publish or receive calls.
 * The driver struct must remain valid for the lifetime of the program.
 *
 * @param bus    Bus identifier (see @ref CanBusId_t).
 * @param driver Pointer to a driver struct containing the send callback.
 */
void CanDb_RegisterBus(CanBusId_t bus, const CanDb_BusDriver_t *driver);

/**
 * @brief Subscribe a callback to a specific CAN message ID.
 *
 * When @ref CanDb_OnRawFrameReceived is fed a frame with a matching ID,
 * this callback is invoked. Only one subscriber per ID is supported —
 * the first subscription wins.
 *
 * @param id       CAN message ID to listen for (see @ref can_ids.h).
 * @param callback Invoked with the raw, not-yet-decoded frame on match.
 */
void CanDb_Subscribe(uint32_t id, void (*callback)(const CanFrame_t *));

/**
 * @brief Route a raw, received CAN frame to its subscriber, if any.
 *
 * Call this from your platform RX interrupt or task with the frame
 * exactly as it came off the bus — nothing has decoded it yet. The
 * frame ID is looked up in the subscriber table and the matching
 * callback is invoked synchronously; that callback is what actually
 * calls the message's `_Decode` function. Frames with no subscriber
 * are silently dropped.
 *
 * @param bus   Bus the frame was received on.
 * @param frame Pointer to the raw received frame.
 */
void CanDb_OnRawFrameReceived(CanBusId_t bus, const CanFrame_t *frame);

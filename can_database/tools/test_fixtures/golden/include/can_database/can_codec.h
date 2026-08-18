/* GENERATED FILE — do not edit by hand.
 * Source: can_messages.yaml, generator: tools/gen_can_database.py
 */


#pragma once
#include <stdint.h>
#include "can_database/can_messages.h"

void CanCodec_PlainMsg_Encode(const CanMsg_PlainMsg_t *m, uint8_t data[4]);
CanMsg_PlainMsg_t CanCodec_PlainMsg_Decode(const uint8_t data[4]);

void CanCodec_ScaledMsg_Encode(const CanMsg_ScaledMsg_t *m, uint8_t data[2]);
CanMsg_ScaledMsg_t CanCodec_ScaledMsg_Decode(const uint8_t data[2]);

void CanCodec_PackedMsg_Encode(const CanMsg_PackedMsg_t *m, uint8_t data[3]);
CanMsg_PackedMsg_t CanCodec_PackedMsg_Decode(const uint8_t data[3]);


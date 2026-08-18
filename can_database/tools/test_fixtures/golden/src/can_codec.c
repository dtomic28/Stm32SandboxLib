/* GENERATED FILE — do not edit by hand.
 * Source: can_messages.yaml, generator: tools/gen_can_database.py
 */


#include "can_database/can_codec.h"
#include "can_database/can_bits.h"
#include <string.h>
#include <math.h>
#include <assert.h>

void CanCodec_PlainMsg_Encode(const CanMsg_PlainMsg_t *m, uint8_t data[4]) {
    memset(data, 0, 4);
    { uint8_t _raw = (uint8_t)m->a; data[0] = (uint8_t)(_raw); }
    { int16_t _raw = (int16_t)m->b; data[1] = (uint8_t)(_raw); data[2] = (uint8_t)(_raw >> 8); }
    { uint8_t _raw = (uint8_t)m->c; data[3] = (uint8_t)(_raw); }
}

CanMsg_PlainMsg_t CanCodec_PlainMsg_Decode(const uint8_t data[4]) {
    CanMsg_PlainMsg_t msg;
    msg.a = (uint8_t)((uint8_t)(data[0]));
    msg.b = (int16_t)((int16_t)(data[1] | (uint32_t)data[2] << 8));
    msg.c = (uint8_t)((uint8_t)(data[3]));
    return msg;
}

void CanCodec_ScaledMsg_Encode(const CanMsg_ScaledMsg_t *m, uint8_t data[2]) {
    memset(data, 0, 2);
    {
        int16_t _raw = (int16_t)lroundf((m->temp - -40.0f) / 0.01f);
        CanBits_Pack(data, (uint32_t)_raw, 0, 16);
    }
}

CanMsg_ScaledMsg_t CanCodec_ScaledMsg_Decode(const uint8_t data[2]) {
    CanMsg_ScaledMsg_t msg;
    msg.temp = (float)((CanBits_SignExtend(CanBits_Unpack(data, 0, 16), 16)) * 0.01f + -40.0f);
    return msg;
}

void CanCodec_PackedMsg_Encode(const CanMsg_PackedMsg_t *m, uint8_t data[3]) {
    memset(data, 0, 3);
    {
        uint32_t _raw = (uint32_t)m->narrow;
        assert((int64_t)_raw <= 16777215);
        CanBits_Pack(data, (uint32_t)_raw, 0, 24);
    }
}

CanMsg_PackedMsg_t CanCodec_PackedMsg_Decode(const uint8_t data[3]) {
    CanMsg_PackedMsg_t msg;
    msg.narrow = (uint32_t)(CanBits_Unpack(data, 0, 24));
    return msg;
}


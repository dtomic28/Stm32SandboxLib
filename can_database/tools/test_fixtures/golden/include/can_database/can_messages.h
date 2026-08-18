/* GENERATED FILE — do not edit by hand.
 * Source: can_messages.yaml, generator: tools/gen_can_database.py
 */


#pragma once
#include <stdint.h>

typedef struct {
    uint8_t a;  /* raw */
    int16_t b;  /* raw */
    uint8_t c;  /* raw */
} CanMsg_PlainMsg_t;

typedef struct {
    float temp;  /* raw */
} CanMsg_ScaledMsg_t;

typedef struct {
    uint32_t narrow;  /* raw */
} CanMsg_PackedMsg_t;


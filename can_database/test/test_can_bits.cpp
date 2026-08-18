/**
 * @file test_can_bits.cpp
 * @brief Direct tests for can_bits.c — the one shared mechanism every
 *        message's non-fast-path signal ultimately depends on. Tested
 *        here in isolation, with synthetic bit ranges, rather than by
 *        relying on some real message happening to need a given shape.
 */
#include <gtest/gtest.h>

extern "C" {
#include "can_database/can_bits.h"
}

TEST(CanBits, PackUnpack_ByteAligned) {
    uint8_t data[2] = {0};
    CanBits_Pack(data, 0x1234, 0, 16);
    EXPECT_EQ(data[0], 0x34);
    EXPECT_EQ(data[1], 0x12);
    EXPECT_EQ(CanBits_Unpack(data, 0, 16), 0x1234u);
}

TEST(CanBits, PackUnpack_SubByte_SingleField) {
    /* 5 bits, value 0b10110 (22), starting at bit 3 of byte 0 */
    uint8_t data[1] = {0};
    CanBits_Pack(data, 0b10110, 3, 5);
    /* bits 3-7 of byte 0 should hold 10110, bits 0-2 untouched (0) */
    EXPECT_EQ(data[0], 0b10110000);
    EXPECT_EQ(CanBits_Unpack(data, 3, 5), 0b10110u);
}

TEST(CanBits, PackUnpack_AdjacentFieldsDoNotOverlap) {
    /* Two 3-bit fields packed back to back in the same byte must not
     * clobber each other. */
    uint8_t data[1] = {0};
    CanBits_Pack(data, 0b101, 0, 3);
    CanBits_Pack(data, 0b011, 3, 3);
    EXPECT_EQ(CanBits_Unpack(data, 0, 3), 0b101u);
    EXPECT_EQ(CanBits_Unpack(data, 3, 3), 0b011u);
}

TEST(CanBits, PackUnpack_SpansByteBoundary) {
    /* 12-bit field starting at bit 4 -- spans byte 0 and byte 1 */
    uint8_t data[2] = {0};
    CanBits_Pack(data, 0xABC, 4, 12);
    EXPECT_EQ(CanBits_Unpack(data, 4, 12), 0xABCu);
    /* bits 0-3 of byte 0 must be untouched */
    EXPECT_EQ(data[0] & 0x0F, 0);
}

TEST(CanBits, SignExtend_8bit) {
    EXPECT_EQ(CanBits_SignExtend(0x7F, 8), 127);
    EXPECT_EQ(CanBits_SignExtend(0x80, 8), -128);
    EXPECT_EQ(CanBits_SignExtend(0xFF, 8), -1);
}

TEST(CanBits, SignExtend_SubByte_5bit) {
    /* 5-bit signed range: -16..15 */
    EXPECT_EQ(CanBits_SignExtend(0b01111, 5), 15);
    EXPECT_EQ(CanBits_SignExtend(0b10000, 5), -16);
    EXPECT_EQ(CanBits_SignExtend(0b11111, 5), -1);
}

TEST(CanBits, SignExtend_24bit) {
    /* The 24-bit-signed case worked through earlier in the codebase's
     * design: -100 packed as a truncated 24-bit two's-complement value
     * must sign-extend back to exactly -100. */
    uint8_t data[3] = {0};
    CanBits_Pack(data, static_cast<uint32_t>(-100), 0, 24);
    int32_t decoded = CanBits_SignExtend(CanBits_Unpack(data, 0, 24), 24);
    EXPECT_EQ(decoded, -100);

    EXPECT_EQ(CanBits_SignExtend(0x7FFFFF, 24), 8388607);
    EXPECT_EQ(CanBits_SignExtend(0x800000, 24), -8388608);
}

TEST(CanBits, Unpack_UnsignedNeverSignExtends) {
    /* Unpack alone (no SignExtend) must zero-extend, even for a value
     * whose top bit is set within its field width. */
    uint8_t data[1] = {0};
    CanBits_Pack(data, 0b1111, 0, 4);
    EXPECT_EQ(CanBits_Unpack(data, 0, 4), 0b1111u);
}

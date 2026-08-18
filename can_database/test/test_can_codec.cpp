/**
 * @file test_can_codec.cpp
 * @brief Mechanism-level smoke tests for generated codec code.
 *
 * Deliberately NOT one test per message. Every message funnels through
 * the same handful of code-generation templates in gen_can_database.py
 * (fast-path byte store, general bit-pack, sign-extend, scale/offset
 * conversion) — validating each template once, on one real message that
 * exercises it, covers every other message using that same shape. The
 * generator's own logic (bit-offset math, fast-path eligibility, schema
 * validation) is covered separately and more thoroughly in
 * tools/test_gen_can_database.py, including a golden-file test that
 * catches any unintended change in generated output.
 *
 * can_bits.c's pack/unpack/sign-extend primitives are tested directly,
 * in isolation with synthetic bit ranges, in test_can_bits.cpp.
 */
#include <gtest/gtest.h>

extern "C" {
#include "can_database/can_codec.h"
}

/* --- Fast path: plain byte-aligned, unscaled signals (PowertrainCtrl) --- */

TEST(CanCodec_Mechanism, ByteAlignedRoundTrip) {
    CanMsg_PowertrainCtrl_t in = { 150, 20, 3000, 0x03 };
    uint8_t data[8] = {0};
    CanCodec_PowertrainCtrl_Encode(&in, data);
    CanMsg_PowertrainCtrl_t out = CanCodec_PowertrainCtrl_Decode(data);
    EXPECT_EQ(out.throttle_pct, 150);
    EXPECT_EQ(out.brake_pct,    20);
    EXPECT_EQ(out.torque_limit, 3000);
    EXPECT_EQ(out.flags,        0x03);
}

TEST(CanCodec_Mechanism, ReservedBytesStayZero) {
    /* PowertrainCtrl's signals use bits 0-31 (4 bytes); bytes 4-7 are
     * reserved and must come back zeroed regardless of prior data[] content. */
    uint8_t data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    CanMsg_PowertrainCtrl_t in = { 0, 0, 0, 0 };
    CanCodec_PowertrainCtrl_Encode(&in, data);
    EXPECT_EQ(data[4], 0x00);
    EXPECT_EQ(data[5], 0x00);
    EXPECT_EQ(data[6], 0x00);
    EXPECT_EQ(data[7], 0x00);
}

/* --- Signed values, including the sign-extension boundary (BatteryStatus) --- */

TEST(CanCodec_Mechanism, SignedRoundTrip) {
    CanMsg_BatteryStatus_t in = { 48000, INT16_MIN, 85, -40, 0x01 };
    uint8_t data[8] = {0};
    CanCodec_BatteryStatus_Encode(&in, data);
    CanMsg_BatteryStatus_t out = CanCodec_BatteryStatus_Decode(data);
    EXPECT_EQ(out.voltage_mv,  48000);
    EXPECT_EQ(out.current_ma,  INT16_MIN);
    EXPECT_EQ(out.soc_pct,     85);
    EXPECT_EQ(out.temp_max,    -40);
    EXPECT_EQ(out.fault_flags, 0x01);
}

/* --- Scaled float signals, with rounding tolerance (WheelSpeeds) --- */

TEST(CanCodec_Mechanism, ScaledFloatRoundTrip) {
    CanMsg_WheelSpeeds_t in = { 120.5f, 121.0f, 119.8f, 120.1f };
    uint8_t data[8] = {0};
    CanCodec_WheelSpeeds_Encode(&in, data);
    CanMsg_WheelSpeeds_t out = CanCodec_WheelSpeeds_Decode(data);
    /* +/- half a 0.1 RPM step for the raw-integer round-trip quantization */
    EXPECT_NEAR(out.fl, 120.5f, 0.05f);
    EXPECT_NEAR(out.fr, 121.0f, 0.05f);
    EXPECT_NEAR(out.rl, 119.8f, 0.05f);
    EXPECT_NEAR(out.rr, 120.1f, 0.05f);
}

TEST(CanCodec_Mechanism, ScaledFloatMaxRawValue) {
    /* 6553.5 RPM is the largest value the uint16_t*0.1 wire encoding can hold */
    CanMsg_WheelSpeeds_t in = { 6553.5f, 6553.5f, 6553.5f, 6553.5f };
    uint8_t data[8] = {0};
    CanCodec_WheelSpeeds_Encode(&in, data);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(data[i], 0xFF);
    }
    CanMsg_WheelSpeeds_t out = CanCodec_WheelSpeeds_Decode(data);
    EXPECT_NEAR(out.fl, 6553.5f, 0.05f);
}

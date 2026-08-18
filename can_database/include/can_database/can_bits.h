#pragma once
#include <stdint.h>

/**
 * @file can_bits.h
 * @brief Bit-level pack/unpack primitives used by generated codec code.
 *
 * LSB-first (Intel/little-endian) bit numbering, matching DBC's default
 * `@1` byte order. Fields need not be byte-aligned. Hand-written and
 * stable — not touched by code generation.
 */

/** @brief Pack the low @p bit_len bits of @p value into @p data at @p start_bit. */
void CanBits_Pack(uint8_t *data, uint32_t value, uint16_t start_bit, uint8_t bit_len);

/** @brief Unpack @p bit_len bits from @p data at @p start_bit, zero-extended. */
uint32_t CanBits_Unpack(const uint8_t *data, uint16_t start_bit, uint8_t bit_len);

/** @brief Sign-extend the low @p bit_len bits of @p value to a full int32_t. */
int32_t CanBits_SignExtend(uint32_t value, uint8_t bit_len);

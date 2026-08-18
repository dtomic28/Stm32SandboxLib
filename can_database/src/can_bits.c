#include "can_database/can_bits.h"

void CanBits_Pack(uint8_t *data, uint32_t value, uint16_t start_bit, uint8_t bit_len) {
    for (uint8_t i = 0; i < bit_len; i++) {
        uint16_t pos = start_bit + i;
        if (value & (1u << i)) {
            data[pos / 8] |= (uint8_t)(1u << (pos % 8));
        } else {
            data[pos / 8] &= (uint8_t)~(1u << (pos % 8));
        }
    }
}

uint32_t CanBits_Unpack(const uint8_t *data, uint16_t start_bit, uint8_t bit_len) {
    uint32_t value = 0;
    for (uint8_t i = 0; i < bit_len; i++) {
        uint16_t pos = start_bit + i;
        if (data[pos / 8] & (1u << (pos % 8))) {
            value |= (1u << i);
        }
    }
    return value;
}

int32_t CanBits_SignExtend(uint32_t value, uint8_t bit_len) {
    if (bit_len < 32 && (value & (1u << (bit_len - 1)))) {
        value |= ~((1u << bit_len) - 1u);
    }
    return (int32_t)value;
}

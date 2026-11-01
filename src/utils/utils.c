#include "utils.h"

uint16_t sign_extend(const uint16_t val, const int bit_amount) {
    const uint16_t highest_bit = (val >> (bit_amount - 1)) & 1;

    if (highest_bit == 1) {
        const uint16_t ones = (uint16_t)(0xffff << bit_amount);
        return ones | val;
    }

    return val;
}

uint16_t swap16(const uint16_t val) {
    return (uint16_t)((val << 8) | (val >> 8));
}

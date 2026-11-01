#ifndef MY_VM_UTILS_H
#define MY_VM_UTILS_H

#include <stdint.h>

// sign-extends a bit_amount-bit value to 16 bits.
uint16_t sign_extend(uint16_t val, int bit_amount);

// convert 16-bit number from little endian to big endian
uint16_t swap16(uint16_t val);

#endif // MY_VM_UTILS_H

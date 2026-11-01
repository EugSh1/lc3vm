#ifndef MY_VM_INSTRUCTION_H
#define MY_VM_INSTRUCTION_H

enum Instruction {
    BR = 0x0,  // 0b0000
    ADD = 0x1, // 0b0001
    LD = 0x2,  // 0b0010
    ST = 0x3,  // 0b0011
    JSR = 0x4, // 0b0100
    AND = 0x5, // 0b0101
    LDR = 0x6, // 0b0110
    STR = 0x7, // 0b0111
    // RTI is not used in userspace, so it is not presented here
    NOT = 0x9, // 0b1001
    LDI = 0xa, // 0b1010
    STI = 0xb, // 0b1011
    JMP = 0xc, // 0b1100
    LEA = 0xe, // 0b1110
    TRAP = 0xf // 0b1111
};

#endif // MY_VM_INSTRUCTION_H

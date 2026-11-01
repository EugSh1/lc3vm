#ifndef MY_VM_TEST_HELPERS_H
#define MY_VM_TEST_HELPERS_H

#define PRINT_PASS printf("[PASS] %s\n", __func__)

// LC-3 instructions
#define LC3_BR(n, z, p, pc_offset9)                                                                \
    ((0x0 << 12) | (((n) & 0x1) << 11) | (((z) & 0x1) << 10) | (((p) & 0x1) << 9) |                \
     ((pc_offset9) & 0x1ff))

#define LC3_ADD_MODE_0(dr, sr1, sr2)                                                               \
    ((0x1 << 12) | (((dr) & 0x7) << 9) | (((sr1) & 0x7) << 6) | ((sr2) & 0x7))
#define LC3_ADD_MODE_1(dr, sr1, imm5)                                                              \
    ((0x1 << 12) | (((dr) & 0x7) << 9) | (((sr1) & 0x7) << 6) | (0x1 << 5) | ((imm5) & 0x1f))

#define LC3_LD(dr, pc_offset9) ((0x2 << 12) | (((dr) & 0x7) << 9) | ((pc_offset9) & 0x1ff))

#define LC3_ST(sr, pc_offset9) ((0x3 << 12) | (((sr) & 0x7) << 9) | ((pc_offset9) & 0x1ff))

#define LC3_JSR(pc_offset11) ((0x4 << 12) | (0x1 << 11) | ((pc_offset11) & 0x7ff))
#define LC3_JSRR(base_r) ((0x4 << 12) | (((base_r) & 0x7) << 6))

#define LC3_AND_MODE_0(dr, sr1, sr2)                                                               \
    ((0x5 << 12) | (((dr) & 0x7) << 9) | (((sr1) & 0x7) << 6) | ((sr2) & 0x7))
#define LC3_AND_MODE_1(dr, sr1, imm5)                                                              \
    ((0x5 << 12) | (((dr) & 0x7) << 9) | (((sr1) & 0x7) << 6) | (0x1 << 5) | ((imm5) & 0x1f))

#define LC3_LDR(dr, base_r, offset6)                                                               \
    ((0x6 << 12) | (((dr) & 0x7) << 9) | (((base_r) & 0x7) << 6) | ((offset6) & 0x3f))

#define LC3_STR(sr, base_r, offset6)                                                               \
    ((0x7 << 12) | (((sr) & 0x7) << 9) | (((base_r) & 0x7) << 6) | ((offset6) & 0x3f))

#define LC3_NOT(dr, sr) ((0x9 << 12) | (((dr) & 0x7) << 9) | (((sr) & 0x7) << 6) | 0x3f)

#define LC3_LDI(dr, pc_offset9) ((0xa << 12) | (((dr) & 0x7) << 9) | ((pc_offset9) & 0x1ff))

#define LC3_STI(sr, pc_offset9) ((0xb << 12) | (((sr) & 0x7) << 9) | ((pc_offset9) & 0x1ff))

#define LC3_JMP(base_r) ((0xc << 12) | (((base_r) & 0x7) << 6))

#define LC3_LEA(dr, pc_offset9) ((0xe << 12) | (((dr) & 0x7) << 9) | ((pc_offset9) & 0x1ff))

#define LC3_TRAP(trapvect8) ((0xf << 12) | ((trapvect8) & 0xff))

#endif // MY_VM_TEST_HELPERS_H

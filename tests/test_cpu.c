#include "cpu/cpu.h"
#include "test_helpers.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void test_st_instruction(void) {
    struct Cpu cpu = cpu_create();

    // Store register to memory
    cpu.pc = 0;
    uint16_t memory[10] = {0};
    cpu.gprs[3] = 0xabcd;
    memory[0] = LC3_ST(3, 2); // target: PC + 1 + 2 = 3
    cpu_step(&cpu, memory);

    assert(memory[3] == 0xabcd);

    PRINT_PASS;
}

static void test_jsr_instruction(void) {
    struct Cpu cpu = cpu_create();

    // JSR (Relative)
    {
        cpu.pc = 0;
        uint16_t memory[10] = {0};
        memory[0] = LC3_JSR(2); // target: PC + 1 + 2 = 3
        cpu_step(&cpu, memory);

        assert(cpu.pc == 3);
        assert(cpu.gprs[7] == 1); // Return address: PC + 1
    }

    // JSRR (Register)
    {
        cpu.pc = 0;
        uint16_t memory[10] = {0};
        cpu.gprs[4] = 0x0005;
        memory[0] = LC3_JSRR(4);
        cpu_step(&cpu, memory);

        assert(cpu.pc == 5);
        assert(cpu.gprs[7] == 1); // Return address: PC + 1
    }

    PRINT_PASS;
}

static void test_ldr_instruction(void) {
    struct Cpu cpu = cpu_create();

    // Load relative
    cpu.pc = 0;
    uint16_t memory[10] = {0};
    cpu.gprs[2] = 4;
    memory[0] = LC3_LDR(1, 2, 2); // target: R2 + 2 = 6
    memory[6] = 0xdead;
    cpu_step(&cpu, memory);

    assert(cpu.gprs[1] == 0xdead);

    PRINT_PASS;
}

static void test_str_instruction(void) {
    struct Cpu cpu = cpu_create();

    // Store relative
    cpu.pc = 0;
    uint16_t memory[10] = {0};
    cpu.gprs[2] = 4;
    cpu.gprs[3] = 0xbeef;
    memory[0] = LC3_STR(3, 2, 2); // target: R2 + 2 = 6
    cpu_step(&cpu, memory);

    assert(memory[6] == 0xbeef);

    PRINT_PASS;
}

static void test_ldi_instruction(void) {
    struct Cpu cpu = cpu_create();

    // Load indirect
    cpu.pc = 0;
    uint16_t memory[10] = {0};
    memory[0] = LC3_LDI(1, 2); // address of value: PC + 1 + 2 = 3
    memory[3] = 5;             // value is at address 5
    memory[5] = 0xcafe;
    cpu_step(&cpu, memory);

    assert(cpu.gprs[1] == 0xcafe);

    PRINT_PASS;
}

static void test_sti_instruction(void) {
    struct Cpu cpu = cpu_create();

    // Store indirect
    cpu.pc = 0;
    uint16_t memory[10] = {0};
    cpu.gprs[3] = 0xbaad;
    memory[0] = LC3_STI(3, 2); // address of target: PC + 1 + 2 = 3
    memory[3] = 5;             // target is at address 5
    cpu_step(&cpu, memory);

    assert(memory[5] == 0xbaad);

    PRINT_PASS;
}

static void test_jmp_instruction(void) {
    struct Cpu cpu = cpu_create();

    // Jump register
    cpu.pc = 0;
    uint16_t memory[10] = {0};
    cpu.gprs[4] = 0x0007;
    memory[0] = LC3_JMP(4);
    cpu_step(&cpu, memory);

    assert(cpu.pc == 7);

    PRINT_PASS;
}

static void test_lea_instruction(void) {
    struct Cpu cpu = cpu_create();

    // Load effective address
    cpu.pc = 0;
    uint16_t memory[10] = {0};
    memory[0] = LC3_LEA(1, 2); // address: PC + 1 + 2 = 3
    cpu_step(&cpu, memory);

    assert(cpu.gprs[1] == 3);

    PRINT_PASS;
}

static void test_rti_instruction(void) {
    struct Cpu cpu = cpu_create();

    // RTI should return ERR_BAD_INSTRUCTION
    cpu.pc = 0;
    uint16_t memory[1] = {0x8000}; // Opcode 0x8 is RTI
    enum CpuStepErrorCode res = cpu_step(&cpu, memory);
    assert(res == ERR_BAD_INSTRUCTION);

    PRINT_PASS;
}

static void test_br_instruction(void) {
    struct Cpu cpu = cpu_create();

    // Branch taken - Positive
    {
        cpu.pc = 0;
        uint16_t memory[10] = {0};
        cpu.cond.positive = 1;
        cpu.cond.negative = 0;
        cpu.cond.zero = 0;
        memory[0] = LC3_BR(0, 0, 1, 2); // BRp, offset 2 -> PC = 0 + 1 + 2 = 3
        cpu_step(&cpu, memory);

        assert(cpu.pc == 3);
    }

    // Branch not taken - Positive condition but BRn
    {
        cpu.pc = 0;
        uint16_t memory[10] = {0};
        cpu.cond.positive = 1;
        cpu.cond.negative = 0;
        cpu.cond.zero = 0;
        memory[0] = LC3_BR(1, 0, 0, 2); // BRn, offset 2 -> should not jump
        cpu_step(&cpu, memory);

        assert(cpu.pc == 1);
    }

    // Branch taken - Zero
    {
        cpu.pc = 0;
        uint16_t memory[10] = {0};
        cpu.cond.positive = 0;
        cpu.cond.negative = 0;
        cpu.cond.zero = 1;
        memory[0] = LC3_BR(0, 1, 0, 5); // BRz, offset 5 -> PC = 0 + 1 + 5 = 6
        cpu_step(&cpu, memory);

        assert(cpu.pc == 6);
    }

    // Branch taken - Negative offset
    {
        cpu.pc = 10;
        uint16_t memory[20] = {0};
        cpu.cond.positive = 0;
        cpu.cond.negative = 1;
        cpu.cond.zero = 0;
        memory[10] = LC3_BR(1, 0, 0, (uint16_t)-3); // BRn, offset -3 -> PC = 10 + 1 - 3 = 8
        cpu_step(&cpu, memory);

        assert(cpu.pc == 8);
    }

    PRINT_PASS;
}

static void test_ld_instruction(void) {
    struct Cpu cpu = cpu_create();

    // Load positive value
    {
        cpu.pc = 0;
        uint16_t memory[10] = {0};
        memory[0] = LC3_LD(1, 2); // target: PC + 1 + 2 = 3
        memory[3] = 0x1234;
        cpu_step(&cpu, memory);

        assert(cpu.gprs[1] == 0x1234);
        assert(cpu.cond.positive == 1);
        assert(cpu.cond.negative == 0);
        assert(cpu.cond.zero == 0);
    }

    // Load negative value
    {
        cpu.pc = 0;
        uint16_t memory[10] = {0};
        memory[0] = LC3_LD(1, 2); // target: PC + 1 + 2 = 3
        memory[3] = (uint16_t)-50;
        cpu_step(&cpu, memory);

        assert(cpu.gprs[1] == (uint16_t)-50);
        assert(cpu.cond.positive == 0);
        assert(cpu.cond.negative == 1);
        assert(cpu.cond.zero == 0);
    }

    // Load zero value
    {
        cpu.pc = 0;
        uint16_t memory[10] = {0};
        memory[0] = LC3_LD(1, 2); // target: PC + 1 + 2 = 3
        memory[3] = 0x0000;
        cpu_step(&cpu, memory);

        assert(cpu.gprs[1] == 0x0000);
        assert(cpu.cond.positive == 0);
        assert(cpu.cond.negative == 0);
        assert(cpu.cond.zero == 1);
    }

    PRINT_PASS;
}

static void test_add_instruction(void) {
    struct Cpu cpu = cpu_create();

    // Mode 0 - positive + positive
    {
        cpu.pc = 0;
        uint16_t memory[1] = {0};
        cpu.gprs[2] = 7;
        cpu.gprs[3] = 4;
        memory[0] = LC3_ADD_MODE_0(1, 2, 3);
        cpu_step(&cpu, memory);

        assert(cpu.gprs[1] == 11);
        assert(cpu.cond.positive == 1);
        assert(cpu.cond.negative == 0);
        assert(cpu.cond.zero == 0);
    }

    // Mode 0 - positive + negative
    {
        cpu.pc = 0;
        uint16_t memory[1] = {0};
        cpu.gprs[2] = 7;
        cpu.gprs[3] = (uint16_t)-9;
        memory[0] = LC3_ADD_MODE_0(1, 2, 3);
        cpu_step(&cpu, memory);

        assert(cpu.gprs[1] == (uint16_t)-2);
        assert(cpu.cond.positive == 0);
        assert(cpu.cond.negative == 1);
        assert(cpu.cond.zero == 0);
    }

    // Mode 1 - positive + positive
    {
        cpu.pc = 0;
        uint16_t memory[1] = {0};
        cpu.gprs[2] = 7;
        memory[0] = LC3_ADD_MODE_1(1, 2, 2);
        cpu_step(&cpu, memory);

        assert(cpu.gprs[1] == 9);
        assert(cpu.cond.positive == 1);
        assert(cpu.cond.negative == 0);
        assert(cpu.cond.zero == 0);
    }

    // Mode 1 - positive + negative
    {
        cpu.pc = 0;
        uint16_t memory[1] = {0};
        cpu.gprs[2] = 3;
        memory[0] = LC3_ADD_MODE_1(1, 2, (uint16_t)-7);
        cpu_step(&cpu, memory);

        assert(cpu.gprs[1] == (uint16_t)-4);
        assert(cpu.cond.positive == 0);
        assert(cpu.cond.negative == 1);
        assert(cpu.cond.zero == 0);
    }

    PRINT_PASS;
}

static void test_and_instruction(void) {
    struct Cpu cpu = cpu_create();

    // Mode 0 - result positive
    {
        cpu.pc = 0;
        uint16_t memory[1] = {0};
        cpu.gprs[2] = 0x00ff;
        cpu.gprs[3] = 0x0f0f;
        memory[0] = LC3_AND_MODE_0(1, 2, 3);
        cpu_step(&cpu, memory);

        assert(cpu.gprs[1] == 0x000f);
        assert(cpu.cond.positive == 1);
        assert(cpu.cond.negative == 0);
        assert(cpu.cond.zero == 0);
    }

    // Mode 0 - result zero
    {
        cpu.pc = 0;
        uint16_t memory[1] = {0};
        cpu.gprs[2] = 0x00ff;
        cpu.gprs[3] = 0xff00;
        memory[0] = LC3_AND_MODE_0(1, 2, 3);
        cpu_step(&cpu, memory);

        assert(cpu.gprs[1] == 0x0000);
        assert(cpu.cond.positive == 0);
        assert(cpu.cond.negative == 0);
        assert(cpu.cond.zero == 1);
    }

    // Mode 1 - positive immediate
    {
        cpu.pc = 0;
        uint16_t memory[1] = {0};
        cpu.gprs[2] = 0xffff;
        memory[0] = LC3_AND_MODE_1(1, 2, 0x07);
        cpu_step(&cpu, memory);

        assert(cpu.gprs[1] == 0x0007);
        assert(cpu.cond.positive == 1);
        assert(cpu.cond.negative == 0);
        assert(cpu.cond.zero == 0);
    }

    // Mode 1 - negative immediate (sign extended)
    {
        cpu.pc = 0;
        uint16_t memory[1] = {0};
        cpu.gprs[2] = 0xffff;
        memory[0] = LC3_AND_MODE_1(1, 2, (uint16_t)-1);
        cpu_step(&cpu, memory);

        assert(cpu.gprs[1] == 0xffff);
        assert(cpu.cond.positive == 0);
        assert(cpu.cond.negative == 1);
        assert(cpu.cond.zero == 0);
    }

    PRINT_PASS;
}

static void test_not_instruction(void) {
    struct Cpu cpu = cpu_create();
    cpu.pc = 0;
    uint16_t memory[1] = {0};
    cpu.gprs[2] = 0x0001;
    memory[0] = LC3_NOT(1, 2);
    cpu_step(&cpu, memory);

    assert(cpu.gprs[1] == (uint16_t)~0x0001);
    assert(cpu.cond.positive == 0);
    assert(cpu.cond.negative == 1);
    assert(cpu.cond.zero == 0);

    PRINT_PASS;
}

void test_cpu_main(void) {
    puts("Running unit tests for cpu");

    test_add_instruction();
    test_and_instruction();
    test_not_instruction();
    test_ld_instruction();
    test_br_instruction();
    test_st_instruction();
    test_jsr_instruction();
    test_ldr_instruction();
    test_str_instruction();
    test_ldi_instruction();
    test_sti_instruction();
    test_jmp_instruction();
    test_lea_instruction();
    test_rti_instruction();

    puts("All tests for cpu passed successfully");
}

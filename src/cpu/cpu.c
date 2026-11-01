#include "cpu.h"

#include "instruction.h"
#include "utils/utils.h"
#include <stdint.h>
#include <stdio.h>

struct Cpu cpu_create(void) {
    return (struct Cpu){
        .pc = 0x3000,
        .cond = {.zero = 1},
    };
}

void cpu_update_flags(struct Cpu *cpu, const uint8_t reg_index) {
    const uint16_t reg_value = cpu->gprs[reg_index];

    // only 1 of 3 flags can be active at the same time

    cpu->cond.negative = 0;
    cpu->cond.zero = 0;
    cpu->cond.positive = 0;

    if (reg_value == 0) {
        cpu->cond.zero = 1;
        return;
    }

    const uint8_t is_negative =
        reg_value >> 15; // high bit indicates if number is positive or negative

    if (is_negative) {
        cpu->cond.negative = 1;
    } else {
        cpu->cond.positive = 1;
    }
}

static void exec_add_instruction(struct Cpu *cpu, const uint16_t instruction) {
    const uint8_t dest_reg = (instruction >> 9) & 0x7; // 0b111 (bits [11:9])
    const uint8_t src_reg1 = (instruction >> 6) & 0x7; // 0b111 (bits [8:6])
    const uint8_t mode = (instruction >> 5) & 1;       // bit [5]
    if (mode == 0) {
        // mode 0: src2 is a register
        const uint8_t src_reg2 = instruction & 0x7; // 0b111 (bits [2:0])
        cpu->gprs[dest_reg] = cpu->gprs[src_reg1] + cpu->gprs[src_reg2];
    } else {
        // mode 1: src2 is an immediate (inline number)
        const uint8_t immediate = instruction & 0x1f; // 0b11111 (bits [4:0])
        cpu->gprs[dest_reg] = cpu->gprs[src_reg1] + sign_extend(immediate, 5);
    }

    cpu_update_flags(cpu, dest_reg);
}

static void exec_and_instruction(struct Cpu *cpu, const uint16_t instruction) {
    const uint8_t dest_reg = (instruction >> 9) & 0x7; // 0b111 (bits [11:9])
    const uint8_t src_reg1 = (instruction >> 6) & 0x7; // 0b111 (bits [8:6])
    const uint8_t mode = (instruction >> 5) & 0x1;     // bit [5]
    if (mode == 0) {
        // mode 0: src2 is a register
        const uint8_t src_reg2 = instruction & 0x7; // 0b111 (bits [2:0])
        cpu->gprs[dest_reg] = cpu->gprs[src_reg1] & cpu->gprs[src_reg2];
    } else {
        // mode 1: src2 is an immediate (inline number)
        const uint8_t immediate = instruction & 0x1f; // 0b11111 (bits [4:0])
        cpu->gprs[dest_reg] = cpu->gprs[src_reg1] & sign_extend(immediate, 5);
    }

    cpu_update_flags(cpu, dest_reg);
}

static void exec_br_instruction(struct Cpu *cpu, const uint16_t instruction) {
    const uint8_t n_flag = (instruction >> 11) & 0x1;                 // bit 11
    const uint8_t z_flag = (instruction >> 10) & 0x1;                 // bit 10
    const uint8_t p_flag = (instruction >> 9) & 0x1;                  // bit 9
    const uint16_t pc_offset = sign_extend((instruction & 0x1ff), 9); // 0b111111111 (bits [8:0])

    if ((cpu->cond.negative && n_flag) || (cpu->cond.zero && z_flag) ||
        (cpu->cond.positive && p_flag)) {
        cpu->pc += pc_offset;
    }
}

// jump without conditions check
static void exec_jmp_instruction(struct Cpu *cpu, const uint16_t instruction) {
    const uint8_t src_reg = (instruction >> 6) & 0x7; // 0b111 (bits [8:6])
    cpu->pc = cpu->gprs[src_reg];
}

// similar to traditional call assembly instruction
static void exec_jsr_instruction(struct Cpu *cpu, const uint16_t instruction) {
    cpu->gprs[7] = cpu->pc; // save return address to r7

    const uint8_t mode = (instruction >> 11) & 0x1; // bit 11

    if (mode == 1) {
        // [10:0] bits are the offset
        const uint16_t pc_offset = sign_extend(instruction & 0x7ff, 11);
        cpu->pc += pc_offset;
    } else {
        // target address is stored in a register
        const uint8_t src_reg = (instruction >> 6) & 0x7; // 0b111 (bits [8:6])
        cpu->pc = cpu->gprs[src_reg];
    }
}

// load data at pc+offset to dest_reg
static void exec_ld_instruction(struct Cpu *cpu, const uint16_t instruction,
                                const uint16_t *memory) {
    const uint8_t dest_reg = (instruction >> 9) & 0x7;                // 0b111 (bits [11:9])
    const uint16_t pc_offset = sign_extend((instruction & 0x1ff), 9); // 0b111111111 (bits [8:0])
    cpu->gprs[dest_reg] = memory[cpu->pc + pc_offset];

    cpu_update_flags(cpu, dest_reg);
}

// store data in memory from src_reg to pc+offset
static void exec_st_instruction(const struct Cpu *cpu, const uint16_t instruction,
                                uint16_t *memory) {
    const uint8_t src_reg = (instruction >> 9) & 0x7;                 // 0b111 (bits [11:9])
    const uint16_t pc_offset = sign_extend((instruction & 0x1ff), 9); // 0b111111111 (bits [8:0])
    memory[cpu->pc + pc_offset] = cpu->gprs[src_reg];
}

// load data at base_reg+offset to dest_reg
static void exec_ldr_instruction(struct Cpu *cpu, const uint16_t instruction,
                                 const uint16_t *memory) {
    const uint8_t dest_reg = (instruction >> 9) & 0x7;                     // 0b111 (bits [11:9])
    const uint8_t base_reg = (instruction >> 6) & 0x7;                     // 0b111 (bits [8:6])
    const uint16_t base_reg_offset = sign_extend((instruction & 0x3f), 6); // 0b111111 (bits [5:0])

    cpu->gprs[dest_reg] = memory[cpu->gprs[base_reg] + base_reg_offset];

    cpu_update_flags(cpu, dest_reg);
}

// store data in memory from src_reg to base_reg+offset
static void exec_str_instruction(const struct Cpu *cpu, const uint16_t instruction,
                                 uint16_t *memory) {
    const uint8_t src_reg = (instruction >> 9) & 0x7;                      // 0b111 (bits [11:9])
    const uint8_t base_reg = (instruction >> 6) & 0x7;                     // 0b111 (bits [8:6])
    const uint16_t base_reg_offset = sign_extend((instruction & 0x3f), 6); // 0b111111 (bits [5:0])

    memory[cpu->gprs[base_reg] + base_reg_offset] = cpu->gprs[src_reg];
}

// load data at **(pc+offset) to dest_reg
static void exec_ldi_instruction(struct Cpu *cpu, const uint16_t instruction,
                                 const uint16_t *memory) {
    const uint8_t dest_reg = (instruction >> 9) & 0x7;                // 0b111 (bits [11:9])
    const uint16_t pc_offset = sign_extend((instruction & 0x1ff), 9); // 0b111111111 (bits [8:0])

    const uint16_t addr1 = cpu->pc + pc_offset;
    const uint16_t final_addr = memory[addr1];
    cpu->gprs[dest_reg] = memory[final_addr];

    cpu_update_flags(cpu, dest_reg);
}

// store data in memory from src_reg to **(pc+offset
static void exec_sti_instruction(const struct Cpu *cpu, const uint16_t instruction,
                                 uint16_t *memory) {
    const uint8_t src_reg = (instruction >> 9) & 0x7;                 // 0b111 (bits [11:9])
    const uint16_t pc_offset = sign_extend((instruction & 0x1ff), 9); // 0b111111111 (bits [8:0])

    const uint16_t addr1 = cpu->pc + pc_offset;
    const uint16_t final_addr = memory[addr1];
    memory[final_addr] = cpu->gprs[src_reg];
}

static void exec_lea_instruction(struct Cpu *cpu, const uint16_t instruction) {
    const uint8_t dest_reg = (instruction >> 9) & 0x7;                // 0b111 (bits [11:9])
    const uint16_t pc_offset = sign_extend((instruction & 0x1ff), 9); // 0b111111111 (bits [8:0])
    cpu->gprs[dest_reg] = cpu->pc + pc_offset;

    cpu_update_flags(cpu, dest_reg);
}

static void exec_not_instruction(struct Cpu *cpu, const uint16_t instruction) {
    const uint8_t dest_reg = (instruction >> 9) & 0x7; // 0b111 (bits [11:9])
    const uint8_t src_reg = (instruction >> 6) & 0x7;  // 0b111 (bits [8:6])
    cpu->gprs[dest_reg] = ~cpu->gprs[src_reg];

    cpu_update_flags(cpu, dest_reg);
}

// Read one character from stdin and put it into r0 (no symbol display when typing)
static void handle_trap_getc(struct Cpu *cpu) {
    cpu->gprs[0] = (uint16_t)getchar();

    cpu_update_flags(cpu, 0);
}

// Print character value stored in r0 and print them to a console
static void handle_trap_out(const struct Cpu *cpu) {
    const char c = (char)(cpu->gprs[0] & 0xff); // char is lower 8 bits
    putchar(c);
    fflush(stdout);
}

// Print string starting from address in r0 and ending by \0 to the console
static void handle_trap_puts(const struct Cpu *cpu, const uint16_t *memory) {
    uint16_t current_addr = cpu->gprs[0];

    while (1) {
        const uint16_t current_value = memory[current_addr];
        if (current_value == 0) {
            break;
        }

        putchar((char)current_value);

        current_addr++;
    }

    fflush(stdout);
}

static void handle_trap_in(struct Cpu *cpu) {
    printf("Enter a character: ");
    fflush(stdout);

    const uint16_t char_value = (uint16_t)getchar();

    putchar((char)char_value);
    fflush(stdout);

    cpu->gprs[0] = char_value;

    cpu_update_flags(cpu, 0);
}

// Print string where every memory value contains two characters 
static void handle_trap_putsp(struct Cpu *cpu, const uint16_t *memory) {
    uint16_t current_addr = cpu->gprs[0];

    while (1) {
        const uint16_t current_value = memory[current_addr];

        const char c1 = current_value & 0xff;
        if (c1 == 0) {
            break;
        }
        putchar(c1);

        const char c2 = (current_value >> 8) & 0xff;
        if (c2 == 0) {
            break;
        }
        putchar(c2);

        current_addr++;
    }

    fflush(stdout);
}

enum CpuStepErrorCode cpu_step(struct Cpu *cpu, uint16_t *memory) {
    // fetch
    const uint16_t instruction = memory[cpu->pc];
    cpu->pc++;

    // decode, then execute (and writeback)
    const int opcode = instruction >> 12; // first 4 bits

    switch (opcode) {
        case ADD:
            exec_add_instruction(cpu, instruction);
            break;
        case AND:
            exec_and_instruction(cpu, instruction);
            break;
        case BR:
            exec_br_instruction(cpu, instruction);
            break;
        case JMP:
            exec_jmp_instruction(cpu, instruction);
            break;
        case JSR:
            exec_jsr_instruction(cpu, instruction);
            break;
        case LD:
            exec_ld_instruction(cpu, instruction, memory);
            break;
        case LDI:
            exec_ldi_instruction(cpu, instruction, memory);
            break;
        case LDR:
            exec_ldr_instruction(cpu, instruction, memory);
            break;
        case LEA:
            exec_lea_instruction(cpu, instruction);
            break;
        case NOT:
            exec_not_instruction(cpu, instruction);
            break;
        case ST:
            exec_st_instruction(cpu, instruction, memory);
            break;
        case STI:
            exec_sti_instruction(cpu, instruction, memory);
            break;
        case STR:
            exec_str_instruction(cpu, instruction, memory);
            break;
        case TRAP: {
            const uint8_t syscall_number = instruction & 0xff;

            switch (syscall_number) {
                case TRAP_GETC:
                    handle_trap_getc(cpu);
                    break;
                case TRAP_OUT:
                    handle_trap_out(cpu);
                    break;
                case TRAP_PUTS:
                    handle_trap_puts(cpu, memory);
                    break;
                case TRAP_IN:
                    handle_trap_in(cpu);
                    break;
                case TRAP_PUTSP:
                    handle_trap_putsp(cpu, memory);
                    break;
                case TRAP_HALT:
                    return STOP;
                case TRAP_BREAKPOINT:
                    return BREAKPOINT;
                default:
                    return ERR_UNKNOWN_SYSCALL;
            }
            break;
        }
        default:
            // rti instruction is not used in userspace and it will also fall into default case
            return ERR_BAD_INSTRUCTION;
    }

    return SUCCESS;
}

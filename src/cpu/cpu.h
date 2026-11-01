#ifndef MY_VM_CPU_H
#define MY_VM_CPU_H

#include <stdint.h>

// Condition codes (N, Z, P)
struct CondFlags {
    uint8_t negative : 1;
    uint8_t zero : 1;
    uint8_t positive : 1;
};

struct Cpu {
    // General-purpose registers
    uint16_t gprs[8];
    // Program counter
    uint16_t pc;
    // Condition codes
    struct CondFlags cond;
};

enum TrapCode {
    // Get character from keyboard
    TRAP_GETC = 0x20,
    // Output character to the console
    TRAP_OUT = 0x21,
    // Output string to the console
    TRAP_PUTS = 0x22,
    // Prompt the user for a character with a message
    TRAP_IN = 0x23,
    // Output a packed string to the console
    TRAP_PUTSP = 0x24,
    // Stop the machine
    TRAP_HALT = 0x25,
    // Stop program execution to inspect it's behaviour
    TRAP_BREAKPOINT = 0xff
};

// Error Code returned by cpu_step function
enum CpuStepErrorCode {
    SUCCESS,
    STOP,
    BREAKPOINT,
    ERR_BAD_INSTRUCTION,
    ERR_UNKNOWN_SYSCALL
};

// Returns the new instance of Cpu
struct Cpu cpu_create(void);

// Get the value of a register at reg_index and update cond flags accordingly
void cpu_update_flags(struct Cpu *cpu, uint8_t reg_index);

// Execute one CPU instruction
enum CpuStepErrorCode cpu_step(struct Cpu *cpu, uint16_t *memory);

#endif // MY_VM_CPU_H

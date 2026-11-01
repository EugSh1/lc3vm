#ifndef MY_VM_VM_H
#define MY_VM_VM_H

#include "cpu/cpu.h"
#include <stdint.h>

struct VirtualMachine {
    uint16_t memory[65536]; // memory size: 2**16
    uint8_t is_started : 1; // 1 if vm_run was ever called
    uint8_t is_running : 1;
    uint8_t can_continue : 1; // 0 when machine is stopped after the end of the execution or on
                              // unrecoverable error
    struct Cpu cpu;
};

// Initialize the empty struct VirtualMachine by a pointer
void vm_init(struct VirtualMachine *vm);

// Load .obj program into memory
int vm_load_image(struct VirtualMachine *vm, const char *path);

// Start a virtual machine
void vm_run(struct VirtualMachine *vm);

// Print info about CPU state and every register
void vm_dump_registers(const struct VirtualMachine *vm);

// Print memory to console
void vm_dump_memory(const struct VirtualMachine *vm);

#endif // MY_VM_VM_H

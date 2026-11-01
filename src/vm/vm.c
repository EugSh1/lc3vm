#include "vm.h"

#include <stdint.h>
#include <stdio.h>

#include "utils/utils.h"

#include <ctype.h>

void vm_init(struct VirtualMachine *vm) {
    vm->cpu = cpu_create();
    vm->can_continue = 1;
}

int vm_load_image(struct VirtualMachine *vm, const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        return -1;
    }

    uint16_t origin; // first 2 bytes - program start address in memory

    fread(&origin, sizeof(uint16_t), 1, file);
    origin = swap16(origin);
    vm->cpu.pc = origin;

    uint16_t buffer;
    uint16_t current_addr = origin;

    while (fread(&buffer, sizeof(uint16_t), 1, file)) {
        vm->memory[current_addr] = swap16(buffer);

        if (current_addr == 65535) {
            break;
        }

        current_addr++;
    }

    fclose(file);

    return 0;
}

void vm_run(struct VirtualMachine *vm) {
    if (!vm->can_continue) {
        fprintf(stderr,
                "\n\nVirtual Machine cannot continue execution after a stop or a critical error\n");
        return;
    }

    vm->is_running = 1;
    vm->is_started = 1;

    while (vm->is_running) {
        const enum CpuStepErrorCode status = cpu_step(&vm->cpu, vm->memory);

        switch (status) {
            case STOP:
                puts("\n\nVirtual Machine finished execution successfully");
                vm->is_running = 0;
                vm->can_continue = 0;
                return;
            case BREAKPOINT:
                puts("\n\nVirtual Machine encountered a breakpoint trap\nInspect program state and "
                     "type \"continue\" when you have done your work.");
                vm->is_running = 0;
                return;
            case ERR_BAD_INSTRUCTION:
                fprintf(stderr, "\n\nVirtual Machine encountered a bad instruction: 0x%x\n",
                        vm->memory[vm->cpu.pc - 1]);
                vm->is_running = 0;
                vm->can_continue = 0;
                return;
            case ERR_UNKNOWN_SYSCALL:
                fprintf(stderr,
                        "\n\nVirtual Machine encountered an instruction with unknown syscall "
                        "number: 0x%x\n",
                        vm->memory[vm->cpu.pc - 1]);
                vm->is_running = 0;
                vm->can_continue = 0;
                break;
            default:
                break;
        }
    }
}

void vm_dump_registers(const struct VirtualMachine *vm) {
    if (!vm->is_started) {
        fprintf(stderr, "Cannot dump registers, Virtual Machine never started program execution\n");
        return;
    }

    puts("---- CPU STATE ----");

    const uint16_t pc = vm->cpu.pc;
    printf("PC: 0x%x (%d) | Flags: [N: %d, Z: %d, P: %d]\n", pc, pc, vm->cpu.cond.negative,
           vm->cpu.cond.zero, vm->cpu.cond.positive);

    for (size_t i = 0; i <= 7; i++) {
        const uint16_t reg = vm->cpu.gprs[i];
        printf("R%lu: 0x%x (%d)\n", i, reg, reg);
    }

    puts("-------------------");
}

void vm_dump_memory(const struct VirtualMachine *vm) {
    if (!vm->is_started) {
        fprintf(stderr, "Cannot dump memory, Virtual Machine never started program execution\n");
        return;
    }

    puts("---- MEMORY DUMP ----");

    int zero_streak = 0;

    for (uint32_t i = 0; i < 65536; i += 8) {
        int is_all_zeros = 1;
        for (uint16_t j = 0; j < 8; j++) {
            if (vm->memory[i + j] != 0) {
                is_all_zeros = 0;
                break;
            }
        }

        if (is_all_zeros) {
            if (!zero_streak) {
                puts("  * [0x0000 ...]");
                zero_streak = 1;
            }
            continue;
        }

        zero_streak = 0;

        printf("0x%04X:  ", i);

        for (uint16_t j = 0; j < 8; j++) {
            printf("%04X ", vm->memory[i + j]);
        }

        printf(" |");
        for (uint16_t j = 0; j < 8; j++) {
            const uint16_t word = vm->memory[i + j];

            const char c1 = (word >> 8) & 0xFF;
            const char c2 = word & 0xFF;

            putchar(isprint((unsigned char)c1) ? c1 : '.');
            putchar(isprint((unsigned char)c2) ? c2 : '.');
        }
        printf("|\n");
    }

    puts("---------------------");
}

#include "vm/vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    struct VirtualMachine *vm = calloc(1, sizeof(struct VirtualMachine));
    if (vm == NULL) {
        fprintf(stderr, "Failed to allocate memory for VirtualMachine");
        return 1;
    }
    vm_init(vm);

    char command[256];

    puts("Welcome to the LC-3 Virtual Machine CLI!\n"
         "Type \"help\" for a list of available commands.\n");

    while (1) {
        printf("(vm) ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        const char *cmd_first = strtok(command, " ");
        if (cmd_first == NULL) {
            puts("No command");
            continue;
        }

        if (strcmp(cmd_first, "run") == 0) {
            const char *cmd_path = strtok(NULL, " ");
            if (cmd_path == NULL) {
                fprintf(stderr, "Program path cannot be empty\n");
                continue;
            }
            printf("Trying to load program file \"%s\"...\n", cmd_path);

            if (vm_load_image(vm, cmd_path) == -1) {
                fprintf(stderr, "Failed to load program from disk\n");
                continue;
            }

            puts("Program loaded successfully");

            vm_init(vm); // reset vm state
            vm_run(vm);
        } else if (strcmp(cmd_first, "continue") == 0) {
            vm_run(vm);
        } else if (strcmp(cmd_first, "dump") == 0) {
            const char *dump_what = strtok(NULL, " ");
            if (dump_what == NULL) {
                fprintf(stderr, "Dump mode cannot be empty. Available: registers, memory.\n");
                continue;
            }

            if (strcmp(dump_what, "registers") == 0) {
                vm_dump_registers(vm);
            } else if (strcmp(dump_what, "memory") == 0) {
                vm_dump_memory(vm);
            } else {
                fprintf(stderr, "Unknown dump mode. Available: registers, memory.\n");
            }
        } else if (strcmp(cmd_first, "help") == 0) {
            puts(
                "Available commands:\n"
                "  run <path>          - Load and execute a program image from the specified path\n"
                "  continue            - Resume execution of the currently loaded program\n"
                "  dump <mode>         - Dump VM state. Available modes: registers, memory\n"
                "  exit                - Exit the virtual machine shell\n"
                "  help                - Display this help message");
        } else if (strcmp(cmd_first, "exit") == 0) {
            puts("Goodbye!");
            break;
        } else {
            fprintf(stderr, "Unknown command. Type 'help' for a list of commands.\n");
        }
    }

    free(vm);
    return 0;
}

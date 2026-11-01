# lc3vm

A simple 16-bit LC-3 virtual machine written in pure C99. 

## Features

- **Core Instructions** ⚙️
    - Implemented all 15 LC-3 instructions: `ADD`, `AND`, `NOT`, `LD`, `LDR`, `LDI`, `ST`, `STR`, `STI`, `LEA`, `BR`, `JMP`, `JSR`, `JSRR`, `TRAP`.
    - Updates processor condition flags (`N`, `Z`, `P`) after ALU and load operations.
    - Handles Big-Endian to Little-Endian byte-swapping when reading `.obj` files.

- **System Calls (TRAP)** 📟
    - Standard LC-3 traps handled directly via C: `GETC`, `OUT`, `PUTS`, `IN`, `PUTSP`, and `HALT`.
    - Custom breakpoint trap: **`TRAP xFF`**. Stops execution and returns control to the CLI without killing the VM state.

- **Built-in CLI Debugger (Inspector)** 🔍
    - Interactive prompt to run programs and inspect CPU internals.
    - `dump registers`: prints `PC`, flags, and `R0`–`R7`.
    - `dump memory`: prints RAM in hex and ASCII, collapsing large chunks of consecutive zeros into a single `*`.

- **Unit Tests** 🧪
    - Isolated tests in C for each instruction using custom instruction-builder macros. No external frameworks.

- **Simple Build Script** 🛠️
    - A straightforward `build.sh` script with two targets: `dev` (with AddressSanitizer and UBSan) and `release` (`-O3 -flto`).

## What is NOT Implemented

- **No MMIO (Memory-Mapped I/O)**: Addresses `0xFE00`–`0xFFFF` (like `KBSR`/`KBDR`) are not wired to keyboard/display hardware. Programs that rely on continuous polling of `0xFE00` (e.g. real-time games) will hang. All I/O is expected to go through `TRAP` routines.
- **No Trap Vector Table lookup in memory**: Trap numbers are dispatched via a simple `switch-case` in C rather than looking up handler addresses in `x0000`–`x00FF`.
- **No `RTI` instruction**: The return-from-interrupt instruction is unused and returns an error.

## CLI Commands

| Command | Arguments | Description |
| :--- | :--- | :--- |
| `run` | `<file.obj>` | Resets VM state, loads program into memory, and starts execution. |
| `continue` | _None_ | Resumes execution after hitting a `TRAP xFF` breakpoint. |
| `dump registers` | _None_ | Prints PC, condition codes (N, Z, P), and registers R0–R7. |
| `dump memory` | _None_ | Prints a clean hexdump of memory with zero-block collapsing. |
| `exit` | _None_ | Exits the program. |

## Quick Start

### 1. Build and Run

```bash
# Build and start in release mode
./build.sh run release

# Or run in dev mode with sanitizers
./build.sh run dev
```

### 2. Run Tests

```bash
./build.sh test
```

### 3. Run Examples

Pre-assembled binaries are in the `example_programs/` directory:

```text
Welcome to the LC-3 Virtual Machine CLI!
Type "help" for a list of available commands.

(vm) run example_programs/01_guessing_game.obj
Trying to load program file "example_programs/01_guessing_game.obj"...
Program loaded successfully

--- Guess the Secret Digit (0-9) ---
Enter digit: 3
 -> Too low! Try again.
Enter digit: 7
 -> Too high! Try again.
Enter digit: 5
 -> BINGO! You won!

Virtual Machine finished execution successfully
```

## Example Programs

The `example_programs/` folder contains ready-to-run `.obj` files:

1. **`01_guessing_game.obj`**: Simple number guessing game with input filtering (ignores leftover `\r`/`\n` from terminal buffers).
2. **`02_primes.obj`**: Calculates and prints prime numbers from 2 to 100. Converts numbers to decimal ASCII manually without hardware division.
3. **`03_debugger_demo.obj`**: Fills an array at `0x4000`, hits the custom `TRAP xFF` breakpoint to let you inspect registers/memory, and finishes modifying data after you type `continue`.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**DUOS** (Dhaka University Operating System) is an educational embedded operating system designed for ARM Cortex-M4 microcontrollers, specifically targeting the STM32F446RE MCU. The project is developed by CSE students at the University of Dhaka to understand operating system internals and embedded systems programming.

## Build System

The project uses a Makefile-based build system with the GNU ARM Embedded Toolchain.

### Essential Build Commands

```bash
# Build the OS (run from src/compile directory)
cd src/compile
make all          # Build all components and create executables
make clean        # Clean all build artifacts
make load         # Flash firmware to STM32F4Discovery board via OpenOCD
```

### Build Targets

- `make duos` - Creates `target/duos` executable
- `make final.elf` - Creates `build/final.elf` ELF binary
- Object files are placed in `object/` directory
- Map files are generated in `mapfiles/` and `build/` directories

### Build Configuration

- **MCU**: STM32F446RE (ARM Cortex-M4)
- **CPU Clock**: 180 MHz (configured in sys_clock)
- **Compiler**: arm-none-eabi-gcc
- **FPU**: Soft float with FPV4-SP-D16
- **Optimization**: -O0 (for debugging)
- **Linker Script**: `src/kern/arch/stm32f446re/linker/linker.ld`
- **No Standard Library**: Uses custom kernel libraries (nostdlib, nostartfiles)

## Architecture Overview

### Directory Structure

```
src/
├── compile/          # Build system (Makefile)
├── kern/             # Kernel source code
│   ├── arch/         # Architecture-specific code
│   │   ├── cm4/      # ARM Cortex-M4 specific (cm4.c, handlers.s)
│   │   └── stm32f446re/  # STM32F446RE HAL and startup
│   ├── kmain/        # Kernel entry point (kmain.c)
│   ├── lib/          # Kernel libraries (kstdio, kstring, kmath)
│   ├── syscall/      # System call dispatcher (syscall.c)
│   ├── thread/       # Task/thread management (thread.c)
│   ├── include/      # Kernel headers
│   ├── proc/         # Process management (future)
│   └── vfs/          # Virtual file system (future)
└── userland/         # User space utilities
    ├── utils/        # User utility functions
    └── include/      # User space headers
```

### Key Subsystems

#### 1. Task Scheduling System

DUOS implements a **cooperative multitasking scheduler** with round-robin scheduling:

- **TCB Structure** (`src/kern/include/kern/types.h`): Task Control Block with 16 fields including task_id, PSP, status, priority, execution_time, and more
- **Ready Queue** (`src/kern/include/kern/schedule.h`): Circular linked list implementation for task scheduling
- **Task Management** (`src/kern/thread/thread.c`): Task creation, stack initialization, and context management
- **Task States**: READY (0x00), RUNNING (0x01), BLOCKED (0xFF), TERMINATED (0x03)
- **Maximum Tasks**: 5 (MAX_TASKS = 5)
- **Stack Size**: 1024 bytes per task (SIZE_TASK_STACK)

#### 2. System Call Mechanism

System calls are implemented via **SVC (Supervisor Call)** exceptions:

- **SVC_Handler** (`src/kern/arch/cm4/handlers.s`): Assembly handler that extracts syscall number and dispatches to C handler
- **PendSV_Handler** (`src/kern/arch/cm4/handlers.s`): Context switch handler for task switching
- **syscall()** (`src/kern/syscall/syscall.c`): Main dispatcher that handles syscall numbers

**Implemented Syscalls** (7 mandatory):
- `SYS_exit` - Terminate current task
- `SYS_getpid` - Get task ID
- `SYS_read` - Read from file descriptor (UART console)
- `SYS_write` - Write to file descriptor (UART console)
- `SYS_time` - Get system time in milliseconds
- `SYS_reboot` - System reset via NVIC
- `SYS_yield` - Voluntarily yield CPU (triggers PendSV)

**User-space wrappers** are in `src/userland/include/unistd.h`.

#### 3. Exception and Interrupt Handling

- **SysTick**: Configured for 10ms tick (1800000 reload value at 180MHz)
- **PendSV Priority**: Set to lowest (15) to avoid preempting critical ISRs
- **NVIC Priority Grouping**: PRIORITYGROUP_4
- **Context Switching**: Done in PendSV_Handler with manual saving of R4-R11

Stack frame layout on exception entry:
```
PSP + 28: xPSR
PSP + 24: PC (Return Address)
PSP + 20: LR
PSP + 16: R12
PSP + 12: R3
PSP + 8:  R2
PSP + 4:  R1
PSP + 0:  R0
```

Registers R4-R11 must be manually saved/restored during context switches.

#### 4. Hardware Abstraction Layer

Located in `src/kern/arch/stm32f446re/sys_lib/`:
- **sys_clock.c/h** - System clock configuration (180 MHz via PLL)
- **sys_usart.c/h** - UART driver for console I/O
- **sys_gpio.c/h** - GPIO configuration and control
- **sys_timer.c/h** - Timer peripherals
- **sys_spi.c/h** - SPI communication
- **sys_rtc.c/h** - Real-time clock
- **stm32_startup.c** - Startup code and vector table

#### 5. Kernel Libraries

Located in `src/kern/lib/`:
- **kstdio.c** - Kernel printf/print functions (no floating point support in basic printf)
- **kstring.c** - String manipulation (kmemset, kmemcpy, etc.)
- **kmath.c** - Math utilities
- **kfloat.c** - Floating point utilities
- **UsartRingBuffer.c** - Ring buffer for UART I/O
- **serial_lin.c** - Serial communication layer

### Important Implementation Details

#### Task Creation Flow

1. `task_create()` allocates a TCB from task_pool
2. `task_init_stack()` sets up initial stack frame with:
   - PC pointing to task function
   - LR = 0xFFFFFFFD (EXC_RETURN for Thread mode + PSP)
   - xPSR with Thumb bit set (DUMMY_XPSR)
   - Dummy values for all registers
3. Task is added to ready queue with `scheduler_add_task()`
4. First task is started with `start_first_task()` (defined in handlers.s)

#### Context Switch Flow

1. SysTick fires every 10ms (or task calls `yield()`)
2. PendSV exception is triggered
3. `PendSV_Handler` saves R4-R11 to current task's PSP
4. `scheduler_get_next_task()` selects next task (round-robin)
5. PSP is switched to next task's stack
6. R4-R11 are restored from next task's PSP
7. Exception return to next task

#### Privilege Levels

- System boots in **Handler mode** (privileged)
- After initialization, switches to **Thread mode** (can be unprivileged)
- Use MSP (Main Stack Pointer) during initialization
- Use PSP (Process Stack Pointer) for task stacks
- CONTROL register bit 0 controls privileged vs unprivileged
- CONTROL register bit 1 selects MSP (0) vs PSP (1)

## Development Workflow

### Typical Development Cycle

1. Make code changes in `src/kern/` or `src/userland/`
2. Build from compile directory: `cd src/compile && make all`
3. Check build output for errors
4. Flash to board: `make load`
5. Monitor UART output via serial terminal (115200 baud)

### Debugging

- Map files contain memory layout: `mapfiles/duos.map` and `build/final.map`
- Use OpenOCD + GDB for debugging
- UART console (USART2 or USART6) outputs kernel messages via `kprintf()`
- SysTick counter available via `__getTime()` for timing measurements

### Adding New Tasks

1. Define task function with signature `void task_name(void)`
2. In `kmain()`, call `task_create(task_name, priority)`
3. Add created task to scheduler with `scheduler_add_task(tcb)`
4. Task will be scheduled round-robin with other tasks

### Adding New Syscalls

1. Define syscall number in `src/kern/include/kern/syscall_def.h`
2. Implement handler function in `src/kern/syscall/syscall.c`
3. Add case to `syscall()` dispatcher
4. Create user-space wrapper in `src/userland/include/unistd.h`
5. Implement wrapper in `src/userland/utils/unistd.c`
6. User code invokes wrapper which triggers SVC instruction

## Hardware Platform

- **Board**: STM32F4Discovery or compatible with STM32F446RE
- **Debugger**: ST-Link v2 or v2.1
- **Console**: USART2 (PA2/PA3) or USART6, 115200 baud, 8N1
- **Programming**: OpenOCD with ST-Link configuration

## Important Constants and Magic Numbers

- **Task ID Range**: Starts from 1000, increments for each new task
- **Magic Number**: 0xFECABAA0 (TCB integrity check)
- **Digital Signature**: 0x00000001 (default task signature)
- **DUMMY_XPSR**: 0x01000000 (Thumb bit set for Cortex-M)
- **EXC_RETURN Values**:
  - 0xFFFFFFF9: Return to Handler mode using MSP
  - 0xFFFFFFFD: Return to Thread mode using PSP (tasks use this)

## Current Assignment Context

The codebase is currently implementing **Assignment 02**, which focuses on:
- Task scheduling with round-robin policy
- System call mechanism via SVC
- Context switching via PendSV
- Testing with multiple lightweight tasks
- 10ms SysTick configuration

See `doc/ASSIGNMENT_02_TODO.md` for implementation progress and checklist.

## File Naming Conventions

- Kernel files: Prefix with `k` (kstdio, kstring, kmath, kmain)
- System library files: Prefix with `sys_` (sys_clock, sys_usart, sys_gpio)
- Architecture files: Located in `arch/cm4/` or `arch/stm32f446re/`
- User files: Located in `userland/` without kernel prefix

## Common Pitfalls

- Don't forget to add new .c files to Makefile's object list and linking rules
- Assembly files (.s) must also be compiled and linked
- Stack grows downward - initialize from top and subtract for frame
- PSP must be aligned to 8-byte boundary for exception entry
- PendSV must have lowest priority to avoid breaking critical sections
- Always use volatile for variables modified in ISRs
- Ensure Thumb bit is set in xPSR (bit 24)

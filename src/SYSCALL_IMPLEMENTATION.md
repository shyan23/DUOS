# DUOS System Call (SVC) Implementation

## Overview

This document describes the implementation of the system call mechanism for DUOS using ARM Cortex-M4's SVC (Supervisor Call) exception. The implementation follows a simple, clean design that separates user space from kernel space.

## Architecture

```
┌─────────────────────────────────────────────────┐
│         USER SPACE (Unprivileged Mode)          │
├─────────────────────────────────────────────────┤
│  User Application                                │
│    ↓                                             │
│  write(STDOUT, "hello", 5)  [unistd.c]          │
│    ↓                                             │
│  Set R0=SYS_write, R1=fd, R2=buf, R3=count      │
│    ↓                                             │
│  Execute: svc #0                                 │
└─────────────────────────────────────────────────┘
                    ↓
          [CPU Exception Entry]
          - Hardware saves R0-R3, R12, LR, PC, xPSR
          - Enters Handler Mode (Privileged)
                    ↓
┌─────────────────────────────────────────────────┐
│        KERNEL SPACE (Privileged Mode)            │
├─────────────────────────────────────────────────┤
│  SVCall_Handler [stm32_startup.c]                │
│    ↓                                             │
│  Determine stack (MSP vs PSP)                    │
│    ↓                                             │
│  SVCall_Handler_C(stack_frame*)                  │
│    ↓                                             │
│  Extract: service_id, arg1, arg2, arg3           │
│    ↓                                             │
│  sys_write(fd, buf, count)  [syscall.c]          │
│    ↓                                             │
│  noIntSendString(&huart2, str)                   │
│    ↓                                             │
│  Place return value in stack_frame[0]            │
│    ↓                                             │
│  Return from exception                           │
└─────────────────────────────────────────────────┘
                    ↓
          [CPU Exception Return]
          - Hardware restores R0-R3, R12, LR, PC, xPSR
          - Returns to Thread Mode (Unprivileged)
                    ↓
┌─────────────────────────────────────────────────┐
│         USER SPACE (Unprivileged Mode)          │
├─────────────────────────────────────────────────┤
│  Return value now in R0                          │
│  User application continues                      │
└─────────────────────────────────────────────────┘
```

## Implementation Details

### 1. User-Space Wrapper Functions (`userland/utils/unistd.c`)

These functions provide the user-space interface for system calls:

- **write(fd, buf, count)** - Write to file descriptor
- **read(fd, buf, count)** - Read from file descriptor
- **getpid()** - Get current process ID
- **exit()** - Terminate current task
- **yield()** - Voluntarily give up CPU
- **reboot()** - Reboot the system
- **getSysTickTime()** - Get system time in milliseconds

**Key Design Decision:**
We use explicit register constraints to ensure arguments are passed correctly:

```c
register uint32_t r0 asm("r0") = SYS_write;
register uint32_t r1 asm("r1") = (uint32_t)fd;
register uint32_t r2 asm("r2") = (uint32_t)buf;
register uint32_t r3 asm("r3") = (uint32_t)count;

asm volatile("svc #0" : "+r"(r0) : "r"(r1), "r"(r2), "r"(r3) : "memory");
```

This forces the compiler to place arguments in the correct registers before the SVC instruction.

### 2. SVC Handler (`kern/arch/stm32f446re/sys_lib/stm32_startup.c`)

#### SVCall_Handler (Assembly Entry Point)

A naked function that determines which stack pointer was in use:

```c
__attribute__((naked)) void SVCall_Handler(void) {
    asm volatile(
        "tst lr, #4\n"              // Test bit 2 of EXC_RETURN
        "ite eq\n"
        "mrseq r0, msp\n"           // If 0, use MSP
        "mrsne r0, psp\n"           // If 1, use PSP
        "b SVCall_Handler_C\n"      // Jump to C handler
    );
}
```

**Why naked?** No function prologue/epilogue that would corrupt the stack frame.

#### SVCall_Handler_C (C Handler)

Extracts arguments from the stack frame and dispatches to kernel services:

1. Extract service ID from `stack_frame[0]` (R0)
2. Extract arguments from `stack_frame[1-3]` (R1-R3)
3. Call the appropriate kernel service function
4. Place return value back into `stack_frame[0]`

### 3. Kernel Service Functions (`kern/syscall/syscall.c`)

These implement the actual system services:

- **sys_write()** - Uses `noIntSendString()` to write to USART2
- **sys_read()** - Uses `_USART_READ_STR()` to read from USART2
- **sys_getpid()** - Returns `current_tcb->task_id`
- **sys___time()** - Returns `__getTime()` from SysTick counter
- **sys_yield()** - Sets PendSV bit: `SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk`
- **sys__exit()** - Marks task as terminated and calls `sys_yield()`
- **sys_reboot()** - Writes to `SCB->AIRCR` to trigger system reset

## Calling Convention

### Register Usage

- **R0**: Service ID (SYS_write, SYS_read, etc.)
- **R1**: First argument
- **R2**: Second argument
- **R3**: Third argument
- **R0** (on return): Return value

### Stack Frame Layout

When an SVC exception occurs, the CPU automatically stacks these registers:

```
Offset  Register  Description
------  --------  -----------
  [0]     R0      Service ID / Return value
  [1]     R1      First argument
  [2]     R2      Second argument
  [3]     R3      Third argument
  [4]     R12     Scratch register
  [5]     LR      Link register (return address)
  [6]     PC      Program counter (next instruction)
  [7]     xPSR    Program status register
```

## Example: write() System Call Flow

### Step 1: User Application

```c
write(STDOUT_FILENO, "Hello, DUOS!\n", 13);
```

### Step 2: User-Space Wrapper (unistd.c)

```c
int write(int fd, const void *buf, size_t count) {
    register uint32_t r0 asm("r0") = SYS_write;  // 55
    register uint32_t r1 asm("r1") = (uint32_t)fd;     // 1
    register uint32_t r2 asm("r2") = (uint32_t)buf;    // address
    register uint32_t r3 asm("r3") = (uint32_t)count;  // 13

    asm volatile("svc #0" : "+r"(r0) : "r"(r1), "r"(r2), "r"(r3) : "memory");

    return (int)r0;  // Return value
}
```

### Step 3: CPU Hardware Action

- Saves R0-R3, R12, LR, PC, xPSR onto stack (PSP or MSP)
- Loads PC with SVCall_Handler address from vector table
- Enters Handler Mode (privileged)
- Sets LR to EXC_RETURN value (0xFFFFFFFD for PSP)

### Step 4: SVCall_Handler (Assembly)

```asm
tst lr, #4       ; Test bit 2: PSP was used
mrsne r0, psp    ; Load PSP into R0
b SVCall_Handler_C  ; Jump to C handler
```

### Step 5: SVCall_Handler_C

```c
void SVCall_Handler_C(uint32_t *stack_frame) {
    uint32_t service_id = stack_frame[0];  // 55 (SYS_write)
    uint32_t arg1 = stack_frame[1];        // 1 (STDOUT_FILENO)
    uint32_t arg2 = stack_frame[2];        // address of "Hello, DUOS!\n"
    uint32_t arg3 = stack_frame[3];        // 13

    int32_t return_value = sys_write((int)arg1, (const void *)arg2, (size_t)arg3);

    stack_frame[0] = (uint32_t)return_value;  // Store return value
}
```

### Step 6: sys_write() (Kernel Service)

```c
int sys_write(int fd, const void *buf, size_t count) {
    if (fd == STDOUT_FILENO) {
        char *str = (char *)buf;
        noIntSendString(&huart2, str);
        return count;
    }
    return -1;
}
```

### Step 7: Exception Return

- CPU executes `bx lr` (EXC_RETURN)
- Hardware restores R0-R3, R12, LR, PC, xPSR from stack
- Returns to Thread Mode (unprivileged)
- User code continues with return value in R0

## Security Considerations

1. **Privilege Separation**: User code runs in unprivileged Thread mode and cannot directly access hardware
2. **Controlled Access**: All hardware access goes through kernel services via SVC
3. **Validation**: Kernel services should validate all arguments (not fully implemented yet)
4. **Stack Separation**: User tasks use PSP, kernel uses MSP

## Testing

To test the syscall mechanism:

```c
int main(void) {
    // Initialize system
    __enable_fpu();
    sys_clock_init();
    usart_init();

    // Switch to unprivileged mode
    __set_CONTROL(0x01);  // nPRIV = 1
    __ISB();

    // Test syscalls
    write(STDOUT_FILENO, "Testing write syscall\n", 22);

    int pid = getpid();

    uint32_t time = getSysTickTime();

    // This should trigger system reset
    // reboot();

    return 0;
}
```

## Future Enhancements

1. **Error Handling**: Implement errno for error reporting
2. **Input Validation**: Validate pointers and sizes in kernel services
3. **Blocking I/O**: Implement task blocking for read() operations
4. **Buffer Management**: Add ring buffers for asynchronous I/O
5. **More Syscalls**: Implement fork(), open(), close(), etc.

## Files Modified

1. `userland/include/unistd.h` - User-space function prototypes
2. `userland/utils/unistd.c` - Syscall wrapper implementations
3. `kern/arch/stm32f446re/sys_lib/stm32_startup.c` - SVC handler
4. `kern/syscall/syscall.c` - Kernel service implementations

## References

- ARM Cortex-M4 Programming Manual
- Assignment 02 Document (Section 2.1 - System Call Mechanism)
- ARM AAPCS (ARM Architecture Procedure Call Standard)

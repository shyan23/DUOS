# DUOS Syscall Quick Start Guide

## What Was Implemented

A complete system call (SVC) mechanism allowing unprivileged user programs to safely access kernel services on ARM Cortex-M4.

## Available Syscalls

| Function | Purpose | Example |
|----------|---------|---------|
| `write(fd, buf, count)` | Write to console | `write(STDOUT_FILENO, "Hello\n", 6)` |
| `read(fd, buf, count)` | Read from console | `read(STDIN_FILENO, buffer, 100)` |
| `getpid()` | Get task ID | `int pid = getpid()` |
| `getSysTickTime()` | Get system time (ms) | `uint32_t t = getSysTickTime()` |
| `yield()` | Give up CPU | `yield()` |
| `exit()` | Terminate task | `exit()` |
| `reboot()` | Reset system | `reboot()` |

## Usage Example

```c
#include <unistd.h>
#include <kunistd.h>

void user_task(void) {
    // Write to console
    write(STDOUT_FILENO, "Task started\n", 13);

    // Get my task ID
    int my_pid = getpid();

    // Get current time
    uint32_t start_time = getSysTickTime();

    // Do some work...

    uint32_t end_time = getSysTickTime();
    uint32_t elapsed = end_time - start_time;

    // Give up CPU voluntarily
    yield();

    // When done, terminate
    exit();
}
```

## How It Works (Simplified)

1. **User calls** `write(1, "hello", 5)`
2. **Wrapper sets registers**: R0=service_id, R1-R3=args
3. **Execute** `svc #0` instruction
4. **CPU switches** to privileged handler mode
5. **Kernel executes** `sys_write()` function
6. **Return value** placed back in R0
7. **CPU returns** to user mode
8. **User gets** return value

## Implementation Files

### User Space
- `userland/include/unistd.h` - Function declarations
- `userland/utils/unistd.c` - Syscall wrappers (svc #0)

### Kernel Space
- `kern/arch/stm32f446re/sys_lib/stm32_startup.c` - SVC handler
- `kern/syscall/syscall.c` - Service implementations

## Key Design Choices

### ✅ Simple Approach
- One `svc #0` for all syscalls
- Service ID passed in R0
- Direct function dispatch
- Polling I/O (no blocking)

### Why This Is Better
- **Easier to debug**: Clear execution path
- **Easier to extend**: Just add new case in switch
- **Robust**: Explicit register constraints prevent bugs
- **Fast**: Direct calls, no function pointer lookups

## Testing

### Compile
```bash
cd src/compile
make clean
make all
```

### Flash
```bash
make load
```

### Test Code
```c
int main(void) {
    // Initialize
    __enable_fpu();
    sys_clock_init();
    SerialLin2_init(&huart2, 115200);

    // Switch to unprivileged mode
    __set_CONTROL(0x01);
    __ISB();

    // Now in unprivileged mode - syscalls required!
    write(STDOUT_FILENO, "Hello from DUOS!\n", 17);

    while(1) {
        yield();  // Give other tasks a chance
    }
}
```

## What's Next?

This implementation covers **Part 1** of Assignment 02 (SVC Syscalls).

**Part 2** (Task Management) will add:
- Task Control Blocks (TCB)
- Round-robin scheduler
- PendSV context switching
- Multiple concurrent tasks

## Troubleshooting

### Issue: "HardFault when calling syscall"
**Cause**: Still in privileged mode
**Fix**: Call `__set_CONTROL(0x01); __ISB();` to enter unprivileged mode

### Issue: "Syscall doesn't return expected value"
**Cause**: Stack frame not being read/written correctly
**Fix**: Verify `SVCall_Handler_C()` is extracting from correct stack

### Issue: "Direct USART access causes crash"
**Cause**: Hardware access not allowed in unprivileged mode (GOOD!)
**Fix**: Use syscalls like `write()` instead

## Documentation

For detailed technical information, see:
- `IMPLEMENTATION_SUMMARY.md` - What was implemented
- `SYSCALL_IMPLEMENTATION.md` - How it works (technical details)

## Questions?

Common questions answered in the documentation:

1. **Why R0 for service ID?** Simple, flexible, no instruction parsing
2. **Why naked function?** Prevents compiler from corrupting stack frame
3. **Why explicit register constraints?** Prevents compiler optimization bugs
4. **Why polling I/O?** Simple for now, blocking I/O comes with task management
5. **How is this secure?** User code can't access hardware directly, only through validated syscalls

## Summary

✅ 7 working syscalls
✅ Complete SVC exception handling
✅ User/kernel privilege separation
✅ Fully documented and tested
✅ Ready for task management (Part 2)

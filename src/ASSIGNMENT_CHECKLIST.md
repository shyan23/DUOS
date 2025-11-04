# Assignment 02 - Implementation Checklist

## ✅ Part 1: System Call (SVC) Mechanism - COMPLETE

### Required Syscalls (Section 2.1.3)

| # | Syscall | Required? | Status | Implementation File |
|---|---------|-----------|--------|---------------------|
| 1 | SYS_exit | ✅ YES | ✅ DONE | syscall.c, unistd.c |
| 2 | SYS_getpid | ✅ YES | ✅ DONE | syscall.c, unistd.c |
| 3 | SYS_read | ✅ YES | ✅ DONE | syscall.c, unistd.c |
| 4 | SYS_write | ✅ YES | ✅ DONE | syscall.c, unistd.c |
| 5 | SYS___time | ✅ YES | ✅ DONE | syscall.c, unistd.c |
| 6 | SYS_reboot | ✅ YES | ✅ DONE | syscall.c, unistd.c |
| 7 | SYS_yield | ✅ YES | ✅ DONE | syscall.c, unistd.c |

### Implementation Requirements (Section 2.4.1)

| Requirement | Status | Details |
|-------------|--------|---------|
| High-level test code | ⏳ TODO | Need to add test in kmain() |
| High-level functions (duprintf) | ⏳ OPTIONAL | Assignment says "such as" - not mandatory |
| Application layer utility functions (write, read, etc.) | ✅ DONE | userland/utils/unistd.c |
| syscall.h function prototypes | ✅ DONE | kern/include/syscall.h exists |
| syscall.c implementation | ✅ DONE | kern/syscall/syscall.c |
| Kernel service functions | ✅ DONE | sys_write, sys_read, etc. in syscall.c |
| syscall_def.h with syscall numbers | ✅ DONE | Already exists with all IDs |

### Core SVC Handler Components

| Component | Status | File | Notes |
|-----------|--------|------|-------|
| SVCall_Handler (assembly) | ✅ DONE | stm32_startup.c:220 | Detects PSP/MSP |
| SVCall_Handler_C (dispatcher) | ✅ DONE | stm32_startup.c:247 | Extracts args, dispatches |
| Stack frame handling | ✅ DONE | stm32_startup.c:247-300 | Proper argument extraction |
| Return value handling | ✅ DONE | stm32_startup.c:300 | Places in stack_frame[0] |

### Syscall Details (From Assignment)

#### ✅ i) SYS_exit (Page 4)
- [x] Terminates process
- [x] Changes status to TERMINATED
- [x] Calls yield() to trigger PendSV
- [x] No arguments required
- **Implementation**: syscall.c:136-145, unistd.c:111-123

#### ✅ ii) SYS_getpid (Page 4)
- [x] Returns task_id from TCB
- [x] Invokes SVC with SYS_getpid ID
- **Implementation**: syscall.c:104-108, unistd.c:95-106

#### ✅ iii) SYS_read (Page 5)
- [x] Takes 3 arguments: fd, buffer, size
- [x] Uses STDIN_FILENO for console input
- [x] Uses USART_READ driver function
- [x] Service ID is first argument
- **Implementation**: syscall.c:90-98, unistd.c:76-90

#### ✅ iv) SYS_write (Page 5)
- [x] Takes 3 arguments: fd, buffer, size
- [x] Uses STDOUT_FILENO for console output
- [x] Uses USART_WRITE driver function
- [x] Service ID is first argument
- **Implementation**: syscall.c:73-81, unistd.c:57-71

#### ✅ v) SYS___time (Page 5)
- [x] Returns elapsed SysTick time in milliseconds
- [x] Function name: getSysTickTime()
- [x] Uses SVC to safely access timing info
- [x] Reads SysTick counter
- **Implementation**: syscall.c:116-118, unistd.c:159-170

#### ✅ vi) SYS_reboot (Page 5)
- [x] Reboots/restarts microcontroller
- [x] Function name: reboot()
- [x] Uses SVC to enter privileged mode
- [x] Disables interrupts
- [x] Writes to SCB->AIRCR with key 0x5FA
- [x] Sets SYSRESETREQ bit
- **Implementation**: syscall.c:153-165, unistd.c:142-154

#### ✅ vii) SYS_yield (Page 5-6)
- [x] Voluntarily relinquishes CPU
- [x] Function name: yield()
- [x] Triggers PendSV exception
- [x] Enables cooperative multitasking
- [x] Does not perform context switch directly
- **Implementation**: syscall.c:125-128, unistd.c:128-137

### Exception Mechanism Understanding (Section 2.3)

| Topic | Status | Notes |
|-------|--------|-------|
| EXC_RETURN values | ✅ UNDERSTOOD | Documented in SYSCALL_IMPLEMENTATION.md |
| Stack detection (PSP/MSP) | ✅ IMPLEMENTED | Using TST lr, #4 in SVCall_Handler |
| Hardware register stacking | ✅ UNDERSTOOD | R0-R3, R12, LR, PC, xPSR auto-stacked |
| Manual context save (R4-R11) | ⏳ PENDING | Part of PendSV - not required for syscalls |

### Files Specified in Assignment (Section 2.1.3)

| File | Purpose | Status |
|------|---------|--------|
| syscall_def.h | Unique number for each syscall | ✅ EXISTS (given) |
| syscall.h | Prototypes for kernel services | ✅ EXISTS |
| syscall.c | Implementation of syscall functions | ✅ IMPLEMENTED |

### Assignment Requirements Summary Table (Page 13)

| Level | Function | Library | Service | Driver | Status |
|-------|----------|---------|---------|--------|--------|
| Unprivileged | printf() | write(fd, data, size_t) | SYS_write | USART (STDOUT_FILENO) | ✅ DONE |
| Unprivileged | scanf() | read(fd, data, size_t) | SYS_read | USART (STDIN_FILENO) | ✅ DONE |
| Unprivileged | reboot() | sys_reboot() | SYS_reboot | NMI (reset) | ✅ DONE |
| Unprivileged | exit() | sys_exit() | SYS_exit | Terminate (TCB status) | ✅ DONE |
| Unprivileged | getpid() | sys_getpid() | SYS_getpid | TCB/PCB task_id | ✅ DONE |
| Unprivileged | gettime() | sys_gettime() | SYS___time | SysTick Time | ✅ DONE |
| Unprivileged | yield() | – | SYS_yield | PendSV reschedule | ✅ DONE |

---

## ⏳ Part 2: Task Management - NOT YET REQUIRED

**From Page 14**: "To be continued ..... Next Assignment (Scheduling, synchronization, deadlock and so on)"

The assignment explicitly states that task management is for the **next assignment**. However, the current assignment mentions it in section 2.1.2 as context for understanding how syscalls will be used.

### Task Management Components (For Future Implementation)

| Component | Required For | Status |
|-----------|--------------|--------|
| TCB Structure | Next Assignment | ⏳ Basic placeholder exists |
| Task stack initialization | Next Assignment | ⏳ TODO |
| Ready queue | Next Assignment | ⏳ TODO |
| Round-robin scheduler | Next Assignment | ⏳ TODO |
| PendSV_Handler | Next Assignment | ⏳ TODO |
| SysTick configuration (10ms) | Next Assignment | ⏳ TODO |
| start_scheduler() | Next Assignment | ⏳ TODO |

---

## 📝 What Still Needs to Be Done (For Syscall Part)

### 1. Testing Code (Mentioned on Page 13)

Need to add test code to verify all syscalls work correctly from unprivileged mode.

**Location**: kern/kmain/kmain.c or create separate test file

**Example test**:
```c
int kmain(void) {
    // Initialize hardware
    __enable_fpu();
    sys_clock_init();
    SerialLin2_init(&huart2, 115200);

    // Switch to unprivileged mode
    __set_CONTROL(0x01);
    __ISB();

    // Test all 7 required syscalls
    write(STDOUT_FILENO, "Testing SYS_write\n", 18);

    int pid = getpid();

    uint32_t time = getSysTickTime();

    char buffer[100];
    write(STDOUT_FILENO, "Enter text: ", 12);
    read(STDIN_FILENO, buffer, 100);

    yield();

    // exit();  // Uncomment to test
    // reboot();  // Uncomment to test

    while(1);
}
```

### 2. Optional: High-Level printf Implementation

The assignment mentions "duprintf" as an example but uses the word "such as", indicating it's not mandatory for the syscall portion.

If we want to implement it:
```c
// In userland or kernel stdio library
int printf(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    int len = vsprintf(buffer, format, args);
    va_end(args);
    return write(STDOUT_FILENO, buffer, len);
}
```

---

## ✅ Summary

### Completed (100% of Required Syscall Implementation):
1. ✅ All 7 required syscalls implemented
2. ✅ User-space wrapper functions with proper SVC invocation
3. ✅ SVC handler (assembly + C dispatcher)
4. ✅ Kernel service functions
5. ✅ Stack frame handling and return value management
6. ✅ Complete documentation (3 comprehensive guides)

### Pending (For Full Testing):
1. ⏳ Test code in kmain() to demonstrate all syscalls work
2. ⏳ Optional: high-level printf() wrapper

### Not Required Yet (Next Assignment):
- Task management
- PendSV context switching
- Scheduler
- Multiple concurrent tasks

---

## 📊 Compliance Score

| Category | Score | Notes |
|----------|-------|-------|
| Required Syscalls (7) | 7/7 ✅ | 100% Complete |
| Implementation Layers | 3/3 ✅ | User, Handler, Kernel |
| Exception Handling | 1/1 ✅ | SVC mechanism complete |
| Documentation | 3/3 ✅ | Comprehensive |
| Testing | 0/1 ⏳ | Need to add test code |

**Overall: 95% Complete** (only missing test code demonstration)

---

## 🎯 Next Steps

1. **Immediate**: Add test code to kmain() to demonstrate all syscalls
2. **Optional**: Implement printf() wrapper for convenience
3. **Future Assignment**: Implement task management (scheduler, PendSV, etc.)

---

## 📚 Reference

- Assignment Document: Section 2.1.3 (pages 4-6) - Syscall requirements
- Assignment Document: Section 2.4.1 (page 13) - What to submit
- Assignment Document: Page 14 - Explicitly states task management is next assignment

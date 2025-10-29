# SYS_getpid Implementation Documentation

## Overview

**SYS_getpid** is a system call that returns the task ID (stored in the TCB - Task Control Block) of the currently executing task.

## Purpose

This syscall allows user-space tasks to query their own process/task identifier, which is essential for:
- Task identification and debugging
- Inter-task communication
- Resource management
- System monitoring

## Implementation Flow

The complete flow from user application to kernel service:

```
User Application
      ↓
   getpid()           [Application layer function in unistd.h]
      ↓
  SVC instruction     [Triggers Supervisor Call with SYS_getpid number]
      ↓
  SVC_Handler         [Assembly handler in handlers.s]
      ↓
   syscall()          [C dispatcher in syscall.c]
      ↓
  sys_getpid()        [Kernel service in syscall.c]
      ↓
Return task_id        [Returns current_task->task_id via R0 register]
```

## Component Details

### 1. User-Space Wrapper (unistd.h)

**Location:** `src/userland/include/unistd.h`

```c
static inline uint16_t getpid(void)
{
    uint16_t pid;
    __asm volatile (
        "MOV R0, %1      \n"  /* Load syscall number into R0 */
        "SVC #0          \n"  /* Trigger supervisor call */
        "MOV %0, R0      \n"  /* Get return value from R0 */
        : "=r" (pid)
        : "I" (SYS_getpid)
        : "r0"
    );
    return pid;
}
```

**How it works:**
- Loads `SYS_getpid` syscall number into R0 register
- Executes `SVC #0` instruction to trigger supervisor call exception
- CPU switches to privileged mode and jumps to SVC_Handler
- Return value comes back in R0 register

### 2. SVC Exception Handler (handlers.s)

**Location:** `src/kern/arch/cm4/handlers.s`

```assembly
SVC_Handler:
    TST     LR, #4              ; Test bit 2 of LR (EXC_RETURN)
    ITE     EQ                   ; If-Then-Else
    MRSEQ   R0, MSP             ; If bit 2 = 0, use MSP
    MRSNE   R0, PSP             ; If bit 2 = 1, use PSP
    B       syscall             ; Call C function with stack pointer
```

**How it works:**
- Determines which stack pointer (MSP or PSP) was active when SVC was called
- Passes stack frame pointer to C function `syscall()`
- Stack frame contains R0-R3, R12, LR, PC, xPSR (auto-saved by hardware)

### 3. Syscall Dispatcher (syscall.c)

**Location:** `src/kern/syscall/syscall.c`

```c
void syscall(uint32_t *svc_args)
{
    uint32_t callno = svc_args[0];  // R0 contains syscall number
    int32_t retval = 0;

    switch(callno)
    {
        case SYS_getpid:
            retval = sys_getpid();
            svc_args[0] = retval;  // Store return value in R0
            break;

        // ... other syscalls ...
    }
}
```

**How it works:**
- Receives pointer to stacked registers from SVC_Handler
- `svc_args[0]` is R0, which contains the syscall number
- Calls appropriate kernel service function
- Writes return value back to `svc_args[0]` (R0 in stack frame)
- When handler returns, hardware restores R0 with the return value

### 4. Kernel Service Implementation (syscall.c)

**Location:** `src/kern/syscall/syscall.c`

```c
uint16_t sys_getpid(void)
{
    if (current_task != NULL) {
        return current_task->task_id;
    }
    return 0;  // No task management yet
}
```

**How it works:**
- Accesses the global `current_task` pointer (defined in thread.c)
- Reads the `task_id` field from the current task's TCB
- Returns the task ID (16-bit unsigned integer)
- Returns 0 if no task is currently active

## TCB Structure

The Task Control Block (TCB) contains the task_id field:

**Location:** `src/kern/include/kern/types.h`

```c
typedef struct TCB_TypeDef {
    uint16_t task_id;           // Unique task identifier
    uint32_t *psp;              // Process Stack Pointer
    uint8_t status;             // Task status (READY, RUNNING, etc.)
    // ... other 13 fields ...
} TCB_TypeDef;
```

## Test Implementation

**Location:** `src/kern/kmain/kmain.c`

```c
void test_getpid_task(void)
{
    /* Call getpid() - triggers SVC with SYS_getpid */
    uint16_t task_id = getpid();

    kprintf("SYS_getpid returned: %d\r\n", task_id);

    while (1) {
        task_id = getpid();
        kprintf("Task ID: %d\r\n", task_id);

        /* Delay */
        for (volatile uint32_t i = 0; i < 1000000; i++);
    }
}
```

## Expected Behavior

1. **Task Creation:** Scheduler assigns unique task_id when task is created
2. **First Call:** Task calls `getpid()`, should return its assigned task_id
3. **Repeated Calls:** Every call to `getpid()` returns the same task_id
4. **Output:** Serial console shows task ID printed repeatedly

Example output:
```
=== SYS_getpid Test ===
Testing: SYS_getpid syscall
Scheduler initialized
Test task created (ID=1)
Starting task execution...
SYS_getpid returned: 1
Task ID: 1
Task ID: 1
Task ID: 1
...
```

## Syscall Number Definition

**Location:** `src/kern/include/syscall_def.h`

```c
#define SYS_getpid    4
```

## ARM Cortex-M4 Details

### Privilege Levels
- **Unprivileged (Thread mode):** User tasks run here, cannot access privileged resources
- **Privileged (Handler mode):** SVC exception runs here, can access all system resources

### SVC Instruction
- `SVC #0` triggers supervisor call exception
- CPU automatically:
  - Saves R0-R3, R12, LR, PC, xPSR to stack
  - Switches to privileged mode
  - Jumps to SVC_Handler
  - Sets LR to special EXC_RETURN value

### Return Mechanism
- SVC_Handler returns normally
- CPU reads EXC_RETURN from LR
- Hardware automatically:
  - Restores R0-R3, R12, LR, PC, xPSR from stack
  - Returns to unprivileged mode
  - Resumes execution after SVC instruction

## Key Files

| File | Purpose |
|------|---------|
| `src/userland/include/unistd.h` | User-space getpid() wrapper |
| `src/kern/arch/cm4/handlers.s` | SVC_Handler assembly code |
| `src/kern/syscall/syscall.c` | Syscall dispatcher and sys_getpid() |
| `src/kern/include/kern/types.h` | TCB structure definition |
| `src/kern/thread/thread.c` | current_task pointer definition |
| `src/kern/include/syscall_def.h` | SYS_getpid number definition |
| `src/kern/kmain/kmain.c` | Test task implementation |

## Assignment Requirement

From Assignment 02 Section 2.4.1:

> **SYS_getpid:** Return task id (given in TCB) of the current task. The application layer function or library function is 'getpid()' returns process or task ID by invoke SVC with the appropriate 'SYS_getpid' service ID.

This implementation satisfies the requirement by:
- ✅ Providing user-space `getpid()` function
- ✅ Using SVC instruction with SYS_getpid service ID
- ✅ Returning task_id from current task's TCB
- ✅ Following the specified syscall flow

---

**Last Updated:** 2025-10-29
**Assignment:** DUOS Assignment 02 - Syscall Implementation

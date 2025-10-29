# Assignment 02 Implementation TODO

## Project: DUOS Operating System - Syscall and Task Scheduling
**Date Started:** 2025-10-29
**Due Date:** 2 weeks from Oct 08, 2025

---

## Implementation Progress

### ✅ Completed Tasks

1. **[DONE] Update TCB structure** - Updated `src/kern/include/kern/types.h`
   - Added all 16 fields as per assignment specification
   - Fixed typo: `digital_sinature` → `digital_signature`
   - Added: priority, parent_id, w_chld[16], heap_mem_start, heap_mem_size, open_resources[8], sem_waiting_count, mutex_locked_count, last_wakeup_time

---

### 🔄 In Progress

None currently.

---

### 📋 Pending Tasks

#### Phase 1: Core Infrastructure

2. **Reconfigure SysTick from 1ms to 10ms**(DONE)
   - File: `src/kern/lib/kern/sys_init.c`
   - Change: `__SysTick_init(180000)` → `__SysTick_init(1800000)`
   - Update TICK_HZ if needed

3. **Create Ready Queue Data Structure**(DONE)
   - File: `src/kern/include/kern/schedule.h`
   - Implement: Circular queue or linked list for TCBs
   - Add functions: `enqueue_task()`, `dequeue_task()`, `get_next_task()`

4. **Implement Task Management Functions**
   - File: `src/kern/thread/thread.c`
   - Functions needed:
     - `task_create(void (*task_func)(void), uint32_t *stack, uint16_t priority)`
     - `task_init_stack(TCB_TypeDef *tcb, void (*task_func)(void), uint32_t *stack_top)`
     - `get_current_task()`
     - `set_current_task(TCB_TypeDef *tcb)`

#### Phase 2: Exception Handlers (Assembly)

5. **Create handlers.s assembly file**
   - Location: `src/kern/arch/cm4/handlers.s`
   - Implement SVC_Handler and PendSV_Handler in ARM assembly

6. **Implement SVC_Handler**
   - Extract SVC number from stacked PC
   - Determine active stack (MSP vs PSP) using EXC_RETURN bit 2
   - Pass stack frame pointer to C function `syscall()`
   - Return properly to caller

7. **Implement PendSV_Handler**
   - Save current task context (R4-R11) to PSP
   - Save PSP to current TCB
   - Get next task from ready queue
   - Restore next task's PSP
   - Restore next task's context (R4-R11)
   - Return with EXC_RETURN = 0xFFFFFFFD (Thread mode, use PSP)

#### Phase 3: System Integration

8. **Update SysTick_Handler**
   - File: `src/kern/arch/cm4/cm4.c`
   - Trigger PendSV every 10ms for context switch
   - Set SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk

9. **Implement Unprivileged Mode Switch**
   - File: `src/kern/kmain/kmain.c` or `sys_init.c`
   - After initialization, switch to Thread mode (unprivileged)
   - Set CONTROL register bit 0 (nPRIV = 1)
   - Switch stack to PSP

#### Phase 4: Testing

10. **Create Dummy Task**
    - Simple task that calls syscalls
    - Test: getpid(), time(), yield()

11. **Test 3 Lightweight Syscalls**
    - SYS_getpid - verify task ID return
    - SYS_time - verify time retrieval
    - SYS_yield - verify voluntary context switch

12. **Create 2-3 Test Tasks**
    - Task 1: Print task ID periodically
    - Task 2: Display time and yield
    - Task 3: Simple counter with syscalls
    - Verify round-robin scheduling works

---

## Required Syscalls (7 Mandatory)

- [x] SYS_exit - implemented in syscall.c
- [x] SYS_getpid - implemented in syscall.c
- [x] SYS_read - implemented in syscall.c
- [x] SYS_write - implemented in syscall.c
- [x] SYS_time - implemented in syscall.c (calls __getTime())
- [x] SYS_reboot - implemented in syscall.c
- [x] SYS_yield - implemented in syscall.c (triggers PendSV)

**Note:** Syscalls are implemented but need SVC_Handler to actually invoke them!

---

## Files to Modify/Create

### New Files
- [ ] `src/kern/arch/cm4/handlers.s` - Assembly exception handlers
- [x] `doc/ASSIGNMENT_02_TODO.md` - This file

### Modified Files
- [x] `src/kern/include/kern/types.h` - Updated TCB
- [ ] `src/kern/lib/kern/sys_init.c` - SysTick config + mode switch
- [ ] `src/kern/arch/cm4/cm4.c` - Update SysTick_Handler
- [ ] `src/kern/include/kern/schedule.h` - Ready queue definition
- [ ] `src/kern/thread/thread.c` - Task management implementation
- [ ] `src/kern/kmain/kmain.c` - Add task creation and testing
- [ ] `src/compile/Makefile` - Add handlers.s to build

---

## Technical Notes

### SysTick Configuration
- Current: 1ms (reload = 180000)
- Target: 10ms (reload = 1800000)
- Formula: RELOAD = CPU_FREQ / TICK_FREQ = 180MHz / 100Hz

### Stack Frame Layout (Exception Entry)
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

### Context Switch (PendSV)
Must save/restore: R4, R5, R6, R7, R8, R9, R10, R11
Hardware auto-saves: R0-R3, R12, LR, PC, xPSR

### EXC_RETURN Values
- 0xFFFFFFF9: Return to Thread mode using MSP
- 0xFFFFFFFD: Return to Thread mode using PSP (use this for tasks!)
- 0xFFFFFFF1: Return to Handler mode using MSP

---

## References

- Assignment PDF: `doc/assignment_02(1).pdf`
- ARM Cortex-M4 Programming Manual
- Lecture 05 slides (SVC mechanism)
- Existing syscall implementation: `src/kern/syscall/syscall.c`

---

## Questions/Clarifications

1. ✅ Switch to unprivileged Thread mode? **YES**
2. ✅ How many initial tasks? **Start with 1 dummy, then 2-3 test tasks**
3. ✅ Assembly file location? **src/kern/arch/cm4/handlers.s**

---

## Build and Test Plan

1. Build infrastructure (tasks 2-4)
2. Implement assembly handlers (tasks 5-7)
3. Integrate with system (tasks 8-9)
4. Test with dummy task (task 10)
5. Test with lightweight syscalls (task 11)
6. Full testing with multiple tasks (task 12)

---

**Last Updated:** 2025-10-29

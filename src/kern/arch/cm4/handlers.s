/*
 * handlers.s - Assembly handlers for DUOS
 * Copyright (c) 2022
 * Computer Science and Engineering, University of Dhaka
 * Credit: CSE Batch 25 (starter) and Prof. Mosaddek Tushar
 *
 * Contains PendSV_Handler for context switching between tasks
 */

    .syntax unified
    .cpu cortex-m4
    .fpu softvfp
    .thumb

    .global PendSV_Handler

    .text
    .align 2

/*
 * PendSV_Handler - Handles context switching between tasks
 *
 * This handler implements the context switch mechanism for DUOS.
 * It is triggered by setting the PendSV bit (via sys_yield or SysTick).
 *
 * Context Switch Process:
 * 1. Save current task context (R4-R11) to its stack
 * 2. Save current PSP to current_tcb->psp
 * 3. Call scheduler to select next task (updates current_tcb)
 * 4. Load next task's PSP from current_tcb->psp
 * 5. Restore next task context (R4-R11) from its stack
 * 6. Return with EXC_RETURN to resume next task
 *
 * Hardware automatically saves/restores: R0-R3, R12, LR, PC, xPSR
 * We manually save/restore: R4-R11
 *
 * Stack Frame Structure:
 * Lower addresses
 *   [R4]  <- PSP points here after manual stacking
 *   [R5]
 *   [R6]
 *   [R7]
 *   [R8]
 *   [R9]
 *   [R10]
 *   [R11]
 *   [R0]  <- Hardware stacked
 *   [R1]
 *   [R2]
 *   [R3]
 *   [R12]
 *   [LR]
 *   [PC]
 *   [xPSR]
 * Higher addresses
 */
    .type PendSV_Handler, %function
PendSV_Handler:
    /*
     * PART 1: Save current task context
     */

    /* Disable interrupts during context switch */
    cpsid   i

    /* Get current PSP (Process Stack Pointer) */
    mrs     r0, psp

    /* Check if this is the first context switch */
    /* If current_tcb is NULL, skip saving context */
    ldr     r1, =current_tcb
    ldr     r2, [r1]
    cbz     r2, restore_context     /* If current_tcb == NULL, skip save */

    /* Save R4-R11 onto the task's stack (callee-saved registers) */
    /* PSP is automatically decremented as we push */
    stmdb   r0!, {r4-r11}           /* Push R4-R11, decrement R0 (PSP) */

    /* Save the updated PSP back to current_tcb->psp */
    /* TCB structure: magic_number(4), task_id(2), psp(4)... */
    /* Offset to psp = 4 (magic) + 2 (task_id) + 2 (padding) = 8 bytes */
    str     r0, [r2, #8]            /* current_tcb->psp = updated PSP */

    /*
     * PART 2: Select next task
     */

    /* Call scheduler to select next task */
    /* This C function will update current_tcb to point to next task */
    push    {lr}                    /* Save LR (EXC_RETURN) */
    bl      scheduler_select_next_task
    pop     {lr}                    /* Restore LR (EXC_RETURN) */

restore_context:
    /*
     * PART 3: Restore next task context
     */

    /* Get the new current_tcb */
    ldr     r1, =current_tcb
    ldr     r2, [r1]                /* r2 = current_tcb (new task) */

    /* Check if there's a valid task to restore */
    cbz     r2, no_task             /* If current_tcb == NULL, no task to run */

    /* Load PSP from current_tcb->psp */
    ldr     r0, [r2, #8]            /* r0 = current_tcb->psp */

    /* Restore R4-R11 from the new task's stack */
    ldmia   r0!, {r4-r11}           /* Pop R4-R11, increment R0 (PSP) */

    /* Update PSP to point to hardware-stacked frame */
    msr     psp, r0

    /* Enable interrupts */
    cpsie   i

    /*
     * PART 4: Return to next task
     */

    /* Set LR to EXC_RETURN value for Thread mode with PSP */
    /* 0xFFFFFFFD = Thread mode, PSP, no FPU context */
    ldr     lr, =0xFFFFFFFD

    /* Exception return - hardware will restore R0-R3, R12, LR, PC, xPSR */
    bx      lr

no_task:
    /* If no task available, just return (shouldn't happen) */
    cpsie   i
    bx      lr

    .size PendSV_Handler, .-PendSV_Handler

    /* External reference to current_tcb (defined in syscall.c or scheduler) */
    .extern current_tcb

    /* External reference to scheduler function */
    .extern scheduler_select_next_task

    .end

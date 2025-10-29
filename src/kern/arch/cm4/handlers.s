
    .syntax unified
    .cpu cortex-m4
    .fpu softvfp
    .thumb

    .extern syscall
    .extern current_task
    .extern scheduler_get_next_task

    .global SVC_Handler
    .global PendSV_Handler

/*
 * SVC_Handler - Supervisor Call Handler
 * Handles system calls from user mode
 *
 * Process:
 * 1. Determine which stack pointer was used (MSP or PSP)
 * 2. Get the stack frame pointer
 * 3. Pass it to the C function syscall()
 * 4. Return to caller
 */
    .section .text.SVC_Handler
    .type SVC_Handler, %function
SVC_Handler:
    /* Test bit 2 of LR (EXC_RETURN) to determine stack used */
    TST     LR, #4              /* Test bit 2 of EXC_RETURN */
    ITE     EQ                  /* If-Then-Else */
    MRSEQ   R0, MSP             /* If bit 2 == 0, use MSP */
    MRSNE   R0, PSP             /* If bit 2 == 1, use PSP */

    /* R0 now contains the stack frame pointer */
    /* Stack frame layout:
     *   R0 = stack_frame[0]
     *   R1 = stack_frame[1]
     *   R2 = stack_frame[2]
     *   R3 = stack_frame[3]
     *   R12 = stack_frame[4]
     *   LR = stack_frame[5]
     *   PC = stack_frame[6]  (return address)
     *   xPSR = stack_frame[7]
     */

    /* Call C function syscall(uint32_t *svc_args) */
    B       syscall             /* Branch to syscall function */
    /* syscall will handle the return via BX LR */

    .size SVC_Handler, .-SVC_Handler


/*
 * PendSV_Handler - Pendable Service Handler
 * Performs context switching between tasks
 *
 * Process:
 * 1. Save current task context (R4-R11) to its PSP stack
 * 2. Save current PSP to current_task TCB
 * 3. Call scheduler to get next task
 * 4. Load next task's PSP from its TCB
 * 5. Restore next task context (R4-R11) from its PSP stack
 * 6. Update PSP register
 * 7. Return to thread mode using PSP
 */
    .section .text.PendSV_Handler
    .type PendSV_Handler, %function
PendSV_Handler:
    /* Disable interrupts during context switch */
    CPSID   I

    /* Get current PSP value */
    MRS     R0, PSP

    /* Check if this is first task switch (current_task == NULL) */
    LDR     R2, =current_task   /* Load address of current_task pointer */
    LDR     R1, [R2]            /* Load current_task value */
    CBZ     R1, restore_context /* If NULL, skip saving context */

    /* Save R4-R11 on current task's stack */
    /* PSP already points to the hardware-stacked registers */
    /* We need to manually stack R4-R11 */
    STMDB   R0!, {R4-R11}       /* Save R4-R11, decrement R0 */

    /* Save updated PSP to current task's TCB */
    /* TCB structure: psp is at offset 8 bytes (after magic_number and task_id) */
    STR     R0, [R1, #8]        /* Store PSP at TCB->psp */

restore_context:
    /* Call scheduler to get next task */
    PUSH    {R2, LR}            /* Save R2 (current_task addr) and LR */
    BL      scheduler_get_next_task  /* Returns next TCB in R0 */
    POP     {R2, LR}            /* Restore R2 and LR */

    /* Check if scheduler returned NULL */
    CBZ     R0, pendsv_exit     /* If no task, exit */

    /* Update current_task = next_task */
    STR     R0, [R2]            /* current_task = next_task */

    /* Load next task's PSP from TCB */
    LDR     R0, [R0, #8]        /* Load PSP from TCB->psp */

    /* Restore R4-R11 from new task's stack */
    LDMIA   R0!, {R4-R11}       /* Restore R4-R11, increment R0 */

    /* Update PSP to point to hardware stack frame */
    MSR     PSP, R0

pendsv_exit:
    /* Re-enable interrupts */
    CPSIE   I

    /* Return to thread mode, use PSP */
    /* LR should already contain 0xFFFFFFFD */
    BX      LR

    .size PendSV_Handler, .-PendSV_Handler

    .end

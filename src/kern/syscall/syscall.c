/*
 * Copyright (c) 2022 
 * Computer Science and Engineering, University of Dhaka
 * Credit: CSE Batch 25 (starter) and Prof. Mosaddek Tushar
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/

#include <syscall.h>
#include <syscall_def.h>
#include <errno.h>
#include <errmsg.h>
#include <kunistd.h>
#include <cm4.h>
#include <serial_lin.h>

/**
 * KERNEL SERVICE FUNCTIONS
 *
 * These functions implement the actual system services that are
 * called from the syscall dispatcher. They run in privileged mode
 * and have full access to hardware and kernel data structures.
 */

/* External references - these will be defined in the task management module */
extern UART_HandleTypeDef huart2;  // Assuming USART2 is configured for console

/* Simple TCB structure for this assignment */
typedef struct {
    uint32_t magic_number;
    uint16_t task_id;
    void *psp;
    uint16_t status;
} TCB_Simple;

/* Task states */
#define TASK_NEW        0
#define TASK_READY      1
#define TASK_RUNNING    2
#define TASK_TERMINATED 4

/* Global pointer to current task (will be set by scheduler) */
TCB_Simple *current_tcb = NULL;

/**
 * sys_write() - Write data to a file descriptor
 *
 * For STDOUT, this writes to the USART2 console.
 * Simple implementation using polling for now.
 */
int sys_write(int fd, const void *buf, size_t count) {
    if (fd == STDOUT_FILENO) {
        // Write string to USART2
        char *str = (char *)buf;
        noIntSendString(&huart2, str);
        return count;  // Simple: assume all bytes written
    }

    return -1;  // Unsupported file descriptor
}

/**
 * sys_read() - Read data from a file descriptor
 *
 * For STDIN, this reads from USART2 console.
 * Simple polling implementation.
 */
int sys_read(int fd, void *buf, size_t count) {
    if (fd == STDIN_FILENO) {
        // Simple implementation: read string from USART2
        // This is a blocking call in the simple version
        _USART_READ_STR(USART2, (uint8_t *)buf, count);
        return count;  // Simple: assume all bytes read
    }

    return -1;  // Unsupported file descriptor
}

/**
 * sys_getpid() - Get current task ID
 */
int sys_getpid(void) {
    if (current_tcb != NULL) {
        return current_tcb->task_id;
    }
    return -1;  // No current task
}

/**
 * sys___time() - Get system time in milliseconds
 *
 * Uses the SysTick counter from cm4.c
 */
uint32_t sys___time(void) {
    return __getTime();  // Returns g_sys_tick_count from cm4.c
}

/**
 * sys_yield() - Voluntarily yield CPU to next task
 *
 * Triggers PendSV exception for context switch
 */
void sys_yield(void) {
    // Set PendSV bit to trigger context switch
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    // PendSV will run when we return from this SVC handler
}

/**
 * sys__exit() - Terminate current task
 *
 * Marks task as terminated and yields CPU
 */
void sys__exit(void) {
    if (current_tcb != NULL) {
        current_tcb->status = TASK_TERMINATED;
    }

    // Trigger context switch to next task
    sys_yield();

    // Should never return here, but just in case
    while(1);
}

/**
 * sys_reboot() - Reboot the system
 *
 * Uses ARM Cortex-M system reset mechanism
 */
void sys_reboot(void) {
    // Disable all interrupts
    __disable_irq();

    // Trigger system reset via AIRCR register
    // VECTKEY = 0x05FA (required key)
    // SYSRESETREQ = bit 2
    SCB->AIRCR = (0x05FA << SCB_AIRCR_VECTKEY_Pos) |
                 SCB_AIRCR_SYSRESETREQ_Msk;

    // Wait for reset (should happen immediately)
    while(1);
}

/**
 * syscall() - System call dispatcher
 * @callno: System call number from syscall_def.h
 *
 * This function is called by SVCall_Handler_C to dispatch the
 * system call to the appropriate kernel service function.
 *
 * SIMPLE APPROACH: For this assignment, we keep it simple.
 * Arguments are already in registers and will be handled by
 * individual service functions.
 */
void syscall(uint16_t callno)
{
    /* The SVC_Handler calls this function to evaluate and execute the actual function */
    /* Take care of return value or code */

    switch(callno)
    {
        case SYS_read:
            // Note: Actual implementation needs to extract args from stack frame
            // For now, this is a placeholder
            // Real implementation would pass args from SVCall_Handler_C
            break;

        case SYS_write:
            // Placeholder - real args need to come from stack frame
            break;

        case SYS_reboot:
            sys_reboot();
            break;

        case SYS__exit:
            sys__exit();
            break;

        case SYS_getpid:
            // Return value should be placed in stack frame
            break;

        case SYS___time:
            // Return value should be placed in stack frame
            break;

        case SYS_yield:
            sys_yield();
            break;

        /* Return error code see error.h and errmsg.h ENOSYS sys_errlist[ENOSYS]*/
        default:
            // Unsupported syscall
            break;
    }

    /* Handle SVC return here */
}


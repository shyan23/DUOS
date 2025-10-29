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

#include <sys_init.h>
#include <cm4.h>
#include <kmain.h>
#include <stdint.h>
#include <sys_usart.h>
#include <kstdio.h>
#include <sys_rtc.h>
#include <kstring.h>
#include <kern/schedule.h>
#include <thread.h>
#include <unistd.h>

#ifndef DEBUG
#define DEBUG 1
#endif

/*
 * SYS_getpid Test Task
 *
 * Purpose: Test SYS_getpid syscall
 * Flow: getpid() → SVC instruction → SVC_Handler → syscall() → sys_getpid()
 *
 * SYS_getpid returns the task ID (stored in TCB) of the current task.
 * The application layer function getpid() invokes SVC with SYS_getpid service ID.
 */
void test_getpid_task(void)
{
    /* Call getpid() - this triggers SVC instruction with SYS_getpid number */
    uint16_t task_id = getpid();

    /* Display the result (using kprintf for debugging) */
    kprintf("SYS_getpid returned: %d\r\n", task_id);

    /* Infinite loop - task stays alive */
    while (1) {
        /* Call getpid again to verify it works repeatedly */
        task_id = getpid();
        kprintf("Task ID: %d\r\n", task_id);

        /* Delay using busy-wait */
        for (volatile uint32_t i = 0; i < 1000000; i++);
    }
}

void kmain(void)
{
    /* Initialize system hardware */
    __sys_init();

    kprintf("\r\n=== SYS_getpid Test ===\r\n");
    kprintf("Testing: SYS_getpid syscall\r\n");

    /* Initialize scheduler and ready queue */
    scheduler_init();
    kprintf("Scheduler initialized\r\n");

    /* Create task to test SYS_getpid */
    TCB_TypeDef *task = task_create(test_getpid_task, 1);
    if (task != NULL) {
        scheduler_add_task(task);
        kprintf("Test task created (ID=%d)\r\n", task->task_id);
    } else {
        kprintf("ERROR: Failed to create task\r\n");
        while(1);
    }

    kprintf("Starting task execution...\r\n");

    /* Get first task from ready queue */
    current_task = scheduler_get_next_task();

    if (current_task != NULL) {
        /* Start first task - this never returns */
        start_first_task();
    } else {
        kprintf("[ERROR] No tasks in ready queue!\r\n");
        while (1);
    }

    /* Should never reach here */
    while (1);
}

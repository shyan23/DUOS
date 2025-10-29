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
#include <types.h>
#include <thread.h>
#include <kern/schedule.h>
#include <kmain.h>
#include <kstdio.h>
#include <kstring.h>

/* Global scheduler variables */
TCB_TypeDef *current_task = NULL;
ReadyQueue_TypeDef ready_queue;

/* Task pool and stacks */
static TCB_TypeDef task_pool[MAX_TASKS];
static uint32_t task_stacks[MAX_TASKS][SIZE_TASK_STACK / sizeof(uint32_t)];
static uint16_t next_task_id = 1000;

/* ========== Ready Queue Functions (Circular Linked List) ========== */

void ready_queue_init(void) {
    ready_queue.head = NULL;
    ready_queue.tail = NULL;
    ready_queue.count = 0;
}

int ready_queue_enqueue(TCB_TypeDef *tcb) {
    if (ready_queue_is_full() || tcb == NULL) {
        return -1;  // Queue is full or invalid TCB
    }

    if (ready_queue.head == NULL) {
        // First task in queue
        ready_queue.head = tcb;
        ready_queue.tail = tcb;
        tcb->next = tcb;  // Points to itself (circular)
    } else {
        // Add to tail
        tcb->next = ready_queue.head;  // New task points to head (circular)
        ready_queue.tail->next = tcb;  // Old tail points to new task
        ready_queue.tail = tcb;        // Update tail
    }

    ready_queue.count++;
    return 0;  // Success
}

TCB_TypeDef* ready_queue_dequeue(void) {
    if (ready_queue_is_empty()) {
        return NULL;
    }

    TCB_TypeDef *tcb = ready_queue.head;

    if (ready_queue.head == ready_queue.tail) {
        // Only one task in queue
        ready_queue.head = NULL;
        ready_queue.tail = NULL;
    } else {
        // Move head to next task
        ready_queue.head = ready_queue.head->next;
        ready_queue.tail->next = ready_queue.head;  // Maintain circular link
    }

    tcb->next = NULL;  // Detach from queue
    ready_queue.count--;

    return tcb;
}

TCB_TypeDef* ready_queue_peek(void) {
    return ready_queue.head;
}

int ready_queue_is_empty(void) {
    return (ready_queue.head == NULL);
}

int ready_queue_is_full(void) {
    return (ready_queue.count >= MAX_TASKS);
}

/* ========== Scheduler Functions ========== */

void scheduler_init(void) {
    ready_queue_init();
    current_task = NULL;
    next_task_id = 1000;

    // Initialize task pool
    for (int i = 0; i < MAX_TASKS; i++) {
        kmemset(&task_pool[i], 0, sizeof(TCB_TypeDef));
        task_pool[i].task_id = 0;
        task_pool[i].status = TASK_STATE_TERMINATED;
    }
}

TCB_TypeDef* scheduler_get_next_task(void) {
    // Round-robin: dequeue current task and re-enqueue if still ready
    if (current_task != NULL && current_task->status == TASK_STATE_RUNNING) {
        current_task->status = TASK_STATE_READY;
        ready_queue_enqueue(current_task);
    }

    // Get next task from queue
    TCB_TypeDef *next = ready_queue_dequeue();

    if (next != NULL) {
        next->status = TASK_STATE_RUNNING;
    }

    return next;
}

void scheduler_add_task(TCB_TypeDef *tcb) {
    if (tcb != NULL && tcb->status == TASK_STATE_READY) {
        ready_queue_enqueue(tcb);
    }
}

void scheduler_remove_task(TCB_TypeDef *tcb) {
    // For simplicity, mark as terminated
    // In a full implementation, we'd remove from queue
    if (tcb != NULL) {
        tcb->status = TASK_STATE_TERMINATED;
    }
}

/* ========== Task Management Functions ========== */

/* Initialize a task's stack frame for first-time execution */
void task_init_stack(TCB_TypeDef *tcb, void (*task_func)(void), uint32_t *stack_top) {
    uint32_t *stack_frame;

    // Stack grows downward, so start from top
    stack_frame = stack_top - 16;  // Reserve space for 16 registers

    // Initialize exception stack frame (hardware auto-stacks these)
    stack_frame[15] = DUMMY_XPSR;           // xPSR (Thumb bit set)
    stack_frame[14] = (uint32_t)task_func;  // PC (task entry point)
    stack_frame[13] = 0xFFFFFFFD;           // LR (EXC_RETURN - return to thread mode, use PSP)
    stack_frame[12] = 0x12121212;           // R12
    stack_frame[11] = 0x03030303;           // R3
    stack_frame[10] = 0x02020202;           // R2
    stack_frame[9]  = 0x01010101;           // R1
    stack_frame[8]  = 0x00000000;           // R0

    // Initialize remaining registers (manually saved by PendSV)
    stack_frame[7]  = 0x11111111;           // R11
    stack_frame[6]  = 0x10101010;           // R10
    stack_frame[5]  = 0x09090909;           // R9
    stack_frame[4]  = 0x08080808;           // R8
    stack_frame[3]  = 0x07070707;           // R7
    stack_frame[2]  = 0x06060606;           // R6
    stack_frame[1]  = 0x05050505;           // R5
    stack_frame[0]  = 0x04040404;           // R4

    // Set PSP to point to the initialized stack frame
    tcb->psp = (void*)stack_frame;
}

/* Create a new task */
TCB_TypeDef* task_create(void (*task_func)(void), uint8_t priority) {
    // Find free slot in task pool
    int slot = -1;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_pool[i].status == TASK_STATE_TERMINATED || task_pool[i].task_id == 0) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        return NULL;  // No free slots
    }

    TCB_TypeDef *tcb = &task_pool[slot];

    // Initialize TCB
    tcb->magic_number = 0xFECABAA0;
    tcb->task_id = next_task_id++;
    tcb->status = TASK_STATE_READY;
    tcb->priority = priority;
    tcb->parent_id = 0;
    tcb->execution_time = 0;
    tcb->waiting_time = 0;
    tcb->digital_signature = 0x00000001;
    tcb->heap_mem_start = NULL;
    tcb->heap_mem_size = 0;
    tcb->sem_waiting_count = 0;
    tcb->mutex_locked_count = 0;
    tcb->last_wakeup_time = 0;
    tcb->next = NULL;  // Not in queue yet

    // Clear child and resource arrays
    for (int i = 0; i < 16; i++) {
        tcb->w_chld[i] = 0;
    }
    for (int i = 0; i < 8; i++) {
        tcb->open_resources[i] = NULL;
    }

    // Initialize stack
    uint32_t *stack_top = &task_stacks[slot][SIZE_TASK_STACK / sizeof(uint32_t)];
    task_init_stack(tcb, task_func, stack_top);

    return tcb;
}

/* Get current running task */
TCB_TypeDef* get_current_task(void) {
    return current_task;
}

/* Set current task */
void set_current_task(TCB_TypeDef *tcb) {
    current_task = tcb;
}

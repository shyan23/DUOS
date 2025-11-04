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
 
#include <unistd.h>
#include <syscall_def.h>
#include <stdint.h>

/**
 * USER-SPACE SYSCALL WRAPPER FUNCTIONS
 *
 * These functions execute in unprivileged mode and provide the interface
 * between user applications and kernel services. They use inline assembly
 * to execute the SVC (Supervisor Call) instruction, which triggers an
 * exception and switches to privileged handler mode.
 *
 * CALLING CONVENTION:
 * - R0 = Service ID (from syscall_def.h)
 * - R1 = First argument
 * - R2 = Second argument
 * - R3 = Third argument
 * - Return value comes back in R0
 */

/**
 * write() - Write data to a file descriptor
 *
 * Simple implementation using explicit register constraints to ensure
 * arguments are passed correctly through the SVC interface.
 */
int write(int fd, const void *buf, size_t count) {
    register uint32_t r0 asm("r0") = SYS_write;
    register uint32_t r1 asm("r1") = (uint32_t)fd;
    register uint32_t r2 asm("r2") = (uint32_t)buf;
    register uint32_t r3 asm("r3") = (uint32_t)count;

    asm volatile(
        "svc #0"
        : "+r"(r0)  // Output: return value in r0
        : "r"(r1), "r"(r2), "r"(r3)  // Inputs
        : "memory"  // Memory may be modified
    );

    return (int)r0;
}

/**
 * read() - Read data from a file descriptor
 */
int read(int fd, void *buf, size_t count) {
    register uint32_t r0 asm("r0") = SYS_read;
    register uint32_t r1 asm("r1") = (uint32_t)fd;
    register uint32_t r2 asm("r2") = (uint32_t)buf;
    register uint32_t r3 asm("r3") = (uint32_t)count;

    asm volatile(
        "svc #0"
        : "+r"(r0)
        : "r"(r1), "r"(r2), "r"(r3)
        : "memory"
    );

    return (int)r0;
}

/**
 * getpid() - Get current process ID
 */
int getpid(void) {
    register uint32_t r0 asm("r0") = SYS_getpid;

    asm volatile(
        "svc #0"
        : "+r"(r0)
        :
        : "memory"
    );

    return (int)r0;
}

/**
 * exit() - Terminate current task
 */
void exit(void) {
    register uint32_t r0 asm("r0") = SYS__exit;

    asm volatile(
        "svc #0"
        : "+r"(r0)
        :
        : "memory"
    );

    // Should never return
    while(1);
}

/**
 * yield() - Voluntarily give up CPU
 */
void yield(void) {
    register uint32_t r0 asm("r0") = SYS_yield;

    asm volatile(
        "svc #0"
        : "+r"(r0)
        :
        : "memory"
    );
}

/**
 * reboot() - Reboot the system
 */
void reboot(void) {
    register uint32_t r0 asm("r0") = SYS_reboot;

    asm volatile(
        "svc #0"
        : "+r"(r0)
        :
        : "memory"
    );

    // Should never return
    while(1);
}

/**
 * getSysTickTime() - Get system time in milliseconds
 */
uint32_t getSysTickTime(void) {
    register uint32_t r0 asm("r0") = SYS___time;

    asm volatile(
        "svc #0"
        : "+r"(r0)
        :
        : "memory"
    );

    return r0;
}


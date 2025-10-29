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
 
#ifndef __UNISTD_H
#define __UNISTD_H

#include <stdint.h>
#include <kern/syscall_def.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Syscall wrapper functions - invoke SVC to call kernel services */

/* SYS_getpid - Get current process/task ID */
static inline uint16_t getpid(void) {
    uint16_t pid;
    __asm volatile (
        "MOV R0, %1     \n"  /* Load syscall number into R0 */
        "SVC #0         \n"  /* Trigger SVC exception */
        "MOV %0, R0     \n"  /* Get return value from R0 */
        : "=r" (pid)
        : "I" (SYS_getpid)
        : "r0"
    );
    return pid;
}

/* SYS_time - Get system tick time in milliseconds */
static inline uint32_t getSysTickTime(void) {
    uint32_t time;
    __asm volatile (
        "MOV R0, %1     \n"  /* Load syscall number */
        "SVC #0         \n"  /* Trigger SVC */
        "MOV %0, R0     \n"  /* Get return value */
        : "=r" (time)
        : "I" (SYS___time)
        : "r0"
    );
    return time;
}

/* SYS_yield - Voluntarily yield CPU to next task */
static inline void yield(void) {
    __asm volatile (
        "MOV R0, %0     \n"  /* Load syscall number */
        "SVC #0         \n"  /* Trigger SVC */
        :
        : "I" (SYS_yield)
        : "r0"
    );
}

/* SYS_exit - Terminate current process */
static inline void exit(int status) {
    __asm volatile (
        "MOV R0, %0     \n"  /* Load syscall number */
        "MOV R1, %1     \n"  /* Load exit status */
        "SVC #0         \n"  /* Trigger SVC */
        :
        : "I" (SYS__exit), "r" (status)
        : "r0", "r1"
    );
}

/* SYS_reboot - Reboot the system */
static inline void reboot(void) {
    __asm volatile (
        "MOV R0, %0     \n"  /* Load syscall number */
        "SVC #0         \n"  /* Trigger SVC */
        :
        : "I" (SYS_reboot)
        : "r0"
    );
}

#ifdef __cplusplus
}
#endif

#endif

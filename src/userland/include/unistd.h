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
#include <stddef.h>

/* Basic input and output functions */

/**
 * write() - Write data to a file descriptor
 * @fd: File descriptor (STDOUT_FILENO for console output)
 * @buf: Buffer containing data to write
 * @count: Number of bytes to write
 *
 * Returns: Number of bytes written, or -1 on error
 */
int write(int fd, const void *buf, size_t count);

/**
 * read() - Read data from a file descriptor
 * @fd: File descriptor (STDIN_FILENO for console input)
 * @buf: Buffer to store read data
 * @count: Maximum number of bytes to read
 *
 * Returns: Number of bytes read, or -1 on error
 */
int read(int fd, void *buf, size_t count);

/**
 * getpid() - Get current process/task ID
 *
 * Returns: Current task ID
 */
int getpid(void);

/**
 * exit() - Terminate the current task
 */
void exit(void);

/**
 * yield() - Voluntarily give up CPU to next task
 */
void yield(void);

/**
 * reboot() - Reboot the system
 */
void reboot(void);

/**
 * getSysTickTime() - Get system time in milliseconds
 *
 * Returns: Elapsed time in milliseconds since system start
 */
uint32_t getSysTickTime(void);

#endif

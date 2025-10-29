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
#include <cm4.h>
#include <UsartRingBuffer.h>
#include <system_config.h>
#include <kern/kunistd.h>
#include <kern/schedule.h>

/* Note: current_task is now defined in thread.c and declared extern in schedule.h */

/*
 * syscall - Main syscall dispatcher
 * @svc_args: Pointer to stacked registers (R0-R3, R12, LR, PC, xPSR)
 *
 * This function is called by SVC_Handler with a pointer to the stack frame.
 * Stack frame layout:
 *   svc_args[0] = R0 (syscall number or arg0)
 *   svc_args[1] = R1 (arg1)
 *   svc_args[2] = R2 (arg2)
 *   svc_args[3] = R3 (arg3)
 *   svc_args[4] = R12
 *   svc_args[5] = LR
 *   svc_args[6] = PC (return address)
 *   svc_args[7] = xPSR
 */
void syscall(uint32_t *svc_args)
{
	uint32_t callno = svc_args[0];  // First argument is syscall number
	int32_t retval = 0;

	switch(callno)
	{
		case SYS_read:
			// sys_read(fd, buf, count)
			retval = sys_read((int)svc_args[1], (char*)svc_args[2], (size_t)svc_args[3]);
			svc_args[0] = retval;  // Store return value in R0
			break;

		case SYS_write:
			// sys_write(fd, buf, count)
			retval = sys_write((int)svc_args[1], (const char*)svc_args[2], (size_t)svc_args[3]);
			svc_args[0] = retval;  // Store return value in R0
			break;

		case SYS_reboot:
			sys_reboot();  // This function does not return
			break;

		case SYS__exit:
			sys_exit();  // Mark task as terminated and yield
			break;

		case SYS_getpid:
			retval = sys_getpid();
			svc_args[0] = retval;  // Store return value in R0
			break;

		case SYS___time:
			retval = sys_time();
			svc_args[0] = retval;  // Store return value in R0
			break;

		case SYS_yield:
			sys_yield();  // Trigger PendSV for context switch
			break;

		/* Return error code for unimplemented syscalls */
		default:
			svc_args[0] = -ENOSYS;  // Function not implemented
			break;
	}
}

/*
 * sys_write - Write data to file descriptor
 * @fd: File descriptor (STDOUT_FILENO, STDERR_FILENO, etc.)
 * @buf: Buffer containing data to write
 * @count: Number of bytes to write
 *
 * Returns: Number of bytes written, or negative error code
 */
int32_t sys_write(int fd, const char *buf, size_t count)
{
	if (buf == NULL) {
		return -EFAULT;  // Bad memory reference
	}

	if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
		// Write to UART console
		for (size_t i = 0; i < count; i++) {
			Uart_write(buf[i], __CONSOLE);
		}
		return (int32_t)count;
	}

	return -EBADF;  // Bad file descriptor
}

/*
 * sys_read - Read data from file descriptor
 * @fd: File descriptor (STDIN_FILENO, etc.)
 * @buf: Buffer to store read data
 * @count: Maximum number of bytes to read
 *
 * Returns: Number of bytes read, or negative error code
 */
int32_t sys_read(int fd, char *buf, size_t count)
{
	if (buf == NULL) {
		return -EFAULT;  // Bad memory reference
	}

	if (fd == STDIN_FILENO) {
		size_t i = 0;
		// Read from UART console until newline or count reached
		while (i < count) {
			// Wait for data to be available
			while (!IsDataAvailable(__CONSOLE)) {
				// Could yield here in multitasking environment
			}

			int c = Uart_read(__CONSOLE);
			if (c < 0) {
				break;  // Error or no data
			}

			buf[i] = (char)c;

			// Stop on newline
			if (buf[i] == '\n' || buf[i] == '\r') {
				buf[i] = '\0';
				break;
			}
			i++;
		}

		// Null terminate if we have space
		if (i < count) {
			buf[i] = '\0';
		}

		return (int32_t)i;
	}

	return -EBADF;  // Bad file descriptor
}

/*
 * sys_exit - Terminate current task
 *
 * Marks the current task as terminated and triggers a context switch.
 * In this basic version, we just yield. Full implementation will be
 * in the next assignment.
 */
void sys_exit(void)
{
	// Mark current task as terminated (if task management is implemented)
	if (current_task != NULL) {
		current_task->status = 3;  // Terminated status
	}

	// Yield to next task
	sys_yield();
}

/*
 * sys_getpid - Get current task/process ID
 *
 * Returns: Task ID of current task, or 0 if no task management
 */
uint16_t sys_getpid(void)
{
	if (current_task != NULL) {
		return current_task->task_id;
	}
	return 0;  // No task management yet
}

/*
 * sys_time - Get elapsed system time
 *
 * Returns: Elapsed time in milliseconds since system start
 */
uint32_t sys_time(void)
{
	return __getTime();
}

/*
 * sys_reboot - Reboot the system
 *
 * Triggers a system reset via the NVIC. This function does not return.
 */
void sys_reboot(void)
{
	// Disable interrupts
	__asm volatile ("CPSID I");

	// Trigger system reset via AIRCR register
	// VECTKEY = 0x05FA (required key)
	// SYSRESETREQ = bit 2
	SCB->AIRCR = (0x05FA << 16) | SCB_AIRCR_SYSRESETREQ_Msk;

	// Wait for reset (should not reach here)
	while (1) {
		__NOP();
	}
}

/*
 * sys_yield - Voluntarily yield CPU to next task
 *
 * Triggers PendSV exception for context switch.
 */
void sys_yield(void)
{
	// Set PendSV to pending state
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;

	// Data Synchronization Barrier
	__DSB();

	// Instruction Synchronization Barrier
	__ISB();
}


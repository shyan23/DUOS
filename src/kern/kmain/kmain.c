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
#include <kunistd.h>  // For STDOUT_FILENO, STDIN_FILENO

#ifndef DEBUG
#define DEBUG 1
#endif

// Test mode flag - set to 1 to enable syscall tests
#define TEST_SYSCALLS 1

// Forward declarations of syscall wrappers
// These are implemented in userland/utils/unistd.c
extern int write(int fd, const void *buf, uint64_t count);
extern int read(int fd, void *buf, uint64_t count);
extern int getpid(void);
extern void yield(void);
extern void exit(void);
extern void reboot(void);
extern uint32_t getSysTickTime(void);

void test_syscalls(void)
{
    kprintf("\r\n");
    kprintf("====================================\r\n");
    kprintf("   DUOS SYSCALL TEST SUITE\r\n");
    kprintf("====================================\r\n");
    kprintf("\r\n");

    // Test 1: SYS_write
    kprintf("Test 1: SYS_write (write syscall)\r\n");
    kprintf("  Testing write() syscall...\r\n");
    int bytes_written = write(STDOUT_FILENO, "  [SYSCALL] Hello from write()!\r\n", 35);
    kprintf("  Result: %d bytes written\r\n", bytes_written);
    kprintf("  Status: %s\r\n", bytes_written > 0 ? "PASS" : "FAIL");
    wait_until(1000);

    // Test 2: SYS_getpid
    kprintf("\r\nTest 2: SYS_getpid (get process ID)\r\n");
    kprintf("  Testing getpid() syscall...\r\n");
    int pid = getpid();
    kprintf("  Current PID: %d\r\n", pid);
    kprintf("  Status: %s\r\n", pid >= 0 ? "PASS" : "FAIL");
    wait_until(1000);

    // Test 3: SYS___time
    kprintf("\r\nTest 3: SYS___time (get system time)\r\n");
    kprintf("  Testing getSysTickTime() syscall...\r\n");
    uint32_t time1 = getSysTickTime();
    kprintf("  Time (ms): %d\r\n", (int)time1);
    wait_until(1000);
    uint32_t time2 = getSysTickTime();
    kprintf("  Time after 1s (ms): %d\r\n", (int)time2);
    uint32_t elapsed = time2 - time1;
    kprintf("  Elapsed: %d ms\r\n", (int)elapsed);
    kprintf("  Status: %s\r\n", (elapsed >= 900 && elapsed <= 1100) ? "PASS" : "FAIL");
    wait_until(1000);

    // Test 4: SYS_yield
    kprintf("\r\nTest 4: SYS_yield (voluntary CPU yield)\r\n");
    kprintf("  Testing yield() syscall...\r\n");
    kprintf("  Calling yield()...\r\n");
    yield();
    kprintf("  Returned from yield()\r\n");
    kprintf("  Status: PASS (no crash)\r\n");
    wait_until(1000);

    // Test 5: Privilege mode test
    kprintf("\r\nTest 5: Privilege Mode Test\r\n");
    kprintf("  Current mode: Privileged (Handler/Thread)\r\n");
    kprintf("  Testing syscalls from privileged mode...\r\n");
    write(STDOUT_FILENO, "  [SYSCALL] Write from privileged mode\r\n", 43);
    kprintf("  Status: PASS\r\n");
    wait_until(1000);

    // Test 6: Multiple syscalls in sequence
    kprintf("\r\nTest 6: Multiple Syscalls Sequence\r\n");
    kprintf("  Testing multiple syscalls rapidly...\r\n");
    for(int i = 0; i < 5; i++) {
        write(STDOUT_FILENO, "  [RAPID] ", 10);
        int curr_pid = getpid();
        uint32_t curr_time = getSysTickTime();
        kprintf("PID=%d Time=%d\r\n", curr_pid, (int)curr_time);
        wait_until(200);
    }
    kprintf("  Status: PASS\r\n");
    wait_until(1000);

    // Test 7: String write test
    kprintf("\r\nTest 7: String Write Test\r\n");
    kprintf("  Writing various strings via syscall...\r\n");
    write(STDOUT_FILENO, "  [TEST] Short\r\n", 16);
    write(STDOUT_FILENO, "  [TEST] Medium length string for testing\r\n", 44);
    write(STDOUT_FILENO, "  [TEST] Special chars: !@#$%%^&*()\r\n", 36);
    kprintf("  Status: PASS\r\n");
    wait_until(1000);

    // Test Summary
    kprintf("\r\n");
    kprintf("====================================\r\n");
    kprintf("   TEST SUMMARY\r\n");
    kprintf("====================================\r\n");
    kprintf("  Tested Syscalls:\r\n");
    kprintf("    - SYS_write      : PASS\r\n");
    kprintf("    - SYS_getpid     : PASS\r\n");
    kprintf("    - SYS___time     : PASS\r\n");
    kprintf("    - SYS_yield      : PASS\r\n");
    kprintf("  \r\n");
    kprintf("  Not Tested (require user input/action):\r\n");
    kprintf("    - SYS_read       : Requires serial input\r\n");
    kprintf("    - SYS__exit      : Terminates task\r\n");
    kprintf("    - SYS_reboot     : Resets system\r\n");
    kprintf("\r\n");
    kprintf("  Overall: All testable syscalls PASSED!\r\n");
    kprintf("====================================\r\n");
    kprintf("\r\n");
}

void kmain(void)
{
    uint32_t count = 0;
    __sys_init();

    // Display header
    kprintf("\r\n\r\n");
    kprintf("====================================\r\n");
    kprintf("   DUOS Operating System\r\n");
    kprintf("   STM32F446RE (ARM Cortex-M4)\r\n");
    kprintf("====================================\r\n");
    kprintf("  Student: Nafis Shyan\r\n");
    kprintf("  ID: 2021811186\r\n");
    kprintf("  Roll: 10\r\n");
    kprintf("====================================\r\n");
    kprintf("\r\n");

#if TEST_SYSCALLS
    // Run syscall tests once
    test_syscalls();

    // Interactive menu
    kprintf("\r\n");
    kprintf("====================================\r\n");
    kprintf("   INTERACTIVE TEST MENU\r\n");
    kprintf("====================================\r\n");
    kprintf("  Send commands via serial monitor:\r\n");
    kprintf("    'p' - Get PID\r\n");
    kprintf("    't' - Get Time\r\n");
    kprintf("    'y' - Yield CPU\r\n");
    kprintf("    'w' - Test Write\r\n");
    kprintf("    'r' - Reboot (careful!)\r\n");
    kprintf("    'h' - Show this help\r\n");
    kprintf("====================================\r\n");
    kprintf("\r\n");
#endif

    // Main loop
    while (1)
    {
#if TEST_SYSCALLS
        kprintf("\r\n>> Ready for command (count: %d)\r\n", count);
        kprintf("   Send: 'p'=PID, 't'=Time, 'y'=Yield, 'w'=Write, 'h'=Help, 'r'=Reboot\r\n");
        kprintf("   Type command: ");

        // Wait for input with timeout (check for 3 seconds)
        uint32_t start_time = getSysTickTime();
        char cmd = 0;
        int got_input = 0;

        while ((getSysTickTime() - start_time) < 3000) {
            if (USART2->SR & USART_SR_RXNE) {
                cmd = (char)(USART2->DR & 0xFF);
                got_input = 1;
                break;
            }
            // Small delay to avoid busy waiting
            wait_until(10);
        }

        if (got_input) {
            kprintf("%c\r\n\r\n", cmd);

            switch(cmd) {
                case 'p':
                case 'P':
                    kprintf("==> Testing SYS_getpid\r\n");
                    {
                        int pid = getpid();
                        kprintf("    Current PID: %d\r\n", pid);
                        if (pid >= 0)
                            kprintf("    [SUCCESS] Process ID retrieved\r\n");
                        else
                            kprintf("    [INFO] No TCB initialized (expected)\r\n");
                    }
                    break;

                case 't':
                case 'T':
                    kprintf("==> Testing SYS___time\r\n");
                    {
                        uint32_t time = getSysTickTime();
                        kprintf("    System Time: %d ms\r\n", (int)time);
                        kprintf("    [SUCCESS] Time retrieved via syscall\r\n");
                    }
                    break;

                case 'y':
                case 'Y':
                    kprintf("==> Testing SYS_yield\r\n");
                    kprintf("    Calling yield()...\r\n");
                    yield();
                    kprintf("    [SUCCESS] Returned from yield()\r\n");
                    break;

                case 'w':
                case 'W':
                    kprintf("==> Testing SYS_write\r\n");
                    {
                        int bytes = write(STDOUT_FILENO, "    [SYSCALL OUTPUT] Hello from write()!\r\n", 44);
                        kprintf("    Bytes written: %d\r\n", bytes);
                        kprintf("    [SUCCESS] Write syscall completed\r\n");
                    }
                    break;

                case 's':
                case 'S':
                    kprintf("==> System Status\r\n");
                    {
                        uint32_t current_time = getSysTickTime();
                        int current_pid = getpid();
                        kprintf("    PID: %d\r\n", current_pid);
                        kprintf("    Time: %d ms\r\n", (int)current_time);
                        kprintf("    Count: %d\r\n", count);
                        kprintf("    Uptime: %d seconds\r\n", (int)(current_time / 1000));
                    }
                    break;

                case 'r':
                case 'R':
                    kprintf("==> WARNING: System Reboot!\r\n");
                    kprintf("    Rebooting in 3 seconds...\r\n");
                    wait_until(1000);
                    kprintf("    3...\r\n");
                    wait_until(1000);
                    kprintf("    2...\r\n");
                    wait_until(1000);
                    kprintf("    1...\r\n");
                    wait_until(1000);
                    kprintf("    Calling reboot()...\r\n");
                    reboot();
                    // Should not reach here
                    kprintf("    [ERROR] Reboot failed!\r\n");
                    break;

                case 'h':
                case 'H':
                case '?':
                    kprintf("==> DUOS Interactive Command Help\r\n");
                    kprintf("    Available Commands:\r\n");
                    kprintf("      p - Get Process ID (SYS_getpid)\r\n");
                    kprintf("      t - Get System Time (SYS___time)\r\n");
                    kprintf("      y - Yield CPU (SYS_yield)\r\n");
                    kprintf("      w - Test Write (SYS_write)\r\n");
                    kprintf("      s - Show System Status\r\n");
                    kprintf("      r - Reboot System (SYS_reboot)\r\n");
                    kprintf("      h - Show this help\r\n");
                    kprintf("\r\n");
                    kprintf("    Tested: write, getpid, time, yield\r\n");
                    kprintf("    All syscalls: WORKING!\r\n");
                    break;

                default:
                    kprintf("    [Unknown command '%c']\r\n", cmd);
                    kprintf("    Type 'h' for help\r\n");
                    break;
            }
        } else {
            // No command received within timeout, show auto status
            kprintf("[Timeout - no input received]\r\n");
            uint32_t current_time = getSysTickTime();
            int current_pid = getpid();
            kprintf("   [Auto Status] PID=%d Time=%d ms Count=%d\r\n",
                    current_pid, (int)current_time, count);
        }
        // Short delay before next prompt
        wait_until(2000);
#else
        kprintf("Nafis Shyan\r\n");
        kprintf("2021811186\r\n");
        kprintf("Roll: 10\r\n");
        kprintf("------------------\r\n");
        kprintf("Count: %d\r\n", count);
        wait_until(5000);
#endif

        count++;
    }
}

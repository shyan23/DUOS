  What I Now Understand (95%+ confidence):

  1. SVC Number Extraction

  - The SVC immediate value encodes the syscall number (e.g., SVC #50 for SYS_read)
  - Extract from stacked PC by reading the instruction before the saved PC

  2. Argument Passing

  - Arguments passed via registers R0-R3 (standard AAPCS)
  - First argument (R0) must be the service ID
  - Remaining arguments follow in R1, R2, R3

  3. Stack Frame Structure

  - Hardware auto-stacks: R0-R3, R12, LR, PC, xPSR
  - Use EXC_RETURN in LR to determine which stack (MSP/PSP)
  - Test bit 2 of LR: if set, use PSP; otherwise MSP

  4. Return Values

  - Return values go in R0 (standard convention)
  - Modify stacked R0 in the stack frame before returning

  5. Syscalls to Implement

  Required: SYS_exit, SYS_getpid, SYS_read, SYS_write, SYS___time, SYS_reboot, SYS_yield

  6. User Space Wrappers

  - Implement in userland/utils/unistd.c
  - Use inline assembly to execute SVC with service ID as first argument

  7. Privilege Switching

  - Start in privileged mode after reset
  - Switch to unprivileged (Thread mode) before running user tasks
  - Use __set_CONTROL(0x01) to enter unprivileged mode

  What I Still Need Clarification On:

  1. Kernel Function Implementations (Medium Uncertainty ~70%)

  For the actual kernel service functions, I need to know:

  - SYS_read: Should this call an existing USART driver function? What's the function signature?
    - Does sys_usart.c have a function like USART_ReadString() or USART_ReceiveChar()?
  - SYS_write: Similar question - what USART write function exists?
    - USART_SendString()? USART_TransmitChar()?
  - SYS_getpid: Where is the current task ID stored?
    - Is there a global current_task pointer to TCB?
    - Is there a scheduler module I should reference?
  - SYS___time: Should this call __getTime() from cm4.c:119?
  - SYS_reboot: Should I write to SCB->AIRCR directly or is there a system function?
  - SYS_yield: Should this trigger PendSV by setting SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk?

  2. SVCall_Handler Implementation Details (Low-Medium Uncertainty ~80%)

  Should the implementation be:

  Option A: Pure Assembly (Naked Function)
  __attribute__((naked)) void SVCall_Handler(void) {
      asm volatile(
          "TST lr, #4\n"
          "ITE EQ\n"
          "MRSEQ r0, MSP\n"
          "MRSNE r0, PSP\n"
          "B SVCall_Handler_C\n"
      );
  }

  void SVCall_Handler_C(uint32_t *stack_frame) {
      // Extract args, call syscall(), modify stack_frame[0] for return
  }

  Option B: Simpler C-based approach
  void SVCall_Handler(void) {
      // Read stacked registers
      // Extract SVC number from instruction
      // Call syscall() with appropriate arguments
  }

  3. User Mode Test Code (Medium Uncertainty ~75%)

  - Should I modify kmain() to:
    a. Initialize system
    b. Switch to unprivileged mode
    c. Call test functions (that use SVC)?
  - Or should there be a separate user task structure?

  4. Error Handling Implementation (Medium Uncertainty ~70%)

  - Should I set a global errno variable?
  - Or return negative error codes in R0?
  - The assignment mentions ENOSYS - where is errno.h and how should it be used?

  5. Extracting SVC Number (Low Uncertainty ~85%)

  I believe I should:
  uint32_t *stacked_pc = (uint32_t *)stack_frame[6];
  uint8_t *svc_instruction = ((uint8_t *)stacked_pc) - 2;
  uint8_t svc_number = svc_instruction[0];
  But should I read the immediate from the instruction, or should the service ID be passed in R0?

  6. CMSIS Functions (Low Uncertainty ~90%)

  - Are __get_CONTROL() and __set_CONTROL() already available via CMSIS?
  - Or do I need to implement them with inline assembly?

  Most Critical Questions:

  1. Are there existing USART driver functions I should call from SYS_read/SYS_write? What are their names?
  2. Is there a current task/TCB pointer that SYS_getpid should reference?
  3. Should the SVC number be extracted from the instruction, or passed in R0? (The assignment says "first argument must be
  service ID" which suggests R0)
  4. Where should test code go? In kmain.c or elsewhere?
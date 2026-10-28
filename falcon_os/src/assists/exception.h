#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdint.h>

#define EXCEPTION_CPU_STATE_REGISTERS 15

// Matches the stack frame pushed by isr.asm and the CPU exception frame.
typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) cpu_state_t;

void calamity_panic(cpu_state_t *state);
void exception_handler(cpu_state_t *state);

#endif

#include "exception.h"

void calamity_panic(cpu_state_t *state) {
    // TODO: Print to your framebuffer or serial port using your kernel text output
    // printf("[CALAMITY] KERNEL PANIC! Exception Vector: %d, Error Code: %d\n", state->int_no, state->error_code);
    // printf("RIP: 0x%llx | RSP: 0x%llx\n", state->rip, state->rsp);

    (void)state;

    // Halt the CPU forever
    __asm__ volatile("cli");
    for (;;) {
        __asm__ volatile("hlt");
    }
}
#include "exception.h"

void calamity_panic(cpu_state_t *state) {
    // TODO: Print to your framebuffer or serial port using your kernel text output
    // printf("[CALAMITY] KERNEL PANIC! Exception Vector: %d, Error Code: %d\n", state->int_no, state->error_code);
    // printf("RIP: 0x%llx | RSP: 0x%llx\n", state->rip, state->rsp);
    // idk, ill figure something out.

    (void)state;

    // Halt the CPU forever. I hate having to do this. 
    //eventually figure out how to recover from a fatal exception.
    //like make our version of the BSOD or something. 
    // TODO: Remind me to make a master .json of all the error codes and their meanings/solutions. 
    __asm__ volatile("cli");
    for (;;) {
        __asm__ volatile("hlt");
    }
}
bits 64

global _start
extern kmain

section .text

_start:
    cli                         ; Disable hardware interrupts completely because we dont care.

    mov rsp, stack_top          ; Point to top of allocated stack
    and rsp, -16                ; Enforce 16-byte stack alignment for C ABI compliance
    cld                         ; Clear direction flag

    call kmain                  ; Jump to C kernel entry point

.hang:
    cli                         ; Ensure interrupts remain disabled
    hlt                         ; Halt CPU
    jmp .hang                   ; Guard against spurious wakeups

section .bss
align 16
stack_bottom:
    resb 16384                  ; 16 KB early boot stack
stack_top:

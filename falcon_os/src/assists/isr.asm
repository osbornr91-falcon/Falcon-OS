bits 64

extern exception_handler

; Macro for exceptions without error codes (pushes a dummy error code)
%macro ISR_NO_ERRCODE 1
global isr%1
isr%1:
    push 0          ; Dummy error code
    push %1         ; Interrupt vector number
    jmp isr_common_stub
%endmacro

; Macro for exceptions with error codes (CPU already pushed code)
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    push %1         ; Interrupt vector number
    jmp isr_common_stub
%endmacro

isr_common_stub:
    ; Save general-purpose registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Pass pointer to stack frame as argument to C handler
    mov rdi, rsp
    call exception_handler

    ; Restore registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx         ; (rcx)
    pop rbx         ; (rbx)
    pop rax

    add rsp, 16     ; Clean up vector number and error code
    iretq

; Generate first few essential CPU exceptions (0 = Division Error, 14 = Page Fault, etc.)
ISR_NO_ERRCODE 0
ISR_NO_ERRCODE 6  ; Invalid Opcode because SOMEBODY CANT FOLLOW INSTRUCTIONS!!
ISR_ERRCODE    14 ; Page Fault because clearly we have no idea what we are doing.
[bits 32]

%macro isr_err_stub 1
isr_stub_%+%1:
    push %1
    jmp ISRCommon
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    ; Push dummy so all interrupts have the same stack structure
    push 0x0
    push %1
    jmp ISRCommon
%endmacro

%include "arch/i686/isr_gen.inc"

extern i686_ISR_CommonHandler

ISRCommon:
    [bits 32]
    pushad

    push esp
    call i686_ISR_CommonHandler
    add esp, 4

    popad

    add esp, 8
    iret


section .data
global stubTable
stubTable:
%assign i 0 
%rep    256 
    dd isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep
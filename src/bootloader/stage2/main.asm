bits 16

section .entry

extern __bss_start
extern __end

extern cstart_
global entry

entry:
    cli

    mov [bootDrive], dl
    
    mov ax, ds  
    mov ss, ax
    mov sp, 0xFFF0
    mov bp, sp

    ; Enable protected mode
    call EnableA20
    lgdt [gdt_Descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp dword 08h:.pmode

.pmode:
    [bits 32]

    mov ax, 0x10
    mov ds, ax
    mov ss, ax
   
    ; clear bss (uninitialized data)
    mov edi, __bss_start
    mov ecx, __end
    sub ecx, edi
    mov al, 0
    cld
    rep stosb

    ; pass argument to c function
    xor edx, edx
    mov dl, [bootDrive]
    push edx
    call cstart_

    cli
    hlt


EnableA20:
    [bits 16]
    ; disable keyboard
    call A20WaitInput
    mov al, KbdControllerDisableKeyboard
    out KbdControllerCommandPort, al

    ; read control output port
    call A20WaitInput
    mov al, KbdControllerReadCtrlOutputPort
    out KbdControllerCommandPort, al

    call A20WaitOutput
    in al, KbdControllerDataPort
    push eax

    ; write control output port
    call A20WaitInput
    mov al, KbdControllerWriteCtrlOutputPort
    out KbdControllerCommandPort, al
    
    call A20WaitInput
    pop eax
    or al, 2                                    ; bit 2 = A20 bit
    out KbdControllerDataPort, al

    ; enable keyboard
    call A20WaitInput
    mov al, KbdControllerEnableKeyboard
    out KbdControllerCommandPort, al

    call A20WaitInput
    ret


A20WaitInput:
    [bits 16]
    ; wait until status bit 2 (input buffer) is 0
    ; by reading from command port, we read status byte
    in al, KbdControllerCommandPort
    test al, 2
    jnz A20WaitInput
    ret

A20WaitOutput:
    [bits 16]
    ; wait until status bit 1 (output buffer) is 1 so it can be read
    in al, KbdControllerCommandPort
    test al, 1
    jz A20WaitOutput
    ret

KbdControllerDataPort               equ 0x60
KbdControllerCommandPort            equ 0x64
KbdControllerDisableKeyboard        equ 0xAD
KbdControllerEnableKeyboard         equ 0xAE
KbdControllerReadCtrlOutputPort     equ 0xD0
KbdControllerWriteCtrlOutputPort    equ 0xD1

gdt_Table:
    ; First entry is 0
    dq 0

    ; 32-bit code segment
    ; Limit 0-15
    dw 0xFFFF
    ; Base (16-31) (32-39)
    dw 0
    db 0
    ; Access byte
    db 0b10011010
    ; Flags and high limit
    db 0b11001111
    ; High base
    db 0

    ; 32-bit data segment
    ; Limit 0-15
    dw 0xFFFF
    ; Base (16-31) (32-39)
    dw 0
    db 0
    ; Access byte
    db 0b10010010
    ; Flags and high limit
    db 0b11001111
    ; High base
    db 0

    ; 16-bit code segment
    ; Limit 0-15
    dw 0xFFFF
    ; Base (16-31) (32-39)
    dw 0
    db 0
    ; Access byte
    db 0b10011010
    ; Flags and high limit
    db 0b00001111
    ; High base
    db 0

    ; 16-bit data segment
    ; Limit 0-15
    dw 0xFFFF
    ; Base (16-31) (32-39)
    dw 0
    db 0
    ; Access byte
    db 0b10010010
    ; Flags and high limit
    db 0b00001111
    ; High base
    db 0

gdt_Descriptor: dw 31
                dd gdt_Table

bootDrive: db 0
[bits 32]

global loadIDT

; Load IDTR at first argument
loadIDT:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    lidt [eax]

    mov esp, ebp
    pop ebp
    ret
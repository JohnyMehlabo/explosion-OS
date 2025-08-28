[bits 32]

global loadGDT

loadGDT:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    lgdt [eax]

    mov eax, 0x8
    push eax
    push .reload
    retf
.reload:

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, ebp
    pop ebp
    ret
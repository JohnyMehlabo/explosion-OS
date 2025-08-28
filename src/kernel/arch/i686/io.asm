global i686_EnableInterrupts
i686_EnableInterrupts:
    sti
    ret

global i686_DisableInterrupts
i686_DisableInterrupts:
    cli
    ret

global i686_Halt
i686_Halt:
    cli
    hlt

global i686_inb
i686_inb:
    mov dx, [esp+4]
    xor eax, eax
    in al, dx
    ret

global i686_outb
i686_outb:
    mov dx, [esp+4]
    mov al, [esp+8]
    out dx, al
    ret

global i686_inw
i686_inw:
    mov dx, [esp+4]
    xor eax, eax
    in ax, dx
    ret

global i686_outw
i686_outw:
    mov dx, [esp+4]
    mov ax, [esp+8]
    out dx, ax
    ret

global i686_inl
i686_inl:
    mov dx, [esp+4]
    xor eax, eax
    in eax, dx
    ret

global i686_outl
i686_outl:
    mov dx, [esp+4]
    mov eax, [esp+8]
    out dx, eax
    ret
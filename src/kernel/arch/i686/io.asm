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

global i686_insw
i686_insw:
    push edi
    mov dx, [esp+8]
    mov edi, [esp+12]
    mov ecx, [esp+16]
    rep insw
    pop edi
    ret

global i686_outsw
i686_outsw:
    push esi
    mov dx, [esp+8]
    mov esi, [esp+12]
    mov ecx, [esp+16]
    rep outsw
    pop esi
    ret

global i686_insl
i686_insl:
    push edi
    mov dx, [esp+8]
    mov edi, [esp+12]
    mov ecx, [esp+16]
    rep insd
    pop edi
    ret
extern __kernel_start
extern __end
extern kstart

global stack_bottom
global stack_top
section .stack
stack_bottom:
    resb 16384 
stack_top:

section .page_tables
    align 4096
page_directory:
    resb 4096
page_tables:
    resb 0x10000

section .entry
; EDI Current PTE offset
; ESI Current memory offset
entry:
    mov eax, [ebp-4]
    mov edi, page_tables - 0xC0000000
    xor esi, esi
.loop:
    cmp esi, __kernel_start
    jl .before_start
    cmp esi, __end - 0xC0000000
    jge .reached_end

    mov edx, esi
    or edx, 3
    mov [edi], edx

.before_start:
    add edi, 4
    add esi, 4096
    jmp .loop

.reached_end:
    
    mov edi, page_tables - 0xC0000000 + 160 * 4
    mov esi, 0xa0000

.vga_map_loop:
    cmp esi, 0xc8000
    jge .end_vga_map_loop

    mov edx, esi
    or edx, 3
    mov DWORD [edi], edx ; Map graphic mode VGA buffer

    add edi, 4
    add esi, 0x1000

    jmp .vga_map_loop
.end_vga_map_loop:

    mov DWORD [page_directory - 0xC0000000 + 0], page_tables - 0xC0000000 + 3
    mov DWORD [page_directory - 0xC0000000 + 768 * 4], page_tables - 0xC0000000 + 3

    mov ecx, page_directory - 0xC0000000
    mov cr3, ecx

    mov ecx, cr0
    or ecx, 0x80010000
    mov cr0, ecx

    lea ecx, [higher_half]
    jmp dword ecx

section .text

higher_half:
    lea esp, [stack_top]
    mov ebp, esp
    
   mov DWORD [page_directory - 0xC0000000 + 0], 0

    mov [ebp-4], eax
    call kstart

    cli
    hlt





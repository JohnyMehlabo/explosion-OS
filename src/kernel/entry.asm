extern __kernel_start
extern __end
extern kstart

global stack_bottom
global stack_top
section .stack
stack_bottom:
    resb 16384 
stack_top:

global page_directory
section .page_tables
    align 4096
page_directory:
    resb 4096
page_tables:
    resb 0x10000

BOOTSTRAP_MEM equ 0x300000

section .entry
; EDI Current PTE offset
; ESI Current memory offset
entry:
    mov eax, [esp+4]
    mov edi, page_tables - 0xC0000000
    xor esi, esi
.loop:
    cmp esi, __kernel_start
    jl .before_start
    cmp esi, __end + BOOTSTRAP_MEM - 0xC0000000
    jge .reached_end

    mov edx, esi
    or edx, 3
    mov [edi], edx

.before_start:
    add edi, 4
    add esi, 4096
    jmp .loop

.reached_end:
    mov ebx, page_tables - 0xC0000000
    xor esi, esi

.directory_loop:
    mov ecx, ebx
    or ecx, 3

    mov DWORD [page_directory - 0xC0000000 + esi * 4], ecx
    mov DWORD [page_directory - 0xC0000000 + 768 * 4 + esi * 4], ecx

    inc esi
    add ebx, 0x1000

    cmp ebx, edi
    jge .finished_directory
    jmp .directory_loop

.finished_directory:
    ; Store the amount of page tables mapped for later unmapping them from the lower half
    mov ebx, esi

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

    mov edx, eax ; Copy boot info addr into edx for setting corresponding flags
    or edx, 3
    mov DWORD [page_tables - 0xc0000000 + 64 * 4], edx

    ; Map the directory recursively
    mov DWORD [page_directory - 0xc0000000 + 1023 * 4], page_directory - 0xc0000000 + 3

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
    
    xor esi, esi
.unmap_loop:
    ; TODO: Unmap the rest of the lower half
    mov DWORD [page_directory - 0xC0000000 + esi * 4], 0

    cmp esi, ebx
    jge .finished_unmapping

    inc esi
    jmp .unmap_loop

.finished_unmapping:
    add eax, 0xc0000000 ; Add the virtual offset to the boot info struct
    push eax
    call kstart

    cli
    hlt





[bits 16]

org 0x7c00

jmp short main
nop

bpb_oem_identifier: db "MSWIN4.1"
bpb_bytes_per_sector: dw 512
bpb_sectors_per_cluster: db 1
bpb_reserved_sectors: dw 1
bpb_fat_count:db 2
bpb_root_directory_entries: dw 224
bpb_sector_count: dw 2880
bpb_media_descriptor_type: db 0xF0
bpb_sectors_per_fat: dw 9
bpb_sectors_per_track: dw 18
bpb_head_count: dw 2
bpb_hidden_sector_count: dd 0
bpb_large_sector_count: dd 0

; EBR
ebr_drive_number: db 0
ebr_reserved: db 0
ebr_signature: db 0x29
ebr_serial_number: dd 0
ebr_volume_label: db "EXPLOSIONOS"
ebr_system_identifier: db "FAT12      "

main: 
    mov ax, 0           ; can't set ds/es directly
    mov ds, ax
    mov es, ax
        
    ; setup stack
    mov ss, ax
    mov sp, 0x7C00

    push es
    push word .after
    retf

.after:
    mov [ebr_drive_number], dl

    push es
    mov ah, 08h
    int 13h
    jc disk_error
    pop es

    and cl, 0x3F                        ; remove top 2 bits
    xor ch, ch
    mov [bpb_sectors_per_track], cx     ; sector count

    inc dh
    mov [bpb_head_count], dh                 ; head count

    ; Size of fat in sectors
    mov ax, [bpb_sectors_per_fat]
    mov bl, [bpb_fat_count]
    xor bh, bh
    mul bx
    ; Add reserved sectors
    add ax, [bpb_reserved_sectors]
    push ax

    mov ax, [bpb_root_directory_entries]
    shl ax, 5
    xor dx, dx
    div word [bpb_bytes_per_sector]

    test dx, dx
    jz .read_root_dir
    inc ax

.read_root_dir:
    mov cl, al
    pop ax
    mov dl, [ebr_drive_number]
    mov bx, buffer
    call disk_read

    xor bx, bx
    mov di, buffer
.find_file:
    mov si, stage2_bin_filename
    mov cx, 11

    push di
    rep cmpsb
    pop di

    je .found_stage2

    add di, 32
    inc bx
    cmp bx, [bpb_root_directory_entries]
    jl .find_file
    jmp .stage2_error

.found_stage2:
    mov ax, [di + 26]
    mov [stage2_cluster], ax

    mov ax, [bpb_reserved_sectors]
    mov bx, buffer
    mov cl, [bpb_sectors_per_fat]
    mov dl, [ebr_drive_number]
    call disk_read

    mov bx, STAGE_2_LOAD_SEGMENT
    mov es, bx
    mov bx, STAGE_2_LOAD_OFFSET

.load_stage2_loop:
    mov ax, [stage2_cluster]
    add ax, 31

    mov cl, 1
    mov dl, [ebr_drive_number]
    call disk_read

    add bx, [bpb_bytes_per_sector]

    ; compute location of next cluster
    mov ax, [stage2_cluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx                              ; ax = index of entry in FAT, dx = cluster mod 2

    mov si, buffer
    add si, ax
    mov ax, [ds:si]                     ; read entry from FAT table at index ax

    or dx, dx
    jz .even

.odd:
    shr ax, 4
    jmp .next_cluster_after

.even:
    and ax, 0x0FFF

.next_cluster_after:
    cmp ax, 0x0FF8                      ; end of chain
    jae .read_finish

    mov [stage2_cluster], ax
    jmp .load_stage2_loop

.read_finish:
    ; jump to our stage2
    mov dl, [ebr_drive_number]          ; boot device in dl

    mov ax, STAGE_2_LOAD_SEGMENT         ; set segment registers
    mov ds, ax
    mov es, ax

    jmp STAGE_2_LOAD_SEGMENT:STAGE_2_LOAD_OFFSET
    jmp halt


.stage2_error:
    mov si, stage2_error_text
    call puts


halt:
    ; Halt processor
    cli
    hlt

; Functions
puts:
    push ax
    push bx
    push si
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .loop
.done:
    pop ax
    pop bx
    pop si
    ret

; Functions
puts_n:
    push ax
    push bx
    push cx
    push si
.loop:
    lodsb
    cmp cx, 0 
    je .done
    dec cx
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .loop
.done:
    pop si
    pop cx
    pop bx
    pop ax
    ret

;   Input:
;       ax = LBA
;
;   Output:
;       cl[0-5] = sector
;       cl[6-7] = cylinder
;       dh = head
;
lba_to_chs:
    push ax
    push dx

    xor dx, dx
    div word [bpb_sectors_per_track] ; ax = LBA / SPT
                                ; dx = LBA % SPT

    inc dx
    mov cx, dx
    
    xor dx, dx
    div word [bpb_head_count] ; dx = (LBA/SPT) % HPC
    mov dh, dl ; dh = Heads
    mov ch, al
    shl ah, 6
    or cl, ah

    pop ax
    mov dl, al                          ; restore DL
    pop ax
    ret

; ax = LBA
; cl = sector count
; dl = unit
; es:bx = buffer
disk_read:
    push ax
    push bx
    push cx
    push dx

    push cx
    call lba_to_chs
    pop ax

    mov ah, 0x02
    int 0x13
    jc disk_error

    pop dx
    pop cx
    pop bx
    pop ax
    ret

disk_error:
    mov si, disk_error_text
    call puts 
    jmp halt

stage2_bin_filename: db "STAGE2  BIN"
stage2_error_text: db "Stage2 not found", 0
disk_error_text: db "Disk error", 0
stage2_cluster: dw 0

STAGE_2_LOAD_SEGMENT equ 0x0
STAGE_2_LOAD_OFFSET equ 0x500

times 510-($-$$) db 0
dw 0xAA55

buffer:

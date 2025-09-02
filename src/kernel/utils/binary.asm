section .text

global _BSF
_BSF:
    mov edx, [esp+4]
    bsf eax, edx
    ret
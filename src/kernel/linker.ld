ENTRY(start)
OUTPUT_FORMAT("binary")
phys = 0x00100000;
virt = 0xC0000000;

SECTIONS
{
    . = phys;

    __kernel_start = .;
    .entry              : { __entry_start = .;      *(.entry)   }

    . += virt;

    .text               : AT(ADDR(.text) - virt) ALIGN(4k) { __text_start = .;       *(.text)    }
    .data               : AT(ADDR(.data) - virt) ALIGN(4k) { __data_start = .;       *(.data)    }
    .rodata             : AT(ADDR(.rodata) - virt) ALIGN(4k) { __rodata_start = .;     *(.rodata)  }
    .bss                : AT(ADDR(.bss) - virt) ALIGN(4k) { 
        __bss_start = .;        
        *(.bss)
        __bss_end = .;        
        *(.stack)
    }
    .page_tables : AT(ADDR(.page_tables) - virt) ALIGN(4k) { __page_tables_start = .; *(.page_tables) }
    
    __end = ALIGN(4k);
}
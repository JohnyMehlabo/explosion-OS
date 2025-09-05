#pragma once

#define TOTAL_MEMORY_SIZE 0x100000000
#define PAGE_SIZE 0x1000

#define KERNEL_VIRTUAL_ADDR 0xc0000000
#define BOOTSTRAP_MEM_SIZE 0x300000

// Virtual address where the 
#define PAGE_DIRECTORY_ADDR 0xfffff000
// Addresses where the page directory is mapped recursively
#define PAGE_TABLE_ENTRIES_ADDR 0xffc00000
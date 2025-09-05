#include "vmm.h"
#include "bootstrap_alloc.h"
#include "pmm.h"
#include "stdio.h"
#include "memory.h"
#include "memdefs.h"
#include "arch/i686/isr.h"
#include "arch/i686/io.h"

#define KERNEL_HALF_SIZE (TOTAL_MEMORY_SIZE - KERNEL_VIRTUAL_ADDR)
#define KERNEL_HALF_PAGE_COUNT (KERNEL_HALF_SIZE / PAGE_SIZE)

#define PTE_RESERVED_BIT 0b100000000

typedef struct Block_ {
    uint8_t* start;
    uint32_t size;
    struct Block_* next;
} Block;

Block* listStart;
Block* memoryBuffer;

extern uint8_t __end;

void VMM_PageFaultHandler(StackData* data) {
    // If first bit set the page fault was caused by a page-protection violation
    if (data->errorCode & 1) {
        printf("FATAL ERROR: Page-protection violation to address 0x%x\n", data->cr2);
        i686_Halt();
    } else {
        uint32_t page = data->cr2 / PAGE_SIZE;
        uint32_t directoryEntry = (data->cr2 / PAGE_SIZE) / 1024;

        // Check if directory entry is present. If it is not then the page can't be reserved
        // as reserving a page creates the underlying page table page
        if (*(((uint32_t*)PAGE_DIRECTORY_ADDR) + directoryEntry) & 1) {   
            uint32_t* pagePtr = ((uint32_t*)PAGE_TABLE_ENTRIES_ADDR) + page;
            if (*pagePtr & PTE_RESERVED_BIT) {
                uint32_t frame = PMM_GetFrames(1);

                *pagePtr |= frame << 12; // Set the physical address
                *pagePtr |= 3; // Set present and supervisor flags
                *pagePtr &= ~PTE_RESERVED_BIT; // Remove reserved bit

                return;
            }
        }
        printf("FATAL ERROR: Accessing non-present page at address 0x%x\n", data->cr2);
        i686_Halt();            
    }
}

void VMM_Init() {
    memoryBuffer = bmalloc((KERNEL_HALF_PAGE_COUNT / 2) * sizeof(Block));
    memset(memoryBuffer, 0x00, (KERNEL_HALF_PAGE_COUNT / 2) * sizeof(Block)); // Fill everything with zeroes to mark every entry as empty

    // First entry containing all available space
    listStart = &memoryBuffer[0];
    memoryBuffer[0].start = &__end + BOOTSTRAP_MEM_SIZE;
    memoryBuffer[0].size = TOTAL_MEMORY_SIZE - ((uint32_t)&__end + BOOTSTRAP_MEM_SIZE);
    memoryBuffer[0].next = NULL;

    uint32_t* pageDirectory = (uint32_t*)PAGE_DIRECTORY_ADDR;
    for (uint32_t i = 0; i < 1024; i++) {
        // If page table is not present, set it as reserved
        if (!(pageDirectory[i] & 1)) {
            pageDirectory[i] |= PTE_RESERVED_BIT;
        }
    }

    // Setup page fault exception handler
    i686_ISR_SetExceptionHandler(14, VMM_PageFaultHandler);
}

Block* VMM_FindBlockStruct() {
    for (uint32_t i = 0; i < (KERNEL_HALF_PAGE_COUNT / 2); i++) {
        if (memoryBuffer[i].start == NULL)
            return &memoryBuffer[i];
    }

    return NULL;
}

// TODO: We don't mark pages as reserved in the PTE, well, because we dont have that
void* VMM_AllocateBlock(uint32_t count) {
    Block* previous = NULL;
    Block* current = listStart;
    uint32_t size = count * PAGE_SIZE;
    
    void* address = NULL;

    while (current != NULL) {
        if (current->size == size) {
            address = current->start;
            // The block is completely gone. Either we set the list start if it is at the beginning 
            // or set the previous "next" block to the current "next" block
            if (previous == NULL) {
                listStart = current->next;
            } else {
                previous->next = current->next;
            }
            current->start = NULL;
            break;
        } else if (current->size > size) {
            address = current->start;
            current->start += count * PAGE_SIZE;
            current->size -= count * PAGE_SIZE;
            break;
        }
        previous = current;
        current = current->next;
    }

    return address;
}

void* VMM_ReservePages(uint32_t count) {
    uint8_t* base = VMM_AllocateBlock(count);
    uint32_t* pageTables = (uint32_t*)PAGE_TABLE_ENTRIES_ADDR;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t pageIndex = (uint32_t)base >> 12 + i;

        pageTables[pageIndex] |= PTE_RESERVED_BIT;
    }

    return base;
}

void* VMM_AllocatePages(uint32_t count, bool contiguous) {
    uint8_t* base = VMM_AllocateBlock(count);
    uint32_t* pageTables = (uint32_t*)PAGE_TABLE_ENTRIES_ADDR;
    uint32_t physicalBase;

    if (contiguous) {
        physicalBase = PMM_GetFrames(count);
    }

    for (uint32_t i = 0; i < count; i++) {
        uint32_t pageIndex = ((uint32_t)base >> 12) + i;
        uint32_t page;

        if (contiguous) {
            page = physicalBase + i;
        } else {
            page = PMM_GetFrames(1);
        }

        pageTables[pageIndex] |= page << 12;
        pageTables[pageIndex] |= 3; // Set present and supervisor bits
    }

    return base;
}

void VMM_FreePages(uint8_t* address, uint32_t count) {
    Block* previous = NULL;
    Block* current = listStart;
    uint32_t size = count * PAGE_SIZE;

    bool coalesceLeft = false, coalesceRight = false;
    

    while (current != NULL) {
        if (current->start > address) {
            break;
        }
        previous = current;
        current = current->next;
    }

    if (previous == NULL) {

    } else if (previous->start + previous->size == address) {
        coalesceLeft = true;
    }
    if (current == NULL) {

    } else if (current->start == address + size) {
        coalesceRight = true;
    }

    if (!coalesceLeft && !coalesceRight) {
        Block* newBlock = VMM_FindBlockStruct();
        newBlock->start = address;
        newBlock->size = size;
        newBlock->next = current;

        if (previous == NULL) {
            listStart = newBlock;
        } else {
            previous->next = newBlock;
        }
    } else if (coalesceLeft && coalesceRight) {
        // We merge the three blocks, the one being freed, the previous one and the one after
        previous->size += size + current->size;
        previous->next = current->next;
        current->start = NULL;
    } else if (coalesceLeft) {
        previous->size += size;
    } else if (coalesceRight) {
        current->start -= size;
        current->size += size;
    }

    // Clear page tables and deallocate physical memory
    uint32_t* pageTables = (uint32_t*)PAGE_TABLE_ENTRIES_ADDR;
    for (uint32_t i = 0; i < count; i++) {
        uint32_t pageIndex = (uint32_t)address >> 12 + i;

        pageTables[pageIndex] &= ~1;
        PMM_FreeFrames(pageTables[pageIndex] >> 12, 1);
    }
}

void* VMM_GetPhysicalMapping(void* addr) {
    uint32_t* pageAddr = ((uint32_t*)PAGE_TABLE_ENTRIES_ADDR) + ((uint32_t)addr >> 12);
    return (void*)(*pageAddr & 0xfffff000);
}
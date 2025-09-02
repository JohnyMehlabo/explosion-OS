#include "pmm.h"
#include "stdint.h"
#include "memory/bootstrap_alloc.h"
#include "memory.h"
#include "stdio.h"
#include "memdefs.h"
#include "utils/binary.h"

uint32_t* bitmap0;
uint32_t* bitmap1;
uint32_t* bitmap2;
uint32_t* bitmap3;
uint32_t* bitmap4;

static uint32_t* bitmaps[5];

#define MEMORY_SIZE 0x100000000
#define PAGE_SIZE 0x1000
#define PAGE_COUNT (MEMORY_SIZE / PAGE_SIZE)

extern uint8_t __end;

void PMM_Init(const SMAPEntry* smapEntries, uint32_t entryCount) {

    // Initialize all bitmaps to 1
    bitmap0 = bmalloc(((PAGE_COUNT / 1) / 32) * sizeof(uint32_t)); 
    memset(bitmap0, 0xff, ((PAGE_COUNT / 1) / 32) * sizeof(uint32_t));
    bitmap1 = bmalloc(((PAGE_COUNT / 2) / 32) * sizeof(uint32_t));
    memset(bitmap1, 0xff, ((PAGE_COUNT / 2) / 32) * sizeof(uint32_t));
    bitmap2 = bmalloc(((PAGE_COUNT / 4) / 32) * sizeof(uint32_t));
    memset(bitmap2, 0xff, ((PAGE_COUNT / 4) / 32) * sizeof(uint32_t));
    bitmap3 = bmalloc(((PAGE_COUNT / 8) / 32) * sizeof(uint32_t));
    memset(bitmap3, 0xff, ((PAGE_COUNT / 8) / 32) * sizeof(uint32_t));
    bitmap4 = bmalloc(((PAGE_COUNT / 16) / 32) * sizeof(uint32_t));
    memset(bitmap4, 0xff, ((PAGE_COUNT / 16) / 32) * sizeof(uint32_t));

    bitmaps[0] = bitmap0;
    bitmaps[1] = bitmap1;
    bitmaps[2] = bitmap2;
    bitmaps[3] = bitmap3;
    bitmaps[4] = bitmap4;

    uint32_t memoryStart = (uint32_t)(&__end - KERNEL_VIRTUAL_ADDR) + BOOTSTRAP_MEM_SIZE;

    uint32_t smapIndex = 0;
    uint32_t addr = (memoryStart + 0xfff) & ~0xfff;

    uint32_t currBase = 0;
    uint32_t currSize = 0;
    uint32_t remainingSize = 0;

    for (smapIndex; smapIndex < entryCount; smapIndex++) {
        // Search trough list to get the the current entry we are located in
        if (smapEntries[smapIndex].baseLo < addr && addr < smapEntries[smapIndex].baseLo + smapEntries[smapIndex].lengthLo) {
            currBase = smapEntries[smapIndex].baseLo;
            currSize = smapEntries[smapIndex].lengthLo;
            remainingSize = currBase + currSize - addr;
            break;
        }
    }

    while (1) {
        if (currBase + currSize < addr) {
            for (smapIndex; smapIndex < entryCount; smapIndex++) {
                // Search trough list to get the the current entry we are located in
                if (smapEntries[smapIndex].baseLo <= addr && addr < smapEntries[smapIndex].baseLo + smapEntries[smapIndex].lengthLo) {
                    currBase = smapEntries[smapIndex].baseLo;
                    currSize = smapEntries[smapIndex].lengthLo;
                    remainingSize = currBase + currSize - addr;
                    break;
                }
            }
            // Check if we have reached the end of the list
            if (smapIndex == entryCount)
                break;
        }
        // Check if memory type is not available
        if (smapEntries[smapIndex].type != 1) {
            addr += 0x1000; 
        }
        else if (remainingSize >= 0x10000 && (addr % 0x10000) == 0) {
            // 64KiB block
            uint32_t blockIndex = addr / 0x10000;
            uint32_t bitmapIndex = blockIndex / 32;
            uint32_t bitIndex = blockIndex % 32;
            addr += 0x10000;
            remainingSize -= 0x10000;

            bitmap4[bitmapIndex] &= ~(1 << bitIndex);
        }
        else if (remainingSize >= 0x8000 && (addr % 0x8000) == 0) {
            // 32KiB block
            uint32_t blockIndex = addr / 0x8000;
            uint32_t bitmapIndex = blockIndex / 32;
            uint32_t bitIndex = blockIndex % 32;
            addr += 0x8000;
            remainingSize -= 0x8000;

            bitmap3[bitmapIndex] &= ~(1 << bitIndex);
        }
        else if (remainingSize >= 0x4000 && (addr % 0x4000) == 0) {
            // 16KiB block
            uint32_t blockIndex = addr / 0x4000;
            uint32_t bitmapIndex = blockIndex / 32;
            uint32_t bitIndex = blockIndex % 32;
            addr += 0x4000;
            remainingSize -= 0x4000;

            bitmap2[bitmapIndex] &= ~(1 << bitIndex);
        }
        else if (remainingSize >= 0x2000 && (addr % 0x2000) == 0) {
            // 8KiB block
            uint32_t blockIndex = addr / 0x2000;
            uint32_t bitmapIndex = blockIndex / 32;
            uint32_t bitIndex = blockIndex % 32;
            addr += 0x2000;
            remainingSize -= 0x2000;

            bitmap1[bitmapIndex] &= ~(1 << bitIndex);
        }
        else if (remainingSize >= 0x1000) {
            uint32_t blockIndex = addr / 0x1000;
            uint32_t bitmapIndex = blockIndex / 32;
            uint32_t bitIndex = blockIndex % 32;
            addr += 0x1000;
            remainingSize -= 0x1000;

            bitmap0[bitmapIndex] &= ~(1 << bitIndex);
        } else {
            // No space. Advance to next smap entry
            addr += 0x1000;
            smapIndex++;

            // Check if last entry
            if (smapIndex == entryCount)
                break;

            currBase = smapEntries[smapIndex].baseLo;
            currSize = smapEntries[smapIndex].lengthLo;
            remainingSize = currBase + currSize - addr;
        }
    }
}

uint32_t PMM_FindAndSplit(uint32_t level) {

    // The last level asked for a block to split. This means there is no free memory
    if (level == 5) return 0;

    uint32_t i;
    for (i = 0; i < (PAGE_COUNT / 32) / (1 << level); i++) {
        if (bitmaps[level][i] != 0xffffffff) {
            uint32_t bitIndex = _BSF(~bitmaps[level][i]);
            bitmaps[level][i] |= (1 << bitIndex);

            return (i * 32 + bitIndex) * (1 << level);
        }
    }
    
        uint32_t address = PMM_FindAndSplit(level + 1);
        if (address != 0) {
            bitmaps[level][(address / 32) / (1 << level)] |= 1 << ((address / (1 << level)) % 32); // Mark first page as used
            bitmaps[level][((address + (1 << level)) / 32) / (1 << level)] &= ~(1 << (((address+(1 << level)) / (1 << level)) % 32)); // Mark second page as free
            return address;
        }
}

uint32_t PMM_GetFrames(uint32_t count) {
    if (count == 1) {
        return PMM_FindAndSplit(0);
    } else if (count == 2) {
        return PMM_FindAndSplit(1);
    } else if (count <= 4) {
        uint32_t address = PMM_FindAndSplit(2);
        if (count == 3)
            bitmap0[(address+3) / 32] &= ~(1 << ((address+3) % 32));
        return address;
    } else if (count <= 8) {
        uint32_t address = PMM_FindAndSplit(3);
        if (count == 8) return address;
        uint32_t remaining = 8 - count;
        if (remaining & 0b1) {
            bitmap0[(address+count) / 32] &= ~(1 << ((address+count) % 32));
            count += 1;
        } else if (remaining & 0b10) {
            bitmap1[((address+count) / 32) / 2] &= ~(1 << (((address+count) / 2) % 32)); // Mark first page as used
        }
        return address;
    } else if (count <= 16) {
        uint32_t address = PMM_FindAndSplit(4);
        if (count == 16) return address;

        // Now we have to check if we have to return some pages to lower allocators
        uint32_t remaining = 16 - count;
        if (remaining & 0b1) {
            bitmap0[(address+count) / 32] &= ~(1 << ((address+count) % 32));
            count += 1;
        } else if (remaining & 0b10) {
            bitmap1[((address+count) / 32) / 2] &= ~(1 << (((address+count) / 2) % 32));
            count += 2;
        } else if (remaining & 0b100) {
            bitmap1[((address+count) / 32) / 4] &= ~(1 << (((address+count) / 4) % 32));
        }
        return address;
    }

    // Nothing was found, return 0
    return 0;
}

void PPM_FreeAndCoalesce(uint8_t level, uint32_t address) {

    if (level == 4) {
        bitmaps[4][((address) / 32) / 16] &= ~(1 << (((address) / 16) % 32));
    }

    if (address / (1 << level) % 2 == 0 && (~bitmaps[level][((address + (1 << level)) / 32) / (1 << level)]) & (1 << ((address / (1 << level) + 1) % 32))) {
        // If two subsequent free pages were to exist, se the other one and tell the next layer to free itself
        bitmaps[level][((address + (1 << level)) / 32) / (1 << level)] |= (1 << ((address / (1 << level) + 1) % 32));
        PPM_FreeAndCoalesce(level+1, address);
    } else if (address / (1 << level) % 2 == 1 && (~bitmaps[level][((address - (1 << level)) / 32) / (1 << level)]) & (1 << ((address / (1 << level) - 1) % 32))) {
        // Check the page before
        bitmaps[level][((address - (1 << level)) / 32) / (1 << level)] |= (1 << ((address / (1 << level) - 1) % 32));
        PPM_FreeAndCoalesce(level+1, address - (1 << level));
    } else {
        // If not just free the current page
        bitmaps[level][(address / 32) / (1 << level)] &= ~(1 << ((address / (1 << level)) % 32));
    }
}

void PMM_FreeFrames(uint32_t address, uint32_t count) {
    if (count == 16) {
        bitmap4[((address) / 32) / 16] &= ~(1 << (((address) / 16) % 32)); // If max size just free page as no coalescation can occur
    }
    if (count & 0b1000) {
        PPM_FreeAndCoalesce(3, address);
        address += 8;
    }
    if (count & 0b100) {
        PPM_FreeAndCoalesce(2, address);
        address += 4;
    }
    if (count & 0b10) {
        PPM_FreeAndCoalesce(1, address);
        address += 2;
    }
    if (count & 0b1) {
        PPM_FreeAndCoalesce(0, address);
        address += 1;
    }
}

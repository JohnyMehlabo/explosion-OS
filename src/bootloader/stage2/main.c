#include "stdint.h"
#include "stdio.h"
#include "disk.h"
#include "fat.h"
#include "memdefs.h"

void printOSInformation(DISK* disk) {

    FAT_File* kernelFile = FAT_Open(disk, "intro.txt");
    char text[1024];
    uint32_t read = FAT_Read(disk, kernelFile, 1023, &text);
    text[read] = '\0';

    puts(text);

    // puts(" _____           _           _               _____ _____ \n"
    //      "|  ___|         | |         (_)             |  _  /  ___|\n"
    //      "| |____  ___ __ | | ___  ___ _  ___  _ __   | | | \\ `--. \n"
    //      "|  __\\ \\/ / '_ \\| |/ _ \\/ __| |/ _ \\| '_ \\  | | | |`--. \\\n"
    //      "| |___>  <| |_) | | (_) \\__ \\ | (_) | | | | \\ \\_/ /\\__/ /\n"
    //      "\\____/_/\\_\\ .__/|_|\\___/|___/_|\\___/|_| |_|  \\___/\\____/ \n"
    //      "          | |                                            \n"
    //      "          |_|                                            \n\n\n");

    // puts("Hello World from C!\nThis is Explosion OS, a simple operating system made for learning\n\nStages completed:\n - Bootloader stage 1: Load stage 2 from FAT into memory\n - Bootloader stage 2: Setup the CPU into protected mode and load kernel from C\nThis is stage 2 of the bootloader\n\n");
}

typedef void (*KernelStart)(uint16_t bootDrive);

void __attribute__((cdecl)) cstart_(uint16_t bootDrive) {
    clrscr();
    
    
    DISK disk;
    DISK_InitializeDisk(bootDrive, &disk);
    FAT_Initialize(&disk);
    
    printOSInformation(&disk);

    puts("Cylinders: "); putd(disk.cylinders); putc('\n');
    puts("Heads: "); putd(disk.heads); putc('\n');
    puts("Sectors: "); putd(disk.sectors); putc('\n');

    
    FAT_File* kernelFile = FAT_Open(&disk, "kernel.bin");
    FAT_Read(&disk, kernelFile, kernelFile->size, (void*)MEMORY_KERNEL_ADDR);

    KernelStart kernelStart = (void*)MEMORY_KERNEL_ADDR;
    kernelStart(bootDrive);

end:
    for(;;);    
}
#include "ide.h"
#include "arch/i686/io.h"
#include "../pci.h"
#include "stdio.h"
#include "arch/i686/irq.h"

#define IDE_PCI_CLASS 0x1
#define IDE_PCI_SUBCLASS 0x1


// Command/Status port bit masks
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

// Error port bit masks
#define ATA_ER_BBK      0x80    // Bad block
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // Media changed
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // Media change request
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

// Command/Status port commands
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

// Identification space offsets
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

// Interface type
#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

// Channels
#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01

// Directions
#define      ATA_READ      0x00
#define      ATA_WRITE     0x01

// Access modes
#define      ATA_CHS       0x00
#define      ATA_LBA28     0x01
#define      ATA_LBA48     0x02

// Access modes
#define      ATA_PIO       0x00
#define      ATA_DMA       0x01

// Task file
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

struct IDEChannel {
    uint32_t base;
    uint32_t ctrl;
    uint32_t bmide;
    uint8_t noInterrupts;
} channels[2];

struct IDEDevice {
    bool reserved;
    uint8_t channel;
    uint8_t drive;
    uint16_t type;
    uint16_t signature;
    uint16_t capabilities;
    uint32_t commandSets;
    uint32_t size;
    uint8_t model[41];
} devices[4];

void IDE_Write(uint8_t channel, uint8_t reg, uint8_t data) {
    if (reg > 0x7 && reg < 0xc)
        IDE_Write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].noInterrupts);
    if (reg < 0x8) 
        i686_outb(channels[channel].base + reg, data);
    else if (reg < 0xc)
        i686_outb(channels[channel].base + reg - 0x6, data);
    else if (reg < 0xe)
        i686_outb(channels[channel].ctrl + reg - 0xa, data);
    else if (reg < 0x16)
        i686_outb(channels[channel].bmide + reg - 0xe, data);
    if (reg > 0x7 && reg < 0xc)
        IDE_Write(channel, ATA_REG_CONTROL, channels[channel].noInterrupts);
}

void IDE_ReadBuffer(uint8_t channel, uint8_t reg, uint32_t* buffer, uint32_t quadCount) {
    if (reg > 0x7 && reg < 0xc)
        IDE_Write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].noInterrupts);
    if (reg < 0x8) 
        i686_insl(channels[channel].base + reg, buffer, quadCount);
    else if (reg < 0xc)
        i686_insl(channels[channel].base + reg - 0x6, buffer, quadCount);
    else if (reg < 0xe)
        i686_insl(channels[channel].ctrl + reg - 0xa, buffer, quadCount);
    else if (reg < 0x16)
        i686_insl(channels[channel].bmide + reg - 0xe, buffer, quadCount);
    if (reg > 0x7 && reg < 0xc)
        IDE_Write(channel, ATA_REG_CONTROL, channels[channel].noInterrupts);
}

uint8_t IDE_Read(uint8_t channel, uint8_t reg) {
    uint8_t result;
    if (reg > 0x7 && reg < 0xc)
        IDE_Write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].noInterrupts);
    if (reg < 0x8) 
        result = i686_inb(channels[channel].base + reg);
    else if (reg < 0xc)
        result = i686_inb(channels[channel].base + reg - 0x6);
    else if (reg < 0xe)
        result = i686_inb(channels[channel].ctrl + reg - 0xa);
    else if (reg < 0x16)
        result = i686_inb(channels[channel].bmide + reg - 0xe);
    if (reg > 0x7 && reg < 0xc)
        IDE_Write(channel, ATA_REG_CONTROL, channels[channel].noInterrupts);

    return result;
}


uint8_t IDE_Polling(uint8_t channel, bool advanced) {
    for (uint8_t i = 0; i < 4; i++) {
        IDE_Read(channel, ATA_REG_ALTSTATUS);
    }

    while (IDE_Read(channel, ATA_REG_STATUS) & ATA_SR_BSY);

    if (advanced) {
        uint8_t status = IDE_Read(channel, ATA_REG_STATUS);

        if (status & ATA_SR_ERR) // Check for errors
            return 1;
        
        if (status & ATA_SR_DF) // Check for device fault
            return 2;

        if ((status & ATA_SR_DRQ) == 0) // Check for DRQ
            return 3;
    }

    return 0;
}

uint8_t identifyBuffer[512];

void sleep(uint8_t c) {
    for (uint8_t i = 0; i < 10; i++) 
        IDE_Read(c, ATA_REG_ALTSTATUS); // TODO: This should be sleep but whatever
}

uint8_t IDE_Access(uint8_t driveNumber, uint8_t direction, uint32_t lba, uint8_t sectorCount, uint8_t* outputBuffer) {
    if (!devices[driveNumber].reserved) return 1;
    
    uint8_t lbaMode;
    uint8_t channel = devices[driveNumber].channel;
    uint8_t drive = devices[driveNumber].drive;
    uint16_t baseIO = channels[channel].base;
    uint16_t sectorSize = 512;
    uint8_t lbaRegs[6];
    uint16_t cylinder;
    uint8_t head, sector;

    IDE_Write(channel, ATA_REG_CONTROL, channels[channel].noInterrupts = 0x02);

    if (lba >= 0x10000000) {
        lbaMode = ATA_LBA48;
        lbaRegs[0] = lba & 0xff;
        lbaRegs[1] = (lba & 0xff00) >> 8;
        lbaRegs[2] = (lba & 0xff0000) >> 16;
        lbaRegs[3] = (lba & 0xff000000) >> 24;
        lbaRegs[4] = 0;
        lbaRegs[5] = 0;
        head = 0;
    } else if (devices[drive].capabilities & 0x200) {
        lbaMode = ATA_LBA28;
        lbaRegs[0] = lba & 0xff;
        lbaRegs[1] = (lba & 0xff00) >> 8;
        lbaRegs[2] = (lba & 0xff0000) >> 16;
        lbaRegs[3] = 0;
        lbaRegs[4] = 0;
        lbaRegs[5] = 0;
        head = (lba & 0xf000000) >> 24;
    } else {
        lbaMode = ATA_CHS;
        sector = (lba % 63) + 1;
        cylinder = (lba + 1 - sector) / (16 * 63);
        lbaRegs[0] = sector;
        lbaRegs[1] = (cylinder >> 0) & 0xff;
        lbaRegs[2] = (cylinder >> 8) & 0xff;
        lbaRegs[3] = 0;
        lbaRegs[4] = 0;
        lbaRegs[5] = 0;
        head = (lba + 1 - sector) % (16 * 63) / 63;
    }

    uint8_t readMode = ATA_PIO;

    IDE_Polling(channel, false);

    if (lbaMode == ATA_CHS)
        IDE_Write(channel, ATA_REG_HDDEVSEL, 0xa0 | (drive << 4) | head);
    else
        IDE_Write(channel, ATA_REG_HDDEVSEL, 0xe0 | (drive << 4) | head);

    if (lbaMode == ATA_LBA48) {
        IDE_Write(channel, ATA_REG_SECCOUNT1, 0);
        IDE_Write(channel, ATA_REG_LBA3, lbaRegs[3]);
        IDE_Write(channel, ATA_REG_LBA4, lbaRegs[4]);
        IDE_Write(channel, ATA_REG_LBA5, lbaRegs[5]);
    }
    IDE_Write(channel, ATA_REG_SECCOUNT0, sectorCount);
    IDE_Write(channel, ATA_REG_LBA0, lbaRegs[0]);
    IDE_Write(channel, ATA_REG_LBA1, lbaRegs[1]);
    IDE_Write(channel, ATA_REG_LBA2, lbaRegs[2]);

    uint8_t command;
    if (lbaMode == ATA_CHS && readMode == ATA_PIO && direction == ATA_READ) command = ATA_CMD_READ_PIO;
    if (lbaMode == ATA_LBA28 && readMode == ATA_PIO && direction == ATA_READ) command = ATA_CMD_READ_PIO;
    if (lbaMode == ATA_LBA48 && readMode == ATA_PIO && direction == ATA_READ) command = ATA_CMD_READ_PIO_EXT;
    if (lbaMode == ATA_CHS && readMode == ATA_DMA && direction == ATA_READ) command = ATA_CMD_READ_DMA;
    if (lbaMode == ATA_LBA28 && readMode == ATA_DMA && direction == ATA_READ) command = ATA_CMD_READ_DMA;
    if (lbaMode == ATA_LBA48 && readMode == ATA_DMA && direction == ATA_READ) command = ATA_CMD_READ_DMA_EXT;
    if (lbaMode == ATA_CHS && readMode == ATA_PIO && direction == ATA_WRITE) command = ATA_CMD_WRITE_PIO;
    if (lbaMode == ATA_LBA28 && readMode == ATA_PIO && direction == ATA_WRITE) command = ATA_CMD_WRITE_PIO;
    if (lbaMode == ATA_LBA48 && readMode == ATA_PIO && direction == ATA_WRITE) command = ATA_CMD_WRITE_PIO_EXT;
    if (lbaMode == ATA_CHS && readMode == ATA_DMA && direction == ATA_WRITE) command = ATA_CMD_WRITE_DMA;
    if (lbaMode == ATA_LBA28 && readMode == ATA_DMA && direction == ATA_WRITE) command = ATA_CMD_WRITE_DMA;
    if (lbaMode == ATA_LBA48 && readMode == ATA_DMA && direction == ATA_WRITE) command = ATA_CMD_WRITE_DMA_EXT;
    IDE_Write(channel, ATA_REG_COMMAND, command);

    uint8_t err;
    if (readMode == ATA_DMA) {
        puts("DMA Not supported\n");
    } else {
        // PIO mode
        if (direction == ATA_READ) {
            // PIO read
            for (uint16_t i = 0; i < sectorCount; i++) {
                if (err = IDE_Polling(channel, true))
                    return err;

                i686_insw(baseIO, (uint16_t*)outputBuffer, sectorSize / 2); // We are reading word by word so we need to divide the sector size
                outputBuffer += sectorSize;
            }
        } else {
            // PIO write
            for (uint16_t i = 0; i < sectorCount; i++) {
                IDE_Polling(channel, false);
                i686_outsw(baseIO, (uint16_t*)outputBuffer, sectorSize / 2); // We are reading word by word so we need to divide the sector size
                outputBuffer += sectorSize;
            }

            if (lbaMode == ATA_LBA48)
                IDE_Write(channel, ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH_EXT);
            else
                IDE_Write(channel, ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);

            IDE_Polling(channel, false);
        }
    }

    return 0;
}

void IDE_IdentifyDrives() {
    uint8_t count = 0;
    for (uint8_t c = 0; c < 2; c++) {
        for (uint8_t d = 0; d < 2; d++) {
            uint8_t status, err = 0;
            devices[count].reserved = false;

            IDE_Write(c, ATA_REG_HDDEVSEL, 0xa0 | d << 4);
            sleep(c);

            IDE_Write(c, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);    

            if (IDE_Read(c, ATA_REG_STATUS) == 0) continue;

            while (1) {
                status = IDE_Read(c, ATA_REG_STATUS);
                if ((status & ATA_SR_ERR)) { err = 1; break; }
                if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break;
            }

            if (err) {
                continue; // TODO: We dont support ATAPI devices
            }

            IDE_ReadBuffer(c, ATA_REG_DATA, (uint32_t*)identifyBuffer, 128);
            devices[count].reserved = true;
            devices[count].type = IDE_ATA;
            devices[count].channel = c;
            devices[count].drive = d;
            devices[count].signature = *((uint16_t*)(identifyBuffer + ATA_IDENT_DEVICETYPE));
            devices[count].capabilities = *((uint16_t*)(identifyBuffer + ATA_IDENT_CAPABILITIES));
            devices[count].commandSets = *((uint32_t*)(identifyBuffer + ATA_IDENT_COMMANDSETS));

            if (devices[count].commandSets & (1 << 26)) // LBA 48
                devices[count].size = *((uint32_t*)(identifyBuffer + ATA_IDENT_MAX_LBA_EXT));
            else // LBA and CHS
                devices[count].size = *((uint32_t*)(identifyBuffer + ATA_IDENT_MAX_LBA));

            for (uint32_t i = 0; i < 40; i += 2) {
                devices[count].model[i] = identifyBuffer[ATA_IDENT_MODEL + i + 1];
                devices[count].model[i + 1] = identifyBuffer[ATA_IDENT_MODEL + i];
            }
            devices[count].model[40] = '\0';
            count++;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (devices[i].reserved) {
            printf("Found drive:\n  ID: %d\n  Model: %s\n  Size: %d\n", i, devices[i].model, devices[i].size);
        }
    }
}

void IDE_IRQ14Handler() {
    IDE_Read(ATA_PRIMARY, ATA_REG_STATUS); // clear
}

void IDE_IRQ15Handler() {
    IDE_Read(ATA_PRIMARY, ATA_REG_STATUS); // clear
}

void IDE_Init() {
    uint8_t bus, device, function;
    PCI_FindDeviceByClass(IDE_PCI_CLASS, IDE_PCI_SUBCLASS, &bus, &device, &function);

    uint32_t bar0 = PCI_ConfigReadLong(bus, device, function, 0x10);
    uint32_t bar1 = PCI_ConfigReadLong(bus, device, function, 0x14);
    uint32_t bar2 = PCI_ConfigReadLong(bus, device, function, 0x18);
    uint32_t bar3 = PCI_ConfigReadLong(bus, device, function, 0x1c);
    uint32_t bar4 = PCI_ConfigReadLong(bus, device, function, 0x20);

    uint32_t progIf = PCI_ConfigReadLong(bus, device, function, 0x8);

    channels[ATA_PRIMARY].base = (bar0 & 0xfffc) + 0x1f0 * (!bar0);
    channels[ATA_PRIMARY].ctrl = (bar1 & 0xfffc) + 0x3f6 * (!bar1);
    channels[ATA_SECONDARY].base = (bar2 & 0xfffc) + 0x170 * (!bar2);
    channels[ATA_SECONDARY].ctrl = (bar3 & 0xfffc) + 0x376 * (!bar3);
    channels[ATA_PRIMARY].bmide = bar4 & 0xfffc;
    channels[ATA_SECONDARY].bmide = (bar4 & 0xfffc) + 8;

    channels[ATA_PRIMARY].noInterrupts = 0x2;
    channels[ATA_SECONDARY].noInterrupts = 0x2;

    IDE_Write(ATA_PRIMARY  , ATA_REG_CONTROL, 2);
    IDE_Write(ATA_SECONDARY, ATA_REG_CONTROL, 2);
    
    i686_IRQ_SetHandler(14, IDE_IRQ14Handler);
    i686_IRQ_SetHandler(15, IDE_IRQ15Handler);

    IDE_IdentifyDrives();
}
ASM=nasm
LD=i686-elf-gcc
CC=i686-elf-gcc
SOURCES_C=$(wildcard *.c)
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/bootloader/stage2/c/%.obj, $(SOURCES_C))
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/bootloader/stage2/asm/%.obj, $(SOURCES_ASM))

.PHONY: all

all: every stage2

stage2: $(BUILD_DIR)/bootloader/stage2/stage2.bin 

$(BUILD_DIR)/bootloader/stage2/stage2.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	$(LD) -T linker.ld -nostdlib -Wl,-Map=$(BUILD_DIR)/stage2.map -o $@ $^ -lgcc

$(BUILD_DIR)/bootloader/stage2/asm/%.obj : %.asm
	$(ASM) -f elf -o $@ $<

$(BUILD_DIR)/bootloader/stage2/c/%.obj: %.c
	mkdir -p $(@D)
	$(CC) -ffreestanding -nostdlib -std=c99 -g -c -o $@ $<
	echo "--> Compiled: " $<

every:
	mkdir -p $(BUILD_DIR)/bootloader/stage2/asm/
	mkdir -p $(BUILD_DIR)/bootloader/stage2/c/

clean:
	rm -f $(BUILD_DIR)/bootloader/stage2/stage2.bin
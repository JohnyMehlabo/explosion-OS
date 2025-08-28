ASM=nasm
LD=i686-elf-gcc
CC=i686-elf-gcc
SOURCES_C=$(shell find . -name '*.c')
SOURCES_ASM=$(shell find . -name '*.asm')
OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/kernel/c/%.obj, $(SOURCES_C))
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.obj, $(SOURCES_ASM))

.PHONY: all

all: every kernel

kernel: $(BUILD_DIR)/kernel/kernel.bin 

$(BUILD_DIR)/kernel/kernel.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	$(LD) -T linker.ld -nostdlib -Wl,-Map=$(BUILD_DIR)/kernel.map -o $@ $^ -lgcc

$(BUILD_DIR)/kernel/asm/%.obj : %.asm
	mkdir -p $(@D)
	$(ASM) -f elf -o $@ $<

$(BUILD_DIR)/kernel/c/%.obj: %.c
	mkdir -p $(@D)
	$(CC) -ffreestanding -nostdlib -std=c99 -g -c -o $@ $< -I .
	echo "--> Compiled: " $<

every:
	mkdir -p $(BUILD_DIR)/kernel/asm/
	mkdir -p $(BUILD_DIR)/kernel/c/

clean:
	rm -f $(BUILD_DIR)/kernel/kernel.bin
ASM = nasm
MAKE = make
SRC = src
BUILD_DIR = build

.PHONY: clean every run

floppy_disk_image.img: bootloader kernel every hdd.img
	dd if=/dev/zero of=floppy_disk_image.img conv=notrunc bs=512 count=2880
	mkfs.vfat -F12 floppy_disk_image.img -n "EXPLOSIONOS"
	dd if=$(BUILD_DIR)/bootloader/stage1/stage1.bin of=floppy_disk_image.img conv=notrunc
	mcopy $(BUILD_DIR)/bootloader/stage2/stage2.bin -i floppy_disk_image.img "::stage2.bin"
	mcopy $(BUILD_DIR)/kernel/kernel.bin -i floppy_disk_image.img "::kernel.bin"
	mcopy os_information.txt -i floppy_disk_image.img "::intro.txt"
	truncate -s 1440k floppy_disk_image.img

	powershell.exe .\\run.bat

hdd.img:
	dd if=/dev/zero of=hdd.img bs=1M count=64
	mkfs.vfat -F 16 hdd.img -n "STORAGE"
	mcopy -i hdd.img os_information.txt ::intro.txt

bootloader: stage1 stage2

stage1: $(BUILD_DIR)/bootloader/stage1/stage1.bin
stage2: $(BUILD_DIR)/bootloader/stage2/stage2.bin

kernel: $(BUILD_DIR)/kernel/kernel.bin

$(BUILD_DIR)/bootloader/stage1/stage1.bin : $(SRC)/bootloader/stage1/main.asm every
	$(ASM) $(SRC)/bootloader/stage1/main.asm -o $(BUILD_DIR)/bootloader/stage1/stage1.bin -fbin

$(BUILD_DIR)/bootloader/stage2/stage2.bin : every
	$(MAKE) -C src/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))

$(BUILD_DIR)/kernel/kernel.bin : every
	$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR))

clean: 
	rm -rf build/**/*

every:
	mkdir -p build/kernel build/bootloader/stage1 build/bootloader/stage2
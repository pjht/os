export PLAT=i386
export CC = $(shell cat psinfo/$(PLAT)/cc.txt)
export AS = $(shell cat psinfo/$(PLAT)/as.txt)
export AR = $(shell cat psinfo/$(PLAT)/ar.txt)
export NASM = $(shell cat psinfo/$(PLAT)/nasm.txt)
EMU = $(shell cat psinfo/$(PLAT)/emu.txt)
GDB = $(shell cat psinfo/$(PLAT)/gdb.txt)
LINK_OBJ = $(wildcard kernel/kernel.a libc/libc.a cpu/$(PLAT)/boot.o)
export CFLAGS = -Wextra -Wall -Wno-unused-parameter -g -ffreestanding
QFLAGS =  -hda ext2.img -m 2G -boot d -cdrom os.iso -serial vc #-chardev socket,id=s1,port=3000,host=localhost -serial chardev:s1

.PHONY: sysroot

all: os.iso

run: os.iso
	@$(EMU) $(QFLAGS) -monitor stdio

debug: os.iso kernel.elf
	@$(EMU) -s $(QFLAGS) &
	@$(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel/kernel.elf"

os.iso: kernel.elf initrd/* initrd/init
	@cp kernel/kernel.elf iso/boot
	@cd initrd; tar -f ../iso/boot/initrd.tar -c *
	@grub-mkrescue -o $@ iso > /dev/null 2>/dev/null

initrd/init: kernel/start.o
	@$(MAKE) -C init
	@cp init/init initrd/init

.PHONY: kernel.elf

kernel.elf:
	@$(MAKE) -C kernel
	@$(MAKE) -C libc
	@$(CC) -z max-page-size=4096 -Xlinker -n -T kernel/cpu/$(PLAT)/linker.ld -o $@ $(CFLAGS) -nostdlib $(LINK_OBJ) -lgcc

clean:
	@$(MAKE) clean -C kernel
	@$(MAKE) clean -C libc
	@$(MAKE) clean -C kernel/cpu

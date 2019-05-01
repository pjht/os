PLAT=i386
C_SOURCES = $(wildcard kernel/*.c drivers/$(PLAT)/*.c drivers/$(PLAT)/*/*.c cpu/$(PLAT)/*.c fs/*.c)
ASM = $(wildcard cpu/$(PLAT)/*.asm)
S_ASM = $(wildcard cpu/$(PLAT)/*.s)
LIBC_SOURCES = $(wildcard libc/*.c libc/*/*.c)
LIBC_HEADERS = $(wildcard libc/*.h libc/*/*.h)
OBJ = $(C_SOURCES:.c=.o cpu/$(PLAT)/boot.o)
ASM_OBJ = $(S_ASM:.s=.o)
S_ASM_OBJ = $(ASM:.asm=.o)
LIBC_OBJ = $(LIBC_SOURCES:.c=.o)
CC = $(shell cat psinfo/$(PLAT)/cc.txt)
AS = $(shell cat psinfo/$(PLAT)/as.txt)
AR = $(shell cat psinfo/$(PLAT)/ar.txt)
NASM = $(shell cat psinfo/$(PLAT)/nasm.txt)
EMU = $(shell cat psinfo/$(PLAT)/emu.txt)
GDB = $(shell cat psinfo/$(PLAT)/gdb.txt)
CFLAGS =  -Isysroot/usr/include -Wextra -Wall -Wno-unused-parameter -g -ffreestanding
QFLAGS =  -hda ext2.img -m 2G -boot d -cdrom os.iso -serial vc #-chardev socket,id=s1,port=3000,host=localhost -serial chardev:s1

.PHONY: sysroot

all: os.iso

run: os.iso
	$(EMU) $(QFLAGS) -monitor stdio

debug: os.iso kernel/kernel.elf
	$(EMU) -s $(QFLAGS) &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel/kernel.elf"

os.iso: kernel/kernel.elf initrd/* initrd/prog.elf
	cp kernel/kernel.elf iso/boot
	ruby makeinitrd.rb initrd iso/boot/initrd
	grub-mkrescue -o $@ iso

initrd/prog.elf: prog/*
	cd prog && make
	cp prog/prog.elf initrd

kernel/kernel.elf: $(OBJ) $(ASM_OBJ) $(S_ASM_OBJ) libc/libc.a
	$(CC) -z max-page-size=4096 -Xlinker -n -T cpu/$(PLAT)/linker.ld -o $@ $(CFLAGS) -nostdlib $^ -lgcc

sysroot: $(LIBC_HEADERS)
	mkdir -p sysroot/usr/include
	cp -r libc/* sysroot/usr/include
	rm -f sysroot/usr/include/libc.a sysroot/usr/include/*.o sysroot/usr/include/*/*.o sysroot/usr/include/*.c sysroot/usr/include/*/*.c


libc/libc.a: $(LIBC_OBJ)
	$(AR) rcs $@ $^

%.o: %.c sysroot
	$(CC) $(CFLAGS)  -c $< -o $@

%.o: %.asm
	$(NASM) $< -o $@

%.o: %.s
	$(AS) $< -o $@

clean:
	rm -rf $(OBJ) $(OBJ) $(ASM_OBJ) $(S_ASM_OBJ) libc/libc.a kernel/cstart.o cpu/memory.h os.iso */*.elf iso/boot/initrd.tar

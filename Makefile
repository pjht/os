PLAT=i386
C_SOURCES = $(wildcard kernel/*.c drivers/$(PLAT)/*.c drivers/$(PLAT)/*/*.c cpu/$(PLAT)/*.c fs/*.c)
LIBC_SOURCES = $(wildcard libc/*.c libc/*/*.c)
OBJ = $(C_SOURCES:.c=.o $(shell cat psinfo/$(PLAT)/o.txt))
LIBC_OBJ = $(LIBC_SOURCES:.c=.o)
CC = $(shell cat psinfo/$(PLAT)/cc.txt)
GDB = $(shell cat psinfo/$(PLAT)/gdb.txt)
CFLAGS = -Ilibc -Wextra -Wall -Wno-unused-parameter -g -ffreestanding
QFLAGS =  -m 2G -boot d -cdrom os.iso -serial vc #-chardev socket,id=s1,port=3000,host=localhost -serial chardev:s1

all: os.iso

run: os.iso
	qemu-system-i386 $(QFLAGS) -monitor stdio

debug: os.iso kernel/kernel.elf
	qemu-system-i386 -s $(QFLAGS) &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel/kernel.elf"

os.iso: kernel/kernel.elf initrd/* initrd/prog.elf
	cp kernel/kernel.elf iso/boot
	ruby makeinitrd.rb initrd iso/boot/initrd
	grub-mkrescue -o $@ iso

initrd/prog.elf: prog/*
	cd prog && make
	cp prog/prog.elf initrd

kernel/kernel.elf: $(OBJ) libc/libc.a
	i386-elf-ld -T linker.ld -o $@ $^

libc/libc.a: $(LIBC_OBJ)
	i386-elf-ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS)  -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.o: %.s
	i386-elf-as $< -o $@

clean:
	rm -rf $(OBJ) libc/libc.a kernel/cstart.o cpu/memory.h os.iso */*.elf iso/boot/initrd.tar

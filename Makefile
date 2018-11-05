C_SOURCES = $(wildcard kernel/*.c drivers/*.c libc/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h libc/*.h)
OBJ = ${C_SOURCES:.c=.o kernel/boot.o drivers/interrupt.o drivers/paging_helpers.o}

CC = /usr/local/bin/i386-elf-gcc
GDB = /usr/local/bin/i386-elf-gdb
CFLAGS = -g

os.iso: kernel/kernel.elf initrd/*
	cp kernel/kernel.elf iso/boot
	cd initrd; \
		tar -c -f "../iso/boot/initrd.tar" *
	grub-mkrescue -o $@ iso

# Used for debugging purposes
kernel/kernel.elf: ${OBJ}
	i386-elf-ld -T linker.ld -o $@ $^

run: os.iso
	qemu-system-i386 -m 2G -d int -d cpu_reset -D qemu.log -boot d -cdrom os.iso

# Open the connection to qemu and load our kernel-object file with symbols
debug: os.iso kernel/kernel.elf
	qemu-system-i386  -s -no-reboot -no-shutdown -boot d -cdrom os.iso &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file kernel/kernel.elf"

# Generic rules for wildcards
# To make an object, always compile from its .c
%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -ffreestanding -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.o: %.s
	i386-elf-as $< -o $@

%.bin: %.asm
	nasm $< -f bin -o $@

clean:
	rm -rf */*.bin */*.o os.iso */*.elf iso/boot/initrd.tar
	rm -rf */*.o

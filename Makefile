PLAT=i386
C_SOURCES = $(wildcard kernel/*.c drivers/$(PLAT)/*.c libc/*.c cpu/$(PLAT)/*.c)
OBJ = $(C_SOURCES:.c=.o kernel/boot.o $(shell cat psinfo/$(PLAT)/o.txt))
CC = $(shell cat psinfo/$(PLAT)/cc.txt)
GDB = $(shell cat psinfo/$(PLAT)/gdb.txt)
CFLAGS = -g

run: os.iso
	qemu-system-i386 -m 2G -d int -d cpu_reset -D qemu.log -boot d -cdrom os.iso

debug: os.iso kernel/kernel.elf
	qemu-system-i386 -s -boot d -cdrom os.iso &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel/kernel.elf"

os.iso: kernel/kernel.elf initrd/*
	cp kernel/kernel.elf iso/boot
	cd initrd; \
		tar -c -f "../iso/boot/initrd.tar" *
	grub-mkrescue -o $@ iso

kernel/kernel.elf: $(OBJ)
	i386-elf-ld -T linker.ld -o $@ $^

%.o: %.c h_files
	$(CC) $(CFLAGS) -ffreestanding -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.o: %.s
	i386-elf-as $< -o $@

h_files: cpu/$(PLAT)/memory.h
	rm -f cpu/memory.h
	cp cpu/$(PLAT)/memory.h cpu/memory.h

clean:
	rm -rf */*/*.o */*.o cpu/memory.h os.iso */*.elf iso/boot/initrd.tar

PLAT=i386
C_SOURCES = $(wildcard kernel/*.c drivers/$(PLAT)/*.c drivers/$(PLAT)/*/*.c libc/*.c cpu/$(PLAT)/*.c fs/*.c)
OBJ = $(C_SOURCES:.c=.o $(shell cat psinfo/$(PLAT)/o.txt))
CC = $(shell cat psinfo/$(PLAT)/cc.txt)
GDB = $(shell cat psinfo/$(PLAT)/gdb.txt)
CFLAGS = -Wall -g -ffreestanding
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

kernel/kernel.elf: $(OBJ)
	i386-elf-ld -T linker.ld -o $@ $^

initrd/prog.elf: kernel/cstart.o
	cd prog;make
	cp prog/prog.elf initrd

%.o: %.c h_files
	$(CC) $(CFLAGS)  -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.o: %.s
	i386-elf-as $< -o $@

h_files: cpu/$(PLAT)/memory.h
	rm -f cpu/memory.h
	cp cpu/$(PLAT)/memory.h cpu/memory.h

pipe:
	rm -f pipe.in pipe.out
	mkfifo pipe.in
	ln pipe.in pipe.out

clean:
	rm -rf $(OBJ) kernel/cstart.o cpu/memory.h os.iso */*.elf iso/boot/initrd.tar

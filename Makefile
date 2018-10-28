C_SOURCES = $(wildcard kernel/*.c drivers/*.c libc/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h libc/*.h)
OBJ = ${C_SOURCES:.c=.o drivers/interrupt.o drivers/paging_helpers.o kernel/loader.o}

CC = /usr/local/bin/i386-elf-gcc
GDB = /usr/local/bin/i386-elf-gdb
CFLAGS = -g

# First rule is run by default
os-image.bin: kernel/kernel.bin
	cat $^ > $@
# '--oformat binary' deletes all symbols as a collateral, so we don't need
# to 'strip' them manually on this case

kernel/kernel.bin: ${OBJ} link.ld
	i386-elf-ld -T link.ld -o $@ $^

# Used for debugging purposes
kernel.elf: ${OBJ}
	i386-elf-ld -T link.ld -o $@ $^

run: os-image.bin
	qemu-system-i386 -kernel os-image.bin

# Open the connection to qemu and load our kernel-object file with symbols
debug: os-image.bin kernel.elf
	qemu-system-i386 -s -kernel os-image.bin &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

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
	rm -rf *.bin *.dis *.o os-image.bin *.elf boot/boot.bin
	rm -rf */*.o

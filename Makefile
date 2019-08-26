PLAT=i386
C_SOURCES = $(wildcard kernel/*.c drivers/$(PLAT)/*.c drivers/$(PLAT)/*/*.c kernel/cpu/$(PLAT)/*.c fs/*.c)
ASM = $(wildcard kernel/cpu/$(PLAT)/*.asm)
S_ASM = $(wildcard kernel/cpu/$(PLAT)/*.s)
LIBC_SOURCES = $(wildcard libc/*.c libc/*/*.c)
LIBC_HEADERS = $(wildcard libc/*.h libc/*/*.h)
OBJ = $(C_SOURCES:.c=.o kernel/cpu/$(PLAT)/boot.o)
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
QFLAGS =  -hda ext2.img -m 2G -boot d -cdrom os.iso -serial file:serout #-chardev socket,id=s1,port=3000,host=localhost -serial chardev:s1
CWD = $(shell pwd)

.PHONY: sysroot

all: os.iso

run: os.iso
	@$(EMU) $(QFLAGS) -monitor stdio

debug: os.iso kernel/kernel.elf
	@$(EMU) -s $(QFLAGS) &
	@$(GDB)
	#gdbgui -g i386-elf-gdb --project $(CWD)

os.iso: kernel/kernel.elf init vfs fsdrv initrd
	@cp kernel/kernel.elf sysroot/boot
	@cd initrd; tar -f ../sysroot/boot/initrd.tar -c *
	@grub-mkrescue -o $@ sysroot >/dev/null 2>/dev/null

init: init/* kernel/start.o
	@cd $@ && make
	@cp $@/$@ initrd/$@

vfs: vfs/* kernel/start.o
	@cd $@ && make
	@cp $@/$@ initrd/$@

fsdrv: fsdrv/* kernel/start.o
	@cd $@ && make
	@cp $@/$@ initrd/$@


kernel/kernel.elf: $(OBJ) $(ASM_OBJ) $(S_ASM_OBJ) sysroot/usr/lib/libc.a
	@$(CC) -z max-page-size=4096 -Xlinker -n -T kernel/cpu/$(PLAT)/linker.ld -o $@ $(CFLAGS) -nostdlib $^ -lgcc

sysroot/usr/lib/libc.a: $(LIBC_OBJ)
	@$(AR) rcs $@ $^

%.o: %.c
	@$(CC) $(CFLAGS)  -c $< -o $@

%.o: %.asm
	@$(NASM) $< -o $@

%.o: %.s
	@$(AS) $< -o $@

clean:
	@rm -rf initrd/* kernel/*.o drivers/*/*.o drivers/*/*/*.o cpu/*/*.o fs/*.o libc/libc.a kernel/cstart.o cpu/memory.h os.iso */*.elf sysroot/boot/initrd.tar

PLAT=i386
TIMER_TYPE=$(shell cat psinfo/$(PLAT)/timer_type.txt)
C_SOURCES = $(wildcard kernel/*.c kernel/cpu/$(PLAT)/*.c kernel/cpu/*.c kernel/timer/$(TIMER_TYPE)/*.c) 
C_HEADERS = $(wildcard kernel/*.h kernel/cpu/$(PLAT)/*.h kernel/cpu/*.h kernel/timer/$(TIMER_TYPE)/*.h) 
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
CFLAGS =  -Isysroot/usr/include -Wextra -Wall -Wno-unused-parameter -g -ffreestanding
QFLAGS =  -hda ext2.img -m 2G -boot d -cdrom os.iso -serial file:serout #-chardev socket,id=s1,port=3000,host=localhost -serial chardev:s1
CWD = $(shell pwd)

.PHONY: sysroot

all: os.iso

run: os.iso
	rm -f serout
	touch serout
	@$(EMU) $(QFLAGS) &
	tail -f serout

debug: os.iso kernel/kernel.elf
	@$(EMU) -s $(QFLAGS) &
	gdb
	#gdbgui -g i386-elf-gdb --project $(CWD)

os.iso: kernel/kernel.elf init vfs devfs vga_drv initrd_drv sysroot/usr/share/man # vfs devfs initrd vga_drv initrd_drv pci
	@cp kernel/kernel.elf sysroot/boot
	@cd initrd; tar -f ../sysroot/boot/initrd.tar -c *
	@grub-mkrescue -o $@ sysroot >/dev/null 2>/dev/null

crts: kernel/crt0.o
	@cp $^ sysroot/usr/lib

init: crts libc
	@cd $@ && make
	@cp $@/$@ initrd/$@

vfs: crts libc
	@cd $@ && make
	@cp $@/$@ initrd/$@

devfs: crts libc
	@cd $@ && make
	@cp $@/$@ initrd/$@

pci: crts libc
	@cd $@ && make
	@cp $@/$@ initrd/$@

vga_drv: crts libc
	@cd $@ && make
	@cp $@/$@ initrd/$@

initrd_drv: crts libc
	@cd $@ && make
	@cp $@/$@ initrd/$@

kernel/kernel.elf: $(OBJ) $(ASM_OBJ) $(S_ASM_OBJ) sysroot/usr/lib/libc.a
	@$(CC) -z max-page-size=4096 -Xlinker -n -T kernel/cpu/$(PLAT)/linker.ld -o $@ $(CFLAGS) -nostdlib $^ -lgcc

libc: sysroot/usr/lib/libc.a

sysroot/usr/lib/libc.a: $(LIBC_OBJ)
	@$(AR) rcs $@ $^

sysroot/usr/share/man: doc
	@ cp -r kernel/docs/man/man9 sysroot/usr/share/man
	@ cp -r libc/docs/man/man3 sysroot/usr/share/man

sysroot/usr/include: $(LIBC_SOURCES) $(LIBC_HEADERS)
	@ cd libc;rsync -R *.h */*.h ../sysroot/usr/include/

kernel/cpu/arch_consts.h: kernel/cpu/$(PLAT)/arch_consts.h
	@cp kernel/cpu/$(PLAT)/arch_consts.h kernel/cpu/arch_consts.h
kernel/cpu/isr.h: kernel/cpu/$(PLAT)/isr.h
	@cp kernel/cpu/$(PLAT)/isr.h kernel/cpu/isr.h

%.o: %.c kernel/cpu/arch_consts.h kernel/cpu/isr.h sysroot/usr/include
	@$(CC) $(CFLAGS)  -c $< -o $@

%.o: %.asm
	@$(NASM) $< -o $@

%.o: %.s
	@$(AS) $< -o $@

clean:
	@rm -rf initrd/* kernel/*.o drivers/*/*.o drivers/*/*/*.o cpu/*/*.o fs/*.o libc/libc.a kernel/cstart.o cpu/memory.h os.iso */*.elf sysroot/boot/initrd.tar

doc: $(C_SOURCES) $(C_HEADERS)
	@doxygen kernel/Doxyfile > /dev/null
	@doxygen libc/Doxyfile > /dev/null

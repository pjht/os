PLAT=i386
C_SOURCES = $(wildcard $(PLAT)/*.c)
ASM = $(wildcard $(PLAT)/*.asm)
S_ASM = $(wildcard $(PLAT)/*.s)
OBJ = $(C_SOURCES:.c=.o)
ASM_OBJ = $(S_ASM:.s=.o)
S_ASM_OBJ = $(ASM:.asm=.o)

cpu.a: $(OBJ) $(ASM_OBJ) $(S_ASM_OBJ)
	@$(AR) rcs $@ $^

%.o: %.c
	@$(CC) -I../../sysroot/usr/include $(CFLAGS)  -c $< -o $@

%.o: %.asm
	@$(NASM) $< -o $@

%.o: %.s
	@$(AS) $< -o $@

clean:
	@rm -rf */*.o

C_SOURCES = $(wildcard *.c cpu/$(PLAT)/*.c)
ASM = $(wildcard cpu/$(PLAT)/*.asm)
S_ASM = $(wildcard cpu/$(PLAT)/*.s)
OBJ = $(C_SOURCES:.c=.o)
ASM_OBJ = $(S_ASM:.s=.o)
S_ASM_OBJ = $(ASM:.asm=.o)

kernel.a: $(OBJ) $(ASM_OBJ) $(S_ASM_OBJ)
	@$(AR) rcs $@ $^

%.o: %.c
	@$(CC)  -I../sysroot/usr/include $(CFLAGS)  -c $< -o $@

%.o: %.asm
	@$(NASM) $< -o $@

%.o: %.s
	@$(AS) $< -o $@


clean:
	@rm -rf *.o cpu/*/*.o

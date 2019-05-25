C_SOURCES = $(wildcard *.c)
OBJ = $(C_SOURCES:.c=.o)
CFLAGS = -I../sysroot/usr/include -Wall -g -ffreestanding
CC = i386-elf-gcc

vfs: $(OBJ)
	@i386-elf-ld -o $@ $^ ../kernel/start.o ../libc/libc.a

%.o: %.c
	@$(CC) $(CFLAGS)  -c $< -o $@

clean:
	@rm -rf *.o prog.elf

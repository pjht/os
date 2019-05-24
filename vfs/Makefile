C_SOURCES = $(wildcard *.c)
OBJ = $(C_SOURCES:.c=.o ../kernel/start.o)
CFLAGS = -I../sysroot/usr/include -Wall -g -ffreestanding
CC = i386-elf-gcc

vfs: $(OBJ)
	@i386-elf-ld -o $@ $^ ../libc/libc.a
	@rm -rf *.o

%.o: %.c
	@$(CC) $(CFLAGS)  -c $< -o $@

clean:
	@rm -rf *.o prog.elf

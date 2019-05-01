C_SOURCES = $(wildcard *.c)
OBJ = $(C_SOURCES:.c=.o ../libc/libc.a ../kernel/start.o)
CFLAGS = -Wall -g -ffreestanding
CC = i386-elf-gcc

prog.elf: $(OBJ)
	i386-elf-ld -o $@ $^
	rm -rf *.o

%.o: %.c
	$(CC) $(CFLAGS)  -c $< -o $@

clean:
	rm -rf *.o prog.elf

C_SOURCES = $(wildcard *.c)
OBJ = $(C_SOURCES:.c=.o)
CFLAGS = -I../sysroot/usr/include -Wall -g
CC = i386-elf-gcc

vfs: $(OBJ) ../libc/*
	@i386-elf-ld -o $@ $(OBJ) ../kernel/start.o ../libc/libc.a

%.o: %.c
	@$(CC) $(CFLAGS)  -c $< -o $@

clean:
	@rm -rf *.o vfs

C_SOURCES = $(wildcard *.c)
OBJ = $(C_SOURCES:.c=.o )
CFLAGS = -Wall -g --sysroot=../sysroot -isystem=/usr/include -nostdlib
CC = i386-elf-gcc

fsdrv: $(OBJ) ../libc/*
		@$(CC)  -z max-page-size=4096 -Xlinker -n -o $@ $(CFLAGS) $(OBJ) ../kernel/start.o ../sysroot/usr/lib/libc.a -lgcc

%.o: %.c
	@$(CC) $(CFLAGS)  -c $< -o $@

clean:
	@rm -rf *.o init

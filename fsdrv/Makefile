C_SOURCES = $(wildcard *.c)
OBJ = $(C_SOURCES:.c=.o )
CFLAGS = -Wall -g
CC = i386-myos-gcc

fsdrv: $(OBJ) ../libc/*
	@$(CC)  -o $@ $(CFLAGS) $(OBJ)

%.o: %.c
	@$(CC) $(CFLAGS)  -c $< -o $@

clean:
	@rm -rf *.o init

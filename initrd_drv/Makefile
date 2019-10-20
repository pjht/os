C_SOURCES = $(wildcard *.c)
OBJ = $(C_SOURCES:.c=.o )
CFLAGS =  -Wall -g
CC = i386-myos-gcc

initrd_drv: $(OBJ) ../libc/*
	@$(CC)  -o $@ $(CFLAGS) $(OBJ)

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf *.o initrd_drv

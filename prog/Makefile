PLAT=i386
C_SOURCES = $(wildcard *.c)
OBJ = $(C_SOURCES:.c=.o ../kernel/cstart.o)
CC = $(shell cat ../psinfo/$(PLAT)/cc.txt)
GDB = $(shell cat ../psinfo/$(PLAT)/gdb.txt)
CFLAGS = -Wall -g -ffreestanding

prog.elf: $(OBJ)
	i386-elf-ld -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS)  -c $< -o $@

clean:
	rm -rf *.o prog.elf

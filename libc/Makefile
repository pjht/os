SOURCES = $(wildcard libc/*.c libc/*/*.c)
HEADERS = $(wildcard libc/*.h libc/*/*.h)
.PHONY: sysroot

libc.a: $(LIBC_OBJ)
	@$(AR) rcs $@ $^

%.o: %.c sysroot
	@$(CC) -I../sysroot/usr/include $(CFLAGS)  -c $< -o $@

clean:
	@rm -rf *.o libc/libc.a

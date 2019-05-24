ENTRY(start)


SECTIONS {
  . = 1M;

  .multiboot :
  {
    *(.multiboot)
  }

  .boottext ALIGN (4K) :
  {
    *(.boot.text)
  }

  .bootrodata ALIGN (4K) :
  {
    *(.boot.rodata)
  }
  .bootdata ALIGN (4K) :
  {
    *(.boot.data)
  }
  .bootbss ALIGN (4K) :
  {
    *(.boot.bss)
  }

  . += 0xffff800000000000;

  .text ALIGN (4K) : AT (ADDR (.text) - 0xffff800000000000)
  {
    *(.text)
  }
  .rodata ALIGN (4K) : AT (ADDR (.rodata) - 0xffff800000000000)
  {
    *(.rodata)
  }
  .data ALIGN (4K) : AT (ADDR (.data) - 0xffff800000000000)
  {
    *(.data)
  }
  .bss ALIGN (4K) : AT (ADDR (.bss) - 0xffff800000000000)
  {
    *(.bss)
  }
}

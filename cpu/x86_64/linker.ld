ENTRY(start)

PHDRS {
  boot PT_LOAD FILEHDR PHDRS;
  kernel PT_NULL;
}

SECTIONS {
  . = 1M;

  .multiboot :
  {
    *(.multiboot)
  } :boot

  .boot.text ALIGN (4K) :
  {
    *(.boot.text)
  } :boot

  .boot.rodata ALIGN (4K) :
  {
    *(.boot.rodata)
  } :boot
  .boot.data ALIGN (4K) :
  {
    *(.boot.data)
  } :boot
  .boot.bss ALIGN (4K) :
  {
    *(.boot.bss)
  } :boot

  . = 0xffff800000100000;

  .text ALIGN (4K) : AT (ADDR (.text) - 0xfff800000000000)
  {
    *(.text)
  } :kernel
  .rodata ALIGN (4K) : AT (ADDR (.rodata) - 0xfff800000000000)
  {
    *(.rodata)
  } :kernel
  .data ALIGN (4K) : AT (ADDR (.data) - 0xfff800000000000)
  {
    *(.data)
  } :kernel
  .bss ALIGN (4K) : AT (ADDR (.bss) - 0xfff800000000000)
  {
    *(.bss)
  } :kernel
}

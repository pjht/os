ENTRY(start)

SECTIONS {

  . = 1M;

  .boot ALIGN (4K) :
  {
      /* ensure that the multiboot header is at the beginning */
      *(.multiboot_header)
  }

  .text ALIGN (4K) :
  {
      *(.boot.text)
  }

  .bss ALIGN (4K) :
  {
      *(.boot.bss)
  }

  .rodata ALIGN (4K) :
  {
      *(.boot.rodata)
  }

  . = 0x800000100000;

  .text 0x800000100000 ALIGN (4K) : AT (ADDR (.text) - 0x800000000000)
  {
    *(.text)
  }
  .rodata 0x800000100000 + SIZEOF (.text) ALIGN (4K) : AT (ADDR (.rodata) - 0x800000000000)
  {
    *(.rodata)
  }
  .data  0x800000100000 + SIZEOF (.text) + SIZEOF (.rodata) ALIGN (4K) : AT (ADDR (.data) - 0x800000000000)
  {
    *(.data)
  }
  .bss  0x800000100000 + SIZEOF (.text) + SIZEOF (.rodata) + SIZEOF (.data) ALIGN (4K) : AT (ADDR (.bss) - 0x800000000000)
  {
    *(.bss)
  }
}

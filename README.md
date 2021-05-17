# os

This is a hobby OS based on a microkernel.
It currently supports:

1. Basic preemptive multitasking with task states & exiting.
2. Physical & virtual memory management
3. Interrupts.
4. The PIT, used to do preemption.
5. The serial port, used as an output for logs. (driver in kernel, will eventually be moved out)
6. RPC as the IPC mechanism
7. A working VFS and devfs
8. VGA text mode

## Building

To build the OS, you will need custom patched versions of GCC and Binutils targeted for my OS.

### Building the cross-compiler

The instructions for building a cross compiler are [here](https://wiki.osdev.org/GCC_Cross-Compiler), though a few changes need to be made.
First, you **must** download GCC 9.2.0 and Binutils 2.32, or the patches may not apply correctly.
Next, you need to apply the patches located in the patches/ directory.
Set $TARGET to i386-myos, not i686-elf.
When running configure for both Binutils and GCC, you must pass --sysroot=\<path to cloned github repo\>/sysroot as an extra argument,
otherwse the compiler will not be able to find the libc headers.
Otherwise, the build is as normal.

Once the cross-compiler is built, the OS can be built by simply typing `make`.

## Additional setup (on Linux or Mac (Unsupported on Mac) )

To run & debug the OS, you willl need to install QEMU, GRUB, and GDB.

Installing GRUB on:

1. Ubuntu. GRUB is already installed.
2. Arch. If you have chosen GRUB as your bootloader, run `sudo pacman -S dosfstools libisoburn`, as arch by default does not have the necessary tools for grub-mkrescue to work, otherwise run `sudo pacman -S grub dosfstools libisoburn`
3. Another linux distro. GRUB is likely already installed, but if it is not, you will need to use your distribution's package manager to install GRUB.
4. Mac. You will need to install Homebrew, then run `brew install i386-elf-grub`

Installing QEMU on

1. Ubuntu. Run `sudo apt get install qemu`
2. Arch. Run `sudo pacman -S qemu qemu-arch-extra`
3. Another linux distro. Use your distribution's package manager to install QEMU.
4. Mac. Run `brew install qemu`

Installing GDB on

1. Ubuntu. Run `sudo apt get install gdb`
2. Arch. Run `sudo pacman -S gdb`
3. Another linux distro. Use your distribution's package manager to install GDB.
4. Mac. Run `brew install gdb`

After this, you can run the OS in QEMU by typing `make run`, or run the OS and start the debugger by typing `make debug`.
The serial port output of the OS is redirected to a file callled serout in the main directory.

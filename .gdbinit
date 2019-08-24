set pagination off
target remote localhost:1234
symbol-file kernel/kernel.elf
add-symbol-file vfs/vfs
b kernel/cpu/i386/tasking.c:129
b kernel/cpu/i386/tasking.c:140
commands 1 2
silent
  disable breakpoints
  symbol-file kernel/kernel.elf
  p task->pid
  if task->pid==2
    add-symbol-file vfs/vfs
    enable breakpoints
  else
    enable breakpoints 1
    c
  end
  c
end

b main
commands
  disable breakpoints 1
end

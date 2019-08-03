set pagination off
target remote localhost:1234
symbol-file kernel/kernel.elf
add-symbol-file vfs/vfs
b tasking.c:123
commands
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

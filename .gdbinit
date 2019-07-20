set pagination off
target remote localhost:1234
symbol-file kernel/kernel.elf
add-symbol-file init/init
b tasking.c:123
commands
silent
  disable breakpoints
  symbol-file kernel/kernel.elf
  p task->pid
  if task->pid==1
    add-symbol-file init/init
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

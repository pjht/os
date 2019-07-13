target remote localhost:1234
symbol-file kernel/kernel.elf
add-symbol-file fsdrv/fsdrv
b tasking.c:120
commands
disable breakpoints
symbol-file kernel/kernel.elf
if task->pid==2
  add-symbol-file fsdrv/fsdrv
  enable breakpoints
else
  enable breakpoints 1
end
c
end

b main
commands
  disable breakpoints 1
end

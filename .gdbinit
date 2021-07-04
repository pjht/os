set pagination off
target remote localhost:1234
symbol-file kernel/kernel.elf
add-symbol-file vfs/vfs
b kernel/tasking.c:226
commands 1
silent
  disable breakpoints
  symbol-file kernel/kernel.elf
  p current_thread->process->pid
  if current_thread->process->pid==2
    add-symbol-file vfs/vfs
    b main
    commands
      p "HIT"
      disable breakpoints 1
    end
    enable breakpoints
  else
    enable breakpoints 1
    c
  end
  c
end

# b main
# commands
#   disable breakpoints 1
# end

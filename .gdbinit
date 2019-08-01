set pagination off
target remote localhost:1234
symbol-file kernel/kernel.elf
printf "Start at kernel\n"
b tasking.c:123
commands
silent
  if task->pid==0
    shell date
    printf "Yield to the kernel\n"
  else
    if task->pid==1
      shell date
      printf "Yield to init\n"
    else
      if task->pid==2
        shell date
        printf "Yield to the VFS\n"
      else
        if task->pid==3
          shell date
          printf "Yield to fsdrv\n"
        else
          shell date
          printf "Yield to unknown\n"
        end
      end
    end
  end
  c
end
b tasking.c:35
commands
silent
  if next_pid==0
    shell date
    printf "Kernel task registered\n"
  else
    if next_pid==1
      shell date
      printf "Init created\n"
    else
      if next_pid==2
        shell date
        printf "VFS created\n"
      else
        if next_pid==3
          shell date
          printf "fsdrv created\n"
        else
          shell date
          printf "Unknown task created\n"
        end
      end
    end
  end
  c
end
b mailboxes.c:14
commands
silent
  shell date
  printf "Mailbox %d created.\n",next_box
  c
end
b mailboxes.c:27
commands
silent
  shell date
  printf "Message sent from box %d to box %d\n",user_msg->from,user_msg->to
  c
end
b mailboxes.c:48
commands
silent
  shell date
  if mailbox.msg_store[mailbox.rd].size==0
    printf "Box %d attempted to get a message, but there were none.\n",box
  else
    printf "Box %d got a message.\n",box
  end
  c
end
c

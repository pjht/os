global __pthread_spin_lock_helper

__pthread_spin_lock_helper:
  mov ebx,[esp+4]
  mov eax,1
  xchg eax,[ebx]
  test eax, eax
  jnz __pthread_spin_lock_helper
  ret

void halt() {
  asm volatile("cli;\
  hltlabel: hlt;\
  jmp hltlabel");
}

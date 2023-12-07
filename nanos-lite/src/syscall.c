#include <common.h>
#include "syscall.h"

#ifndef CONFIG_STRACE
#define CONFIG_STRACE
#endif

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  #ifdef CONFIG_STRACE
    printf("Syscall NO.%u\n", a[0]);
  #endif
  switch (a[0]) {
    case 0: halt(0); break;
    case 1: yield(); c->GPRx = 0; break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

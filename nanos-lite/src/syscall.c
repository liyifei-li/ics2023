#include <common.h>
#include "syscall.h"

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  #ifdef CONFIG_STRACE
    printf("Syscall NO.%u\n", a[0]);
  #endif
  switch (a[0]) {
    case SYS_exit: halt(0); break;
    case SYS_yield: yield(); c->GPRx = 0; break;
    case SYS_write:
      if (a[1] == 1 || a[1] == 2) {
        char *buf = (char *)a[2];
        size_t count = (size_t)a[3];
        for (int i = 0; i < count; i++) {
          putch(*(buf + i));
          assert(0);
        }
        c->GPRx = count;
      }
    break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

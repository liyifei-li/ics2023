#include <common.h>
#include "syscall.h"

int do_syswrite(int fd, char *buf, size_t count);
int do_sysbrk(void *addr);

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  #ifdef CONFIG_STRACE
    Log("Syscall NO.%u", a[0]);
  #endif
  switch (a[0]) {
    case SYS_exit: halt(0); break;
    case SYS_yield: yield(); c->GPRx = 0; break;
    case SYS_write: c->GPRx = do_syswrite(a[1], (char *)a[2], a[3]); break;
    case SYS_brk: c->GPRx = -1; break;//do_sysbrk((void *)a[1]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

int do_syswrite(int fd, char *buf, size_t count) {
  if (fd == 1 || fd == 2) {
    for (int i = 0; i < count; i++) {
      putch(*(buf + i));
    }
    return count;
  }
  return -1;
}

int do_sysbrk(void *addr) {
  return 0;
}

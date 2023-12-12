#include <common.h>
#include "syscall.h"
#include <fs.h>
#include <sys/time.h>

int sys_brk(void *addr);
int sys_execve(const char *fname, char * const argv[], char *const envp[]);
int sys_gettimeofday(struct timeval *tv, struct timezone *tz);

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  #ifdef CONFIG_STRACE
    if (a[0] == SYS_read || a[0] == SYS_write || a[0] == SYS_close || a[0] == SYS_lseek)
      Log("Syscall NO.%u, fd = %d", a[0], a[1]);
    else
      Log("Syscall NO.%u", a[0]);
  #endif
  switch (a[0]) {
    case SYS_exit: halt(0); break;
    case SYS_yield: yield(); c->GPRx = 0; break;
    case SYS_open: c->GPRx = fs_open((char *)a[1], a[2], a[3]); break;
    case SYS_read: c->GPRx = fs_read(a[1], (void *)a[2], a[3]); break;
    case SYS_write: c->GPRx = fs_write(a[1], (void *)a[2], a[3]); break;
    case SYS_close: c->GPRx = fs_close(a[1]); break;
    case SYS_lseek: c->GPRx = fs_lseek(a[1], a[2], a[3]); break;
    case SYS_brk: c->GPRx = sys_brk((void *)a[1]); break;
//    case SYS_execve: c->GPRx = do_sysexecve((const char *)a[1], (char *)a[2], (char *)a[3]); break;
    case SYS_gettimeofday: c->GPRx = sys_gettimeofday((struct timeval *)a[1], (struct timezone *)a[2]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

int sys_brk(void *addr) {
  return 0;//OK for PA3
}

int sys_gettimeofday(struct timeval *tv, struct timezone *tz) {
  return 0;
}

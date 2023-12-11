#include <common.h>
#include "syscall.h"
#include <fs.h>

#define off_t size_t

int do_sysopen(const char *pathname, int flags, int mode);
int do_sysread(int fd, void *buf, size_t count);
int do_syswrite(int fd, void *buf, size_t count);
int do_sysclose(int fd);
off_t do_syslseek(int fd, off_t offset, int whence);
int do_sysbrk(void *addr);
int do_sysexecve(const char *fname, char * const argv[], char *const envp[]);
//int gettimeofday(struct timeval *tv, struct timezone *tz);

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
    case SYS_open: c->GPRx = do_sysopen((char *)a[1], a[2], a[3]); break;
    case SYS_read: c->GPRx = do_sysread(a[1], (void *)a[2], a[3]); break;
    case SYS_write: c->GPRx = do_syswrite(a[1], (void *)a[2], a[3]); break;
    case SYS_close: c->GPRx = do_sysclose(a[1]); break;
    case SYS_lseek: c->GPRx = do_syslseek(a[1], a[2], a[3]); break;
    case SYS_brk: c->GPRx = do_sysbrk((void *)a[1]); break;
//    case SYS_execve: c->GPRx = do_sysexecve((const char *)a[1], (char *)a[2], (char *)a[3]); break;
 //   case SYS_gettimeofday: c->GPRx = do_sysgettimeofday(a[1], a[2], a[3]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

int do_sysopen(const char *pathname, int flags, int mode) {
  return fs_open(pathname, flags, mode);
}

int do_syswrite(int fd, void *buf, size_t count) {
  char *p = buf;
  assert(fd == 1 || fd == 2);
  if (fd == 1 || fd == 2) {
    for (int i = 0; i < count; i++) {
      putch(*(p + i));
    }
    return count;
  }
  return -1;
}

int do_sysbrk(void *addr) {
  return 0;//OK for PA3
}

int do_sysread(int fd, void *buf, size_t count) {
  assert(0);
  return 0;
}

int do_sysclose(int fd) {
  assert(0);
  return 0;
}

off_t do_syslseek(int fd, off_t offset, int whence) {
  return fs_lseek(fd, offset, whence);
}

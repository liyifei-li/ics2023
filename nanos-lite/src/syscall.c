#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  printf("%d", EVENT_YIELD);
  Log("aaaaa\n\n\n");
  switch (a[0]) {
    case 1: printf("OMG its yield111\n"); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

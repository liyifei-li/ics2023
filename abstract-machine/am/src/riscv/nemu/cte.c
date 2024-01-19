#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

void __am_get_cur_as(Context *c);

void __am_switch(Context *c);

Context* __am_irq_handle(Context *c) {
  __am_get_cur_as(c);
  if (user_handler) { //do_event
    Event ev = {0};
    switch (c->mcause) {
      case -1: ev.event = EVENT_YIELD; break;
      case 0: ev.event = EVENT_SYSCALL; break;
      case 1: ev.event = EVENT_SYSCALL; break;
      case 2: ev.event = EVENT_SYSCALL; break;
      case 3: ev.event = EVENT_SYSCALL; break;
      case 4: ev.event = EVENT_SYSCALL; break;
      case 7: ev.event = EVENT_SYSCALL; break;
      case 8: ev.event = EVENT_SYSCALL; break;
      case 9: ev.event = EVENT_SYSCALL; break;
      case 13: ev.event = EVENT_SYSCALL; break;
      case 19: ev.event = EVENT_SYSCALL; break;
      default: ev.event = EVENT_ERROR; break;
    }
    c = user_handler(ev, c); //do_event
    assert(c != NULL);
  }
  __am_switch(c);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));
  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *c = (Context *)kstack.end - 1;
  c->mstatus = 0x1800;//To pass difftest
  c->mepc = (uintptr_t)entry;
  c->GPRx = (uintptr_t)arg;
  c->pdir = NULL;
  printf("kcontext: c: %p, &c->pdir: %p\n", c, &c->pdir);
  return c;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}

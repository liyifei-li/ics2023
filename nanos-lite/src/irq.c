#include <common.h>
#include <proc.h>
#include <syscall.h>

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    case EVENT_NULL:      assert(0);       break;
    case EVENT_YIELD:     c = schedule(c); break;
    case EVENT_SYSCALL:   do_syscall(c);   break;
    case EVENT_PAGEFAULT: assert(0);       break;
    case EVENT_ERROR:     assert(0);       break;
    case EVENT_IRQ_TIMER:                  break;
    case EVENT_IRQ_IODEV: assert(0);       break;
    default: panic("Unhandled event ID = %d", e.event);
  }
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}

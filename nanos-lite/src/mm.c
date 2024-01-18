#include <memory.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  assert(pf != NULL);
  void *ret = pf;
  pf += nr_page * PGSIZE;
  assert(((uintptr_t)ret & 0xfff) == 0);
  return ret;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  size_t nr_page = n / PGSIZE;
  assert(nr_page * PGSIZE == n);
  void *ret = new_page(nr_page);
  memset(ret, 0, n);
  return ret;
}
#endif

void free_page(void *p) {
  // panic("not implement yet");
  // Implemented by doing nothing
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  printf("%p\n", brk);
  assert(0);
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}

#include <memory.h>
#include <proc.h>

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
  if (brk <= current->max_brk) {
    return 0;
  }
  void *curpage = (void *)(((current->max_brk + 0xfff) & 0xfffff000));
  while ((uintptr_t)curpage <= brk) {
//    printf("curpage: %p\n", curpage);
    void *newpage = new_page(1);
    map(&current->as, curpage, newpage, 0);
    curpage += PGSIZE;
  }
  current->max_brk = brk;
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}

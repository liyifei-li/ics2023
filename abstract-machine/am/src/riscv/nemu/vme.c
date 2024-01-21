#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  if (c->pdir != NULL) c->pdir = (vme_enable ? (void *)get_satp() : NULL);
  // printf("am_get_cur_as: c->pdir = %p\n", &c->pdir);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    // printf("set_satp\n");
    set_satp(c->pdir);
  }
}

#define PTESIZE 4
#define VA ((uintptr_t)va)
#define PA ((uintptr_t)pa)
#define VPN1(n) ((n) >> 22)
#define VPN0(n) (((n) >> 12) & 0x3ff)
#define PPN1(n) ((n) >> 22)
#define PPN0(n) (((n) >> 10) & 0x3ff)
#define TOPPN(n) ((n) >> 2)
#define OFFSET(n) ((n) & 0xfff)

void map(AddrSpace *as, void *va, void *pa, int prot) {
  // printf("map: %p %p\n", va, pa);
  assert(OFFSET(VA) == 0);
  assert(OFFSET(PA) == 0);

  PTE PTE1 = (PTE)as->ptr + PTESIZE * VPN1(VA);
  // printf("%p\n", as->ptr);
  if (*(PTE *)PTE1 == 0) {
    *(PTE *)PTE1 = (PTE)pgalloc_usr(PGSIZE);
    *(PTE *)PTE1 |= PTE_V;
  }
  // printf("0x%8x\n", *(PTE *)PTE1);
  PTE PTE0 = (*(PTE *)PTE1 & 0xfffff000) + PTESIZE * VPN0(VA);
  *(PTE *)PTE0 = (PTE_V | PTE_R | PTE_W | PTE_X) | TOPPN(PA);
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *c = kstack.end - sizeof(Context) - 4;
  c->gpr[2] = (uintptr_t)kstack.end - 4;
  printf("c->gpr[2]: %p\n", c->gpr[2]);
  c->mstatus = 0x1880;
  c->mepc = (uintptr_t)entry;
  c->pdir = as->ptr;
  c->np = 0; // USER
  // printf("ucontext: c: %p, pdir: %p\n", c, c->pdir);
  return c;
}

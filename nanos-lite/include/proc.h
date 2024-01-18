#ifndef __PROC_H__
#define __PROC_H__

#include <common.h>
#include <memory.h>

#define STACK_SIZE (8 * PGSIZE)

typedef union {
  uint8_t stack[STACK_SIZE] PG_ALIGN;
  struct {
    Context *cp;
    AddrSpace as;
    // we do not free memory, so use `max_brk' to determine when to call _map()
    char bufa[4096];
    uintptr_t max_brk;
    char bufb[4096];
  };
} PCB;

extern PCB *current;
void naive_uload(PCB *pcb, const char *filename);
void switch_boot_pcb();
void context_kload(PCB *pcb, void (*entry)(void *), void *arg);
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);
Context* schedule(Context *prev);
#endif

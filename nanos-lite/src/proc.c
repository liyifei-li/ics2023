#include <proc.h>
#include <fs.h>
#include <loader.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    if (j % 1000 == 0)
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j / 1000);
    j ++;
    yield();
  }
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
//  protect(&pcb->as);
  pcb->cp = kcontext((Area) { pcb->stack, pcb + 1 }, entry, arg);
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  assert(argv != NULL && envp != NULL);
  protect(&pcb->as);
  int argv_length = 0;
  while (argv[argv_length] != NULL) argv_length++;
  int envp_length = 0;
  while (envp[envp_length] != NULL) envp_length++;
  uintptr_t *argv_ptr[argv_length + 1], *envp_ptr[envp_length + 1];
  void *cur = new_page(8) + 8 * PGSIZE - 1;
  size_t len;
  argv_ptr[argv_length] = NULL;
  envp_ptr[envp_length] = NULL;
  for (int i = 0; i < argv_length; i++) {
    len = strlen(argv[i]);
    cur -= len + 1;
    strcpy((char *)cur, argv[i]);
    argv_ptr[i] = cur;
  }
  for (int i = 0; i < envp_length; i++) {
    len = strlen(envp[i]);
    cur -= len + 1;
    strcpy((char *)cur, envp[i]);
    envp_ptr[i] = cur;
  }
  envp_ptr[envp_length] = 0;
  cur = (void *)((uintptr_t)cur & 0xfffffffc);
  for (int i = envp_length; i >= 0; i--) {
    cur -= 4;//4 is the length of void *
    *(uintptr_t *)cur = (uintptr_t)envp_ptr[i];
  }
  for (int i = argv_length; i >= 0; i--) {
    cur -= 4;
    *(uintptr_t *)cur = (uintptr_t)argv_ptr[i];
  }
  cur -= 4;
  *(uintptr_t *)cur = argv_length;
  void *entry = (void *)loader(pcb, filename);
  
  pcb->cp = ucontext(&pcb->as, (Area) { pcb->stack, pcb + 1 }, entry);
  pcb->cp->GPRx = (uintptr_t)cur;
}

void init_proc() {
  char *argv[] = {"/bin/pal", "--skip", NULL};
  char *envp[] = {NULL};
  context_kload(&pcb[0], hello_fun, (void *)1);
  context_uload(&pcb[1], "/bin/pal", argv, envp);
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/nterm");

}

Context *schedule(Context *prev) {
  printf("%p %p\n", current, &pcb[1]);
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}

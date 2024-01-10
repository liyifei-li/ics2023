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
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  pcb->cp = kcontext((Area) { pcb->stack, pcb + 1 }, entry, arg);
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  int argv_length = 0;
  while (argv[argv_length] != NULL) argv_length++;
  int envp_length = 0;
  while (envp[envp_length] != NULL) envp_length++;
  uintptr_t argv_ptr[argv_length + 1], envp_ptr[envp_length + 1];
  uintptr_t cur = (uintptr_t)heap.end - 1;
  size_t len;
  argv_ptr[argv_length] = 0;
  envp_ptr[envp_length] = 0;
  for (int i = 0; i < argv_length; i++) {
    len = strlen(argv[i]);
    cur -= len + 1;
    strcpy((char *)cur, argv[i]);
    argv_ptr[i] = (uintptr_t)cur;
  }
  argv_ptr[argv_length] = 0;
  for (int i = 0; i < envp_length; i++) {
    len = strlen(envp[i]);
    cur -= len + 1;
    strcpy((char *)cur, envp[i]);
    envp_ptr[i] = (uintptr_t)cur;
    printf("%s\n", envp_ptr[i]);
  }
  envp_ptr[envp_length] = 0;
  printf("%s\n", envp_ptr[1]);
  cur = (cur - 1) & 0xfffffffc;
  printf("%s\n", envp_ptr[1]);
  for (int i = envp_length; i >= 0; i--) {
    *(uintptr_t *)cur = envp_ptr[i];
    cur -= 4;//4 is the length of uintptr_t *
  }
  for (int i = argv_length; i >= 0; i--) {
    *(uintptr_t *)cur = argv_ptr[i];
    cur -= 4;//4 is the length of uintptr_t *
  }
  printf("%s\n", envp_ptr[1]);
  assert(0);
  *(uintptr_t *)cur = argv_length;
  void *entry = (void *)loader(pcb, filename);
  
  pcb->cp = ucontext(NULL, (Area) { pcb->stack, pcb + 1 }, entry);
  pcb->cp->GPRx = cur;
}

void init_proc() {
  char *argv[] = {"--skip", "test", NULL};
  char *envp[] = {"argp1=1", "test=apples", NULL};
  context_kload(&pcb[0], hello_fun, (void *)1);
  context_uload(&pcb[1], "/bin/pal", argv, envp);
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/nterm");

}

Context *schedule(Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}

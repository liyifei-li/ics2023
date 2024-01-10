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
  uintptr_t argv_ptr[argv_length], envp_ptr[envp_length];
  char *cur = (char *)heap.end - 200;
  size_t len;
  argv_ptr[1] = 0;
  envp_ptr[1] = 0;
  for (int i = 0; i < argv_length; i++) {
    len = strlen(argv[i]);
    cur -= len + 1;
    printf("1");
    strncpy(cur, argv[i], len);
    printf("2");
    argv_ptr[i] = (uintptr_t)cur;
  }
    printf("%s\n", argv_ptr[1]);
  for (int i = 0; i < envp_length; i++) {
    len = strlen(envp[i]);
    cur -= len + 1;
    strncpy(cur, envp[i], len);
    envp_ptr[i] = (uintptr_t)cur;
  }
    printf("%s\n", envp_ptr[1]);
  assert(0);
  void *entry = (void *)loader(pcb, filename);
  
  pcb->cp = ucontext(NULL, (Area) { pcb->stack, pcb + 1 }, entry);
}

void init_proc() {
  char *argv[] = {"--skip", "test", NULL};
  char *argp[] = {"argp1=1", "test=apples", NULL};
  context_kload(&pcb[0], hello_fun, (void *)1);
  context_uload(&pcb[1], "/bin/pal", argv, argp);
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

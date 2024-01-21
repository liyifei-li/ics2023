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
    j++;
    yield();
  }
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  pcb->cp = kcontext((Area) { pcb->stack, pcb->stack + STACK_SIZE }, entry, arg);
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  assert(argv != NULL && envp != NULL);
  protect(&pcb->as);
  int argv_length = 0;
  while (argv[argv_length] != NULL) argv_length++;
  int envp_length = 0;
  while (envp[envp_length] != NULL) envp_length++;
  uintptr_t *argv_ptr[argv_length + 1], *envp_ptr[envp_length + 1];
  void *pstack = new_page(8);
  void *pstack_end = pstack + 8 * PGSIZE - 1;
  void *cur = pstack_end;
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

  void *vstack = (void *)0x7fff8000;
  for (int i = 0; i < 8; i++) {
    // printf("mapped from %p to %p\n", vstack + i * PGSIZE, pstack + i * PGSIZE);
    map(&pcb->as, vstack + i * PGSIZE, pstack + i * PGSIZE, 0);
  }
  void *vstack_end = (void *)0x7fffffff;
  void *vcur = vstack_end - (pstack_end - cur);

  pcb->cp = ucontext(&pcb->as, (Area) { pcb->stack, pcb->stack + STACK_SIZE }, entry);
  pcb->cp->GPRx = (uintptr_t)vcur;
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
  static unsigned char cnt = 0;
  current->cp = prev;
  current = &pcb[0];
  /*
  if (cnt == 0) {
    current = &pcb[0];
  }
  else {
    current = &pcb[1];
  }
  */
  cnt++;
  // current = &pcb[1];
  // printf("&current: %p\n", &current); //0x8214a010
  // printf("%s: current: %p, current->cp->mstatus: %p\n", current == &pcb[0] ? "kernel" : "user", current, current->cp->mstatus);
  // 0x82129fdc
  // printf("hello_fun:%p\n", (void *)hello_fun);
  return current->cp;
}

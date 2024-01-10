#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  int argc = *(int *)args;
  args++;
  char **argv = (char **)args;
  args += argc + 1;
  char **envp = (char **)args;
  exit(main(argc, argv, envp));
  assert(0);
}

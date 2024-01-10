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
  args += argc;
  char **envp = (char **)args;
  printf("%d %s %s\n", argc, argv[1], envp[1]);
  exit(main(argc, argv, envp));
  assert(0);
}

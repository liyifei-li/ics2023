#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  environ = envp;
  for (int i = 0; i < 2; i++) {
    printf("%s %s\n", argv[i], envp[i]);
  }
  exit(main(argc, argv, argp));
  assert(0);
}

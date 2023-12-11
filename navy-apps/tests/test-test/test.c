#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main() {
  printf("Hello\n");
  printf("Bye\n");
  FILE *asdf = NULL;
  asdf = asdf;
  printf("Bye\n");
  assert(0);
  return 0;
}

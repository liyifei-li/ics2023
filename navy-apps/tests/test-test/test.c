#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main() {
  FILE *fp = fopen("/share/files/num", "r+");
  assert(0);
  printf("Hello\n");
  printf("Bye\n");
  FILE *asdf = NULL;
  asdf = asdf;
  printf("Bye\n");
  assert(0);
  return 0;
}

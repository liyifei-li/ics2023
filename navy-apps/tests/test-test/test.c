#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main() {
  FILE *fp = fopen("/share/files/num", "r+");
  for (int i = 0; i < 1000000; i++) printf("114514\n");
  /*
  printf("Hello\n");
  printf("Bye\n");
  FILE *asdf = NULL;
  asdf = asdf;
  printf("Bye\n");
  assert(0);
  */
  return 0;
}

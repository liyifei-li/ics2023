#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main() {
  FILE *fp = fopen("/share/files/num", "r+");
  printf("112514\n");
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

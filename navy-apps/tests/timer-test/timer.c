#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <NDL.h>

uint32_t cur, get;

int main() {
  cur = NDL_GetTicks();
  while (1) {
    get = NDL_GetTicks();
    if (get - cur >= 500000) {
      printf("test\n");
      cur = get;
    }
  }
  return 0;
}

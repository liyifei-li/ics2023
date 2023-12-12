#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

struct timeval cur, get;

int main() {
  gettimeofday(&cur, NULL);
  while (1) {
    gettimeofday(&get, NULL);
    if (1000000 * (get.tv_sec - cur.tv_sec) + (get.tv_usec - cur.tv_usec) >= 500000) {
      printf("test\n");
      cur = get;
    }
  }
  return 0;
}

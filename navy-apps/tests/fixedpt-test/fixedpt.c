#include <unistd.h>
#include <stdio.h>
#include <fixedptc.h>

int main() {
  fixedpt a, b, c;
  a = fixedpt_rconst(100);
  b = fixedpt_rconst(-1);
  c = fixedpt_divi(a, -1);
  printf("ANS: %d\n", c);

  return 0;
}

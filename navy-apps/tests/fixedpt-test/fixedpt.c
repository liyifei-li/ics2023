#include <unistd.h>
#include <stdio.h>
#include <fixedptc.h>

int main() {
  fixedpt a, b, c;
  a = fixedpt_rconst(10);
  b = fixedpt_rconst(-20);
  c = fixedpt_div(a, b);
  printf("ANS: %d\n", c);

  return 0;
}

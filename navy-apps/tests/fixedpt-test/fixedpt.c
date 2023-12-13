#include <unistd.h>
#include <stdio.h>
#include <fixedptc.h>

int main() {
  fixedpt a, b, c;
  a = fixedpt_fromint(10);
  b = fixedpt_fromint(-20);
  c = fixedpt_div(a, b);
  printf("ANS: %f\n", fixedpt_tofloat(c));

  return 0;
}

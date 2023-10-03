/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

char *loc = buf;
uint32_t len;

uint32_t choose(uint32_t n) {
  return rand() % n;
}

void gen(char ch) {
  if (len >= 128) return;
  *(loc + len) = ch;
  len++;
}

void gen_num() {
  if (len >= 128) return;
  uint32_t num = choose(10);
  gen(num + '0');
  return;
}

void gen_rand_op() {
  if (len >= 128) return;
  switch(choose(4)) {
    case 0:
      gen('+');
      break;
    case 1:
      gen('-');
      break;
    case 2:
      gen('*');
      break;
    case 3:
      gen('/');
      break;
  }
  return;
}

static void gen_rand_expr() {
//  buf[0] = '\0';
  if (len >= 128) return;
  switch(choose(3)) {
    case 0:
      gen_num();
      break;
    case 1:
      gen('(');
      gen_rand_expr();
      gen(')');
      break;
    case 2:
      gen_rand_expr();
      gen_rand_op();
      gen_rand_expr();
      break;
  }
  return;
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;

  memset(buf, 0, sizeof(buf));
  len = 0;

  for (i = 0; i < loop; i ++) {
    len = 0;
    memset(buf, 0, sizeof(buf));
    gen_rand_expr();

    while (len >= 128) {
      len = 0;
      memset(buf, 0, sizeof(buf));
      gen_rand_expr();
    }

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    int retval;
    ret = fscanf(fp, "%d", &result);
    retval = pclose(fp);
    if (retval != 0) {
      continue;
    }
    printf("%u %s\n", result, buf);
  }
  return 0;
}

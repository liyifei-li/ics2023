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
uint32_t len, lenlen;
static char bufbuf[65536] = {};
char *locloc = bufbuf;


uint32_t choose(uint32_t n) {
  return rand() % n;
}

void gen(char ch) {
  if (len >= 128) return;
  *(loc + len) = ch;
  len++;
  *(locloc + lenlen) = ch;
  lenlen++;
}

void gengen(char ch) {
  if (len >= 128)
    return;
  *(locloc + lenlen) = ch;
  lenlen++;
}

void putspace() {
  uint32_t num = choose(3);
  for (int i = 0; i < num; i++) 
    gen(' ');
}

void gen_num() {
  if (len >= 128)
    return;
  uint32_t num = choose(10);
  putspace();
  gengen('(');
  gengen('u');
  gengen('n');
  gengen('s');
  gengen('i');
  gengen('g');
  gengen('n');
  gengen('e');
  gengen('d');
  gengen(')');
  gengen('(');
  gen(num + '0');
  putspace();
  gengen(')');
  return;
}

void gen_rand_op() {
  if (len >= 128)
    return;
  putspace();
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
  if (len >= 128)
    return;
  putspace();
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

void init() {
  len = 0;
  lenlen = 0;
  memset(buf, 0, sizeof(buf));
  memset(bufbuf, 0, sizeof(bufbuf));
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;

  init();
  for (i = 0; i < loop; i ++) {
    init();
    gen_rand_expr();

    while (len >= 128) {
      init();
      gen_rand_expr();
    }

    sprintf(code_buf, code_format, bufbuf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -Wall -Werror -o /tmp/.expr");
    if (ret != 0) {
      i--;
      continue;
    }

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    if (ret == 0) {
      i--;
      continue;
    }
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}

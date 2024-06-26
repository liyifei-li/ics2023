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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

#include <memory/paddr.h>
#include <memory/vaddr.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NEQ, TK_AND, TK_DECINT, TK_HEXINT, TK_REG, DEREF, UMINUS,

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  {" +", TK_NOTYPE},      // spaces
  {"0x[0-9a-f]+", TK_HEXINT},// hexadecimal integer
  {"[0-9]+", TK_DECINT},  // decimal integer
  {"\\+", '+'},           // plus
  {"\\-", '-'},           // minus
  {"\\*", '*'},           // multiply OR pointer
  {"/", '/'},             // divide
  {"\\(", '('},           // left parentheses
  {"\\)", ')'},           // right parentheses
  {"\\$(0|pc|ra|[sgt]p|([astx][0-9]+))", TK_REG}, // register (no error handle)
  {"==", TK_EQ},          // equal
  {"!=", TK_NEQ},         // not equal
  {"<", '<'},             // below
  {">", '>'},             // above
  {"&&", TK_AND},         // logical and
  {"$a", DEREF},          // dereference, will never match
  {"$a", UMINUS},         // unary minus, will never match
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  int precedence;
  char str[32];
} Token;

static Token tokens[1024] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
/*
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
*/
        position += substr_len;

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case TK_DECINT:
            if (substr_len >= 32)
              panic("decimal integer too long");
            tokens[nr_token].precedence = 0;
            tokens[nr_token].type = TK_DECINT;
            memset(tokens[nr_token].str, 0, sizeof(tokens[nr_token].str));
            strncpy(tokens[nr_token++].str, substr_start, substr_len);
            break;
          case TK_HEXINT:
            if (substr_len >= 32)
              panic("hexadecimal integer too long");
            tokens[nr_token].precedence = 0;
            tokens[nr_token].type = TK_HEXINT;
            memset(tokens[nr_token].str, 0, sizeof(tokens[nr_token].str));
            strncpy(tokens[nr_token++].str, substr_start, substr_len);
            break;
          case TK_REG:
            if (substr_len >= 32)
              panic("register name too long");
            tokens[nr_token].precedence = 0;
            tokens[nr_token].type = TK_REG;
            memset(tokens[nr_token].str, 0, sizeof(tokens[nr_token].str));
            strncpy(tokens[nr_token++].str, substr_start, substr_len);
            break;
          case '+':
            tokens[nr_token].precedence = 4;
            tokens[nr_token++].type = '+';
            break;
          case '-':
            tokens[nr_token].precedence = 4;
            tokens[nr_token++].type = '-';
            break;
          case '*':
            tokens[nr_token].precedence = 3;
            tokens[nr_token++].type = '*';
            break;
          case '/':
            tokens[nr_token].precedence = 3;
            tokens[nr_token++].type = '/';
            break;
          case '(':
            tokens[nr_token].precedence = 1;
            tokens[nr_token++].type = '(';
            break;
          case ')':
            tokens[nr_token].precedence = 1;
            tokens[nr_token++].type = ')';
            break;
          case '<':
            tokens[nr_token].precedence = 6;
            tokens[nr_token++].type = '<';
            break;
          case '>':
            tokens[nr_token].precedence = 6;
            tokens[nr_token++].type = '>';
            break;
          case TK_EQ:
            tokens[nr_token].precedence = 7;
            tokens[nr_token++].type = TK_EQ;
            break;
          case TK_NEQ:
            tokens[nr_token].precedence = 7;
            tokens[nr_token++].type = TK_NEQ;
            break;
          case TK_AND:
            tokens[nr_token].precedence = 11;
            tokens[nr_token++].type = TK_AND;
            break;
          default: TODO();
        }
        break;
      }
    }
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  for (i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*' && (i == 0 || tokens[i - 1].type == '('
    || tokens[i - 1].type == '+' || tokens[i - 1].type == '-'
    || tokens[i - 1].type == '*' || tokens[i - 1].type == '/'
    || tokens[i - 1].type == '<' || tokens[i - 1].type == '>'
    || tokens[i - 1].type == TK_EQ || tokens[i - 1].type == TK_NEQ
    || tokens[i - 1].type == TK_AND || tokens[i - 1].type == DEREF
    || tokens[i - 1].type == UMINUS)) {
      tokens[i].type = DEREF;
      tokens[i].precedence = 2;
    }
    if (tokens[i].type == '-' && (i == 0 || tokens[i - 1].type == '('
    || tokens[i - 1].type == '+' || tokens[i - 1].type == '-'
    || tokens[i - 1].type == '*' || tokens[i - 1].type == '/'
    || tokens[i - 1].type == '<' || tokens[i - 1].type == '>'
    || tokens[i - 1].type == TK_EQ || tokens[i - 1].type == TK_NEQ
    || tokens[i - 1].type == TK_AND || tokens[i - 1].type == DEREF
    || tokens[i - 1].type == UMINUS)) {
      tokens[i].type = UMINUS;
      tokens[i].precedence = 2;
    }
  }
  return true;
}

typedef struct expr_res {
  uint32_t value;
  bool error;
} exprs;

bool check_parentheses(uint32_t p, uint32_t q) {
  uint32_t cnt = 0;
  for (int i = p; i < q - 1; i++) {
    if (tokens[i].type == '(')
      cnt++;
    if (tokens[i].type == ')')
      cnt--;
    if (cnt == 0)
      return false;
  }
  if (cnt != 1 || tokens[q - 1].type != ')') {
    return false;
  }
  return true;
}

uint32_t find_mainop(uint32_t p, uint32_t q) {
  uint32_t ret = p, pre = 0, cnt = 0, unary = 0x3f3f3f3f;
  for (uint32_t i = p; i < q; i++) {
    if (tokens[i].type == '(')
      cnt++;
    if (tokens[i].type == ')')
      cnt--;
    if (cnt == 0) {
      if (pre <= tokens[i].precedence) {
        ret = i;
        pre = tokens[i].precedence;
      }
      if (unary == 0x3f3f3f3f && tokens[i].precedence == 2)
        unary = i;
    }
  }
//  Log("ret = %d", ret);
  if (tokens[ret].precedence == 2)
    return unary;
  return ret; 
}

uint32_t deval(uint32_t p, bool *success) {
  uint32_t ret;
  if (tokens[p].type == TK_DECINT)
    sscanf(tokens[p].str, "%d", &ret);
  else if (tokens[p].type == TK_HEXINT)
    sscanf(tokens[p].str, "%x", &ret);
  else
    ret = isa_reg_str2val(tokens[p].str, success);
  return ret;
}

exprs eval(uint32_t p, uint32_t q) {
  exprs ret = {0, 0};
  if (p >= q)
    ret.error = 1;
  else if (p + 1 == q) {
    if (tokens[p].type == TK_DECINT || tokens[p].type == TK_HEXINT || tokens[p].type == TK_REG) {
      bool success = 1;
      ret.value = deval(p, &success);
      ret.error = !success;
    }
    else {
      ret.error = 1;
    }
  }
  else {
    if (check_parentheses(p, q))
      return eval(p + 1, q - 1);
    uint32_t op = find_mainop(p, q);
    if (tokens[op].type == DEREF) {
      ret = eval(p + 1, q);
      if (ret.error == 1)
        ret.value = 0;
      else
        ret.value = paddr_read(ret.value, 4);
    }
    else if (tokens[op].type == UMINUS) {
      ret = eval(p + 1, q);
      if (ret.error == 1)
        ret.value = 0;
      else
        ret.value = -ret.value;
    }
    else {
      exprs subret1 = eval(p, op);
      exprs subret2 = eval(op + 1, q);
      if (subret1.error || subret2.error)
        ret.error = 1;
      else {
        switch (tokens[op].type) {
          case '+':
            ret.value = subret1.value + subret2.value;
            break; 
          case '-':
            ret.value = subret1.value - subret2.value;
            break;
          case '*':
            ret.value = subret1.value * subret2.value;
            break;
          case '/':
            ret.value = subret1.value / subret2.value;
            break;
          case '<':
            ret.value = subret1.value < subret2.value;
            break;
          case '>':
            ret.value = subret1.value > subret2.value;
            break;
          case TK_EQ:
            ret.value = subret1.value == subret2.value;
            break;
          case TK_NEQ:
            ret.value = subret1.value != subret2.value;
            break;
          case TK_AND:
            ret.value = subret1.value && subret2.value;
            break;
          default: TODO();
        }
      }
    }
  }
  return ret;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    success = false;
    return 0;
  }
  exprs result = eval(0, nr_token);
  if (result.error) {
    *success = false;
    return 0;
  }
  *success = true;
  return result.value;
}

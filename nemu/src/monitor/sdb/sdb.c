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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

#include <memory/paddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

uint32_t new_wp(char *exprloc);
void free_wp(uint32_t N);
void wp_display();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
  char *token;
  uint32_t N = 0;
  token = strtok(args, " ");
  if (token == NULL)
    N = 1;
  else
    sscanf(token, "%d", &N);
  cpu_exec(N);
  return 0;
}

static int cmd_info(char *args) {
  char *token;
  token = strtok(args, " ");
  if (token == NULL)
    return 0;
  else if (strcmp(token, "r") == 0)
    isa_reg_display();
  else if (strcmp(token, "w") == 0)
    wp_display();
  else
    Log("Info command not found");
  return 0;
}

static int cmd_x(char *args) {
  char *token;
  uint32_t N;
  uint32_t *data;
  bool success = true;
  token = strtok(args, " ");
  sscanf(token, "%d", &N);
  token = strtok(NULL, " ");
  word_t result = expr(token, &success);
  if (success == false)
    Log("Failed to interpret expression");
  else {
    for (uint32_t i = 0; i < N; i++) {
      data = (uint32_t *)guest_to_host(result);
      printf("0x%08x: 0x%08x\n", result, *data);
      result += 4;
    }
  }
  return 0;
}

/*
static int cmd_x(char *args) {
  char *token;
  uint32_t N;
  uint32_t EXPR;
  word_t data;
  token = strtok(args, " ");
  sscanf(token, "%d", &N);
  token = strtok(NULL, " ");
  sscanf(token, "%x", &EXPR);
  for (uint32_t i = 0; i < N; i++) {
    data = paddr_read(EXPR, 4);
    printf("%#x\n", data);
    EXPR += 4;
  }
  return 0;
}
*/


static int cmd_p(char *args) {
  if (args == NULL)
    return 0;
  bool success = true;
  word_t result = expr(args, &success);
  if (success == false)
    Log("Failed to interpret expression");
  else
    printf("%u\n", result);
  return 0;
}

static int cmd_w(char *args) {
  if (args == NULL)
    return 0;
  uint32_t new = new_wp(args);
  if (new != -1)
    Log("Watchpoint NO.%d created", new);
  else
    Log("Invalid expression");
  return 0;
}

static int cmd_d(char *args) {
  if (args == NULL)
    return 0;
  int N;
  sscanf(args, "%d", &N);
  free_wp(N);
  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Step one instruction exactly. si [N] steps N instructions.", cmd_si },
  { "info", "Generic command for showing things about the program being debugged.", cmd_info },
  { "x", "Examine memory: x N EXPR.", cmd_x },
  { "p", "Print value of expression EXPR.", cmd_p },
  { "w", "Set watchpoint, w EXPR sets watchpoint for EXPR.", cmd_w },
  { "d", "Delete watchpoint, d N deletes the watchpoint numbered N.", cmd_d },
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void expr_test() {
  FILE *fp = fopen("/home/e7cf09/ics2023/nemu/tools/gen-expr/input", "r");
  assert (fp != NULL);
  uint32_t result;
  char *fgets_loc = NULL;
  char expr_ptr[1024];
  bool success;
  int ret;
  ret = ret;
  *fgets_loc = *fgets_loc;
  for (int i = 0; i < 10000; i++) {
    Log("%d", i);
    ret = fscanf(fp, "%u", &result);
    fgets_loc = fgets(expr_ptr, 1024, fp);
    expr_ptr[strlen(expr_ptr) - 1] = '\0';
    Log("%u %s", result, expr_ptr);
    word_t output = expr(expr_ptr, &success);
    if (success == 0) panic("unsuccessful");
    if (output != result) panic("unsucessful at %d, result=%u, output=%u\n", i + 1, result, output);
  }
  fclose(fp);
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();

//  expr_test();
}

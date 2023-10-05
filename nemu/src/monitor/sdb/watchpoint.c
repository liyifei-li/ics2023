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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  char str[1024];
  uint32_t value;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

int new_wp(char *exprloc) {
  if (free_ == NULL) {
    Log("Watchpoint number limit exceeded");
    //return NULL;
    assert(0);
  }
    uint32_t value; 
    bool success;
    value = expr(exprloc, &success);
    if (success == 0) {
      return -1;
    }
    WP *ret = free_;
    strcpy(ret->str, exprloc);
    free_ = free_->next;
    ret->next = head;
    ret->value = value;
    head = ret;
  return ret->NO;
}

void free_wp(int N) {
  if (head == NULL) {
    Log("Watchpoint did not found");
    return;
  }
  WP *cur = head;
  if (cur->NO == N) {
    head = head->next;
    cur->next = free_;
    free_ = cur;
    Log("Watchpoint %d freed", free_->NO);
    return;
  }
  WP *cnxt = cur->next;
  while (cnxt != NULL && cnxt->NO != N) {
    cur = cur->next;
    cnxt = cnxt->next;
    assert(cur != NULL);
    Log("%d %d", cur->NO, cnxt->NO);
  }
  if (cnxt == NULL) {
    Log("Watchpoint did not found");
    return;
  }
  cur->next = cnxt->next;
  cnxt->next = free_;
  free_ = cnxt;
  Log("Watchpoint %d freed", free_->NO);
  return;
}

void wp_traverse() {
  WP *cur = head;
  uint32_t value;
  bool success;
  while (cur != NULL) {
    value = expr(cur->str, &success);
    if (value != cur->value) {
      Log("NO:%d Expression: %s Value(old): %u Value(new): %u", cur->NO, cur->str, cur->value, value);
      nemu_state.state = NEMU_STOP;
    }
    cur->value = value;
    cur = cur->next;
  }
}

void wp_display() {
  if (head == NULL) {
    Log("There are no watchpoints");
    return;
  }
  WP *cur = head;
  uint32_t value;
  bool success;
  while (cur != NULL) {
    value = expr(cur->str, &success);
    assert(success == 1);
    Log("Watchpoint NO.%d has value %u", cur->NO, value);
    cur = cur->next;
  }
  return;
}

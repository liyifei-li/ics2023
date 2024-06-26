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
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

uint32_t regsize = sizeof(regs) / sizeof(regs[0]);

void isa_reg_display() {
  for (uint32_t i = 0; i < regsize; i++) { 
    printf("%-15s0x%-20x%u\n", reg_name(i), gpr(i), gpr(i));
  } 
}

word_t isa_reg_str2val(const char *s, bool *success) {
  *success = 0;
  if (strcmp("pc", s + 1) == 0) {
    *success = 1;
    return cpu.pc;
  }
  if (strcmp(regs[0], s) == 0) {
    *success = 1;
    return 0;
  }
  for (uint32_t i = 1; i < regsize; i++) {
    if (strcmp(regs[i], s + 1) == 0) {
      *success = 1;
      return gpr(i);
    }
  }
  return 0;
}

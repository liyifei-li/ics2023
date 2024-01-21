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

#define IRQ_TIMER 0x80000007
#define MIE_MASK 0x8
#define MPIE_MASK 0x80

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  cpu.mepc = ((NO == IRQ_TIMER) || (NO == 0)) ? epc : epc + 4;
  cpu.mcause = NO;
  // printf("start of exception: %d\n", NO);
    if (cpu.mstatus & MIE_MASK) {
      cpu.mstatus |= MPIE_MASK;
    }
    else {
      cpu.mstatus &= ~MPIE_MASK;
    }
    cpu.mstatus &= ~MIE_MASK;
    assert(cpu.mstatus != 0x1800);

  #ifdef CONFIG_ETRACE
    printf("intr NO.%d raised at pc=0x%08x\n", NO, epc);
  #endif
  return cpu.mtvec;
}

word_t isa_query_intr() {
  if (cpu.INTR == true && (cpu.mstatus & MIE_MASK)) {
    cpu.INTR = false;
    return IRQ_TIMER;
  }
  return INTR_EMPTY;
}

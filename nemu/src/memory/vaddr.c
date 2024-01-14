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
#include <memory/paddr.h>

word_t vaddr2paddr(vaddr_t addr, int len, int type) {
  switch (isa_mmu_check(addr, len, type)) {
    case MMU_DIRECT: break;
    case MMU_TRANSLATE: addr = (isa_mmu_translate(addr, len, 0) & 0xfffff000) | (addr & 0x00000fff); break;
    default: assert(0);
  }
  return addr;
}

word_t vaddr_ifetch(vaddr_t addr, int len) {
  addr = vaddr2paddr(addr, len, MEM_TYPE_IFETCH);
  return paddr_read(addr, len);
}

word_t vaddr_read(vaddr_t addr, int len) {
  addr = vaddr2paddr(addr, len, MEM_TYPE_READ);
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  addr = vaddr2paddr(addr, len, MEM_TYPE_WRITE);
  paddr_write(addr, len, data);
}

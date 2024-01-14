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
#include <memory/vaddr.h>
#include <memory/paddr.h>

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  printf("0x%8x\n", vaddr);
  uint32_t VPN1 = (vaddr >> 22);
  uint32_t VPN0 = (vaddr >> 12) & 0x3ff;
  PTE PTE1 = (cpu.satp << 12) + 4 * VPN1;
  printf("0x%8x\n", VPN1);
  PTE PTE0 = paddr_read(PTE1, 4);
  assert(PTE0 & PTE_V);//PTE_V
  paddr_t leaf = paddr_read((PTE0 & 0xfffff000) + 4 * VPN0, 4);
  assert(leaf & PTE_V);
  switch (type) {
    case MEM_TYPE_IFETCH: assert(leaf & PTE_X); break;
    case MEM_TYPE_READ:   assert(leaf & PTE_R); break;
    case MEM_TYPE_WRITE:  assert(leaf & PTE_W); break;
    default:              assert(0);
  }
  paddr_t pageaddr = 0xfffff000 & paddr_read((PTE0 & 0xfffff000) + 4 * VPN0, 4);
  paddr_t paddr = pageaddr | (0x00000fff & vaddr);
  assert(paddr == vaddr);
  return pageaddr | MEM_RET_OK;
}

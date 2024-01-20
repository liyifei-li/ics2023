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

#define PTESIZE 4
#define VPN1(n) ((n) >> 22)
#define VPN0(n) (((n) >> 12) & 0x3ff)
#define PPN1(n) ((n) >> 22)
#define PPN0(n) (((n) >> 10) & 0x3ff)
#define TOPA(n) (((n) & 0xfffffc00) << 2)
#define PTEPAGE(n) ((n) & 0xfffff000)
#define OFFSET(n) ((n) & 0xfff)

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  // printf("vaddr: 0x%08x pc: 0x%08x\n", vaddr, cpu.pc);
  assert(vaddr < 0x70000000 || vaddr >= 0x7fff8000);
  vaddr_t PTE1_ADDR = (cpu.satp << 12) + PTESIZE * VPN1(vaddr);
  PTE PTE1 = paddr_read(PTE1_ADDR, PTESIZE);
  // if (vaddr >= 0x7fff8000 && vaddr < 0x80000000) printf("0x%8x %x %x\n", vaddr, PTE1_ADDR, VPN1(vaddr));
  // printf("0x%8x %x %x\n", vaddr, PTE1_ADDR, VPN1(vaddr));
  if (PTE1 == 0) {
    printf("vaddr: 0x%08x, pc: 0x%08x\n", vaddr, cpu.pc);
    assert(vaddr < 0x7fff8000 || vaddr >= 0x80000000);
  }
  assert(PTE1);
  assert(PTE1 & PTE_V);
  vaddr_t PTE0_ADDR = PTEPAGE(PTE1) + PTESIZE * VPN0(vaddr);
  PTE PTE0 = paddr_read(PTE0_ADDR, PTESIZE);
  if (PTE0 == 0) assert(vaddr < 0x7fff8000 || vaddr >= 0x80000000);
  assert(PTE0);
  assert(PTE0 & PTE_V);
//  printf("satp: 0x%8x\n", cpu.satp << 12);
//  printf("PTE1: 0x%8x\n", PTE1);
//  printf("PTE0: 0x%8x\n", PTE0);
  switch (type) {
    case MEM_TYPE_IFETCH: assert(PTE0 & PTE_X); break;
    case MEM_TYPE_READ:   assert(PTE0 & PTE_R); break;
    case MEM_TYPE_WRITE:  assert(PTE0 & PTE_W); break;
    default:              assert(0);
  }
  paddr_t paddr = TOPA(PTE0);
  return paddr | MEM_RET_OK;
}

#ifndef __MM_H__
#define __MM_H__

#include <common.h>

void* new_page(size_t nr_page);
int mm_brk(uintptr_t brk);

#endif

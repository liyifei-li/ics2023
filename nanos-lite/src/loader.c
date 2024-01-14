#include <proc.h>
#include <elf.h>
#include <fs.h>
#include <mm.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
# define Elf_Addr Elf64_Addr
# define Elf_Off  Elf64_Off
# define word_t   uint64_t
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
# define Elf_Addr Elf32_Addr
# define Elf_Off  Elf32_Off
#define  word_t   uint32_t
#endif

#if defined(__ISA_AM_NATIVE__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__riscv)
# define EXPECT_TYPE EM_RISCV
#else
# error Unsupported ISA
#endif

uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);

  Elf_Ehdr ehdr;
  fs_lseek(fd, 0, SEEK_SET);
  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));

  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
  assert(ehdr.e_machine == EXPECT_TYPE);

  Elf_Addr phoff = ehdr.e_phoff;
  uint16_t phnum = ehdr.e_phnum;
  Elf_Phdr phdr[phnum];
  fs_lseek(fd, phoff, SEEK_SET);
  fs_read(fd, &phdr, phnum * sizeof(Elf_Phdr));
  for (int i = 0; i < phnum; i++) {
    if (phdr[i].p_type == PT_LOAD) {
      word_t filesz = phdr[i].p_filesz;
      word_t memsz = phdr[i].p_memsz;
      Elf_Off offset = phdr[i].p_offset;
      Elf_Addr vaddr = phdr[i].p_vaddr;
      /*
      fs_lseek(fd, offset, SEEK_SET);
      while ((uint32_t)curpage <= vaddr + filesz) {
        void *newpage = pgalloc_usr(PGSIZE);
        map(&pcb->as, curpage, newpage, 0);
        uint32_t pgoffset = vaddr > (uint32_t)curpage ? vaddr - (uint32_t)curpage : 0;
        uint32_t pagesz = (uint32_t)curpage + PGSIZE > vaddr + memsz ? vaddr + memsz - (uint32_t)curpage : cursize;
        fs_read(fd, (void *)vaddr, );
        curpage += PGSIZE;
      }
      */
      void *curpage = (void *)(vaddr & 0xfffff000);
      while ((uintptr_t)curpage <= vaddr + memsz) {
        void *newpage = new_page(1);
        printf("%p %p %p %p\n", vaddr, curpage, newpage, vaddr + memsz);
        map(&pcb->as, curpage, newpage, 0);
        curpage += PGSIZE;
      }
      fs_lseek(fd, offset, SEEK_SET);
      fs_read(fd, (void *)vaddr, filesz);
      memset((void *)(vaddr + filesz), 0, memsz - filesz);
    }
  }
  void *stack = pcb->as.area.end - 8 * PGSIZE;
  for (int i = 0; i < 8; i++) {
    void *newpage = new_page(1);
    map(&pcb->as, stack, newpage, 0);
    stack += PGSIZE;
  }
  Elf_Addr entry = ehdr.e_entry;
  return entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

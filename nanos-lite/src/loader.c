#include <proc.h>
#include <elf.h>

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

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr ehdr;
  ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
  assert(ehdr.e_machine == EXPECT_TYPE);
  Elf_Addr phoff = ehdr.e_phoff;
  uint16_t phnum = ehdr.e_phnum;
  Elf_Phdr phdr[phnum];
  ramdisk_read(&phdr, phoff, phnum * sizeof(Elf_Phdr));
  for (int i = 0; i < phnum; i++) {
    if (phdr[i].p_type == PT_LOAD) {
      word_t filesz = phdr[i].p_filesz;
      word_t memsz = phdr[i].p_memsz;
      Elf_Off offset = phdr[i].p_offset;
      Elf_Addr vaddr = phdr[i].p_vaddr;
      ramdisk_read((void *)vaddr, offset, filesz);
      memset((void *)(vaddr + filesz), 0, memsz - filesz);
    }
  }
  Elf_Addr entry = ehdr.e_entry;
  return entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}


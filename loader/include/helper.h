#ifndef _HELPER_H
#define _HELPER_H

#define _XOPEN_SOURCE 700

#include <assert.h>
#include <elf.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#define MAX_ARG_STRLEN (PAGE_SIZE * 32)  // https://elixir.bootlin.com/linux/latest/source/include/uapi/linux/binfmts.h#L15
#define ELF_MIN_ALIGN PAGE_SIZE
#define ELF_PAGESTART(_v) ((_v) & ~(int)(ELF_MIN_ALIGN - 1))
#define ELF_PAGEOFFSET(_v) ((_v) & (ELF_MIN_ALIGN - 1))
#define ELF_PAGEALIGN(_v) (((_v) + ELF_MIN_ALIGN - 1) & ~(ELF_MIN_ALIGN - 1))

#define MAX_ELF_INFO 256
#define STACK_SIZE (MAX_ARG_STRLEN * 32)
#define PHDR_TABLE_SIZE 64

struct linux_binprm_lite {
    int argc, envc;
    char** argv;
    char** envp;
    char** auxp;
};

void handle_error(const char* msg);
void* set_brk(unsigned long start, unsigned long end, unsigned long size, int prot);
int make_prot(unsigned int p_flags);
void* elf_map(int fd, unsigned long addr, unsigned long size, Elf64_Phdr* eppnt, int prot, int flags);
int padzero(unsigned long elf_bss);
void stack_check(void* top_of_stack, uint64_t argc, char** argv);

#endif

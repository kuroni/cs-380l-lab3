#include "helper.h"

// Common helper functions
void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// ELF helper functions
void* set_brk(unsigned long start, unsigned long end, unsigned long size, int prot) {
    if (size == -1) {
        start = ELF_PAGEALIGN(start); // this is weird, for apager we use PAGEALIGN for this???
        end = ELF_PAGEALIGN(end);
        size = end - start;
    } else {
        start = ELF_PAGESTART(start);
        end = ELF_PAGEALIGN(end);
        size = ELF_PAGEALIGN(size);
        if (end - start < size) {
            size = end - start;
        }
    }
    // need to place at exactly the start address
    int flags = MAP_PRIVATE | MAP_ANON | MAP_FIXED;
    fprintf(stderr, "bss mapping: size %lu, address %p\n", size, (void*)start);
    return end > start ? mmap((void*)start, size, prot, flags, -1, 0) : NULL;
}

int make_prot(unsigned int p_flags) {
    int prot = 0;
    if (p_flags & PF_R) {
        prot |= PROT_READ;
    }
    if (p_flags & PF_W) {
        prot |= PROT_WRITE;
    }
    if (p_flags & PF_X) {
        prot |= PROT_EXEC;
    }
    return prot;
}

void* elf_map(int fd, unsigned long addr, unsigned long size, Elf64_Phdr* eppnt, int prot, int flags) {
    if (size == -1) {
        // size is unspecified, we need to calculate the size ourselves
        size = eppnt->p_filesz + ELF_PAGEOFFSET(addr);
    }
    unsigned long off = eppnt->p_offset - ELF_PAGEOFFSET(eppnt->p_vaddr);
    addr = ELF_PAGESTART(addr);
    size = ELF_PAGEALIGN(size);
    off += addr - ELF_PAGESTART(eppnt->p_vaddr);
    fprintf(stderr, "file-backed mapping: file %d, offset %lu, size %lu, address %p\n", fd, off, size, (void*)addr);
    return !size ? (void*)addr : mmap((void*)addr, size, prot, flags, fd, off);
}

int padzero(unsigned long elf_bss) {
    unsigned long nbyte;

    nbyte = ELF_PAGEOFFSET(elf_bss);
    if (nbyte) {
        nbyte = ELF_MIN_ALIGN - nbyte;
        memset((void*)elf_bss, 0, nbyte);
    }
    return 0;
}

/**
 * Routine for checking stack made for child program.
 * top_of_stack: stack pointer that will given to child program as %rsp
 * argc: Expected number of arguments
 * argv: Expected argument strings
 */
void stack_check(void* top_of_stack, uint64_t argc, char** argv) {
    fprintf(stderr, "----- stack check -----\n");

    assert(((uint64_t)top_of_stack) % 8 == 0);
    fprintf(stderr, "top of stack is 8-byte aligned\n");

    uint64_t* stack = top_of_stack;
    uint64_t actual_argc = *(stack++);
    fprintf(stderr, "argc: %lu\n", actual_argc);
    assert(actual_argc == argc);

    for (int i = 0; i < argc; i++) {
        char* argp = (char*)*(stack++);
        assert(strcmp(argp, argv[i]) == 0);
        fprintf(stderr, "arg %d: %s\n", i, argp);
    }
    // Argument list ends with null pointer
    assert(*(stack++) == 0);

    int envp_count = 0;
    while (*(stack++) != 0)
        envp_count++;

    fprintf(stderr, "env count: %d\n", envp_count);

    Elf64_auxv_t* auxv_start = (Elf64_auxv_t*)stack;
    Elf64_auxv_t* auxv_null = auxv_start;
    while (auxv_null->a_type != AT_NULL) {
        auxv_null++;
    }
    fprintf(stderr, "aux count: %lu\n", auxv_null - auxv_start);
    fprintf(stderr, "----- end stack check -----\n");
}

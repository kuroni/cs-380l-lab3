#include "helper.h"

void* sp;
int fd, phdr_count;
Elf64_Phdr phdr_table[PHDR_TABLE_SIZE];

int create_elf_tables(struct linux_binprm_lite* bprm, Elf64_Ehdr* exec, unsigned long e_entry) {
    Elf64_auxv_t elf_info[MAX_ELF_INFO];
    int argc = bprm->argc, envc = bprm->envc;
    const char** argv = bprm->argv;
    const char** envp = bprm->envp;

    // almost verbatim copy from https://elixir.bootlin.com/linux/latest/source/fs/binfmt_elf.c#L175
    int ei_index = 0;
    // Access the auxv vectors after envp (https://articles.manugarg.com/aboutelfauxiliaryvectors)
    for (Elf64_auxv_t* auxv = (Elf64_auxv_t*)bprm->auxp; auxv->a_type != AT_NULL; auxv++, ei_index++) {
        elf_info[ei_index] = *auxv;
        switch (auxv->a_type) {
            case AT_PHDR: elf_info[ei_index].a_un.a_val = 0; break; // phdr_addr from load_elf_binary is always 0
            case AT_PHENT: elf_info[ei_index].a_un.a_val = sizeof(Elf64_Phdr); break;
            case AT_PHNUM: elf_info[ei_index].a_un.a_val = exec->e_phnum; break;
            case AT_ENTRY: elf_info[ei_index].a_un.a_val = e_entry; break;
            default: break;
        }
    }

    ei_index += 2;
    void* stack = sp;
    // put argc + argv
    *((unsigned long*)stack) = (unsigned long)argc; stack += sizeof(unsigned long);
    for (int i = 0; i < argc; i++) {
        *((char**)stack) = argv[i]; stack += sizeof(char*);
    }
    *((char**)stack) = (char*)NULL; stack += sizeof(char*);
    // put envp
    for (int i = 0; i < envc; i++) {
        *((char**)stack) = envp[i]; stack += sizeof(char*);
    }
    *((char**)stack) = (char*)NULL; stack += sizeof(char*);
    // put auxv
    memcpy(stack, elf_info, sizeof(Elf64_auxv_t) * ei_index);
    // stack_check(sp, argc, argv);
    return EXIT_SUCCESS;
}

void print_phdr(Elf64_Phdr* phdr) {
    fprintf(stderr, "    p_type   %u\n", phdr->p_type);
    fprintf(stderr, "    p_flags  %u\n", phdr->p_flags);
    fprintf(stderr, "    p_offset %lu\n", phdr->p_offset);
    fprintf(stderr, "    p_vaddr  %lu\n", phdr->p_vaddr);
    fprintf(stderr, "    p_filesz %lu\n", phdr->p_filesz);
    fprintf(stderr, "    p_memsz  %lu\n", phdr->p_memsz);
    fprintf(stderr, "    p_align  %lu\n", phdr->p_align);
}

int load_elf_binary(struct linux_binprm_lite* bprm, int fd, Elf64_Ehdr* elf_ex) {
    phdr_count = elf_ex->e_phnum;
    if (lseek(fd, elf_ex->e_phoff, SEEK_SET) == EXIT_FAILURE) {
        handle_error("lseek");
    }

    unsigned long elf_bss = 0, elf_brk = 0;
    int bss_prot = 0;

    for (int i = 0; i < phdr_count; i++) {
        Elf64_Phdr* elf_ppnt = &phdr_table[i];
        if (read(fd, elf_ppnt, sizeof(Elf64_Phdr)) == EXIT_FAILURE) {
            handle_error("read");
        }
        // fprintf(stderr, "%d:\n", i);
        // print_phdr(elf_ppnt);
        // fprintf(stderr, "\n");

        // almost verbatim copy from https://elixir.bootlin.com/linux/v6.4.14/source/fs/binfmt_elf.c#L1031
        if (elf_ppnt->p_type != PT_LOAD) {
            continue;
        }
        int elf_prot = make_prot(elf_ppnt->p_flags);
        int elf_flags = MAP_PRIVATE | MAP_FIXED | MAP_EXECUTABLE;
        if (elf_map(fd, elf_ppnt->p_vaddr, -1, elf_ppnt, elf_prot, elf_flags) == MAP_FAILED) {
            handle_error("mmap");
        }

        unsigned long k;
        k = elf_ppnt->p_vaddr + elf_ppnt->p_filesz;
        if (k > elf_bss) {
            elf_bss = k;
        }
        k = elf_ppnt->p_vaddr + elf_ppnt->p_memsz;
        if (k > elf_brk) {
            bss_prot = elf_prot;
            elf_brk = k;
        }
    }
    if (elf_bss != elf_brk && padzero(elf_bss)) {
        handle_error("malloc");
    }
    Elf64_Addr elf_entry = elf_ex->e_entry;
    if (create_elf_tables(bprm, elf_ex, elf_entry) < 0) {
        return EXIT_FAILURE;
    }
    // execute
    asm("movq $0, %rax");
    asm("movq $0, %rbx");
    asm("movq $0, %rcx");
    asm("movq $0, %rdx");
    asm("movq %0, %%rsp" : : "r" (sp));
    asm("jmp *%0" : : "c" (elf_entry));
    return EXIT_FAILURE;
}

void init_stack(int argc, char** argv, int envc, char** envp, struct linux_binprm_lite* bprm) {
    // we store a copy of argv's contents and envp's contents at the end of stack_content
    // and make room argc, *argv, **envp at the beginning
    int prot = PROT_READ | PROT_WRITE;
    int flag = MAP_ANON | MAP_GROWSDOWN | MAP_PRIVATE;
    if ((sp = mmap(NULL, STACK_SIZE, prot, flag, -1, 0)) == MAP_FAILED) {
        handle_error("mmap");
    }
    memset(sp, 0, STACK_SIZE);
    void* end_of_stack = sp + STACK_SIZE;
    // fill in the content of bprm
    bprm->argc = argc - 1; bprm->envc = envc; bprm->auxp = envp + envc + 1;
    if ((bprm->argv = malloc((argc - 1) * sizeof(char*))) == NULL) {
        handle_error("malloc");
    }
    if ((bprm->envp = malloc(envc * sizeof(char*))) == NULL) {
        handle_error("malloc");
    }
    for (int i = envc - 1; i >= 0; i--) {
        size_t len = strlen(envp[i]) + 1;
        end_of_stack -= len;
        bprm->envp[i] = end_of_stack;
        memcpy(end_of_stack, envp[i], len);
        // fprintf(stderr, "envp[%d]: %s\n", i, (char*)end_of_stack);
    }
    for (int i = argc - 1; i > 0; i--) {
        size_t len = strlen(argv[i]) + 1;
        end_of_stack -= len;
        bprm->argv[i - 1] = end_of_stack;
        memcpy(end_of_stack, argv[i], len);
        // fprintf(stderr, "argv[%d]: %s\n", i - 1, (char*)end_of_stack);
    }
}

void handler(int sig, siginfo_t* info, void* context) {
    unsigned long addr = (unsigned long)info->si_addr;
    int pt = -1;
    for (int i = 0; i < phdr_count; i++) {
        Elf64_Phdr* elf_ppnt = &phdr_table[i];
        if (elf_ppnt->p_type != PT_LOAD) {
            continue;
        }
        if (elf_ppnt->p_vaddr <= addr && addr <= elf_ppnt->p_vaddr + elf_ppnt->p_memsz) {
            pt = i;
            break;
        }
    }
    if (pt == -1) {
        exit(-EXIT_FAILURE);
    } else {
        Elf64_Phdr* elf_ppnt = &phdr_table[pt];
        unsigned long vaddr = elf_ppnt->p_paddr;
        unsigned long elf_bss = elf_ppnt->p_vaddr + elf_ppnt->p_filesz;
        unsigned long elf_brk = elf_ppnt->p_vaddr + elf_ppnt->p_memsz;
        int elf_prot = make_prot(elf_ppnt->p_flags);
        // first case in dpager is not a thing anymore
        // eager allocation, let's do 2 at the same time!
        if (set_brk(addr, elf_brk, 2 * PAGE_SIZE, elf_prot) == MAP_FAILED) {
            exit(-EXIT_FAILURE);
        }
    }
}

int main(int argc, char** argv, char** envp) {
    if (argc < 2) {
        printf("USAGE: ./hpager <filename> <args>");
        return EXIT_FAILURE;
    }
    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        handle_error("open");
    }
    Elf64_Ehdr elf_ex;
    if (read(fd, &elf_ex, sizeof(Elf64_Ehdr)) == EXIT_FAILURE) {
        handle_error("read");
    }
    // initialize envc, this will be helpful later
    int envc = 0;
    for (char** _ = envp; *_ != NULL; _++) {
        envc++;
    }
    // initialize sigsegv handler
    struct sigaction action;
    action.sa_sigaction = handler;
    action.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(SIGSEGV, &action, NULL);
    struct linux_binprm_lite bprm;
    init_stack(argc, argv, envc, envp, &bprm);
    load_elf_binary(&bprm, fd, &elf_ex);
    close(fd);
}

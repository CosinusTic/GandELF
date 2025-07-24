#include "include/parse_elf.h"
#include "include/pretty_print.h"

#include <elf.h>
#include <stdio.h>

// ELF header
static void print_target_sys(Elf64_Ehdr *hdr);
static void print_arch(Elf64_Ehdr *hdr);
static void print_ftype(Elf64_Ehdr *hdr);

// Program headers
static void print_seg_flag(Elf64_Phdr *hdr);
static void print_seg_type(Elf64_Phdr *hdr);
static void print_Phdr(Elf64_Phdr *hdr);

// Section headers

void print_Ehdr(Elf64_Ehdr *hdr)
{
    puts("---------- ELF headers ----------");
    print_arch(hdr);
    print_target_sys(hdr);
    print_ftype(hdr);
    putchar('\n');
}

void print_Shdrs(void *buf, Elf64_Ehdr *ehdr)
{
    if (ehdr->e_shnum <= 0)
        return;

    puts("-------- Section headers --------");

    Elf64_Half i = 0;
    Elf64_Shdr *start = get_shdrs(buf, ehdr);
    printf("%d entries\n", ehdr->e_shnum);
    while (i < ehdr->e_shnum)
    {
        printf("Section %d\n", i);
        printf("At memory:\t0x%08lx\n", start->sh_addr);
        i++;
    }
}

void print_Phdrs(void *buf, Elf64_Ehdr *ehdr)
{
    if (ehdr->e_phnum <= 0)
        return;

    puts("-------- Program headers --------");

    Elf64_Half i = 0;
    Elf64_Phdr *start = get_phdrs(buf, ehdr);
    while (i < ehdr->e_phnum)
    {
        Elf64_Phdr *current = &start[i]; // Syntactic sugar
        printf("%d\n", i + 1);
        print_Phdr(current);
        puts("--");
        i++;
    }

    putchar('\n');
}

static void print_Phdr(Elf64_Phdr *hdr)
{
    print_seg_flag(hdr);
    print_seg_type(hdr);
    printf("\tSegment physical address:\t0x%08lx\n", hdr->p_paddr);
    printf("\tSegment virtual address:\t0x%08lx\n", hdr->p_vaddr);
    printf("\tSegment disk size:\t\t%lu\n", hdr->p_filesz);
    printf("\tSegment memory size:\t\t%lu\n", hdr->p_memsz);
}

static void print_seg_type(Elf64_Phdr *hdr)
{
    printf("\tSegment type:\t\t\t");
    switch (hdr->p_type)
    {
    case PT_NULL:
        printf("Unused");
        break;
    case PT_LOAD:
        printf("Loadable");
        break;
    case PT_DYNAMIC:
        printf("Dynamic linking");
        break;
    case PT_INTERP:
        printf("Program interpreter");
        break;
    case PT_GNU_STACK:
        printf("Exception handler");
        break;
    default:
        printf("Other");
        break;
    }

    putchar('\n');
}

static void print_seg_flag(Elf64_Phdr *hdr)
{
    printf("\tSegment flag:\t\t\t");
    switch (hdr->p_flags)
    {
    case PF_X:
        printf("Executable segment");
        break;
    case PF_W:
        printf("Writeable segment");
        break;
    case PF_R:
        printf("Readable segment");
        break;
    default:
        printf("Unknown");
        break;
    }

    putchar('\n');
}

static void print_target_sys(Elf64_Ehdr *hdr)
{
    printf("Target system:\t");
    switch (hdr->e_ident[EI_OSABI])
    {
    case ELFOSABI_NETBSD:
        printf("Net BSD");
        break;
    case ELFOSABI_FREEBSD:
        printf("Free BSD");
        break;
    case ELFOSABI_LINUX:
        printf("Linux");
        break;
    case ELFOSABI_SYSV:
        printf("Unix System V");
        break;
    case ELFOSABI_SOLARIS:
        printf("Solaris");
        break;
    case ELFOSABI_ARM:
        printf("ARM architecture");
        break;
    case ELFOSABI_STANDALONE:
        printf("Standalone executable (embedded)");
        break;
    default:
        printf("?");
        break;
    }
    putchar('\n');
}

static void print_arch(Elf64_Ehdr *hdr)
{
    printf("Architecture:\t");
    switch (hdr->e_ident[EI_CLASS])
    {
    case ELFCLASS64:
        printf("x86_64");
        break;
    case ELFCLASS32:
        printf("x86 (32 bits)");
        break;
    default:
        printf("?");
    }

    putchar('\n');
}

static void print_ftype(Elf64_Ehdr *hdr)
{
    printf("Type:\t\t");
    switch (hdr->e_type)
    {
    case ET_REL:
        printf("Relocatable file");
        break;
    case ET_EXEC:
        printf("Executable file");
        break;
    case ET_DYN:
        printf("Shared object");
        break;
    case ET_CORE:
        printf("Core file");
        break;
    case ET_NONE:
    default:
        printf("?");
        break;
    }

    putchar('\n');
}

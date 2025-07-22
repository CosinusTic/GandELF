#include "include/parse_elf.h"
#include "include/pretty_print.h"

#include <elf.h>
#include <stdio.h>

// ELF header
static void print_target_sys(Elf64_Ehdr *hdr);
static void print_arch(Elf64_Ehdr *hdr);
static void print_ftype(Elf64_Ehdr *hdr);

// Program headers
static void print_segment_flag(Elf64_Phdr *hdr);
static void print_Phdr(Elf64_Phdr *hdr);

void print_Ehdr(Elf64_Ehdr *hdr)
{
    puts("---------- ELF headers ----------");
    print_arch(hdr);
    print_target_sys(hdr);
    print_ftype(hdr);
}

void print_Phdrs(void *buf, Elf64_Ehdr *ehdr)
{
    if (ehdr->e_phnum <= 0)
        return;
    puts("-------- Program headers --------");
    Elf64_Half i = 0;
    Elf64_Phdr *start = get_phdr(buf, ehdr->e_phoff);
    while (i < ehdr->e_phnum)
    {
        Elf64_Phdr *current = &start[i]; // Syntactic sugar
        printf("%d\n", i + 1);
        print_Phdr(current);
        i++;
    }
}

static void print_Phdr(Elf64_Phdr *hdr)
{
    print_segment_flag(hdr);
}

static void print_segment_flag(Elf64_Phdr *hdr)
{
    printf("\tSegment flag:\t");
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
    printf("\n");
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
    printf("\n");
}

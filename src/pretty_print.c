#include "include/utils.h"

#include <elf.h>
#include <stdio.h>

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
    printf("\n");
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

void print_headers(const struct file *f)
{
    Elf64_Ehdr *hdr = (Elf64_Ehdr *)f->content;
    print_arch(hdr);
    print_target_sys(hdr);
}

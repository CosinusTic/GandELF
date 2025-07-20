#include "include/utils.h"

#include <elf.h>
#include <stdio.h>

void print_headers(const struct file *f)
{
    Elf64_Ehdr *fheaders = (Elf64_Ehdr *)f->content;

    printf("Architecture:\t");
    if (fheaders->e_ident[EI_CLASS] == ELFCLASS64)
        printf("x86_64");
    else if (fheaders->e_ident[EI_CLASS] == ELFCLASS32)
        printf("x86 (32 bits)");
    else
        printf("Unknown");
    printf("\n");
}

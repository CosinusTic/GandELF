#include "include/parse_elf.h"
#include "include/utils.h"
#include "include/pretty_print.h"

#include <stdio.h>

#define TARGET "dizzy"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("[-] Usage: ./%s <target_program>\n", TARGET);
        return 1;
    }

    char *target_bin = argv[1];
    struct file *f = file_map(target_bin);
    if (!f)
    {
        puts("[-] Failed to parse file argument");
        return 1;
    }

    if (!is_elf(f))
    {
        puts("[-] File is not of ELF format, can't parse");
        file_unmap(&f);
        return 1;
    }

    Elf64_Ehdr *ehdr = get_ehdr(f->content);
    Elf64_Phdr *phdr = get_phdr(f->content, ehdr->e_phoff);

    print_Ehdr(ehdr);
    print_Phdrs(f->content, ehdr);

    printf("Program header start address: %p\n", (void *)phdr);

    file_unmap(&f);

    return 0;
}

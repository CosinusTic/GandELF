#include "include/parse_elf.h"
#include "include/utils.h"
#include "include/pretty_print.h"
#include "include/dump.h"

#include <stdio.h>
#include <stdlib.h>

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
    Elf64_Phdr *phdrs = get_phdrs(f->content, ehdr);
    Elf64_Shdr *shdrs = get_shdrs(f->content, ehdr);

    if (shdrs)
        printf("Section headers parsed\n");
    if (phdrs)
        printf("Program headers parsed\n");

    print_Ehdr(ehdr);
    print_Phdrs(f->content, ehdr);
    print_Shdrs(f->content, ehdr);

    struct impsec *impsec = get_impsec(f->content, ehdr);

    unsigned char *textsec_ptr =
        (unsigned char *)f->content + impsec->text->sh_offset;
    size_t textsec_size = impsec->text->sh_size;

    printf(".text\t%p\t%zu bytes\n", (void *)textsec_ptr, textsec_size);
    hexdump(textsec_ptr, textsec_size);

    free(impsec);
    file_unmap(&f);
    return 0;
}

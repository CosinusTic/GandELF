#include "include/parse_elf.h"
#include "include/utils.h"
#include "include/pretty_print.h"

#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <stdio.h>
#include <string.h>

#define TARGET "gandelf"

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
    if (!impsec)
    {
        puts("[-] Failed to extract important sections");
        file_unmap(&f);
        return 1;
    }
    size_t text_index = impsec->text - shdrs; // Calculate .text's index
    struct sec *text_sec = NULL;

    if (!impsec->text)
    {
        puts("[-] No .text section found");
        free(impsec);
        file_unmap(&f);
        return 1;
    }

    text_sec = sec_resolve(f, impsec->text);

    printf(".text\t%p\t%zu bytes\n", text_sec->addr, text_sec->size);
    hexdump(text_sec->addr, text_sec->size);

    struct sym_list lst =
        get_text_funcs(f->content, impsec, text_index, f->size);
    print_text_funcs(&lst);

    for (size_t i = 0; i < lst.count; i++)
        free(lst.items[i].name);
    free(lst.items);

    free(impsec);
    free(text_sec);
    file_unmap(&f);
    return 0;
}

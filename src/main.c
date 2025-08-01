#include "include/parse_elf.h"
#include "include/utils.h"
#include "include/pretty_print.h"

#include <stdio.h>
#include <stdlib.h>

#define TARGET "dizzy"

#include <elf.h>
#include <stdio.h>

void print_symbols_in_text(void *buf, struct impsec *impsec, size_t text_index)
{
    if (!impsec->symtab || !impsec->strtab || !impsec->text)
    {
        puts("[-] Required sections (.symtab, .strtab, .text) missing");
        return;
    }

    Elf64_Sym *symtab = (Elf64_Sym *)((char *)buf + impsec->symtab->sh_offset);
    size_t sym_count = impsec->symtab->sh_size / sizeof(Elf64_Sym);

    const char *strtab = (const char *)buf + impsec->strtab->sh_offset;

    printf("[*] Listing function symbols in .text:\n");

    for (size_t i = 0; i < sym_count; i++)
    {
        Elf64_Sym *sym = &symtab[i];

        if (sym->st_name == 0)
            continue; // unnamed

        if (ELF64_ST_TYPE(sym->st_info) != STT_FUNC)
            continue; // not a function

        if (sym->st_shndx != text_index)
            continue; // not in .text

        const char *name = strtab + sym->st_name;

        printf(" - %s:\n", name);
        printf("\taddress: 0x%lx\n", sym->st_value);
        printf("\tsize:    %lu bytes\n", sym->st_size);
        printf("\tentsize: %lu bytes\n",
               sym->st_size); // entsize often same as size
    }
}

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

    print_symbols_in_text(f->content, impsec, text_index);
    // unsigned char *textsec_ptr =
    //     (unsigned char *)f->content + impsec->text->sh_offset;
    // size_t textsec_size = impsec->text->sh_size;

    // printf(".text\t%p\t%zu bytes\n", (void *)textsec_ptr, textsec_size);
    // hexdump(textsec_ptr, textsec_size);

    free(impsec);
    free(text_sec);
    file_unmap(&f);
    return 0;
}

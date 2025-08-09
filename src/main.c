#include "include/parse_elf.h"
#include "include/utils.h"
#include "include/pretty_print.h"

#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <stdio.h>
#include <string.h>

#define TARGET "gandelf"

static char *xstrdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p)
        memcpy(p, s, n);
    return p;
}

struct sym_info
{
    char *name;
    Elf64_Addr addr;
    size_t size;
    unsigned char *bytes;
};

struct sym_list
{
    struct sym_info *items;
    size_t count;
};

struct sym_list get_text_funcs(void *buf, struct impsec *impsec,
                               size_t text_index, size_t file_size)
{
    struct sym_list out = { 0 };
    if (!impsec || !impsec->strtab || !impsec->symtab || !impsec->text)
        return out;

    Elf64_Sym *symtab = (Elf64_Sym *)((char *)buf + impsec->symtab->sh_offset);
    size_t sym_count = impsec->symtab->sh_size / sizeof(Elf64_Sym);
    const char *strtab = (const char *)buf + impsec->strtab->sh_offset;
    Elf64_Shdr *text = impsec->text;

    size_t fun_count = 0;
    for (size_t i = 0; i < sym_count; i++)
    {
        Elf64_Sym *s = &symtab[i];
        if (s->st_name && ELF64_ST_TYPE(s->st_info) == STT_FUNC
            && s->st_shndx == text_index)
            fun_count++;
    }
    if (fun_count == 0)
        return out;

    struct sym_info *funcs = calloc(fun_count, sizeof(*funcs));
    if (!funcs)
        return out;

    size_t j = 0;
    for (size_t i = 0; i < sym_count; i++)
    {
        Elf64_Sym *s = &symtab[i];
        if (!(s->st_name && ELF64_ST_TYPE(s->st_info) == STT_FUNC
              && s->st_shndx == text_index))
            continue;

        size_t sym_off = (size_t)(s->st_value - text->sh_addr);
        size_t off = (size_t)text->sh_offset + sym_off;

        // bounds check
        if (off > file_size || (s->st_size && off + s->st_size > file_size))
            continue;

        funcs[j].name = xstrdup(strtab + s->st_name);
        if (!funcs[j].name)
        { /* handle OOM if you want */
        }
        funcs[j].addr = s->st_value;
        funcs[j].size = s->st_size;
        funcs[j].bytes = (unsigned char *)buf + off;
        j++;
    }

    out.items = funcs;
    out.count = j; // may be <= fun_count if some failed bounds
    return out;
}

void print_text_funcs(const struct sym_list *lst)
{
    puts(".text functions:");
    for (size_t i = 0; i < lst->count; i++)
    {
        const struct sym_info *f = &lst->items[i];
        printf("\n\tAddr:\t0x%016llx\n\tname:\t%s\n\tsize:\t%zu\n",
               (unsigned long long)f->addr, f->name ? f->name : "(null)",
               f->size);
        // hexdump(unsigned char *ptr, size_t size);
        hexdump(f->bytes, f->size);
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

#include "include/utils.h"
#include "include/parse_elf.h"

#include <elf.h>
#include <stdlib.h>
#include <string.h>

int is_elf(const struct file *f)
{
    Elf64_Ehdr *hdr = (Elf64_Ehdr *)f->content;
    return (hdr->e_ident[EI_MAG0] == 0x7f && hdr->e_ident[EI_MAG1] == 'E'
            && hdr->e_ident[EI_MAG2] == 'L' && hdr->e_ident[EI_MAG3] == 'F')
        ? 1
        : 0;
}

Elf64_Ehdr *get_ehdr(void *buf)
{
    return (Elf64_Ehdr *)buf;
}

Elf64_Phdr *get_phdrs(void *buf, Elf64_Ehdr *hdr)
{
    void *phdr = (char *)buf + hdr->e_phoff;

    return (Elf64_Phdr *)phdr;
}

Elf64_Shdr *get_shdrs(void *buf, Elf64_Ehdr *hdr)
{
    void *shdr = (char *)buf + hdr->e_shoff;

    return (Elf64_Shdr *)shdr;
}

// Get the .text, .symtab, .strtab sections
struct impsec *get_impsec(void *buf, Elf64_Ehdr *ehdr)
{
    if (ehdr->e_shnum <= 0)
        return NULL;

    struct impsec *sec = malloc(sizeof(struct impsec));
    if (!sec)
        return NULL;

    Elf64_Half i = 0;
    Elf64_Shdr *sh_start = get_shdrs(buf, ehdr);
    char *shstrtab = (char *)buf + sh_start[ehdr->e_shstrndx].sh_offset;
    while (i < ehdr->e_shnum)
    {
        void *ptr = (char *)sh_start + i * ehdr->e_shentsize;
        Elf64_Shdr *sh_cur = (Elf64_Shdr *)ptr;

        const char *section_name = shstrtab + sh_cur->sh_name;

        if (sh_cur->sh_type == SHT_PROGBITS
            && (sh_cur->sh_flags & SHF_EXECINSTR)
            && strcmp(section_name, ".text") == 0)
            sec->text = sh_cur;

        if (sh_cur->sh_type == SHT_SYMTAB)
        {
            sec->symtab = sh_cur;

            if (sh_cur->sh_link < ehdr->e_shnum)
                sec->strtab = &sh_start[sh_cur->sh_link];
        }
        i++;
    }

    return sec;
}

struct sec *sec_resolve(struct file *f, Elf64_Shdr *shdr)
{
    if (!shdr)
        return NULL;

    struct sec *sec = malloc(sizeof(struct sec));
    if (!sec)
        return NULL;

    sec->addr = (void *)((char *)f->content + shdr->sh_offset);
    sec->size = shdr->sh_size;
    sec->entsize = shdr->sh_entsize;

    return sec;
}

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

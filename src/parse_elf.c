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
            sec->symtab = sh_cur;

        if (sh_cur->sh_type == SHT_STRTAB)
            sec->strtab = sh_cur;

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

    sec->addr = (void *)f->content + shdr->sh_offset;
    sec->size = shdr->sh_size;

    return sec;
}

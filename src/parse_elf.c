#include "include/utils.h"
#include "include/parse_elf.h"

#include <elf.h>
#include <stdlib.h>

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

// Get the important sections (.text, .symtab, .strtab)
struct impsec *get_impsec(void *buf, Elf64_Ehdr *ehdr)
{
    if (ehdr->e_shnum <= 0)
        return NULL;

    struct impsec *sec = malloc(sizeof(struct impsec));
    if (!sec)
        return NULL;

    Elf64_Half i = 0;
    Elf64_Shdr *sh_start = get_shdrs(buf, ehdr);
    while (i < ehdr->e_shnum)
    {
        void *ptr = (char *)sh_start + i * ehdr->e_shentsize;
        Elf64_Shdr *sh_cur = (Elf64_Shdr *)ptr;
        switch (sh_cur->sh_type)
        {
        case SHT_SYMTAB:
            sec->symtab = sh_cur;
            break;
        case SHT_PROGBITS:
            sec->text = sh_cur;
            break;
        case SHT_STRTAB:
            sec->strtab = sh_cur;
            break;
        default:
            break;
        }

        i++;
    }

    return sec;
}

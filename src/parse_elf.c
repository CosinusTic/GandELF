#include "include/utils.h"

#include <elf.h>

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

Elf64_Phdr *get_phdr(void *buf, Elf64_Off offset)
{
    void *phdr = (char *)buf + offset;

    return (Elf64_Phdr *)phdr;
}

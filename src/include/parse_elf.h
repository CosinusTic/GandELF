#ifndef PARSE_ELF_H
#define PARSE_ELF_H

#include "utils.h"

#include <elf.h>

struct impsec
{
    Elf64_Shdr *symtab;
    Elf64_Shdr *strtab;
    Elf64_Shdr *text;
};

struct sec // A custom struct representing a given section
{
    void *addr;
    size_t size;
};

int is_elf(const struct file *f);
Elf64_Ehdr *get_ehdr(void *buf);
Elf64_Phdr *get_phdrs(void *buf, Elf64_Ehdr *hdr); // Pointer to first entry
Elf64_Shdr *get_shdrs(void *buf, Elf64_Ehdr *hdr); // Pointer to first entry
struct impsec *get_impsec(void *buf,
                          Elf64_Ehdr *ehdr); // Important section headers
struct sec *sec_resolve(struct file *f,
                        Elf64_Shdr *shdr); // Get the section headers's section

#endif /* !PARSE_ELF_H */

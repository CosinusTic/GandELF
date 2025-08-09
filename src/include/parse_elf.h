#ifndef PARSE_ELF_H
#define PARSE_ELF_H

#include "utils.h"

#include <elf.h>

struct impsec // Important sections (.text, .strtab, .symtab)
{
    Elf64_Shdr *symtab;
    Elf64_Shdr *strtab;
    Elf64_Shdr *text;
};

struct sec // Abstraction for a section
{
    void *addr;
    size_t size;
    size_t entsize;
    const char *secname;
};

struct sym_info // Parsed symbol
{
    char *name;
    Elf64_Addr addr;
    size_t size;
    unsigned char *bytes;
};

struct sym_list // List of symbols
{
    struct sym_info *items;
    size_t count;
};

int is_elf(const struct file *f);
Elf64_Ehdr *get_ehdr(void *buf);
Elf64_Phdr *get_phdrs(void *buf, Elf64_Ehdr *hdr); // Pointer to first entry
Elf64_Shdr *get_shdrs(void *buf, Elf64_Ehdr *hdr); // Pointer to first entry
struct impsec *get_impsec(void *buf,
                          Elf64_Ehdr *ehdr); // Important section headers
struct sec *sec_resolve(struct file *f,
                        Elf64_Shdr *shdr); // Get the section headers's section

struct sym_list get_text_funcs(
    void *buf, struct impsec *impsec, size_t text_index,
    size_t file_size); // Get the .text section's function type symbols
#endif /* !PARSE_ELF_H */

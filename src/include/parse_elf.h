#ifndef PARSE_ELF_H
#define PARSE_ELF_H

#include "utils.h"

#include <elf.h>

int is_elf(const struct file *f);
Elf64_Ehdr *get_ehdr(void *buf);
Elf64_Phdr *get_phdrs(void *buf, Elf64_Ehdr *hdr); // Pointer to first entry
Elf64_Shdr *get_shdrs(void *buf, Elf64_Ehdr *hdr); // Pointer to first entry

#endif /* !PARSE_ELF_H */

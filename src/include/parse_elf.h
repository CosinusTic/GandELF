#ifndef PARSE_ELF_H
#define PARSE_ELF_H

#include "utils.h"

#include <elf.h>

int is_elf(const struct file *f);
Elf64_Ehdr *get_ehdr(void *buf);
Elf64_Phdr *get_phdr(void *buf, Elf64_Off offset);

#endif /* !PARSE_ELF_H */

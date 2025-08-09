#ifndef PRETTY_PRINT_H
#define PRETTY_PRINT_H

#include "utils.h"
#include "parse_elf.h"

#include <elf.h>

void print_Ehdr(Elf64_Ehdr *hdr);
void print_Phdrs(void *buf, Elf64_Ehdr *ehdr);
void print_Shdrs(void *buf, Elf64_Ehdr *ehdr);
void hexdump(unsigned char *ptr, size_t size);
void print_text_funcs(const struct sym_list *lst);

#endif /* !PRETTY_PRINT_H */

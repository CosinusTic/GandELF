#ifndef PRETTY_PRINT_H
#define PRETTY_PRINT_H

#include "utils.h"

#include <elf.h>

void print_Ehdr(Elf64_Ehdr *hdr);
void print_Phdrs(void *buf, Elf64_Ehdr *ehdr);
void print_Shdrs(void *buf, Elf64_Ehdr *ehdr);
void hexdump(unsigned char *ptr, size_t size);

#endif /* !PRETTY_PRINT_H */

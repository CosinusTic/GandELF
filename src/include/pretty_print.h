#ifndef PRETTY_PRINT_H
#define PRETTY_PRINT_H

#include "utils.h"
#include "parse_elf.h"
#include <elf.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

void print_Ehdr(Elf64_Ehdr *hdr);
void print_Phdrs(void *buf, Elf64_Ehdr *ehdr);
void print_Shdrs(void *buf, Elf64_Ehdr *ehdr);
void hexdump(unsigned char *ptr, size_t size);
void print_text_funcs(const struct sym_list *lst);

#endif /* !PRETTY_PRINT_H */

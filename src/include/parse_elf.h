#ifndef PARSE_ELF_H
#define PARSE_ELF_H

#include "utils.h"

#include <elf.h>

int is_elf(const struct file *f);

#endif /* !PARSE_ELF_H */

#include "include/utils.h"
#include <elf.h>

int is_elf(const struct file *f)
{
    Elf64_Ehdr *fheaders = (Elf64_Ehdr *)f->content;
    return (fheaders->e_ident[EI_MAG0] == 0x7f
            && fheaders->e_ident[EI_MAG1] == 'E'
            && fheaders->e_ident[EI_MAG2] == 'L'
            && fheaders->e_ident[EI_MAG3] == 'F')
        ? 1
        : 0;
}

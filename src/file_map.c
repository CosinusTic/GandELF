#include "include/file_map.h"

#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static int is_elf(const struct file *f)
{
    Elf64_Ehdr *fheaders = (Elf64_Ehdr *)f->content;
    return (fheaders->e_ident[EI_MAG0] == 0x7f
            && fheaders->e_ident[EI_MAG1] == 'E'
            && fheaders->e_ident[EI_MAG2] == 'L'
            && fheaders->e_ident[EI_MAG3] == 'F')
        ? 1
        : 0;
}

struct file *file_map(const char *filename)
{
    struct stat s = { 0 };
    struct file *new = malloc(sizeof(struct file));

    if (new == NULL)
    {
        perror("Cannot malloc a struct file");
        goto error_malloc;
    }

    if ((new->name = malloc(strlen(filename) + 1)) == NULL)
    {
        perror("Cannot malloc a string of strlen(filename)");
        goto error_name;
    }
    new->name = strcpy(new->name, filename);

    if ((new->fd = open(new->name, O_RDONLY)) == -1)
    {
        perror("Cannot open(filename) in read-only");
        goto error_open;
    }

    if ((stat(new->name, &s)) == -1)
    {
        perror("Cannot stat(filename)");
        goto error_stat;
    }
    new->size = s.st_size;

    if ((new->content =
             mmap(NULL, new->size, PROT_READ, MAP_PRIVATE, new->fd, 0))
        == MAP_FAILED)
    {
        perror("Cannot mmap for the file size");
        goto error_mmap;
    }

    if (!is_elf(new))
        goto error_format;

    return new;

error_format:
    puts("[-] Binary is not an ELF");
error_mmap:
error_stat:
    close(new->fd);
error_open:
    free(new->name);
error_name:
    free(new);
error_malloc:
    return NULL;
}

void file_unmap(struct file **f)
{
    free((*f)->name);
    close((*f)->fd);
    munmap((*f)->content, (*f)->size);
    free(*f);
    *f = NULL;
}

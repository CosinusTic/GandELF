#ifndef FILE_MAP_H
#define FILE_MAP_H

#include <stddef.h>

struct file
{
    int fd;
    size_t size;
    char *name;
    void *content;
};

struct file *file_map(const char *filename);
void file_unmap(struct file **file);
char *xstrdup(const char *s);
struct sym_list;
void free_symlist(struct sym_list l);

#endif

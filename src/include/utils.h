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

#endif

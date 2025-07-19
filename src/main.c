#include "include/file_map.h"

#include <stdio.h>

#define TARGET "dizzy"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("[-] Usage: ./%s <target_program>\n", TARGET);
        return -1;
    }

    char *target_bin = argv[1];
    struct file *f = file_map(target_bin);
    if (!f)
    {
        puts("[-] Failed to parse file argument");
        return -1;
    }

    printf("[+] Ready to start working on %s!\n", target_bin);
    printf("[+] Info on %s:\n\t=> Size: %lu bytes\n\t=> fd: %d\n", target_bin,
           f->size, f->fd);

    file_unmap(&f);

    return 0;
}

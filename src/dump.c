#include "include/dump.h"

#include <elf.h>
#include <stdio.h>

void hexdump(unsigned char *ptr, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        printf("0x%02x", ptr[i]);
        if (i % 10 == 0)
            putchar('\n');
        else
            putchar('\t');
    }
    putchar('\n');
}

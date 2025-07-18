#include <stdio.h>

#define TARGET "dizzy"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("[-] Usage: ./%s <target_program>\n", TARGET);
        return -1;
    }

    char *child_proc = argv[1];

    printf("[+] Ready to start working on %s!\n", child_proc);

    return 0;
}

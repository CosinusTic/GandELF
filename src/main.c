#include "include/parse_elf.h"
#include "include/utils.h"
#include "include/pretty_print.h"
#include "include/disas.h"

#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <stdio.h>
#include <string.h>

#define TARGET "gandelf"

// Program arguments
#define ARGS_MIN 3
#define ARGS_MAX 7
#define DISAS 'd' // -d disas .text (+option to select symbol in .text)
#define F_INFO 'f' // -f print file info
#define F_HEADERS 'h' // -h print headers
#define HEXDUMP                                                                \
    'x' // -x hexdump of .text(+option to select a given symbol in .text)

// Check if given string is a program argument (distinguish from argument
// option)
static int is_arg(const char *arg)
{
    return arg && arg[0] == '-' && arg[1] && !arg[2]
        && (arg[1] == DISAS || arg[1] == F_INFO || arg[1] == F_HEADERS
            || arg[1] == HEXDUMP);
}

int main(int argc, char **argv)
{
    // Ensure correct usage
    if (argc < ARGS_MIN || argc > ARGS_MAX)
    {
        fprintf(
            stderr,
            "[-] Usage: ./%s target_program [options...]\nOptions=-d(+optional "
            "symbol), -f, -h, -x(+optional section)\n",
            TARGET);
        return 1;
    }

    // Load file & assert ELF
    char *target_bin = argv[1];
    struct file *f = file_map(target_bin);
    if (!f)
    {
        fprintf(stderr,
                "[-] Failed to parse file argument (file does not exist)\n");
        return 1;
    }
    if (!is_elf(f))
    {
        fprintf(stderr, "[-] File is not of ELF format, can't parse\n");
        file_unmap(&f);
        return 1;
    }

    // Collect file info
    Elf64_Ehdr *ehdr = get_ehdr(f->content);
    Elf64_Phdr *phdrs = get_phdrs(f->content, ehdr);
    Elf64_Shdr *shdrs = get_shdrs(f->content, ehdr);

    if (!ehdr || !phdrs || !shdrs)
    {
        fprintf(stderr, "[-] Missing required headers for ELF parsing\n");
        file_unmap(&f);
        return 1;
    }

    // Collect important ELF sections
    struct impsec *impsec = get_impsec(f->content, ehdr);
    if (!impsec)
    {
        fprintf(stderr, "[-] Failed to extract important sections\n");
        file_unmap(&f);
        return 1;
    }

    // Resolve & map text section into a wrapper
    size_t text_index = impsec->text - shdrs; // Calculate .text's index
    struct sec *text_sec = NULL;

    if (!impsec->text)
    {
        puts("[-] No .text section found");
        free(impsec);
        file_unmap(&f);
        return 1;
    }

    text_sec = sec_resolve(f, impsec->text);
    struct sym_list lst =
        get_text_funcs(f->content, impsec, text_index, f->size);
    if (!text_sec)
    {
        fprintf(stderr, "[-] Failed to retrieve text section or its symbols\n");
    }

    // Handle args & run program
    int i = 2;
    while (i < argc)
    {
        if (is_arg(argv[i]))
        {
            char opt = (char)argv[i][1];
            switch (opt)
            {
            case DISAS:
                if (i + 1 < argc && !is_arg(argv[i + 1]))
                {
                    for (size_t j = 0; j < lst.count; j++)
                    {
                        struct sym_info *sym_info = &(lst.items[j]);
                        if (strcmp(sym_info->name, argv[i + 1])
                            == 0) // .text symbol matched
                        {
                            printf("x86 disassembly of symbol %s\n",
                                   sym_info->name);
                            disas((uint8_t *)sym_info->bytes, sym_info->size,
                                  (uint64_t)sym_info->addr);
                            break;
                        }
                    }
                    i++;
                }
                else
                {
                    for (size_t j = 0; j < lst.count; j++)
                    {
                        struct sym_info *sym_info = &lst.items[j];
                        printf("x86 disassembly of symbol %s\n",
                               sym_info->name);
                        disas((uint8_t *)sym_info->bytes, sym_info->size,
                              (uint64_t)sym_info->addr);
                    }
                }
                break;
            case F_INFO:
                print_Ehdr(ehdr);
                break;
            case F_HEADERS:
                print_Phdrs(f->content, ehdr);
                print_Shdrs(f->content, ehdr);
                break;
            case HEXDUMP:
                if (i + 1 < argc && !is_arg(argv[i + 1]))
                {
                    for (size_t j = 0; j < lst.count; j++)
                    {
                        struct sym_info *sym_info = &lst.items[j];
                        if (strcmp(sym_info->name, argv[i + 1])
                            == 0) // .text symbol matched
                        {
                            printf("Hex dump of symbol %s:\n", argv[i + 1]);
                            hexdump(sym_info->bytes, sym_info->size);
                            break;
                        }
                    }
                    i++;
                }
                else
                {
                    printf("Hex dump of .text:\n");
                    print_text_funcs(&lst);
                }
                break;
            default:
                fprintf(
                    stderr,
                    "%c - wrong option for program, either -d, -f, -h, -x\n",
                    opt);
                break;
            }
        }
        else
        {
            printf("\toption: %s\n", argv[i]);
        }
        i++;
    }

    // for (size_t i = 0; i < lst.count; i++)
    //     if (strcmp(lst.items[i].name, "mult") == 0
    //         || strcmp(lst.items[i].name, "divide") == 0)
    //     {
    //         printf("x86 disas of symbol %s\n", lst.items[i].name);
    //         disas((uint8_t *)lst.items[i].bytes, lst.items[i].size,
    //               (uint64_t)lst.items[i].addr);
    //     }

    free_symlist(lst);
    free(impsec);
    free(text_sec);
    file_unmap(&f);
    return 0;
}

#include "include/parse_elf.h"
#include "include/pretty_print.h"

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ELF header
static void print_target_sys(Elf64_Ehdr *hdr);
static void print_arch(Elf64_Ehdr *hdr);
static void print_ftype(Elf64_Ehdr *hdr);

// Program headers
static void print_seg_flag(Elf64_Phdr *hdr);
static void print_seg_type(Elf64_Phdr *hdr);
static void print_Phdr(Elf64_Phdr *hdr);

// Section headers
static void print_Shdr(Elf64_Shdr *shdr);

void print_Ehdr(Elf64_Ehdr *hdr)
{
    puts("---------- ELF headers ----------");
    print_arch(hdr);
    print_target_sys(hdr);
    print_ftype(hdr);
    putchar('\n');
}

void print_Phdrs(void *buf, Elf64_Ehdr *ehdr)
{
    if (ehdr->e_phnum <= 0)
        return;

    puts("-------- Program headers --------");

    Elf64_Half i = 0;
    Elf64_Phdr *ph_start = get_phdrs(buf, ehdr);
    while (i < ehdr->e_phnum)
    {
        void *ptr = (char *)ph_start + i * ehdr->e_phentsize;
        Elf64_Phdr *ph_cur = (Elf64_Phdr *)ptr;
        printf("%d\n", i + 1);
        print_Phdr(ph_cur);
        puts("--");
        i++;
    }

    putchar('\n');
}

void print_Shdrs(void *buf, Elf64_Ehdr *ehdr)
{
    if (ehdr->e_shnum <= 0)
        return;

    puts("-------- Section headers --------");

    struct impsec *sec = get_impsec(buf, ehdr);

    Elf64_Half i = 0;
    Elf64_Shdr *sh_start = get_shdrs(buf, ehdr);
    while (i < ehdr->e_shnum)
    {
        void *ptr = (char *)sh_start + i * ehdr->e_shentsize;
        Elf64_Shdr *sh_cur = (Elf64_Shdr *)ptr;
        printf("Section %d\n", i);
        if ((sh_cur->sh_flags & SHF_ALLOC) && sh_cur->sh_addr != 0x0)
        {
            print_Shdr(sh_cur);
            const char *shstrtab_data = (char *)buf + sec->strtab->sh_offset;
            const char *secname = shstrtab_data + sh_cur->sh_name;
            printf("Section name:\t%s\n", secname);
        }

        else
            printf("Unmapped\n");
        puts("--");
        i++;
    }
}

// Dump memory byte by byte in hex format
void hexdump(unsigned char *ptr, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        printf("0x%02x", ptr[i]);
        if (i % 10 == 0 && i != 0)
            putchar('\n');
        else
            putchar('\t');
    }
    putchar('\n');
}

// void print_symtab(struct sec *symtab, struct sec *strtab)
// {
//     if (!symtab || !strtab || !symtab->addr || !strtab->addr)
//         return;
//
//     printf("Function Symbols:\n");
//
//     size_t offset = 0;
//     while (offset + sizeof(Elf64_Sym) <= symtab->size)
//     {
//         Elf64_Sym *sym = (Elf64_Sym *)((char *)symtab->addr + offset);
//
//         // Check if it's a function symbol with a valid size
//         if (ELF64_ST_TYPE(sym->st_info) == STT_FUNC && sym->st_size > 0)
//         {
//             const char *name = (const char *)strtab->addr + sym->st_name;
//             printf(" - %s at 0x%lx (%lu bytes)\n", name, sym->st_value,
//                    sym->st_size);
//         }
//
//         offset += symtab->entsize;
//     }
// }

static void print_Shdr(Elf64_Shdr *shdr)
{
    printf("Virtual address:\t0x%08lx\n", shdr->sh_addr);
}

static void print_Phdr(Elf64_Phdr *phdr)
{
    print_seg_flag(phdr);
    print_seg_type(phdr);
    printf("\tSegment physical address:\t0x%08lx\n", phdr->p_paddr);
    printf("\tSegment virtual address:\t0x%08lx\n", phdr->p_vaddr);
    printf("\tSegment disk size:\t\t%lu\n", phdr->p_filesz);
    printf("\tSegment memory size:\t\t%lu\n", phdr->p_memsz);
}

static void print_seg_type(Elf64_Phdr *phdr)
{
    printf("\tSegment type:\t\t\t");
    switch (phdr->p_type)
    {
    case PT_NULL:
        printf("Unused");
        break;
    case PT_LOAD:
        printf("Loadable");
        break;
    case PT_DYNAMIC:
        printf("Dynamic linking");
        break;
    case PT_INTERP:
        printf("Program interpreter");
        break;
    case PT_GNU_STACK:
        printf("Exception handler");
        break;
    default:
        printf("Other");
        break;
    }

    putchar('\n');
}

static void print_seg_flag(Elf64_Phdr *phdr)
{
    printf("\tSegment flag:\t\t\t");
    switch (phdr->p_flags)
    {
    case PF_X:
        printf("Executable segment");
        break;
    case PF_W:
        printf("Writeable segment");
        break;
    case PF_R:
        printf("Readable segment");
        break;
    default:
        printf("Unknown");
        break;
    }

    putchar('\n');
}

static void print_target_sys(Elf64_Ehdr *ehdr)
{
    printf("Target system:\t");
    switch (ehdr->e_ident[EI_OSABI])
    {
    case ELFOSABI_NETBSD:
        printf("Net BSD");
        break;
    case ELFOSABI_FREEBSD:
        printf("Free BSD");
        break;
    case ELFOSABI_LINUX:
        printf("Linux");
        break;
    case ELFOSABI_SYSV:
        printf("Unix System V");
        break;
    case ELFOSABI_SOLARIS:
        printf("Solaris");
        break;
    case ELFOSABI_ARM:
        printf("ARM architecture");
        break;
    case ELFOSABI_STANDALONE:
        printf("Standalone executable (embedded)");
        break;
    default:
        printf("?");
        break;
    }
    putchar('\n');
}

static void print_arch(Elf64_Ehdr *ehdr)
{
    printf("Architecture:\t");
    switch (ehdr->e_ident[EI_CLASS])
    {
    case ELFCLASS64:
        printf("x86_64");
        break;
    case ELFCLASS32:
        printf("x86 (32 bits)");
        break;
    default:
        printf("?");
    }

    putchar('\n');
}

static void print_ftype(Elf64_Ehdr *ehdr)
{
    printf("Type:\t\t");
    switch (ehdr->e_type)
    {
    case ET_REL:
        printf("Relocatable file");
        break;
    case ET_EXEC:
        printf("Executable file");
        break;
    case ET_DYN:
        printf("Shared object");
        break;
    case ET_CORE:
        printf("Core file");
        break;
    case ET_NONE:
    default:
        printf("?");
        break;
    }

    putchar('\n');
}

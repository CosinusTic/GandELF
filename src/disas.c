// src/disas.c
#include "include/opcodes.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Readability constants for modrm types
#define N 0 // MODRM_NONE
#define R 1 // MODRM_REG
#define D 2 // MODRM_DIGIT
#define G(x) (x) // digit group

struct asm_ins
{
    // Prefixes / mode
    bool has_66, has_67;
    bool lock, rep, repne;
    uint8_t rex; // 0100WRXB
    int op_size; // 16/32/64 (Z-width)
    int addr_size; // (unused for now)

    // Opcode bytes
    uint8_t map; // 1, 0x0F, 0x38, 0x3A
    uint8_t op; // opcode byte in that map

    const struct opcode_info *op_desc; // Custom opcode descriptor

    // ModR/M / SIB
    bool has_modrm, has_sib;
    uint8_t modrm, sib;
    uint8_t mod, reg, rm; // 3-bit fields (extended by REX)
    uint8_t scale, index, base; // from SIB

    // Displacement / immediates
    int disp_size;
    int imm_size; // TODO: resolve
    int64_t disp; // sign-extended disp8/disp32
    uint64_t imm; // raw immediate value TODO: use this
};

/* Parsing order:
[prefixes] -> [0F?/map] -> [opcode byte] -> [ModR/M? -> SIB? -> disp?] -> [imm?]
*/

static const char *reg8_no_rex[16] = { "al",   "cl",   "dl",   "bl",
                                       "ah",   "ch",   "dh",   "bh",
                                       "r8b",  "r9b",  "r10b", "r11b",
                                       "r12b", "r13b", "r14b", "r15b" };
static const char *reg8_rex[16] = { "al",   "cl",   "dl",   "bl",
                                    "spl",  "bpl",  "sil",  "dil",
                                    "r8b",  "r9b",  "r10b", "r11b",
                                    "r12b", "r13b", "r14b", "r15b" };
static const char *reg16[16] = { "ax",   "cx",   "dx",   "bx",  "sp",   "bp",
                                 "si",   "di",   "r8w",  "r9w", "r10w", "r11w",
                                 "r12w", "r13w", "r14w", "r15w" };
static const char *reg32[16] = { "eax",  "ecx",  "edx",  "ebx", "esp",  "ebp",
                                 "esi",  "edi",  "r8d",  "r9d", "r10d", "r11d",
                                 "r12d", "r13d", "r14d", "r15d" };
static const char *reg64[16] = { "rax", "rcx", "rdx", "rbx", "rsp", "rbp",
                                 "rsi", "rdi", "r8",  "r9",  "r10", "r11",
                                 "r12", "r13", "r14", "r15" };

static const char *reg_name(unsigned reg, int width, uint8_t rex)
{
    switch (width)
    {
    case 8:
        return (rex ? reg8_rex : reg8_no_rex)[reg & 15];
    case 16:
        return reg16[reg & 15];
    case 32:
        return reg32[reg & 15];
    case 64:
        return reg64[reg & 15];
    default:
        return "??";
    }
}

static void extract_modrm(struct asm_ins *ins)
{
    ins->mod = ins->modrm >> 6;
    ins->reg = (ins->modrm >> 3) & 7;
    ins->rm = ins->modrm & 7;

    if (ins->rex & 0x4)
        ins->reg |= 8; // REX.R
    if (ins->rex & 0x1)
        ins->rm |= 8; // REX.B
}

static void extract_sib(struct asm_ins *ins)
{
    ins->scale = ins->sib >> 6;
    ins->index = (ins->sib >> 3) & 7;
    ins->base = ins->sib & 7;

    if (ins->rex & 0x2)
        ins->index |= 8; // REX.X
    if (ins->rex & 0x1)
        ins->base |= 8; // REX.B
}

// Memory formatter: [base + disp]
static void format_mem(char *buf, size_t cap, const struct asm_ins *ins)
{
    const char *base = reg_name(ins->rm & 15, 64, ins->rex);
    if (ins->disp_size == 0)
        snprintf(buf, cap, "[%s]", base);
    else if (ins->disp_size == 1)
        snprintf(buf, cap, "[%s%+d]", base, (int)ins->disp);
    else
        snprintf(buf, cap, "[%s%+d]", base, (int)ins->disp);
}

static const struct opcode_info *get_opcode_info(uint8_t op, uint8_t map)
{
    switch (map)
    {
    case 1:
        return &modrm_prim_map[op];
    case 0x0F:
        return &modrm_0f_map[op];
    case 0x38:
        return &modrm_0f38_map[op];
    case 0x3A:
        return &modrm_0f3a_map[op];
    default:
        return NULL;
    }
}

static void print_operand(char *buf, size_t cap, const struct asm_ins *ins,
                          uint8_t ot, int index)
{
    (void)index;
    switch (ot)
    {
    case OT_REG:
    case OT_REGZ: {
        // If reg comes from opcode bits, index==special marker
        unsigned regid;
        if (ins->op_desc->modrm_kind == N)
        {
            // register in low bits of opcode
            regid = (ins->op & 7) | ((ins->rex & 0x1) ? 8 : 0);
        }
        else
        {
            regid = ins->reg;
        }
        snprintf(buf, cap, "%%%s", reg_name(regid, ins->op_size, ins->rex));
        break;
    }
    case OT_RM:
    case OT_RMZ:
        if (ins->mod == 3)
        {
            snprintf(buf, cap, "%%%s",
                     reg_name(ins->rm, ins->op_size, ins->rex));
        }
        else
        {
            char mem[64];
            format_mem(mem, sizeof(mem), ins);
            snprintf(buf, cap, "%s", mem);
        }
        break;
    case OT_IMM8:
        snprintf(buf, cap, "$0x%" PRIx8, (uint8_t)ins->imm);
        break;
    case OT_IMM16:
        snprintf(buf, cap, "$0x%" PRIx16, (uint16_t)ins->imm);
        break;
    case OT_IMM32:
        snprintf(buf, cap, "$0x%" PRIx32, (uint32_t)ins->imm);
        break;
    case OT_IMM64:
        snprintf(buf, cap, "$0x%" PRIx64, ins->imm);
        break;
    default:
        snprintf(buf, cap, "<?>");
    }
}

static void print_instruction(const struct asm_ins *ins)
{
    printf("%s", ins->op_desc->mnemonic);

    for (int i = 0; i < ins->op_desc->operand_count; i++)
    {
        char opbuf[64];
        if (i)
            printf(", ");
        print_operand(opbuf, sizeof(opbuf), ins, ins->op_desc->operand_types[i],
                      i);
        printf("%s", opbuf);
    }
    putchar('\n');
}

size_t decode64(const uint8_t *p, size_t max, struct asm_ins *ins)
{
    const uint8_t *start = p;
    const uint8_t *end = p + max;
    memset(ins, 0, sizeof(*ins));

    // Prefixes
    while (1)
    {
        if (p >= end)
            return 0;
        uint8_t b = *p;
        if (b == 0x66)
        {
            ins->has_66 = true;
            p++;
            continue;
        }
        if (b == 0x67)
        {
            ins->has_67 = true;
            p++;
            continue;
        }
        if (b == 0xF0)
        {
            ins->lock = true;
            p++;
            continue;
        }
        if (b == 0xF3)
        {
            ins->rep = true;
            p++;
            continue;
        }
        if (b >= 0x40 && b <= 0x4F)
        {
            ins->rex = b;
            p++;
            continue;
        }
        break;
    }

    // Map
    if (p >= end)
        return 0;
    if (*p == 0x0F)
    {
        p++;
        if (p >= end)
            return 0;
        if (*p == 0x38)
        {
            ins->map = 0x38;
            p++;
        }
        else if (*p == 0x3A)
        {
            ins->map = 0x3A;
            p++;
        }
        else
            ins->map = 0x0F;
    }
    else
    {
        ins->map = 1;
    }

    // Opcode
    if (p >= end)
        return 0;
    ins->op = *p++;

    // Operand-size (Z)
    ins->op_size = 32;
    if (ins->rex & 0x08) // REX.W
        ins->op_size = 64;
    else if (ins->has_66)
        ins->op_size = 16;

    // Descriptor
    ins->op_desc = get_opcode_info(ins->op, ins->map);
    if (!ins->op_desc)
        return 0;

    // ModR/M handling
    if (ins->op_desc->modrm_kind != N)
    {
        if (p >= end)
            return 0;
        ins->has_modrm = true;
        ins->modrm = *p++;
        extract_modrm(ins);
    }

    // SIB + displacement handling
    if (ins->has_modrm)
    {
        // SIB if mod!=3 and r/m==100b
        if (ins->mod != 3 && (ins->rm & 7) == 4)
        {
            if (p >= end)
                return 0;
            ins->has_sib = true;
            ins->sib = *p++;
            extract_sib(ins);
        }

        // displacement size
        if (ins->mod == 1)
            ins->disp_size = 1;
        else if (ins->mod == 2)
            ins->disp_size = 4;
        else if (ins->mod == 0)
        {
            if ((ins->rm & 7) == 5)
                ins->disp_size = 4;
            if (ins->has_sib && ((ins->sib & 7) == 5))
                ins->disp_size = 4;
        }

        // read displacement
        if (ins->disp_size)
        {
            if (p + ins->disp_size > end)
                return 0;
            if (ins->disp_size == 1)
                ins->disp = (int8_t)*p++;
            else
            {
                int32_t d32;
                memcpy(&d32, p, 4);
                p += 4;
                ins->disp = d32;
            }
        }
    }

    // Immediates (only used for bounds check & pointer update)
    if (ins->imm_size)
    {
        if (p + ins->imm_size > end)
            return 0;
        ins->imm = 0;
        memcpy(&ins->imm, p, ins->imm_size);
        p += ins->imm_size;
    }

    return (size_t)(p - start);
}

/* ---------------- Pretty-printer (simple) ---------------- */

/* Syntax
 * Intel: mov dst, src
 * AT&T:  mov src, dst
 */

static void print_simple(const struct asm_ins *ins)
{
    if (ins->op_desc && ins->op_desc->mnemonic[0])
        print_instruction(ins);
    else
        printf("db 0x%02X\n", ins->op);
}

void disas(const uint8_t *ptr, size_t size)
{
    puts("Test parsing of bytes");

    const uint8_t *p = ptr;
    const uint8_t *end = ptr + size;
    struct asm_ins ins;

    while (p < end)
    {
        size_t n = decode64(p, (size_t)(end - p), &ins);
        if (!n)
        {
            puts("Decoding error");
            break;
        }

        printf("Bytes parsed:");
        for (size_t i = 0; i < n; i++)
            printf(" 0x%02X", p[i]);
        putchar('\n');

        print_simple(&ins);
        p += n;
    }
}

// src/disas.c
#include "include/opcodes.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct asm_ins
{
    // Prefixes / mode
    bool has_66, has_67;
    bool lock, rep, repne;
    uint8_t rex; // 0100WRXB, byte extension
    uint8_t rex_w, rex_r, rex_x, rex_b;
    int op_size; // 16/32/64 (Z-width)
    int addr_size; // TODO: Use

    // Opcode bytes
    uint8_t map; // 1, 0x0F, 0x38, 0x3A
    uint8_t op; // opcode byte in that map

    const struct opcode_info *op_desc; // Custom opcode descriptor

    // ModR/M / SIB
    bool has_modrm, has_sib;
    uint8_t modrm; // Operands encoding
    uint8_t mod; // 2 bits, displacement size
    uint8_t reg; // 3 bits, either opcode extension of register reference
    uint8_t rm; // 3-bits, direct or indirect register operand (extended by REX)

    uint8_t sib; // Memory addressing
    uint8_t scale, index, base; // Addr = base + (index * scale) + disp

    // Displacement / immediates
    int disp_size;
    int imm_size; // Size if encoded operands in instruction
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

static int resolve_imm_size(struct asm_ins *ins)
{
    const struct opcode_info *d = ins->op_desc;

    if (!ins->op_desc)
        return 0;
    if (d->imm_size)
        return d->imm_size;

    for (int i = 0; i < d->operand_count && i < 3; i++)
    {
        switch (d->operand_types[i])
        {
        case OT_IMM8:
        case OT_REL8:
            return 1;
        case OT_IMM16:
            return 2;
        case OT_IMM32:
        case OT_REL32:
            return 4;
        case OT_IMM64:
            return 8;
        case OT_IMMZ: {
            if (ins->rex_w)
                return 8;
            if (ins->has_66)
                return 2;
            return 4;
        }
        default:
            break;
        }
    }

    return 0;
}

static void decode_modrm(struct asm_ins *ins)
{
    ins->mod = ins->modrm >> 6;
    ins->reg = (ins->modrm >> 3) & 7;
    ins->rm = ins->modrm & 7;

    if (ins->op_desc->modrm_kind == R && ins->rex_r)
        ins->reg |= 8;
    if (ins->rex_b)
        ins->rm |= 8;
}

static void decode_sib(struct asm_ins *ins)
{
    ins->scale = ins->sib >> 6;
    ins->index = (ins->sib >> 3) & 7;
    ins->base = ins->sib & 7;

    if (ins->rex_x)
        ins->index |= 8;
    if (ins->rex_b)
        ins->base |= 8;
}

// 0100WRXB, byte extension
static void decode_rex(struct asm_ins *ins)
{
    if (ins->rex)
    {
        const uint8_t r = ins->rex;
        ins->rex_b = (r >> 0) & 1;
        ins->rex_x = (r >> 1) & 1;
        ins->rex_r = (r >> 2) & 1;
        ins->rex_w = (r >> 3) & 1;
    }
}

// Memory formatter: addr =  base + index*scale + disp
static void format_mem(char *buf, size_t cap, const struct asm_ins *ins)
{
    // Address register width (32 when 67h, else 64)
    const int aw = ins->addr_size;

    // RIP-relative
    if (!ins->has_sib && ins->mod != 3 && ((ins->rm & 7) == 5) && aw == 64)
    {
        // disp32 is relative to next RIP, just show [rip+disp]
        if (ins->disp_size == 0)
            snprintf(buf, cap, "[rip]");
        else if (ins->disp_size == 1)
            snprintf(buf, cap, "[rip%+d]", (int)(int8_t)ins->disp);
        else
            snprintf(buf, cap, "[rip%+d]", (int)(int32_t)ins->disp);
        return;
    }

    // SIB
    if (ins->has_sib && ins->mod != 3)
    {
        // Decode components
        unsigned base = ins->base;
        unsigned index = ins->index;
        int scale = 1 << ins->scale;

        const char *base_s = NULL;
        const char *index_s = NULL;

        // Base = none when mod==0 && base==5  (disp32 only)
        bool have_base = !(ins->mod == 0 && ((base & 7) == 5));
        bool have_index = ((index & 7) != 4); // 4 means "no index"

        if (have_base)
            base_s = reg_name(base, aw, ins->rex);
        if (have_index)
            index_s = reg_name(index, aw, ins->rex);

        // Build
        if (have_base && have_index)
        {
            if (ins->disp_size == 0)
                snprintf(buf, cap, "[%s+%s*%d]", base_s, index_s, scale);
            else if (ins->disp_size == 1)
                snprintf(buf, cap, "[%s+%s*%d%+d]", base_s, index_s, scale,
                         (int)(int8_t)ins->disp);
            else
                snprintf(buf, cap, "[%s+%s*%d%+d]", base_s, index_s, scale,
                         (int)(int32_t)ins->disp);
        }
        else if (have_base)
        {
            if (ins->disp_size == 0)
                snprintf(buf, cap, "[%s]", base_s);
            else if (ins->disp_size == 1)
                snprintf(buf, cap, "[%s%+d]", base_s, (int)(int8_t)ins->disp);
            else
                snprintf(buf, cap, "[%s%+d]", base_s, (int)(int32_t)ins->disp);
        }
        else if (have_index)
        {
            if (ins->disp_size == 0)
                snprintf(buf, cap, "[%s*%d]", index_s, scale);
            else if (ins->disp_size == 1)
                snprintf(buf, cap, "[%s*%d%+d]", index_s, scale,
                         (int)(int8_t)ins->disp);
            else
                snprintf(buf, cap, "[%s*%d%+d]", index_s, scale,
                         (int)(int32_t)ins->disp);
        }
        else
        {
            // pure disp
            if (ins->disp_size == 0)
                snprintf(buf, cap, "[0]");
            else if (ins->disp_size == 1)
                snprintf(buf, cap, "[%d]", (int)(int8_t)ins->disp);
            else
                snprintf(buf, cap, "[%d]", (int)(int32_t)ins->disp);
        }
        return;
    }

    // Simple base+disp (no SIB)
    const char *base = reg_name(ins->rm, aw, ins->rex);
    if (ins->disp_size == 0)
        snprintf(buf, cap, "[%s]", base);
    else if (ins->disp_size == 1)
        snprintf(buf, cap, "[%s%+d]", base, (int)(int8_t)ins->disp);
    else
        snprintf(buf, cap, "[%s%+d]", base, (int)(int32_t)ins->disp);
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

static int width_from_kind(uint8_t kind, int z)
{
    switch (kind)
    {
    case OT_REG8:
    case OT_RM8:
        return 8;
    case OT_REG16:
    case OT_RM16:
        return 16;
    case OT_REG32:
    case OT_RM32:
        return 32;
    case OT_REG64:
    case OT_RM64:
        return 64;
    case OT_REGZ:
    case OT_RMZ:
        return z;
    default:
        return z; // OT_REG/OT_RM fallback
    }
}

static void print_operand_generic(const struct asm_ins *ins, uint8_t kind,
                                  int idx)
{
    (void)idx;
    switch (kind)
    {
    // registers encoded via ModR/M or, if no ModR/M, in opcode (push/pop)
    case OT_REG:
    case OT_REG8:
    case OT_REG16:
    case OT_REG32:
    case OT_REG64:
    case OT_REGZ: {
        int w = width_from_kind(kind, ins->op_size);
        unsigned regid;

        if (!ins->has_modrm)
        {
            // No ModR/M: take reg from low 3 bits of opcode (opcode+rd form).
            regid = (ins->op & 7) | ((ins->rex & 0x1) ? 8 : 0);
            // In long mode, treat pushes/pops etc. as 64-bit even if Z says 32
            if (w == ins->op_size
                && ((ins->op & 0xF8) == 0x50 || (ins->op & 0xF8) == 0x58))
                w = 64;
        }
        else
            regid = ins->reg;
        printf("%s", reg_name(regid, w, ins->rex));
        break;
    }

    // r/m operand (depending on mod)
    case OT_RM:
    case OT_RM8:
    case OT_RM16:
    case OT_RM32:
    case OT_RM64:
    case OT_RMZ: {
        int w = width_from_kind(kind, ins->op_size);
        if (ins->mod == 3)
            printf("%s", reg_name(ins->rm, w, ins->rex));
        else
        {
            char mem[64];
            format_mem(mem, sizeof mem, ins);
            printf("%s", mem);
        }
        break;
    }

    // fixed registers
    case OT_AL:
        printf("al");
        break;
    case OT_AX:
        printf("ax");
        break;
    case OT_EAX:
        printf("eax");
        break;
    case OT_RAX:
        printf("rax");
        break;

    // immediates
    case OT_IMM8:
        printf("0x%02" PRIx64, (uint64_t)(ins->imm & 0xff));
        break;
    case OT_IMM16:
        printf("0x%04" PRIx64, (uint64_t)(ins->imm & 0xffff));
        break;
    case OT_IMM32:
        printf("0x%08" PRIx64, (uint64_t)(ins->imm & 0xffffffffULL));
        break;
    case OT_IMM64:
        printf("0x%016" PRIx64, (uint64_t)ins->imm);
        break;

    // rel8/rel32 (print as signed displacements)
    case OT_REL8:
        printf(".+%d", (int8_t)ins->imm);
        break;
    case OT_REL32:
        printf(".+%d", (int32_t)ins->imm);
        break;
    default:
        printf("<?>");
        break;
    }
}

static void print_simple(const uint8_t *addr, size_t len,
                         const struct asm_ins *ins)
{
    // bytes column (8 bytes max)
    printf("%-16s", "");
    for (size_t i = 0; i < len && i < 8; i++)
        printf("%02X ", addr[i]);
    for (size_t i = len; i < 8 && i < 8; i++)
        printf("   ");

    // Mnemonic
    if (!ins->op_desc || !ins->op_desc->mnemonic || !ins->op_desc->mnemonic[0])
    {
        printf("db 0x%02X\n", ins->op);
        return;
    }
    printf("%s", ins->op_desc->mnemonic);

    // Operands
    if (ins->op_desc->operand_count == 0)
    {
        putchar('\n');
        return;
    }
    putchar(' ');
    for (int i = 0; i < ins->op_desc->operand_count; i++)
    {
        if (i)
            printf(", ");
        print_operand_generic(ins, ins->op_desc->operand_types[i], i);
    }
    putchar('\n');
}

static size_t get_prefixes(const uint8_t *p, struct asm_ins *ins, size_t max)
{
    const uint8_t *start = p;
    const uint8_t *end = p + max;

    uint8_t pending_rex = 0;

    while (1)
    {
        if (p >= end)
            return (size_t)-1;

        uint8_t b = *p;

        if (b == 0x66)
        {
            ins->has_66 = true;
            p++;
            pending_rex = 0;
            continue;
        }
        if (b == 0x67)
        {
            ins->has_67 = true;
            p++;
            pending_rex = 0;
            continue;
        }
        if (b == 0xF0)
        {
            ins->lock = true;
            p++;
            pending_rex = 0;
            continue;
        }
        if (b == 0xF3)
        {
            ins->rep = true;
            p++;
            pending_rex = 0;
            continue;
        }
        if (b == 0xF2)
        {
            ins->repne = true;
            p++;
            pending_rex = 0;
            continue;
        }
        if (b == 0x2E || b == 0x36 || b == 0x3E || b == 0x26 || b == 0x64
            || b == 0x65) // Segment overrides
        {
            p++;
            pending_rex = 0;
            continue;
        }
        if (b >= 0x40 && b <= 0x4F)
        {
            pending_rex = b;
            p++;
            continue;
        }

        break;
    }

    // Last effective REX is the only valid REX
    ins->rex = pending_rex;

    return (size_t)(p - start);
}

size_t decode64(const uint8_t *p, size_t max, struct asm_ins *ins)
{
    const uint8_t *start = p;
    const uint8_t *end = p + max;
    memset(ins, 0, sizeof(*ins));

    p += get_prefixes(p, ins, max);
    decode_rex(ins);

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
    ins->op_size = ins->rex_w ? 64 : (ins->has_66 ? 16 : 32);
    ins->addr_size = ins->has_67 ? 32 : 64;

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
        decode_modrm(ins);
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
            decode_sib(ins);
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
            if (ins->map == 0 && ins->has_sib && ins->base == 5)
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

    ins->imm_size = resolve_imm_size(ins);

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

/* Syntax
 * Intel: mov dst, src
 * AT&T:  mov src, dst
 */

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

        print_simple(p, n, &ins);
        p += n;
    }
}

#include "include/disas.h"
#include "include/opcodes.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

/*
 * 0F: whether the opcode map is two-byte (starts with 0x0F). If yes, you must
read a second opcode byte; for 3-byte maps you’ll see 0F 38 or 0F 3A, etc.

po / so (primary / secondary opcode): the actual opcode byte(s). For one-byte
map it’s just po. For two-byte, it’s 0F + po. For three-byte, 0F + so + po
(e.g., 0F 38 xx).

e (extension): tells you about ModR/M.reg usage:
    /r → instruction has a ModR/M and the reg field selects a register operand.
    /0…/7 → instruction has a ModR/M and reg must be that value (selects an
opcode subgroup).
mnemonic: human-readable instruction name (ADD, MOV, …).

op1, op2, …: operand forms. Shorthands:
    r/m8|16|32|64 → either register or memory of that size (decided by
ModR/M.mod+rm).
    r8|16|32|64 → register of that size (from ModR/M.reg or the opcode itself).
    AL/AX/EAX/RAX → accumulator special encodings.
    imm8|imm16|imm32|imm64 → immediate of that size follows the encoding.
    rel8|rel32 → relative immediate for branches.

The rest (tested/modified flags, CPU “proc”, etc.) are semantics/availability;
not needed to compute lengths.


-- Memo
Prefixes (0+ bytes): F0/F2/F3, 66, 67, segment overrides, and in 64-bit: REX
(40–4F). Record flags (e.g., REX.W, operand-size=16 when 66).

Opcode:
    1 byte (primary map), or
    0F + 1 byte (two-byte map), or
    0F 38/0F 3A + 1 byte (three-byte maps).

ModR/M (0 or 1):
    Required when the table shows /r or /0..7 (opcode extension), or when
operand is r/m....
    Decode mod/reg/rm.

SIB (0 or 1):
    Only if mod != 11 and rm == 100b (in 32/64-bit addressing).

Displacement (0/1/4 bytes typically in 64-bit):
    From mod:
        00: none, except rm=101b → disp32 (and in 64-bit this is RIP-relative).
        01: disp8
        10: disp32

Immediate(s) (0+ bytes):
    From the operands column: imm8/16/32/64, or rel8/rel32, etc.

--
*/

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
    int addr_size; // 32 or 64 (67h overrides to 32)

    // Opcode bytes
    uint8_t map; // 1, 0x0F, 0x38, 0x3A
    uint8_t op; // opcode byte in that map

    const struct opcode_info *op_desc; // Custom opcode descriptor

    // ModR/M / SIB
    bool has_modrm, has_sib;
    uint8_t modrm, sib;
    uint8_t mod, reg, rm;
    uint8_t scale, index, base;

    // Displacement / immediates
    int disp_size;
    int imm_size; // resolved for this instance
    int64_t disp; // sign-extended disp8/disp32
    uint64_t imm; // raw immediate value
};

// --------- register name helpers (already similar in your other version)
// ----------
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

    // REX extensions: R extends reg, B extends r/m (and base), X extends index
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

static void format_mem(char *buf, size_t cap, const struct asm_ins *ins)
{
    // Only base+disp
    const char *base =
        reg_name(ins->rm & 15, 64, ins->rex); // use 64-bit base names
    if (ins->disp_size == 0)
        snprintf(buf, cap, "[%s]", base);
    else if (ins->disp_size == 1)
        snprintf(buf, cap, "[%s%+d]", base, (int)(int8_t)ins->disp);
    else
        snprintf(buf, cap, "[%s%+d]", base, (int)(int32_t)ins->disp);
}

static void print_mov_add_operands(const struct asm_ins *ins, uint8_t op,
                                   int op_size)
{
    // op_size is 16/32/64; pick register width Z
    int z = op_size;
    if (op == 0x89)
    {
        // mov r/mZ, rZ  (reg -> r/m)
        if (ins->mod == 3)
        {
            printf("%%%s, %%%s", reg_name(ins->rm, z, ins->rex),
                   reg_name(ins->reg, z, ins->rex));
        }
        else
        {
            char mem[64];
            format_mem(mem, sizeof mem, ins);
            printf("%s, %%%s", mem, reg_name(ins->reg, z, ins->rex));
        }
    }
    else if (op == 0x8B)
    {
        // mov rZ, r/mZ  (r/m -> reg)
        if (ins->mod == 3)
        {
            printf("%%%s, %%%s", reg_name(ins->reg, z, ins->rex),
                   reg_name(ins->rm, z, ins->rex));
        }
        else
        {
            char mem[64];
            format_mem(mem, sizeof mem, ins);
            printf("%%%s, %s", reg_name(ins->reg, z, ins->rex), mem);
        }
    }
    else if (op == 0x01)
    {
        // add r/mZ, rZ   (dest is r/m)
        if (ins->mod == 3)
        {
            printf("%%%s, %%%s", reg_name(ins->rm, z, ins->rex),
                   reg_name(ins->reg, z, ins->rex));
        }
        else
        {
            char mem[64];
            format_mem(mem, sizeof mem, ins);
            printf("%s, %%%s", mem, reg_name(ins->reg, z, ins->rex));
        }
    }
    else
    {
        printf("<?>");
    }
}

/*
-- parsing order
[prefixes] -> [0F?/map] -> [opcode byte] -> [ModR/M? -> SIB? -> disp?] ->
[imm?]
--
*/

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

size_t decode64(const uint8_t *p, size_t max, struct asm_ins *_asm)
{
    const uint8_t *start = p;
    const uint8_t *end = p + max;
    memset(_asm, 0, sizeof(struct asm_ins));

    // Get prefixes
    while (1)
    {
        if (p >= end)
            return 0;
        uint8_t b = *p;
        if (b == 0x66)
        {
            _asm->has_66 = true;
            p++;
            continue;
        }
        if (b == 0x67)
        {
            _asm->has_67 = true;
            p++;
            continue;
        }
        if (b == 0xF0)
        {
            _asm->lock = true;
            p++;
            continue;
        }
        if (b == 0xF3)
        {
            _asm->rep = true;
            p++;
            continue;
        }
        if (b >= 0x40 && b <= 0x4F)
        {
            _asm->rex = b;
            p++;
            continue;
        }
        break;
    }

    // Get byte mapping
    if (*p == 0x0F)
    {
        p++;
        if (p >= end)
            return 0;
        if (*p == 0x38)
        {
            _asm->map = 0x38;
            p++;
        }
        else if (*p == 0x3A)
        {
            _asm->map = 0x3A;
            p++;
        }
        else
            _asm->map = 0x0F;
    }
    else
        _asm->map = 1;

    // Opcode
    _asm->op = *p++;

    // Get operand size
    _asm->op_size = 32;
    if (_asm->rex & 0x08) // Check that xxxx xxxx & 0000 1000 != 0000 0000
        _asm->op_size = 64;
    else if (_asm->has_66)
        _asm->op_size = 16;

    // Get static info on opcode
    _asm->op_desc = get_opcode_info(_asm->op, _asm->map);
    if (!_asm->op_desc)
        return 0;

    if (_asm->op_desc->modrm_kind != N)
    {
        if (p >= end)
            return 0;
        _asm->has_modrm = true;
        _asm->modrm = *p++;
        extract_modrm(_asm);
    }

    // Decode modrm & check SIB + get disp_size
    if (_asm->has_modrm)
    {
        if (p >= end)
            return 0;
        uint8_t mod = _asm->modrm >> 6;
        // uint8_t reg = (_asm->modrm >> 3) & 7;
        uint8_t rm = _asm->modrm & 7;

        if (mod != 3 && rm == 4) // Check for SIB
        {
            if (p >= end)
                return 0;
            _asm->has_sib = true;
            _asm->sib = *p++;
            extract_sib(_asm);
        }
        if (mod == 1)
            _asm->disp_size = 1;
        else if (mod == 2 || (mod == 0 && rm == 5))
            _asm->disp_size = 4; // RIP relative addressing or 32 bit addressing

        if (p + _asm->disp_size > end)
            return 0;
        p += _asm->disp_size;
    }

    // Get immediates size
    if (_asm->imm_size > 0)
    {
        if (p + _asm->imm_size > end)
            return 0;
        p += _asm->imm_size;
    }

    return (size_t)(p - start);
}

static void print_simple(const uint8_t *addr, size_t len,
                         const struct asm_ins *ins)
{
    // bytes
    printf("%-16s", ""); // room for address column if you add one later
    for (size_t i = 0; i < len && i < 8; i++)
        printf("%02X ", addr[i]);
    if (len > 8)
        printf("... ");
    if (len < 8)
    {
        for (size_t i = len; i < 8; i++)
            printf("   ");
    }

    // mnemonic/opcode families we actually hit
    if (ins->op == 0xC3)
    {
        puts("ret");
        return;
    }

    // PUSH/POP opcode+rd families
    if ((ins->op & 0xF8) == 0x50)
    {
        unsigned rd = (ins->op & 7) | ((ins->rex & 0x1) ? 8 : 0);
        printf("push %%%s\n", reg_name(rd, ins->op_size, ins->rex));
        return;
    }
    if ((ins->op & 0xF8) == 0x58)
    {
        unsigned rd = (ins->op & 7) | ((ins->rex & 0x1) ? 8 : 0);
        printf("pop %%%s\n", reg_name(rd, ins->op_size, ins->rex));
        return;
    }

    // Table-driven for mov/add we’ve seen
    switch (ins->op)
    {
    case 0x89:
        printf("mov ");
        print_mov_add_operands(ins, 0x89, ins->op_size);
        puts("");
        break;
    case 0x8B:
        printf("mov ");
        print_mov_add_operands(ins, 0x8B, ins->op_size);
        puts("");
        break;
    case 0x01:
        printf("add ");
        print_mov_add_operands(ins, 0x01, ins->op_size);
        puts("");
        break;
    default:
        puts("db 0x??");
        break; // fallback stub
    }
}

void disas(const uint8_t *ptr, size_t size)
{
    puts("Test parsing of bytes");

    const uint8_t *p = ptr;
    const uint8_t *end = ptr + size;
    struct asm_ins ins;

    while (p < end)
    {
        size_t num_bytes_parsed = decode64(p, (size_t)(end - p), &ins);
        if (num_bytes_parsed == 0)
        {
            puts("Decoding error");
            break;
        }

        printf("Bytes parsed:");
        for (size_t i = 0; i < num_bytes_parsed; i++)
            printf(" 0x%02X", p[i]);
        putchar('\n');

        // printf("\n\tmap=0x%X\n\topcode=0x%X\n\tmodrm_kind=%u\n\thas_modrm=%"
        //        "d\n\tmodrm=0x%X "
        //        "sib=0x%X disp_size=%d imm_size=%d op_size=%d\n\n",
        //        ins.map, ins.op, ins.op_desc->modrm_kind, ins.has_modrm,
        //        ins.modrm, ins.sib, ins.disp_size, ins.imm_size, ins.op_size);

        print_simple(p, num_bytes_parsed, &ins);
        p += num_bytes_parsed;
    }
}

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

    // Static descriptor for this opcode
    const struct opcode_info *op_desc; // points into the map table you indexed

    // ModR/M / SIB
    bool has_modrm, has_sib;
    uint8_t modrm, sib;
    uint8_t mod, reg, rm; // extracted fields (3-bit each)
    uint8_t scale, index, base; // from SIB when present (2/3/3 bits)

    // Displacement / immediates
    int disp_size;
    int imm_size; // resolved for this instance
    int64_t disp; // sign-extended disp8/disp32
    uint64_t imm; // raw immediate value
};

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

// In bytes
// static int resolve_imm_size(const struct opcode_info *d, int op_size)
// {
//     if (!d)
//         return 0;
//     if (d->imm_size)
//         return d->imm_size;
//
//     for (int i = 0; i < d->operand_count && i < 3; i++)
//         switch (d->operand_types[i])
//         {
//         case OT_IMM8:
//             return 1;
//         case OT_IMM16:
//             return 2;
//         case OT_IMM32:
//             return 4;
//         case OT_IMM64:
//             return 8;
//         // Backup to operand size
//         case OT_REGZ:
//         case OT_RMZ:
//         default:
//             break;
//         }
//
//     if (op_size == 16)
//         return 2;
//     if (op_size == 32)
//         return 4;
//     if (op_size == 64)
//         return 8;
//
//     return 0;
// }

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

        printf("\n\tmap=0x%X\n\topcode=0x%X\n\tmodrm_kind=%u\n\thas_modrm=%"
               "d\n\tmodrm=0x%X "
               "sib=0x%X disp_size=%d imm_size=%d op_size=%d\n\n",
               ins.map, ins.op, ins.op_desc->modrm_kind, ins.has_modrm,
               ins.modrm, ins.sib, ins.disp_size, ins.imm_size, ins.op_size);

        p += num_bytes_parsed;
    }
}

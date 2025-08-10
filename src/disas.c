#include "include/disas.h"
#include <inttypes.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/*
 * start from the left to first identify if it's a prefix, two byte, then 0x0f
 * refix, then primary opcode, etc Ex: start with prefix, layered finite-state
 * decoder, where each stage consumes as many bytes as it knows how to handle,
 * then hands off to the next stage
 */

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

struct asm_instruction
{
    bool has_66, has_67, lock, rep, repne;
    uint8_t rex; // bits W R X B, expand SIB bytes, specify operand size and
                 // access r8-15
    uint8_t map; // 1, 2, or 3 bytes overall instruction size
    uint8_t op; // opcode
    uint8_t modrm, sib; // Operands encoding
    bool has_modrm, has_sib;
    int disp_size, imm_size;
    int op_size; // 16/32/64
};

/*
-- parsing order
[prefixes] -> [0F?/map] -> [opcode byte] -> [ModR/M? -> SIB? -> disp?] ->
[imm?]
--
*/

size_t decode64(const uint8_t *p, size_t max, struct asm_instruction *_asm)
{
    const uint8_t *start = p;
    const uint8_t *end = p + max;
    memset(_asm, 0, sizeof(struct asm_instruction));

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

    // Get instruction size
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
    if ((_asm->rex & 0x08)
        == 0x01) // check that xxxx xxxx & 1111 1111 = 0000 0001
        _asm->op_size = 64;
    else if ((_asm->rex & 0x08) == 0x01 && _asm->has_66)
        _asm->op_size = 16;

    // TODO: Set has_modrm, imm_size
    if ()

        return 0;
}

static const uint8_t *state_0x0f(const uint8_t *p);
static const uint8_t *state_prim_opcode(uint8_t byte, const uint8_t *p);

static const uint8_t *state_0x0f(const uint8_t *p)
{
    uint8_t byte = *p++;
    return state_prim_opcode(byte, p);
}

static const uint8_t *state_prim_opcode(uint8_t byte, const uint8_t *p)
{
    printf("OPCODE");
    if (byte <= 0x05)
    {
        printf("\tADD\n");
        // state_AND() here
    }
    else if (byte <= 0x0d)
    {
        printf("\tOR\n");
        // state_OR() here
    }
    // ...
    return p; // return where we are after consuming bytes
}

void disas(const uint8_t *ptr, size_t remaining)
{
    puts("Test parsing of bytes");

    const uint8_t *p = ptr;
    const uint8_t *end = ptr + remaining;

    while (p < end)
    {
        uint8_t byte = *p++;

        if (byte == 0x0f)
        {
            printf("(2-byte prefix)\n");
            p = state_0x0f(p);
        }
        else if (byte <= 0x25)
        {
            p = state_prim_opcode(byte, p);
        }
        // ... etc
        else
            printf("Unknown opcode: 0x%02x\n", byte);
    }
}

#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>
#include <stddef.h>

/* Readability constants */
#define N 0 /* MODRM_NONE */
#define R 1 /* MODRM_REG  (/r) */
#define D 2 /* MODRM_DIGIT (/0-7) */
#define G(x) (x) /* digit for single-digit groups (else 0) */

struct opcode_info
{
    uint8_t modrm_kind; /* N / R / D */
    uint8_t group_digit; /* 0-7 if single-digit group, else 0 */
};

/* ================================
 * Primary opcode map (map = 1)
 * ================================ */
static const struct opcode_info modrm_prim_map[256] = {
    [0x00] = { R, 0 },    [0x01] = { R, 0 },    [0x02] = { R, 0 },
    [0x03] = { R, 0 },    [0x04] = { N, 0 },    [0x05] = { N, 0 },
    [0x06] = { N, 0 },    [0x07] = { N, 0 },    [0x08] = { R, 0 },
    [0x09] = { R, 0 },    [0x0A] = { R, 0 },    [0x0B] = { R, 0 },
    [0x0C] = { N, 0 },    [0x0D] = { N, 0 },    [0x0E] = { N, 0 },
    [0x0F] = { N, 0 },

    [0x10] = { R, 0 },    [0x11] = { R, 0 },    [0x12] = { R, 0 },
    [0x13] = { R, 0 },    [0x14] = { N, 0 },    [0x15] = { N, 0 },
    [0x16] = { N, 0 },    [0x17] = { N, 0 },    [0x18] = { R, 0 },
    [0x19] = { R, 0 },    [0x1A] = { R, 0 },    [0x1B] = { R, 0 },
    [0x1C] = { N, 0 },    [0x1D] = { N, 0 },    [0x1E] = { N, 0 },
    [0x1F] = { N, 0 },

    [0x20] = { R, 0 },    [0x21] = { R, 0 },    [0x22] = { R, 0 },
    [0x23] = { R, 0 },    [0x24] = { N, 0 },    [0x25] = { N, 0 },
    [0x26] = { N, 0 },    [0x27] = { N, 0 },    [0x28] = { R, 0 },
    [0x29] = { R, 0 },    [0x2A] = { R, 0 },    [0x2B] = { R, 0 },
    [0x2C] = { N, 0 },    [0x2D] = { N, 0 },    [0x2E] = { N, 0 },
    [0x2F] = { N, 0 },

    [0x30] = { R, 0 },    [0x31] = { R, 0 },    [0x32] = { R, 0 },
    [0x33] = { R, 0 },    [0x34] = { N, 0 },    [0x35] = { N, 0 },
    [0x36] = { N, 0 },    [0x37] = { N, 0 },    [0x38] = { R, 0 },
    [0x39] = { R, 0 },    [0x3A] = { R, 0 },    [0x3B] = { R, 0 },
    [0x3C] = { N, 0 },    [0x3D] = { N, 0 },    [0x3E] = { N, 0 },
    [0x3F] = { N, 0 },

    [0x40] = { N, 0 },    [0x41] = { N, 0 },    [0x42] = { N, 0 },
    [0x43] = { N, 0 },    [0x44] = { N, 0 },    [0x45] = { N, 0 },
    [0x46] = { N, 0 },    [0x47] = { N, 0 },    [0x48] = { N, 0 },
    [0x49] = { N, 0 },    [0x4A] = { N, 0 },    [0x4B] = { N, 0 },
    [0x4C] = { N, 0 },    [0x4D] = { N, 0 },    [0x4E] = { N, 0 },
    [0x4F] = { N, 0 },

    [0x50] = { N, 0 },    [0x51] = { N, 0 },    [0x52] = { N, 0 },
    [0x53] = { N, 0 },    [0x54] = { N, 0 },    [0x55] = { N, 0 },
    [0x56] = { N, 0 },    [0x57] = { N, 0 },    [0x58] = { N, 0 },
    [0x59] = { N, 0 },    [0x5A] = { N, 0 },    [0x5B] = { N, 0 },
    [0x5C] = { N, 0 },    [0x5D] = { N, 0 },    [0x5E] = { N, 0 },
    [0x5F] = { N, 0 },

    [0x60] = { N, 0 },    [0x61] = { N, 0 },    [0x62] = { R, 0 },
    [0x63] = { R, 0 },    [0x64] = { N, 0 },    [0x65] = { N, 0 },
    [0x66] = { N, 0 },    [0x67] = { N, 0 },    [0x68] = { N, 0 },
    [0x69] = { R, 0 },    [0x6A] = { N, 0 },    [0x6B] = { R, 0 },
    [0x6C] = { N, 0 },    [0x6D] = { N, 0 },    [0x6E] = { N, 0 },
    [0x6F] = { N, 0 },

    [0x70] = { N, 0 },    [0x71] = { N, 0 },    [0x72] = { N, 0 },
    [0x73] = { N, 0 },    [0x74] = { N, 0 },    [0x75] = { N, 0 },
    [0x76] = { N, 0 },    [0x77] = { N, 0 },    [0x78] = { N, 0 },
    [0x79] = { N, 0 },    [0x7A] = { N, 0 },    [0x7B] = { N, 0 },
    [0x7C] = { N, 0 },    [0x7D] = { N, 0 },    [0x7E] = { N, 0 },
    [0x7F] = { N, 0 },

    [0x80] = { D, G(0) }, [0x81] = { D, G(0) }, [0x82] = { N, 0 },
    [0x83] = { D, G(0) }, [0x84] = { R, 0 },    [0x85] = { R, 0 },
    [0x86] = { R, 0 },    [0x87] = { R, 0 },    [0x88] = { R, 0 },
    [0x89] = { R, 0 },    [0x8A] = { R, 0 },    [0x8B] = { R, 0 },
    [0x8C] = { R, 0 },    [0x8D] = { R, 0 },    [0x8E] = { R, 0 },
    [0x8F] = { D, G(0) },

    [0x90] = { N, 0 },    [0x91] = { N, 0 },    [0x92] = { N, 0 },
    [0x93] = { N, 0 },    [0x94] = { N, 0 },    [0x95] = { N, 0 },
    [0x96] = { N, 0 },    [0x97] = { N, 0 },    [0x98] = { N, 0 },
    [0x99] = { N, 0 },    [0x9A] = { N, 0 },    [0x9B] = { N, 0 },
    [0x9C] = { N, 0 },    [0x9D] = { N, 0 },    [0x9E] = { N, 0 },
    [0x9F] = { N, 0 },

    [0xA0] = { N, 0 },    [0xA1] = { N, 0 },    [0xA2] = { N, 0 },
    [0xA3] = { N, 0 },    [0xA4] = { N, 0 },    [0xA5] = { N, 0 },
    [0xA6] = { N, 0 },    [0xA7] = { N, 0 },    [0xA8] = { N, 0 },
    [0xA9] = { N, 0 },    [0xAA] = { N, 0 },    [0xAB] = { N, 0 },
    [0xAC] = { N, 0 },    [0xAD] = { N, 0 },    [0xAE] = { N, 0 },
    [0xAF] = { N, 0 },

    [0xB0] = { N, 0 },    [0xB1] = { N, 0 },    [0xB2] = { N, 0 },
    [0xB3] = { N, 0 },    [0xB4] = { N, 0 },    [0xB5] = { N, 0 },
    [0xB6] = { N, 0 },    [0xB7] = { N, 0 },    [0xB8] = { N, 0 },
    [0xB9] = { N, 0 },    [0xBA] = { D, G(0) }, [0xBB] = { N, 0 },
    [0xBC] = { N, 0 },    [0xBD] = { N, 0 },    [0xBE] = { N, 0 },
    [0xBF] = { N, 0 },

    [0xC0] = { D, G(0) }, [0xC1] = { D, G(0) }, [0xC2] = { N, 0 },
    [0xC3] = { N, 0 },    [0xC4] = { R, 0 },    [0xC5] = { R, 0 },
    [0xC6] = { D, G(0) }, [0xC7] = { D, G(0) }, [0xC8] = { N, 0 },
    [0xC9] = { N, 0 },    [0xCA] = { N, 0 },    [0xCB] = { N, 0 },
    [0xCC] = { N, 0 },    [0xCD] = { N, 0 },    [0xCE] = { N, 0 },
    [0xCF] = { N, 0 },

    [0xD0] = { D, G(0) }, [0xD1] = { D, G(0) }, [0xD2] = { D, G(0) },
    [0xD3] = { D, G(0) }, [0xD4] = { N, 0 },    [0xD5] = { N, 0 },
    [0xD6] = { N, 0 },    [0xD7] = { N, 0 },    [0xD8] = { R, 0 },
    [0xD9] = { R, 0 },    [0xDA] = { R, 0 },    [0xDB] = { R, 0 },
    [0xDC] = { R, 0 },    [0xDD] = { R, 0 },    [0xDE] = { R, 0 },
    [0xDF] = { R, 0 },

    [0xE0] = { N, 0 },    [0xE1] = { N, 0 },    [0xE2] = { N, 0 },
    [0xE3] = { N, 0 },    [0xE4] = { N, 0 },    [0xE5] = { N, 0 },
    [0xE6] = { N, 0 },    [0xE7] = { N, 0 },    [0xE8] = { N, 0 },
    [0xE9] = { N, 0 },    [0xEA] = { N, 0 },    [0xEB] = { N, 0 },
    [0xEC] = { N, 0 },    [0xED] = { N, 0 },    [0xEE] = { N, 0 },
    [0xEF] = { N, 0 },

    [0xF0] = { N, 0 },    [0xF1] = { N, 0 },    [0xF2] = { N, 0 },
    [0xF3] = { N, 0 },    [0xF4] = { N, 0 },    [0xF5] = { N, 0 },
    [0xF6] = { D, G(0) }, [0xF7] = { D, G(0) }, [0xF8] = { N, 0 },
    [0xF9] = { N, 0 },    [0xFA] = { N, 0 },    [0xFB] = { N, 0 },
    [0xFC] = { N, 0 },    [0xFD] = { N, 0 },    [0xFE] = { D, G(0) },
    [0xFF] = { D, G(0) },
};

/* ==================================
 * 0F two-byte opcode map (map = 0x0F)
 * NOTE: Many entries default to N.
 * Fill more as you implement them.
 * ================================== */
static const struct opcode_info modrm_0f_map[256] = {
    /* 0F 00..0F often /digit (SLDT, STR, LLDT, LTR, VERR/VERW, etc.) */
    [0x00] = { D, G(0) },
    [0x01] = { D, G(0) },
    [0x02] = { N, 0 },
    [0x03] = { N, 0 },
    [0x04] = { N, 0 },
    [0x05] = { N, 0 },
    [0x06] = { N, 0 },
    [0x07] = { N, 0 },
    [0x08] = { N, 0 },
    [0x09] = { N, 0 },
    [0x0A] = { N, 0 },
    [0x0B] = { N, 0 },
    [0x0C] = { N, 0 },
    [0x0D] = { N, 0 },
    [0x0E] = { N, 0 },
    [0x0F] = { N, 0 },

    /* 0F 10..1F: many are /r (MOVUPS/MOVUPS, MOVDQA, etc.); 0F 1F /0 is NOP r/m
     */
    [0x10] = { R, 0 },
    [0x11] = { R, 0 },
    [0x12] = { R, 0 },
    [0x13] = { R, 0 },
    [0x14] = { R, 0 },
    [0x15] = { R, 0 },
    [0x16] = { R, 0 },
    [0x17] = { R, 0 },
    [0x18] = { D, G(0) },
    /* prefetch hints /digit */[0x19] = { N, 0 },
    [0x1A] = { N, 0 },
    [0x1B] = { N, 0 },
    [0x1C] = { N, 0 },
    [0x1D] = { N, 0 },
    [0x1E] = { N, 0 },
    [0x1F] = { D, G(0) }, /* 0F 1F /0 NOP r/m */

    /* 0F 20..2F: MOV from/to CR/DR (/r-ish enc), MOVAPS/MOVNTPS etc. */
    [0x20] = { R, 0 },
    [0x21] = { R, 0 },
    [0x22] = { R, 0 },
    [0x23] = { R, 0 },
    [0x24] = { N, 0 },
    [0x25] = { N, 0 },
    [0x26] = { N, 0 },
    [0x27] = { N, 0 },
    [0x28] = { R, 0 },
    [0x29] = { R, 0 },
    [0x2A] = { R, 0 },
    [0x2B] = { R, 0 },
    [0x2C] = { R, 0 },
    [0x2D] = { R, 0 },
    [0x2E] = { R, 0 },
    [0x2F] = { R, 0 },

    /* 0F 30..3F: system (WRMSR/RDMSR/etc. usually none), MOVQ xmm/m64 /r etc.
     */
    [0x30] = { N, 0 },
    [0x31] = { N, 0 },
    [0x32] = { N, 0 },
    [0x33] = { N, 0 },
    [0x34] = { N, 0 },
    [0x35] = { N, 0 },
    [0x36] = { N, 0 },
    [0x37] = { N, 0 },
    [0x38] = { R, 0 },
    [0x39] = { R, 0 },
    [0x3A] = { R, 0 },
    [0x3B] = { R, 0 },
    [0x3C] = { R, 0 },
    [0x3D] = { R, 0 },
    [0x3E] = { R, 0 },
    [0x3F] = { R, 0 },

    /* 0F 40..4F: CMOVcc /r */
    [0x40] = { R, 0 },
    [0x41] = { R, 0 },
    [0x42] = { R, 0 },
    [0x43] = { R, 0 },
    [0x44] = { R, 0 },
    [0x45] = { R, 0 },
    [0x46] = { R, 0 },
    [0x47] = { R, 0 },
    [0x48] = { R, 0 },
    [0x49] = { R, 0 },
    [0x4A] = { R, 0 },
    [0x4B] = { R, 0 },
    [0x4C] = { R, 0 },
    [0x4D] = { R, 0 },
    [0x4E] = { R, 0 },
    [0x4F] = { R, 0 },

    /* 0F 50..5F: SSE MOVMSK, SQRTPS, ANDPS, etc. mostly /r */
    [0x50] = { R, 0 },
    [0x51] = { R, 0 },
    [0x52] = { R, 0 },
    [0x53] = { R, 0 },
    [0x54] = { R, 0 },
    [0x55] = { R, 0 },
    [0x56] = { R, 0 },
    [0x57] = { R, 0 },
    [0x58] = { R, 0 },
    [0x59] = { R, 0 },
    [0x5A] = { R, 0 },
    [0x5B] = { R, 0 },
    [0x5C] = { R, 0 },
    [0x5D] = { R, 0 },
    [0x5E] = { R, 0 },
    [0x5F] = { R, 0 },

    /* 0F 60..6F: PUNPCK* / MOVDQA/MOVDQU /r */
    [0x60] = { R, 0 },
    [0x61] = { R, 0 },
    [0x62] = { R, 0 },
    [0x63] = { R, 0 },
    [0x64] = { R, 0 },
    [0x65] = { R, 0 },
    [0x66] = { R, 0 },
    [0x67] = { R, 0 },
    [0x68] = { R, 0 },
    [0x69] = { R, 0 },
    [0x6A] = { R, 0 },
    [0x6B] = { R, 0 },
    [0x6C] = { R, 0 },
    [0x6D] = { R, 0 },
    [0x6E] = { R, 0 },
    [0x6F] = { R, 0 },

    /* 0F 70..7F: PSHUFW/PSHUFD imm8 (still /r), MOVD/MOVQ /r, MOVDQA/Q /r */
    [0x70] = { R, 0 },
    [0x71] = { D, G(0) },
    [0x72] = { D, G(0) },
    [0x73] = { D, G(0) },
    [0x74] = { R, 0 },
    [0x75] = { R, 0 },
    [0x76] = { R, 0 },
    [0x77] = { N, 0 },
    [0x78] = { N, 0 },
    [0x79] = { N, 0 },
    [0x7A] = { N, 0 },
    [0x7B] = { N, 0 },
    [0x7C] = { R, 0 },
    [0x7D] = { R, 0 },
    [0x7E] = { R, 0 },
    [0x7F] = { R, 0 },

    /* 0F 80..8F: Jcc rel32 (no ModR/M) */
    [0x80] = { N, 0 },
    [0x81] = { N, 0 },
    [0x82] = { N, 0 },
    [0x83] = { N, 0 },
    [0x84] = { N, 0 },
    [0x85] = { N, 0 },
    [0x86] = { N, 0 },
    [0x87] = { N, 0 },
    [0x88] = { N, 0 },
    [0x89] = { N, 0 },
    [0x8A] = { N, 0 },
    [0x8B] = { N, 0 },
    [0x8C] = { N, 0 },
    [0x8D] = { N, 0 },
    [0x8E] = { N, 0 },
    [0x8F] = { N, 0 },

    /* 0F 90..9F: SETcc r/m8 (/r) */
    [0x90] = { R, 0 },
    [0x91] = { R, 0 },
    [0x92] = { R, 0 },
    [0x93] = { R, 0 },
    [0x94] = { R, 0 },
    [0x95] = { R, 0 },
    [0x96] = { R, 0 },
    [0x97] = { R, 0 },
    [0x98] = { R, 0 },
    [0x99] = { R, 0 },
    [0x9A] = { R, 0 },
    [0x9B] = { R, 0 },
    [0x9C] = { R, 0 },
    [0x9D] = { R, 0 },
    [0x9E] = { R, 0 },
    [0x9F] = { R, 0 },

    /* 0F A0..AF: PUSH/POP fs/gs (none), CPUID (none), BT/BTS/BTR/BTC (/r), IMUL
       (/r) */
    [0xA0] = { N, 0 },
    [0xA1] = { N, 0 },
    [0xA2] = { N, 0 },
    [0xA3] = { N, 0 },
    [0xA4] = { R, 0 },
    [0xA5] = { R, 0 },
    [0xA6] = { R, 0 },
    [0xA7] = { R, 0 },
    [0xA8] = { R, 0 },
    [0xA9] = { R, 0 },
    [0xAA] = { R, 0 },
    [0xAB] = { R, 0 },
    [0xAC] = { R, 0 },
    [0xAD] = { R, 0 },
    [0xAE] = { D, G(0) }, /* XSAVE/XRSTOR /digit area */
    [0xAF] = { R, 0 }, /* IMUL rZ, r/mZ */

    /* 0F B0..BF: CMPXCHG (/r), BSWAP (none), MOVZX/MOVSX (/r) */
    [0xB0] = { R, 0 },
    [0xB1] = { R, 0 },
    [0xB2] = { R, 0 },
    [0xB3] = { R, 0 },
    [0xB4] = { R, 0 },
    [0xB5] = { R, 0 },
    [0xB6] = { R, 0 },
    [0xB7] = { R, 0 },
    [0xB8] = { N, 0 },
    [0xB9] = { N, 0 },
    [0xBA] = { D, G(0) },
    [0xBB] = { R, 0 },
    [0xBC] = { R, 0 },
    [0xBD] = { R, 0 },
    [0xBE] = { R, 0 },
    [0xBF] = { R, 0 },

    /* 0F C0..CF: XADD (/r), BSWAP r (none), pinsrw/pextrw (/r) */
    [0xC0] = { R, 0 },
    [0xC1] = { R, 0 },
    [0xC2] = { R, 0 },
    [0xC3] = { R, 0 },
    [0xC4] = { R, 0 },
    [0xC5] = { R, 0 },
    [0xC6] = { R, 0 },
    [0xC7] = { R, 0 },
    [0xC8] = { N, 0 },
    [0xC9] = { N, 0 },
    [0xCA] = { N, 0 },
    [0xCB] = { N, 0 },
    [0xCC] = { N, 0 },
    [0xCD] = { N, 0 },
    [0xCE] = { N, 0 },
    [0xCF] = { N, 0 },

    /* 0F D0..DF: psrlw/psrld/psrlq (/r), pshufw (/r), etc. */
    [0xD0] = { R, 0 },
    [0xD1] = { R, 0 },
    [0xD2] = { R, 0 },
    [0xD3] = { R, 0 },
    [0xD4] = { R, 0 },
    [0xD5] = { R, 0 },
    [0xD6] = { R, 0 },
    [0xD7] = { R, 0 },
    [0xD8] = { R, 0 },
    [0xD9] = { R, 0 },
    [0xDA] = { R, 0 },
    [0xDB] = { R, 0 },
    [0xDC] = { R, 0 },
    [0xDD] = { R, 0 },
    [0xDE] = { R, 0 },
    [0xDF] = { R, 0 },

    /* 0F E0..EF: pavgb/etc. (/r) … 0F E6 CVTTPD2DQ (/r)… 0F E7 MOVNTDQ (/r)… */
    [0xE0] = { R, 0 },
    [0xE1] = { R, 0 },
    [0xE2] = { R, 0 },
    [0xE3] = { R, 0 },
    [0xE4] = { R, 0 },
    [0xE5] = { R, 0 },
    [0xE6] = { R, 0 },
    [0xE7] = { R, 0 },
    [0xE8] = { R, 0 },
    [0xE9] = { R, 0 },
    [0xEA] = { R, 0 },
    [0xEB] = { R, 0 },
    [0xEC] = { R, 0 },
    [0xED] = { R, 0 },
    [0xEE] = { R, 0 },
    [0xEF] = { R, 0 },

    /* 0F F0..FF: LDDQU (/r), PSLLDQ/PSRLDQ via /digit in 0F 73, etc. */
    [0xF0] = { R, 0 },
    [0xF1] = { R, 0 },
    [0xF2] = { R, 0 },
    [0xF3] = { R, 0 },
    [0xF4] = { R, 0 },
    [0xF5] = { R, 0 },
    [0xF6] = { R, 0 },
    [0xF7] = { R, 0 },
    [0xF8] = { R, 0 },
    [0xF9] = { R, 0 },
    [0xFA] = { R, 0 },
    [0xFB] = { R, 0 },
    [0xFC] = { R, 0 },
    [0xFD] = { R, 0 },
    [0xFE] = { R, 0 },
    [0xFF] = { R, 0 },
};

/* ==================================
 * 0F 38 three-byte opcode map (map = 0x38)
 * (SSSE3/SSE4a/SSE4.1 groups; mostly /r)
 * ================================== */
static const struct opcode_info modrm_0f38_map[256] = {
    /* Common ones filled; rest default N */
    [0x00] = { R, 0 }, [0x01] = { R, 0 }, [0x02] = { R, 0 }, [0x03] = { R, 0 },
    [0x04] = { R, 0 }, [0x05] = { R, 0 }, [0x06] = { R, 0 }, [0x07] = { R, 0 },
    [0x0F] = { R, 0 }, [0x1E] = { R, 0 }, [0x1F] = { R, 0 }, [0x20] = { R, 0 },
    [0x21] = { R, 0 }, [0x22] = { R, 0 }, [0x23] = { R, 0 }, [0x2A] = { R, 0 },
    [0x2B] = { R, 0 }, [0x2C] = { R, 0 }, [0x2D] = { R, 0 }, [0x2E] = { R, 0 },
    [0x2F] = { R, 0 }, [0x40] = { R, 0 }, [0x41] = { R, 0 }, [0x42] = { R, 0 },
    [0x43] = { R, 0 }, [0x80] = { R, 0 }, [0x81] = { R, 0 }, [0x82] = { R, 0 },
    [0x83] = { R, 0 }, [0x90] = { R, 0 }, [0x91] = { R, 0 }, [0x92] = { R, 0 },
    [0x93] = { R, 0 }, [0xF0] = { R, 0 }, [0xF1] = { R, 0 }, [0xF2] = { R, 0 },
    [0xF3] = { R, 0 },
    /* Fill out more as needed */
};

/* ==================================
 * 0F 3A three-byte opcode map (map = 0x3A)
 * (SSE4.1/SSE4.2 with imm8 in many cases; still /r)
 * ================================== */
static const struct opcode_info modrm_0f3a_map[256] = {
    [0x00] = { R, 0 }, [0x01] = { R, 0 }, [0x02] = { R, 0 }, [0x03] = { R, 0 },
    [0x08] = { R, 0 }, [0x09] = { R, 0 }, [0x0A] = { R, 0 }, [0x0B] = { R, 0 },
    [0x0C] = { R, 0 }, [0x0D] = { R, 0 }, [0x0E] = { R, 0 }, [0x0F] = { R, 0 },
    [0x14] = { R, 0 }, [0x15] = { R, 0 }, [0x16] = { R, 0 }, [0x17] = { R, 0 },
    [0x20] = { R, 0 }, [0x21] = { R, 0 }, [0x22] = { R, 0 }, [0x23] = { R, 0 },
    [0x30] = { R, 0 }, [0x31] = { R, 0 }, [0x32] = { R, 0 }, [0x33] = { R, 0 },
    [0x40] = { R, 0 }, [0x41] = { R, 0 }, [0x42] = { R, 0 }, [0x44] = { R, 0 },
    [0x60] = { R, 0 }, [0x61] = { R, 0 },
    /* Fill out more as needed */
};

/* Helper to choose the right map pointer */
static inline const struct opcode_info *gandelf_lookup_modrm(uint8_t map,
                                                             uint8_t opcode)
{
    switch (map)
    {
    case 1:
        return &modrm_prim_map[opcode];
    case 0x0F:
        return &modrm_0f_map[opcode];
    case 0x38:
        return &modrm_0f38_map[opcode];
    case 0x3A:
        return &modrm_0f3a_map[opcode];
    default:
        return NULL;
    }
}

#endif /* OPCODES_H */

#include "include/disas.h"
#include <inttypes.h>

#include <stdio.h>

/*
 * start from the left to first identify if it's a prefix, two byte, then 0x0f
 * refix, then primary opcode, etc Ex: start with prefix, layered finite-state
 * decoder, where each stage consumes as many bytes as it knows how to handle,
 * then hands off to the next stage
 */

static const uint8_t *state_0x0f(const uint8_t *p);
static const uint8_t *state_prim_opcode(uint8_t byte, const uint8_t *p);
static const uint8_t *state_0x0f(const uint8_t *p)
{
    uint8_t byte = *p++;
    return state_prim_opcode(byte, p);
}

static const uint8_t *state_prim_opcode(uint8_t byte, const uint8_t *p)
{
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

#ifndef DISAS_H
#define DISAS_H

#include <stddef.h>
#include <inttypes.h>

void disas(const uint8_t *ptr, size_t remaining, uint64_t start_rip);

#endif /* !DISAS_H */

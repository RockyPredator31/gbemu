#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>
#include "../utils/utils.h"

typedef struct
{
    // 8-Bit Register
    uint8_t a, f;      // AF Register
    uint8_t b, c;      // BC Register
    uint8_t d, e;      // DE Register
    uint8_t h, l;      // HL Register
    // 16 Bit Register
    uint16_t sp;       // Stack Pointer
    uint16_t pc;       // Program Counter

    // Interne Zustände
    bool halted;
    bool stopped;
    uint32_t cycles;

} CPU;


void cpu_init(CPU* cpu, GB_Version gbv);

#endif
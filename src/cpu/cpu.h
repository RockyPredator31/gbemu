#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>
#include "../gb.h"

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

uint8_t cpu_fetch(GameBoy* gb);
void cpu_decode_and_execute(GameBoy* gb, uint8_t opcode);
void cpu_step(GameBoy* gb);

// ====== Flag SET =======
void cpu_set_z(CPU* cpu);
void cpu_set_n(CPU* cpu);
void cpu_set_h(CPU* cpu);
void cpu_set_c(CPU* cpu);
// ====== Flag CLEAR =======
void cpu_clear_z(CPU* cpu);
void cpu_clear_n(CPU* cpu);
void cpu_clear_h(CPU* cpu);
void cpu_clear_c(CPU* cpu);

// ===== Operations ========
void op_nop(GameBoy* gb);


#endif
#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

typedef struct GameBoy GameBoy;

typedef enum
{
    DMG = 0,
    CGB
} GB_Version;

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
    bool halt_bug;
    bool stopped;
    uint32_t cycles;
    uint8_t ime;
    uint8_t ei_delay;

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

// =============== GET Flag ==========================
uint8_t cpu_get_z(const CPU* cpu);
uint8_t cpu_get_n(const CPU* cpu);
uint8_t cpu_get_h(const CPU* cpu);
uint8_t cpu_get_c(const CPU* cpu);

// ========= Get Registers ==========================
uint16_t cpu_get_af(const CPU* cpu);
uint16_t cpu_get_bc(const CPU* cpu);
uint16_t cpu_get_de(const CPU* cpu);
uint16_t cpu_get_hl(const CPU* cpu);

// ========= Set Register ===========================
void cpu_set_af(CPU* cpu, uint16_t value);
void cpu_set_bc(CPU* cpu, uint16_t value);
void cpu_set_de(CPU* cpu, uint16_t value);
void cpu_set_hl(CPU* cpu, uint16_t value);



#endif
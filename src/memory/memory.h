#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdbool.h>

typedef struct 
{
    // Interne Game Boy Speicher
    uint8_t vram[0x2000];
    uint8_t wram[0x2000];   // Work RAM
    uint8_t oam[0xA0];
    uint8_t hram[0x80];
    uint8_t io[0x80];       // I/O Register

} Memory;

void memory_init(Memory* memory);

#endif
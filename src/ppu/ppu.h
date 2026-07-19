#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>

typedef struct GameBoy GameBoy;

typedef struct
{
    int32_t scanline_counter;
    uint8_t buffer[160 * 144];
    bool frame_ready;
} PPU;

void ppu_init(PPU* ppu);

void ppu_update(GameBoy* gb, int32_t cycle);

void ppu_check_lyc(GameBoy* gb);

void ppu_render_scanline(GameBoy* gb);

#endif
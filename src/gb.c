#include "gb.h"
#include <stdlib.h>

void gb_init(GameBoy* gb, uint8_t* rom, size_t rom_size)
{
    cartridge_init(&gb->cartridge, rom, rom_size);
    memory_init(&gb->memory);
    cpu_init(&gb->cpu, DMG);
}



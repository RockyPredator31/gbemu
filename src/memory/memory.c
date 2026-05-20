#include "memory.h"

void memory_init(Memory* memory, uint8_t* rom, size_t rom_size)
{
    memory->rom = rom;
    memory->rom_size = rom_size;
    memory->eram = NULL;
    memory->eram_size = 0;
    
}
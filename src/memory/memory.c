#include "memory.h"
#include <stdlib.h>

//initializes memory
void memory_init(Memory* memory, uint8_t* rom, size_t rom_size)
{
    memory->rom = rom;
    memory->rom_size = rom_size;
    memory->eram = NULL;
    memory->eram_size = 0;    
}

//allocates eram
void memory_allocate(Memory* memory, size_t eram_size)
{
    memory->eram = (uint8_t*)malloc(eram_size);
}
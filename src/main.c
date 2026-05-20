#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "cartridge/cartridge.h"

int main(int argc, char *argv[])
{
    Cartridge cart = {0};

    FILE* f = fopen("roms/tetris.gb", "rb");
    if (!f) {
        printf("Fehler: ROM '%s' nicht gefunden!\n", "roms/tetris.gb");
        return 0;
    }

    fseek(f, 0, SEEK_END);
    size_t rom_size = ftell(f);
    rewind(f);

    uint8_t* rom = (uint8_t*)malloc(rom_size);
    fread(rom, 1, rom_size, f);
    fclose(f);

    cartridge_init(&cart, rom, rom_size);
    
    free(cart.rom);
    return 0;
}
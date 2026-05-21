#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "gb.h"

int main(int argc, char *argv[])
{
    GameBoy gb = {0};

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

    cartridge_init(&gb.cartridge, rom, rom_size);

    printf("Cartridge: %s | MBC: %d | ROM: %zu KB | ERAM: %zu KB", gb.cartridge.title, gb.cartridge.mbc_type, gb.cartridge.rom_size / 1024, gb.cartridge.eram_size / 1024);

    free(gb.cartridge.rom);
    free(gb.cartridge.eram);
    return 0;
}
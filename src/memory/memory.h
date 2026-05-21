#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdbool.h>

typedef struct 
{
    // Cartridge Speicher (wird vom Cartridge-Modul verwaltet)
    uint8_t* rom;           // Zeiger auf die geladene ROM
    size_t   rom_size;

    uint8_t* eram;          // External RAM (Save-Speicher)
    // eram ist hier die verwendbare Ram größe. Bei MBC2 ist es im ROM als 0 nagegeben, es sind aber 512 bytes vorhanden, aber als 0 angegeben. Liegt daran wie die hardware gebaut wurde, ist hier aber egal.
    // deswegen wird in dem fall MBC2 immer 512 bytes eram_size sein, egal was in rom[0x0149] steht.
    size_t   eram_size;

    // Interne Game Boy Speicher
    uint8_t vram[0x2000];
    uint8_t wram[0x2000];   // Work RAM
    uint8_t oam[0xA0];
    uint8_t hram[0x80];
    uint8_t io[0x80];       // I/O Register

} Memory;

void memory_init(Memory* memory, uint8_t* rom, size_t rom_size);

void memory_allocate(Memory* memory, size_t eram_size);

#endif
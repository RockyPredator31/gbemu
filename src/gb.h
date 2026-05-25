#ifndef GB_H
#define GB_H

#include "cartridge/cartridge.h"
#include "memory/memory.h"
#include "cpu/cpu.h"

//SCREEN
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

//MEMORY START ADRESSES
#define VRAM_START 0x8000
#define WRAM_START 0xC000
#define OAM_START 0xFE00
#define IO_REGISTERS_START 0xFF
#define HRAM_START 0xFF80
#define IE 0xFFFF                 //Interrupt enabled

typedef struct GameBoy GameBoy;

typedef struct GameBoy
{
    Cartridge cartridge;
    Memory memory;
    CPU cpu;
    uint8_t InterruptEnableRegister;



} GameBoy;


void gb_init(GameBoy* gb, uint8_t* rom, size_t rom_size);

uint8_t memory_read(GameBoy* gb, uint16_t addr);

uint16_t memory_read16(GameBoy* gb, uint16_t addr);

void memory_write(GameBoy* gb, uint16_t addr, uint8_t value);

void memory_write16(GameBoy* gb, uint16_t addr, uint16_t value);

#endif
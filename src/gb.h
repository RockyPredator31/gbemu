#ifndef GB_H
#define GB_H

#include "cartridge/cartridge.h"
#include "memory/memory.h"

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



typedef struct 
{
    Cartridge cartridge;
    Memory memory;

    uint8_t InterruptEnableRegister;



} GameBoy;

uint8_t read(GameBoy* gb, uint16_t addr);

void write(GameBoy* gb, uint16_t addr, uint8_t write);

#endif
#include "gb.h"


uint8_t memory_read(GameBoy* gb, uint16_t addr)
{
    if(addr >= 0x0000 && addr <= 0x3FFF)
    {
        return gb->cartridge.rom[addr % gb->cartridge.rom_size];
    }
    if(addr >= 0x4000 && addr <= 0x7FFF)
    {
        if(gb->cartridge.rom_size > 0x3FFF)
        {
            return gb->cartridge.rom[addr % gb->cartridge.rom_size];
        }
    }
    if(addr >= 0x8000 && addr <= 0x9FFF)
    {
        return gb->memory.vram[addr - 0x8000];
    }
    if(addr >= 0xA000 && addr <= 0xBFFF)
    {
        if(gb->cartridge.ram_enabled && gb->cartridge.eram_size > 0)
        {
            return gb->cartridge.eram[(addr - 0xA000) % gb->cartridge.eram_size];
        }
    }
    if(addr >= 0xC000 && addr <= 0xDFFF)
    {
        return gb->memory.wram[addr - 0xC000];
    }
    if(addr >= 0xE000 && addr <= 0xFDFF)
    {
        return memory_read(gb, addr - 0x2000);
    }
    if(addr >= 0xFE00 && addr <= 0xFE9F)
    {
        return gb->memory.oam[addr - 0xFE00];
    }
    if(addr >= 0xFF00 && addr <= 0xFF7F)
    {
        return gb->memory.io[addr - 0xFF00];
    }
    if(addr >= 0xFF80 && addr <= 0xFFFE)
    {
        return gb->memory.hram[addr - 0xFF80];
    }
    if(addr == 0xFFFF)
    {
        return gb->InterruptEnableRegister;
    }
    
    return 0;
}
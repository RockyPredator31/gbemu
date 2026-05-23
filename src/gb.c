#include "gb.h"

void gb_init(GameBoy* gb, uint8_t* rom, size_t rom_size)
{
    cartridge_init(&gb->cartridge, rom, rom_size);
    memory_init(&gb->memory);
}


uint8_t memory_read(GameBoy* gb, uint16_t addr)
{

    // ============= ROM (0x0000 - 0x7FFF) ===============
    if(addr <= 0x7FFF)
    {
        if(gb->cartridge.read != NULL)
            return gb->cartridge.read(&gb->cartridge, addr);
        else
            return gb->cartridge.rom[addr % gb->cartridge.rom_size]; // Fallback
    }


    // ============== VRAM =========================
    if(addr >= 0x8000 && addr <= 0x9FFF)
    {
        return gb->memory.vram[addr - 0x8000];
    }


    // ================== External RAM (ERAM) ============================
    if(addr >= 0xA000 && addr <= 0xBFFF)
    {
        if(gb->cartridge.ram_enabled && gb->cartridge.eram_size > 0 && gb->cartridge.eram != NULL)
        {
            uint32_t offset = addr - 0xA000;

            // Bank Switching, falls mehr als 8KB ERAM vorhanden
            if (gb->cartridge.eram_size > 0x2000)
            {
                offset += (uint32_t)gb->cartridge.ram_bank * 0x2000UL;
            }

            if (offset < gb->cartridge.eram_size)
                return gb->cartridge.eram[offset];
        }
        return 0xFF;
    }


    // ==================== WRAM =====================================
    if(addr >= 0xC000 && addr <= 0xDFFF)
    {
        return gb->memory.wram[addr - 0xC000];
    }


    // ================== Echo Ram (WRAM direkt aufrufen) ============
    if(addr >= 0xE000 && addr <= 0xFDFF)
    {
        return gb->memory.wram[addr - 0xE000];
    }


    // ================== OAM =========================
    if(addr >= 0xFE00 && addr <= 0xFE9F)
    {
        return gb->memory.oam[addr - 0xFE00];
    }


    // ================== IO ===============
    if(addr >= 0xFF00 && addr <= 0xFF7F)
    {
        return gb->memory.io[addr - 0xFF00];
    }


    // ================== HRAM =================
    if(addr >= 0xFF80 && addr <= 0xFFFE)
    {
        return gb->memory.hram[addr - 0xFF80];
    }


    // ================== IE =========================
    if(addr == 0xFFFF)
    {
        return gb->InterruptEnableRegister;
    }
    
    return 0xFF;
}

void memory_write(GameBoy* gb, uint16_t addr, uint8_t value)
{

    // ============= ROM (0x0000 - 0x7FFF) ===============
    if(addr <= 0x7FFF)
    {
        if(gb->cartridge.write != NULL)
            gb->cartridge.write(&gb->cartridge, addr, value);
        return;
    }


    // ============== VRAM =========================
    if(addr >= 0x8000 && addr <= 0x9FFF)
    {
        gb->memory.vram[addr - 0x8000] = value;
        return;
    }


    // ================== External RAM (ERAM) ============================
    if(addr >= 0xA000 && addr <= 0xBFFF)
    {
        if(gb->cartridge.ram_enabled && gb->cartridge.eram_size > 0 && gb->cartridge.eram != NULL)
        {
           uint32_t offset = addr - 0xA000;

            if (gb->cartridge.eram_size > 0x2000)
            {
                offset += (uint32_t)gb->cartridge.ram_bank * 0x2000UL;
            }

            if (offset < gb->cartridge.eram_size)
                gb->cartridge.eram[offset] = value;
        }
        return;
    }


    // ==================== WRAM =====================================
    if(addr >= 0xC000 && addr <= 0xDFFF)
    {
        gb->memory.wram[addr - 0xC000] = value;
        return;
    }


    // ================== Echo Ram (WRAM direkt aufrufen) ============
    if(addr >= 0xE000 && addr <= 0xFDFF)
    {
       gb->memory.wram[addr - 0xE000] = value;
       return;
    }


    // ================== OAM =========================
    if(addr >= 0xFE00 && addr <= 0xFE9F)
    {
       gb->memory.oam[addr - 0xFE00] = value;
       return;
    }


    // ================== IO ===============
    if(addr >= 0xFF00 && addr <= 0xFF7F)
    {
        gb->memory.io[addr - 0xFF00] = value;
        return;
    }


    // ================== HRAM =================
    if(addr >= 0xFF80 && addr <= 0xFFFE)
    {
        gb->memory.hram[addr - 0xFF80] = value;
        return;
    }


    // ================== IE =========================
    if(addr == 0xFFFF)
    {
        gb->InterruptEnableRegister = value;
        return;
    }

}
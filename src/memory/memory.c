#include "memory.h"
#include "../gb.h"
#include <string.h>

void memory_init(Memory* memory)
{
    if (memory == NULL) return;

    // Alles auf 0 setzen
    memset(memory, 0, sizeof(Memory));

    // ====================== Wichtige IO-Register Standardwerte ======================

    // --- Joypad ---
    memory->io[0x00] = 0xCF;        // P1   - Alle Tasten nicht gedrückt

    // --- Timer ---
    memory->io[0x04] = 0x00;        // DIV
    memory->io[0x05] = 0x00;        // TIMA
    memory->io[0x06] = 0x00;        // TMA
    memory->io[0x07] = 0xF8;        // TAC

    // --- Interrupts ---
    memory->io[0x0F] = 0xE1;        // IF (Interrupt Flag)

    // --- Sound (wichtige Werte) ---
    memory->io[0x10] = 0x80;        // NR10
    memory->io[0x11] = 0xBF;        // NR11
    memory->io[0x12] = 0xF3;        // NR12
    memory->io[0x14] = 0xBF;        // NR14
    memory->io[0x16] = 0x3F;        // NR21
    memory->io[0x17] = 0x00;        // NR22
    memory->io[0x19] = 0xBF;        // NR24
    memory->io[0x1A] = 0x7F;        // NR30
    memory->io[0x1B] = 0xFF;        // NR31
    memory->io[0x1C] = 0x9F;        // NR32
    memory->io[0x1E] = 0xBF;        // NR34
    memory->io[0x20] = 0xFF;        // NR41
    memory->io[0x21] = 0x00;        // NR42
    memory->io[0x22] = 0x00;        // NR43
    memory->io[0x23] = 0xBF;        // NR44
    memory->io[0x24] = 0x77;        // NR50
    memory->io[0x25] = 0xF3;        // NR51
    memory->io[0x26] = 0xF1;        // NR52 - Sound enabled

    // --- Graphics (LCD) ---
    memory->io[0x40] = 0x91;        // LCDC - Display on, Background on
    memory->io[0x41] = 0x81;        // STAT
    memory->io[0x42] = 0x00;        // SCY
    memory->io[0x43] = 0x00;        // SCX
    memory->io[0x45] = 0x00;        // LYC
    memory->io[0x47] = 0xFC;        // BGP  (Background Palette)
    memory->io[0x48] = 0xFF;        // OBP0 (Sprite Palette 0)
    memory->io[0x49] = 0xFF;        // OBP1 (Sprite Palette 1)
    memory->io[0x4A] = 0x00;        // WY
    memory->io[0x4B] = 0x00;        // WX

    // Weitere häufig genutzte Register
    memory->io[0x4D] = 0xFF;        // KEY1 (CGB Speed Switch)
    memory->io[0x4F] = 0xFF;        // VBK  (VRAM Bank)
    memory->io[0x70] = 0xFF;        // SVBK (WRAM Bank)
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
            {
                return gb->cartridge.eram[offset];
            }
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
        if (addr == 0xFF04) {
            gb->timer.div_counter = 0;
            gb->memory.io[0x04] = 0;
            return;
        }
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

uint16_t memory_read16(GameBoy* gb, uint16_t addr)
{
    return memory_read(gb, addr) | ((uint16_t)memory_read(gb, addr + 1) << 8);
}

void memory_write16(GameBoy* gb, uint16_t addr, uint16_t value)
{
    memory_write(gb, addr,     value & 0xFF);     // Low Byte
    memory_write(gb, addr + 1, value >> 8);       // High Byte
}
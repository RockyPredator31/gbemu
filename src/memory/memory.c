#include "memory.h"
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
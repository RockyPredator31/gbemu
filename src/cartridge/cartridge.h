#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MBC0 = 0,      // No MBC (32K ROM only)
    MBC1,
    MBC2,
    MBC3,
    MBC5,
    MBC_NONE = 0xFF
} MBC_Type;

typedef struct 
{
    // Bank Switching
    uint8_t rom_bank;
    uint8_t ram_bank;

    // MBC Status
    uint8_t mbc_type;        // z.B. 0x01 = MBC1, 0x03 = MBC1+RAM+Battery usw.

    bool     ram_enabled;        // RAM Zugriff freigeschaltet? (wichtig!)
    bool     has_battery;        // Für Save-Funktion später
    bool     has_rtc;            // MBC3 mit Uhr
    bool     has_rumble;         // Vibration bei manchen MBC5

    // Cartridge Titel
    char     title[17];

    uint8_t  rom_banks;          // Anzahl ROM-Banken (z.B. 64 = 1MB)
    uint8_t  ram_banks;          // Anzahl RAM-Banken

} Cartridge;

void cartridge_init(Cartridge* cart, Memory* memory);

#endif
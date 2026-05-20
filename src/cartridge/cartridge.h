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
    //ROM
    uint8_t* rom;
    size_t   rom_size;

    // External RAM
    uint8_t* eram;
    // eram ist hier die verwendbare Ram größe. Bei MBC2 ist es im ROM als 0 nagegeben, es sind aber 512 bytes vorhanden, aber als 0 angegeben. Liegt daran wie die hardware gebaut wurde, ist hier aber egal.
    // deswegen wird in dem fall MBC2 immer 512 bytes eram_size sein, egal was in rom[0x0149] steht.
    size_t   eram_size;     // 0 = kein RAM vorhanden

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

    // Header checksum
    uint8_t header_checksum;

} Cartridge;

void cartridge_init(Cartridge* cart, uint8_t* rom, size_t rom_size);

#endif
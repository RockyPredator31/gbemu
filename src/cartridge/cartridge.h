#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include "../memory/memory.h"

typedef void (*MBC_WriteFunc)(struct Cartridge* cart, uint16_t addr, uint8_t value);

typedef uint8_t (*MBC_ReadFunc)(struct Cartridge* cart, uint16_t addr);
typedef enum 
{
    MBC0 = 0,      // No MBC (32K ROM only)
    MBC1,
    MBC2,
    MBC3,
    MBC5,
    MBC_NONE = 0xFF
} MBC_Type;

typedef struct 
{
    uint8_t* rom;
    size_t   rom_size;      

    // Bank Switching
    uint8_t rom_bank;
    uint8_t ram_bank;

    uint8_t  rom_banks;          // Anzahl ROM-Banken (z.B. 64 = 1MB)
    uint8_t  ram_banks;          // Anzahl RAM-Banken

    uint8_t* eram;
    size_t eram_size;

    // MBC Status
    uint8_t mbc_type;        // z.B. 0x01 = MBC1, 0x03 = MBC1+RAM+Battery usw.

    bool     ram_enabled;        // RAM Zugriff freigeschaltet? (wichtig!)
    bool     has_battery;        // Für Save-Funktion später
    bool     has_rtc;            // MBC3 mit Uhr
    bool     has_rumble;         // Vibration bei manchen MBC5

    // Cartridge Titel
    char     title[17];

    MBC_WriteFunc write;    // Funktionszeiger für write
    MBC_ReadFunc read;      // Funktionszeiger für read

} Cartridge;

void cartridge_init(Cartridge* cart, uint8_t* rom, size_t rom_size);

void mbc1_write(Cartridge* cart, uint16_t addr, uint8_t value);
void mbc2_write(Cartridge* cart, uint16_t addr, uint8_t value);
void mbc3_write(Cartridge* cart, uint16_t addr, uint8_t value);
void mbc5_write(Cartridge* cart, uint16_t addr, uint8_t value);
void mbc0_write(Cartridge* cart, uint16_t addr, uint8_t value);

uint8_t mbc1_read(Cartridge* cart, uint16_t addr);
uint8_t mbc2_read(Cartridge* cart, uint16_t addr);
uint8_t mbc3_read(Cartridge* cart, uint16_t addr);
uint8_t mbc5_read(Cartridge* cart, uint16_t addr);
uint8_t mbc0_read(Cartridge* cart, uint16_t addr);

#endif
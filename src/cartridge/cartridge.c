#include "cartridge.h"
#include "../gb.h"
#include <string.h>
#include <stdio.h>

void cartridge_init(Cartridge* cart, uint8_t* rom, size_t rom_size)
{
    cart->rom = rom;
    cart->rom_size = rom_size;
    cart->rom_bank = 1;
    cart->ram_enabled = false;
    cart->ram_bank = 0;
    cart->header_checksum = cart->rom[0x014D];

    //Titel auslesen
    memcpy(cart->title, &cart->rom[0x0134], 16);
    cart->title[16] = '\0';

    // MBC Typ auslesen (Adresse 0x0147)
    uint8_t mbc_byte = cart->rom[0x0147];

    // Vereinfacht. Nicht alle mbc Typen.
    switch (mbc_byte) {
        case 0x00: cart->mbc_type = MBC0; break;
        case 0x01: case 0x02: case 0x03: cart->mbc_type = MBC1; break;
        case 0x05: case 0x06: cart->mbc_type = MBC2; break;
        case 0x0F: case 0x10: case 0x11: case 0x12: case 0x13: 
            cart->mbc_type = MBC3; break;
        case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: 
            cart->mbc_type = MBC5; break;
        default: 
            cart->mbc_type = MBC0;
            printf("Unbekannter MBC-Typ: 0x%02X\n", mbc_byte);
    }

    if(cart->mbc_type == MBC2)
    {
        cart->eram_size = 512;
    }else
    {
        uint8_t ram_code = cart->rom[0x0149];

        switch(ram_code)
        {
            case 0x00: cart->eram_size = 0;             break;
            case 0x02: cart->eram_size = 8 * 1024;      break;
            case 0x03: cart->eram_size = 32 * 1024;     break;
            case 0x04: cart->eram_size = 128 * 1024;    break;
            case 0x05: cart->eram_size = 64 * 1024;     break;
            default: cart->eram_size = 0;
        }
    }


    uint8_t checksum = 0;
    for (uint16_t address = 0x0134; address <= 0x014C; address++) {
        checksum = checksum - cart->rom[address] - 1;
    }

    printf("Cartridge: %s | MBC: %d | ROM: %zu KB | Header Checksum: %04X | Header computed Checksum: %04X\n", 
    cart->title, cart->mbc_type, cart->rom_size/1024, cart->header_checksum, checksum);

}
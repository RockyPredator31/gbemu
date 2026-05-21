#include "cartridge.h"
#include <string.h>
#include <stdio.h>

void cartridge_init(Cartridge* cart, uint8_t* rom, size_t rom_size)
{
    cart->rom = rom;
    cart->rom_size = rom_size;
    cart->rom_bank = 1;
    cart->ram_enabled = false;
    cart->ram_bank = 0;
    cart->eram = NULL;
    cart->eram_size = 0;
    cart->banking_mode = 0;

    //Titel auslesen
    memcpy(cart->title, &rom[0x0134], 16);
    cart->title[16] = '\0';

    // MBC Typ auslesen (Adresse 0x0147)
    uint8_t mbc_byte = rom[0x0147];

    // Vereinfachtes bestimmen des MBC-Typen
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


    // ERAM size bestimmen
    if(cart->mbc_type == MBC2)
    {
        cart->eram_size = 512;
    }else
    {
        uint8_t ram_code = rom[0x0149];

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

    // ERAM Allokieren
    if(cart->eram_size > 0)
    {
        cart->eram = (uint8_t*)malloc(cart->eram_size);
    }

    // read/write func pointer setzen
    switch(cart->mbc_type)
    {
        case MBC0:  
            cart->write = mbc0_write;  
            cart->read = mbc0_read;
            break;
        case MBC1:  
            cart->write = mbc1_write;  
            cart->read = mbc1_read;
            break;
        case MBC2:  
            cart->write = mbc2_write;  
            cart->read = mbc2_read;
            break;
        case MBC3:  
            cart->write = mbc3_write;  
            cart->read = mbc3_read;
            break;
        case MBC5:  
            cart->write = mbc5_write;  
            cart->read = mbc5_read;
            break;
        default:    
            cart->write = mbc0_write;  
            cart->read = mbc0_read;
            break;
    }

}

void mbc0_write(Cartridge* cart, uint16_t addr, uint8_t value)
{
    (void)cart;
    (void)addr;
    (void)value;
}

void mbc1_write(Cartridge* cart, uint16_t addr, uint8_t value)
{
     if (addr >= 0x0000 && addr <= 0x1FFF)           // RAM Enable
    {
        cart->ram_enabled = ((value & 0x0F) == 0x0A);
    }
    else if (addr >= 0x2000 && addr <= 0x3FFF)      // ROM Bank Number (untere 5 Bits)
    {
        uint8_t bank = value & 0x1F;                // nur die unteren 5 Bits
        if (bank == 0) bank = 1;                    // Bank 0 ist verboten hier

        cart->rom_bank = (cart->rom_bank & 0xE0) | bank;   // untere 5 Bits setzen
    }
    else if (addr >= 0x4000 && addr <= 0x5FFF)      // RAM Bank oder obere ROM Bits
    {
        if (cart->banking_mode == 0)                // Normal Mode (meistens benutzt)
        {
            cart->rom_bank = (cart->rom_bank & 0x1F) | ((value & 0x03) << 5);
        }
        else                                        // Advanced Banking Mode
        {
            cart->ram_bank = value & 0x03;
        }
    }
    else if (addr >= 0x6000 && addr <= 0x7FFF)      // Banking Mode Select
    {
        cart->banking_mode = value & 0x01;
    }
}

void mbc2_write(Cartridge* cart, uint16_t addr, uint8_t value)
{

}

void mbc3_write(Cartridge* cart, uint16_t addr, uint8_t value)
{

}

void mbc5_write(Cartridge* cart, uint16_t addr, uint8_t value)
{

}

uint8_t mbc0_read(Cartridge* cart, uint16_t addr)
{
     if (addr < cart->rom_size)
     {
         return cart->rom[addr];
     }
    return 0xFF;        
}

uint8_t mbc1_read(Cartridge* cart, uint16_t addr)
{
    if (addr <= 0x3FFF)                     // Bank 0 (immer fest)
    {
        return cart->rom[addr];
    }
    else if (addr <= 0x7FFF)                // Bank 1-N
    {
        uint32_t bank = cart->rom_bank;
        
        // Bank 0 kann hier nicht ausgewählt werden → wird zu Bank 1
        if (bank == 0) bank = 1;

        uint32_t offset = bank * 0x4000UL + (addr - 0x4000);

        if (offset < cart->rom_size)
            return cart->rom[offset];
    }

    return 0xFF;
}

uint8_t mbc2_read(Cartridge* cart, uint16_t addr)
{

}

uint8_t mbc3_read(Cartridge* cart, uint16_t addr)
{

}

uint8_t mbc5_read(Cartridge* cart, uint16_t addr)
{

}
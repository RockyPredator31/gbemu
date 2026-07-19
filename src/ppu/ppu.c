#include "ppu.h"
#include "../gb.h"

void ppu_init(PPU* ppu)
{
    ppu->scanline_counter = 0;
}

void ppu_update(GameBoy* gb, int32_t cycle)
{
    gb->ppu.scanline_counter += cycle;

    if(gb->ppu.scanline_counter >= 456)
    {
        gb->ppu.scanline_counter -= 456;

        if(gb->memory.io[0x44] < 144)
        {
            ppu_render_scanline(gb);
        }

        gb->memory.io[0x44]++;

        ppu_check_lyc(gb);
        
        if(gb->memory.io[0x44] == 144)
        {
            interrupt_request(gb, INTERRUPT_VBLANK);
        }else
        {
            if(gb->memory.io[0x44] > 153)
            {
                gb->memory.io[0x44] = 0;
                gb->ppu.frame_ready = true;
            }
        }
    }

    
}

void ppu_check_lyc(GameBoy* gb) {
    uint8_t ly = gb->memory.io[0x44]; // LY Register
    uint8_t lyc = gb->memory.io[0x45]; // LYC Register
    uint8_t stat = gb->memory.io[0x41]; // STAT Register

    if (ly == lyc) {
        stat |= (1 << 2);
        
        // If LYC int select set, request interrupt
        if (stat & (1 << 6)) {
            request_interrupt(gb, INTERRUPT_LCD);
        }
    } else {
        stat &= ~(1 << 2); // Reset LYC == LY Bit
    }

    gb->memory.io[0x41] = stat;
}

void ppu_render_scanline(GameBoy* gb)
{
    uint8_t LCDC_Register = gb->memory.io[0x40];
    uint8_t LY = gb->memory.io[0x44];
    bool BG_Window_Enabled = (LCDC_Register & 1); // BIT 0
    bool Sprite_Enabled = (LCDC_Register & (1 << 1)); // BIT 1
    bool OBJ_Size = (LCDC_Register & (1 << 2)); // BIT 2
    bool BG_Tile_Map = (LCDC_Register & (1 << 3)); // BIT 3
    bool Tile_Data_Area = (LCDC_Register & (1 << 4)); // BIT 4
    bool Window_Enabled = (LCDC_Register & (1 << 5)); // BIT 5
    bool Window_Map_Area = (LCDC_Register & (1 << 6)); // BIT 6



    if(LCDC_Register & (1 << 7))
    {
        // Turn screen off
        return;
    }

    // in DMG the Window_Enabled Bit is Overriten by Bit 0 if it is clear.
    if(gb->gbVersion == DMG){
        Window_Enabled = (Window_Enabled & BG_Window_Enabled);
    }

    

}
#include "interrupts.h"
#include "../gb.h"

void interrupt_handler(GameBoy* gb)
{
    uint8_t ie = gb->InterruptEnableRegister;
    uint8_t if_flag = gb->memory.io[0x0F];
    uint8_t pending = (ie & if_flag);

    if(pending == 0)
    {
        return;
    }

    if(gb->cpu.halted)
    {
        gb->cpu.halted = false;
    }

    if (!gb->cpu.ime) {
        return;
    }

    for(int i = 0; i < 5; i++)
    {
        if(pending & (1U << i))
        {   
            gb->cpu.ime = false;
            gb->memory.io[0x0F] &= ~(1U << i);

            cpu_push16(gb, gb->cpu.pc);

            gb->cpu.cycles += 20;
            
            switch(i)
            {
                case 0: // V-Blank
                    gb->cpu.pc = 0x0040; 
                    break; 
                case 1: // LCD STAT
                    gb->cpu.pc = 0x0048; 
                    break; 
                case 2: // Timer
                    gb->cpu.pc = 0x0050; 
                    break; 
                case 3: // Serial
                    gb->cpu.pc = 0x0058; 
                    break; 
                case 4: // Joypad
                    gb->cpu.pc = 0x0060; 
                    break; 
            }

            break;
        }
    }
}
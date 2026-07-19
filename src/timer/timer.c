#include "timer.h"
#include "../gb.h"

void timer_init(Timer* timer)
{
    timer->div_counter = 0;
}

void timer_update(GameBoy* gb, uint32_t cycles)
{
    uint16_t old_div = gb->timer.div_counter;
    gb->timer.div_counter += cycles;

    gb->memory.io[0x04] = (gb->timer.div_counter >> 8U); // Sets the DIV counter

    // Check if Timer is enabled
    uint8_t tac = gb->memory.io[0x07];
    if ((tac & (1U << 2U)) == 0) {
        return;
    }

    // depending on TAC determine the bit to check
    int bit_to_check = 0;
    switch (tac & 0x03) {
        case 0b00: // 4096 Hz
            bit_to_check = 9;  
            break; 
        case 0b01: // 262144 Hz
            bit_to_check = 3;  
            break; 
        case 0b10: // 65536 Hz
            bit_to_check = 5;  
            break; 
        case 0b11: // 16384 Hz
            bit_to_check = 7;  
            break; 
    }


    uint32_t ticks = (gb->timer.div_counter >> bit_to_check) - (old_div >> bit_to_check);

    while (ticks > 0) {
        uint8_t tima = gb->memory.io[0x05];
        // Check for Overflow
        if (tima == 0xFF) {
            gb->memory.io[0x05] = gb->memory.io[0x06]; 
            
            // Request Interrupt TIMA Overflow
            gb->memory.io[0x0F] |= (1U << 2U); 
        } else {
            gb->memory.io[0x05]++;
        }
        ticks--;
    }
}
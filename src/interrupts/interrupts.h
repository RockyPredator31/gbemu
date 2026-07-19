#ifndef INTERRUPTS_C
#define INTERRUPTS_C

#include <stdint.h>
#include <stdbool.h>

typedef struct GameBoy GameBoy;

typedef enum 
{
    INTERRUPT_VBLANK = 0,
    INTERRUPT_LCD,
    INTERRUPT_TIMER,
    INTERRUPT_SERIAL,
    INTERRUPT_JOYPAD
} INTERRUPT_BIT;

void interrupt_handler(GameBoy* gb);

void interrupt_request(GameBoy* gb, INTERRUPT_BIT bit );

#endif
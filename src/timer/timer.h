#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct GameBoy GameBoy;

typedef struct
{
    uint16_t div_counter;
}Timer;

void timer_init(Timer* timer);

void timer_update(GameBoy* gb, uint32_t cycle);


#endif
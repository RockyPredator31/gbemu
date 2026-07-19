#ifndef INTERRUPTS_C
#define INTERRUPTS_C

#include <stdint.h>
#include <stdbool.h>

typedef struct GameBoy GameBoy;

void interrupt_handler(GameBoy* gb);

#endif
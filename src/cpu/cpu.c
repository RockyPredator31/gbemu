#include "cpu.h"

void cpu_init(CPU* cpu, GB_Version gbv)
{
    switch (gbv)
    {
    case DMG:
        cpu->a = 0x01;
        cpu->f = 0x80; // z=1 | n=0 | h=0 | c=0
        cpu->b = 0x00;
        cpu->c = 0x13;
        cpu->d = 0x00;
        cpu->e = 0xD8;
        cpu->h = 0x01;
        cpu->l = 0x4D;
        cpu->pc = 0x0100;
        cpu->sp = 0xFFFE;

        cpu->halted = false;
        cpu->stopped = false;
        cpu->cycles = 0;
        break;
    
    default:
        break;
    }
}
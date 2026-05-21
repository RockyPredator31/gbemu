#include "memory.h"
#include <string.h>

void memory_init(Memory* memory)
{
    memset(memory, 0, sizeof(Memory));
}
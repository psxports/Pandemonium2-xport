#include "game.h"
#include <stdlib.h>

// Decode the constructor identifier still stored in legacy resource records
ObjectCallback::ValueHandler DecodeObjectFactory(unsigned int original_address)
{
    switch (original_address)
    {
        case 0x8002C608: return p2_func_8002c608;
        case 0x8002AFA8: return p2_func_8002afa8;
        default:
            fprintf(stderr, "P2: Undecoded resource constructor %08X\n", original_address);
            abort();
    }
}

#include "gameplay_input.h"
#include "psx.h"

GameplayInput gameplay_input = {0};

unsigned int GameplayInput::ApplyDemo()
{
    FUNCTION_MARKER(0x800276B4, "SLUS_005.78");
    const unsigned int source = demo_offset;
    unsigned int result = source + 4;
    if (source != 0)
    {
        demo_offset = result;
        result = r_u32(source);
        buttons = result;
    }
    return result;
}

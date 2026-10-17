#include "xport.h"
#include "random_generator.h"

RandomGenerator random_generator = {OriginalDefaults::RandomSeed};

unsigned int RandomGenerator::Next()
{
    FUNCTION_MARKER(0x80068198, "SLUS_005.78");
    const unsigned int high = (unsigned int)(((uint64)(seed) * 0x069C16BD) >> 32);
    const unsigned int quotient = (high + ((seed - high) >> 1)) >> 16;
    const unsigned int remainder = seed - quotient * 127773;
    seed = remainder * 16807 - quotient * 2836;
    // Preserve the original wrapped correction rather than substituting another generator
    if ((seed & 0x80000000u) != 0)
        seed -= 0x7FFFFFFF;
    else if (seed == 0)
        seed = 1;
    return seed;
}

unsigned int RandomGenerator::Reset()
{
    FUNCTION_MARKER(0x80068264, "SLUS_005.78");
    const unsigned int prev = seed;
    seed = 1;
    return prev;
}


#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H
#include <stddef.h>

#include "psx.h"

class RandomGenerator;

class RandomGenerator
{
public:
    uint32 seed;

    uint32 Next();
    uint32 Reset();
};

extern RandomGenerator random_generator;

#endif

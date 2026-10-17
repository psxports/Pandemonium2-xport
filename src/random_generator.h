#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <stddef.h>
#include "original_defaults.h"

class RandomGenerator
{
public:

    unsigned int Next();
    unsigned int Reset();

    unsigned int seed;
};

extern RandomGenerator random_generator;

#endif

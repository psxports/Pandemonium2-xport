#ifndef CONTROL_BINDINGS_H
#define CONTROL_BINDINGS_H
#include "original_defaults.h"
class ControlBindings
{
public:
    typedef unsigned char Scheme[4];
    const Scheme *active_scheme;
    unsigned char selected_index;
};
extern ControlBindings control_bindings;
#endif

#ifndef PASSWORD_H
#define PASSWORD_H

#include "psx.h"

extern "C"
{
uint32 DrawPassword(uint32 x, uint32 y, sint8 interactive);
uint32 DrawPasswordScreen(sint32 requested_mode, uint32 invalid, uint32 accepted);
uint32 GeneratePassword(void);
sint32 ApplyCheatPassword(void);
uint32 EditPassword(uint32 requested_mode);
}

#endif

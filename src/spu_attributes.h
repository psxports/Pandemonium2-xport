#ifndef SPU_ATTRIBUTES_H
#define SPU_ATTRIBUTES_H

#include "psx_spu.h"

extern "C"
{
    uint32 p2_sdk_8007101c(const SpuVoiceAttr *attributes);
    uint32 p2_sdk_80070dc4(const SpuVoiceAttr *attributes);
}

#endif

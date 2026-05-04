#include "Engine/Basics/BPEEnvironment.h"

#if BPE_TARGET == BPE_TARGET_PS3
#include "stdint.h"
#include "cell/spurs.h"
#include "spu_printf.h"
#endif

#define BPE_PAD_16(x) ((x + 15) & ~15)

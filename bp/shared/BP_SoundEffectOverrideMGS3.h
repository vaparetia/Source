//----------------------------------------------------------------------------
// BP_SoundEffectOverride.h
//----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "iop/psq.h"

//----------------------------------------------------------------------------

void BP_InitSoundEffectOverride();
PSQ_IWAV * BP_GetWavOverrideIWAV( int i );
void * BP_Override_SoundEffect( int sound_code );

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------

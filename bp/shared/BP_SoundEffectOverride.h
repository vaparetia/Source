//----------------------------------------------------------------------------
// BP_SoundEffectOverride.h
//----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------

void BP_InitSoundEffectOverride();
void * BP_GetWavOverrideAddress( unsigned int wavCode, int * pOutSize );
int BP_Override_SoundEffect( int sound_code, void ** const ppOutTrackAddr0, void ** const ppOutTrackAddr1 );

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------

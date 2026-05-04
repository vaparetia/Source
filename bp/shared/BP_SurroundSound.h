//----------------------------------------------------------------------------
// BP_SurroundSound.h
//----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
extern void BP_SurroundSoundCalcVols( float * const outSpeakerVols, const int speakerCount, const float pan3d, const float vol3d );
//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

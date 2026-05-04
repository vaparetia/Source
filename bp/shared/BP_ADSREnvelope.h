//----------------------------------------------------------------------------
// BP_ADSREnvelope.h
// Multiplatform PS2 ADSR envelope emulation support
//----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------

typedef enum EADSRState
{
   kADSRState_Off,
   kADSRState_Attack,
   kADSRState_Decay,
   kADSRState_Sustain,
   kADSRState_Release
} EADSRState;

typedef struct _SADSREnvelopeState
{
   unsigned short    mADSR1;  //PS2 voice register
   unsigned short    mADSR2;  //PS2 voice register
   EADSRState        mADSRState;
   uint64            mInitEnvelopeTimeUs;       //global microseconds when envelope was started
   int               mCurrEnvelopeTimeElapsedMs;//current playback time in ms since key on
   float             mEnvelopeScale;   //current envelope volume scale
} SADSREnvelopeState;

extern void BP_InitADSREnvelope( SADSREnvelopeState * const pEnvelope );
extern void BP_UpdateADSREnvelope( SADSREnvelopeState * const pEnvelope );

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

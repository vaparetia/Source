//----------------------------------------------------------------------------
// CSoundSystem.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//----------------------------------------------------------------------------

#if BPE_TARGET != BPE_TARGET_RVL
#define BPE_FMOD
#endif

//----------------------------------------------------------------------------

namespace FMOD
{
   class Channel;
   class ChannelGroup;
   class DSP;
   class System;
   class EventSystem;
   class EventCategory;
};

class CVector3;
class IScriptManager;

//----------------------------------------------------------------------------

enum ESoundType
{
   kSoundType_2d,
   kSoundType_3d
};

enum EChannelGroup
{
   kCG_All,

   kCG_Game,
   kCG_Music,
   kCG_UI,
   
   kCG_Count,
   kCG_Invalid = -1
};

//----------------------------------------------------------------------------

class CSoundSystem
{
   typedef std::map<std::string, FMOD::Channel*> TGlobalSoundMap;

public:
   static const int kNumChannels = 32;

public:
   ENGINE_API explicit CSoundSystem();
   ENGINE_API ~CSoundSystem();

   ENGINE_API static CSoundSystem * GlobalInstance();

   ENGINE_API void SetListenerInfo(CVector3 const & listenerPosition,
                                   CVector3 const & listenerVelocity,
                                   CVector3 const & listenerForward,
                                   CVector3 const & listenerUp);

   ENGINE_API void Update(real32 const deltaTime);

   ENGINE_API void SetDesiredLPFilter(real32 const frequencyFactor, real32 const fadeTime);

   ENGINE_API void ResetChannelGroups();

   ENGINE_API void AddGlobalSound(std::string const & name, FMOD::Channel* pChannel);
   ENGINE_API void RemoveGlobalSound(std::string const & name, FMOD::Channel* pChannel);
   ENGINE_API FMOD::Channel* GetGlobalSound(std::string const & name) const;
   ENGINE_API void StopGlobalSounds();

   FMOD::System*        FModSystem()       { return mpSystem; }
   FMOD::EventSystem*   FModEventSystem()  { return mpEventSystem; }
   FMOD::ChannelGroup*  ChannelGroup(EChannelGroup id) { return mChannelGroups[id]; }
   ENGINE_API FMOD::EventCategory* EventCategory(EChannelGroup id);


   // Hack to workaround lack of construct from memory for FEV/FSB
   // Basically assumes that any FMOD file loads will come from this 'file'.
   // We will have some very bad things happen if any async loading occurs.
   
   ENGINE_API void SetFMODMemoryFile(void * pMemory, int const size);
   ENGINE_API void ResetFMODMemoryFile();

private:
   FMOD::System*        mpSystem;
   FMOD::EventSystem*   mpEventSystem;
   FMOD::DSP*           mpLowPassFilter;
   bpe::reserved_vector<FMOD::ChannelGroup*, kCG_Count> mChannelGroups;
   
   TGlobalSoundMap mGlobalSounds;

   real32         mLPFilterFreqFactorStart;
   real32         mLPFilterFreqFactorDesired;
   real32         mLPFilterFreqFactorT;
   real32         mLPFilterFreqFactorFadeTime;
};

//----------------------------------------------------------------------------



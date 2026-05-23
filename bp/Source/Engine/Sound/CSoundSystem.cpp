//----------------------------------------------------------------------------
// CSoundSystem.cpp
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CSoundSystem.h"

//----------------------------------------------------------------------------

#include "Engine/Script/CScriptVar.h"

#if BPE_TARGET == BPE_TARGET_PS3
#include "Engine/System/CSPURSManager.h"
#include "fmodps3.h"
extern char _binary_fmodex_spurs_stripped_elf_start[];
extern char _binary_fmodex_spurs_mpeg_stripped_elf_start[];
#endif

#ifdef BPE_FMOD
#include "fmod.hpp"
#include "fmod_event.h"
#endif

//----------------------------------------------------------------------------

#include "Engine/Math/MathUtils.h"

//----------------------------------------------------------------------------

static CSoundSystem* sSoundSystem = NULL;
CScriptVar gSoundSystemDiagnostic("snd_showDiagnostic", false);

//----------------------------------------------------------------------------

CSoundSystem::CSoundSystem()
:  mpSystem(NULL)
,  mpLowPassFilter(NULL)
,  mLPFilterFreqFactorStart(1.0f)
,  mLPFilterFreqFactorDesired(1.0f)
,  mLPFilterFreqFactorT(1.0f)
,  mLPFilterFreqFactorFadeTime(0.0f)
{
#ifdef BPE_FMOD
   FMOD_RESULT res = EventSystem_Create( &mpEventSystem );
   if( res == FMOD_OK )
   {
      // Prevent from loading all .dlls in current path
      mpEventSystem->setPluginPath(NULL);
      mpEventSystem->getSystemObject(&mpSystem);
      res = mpSystem->setSoftwareChannels(kNumChannels); // Force software channel count to match that of virtual channels. Fixes stuttering on PS3 caused by >100% DSP usage.

#if BPE_TARGET == BPE_TARGET_PS3

      FMOD_PS3_EXTRADRIVERDATA extradriverdata;
      memset(&extradriverdata, 0, sizeof(FMOD_PS3_EXTRADRIVERDATA));

      extradriverdata.spurs                               = gpSpursManager->mpSPURS_FMOD.get();                /* Using SPURS */
      extradriverdata.spu_mixer_elfname_or_spursdata      = _binary_fmodex_spurs_stripped_elf_start;     /* Pointer to SPURS data */
      extradriverdata.spu_streamer_elfname_or_spursdata   = _binary_fmodex_spurs_mpeg_stripped_elf_start;    /* Pointer to SPURS data  */
      extradriverdata.spu_priority_mixer                  = 16;  /* Default     THIS WILL BE IGNORED */
      extradriverdata.spu_priority_streamer               = 200; /* Default     THIS WILL BE IGNORED */
      extradriverdata.spu_priority_at3                    = 200;                  /* Default */

      extradriverdata.force5point1     = 0;   // Depreciated
      extradriverdata.attenuateDDLFE   = 0;   // Depreciated

      res = mpEventSystem->init( kNumChannels, FMOD_INIT_NORMAL, (void*) &extradriverdata );
#else

   // use this to debug audio output in 5.1 surround mode
   #if 0
      mpSystem->setOutput(FMOD_OUTPUTTYPE_WAVWRITER);
      mpSystem->setSpeakerMode(FMOD_SPEAKERMODE_5POINT1);
      res = mpEventSystem->init( kNumChannels, FMOD_INIT_NORMAL, "c:\\output.wav" );
   #else
      res = mpEventSystem->init( kNumChannels, FMOD_INIT_NORMAL, 0 );
   #endif   

#endif
      BPE_VERIFY( res == FMOD_OK, false, "couldn't initialize fmod system" );
   }

   BPE_VERIFY( sSoundSystem == NULL, false, "Only one sound system can be initialized" );
   sSoundSystem = this;

   // Disabled low pass filter since we're not currently using it, might give us back some performance.
   /*
   mpSystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &mpLowPassFilter);
   mpSystem->addDSP(mpLowPassFilter);
   */

   mChannelGroups.resize(kCG_Count, NULL);

   for( int i = 0; i < kCG_Count; ++i )
   {
      FMOD::ChannelGroup* pChannelGroup = NULL;
      if( i == kCG_All )
      {
         res = mpSystem->getMasterChannelGroup(&pChannelGroup);
      }
      else
      {
         res = mpSystem->createChannelGroup(NULL, &pChannelGroup);
         
         // add this channel group as a child to the master channel group
         if( pChannelGroup )
            mChannelGroups[kCG_All]->addGroup(pChannelGroup);
      }

      BPE_VERIFY( res == FMOD_OK, false, "couldn't create channel group" );
   
      mChannelGroups[i] = pChannelGroup;
   }

   ResetChannelGroups();

   FMOD_REVERB_PROPERTIES reverb = FMOD_PRESET_AUDITORIUM;
   mpSystem->setReverbProperties(&reverb);
#endif
}

//----------------------------------------------------------------------------

CSoundSystem::~CSoundSystem()
{
   sSoundSystem = NULL;

#ifdef BPE_FMOD
   if( mpEventSystem )
   {
      for( int i = 1; i < kCG_Count; ++i )
      {
         mChannelGroups[i]->release();
      }
      //mpSystem->close();
      //mpSystem->release();
      // I'm assuming this shuts down the regular FMOD system cleanly.
      // All eventgroups/projects must be freed before calling this to prevent crash
      // UPDATE:
      // Ah, looks like if a project has been reloaded then FMOD crashes.
      // Something to look into later.

      // Also, looks like FMOD can also cause a shutdown hang if the game has crashed.
      // I've commented out the event system shutdown to workaround this issue as this is 
      // particularly annoying when remote debugging in Windows Vista.      
      // We should look into a better method for handling Win32 crashes.
      //mpEventSystem->unload();
      //mpEventSystem->release();
      mpEventSystem = NULL;
      mpSystem = NULL;
   }
#endif
}

//----------------------------------------------------------------------------

CSoundSystem * CSoundSystem::GlobalInstance()
{
   return sSoundSystem;
}

//----------------------------------------------------------------------------

void CSoundSystem::SetListenerInfo(CVector3 const & listenerPosition,
                                   CVector3 const & listenerVelocity,
                                   CVector3 const & listenerForward,
                                   CVector3 const & listenerUp)
{
#ifdef BPE_FMOD
   mpSystem->set3DListenerAttributes(0, 
                                     reinterpret_cast<FMOD_VECTOR const *>(&listenerPosition),
                                     reinterpret_cast<FMOD_VECTOR const *>(&listenerVelocity),
                                     reinterpret_cast<FMOD_VECTOR const *>(&listenerForward),
                                     reinterpret_cast<FMOD_VECTOR const *>(&listenerUp));
#endif
}

//----------------------------------------------------------------------------

void CSoundSystem::Update(real32 const deltaTime)
{
#ifdef BPE_FMOD
   mpEventSystem->update();

   if( gSoundSystemDiagnostic.GetBool() )
   {
      int totalChannels;
      mpSystem->getSoftwareChannels(&totalChannels);   
      
      int channelsPlaying;
      mpSystem->getChannelsPlaying(&channelsPlaying);
    
      real32 dsp, stream, update, total;
      mpSystem->getCPUUsage(&dsp, &stream, &update, &total);
      bpe_console_printf("SoundSystem: Channels: [%d/%d] CPU: %.3f%% DSP, %.3f%% Stream, %.3f%% Update, %.3f%% Total", channelsPlaying, totalChannels, dsp, stream, update, total);
   }


   if( mLPFilterFreqFactorFadeTime > 0.0f )
   {
      mLPFilterFreqFactorT = bpe::min_val(1.0f, mLPFilterFreqFactorT + (deltaTime / mLPFilterFreqFactorFadeTime));
   }
   else
   {
      mLPFilterFreqFactorT = 1.0f;
   }
   
   real32 currentLPFilterFactor = MathUtils::Lerp(mLPFilterFreqFactorT, mLPFilterFreqFactorStart, mLPFilterFreqFactorDesired);

   if( mpLowPassFilter )
   {
      mpLowPassFilter->setParameter(FMOD_DSP_LOWPASS_CUTOFF, currentLPFilterFactor * 22000.0f);
   }
#endif
}

//----------------------------------------------------------------------------

void CSoundSystem::SetDesiredLPFilter(real32 const frequencyFactor, real32 const fadeTime)
{
   mLPFilterFreqFactorStart = MathUtils::Lerp(mLPFilterFreqFactorT, mLPFilterFreqFactorStart, mLPFilterFreqFactorDesired);
   mLPFilterFreqFactorDesired = frequencyFactor;
   mLPFilterFreqFactorT = 0.0f;
   mLPFilterFreqFactorFadeTime = fadeTime; 
}

//----------------------------------------------------------------------------

void CSoundSystem::ResetChannelGroups()
{
#ifdef BPE_FMOD
   for( int i = 0; i < kCG_Count; ++i )
   {
      EChannelGroup const group = static_cast<EChannelGroup>(i);
      FMOD::ChannelGroup * pChannelGroup = ChannelGroup(group);
      pChannelGroup->setPitch(1.0f);
      pChannelGroup->setVolume(1.0f);

      FMOD::EventCategory * pEventCategory = EventCategory(group);
      if (pEventCategory)
      {
         //pEventCategory->setPitch(1.0f);   // Reseting this causes all events to become 4x pitch. Investigate later.
         pEventCategory->setVolume(1.0f);
      }
   }
#endif
}

//----------------------------------------------------------------------------
FMOD::EventCategory* CSoundSystem::EventCategory(EChannelGroup id)
{
   // Event system
   char *pCategoryName = NULL;
   switch (id)
   {
   case kCG_All:
      pCategoryName = "master";
      break;
   case kCG_Game:
      pCategoryName = "game";
      break;
   case kCG_Music:
      pCategoryName = "music";
      break;
   case kCG_UI:
      pCategoryName = "ui";
      break;
   }

   if (!pCategoryName)
   {
      return NULL;
   }

#ifdef BPE_FMOD
   FMOD::EventCategory * pCategory = NULL;
   FMOD_RESULT result = CSoundSystem::GlobalInstance()->FModEventSystem()->getCategory(pCategoryName, &pCategory);
   if ((result == FMOD_OK) && pCategory)
   {
      return pCategory;
   }
#endif

   return NULL;
}

//----------------------------------------------------------------------------

void CSoundSystem::AddGlobalSound(std::string const & name, FMOD::Channel* pChannel)
{
#ifdef BPE_FMOD
   mGlobalSounds.insert(TGlobalSoundMap::value_type(name, pChannel));
#endif
}

//----------------------------------------------------------------------------

void CSoundSystem::RemoveGlobalSound(std::string const & name, FMOD::Channel* pChannel)
{
#ifdef BPE_FMOD
   mGlobalSounds.erase(mGlobalSounds.find(name));
#endif
}

//----------------------------------------------------------------------------

FMOD::Channel* CSoundSystem::GetGlobalSound(std::string const & name) const
{
   TGlobalSoundMap::const_iterator found = mGlobalSounds.find(name);
   
   if( found != mGlobalSounds.end() )
      return found->second;

   return NULL;
}

//----------------------------------------------------------------------------

void CSoundSystem::StopGlobalSounds()
{
#ifdef BPE_FMOD
   foreach(TGlobalSoundMap::value_type & current, mGlobalSounds)
   {
      current.second->stop();
   }
#endif
   mGlobalSounds.clear();
}

//----------------------------------------------------------------------------
static uint8 * spFMODMemoryFile = NULL;
static int     sFMODMemoryFileSize = 0;
static int     sFMODMemoryFileReadPosition = 0;

#ifdef BPE_FMOD

static FMOD_RESULT F_CALLBACK _FMOD_open(const char *name, int unicode, unsigned int *filesize, void **handle, void **userdata)
{
   static int skFakeHandle =  0xDEADBEEF;
   *handle = (int*) skFakeHandle;
   *filesize = sFMODMemoryFileSize;
   return FMOD_OK;
}

static FMOD_RESULT F_CALLBACK _FMOD_close(void *handle, void *userdata)
{
   return FMOD_OK;
}

static FMOD_RESULT F_CALLBACK _FMOD_read(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata)
{
   if (!handle)
   {
      return FMOD_ERR_INVALID_PARAM;
   }

   if (bytesread)
   {
      BPE_VERIFY((sFMODMemoryFileReadPosition + sizebytes) <= sFMODMemoryFileSize, false, "Size too large in _FMOD_read");
      *bytesread = sizebytes;
      memcpy(buffer, spFMODMemoryFile + sFMODMemoryFileReadPosition, sizebytes);
      sFMODMemoryFileReadPosition += sizebytes;      
   }

   return FMOD_OK;
}

static FMOD_RESULT F_CALLBACK _FMOD_seek(void *handle, unsigned int pos, void *userdata)
{
   sFMODMemoryFileReadPosition = pos; 
   return FMOD_OK;
}

#endif

//----------------------------------------------------------------------------

void CSoundSystem::SetFMODMemoryFile(void * pMemory, int const size)
{
   spFMODMemoryFile = (uint8*)pMemory;
   sFMODMemoryFileSize = size;
   sFMODMemoryFileReadPosition = 0;
#ifdef BPE_FMOD
   FMOD_RESULT result = mpSystem->setFileSystem(_FMOD_open, _FMOD_close, _FMOD_read, _FMOD_seek, 1);

#endif
}

//----------------------------------------------------------------------------

void CSoundSystem::ResetFMODMemoryFile()
{
   spFMODMemoryFile = NULL;
   sFMODMemoryFileSize = 0;
   sFMODMemoryFileReadPosition = 0;
#ifdef BPE_FMOD
   FMOD_RESULT result = mpSystem->setFileSystem(NULL, NULL, NULL, NULL, 2048);
#endif
}

//----------------------------------------------------------------------------


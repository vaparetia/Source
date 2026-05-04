#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "BP_SoundSupportVTA_Helpers.h"

#include <kernel/threadmgr.h>
#include <libsysmodule.h>

#if SULPHA_ENABLED == 1
#include <sulpha.h>
#include <ngs/sulpha_ngs.h>
#endif

#include "BP_SoundSupportVTA.h"
#include "BP_FileSupport.h"
#include "BP_Debug.h"
#include "Engine/System/VTAThreadPriorities.h"
#include "BP_Memory.h"


//----------------------------------------------------------------------------
extern unsigned char gBP_SpuMemory[];
extern int gReverbModePS2;
extern int gReverbDepthPS2;

NVitaSound::SNgsState gNgsState;
bool gs_bIsRunning = true;
int gBP_EnableAudioReverb = 1;

#if SULPHA_ENABLED == 1
static void *gspSulphaMem;
static void *gspSulphaNgsMem;
#endif

extern "C"
{
   void bp_sddrv_loop_iteration();
   int BP_FRAMES_PER_SEC();
   int BP_IsDebugPaused();
};

//----------------------------------------------------------------------------
void NVitaSound::game_audio_update()
{
   double nowFrameTime = CStopWatch::gGlobalTime.GetElapsedTime();

   BP_Debug_PushCPUMarker("GameAudioUpdate");

#if MGS_VERSION==3
   const double kSoundUpdateCycleMs = 10;
   const int kMaxSoundUpdatesPerEndFrame = 3;
#elif MGS_VERSION==2
   double kSoundUpdateCycleMs = 2.5;//gBP_AudioUpdateInterval;
   const int kMaxSoundUpdatesPerEndFrame = BP_FRAMES_PER_SEC()/3; //3 50/60hz frames worth of updates
#endif

   static double sSoundExecutedMs = 0;
   double elapsedMs = nowFrameTime * 1000.0;
   int soundUpdates = 0;

   if( elapsedMs > sSoundExecutedMs )
   {
      //Force at least one sound update.
      bp_sddrv_loop_iteration();
      sSoundExecutedMs += kSoundUpdateCycleMs;
      ++soundUpdates;

      //Loop to perform the rest.
      for( ; sSoundExecutedMs < elapsedMs; sSoundExecutedMs += kSoundUpdateCycleMs )
      {
         if( soundUpdates++ == kMaxSoundUpdatesPerEndFrame )
         {
            //Keep a cap on the number of sound updates, as DG_TickCount can go up by a few dozen increments
            //at a time while the debugger is stepping.
            sSoundExecutedMs = elapsedMs;
            break;
         }
         bp_sddrv_loop_iteration();
      }
   }
   else
   {
#if MGS_VERSION==3
      //Do force at least one sound update! (MGS3 only-- they spam the sound command queue with volume
      //and rotation commands so it is necessary to clear them out.  In MGS2 we will mess up the midi music timing doing this.)
      bp_sddrv_loop_iteration();
#else
      // Force one sound update if debug paused so that sound commands are flushed
      if( BP_IsDebugPaused() )
      {
         bp_sddrv_loop_iteration();
      }
#endif
   }

   BP_Debug_PopCPUMarker();
}

//----------------------------------------------------------------------------
int NVitaSound::audio_update_thread_main(SceSize args, void *argc)
{
   int returnCode = SCE_OK;
   int currBuffer = 0;
   short outputData[2][NVitaSound::kNgsSysGranularity * 2];

   while ((volatile bool)gs_bIsRunning)
   {
      // Process game audio updates before updating the ngs system. This ensures audio state from the game stays in 
      // sync with the hardware audio. (e.g. voices are key'd on at the same time when necessary).
      game_audio_update();

      // Pulse the NGS sound system.
      returnCode = sceNgsSystemUpdate(gNgsState.mNgsSysHandle);
      BPE_VERIFY(returnCode == SCE_OK, false, "Error invoking NGS system update.");

      // Get latest processed data...
      returnCode = sceNgsVoiceGetStateData(gNgsState.mMasterVoice, SCE_NGS_MASTER_BUSS_OUTPUT_MODULE, 
         outputData[currBuffer], sizeof(short) * NVitaSound::kNgsSysGranularity * 2);
      
      BPE_VERIFY(returnCode == SCE_OK, false, "Error getting latest processed data.");

      /* Uncomment to send random noise to the audio output device for testing purposes..
      for (int i = 0; i < SYS_GRANULARITY * 2; i++)
         outputData[currBuffer][i] = frnd() * 32768;
      */

#if SULPHA_ENABLED == 1
      update_sulpha();
#endif

      // Send processed audio data to hardware.
      sceAudioOutOutput(gNgsState.mAudioHardwarePort, outputData[currBuffer]);
      currBuffer ^= 1;
   }

   sceKernelExitDeleteThread(returnCode);
   return returnCode;
}

//----------------------------------------------------------------------------
void NVitaSound::init_audio()
{
   int returnCode = SCE_OK;
   SceNgsSystemInitParams initParams;
   size_t size;

   // initialize NGS lib
   returnCode = sceSysmoduleLoadModule(SCE_SYSMODULE_NGS);
   BPE_VERIFY(returnCode == SCE_OK, false, "Failed to initialize NGS sys module.");

   initParams.nMaxRacks = 13;
   initParams.nMaxVoices = (BP_DOSTREAM_COUNT * BP_DOSTREAM_TRACK_COUNT) + 
      (NVitaSound::kNumSpuVoiceChannels * NVitaSound::kNumSpuVoices) + 3; // +2 == master voice, reverb voice and standalone voice
   initParams.nGranularity = NVitaSound::kNgsSysGranularity;
   initParams.nSampleRate = NVitaSound::kNgsSysSampleRate;
   initParams.nMaxModules = NVitaSound::kNumNgsModules;

   // Determine memory requirement for NGS system
   returnCode = sceNgsSystemGetRequiredMemorySize(&initParams, &size);
   BPE_VERIFY(returnCode == SCE_OK, false, "Failed to get the required memory size for NGS.");

   gNgsState.mpNgsSysMem = BP_Memory_Alloc( size, SCE_NGS_MEMORY_ALIGN_SIZE, kMT_Permanent, kMC_Sound );
   BPE_VERIFY(gNgsState.mpNgsSysMem != NULL, false, "Failed to allocate NGS sys memory.");

   returnCode = sceNgsSystemInit(gNgsState.mpNgsSysMem, size, &initParams, &gNgsState.mNgsSysHandle);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Failed to initialize NGS system.");
   sceNgsSystemSetParamErrorCallback(gNgsState.mNgsSysHandle, NVitaSound::ngs_system_error_callback);

   sceNgsSystemSetFlags(gNgsState.mNgsSysHandle, SCE_NGS_SYSTEM_FLAG_NO_MODULE_PARAM_CHECKING);

   // Create the master voice or master rack.
   const struct SceNgsVoiceDefinition* pMasterBussVoiceDef = sceNgsVoiceDefGetMasterBuss();
   SceNgsRackDescription rackDesc;
   SceNgsBufferInfo bufferInfo;

   rackDesc.nChannelsPerVoice = 2; // Always output stereo audio.
   rackDesc.nVoices = 1; // Only need one voice as it's the "output" voice.
   rackDesc.pVoiceDefn = pMasterBussVoiceDef;
   rackDesc.nMaxPatchesPerInput = (BP_DOSTREAM_COUNT * BP_DOSTREAM_TRACK_COUNT) + 
      (NVitaSound::kNumSpuVoiceChannels * NVitaSound::kNumSpuVoices) + 2; // 1 for the reverb voice and 1 for single wav voice.
   rackDesc.nPatchesPerOutput = 0;
   returnCode = sceNgsRackGetRequiredMemorySize(gNgsState.mNgsSysHandle, &rackDesc, &bufferInfo.size);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Unable to determine memory size for master rack.");

   gNgsState.mpMasterRackMem = BP_Memory_Alloc( bufferInfo.size, SCE_NGS_MEMORY_ALIGN_SIZE, kMT_Permanent, kMC_Sound );
   BPE_VERIFY(gNgsState.mpMasterRackMem != NULL, false, "Error allocating memory for master rack.");
   memset(gNgsState.mpMasterRackMem, 0, bufferInfo.size);

   bufferInfo.data = gNgsState.mpMasterRackMem;
   returnCode = sceNgsRackInit(gNgsState.mNgsSysHandle, &bufferInfo, &rackDesc, &gNgsState.mMasterRack);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Error initializing master rack.");

   // Get the master voice from the master rack. The master voice is then connected to the other racks via patches.
   returnCode = sceNgsRackGetVoiceHandle(gNgsState.mMasterRack, 0, &gNgsState.mMasterVoice);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Error retrieving handle to master voice.");

   NVitaSound::init_reverb();

   for (int i = 0; i < BP_DOSTREAM_COUNT; i++)
   {
      gNgsState.mAudioStreams[i] = new CAudioStream();
      gNgsState.mAudioStreams[i]->Initialize();
   }

   NVitaSound::init_spu_voices();

   NVitaSound::init_standlone_voice();

   // Start the master voice so it is always available to play audio as other voices send data to it via their patches.
   returnCode = sceNgsVoicePlay(gNgsState.mMasterVoice);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Error starting the master voice.");

   // Open the actual device audio port and create a thread to pull data and keep the port fed as audio samples are 
   // made available on the master voice.
   gNgsState.mAudioHardwarePort = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, NVitaSound::kNgsSysGranularity, 
      NVitaSound::kNgsSysSampleRate, SCE_AUDIO_OUT_PARAM_FORMAT_S16_STEREO);
   BPE_VERIFY(gNgsState.mAudioHardwarePort >= 0, false, "Could not open audio hardware port.");

   gNgsState.mAudioOutThread = sceKernelCreateThread("Audio Update Thread", NVitaSound::audio_update_thread_main,
      NVtaThreadPriorities::kPrioritySoundHWUpdate, NVitaSound::kAudioThreadStackSize, 0, SCE_KERNEL_CPU_MASK_USER_ALL, NULL);
   BPE_VERIFY(gNgsState.mAudioOutThread >= 0, false, "Unable to create audio output thread.");

   returnCode = sceKernelStartThread(gNgsState.mAudioOutThread, 0, NULL);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to start audio out thread.");
}

//----------------------------------------------------------------------------
void NVitaSound::shutdown_audio()
{
   gs_bIsRunning = false;
   
   for (int spuChannel = 0; spuChannel < kNumSpuVoiceChannels; spuChannel++)
   {
      for (int currVoice = 0; currVoice < kNumSpuVoices; currVoice++)
      {
         delete gNgsState.mpSpuVoiceState[spuChannel][currVoice];
      }
   }

   for (int i = 0; i < BP_DOSTREAM_COUNT; i++)
      delete gNgsState.mAudioStreams[i];
}

//----------------------------------------------------------------------------
void NVitaSound::ngs_system_error_callback(const SceNgsCallbackInfo *pCallbackInfo)
{
   BPE_VERIFY(false, false, "Received an error in the NGS callback.");
}

//----------------------------------------------------------------------------
void NVitaSound::get_vag_endpoints(SSpuVoiceState* pSpuVoice)
{
   pSpuVoice->mBlockBegin = 0;
   pSpuVoice->mBlockEnd = 0;
   pSpuVoice->mBlockLoop = false;
   pSpuVoice->mUseReplacementVagBuffer = false;

   unsigned int begin = pSpuVoice->mpMemoryStream ? 0 : pSpuVoice->mSSA;
   unsigned int end = pSpuVoice->mpMemoryStream ? pSpuVoice->mMemoryStreamSize : BP_SPU_MEMORY_SIZE;
   for (unsigned int i = begin; i < end; i += 16)
   {
      const unsigned char* pVagPacket = pSpuVoice->mpMemoryStream ? &pSpuVoice->mpMemoryStream[i] : &gBP_SpuMemory[i];
      unsigned char flag = pVagPacket[1];

      // If we reach a packet with a 7 (all three flags are set) then expand it into two packets compatible with the Vita adpcm decoder.
      // The first packet marks the beginning of the loop, the second packet marks the end.
      if (flag == 7)
      {
         pSpuVoice->mBlockLoop = true;
         pSpuVoice->mUseReplacementVagBuffer = true;
         pSpuVoice->mBlockBegin = i;

         // Block is is not 32 bytes because the original data was only 1 packet which is used as a position indicator 
         // to the game.
         pSpuVoice->mBlockEnd = i + 16;

         memcpy(&pSpuVoice->mReplacementVag[0], pVagPacket, 16);
         memcpy(&pSpuVoice->mReplacementVag[16], pVagPacket, 16);
         pSpuVoice->mReplacementVag[1] = 6; // 6 indicates the beginning of a loop
         pSpuVoice->mReplacementVag[17] = 3; // 3 indicates the end of a loop

         break;
      }
      else
      {
         if( flag & VAG_LOOPBLOCK )
         {
            pSpuVoice->mBlockLoop = true;
         }

         if( flag & VAG_LOOPSTART )
         {
            pSpuVoice->mBlockBegin = i;
         }

         if( flag & VAG_LOOPEND )
         {
            pSpuVoice->mBlockEnd = i + 16;
            break;
         }
      }
   }

   //LSAX register was manually set.  Here's our block begin.
   if (pSpuVoice->mLSAX)
   {
      pSpuVoice->mBlockBegin = pSpuVoice->mLSAX;
      pSpuVoice->mBlockLoop = true;
      pSpuVoice->mLSAX = 0;
   }

   BPE_VERIFY(pSpuVoice->mBlockEnd, false, "Invalid vag endpoint.");
}

//----------------------------------------------------------------------------
void NVitaSound::apply_standalone_voice_reverb()
{
   SceNgsPatchSetupInfo patchSetupInfo;
   SceNgsPatchRouteInfo patchRouteInfo;

   // Always route the voice to the master voice for the dry signal, and if reverb is enabled route to the reverb 
   // voice as well for the wet mix.
   int returnCode = SCE_OK;

   BPE_VERIFY(gNgsState.mStandaloneWavVoice, false, "Invalid voice.");
   BPE_VERIFY(gNgsState.mReverbVoice, false, "Invalid reverb voice.");
   BPE_VERIFY(!gNgsState.mStandAloneWavVoicePatchHandleWet, false, "Patch handle was already created.");

   patchSetupInfo.hVoiceSource = gNgsState.mStandaloneWavVoice;
   patchSetupInfo.nSourceOutputIndex = 0;
   patchSetupInfo.nSourceOutputSubIndex = SCE_NGS_VOICE_PATCH_AUTO_SUBINDEX;
   patchSetupInfo.hVoiceDestination = gNgsState.mReverbVoice;
   patchSetupInfo.nTargetInputIndex = 0;

   returnCode = sceNgsPatchCreateRouting(&patchSetupInfo, &gNgsState.mStandAloneWavVoicePatchHandleWet);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to create patch route for voice.");

   returnCode = sceNgsPatchGetInfo(gNgsState.mStandAloneWavVoicePatchHandleWet, &patchRouteInfo, NULL);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get patch info. for voice.");

   patchRouteInfo.vols.m[0][0] = 1.0f;
   patchRouteInfo.vols.m[0][1] = 1.0f;

   returnCode = sceNgsVoicePatchSetVolumesMatrix(gNgsState.mStandAloneWavVoicePatchHandleWet, &patchRouteInfo.vols);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to set volume matrix on voice.");
}

//----------------------------------------------------------------------------
void NVitaSound::init_standlone_voice()
{
   struct SceNgsVoiceDefinition const* pVoiceDef = sceNgsVoiceDefGetTemplate1();

   SceNgsRackDescription rackDesc;
   SceNgsBufferInfo bufferInfo;
   int returnCode = SCE_OK;

   memset(&rackDesc, 0, sizeof(rackDesc));

   rackDesc.nChannelsPerVoice = 1; // each channel of an spu voice is mono
   rackDesc.nVoices = 1;
   rackDesc.pVoiceDefn = pVoiceDef;
   rackDesc.nMaxPatchesPerInput = 0;
   rackDesc.nPatchesPerOutput = 2;

   returnCode = sceNgsRackGetRequiredMemorySize(gNgsState.mNgsSysHandle, &rackDesc, &bufferInfo.size);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Unable to determine memory size for audio rack.");

   gNgsState.mpStandaloneWavVoiceRackMem = BP_Memory_Alloc(bufferInfo.size, SCE_NGS_MEMORY_ALIGN_SIZE, kMT_Permanent, kMC_Sound);
   BPE_VERIFY(gNgsState.mpStandaloneWavVoiceRackMem, false, "Error allocating buffer for voice rack.");
   memset(gNgsState.mpStandaloneWavVoiceRackMem, 0, bufferInfo.size);

   bufferInfo.data = gNgsState.mpStandaloneWavVoiceRackMem;
   returnCode = sceNgsRackInit(gNgsState.mNgsSysHandle, &bufferInfo, &rackDesc, &gNgsState.mStandaloneWavVoiceRack);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Error initializing audio rack.");

   SceNgsPatchRouteInfo patchRouteInfo;
   returnCode = sceNgsRackGetVoiceHandle(gNgsState.mStandaloneWavVoiceRack, 0, &gNgsState.mStandaloneWavVoice);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get voice handle from audio rack.");

   // Always patch the standalone voice to the master voice for the dry signal. If reverb is enabled on the voice also
   // patch to the reverb voice.
   SceNgsPatchSetupInfo patchSetupInfo;

   patchSetupInfo.hVoiceSource = gNgsState.mStandaloneWavVoice;
   patchSetupInfo.nSourceOutputIndex = 0;
   patchSetupInfo.nSourceOutputSubIndex = SCE_NGS_VOICE_PATCH_AUTO_SUBINDEX;
   patchSetupInfo.hVoiceDestination = gNgsState.mMasterVoice;
   patchSetupInfo.nTargetInputIndex = 0;

   returnCode = sceNgsPatchCreateRouting(&patchSetupInfo, &gNgsState.mStandAloneWavVoicePatchHandleDry);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to create patch route for voice.");

   apply_standalone_voice_reverb();

   // Set initial volume on dry signal...
   returnCode = sceNgsPatchGetInfo(gNgsState.mStandAloneWavVoicePatchHandleDry, &patchRouteInfo, NULL);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get patch info. for voice.");

   patchRouteInfo.vols.m[0][0] = 1.0f; // left to left
   patchRouteInfo.vols.m[0][1] = 1.0f; // right to right

   returnCode = sceNgsVoicePatchSetVolumesMatrix(gNgsState.mStandAloneWavVoicePatchHandleDry, &patchRouteInfo.vols);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to set volume matrix on voice.");

   // Initialize the playback parameters of the voice..
   returnCode = sceNgsVoiceLockParams(gNgsState.mStandaloneWavVoice, SCE_NGS_VOICE_T1_PCM_PLAYER, 
      SCE_NGS_PLAYER_PARAMS_STRUCT_ID, &bufferInfo);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to lock voice parameters.");

   memset(bufferInfo.data, 0, bufferInfo.size);

   SceNgsPlayerParams* pPcmParams = (SceNgsPlayerParams*)bufferInfo.data;
   pPcmParams->desc.id = SCE_NGS_PLAYER_PARAMS_STRUCT_ID;
   pPcmParams->desc.size = sizeof(SceNgsPlayerParams);
   pPcmParams->nChannels = 1;
   pPcmParams->nType = SCE_NGS_PLAYER_TYPE_PCM;

   returnCode = sceNgsVoiceUnlockParams(gNgsState.mStandaloneWavVoice, SCE_NGS_VOICE_T1_PCM_PLAYER);

   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to unlock voice parameters on direct output stream.");
}

//----------------------------------------------------------------------------
void NVitaSound::init_spu_voices()
{
   struct SceNgsVoiceDefinition const* pVoiceDef = sceNgsVoiceDefGetTemplate1();

   // We use two rack for the spu voices. One rack for channel 0 and one rack for channel 1
   SceNgsRackDescription rackDesc;
   SceNgsBufferInfo bufferInfo;
   int returnCode = SCE_OK;

   for (int spuChannel = 0; spuChannel < kNumSpuVoiceChannels; spuChannel++)
   {
      memset(&rackDesc, 0, sizeof(rackDesc));

      rackDesc.nChannelsPerVoice = 1; // each channel of an spu voice is mono
      rackDesc.nVoices = kNumSpuVoices;
      rackDesc.pVoiceDefn = pVoiceDef;
      rackDesc.nMaxPatchesPerInput = 0;
      rackDesc.nPatchesPerOutput = 2;

      returnCode = sceNgsRackGetRequiredMemorySize(gNgsState.mNgsSysHandle, &rackDesc, &bufferInfo.size);
      BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Unable to determine memory size for SPU rack.");

      gNgsState.mpSpuVoiceRackMem[spuChannel] = BP_Memory_Alloc( bufferInfo.size, SCE_NGS_MEMORY_ALIGN_SIZE, kMT_Permanent, kMC_Sound );
      BPE_VERIFY(gNgsState.mpSpuVoiceRackMem[spuChannel] != NULL, false, "Error allocating memory for SPU channel rack.");
      memset(gNgsState.mpSpuVoiceRackMem[spuChannel], 0, bufferInfo.size);

      bufferInfo.data = gNgsState.mpSpuVoiceRackMem[spuChannel];
      returnCode = sceNgsRackInit(gNgsState.mNgsSysHandle, &bufferInfo, &rackDesc, &gNgsState.mSpuVoiceRack[spuChannel]);
      BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Error initializing SPU channel rack.");

      for (int currVoice = 0; currVoice < kNumSpuVoices; currVoice++)
      {
         SceNgsPatchRouteInfo patchRouteInfo;

         SSpuVoiceState* pSpuVoice = new SSpuVoiceState();
         gNgsState.mpSpuVoiceState[spuChannel][currVoice] = pSpuVoice;

         returnCode = sceNgsRackGetVoiceHandle(gNgsState.mSpuVoiceRack[spuChannel], currVoice, &pSpuVoice->mVoiceHandle);
         BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get voice handle for SPU rack.");

         // Always patch the SPU voices to the master voice for the dry signal. If reverb is enabled on the voice also
         // patch to the reverb voice.
         SceNgsPatchSetupInfo patchSetupInfo;
         
         patchSetupInfo.hVoiceSource = pSpuVoice->mVoiceHandle;
         patchSetupInfo.nSourceOutputIndex = 0;
         patchSetupInfo.nSourceOutputSubIndex = SCE_NGS_VOICE_PATCH_AUTO_SUBINDEX;
         patchSetupInfo.hVoiceDestination = gNgsState.mMasterVoice;
         patchSetupInfo.nTargetInputIndex = 0;

         returnCode = sceNgsPatchCreateRouting(&patchSetupInfo, &pSpuVoice->mPatchHandleDry);
         BPE_VERIFY(returnCode == SCE_OK, false, "Unable to create patch route for SPU channel voice.");

         apply_spu_voice_reverb(pSpuVoice);

         // Set initial volume on dry signal...
         returnCode = sceNgsPatchGetInfo(pSpuVoice->mPatchHandleDry, &patchRouteInfo, NULL);
         BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get patch info. for SPU channel voice.");

         patchRouteInfo.vols.m[0][0] = 1.0f; // left to left
         patchRouteInfo.vols.m[0][1] = 1.0f; // right to right

         returnCode = sceNgsVoicePatchSetVolumesMatrix(pSpuVoice->mPatchHandleDry, &patchRouteInfo.vols);
         BPE_VERIFY(returnCode == SCE_OK, false, "Unable to set volume matrix on SPU channel voice.");

         // Initialize the playback parameters of the voice..
         pSpuVoice->mParamsBlock.params.desc.id = SCE_NGS_PLAYER_PARAMS_STRUCT_ID;
         pSpuVoice->mParamsBlock.params.desc.size = sizeof(SceNgsPlayerParams);
         pSpuVoice->mParamsBlock.params.nChannels = 1;
         pSpuVoice->mParamsBlock.params.nType = SCE_NGS_PLAYER_TYPE_ADPCM;

         pSpuVoice->mParamsBlock.moduleInfo.moduleId = SCE_NGS_VOICE_T1_PCM_PLAYER;
         pSpuVoice->mParamsBlock.moduleInfo.chan = SCE_NGS_MODULE_ALL_CHANNELS;

         SceInt32 errorCount = 0;
         int returnCode = sceNgsVoiceSetParamsBlock(pSpuVoice->mVoiceHandle, 
            (SceNgsModuleParamHeader *)&pSpuVoice->mParamsBlock, 
            sizeof(SceNgsPlayerParamsBlock), &errorCount);

         BPE_VERIFY(returnCode == SCE_OK, false, "Error setting voice parameters blocks.");
      }
   }
}

//----------------------------------------------------------------------------
void NVitaSound::init_reverb()
{
   const struct SceNgsVoiceDefinition *pReverbBussVoiceDef = sceNgsVoiceDefGetReverbBuss();
   BPE_VERIFY(pReverbBussVoiceDef, false, "Unable to get reverb voice template");

   gNgsState.mpCustomReverbParams = new SReverbParams();

   SceNgsRackDescription rackDesc;
   SceNgsBufferInfo bufferInfo;

   rackDesc.nChannelsPerVoice = 2;
   rackDesc.nVoices = 1;
   rackDesc.pVoiceDefn = pReverbBussVoiceDef;
   rackDesc.nMaxPatchesPerInput = (NVitaSound::kNumSpuVoices * NVitaSound::kNumSpuVoiceChannels) + 
      (BP_DOSTREAM_COUNT * BP_DOSTREAM_TRACK_COUNT);
   rackDesc.nPatchesPerOutput = 1;

   int returnCode = sceNgsRackGetRequiredMemorySize(gNgsState.mNgsSysHandle, &rackDesc, &bufferInfo.size);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get memory requirements for reverb rack.");

   gNgsState.mpReverbRackMem = BP_Memory_Alloc( bufferInfo.size, SCE_NGS_MEMORY_ALIGN_SIZE, kMT_Permanent, kMC_Sound );
   BPE_VERIFY(gNgsState.mpReverbRackMem, false, "Unable to allocate memory for reverb rack.");
   memset(gNgsState.mpReverbRackMem, 0, bufferInfo.size);

   bufferInfo.data = gNgsState.mpReverbRackMem;
   returnCode = sceNgsRackInit(gNgsState.mNgsSysHandle, &bufferInfo, &rackDesc, &gNgsState.mReverbRack);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Unable to initialize reverb rack.");

   returnCode = sceNgsRackGetVoiceHandle(gNgsState.mReverbRack, 0, &gNgsState.mReverbVoice);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get reverb voice.");

   // Patch the reverb voice to the master voice. This way, any voice that wants reverb simply patches itself to the 
   // reverb voice. If a voice does not want a reverb effect applied, the voice bypasses the reverb voice and patches 
   // itself directly to the master voice.
   SceNgsPatchSetupInfo patchInfo;
   SceNgsPatchRouteInfo patchRouteInfo;

   patchInfo.hVoiceSource = gNgsState.mReverbVoice;
   patchInfo.nSourceOutputIndex = 0;
   patchInfo.nSourceOutputSubIndex = SCE_NGS_VOICE_PATCH_AUTO_SUBINDEX;
   patchInfo.hVoiceDestination = gNgsState.mMasterVoice;
   patchInfo.nTargetInputIndex = 0;

   returnCode = sceNgsPatchCreateRouting(&patchInfo, &gNgsState.mReverbPatch);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to create patch route for SPU channel voice.");

   returnCode = sceNgsPatchGetInfo(gNgsState.mReverbPatch, &patchRouteInfo, NULL);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get patch info. for SPU channel voice.");

   patchRouteInfo.vols.m[0][0] = 1.0f; // left to left
   patchRouteInfo.vols.m[0][1] = 0.0f; // left to right
   patchRouteInfo.vols.m[1][0] = 0.0f; // right to left
   patchRouteInfo.vols.m[1][1] = 1.0f; // right to right

   returnCode = sceNgsVoicePatchSetVolumesMatrix(gNgsState.mReverbPatch, &patchRouteInfo.vols);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to set volume matrix on SPU channel voice.");

   // Just as we do with the master voice, tell the reverb voice to play and forget about it.
   returnCode = sceNgsVoicePlay(gNgsState.mReverbVoice);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to start reverb voice.");
}

//----------------------------------------------------------------------------
int NVitaSound::get_spu_stream_byte_pos(SSpuVoiceState* const pSpuVoice)
{
   int returnCode = SCE_OK;

   SceNgsVoiceInfo info;
   sceNgsVoiceGetInfo(pSpuVoice->mVoiceHandle, &info);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get voice info.");

   int bufferPos = 0;
   if (!(info.uVoiceState & SCE_NGS_VOICE_STATE_ACTIVE) || (info.uVoiceState & SCE_NGS_VOICE_STATE_PENDING))
   {
      bufferPos = pSpuVoice->mSSA;
   }
   else
   {
      SceNgsPlayerStates voiceState;
      returnCode = sceNgsVoiceGetStateData(pSpuVoice->mVoiceHandle, SCE_NGS_VOICE_T1_PCM_PLAYER, &voiceState, 
         sizeof(SceNgsPlayerStates));
      BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get voice state.");

      // For spu streams we need to determine what buffer is playing and take the starting address of the buffer into
      // account.      
      if (voiceState.nCurrentBuffer == 0)
      {
         // If we are using the replacement buffer (which is 32 bytes) mod the position by 16 in order to maintain
         // the same byte offset of the original data which was 16 bytes.
         if (pSpuVoice->mUseReplacementVagBuffer)
         {
            bufferPos = pSpuVoice->mSSA + (voiceState.nCurrentBytePositionInBuffer % 16);
         }
         else
         {
            if (pSpuVoice->mpMemoryStream)
            {
               bufferPos = voiceState.nCurrentBytePositionInBuffer;
            }
            else
            {
               bufferPos = pSpuVoice->mSSA + voiceState.nCurrentBytePositionInBuffer;
            }
            
         }
      }
      else
      {
         BPE_VERIFY(voiceState.nCurrentBuffer == 1, false, "Should only be playing from the second buffer.");
         bufferPos = pSpuVoice->mBlockBegin + voiceState.nCurrentBytePositionInBuffer;
      }
   }

   return bufferPos;
}

//----------------------------------------------------------------------------
void NVitaSound::apply_spu_voice_volume(SSpuVoiceState* const pSpuVoice, bool simulateEnvelopes)
{
   SceNgsPatchRouteInfo patchInfo;
   sceNgsPatchGetInfo(pSpuVoice->mPatchHandleDry, &patchInfo, NULL);

   float muteVolume = pSpuVoice->mpMemoryStream ? pSpuVoice->mMemoryStreamMuteVolume : 1;
   EADSRState prevADSRState = pSpuVoice->mADSREnvelope.mADSRState;
   
   if(prevADSRState != kADSRState_Off)
   {
      if (simulateEnvelopes)
      {
         BP_UpdateADSREnvelope(&pSpuVoice->mADSREnvelope);
      }
   }

   patchInfo.vols.m[0][0] = pSpuVoice->mVolumeL * muteVolume * pSpuVoice->mADSREnvelope.mEnvelopeScale;
   patchInfo.vols.m[0][1] = pSpuVoice->mVolumeR * muteVolume * pSpuVoice->mADSREnvelope.mEnvelopeScale;

   int returnCode = sceNgsVoicePatchSetVolumesMatrix(pSpuVoice->mPatchHandleDry, &patchInfo.vols);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to set voice volumes.");

   NVitaSound::apply_spu_voice_reverb(pSpuVoice);

   // Full stop, don't key off the voice since we've already simulated the key off envelope processing.
   bool bKeyOffAfterRelease = (prevADSRState != kADSRState_Off) && (pSpuVoice->mADSREnvelope.mADSRState == kADSRState_Off);
   if (bKeyOffAfterRelease)
   {
      sceNgsVoiceKeyOff(pSpuVoice->mVoiceHandle);
   }
}

//----------------------------------------------------------------------------
void NVitaSound::apply_spu_voice_reverb(SSpuVoiceState* const pSpuVoice)
{
   SceNgsPatchSetupInfo patchSetupInfo;
   SceNgsPatchRouteInfo patchRouteInfo;

   // Always route the voice to the master voice for the dry signal, and if reverb is enabled route to the reverb 
   // voice as well for the wet mix.
   bool bReverbEnabled = (pSpuVoice->mReverbEnabledR && gBP_EnableAudioReverb);
   float muteVolume = pSpuVoice->mpMemoryStream ? pSpuVoice->mMemoryStreamMuteVolume : 1;
   int returnCode = SCE_OK;

   // If reverb enabled patch the voice to the reverb voice in addition to already being patched to the master voice.
   if (bReverbEnabled)
   {
      if (!pSpuVoice->mPatchHandleWet) // If already connected to the reverb voice no need to re-patch it just update volume
      {
         patchSetupInfo.hVoiceSource = pSpuVoice->mVoiceHandle;
         patchSetupInfo.nSourceOutputIndex = 0;
         patchSetupInfo.nSourceOutputSubIndex = SCE_NGS_VOICE_PATCH_AUTO_SUBINDEX;
         patchSetupInfo.hVoiceDestination = gNgsState.mReverbVoice;
         patchSetupInfo.nTargetInputIndex = 0;

         returnCode = sceNgsPatchCreateRouting(&patchSetupInfo, &pSpuVoice->mPatchHandleWet);
         BPE_VERIFY(returnCode == SCE_OK, false, "Unable to create patch route for SPU channel voice.");
      }

      returnCode = sceNgsPatchGetInfo(pSpuVoice->mPatchHandleWet, &patchRouteInfo, NULL);
      BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get patch info. for SPU channel voice.");

      patchRouteInfo.vols.m[0][0] = muteVolume * pSpuVoice->mVolumeL * pSpuVoice->mADSREnvelope.mEnvelopeScale;
      patchRouteInfo.vols.m[0][1] = muteVolume * pSpuVoice->mVolumeR * pSpuVoice->mADSREnvelope.mEnvelopeScale;

      returnCode = sceNgsVoicePatchSetVolumesMatrix(pSpuVoice->mPatchHandleWet, &patchRouteInfo.vols);
      BPE_VERIFY(returnCode == SCE_OK, false, "Unable to set volume matrix on SPU channel voice.");
   }
   else
   {
      // Remove the patch to the reverb voice.
      if (pSpuVoice->mPatchHandleWet)
      {
         returnCode = sceNgsPatchRemoveRouting(pSpuVoice->mPatchHandleWet);
         BPE_VERIFY(returnCode == SCE_OK, false, "Error removing patch routing.");
         pSpuVoice->mPatchHandleWet = 0;
      }
   }
}

//----------------------------------------------------------------------------
void NVitaSound::key_on_spu_voice_stream(SSpuVoiceState* pSpuVoice)
{
   SceNgsBufferInfo bufferInfo;
   SceNgsVoiceInfo voiceInfo;
   int returnCode = SCE_OK;
   
   sceNgsVoiceGetInfo(pSpuVoice->mVoiceHandle, &voiceInfo);
   if (voiceInfo.uVoiceState != SCE_NGS_VOICE_STATE_AVAILABLE)
      sceNgsVoiceKill(pSpuVoice->mVoiceHandle);

   returnCode = sceNgsVoiceInit(pSpuVoice->mVoiceHandle, NULL, SCE_NGS_VOICE_INIT_BASE);
   BPE_VERIFY(returnCode == SCE_OK, false, "Error initializing voice before playback.");

   get_vag_endpoints(pSpuVoice);
   BP_InitADSREnvelope(&pSpuVoice->mADSREnvelope);

   SceNgsPlayerParams* pPcmParams = &pSpuVoice->mParamsBlock.params;

   pPcmParams->fPlaybackScalar = 1.0f;
   pPcmParams->nLeadInSamples = 0;
   pPcmParams->nLimitNumberOfSamplesPlayed = 0;
   pPcmParams->nStartBuffer = 0;
   pPcmParams->nStartByte = 0;
   pPcmParams->nChannelMap[0] = SCE_NGS_PLAYER_LEFT_CHANNEL;
   pPcmParams->nChannelMap[1] = SCE_NGS_PLAYER_LEFT_CHANNEL;

   if (pSpuVoice->mBlockLoop)
   {
      BPE_VERIFY(!pSpuVoice->mpMemoryStream, false, "Memory streams should not have loops.");

      if (pSpuVoice->mUseReplacementVagBuffer)
      {
         pPcmParams->buffs[0].pBuffer = (void*)(&pSpuVoice->mReplacementVag[0]);
         pPcmParams->buffs[0].nNumBytes = sizeof(pSpuVoice->mReplacementVag);
         pPcmParams->buffs[0].nNextBuff = SCE_NGS_PLAYER_NO_NEXT_BUFFER;
         pPcmParams->buffs[0].nLoopCount = SCE_NGS_PLAYER_LOOP_CONTINUOUS;
      }
      else
      {
         unsigned int sb_size = pSpuVoice->mBlockEnd - pSpuVoice->mSSA;
         unsigned int loop_size = pSpuVoice->mBlockEnd - pSpuVoice->mBlockBegin;

         pPcmParams->buffs[0].pBuffer = (void*)(gBP_SpuMemory + pSpuVoice->mSSA);
         pPcmParams->buffs[0].nNumBytes = sb_size;
         pPcmParams->buffs[0].nNextBuff = 1;
         pPcmParams->buffs[0].nLoopCount = 0;

         pPcmParams->buffs[1].pBuffer = (void*)(gBP_SpuMemory + pSpuVoice->mBlockBegin);
         pPcmParams->buffs[1].nNumBytes = loop_size;
         pPcmParams->buffs[1].nNextBuff = SCE_NGS_PLAYER_NO_NEXT_BUFFER;
         pPcmParams->buffs[1].nLoopCount = SCE_NGS_PLAYER_LOOP_CONTINUOUS;
      }
   }
   else
   {
      pPcmParams->buffs[0].nNextBuff = SCE_NGS_PLAYER_NO_NEXT_BUFFER;
      pPcmParams->buffs[0].nLoopCount = 0;

      if (!pSpuVoice->mpMemoryStream)
      {
         pPcmParams->buffs[0].pBuffer = (void*)(&gBP_SpuMemory[0] + pSpuVoice->mSSA);
         pPcmParams->buffs[0].nNumBytes = pSpuVoice->mBlockEnd - pSpuVoice->mSSA;
      }
      else
      {
         pPcmParams->buffs[0].pBuffer = (void*)(pSpuVoice->mpMemoryStream);
         pPcmParams->buffs[0].nNumBytes = pSpuVoice->mMemoryStreamSize;
      }
   }

   SceInt32 errorCount = 0;
   returnCode = sceNgsVoiceSetParamsBlock(pSpuVoice->mVoiceHandle,
      (SceNgsModuleParamHeader *)&pSpuVoice->mParamsBlock, 
      sizeof(SceNgsPlayerParamsBlock), &errorCount);

   BPE_VERIFY(returnCode == SCE_OK, false, "Error setting parameter block on voice.");

   returnCode = sceNgsVoicePlay(pSpuVoice->mVoiceHandle);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to play SPU stream.");

   apply_spu_voice_volume(pSpuVoice, true);
}

//----------------------------------------------------------------------------
void NVitaSound::apply_spu_voice_keyoff(SSpuVoiceState* pSpuVoice, float deltaTime)
{
}

//----------------------------------------------------------------------------
void NVitaSound::apply_spu_pitch(SSpuVoiceState* const pSpuVoice)
{
   float playBackScaler = 1.0f;
   if (pSpuVoice->mPitch > 192000.0f)
   {
      playBackScaler = pSpuVoice->mPitch / 192000.0f;
      pSpuVoice->mPitch = 192000.0f;
   }

   pSpuVoice->mParamsBlock.params.fPlaybackFrequency = pSpuVoice->mPitch;
   pSpuVoice->mParamsBlock.params.fPlaybackScalar = playBackScaler;

   SceInt32 errorCount = 0;
   int returnCode = sceNgsVoiceSetParamsBlock(pSpuVoice->mVoiceHandle,
      (SceNgsModuleParamHeader *)&pSpuVoice->mParamsBlock, 
      sizeof(SceNgsPlayerParamsBlock), &errorCount);

   BPE_VERIFY(returnCode == SCE_OK, false, "Error setting parameter block on voice.");
}

//----------------------------------------------------------------------------
void NVitaSound::apply_system_reverb()
{
   extern float gBP_MasterReverbStrength;
   extern int gBP_EnableAudioReverb;
   extern int gBP_OverriddenVitaReverbMode;
   extern int gBP_UseCustomReverbSettings;

   // Integer to Vita reverb preset.
   static int skVitaReverbModes[] =
   {
      SCE_NGS_REVERB_PRESET_PADDED_CELL,           
      SCE_NGS_REVERB_PRESET_ROOM,                  
      SCE_NGS_REVERB_PRESET_BATHROOM,              
      SCE_NGS_REVERB_PRESET_LIVING_ROOM,           
      SCE_NGS_REVERB_PRESET_STONE_ROOM,            
      SCE_NGS_REVERB_PRESET_AUDITORIUM,            
      SCE_NGS_REVERB_PRESET_CONCERT_HALL,          
      SCE_NGS_REVERB_PRESET_CAVE,                  
      SCE_NGS_REVERB_PRESET_ARENA,                 
      SCE_NGS_REVERB_PRESET_HANGAR,                
      SCE_NGS_REVERB_PRESET_CARPETED_HALLWAY,      
      SCE_NGS_REVERB_PRESET_HALLWAY,               
      SCE_NGS_REVERB_PRESET_STONE_CORRIDOR,        
      SCE_NGS_REVERB_PRESET_ALLEY,                 
      SCE_NGS_REVERB_PRESET_FOREST,                
      SCE_NGS_REVERB_PRESET_CITY,                  
      SCE_NGS_REVERB_PRESET_MOUNTAINS,             
      SCE_NGS_REVERB_PRESET_QUARRY,                
      SCE_NGS_REVERB_PRESET_PLAIN,                 
      SCE_NGS_REVERB_PRESET_PARKING_LOT,           
      SCE_NGS_REVERB_PRESET_UNDERWATER,            
      SCE_NGS_REVERB_PRESET_SMALL_ROOM,            
      SCE_NGS_REVERB_PRESET_MEDIUM_ROOM,           
      SCE_NGS_REVERB_PRESET_LARGE_ROOM,            
      SCE_NGS_REVERB_PRESET_MEDIUM_HALL,           
      SCE_NGS_REVERB_PRESET_LARGE_HALL,            
   };                                              

   SceNgsBufferInfo bufferInfo;
   int returnCode = SCE_OK;

   returnCode = sceNgsVoiceLockParams(gNgsState.mReverbVoice, SCE_NGS_REVERB_VOICE_REVERB_MODULE, 
      SCE_NGS_REVERB_PARAMS_STRUCT_ID, &bufferInfo);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to lock reverb voice.");


   if (gBP_OverriddenVitaReverbMode > 0)
   {
      returnCode = sceNgsModuleGetPreset(gNgsState.mNgsSysHandle, SCE_NGS_REVERB_ID, skVitaReverbModes[gBP_OverriddenVitaReverbMode - 1], 
         &bufferInfo);
   }
   else
   {
      // Pass in the global so it can still be adjusted with the debug menu if necessary.
#if MGS_VERSION == 3
      get_reverb_params_MGS3(&bufferInfo, &gBP_MasterReverbStrength);
#elif MGS_VERSION == 2
      get_reverb_params_MGS2(&bufferInfo, &gBP_MasterReverbStrength);
#else
#error Wrong MGS version.
#endif
   }
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get reverb preset.");

   SceNgsReverbParams* pReverbParams = (SceNgsReverbParams*)bufferInfo.data;
   if (gBP_UseCustomReverbSettings)
   {
      pReverbParams->fRoom = gNgsState.mpCustomReverbParams->mRoom;
      pReverbParams->fRoomHF = gNgsState.mpCustomReverbParams->mRoomHF;
      pReverbParams->fDecayTime = gNgsState.mpCustomReverbParams->mDecayTime;
      pReverbParams->fDecayHFRatio = gNgsState.mpCustomReverbParams->mDecayHFRatio;
      pReverbParams->fReflections = gNgsState.mpCustomReverbParams->mReflections;
      pReverbParams->fReflectionsDelay = gNgsState.mpCustomReverbParams->mReflectionsDelay;
      pReverbParams->fReverb = gNgsState.mpCustomReverbParams->mReverb;
      pReverbParams->fReverbDelay = gNgsState.mpCustomReverbParams->mReverbDelay;
      pReverbParams->fDiffusion = gNgsState.mpCustomReverbParams->mDiffusion;
      pReverbParams->fDensity = gNgsState.mpCustomReverbParams->mDensity;
      pReverbParams->fHFReference = gNgsState.mpCustomReverbParams->mHFReference;
      pReverbParams->fEarlyReflectionScalar = gNgsState.mpCustomReverbParams->mEarlyReflectionScalar;
      pReverbParams->fLFReference = gNgsState.mpCustomReverbParams->mLFReference;
      pReverbParams->fRoomLF = gNgsState.mpCustomReverbParams->mRoomLF;
      pReverbParams->fDryMB = gNgsState.mpCustomReverbParams->mDryMB;
   }
   else
   {
      gNgsState.mpCustomReverbParams->mRoom = pReverbParams->fRoom;
      gNgsState.mpCustomReverbParams->mRoomHF = pReverbParams->fRoomHF;
      gNgsState.mpCustomReverbParams->mDecayTime = pReverbParams->fDecayTime;
      gNgsState.mpCustomReverbParams->mDecayHFRatio = pReverbParams->fDecayHFRatio;
      gNgsState.mpCustomReverbParams->mReflections = pReverbParams->fReflections;
      gNgsState.mpCustomReverbParams->mReflectionsDelay = pReverbParams->fReflectionsDelay;
      gNgsState.mpCustomReverbParams->mReverb = pReverbParams->fReverb;
      gNgsState.mpCustomReverbParams->mReverbDelay = pReverbParams->fReverbDelay;
      gNgsState.mpCustomReverbParams->mDiffusion = pReverbParams->fDiffusion;
      gNgsState.mpCustomReverbParams->mDensity = pReverbParams->fDensity;
      gNgsState.mpCustomReverbParams->mHFReference = pReverbParams->fHFReference;
      gNgsState.mpCustomReverbParams->mEarlyReflectionScalar = pReverbParams->fEarlyReflectionScalar;
      gNgsState.mpCustomReverbParams->mLFReference = pReverbParams->fLFReference;
      gNgsState.mpCustomReverbParams->mRoomLF = pReverbParams->fRoomLF;
      gNgsState.mpCustomReverbParams->mDryMB = pReverbParams->fDryMB;
   }

   returnCode = sceNgsVoiceUnlockParams(gNgsState.mReverbVoice, SCE_NGS_REVERB_VOICE_REVERB_MODULE);
   if (returnCode != SCE_OK)
   {
      char msgBuf[128];
      sceNgsVoiceGetParamsOutOfRange(gNgsState.mReverbVoice, SCE_NGS_REVERB_VOICE_REVERB_MODULE, msgBuf);
      BPE_VERIFY(returnCode == SCE_OK, false, "Unable to unlock voice parameters on direct output stream.");
   }

   SceNgsPatchRouteInfo patchRouteInfo;
   returnCode = sceNgsPatchGetInfo(gNgsState.mReverbPatch, &patchRouteInfo, NULL);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get patch info. for SPU channel voice.");

   float ps2Depth = ((float)gReverbDepthPS2 / 0x7fff) * gBP_MasterReverbStrength;

   if (gBP_EnableAudioReverb)
   {
      patchRouteInfo.vols.m[0][0] = ps2Depth; 
      patchRouteInfo.vols.m[0][1] = 0.0f;
      patchRouteInfo.vols.m[1][0] = 0.0f; 
      patchRouteInfo.vols.m[1][1] = ps2Depth;
   }
   else
   {
      patchRouteInfo.vols.m[0][0] = 0.0f;
      patchRouteInfo.vols.m[0][1] = 0.0f;
      patchRouteInfo.vols.m[1][0] = 0.0f;
      patchRouteInfo.vols.m[1][1] = 0.0f;
   }

   returnCode = sceNgsVoicePatchSetVolumesMatrix(gNgsState.mReverbPatch, &patchRouteInfo.vols);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to set volume matrix on SPU channel voice.");
}

//----------------------------------------------------------------------------
void NVitaSound::get_reverb_params_MGS2(SceNgsBufferInfo* pReverb, float* pStrength)
{
   int returnCode = SCE_OK;

   // For MGS2 we always use the stone room preset and adjust its decay time and strength based on feedback from KP.
   returnCode = sceNgsModuleGetPreset(gNgsState.mNgsSysHandle, SCE_NGS_REVERB_ID, SCE_NGS_REVERB_PRESET_STONE_ROOM, 
      pReverb);
   BPE_VERIFY(returnCode == SCE_OK, false, "Error getting reverb preset.");

   SceNgsReverbParams* pReverbParams = (SceNgsReverbParams*)pReverb->data;
   pReverbParams->fDecayTime = 1.7f;

   *pStrength = 2.2f;
}

//----------------------------------------------------------------------------
void NVitaSound::get_reverb_params_MGS3(SceNgsBufferInfo* pReverb, float* pStrength)
{

}

//----------------------------------------------------------------------------
bool NVitaSound::is_bgm_stream(int streamTop)
{
   const char* const pStreamName = BP_FindStreamName(streamTop);

   if (pStreamName)
   {
      if (strstr(pStreamName, "/bgm/" ) || strstr(pStreamName, "/bgm_2/" ) )
         return true;
   }

   return false;
}

#if SULPHA_ENABLED == 1

//----------------------------------------------------------------------------
void NVitaSound::start_sulpha_tracing()
{
   int returnCode = SCE_NGS_OK;

   SceSulphaConfig sulphaConfig;
   SceSulphaNgsConfig sulphaNgsConfig;
   SceUInt32 buffSize;

   returnCode = sceSysmoduleLoadModule(SCE_SYSMODULE_SULPHA);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Sulpha init error.");

   returnCode = sceSulphaNetworkInit();
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Sulpha init error.");

   returnCode = sceSulphaGetDefaultConfig(&sulphaConfig);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Sulpha init error.");
   
   returnCode = sceSulphaGetNeededMemory(&sulphaConfig, &buffSize);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Sulpha init error.");

   gspSulphaMem = malloc(buffSize);
   BPE_VERIFY(gspSulphaMem, false, "Could not allocate sulpha memory.");
   
   returnCode = sceSulphaInit(&sulphaConfig, gspSulphaMem, buffSize);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Sulpha init error.");

   returnCode = sceSulphaNgsGetDefaultConfig(&sulphaNgsConfig);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Sulpha init error.");

   returnCode = sceSulphaNgsGetNeededMemory(&sulphaNgsConfig, &buffSize);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Sulpha init error.");

   gspSulphaNgsMem = malloc(buffSize);
   BPE_VERIFY(gspSulphaNgsMem, false, "Could not allocate sulpha buffer.");

   returnCode = sceSulphaNgsInit(&sulphaNgsConfig, gspSulphaNgsMem, buffSize);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Sulpha init error.");
}

void NVitaSound::update_sulpha()
{
   int res = sceSulphaUpdate();
   BPE_VERIFY(res == SCE_NGS_OK, false, "Error updating sulpha.");
}

#endif

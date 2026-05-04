//----------------------------------------------------------------------------
// BP_SoundSupportPS3.cpp
//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/System/CSyncCriticalSection.h"
#include "Engine/Graphics/CColor.h"

#include "BP_BuildDefines.h"

#include "BP_SoundSupport.h"
#include "BP_SurroundSound.h"
#include "BP_ADSREnvelope.h"

#include "BP_RendererDebug.h"
#include "BP_Debug.h"

#include "libsd.h"

#include "MGS_Common.h"

#include <cell/audio.h>
#include <cell/mstream.h>
#include <sys/ppu_thread.h>
#include <cell/snd3.h>
#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>

extern "C" void BP_BuildMSEnvelope( const unsigned short adsr1, const unsigned short adsr2, CellMSEnvelope * const pEnvelope );

extern "C" int DG_TickCount;

extern int gbSoundSupportSurroundSound;
extern int gSoundSupportSurroundSoundChannelCount;
extern int gDirectOutputStreamsPaused;

extern unsigned char gDirectOutputStreamPlaybackBuffer[BP_DOSTREAM_COUNT+BP_DOSTREAM_TRACK_COUNT-1][BP_DOSTREAM_BUFFER_SIZE];
extern unsigned char gBP_SpuMemory[];

#define BP_MULTISTREAM_SURROUND_SOUND

#define BP_MULTISTREAM_CHANNELS 8

#define CELL_MS_SPEAKER_COUNT 8
#define DSP_MULTI_CHANNEL_COUNT 8

#define BP_REVERB_SUBNUM 24
#define BP_NOREVERB_SUBNUM 25
int BP_REVERB_PLAYSUB = BP_REVERB_SUBNUM | CELL_MS_BUS_FLAG;
int BP_NOREVERB_PLAYSUB = BP_NOREVERB_SUBNUM | CELL_MS_BUS_FLAG;

#define BP_USE_NEW_ADSR_ENVELOPES

//#define BP_DUMP_MS_WAVS

//#define USE_SULPHA

//Amount of memory to supply to Sulpha as a buffer
#define SULPHA_MEMORYBUFFER_SIZE (1024 * 1024)

// Number of items (PCM data, streams, busses and DPS's) to name
#define SULPHA_NUM_NAMED_OBJECTS (70)

//----------------------------------------------------------------------------

CellSpurs*        g_SpursForSound = NULL;

static CellAudioPortParam   audioParam;
static CellAudioPortConfig  portConfig;
static void *               spMultiStreamMemory    = NULL;
static void *               spSulphaMemory		 = NULL;

static sys_ppu_thread_t     sMultiStreamPuThread   = 0;
static volatile bool sbReceivedExitGameRequest=false;
static volatile bool sbMultiStreamUpdateThreadRunning=false;
static volatile bool sbMultiStreamInitialized = false;

static int           sMultiStreamPortNum = -1;

static bool sbReverbReverse=false;	//test flag to swap reverb on/off
static int sDSPHandle;
//Default reverb params for each PS2 effect type
static CellMSFXReverbParams skDefaultPS2ReverbParams[SD_REV_MODE_MAX];
//Current default reverb params
static CellMSFXReverbParams sDefaultReverbParams;
extern int gReverbModePS2;
extern int gReverbDepthPS2;
CellMSFXReverbParams sReverbParam[DSP_MULTI_CHANNEL_COUNT];
static void *spReverbBuffer = NULL;

//----------------------------------------------------------------------------
//Addresses of embedded (linked) .pic file SPU program 
extern char _binary_mstream_dsp_reverb_pic_start[];
extern char _binary_mstream_dsp_reverb_pic_end[];
extern char _binary_mstream_dsp_reverb_pic_size[];
//----------------------------------------------------------------------------

typedef struct _SSpuVoiceStatePS3
{
   unsigned int      mSSA;
   unsigned int      mLSAX;
   unsigned int      mBlockBegin;
   unsigned int      mBlockEnd;
   bool              mbBlockLoop;
   int               mMSChannel;
   int               mMSPitch;
   float             mMSVolumeL;
   float             mMSVolumeR;
   bool              mbReverbEnabledL;
   bool              mbReverbEnabledR;

   //ADSR envelope support
   SADSREnvelopeState   mADSREnvelope;

   //Surround sound support
   int               mSurroundType;
   float             mPan3d;
   float             mVol3d;
   float             mMSSurroundVolumes[CELL_MS_SPEAKER_COUNT];

   //If not NULL, this replaces SSA as the source of the waveform playback.
   const unsigned char * mpMemoryStream;
   int               mMemoryStreamSize;
   float             mMemoryStreamMuteVolume;
} SSpuVoiceState;

enum EDirectOutputStreamState
{
   kDOSS_Closed,
   kDOSS_Pending,
   kDOSS_Playing
};

typedef struct _SDirectOutputStreamStatePS3
{
   int               mTrackCount;
   int               mChannelCount;
   int               mTotalStreamSize;
   int               mPlaybackBufferSize;
   int               mFrequency;
   int               mFormat;
   int               mStreamType;
   EDirectOutputStreamState   mState;

   int               mMSChannel[BP_DOSTREAM_TRACK_COUNT];
   int               mBufferedBytes;
   int               mPrevPlaybackPosition;
   int               mPlaybackPosition;
   int               mCurrentPlayedBytes;
   double            mCurrentPlayTime;
   bool              mbPaused;

#ifndef GOLD_VERSION
   int               mSimulatedTickCount;
   bool              mbSimulatedPaused;
   bool              mbSimulatedPlayback;
#endif
} SDirectOutputStreamState;

static SSpuVoiceState sSpuVoiceState[2][24];
static SDirectOutputStreamState sDirectOutputStreamState[BP_DOSTREAM_COUNT];
static CSyncCriticalSection sMultistreamSync;

//----------------------------------------------------------------------------
static void bp_apply_stream_pause( int channel );

static void get_vag_endpoints( SSpuVoiceState & voiceState )
{
   voiceState.mBlockBegin = 0;
   voiceState.mBlockEnd = 0;
   voiceState.mbBlockLoop = false;

   int begin = voiceState.mpMemoryStream ? 0 : voiceState.mSSA;
   int end = voiceState.mpMemoryStream ? voiceState.mMemoryStreamSize : BP_SPU_MEMORY_SIZE;
   if( voiceState.mpMemoryStream == NULL && begin >= BP_SPU_MEMORY_SIZE )
   {
      //SSA is outside of emulated SPU memory!
      BP_BREAK;
   }
   for( int i=begin; i < end; i+=16 )
   {
      const unsigned char * vagPacket = voiceState.mpMemoryStream ? &voiceState.mpMemoryStream[i] : &gBP_SpuMemory[i];
      const unsigned char flag = vagPacket[1];
      if( flag & VAG_LOOPBLOCK )
      {
         voiceState.mbBlockLoop = true;
      }
      if( flag & VAG_LOOPSTART )
      {
         voiceState.mBlockBegin = i;
      }
      if( flag & VAG_LOOPEND )
      {
         voiceState.mBlockEnd = i+16;
         break;
      }
   }
   //LSAX register was manually set.  Here's our block begin.
   if( voiceState.mLSAX )
   {
      voiceState.mBlockBegin = voiceState.mLSAX;
      voiceState.mbBlockLoop = true;
      voiceState.mLSAX = 0;
   }
   if( !voiceState.mBlockEnd )
   {
      printf("Warning: could not get vag endpoint!\n");
      BP_TED_BREAK;
   }
}

static void bp_apply_adsr_envelope( const SSpuVoiceState & voiceState )
{
#ifndef BP_USE_NEW_ADSR_ENVELOPES
   CellMSEnvelope envelope;
   BP_BuildMSEnvelope( voiceState.mADSREnvelope.mADSR1, voiceState.mADSREnvelope.mADSR2, &envelope );
   int ret = cellMSEnvSetup(voiceState.mMSChannel, &envelope);
   if( ret != 0 )
   {
      printf("Error: couldn't set up envelope\n");
   }
#endif
}

static void bp_apply_volume( SSpuVoiceState & voiceState )
{
   const EADSRState prevADSRState = voiceState.mADSREnvelope.mADSRState;
#ifndef BP_USE_NEW_ADSR_ENVELOPES
   voiceState.mADSREnvelope.mEnvelopeScale = 1.f;
#else
   if( prevADSRState != kADSRState_Off )
   {
      BP_UpdateADSREnvelope( &voiceState.mADSREnvelope );
   }
#endif
   const bool bKeyOffAfterRelease = ( prevADSRState != kADSRState_Off ) && ( voiceState.mADSREnvelope.mADSRState == kADSRState_Off );

   float masterVolume = voiceState.mpMemoryStream ? voiceState.mMemoryStreamMuteVolume : 1.f;
   masterVolume *= voiceState.mADSREnvelope.mEnvelopeScale;
   float stereoVolL = voiceState.mMSVolumeL * masterVolume;
   float stereoVolR = voiceState.mMSVolumeR * masterVolume;
   float vol3d = voiceState.mVol3d * masterVolume;
   BP_GetVoiceSpeakerVolumes( stereoVolL, stereoVolR, voiceState.mPan3d, vol3d, (BP_SURROUND_VOICE_TYPE)voiceState.mSurroundType, voiceState.mMSSurroundVolumes );

   cellMSCoreSetVolume8( voiceState.mMSChannel, CELL_MS_DRY, CELL_MS_CHANNEL_0, voiceState.mMSSurroundVolumes );

   if( bKeyOffAfterRelease )
   {
      //Full stop.
      cellMSCoreStop(voiceState.mMSChannel,CELL_MS_STOP);
   }
}

//----------------------------------------------------------------------------

/**********************************************************************************
MultiStreamUpdateThread

	This thread updates MultiStream and libAudio:

	1) Updates MultiStream to generate more PCM data.
	2) Any MultiStream callbacks are then called
	3) Output PCM data is sent to libaudio
	4) It waits (allowing other threads to run) for lib audio to process data before repeating process
**********************************************************************************/
static void _Multi_Stream_Update_Thread(uint64_t param)
{
	int ret;
	//Remove compiler warnings
	(void)param;

	ret=cellAudioPortStart(sMultiStreamPortNum);
	BPE_VERIFY(ret==CELL_OK,false,"");
	while(!sbReceivedExitGameRequest)
	{
		// cellMSSystemSignalSPU does not sleep if it returns 0 (system pause is on), 
		// so sleep here to allow other threads to process
      const long fps60 = 1000000 / 60;

		if(!cellMSSystemSignalSPU())
			sys_timer_usleep(fps60 / 32);	

      //Critical section around all callbacks, ensuring that this thread is not inside a callback
      //when a stream is modified by the main thread.
      {
         CSyncCriticalSectionLocker lock(sMultistreamSync);
   		cellMSSystemGenerateCallbacks();
      }
	}
	cellAudioPortStop(sMultiStreamPortNum);
   sbMultiStreamUpdateThreadRunning = false;
   sys_ppu_thread_exit(0);
}

/**********************************************************************************
StartMultiStreamUpdateThread
	This function creates the thread to update multistream.

	Requires:
		_thread		Thread function to call to handle MS/libaudio buffer updates

**********************************************************************************/
long StartMultiStreamUpdateThread(void _thread (uint64_t param))
{
   // create the MultiStream / libaudio update thread
   const int stackSize = 0x4000;
   const int threadPrio = 0;
   int nRet = sys_ppu_thread_create(&sMultiStreamPuThread, _thread, NULL, threadPrio, stackSize, SYS_PPU_THREAD_CREATE_JOINABLE, "MultiStream PU Thread");
   BPE_VERIFY(nRet==0, false, "ERROR creating Multistream update thread!!!");
   printf("Multistream thread (%d) created OK.\n", (int)sMultiStreamPuThread);
   sbMultiStreamUpdateThreadRunning = true;

   return 0;
}

/**********************************************************************************

audioInitCell
	Initialises low level video and audio.
	This is not MultiStream specific.

	Returns: audio port number returned from cellAudioPortOpen(..)

**********************************************************************************/

static int audioInitCell()
{
	int ret=CELL_OK;
	sbReceivedExitGameRequest = false;

   unsigned int returnPortNum = 0;

   // Initialize the audio driver.
	ret = cellAudioInit();
	BPE_VERIFY(ret==CELL_OK,false,"");

   CellAudioOutState a_state;
   ret = cellAudioOutGetState(CELL_AUDIO_OUT_PRIMARY, 0, &a_state);
   if (ret !=CELL_OK)
   {
      printf("error audioGetStatus\n");
   }
   else
   {
      printf("audioInitCell: audio state: encoder %d, downmixer %d, channels %d\n", a_state.encoder, a_state.downMixer, a_state.soundMode.channel );
   }

   //manually configure audio to best available.
   CellAudioOutConfiguration audioConfig;
   memset(&audioConfig, 0, sizeof(CellAudioOutConfiguration));
#ifdef BP_MULTISTREAM_SURROUND_SOUND
   int ch_pcm = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_LPCM, CELL_AUDIO_OUT_FS_48KHZ, 0);
   printf("audioInitCell: Channels PCM %d\n", ch_pcm );

   //DO NOT use system downmixer, so we have full control over downmixing.
   //We have to manually downmix on X360 anyway.
   //Manual downmixing will be performed only for:
   //* legacy stereo mode with 6-channel cinematic streams
   //* 5.1 mode with 8-channel 3D pan voices
   audioConfig.downMixer = CELL_AUDIO_OUT_DOWNMIXER_NONE;

   if( ch_pcm < BP_MULTISTREAM_CHANNELS )
   {
      //Configure a downmixer.
      switch( ch_pcm )
      {
      case 6:
         {
            //6-channel PCM next best
            //Downmix to 6 channel pcm
            audioConfig.channel   = ch_pcm;
            audioConfig.encoder   = CELL_AUDIO_OUT_CODING_TYPE_LPCM;
            gSoundSupportSurroundSoundChannelCount = 6;
         }
         break;
      case 2:
         {
            //Check for bitstream encoding availability
            int ch_ac3 = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_AC3, CELL_AUDIO_OUT_FS_48KHZ, 0);
            int ch_dts = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_DTS, CELL_AUDIO_OUT_FS_48KHZ, 0);
            printf("audioInitCell: Channels AC3 %d\n", ch_ac3 );
            printf("audioInitCell: Channels DTS %d\n", ch_dts );

            if( ch_ac3 == 6 )
            {
               //downmix to 6 channel dolby digital
               audioConfig.channel = ch_ac3;
               audioConfig.encoder = CELL_AUDIO_OUT_CODING_TYPE_AC3;
               gSoundSupportSurroundSoundChannelCount = 6;
            }
            else if( ch_dts == 6 )
            {
               //downmix to 6 channel dts
               audioConfig.channel = ch_dts;
               audioConfig.encoder = CELL_AUDIO_OUT_CODING_TYPE_DTS;
               gSoundSupportSurroundSoundChannelCount = 6;
            }
            else
            {
               //Non-surround 2 channel pcm
               audioConfig.channel   = ch_pcm;
               audioConfig.encoder   = CELL_AUDIO_OUT_CODING_TYPE_LPCM;
               gbSoundSupportSurroundSound = 0;  //revert to legacy stereo calculations.
               gSoundSupportSurroundSoundChannelCount = 2;
            }
         }
         break;
      default:
         printf("Error: unexpected pcm channel count! %d\n", ch_pcm );
         BP_BREAK;
      }

      printf("audioInitCell: Configure: Channels=%d, Encoder=%d, DownMixer=%d\n", audioConfig.channel, audioConfig.encoder, audioConfig.downMixer );
      ret = cellAudioOutConfigure(CELL_AUDIO_OUT_PRIMARY, &audioConfig, NULL, 0);
      BPE_VERIFY(ret==CELL_OK,false,"");
   }
   //else no configuration needed (8 pcm channels is best).
#else
   //always downmix to stereo.
   audioConfig.channel   = 2;
   audioConfig.encoder   = CELL_AUDIO_OUT_CODING_TYPE_LPCM;
   ret = cellAudioOutConfigure(CELL_AUDIO_OUT_PRIMARY, &audioConfig, NULL, 0);
   BPE_VERIFY(ret==CELL_OK,false,"");
#endif

	// audio port open.
	memset(&audioParam, 0, sizeof(CellAudioPortParam));
	audioParam.nChannel = CELL_AUDIO_PORT_8CH;
	audioParam.nBlock   = CELL_AUDIO_BLOCK_8;

	ret = cellAudioPortOpen(&audioParam, &returnPortNum);
	BPE_VERIFY(ret==CELL_OK,false,"");
	printf("cellAudioPortOpen() : %d  port %d\n", ret, returnPortNum);
	if (ret != CELL_OK)
	{
		cellAudioQuit();
		printf("Error cellAudioPortOpen()\n");
		return -1;
	}
	 
	// get port config.
	ret = cellAudioGetPortConfig(returnPortNum, &portConfig);
	BPE_VERIFY(ret==CELL_OK,false,"");
	printf("cellAudioGetPortConfig() : %d\n", ret);
	if (ret != CELL_OK)
	{
		cellAudioQuit();
		printf("Error cellAudioGetPortConfig\n");
		return -1;
	}

   printf("Audio config: [%d channels]\n", (int)portConfig.nChannel);
	ret=cellMSSystemConfigureLibAudio(&audioParam, &portConfig);
	BPE_VERIFY(ret==CELL_OK,false,"");

	return returnPortNum;
}

/**********************************************************************************

InitialiseAudio

	This function sets up the audio system.

	Requires:	nStreams		Maximum number of streams to be active at any time
				nmaxSubs		Maximum number of sub channels to init in MultiStream
				_nPortNumber	Reference to int - Returns port number from CELL audio init
				_flags			MultiStream flags to pass to cellMSSystemGetNeededMemorySize and cellMSSystemInit* functions
							Default flag setting is no flags: "CELL_MS_NOFLAGS"

	Returns:	0	OK
				-1	Error


**********************************************************************************/

static bool InitSulpha(unsigned int _memoryBufferSize, unsigned int _numNames)
{
#ifdef USE_SULPHA
   spSulphaMemory = malloc(_memoryBufferSize);
   if(!spSulphaMemory)
      return false;

   int result = cellMSSulphaInit(spSulphaMemory, _memoryBufferSize, _numNames);
   printf("cellMSSulphaInit returned %d\n", result);

   // As well as using the Sulpha DECI3 connection, it is also possible to 
   // write directly to a file using cellMSSulphaFileConnect/Disconnect.
   result = cellMSSulphaDECI3Start();
   printf("cellMSSulphaDECI3Start returned %d\n", result);
#endif //USE_SULPHA

   return true;
}

long InitialiseAudio( const long nStreams, const long nmaxSubs, int *_nPortNumber, const int _nFlags)
{
	uint8_t prios[8] = {1, 0, 0, 0, 0, 0, 0, 0};
	int nMemoryNeeded;
	int ret;
// Setup system memory allocation
	CellMSSystemConfig cfg;
	cfg.channelCount=nStreams;
	cfg.subCount=nmaxSubs;

	cfg.dspPageCount=5;
	cfg.flags=_nFlags | CELL_MS_DISABLE_SPU_PRINTF_SERVER;	//default is CELL_MS_NOFLAGS

    *_nPortNumber = audioInitCell();
	BPE_VERIFY(*_nPortNumber>= 0, false, "InitialiseAudio: Failed to find valid port number!");

	// Initialise MultiStream

	nMemoryNeeded = cellMSSystemGetNeededMemorySize(&cfg);
	spMultiStreamMemory = memalign(128, nMemoryNeeded);

	//Init Sulpha (audio debugger)
	if(!InitSulpha(SULPHA_MEMORYBUFFER_SIZE , SULPHA_NUM_NAMED_OBJECTS))
		return -1;

	// Initialise SPURS MultiStream version
   ret = cellMSSystemInitSPURS(spMultiStreamMemory, &cfg, g_SpursForSound, &prios[0]);
	if(ret)
	{
		int error=cellMSSystemGetLastError();
		BPE_VERIFY(ret==0,false,"");
	}

	sbMultiStreamInitialized = true;

    return 0;
}

void BP_MultiStream_SetMasterVol(float volume)
{
   if (!sbMultiStreamInitialized)
   {
      // Prevent asset on shutdown when multistream isn't yet running
      return;
   }

   // bus volumes. (Maximum volume for each speaker)
   float fBusVols[64] = {
      volume, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, volume, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, volume, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, volume, 0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f, volume, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f, 0.0f, volume, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, volume, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, volume};
   // Setup the volumes on the master bus
   cellMSCoreSetVolume64(CELL_MS_MASTER_BUS, CELL_MS_DRY, fBusVols);

   cellMSCoreSetVolume64(BP_NOREVERB_PLAYSUB, CELL_MS_DRY, fBusVols);
   cellMSCoreSetVolume64(BP_REVERB_PLAYSUB, CELL_MS_WET, fBusVols);
}

static void BP_MultiStream_SetReverb(int channel,bool flag)
{
   if((flag && !sbReverbReverse) || (sbReverbReverse && !flag))
   {
      cellMSStreamSetSub(channel,BP_REVERB_PLAYSUB);
   }
   else
   {
      cellMSStreamSetSub(channel,BP_NOREVERB_PLAYSUB);
   }
}

void BP_LoadReverbDSP(void)
{
   //int nDSPFileHandle;
   //long nSizeDSPData;
   //long pDSPData = NULL;
   CellMSDSP DSPInfo;

   // Load DSP effect into RAM
   //nDSPFileHandle=OpenFile( REVERBTD_DSP_FILENAME, &nSizeDSPData );
   //printf("DSP effect file handle: %d. Size required: 0x%x\n",nDSPFileHandle,(int)nSizeDSPData);

   //pDSPData = (long)memalign(128,nSizeDSPData);
   //LoadFile(nDSPFileHandle,pDSPData,nSizeDSPData);

   // Handle transfer to SPU
   // Make sure MultiStream is not being update.
   sDSPHandle= cellMSDSPLoadDSPFromMemory((void*)_binary_mstream_dsp_reverb_pic_start, &DSPInfo, CELL_MS_ALLOC_DSPHANDLE,CELL_MS_ALLOC_PAGE);

   if (sDSPHandle==-1)
   {
      printf("cellMSDSPLoadDSPFromMemory Failed\n");
      while(1){};
   }
   //free((void*)pDSPData);	// Can now free PU memory for DSP effect
   printf("Loaded DSP. Handle: %d, mem used 0x%x, mem available : 0x%x\n", DSPInfo.handle, DSPInfo.memoryUsed, DSPInfo.memoryAvail);
}

void InitTimeDomainReverbParams()
{
   //   int nChannel;

   // setup of time domain reverb parameters
#if 1
   sDefaultReverbParams.fPreDelayMs = 0.0f; // pre-delay before reverb

   // reverb input filters
   sDefaultReverbParams.fHighPassHz = 150.0f;     // cut very low frequencies from reverb
   sDefaultReverbParams.fLowPassHz  = 22000.0f;  // cut very high frequencies from reverb

   // early reflections
   sDefaultReverbParams.fFrontBack  = 0.5f;  // early reflection pattern - based on listener->source distance in room
   sDefaultReverbParams.fLeftRight  = 0.5f;  // early and late reflections - subtle modifications give sense of l/r

   sDefaultReverbParams.fEarlyLate  = 0.5f; // mix parameter between early and late reflections

   // late reverberation tail parameters
   sDefaultReverbParams.fDamping    = 0.1f; // larger values roll off higher frequencies faster (simulating wall materials)
   sDefaultReverbParams.fDiffusion  = 0.01f;// increases number of small later reflections
   sDefaultReverbParams.fRoomSize   = 0.8f; // size of room, larger values increase decay time

   // linear volume controls for the final output
   sDefaultReverbParams.fWetVolume = 2.0f;
   sDefaultReverbParams.fDryVolume = 1.0f;
#else
   rparam.fWetVolume=1.0f;
   rparam.fDryVolume=1.0f;
   rparam.fPreDelayMs=120.0f;
   rparam.fLowPassHz=20000.0f;
   rparam.fHighPassHz=1000.0f;
   rparam.fRoomSize=0.9f;
   rparam.fDamping = 0.0f;
   rparam.fDiffusion = 0.2f;
   rparam.fEarlyLate = 0.5f;
   rparam.fLeftRight = 0.0f;
   rparam.fFrontBack = 0.0f;
#endif

   //TODO: fill in these used reverb types with proper parameters to match PS2.
   //For now they're all set to the STUDIO_C base parameters from CP3.
   skDefaultPS2ReverbParams[SD_REV_MODE_OFF] = sDefaultReverbParams;
   skDefaultPS2ReverbParams[SD_REV_MODE_OFF].fWetVolume = 0.f; //cause reverb not to apply.

   skDefaultPS2ReverbParams[SD_REV_MODE_STUDIO_C] = sDefaultReverbParams;
   skDefaultPS2ReverbParams[SD_REV_MODE_STUDIO_C].fWetVolume = 2.0f;
   skDefaultPS2ReverbParams[SD_REV_MODE_STUDIO_C].fLowPassHz = 18000.0f;
   skDefaultPS2ReverbParams[SD_REV_MODE_STUDIO_C].fDamping = 0.2f;
   skDefaultPS2ReverbParams[SD_REV_MODE_STUDIO_C].fDiffusion = 0.9f;
   skDefaultPS2ReverbParams[SD_REV_MODE_STUDIO_C].fRoomSize = 0.9f;

   skDefaultPS2ReverbParams[SD_REV_MODE_HALL] = sDefaultReverbParams;
   skDefaultPS2ReverbParams[SD_REV_MODE_HALL].fWetVolume = 2.5f;
   skDefaultPS2ReverbParams[SD_REV_MODE_HALL].fLowPassHz = 15000.0f;
   skDefaultPS2ReverbParams[SD_REV_MODE_HALL].fDamping = 0.75f;
   skDefaultPS2ReverbParams[SD_REV_MODE_HALL].fDiffusion = 0.6f;
   skDefaultPS2ReverbParams[SD_REV_MODE_HALL].fRoomSize = 0.75f;

   skDefaultPS2ReverbParams[SD_REV_MODE_SPACE] = sDefaultReverbParams;
   skDefaultPS2ReverbParams[SD_REV_MODE_SPACE].fWetVolume = 3.0f;
   skDefaultPS2ReverbParams[SD_REV_MODE_SPACE].fLowPassHz = 22000.0f;
   skDefaultPS2ReverbParams[SD_REV_MODE_SPACE].fDamping = 0.0f;
   skDefaultPS2ReverbParams[SD_REV_MODE_SPACE].fDiffusion = 1.0f;
   skDefaultPS2ReverbParams[SD_REV_MODE_SPACE].fRoomSize = 0.9f;

}

void bp_reapply_reverb_settings()
{
   int i;

   const float ps2Depth = (float)gReverbDepthPS2 / 0x7fff;
   float fDepth = ps2Depth * sDefaultReverbParams.fWetVolume;

   // copy across default settings and
   // setup each channels reverb settings with slightly different values
   for( i = 0; i < DSP_MULTI_CHANNEL_COUNT; ++i )
   {
      sReverbParam[i] = sDefaultReverbParams;

      sReverbParam[i].fWetVolume = fDepth;

      switch( i )
      {
      case CELL_MS_SPEAKER_FL:
         sReverbParam[i].fLeftRight = 0.0f;
         sReverbParam[i].fFrontBack = 0.3f;
         sReverbParam[i].fEarlyLate = 0.4f;
         break;

      case CELL_MS_SPEAKER_FR:
         sReverbParam[i].fLeftRight = 1.0f;
         sReverbParam[i].fFrontBack = 0.3f;
         sReverbParam[i].fEarlyLate = 0.4f;
         break;

      case CELL_MS_SPEAKER_FC:
         sReverbParam[i].fFrontBack = 0.3f;
         sReverbParam[i].fEarlyLate = 0.4f;
         break;

      case CELL_MS_SPEAKER_EL:
         sReverbParam[i].fLeftRight  = 0.0f;
         sReverbParam[i].fPreDelayMs = 0.1f;
         break;

      case CELL_MS_SPEAKER_ER:
         sReverbParam[i].fLeftRight = 1.0f;
         sReverbParam[i].fPreDelayMs = 0.1f;
         break;

      case CELL_MS_SPEAKER_RL:
         sReverbParam[i].fLeftRight = 0.0f;
         sReverbParam[i].fFrontBack = 0.7f;
         sReverbParam[i].fEarlyLate = 0.6f;
         sReverbParam[i].fPreDelayMs = 0.4f;
         break;

      case CELL_MS_SPEAKER_RR:
         sReverbParam[i].fLeftRight = 1.0f;
         sReverbParam[i].fFrontBack = 0.7f;
         sReverbParam[i].fEarlyLate = 0.6f;
         sReverbParam[i].fPreDelayMs = 0.4f;
         break;

      case CELL_MS_SPEAKER_LFE:
         // we don't require much if and reverb on the LFE
         sReverbParam[i].fWetVolume = 0.3f;
         break;
      }

      int ret= cellMSFXReverbSetParams( spReverbBuffer, &sReverbParam[i], i );
      if (ret<0)
      {
         printf("cellMSFXReverbSetParams failed\n");
         while(1){};
      }
   }
}

void BP_InitReverbDSP(int nHandle)
{
   char* pParamAddr = NULL;
   int ret;
   int size;
   int i;


   // Set DSP effect to slot 0 of stream
   cellMSCoreSetDSP( BP_REVERB_PLAYSUB, CELL_MS_DSP_SLOT_0, nHandle);
   pParamAddr = (char *)cellMSCoreGetDSPParamAddr( BP_REVERB_PLAYSUB, CELL_MS_DSP_SLOT_0 );

   // Get size required for 1 active reverb for a required slot.
   size = cellMSFXReverbGetNeededMemorySize(DSP_MULTI_CHANNEL_COUNT);
   spReverbBuffer = memalign(128,size);

   printf("Allocated Reverb buffer size: 0x%x to address 0x%x\n",size,(int)spReverbBuffer);

   /*
   Initialise the Reverb DSP.
   Set the initial pitch to 0.0f (original pitch). This automatically sets pitches for all multi-channel dsp effects
   Set to use to process three audio channels
   */

   InitTimeDomainReverbParams();

   ret= cellMSFXReverbInit(pParamAddr, spReverbBuffer, &sDefaultReverbParams, DSP_MULTI_CHANNEL_COUNT );
   if (ret<0)
   {
      printf("Init Reverb return failed\n");
      while(1){};
   }

   for (i=0;i<BP_MULTISTREAM_CHANNELS;i++)
   {
      cellMSCoreSetMask(BP_REVERB_PLAYSUB, CELL_MS_INMASK, CELL_MS_DSP_SLOT_0, i, 0);	// Allow input from none
      cellMSCoreSetMask(BP_REVERB_PLAYSUB, CELL_MS_OUTMASK, CELL_MS_DSP_SLOT_0, i, 0);	// Allow input from none
   }
   for (i=0;i<DSP_MULTI_CHANNEL_COUNT;i++)
   {
      cellMSCoreSetMask(BP_REVERB_PLAYSUB, CELL_MS_INMASK, CELL_MS_DSP_SLOT_0, i, 1<<i);	// Allow input from speaker n
      cellMSCoreSetMask(BP_REVERB_PLAYSUB, CELL_MS_OUTMASK, CELL_MS_DSP_SLOT_0, i, 1<<i);	// Allow output to speaker n
   }

   bp_reapply_reverb_settings();

   // Set the DSP slot (containing the Reverb DSP) to process twice
   //	cellMSCoreSetMask(BP_REVERB_PLAYSUB, CELL_MS_INMASK, CELL_MS_DSP_SLOT_0, 0, 0xff);	// Allow input from all
   //	cellMSCoreSetMask(BP_REVERB_PLAYSUB, CELL_MS_OUTMASK, CELL_MS_DSP_SLOT_0, 0, 0xff);	// Output to all

   // Turn DSP effect on for slot 0
   cellMSCoreBypassDSP( BP_REVERB_PLAYSUB, CELL_MS_DSP_SLOT_0, CELL_MS_NOTBYPASSED );
}

static int BP_DebugMenu_ActionCallback_ReverbSettings_Changed(int data)
{
   bp_reapply_reverb_settings();
   return 0;
}

static int BP_DebugMenu_ActionCallback_ReverbSettings_Reset(int data)
{
   //Copy out current settings to the default settings for current type.
   skDefaultPS2ReverbParams[gReverbModePS2] = sDefaultReverbParams;
   //dump to TTY
   printf("Current Reverb Settings:\n");

   return 0;
}

void BP_SoundSupport_InitDebugMenuPlatformSpecific( int soundMenu, int reverbMenu )
{
#if BP_ENABLE_DEBUG_MENU
   int item;
   item = BP_DebugMenu_AddFloat(reverbMenu, "WetVolume", &sDefaultReverbParams.fWetVolume, 0.0f, 8.f, 0.05f, 0.2f);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddFloat(reverbMenu, "HighPassHz", &sDefaultReverbParams.fHighPassHz, 0.0f, 22000.f, 1.f, 100.f);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddFloat(reverbMenu, "LowPassHz", &sDefaultReverbParams.fLowPassHz, 0.0f, 44000.f, 1.f, 100.f);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddFloat(reverbMenu, "Damping", &sDefaultReverbParams.fDamping, 0.0f, 1.f, 0.01f, 0.1f);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddFloat(reverbMenu, "Diffusion", &sDefaultReverbParams.fDiffusion, 0.0f, 1.f, 0.01f, 0.1f);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddFloat(reverbMenu, "RoomSize", &sDefaultReverbParams.fRoomSize, 0.0f, 4.f, 0.01f, 0.1f);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   BP_DebugMenu_AddAction(reverbMenu, "Store settings", BP_DebugMenu_ActionCallback_ReverbSettings_Reset, 0);
#endif
}

void BP_SoundSupport_Init_PlatformSpecific()
{
#ifdef BP_MULTISTREAM_SURROUND_SOUND
   gbSoundSupportSurroundSound = 1;
   gSoundSupportSurroundSoundChannelCount = 8;
#else
   gbSoundSupportSurroundSound = 0;
   gSoundSupportSurroundSoundChannelCount = 2;
#endif
   // Initialise MultiStream

   const int maxStreams = 48+16;
   const int maxSubs = 31;

   if( InitialiseAudio(maxStreams, maxSubs, &sMultiStreamPortNum, CELL_MS_NOFLAGS) != 0 )
   {
      printf( "ERROR starting multistream, exiting\n"); while(1){};
   }

   cellMSCoreRoutingInit(0xff000000);	//BP Soren: Set to time domain. ???

   BP_MultiStream_SetMasterVol(0.0f);

   StartMultiStreamUpdateThread(_Multi_Stream_Update_Thread);

   BP_LoadReverbDSP();
   BP_InitReverbDSP(sDSPHandle);

   for( int i=0;i<2;i++ )
   {
      for( int j=0;j<24;j++ )
      {
         sSpuVoiceState[i][j].mMSChannel=cellMSStreamOpen();
      }
   }

   for( int i=0; i < BP_DOSTREAM_COUNT; ++i )
   {
      for( int j=0; j < BP_DOSTREAM_TRACK_COUNT; ++j )
      {
         sDirectOutputStreamState[i].mMSChannel[j] = cellMSStreamOpen();
      }
   }
}

void BP_SoundSupport_ApplyReverbSettings_PlatformSpecific()
{
   sDefaultReverbParams = skDefaultPS2ReverbParams[gReverbModePS2];
   bp_reapply_reverb_settings();
}

void BP_SoundSupport_ApplyDirectOutputStreamsPaused_PlatformSpecific()
{
   for( int ch=0; ch<BP_DOSTREAM_COUNT; ++ch )
   {
      bp_apply_stream_pause(ch);
   }
}

void BP_SoundSupport_Shutdown()
{
   if (!sbMultiStreamInitialized)
   {
      // Don't shutdown MS if not initialized.
      // Fixes crash on disk eject on game startup
      return;
   }

   // close out the PPU pumper thread
   sbReceivedExitGameRequest = true;

   // Wait for thread to finish
   uint64_t exitCode;
   sys_ppu_thread_join(sMultiStreamPuThread, &exitCode);

   // all clear
   cellMSSystemClose();
}

void BP_SoundSupport_SetMasterVolume( float volume )
{
   BP_MultiStream_SetMasterVol( volume );
}

void BP_SoundSupport_Update()
{
#ifdef BP_USE_NEW_ADSR_ENVELOPES
   //tick all ADSR envelopes.
   for( int core=0; core < 2; ++core )
   {
      for( int voice=0; voice < 24; ++voice )
      {
         SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];
         if( voiceState.mADSREnvelope.mADSRState != kADSRState_Off )
         {
            bp_apply_volume( voiceState );
         }
      }
   }
#endif
}

void bp_direct_output_stream_callback(int streamNumber, void * userData, int callbackType, void * pWriteBuffer, int nBufferSize)
{
   //Remove compiler warnings
   (void)pWriteBuffer;
   (void)nBufferSize;

   switch( callbackType )
   {
   case CELL_MS_CALLBACK_MOREDATA:
      {
         SDirectOutputStreamState * pStreamState = (SDirectOutputStreamState*)userData;
         if( pStreamState->mTotalStreamSize != 0 )
         {
            for( int i=0; i < pStreamState->mTrackCount; ++i )
            {
               if( pStreamState->mMSChannel[i] == streamNumber )
               {
                  //Callback is for this track of this direct output stream channel.
                  int dostream_channel = pStreamState - &sDirectOutputStreamState[0];
                  int remainBytes = pStreamState->mTotalStreamSize - pStreamState->mBufferedBytes;
//                  printf("CB: TOT %d BUF %d REMAIN %d PLAYED %d\n", pStreamState->mTotalStreamSize, pStreamState->mBufferedBytes, remainBytes, pStreamState->mCurrentPlayedBytes );
                  if( remainBytes == 0 )
                  {
                     //Shouldn't ever get the callback when this is the case.
                     BP_TED_BREAK;
                  }
                  else if( remainBytes <= pStreamState->mPlaybackBufferSize )
                  {
                     //Playback needs to stop inside this iteration over the playback buffer.
                     const unsigned char * const playbackBuffer = BP_GetDirectOutputStreamPlaybackBuffer(dostream_channel) + i * pStreamState->mPlaybackBufferSize;
                     cellMSStreamSetFirstRead(streamNumber,playbackBuffer,remainBytes);
                     pStreamState->mBufferedBytes = pStreamState->mTotalStreamSize;
                     //Don't need another read after this one.
                     cellMSStreamSetSecondRead(streamNumber,NULL,0);
                     // Kill the callback, we're done.
                     cellMSStreamSetCallbackFunc(streamNumber, NULL);
//                     printf("CB: LAST\n" );
                  }
                  else
                  {
                     //Keep going.
                     pStreamState->mBufferedBytes += pStreamState->mPlaybackBufferSize;
                  }
                  break;
               }
            }
         }
      }
      break;

   case CELL_MS_CALLBACK_CLOSESTREAM:
   case CELL_MS_CALLBACK_FINISHSTREAM:
      break;

   default:
      printf("ERROR - unknown StreamCallback type: %d\n", callbackType);
      break;
   }
}

void bp_ms_voice_callback(int streamNumber, void * userData, int callbackType, void * pWriteBuffer, int nBufferSize)
{
   switch( callbackType )
   {
   case CELL_MS_CALLBACK_MOREDATA:
      {
         CellMSInfo MS_Info;
         cellMSStreamGetInfo( streamNumber, &MS_Info );
         //First and second buffers have already been swapped.  Copy the first (looped) buffer into the second
         //so they both point to the loop buffer.
         cellMSStreamSetSecondRead(streamNumber,MS_Info.FirstBuffer,MS_Info.FirstBufferSize);
         //Don't need the callback anymore.
         cellMSStreamSetCallbackFunc(streamNumber, NULL);
      }
      break;
   case CELL_MS_CALLBACK_CLOSESTREAM:
   case CELL_MS_CALLBACK_FINISHSTREAM:
      break;

   default:
      printf("ERROR - unknown StreamCallback type: %d\n", callbackType);
      break;
   }
}

void bp_set_mts_multistream_channel( const int dostream_channel, const int track )
{
   SDirectOutputStreamState & streamState = sDirectOutputStreamState[dostream_channel];

   const int msChannel = streamState.mMSChannel[track];
   const unsigned char * const playbackBuffer = BP_GetDirectOutputStreamPlaybackBuffer(dostream_channel) + track * streamState.mPlaybackBufferSize;

   CellMSInfo  MS_Info;

   MS_Info.SubBusGroup         = BP_REVERB_PLAYSUB;

   MS_Info.FirstBuffer         = (void *)(playbackBuffer+0);
   MS_Info.FirstBufferSize     = streamState.mPlaybackBufferSize;

   MS_Info.SecondBuffer        = (void *)(playbackBuffer+0);
   MS_Info.SecondBufferSize    = streamState.mPlaybackBufferSize;

   MS_Info.Pitch               = streamState.mFrequency;
   MS_Info.numChannels         = streamState.mChannelCount;
   MS_Info.flags				= 0;

   // Initial delay (in samples) before playback starts. Allows for sample accurate playback
   MS_Info.initialOffset		= 0;

   // Input data type
   switch( streamState.mFormat )
   {
   case BP_DOSTREAM_FORMAT_PCM:
      MS_Info.inputType = CELL_MS_16BIT_BIG;
      break;
   case BP_DOSTREAM_FORMAT_VAG:
      MS_Info.inputType = CELL_MS_ADPCM;
      break;
   default:
      BP_TODO_BREAK;
   }

   cellMSCoreInit(msChannel);
   cellMSStreamSetInfo(msChannel, &MS_Info);
   if( streamState.mTotalStreamSize != 0 )
   {
      //This stream needs to automatically stop when it hits the end of the data.
      //Set a callback to do so.
      cellMSStreamSetCallbackFunc( msChannel, bp_direct_output_stream_callback );
      cellMSStreamSetCallbackData( msChannel, &streamState );
   }

   //No reverb on this type of stream (cinematics only!)
   BP_MultiStream_SetReverb(msChannel,false);

   cellMSStreamPlay(msChannel);
}

static void bp_apply_stream_pause( int channel )
{
   CELL_MS_PAUSE ms_pause = ( sDirectOutputStreamState[channel].mbPaused || gDirectOutputStreamsPaused ) ? CELL_MS_PAUSED : CELL_MS_NOTPAUSED;
   for ( int i=0; i < sDirectOutputStreamState[channel].mTrackCount; ++i )
   {
      cellMSCoreSetPause( sDirectOutputStreamState[channel].mMSChannel[i], ms_pause );
   }
}

unsigned int BP_PlayWaveform( const unsigned char * const waveformData, const int size, const int frequency, const int channelCount )
{
   int msChannel;
   CellMSInfo  MS_Info;
   int ret;

   msChannel = cellMSStreamOpen();
   BPE_ASSERT_NO_MSG( msChannel != -1 );

   MS_Info.SubBusGroup         = CELL_MS_MASTER_BUS; // Which sub bus we are routing through

   MS_Info.FirstBuffer         = (void*)waveformData;
   MS_Info.FirstBufferSize     = size;
   MS_Info.SecondBuffer         = NULL;
   MS_Info.SecondBufferSize     = 0;

   // Set pitch and number of channels
   MS_Info.Pitch               = frequency;
   MS_Info.numChannels         = channelCount;
   MS_Info.flags				= CELL_MS_STREAM_AUTOCLOSE;

   // Initial delay (in samples) before playback starts. Allows for sample accurate playback
   MS_Info.initialOffset		= 0;

   // Input data type (WAV = 16Bit, Big Endian)
   // Data is byteswapped when loaded because X360 can't play back LE data directly
   MS_Info.inputType = CELL_MS_16BIT_BIG;

   ret = cellMSStreamSetInfo(msChannel, &MS_Info);

   if( ret != CELL_OK )
   {
      printf("Error cellMSStreamSetInfo: %d 0x%08x\n", cellMSSystemGetLastError(), cellMSSystemGetLastError() );
      return -1;
   }

   cellMSStreamPlay(msChannel);
   return msChannel;
}

void BP_StopWaveform( const unsigned int handle )
{
   cellMSStreamClose( handle );
   while( cellMSStreamGetStatus( handle ) != CELL_MS_STREAM_CLOSED )
   {
   }

   printf("BP_StopWaveform: %08x\n", handle );
}

void BP_SetWaveformVolume( const unsigned int handle, const float volume )
{
   //NOTE: this supports stereo playback only!
   float vols0[CELL_MS_SPEAKER_COUNT] = { 0 };
   float vols1[CELL_MS_SPEAKER_COUNT] = { 0 };
   vols0[CELL_MS_SPEAKER_FL] = volume;
   vols1[CELL_MS_SPEAKER_FR] = volume;
   cellMSCoreSetVolume8( handle, CELL_MS_DRY, CELL_MS_CHANNEL_0, vols0 );
   cellMSCoreSetVolume8( handle, CELL_MS_DRY, CELL_MS_CHANNEL_1, vols1 );
}

void BP_InitDirectOutputStream(DirectOutputStreamInitData const* pInitInfo)
{
   BPE_ASSERT_NO_MSG( pInitInfo->trackCount > 0 && pInitInfo->trackCount <= BP_DOSTREAM_TRACK_COUNT );
   BPE_ASSERT_NO_MSG( pInitInfo->format == BP_DOSTREAM_FORMAT_PCM || pInitInfo->format == BP_DOSTREAM_FORMAT_VAG );
   //Indicated size measured in bytes, includes all stereo / surround channels, and is per track!
   if( pInitInfo->channel == 2 )
   {
      BPE_ASSERT_NO_MSG( pInitInfo->format == BP_DOSTREAM_FORMAT_PCM );
      //MGS3: playback buffer size for BGM varies based on track count.  If there are fewer than 3 tracks,
      //the playback buffer actually spans multiple BP_DOSTREAM_BUFFER_SIZE elements.
      //The entire playback buffer size for MTA streams is BP_DOSTREAM_TRACK_COUNT the normal size.
      BPE_ASSERT_NO_MSG( pInitInfo->playbackBufferSize <= BP_DOSTREAM_BUFFER_SIZE * BP_DOSTREAM_TRACK_COUNT );
   }
   else
   {
      BPE_ASSERT_NO_MSG( pInitInfo->playbackBufferSize <= BP_DOSTREAM_BUFFER_SIZE );
   }
   BP_StopDirectOutputStream( pInitInfo->channel );

   {
      CSyncCriticalSectionLocker lock(sMultistreamSync);

      SDirectOutputStreamState & streamState = sDirectOutputStreamState[pInitInfo->channel];
      streamState.mTrackCount = pInitInfo->trackCount;
      streamState.mChannelCount = pInitInfo->channelCount;
      streamState.mTotalStreamSize = pInitInfo->totalStreamSize;
      streamState.mPlaybackBufferSize = pInitInfo->playbackBufferSize;
      streamState.mFrequency = pInitInfo->freq;
      streamState.mFormat = pInitInfo->format;
      streamState.mStreamType = pInitInfo->streamType;
      streamState.mState = kDOSS_Pending;
      streamState.mbPaused = false;
   }
}

void BP_ContinuePlayDirectOutputStream( const int channel )
{
   if( sDirectOutputStreamState[channel].mState == kDOSS_Pending )
   {
      CSyncCriticalSectionLocker lock(sMultistreamSync);
      sDirectOutputStreamState[channel].mState = kDOSS_Playing;
      for( int i=0; i < sDirectOutputStreamState[channel].mTrackCount; ++i )
      {
         bp_set_mts_multistream_channel( channel, i );
      }
#ifndef GOLD_VERSION
      sDirectOutputStreamState[channel].mSimulatedTickCount = DG_TickCount;
      sDirectOutputStreamState[channel].mbSimulatedPlayback = false;
      sDirectOutputStreamState[channel].mbSimulatedPaused = false;
#endif
      //Starting with one bufferful before the first stream callback.
      sDirectOutputStreamState[channel].mBufferedBytes = sDirectOutputStreamState[channel].mPlaybackBufferSize;

      sDirectOutputStreamState[channel].mPrevPlaybackPosition = 0;
      sDirectOutputStreamState[channel].mPlaybackPosition = 0;
      sDirectOutputStreamState[channel].mCurrentPlayedBytes = 0;
      sDirectOutputStreamState[channel].mCurrentPlayTime = 0;
   }
}

unsigned char * BP_GetDirectOutputStreamPlaybackBuffer( const int channel )
{
   BPE_ASSERT_NO_MSG( channel >= 0 && channel < BP_DOSTREAM_COUNT );
   //N.B. the last channel "spans" BP_DOSTREAM_TRACK_COUNT buffers worth.
   return gDirectOutputStreamPlaybackBuffer[channel];
}

int BP_GetDirectOutputStreamReadAheadSize( const int channel )
{
   return sDirectOutputStreamState[channel].mPlaybackBufferSize;
}

void BP_NotifyDirectOutputStreamBytesAvailable(int channel, void* buffer, int sizeBytes)
{
   // Not necessary to do anything on PS3.
   (void)channel;
   (void)sizeBytes;
   (void)buffer;
}

void BP_SetDirectOutputStreamPaused( const int channel, int paused )
{
#ifndef GOLD_VERSION
   if( sDirectOutputStreamState[channel].mbSimulatedPlayback )
   {
      sDirectOutputStreamState[channel].mbSimulatedPaused = paused ? true : false;
   }
#endif
   sDirectOutputStreamState[channel].mbPaused = paused ? true : false;
   bp_apply_stream_pause( channel );
}

void BP_StopDirectOutputStream( const int channel )
{
   sDirectOutputStreamState[channel].mState = kDOSS_Closed;
   for ( int i=0; i < sDirectOutputStreamState[channel].mTrackCount; ++i )
   {
      cellMSCoreStop(sDirectOutputStreamState[channel].mMSChannel[i],CELL_MS_KEYOFF);
   }

   sDirectOutputStreamState[channel].mBufferedBytes = 0;
   sDirectOutputStreamState[channel].mPrevPlaybackPosition = 0;
   sDirectOutputStreamState[channel].mPlaybackPosition = 0;
   sDirectOutputStreamState[channel].mCurrentPlayedBytes = 0;
   sDirectOutputStreamState[channel].mCurrentPlayTime = 0;
}

extern int BP_IsDirectOutputStreamDone( const int channel )
{
   if( sDirectOutputStreamState[channel].mTotalStreamSize == 0 )
   {
      //Not set up to stop at the end automatically.
      return 0;
   }

   //Stream is done if all bytes have been buffered and all tracks are off.
   if( sDirectOutputStreamState[channel].mBufferedBytes < sDirectOutputStreamState[channel].mTotalStreamSize )
   {
      return 0;
   }

   for ( int i=0; i < sDirectOutputStreamState[channel].mTrackCount; ++i )
   {
      int status = cellMSStreamGetStatus( sDirectOutputStreamState[channel].mMSChannel[i] );
      if( status != CELL_MS_STREAM_OFF )
      {
         //This track, at least, is not done.
         return 0;
      }
   }
   return 1;
}

int BP_GetDirectOutputStreamPosition( const int channel )
{
   if( sDirectOutputStreamState[channel].mState == kDOSS_Playing )
   {
#ifndef GOLD_VERSION

      if( ( gBP_PauseFrameStep ) || ( ( gBP_DirectOutputStreamSpeed != 1 ) && ( gBP_DirectOutputStreamIsDemo != 0 ) ) )
      {
         sDirectOutputStreamState[channel].mbSimulatedPlayback = true;
      }
      if( sDirectOutputStreamState[channel].mbSimulatedPlayback )
      {
         //advance play position only if the stream is playing.
         int last = sDirectOutputStreamState[channel].mSimulatedTickCount;
         sDirectOutputStreamState[channel].mSimulatedTickCount = DG_TickCount;
         int diffTicks = sDirectOutputStreamState[channel].mSimulatedTickCount - last;
         float streamSpeed = 1.0f;//gBP_DirectOutputStreamSpeed is applied to delta tick count
         int diffSamples = (int)(streamSpeed * sDirectOutputStreamState[channel].mFrequency * diffTicks / (300/BASE_TICK));
         float diffTime = streamSpeed * diffTicks / (300/BASE_TICK);
         int diffBytes = diffSamples * sDirectOutputStreamState[channel].mChannelCount;
         switch( sDirectOutputStreamState[channel].mFormat )
         {
         case BP_DOSTREAM_FORMAT_PCM:
            diffBytes *= sizeof( short );
            break;
         case BP_DOSTREAM_FORMAT_VAG:
            diffBytes = ( diffBytes * 16 ) / 28;
            break;
         default:
            BP_TODO_BREAK;
         }
         sDirectOutputStreamState[channel].mCurrentPlayTime += diffTime;
         sDirectOutputStreamState[channel].mPrevPlaybackPosition = sDirectOutputStreamState[channel].mPlaybackPosition;
         sDirectOutputStreamState[channel].mPlaybackPosition += diffBytes;
         sDirectOutputStreamState[channel].mCurrentPlayedBytes += diffBytes;

         while( sDirectOutputStreamState[channel].mPlaybackPosition >= sDirectOutputStreamState[channel].mPlaybackBufferSize )
         {
            sDirectOutputStreamState[channel].mPlaybackPosition -= sDirectOutputStreamState[channel].mPlaybackBufferSize;
         }
         return sDirectOutputStreamState[channel].mPlaybackPosition;
      }
      //Always update simulated tick count.
      sDirectOutputStreamState[channel].mSimulatedTickCount = DG_TickCount;
#endif
      //address of waveform data that should be read next
      int currentOffset = 0;
      int slot_adr = 0;
      cellMSStreamGetCurrentPlayPos(sDirectOutputStreamState[channel].mMSChannel[0],&currentOffset,&slot_adr);
      if( slot_adr )
         slot_adr-=(int)BP_GetDirectOutputStreamPlaybackBuffer(channel);
      sDirectOutputStreamState[channel].mPrevPlaybackPosition = sDirectOutputStreamState[channel].mPlaybackPosition;
      sDirectOutputStreamState[channel].mPlaybackPosition = (slot_adr+currentOffset);
      int diffBytes = sDirectOutputStreamState[channel].mPlaybackPosition - sDirectOutputStreamState[channel].mPrevPlaybackPosition;
      if( diffBytes < 0 )
      {
         //wrapped around
         diffBytes = sDirectOutputStreamState[channel].mPlaybackBufferSize - sDirectOutputStreamState[channel].mPrevPlaybackPosition + sDirectOutputStreamState[channel].mPlaybackPosition;
      }
      sDirectOutputStreamState[channel].mCurrentPlayedBytes += diffBytes;
      switch( sDirectOutputStreamState[channel].mFormat )
      {
      case BP_DOSTREAM_FORMAT_PCM:
         sDirectOutputStreamState[channel].mCurrentPlayTime = (double)sDirectOutputStreamState[channel].mCurrentPlayedBytes / sizeof(short);
         break;
      case BP_DOSTREAM_FORMAT_VAG:
         sDirectOutputStreamState[channel].mCurrentPlayTime = (double)sDirectOutputStreamState[channel].mCurrentPlayedBytes * 28 / 16;
         break;
      default:
         BP_TODO_BREAK;
      }

      sDirectOutputStreamState[channel].mCurrentPlayTime /= ( sDirectOutputStreamState[channel].mFrequency * sDirectOutputStreamState[channel].mChannelCount );

      return sDirectOutputStreamState[channel].mPlaybackPosition;
   }
   else
   {
//      printf("BP_GetDirectOutputStreamPosition(): not playing\n");
      return 0;
   }
}

double BP_GetDirectOutputStreamPlayTime( const int channel )
{
   return sDirectOutputStreamState[channel].mCurrentPlayTime;
}

void BP_SetDirectOutputStreamVolume_Direct( const int channel, const int track, const float fVol )
{
   BPE_ASSERT( track >= 0 && track < BP_DOSTREAM_MAX_TRACK_VALUE, "" );

   SDirectOutputStreamState & streamState = sDirectOutputStreamState[channel];

   if( track >= BP_DOSTREAM_TRACK_COUNT || streamState.mChannelCount == 0 )
   {
      //Unused stream / track
      return;
   }

   BPE_ASSERT( streamState.mStreamType == BP_DOSTREAM_TYPE_DIRECT, "Wrong volume function for this stream type!" );

   float fVols64[64];
   BP_GetDirectOutputStreamSpeakerVolumes_Direct( streamState.mChannelCount, fVol, fVols64 );
   cellMSCoreSetVolume64(sDirectOutputStreamState[channel].mMSChannel[track], CELL_MS_DRY, fVols64);
}

void BP_SetDirectOutputStreamVolume_Voice( const int channel, int stereoVols[2][2],
                                          float pan3d, float vol3d, BP_SURROUND_VOICE_TYPE surroundType)
{
   //no such thing on PS3
}

void BP_SetDirectOutputStreamDsp_Voice( const int channel, int bDspEnabled )
{
   //no such thing on PS3
}

void BP_PlayMemStream( const int core, const int voice, const void * const playbackBuffer, const int playbackBufferSize )
{
   BPE_ASSERT( core < 2 && voice < 24, "" );
   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];
   cellMSCoreStop(voiceState.mMSChannel,CELL_MS_STOP);
   voiceState.mpMemoryStream = (const unsigned char*)playbackBuffer;
   voiceState.mMemoryStreamSize = playbackBufferSize;
   voiceState.mMemoryStreamMuteVolume = 1;
}

int BP_GetMemStreamPosition(const int core, const int voice)
{
   BPE_ASSERT( core < 2 && voice < 24, "" );
   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];
   int currentOffset = 0;
   int slot_adr = 0;
   cellMSStreamGetCurrentPlayPos(voiceState.mMSChannel,&currentOffset,&slot_adr);
   return currentOffset;
}

void BP_SetMemStreamMuted( const int core, const int voice, const int muted )
{
   BPE_ASSERT( core < 2 && voice < 24, "" );
   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];
   if( voiceState.mpMemoryStream )
   {
      voiceState.mMemoryStreamMuteVolume = muted ? 0 : 1;
      bp_apply_volume( voiceState );
   }
}

void BP_sceSdSetParam( unsigned short entry, unsigned short value )
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT( voice < 24, "" );

   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];

   switch( reg )
   {
   case SD_VP_VOLL:
      {
         //voice volume (left)
         voiceState.mMSVolumeL = BP_PS2VolToFVol( value );
         //Volume settings now applied all at once with BP_SoundSupport_ApplyVoiceVolume()
         //bp_apply_volume( voiceState );
      }
      break;
   case SD_VP_VOLR:
      {
         //voice volume (right)
         voiceState.mMSVolumeR = BP_PS2VolToFVol( value );
         //Volume settings now applied all at once with BP_SoundSupport_ApplyVoiceVolume()
         //bp_apply_volume( voiceState );
      }
      break;
   case SD_VP_PITCH:
      {
         //pitch when sound is generated
         voiceState.mMSPitch = (((int)value*48000)+2048)/4096;
         BPE_ASSERT_NO_MSG( voiceState.mMSPitch > 0 );
         if( voiceState.mMSPitch > 96000 )
         {
            printf("Warning: pitch too high for ADPCM in Multistream [%d][%2d] (%d).  Add an override for this SE!\n", core, voice, voiceState.mMSPitch);
 //           BP_TED_BREAK;
         }
         cellMSStreamSetPitch(voiceState.mMSChannel,voiceState.mMSPitch);
      }
      break;
   case SD_VP_ADSR1:
      {
         //envelope
         voiceState.mADSREnvelope.mADSR1 = value;
      }
      break;
   case SD_VP_ADSR2:
      {
         //envelope (2)
         voiceState.mADSREnvelope.mADSR2 = value;
#ifndef BP_USE_NEW_ADSR_ENVELOPES
         if( !( cellMSStreamGetStatus( voiceState.mMSChannel ) & CELL_MS_STREAM_ON ) )
         {
            bp_apply_adsr_envelope( voiceState );
         }
#endif
      }
      break;
   case SD_P_EVOLL:
      {
         //Effect return volume (left)
         //NOTE: MGS2/3 always sets L/R together and to the same value.
         //Set it here and verify that the value matches the R setting below.
         //Then apply it once there.
         gReverbDepthPS2 = value;
         float fDepth = (float)value / 0x7fff;
         //printf("SD_P_EVOLL: %d -> %f\n", value, fDepth );
      }
      break;
   case SD_P_EVOLR:
      {
         //Effect return volume (right)
         //NOTE: MGS2/3 always L/R together and to the same value.
         //See above.
         if( value != gReverbDepthPS2 )
            BP_TODO_BREAK;

         bp_reapply_reverb_settings();
      }
      break;
   default:
      {
         printf( "Warning: unimplemented sceSdSetParam(%d).  BUG to Ted please!\n", reg );
         BPE_ASSERT_NO_MSG(false);
      }
      break;
   }
}

unsigned short BP_sceSdGetParam( unsigned short entry )
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT( voice < 24, "" );

   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];

   switch( reg )
   {
   case SD_VP_ENVX:
      {
         //envelope value
         int flag=cellMSStreamGetStatus(voiceState.mMSChannel);
         if(flag&CELL_MS_STREAM_ON)
         {
            const int status = cellMSStreamGetStatus( voiceState.mMSChannel );
            unsigned short releaseLevel;

            //Don't actually check the stream level until it's playing.
            if( status & CELL_MS_STREAM_PENDING )
            {
               releaseLevel = 0xffff;
            }
            else
            {
#ifndef BP_USE_NEW_ADSR_ENVELOPES
               const float currEnv = cellMSStreamGetCurrentEnv(voiceState.mMSChannel);
#else
               const float currEnv = voiceState.mADSREnvelope.mEnvelopeScale;
#endif
               releaseLevel = (unsigned short)(currEnv * 0xffff);
            }

            return releaseLevel;
         }
         else
         {
            return 0;
         }
      }
      break;
   default:
      {
         printf( "Warning: unimplemented sceSdGetParam(%d).  BUG to Ted please!\n", reg );
         BPE_ASSERT_NO_MSG(false);
         return 0;
      }
      break;
   }
}

void BP_sceSdSetAddr( unsigned short entry, unsigned int value )
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT( voice < 24, "" );

   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];

   switch( reg )
   {
   case SD_VA_SSA:
      {
         //waveform data starting address
         voiceState.mSSA = value;
         cellMSCoreStop(voiceState.mMSChannel,CELL_MS_STOP);
         //stream is actually initialized in the KEYON event so the endpoints can be calculated.
         voiceState.mpMemoryStream = NULL;   //cancel any memory stream setting
         voiceState.mMemoryStreamMuteVolume = 1;
      }
      break;
   case SD_VA_LSAX:
      {
         //loop point address
         //manually setting loop block point for this key
         voiceState.mLSAX = value;
      }
      break;
   default:
      {
         printf( "Warning: unimplemented sceSdSetAddr(%d).  BUG to Ted please!\n", reg );
         BPE_ASSERT_NO_MSG(false);
      }
      break;
   }

}

unsigned int BP_sceSdGetAddr( unsigned short entry )
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT( voice < 24, "" );

   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];

   switch( reg )
   {
   case SD_VA_NAX:
      {
         //address of waveform data that should be read next
         int currentOffset = 0;
         int slot_adr = 0;
         cellMSStreamGetCurrentPlayPos(voiceState.mMSChannel,&currentOffset,&slot_adr);
         if( slot_adr )
            slot_adr-=(int)&gBP_SpuMemory[0];
         return slot_adr+currentOffset;
      }
      break;
   default:
      {
         printf( "Warning: unimplemented sceSdGetAddr(%d).  BUG to Ted please!\n" );
         BPE_ASSERT_NO_MSG(false);
         return 0;
      }
      break;
   }
}

void BP_sceSdSetSwitch( unsigned short entry, unsigned int value )
{
   int reg = entry & 0xffc0;
   int core = entry & 1;

   switch( reg )
   {
   case SD_S_KOFF:
      {
         //key off (end voice generation)
         for( int i=0; i < 24; ++i )
         {
            int voiceBit = 1 << i;
            if( value & voiceBit )
            {
               SSpuVoiceState & voiceState = sSpuVoiceState[core][i];
#ifndef BP_USE_NEW_ADSR_ENVELOPES
               cellMSCoreStop(voiceState.mMSChannel,CELL_MS_KEYOFF);
#else
               if( voiceState.mADSREnvelope.mADSRState != kADSRState_Off )
               {
                  //Key off with release.
                  voiceState.mADSREnvelope.mADSRState = kADSRState_Release;
               }
#endif
            }
         }
      }
      break;
   case SD_S_KON:
      {
         //key on (start voice generation)
         for( int i=0; i < 24; ++i )
         {
            int voiceBit = 1 << i;
            if( value & voiceBit )
            {
               SSpuVoiceState & voiceState = sSpuVoiceState[core][i];

               cellMSCoreStop(voiceState.mMSChannel,CELL_MS_STOP);

               get_vag_endpoints( voiceState );

               CellMSInfo  MS_Info;
               // Which sub bus we are routing through. Note that SUBBUS_1 is set to frequency domain

               MS_Info.SubBusGroup         = BP_REVERB_PLAYSUB;

               // Set address and size of data to play
               if( voiceState.mbBlockLoop )
               {
                  int sb_size = voiceState.mBlockEnd - voiceState.mSSA;
                  int loop_size = voiceState.mBlockEnd - voiceState.mBlockBegin;

                  if( voiceState.mpMemoryStream )
                     BP_TODO_BREAK;

                  MS_Info.FirstBuffer         = (void *)(gBP_SpuMemory+voiceState.mSSA);
                  MS_Info.FirstBufferSize     = sb_size; // size in bytes

                  MS_Info.SecondBuffer         = (void *)(gBP_SpuMemory+voiceState.mBlockBegin);
                  MS_Info.SecondBufferSize     = loop_size; // size in bytes
               }
               else
               {
                  if( !voiceState.mpMemoryStream )
                  {
                     int sb_size = voiceState.mBlockEnd - voiceState.mSSA;

                     MS_Info.FirstBuffer         = (void *)(gBP_SpuMemory+voiceState.mSSA);
                     MS_Info.FirstBufferSize     = sb_size; // size in bytes
                  }
                  else
                  {
                     MS_Info.FirstBuffer         = (void *)voiceState.mpMemoryStream;
                     MS_Info.FirstBufferSize     = voiceState.mMemoryStreamSize;
                  }
                  MS_Info.SecondBuffer         = NULL;
                  MS_Info.SecondBufferSize     = 0; // size in bytes
               }

               MS_Info.Pitch               = voiceState.mMSPitch;
               MS_Info.numChannels         = 1;
               MS_Info.flags				= 0;

               // Initial delay (in samples) before playback starts. Allows for sample accurate playback
               MS_Info.initialOffset		= 0;

               // Input data type (ADPCM)
               MS_Info.inputType = CELL_MS_ADPCM;

#ifdef BP_DUMP_MS_WAVS
               {
                  char dumpWavFilename[FILENAME_MAX];
                  static int dumpCount = 0;
                  sprintf( dumpWavFilename, "/app_home/%d_%08x_%d%s.adpcm", dumpCount++, MS_Info.FirstBuffer, MS_Info.FirstBufferSize, MS_Info.SecondBuffer ? "_l" : "" );
                  FILE * fp = fopen( dumpWavFilename, "wb" );
                  fwrite( MS_Info.FirstBuffer, MS_Info.FirstBufferSize, 1, fp );
                  fclose( fp );

               }
#endif
               {
                  CSyncCriticalSectionLocker lock(sMultistreamSync);
                  cellMSCoreInit(voiceState.mMSChannel);

                  cellMSStreamSetInfo(voiceState.mMSChannel, &MS_Info);
                  float muteVolume = voiceState.mpMemoryStream ? voiceState.mMemoryStreamMuteVolume : 1;
                  BP_MultiStream_SetReverb(voiceState.mMSChannel,voiceState.mbReverbEnabledR);

                  if( voiceState.mbBlockLoop )
                  {
                     //This stream needs to set its first(->second) read to its loop read settings after the first buffer's playback.
                     //Set a callback to do so.
                     cellMSStreamSetCallbackFunc( voiceState.mMSChannel, bp_ms_voice_callback );
                     cellMSStreamSetCallbackData( voiceState.mMSChannel, &voiceState );
                  }
                  cellMSStreamPlay(voiceState.mMSChannel);
                  BP_InitADSREnvelope( &voiceState.mADSREnvelope );
                  bp_apply_volume( voiceState );
                  bp_apply_adsr_envelope( voiceState );
                  //printf("KON (%d,%2d) %04x.%04x: %d\n", core, i, voiceState.mADSREnvelope.mADSR1, voiceState.mADSREnvelope.mADSR2, CStopWatch::gGlobalTime.GetElapsedMicroSeconds() );
               }
            }
         }
      }
      break;
   case SD_S_NON:
      {
         //allocation to noise generator
         if( value != 0 )
         {
            //use the noise generator for this voice
            printf("Warning: unimplemented SD_S_NON.  BUG to Ted please!\n");
            BPE_ASSERT_NO_MSG(false);
         }
      }
      break;
   case SD_S_PMON:
      {
         //pitch modulation per voice
         if( value != 0 )
         {
            BP_TODO_BREAK;
         }
      }
      break;
   case SD_S_VMIXEL:
   case SD_S_VMIXER:
      {
         //Voice output mixing flag per voice (wet left)
         //Voice output mixing flag per voice (wet right)
         for( int i=0; i < 24; ++i )
         {
            SSpuVoiceState & voiceState = sSpuVoiceState[core][i];

            int voiceBit = 1 << i;
            bool bReverbEnabled = ( value & voiceBit ) != 0;
            if( reg == SD_S_VMIXEL )
            {
               voiceState.mbReverbEnabledL = bReverbEnabled;
            }
            else
            {
               //TED - they always set SD_S_VMIXEL / SD_S_VMIXER together, in that order,
               //and to the same value.  Apply once as a flag for the entire channel
               //rather than "properly" implementing via separate L/R wet volumes.
               if( bReverbEnabled != voiceState.mbReverbEnabledL )
                  BP_TODO_BREAK;

               voiceState.mbReverbEnabledR = bReverbEnabled;
               BP_MultiStream_SetReverb( voiceState.mMSChannel, bReverbEnabled );
            }
         }
      }
      break;
   default:
      {
         printf( "Warning: unimplemented sceSdSetSwitch(%d).  BUG to Ted please!\n", reg );
         BPE_ASSERT_NO_MSG(false);
      }
      break;
   }
}

void BP_sceSdSetCoreAttr( unsigned short entry, unsigned short value )
{
   int reg = entry & 0xfffe;
   int core = entry & 1;
   switch (reg)
   {
   case SD_C_NOISE_CLK:
      {
         //noise generator
      }
      break;
   case SD_C_EFFECT_ENABLE:
      {
         //Enable / disable writing to effect work area.  Presumably we don't need this hook!
      }
      break;
   default:
      {
         printf( "Warning: unimplemented sceSdSetCoreAttr(%d).  BUG to Ted please!\n", reg );
         BPE_ASSERT_NO_MSG(false);
      }
      break;
   }

}

unsigned short BP_sceSdNote2Pitch ( unsigned short center_note, unsigned short center_fine, unsigned short note, short fine)
{
   return cellSnd3Note2Pitch( center_note, center_fine, note, fine );
}

void BP_SoundSupport_SurroundSoundSetVoiceType( const int core, const int voice, BP_SURROUND_VOICE_TYPE voiceType )
{
   BPE_ASSERT( core >= 0 && core < 2, "" );
   BPE_ASSERT( voice >= 0 && voice < 24, "" );

   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];
   voiceState.mSurroundType = voiceType;
   //Volume settings now applied all at once with BP_SoundSupport_ApplyVoiceVolume()
//   bp_apply_volume( voiceState );
}

void BP_SoundSupport_SurroundSoundSetVoicePanVol( const int core, const int voice, float pan, float vol )
{
   BPE_ASSERT( core >= 0 && core < 2, "" );
   BPE_ASSERT( voice >= 0 && voice < 24, "" );

   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];
   voiceState.mPan3d = pan;
   voiceState.mVol3d = vol;
   //Volume settings now applied all at once with BP_SoundSupport_ApplyVoiceVolume()
//   bp_apply_volume( voiceState );
}

void BP_SoundSupport_ApplyVoiceVolume( const int core, const int voice )
{
   BPE_ASSERT( core >= 0 && core < 2, "" );
   BPE_ASSERT( voice >= 0 && voice < 24, "" );

   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];
   bp_apply_volume( voiceState );
}

void BP_SoundSupport_InitADPCMStreamingBuffer( const int channel, const int lr, const int spuAddr, const int spuSize )
{
   //Stub when we're not doing ADPCM decoding on our side.
}

void BP_SoundSupport_ShowStreamStats_PlatformSpecific()
{
   for( int i = 0; i < BP_DOSTREAM_COUNT; ++i )
   {
      const SDirectOutputStreamState & doStream = sDirectOutputStreamState[i];
      BP_DebugText_Print("Stream[%d]: State: %d Played Bytes: %d Total Size: %d", 
         i, 
         doStream.mState,
         doStream.mCurrentPlayedBytes,
         doStream.mTotalStreamSize
         );
   }
}

void BP_SoundSupport_ShowVoiceStats_PlatformSpecific()
{
   for( int core=0; core < 2; ++core )
   {
      for( int voice=0; voice < 24; ++voice )
      {
         static const char kADSRChar[] = { '0', 'A', 'D', 'S', 'R' };
         SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];
         BP_DebugText_Print("[%d][%2d]: %c %f\n", core, voice, kADSRChar[voiceState.mADSREnvelope.mADSRState], voiceState.mADSREnvelope.mEnvelopeScale );
      }
   }
}

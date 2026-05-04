//----------------------------------------------------------------------------
// BP_SoundEffectOverride.cpp
//----------------------------------------------------------------------------

#include "Engine/StdAfx.h"

#include "Engine/System/CStopWatch.h"
#include "Engine/Graphics/CColor.h"
#include "BP_RendererDebug.h"
#include "Engine/System/COsContext.h"

#include "BP_EndianSupport.h"
#include "BP_FileSupport.h"
#include "BP_SoundEffectOverrideMGS3.h"

#include "libfs.h"

#include "sddefine.h"

extern "C" char *GM_GetArea();

//#define BP_DISABLE_SOUNDEFFECT_OVERRIDE

#define BP_SE_OVERRIDE_MIDI_PROGRAM_SIZE 64

extern int gBP_EnableSfxWavReplacements;
//----------------------------------------------------------------------------

namespace
{
   struct SBPSoundEffectOverride
   {
      unsigned int      mSoundCode;
      PSQ_IWAV          mIWAV;
      unsigned char     mMidiProgram[BP_SE_OVERRIDE_MIDI_PROGRAM_SIZE];

      bool operator < ( const SBPSoundEffectOverride & rhs ) const
      {
         return this->mSoundCode < rhs.mSoundCode;
      }
   };

   struct SBPSoundEffectOverrideHeader
   {
      char                    mCookie[4];
      int                     mCount;
      SBPSoundEffectOverride  mEntries[1];   //variable-length array
   };

   SBPSoundEffectOverrideHeader *   gpSoundEffectOverrides = NULL;
}

//----------------------------------------------------------------------------

void BP_InitSoundEffectOverride()
{
#ifndef BP_DISABLE_SOUNDEFFECT_OVERRIDE
   BPE_ASSERT_NO_MSG( gpSoundEffectOverrides == NULL );

   char fullPath[FILENAME_MAX];
   sprintf( fullPath, "misc/BP_SE.DAT");

   int filesize = BP_GetFileSizeAttr( fullPath, NULL );
   gpSoundEffectOverrides = (SBPSoundEffectOverrideHeader*)( malloc(filesize) );
   {
      void * fp = BP_OpenFile( fullPath, NULL );
      BP_ReadFile( fp, gpSoundEffectOverrides, filesize );
      BP_CloseFile( fp );
   }
   if( strncmp( gpSoundEffectOverrides->mCookie, "SEO3", 4 ) )
   {
      BP_BREAK;
   }
   BP_LE_SwapSInt_Inp( &gpSoundEffectOverrides->mCount );
   //Fix up entries.
   for( int i=0; i < gpSoundEffectOverrides->mCount; ++i )
   {
      SBPSoundEffectOverride & seOverride = gpSoundEffectOverrides->mEntries[i];
      BP_LE_SwapUInt_Inp( &seOverride.mSoundCode );
      BP_LE_SwapUInt_Inp( &seOverride.mIWAV.adsr );
      BP_LE_SwapUInt_Inp( &seOverride.mIWAV.addr );
      BP_LE_SwapUShort_Inp( &seOverride.mIWAV.frq );
      BP_LE_SwapUShort_Inp( &seOverride.mIWAV.size );
      seOverride.mIWAV.addr = (int)gpSoundEffectOverrides + (int)seOverride.mIWAV.addr;

#if 1 //BP_PATCH: hack to remove reverb flag from some system SE that weren't supposed to have it.  Oops.
      //See KJPFB-344
      switch( seOverride.mSoundCode )
      {
      case SD_S_CODEC_CHARA_OPEN_1 & 0x7ff:
      case SD_S_CODEC_CHARA_CLOSE_1 & 0x7ff:
      case SD_S_CODEC_CANSEL_1 & 0x7ff:
      case SD_S_LIFE_DOWN_1 & 0x7ff:
      case SD_S_WEAPON_SELECT_1 & 0x7ff:
         {
            unsigned char * const reverbCmd = seOverride.mMidiProgram + 33;
            if( reverbCmd[0] != 0x5b )
               BP_BREAK;
            if( reverbCmd[1] != 21 )
               BP_BREAK;
            reverbCmd[1] = 0;
         }
         break;
      }

#endif
   }
#endif
}

PSQ_IWAV * BP_GetWavOverrideIWAV( int i )
{
#ifndef BP_DISABLE_SOUNDEFFECT_OVERRIDE
   BPE_ASSERT( i < gpSoundEffectOverrides->mCount, "wav index out of range" );
   SBPSoundEffectOverride & seOverride = gpSoundEffectOverrides->mEntries[i];
   return &seOverride.mIWAV;
#else
   BP_BREAK;   //normal sound macros should never call this.
   return NULL;
#endif
}

void * BP_Override_SoundEffect( int sound_code )
{
#ifndef BP_DISABLE_SOUNDEFFECT_OVERRIDE

   if( const char * areaName = GM_GetArea() )
   {
      //Disable sound effect overrides while playing MG1 or MG2 because
      //the SE id's are in general aliased against the main game's SE id's.
      if( !strcmp( areaName, "mg1" ) || !strcmp( areaName, "mg2" ) )
      {
         //(Except for special cases used by save / load UI screens...)
         //SE #define's are locally redefined for MG1 / MG2 as below, see actor_mg.h

#if 0
#define	SD_S_NORM_SELECT_1                         (0x0001a06f)
#define	SD_S_NORM_ENTER_1                          (0x0001a070)
#define	SD_S_NORM_CANSEL_1                         (0x0001a071)
#define	SD_S_NG_BUZZER_1                           (0x00003073)
#define	SD_S_SAVE_OK_1                             (0x0001a09c)
#endif

         switch( sound_code )
         {
         case 0x0001a06f & 0x7ff:
            sound_code = SD_S_NORM_SELECT_1 & 0x7ff;
            break;
         case 0x0001a070 & 0x7ff:
            sound_code = SD_S_NORM_ENTER_1 & 0x7ff;
            break;
         case 0x0001a071 & 0x7ff:
            sound_code = SD_S_NORM_CANSEL_1 & 0x7ff;
            break;
         case 0x00003073 & 0x7ff:
            sound_code = SD_S_NG_BUZZER_1 & 0x7ff;
            break;
         case 0x0001a09c & 0x7ff:
            sound_code = SD_S_SAVE_OK_1 & 0x7ff;
            break;
         default:
            //No other overrides allowed.
            return NULL;
         }
      }
   }

   if( gBP_EnableSfxWavReplacements && gpSoundEffectOverrides != NULL )
   {
      SBPSoundEffectOverride key;
      key.mSoundCode = sound_code;
      SBPSoundEffectOverride * pOverride = std::lower_bound( gpSoundEffectOverrides->mEntries, gpSoundEffectOverrides->mEntries+gpSoundEffectOverrides->mCount, key );
      if( pOverride != gpSoundEffectOverrides->mEntries+gpSoundEffectOverrides->mCount && pOverride->mSoundCode == key.mSoundCode )
      {
         //This sound effect is in the override table.  return a pointer to its midi program.
         return pOverride->mMidiProgram;
      }
   }
#endif
   return NULL;
}


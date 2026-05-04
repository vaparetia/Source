//----------------------------------------------------------------------------
// BP_SoundEffectOverride.cpp
//----------------------------------------------------------------------------

#include "Engine/StdAfx.h"

#include "Engine/System/COsContext.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Graphics/CColor.h"
#include "BP_RendererDebug.h"

#include "BP_EndianSupport.h"
#include "BP_FileSupport.h"
#include "BP_SoundEffectOverride.h"

#include "g_sound.h"
#include "se_defin.h"

#include "libfs.h"
#include "BP_Memory.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//BP_CODE_DATA_PATCH
#define BP_SE_OVERRIDE_CODE_DATA_PATCH

#ifdef BP_SE_OVERRIDE_CODE_DATA_PATCH
#include "BP_SE_PATCH_JP.cpp"
#endif

//#define BP_DISABLE_SOUNDEFFECT_OVERRIDE

extern int gBP_EnableSfxWavReplacements;
extern int gReverbDepthPS2;

extern "C" unsigned int BP_GetSelectedPlayerGcl();

//----------------------------------------------------------------------------

namespace
{
   enum ESECategory
   {
      kSECategory_InternalSnake,       //resident snake voice
      kSECategory_InternalRaiden,      //resident raiden voice
      kSECategory_InternalShared,      //resident only need one copy
      kSECategory_External,            //general nonresident
      kSECategory_ExternalVrVox,       //VR / Alternative stage only overrides (VR enemy vox)
   };

   struct SBPMidiCommand
   {
      unsigned char  mdata4;
      unsigned char  mdata3;
      unsigned char  mdata2;
      unsigned char  mdata1;
   };

   struct SBPSoundEffectOverrideTrack
   {
      void *            mWavAddress;
      int               mWavSize;
      SBPMidiCommand    mMidiProgram[9];
   };
   struct SBPSoundEffectOverride
   {
      unsigned int                  mSoundCode;
      int                           mTrackCount;
      SBPSoundEffectOverrideTrack   mTracks[2];

      bool operator < ( const SBPSoundEffectOverride & rhs ) const
      {
         return this->mSoundCode < rhs.mSoundCode;
      }
   };

   struct SBPSoundEffectOverrideHeader
   {
      char                    mCookie[4];
      int                     mVersion;
      int                     mCount;
      SBPSoundEffectOverride  mEntries[1];   //variable-length array
   };

   SBPSoundEffectOverrideHeader *   gpSoundEffectOverrides = NULL;
   SBPSoundEffectOverrideHeader *   gpSoundEffectOverrides_Patch = NULL;

   //List of SE that were built as stereo and need to be patched at runtime back to mono.
   //Problem was that this system has to go through the original MGS2 SE system / PS2 hardware
   //emulation and the only way to play a "stereo" signal through that system is with two
   //memory stream tracks.  These do not start at *exactly* the same time, especially on
   //X360 where there is an overhead each time a voice is started because it has to be
   //recreated first and also likely because the audio thread runs on a different core.
   //About 2% of the time on PS3 and 84% of the time on X360 there was a noticeable
   //(to the sound engineer reviewing) phase shift b/t L and R channels.
   //So all of the SE that don't really need to be stereo are now reverted to mono using
   //just the left channel.  These are the ones where the phase difference was noticeable
   //anyway because the waveforms are very similar between the channels.
   //Note: The SE that clearly have stereo information in them are being left as stereo with
   //the above problem unresolved.  They sound blatantly wrong as mono and the problem
   //shouldn't be really noticeable in them anyway.
   //NOTE: the proper data-side fix to these is to remove BP_STEREO from the comments for these
   //in bp_override_se.h
   const int skPatchToMonoSE[] =
   {
      SD_S_FULL0005,
      SD_S_KAIHUKU1,
      SD_S_ITEM0003,
      SD_S_IDISP02,
      SD_S_IGET02,
      SD_S_ISEL02,
      SD_S_IDEC02,
      SD_S_R_DISP01,
      SD_S_R_CANCEL,
      SD_S_R_TUNE01,
      SD_S_R_WINDW1,
      SD_S_R_WINDW2,
      SD_S_COUNTRV1,
      SD_S_TYPING03,
      SD_S_LINEMOV1,
      SD_S_V_CANS02,
      SD_S_WIN01,
      SD_S_WINOPN01,
      SD_S_WINCLS01,

      // Vita specific entries...
      SD_S_CUR01,
   };
}

extern "C"
{
int gBP_SEInternalResidentCategory = 0;
}

static bool bp_is_vr_area()
{
   if( const char * const area = RenderBackend()->GetCurrentAreaDebugName() )
   {
      if(  !strncmp( area, "wp", 2 )
        || !strncmp( area, "sp", 2 )
        || !strncmp( area, "twp", 3 ) 
        || !strncmp( area, "st", 2 ) 
        || !strncmp( area, "tsp", 3 ) 
        || !strncmp( area, "vs", 2 ) 
        )
      {
         return true;
      }
   }

   return false;
}

//----------------------------------------------------------------------------

static void do_init_soundeffectoverride( SBPSoundEffectOverrideHeader* const pOverrides )
{
   if( strncmp( pOverrides->mCookie, "SEO2", 4 ) )
   {
      BP_BREAK;
   }
   BP_LE_SwapSInt_Inp( &pOverrides->mVersion );
   if( pOverrides->mVersion != 1 )
   {
      BP_BREAK;
   }

   BP_LE_SwapSInt_Inp( &pOverrides->mCount );
   //Fix up entries.
   for( int i=0; i < pOverrides->mCount; ++i )
   {
      SBPSoundEffectOverride & seOverride = pOverrides->mEntries[i];
      BP_LE_SwapUInt_Inp( &seOverride.mSoundCode );
      BP_LE_SwapSInt_Inp( &seOverride.mTrackCount );
      for( int tr=0; tr<2; ++tr )
      {
         SBPSoundEffectOverrideTrack & seOverrideTrack = seOverride.mTracks[tr];
         BP_LE_SwapSInt_Inp( &seOverrideTrack.mWavSize );
         BP_LE_SwapPtr_Inp( &seOverrideTrack.mWavAddress );
         seOverrideTrack.mWavAddress = (char*)pOverrides + (int)seOverrideTrack.mWavAddress;
      }
   }
}

static const SBPSoundEffectOverride * const find_se_override( const SBPSoundEffectOverrideHeader * const pOverrides, const int sound_code, const int category )
{
   SBPSoundEffectOverride key;
   key.mSoundCode = (unsigned int)sound_code | (unsigned int)( category << 24 );
   const SBPSoundEffectOverride * const pOverride = std::lower_bound( pOverrides->mEntries, pOverrides->mEntries+pOverrides->mCount, key );
   if( pOverride != pOverrides->mEntries+pOverrides->mCount && pOverride->mSoundCode == key.mSoundCode )
   {
      return pOverride;
   }

   return NULL;
}

//----------------------------------------------------------------------------

void BP_InitSoundEffectOverride()
{
#ifndef BP_DISABLE_SOUNDEFFECT_OVERRIDE
   BPE_ASSERT_NO_MSG( gpSoundEffectOverrides == NULL );

   const bool bJapan = OsContext()->mBuildSKU==COsContext::kBS_Japan;
   char fullPath[FILENAME_MAX];
   sprintf( fullPath, "misc/%s/BP_SE.DAT", bJapan ? "jp" : "us" );
   int filesize = BP_GetFileSizeAttr( fullPath, NULL );
   gpSoundEffectOverrides = reinterpret_cast<SBPSoundEffectOverrideHeader*>( BP_Memory_Alloc( filesize, 128, kMT_Permanent, kMC_Sound ) );
   {
      void * fp = BP_OpenFile( fullPath, NULL );
      BP_ReadFile( fp, gpSoundEffectOverrides, filesize );
      BP_CloseFile( fp );
   }

   do_init_soundeffectoverride( gpSoundEffectOverrides );

#ifdef BP_SE_OVERRIDE_CODE_DATA_PATCH
   if( bJapan )
   {
      //Patch overrides added as binary data into code.
      gpSoundEffectOverrides_Patch = (SBPSoundEffectOverrideHeader*)(binary_BP_SE_PATCH_JP_data);
      do_init_soundeffectoverride( gpSoundEffectOverrides_Patch );
   }
#endif

#ifdef BP_SE_OVERRIDE_CODE_DATA_PATCH
   //Apply runtime patch to revert stereo wav overrides to mono to avoid phasing effect described above.
   for( int i=0; i < BPE_ARRAY_SIZE(skPatchToMonoSE); ++i )
   {
      const int sound_code = skPatchToMonoSE[i];
      const int category = kSECategory_InternalShared;   //all the problematic stereo SE are such

      SBPSoundEffectOverride key;
      key.mSoundCode = (unsigned int)sound_code | (unsigned int)( category << 24 );
      SBPSoundEffectOverride * pOverride = std::lower_bound( gpSoundEffectOverrides->mEntries, gpSoundEffectOverrides->mEntries+gpSoundEffectOverrides->mCount, key );
      if( pOverride != gpSoundEffectOverrides->mEntries+gpSoundEffectOverrides->mCount && pOverride->mSoundCode == key.mSoundCode )
      {
         //Need to set track count to 1 and update pan to center on the first track.
         if( pOverride->mTrackCount != 2 )
            BP_BREAK;
         pOverride->mTracks[0].mMidiProgram[6].mdata3 = 0;  //center pan
         pOverride->mTrackCount = 1;
         memset( &pOverride->mTracks[1], 0, sizeof( pOverride->mTracks[1] ) );   //zero out second track
      }
      else
      {
         BP_BREAK;
      }
   }
#endif

#endif
}

void * BP_GetWavOverrideAddress( unsigned int wavCode, int * pOutSize )
{
#ifndef BP_DISABLE_SOUNDEFFECT_OVERRIDE
   unsigned int entryIndex = wavCode & 0x3ff;
   unsigned int track = wavCode >> 11;
   unsigned int is_patch = ( wavCode >> 10 ) & 1;
   const SBPSoundEffectOverrideHeader * const pOverrides = is_patch ? gpSoundEffectOverrides_Patch : gpSoundEffectOverrides;
   BPE_ASSERT( entryIndex < pOverrides->mCount, "wav index out of range" );
   BPE_ASSERT( track < 2, "invalid track" );
   const SBPSoundEffectOverride & seOverride = pOverrides->mEntries[entryIndex];
   *pOutSize = seOverride.mTracks[track].mWavSize;
   return (void*)seOverride.mTracks[track].mWavAddress;
#else
   BP_BREAK;   //normal sound macros should never call this.
   return NULL;
#endif
}

int BP_Override_SoundEffect( int sound_code, void ** const ppOutTrackAddr0, void ** const ppOutTrackAddr1 )
{
#ifndef BP_DISABLE_SOUNDEFFECT_OVERRIDE

   if( !gBP_EnableSfxWavReplacements )
   {
      return 0;
   }

   if( !gpSoundEffectOverrides )
   {
      //were not loaded.
      return 0;
   }

   // AS MCampbell - Do not override these sound effects on Vita. It is not necessary and produces artifacts.
   // Bug: MGSTWO-2903
#if defined(BP_VITA)
   switch (sound_code)
   {
   case SD_P_FOOT2S1L:
   case SD_P_FOOT2S1R:
   case SD_P_FOOTW01L:
   case SD_P_FOOTW01R:
   case SD_P_FOOTW02L:
   case SD_P_FOOTW02R:
   case SD_P_FOOTW03L:
   case SD_P_FOOTW03R:
   case SD_P_FOOTW04L:
   case SD_P_FOOTW04R:
   case SD_P_FOOTW05L:
   case SD_P_FOOTW05R:
   case SD_P_FOOTW06L:
   case SD_P_FOOTW06R:
   case SD_P_FOOTW07L:
   case SD_P_FOOTW07R:
   case SD_P_FOOTW08L:
   case SD_P_FOOTW08R:
      return 0;
   }
#endif

   //TED - These SE codes for Emma voice were in the header file
   //but we have no source .aif files for them.  If they actually existed,
   //we'll have to dig them out of the extracted waveforms from JP/SOL.
   switch( sound_code )
   {
   case SD_V_EMADWN02:
   case SD_V_EMAEXP01:
   case SD_V_EMAEXP02:
   case SD_V_EMAGIV02:
      BP_TODO_BREAK;
   }
   int category = 0;
   unsigned int selectedPlayerStrId = BP_GetSelectedPlayerGcl();

   if( sound_code <= 0xff || sound_code == 0x5be || sound_code == 0x512 || (sound_code >= 0x513 && sound_code <= 0x516) || sound_code == 0x435 || sound_code == 0x436 )
   {
      //Internal SE.
      //Either Snake, Raiden, or shared.  Not a great way on this end to tell which except to assume shared unless id
      //matches one we know was overridden as Japanese voice.
      bool bShared = false;
      if( sound_code <= 0xff )
      {
         switch( sound_code )
         {
            //internal vox sfx cases taken from bp_override_se_jp.h
         case SD_V_PDMG01:
         case SD_V_PDMG02:
         case SD_V_POUT0001:
         case SD_V_PDMG03:
         case SD_V_PDONMU01:
         case SD_V_PELUDE01:
         case SD_V_PFALL01:
         case SD_V_PVOMIT01:
         case SD_V_KACHA01:
         case SD_V_KACHA02:
         case SD_V_KACHA03:
         case SD_V_KACHA04:
         case SD_V_PKIAI01:
            bShared = false;
            break;
         default:
            bShared = true;   //all other SE in internal are shared.
         }
      }
      
      if( bShared )
      {
         category = kSECategory_InternalShared;
      }
      else
      {
         //See if we've got raiden or snake loaded.
         //These are strid's of Japanese string literals.
         //See a_sload.h, line 122 for example, to see the list.
         //NOTE: sound_code < 0xff and sound_code == 0x5be correspond to se id's of sounds that were in internal sound pakfiles.
         //sound_code == 0x512 is the hero sneeze sfx which was only in sound pakfiles for areas where you could catch a cold.
         //sound_code >= 0x513 && sound_code <= 0x516 is the hero getting electrocuted in Shell 2 Core 1F (w31a or ta31a)
         switch(selectedPlayerStrId)
         {
         default:
            printf("Warning: unrecognized player strid 0x%08x.  Defaulting to choice based on resident pakfile.\n", selectedPlayerStrId);
            BP_TED_BREAK;
         case 0:           //uninitialized (harmless; comes up when entering boss survival)
         case 0x00553d37:  //None (a_varinit.h line 30)
            {
               //In the main tanker and plant chapters, player id is just set to none (it may have been added for substance).
               //In this case, let the loaded resident pak file decide which guy is playing.
               //This rule works everywhere we know of except snake tales anyway.
               switch(gBP_SEInternalResidentCategory)
               {
               case 0:
                  category = kSECategory_InternalSnake;
                  break;
               case 1:
                  category = kSECategory_InternalRaiden;
                  break;
               default:
                  BP_BREAK;
               }
            }
            break;
         case 0x009b1527:  //Raiden
         case 0x001c1541:  //Blade Raiden
         case 0x00221542:  //Naked Raiden
            category = kSECategory_InternalRaiden;
            break;
         case 0x0028dce6:  //Snake
         case 0x0061ad7e:  //Pliskin
         case 0x003d04c0:  //Tuxedo Snake
         case 0x0057403e:  //Previous Snake
            category = kSECategory_InternalSnake;
            break;
         }
      }

#ifdef BP_SE_OVERRIDE_CODE_DATA_PATCH
      if( sound_code == 0x435 || sound_code == 0x436 )
      {
         //Special case added after ship to handle two vox in Vamp battle that need Snake versions, otherwise
         //he's got Raiden's voice in Boss Survival.
         //The Raiden version is in the generic "External" category in the ship BP_SE.DAT because we didn't know about it.
         //SO in this case, If it's Raiden, set it back to external and the existing BP_SE.DAT will handle it.
         //Otherwise, the patch BP_SE.DAT will hit.
         //NOTE: the proper data fix is to consider these the same as the "sneeze" sfx and still fetch from the appropriate folders.
         if( category == kSECategory_InternalRaiden )
         {
            category = kSECategory_External;
         }
      }
#endif

   }
   else
   {
      //External SE.
      if (sound_code >= 0x100 && sound_code <= 0x165)
      {
         //An additional copy of each enemy vox SE exists for VR missions since all the guys are VR robot thingys
         //and have a filter over their voice.  If we're in a VR mission, take the VR version.
         if( bp_is_vr_area() )
         {
            category = kSECategory_ExternalVrVox;
         }
         else
         {
            category = kSECategory_External;
         }
      }
      else
      {
         //default case for all generic SE.
         category = kSECategory_External;
      }
   }

   const SBPSoundEffectOverride * pOverride = find_se_override( gpSoundEffectOverrides, sound_code, category );
   if( !pOverride )
   {
      if( gpSoundEffectOverrides_Patch )
      {
         pOverride = find_se_override( gpSoundEffectOverrides_Patch, sound_code, category );
      }
   }
   if( pOverride )
   {
      //This sound effect is in the override table.  return a pointer to its midi program.
      *ppOutTrackAddr0 = (void*)pOverride->mTracks[0].mMidiProgram;
      *ppOutTrackAddr1 = (void*)pOverride->mTracks[1].mMidiProgram;
      return pOverride->mTrackCount;
   }

#endif
   return 0;
}


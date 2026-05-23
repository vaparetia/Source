//----------------------------------------------------------------------------
// COsContext.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "COsContext.h"

#if BPE_TARGET != BPE_TARGET_DREAMCAST
#include "../../../../../../Builds/DiskBuilds/UseDiskBuild.h"
#endif

#ifndef BP_TGS_DEMO
#  define BP_TGS_DEMO() 0
#endif


//----------------------------------------------------------------------------

CBaseOsContext * gpOsContext = NULL;

//----------------------------------------------------------------------------

char const CBaseOsContext::skTitleId_JP[kTitleIdLen] = "BLJM61001";
char const CBaseOsContext::skTitleId_NA[kTitleIdLen] = "BLUS30847";
char const CBaseOsContext::skTitleId_EU[kTitleIdLen] = "BLES01419";
char const CBaseOsContext::skTitleId_AP[kTitleIdLen] = "BLAS50383";
char const CBaseOsContext::skTitleId_KR[kTitleIdLen] = "BLKS20316";

// See 'CP4 PS3 Title Id Overview' for PS3 Title Ids
//
// NOTE (Correspondence 17th Aug and conference call 18th Aug):
// Bluepoint thought we�d get different Title Ids per SKU for PS3 Save Data.
// We now believe that SCEJ thought that save data would be shared across all SKUs so only one save data title id would be needed. 
// As the PS2 versions had incompatible save data for each SKU, we can�t easily do this. Need to find a solution.
//
// In call, KJP were OK with using disk title id for save data, as long as PSN version worked OK (as per original BP recommendation).
// This removes need for separate title ids for save directories for all SKUs.

#if MGS_VERSION==2

char const CBaseOsContext::skTitleId_JP_SaveData[kTitleIdLen] = "BLJM61001";
char const CBaseOsContext::skTitleId_NA_SaveData[kTitleIdLen] = "BLUS30847";
char const CBaseOsContext::skTitleId_EU_SaveData[kTitleIdLen] = "BLES01419";
char const CBaseOsContext::skTitleId_AP_SaveData[kTitleIdLen] = "BLAS50383";
char const CBaseOsContext::skTitleId_KR_SaveData[kTitleIdLen] = "BLKS20316";

#elif MGS_VERSION==3

char const CBaseOsContext::skTitleId_JP_SaveData[kTitleIdLen] = "BLJM61001";
char const CBaseOsContext::skTitleId_NA_SaveData[kTitleIdLen] = "BLUS30847";
char const CBaseOsContext::skTitleId_EU_SaveData[kTitleIdLen] = "BLES01419";
char const CBaseOsContext::skTitleId_AP_SaveData[kTitleIdLen] = "BLAS50383";
char const CBaseOsContext::skTitleId_KR_SaveData[kTitleIdLen] = "BLKS20316";

#endif

//----------------------------------------------------------------------------

CBaseOsContext::CBaseOsContext()
:  mShouldTerminateApplication(false)
,  mShutdownType( kST_StandardShutdown )
,  mShouldPauseApplication(false)
,  mApplicationPauseOnPadIntercepted(true)
,  mReleaseMode(false)
,  mDevelopMode(true)
,  mIsDownloadableVersion(false)
,  mLanguage(kL_English)
,  mBuildSKU(kBS_Europe)
,  mSystemRegion(kR_Europe)
,  mControllerRegion(kCR_USA)
,  mBootType(kBT_Disc)
,  mUnsafeToShutDownFlags( 0 )
{
   BPE_VERIFY(gpOsContext == NULL, false, "only one instance of os context supported");
   gpOsContext = this;

   // Initialize default title id
   strcpy(mTitleId, skTitleId_EU);
   strcpy(mTitleId_SaveData, skTitleId_EU_SaveData);

   strcpy( mBuildVersionString, "UNKNOWN" );
}

//----------------------------------------------------------------------------

CBaseOsContext::~CBaseOsContext()
{
}

//----------------------------------------------------------------------------

char const * CBaseOsContext::LanguageToString(ELanguage const language)
{
   switch (language)
   {
      case kL_English:
         return "ENGLISH";
      case kL_Japanese:
         return "JAPANESE";
      case kL_French:
         return "FRENCH";
      case kL_German:
         return "GERMAN";
      case kL_Italian:
         return "ITALIAN";
      case kL_Portuguese:
         return "PORTUGUESE";
      case kL_Spanish:
         return "SPANISH";
      case kL_Dutch:
         return "DUTCH";
   }
   // English by default
   return "ENGLISH";
}

//----------------------------------------------------------------------------

char const * CBaseOsContext::RegionToString(ESystemRegion const region)
{
   switch (region)
   {
      case kR_USA:
         return "USA";
      case kR_Japan:
         return "JAPAN";
      case kR_Europe:
         return "EUROPE";      
   }
   // US by default
   return "US";
}

//----------------------------------------------------------------------------

bool CBaseOsContext::GetApplicationPauseOnPadIntercepted() const
{
   return mApplicationPauseOnPadIntercepted;
}

//----------------------------------------------------------------------------

void CBaseOsContext::SetApplicationPauseOnPadIntercepted(bool const shouldPause)
{
   mApplicationPauseOnPadIntercepted = shouldPause;
}

//----------------------------------------------------------------------------

void CBaseOsContext::SetReleaseMode()
{
   mReleaseMode = true;
   // No develop mode if we're running in shipping mode
   mDevelopMode = false;
}

//----------------------------------------------------------------------------

void CBaseOsContext::SetDevelopMode()
{
   if (IsReleaseMode())
   {
      bpe_debugger_printf("CBaseOsContext: can't switch to develop mode if running in release mode!\n");
      return;
   }

   mDevelopMode = true;
}

//----------------------------------------------------------------------------

#if BPE_TARGET != BPE_TARGET_PS3
CBaseOsContext::EBootType CBaseOsContext::GetBootType() const
{
   // Function only supported on PS3
   BPE_VERIFYA(false, "CBaseOsContext::GetBootType() not supported on current platform!");
   return mBootType;
}
#endif

//----------------------------------------------------------------------------

void CBaseOsContext::ProcessPlatform()
{
}

//----------------------------------------------------------------------------

void CBaseOsContext::ForceSpecificLanguageBasedOnBuildSKU()
{
   switch (mBuildSKU)
   {
   case kBS_Europe:
      {
         if (  (mLanguage != kL_English) &&
            (mLanguage != kL_French) &&
            (mLanguage != kL_Italian) &&
            (mLanguage != kL_German) &&
            (mLanguage != kL_Spanish))
         {
            // Unknown language for this SKU, use English
            mLanguage = kL_English;
         }
      }
      break;

   case kBS_USA:
      if( (mLanguage != kL_English) &&
          (mLanguage != kL_French) && 
          (mLanguage != kL_Spanish) )
      {
         // Unknown language for this SKU, use English
         mLanguage = kL_English;
      }
      break;

   case kBS_Japan:
      mLanguage = kL_Japanese;
      break;

   default:
      BPE_VERIFYA(false, "Unknown build SKU");
      break;
   }
}

//----------------------------------------------------------------------------
// Custom functions for setting MGS languages to match application specific values
// Messy, but will work

// (from MGS2:g_define.h or MGS:game.h; they're the same)

enum {
   //	GM_LANG_JAPANESE			= 0,
   GM_LANG_DEFAULT				= 0,
   GM_LANG_ENGLISH				= 1,
   GM_LANG_ENGLISH_USA			= 1,
   GM_LANG_ENGLISH_ENGLAND		= 1,
   GM_LANG_FRENCH				= 2,
   GM_LANG_GERMANY				= 3,
   GM_LANG_ITALY 				= 4,
   GM_LANG_SPANISH				= 5,
   GM_LANG_KOREAN				= 6,
   GM_LANG_JAPANESE			= 7,
} ;

#if BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET == BPE_TARGET_X360
int gBP_DisableApplicationPause = 0;
#endif

extern "C"
{
#if BP_TGS_DEMO()
   int ENGINE_API gBP_DisableSaveLoadChecks = 1;
#else
   int ENGINE_API gBP_DisableSaveLoadChecks = 0;
#endif

int ENGINE_API BP_DisableSaveLoadChecks_c()
{
   return gBP_DisableSaveLoadChecks;
}

int ENGINE_API BP_COsContext_GetLanguage_MGS()
{
   // Activate language (including region override) already determined in PS3COsContext::InitializeBuildSKU
   BPE_VERIFY(gpOsContext != NULL, false, "gpOsContext not initialized.");

   switch( gpOsContext->mBuildSKU )
   {
   case COsContext::kBS_Europe:
      switch (gpOsContext->mLanguage)
      {
         case COsContext::kL_English:
            return GM_LANG_ENGLISH;

         case COsContext::kL_French:
            return GM_LANG_FRENCH;

         case COsContext::kL_German:
            return GM_LANG_GERMANY;

         case COsContext::kL_Italian:
            return GM_LANG_ITALY;

         case COsContext::kL_Spanish:
            return GM_LANG_SPANISH;
      }
      break;  
	  
   // Now we're using EU data for NA, enable support for French and Spanish
   case COsContext::kBS_USA: 
      switch (gpOsContext->mLanguage)
      {
         case COsContext::kL_French:
            return GM_LANG_FRENCH;

         case COsContext::kL_Spanish:
            return GM_LANG_SPANISH;

         default:
            return GM_LANG_ENGLISH;
      }
      break;
	  
   case COsContext::kBS_Japan:
      {
         return GM_LANG_JAPANESE;
      }
      break;
   }

   BPE_VERIFYA(false, "Invalid language!");
   return GM_LANG_DEFAULT;
}

//----------------------------------------------------------------------------

//Replacement for preprocessor defines that determine multilanguage behavior in Shadow
int ENGINE_API BP_COsContext_GetBuildSKU()
{
   return gpOsContext->mBuildSKU;
}

int ENGINE_API BP_COsContext_ShouldPauseApplication()
{
#if BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET == BPE_TARGET_X360
   if( gBP_DisableApplicationPause )
   {
      return false;
   }
   else
#endif
   {
      return gpOsContext->mShouldPauseApplication ? 1 : 0;
   }
}

}; //extern "C"

//----------------------------------------------------------------------------


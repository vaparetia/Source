//----------------------------------------------------------------------------
// COsContext.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

class ENGINE_API CBaseOsContext
{
public:
   CBaseOsContext();
   virtual ~CBaseOsContext();

   static std::string GetMachineName();
   static std::string GetCurrentUserName();

   enum EControllerRegion
   {
      kCR_USA,
      kCR_Japan,

      kCR_Count
   };

   // Build SKU determined from title id
   enum EBuildSKU
   {
      kBS_USA,
      kBS_Japan,
      kBS_Europe,      

      kBS_Count
   };

   // Use system region for determining if using UK or US English
   // Subset of EBuildSKU
   enum ESystemRegion
   {
      kR_USA,
      kR_Japan,
      kR_Europe,      

      kR_Count
   };

   enum ELanguage
   {
      kL_English,       // No setting for US/UK English, use in combination with ESystemRegion
      kL_Japanese,
      kL_French,
      kL_German,
      kL_Italian,
      kL_Portuguese,
      kL_Spanish,
      kL_Dutch,
      kL_Russian,

      kL_Count
   };

   enum EBootType
   {
      kBT_Disc,
      kBT_DiscPatch,
      kBT_HDD,          // HDD games never have a patch flag as per SDK documentation

      kBT_Count
   };

   enum EUnsafeToShutDownFlags
   {
      kUSDF_Trophies   = 1 << 0,
      kUSDF_SaveData   = 1 << 1,
      kUSDF_Precaching = 1 << 2
   };

   enum EProfileStatus
   {
      kPS_None,
      kPS_Valid,
      kPS_Guest,
   };

   enum EShutdownType
   {
      kST_StandardShutdown,      // Shutdown goes back to OS
      kST_RestartGameSession,    // Reboots the current game executable
      kST_ReturnToBootloader,    // Returns to a bootloader
      kST_LaunchPWForInvite      // Launches PW with command line notification of invite
   };

   static char const *
               LanguageToString(ELanguage const language);
   static char const *
               RegionToString(ESystemRegion const region);
   // We've got a couple of special cases for this because of NP commerce
   bool        GetApplicationPauseOnPadIntercepted() const;    
   void        SetApplicationPauseOnPadIntercepted(bool const shouldPause);

   void        SetReleaseMode();    // switch into release mode
   void        SetDevelopMode();    // switch into develop mode (not possible if already in release mode)
   bool        IsReleaseMode() const   { return mReleaseMode; };
   bool        IsDevelopMode() const   { return mDevelopMode; };
   bool        IsDownloadableVersion() const { return mIsDownloadableVersion; }

   EBootType   GetBootType() const;

   // Debug only helper function, do _not_ use for ReleaseMode builds.
   static void TerminateProcess();

   // Handle 'system' callbacks for PS3/X360
   virtual void        ProcessPlatform();

   void        SetUnsafeToShutDownFlag( uint32 const flag );
   void        ClearUnsafeToShutDownFlag( uint32 const flag );

   bool        IsSafeToShutDown() const;

   EProfileStatus GetUserProfile();

   void ForceSpecificLanguageBasedOnBuildSKU();

public:
   bool              mShouldTerminateApplication;
   EShutdownType     mShutdownType;
   bool              mShouldRebootApplication;
   bool              mShouldPauseApplication;
   bool              mApplicationPauseOnPadIntercepted;
   ELanguage         mLanguage;
   EBuildSKU         mBuildSKU;
   ESystemRegion     mSystemRegion;
   EControllerRegion mControllerRegion;
   EBootType         mBootType;

   // Title Id
   enum {         kTitleIdLen = 10 };
   char           mTitleId[kTitleIdLen];
   // KJP/SCEJ requested different title ids (vs disc) for MGS2 and MGS3
   // See doc: XXXXXXXXXXXXXX
   char           mTitleId_SaveData[kTitleIdLen];

   static char const skTitleId_EU[kTitleIdLen];
   static char const skTitleId_NA[kTitleIdLen];
   static char const skTitleId_JP[kTitleIdLen];
   static char const skTitleId_AP[kTitleIdLen]; // Aliases to NA SKU
   static char const skTitleId_KR[kTitleIdLen]; // Aliases to NA SKU

   static char const skTitleId_EU_SaveData[kTitleIdLen];
   static char const skTitleId_NA_SaveData[kTitleIdLen];
   static char const skTitleId_JP_SaveData[kTitleIdLen];
   static char const skTitleId_AP_SaveData[kTitleIdLen]; // Aliases to NA SKU
   static char const skTitleId_KR_SaveData[kTitleIdLen]; // Aliases to NA SKU

   char mBuildVersionString[ 64 ];

protected:
   bool           mReleaseMode;                 // Is running in shipping mode?
   bool           mDevelopMode;                 // Is developMode=1
   bool           mIsDownloadableVersion;

   volatile uint32   mUnsafeToShutDownFlags;
};

//----------------------------------------------------------------------------

extern ENGINE_API CBaseOsContext * gpOsContext;

extern "C" int ENGINE_API BP_COsContext_GetLanguage_MGS();

//----------------------------------------------------------------------------

#include BPE_PLATFORM_SPECIFIC2(Engine/System, COsContext.h)

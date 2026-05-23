//----------------------------------------------------------------------------
// Win32COsContext.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "COsContext.h"

#include <xtl.h>
#include <vector>

//----------------------------------------------------------------------------
// Device UI request structures
enum EDialogRequestType
{
   kDRT_ProfileStatusChanged,
   kDRT_DeviceSelector,
   kDRT_NotEnoughSpace,
   kDRT_StorageDeviceUnavailable,
   kDRT_ExitGameplay,
};

enum EDialogRequestStatus_Reserved
{
   kDRS_R__Waiting,
   kDRS_R__Done,
   kDRS_R__Count,
};


enum EDialogRequestStatus_ProfileStatusChanged
{
   kDRS_PSC__CheckifProfileActuallyChanged = kDRS_R__Count,
   kDRS_PSC__DisplayProfileChangedDialog,
   kDRS_PSC__DisplayingProfileChangedDialog,
   kDRS_PSC__Rebooting,

   kDRS_PSC__AlmostDone,
};

enum EDialogRequestStatus_DeviceSelector
{
   kDRS_DS__DisplayDeviceSelector = kDRS_R__Count,
   kDRS_DS__DisplayingDisplayDeviceSelector,

   kDRS_DS__Check_ValidDevice,

   kDRS_DS__DisplayNoDeviceSelectedDialog,
   kDRS_DS__DisplayingNoDeviceSelectedDialog,

   kDRS_DS__CheckDeviceForFreeSpace,
   kDRS_DS__CheckingDeviceForFreeSpace,
   kDRS_DS__DisplayDeviceNoFreeSpace,
   kDRS_DS__DisplayingDeviceNoFreeSpace,

   kDRS_DS__AlmostDone,
};

enum EDialogRequestStatus_NotEnoughSpace
{
   kDRS_NES__DisplayNotEnoughSaveDialog = kDRS_R__Count,
   kDRS_NES__DisplayingNotEnoughSaveDialog,

   kDRS_NES__AlmostDone,
};

enum EDialogRequestStatus_StorageDeviceUnavailable
{
   kDRS_SDU__DisplayStroageDeviceUnavailableDialog = kDRS_R__Count,
   kDRS_SDU__DisplayingStroageDeviceUnavailableDialog,

   kDRS_SDU__AlmostDone,
};

enum EDialogRequestStatus_ExitGameplay
{
   kDRS_EG__DisplayExitGameplayDialog = kDRS_R__Count,
   kDRS_EG__DisplayingExitGameplayDialog,

   kDRS_EG__AlmostDone,
};

struct OSDialogRequest
{
   EDialogRequestType mType;

   int mStatus;
   uint32 mExtraInformation;

   OSDialogRequest(EDialogRequestType dialogRequestType)
      : mType(dialogRequestType)
      , mStatus(kDRS_R__Waiting)
   {
   }

   bool IsDone() const { return mStatus == kDRS_R__Done; }
};

//----------------------------------------------------------------------------
class ENGINE_API COsContext : public CBaseOsContext
{
public:
   COsContext();
   ~COsContext();

   virtual void         ProcessPlatform();

   void DeleteDialogRequest(OSDialogRequest* pDR);

   OSDialogRequest*     ShowDeviceUI(uint32 neededSize);
   OSDialogRequest*     ShowDeviceUnavailableUI();

   enum { kInvalidUser = 0xffffffff };

   void GetUserProfile_();

   void                 StartUIRequest();
   bool                 DoneWithUIWait();
   bool                 IsYAxisInverted() const { return mInvertYAxis; }

   void                 AddAchievement(XUSER_ACHIEVEMENT const & Achievement);

public:
   BPE_HANDLE           mInstanceHandle;
   BPE_HANDLE           mMainWindow;
   unsigned int         mInitialFPUControlWord;

   static uint32 const  skFPUControlWordMask;
   static uint32 const  skFPUControlWordExceptions;

   uint32               mSignedInUser;
   XCONTENTDEVICEID     mSaveLoadDeviceId;
   bool                 mShouldCheckForFreeSpace;
   bool                 mDidEverHaveValidDevice;
   bool                 mHasGottenSaveDeviceChangedMessage;

   bool                 mDisableAchievements;


   HANDLE               mNotificationListener;

   bool                 mIsShowingUI;
   bool                 mIsOpeningUI;
   bool                 mIsClosingUI;   

   enum EInternalProfileStatus
   {
      kIPS_CheckingForSignIn,
      kIPS_DisplaySignInDialog,
      kIPS_DisplayingSignInDialog,

      kIPS_HasGottenProfile,
      kIPS_GetProfileInformation,
      kIPS_GettingProfileInformation,
      kIPS_GetPostProfileInformation,
      kIPS_Done,
   };
   EProfileStatus mProfileStatus;
   bool mCaresAboutChangeProfileEvents;
   bool mCanCareAboutChangeProfileEvents;
   EInternalProfileStatus mInteralProfileStatus;
   XUSER_READ_PROFILE_SETTING_RESULT* mpProfileSettingsResult;
   bool mInvertYAxis;
   int mLastExitGameRequestRealResult;

   // If this is set all we do is tell the user they are going back to the title screen, 
   // once they accept their fate by pressing A or dismissing the guide via the middle X Box button
   // the application restarts the current .xex
   bool CheckIfProfileActuallyChanged();
   enum EInternalProfileChangedStatus
   {
      kIPCS_Idle,
      kIPCS_DisplayProfileChangedDialog,
      kIPCS_DisplayingProfileChangedDialog,
      kIPCS_Rebooting,
   };
   XUID                 mLastKnownOfflineXUID_User0;
   XUSER_SIGNIN_INFO    mUserSignInInfo_User0;
   XOVERLAPPED mInternalProfileStatusOverlapped;

   void AddDialogRequest(OSDialogRequest* pDR)
   {
      EnterCriticalSection(&mDialogRequestCriticalSection);
      maCurrentDialogRequest.push_back(pDR);
      LeaveCriticalSection(&mDialogRequestCriticalSection);
   }

   bool IsPlayerActive() const { return mIsSignedInPlayerActive; }
   

private:
   // Misc initialization functions
   void                    InitializeBuildSKU();
   void                    InitializeLanguage();

   void Handle__kDRT_ProfileStatusChanged();
   void Handle__kDRT_DeviceSelector();
   void Handle__kDRT_NotEnoughSpace();
   void Handle__kDRT_StorageDeviceUnavailable();
   void Handle__kDRT_ExitGameplay();
   void HandleDialogRequest();
   void HandleAchievement();

   MESSAGEBOX_RESULT mDialogRequestMessageBoxResult;
   XOVERLAPPED mDialogRequestOverlapped;
   OSDialogRequest* mpCurrentDialogRequest;
   std::vector<OSDialogRequest*> maCurrentDialogRequest;
   CRITICAL_SECTION mDialogRequestCriticalSection;

   XOVERLAPPED mAchievementOverlapped;
   int mHasPendingAchivements;
   std::vector<XUSER_ACHIEVEMENT> maCurrentDialogRequest_UT;
   std::vector<XUSER_ACHIEVEMENT> maCurrentDialogRequest_RT;
   CRITICAL_SECTION mAchievementCriticalSection;

   bool mIsSignedInPlayerActive;
   bool mNoControllerActivity;
};

//----------------------------------------------------------------------------

__forceinline COsContext * OsContext()
{
   return static_cast<COsContext*>(gpOsContext);
}



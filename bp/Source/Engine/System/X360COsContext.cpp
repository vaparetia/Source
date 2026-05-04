//----------------------------------------------------------------------------
// X360COsContext.cpp
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"

#include BPE_PLATFORM_SPECIFIC( COsContext.h )

#define NPL "(Not localized!) "

#include "CStopWatch.h"

#include <xtl.h>
#include <Xbdm.h>
#include "Renderer/Base/Backend/X360/X360CRenderBackend.h"

int gCommandLineHadProfileInfo = 0;
int gCommandLineActiveController = 0;
int gCommandLineProfileStatus = CBaseOsContext::kPS_None;
int gCommandLineYAxisInverted = false;

int gRemappedControllerIndex[4] = {-1, -1, -1, -1};
int gMainControllerDisconnected = 0;

int gBP_DisableMemoryCardSizeCheck = 0;

extern "C"
{
   extern int gBP_DisableSaveLoadChecks;
}

char* Ok_MessageButtonCaptions[][1] = 
{
   {"Ok"},//kL_English
   {"OK"},//kL_Japanese
   {"OK"},//kL_French
   {"OK"},//kL_German
   {"OK"},//kL_Italian
   {NPL "OK"},//kL_Portuguese
   {"Aceptar"},//kL_Spanish
};
char* ProfileChanged_Title[] = 
{
   "Sign-In Change",//kL_English
   "サインインの変更",//kL_Japanese
   "Modification de connexion",//kL_French
   "Anmeldestatus geändert",//kL_German
   "Cambiamento di connessione",//kL_Italian
   NPL "Sign-In Change",//kL_Portuguese
   "Cambiar inicio de sesión",//kL_Spanish
};
char* ProfileChanged_Message[] = 
{
   "The Primary user's sign-in status has changed. You must go back to the title screen",//kL_English
   "メインユーザーのサインイン状況が変更されました。タイトル画面に戻る必要があります",//kL_Japanese
   "Le statut de connexion de l'utilisateur principal a été modifié. Veuillez revenir à l'écran principal.",//kL_French
   "Der Anmeldestatus des Hauptnutzers hat sich geändert. Bitte wechseln Sie zum Titelbildschirm.",//kL_German
   "Si è verificato un cambiamento di connessione dell'utente principale. Verrai riportato alla schermata del titolo.",//kL_Italian
   NPL "The Primary user's sign-in status has changed. You must go back to the title screen",//kL_Portuguese
   "El estado de la sesión del usuario principal ha cambiado. Debes volver a la pantalla de título.",//kL_Spanish
};

char* FailedToSave_Message[] = 
{
   "Failed to Save",//kL_English
   "セーブに失敗しました",//kL_Japanese
   "Échec de la sauvegarde",//kL_French
   "Speichern fehlgeschlagen.",//kL_German
   "Impossibile salvare.",//kL_Italian
   NPL "Failed to Save",//kL_Portuguese
   "Error al guardar",//kL_Spanish
};

char* NotEnoughSpaceSave_Message[] = 
{
   "Unable to save. There is not enough free space on the selected storage device.",//kL_English
   "セーブできません。選択されたデータ保存機器に空き容量が不足しています。",//kL_Japanese
   "Échec de la sauvegarde. Le périphérique de stockage sélectionné ne dispose pas de l'espace libre nécessaire.",//kL_French
   "Speichern nicht möglich. Das ausgewählte Gerät enthält nicht genügend freien Speicherplatz.",//kL_German
   "Impossibile salvare. La periferica di memorizzazione selezionata non ha spazio libero a sufficienza.",//kL_Italian
   NPL "Unable to save. There is not enough free space on the selected storage device.",//kL_Portuguese
   "Imposible guardar. El espacio disponible del dispositivo seleccionado no es suficiente.",//kL_Spanish
};

char* StorageDeviceUnavailable_Message[] = 
{
   "The in-use storage device is unavailable",//kL_English
   "使用中のデータ保存機器は利用できません",//kL_Japanese
   "Le périphérique de stockage utilisé est indisponible.",//kL_French
   "Das verwendete Speichergerät ist nicht verfügbar.",//kL_German
   "La periferica di memorizzazione in uso non è disponibile.",//kL_Italian
   NPL "The in-use storage device is unavailable",//kL_Portuguese
   "Dispositivo de almacenamiento no disponible",//kL_Spanish
};

//
char* YesNoSelectDevice_MessageButtonCaptions[][2] = 
{
   //kL_English
   {
      "Yes",
      "No, Select Device", 
   }, 

   //kL_Japanese
   {
      "はい",
      "機器を選択する",
   }, 

   //kL_French
   {
      "Oui", 
      "Non, sélectionner un périphérique",
   }, 

   //kL_German
   {
      "Ja",
      "Nein, Gerät wählen",
   }, 

   //kL_Italian
   {
      "Sì", 
      "No, seleziona periferica",
   }, 

   //kL_Portuguese
   {
      NPL "Yes",
      NPL "No, Select Device", 
   }, 

   //kL_Spanish
   {
      "Sí",
      "No, seleccionar dispositivo", 
   }, 
};

char* YesNoGeneric_MessageButtonCaptions[][2] = 
{
   //kL_English
   {
      "Yes",
      "No", 
   }, 

   //kL_Japanese
   {
      "はい",
      "いいえ",
   }, 

   //kL_French
   {
      "Oui", 
      "Non",
   }, 

   //kL_German
   {
      "Ja",
      "Nein",
   }, 

   //kL_Italian
   {
      "Sì", 
      "No",
   }, 

   //kL_Portuguese
   {
      NPL "Yes",
      NPL "No", 
   }, 

   //kL_Spanish
   {
      "Sí",
      "No",
   }, 
};

char* Warning_Title[] = 
{
   "Warning",//kL_English
   "警告",//kL_Japanese
   "Attention",//kL_French
   "Warnung",//kL_German
   "Attenzione",//kL_Italian
   NPL "Warning",//kL_Portuguese
   "Atención",//kL_Spanish
};
char* NoDeviceSelected_Message[] = 
{
   "You have not selected a storage device. You will not be able to save your progress. Are you sure you would like to proceed?",//kL_English
   "利用可能なデータ保存機器がありません。このままゲームを開始しますか？",//kL_Japanese
   "Aucun périphérique de stockage n'est disponible. Voulez-vous démarrer le jeu en l'état ?",//kL_French
   "Es ist kein Speichergerät verfügbar. Möchten Sie das Spiel trotzdem starten?",//kL_German
   "Nessuna periferica di memorizzazione disponibile. Vuoi avviare lo stesso il gioco?",//kL_Italian
   NPL "You have not selected a storage device. You will not be able to save your progress. Are you sure you would like to proceed?",//kL_Portuguese
   "No hay ningún dispositivo de almacenamiento. ¿Quieres empezar la partida?",//kL_Spanish
};

char* ExitGameplay_Message[] = 
{
   "Are you sure you want to exit? Any unsaved progress will be lost.",//kL_English
   "終了してもよろしいですか？　セーブされていない進行状況は失われます。",//kL_Japanese
   "Voulez-vous vraiment quitter ? Les progressions non sauvegardées seront perdues.",//kL_French
   "Möchten Sie das Spiel wirklich verlassen? Nicht gespeicherte Fortschritte gehen verloren.",//kL_German
   "Vuoi davvero uscire? Tutti i progressi non salvati andranno persi.",//kL_Italian
   NPL "Are you sure you want to exit? Any unsaved progress will be lost.",//kL_Portuguese
   "¿Seguro que quieres salir? Perderás los progresos no guardados",//kL_Spanish
};

//----------------------------------------------------------------------------

uint32 const COsContext::skFPUControlWordMask = _MCW_DN | _MCW_IC | _MCW_RC | _MCW_PC;
uint32 const COsContext::skFPUControlWordExceptions = _EM_INVALID|_EM_ZERODIVIDE;

//----------------------------------------------------------------------------

HRESULT BP_XShowMessageBoxUI_UTF8(
                                  DWORD dwUserIndex, 
                                  char* wszTitle, 
                                  char* wszText, 
                                  DWORD cButtons, 
                                  char** pwszButtons, 
                                  DWORD dwFocusButton, 
                                  DWORD dwFlags, 
                                  MESSAGEBOX_RESULT *pResult, 
                                  XOVERLAPPED *pOverlapped
                                  )
{
   BPE_VERIFY(cButtons < 4, false, "BP_XShowMessageBoxUI_UTF8 only supports 3 buttons!");

   static WCHAR __wszTitle[MAX_PATH];
   static WCHAR __wszText[MAX_PATH];
   static WCHAR __pwszButtons[4][MAX_PATH];
   static LPCWSTR __apwszButtons[4];

   MultiByteToWideChar(CP_UTF8, 0, wszTitle, -1, __wszTitle, MAX_PATH);
   MultiByteToWideChar(CP_UTF8, 0, wszText, -1, __wszText, MAX_PATH);
   for( int ii=0; ii<cButtons; ++ii )
   {
      MultiByteToWideChar(CP_UTF8, 0, pwszButtons[ii], -1, __pwszButtons[ii], MAX_PATH);
      __apwszButtons[ii] = __pwszButtons[ii];
   }

   return XShowMessageBoxUI
      (
      dwUserIndex,
      __wszTitle,
      __wszText,
      cButtons,
      __apwszButtons,
      dwFocusButton,
      dwFlags,
      pResult,
      pOverlapped
      );
}

//----------------------------------------------------------------------------

COsContext::COsContext()
:  CBaseOsContext()
,  mInstanceHandle(NULL)
,  mMainWindow(NULL)
,  mSignedInUser( gCommandLineActiveController )
,  mSaveLoadDeviceId(XCONTENTDEVICE_ANY)
,  mIsShowingUI(false)
,  mIsOpeningUI(false)
,  mIsClosingUI(false)
,  mProfileStatus((EProfileStatus)gCommandLineProfileStatus)
,  mCaresAboutChangeProfileEvents(true) // By default we care about profile change events
,  mCanCareAboutChangeProfileEvents(false) // we can not care about profile change events until we have a valid sign in thou
,  mInteralProfileStatus(kIPS_CheckingForSignIn)
,  mpCurrentDialogRequest(NULL)
,  mpProfileSettingsResult(NULL)
,  mInvertYAxis(gCommandLineYAxisInverted != 0)
,  mLastExitGameRequestRealResult( 0 )
,  mShouldCheckForFreeSpace(true)
,  mDidEverHaveValidDevice(false)
,  mHasPendingAchivements(0)
,  mHasGottenSaveDeviceChangedMessage(false)
,  mIsSignedInPlayerActive(false)
,  mNoControllerActivity(false)
,  mDisableAchievements(false)
{
   // fill in controller remap table
   gRemappedControllerIndex[0] = mSignedInUser;
   // look for first controller that is not us
   int iControllerNum = 0;
   if( iControllerNum == mSignedInUser )
   {
      ++iControllerNum;
   }
   for( int ii=1; ii<4; ++ii)
   {
      gRemappedControllerIndex[ii] = iControllerNum;
      ++iControllerNum;
      if( iControllerNum == mSignedInUser )
      {
         ++iControllerNum;
      }
   }

   ZeroMemory( &mUserSignInInfo_User0, sizeof(mUserSignInInfo_User0) );
   ZeroMemory( &mLastKnownOfflineXUID_User0, sizeof(mLastKnownOfflineXUID_User0) );

   if( gCommandLineHadProfileInfo )
   {
      mInteralProfileStatus = kIPS_GetPostProfileInformation;
   }
   // Keep initial FPU control word state for later reference
   _controlfp_s(&mInitialFPUControlWord, 0, 0);

   InitializeCriticalSection(&mDialogRequestCriticalSection);
   InitializeCriticalSection(&mAchievementCriticalSection);

   ZeroMemory( &mDialogRequestOverlapped, sizeof( XOVERLAPPED ) );
   ZeroMemory( &mAchievementOverlapped, sizeof( XOVERLAPPED ) );

   XOnlineStartup();
   mNotificationListener = XNotifyCreateListener(XNOTIFY_SYSTEM);

   InitializeBuildSKU();
   InitializeLanguage();
}

//----------------------------------------------------------------------------

COsContext::~COsContext()
{
   // TODO: verify this gets hit and check when, before checking in
   XCloseHandle(mNotificationListener);
   XOnlineCleanup();

   DeleteCriticalSection(&mDialogRequestCriticalSection);
   DeleteCriticalSection(&mAchievementCriticalSection);
}

//---------------------------------------------------------------------------
bool COsContext::CheckIfProfileActuallyChanged()
{
   if( XUserGetSigninInfo( mSignedInUser, XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY, &mUserSignInInfo_User0 ) != ERROR_SUCCESS )
   {
      // if we can not get the status of the user the obviously are not the same user!
      return true;
   }
   if( memcmp(&mLastKnownOfflineXUID_User0, &mUserSignInInfo_User0.xuid, sizeof(XUID)) != 0 )
   {
      return true;
   }
   return false;
}

//---------------------------------------------------------------------------
extern "C"
{
   extern char* gpExeToRelaunch;
}

inline int GetRealResultFromMessageBox(int defaultResult, MESSAGEBOX_RESULT* pMessageBoxResult, XOVERLAPPED* pOverlapped )
{
   int returnResult = -1;
   DWORD dwResult = XGetOverlappedResult( pOverlapped, NULL, TRUE );
   switch( dwResult )
   {
   case ERROR_SUCCESS:
      {
         switch( pMessageBoxResult->dwButtonPressed )
         {
         case XMB_CANCELID:
            {
               returnResult = defaultResult;
            }
            break;
         default:
            {
               // Invalid option
               // Lets ask them again
               returnResult = pMessageBoxResult->dwButtonPressed;
            }
            break;
         }
      }
      break;
   case ERROR_CANCELLED: // fall thru
   default:
      {
         returnResult = defaultResult;
      }
      break;
   }
   return returnResult;
}

void COsContext::Handle__kDRT_ProfileStatusChanged()
{
   switch(mpCurrentDialogRequest->mStatus)
   {
   case kDRS_R__Waiting:
      {
         mpCurrentDialogRequest->mStatus = kDRS_PSC__Rebooting;
      }
      break;
   case kDRS_PSC__DisplayProfileChangedDialog:
      {
         ZeroMemory( &mDialogRequestOverlapped, sizeof( XOVERLAPPED ) );
         DWORD dwRet = BP_XShowMessageBoxUI_UTF8(
            mSignedInUser,
            ProfileChanged_Title[mLanguage], 
            ProfileChanged_Message[mLanguage],
            1, 
            Ok_MessageButtonCaptions[mLanguage], 
            0, 
            XMB_WARNINGICON, 
            &mDialogRequestMessageBoxResult, 
            &mDialogRequestOverlapped );
         StartUIRequest();
         mpCurrentDialogRequest->mStatus = kDRS_PSC__DisplayingProfileChangedDialog;
      }
      break;
   case kDRS_PSC__DisplayingProfileChangedDialog:
      {
         if( XHasOverlappedIoCompleted(&mDialogRequestOverlapped) )
         {
            mpCurrentDialogRequest->mStatus = kDRS_PSC__Rebooting;
         }
      }
      break;
   case kDRS_PSC__Rebooting:
      {
         // restart the application
         XLaunchNewImage( gpExeToRelaunch, XLAUNCH_FLAG_MATCH_MIN_VERSION );
         mpCurrentDialogRequest->mStatus = kDRS_PSC__AlmostDone;
      }  
      break;
   case kDRS_PSC__AlmostDone:
      {
         delete mpCurrentDialogRequest;
         mpCurrentDialogRequest->mStatus = kDRS_R__Done;
         mpCurrentDialogRequest = NULL;
      }
      break;
   }
}

void COsContext::Handle__kDRT_DeviceSelector()
{
   switch(mpCurrentDialogRequest->mStatus)
   {
   case kDRS_R__Waiting:
      {
         mpCurrentDialogRequest->mStatus = kDRS_DS__DisplayDeviceSelector;
      }
      break;
   case kDRS_DS__DisplayDeviceSelector:
      {
         mSaveLoadDeviceId = XCONTENTDEVICE_ANY;
         ULARGE_INTEGER bytesRequested = {0};
         if( gBP_DisableMemoryCardSizeCheck == 0 )
         {
            bytesRequested.LowPart = mpCurrentDialogRequest->mExtraInformation;
         }
         DWORD dwContentFlags = 0;
         if( mDidEverHaveValidDevice )
         {
            dwContentFlags |= XCONTENTFLAG_FORCE_SHOW_UI;
         }

         ZeroMemory( &mDialogRequestOverlapped, sizeof( XOVERLAPPED ) );
         DWORD dwRet = XShowDeviceSelectorUI( 
            mSignedInUser,
            XCONTENTTYPE_SAVEDGAME,
            dwContentFlags,
            bytesRequested,
            &mSaveLoadDeviceId,
            &mDialogRequestOverlapped );
         StartUIRequest();
         mpCurrentDialogRequest->mStatus = kDRS_DS__DisplayingDisplayDeviceSelector;
      }
      break;
   case kDRS_DS__DisplayingDisplayDeviceSelector:
      {
         if( XHasOverlappedIoCompleted(&mDialogRequestOverlapped) )
         {
            mpCurrentDialogRequest->mStatus = kDRS_DS__Check_ValidDevice;
         }
      }
      break;
   case kDRS_DS__Check_ValidDevice:
      {
        if( mSaveLoadDeviceId != XCONTENTDEVICE_ANY )
        {
            mpCurrentDialogRequest->mStatus = kDRS_DS__AlmostDone;
        }
        else
        {
             mpCurrentDialogRequest->mStatus = kDRS_DS__DisplayNoDeviceSelectedDialog;
        }
      }  
      break;
   //
   case kDRS_DS__DisplayNoDeviceSelectedDialog:
      {
         ZeroMemory( &mDialogRequestOverlapped, sizeof( XOVERLAPPED ) );
         DWORD dwRet = BP_XShowMessageBoxUI_UTF8(
            mSignedInUser,
            Warning_Title[mLanguage], 
            NoDeviceSelected_Message[mLanguage],
            2, 
            YesNoSelectDevice_MessageButtonCaptions[mLanguage], 
            0, 
            XMB_WARNINGICON, 
            &mDialogRequestMessageBoxResult, 
            &mDialogRequestOverlapped );
         StartUIRequest();
         mpCurrentDialogRequest->mStatus = kDRS_DS__DisplayingNoDeviceSelectedDialog;
      }
      break;
   case kDRS_DS__DisplayingNoDeviceSelectedDialog:
      {
         if( XHasOverlappedIoCompleted(&mDialogRequestOverlapped) )
         {
            int realResult = GetRealResultFromMessageBox(0, &mDialogRequestMessageBoxResult, &mDialogRequestOverlapped);
            if( realResult == 0 ) // Selected Yes
            {
               mpCurrentDialogRequest->mStatus = kDRS_DS__AlmostDone;
            }
            else// Selected No
            {
               mpCurrentDialogRequest->mStatus = kDRS_DS__DisplayDeviceSelector;
            }
         }
      }
      break;
   case kDRS_DS__AlmostDone:
      {
         mpCurrentDialogRequest->mStatus = kDRS_R__Done;
         mpCurrentDialogRequest = NULL;
         mShouldCheckForFreeSpace = false;
         mDidEverHaveValidDevice = mSaveLoadDeviceId != XCONTENTDEVICE_ANY;
         mHasGottenSaveDeviceChangedMessage = false;
      }
      break;
   }
}

void COsContext::Handle__kDRT_NotEnoughSpace()
{
   switch(mpCurrentDialogRequest->mStatus)
   {
   case kDRS_R__Waiting:
      {
         mpCurrentDialogRequest->mStatus = kDRS_NES__DisplayNotEnoughSaveDialog;
      }
      break;
   case kDRS_NES__DisplayNotEnoughSaveDialog:
      {
         ZeroMemory( &mDialogRequestOverlapped, sizeof( XOVERLAPPED ) );
         DWORD dwRet = BP_XShowMessageBoxUI_UTF8(
            mSignedInUser,
            Warning_Title[mLanguage], 
            NotEnoughSpaceSave_Message[mLanguage],
            1, 
            Ok_MessageButtonCaptions[mLanguage], 
            0, 
            XMB_WARNINGICON, 
            &mDialogRequestMessageBoxResult, 
            &mDialogRequestOverlapped );
         StartUIRequest();
         mpCurrentDialogRequest->mStatus = kDRS_NES__DisplayingNotEnoughSaveDialog;
      }
      break;
   case kDRS_NES__DisplayingNotEnoughSaveDialog:
      {
         if( XHasOverlappedIoCompleted(&mDialogRequestOverlapped) )
         {
            mpCurrentDialogRequest->mStatus = kDRS_NES__AlmostDone;
         }
      }
      break;
   case kDRS_NES__AlmostDone:
      {
         mpCurrentDialogRequest->mStatus = kDRS_R__Done;
         mpCurrentDialogRequest = NULL;
      }
      break;
   }
}

void COsContext::Handle__kDRT_StorageDeviceUnavailable()
{
   switch(mpCurrentDialogRequest->mStatus)
   {
   case kDRS_R__Waiting:
      {
         mpCurrentDialogRequest->mStatus = kDRS_SDU__DisplayStroageDeviceUnavailableDialog;
      }
      break;
   case kDRS_SDU__DisplayStroageDeviceUnavailableDialog:
      {
         ZeroMemory( &mDialogRequestOverlapped, sizeof( XOVERLAPPED ) );
         DWORD dwRet = BP_XShowMessageBoxUI_UTF8(
            mSignedInUser,
            Warning_Title[mLanguage], 
            StorageDeviceUnavailable_Message[mLanguage],
            1, 
            Ok_MessageButtonCaptions[mLanguage], 
            0, 
            XMB_WARNINGICON, 
            &mDialogRequestMessageBoxResult, 
            &mDialogRequestOverlapped );
         StartUIRequest();
         mpCurrentDialogRequest->mStatus = kDRS_PSC__DisplayingProfileChangedDialog;
      }
      break;
   case kDRS_SDU__DisplayingStroageDeviceUnavailableDialog:
      {
         if( XHasOverlappedIoCompleted(&mDialogRequestOverlapped) )
         {
            mpCurrentDialogRequest->mStatus = kDRS_SDU__AlmostDone;
         }
      }
      break;
   case kDRS_SDU__AlmostDone:
      {
         mpCurrentDialogRequest->mStatus = kDRS_R__Done;
         mpCurrentDialogRequest = NULL;
      }
      break;
   }
}

void COsContext::Handle__kDRT_ExitGameplay()
{
   switch(mpCurrentDialogRequest->mStatus)
   {
   case kDRS_R__Waiting:
      {
         mpCurrentDialogRequest->mStatus = kDRS_SDU__DisplayStroageDeviceUnavailableDialog;
      }
      break;
   case kDRS_EG__DisplayExitGameplayDialog :
      {
         ZeroMemory( &mDialogRequestOverlapped, sizeof( XOVERLAPPED ) );
         DWORD dwRet = BP_XShowMessageBoxUI_UTF8(
            mSignedInUser,
            Warning_Title[mLanguage], 
            ExitGameplay_Message[mLanguage],
            2, 
            YesNoGeneric_MessageButtonCaptions[mLanguage], 
            1,             //BP JG - default to NO
            XMB_WARNINGICON, 
            &mDialogRequestMessageBoxResult, 
            &mDialogRequestOverlapped );
         StartUIRequest();
         mpCurrentDialogRequest->mStatus = kDRS_EG__DisplayingExitGameplayDialog;
      }
      break;
   case kDRS_EG__DisplayingExitGameplayDialog:
      {
         if( XHasOverlappedIoCompleted(&mDialogRequestOverlapped) )
         {
            // Default for "Cancel" is 1 -> no.
            int realResult = GetRealResultFromMessageBox(1, &mDialogRequestMessageBoxResult, &mDialogRequestOverlapped);
            //0 means yes, 1 means no
            mLastExitGameRequestRealResult = realResult;
            mpCurrentDialogRequest->mStatus = kDRS_EG__AlmostDone;
         }
      }
      break;
   case kDRS_EG__AlmostDone:
      {
         mpCurrentDialogRequest->mStatus = kDRS_R__Done;
         mpCurrentDialogRequest = NULL;
      }
      break;
   }
}

//----------------------------------------------------------------------------
void COsContext::HandleDialogRequest()
{
   // If necessary get new current dialog request
   if( mpCurrentDialogRequest == NULL )
   {
      EnterCriticalSection(&mDialogRequestCriticalSection);
      std::vector<OSDialogRequest*>::iterator itHighestPrio = maCurrentDialogRequest.end();
      for( std::vector<OSDialogRequest*>::iterator it=maCurrentDialogRequest.begin(); it != maCurrentDialogRequest.end(); ++it )
      {
         // we must handle these according to priority (priority is determined by type, 0 hi, + low)
         OSDialogRequest* pDialogRequest = *it;
         if( itHighestPrio == maCurrentDialogRequest.end() )
         {
            itHighestPrio = it;
         }
         else if( pDialogRequest->mType < (*itHighestPrio)->mType )
         {
            itHighestPrio = it;
         }
      }
      if( itHighestPrio != maCurrentDialogRequest.end() )
      {
         mpCurrentDialogRequest = *itHighestPrio;
         maCurrentDialogRequest.erase( itHighestPrio );
      }
      LeaveCriticalSection(&mDialogRequestCriticalSection);
   }
   // process current dialog request
   if( mpCurrentDialogRequest )
   {
      if( DoneWithUIWait() )
      {
         switch( mpCurrentDialogRequest->mType )
         {
         case kDRT_ProfileStatusChanged:
            {
               Handle__kDRT_ProfileStatusChanged();
            }
            break;
         case kDRT_DeviceSelector:
            {
               Handle__kDRT_DeviceSelector();
            }
            break;
         case kDRT_NotEnoughSpace:
            {
               Handle__kDRT_NotEnoughSpace();
            }
            break;
         case kDRT_StorageDeviceUnavailable:
            {
               Handle__kDRT_StorageDeviceUnavailable();
            }
            break;
         case kDRT_ExitGameplay:
            {
               Handle__kDRT_ExitGameplay();
            }
            break;
         }
      }
   }
}

//----------------------------------------------------------------------------
static CStopWatch sAchievementAddedStopWatch;
void COsContext::HandleAchievement()
{
   if( !mDisableAchievements )
   {
      if( XHasOverlappedIoCompleted(&mAchievementOverlapped) )
      {
         maCurrentDialogRequest_RT.clear();
         mHasPendingAchivements = 0;
      }
      if( !mHasPendingAchivements )
      {
         {
            EnterCriticalSection(&mAchievementCriticalSection);
            if( maCurrentDialogRequest_UT.size() )
            {
               // if 4 frames with no achievements added
               if( sAchievementAddedStopWatch.GetElapsedMilliseconds() > 16*4 )
               {
                  maCurrentDialogRequest_RT.swap(maCurrentDialogRequest_UT);
               }
            }
            LeaveCriticalSection(&mAchievementCriticalSection);
         }
         if( maCurrentDialogRequest_RT.size() )
         {
            ZeroMemory( &mAchievementOverlapped, sizeof( XOVERLAPPED ) );
            XUserWriteAchievements(maCurrentDialogRequest_RT.size(), &maCurrentDialogRequest_RT[0], &mAchievementOverlapped);
            mHasPendingAchivements = 1;
         }
      }
   }
}

void COsContext::AddAchievement(XUSER_ACHIEVEMENT const & Achievement)
{
   EnterCriticalSection(&mAchievementCriticalSection);
   sAchievementAddedStopWatch.Reset();
   maCurrentDialogRequest_UT.push_back(Achievement);
   LeaveCriticalSection(&mAchievementCriticalSection);
}

//----------------------------------------------------------------------------
static CStopWatch sUIStopWatchOpenUI;
static CStopWatch sUIStopWatchCloseUI;
void COsContext::ProcessPlatform()
{
   GetUserProfile_();

   {      
      // Process dialog requests
      HandleDialogRequest();
   }

   gBP_DisableSaveLoadChecks = (mSaveLoadDeviceId == XCONTENTDEVICE_ANY) && (!mShouldCheckForFreeSpace);
   mShouldPauseApplication = (mIsShowingUI) || gMainControllerDisconnected;
   // mNoControllerActivity == false, not in super long demo/codec
   mIsSignedInPlayerActive = mNoControllerActivity == false;

   DWORD MessageID;
   ULONG_PTR Params;
   DWORD Result = 1;
   while( Result )
   {
      Result = XNotifyGetNext(mNotificationListener, 0, &MessageID, &Params);
      if( Result )
      {
         if( MessageID == XN_SYS_UI )
         {
            if( Params )
            {
               // We are about to show a dialog
               sUIStopWatchOpenUI.Reset();
               mIsOpeningUI = true;
               mIsShowingUI = true;
            }
            else
            {
               // We are about to hide a dialog
               sUIStopWatchCloseUI.Reset();
               mIsShowingUI = false;
               mIsClosingUI = true;
            }
         }
         else if( MessageID == XN_SYS_STORAGEDEVICESCHANGED )
         {
            mHasGottenSaveDeviceChangedMessage = true;
         }
         else if( MessageID == XN_SYS_INPUTACTIVITYCHANGED )
         {
            mNoControllerActivity = !Params;
         }
         else if( MessageID == XN_SYS_SIGNINCHANGED )
         {
            if( mCaresAboutChangeProfileEvents && mCanCareAboutChangeProfileEvents )
            {
               if( CheckIfProfileActuallyChanged() )
               {
                  // disable achievements just in case we are in a scene that automatically continues on
                  // to the part that awards the achievement
                  mDisableAchievements = true;
                  OSDialogRequest* pDialogRequest = new OSDialogRequest(kDRT_ProfileStatusChanged);
                  AddDialogRequest(pDialogRequest);
               }
            }
         }
      }
   }

   HandleAchievement();
}

void COsContext::StartUIRequest()
{
   sUIStopWatchOpenUI.Reset();
}

bool COsContext::DoneWithUIWait()
{
   if( !mIsShowingUI && sUIStopWatchOpenUI.GetElapsedMilliseconds() > 500 && sUIStopWatchCloseUI.GetElapsedMilliseconds() > 500 )
   {
      return true;
   }
   return false;
}

void COsContext::DeleteDialogRequest(OSDialogRequest* pDR)
{
   BPE_VERIFY( pDR->mStatus == kDRS_R__Done, false, "Can only delete a finished DialogRequest" );
}

//----------------------------------------------------------------------------

OSDialogRequest* COsContext::ShowDeviceUI(uint32 neededSize)
{
   if( mSignedInUser != kInvalidUser )
   {
      OSDialogRequest* pRet = new OSDialogRequest(kDRT_DeviceSelector);
      pRet->mExtraInformation = neededSize;
      AddDialogRequest(pRet);
      return pRet;
   }
   else
   {
      printf("Warning: ignoring ShowDeviceUI.  No signed in user.\n");
   }

   return NULL;
}

//----------------------------------------------------------------------------

OSDialogRequest* COsContext::ShowDeviceUnavailableUI()
{
   if( mSignedInUser != kInvalidUser )
   {
      OSDialogRequest* pRet = new OSDialogRequest(kDRT_StorageDeviceUnavailable);
      AddDialogRequest(pRet);
      return pRet;
   }
   else
   {
      printf("Warning: ignoring ShowDeviceUI.  No signed in user.\n");
   }

   return NULL;
}

//----------------------------------------------------------------------------

std::string CBaseOsContext::GetMachineName()
{
#if 0 //!GOLD_VERSION
  char computerName[256];
  uint32 bufferSize = sizeof(computerName);
  // Get the real xbox name.
  if (DmGetXboxName(computerName, &bufferSize) == XBDM_NOERR)
  {
     return std::string( computerName );
  }
#endif

  return std::string("Xbox360");
}

//----------------------------------------------------------------------------

std::string CBaseOsContext::GetCurrentUserName()
{
  int InIndex = OsContext()->mSignedInUser;
  if (XUserGetSigninState(InIndex) != eXUserSigninState_NotSignedIn)
  {
     XUID xuid;
     if(XUserGetXUID( InIndex, &xuid ) == ERROR_SUCCESS)
     {
        char userName[256];
        uint32 bufferSize = sizeof(userName);
        if (XUserGetName( InIndex, userName, bufferSize ) == ERROR_SUCCESS)
        {
           return std::string( userName );
        }
     }
  }

  return std::string("x360 user");
}

//----------------------------------------------------------------------------

void CBaseOsContext::TerminateProcess()
{
   // Force verify, we don't support 'forced' terminate process
   BPE_VERIFYA(false, "CBaseOsContext::TerminateProcess().");
}

//----------------------------------------------------------------------------

void CBaseOsContext::SetUnsafeToShutDownFlag( uint32 const flag )
{
   InterlockedOr((LONG*)&mUnsafeToShutDownFlags, flag);
}

//----------------------------------------------------------------------------

void CBaseOsContext::ClearUnsafeToShutDownFlag( uint32 const flag )
{
   InterlockedAnd((LONG*)&mUnsafeToShutDownFlags, ~flag);
}

//----------------------------------------------------------------------------

bool CBaseOsContext::IsSafeToShutDown() const
{
   return mUnsafeToShutDownFlags == 0;
}

//----------------------------------------------------------------------------

void Handle__kIPS_CheckingForSignIn(COsContext* pOsContext)
{
   // check to see if we already have a signed in profile for controller
   if( XUserGetSigninState( pOsContext->mSignedInUser ) != eXUserSigninState_NotSignedIn )
   {
      pOsContext->mInteralProfileStatus = COsContext::kIPS_GetProfileInformation;
      pOsContext->mLastKnownOfflineXUID_User0 = pOsContext->mUserSignInInfo_User0.xuid;
   }
   else
   {
      pOsContext->mInteralProfileStatus = COsContext::kIPS_DisplaySignInDialog;
   }
}

LPCWSTR SignInStatusChanged = L"Game profile sign in status change. You will be returned to the title screen.";

void Handle__kIPS_DisplaySignInDialog(COsContext* pOsContext)
{
   XShowSigninUI(1, 0);
   pOsContext->StartUIRequest();

   pOsContext->mInteralProfileStatus = COsContext::kIPS_DisplayingSignInDialog;
}

void Handle__kIPS_DisplayingSignInDialog(COsContext* pOsContext)
{
   pOsContext->mInteralProfileStatus = COsContext::kIPS_CheckingForSignIn;
   // check to see if we already have a signed in profile for controller
   if( XUserGetSigninState( pOsContext->mSignedInUser ) != eXUserSigninState_NotSignedIn )
   {
      pOsContext->mInteralProfileStatus = COsContext::kIPS_GetProfileInformation;
   }
   else
   {
      pOsContext->mProfileStatus = COsContext::kPS_Guest;
      pOsContext->mInteralProfileStatus = COsContext::kIPS_Done;
   }
}

DWORD gSettingIDs[] = { XPROFILE_GAMER_YAXIS_INVERSION };
void Handle__kIPS_GetProfileInformation(COsContext* pOsContext)
{
   // First time is just to get size of return value
   DWORD cbResults = 0;
   DWORD dwErr = XUserReadProfileSettings( 0, pOsContext->mSignedInUser, sizeof(gSettingIDs) / sizeof(gSettingIDs[0]), gSettingIDs, &cbResults, 0, 0 );
   BPE_VERIFY(dwErr == ERROR_INSUFFICIENT_BUFFER, false, "XUserReadProfileSettings should have returned ERROR_INSUFFICIENT_BUFFER!");
   // now we call the actual function with overlap
   if( pOsContext->mpProfileSettingsResult )
   {
      delete [] (BYTE*)pOsContext->mpProfileSettingsResult;
      pOsContext->mpProfileSettingsResult = NULL;
   }

   if( cbResults > 0 )
   {
      pOsContext->mpProfileSettingsResult = (XUSER_READ_PROFILE_SETTING_RESULT*)(new BYTE[cbResults]);

      ZeroMemory(&pOsContext->mInternalProfileStatusOverlapped, sizeof(XOVERLAPPED));
      dwErr = XUserReadProfileSettings( 0, pOsContext->mSignedInUser, sizeof(gSettingIDs) / sizeof(gSettingIDs[0]), gSettingIDs, &cbResults,
         pOsContext->mpProfileSettingsResult, // This argument now holds the allocated buffer.
         &pOsContext->mInternalProfileStatusOverlapped);

      pOsContext->mInteralProfileStatus = COsContext::kIPS_GettingProfileInformation;
   }
   else
   {
      pOsContext->mInteralProfileStatus = COsContext::kIPS_GetPostProfileInformation;
   }
}

void Handle__kIPS_GettingProfileInformation(COsContext* pOsContext)
{
   if( XHasOverlappedIoCompleted(&pOsContext->mInternalProfileStatusOverlapped) )
   {
      if( pOsContext->mpProfileSettingsResult->pSettings )
      {
         pOsContext->mInvertYAxis = pOsContext->mpProfileSettingsResult->pSettings->data.nData == XPROFILE_YAXIS_INVERSION_ON;
      }
      pOsContext->mInteralProfileStatus = COsContext::kIPS_GetPostProfileInformation;
   }
}

void Handle__kIPS_GetPostProfileInformation(COsContext* pOsContext)
{
   if( XUserGetSigninInfo( pOsContext->mSignedInUser, XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY, &pOsContext->mUserSignInInfo_User0 ) == ERROR_SUCCESS )
   {
      pOsContext->mLastKnownOfflineXUID_User0 = pOsContext->mUserSignInInfo_User0.xuid;
   }
   pOsContext->mProfileStatus = COsContext::kPS_Valid;
   pOsContext->mCanCareAboutChangeProfileEvents = true;

   pOsContext->mInteralProfileStatus = COsContext::kIPS_Done;
}

//----------------------------------------------------------------------------

CBaseOsContext::EProfileStatus CBaseOsContext::GetUserProfile()
{
   COsContext* pOsContext = (COsContext*)this;
   return pOsContext->mProfileStatus;
}

//----------------------------------------------------------------------------

void COsContext::GetUserProfile_()
{
   switch( mInteralProfileStatus )
   {
   case kIPS_CheckingForSignIn:
      Handle__kIPS_CheckingForSignIn(this);
      break;
   case kIPS_DisplayingSignInDialog:
      if( DoneWithUIWait() )
         Handle__kIPS_DisplayingSignInDialog(this);
      break;
   case kIPS_DisplaySignInDialog:
      if( DoneWithUIWait() )
         Handle__kIPS_DisplaySignInDialog(this);
      break;
   //
   case kIPS_GetProfileInformation:
      Handle__kIPS_GetProfileInformation(this);
      break;
   case kIPS_GettingProfileInformation:
      Handle__kIPS_GettingProfileInformation(this);
      break;
   case kIPS_GetPostProfileInformation:
      Handle__kIPS_GetPostProfileInformation(this);
      break;
   case kIPS_Done:
      break;
   default:
      BPE_VERIFYA(false, "Unhandled case: switch( mInteralProfileStatus )");
      break;
   }
}

//----------------------------------------------------------------------------

// AndyO: Temp includes for fake disc region check
#include "Engine/Resource/CResourceManager.h"
#include "CFileUtils.h"

void COsContext::InitializeBuildSKU()
{
   // Default NA
   mBuildSKU = kBS_USA;
   mSystemRegion = kR_USA;
   strcpy( mTitleId_SaveData, skTitleId_NA_SaveData );

   // AndyO:
   //
   // X360 title id's are compiled into executable.
   // As we don't want to create separate XEX builds for NA, EU and JP just yet,
   // we will check for the presence of a dummy file to determine the build SKU.
   //
   // THIS IS TEMPORARY CODE - NOT INTENDED FOR SHIP!
   //
   if (CFileUtils::DoesExist(CResourceManager::GetLocalPath("$/region_jp.bin")))
   {
      mBuildSKU = kBS_Japan;
      mSystemRegion = kR_Japan;
      strcpy( mTitleId_SaveData, skTitleId_JP_SaveData );
   }
   else if (CFileUtils::DoesExist(CResourceManager::GetLocalPath("$/region_eu.bin")))
   {
      mBuildSKU = kBS_Europe;
      mSystemRegion = kR_Europe;
      strcpy( mTitleId_SaveData, skTitleId_EU_SaveData );
   }

   bpe_debugger_printf("COsContext::InitializeBuildSKU - TitleId: %s - SaveId: %s - BuildSKU: %d - SystemRegion: %d\n", mTitleId, mTitleId_SaveData, mBuildSKU, mSystemRegion);
}

//----------------------------------------------------------------------------

void COsContext::InitializeLanguage()
{
   DWORD language = XGetLanguage();
   switch( language )
   {
   case XC_LANGUAGE_ENGLISH:
      mLanguage = kL_English;
      break;
   case XC_LANGUAGE_JAPANESE:
      mLanguage = kL_Japanese;
      break;
   case XC_LANGUAGE_GERMAN:
      mLanguage = kL_German;
      break;
   case XC_LANGUAGE_FRENCH:
      mLanguage = kL_French;
      break;
   case XC_LANGUAGE_SPANISH:
      mLanguage = kL_Spanish;
      break;
   case XC_LANGUAGE_ITALIAN:
      mLanguage = kL_Italian;
      break;
   default:
      mLanguage = kL_English;
      break;
   }
  
   ForceSpecificLanguageBasedOnBuildSKU();
}


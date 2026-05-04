#pragma once

#ifndef __TRANSFARRING_UI_STRINGS__H__
#define __TRANSFARRING_UI_STRINGS__H__

#include "TransfarringDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
   kTextEnglish,
   kTextJapanese,
   kTextFrench,
   kTextGerman,
   kTextItalian,
   kTextSpanish,
}
ETextLanguage;

typedef enum
{
   kTString_USE_OVERRIDE,
   
   kTString_Empty,
   kTString_Transfarring,
   kTString_Connection,

   kTString_WifiTransfarring,
   kTString_ConnectToPs3,
   kTString_ManageSaves,
   kTString_DisconnectFromPs3,
   kTString_ListenForVita,
   kTString_StopListeningForVita,
   kTString_Cancel,

   kTString_WifiTransfarring_jp_exp,
   kTString_ConnectToPs3_jp_exp,
   kTString_ManageSaves_jp_exp,
   kTString_DisconnectFromPs3_jp_exp,
   kTString_ListenForVita_jp_exp,
   kTString_StopListeningForVita_jp_exp,
   kTString_Cancel_jp_exp,

   kTString_MainGame,
   kTString_SnakeTales,
   kTString_VrMisions,
   kTString_PS3,
   kTString_VITA,
   kTString_Cloud,
   kTString_Yes,
   kTString_No,
   kTString_ConnectionLostMessage,
   kTString_TransfarringExplanation_Disconnected_PS3,
   kTString_TransfarringExplanation_Connected_PS3,
   kTString_TransfarringExplanation_Disconnected_VITA,
   kTString_TransfarringExplanation_Connected_VITA,
   kTString_TransfarringExplanation_Connecting_VITA,
   kTString_TransfarringExplanation_Listening_PS3,
   
   kTString_BrokenLinkLabel,
   kTString_BrokenLinkDate,
   kTString_BrokenLinkPlayTime,
   kTString_BrokenLinkDifficulty,
  
   kTString_Loading,
  
   kTString_Error_Processing,
   kTString_Error_Success,
   kTString_Error_FailedToSend, 
   kTString_Error_FailedToReceive, 
   kTString_Error_NotEnoughRemoteSaveSlots, 
   kTString_Error_NoCloudSlotsAvailable, 
   kTString_Error_NotEnoughLocalSaveSlots, 
   kTString_Error_Offline, 
   kTString_Error_SetSlotData_SizeTooLarge, 
   kTString_Error_GetSlotData_SlotNotFound, 
   kTString_Error_SignedInButDisconnected,
   kTString_Error_WrongFileType, 
   kTString_Error_WorkInProgress, 
   kTString_Error_Unknown,
   kTString_Error_FailedToConnectToPS3,
   kTString_Error_FailedToDisconnectFromPS3,
   kTString_Error_CorruptSaveData,
   kTString_Error_OutOfSpaceLocal,
   kTString_Error_OutOfSpaceRemote,
   kTString_Error_FailedToOpenAccessPoint,
   kTString_Error_WifiVersionMismatch,
   kTString_Error_Storage,
   kTString_Error_Ownership,
   
   kTString_ConflictSaveMessage,
   kTString_ConflictLoadMessage,
   
   kString_TrophiesDisabled,
   kString_TutorialText,
   kString_TutorialText_2,
   kString_TutorialText_3,
   kString_TutorialLabel,
   kString_SubHeader,
   kString_LockedFileMessage_Save,
   kString_LockedFileMessage_Load,
   kString_TutorialText_4,
   kString_CorruptFileLabel,
   kString_StartGameTrophyMessage,
   kTString_Cloud_NotConnected,

   kString_FileLocked,
   kString_FileLinked,
   kString_FileLockedAndLinked,
   kString_FileLockedVita,
   kString_FileLinkedVita,
   kString_FileLockedAndLinkedVita,
   kString_FileLockedOrphaned,
   kString_FileLinkedOrphaned,
   kString_FileCloud,
   kString_FileCloudLinked,
   kString_FileCloudOrphanedLocal,
   kString_FileCloudOrphanedCloud,
   kString_FileBroken,
   kString_FileBrokenCloud,
   kString_FileWithConflict,
   
   kTString_Button_TransfarrToVita,
   kTString_Prompt_TransfarrToVita,
   kTString_InProgress_TransfarrToVita,
   kTString_Success_TransfarrToVita,
   
   kTString_Button_TransfarrToVitaOrphaned,
   kTString_Prompt_TransfarrToVitaOrphaned,
   kTString_InProgress_TransfarrToVitaOrphaned,
   kTString_Success_TransfarrToVitaOrphaned,

   kTString_Button_TransfarrToVitaLinked,
   kTString_Prompt_TransfarrToVitaLinked,
   kTString_InProgress_TransfarrToVitaLinked,
   kTString_Success_TransfarrToVitaLinked,
   
   kTString_Button_TransfarrFromVita,
   kTString_Prompt_TransfarrFromVita,
   kTString_InProgress_TransfarrFromVita,
   kTString_Success_TransfarrFromVita,
   
   kTString_Button_TransfarrFromVitaOrphaned,
   kTString_Prompt_TransfarrFromVitaOrphaned,
   kTString_InProgress_TransfarrFromVitaOrphaned,
   kTString_Success_TransfarrFromVitaOrphaned,
   
   kTString_Button_TransfarrFromVitaLinked,
   kTString_Prompt_TransfarrFromVitaLinked,
   kTString_InProgress_TransfarrFromVitaLinked,
   kTString_Success_TransfarrFromVitaLinked,
   
   kTString_Button_Cloudify,
   kTString_Prompt_Cloudify,
   kTString_InProgress_Cloudify,
   kTString_Success_Cloudify,
   
   kTString_Button_Delete,
   kTString_Prompt_Delete,
   kTString_InProgress_Delete,
   kTString_Success_Delete,
   
   kTString_Button_UncloudifyLinked,
   kTString_Prompt_UncloudifyLinked,
   kTString_InProgress_UncloudifyLinked,
   kTString_Success_UncloudifyLinked,
   
   kTString_Button_UncloudifyOrphanedLocal,
   kTString_Prompt_UncloudifyOrphanedLocal,
   kTString_InProgress_UncloudifyOrphanedLocal,
   kTString_Success_UncloudifyOrphanedLocal,
   
   kTString_Button_UncloudifyOrphanedCloud,
   kTString_Prompt_UncloudifyOrphanedCloud,
   kTString_InProgress_UncloudifyOrphanedCloud,
   kTString_Success_UncloudifyOrphanedCloud,
   
   kTString_Button_Unlock,
   kTString_Prompt_Unlock,
   kTString_InProgress_Unlock,
   kTString_Success_Unlock,

   kTString_Button_Unlock_Vita,
   kTString_Prompt_Unlock_Vita,
   kTString_InProgress_Unlock_Vita,
   kTString_Success_Unlock_Vita,
   
   kTString_Button_Unlink,
   kTString_Prompt_Unlink,
   kTString_InProgress_Unlink,
   kTString_Success_Unlink,

   kTString_Button_Unlink_Vita,
   kTString_Prompt_Unlink_Vita,
   kTString_InProgress_Unlink_Vita,
   kTString_Success_Unlink_Vita,
   
   kTString_Button_SolveConflict,
   kTString_Prompt_SolveConflict,
   kTString_InProgress_SolveConflict,
   kTString_Success_SolveConflict,
   kTString_Choice_SolveConflict,
   kTString_Choice_SolveConflictLocal,
   kTString_Choice_SolveConflictRemote,
   
   kTString_PsnConnectionLost,
   kTString_PsnConnectionLostPS3,

   kTString_VitaWifiPrompt,

   kTString_ConnectionStatus_Vita,
   kTString_ConnectionStatus_PS3,

   kTString_MainMenu_Transfarring,
   kTString_MainMenu_GameSelect,

   kTString_TrophiesWillBeLockedPrompt,

   kTransfarringStringCount
}
ETransfarringStringId;

char const * GetTransfarringString( ETransfarringStringId const id );
char const * GetTransfarringStringForErrorCode( ETransfarringError const error );
ETransfarringStringId GetTransfarringStringIdForErrorCode( ETransfarringError const error );

ETextLanguage GetTextLanguage();

char const * GetLanguageDependentImageStringPostfix();

#ifdef __cplusplus
};
#endif
#endif
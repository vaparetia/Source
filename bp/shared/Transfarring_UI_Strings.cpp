#include "Engine/Stdafx.h"

#include "Transfarring_UI_Strings.h"
#include "TransfarringDefines.h"

#include <string>
#include <stdio.h>
#include "Engine/Basics/BPEAssert.h"
#include "Engine/System/COsContext.h"

#include "MGS_Common.h"
#include "BP_Misc.h"

char const * const gTransfarringUIStrings[] =
{
   "",
   //kTString_Empty
   "",
   //kTString_Transfarring
   "!TRANSFARRING",
   //kTString_Connection
   "!CONNECTION",
   
   //kTString_WifiTransfarring
   "!WIFITRANSFARRING",
   //kTString_ConnectToPs3
   "!CONNECT_TO_PS3",
   //kTString_ManageSaves
   "!MANAGESAVEFILES",
   //kTString_DisconnectFromPs3
   "!DISCONNECT_FROM_PS3",
   //kTString_ListenForVita
   "!LISTEN_FOR_VITA",
   //kTString_StopListeningForVita
   "!STOP_LISTENING_FOR_VITA",
   //kTString_Cancel
   "!CANCEL",

   //kTString_WifiTransfarring_jp_exp
   "!WIFITRANSFARRING_JP_EXP",
   //kTString_ConnectToPs3_jp_exp
   "!CONNECT_TO_PS3_JP_EXP",
   //kTString_ManageSaves_jp_exp
   "!MANAGESAVEFILES_JP_EXP",
   //kTString_DisconnectFromPs3_jp_exp
   "!DISCONNECT_FROM_PS3_JP_EXP",
   //kTString_ListenForVita_jp_exp
   "!LISTEN_FOR_VITA_JP_EXP",
   //kTString_StopListeningForVita_jp_exp
   "!STOP_LISTENING_FOR_VITA_JP_EXP",
   //kTString_Cancel_jp_exp
   "!CANCEL_JP_EXP",

   //kTString_MainGame
   "!MAIN_GAME",
   //kTString_SnakeTales
   "!SNAKE_TALES",
   //kTString_VrMisions
   "!VR_MISSIONS",
   //kTString_PS3
   "!PS3",
   //kTString_VITA
   "!VITA",
   //kTString_Cloud
   "!CLOUD",
   //kTString_Yes
   "!YES",
   //kTString_No
   "!NO",
   //kTString_ConnectionLostMessage
   "!CONNECTION_LOST",
   //kTString_TransfarringExplanation_Disconnected_PS3
   "!DISCONNECTED_EXPLANATION_PS3",
   //kTString_TransfarringExplanation_Connected_PS3
   "!CONNECTED_EXPLANATION_PS3",
   //kTString_TransfarringExplanation_Disconnected_VITA
   "!DISCONNECTED_EXPLANATION_VITA",
   //kTString_TransfarringExplanation_Connected_VITA
   "!CONNECTED_EXPLANATION_VITA",
   //kTString_TransfarringExplanation_Connecting_VITA
   "!CONNECTING_EXPLANATION_VITA",
   //kTString_TransfarringExplanation_Listening_PS3
   "!LISTENING_EXPLANATION_PS3",
   
   //kTString_BrokenLinkLabel
   "!BROKEN_LINK_LABEL",
   //kTString_BrokenLinkDate
   "!BROKEN_LINK_DATE",
   //kTString_BrokenLinkPlayTime
   "!BROKEN_LINK_PLAYTIME",
   //kTString_BrokenLinkDifficulty
   "!BROKEN_LINK_DIFFICULTY",
   
   //kTString_Loading
   "!LOADING",
   
   //kTString_Error_Processing
   "!ERROR_PROCESSING",
   //kTString_Error_Success
   "!ERROR_SUCCESS",
   //kTString_Error_FailedToSend
   "!ERROR_FAILEDTOSEND", 
   //kTString_Error_FailedToReceive
   "!ERROR_FAILEDTORECEIVE", 
   //kTString_Error_NotEnoughRemoteSaveSlots
   "!ERROR_NOTENOUGHREMOTESAVESLOTS", 
   //kTString_Error_NoCloudSlotsAvailable
   "!ERROR_NOCLOUDSLOTSAVAILABLE", 
   //kTString_Error_NotEnoughLocalSaveSlots
   "!ERROR_NOTENOUGHLOCALSAVESLOTS", 
   //kTString_Error_Offline
   "!ERROR_OFFLINE", 
   //kTString_Error_SetSlotData_SizeTooLarge
   "!ERROR_SETSLOTDATA_SIZETOOLARGE", 
   //kTString_Error_GetSlotData_SlotNotFound
   "!ERROR_GETSLOTDATA_SLOTNOTFOUND", 
   //kTString_Error_SignedInButDisconnected
   "!ERROR_SIGNEDINBUTDISCONNECTED",
   //kTString_Error_WrongFileType
   "!ERROR_WRONGFILETYPE", 
   //kTString_Error_WorkInProgress
   "!ERROR_WORKINPROGRESS",
   //kTString_Error_Unknown
   "!ERROR_UNKNOWN",
   //kTString_Error_FailedToConnectToPS3
   "!ERROR_FAILEDTOCONNECTTOPS3", 
   //kTString_Error_FailedToDisconnectFromPS3
   "!ERROR_FAILEDTODISCONNECTFROMPS3", 
   //kTString_Error_CorruptSaveData
   "!ERROR_CORRUPTSAVEDATA", 
   //kTString_Error_OutOfSpaceLocal
   "!ERROR_OUTOFSPACELOCAL", 
   //kTString_Error_OutOfSpaceRemote
   "!ERROR_OUTOFSPACEREMOTE",
   //kTString_Error_FailedToOpenAccessPoint
   "!ERROR_FAILEDTOOPENACCESSPOINT",
   //kTString_Error_WifiVersionMismatch
   "!ERROR_WIFIVERSIONMISMATCH",
   //kTString_Error_Storage
   "!ERROR_STORAGE_ERROR",
   //kTString_Error_Ownership
   "!ERROR_OWNERSHIP",

   //kTString_ConflictSaveMessage
   "!CONFLICT_SAVE_MESSAGE",
   //kTString_ConflictSaveMessage,
   "!CONFLICT_LOAD_MESSAGE",

   //kString_TrophiesDisabled
   "!TROPHIES_DISABLED",
   //kString_TutorialText
   "!TUTORIAL_TEXT",
   //kString_TutorialText_2
   "!TUTORIAL_TEXT_2",
   //kString_TutorialText_3
   "!TUTORIAL_TEXT_3",
   //kString_TutorialLabel
   "!TUTORIAL_LABEL",
   //kString_SubHeader
   "!SUBHEADER",

   //kString_LockedFileMessage_Save
   "!LOCKED_FILE_MESSAGE_SAVE",
   //kString_LockedFileMessage_Load
   "!LOCKED_FILE_MESSAGE_LOAD",
   //kString_TutorialText_4
   "!TUTORIAL_TEXT_4",
   //kString_CorruptFileLabel
   "!CORRUPT_FILE_LABEL",
   //kString_StartGameTrophyMessage
   "!START_GAME_TROPHY_MESSAGE",
   //kTString_Cloud_NotConnected
   "!CLOUD_NOTCONNECTED",

   //kString_FileLocked
   "!FILE_LOCKED",
   //kString_FileLinked
   "!FILE_LINKED",
   //kString_FileLockedAndLinked
   "!FILE_LOCKED_AND_LINKED",
   //kString_FileLockedVita
   "!FILE_LOCKED_VITA",
   //kString_FileLinkedVita
   "!FILE_LINKED_VITA",
   //kString_FileLockedAndLinkedVita
   "!FILE_LOCKED_AND_LINKED_VITA",
   //kString_FileLockedOrphaned
   "!FILE_LOCKED_ORPHANED",
   //kString_FileLinkedOrphaned
   "!FILE_LINKED_ORPHANED",
   //kString_FileCloud
   "!FILE_CLOUD",
   //kString_FileCloudLinked
   "!FILE_CLOUD_LINKED",
   //kString_FileCloudOrphanedLocal
   "!FILE_CLOUD_ORPHANED_LOCAL",
   //kString_FileCloudOrphanedCloud
   "!FILE_CLOUD_ORPHANED_CLOUD",
   //kString_FileBroken
   "!FILE_BROKEN",
   //kString_FileBrokenCloud
   "!FILE_BROKEN_CLOUD",
   //kString_FileWithConflict
   "!FILE_WITH_CONFLICT",

   /*kType_ToVita*/
   //kTString_Button_TransfarrToVita
   "!BUTTON_TRANSFAR_TO_VITA",
   //kTString_Prompt_TransfarrToVita
   "!PROMPT_TRANSFAR_TO_VITA",
   //kTString_InProgress_TransfarrToVita
   "!INPROGRESS_TRANSFAR_TO_VITA",
   //kTString_Success_TransfarrToVita
   "!SUCCESS_TRANSFAR_TO_VITA",

   /*kType_ToVitaOrphaned*/
   //kTString_Button_TransfarrToVitaOrphaned
   "!BUTTON_TRANSFAR_TO_VITA_ORPHANED",
   //kTString_Prompt_TransfarrToVitaOrphaned
   "!PROMPT_TRANSFAR_TO_VITA_ORPHANED",
   //kTString_InProgress_TransfarrToVitaOrphaned
   "!INPROGRESS_TRANSFAR_TO_VITA_ORPHANED",
   //kTString_Success_TransfarrToVitaOrphaned
   "!SUCCESS_TRANSFAR_TO_VITA_ORPHANED",

   /*kType_ToVitaLinked*/
   //kTString_Button_TransfarrToVitaLinked
   "!BUTTON_TRANSFAR_TO_VITA_LINKED",
   //kTString_Prompt_TransfarrToVitaLinked
   "!PROMPT_TRANSFAR_TO_VITA_LINKED",
   //kTString_InProgress_TransfarrToVitaLinked
   "!INPROGRESS_TRANSFAR_TO_VITA_LINKED",
   //kTString_Success_TransfarrToVitaLinked
   "!SUCCESS_TRANSFAR_TO_VITA_LINKED",

   /*kType_FromVita*/
   //kTString_Button_TransfarrFromVita
   "!BUTTON_TRANSFAR_FROM_VITA",
   //kTString_Prompt_TransfarrFromVita
   "!PROMPT_TRANSFAR_FROM_VITA",
   //kTString_InProgress_TransfarrFromVita
   "!INPROGRESS_TRANSFAR_FROM_VITA",
   //kTString_Success_TransfarrFromVita
   "!SUCCESS_TRANSFAR_FROM_VITA",

   /*kType_FromVitaOrphaned*/
   //kTString_Button_TransfarrFromVitaOrphaned
   "!BUTTON_TRANSFAR_FROM_VITA_ORPHANED",
   //kTString_Prompt_TransfarrFromVitaOrphaned
   "!PROMPT_TRANSFAR_FROM_VITA_ORPHANED",
   //kTString_InProgress_TransfarrFromVitaOrphaned
   "!INPROGRESS_TRANSFAR_FROM_VITA_ORPHANED",
   //kTString_Success_TransfarrFromVitaOrphaned
   "!SUCCESS_TRANSFAR_FROM_VITA_ORPHANED",

   /*kType_FromVitaLinked*/
   //kTString_Button_TransfarrFromVitaLinked
   "!BUTTON_TRANSFAR_FROM_VITA_LINKED",
   //kTString_Prompt_TransfarrFromVitaLinked
   "!PROMPT_TRANSFAR_FROM_VITA_LINKED",
   //kTString_InProgress_TransfarrFromVitaLinked
   "!INPROGRESS_TRANSFAR_FROM_VITA_LINKED",
   //kTString_Success_TransfarrFromVitaLinked
   "!SUCCESS_TRANSFAR_FROM_VITA_LINKED",

   /*kType_Cloudify*/
   //kTString_Button_Cloudify
   "!BUTTON_CLOUDIFY",
   //kTString_Prompt_Cloudify
   "!PROMPT_CLOUDIFY",
   //kTString_InProgress_Cloudify
   "!INPROGRESS_CLOUDIFY",
   //kTString_Success_Cloudify
   "!SUCCESS_CLOUDIFY",

   /*kType_Delete*/
   //kTString_Button_Delete
   "!BUTTON_DELETE",
   //kTString_Prompt_Delete
   "!PROMPT_DELETE",
   //kTString_InProgress_Delete
   "!INPROGRESS_DELETE",
   //kTString_Success_Delete
   "!SUCCESS_DELETE",

   /*kType_UncloudifyLinked*/
   //kTString_Button_UncloudifyLinked
   "!BUTTON_UNCLOUDIFY_LINKED",
   //kTString_Prompt_UncloudifyLinked
   "!PROMPT_UNCLOUDIFY_LINKED",
   //kTString_InProgress_UncloudifyLinked
   "!INPROGRESS_UNCLOUDIFY_LINKED",
   //kTString_Success_UncloudifyLinked
   "!SUCCESS_UNCLOUDIFY_LINKED",

   /*kType_UncloudifyOrphanedLocal*/
   //kTString_Button_UncloudifyOrphanedLocal
   "!BUTTON_UNCLOUDIFY_ORPHANED_LOCAL",
   //kTString_Prompt_UncloudifyOrphanedLocal
   "!PROMPT_UNCLOUDIFY_ORPHANED_LOCAL",
   //kTString_InProgress_UncloudifyOrphanedLocal
   "!INPROGRESS_UNCLOUDIFY_ORPHANED_LOCAL",
   //kTString_Success_UncloudifyOrphanedLocal
   "!SUCCESS_UNCLOUDIFY_ORPHANED_LOCAL",

   /*kType_UncloudifyOrphanedCloud*/
   //kTString_Button_UncloudifyOrphanedCloud
   "!BUTTON_UNCLOUDIFY_ORPHANED_CLOUD",
   //kTString_Prompt_UncloudifyOrphanedCloud
   "!PROMPT_UNCLOUDIFY_ORPHANED_CLOUD",
   //kTString_InProgress_UncloudifyOrphanedCloud
   "!INPROGRESS_UNCLOUDIFY_ORPHANED_CLOUD",
   //kTString_Success_UncloudifyOrphanedCloud
   "!SUCCESS_UNCLOUDIFY_ORPHANED_CLOUD",

   /*kType_Unlock*/
   //kTString_Button_Unlock
   "!BUTTON_UNLOCK",
   //kTString_Prompt_Unlock
   "!PROMPT_UNLOCK",
   //kTString_InProgress_Unlock
   "!INPROGRESS_UNLOCK",
   //kTString_Success_Unlock
   "!SUCCESS_UNLOCK",

   /*kType_Unlock_Vita*/
   //kTString_Button_Unlock_Vita
   "!BUTTON_UNLOCK_VITA",
   //kTString_Prompt_Unlock_Vita
   "!PROMPT_UNLOCK_VITA",
   //kTString_InProgress_Unlock_Vita
   "!INPROGRESS_UNLOCK_VITA",
   //kTString_Success_Unlock_Vita
   "!SUCCESS_UNLOCK_VITA",

   /*kType_Unlink*/
   //kTString_Button_Unlink
   "!BUTTON_UNLINK",
   //kTString_Prompt_Unlink
   "!PROMPT_UNLINK",
   //kTString_InProgress_Unlink
   "!INPROGRESS_UNLINK",
   //kTString_Success_Unlink
   "!SUCCESS_UNLINK",

   /*kType_Unlink_Vita*/
   //kTString_Button_Unlink_Vita
   "!BUTTON_UNLINK_VITA",
   //kTString_Prompt_Unlink_Vita
   "!PROMPT_UNLINK_VITA",
   //kTString_InProgress_Unlink_Vita
   "!INPROGRESS_UNLINK_VITA",
   //kTString_Success_Unlink_Vita
   "!SUCCESS_UNLINK_VITA",

   /*kType_SolveConflict*/
   //kTString_Button_SolveConflict
   "!BUTTON_SOLVE_CONFLICT",
   //kTString_Prompt_SolveConflict
   "!PROMPT_SOLVE_CONFLICT",
   //kTString_InProgress_SolveConflict
   "!INPROGRESS_SOLVE_CONFLICT",
   //kTString_Success_SolveConflict
   "!SUCCESS_SOLVE_CONFLICT",
   //kTString_Choice_SolveConflict
   "!CHOICE_SOLVE_CONFLICT",
   //kTString_Choice_SolveConflictLocal
   "!CHOICE_SOLVE_CONFLICT_LOCAL",
   //kTString_Choice_SolveConflictRemote
   "!CHOICE_SOLVE_CONFLICT_REMOTE",

   //kTString_PsnConnectionLost
   "!PSN_LOST_CONNECTION",
   //kTString_PsnConnectionLostPS3
   "!PSN_LOST_CONNECTION_PS3",

   //kTString_VitaWifiPrompt
   "!GOTO_PS3_TO_CONTINUE",

   //kTString_ConnectionStatus_Vita
   "!CONNECTION_STATUS_VITA",
   //kTString_ConnectionStatus_PS3
   "!CONNECTION_STATUS_PS3",

   //kTString_MainMenu_Transfarring
   "!MAINMENU_TRANSFARRING",
   //kTString_MainMenu_GameSelect
   "!MAINMENU_GAMESELECT",

   //kTString_TrophiesWillBeLockedPrompt
   "!TROPHIES_WILL_BE_LOCKED_PROMPT",
};

BPE_CTASSERT( kTransfarringStringCount == BPE_ARRAY_SIZE( gTransfarringUIStrings ) );
BPE_CTASSERT( ( kTString_Error_Ownership - kTString_Error_Processing + 1 ) == kTE_MaxNumErrors );

char const * GetTransfarringString(ETransfarringStringId const id)
{
   BPE_VERIFY( id > 0 && id < kTransfarringStringCount, false, "String id out of range." );
   BPE_VERIFY( gTransfarringUIStrings[id], false, "String missing in table." );

   char const *transfarringString = gTransfarringUIStrings[ id ];

   if ( transfarringString[0] == '!' )
   {
      // If the transfarring string starts with a bang, then this is to be used as a string lookup

      return BP_GetCustomOverrideString( "TRANSFARRING", transfarringString + 1 );
   }
   else
   {
      return transfarringString;
   }
}

ETransfarringStringId GetTransfarringStringIdForErrorCode( ETransfarringError const error )
{
   if( error >= kTE_Processing && error < kTE_MaxNumErrors)
   {
      return (ETransfarringStringId)( error + kTString_Error_Processing );
   }
   else
   {
      return kTString_Error_Unknown;
   }
}

char const * GetTransfarringStringForErrorCode( ETransfarringError const error )
{
   return GetTransfarringString( GetTransfarringStringIdForErrorCode( error ) );
}

ETextLanguage GetTextLanguage()
{
   switch ( gpOsContext->mLanguage )
   {
   case COsContext::kL_Italian:
      return kTextItalian;
   case COsContext::kL_Spanish:
      return kTextSpanish;
   case COsContext::kL_German:
      return kTextGerman;
   case COsContext::kL_French:
      return kTextFrench;
   default:
      return kTextEnglish;
   }
}

char const * GetLanguageDependentImageStringPostfix()
{
   switch ( gpOsContext->mLanguage )
   {
   case COsContext::kL_Italian:
      return "_it";
   case COsContext::kL_Spanish:
      return "_sp";
   case COsContext::kL_German:
      return "_gr";
   case COsContext::kL_French:
      return "_fr";
   default:
      return "";
   }
}
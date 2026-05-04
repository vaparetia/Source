//----------------------------------------------------------------------------
#include "stdafx.h"

#include "transfarring_menu.h"

#if MGS_VERSION == 2
#include "dmapack.h"
#include "../titlescr/titlescr.h"
#include "../mcman/mcman.h"
#include "../mcman/mcman_shared.h"
#endif

#if MGS_VERSION == 3
#include "common.h"
#include "../arai.h"
#include "../ut/a_pad.h"
#include "../../source/game/savedata.h"
#include "../../source/user/arai/mc/mc.h"
#include "../mc/mc_shared.h"
#endif

#include "transfarring_menu_data.h"
#include "transfarring_menu_rendering.h"

#ifdef BP_PS3
#include "Transfarring_PS3CGlue.h"
#else
#include "TransfarringVTACGlue.h"
#endif

#include "BP_Network.h"
#include "BP_TUS.h"

#include "BP_CommonDialog.h"

#include "libfs.h"

#ifdef BP_PS3
   #define TRANSFARRING_WARN_LEGACY 1
#endif

static int sFakeTick = 0;

typedef enum
{
   kHideIcons,
   kUseIcons,
}
EUseIcons;

typedef enum
{
   kType_None,
   kType_ToVita,
   kType_ToVitaOrphaned,
   kType_ToVitaLinked,
   kType_FromVita,
   kType_FromVitaOrphaned,
   kType_FromVitaLinked,
   kType_Cloudify,
   kType_Delete,
   kType_UncloudifyLinked,
   kType_UncloudifyOrphanedLocal,
   kType_UncloudifyOrphanedCloud,
   kType_Unlock,
   kType_Unlink,
   kType_Unlock_Vita,
   kType_Unlink_Vita,
   kType_SolveConflict,
}
ETransfarringType;

typedef enum
{
   kLinkedState_None,
   kLinkedState_Linked,
   kLinkedState_Broken,
}
ELinkedState;

typedef enum
{
   kPlayAction_No,
   kPlayAction_Yes,
}
EPlayAnimationAction;

typedef enum
{
   kSaveLocal,
   kSaveRemote,
}
EPlatformSaveType;

typedef enum
{
   kMainMenu_Button_0,
   kMainMenu_Button_1,
   kMainMenu_Button_2,
}
EMainMenuOptions;

typedef enum
{
   kMainMenu_Connected,
   kMainMenu_Disconnected,
}
EMainMenuStatus;

typedef enum
{
   kSaveTable_None = 0,
   kSaveTable_Wifi,
   kSaveTable_Cloud,
}
ESaveTableTypes;

typedef enum
{
   kPageDefault = 0,
   kPage_1,
   kPage_2,
   kPage_3,
   kNumPages,
}
EExplanationPage;

typedef enum
{
   kLoadingFilesStep,
   kSynchingFilesStep,
   kDoneStep,
}
EManageSavesLoadingStep;

typedef enum
{
   kCheckTrophiesBegin,
   kCheckTrophiesLoading,
   kCheckTrophiesPrompt,
   kCheckTrophiesTransfarr,
   kCheckTrophiesDone,
}
ECheckTrophiesDisabledStep;

#define SAVE_FILE_ID_ARRAY_LENGTH  4
#define SAVE_FILE_PLAYTIME_ARRAY_LENGTH  11
#define SAVE_FILE_DATE_ARRAY_LENGTH 11
#define SAVE_FILE_CLEAR_COUNT_ARRAY_LENGTH 6
#define SAVE_FILE_CHAR_BUFFER_LENGTH 64

typedef struct
{
   char mIdStr[SAVE_FILE_ID_ARRAY_LENGTH];
   char mPlayTimeStr[SAVE_FILE_PLAYTIME_ARRAY_LENGTH];
   char mDateStr[SAVE_FILE_DATE_ARRAY_LENGTH];
   char mClearCountStr[SAVE_FILE_CLEAR_COUNT_ARRAY_LENGTH];
   
   long long mTransfarringID[2];

   unsigned int mSaveIndex;
   unsigned int mStageIndex;
   unsigned int mDifficulty;
   unsigned int mClearCount;
   int mIsCorrupt;
}
TransfarringSaveUiData;

extern int BP_FRAMES_PER_SEC(void);

static int TransfarringIdCompare(TransfarringSaveUiData *a, TransfarringSaveUiData *b)
{
   unsigned long long const numberA_0 = a->mTransfarringID[0];
   unsigned long long const numberA_1 = a->mTransfarringID[1];

   unsigned long long const numberB_0 = b->mTransfarringID[0];
   unsigned long long const numberB_1 = b->mTransfarringID[1];

   int const halfComp = numberA_0 < numberB_0 ? -1 : (numberA_0 > numberB_0);

   return halfComp ? halfComp : (numberA_1 < numberB_1 ? -1 : (numberA_1 > numberB_1));
}

static int TransfarringSaveFileSortComparator(const void* a, const void* b)
{
   return -TransfarringIdCompare(*(TransfarringSaveUiData**)a , *(TransfarringSaveUiData**)b);
}

#if MGS_VERSION == 2
extern int BP_Pad_GetOkAssignment();
extern int BP_Pad_GetCancelAssignment();

// Map MGS3 pad funtion names
#define A_Pad_Cancel(padwork) (*(padwork) & BP_Pad_GetCancelAssignment())
#define A_Pad_Down(padwork) (*(padwork) & PAD_D)
#define A_Pad_Up(padwork) (*(padwork) & PAD_U)
#define A_Pad_OK(padwork) (*(padwork) & BP_Pad_GetOkAssignment())
#define A_Pad_Left(padwork) (*(padwork) & PAD_L)
#define A_Pad_Right(padwork) (*(padwork) & PAD_R)
#define A_Pad_R1(padwork) (*(padwork) & PAD_R1)
#define A_Pad_L1(padwork) (*(padwork) & PAD_L1)
#define A_Pad_Sq(padwork) (*(padwork) & PAD_Y)
#define A_Pad_Tri(padwork) (*(padwork) & PAD_X)

#define MAX_NUMBER_OF_SAVE_SLOTS DATA_FILE_MAX
#define PlayMenuSelectionSound() GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01)
#define PlayMenuOkSound() GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01)
#define PlayMenuCancelSound() GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_V_CANS02)
#endif // MGS_VERSION == 2

#if MGS_VERSION == 3
#define MAX_NUMBER_OF_SAVE_SLOTS MC_GAME_LIST_MAX
#define PlayMenuSelectionSound()  A_SdSet(SD_S_NORM_SELECT_1)
#define PlayMenuOkSound() A_SdSet(SD_S_NORM_ENTER_1)
#define PlayMenuCancelSound() A_SdSet(SD_S_NORM_CANSEL_1)

#define A_Pad_Sq(p) A_Pad_Press(p, A_PAD_ZOOMOUT)
#endif

#define SAVE_SLOT_QUANT_PER_COLUMN 6
#define PRIMITIVES_PER_SAVE_SLOT 12
#define PRIMITIVES_PER_LINKED_SLOT 5
#define MAX_MENU_STATES 10
#define SAVE_SLOT_DATA_ARRAY_SIZE (MAX_NUMBER_OF_SAVE_SLOTS * 2)

#define MODAL_MESSAGE_DURATION_IN_UPDATE_TICKS 240

typedef struct  
{
   int x;
   int y;
}
SCursor;

struct STransfarringWork;

typedef void (*StateRunFunction)(struct STransfarringWork *work);
typedef void (*StatePushedFunction)(struct STransfarringWork *work);
typedef void (*StatePoppedFunction)(struct STransfarringWork *work);

typedef struct
{
   StateRunFunction mRun;
   StatePushedFunction mPushed;
   StatePoppedFunction mPopped;
}
STransfarringMenuState;

//Forward declaration of state functions
static void EnteringStateRun(struct STransfarringWork *work);
static void EnteringStatePushed(struct STransfarringWork *work);
static void MainMenuDisconnectedPS3StateRun(struct STransfarringWork *work);
static void MainMenuDisconnectedPS3StatePushed(struct STransfarringWork *work);
static void MainMenuDisconnectedPS3StatePopped(struct STransfarringWork *work);
static void MainMenuListeningPS3StateRun(struct STransfarringWork *work);
static void MainMenuListeningPS3StatePushed(struct STransfarringWork *work);
static void MainMenuListeningPS3StatePopped(struct STransfarringWork *work);
static void MainMenuConnectedPS3StateRun(struct STransfarringWork *work);
static void MainMenuConnectedPS3StatePushed(struct STransfarringWork *work);
static void MainMenuConnectedPS3StatePopped(struct STransfarringWork *work);
static void MainMenuDisconnectedVITAStateRun(struct STransfarringWork *work);
static void MainMenuDisconnectedVITAStatePushed(struct STransfarringWork *work);
static void MainMenuDisconnectedVITAStatePopped(struct STransfarringWork *work);
static void MainMenuConnectedVITAStateRun(struct STransfarringWork *work);
static void MainMenuConnectedVITAStatePushed(struct STransfarringWork *work);
static void MainMenuConnectedVITAStatePopped(struct STransfarringWork *work);
static void LeavingStateRun(struct STransfarringWork *work);
static void LeavingStatePushed(struct STransfarringWork *work);
static void WifiTransfarStateRun(struct STransfarringWork *work);
static void WifiTransfarStatePushed(struct STransfarringWork *work);
static void WifiTransfarStatePopped(struct STransfarringWork *work);
static void WifiTransfarLoadingStateRun(struct STransfarringWork *work);
static void WifiTransfarLoadingStatePushed(struct STransfarringWork *work);
static void WifiTransfarLoadingStatePopped(struct STransfarringWork *work);
static void WifiTransfarYesNoStateRun(struct STransfarringWork *work);
static void WifiTransfarTransfarringStateRun(struct STransfarringWork *work);
static void WifiTransfarTransfarringStatePushed(struct STransfarringWork *work);
static void WifiTransfarTransfarringStatePopped(struct STransfarringWork *work);
static void ModalMessageStateRun(struct STransfarringWork *work);
static void ModalMessageStatePushed(struct STransfarringWork *work);
static void ModalMessageStatePopped(struct STransfarringWork *work);
static void ManageSavesStateRun(struct STransfarringWork *work);
static void ManageSavesStatePushed(struct STransfarringWork *work);
static void ManageSavesStatePopped(struct STransfarringWork *work);
static void ManageSavesLoadingStateRun(struct STransfarringWork *work);
static void ManageSavesLoadingStatePushed(struct STransfarringWork *work);
static void ManageSavesLoadingStatePopped(struct STransfarringWork *work);
static void ManageSavesYesNoStateRun(struct STransfarringWork *work);
static void ManageSavesTransfarringStateRun(struct STransfarringWork *work);
static void ManageSavesTransfarringStatePushed(struct STransfarringWork *work);
static void ManageSavesTransfarringStatePopped(struct STransfarringWork *work);
static void ManageSavesSolveConflictStateRun(struct STransfarringWork *work);
static void ManageSavesSolveConflictStatePushed(struct STransfarringWork *work);
static void ManageSavesSolveConflictStatePopped(struct STransfarringWork *work);
static void SystemModalYesNoStateRun(struct STransfarringWork *work);
static void CheckTrophiesDisabledStateRun(struct STransfarringWork *work);
static void CheckTrophiesDisabledStatePushed(struct STransfarringWork *work);
static void CheckTrophiesDisabledStatePopped(struct STransfarringWork *work);

static void SharedYesNoStatePushed(struct STransfarringWork *work);
static void SharedYesNoStatePopped(struct STransfarringWork *work);

static STransfarringMenuState kTransfarringMenuStates[kTransfarStateCount] =
{
   //kTransfarState_Entering
   { EnteringStateRun, EnteringStatePushed, 0 },
   //kTransfarState_MainMenu_Disconnected_PS3
   { MainMenuDisconnectedPS3StateRun, MainMenuDisconnectedPS3StatePushed, MainMenuDisconnectedPS3StatePopped },
   //kTransfarState_MainMenu_Listening_PS3
   { MainMenuListeningPS3StateRun, MainMenuListeningPS3StatePushed, MainMenuListeningPS3StatePopped },
   //kTransfarState_MainMenu_Connected_PS3
   { MainMenuConnectedPS3StateRun, MainMenuConnectedPS3StatePushed, MainMenuConnectedPS3StatePopped },
   //kTransfarState_MainMenu_Disconnected_VITA
   { MainMenuDisconnectedVITAStateRun, MainMenuDisconnectedVITAStatePushed, MainMenuDisconnectedVITAStatePopped },
   //kTransfarState_MainMenu_Connected_VITA
   { MainMenuConnectedVITAStateRun, MainMenuConnectedVITAStatePushed, MainMenuConnectedVITAStatePopped },
   //kTransfarState_Leaving
   { LeavingStateRun, LeavingStatePushed, 0 },
   //kTransfarState_WifiTransfar
   { WifiTransfarStateRun, WifiTransfarStatePushed, WifiTransfarStatePopped },
   //kTransfarState_WifiTransfarLoading
   { WifiTransfarLoadingStateRun, WifiTransfarLoadingStatePushed, WifiTransfarLoadingStatePopped},
   //kTransfarState_WifiTransfar_YesNo
   { WifiTransfarYesNoStateRun, SharedYesNoStatePushed, SharedYesNoStatePopped },
   //kTransfarState_WifiTransfar_Transfarring
   { WifiTransfarTransfarringStateRun, WifiTransfarTransfarringStatePushed, WifiTransfarTransfarringStatePopped },
   //kTransfarState_ModalMessage
   { ModalMessageStateRun, ModalMessageStatePushed, ModalMessageStatePopped},
   //kTransfarState_ManageSaves
   { ManageSavesStateRun, ManageSavesStatePushed, ManageSavesStatePopped},
   //kTransfarState_ManageSavesLoading
   { ManageSavesLoadingStateRun, ManageSavesLoadingStatePushed, ManageSavesLoadingStatePopped},
   //kTransfarState_ManageSaves_YesNo
   { ManageSavesYesNoStateRun, SharedYesNoStatePushed, SharedYesNoStatePopped },
   //kTransfarState_ManageSaves_Transfarring
   { ManageSavesTransfarringStateRun, ManageSavesTransfarringStatePushed, ManageSavesTransfarringStatePopped },
   //kTransfarState_ManageSaves_SolveConflict
   { ManageSavesSolveConflictStateRun, ManageSavesSolveConflictStatePushed, ManageSavesSolveConflictStatePopped },
   //kTransfarState_SystemModal_YesNo
   { SystemModalYesNoStateRun, 0, 0 },
   //kTransfarState_CheckTrophiesDisabled
   { CheckTrophiesDisabledStateRun, CheckTrophiesDisabledStatePushed, CheckTrophiesDisabledStatePopped },
};

typedef struct STransfarringWork
{
#if MGS_VERSION == 2
   GV_ACT_EX actor;

   int parent_name;
   int pad_work;
   int ans;
#endif
#if MGS_VERSION == 3
   GV_ACT actor;

   A_PAD_WORK pad_work;
#endif
   int mObjCount;

   //General use cursor
   SCursor mGeneralCursor;

   //Morph handling
   SMenuAction *pCurrentAction;
   int mActionTime;

   //State stack
   int mMenuStateStackIndex;
   STransfarringMenuState *mMenuStateStack[MAX_MENU_STATES];

   //Rendering data
   struct STransfarringGlobalRenderingData *mRenderingData;

   //Modal state handling
   int mModalStateStartTick;
   ETransfarringStringId mModalMessageId;
   char const *mModalMessageOverride;

   //Used to keep track of the last transfarring operation
   ETransfarringType mTransfarringType;
   ETransfarringType mTypeButtonX;
   ETransfarringType mTypeButtonSq;

   //Save table variables
   int mWasConnectedToPsn;
   ESaveTableTypes mSaveTableType;
   ESaveType mCurrentSaveType;

   //Save game data
   int mSavesBufferIndex;
   TransfarringSaveUiData mSavesBuffer[MAX_NUMBER_OF_SAVE_SLOTS * 2];
   TransfarringSaveUiData *mSaves_PS3[SAVE_SLOT_DATA_ARRAY_SIZE];
   TransfarringSaveUiData *mSaves_VITA[SAVE_SLOT_DATA_ARRAY_SIZE];
   //Save table cursor handling
   SCursor mSaveTableCursor;
   int mSaveTableBaseIndex;

   //Loading Bar
   int mLoadingBarFadingIn;

   int mIsConnecting;

   EExplanationPage mExplanationPage;
   ETransfarringStringId mDefaultExplanationId;

   EManageSavesLoadingStep mManageSavesLoadingStep;

   int mAccessPointOpened;

   int mExitCurrentState;

   //kTransfarState_SystemModal_YesNo
   unsigned long long mSystemModalHandle;
   int mSystemModalYesNoReply;

   //kTransfarState_CheckTrophiesDisabled
   ECheckTrophiesDisabledStep mCheckTrophiesDisabledStep;

#if defined(BP_VITA)
   int mSystemResumeCount;
#endif
}
TransfarringWork;

#if MGS_VERSION == 3
// global that is monitored by the main menu
int gTransfarringMenuDone = 0;
#endif

static void UpdatePad(TransfarringWork* work)
{
#if MGS_VERSION == 2
   int press=GV_PadDataDirect[0].press & (PAD_U|PAD_D|PAD_L|PAD_R|PAD_L1|PAD_R1|PAD_OK|PAD_CANCEL|PAD_Y|PAD_X);
   work->pad_work=press;
#endif
#if MGS_VERSION == 3
   A_Pad_Act(&work->pad_work);
#endif
}

static unsigned int morph_color(unsigned int p, unsigned int q, float t)
{
   int pr = p&0xff;
   int pg = (p>>8)&0xff;
   int pb = (p>>16)&0xff;
   int pa = p>>24;

   int qr = q&0xff;
   int qg = (q>>8)&0xff;
   int qb = (q>>16)&0xff;
   int qa = q>>24;

   int fr = (int)((1.0f - t)*pr + t*qr + 0.5f);
   int fg = (int)((1.0f - t)*pg + t*qg + 0.5f);
   int fb = (int)((1.0f - t)*pb + t*qb + 0.5f);
   int fa = (int)((1.0f - t)*pa + t*qa + 0.5f);

   return (fa<<24)|(fb<<16)|(fg<<8)|fr;
}

static void UpdateScene(TransfarringWork *work)
{
   SMenuAction *action = work->pCurrentAction;
   int actionOver = TRUE;
   int objIndex, actionItemIndex;

   //if ((DG_TickCount % 30) == 0)
   if ((sFakeTick % 30) == 0)
   {
      for (objIndex = 0; objIndex < work->mObjCount; ++objIndex)
      {
         SMenuObject *obj = &gTransfarringMenuObjects[objIndex];
         if (obj->type == kAnim && (obj->curStatus.col & 0xff000000) > 0)
         {
            ++obj->currentFrame;
            if (obj->currentFrame >= MAX_ANIM_FRAMES || obj->tex[obj->currentFrame] == NULL)
            {
               obj->currentFrame = 0;
            }
         }
      }
   }

   if (action != NULL)
   {
      for (objIndex = 0; objIndex < MAX_ACTION_OBJECTS; ++objIndex)
      {
         SMenuObjectAction *objAct = &action->objActions[objIndex];
         if (objAct->object == MENU_OBJECT_NONE)
         {
            break;
         }
         {
            SMenuObject *obj = &gTransfarringMenuObjects[objAct->object];
            int itemStartTime = 0;
            for (actionItemIndex = 0; actionItemIndex < MAX_ACTION_ITEMS; ++actionItemIndex)
            {
               SMenuActionItem *actItem = &objAct->items[actionItemIndex];
               int itemEndTime = itemStartTime + actItem->duration;
               if (actItem->type == kEnd)
               {
                  break;
               }
               if (work->mActionTime >= itemStartTime && work->mActionTime <= itemEndTime)
               {
                  actionOver = FALSE;
                  if (actItem->type == kSet)
                  {
                     obj->status = actItem->status;
                     obj->curStatus = actItem->status;
                  }
                  else if (work->mActionTime == itemEndTime)
                  {
                     obj->status = actItem->status;
                     obj->curStatus = actItem->status;
                  }
                  else
                  {
                     // morph
                     float t = ((float)(work->mActionTime - itemStartTime))/((float)(actItem->duration));
                     obj->curStatus.x = (int)((1.0f - t)*obj->status.x + t*actItem->status.x + 0.5f);
                     obj->curStatus.y = (int)((1.0f - t)*obj->status.y + t*actItem->status.y + 0.5f);
                     obj->curStatus.w = (int)((1.0f - t)*obj->status.w + t*actItem->status.w + 0.5f);
                     obj->curStatus.h = (int)((1.0f - t)*obj->status.h + t*actItem->status.h + 0.5f);
                     obj->curStatus.col = morph_color(obj->status.col, actItem->status.col, t);
                  }
               }
               itemStartTime += actItem->duration;
            }
         }
      }
      ++work->mActionTime;
   }
   if (actionOver)
   {
      work->pCurrentAction = NULL;
   }

   sFakeTick++;
}

static void HideMenuObject(SMenuObject *obj)
{
   ASSERT(obj);
   obj->status.col = skInvisible;
   obj->curStatus.col = skInvisible;
}

static void DisplayMenuObject(SMenuObject *obj)
{
   ASSERT(obj);
   obj->status.col = skWhite;
   obj->curStatus.col = skWhite;
}

static void DisplayMenuObjectWithColor(SMenuObject *obj, unsigned color)
{
   ASSERT(obj);
   obj->status.col = color;
   obj->curStatus.col = color;
}

static void SetMenuObjectText(SMenuObject *obj, ETransfarringStringId stringId, char const *stringOverride)
{
   if(obj)
   {
      obj->textId = stringId;
      obj->textOverride = stringOverride;

      if(obj->type == kBpText)
      {
         Transfarring_FlagBpTextChanged(obj);
      }
   }
}

static void DisplaySystemDialogErrorMessage(STransfarringError error)
{
   if(error.mLowLevelError)
   {
      BP_CommonDialog_WantsErrorDialog(error.mLowLevelError);
   }
   else
   {
      BP_CommonDialog_WantsMessageDialog(GetTransfarringStringForErrorCode(error.mHighLevelError), kMDL_OK);
   }
}

static int GetNumLocalSaveFiles(TransfarringWork *work)
{
   if(Transfarring_IsLocalFileListValid())
   {
      return Transfarring_GetNumLocalSaveFiles(work->mCurrentSaveType);
   }
   return 0;
}

static TransfarringSaveFile* GetLocalSaveFile(TransfarringWork *work, int index)
{
   if(work->mSaveTableType == kSaveTable_Cloud)
   {
      return (TransfarringSaveFile*)Transfarring_GetLocalSaveFile(work->mCurrentSaveType, index);
   }
#ifdef BP_PS3
   else if(work->mSaveTableType == kSaveTable_Wifi)
   {
      TransfarringSaveFile *const save = (TransfarringSaveFile*)Transfarring_GetLocalSaveFile(work->mCurrentSaveType, index);
      if(save && !Transfarring_IsFileCloud(work->mCurrentSaveType, save->mSaveIndex))
      {
         return save;
      }
   }
#endif
   return 0;
}

static int GetNumRemoteSaveFiles(TransfarringWork *work)
{
   if(work->mSaveTableType == kSaveTable_Cloud && BP_Network_IsSignedIn()
      && Transfarring_IsCloudFileListValid())
   {
      return Transfarring_GetNumCloudSaveFiles(work->mCurrentSaveType);
   }
#ifdef BP_PS3
   else if(work->mSaveTableType == kSaveTable_Wifi && Transfarring_IsConnectedToVita())
   {
      return Transfarring_GetNumRemoteSaveFiles(work->mCurrentSaveType);
   }
#endif
   return 0;
}

static TransfarringSaveFile* GetRemoteSaveFile(TransfarringWork *work, int index)
{
   if(work->mSaveTableType == kSaveTable_Cloud)
   {
      return (TransfarringSaveFile*)Transfarring_GetCloudSaveFile(work->mCurrentSaveType, index);
   }
#ifdef BP_PS3
   else if(work->mSaveTableType == kSaveTable_Wifi)
   {
      TransfarringSaveFile *const save = (TransfarringSaveFile*)Transfarring_GetRemoteSaveFile(work->mCurrentSaveType, index);
      if(save && !Transfarring_IsRemoteFileCloud(save))
      {
         return save;
      }
   }
#endif
   return 0;
}

static int Transfarring_IsFileDataValid(TransfarringWork *work)
{
   if(work->mSaveTableType == kSaveTable_Cloud)
   {
      return Transfarring_IsLocalFileListValid() && (!BP_Network_IsSignedIn() || Transfarring_IsCloudFileListValid());
   }
#ifdef BP_PS3
   return Transfarring_IsConnectedToVita() && Transfarring_IsLocalFileListValid();
#else
   return Transfarring_IsConnectedToPS3() && Transfarring_IsLocalFileListValid();
#endif
}

static int IsMenuStateStackEmpty(TransfarringWork *work)
{
   return work->mMenuStateStackIndex < 0;
}

static STransfarringMenuState* GetCurrentMenuState(TransfarringWork *work)
{
   return IsMenuStateStackEmpty(work) ? 0 : work->mMenuStateStack[work->mMenuStateStackIndex];
}

static void PushMenuState(TransfarringWork *work, ETransfarState stateIndex)
{
   ASSERT(work->mMenuStateStackIndex < MAX_MENU_STATES - 1);
   if(work->mMenuStateStackIndex < MAX_MENU_STATES - 1)
   {
      STransfarringMenuState *state = &kTransfarringMenuStates[stateIndex];
      work->mMenuStateStack[++work->mMenuStateStackIndex] = state;
      if(state->mPushed)
      {
         state->mPushed(work);
      }
   }
}

static void PopMenuState(TransfarringWork *work)
{
   STransfarringMenuState *state = GetCurrentMenuState(work);
   if(state)
   {
      work->mMenuStateStack[work->mMenuStateStackIndex--] = 0;
      if(state->mPopped)
      {
         state->mPopped(work);
      }
   }
}

static void UpdateMenuState(TransfarringWork *work)
{
   STransfarringMenuState *state = GetCurrentMenuState(work);
   if(state && state->mRun)
   {
      state->mRun(work);
   }
}

static void PlayAction(TransfarringWork *work, EMenuAction action)
{
   int i;
   // set the morph start state
   for (i = 0; i < work->mObjCount; ++i)
   {
      SMenuObject *obj = &gTransfarringMenuObjects[i];
      obj->status = obj->curStatus;
   }
   work->pCurrentAction = &gTransfarringMenuActions[action];
   work->mActionTime = 0;
}

void ShowLoadingBar(TransfarringWork *work)
{
   SMenuObject *obj = &gTransfarringMenuObjects[kObj_LoadingBarRoot];
   DisplayMenuObject(obj);
#if MGS_VERSION == 2
   work->mLoadingBarFadingIn = 0;
#endif
}

void HideLoadingBar(TransfarringWork *work)
{
   HideMenuObject(&gTransfarringMenuObjects[kObj_LoadingBarRoot]);
}

void BlinkLoadingBar(TransfarringWork *work)
{
   if(!work->pCurrentAction)
   {
      if(work->mLoadingBarFadingIn)
      {
         PlayAction(work, kAction_WifiTransfar_FadeLoadingBarIn);
      }
      else
      {
         PlayAction(work, kAction_WifiTransfar_FadeLoadingBarOut);
      }
      work->mLoadingBarFadingIn = !work->mLoadingBarFadingIn;
   }
}

void UpdateLoadingBar(TransfarringWork *work)
{
#if MGS_VERSION == 3
   SMenuObject * const obj = &gTransfarringMenuObjects[kObj_LoadingBarForeground];
   
   float width =  (Transfarring_GetProgress() * 0.01f);
   width = width > 1.0f ? 1.0f : width;
   
   obj->curStatus.col = skLoadingBarForegroundColor;
   obj->curStatus.w = obj->refStatus.w * width;
#endif
#if MGS_VERSION == 2
   BlinkLoadingBar(work);
#endif
}

static void DisplayStatusMessageAndSaveType(TransfarringWork *work, char const *message, ETransfarringStringId saveTypeId)
{
   SetMenuObjectText(&gTransfarringMenuObjects[kObj_StatusMessageText], kTString_USE_OVERRIDE, message);
   SetMenuObjectText(&gTransfarringMenuObjects[kObj_SaveTypeText], saveTypeId, 0);
}

static void DisplayStatusMessageByIdAndSaveType(TransfarringWork *work, ETransfarringStringId id, ETransfarringStringId saveTypeId)
{
   SetMenuObjectText(&gTransfarringMenuObjects[kObj_StatusMessageText], id, 0);
   SetMenuObjectText(&gTransfarringMenuObjects[kObj_SaveTypeText], saveTypeId, 0);
}

static void DisplayStatusMessage(TransfarringWork *work, char const *message)
{
   DisplayStatusMessageAndSaveType(work, message, kTString_Empty);
}

static void DisplayStatusMessageById(TransfarringWork *work, ETransfarringStringId id)
{
   DisplayStatusMessageByIdAndSaveType(work, id, kTString_Empty);
}

static TransfarringSaveUiData* GetSaveForCursorPosition(TransfarringWork *work, int cursorX, int cursorY)
{
   ASSERT(cursorX >=0 && cursorX < 2);
   ASSERT(cursorY >=0 && cursorY < SAVE_SLOT_QUANT_PER_COLUMN);

   if(cursorX)
   {
      return work->mSaves_VITA[cursorY + work->mSaveTableBaseIndex];
   }
   else
   {
      return work->mSaves_PS3[cursorY + work->mSaveTableBaseIndex];
   }
}

static TransfarringSaveUiData* GetSaveForCursor(TransfarringWork *work, SCursor *cursor)
{
   return GetSaveForCursorPosition(work, cursor->x, cursor->y);
}

static void GetSaveTypeAndIndexFromCursor(TransfarringWork *work, EPlatformSaveType *outType, int *outIndex, SCursor *cursor)
{
   *outType = (EPlatformSaveType) cursor->x;
   *outIndex = cursor->y + work->mSaveTableBaseIndex;
}

static int HasSaveBeenTransfarred(TransfarringSaveUiData *save)
{
   return save && save->mTransfarringID[0] && save->mTransfarringID[1];
}

static int AreSaveSlotsLinkedAtCursorY(TransfarringWork *work, int y)
{
   TransfarringSaveUiData *ps3 = GetSaveForCursorPosition(work, 0, y);
   TransfarringSaveUiData *vita = GetSaveForCursorPosition(work, 1, y);
   return ps3 && vita;
}

static int AreSaveSlotsOrphanedAtCursorY(TransfarringWork *work, int y)
{
   TransfarringSaveUiData *ps3 = GetSaveForCursorPosition(work, 0, y);
   TransfarringSaveUiData *vita = GetSaveForCursorPosition(work, 1, y);

   if(work->mSaveTableType == kSaveTable_Wifi)
   {
      return (ps3 && !ps3->mIsCorrupt && !vita && HasSaveBeenTransfarred(ps3) && !Transfarring_IsFileCloud(work->mCurrentSaveType, ps3->mSaveIndex))
         || (!ps3 && vita && !vita->mIsCorrupt && HasSaveBeenTransfarred(vita));
   }
   else
   {
      return (ps3 && !ps3->mIsCorrupt && !vita && BP_Network_IsSignedIn() && Transfarring_IsFileCloud(work->mCurrentSaveType, ps3->mSaveIndex))
         || (!ps3 && vita && !vita->mIsCorrupt);
   }
}

static void HideSaveSlotIcons(int slotBaseIndex)
{
   SMenuObject *obj;

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotLock_0 - kObj_LeftSlot_0];
   HideMenuObject(obj);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotLink_0 - kObj_LeftSlot_0];
   HideMenuObject(obj);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotCloud_0 - kObj_LeftSlot_0];
   HideMenuObject(obj);
}

static void SetWifiTransfarSaveSlotData(int slotBaseIndex, ESaveType saveType, TransfarringSaveUiData *slotData,
                                        EPlatformSaveType saveOrigin, ESaveTableTypes saveTableType, EUseIcons useIcons)
{
   if(slotData)
   {
      SMenuObject *obj = &gTransfarringMenuObjects[slotBaseIndex];   

      DisplayMenuObject(obj);

      obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotId_0 - kObj_LeftSlot_0];
      SetMenuObjectText(obj, kTString_USE_OVERRIDE, slotData->mIdStr);
     
      obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotClear_0 - kObj_LeftSlot_0];
      if(saveType == kST_Game && slotData->mClearCount > 0)
      {
#if MGS_VERSION == 3
         SetMenuObjectText(obj, kTString_USE_OVERRIDE, slotData->mClearCountStr);
#else
         //MGS2's clear count is only cleared/not cleared so we are just displaying a 'C' (MGSTWO-3313).
         SetMenuObjectText(obj, kTString_USE_OVERRIDE, " C ");
#endif
      }
      else
      {
         SetMenuObjectText(obj, kTString_Empty, 0);
      }
      
      obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotDiff_0 - kObj_LeftSlot_0];
#if MGS_VERSION == 3
      SetMenuObjectText(obj, kTString_USE_OVERRIDE, BP_SafeGetDifficultyStr(slotData->mDifficulty));
#endif
#if MGS_VERSION == 2
      if(saveType != kST_Game)
      {
         SetMenuObjectText(obj, kTString_Empty, 0);
      }
      else
      {
         int diff = (slotData->mDifficulty - GM_LEVEL_VERYEASY) / 10;
         if(!(slotData->mDifficulty % 10) && diff >= 0 && diff <= 5)
         {
            SetMenuObjectText(obj, kTString_USE_OVERRIDE, BP_SafeGetDifficultyStr(diff));
         }
         else
         {
            SetMenuObjectText(obj, kTString_BrokenLinkDifficulty, 0);
         }
      }
#endif

      obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotDate_0 - kObj_LeftSlot_0];
      SetMenuObjectText(obj, kTString_USE_OVERRIDE, slotData->mDateStr);

      obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotTime_0 - kObj_LeftSlot_0];
#if MGS_VERSION == 3
      SetMenuObjectText(obj, kTString_USE_OVERRIDE, slotData->mPlayTimeStr);
#endif
#if MGS_VERSION == 2
      if(saveType == kST_SnakeTales)
      {
         SetMenuObjectText(obj, kTString_Empty, 0);
      }
      else
      {
         SetMenuObjectText(obj, kTString_USE_OVERRIDE, slotData->mPlayTimeStr);
      }
#endif

      obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotHigh_0 - kObj_LeftSlot_0];
      obj->status = obj->refStatus;
      obj->curStatus = obj->refStatus;

      if(useIcons == kUseIcons && saveOrigin == kSaveLocal)
      {
         SMenuObject * const lockObj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotLock_0 - kObj_LeftSlot_0];
         SMenuObject * const linkObj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotLink_0 - kObj_LeftSlot_0];
         SMenuObject * const cloudObj = obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotCloud_0 - kObj_LeftSlot_0];

         if(Transfarring_IsFileLocked(saveType, slotData->mSaveIndex))
         {
            DisplayMenuObjectWithColor(lockObj, skLockColor);
            HideMenuObject(linkObj);
            HideMenuObject(cloudObj);
         }
         else if(Transfarring_IsFileLinked(saveType, slotData->mSaveIndex))
         {
            HideMenuObject(lockObj);
            DisplayMenuObjectWithColor(linkObj, skLockColor);
            HideMenuObject(cloudObj);
         }
         else if(Transfarring_IsFileCloud(saveType, slotData->mSaveIndex))
         {
            HideMenuObject(lockObj);
            HideMenuObject(linkObj);

            if(BP_Network_IsSignedIn() && Transfarring_HasCloudConflict(saveType, slotData->mSaveIndex))
            {
               DisplayMenuObjectWithColor(cloudObj, skCloudConflictColor);
            }
            else
            {
               DisplayMenuObjectWithColor(cloudObj, skCloudColor);
            }
         }
         else
         {
            HideSaveSlotIcons(slotBaseIndex);
         }
      }
      else
      {
        HideSaveSlotIcons(slotBaseIndex);
      }
   }
}

static void SetWifiTransfarBrokenLinkSlotData(int slotBaseIndex)
{
   SMenuObject *obj = &gTransfarringMenuObjects[slotBaseIndex];

   DisplayMenuObject(obj);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotId_0 - kObj_LeftSlot_0];
   SetMenuObjectText(obj, kTString_BrokenLinkLabel, 0);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotClear_0 - kObj_LeftSlot_0];
   SetMenuObjectText(obj, kTString_Empty, 0);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotDiff_0 - kObj_LeftSlot_0];
   SetMenuObjectText(obj, kTString_BrokenLinkDifficulty, 0);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotDate_0 - kObj_LeftSlot_0];
   SetMenuObjectText(obj, kTString_BrokenLinkDate, 0);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotTime_0 - kObj_LeftSlot_0];
   SetMenuObjectText(obj, kTString_BrokenLinkPlayTime, 0);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotHigh_0 - kObj_LeftSlot_0];
   obj->status = obj->refStatus;
   obj->curStatus = obj->refStatus;

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotLock_0 - kObj_LeftSlot_0];
   HideMenuObject(obj);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotLink_0 - kObj_LeftSlot_0];
   HideMenuObject(obj);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotCloud_0 - kObj_LeftSlot_0];
   HideMenuObject(obj);
}

static void SetWifiTransfarCorruptLinkSlotData(int slotBaseIndex)
{
   SMenuObject *obj = &gTransfarringMenuObjects[slotBaseIndex];

   DisplayMenuObject(obj);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotId_0 - kObj_LeftSlot_0];
   SetMenuObjectText(obj, kString_CorruptFileLabel, 0);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotClear_0 - kObj_LeftSlot_0];
   SetMenuObjectText(obj, kTString_Empty, 0);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotDiff_0 - kObj_LeftSlot_0];
   SetMenuObjectText(obj, kTString_BrokenLinkDifficulty, 0);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotDate_0 - kObj_LeftSlot_0];
   SetMenuObjectText(obj, kTString_BrokenLinkDate, 0);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotTime_0 - kObj_LeftSlot_0];
   SetMenuObjectText(obj, kTString_BrokenLinkPlayTime, 0);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotHigh_0 - kObj_LeftSlot_0];
   obj->status = obj->refStatus;
   obj->curStatus = obj->refStatus;

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotLock_0 - kObj_LeftSlot_0];
   HideMenuObject(obj);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotLink_0 - kObj_LeftSlot_0];
   HideMenuObject(obj);

   obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LeftSlotCloud_0 - kObj_LeftSlot_0];
   HideMenuObject(obj);
}

static void SetWifiTransfarEmptySlotData(int slotBaseIndex)
{
   SMenuObject *obj = &gTransfarringMenuObjects[slotBaseIndex];
   HideMenuObject(obj);
}

static void SetWifiTransfarLinkedSlotData(int slotBaseIndex, ELinkedState state)
{
   SMenuObject *obj = &gTransfarringMenuObjects[slotBaseIndex];

   if(state == kLinkedState_None)
   {
      HideMenuObject(obj);
   }
   else
   {
      DisplayMenuObject(obj);

      obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LinkedSlotText_0 - kObj_LinkedSlot_0];
      if(state == kLinkedState_Linked)
      {
        DisplayMenuObject(obj);
      }
      else
      {
         DisplayMenuObjectWithColor(obj, skBrokenLinkTint);
      }

      obj = &gTransfarringMenuObjects[slotBaseIndex + kObj_LinkedSlotHigh_0 - kObj_LinkedSlot_0];
      obj->status = obj->refStatus;
      obj->curStatus = obj->refStatus;
   }
}

static void UpdateSaveTableView(TransfarringWork *work)
{
   int i;

   for(i = 0; i < SAVE_SLOT_QUANT_PER_COLUMN; ++i)
   {
      int linkedIndex = kObj_LinkedSlot_0 + i * PRIMITIVES_PER_LINKED_SLOT;
      int leftIndex = kObj_LeftSlot_0 + i * PRIMITIVES_PER_SAVE_SLOT;
      int rightIndex = kObj_RightSlot_0 + i * PRIMITIVES_PER_SAVE_SLOT;

      TransfarringSaveUiData *ps3Save = GetSaveForCursorPosition(work, 0, i);
      TransfarringSaveUiData *vitaSave = GetSaveForCursorPosition(work, 1, i);

      if(AreSaveSlotsLinkedAtCursorY(work, i))
      {
         //Left
         SetWifiTransfarSaveSlotData(leftIndex, work->mCurrentSaveType, ps3Save, kSaveLocal, work->mSaveTableType, kUseIcons);
         //Right
         SetWifiTransfarSaveSlotData(rightIndex, work->mCurrentSaveType, vitaSave, kSaveRemote, work->mSaveTableType, kHideIcons);
         //Linked
         SetWifiTransfarLinkedSlotData(linkedIndex, kLinkedState_Linked);
      }
      else
      {
         if(ps3Save)
         {
            if(ps3Save->mIsCorrupt)
            {
               //Left
               SetWifiTransfarCorruptLinkSlotData(leftIndex);
               //Right
               SetWifiTransfarEmptySlotData(rightIndex);
               //Linked
               SetWifiTransfarLinkedSlotData(linkedIndex, kLinkedState_None);
            }
            else
            {
               //Left
               SetWifiTransfarSaveSlotData(leftIndex, work->mCurrentSaveType, ps3Save, kSaveLocal, work->mSaveTableType, kUseIcons);
               if(HasSaveBeenTransfarred(ps3Save) && (
                  (work->mSaveTableType == kSaveTable_Wifi)
                  || (work->mSaveTableType == kSaveTable_Cloud && Transfarring_IsFileCloud(work->mCurrentSaveType, ps3Save->mSaveIndex) && BP_Network_IsSignedIn())))
               {
                  //Right
                  SetWifiTransfarBrokenLinkSlotData(rightIndex);
                  //Linked
                  SetWifiTransfarLinkedSlotData(linkedIndex, kLinkedState_Broken);
               }
               else
               {
                  //Right
                  SetWifiTransfarEmptySlotData(rightIndex);
                  //Linked
                  SetWifiTransfarLinkedSlotData(linkedIndex, kLinkedState_None);
               }
            }
         }
         else if(vitaSave)
         {
            if(vitaSave->mIsCorrupt)
            {
               //Left
               SetWifiTransfarCorruptLinkSlotData(rightIndex);
               //Right
               SetWifiTransfarEmptySlotData(leftIndex);
               //Linked
               SetWifiTransfarLinkedSlotData(linkedIndex, kLinkedState_None);
            }
            else
            {
               //Right
               SetWifiTransfarSaveSlotData(rightIndex, work->mCurrentSaveType, vitaSave, kSaveRemote, work->mSaveTableType, kHideIcons);
               if(HasSaveBeenTransfarred(vitaSave))
               {
                  //Left
                  SetWifiTransfarBrokenLinkSlotData(leftIndex);
                  //Linked
                  SetWifiTransfarLinkedSlotData(linkedIndex, kLinkedState_Broken);
               }
               else
               {
                  //Left
                  SetWifiTransfarEmptySlotData(leftIndex);
                  //Linked
                  SetWifiTransfarLinkedSlotData(linkedIndex, kLinkedState_None);
               }
            }
         }
         else
         {
            //Left
            SetWifiTransfarEmptySlotData(leftIndex);
            //Right
            SetWifiTransfarEmptySlotData(rightIndex);
            //Linked
            SetWifiTransfarLinkedSlotData(linkedIndex, kLinkedState_None);
         }
      }
   }
}

static int IsSaveTableCursorPositionValid(TransfarringWork *work, SCursor *cursor)
{
   return (int)GetSaveForCursor(work, cursor);
}

static void DisplayButtonOptions(int showX, int showSQ, ETransfarringStringId xId, ETransfarringStringId sqId)
{
   SMenuObject *rootX = &gTransfarringMenuObjects[kObj_ButtonDisplay_X_Root];
   SMenuObject *rootSQ = &gTransfarringMenuObjects[kObj_ButtonDisplay_SQ_Root];

   if(showX)
   {
      DisplayMenuObject(rootX);
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_ButtonDisplay_X_Text], xId, 0);
   }
   else
   {
      HideMenuObject(rootX);
   }

   if(showSQ)
   {
      DisplayMenuObject(rootSQ);
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_ButtonDisplay_SQ_Text], sqId, 0);
   }
   else
   {
      HideMenuObject(rootSQ);
   }
}

static ETransfarringStringId GetStringIdForTransfarringType(ETransfarringType type)
{
   ETransfarringStringId textId = kTString_Empty;

   switch(type)
   {
      case kType_ToVita:
         textId = kTString_Button_TransfarrToVita;
         break;
      case kType_ToVitaOrphaned:
         textId = kTString_Button_TransfarrToVitaOrphaned;
         break;
      case kType_ToVitaLinked:
         textId = kTString_Button_TransfarrToVitaLinked;
         break;
      case kType_FromVita:
         textId = kTString_Button_TransfarrFromVita;
         break;
      case kType_FromVitaOrphaned:
         textId = kTString_Button_TransfarrFromVitaOrphaned;
         break;
      case kType_FromVitaLinked:
         textId = kTString_Button_TransfarrFromVitaLinked;
         break;
      case kType_Cloudify:
         textId = kTString_Button_Cloudify;
         break;
      case kType_Delete:
         textId = kTString_Button_Delete;
         break;
      case kType_UncloudifyLinked:
         textId = kTString_Button_UncloudifyLinked;
         break;
      case kType_UncloudifyOrphanedLocal:
         textId = kTString_Button_UncloudifyOrphanedLocal;
         break;
      case kType_UncloudifyOrphanedCloud:
         textId = kTString_Button_UncloudifyOrphanedCloud;
         break;
      case kType_Unlock:
         textId = kTString_Button_Unlock;
         break;
      case kType_Unlock_Vita:
         textId = kTString_Button_Unlock_Vita;
         break;
      case kType_Unlink:
         textId = kTString_Button_Unlink;
         break;
      case kType_Unlink_Vita:
         textId = kTString_Button_Unlink_Vita;
         break;
      case kType_SolveConflict:
         textId = kTString_Button_SolveConflict;
         break;
      default:
         break;
   }
     
   return textId;
}

static void UpdateButtonOptions(TransfarringWork *work, SCursor *cursor)
{
   work->mTypeButtonX = kType_None;
   work->mTypeButtonSq = kType_None;

   if(work->mSaveTableType == kSaveTable_Wifi)
   {
      if(Transfarring_IsFileDataValid(work))
      {
         if(AreSaveSlotsLinkedAtCursorY(work, cursor->y))
         {
            TransfarringSaveUiData * const save = GetSaveForCursorPosition(work, 0, cursor->y);
             if(save && !save->mIsCorrupt)
            {
               if(Transfarring_IsFileLocked(work->mCurrentSaveType, save->mSaveIndex))
               {
                  work->mTypeButtonX = kType_FromVitaLinked;
               }
               else
               {
                  work->mTypeButtonX = kType_ToVitaLinked;
               }
            }
         }
         else
         {
            TransfarringSaveUiData *save = GetSaveForCursor(work, cursor);
            if(save && !save->mIsCorrupt)
            {
               if(cursor->x)
               {
                  if(HasSaveBeenTransfarred(save))
                  {
                     work->mTypeButtonX = kType_FromVitaOrphaned;
                  }
                  else
                  {
                     work->mTypeButtonX = kType_FromVita;
                  }
               }
               else if(!Transfarring_IsFileCloud(work->mCurrentSaveType, save->mSaveIndex))
               {
                  if(HasSaveBeenTransfarred(save))
                  {
                     work->mTypeButtonX = kType_ToVitaOrphaned;
                  }
                  else
                  {
                     work->mTypeButtonX = kType_ToVita;
                  }
               }
            }
         }
      }
   }
   else if(work->mSaveTableType == kSaveTable_Cloud)
   {
      if(IsSaveTableCursorPositionValid(work, cursor) && Transfarring_IsLocalFileListValid())
      {
         if(BP_Network_IsSignedIn() && Transfarring_IsCloudFileListValid())
         {
            if(AreSaveSlotsLinkedAtCursorY(work, cursor->y))
            {
               TransfarringSaveUiData * const save = GetSaveForCursorPosition(work, 0, cursor->y);
               if(save && Transfarring_HasCloudConflict(work->mCurrentSaveType, save->mSaveIndex))
               {
                  work->mTypeButtonX = kType_SolveConflict;
               }
               work->mTypeButtonSq = kType_UncloudifyLinked;
            }
            else
            {
               TransfarringSaveUiData *save = GetSaveForCursor(work, cursor);
               if(save)
               {
                  if(cursor->x)
                  {
                     work->mTypeButtonSq = kType_UncloudifyOrphanedCloud;
                  }
                  else
                  {
                     if(save->mIsCorrupt)
                     {
#ifdef BP_VITA
                        work->mTypeButtonSq = kType_Delete;
#endif
                     }
                     else if(Transfarring_IsFileCloud(work->mCurrentSaveType, save->mSaveIndex))
                     {
                        work->mTypeButtonSq = kType_UncloudifyOrphanedLocal;
                     }
                     else if(Transfarring_IsFileLocked(work->mCurrentSaveType, save->mSaveIndex))
                     {
#ifdef BP_VITA
                        work->mTypeButtonSq = kType_Unlock_Vita;
#else
                        work->mTypeButtonSq = kType_Unlock;
#endif
                     }
                     else if(Transfarring_IsFileLinked(work->mCurrentSaveType, save->mSaveIndex))
                     {
#ifdef BP_VITA
                        work->mTypeButtonSq = kType_Unlink_Vita;
#else
                        work->mTypeButtonSq = kType_Unlink;
#endif
                     }
                     else
                     {
                        work->mTypeButtonX = kType_Cloudify;
#ifdef BP_VITA
                        work->mTypeButtonSq = kType_Delete;
#endif
                     }
                  }
               }
            }
         }
         else if(!cursor->x)
         {
            TransfarringSaveUiData *save = GetSaveForCursor(work, cursor);
            if(save)
            {
               if(!save->mIsCorrupt && Transfarring_IsFileCloud(work->mCurrentSaveType, save->mSaveIndex))
               {
                  work->mTypeButtonSq = kType_UncloudifyOrphanedLocal;
               }
               else if(!save->mIsCorrupt && Transfarring_IsFileLocked(work->mCurrentSaveType, save->mSaveIndex))
               {
#ifdef BP_VITA
                  work->mTypeButtonSq = kType_Unlock_Vita;
#else
                  work->mTypeButtonSq = kType_Unlock;
#endif
               }
               else if(!save->mIsCorrupt && Transfarring_IsFileLinked(work->mCurrentSaveType, save->mSaveIndex))
               {
#ifdef BP_VITA
                  work->mTypeButtonSq = kType_Unlink_Vita;
#else
                  work->mTypeButtonSq = kType_Unlink;
#endif
               }
               else
               {
#ifdef BP_VITA
                  work->mTypeButtonSq = kType_Delete;
#endif
               }
            }
         }
      }
   }
}

static void UpdateButtonOptionStageNameAndTypeMessage(TransfarringWork *work, SCursor *cursor)
{
   TransfarringSaveUiData * const save = GetSaveForCursor(work, cursor);

   UpdateButtonOptions(work, cursor);

   DisplayButtonOptions(work->mTypeButtonX != kType_None, work->mTypeButtonSq != kType_None, 
      GetStringIdForTransfarringType(work->mTypeButtonX), GetStringIdForTransfarringType(work->mTypeButtonSq));

   if(save)
   {
      if(!save->mIsCorrupt)
      {
         char const * stageString = "";
         TransfarringSaveUiData * const localSave = GetSaveForCursorPosition(work, 0, cursor->y);
         TransfarringSaveUiData * const remoteSave = GetSaveForCursorPosition(work, 1, cursor->y);

#if MGS_VERSION == 2
         if((work->mCurrentSaveType == kST_Game || work->mCurrentSaveType == kST_SnakeTales) && save->mStageIndex < MC_N_STAGES)
#endif
#if MGS_VERSION == 3
         if(work->mCurrentSaveType == kST_Game)
#endif      
         {
            if(localSave && remoteSave)
            {
               if(Transfarring_IsFileLocked(work->mCurrentSaveType, localSave->mSaveIndex))
               {
                  stageString = BP_SafeGetStageNameStr(remoteSave->mStageIndex);
               }
               else
               {
                  stageString = BP_SafeGetStageNameStr(localSave->mStageIndex);
               }
            }
            else
            {
               stageString = BP_SafeGetStageNameStr(save->mStageIndex);
            }
         }
         
         if(work->mSaveTableType == kSaveTable_Wifi)
         {
            if(AreSaveSlotsLinkedAtCursorY(work, cursor->y))
            {
               DisplayStatusMessageAndSaveType(work, stageString, Transfarring_IsFileLocked(work->mCurrentSaveType, localSave->mSaveIndex)
                  ? kString_FileLockedAndLinked : kString_FileLockedAndLinkedVita);
            }
            else if(AreSaveSlotsOrphanedAtCursorY(work, cursor->y))
            {
               DisplayStatusMessageAndSaveType(work, stageString, cursor->x ? kString_FileLinkedOrphaned : kString_FileLockedOrphaned);
            }
            else
            {
               DisplayStatusMessage(work, stageString);
            }
         }
         else
         {
            if(AreSaveSlotsLinkedAtCursorY(work, cursor->y))
            {
               TransfarringSaveUiData * const localSave = GetSaveForCursorPosition(work, 0, cursor->y);
               if(localSave && Transfarring_HasCloudConflict(work->mCurrentSaveType, localSave->mSaveIndex))
               {
                  DisplayStatusMessageAndSaveType(work, stageString, kString_FileWithConflict);
               }
               else
               {
                  DisplayStatusMessageAndSaveType(work, stageString, kString_FileCloudLinked);
               }
            }
            else if(AreSaveSlotsOrphanedAtCursorY(work, cursor->y))
            {
               DisplayStatusMessageAndSaveType(work, stageString, cursor->x ? kString_FileCloudOrphanedCloud : kString_FileCloudOrphanedLocal);
            }
            else if(Transfarring_IsFileCloud(work->mCurrentSaveType, save->mSaveIndex))
            {
               //Cloud saves should be linked/orphaned/corrupt only.
               ASSERT(!cursor->x);
               DisplayStatusMessageAndSaveType(work, stageString, kString_FileCloud);
            }
            else if(Transfarring_IsFileLocked(work->mCurrentSaveType, save->mSaveIndex))
            {
               //Cloud saves should be linked/orphaned/corrupt only.
               ASSERT(!cursor->x);
#if defined(BP_VITA)
               DisplayStatusMessageAndSaveType(work, stageString, kString_FileLockedVita);
#else
               DisplayStatusMessageAndSaveType(work, stageString, kString_FileLocked);
#endif
            }
            else if(Transfarring_IsFileLinked(work->mCurrentSaveType, save->mSaveIndex))
            {
               //Cloud saves should be linked/orphaned/corrupt only.
               ASSERT(!cursor->x);
#if defined(BP_VITA)
               DisplayStatusMessageAndSaveType(work, stageString, kString_FileLinkedVita);
#else
               DisplayStatusMessageAndSaveType(work, stageString, kString_FileLinked);
#endif
            }
            else
            {
               DisplayStatusMessage(work, stageString);
            }
         }
      }
      else
      {
         DisplayStatusMessageByIdAndSaveType(work, kTString_Empty, kString_FileBroken);
      }
   }
   else
   {
      DisplayStatusMessageById(work, kTString_Empty);
   }
}

static void SelectWifiTransfarSave(TransfarringWork *work, SCursor *cursor)
{
   SMenuObject * obj;
   unsigned const selectedColor = gTransfarringMenuActions[kAction_WifiTransfar_SaveSelect].objActions[1].items[0].status.col;

   UpdateSaveTableView(work);

   if(IsSaveTableCursorPositionValid(work, &work->mSaveTableCursor))
   {
      if(AreSaveSlotsLinkedAtCursorY(work, cursor->y) || AreSaveSlotsOrphanedAtCursorY(work, cursor->y))
      {
         obj = &gTransfarringMenuObjects[kObj_LinkedSlotHigh_0 + PRIMITIVES_PER_LINKED_SLOT * cursor->y];
      }
      else
      {
         obj = &gTransfarringMenuObjects[(cursor->x ? kObj_RightSlotHigh_0 : kObj_LeftSlotHigh_0) + PRIMITIVES_PER_SAVE_SLOT * cursor->y];
      }

      DisplayMenuObjectWithColor(obj, selectedColor);
   }
   
   UpdateButtonOptionStageNameAndTypeMessage(work, cursor);
}

static void SelectWifiTransfarSaveWithAnimation(TransfarringWork *work, SCursor *cursorPrev, SCursor *cursorCurr)
{
   int objPrevId, objCurrId;
   SMenuObject *obj;
   unsigned selectedColor = gTransfarringMenuActions[kAction_WifiTransfar_SaveSelect].objActions[1].items[0].status.col;

   if(AreSaveSlotsLinkedAtCursorY(work, cursorPrev->y) || AreSaveSlotsOrphanedAtCursorY(work, cursorPrev->y))
   {
      objPrevId = kObj_LinkedSlotHigh_0 + PRIMITIVES_PER_LINKED_SLOT * cursorPrev->y;
      gTransfarringMenuActions[kAction_WifiTransfar_SaveSelect].objActions[0].items[0].status.w = SAVE_TABLE_WIDTH;
   }
   else
   {
      objPrevId = (cursorPrev->x ? kObj_RightSlotHigh_0 : kObj_LeftSlotHigh_0) + PRIMITIVES_PER_SAVE_SLOT * cursorPrev->y;
      gTransfarringMenuActions[kAction_WifiTransfar_SaveSelect].objActions[0].items[0].status.w = SAVE_SLOT_HIGHLIGHT_WIDTH;
   }
   if(AreSaveSlotsLinkedAtCursorY(work, cursorCurr->y) || AreSaveSlotsOrphanedAtCursorY(work, cursorCurr->y))
   {
      objCurrId = kObj_LinkedSlotHigh_0 + PRIMITIVES_PER_LINKED_SLOT * cursorCurr->y;
      gTransfarringMenuActions[kAction_WifiTransfar_SaveSelect].objActions[1].items[0].status.w = SAVE_TABLE_WIDTH;
   }
   else
   {
      objCurrId = (cursorCurr->x ? kObj_RightSlotHigh_0 : kObj_LeftSlotHigh_0) + PRIMITIVES_PER_SAVE_SLOT * cursorCurr->y;
      gTransfarringMenuActions[kAction_WifiTransfar_SaveSelect].objActions[1].items[0].status.w = SAVE_SLOT_HIGHLIGHT_WIDTH;
   }

   gTransfarringMenuActions[kAction_WifiTransfar_SaveSelect].objActions[0].object = objPrevId;
   gTransfarringMenuActions[kAction_WifiTransfar_SaveSelect].objActions[1].object = objCurrId;

   UpdateSaveTableView(work);

   obj = &gTransfarringMenuObjects[objPrevId];
   DisplayMenuObjectWithColor(obj, selectedColor);
   
   PlayAction(work, kAction_WifiTransfar_SaveSelect);
   PlayMenuSelectionSound();

   UpdateButtonOptionStageNameAndTypeMessage(work, cursorCurr);
}

static int UpdateSaveTableCursor(TransfarringWork *work, int dX, int dY, EPlayAnimationAction playAction)
{
   SCursor oldCursor = work->mSaveTableCursor;
   int oldBaseIndex = work->mSaveTableBaseIndex;
   int valid = 1;

   work->mSaveTableCursor.x += dX;
   work->mSaveTableCursor.y += dY;

   work->mSaveTableCursor.x = work->mSaveTableCursor.x > 1 ? 1 : (work->mSaveTableCursor.x < 0 ? 0 : work->mSaveTableCursor.x);

   if(work->mSaveTableCursor.y >= SAVE_SLOT_QUANT_PER_COLUMN)
   {
      work->mSaveTableBaseIndex = work->mSaveTableBaseIndex + work->mSaveTableCursor.y - SAVE_SLOT_QUANT_PER_COLUMN + 1;
      work->mSaveTableCursor.y = SAVE_SLOT_QUANT_PER_COLUMN - 1;

      valid = (work->mSaveTableBaseIndex + SAVE_SLOT_QUANT_PER_COLUMN - 1) < SAVE_SLOT_DATA_ARRAY_SIZE;
   }
   else if(work->mSaveTableCursor.y < 0)
   {
      work->mSaveTableBaseIndex = work->mSaveTableBaseIndex + work->mSaveTableCursor.y;
      work->mSaveTableCursor.y = 0;

      valid = work->mSaveTableBaseIndex >= 0;
   }

   if(valid && !IsSaveTableCursorPositionValid(work, &work->mSaveTableCursor))
   {
      work->mSaveTableCursor.x = 1 - work->mSaveTableCursor.x;
      valid = IsSaveTableCursorPositionValid(work, &work->mSaveTableCursor);
   }

   if(valid)
   {
      if(playAction == kPlayAction_Yes)
      {
         oldCursor.y -= work->mSaveTableBaseIndex - oldBaseIndex;
         SelectWifiTransfarSaveWithAnimation(work, &oldCursor, &work->mSaveTableCursor);
      }
      else
      {
         SelectWifiTransfarSave(work, &work->mSaveTableCursor);
      }
   }
   else
   {
      work->mSaveTableCursor = oldCursor;
      work->mSaveTableBaseIndex = oldBaseIndex;
   }

   return valid;
}

static int SetSaveTableCursorBySaveIndex(TransfarringWork *work, EPlatformSaveType saveType, int saveIndex, EPlayAnimationAction playAction)
{
   int valid = 1;

   int saveIndexInOldBase = saveIndex - work->mSaveTableBaseIndex;

   if(saveIndexInOldBase >= SAVE_SLOT_QUANT_PER_COLUMN || saveIndexInOldBase < 0)
   {
      work->mSaveTableCursor.x = 0;
      work->mSaveTableCursor.y = 0;
      work->mSaveTableBaseIndex = 0;
      valid = UpdateSaveTableCursor(work, saveType, saveIndex, playAction);
   }
   else
   {
      SCursor newCursor = { saveType, saveIndexInOldBase };

      if(!IsSaveTableCursorPositionValid(work, &newCursor))
      {
         newCursor.x = 1 - newCursor.x;
         valid = IsSaveTableCursorPositionValid(work, &newCursor);
      }

      if(valid)
      {
         work->mSaveTableCursor = newCursor;
         SelectWifiTransfarSave(work, &work->mSaveTableCursor);
      }
   }

   return valid;
}

static TransfarringSaveUiData* CreateSaveUiDataItem(TransfarringWork *work, TransfarringSaveFile *save,
                                                    EPlatformSaveType saveType, ESaveTableTypes saveTableType, int index)
{
   TransfarringSaveUiData *data;

   ASSERT(work->mSavesBufferIndex < (sizeof(work->mSavesBuffer) / sizeof(work->mSavesBuffer[0])));

   data = &work->mSavesBuffer[work->mSavesBufferIndex++];
   GV_ZeroMemory(data, sizeof(TransfarringSaveUiData));

   data->mSaveIndex = save->mSaveIndex;
   data->mStageIndex = save->mStageIndex;
   data->mDifficulty = save->mDifficulty;
   data->mClearCount = save->mClearCount;

#if MGS_VERSION == 2
   // The PS3 only uses one bit to store the clear count. Cap the MGS2 clear count at 1, but only for display purposes.
   if( data->mClearCount > 0 )
   {
      data->mClearCount = 1;
   }
#endif

   data->mTransfarringID[0] = save->transfarringID[0];
   data->mTransfarringID[1] = save->transfarringID[1];

   if(work->mSaveTableType == kSaveTable_Cloud && saveType == kSaveRemote)
   {
      data->mIsCorrupt = Transfarring_IsCloudFileCorrupt(work->mCurrentSaveType, index);
   }
   else if( saveType == kSaveLocal )
   {
      data->mIsCorrupt = Transfarring_IsLocalFileCorrupt( work->mCurrentSaveType, index );
   }

   if(!data->mIsCorrupt)
   {
      unsigned int clock;
      int i, hour, minute, second;
      int inter = BP_FRAMES_PER_SEC();
      char buffer[SAVE_FILE_CHAR_BUFFER_LENGTH];

      //SLOT NUMBER
      if(saveType == kSaveRemote && saveTableType == kSaveTable_Cloud)
      {
         sprintf(buffer, "C%02d", save->mSaveIndex);
      }
      else
      {
         sprintf(buffer, "%03d", save->mSaveIndex);
      }
      for(i = 0; i < SAVE_FILE_ID_ARRAY_LENGTH - 1; ++i)
      {
         data->mIdStr[i] = buffer[i];
      }
      data->mIdStr[SAVE_FILE_ID_ARRAY_LENGTH - 1] = 0;
      
      //CLEAR COUNT
      sprintf(buffer, "%03d", data->mClearCount);
      for(i = 0; i < SAVE_FILE_CLEAR_COUNT_ARRAY_LENGTH - 1; ++i)
      {
         data->mClearCountStr[i] = buffer[i];
      }
      data->mClearCountStr[SAVE_FILE_CLEAR_COUNT_ARRAY_LENGTH - 1] = 0;

      //PLAY TIME
      clock = save->mPlayTime / inter;

#if MGS_VERSION == 3
      MC_Clock2HMS(&clock, &hour, &minute, &second);
#endif
#if MGS_VERSION == 2
      if( clock >= 60 * 60 * 10000) clock = 60 * 60 * 10000 - 1;
      hour = clock / 3600;
      minute = (clock / 60) % 60;
      second = clock % 60;
#endif

      sprintf(buffer, "%04d:%02d:%02d", hour, minute, second);
      for(i = 0; i < SAVE_FILE_PLAYTIME_ARRAY_LENGTH - 1; ++i)
      {
         data->mPlayTimeStr[i] = buffer[i];
      }
      data->mPlayTimeStr[SAVE_FILE_PLAYTIME_ARRAY_LENGTH - 1] = 0;

      //DATE
      sprintf(buffer, "%04d.%02d.%02d", save->mSaveDate.mYear, save->mSaveDate.mMonth, save->mSaveDate.mDay);
      for(i = 0; i < SAVE_FILE_DATE_ARRAY_LENGTH - 1; ++i)
      {
         data->mDateStr[i] = buffer[i];
      }
      data->mDateStr[SAVE_FILE_DATE_ARRAY_LENGTH - 1] = 0;
   }
   return data;
}

static void DestroySaveUiData(TransfarringWork *work)
{
   int i;

   for(i = 0; i < SAVE_SLOT_DATA_ARRAY_SIZE; ++i)
   {
      work->mSaves_PS3[i] = 0;
      work->mSaves_VITA[i] = 0;
   }

   work->mSavesBufferIndex = 0;
}

static void LoadSaveGameTableData(TransfarringWork *work)
{
   int i;
   int size;
   TransfarringSaveUiData *ps3Saves[MAX_NUMBER_OF_SAVE_SLOTS + 1] = { 0 };
   TransfarringSaveUiData *vitaSaves[MAX_NUMBER_OF_SAVE_SLOTS + 1] = { 0 };
   TransfarringSaveUiData **ppDestPs3SavePointer, **ppDestVitaSavePointer, **ppSrcPs3SavePointer, **ppSrcVitaSavePointer;

   if(Transfarring_IsFileDataValid(work))
   {
      DestroySaveUiData(work);

      size = GetNumLocalSaveFiles(work);
      ppDestPs3SavePointer = ps3Saves;
      for(i = 0; i < size && i < MAX_NUMBER_OF_SAVE_SLOTS; ++i)
      {
         TransfarringSaveFile *tmp = GetLocalSaveFile(work, i);
         if(tmp)
         {
            *ppDestPs3SavePointer++ = CreateSaveUiDataItem(work, tmp, kSaveLocal, work->mSaveTableType, i);
         }
      }
      qsort(ps3Saves, ppDestPs3SavePointer - ps3Saves, sizeof(TransfarringSaveFile*), TransfarringSaveFileSortComparator);

      size = GetNumRemoteSaveFiles(work);
      ppDestVitaSavePointer = vitaSaves;
      for(i = 0; i < size && i < MAX_NUMBER_OF_SAVE_SLOTS; ++i)
      {
         TransfarringSaveFile *tmp = GetRemoteSaveFile(work, i);
         if(tmp)
         {
            *ppDestVitaSavePointer++ = CreateSaveUiDataItem(work, tmp, kSaveRemote, work->mSaveTableType, i);
         }
      }
      qsort(vitaSaves, ppDestVitaSavePointer - vitaSaves, sizeof(TransfarringSaveFile*), TransfarringSaveFileSortComparator);

      ppSrcPs3SavePointer = ps3Saves;
      ppSrcVitaSavePointer = vitaSaves;

      //Copy PS3 saves + linked Vita saves
      while(*ppSrcPs3SavePointer)
      {
         if(*ppSrcVitaSavePointer && HasSaveBeenTransfarred(*ppSrcPs3SavePointer) && HasSaveBeenTransfarred(*ppSrcVitaSavePointer))
         {
            int compRes = TransfarringIdCompare(*ppSrcPs3SavePointer, *ppSrcVitaSavePointer);

            if(compRes < 0)
            {
               ++ppSrcVitaSavePointer;
            }
            else if(compRes > 0)
            {
               int index = (*ppSrcPs3SavePointer)->mSaveIndex;
               work->mSaves_PS3[index] = *ppSrcPs3SavePointer++;
            }
            else
            {
               int index = (*ppSrcPs3SavePointer)->mSaveIndex;
               work->mSaves_PS3[index] = *ppSrcPs3SavePointer;
               work->mSaves_VITA[index] = *ppSrcVitaSavePointer;

               *ppSrcVitaSavePointer = 0;

               ++ppSrcPs3SavePointer;
               ++ppSrcVitaSavePointer;
            }   
         }
         else
         {
            int index = (*ppSrcPs3SavePointer)->mSaveIndex;
            work->mSaves_PS3[index] = *ppSrcPs3SavePointer++;
         }
      }

      //Remove empty PS3 slots 
      ppDestPs3SavePointer = work->mSaves_PS3;
      ppDestVitaSavePointer = work->mSaves_VITA;

      for(i = 0; i < SAVE_SLOT_DATA_ARRAY_SIZE; ++i)
      {
         if(work->mSaves_PS3[i])
         {
            *ppDestPs3SavePointer++ = work->mSaves_PS3[i];
            *ppDestVitaSavePointer++ = work->mSaves_VITA[i];
         }
      }

      //Copy remaining Vita saves
      for(i = 0; i < MAX_NUMBER_OF_SAVE_SLOTS; ++i)
      {
         if(vitaSaves[i])
         {
            *ppDestPs3SavePointer++ = 0;
            *ppDestVitaSavePointer++ = vitaSaves[i];
         }
      }

      //Clear remaining buffer
      while(ppDestPs3SavePointer != &work->mSaves_PS3[SAVE_SLOT_DATA_ARRAY_SIZE])
      {
         *ppDestPs3SavePointer++ = 0;
         *ppDestVitaSavePointer++ = 0;
      }
   }
}

static void SetSaveGameType(TransfarringWork *work, ESaveType saveType)
{
   ETransfarringStringId stringId = kTString_MainGame;
   SMenuObject * const pObj = &gTransfarringMenuObjects[kObj_WifiTransfar_BarText];

   work->mCurrentSaveType = saveType;

   DestroySaveUiData(work);
   LoadSaveGameTableData(work);

   work->mSaveTableCursor.x = 0;
   work->mSaveTableCursor.y = 0;
   work->mSaveTableBaseIndex = 0;

   if(!IsSaveTableCursorPositionValid(work, &work->mSaveTableCursor))
   {
      work->mSaveTableCursor.x = 1;
   }

   SelectWifiTransfarSave(work, &work->mSaveTableCursor);

#if MGS_VERSION == 2
   {
      SMenuObject * const pButtonR = &gTransfarringMenuObjects[kObj_WifiTransfar_BarR1];   
      
      switch(work->mCurrentSaveType)
      {
      case kST_Game:
         stringId = kTString_MainGame;
         break;
      case kST_SnakeTales:
         stringId = kTString_SnakeTales;
         break;
      case kST_VR:
         stringId = kTString_VrMisions;
         break;
      default:
         ASSERT(0);
         break;
      }
      pButtonR->curStatus.x = GAME_TYPE_X + BUTTON_R_DELTA_X
         + Transfarring_GetApproximatedTextWidth(pObj->scale, GetTransfarringString(stringId));
   }
#endif
   SetMenuObjectText(pObj, stringId, 0);
}

static void DisplayModalMessage(TransfarringWork *work, ETransfarringStringId id, char const *overrideString)
{
   work->mModalMessageId = id;
   work->mModalMessageOverride = overrideString;

   PushMenuState(work, kTransfarState_ModalMessage);
}

static void DisplaySystemDialogYesNoModal(TransfarringWork *work, ETransfarringStringId id, char const *overrideString)
{
   work->mSystemModalHandle = BP_CommonDialog_WantsMessageDialog(overrideString ? overrideString : GetTransfarringString(id), kMDL_YesNo);

   PushMenuState(work, kTransfarState_SystemModal_YesNo);
}

static int CanExitTransfarringMenu()
{
   int curStatus = Transfarring_GetTransfarringStatus();
#if defined(BP_PS3)
   return curStatus == kPTS_Uninitialized || curStatus == kPTS_AwaitingConnection || curStatus == kPTS_Idle;
#elif defined(BP_VITA)
   return curStatus == kVTS_Uninitialized || curStatus == kVTS_AwaitingConnection || curStatus == kVTS_Idle;
#else
   return 1;
#endif
}

static void ChangeExplanationPage(TransfarringWork *work)
{
   SMenuObject * const obj = &gTransfarringMenuObjects[kObj_TransfarringExplanationText];
   
   work->mExplanationPage = (EExplanationPage)((work->mExplanationPage + 1) % kNumPages);
   
   switch(work->mExplanationPage)
   {
   case kPageDefault:
      SetMenuObjectText(obj, work->mDefaultExplanationId, 0);
      break;
   case kPage_1:
      SetMenuObjectText(obj, kString_TutorialText, 0);
      break;
   case kPage_2:
      SetMenuObjectText(obj, kString_TutorialText_2, 0);
      break;
   case kPage_3:
      SetMenuObjectText(obj, kString_TutorialText_3, 0);
      break;
   default:
      ASSERT(0);
      break;
   }
}

static void SetDefaultExplanationId(TransfarringWork *work, ETransfarringStringId id)
{
   work->mDefaultExplanationId = id;
   
   work->mExplanationPage = kPageDefault;
   SetMenuObjectText(&gTransfarringMenuObjects[kObj_TransfarringExplanationText], work->mDefaultExplanationId, 0);
}

static void ExitFromMainMenuState(TransfarringWork *work)
{
   if (CanExitTransfarringMenu())
   {
#if MGS_VERSION == 3
      A_SdSet(SD_S_NORM_CANSEL_1);
#endif
      PopMenuState(work);
      PushMenuState(work, kTransfarState_Leaving);
   }
}

static int Init(TransfarringWork *work)
{
   int i;

   sFakeTick = 0;

#if MGS_VERSION == 2
   work->pCurrentAction = 0;
   work->pad_work = 0;
   work->ans = -1;
#endif

#if MGS_VERSION == 3
   A_Pad_Init(&work->pad_work);
   gTransfarringMenuDone = FALSE;
#endif

   work->mRenderingData = Transfarring_InitRenderingData();

   // run through and link up the menus
   work->mObjCount = gTransfarringMenuObjCount;
   for (i = 0; i < work->mObjCount; ++i)
   {
      SMenuObject *obj = &gTransfarringMenuObjects[i];
      obj->pFirstChild = NULL;
      obj->pSibling = NULL;
      obj->curStatus = obj->refStatus;

      Transfarring_InitMenuObjectRenderingData(obj);
   }
   for (i = 1; i < work->mObjCount; ++i)
   {
      SMenuObject *obj = &gTransfarringMenuObjects[i];
      int parentIndex = obj->parent;
      if (parentIndex != MENU_OBJECT_NONE)
      {
         SMenuObject *par = &gTransfarringMenuObjects[parentIndex];
         SMenuObject *sibling = par->pFirstChild;
         if (sibling == NULL)
         {
            par->pFirstChild = obj;
         }
         else
         {
            while (sibling->pSibling)
            {
               sibling = sibling->pSibling;
            }
            sibling->pSibling = obj;
         }
      }
   }

   //State Machine
   work->mMenuStateStackIndex = -1;
   PushMenuState(work, kTransfarState_Entering);

   work->mIsConnecting = 0;

   return 0;
}

static void Act(TransfarringWork *work)
{
   UpdateScene(work);
   Transfarring_RenderScene(work->mRenderingData);
   UpdatePad(work);
   UpdateMenuState(work);
}

static void Die(TransfarringWork *work)
{
   int i;

   DestroySaveUiData(work);

   for (i = 0; i < work->mObjCount; ++i)
   {
      SMenuObject *obj = &gTransfarringMenuObjects[i];
      Transfarring_DestroyMenuObjectRenderingData(obj);
   }

#if MGS_VERSION == 2
   if(work->parent_name!=0 &&
      work->parent_name!=1){

         GV_MSG msg;
         int message[5];

         msg.address=work->parent_name;
         msg.message=message;
         msg.message_len=sizeof(message)/sizeof(message[0]);

         message[0]=work->ans;
         //message[1]=work->game_cursor;

         GV_SendMessage(&msg);
   }
#endif

   Transfarring_DestroyRenderingData(work->mRenderingData);
   work->mRenderingData = 0;

#ifdef BP_VITA
   // TODO: Check for errors; it's possible that the thread will hang if this fails
   Transfarring_PostNetworkThreadWork_DisconnectFromPS3();
#endif
}

#if MGS_VERSION == 2
void *NewTransfarringMenu(int parent_name)
{
   TransfarringWork *work ;

   work=(TransfarringWork *)GV_NewActor(GV_ACTOR_USER,sizeof(TransfarringWork));

   if(work!=NULL)
   {
      work->parent_name=parent_name;

      GV_SetActor(&(work->actor),Act,Die);
      GV_ActorEX(&(work->actor));
   }

   Init(work);

   return (void *)work ;
}
#endif

#if MGS_VERSION == 3
GV_HANDLE NewTransfarringMenu()
{
   GV_ACTOR_CREATE(
      GV_CLASS_OBJECT | GV_LEVEL( GV_ACTOR_MANAGER ) | GV_PRIO( 1 ),
      sizeof( TransfarringWork ),
      Act, Die, Init( (TransfarringWork *)work )
      );
}
#endif

//-------------------------------------------------------------------------------------------------
//State Functions
//-------------------------------------------------------------------------------------------------

//Main Menu Helpers
typedef void (*MainMenuButtonSelectionFunc)(TransfarringWork *work, EMainMenuOptions button);
typedef void (*MainMenuButtonCancelFunc)(TransfarringWork *work);

static void GeneralMainMenuRunFunc(TransfarringWork *work, MainMenuButtonSelectionFunc handleOk, MainMenuButtonCancelFunc handleCancel)
{
   if(!work->pCurrentAction)
   {
      if (A_Pad_Cancel(&work->pad_work))
      {
         PlayMenuCancelSound();
         if( handleCancel )
         {
            handleCancel(work);
         }
         else
         {
            ExitFromMainMenuState(work);
         }
      }
      else if (A_Pad_Down(&work->pad_work))
      {
         work->mGeneralCursor.y = (work->mGeneralCursor.y + 1) % 3;
         PlayAction(work, (EMenuAction)(kAction_CursorButton_0 + work->mGeneralCursor.y));
         PlayMenuSelectionSound();
      }
      else if (A_Pad_Up(&work->pad_work))
      {
         work->mGeneralCursor.y = (work->mGeneralCursor.y + 2) % 3;
         PlayAction(work, (EMenuAction)(kAction_CursorButton_0 + work->mGeneralCursor.y));
         PlayMenuSelectionSound();
      }
      else if (A_Pad_OK(&work->pad_work))
      {
         handleOk(work, (EMainMenuOptions)work->mGeneralCursor.y);
         PlayMenuOkSound();
      }
   }
}

//kTransfarState_Entering
static void EnteringStateRun(TransfarringWork *work)
{
   if (!work->pCurrentAction)
   {
      PopMenuState(work);
#if defined(BP_PS3)
      PushMenuState(work, kTransfarState_MainMenu_Disconnected_PS3);
#else
      PushMenuState(work, kTransfarState_MainMenu_Disconnected_VITA);
#endif
   }
}

static void EnteringStatePushed(TransfarringWork *work)
{
#ifdef BP_PS3
   SetDefaultExplanationId(work, kTString_TransfarringExplanation_Disconnected_PS3);
#else
   SetDefaultExplanationId(work, kTString_TransfarringExplanation_Disconnected_VITA);
#endif

   work->mGeneralCursor.x = 0;
   work->mGeneralCursor.y = 0;

   if(BP_Area_JP())
   {
      DisplayMenuObject(&gTransfarringMenuObjects[kObj_JpExpRoot]);
   }

   PlayAction(work, kAction_FadeIn);
}

//kTransfarState_MainMenu_Disconnected_PS3
static void MainMenuDisconnectedPS3StateHandleOk(TransfarringWork *work, EMainMenuOptions button)
{
#if BP_PS3
   switch(button)
   {
   case kMainMenu_Button_0:
      PushMenuState(work, kTransfarState_ManageSavesLoading);
      break;
   case kMainMenu_Button_1:
      {
         STransfarringError const error = Transfarring_PostNetworkThreadWork_OpenAccessPoint();
         if(error.mHighLevelError == kTE_Success)
         {
            PopMenuState(work);
            PushMenuState(work, kTransfarState_MainMenu_Listening_PS3);
         }
         else 
         {
            DisplaySystemDialogErrorMessage(error);
         }
         break;
      }
   case kMainMenu_Button_2:
      ExitFromMainMenuState(work);
      break;
   }
#endif
}

void MainMenuDisconnectedPS3UpdateJpExp(TransfarringWork *work)
{
   switch(work->mGeneralCursor.y)
   {
   case kMainMenu_Button_0:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_ManageSaves_jp_exp, 0);
      break;
   case kMainMenu_Button_1:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_ListenForVita_jp_exp, 0);
      break;
   case kMainMenu_Button_2:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_Cancel_jp_exp, 0);
      break;
   }
}

static void MainMenuDisconnectedPS3StateRun(TransfarringWork *work)
{
   int const prevCursor = work->mGeneralCursor.y;

   if(A_Pad_Tri(&work->pad_work))
   {
      ChangeExplanationPage(work);
   }
   GeneralMainMenuRunFunc(work, MainMenuDisconnectedPS3StateHandleOk, NULL);

   if(work->mGeneralCursor.y != prevCursor)
   {
      MainMenuDisconnectedPS3UpdateJpExp(work);
   }
}
static void MainMenuDisconnectedPS3StatePushed(TransfarringWork *work)
{
   DisplayMenuObject(&gTransfarringMenuObjects[kObj_MoreInfoRoot]);
   SetDefaultExplanationId(work, kTString_TransfarringExplanation_Disconnected_PS3);
   MainMenuDisconnectedPS3UpdateJpExp(work);
}
static void MainMenuDisconnectedPS3StatePopped(TransfarringWork *work)
{}

//kTransfarState_MainMenu_Listening_PS3
static void MainMenuListeningPS3StateHandleCancel(TransfarringWork *work)
{
#if BP_PS3
   // If posting fails here, it means some other work was running; the access point was still being opened
   STransfarringError postError = Transfarring_PostNetworkThreadWork_CloseAccessPoint();
   if( postError.mHighLevelError == kTE_Success )
   {
      PopMenuState(work);
      PushMenuState(work, kTransfarState_MainMenu_Disconnected_PS3);
   }
#endif
}

static void MainMenuListeningPS3StateHandleOk(TransfarringWork *work, EMainMenuOptions button)
{
   switch(button)
   {
   case kMainMenu_Button_0:
      break;
   case kMainMenu_Button_1:
      MainMenuListeningPS3StateHandleCancel(work);
      break;
   case kMainMenu_Button_2:
      break;
   }
}

void MainMenuListeningPS3UpdateJpExp(TransfarringWork *work)
{
   switch(work->mGeneralCursor.y)
   {
   case kMainMenu_Button_0:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_Empty, 0);
      break;
   case kMainMenu_Button_1:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_StopListeningForVita_jp_exp, 0);
      break;
   case kMainMenu_Button_2:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_Cancel_jp_exp, 0);
      break;
   }
}

static void MainMenuListeningPS3StateRun(TransfarringWork *work)
{
#ifdef BP_PS3
   int const prevCursor = work->mGeneralCursor.y;

   // Check for access point error
   if( Transfarring_IsTransfarringStatusError() )
   {
      DisplaySystemDialogErrorMessage(Transfarring_GetWifiTransfarringError());
      Transfarring_ClearTransfarringError();
      if( !Transfarring_IsAccessPointOpen() )
      {
         PopMenuState(work);
         PushMenuState(work, kTransfarState_MainMenu_Disconnected_PS3);
      }
   }
   else if(!work->pCurrentAction)
   {
      if(work->mAccessPointOpened)
      {
         if(Transfarring_IsConnectedToVita())
         {
            PopMenuState(work);
            PushMenuState(work,kTransfarState_MainMenu_Connected_PS3);
         }
         else
         {
            GeneralMainMenuRunFunc(work, MainMenuListeningPS3StateHandleOk, MainMenuListeningPS3StateHandleCancel);
         }
      }
      else if(Transfarring_GetTransfarringStatus() == kPTS_AwaitingConnection)
      {
         work->mAccessPointOpened = 1;
         work->mGeneralCursor.y = 1;
         PlayAction(work, kAction_DisconnectedToListening_PS3_EnableButtons);
      }
   }
   
   if(work->mGeneralCursor.y != prevCursor)
   {
      MainMenuListeningPS3UpdateJpExp(work);
   }
#endif
}
static void MainMenuListeningPS3StatePushed(TransfarringWork *work)
{
   work->mAccessPointOpened = 0;
   HideMenuObject(&gTransfarringMenuObjects[kObj_MoreInfoRoot]);
   SetDefaultExplanationId(work, kTString_TransfarringExplanation_Listening_PS3);
   MainMenuListeningPS3UpdateJpExp(work);
   PlayAction(work, kAction_DisconnectedToListening_PS3);
}
static void MainMenuListeningPS3StatePopped(TransfarringWork *work)
{
   work->mGeneralCursor.y = 1;
   work->mAccessPointOpened = 0;
   DisplayMenuObject(&gTransfarringMenuObjects[kObj_MoreInfoRoot]);
   SetDefaultExplanationId(work, kTString_TransfarringExplanation_Disconnected_PS3);
   PlayAction(work, kAction_ListeningToDisconnected_PS3);
}

//kTransfarState_MainMenu_Connected_PS3
static void MainMenuConnectedPS3StateHandleCancel(TransfarringWork *work)
{
#if BP_PS3
   // If posting fails here, it means some other work was running; the access point was still being opened
   STransfarringError postError = Transfarring_PostNetworkThreadWork_CloseAccessPoint();
   if( postError.mHighLevelError == kTE_Success )
   {
      SetDefaultExplanationId(work, kTString_TransfarringExplanation_Disconnected_PS3);
      PopMenuState(work);
      PlayAction(work, kAction_ConnectedToDisconnected_PS3);
      PushMenuState(work, kTransfarState_MainMenu_Disconnected_PS3);
   }
#endif
}

static void MainMenuConnectedPS3StateHandleOk(TransfarringWork *work, EMainMenuOptions button)
{
#ifdef BP_PS3
   switch(button)
   {
   case kMainMenu_Button_0:
      if(Transfarring_IsConnectedToVita())
      {
         PushMenuState(work, kTransfarState_WifiTransfarLoading);
      }
      break;
   case kMainMenu_Button_1:
      MainMenuConnectedPS3StateHandleCancel(work);
      break;
   case kMainMenu_Button_2:
      break;
   }
#endif
}

void MainMenuConnectedPS3UpdateJpExp(TransfarringWork *work)
{
   switch(work->mGeneralCursor.y)
   {
   case kMainMenu_Button_0:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_WifiTransfarring_jp_exp, 0);
      break;
   case kMainMenu_Button_1:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_StopListeningForVita_jp_exp, 0);
      break;
   case kMainMenu_Button_2:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_Cancel_jp_exp, 0);
      break;
   }
}

static void MainMenuConnectedPS3StateRun(TransfarringWork *work)
{
#ifdef BP_PS3
   int const prevCursor = work->mGeneralCursor.y;
   
   if(!work->pCurrentAction && !Transfarring_IsConnectedToVita())
   {
      SetDefaultExplanationId(work, kTString_TransfarringExplanation_Listening_PS3);
      PopMenuState(work);
      PushMenuState(work,kTransfarState_MainMenu_Listening_PS3);
   }
   else
   {
      GeneralMainMenuRunFunc(work, MainMenuConnectedPS3StateHandleOk, MainMenuConnectedPS3StateHandleCancel);
   }

   if(work->mGeneralCursor.y != prevCursor)
   {
      MainMenuConnectedPS3UpdateJpExp(work);
   }
#endif
}
static void MainMenuConnectedPS3StatePushed(TransfarringWork *work)
{
   work->mGeneralCursor.y = 0;

   HideMenuObject(&gTransfarringMenuObjects[kObj_MoreInfoRoot]);
   SetDefaultExplanationId(work, kTString_TransfarringExplanation_Connected_PS3);
   MainMenuConnectedPS3UpdateJpExp(work);
   PlayAction(work, kAction_ListeningToConnected_PS3);
}
static void MainMenuConnectedPS3StatePopped(TransfarringWork *work)
{
   DisplayMenuObject(&gTransfarringMenuObjects[kObj_MoreInfoRoot]);
}

//kTransfarState_MainMenu_Disconnected_VITA
#if defined(BP_VITA)
static void MainMenuDisconnectedVITAStateHandleOk(TransfarringWork *work, EMainMenuOptions button)
{
   if(!work->mIsConnecting)
   {
      switch(button)
      {
      case kMainMenu_Button_0:
            PushMenuState(work, kTransfarState_ManageSavesLoading);
         break;
      case kMainMenu_Button_1:
         if(!Transfarring_IsConnectedToPS3(work))
         {
            STransfarringError const error = Transfarring_PostNetworkThreadWork_ConnectToPS3();
            if(error.mHighLevelError != kTE_Success)
            {
               DisplaySystemDialogErrorMessage(error);
            }
            else
            {
               DisplayMenuObjectWithColor(&gTransfarringMenuObjects[kObj_Button_0_Text_Cloud], skGreyedOutTextColor);
               DisplayMenuObjectWithColor(&gTransfarringMenuObjects[kObj_Button_1_Text_Listen], skGreyedOutTextColor);
               DisplayMenuObjectWithColor(&gTransfarringMenuObjects[kObj_Button_2_Text], skGreyedOutTextColor);
               HideMenuObject(&gTransfarringMenuObjects[kObj_MoreInfoRoot]);
               work->mIsConnecting = 1;
            }
         }
         break;
      case kMainMenu_Button_2:
         ExitFromMainMenuState(work);
         break;
      }
   }
}
#endif

void MainMenuDisconnectedVITAUpdateJpExp(TransfarringWork *work)
{
   switch(work->mGeneralCursor.y)
   {
   case kMainMenu_Button_0:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_ManageSaves_jp_exp, 0);
      break;
   case kMainMenu_Button_1:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_ConnectToPs3_jp_exp, 0);
      break;
   case kMainMenu_Button_2:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_Cancel_jp_exp, 0);
      break;
   }
}

static void MainMenuDisconnectedVITAStateRun(TransfarringWork *work)
{
#if defined(BP_VITA)
   int const prevCursor = work->mGeneralCursor.y;
   
   if(A_Pad_Tri(&work->pad_work))
   {
      ChangeExplanationPage(work);
   }
   if(work->mIsConnecting)
   {
      // Wait for network thread to either finish or error out
      if( Transfarring_IsTransfarringStatusError() )
      {
         STransfarringError const error = Transfarring_GetWifiTransfarringError();
         DisplaySystemDialogErrorMessage(error);
         Transfarring_ClearTransfarringError();

         DisplayMenuObjectWithColor(&gTransfarringMenuObjects[kObj_Button_0_Text_Cloud], skTextColor);
         DisplayMenuObjectWithColor(&gTransfarringMenuObjects[kObj_Button_1_Text_Listen], skTextColor);
         DisplayMenuObjectWithColor(&gTransfarringMenuObjects[kObj_Button_2_Text], skTextColor);
         DisplayMenuObject(&gTransfarringMenuObjects[kObj_MoreInfoRoot]);

         work->mIsConnecting = 0;
      }
      else if( Transfarring_IsTransfarringStatusIdle() )
      {
         if(!work->pCurrentAction && Transfarring_IsConnectedToPS3(work))
         {
            PopMenuState(work);
            PushMenuState(work, kTransfarState_MainMenu_Connected_VITA);
            work->mIsConnecting = 0;
         }
      }
   }
   else
   {
      GeneralMainMenuRunFunc(work, MainMenuDisconnectedVITAStateHandleOk, NULL);
   }
   
   if(work->mGeneralCursor.y != prevCursor)
   {
      MainMenuDisconnectedVITAUpdateJpExp(work);
   }
#endif
}
static void MainMenuDisconnectedVITAStatePushed(TransfarringWork *work)
{
   DisplayMenuObject(&gTransfarringMenuObjects[kObj_MoreInfoRoot]);
   SetDefaultExplanationId(work, kTString_TransfarringExplanation_Disconnected_VITA);
   MainMenuDisconnectedVITAUpdateJpExp(work);
}

static void MainMenuDisconnectedVITAStatePopped(TransfarringWork *work)
{}

//kTransfarState_MainMenu_Connected_VITA
#if defined(BP_VITA)
static void MainMenuConnectedVITAStateHandleCancel(TransfarringWork *work)
{
   if(Transfarring_IsConnectedToPS3()
      && Transfarring_GetTransfarringStatus() == kVTS_Idle)
   {
      Transfarring_PostNetworkThreadWork_DisconnectFromPS3();
   }
}

static void MainMenuConnectedVITAStateHandleOk(TransfarringWork *work, EMainMenuOptions button)
{
   switch(button)
   {
   case kMainMenu_Button_0:
      BP_CommonDialog_WantsMessageDialog(GetTransfarringString(kTString_VitaWifiPrompt), kMDL_OK);
      break;
   case kMainMenu_Button_1:
      MainMenuConnectedVITAStateHandleCancel(work);
      break;
   case kMainMenu_Button_2:
      break;
   }
}
#endif

void MainMenuConnectedVITAUpdateJpExp(TransfarringWork *work)
{
   switch(work->mGeneralCursor.y)
   {
   case kMainMenu_Button_0:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_Empty, 0);
      break;
   case kMainMenu_Button_1:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_DisconnectFromPs3_jp_exp, 0);
      break;
   case kMainMenu_Button_2:
      SetMenuObjectText(&gTransfarringMenuObjects[kObj_JpExpText], kTString_Cancel_jp_exp, 0);
      break;
   }
}

static void MainMenuConnectedVITAStateRun(TransfarringWork *work)
{
#if defined(BP_VITA)
   int const prevCursor = work->mGeneralCursor.y;
   
   if( Transfarring_IsTransfarringStatusError() )
   {
      STransfarringError const error = Transfarring_GetWifiTransfarringError();
      DisplaySystemDialogErrorMessage(error);
      Transfarring_ClearTransfarringError();
   }
   else if(!work->pCurrentAction && !Transfarring_IsConnectedToPS3())
   {
      PopMenuState(work);
      PushMenuState(work, kTransfarState_MainMenu_Disconnected_VITA);
   }
   else
   {
      SMenuObject *const disconnectObj = &gTransfarringMenuObjects[kObj_Button_1_Text_Stop];
      if(Transfarring_GetTransfarringStatus() == kVTS_Idle)
      {
         DisplayMenuObjectWithColor(disconnectObj, skTextColor);
      }
      else
      {
         DisplayMenuObjectWithColor(disconnectObj, skGreyedOutTextColor);
      }
      GeneralMainMenuRunFunc(work, MainMenuConnectedVITAStateHandleOk, MainMenuConnectedVITAStateHandleCancel);
   }

   if(work->mGeneralCursor.y != prevCursor)
   {
     MainMenuConnectedVITAUpdateJpExp(work);
   }
#endif
}
static void MainMenuConnectedVITAStatePushed(TransfarringWork *work)
{
   HideMenuObject(&gTransfarringMenuObjects[kObj_MoreInfoRoot]);
   SetDefaultExplanationId(work, kTString_TransfarringExplanation_Connected_VITA);
   MainMenuConnectedVITAUpdateJpExp(work);
   PlayAction(work, kAction_DisconnectedToConnected_VITA);
}
static void MainMenuConnectedVITAStatePopped(TransfarringWork *work)
{
   DisplayMenuObject(&gTransfarringMenuObjects[kObj_MoreInfoRoot]);
   SetDefaultExplanationId(work, kTString_TransfarringExplanation_Disconnected_VITA);
   PlayAction(work, kAction_ConnectedToDisconnected_VITA);
}

//kTransfarState_Leaving
static void LeavingStateRun(TransfarringWork *work)
{
   if( Transfarring_IsTransfarringStatusError() )
   {
      STransfarringError const error = Transfarring_GetWifiTransfarringError();
      DisplaySystemDialogErrorMessage(error);
      Transfarring_ClearTransfarringError();
   }
   else if( Transfarring_IsTransfarringStatusIdle() )
   {
      if (!work->pCurrentAction)
      {
#if MGS_VERSION == 2
         work->ans = TITLE_MSG_MODECANCEL;
         GV_DestroyActor(work);
#endif
#if MGS_VERSION == 3
         // communicate back to the main menu
         gTransfarringMenuDone = TRUE;
         GV_actorDestroy(work);
#endif
      }
   }
}

static void LeavingStatePushed(TransfarringWork *work)
{
   Transfarring_PostNetworkThreadWork_CleanupSystem();
   PlayAction(work, kAction_FadeOut);
}

//kTransfarState_WifiTransfar
static void WifiTransfarStateRun(TransfarringWork *work)
{
#if defined(BP_PS3)   
   if(work->mExitCurrentState)
   {
      PopMenuState(work);
   }
   else if(!Transfarring_IsConnectedToVita())
   {
      work->mExitCurrentState = 1;
      DisplayModalMessage(work, kTString_ConnectionLostMessage, 0);
   }
   else if (A_Pad_Cancel(&work->pad_work))
   {
      PopMenuState(work);
      PlayMenuCancelSound();
   }
   else if (A_Pad_Down(&work->pad_work))
   {
      UpdateSaveTableCursor(work, 0, 1, kPlayAction_Yes);
   }
   else if (A_Pad_Up(&work->pad_work))
   {
      UpdateSaveTableCursor(work, 0, -1, kPlayAction_Yes);
   }
   else if (A_Pad_Left(&work->pad_work))
   {
      UpdateSaveTableCursor(work, -1, 0, kPlayAction_Yes);
   }
   else if(A_Pad_Right(&work->pad_work))
   {
      UpdateSaveTableCursor(work, 1, 0, kPlayAction_Yes);
   }
   else if (A_Pad_OK(&work->pad_work))
   {
      UpdateButtonOptions(work, &work->mSaveTableCursor);
      if(work->mTypeButtonX != kType_None)
      {
         PlayMenuOkSound();
         work->mTransfarringType = work->mTypeButtonX;
         PushMenuState(work, kTransfarState_WifiTransfar_YesNo);
      }
   }
   else if (A_Pad_Sq(&work->pad_work))
   {
      UpdateButtonOptions(work, &work->mSaveTableCursor);
      if(work->mTypeButtonSq != kType_None)
      {
         PlayMenuOkSound();
         work->mTransfarringType = work->mTypeButtonSq;
         PushMenuState(work, kTransfarState_WifiTransfar_YesNo);
      }
   }
#if MGS_VERSION == 2
   else if(Transfarring_IsFileDataValid(work))
   {
      if(A_Pad_L1(&work->pad_work))
      {
         PlayMenuOkSound();
         switch(work->mCurrentSaveType)
         {
         case kST_Game:
            SetSaveGameType(work, kST_VR);
            break;
         case kST_SnakeTales:
            SetSaveGameType(work, kST_Game);
            break;
         case kST_VR:
            SetSaveGameType(work, kST_SnakeTales);
            break;
         default:
            break;
         }
      }
      else if(A_Pad_R1(&work->pad_work))
      {
         PlayMenuOkSound();
         switch(work->mCurrentSaveType)
         {
         case kST_Game:
            SetSaveGameType(work, kST_SnakeTales);
            break;
         case kST_SnakeTales:
            SetSaveGameType(work, kST_VR);
            break;
         case kST_VR:
            SetSaveGameType(work, kST_Game);
            break;
         default:
            break;
         }
      }
   }
#endif
#endif
}

static void WifiTransfarStatePushed(TransfarringWork* work)
{
   SetSaveGameType(work, kST_Game);
}

static void WifiTransfarStatePopped(TransfarringWork* work)
{
   work->mSaveTableType = kSaveTable_None;
   work->mGeneralCursor.x = 0;
   work->mGeneralCursor.y = 0;
   PlayAction(work, kAction_LeaveWifiTransfarState);
}

//kTransfarState_WifiTransfarLoading
static void WifiTransfarLoadingStateRun(struct STransfarringWork *work)
{
   BlinkLoadingBar( work );

   if( work->mExitCurrentState )
   {
      PopMenuState( work );
   }
   else if( Transfarring_IsTransfarringStatusError() )
   {
      STransfarringError const error = Transfarring_GetWifiTransfarringError();
      
      work->mExitCurrentState = 1;

      if( error.mHighLevelError == kTE_Unknown )
      {
         DisplaySystemDialogErrorMessage( error );
      }
      else
      {
         DisplayModalMessage( work, GetTransfarringStringIdForErrorCode( error.mHighLevelError ), 0 );
      }
      Transfarring_ClearTransfarringError();
   }
   else if( Transfarring_IsTransfarringStatusIdle() && Transfarring_IsLocalFileListValid() )
   {
      PopMenuState( work );
      PushMenuState( work, kTransfarState_WifiTransfar );
   }
}
static void WifiTransfarLoadingStatePushed(struct STransfarringWork *work)
{
   Transfarring_PostNetworkThreadWork_LoadFileList();

   work->mSaveTableType = kSaveTable_Wifi;
   work->mExitCurrentState = 0;

   SetSaveGameType(work, kST_Game);

   DisplayButtonOptions(0, 0, kTString_Empty, kTString_Empty);

   SetMenuObjectText(&gTransfarringMenuObjects[kObj_WifiTransfar_LabelPS3], kTString_PS3, 0);
   SetMenuObjectText(&gTransfarringMenuObjects[kObj_WifiTransfar_LabelVita], kTString_VITA, 0);
 
   ShowLoadingBar(work);
   DisplayStatusMessageById(work, kTString_Loading);
   PlayAction(work, kAction_EnterWifiTransfarState);
}

static void WifiTransfarLoadingStatePopped(struct STransfarringWork *work)
{
   HideLoadingBar(work);
   DisplayStatusMessageById(work, kTString_Empty);
}

//kTransfarState_WifiTransfar_YesNo
static void WifiTransfarYesNoStateRun(TransfarringWork *work)
{
#ifdef BP_PS3
   if (!Transfarring_IsConnectedToVita() || A_Pad_Cancel(&work->pad_work))
   {
      PopMenuState(work);
      PlayMenuCancelSound();
   }
   else if (A_Pad_Down(&work->pad_work) || A_Pad_Up(&work->pad_work))
   {
      work->mGeneralCursor.y = 1 - work->mGeneralCursor.y;
      PlayAction(work, (EMenuAction)(kAction_WifiTransfar_SelectYes + work->mGeneralCursor.y));
      PlayMenuSelectionSound();
   }
   else if (A_Pad_OK(&work->pad_work))
   {
      if(work->mGeneralCursor.y)
      {
         PopMenuState(work);
      }
      else if(work->mTransfarringType != kType_None)
      {
         int pushNewState = 0;
         STransfarringError error;

         switch(work->mTransfarringType)
         {
         case kType_ToVita:
#ifdef TRANSFARRING_WARN_LEGACY
            {
               //Redirect to kTransfarState_CheckTrophiesDisabled.
               PushMenuState(work, kTransfarState_CheckTrophiesDisabled);
               pushNewState = 0;
            }
            break;
#endif
         case kType_ToVitaOrphaned:
         case kType_ToVitaLinked:
            {
               TransfarringSaveUiData *save = GetSaveForCursorPosition(work, 0, work->mSaveTableCursor.y);
               if(save)
               {
                  error = Transfarring_PostNetworkThreadWork_LockLocalSaveFile(work->mCurrentSaveType, save->mSaveIndex);
                  pushNewState = 1;
               }
               break;
            }
         case kType_FromVita:
         case kType_FromVitaOrphaned:
         case kType_FromVitaLinked:            
            {
               TransfarringSaveUiData *save = GetSaveForCursorPosition(work, 1, work->mSaveTableCursor.y);
               if(save)
               {
                  error = Transfarring_PostNetworkThreadWork_ReceiveSaveFile(work->mCurrentSaveType, save->mSaveIndex);
                  pushNewState = 1;
               }
               break;
            }
         case kType_Unlock:
         case kType_Unlock_Vita:
            {
               TransfarringSaveUiData *save = GetSaveForCursorPosition(work, 0, work->mSaveTableCursor.y);
               if(save)
               {
                  error = Transfarring_PostWork_UnlockSaveFile(work->mCurrentSaveType, save->mSaveIndex);
                  pushNewState = 1;
               }
               break;
            }
         case kType_Unlink:
         case kType_Unlink_Vita:
            {
               TransfarringSaveUiData *save = GetSaveForCursorPosition(work, 1, work->mSaveTableCursor.y);
               if(save)
               {
                  error = Transfarring_PostNetworkThreadWork_UnlinkRemoteSaveFile(work->mCurrentSaveType, save->mSaveIndex);
                  pushNewState = 1;
               }
               break;
            }
         default:
            ASSERT(0);
            break;
         }

         if(pushNewState)
         {
            if(error.mHighLevelError == kTE_Success)
            {
               PushMenuState(work, kTransfarState_WifiTransfar_Transfarring);
            }
            else if(error.mHighLevelError == kTE_Unknown)
            {
               PopMenuState(work);
               DisplaySystemDialogErrorMessage(error);
            }
            else
            {
               PopMenuState(work);
               DisplayModalMessage(work, GetTransfarringStringIdForErrorCode(error.mHighLevelError), 0);
            }
         }
      }
   }
#endif
}

//kTransfarState_WifiTransfar_Transfarring
static void WifiTransfarTransfarringStateRun(TransfarringWork *work)
{
#ifdef BP_PS3
   if(!Transfarring_IsConnectedToVita() || work->mTransfarringType == kType_None)
   {
      PopMenuState(work);
      PopMenuState(work);
   }
   else 
   {
      UpdateLoadingBar(work);
      {
         EPS3TransfarringStatus const status = (EPS3TransfarringStatus) Transfarring_GetTransfarringStatus();
         if(status == kPTS_Idle)
         {
            switch(work->mTransfarringType)
            {
            case kType_ToVita:
               DisplayModalMessage(work, kTString_Success_TransfarrToVita, 0);
               break;
            case kType_ToVitaOrphaned:
               DisplayModalMessage(work, kTString_Success_TransfarrToVitaOrphaned, 0);
               break;
            case kType_ToVitaLinked:
               DisplayModalMessage(work, kTString_Success_TransfarrToVitaLinked, 0);
               break;
            case kType_FromVita:
               DisplayModalMessage(work, kTString_Success_TransfarrFromVita, 0);
               break;
            case kType_FromVitaOrphaned:
               DisplayModalMessage(work, kTString_Success_TransfarrFromVitaOrphaned, 0);
               break;
            case kType_FromVitaLinked:
               DisplayModalMessage(work, kTString_Success_TransfarrFromVitaLinked, 0);
               break;
            default:
               ASSERT(0);
               break;
            }
            work->mTransfarringType = kType_None;
         }
         else if(status == kPTS_Error)
         {
            STransfarringError const error = Transfarring_GetWifiTransfarringError();

            if(error.mHighLevelError == kTE_Unknown)
            {
               DisplaySystemDialogErrorMessage(error);
            }
            else
            {
               DisplayModalMessage(work, GetTransfarringStringIdForErrorCode(error.mHighLevelError), 0);
            }
            Transfarring_ClearTransfarringError();
            work->mTransfarringType = kType_None;
         }
      }
   }
#endif
}

static void WifiTransfarTransfarringStatePushed(TransfarringWork *work)
{
   PlayAction(work, kAction_WifiTransfar_HideYesNo);
   
   switch(work->mTransfarringType)
   {
      case kType_ToVita:
         DisplayStatusMessageById(work, kTString_InProgress_TransfarrToVita);
         break;
      case kType_ToVitaOrphaned:
         DisplayStatusMessageById(work, kTString_InProgress_TransfarrToVitaOrphaned);
         break;
      case kType_ToVitaLinked:
         DisplayStatusMessageById(work, kTString_InProgress_TransfarrToVitaLinked);
         break;
      case kType_FromVita:
         DisplayStatusMessageById(work, kTString_InProgress_TransfarrFromVita);
         break;
      case kType_FromVitaOrphaned:
         DisplayStatusMessageById(work, kTString_InProgress_TransfarrFromVitaOrphaned);
         break;
      case kType_FromVitaLinked:
         DisplayStatusMessageById(work, kTString_InProgress_TransfarrFromVitaLinked);
         break;
      case kType_Cloudify:
         DisplayStatusMessageById(work, kTString_InProgress_Cloudify);
         break;
      case kType_Delete:
         DisplayStatusMessageById(work, kTString_InProgress_Delete);
         break;
      case kType_UncloudifyLinked:
         DisplayStatusMessageById(work, kTString_InProgress_UncloudifyLinked);
         break;
      case kType_UncloudifyOrphanedLocal:
         DisplayStatusMessageById(work, kTString_InProgress_UncloudifyOrphanedLocal);
         break;
      case kType_UncloudifyOrphanedCloud:
         DisplayStatusMessageById(work, kTString_InProgress_UncloudifyOrphanedCloud);
         break;
      case kType_Unlock:
         DisplayStatusMessageById(work, kTString_InProgress_Unlock);
         break;
      case kType_Unlock_Vita:
         DisplayStatusMessageById(work, kTString_InProgress_Unlock_Vita);
         break;
      case kType_Unlink:
         DisplayStatusMessageById(work, kTString_InProgress_Unlink);
         break;
      case kType_Unlink_Vita:
         DisplayStatusMessageById(work, kTString_InProgress_Unlink_Vita);
         break;
      case kType_SolveConflict:
         DisplayStatusMessageById(work, kTString_InProgress_SolveConflict);
         break;
      default:
         ASSERT(0);
         break;
   }
   ShowLoadingBar(work);
}

static void WifiTransfarTransfarringStatePopped(TransfarringWork *work)
{
   HideLoadingBar(work);
   LoadSaveGameTableData(work);
}

//kTransfarState_ModalMessage
static void ModalMessageStateRun(struct STransfarringWork *work)
{
   if(A_Pad_Cancel(&work->pad_work) || A_Pad_OK(&work->pad_work)
      || (sFakeTick - work->mModalStateStartTick > MODAL_MESSAGE_DURATION_IN_UPDATE_TICKS))
   {
      PopMenuState(work);
      PlayMenuOkSound();
   }
}

static void ModalMessageStatePushed(struct STransfarringWork *work)
{
   work->mModalStateStartTick = sFakeTick;

   if(work->mModalMessageId == kTString_USE_OVERRIDE)
   {
      DisplayStatusMessage(work, work->mModalMessageOverride);
   }
   else
   {
      DisplayStatusMessageById(work, work->mModalMessageId);
   }
}

static void ModalMessageStatePopped(struct STransfarringWork *work)
{
   DisplayStatusMessageById(work, kTString_Empty);
}

//kTransfarState_ManageSaves
static void ManageSavesStateRun(TransfarringWork *work)
{
   int const connectedToPsn = BP_Network_IsSignedIn();

   if(connectedToPsn != work->mWasConnectedToPsn
#if defined(BP_VITA)
      || work->mSystemResumeCount != Transfarring_GetSystemResumeCount()
#endif
   )
   {
      work->mWasConnectedToPsn = connectedToPsn;

      PopMenuState(work);
      PushMenuState(work, kTransfarState_ManageSavesLoading);
   }
   else if(work->mExitCurrentState)
   {
      PopMenuState(work);
   }
   else if(!Transfarring_IsFileDataValid(work))
   {
      work->mExitCurrentState = 1;
      DisplayModalMessage(work, kTString_ConnectionLostMessage, 0);
   }
   else if (A_Pad_Cancel(&work->pad_work))
   {
      PopMenuState(work);
      PlayMenuCancelSound();
   }
   else if (A_Pad_Down(&work->pad_work))
   {
      UpdateSaveTableCursor(work, 0, 1, kPlayAction_Yes);
   }
   else if (A_Pad_Up(&work->pad_work))
   {
      UpdateSaveTableCursor(work, 0, -1, kPlayAction_Yes);
   }
   else if (A_Pad_Left(&work->pad_work))
   {
      UpdateSaveTableCursor(work, -1, 0, kPlayAction_Yes);
   }
   else if(A_Pad_Right(&work->pad_work))
   {
      UpdateSaveTableCursor(work, 1, 0, kPlayAction_Yes);
   }
   else if (A_Pad_OK(&work->pad_work))
   {
      UpdateButtonOptions(work, &work->mSaveTableCursor);
      if(work->mTypeButtonX != kType_None)
      {
         PlayMenuOkSound();
         work->mTransfarringType = work->mTypeButtonX;
         PushMenuState(work, kTransfarState_ManageSaves_YesNo);
      }
   }
   else if (A_Pad_Sq(&work->pad_work))
   {
      UpdateButtonOptions(work, &work->mSaveTableCursor);
      if(work->mTypeButtonSq != kType_None)
      {
         PlayMenuOkSound();
         work->mTransfarringType = work->mTypeButtonSq;
         PushMenuState(work, kTransfarState_ManageSaves_YesNo);
      }
   }
   else if(A_Pad_Tri(&work->pad_work) && !connectedToPsn)
   {
      BP_TUS_WantsSignin();
   }
#if MGS_VERSION == 2
   else if(Transfarring_IsFileDataValid(work))
   {
      if(A_Pad_L1(&work->pad_work))
      {
         PlayMenuOkSound();
         switch(work->mCurrentSaveType)
         {
         case kST_Game:
            SetSaveGameType(work, kST_VR);
            break;
         case kST_SnakeTales:
            SetSaveGameType(work, kST_Game);
            break;
         case kST_VR:
            SetSaveGameType(work, kST_SnakeTales);
            break;
         default:
            break;
         }
      }
      else if(A_Pad_R1(&work->pad_work))
      {
         PlayMenuOkSound();
         switch(work->mCurrentSaveType)
         {
         case kST_Game:
            SetSaveGameType(work, kST_SnakeTales);
            break;
         case kST_SnakeTales:
            SetSaveGameType(work, kST_VR);
            break;
         case kST_VR:
            SetSaveGameType(work, kST_Game);
            break;
         default:
            break;
         }
      }
   }
#endif
}

static void TransitionBackToMainMenu(TransfarringWork* work)
{
   work->mSaveTableType = kSaveTable_None;
   work->mGeneralCursor.x = 0;
   work->mGeneralCursor.y = 0;

   HideMenuObject(&gTransfarringMenuObjects[kObj_WifiTransfar_PsnConnectionStatus_Root]);
   PlayAction(work, kAction_LeaveManageSavesState);
}

static void ManageSavesStatePushed(TransfarringWork* work)
{
#if defined(BP_VITA)
   work->mSystemResumeCount = Transfarring_GetSystemResumeCount();
#endif
   DisplayMenuObject(&gTransfarringMenuObjects[kObj_WifiTransfar_PsnConnectionStatus_Root]);
   SetSaveGameType(work, kST_Game);
}

static void ManageSavesStatePopped(TransfarringWork* work)
{
   TransitionBackToMainMenu(work);
}

//kTransfarState_ManageSavesLoading
static void ManageSavesLoadingStateRun(struct STransfarringWork *work)
{
   BlinkLoadingBar( work );
   
   if( work->mExitCurrentState )
   {
      PopMenuState( work );
      TransitionBackToMainMenu( work );
   }
   else if( Transfarring_IsTransfarringStatusError() )
   {
      STransfarringError const error = Transfarring_GetWifiTransfarringError();

      work->mExitCurrentState = 1;

      if(error.mHighLevelError == kTE_Unknown)
      {
         DisplaySystemDialogErrorMessage(error);
      }
      else
      {
         DisplayModalMessage(work, GetTransfarringStringIdForErrorCode(error.mHighLevelError), 0);
      }

      Transfarring_ClearTransfarringError();
   }
   else if( Transfarring_IsTransfarringStatusIdle() && work->mManageSavesLoadingStep == kLoadingFilesStep )
   {
      if( BP_Network_IsSignedIn() )
      {
         STransfarringError const error = Transfarring_PostWork_SyncCloudFiles( NULL, 1, 0 );
         work->mManageSavesLoadingStep = kSynchingFilesStep;

         if( error.mHighLevelError != kTE_Success )
         {
            work->mManageSavesLoadingStep = kDoneStep;

            if( error.mHighLevelError == kTE_Unknown )
            {
               DisplaySystemDialogErrorMessage( error );
            }
            else
            {
               DisplayModalMessage( work, GetTransfarringStringIdForErrorCode( error.mHighLevelError ), 0 );
            }
         }
      }
      else
      {
         work->mManageSavesLoadingStep = kDoneStep;
      }
   }
   else if( work->mManageSavesLoadingStep == kSynchingFilesStep )
   {
      STransfarringError const error = Transfarring_ProcessWork();

      if(error.mHighLevelError == kTE_Success)
      {
         work->mManageSavesLoadingStep = kDoneStep;
      }
      else if(error.mHighLevelError != kTE_Processing)
      {
         work->mManageSavesLoadingStep = kDoneStep;
         
         if( error.mHighLevelError == kTE_Unknown )
         {
            DisplaySystemDialogErrorMessage( error );
         }
         else
         {
            DisplayModalMessage( work, GetTransfarringStringIdForErrorCode( error.mHighLevelError ), 0 );
         }
      }
   }
   else if( work->mManageSavesLoadingStep == kDoneStep )
   {
      PopMenuState(work);
      PushMenuState(work, kTransfarState_ManageSaves);
   }
}
static void ManageSavesLoadingStatePushed(struct STransfarringWork *work)
{
   SMenuObject *connectedObj = &gTransfarringMenuObjects[kObj_WifiTransfar_PsnConnectionStatus_Connected];
   SMenuObject *disconnectedObj = &gTransfarringMenuObjects[kObj_WifiTransfar_PsnConnectionStatus_Disconnected];

   work->mManageSavesLoadingStep = kLoadingFilesStep;

   work->mSaveTableType = kSaveTable_Cloud;
   work->mExitCurrentState = 0;
   work->mWasConnectedToPsn = BP_Network_IsSignedIn();

   SetSaveGameType(work, kST_Game);

   DisplayButtonOptions(0, 0, kTString_Empty, kTString_Empty);

   DisplayStatusMessageById(work, kTString_Loading);

   ShowLoadingBar(work);

#ifdef BP_PS3
   SetMenuObjectText(&gTransfarringMenuObjects[kObj_WifiTransfar_LabelPS3], kTString_PS3, 0);
#else
   SetMenuObjectText(&gTransfarringMenuObjects[kObj_WifiTransfar_LabelPS3], kTString_VITA, 0);
#endif

   if(BP_Network_IsSignedIn())
   {
      DisplayMenuObjectWithColor(connectedObj, skPsnStatusColor);
      HideMenuObject(disconnectedObj);

      SetMenuObjectText(&gTransfarringMenuObjects[kObj_WifiTransfar_LabelVita], kTString_Cloud, 0);
   }
   else
   {
      HideMenuObject(connectedObj);
      DisplayMenuObjectWithColor(disconnectedObj, skPsnStatusColor);

      SetMenuObjectText(&gTransfarringMenuObjects[kObj_WifiTransfar_LabelVita], kTString_Cloud_NotConnected, 0);
   }

   PlayAction(work, kAction_EnterManageSavesState);

   {
      STransfarringError error = Transfarring_PostNetworkThreadWork_LoadFileList();

      if(error.mHighLevelError != kTE_Success)
      {
         work->mExitCurrentState = 1;
         DisplaySystemDialogErrorMessage(error);
         Transfarring_ClearTransfarringError();
      }
   }
}

static void ManageSavesLoadingStatePopped(struct STransfarringWork *work)
{
   HideLoadingBar(work);
   DisplayStatusMessageById(work, kTString_Empty);
}

//kTransfarState_ManageSaves_YesNo
static void ManageSavesYesNoStateRun(TransfarringWork *work)
{
   if(!Transfarring_IsFileDataValid(work) || A_Pad_Cancel(&work->pad_work))
   {
      PopMenuState(work);
      PlayMenuCancelSound();
   }
   else if(A_Pad_Down(&work->pad_work) || A_Pad_Up(&work->pad_work))
   {
      work->mGeneralCursor.y = 1 - work->mGeneralCursor.y;
      PlayAction(work, (EMenuAction)(kAction_WifiTransfar_SelectYes + work->mGeneralCursor.y));
      PlayMenuSelectionSound();
   }
   else if(A_Pad_OK(&work->pad_work))
   {
      PlayMenuOkSound();
      if(work->mGeneralCursor.y)
      {
         PopMenuState(work);
      }
      else if(work->mTransfarringType != kType_None)
      {
         TransfarringSaveUiData *save = GetSaveForCursor(work, &work->mSaveTableCursor);

         if(save)
         {
            STransfarringError error;

            switch(work->mTransfarringType)
            {
            case kType_Cloudify:
#ifdef TRANSFARRING_WARN_LEGACY
               //Redirect to kTransfarState_CheckTrophiesDisabled
               PushMenuState(work, kTransfarState_CheckTrophiesDisabled);
#else
               error = Transfarring_PostWork_FlagCloudFile(work->mCurrentSaveType, save->mSaveIndex);
#endif
               break;
              
#ifdef BP_VITA
            case kType_Delete:
               error = Transfarring_PostWork_DeleteSaveFile(NULL, work->mCurrentSaveType, save->mSaveIndex);
               break;
#endif
            case kType_UncloudifyLinked:
            case kType_UncloudifyOrphanedLocal:
            case kType_UncloudifyOrphanedCloud:
               error = Transfarring_PostWork_UnflagCloudFile((unsigned char*)save->mTransfarringID);
               break;
            case kType_Unlock:
            case kType_Unlock_Vita:
            case kType_Unlink:
            case kType_Unlink_Vita:
               error = Transfarring_PostWork_UnlockSaveFile(work->mCurrentSaveType, save->mSaveIndex);
               break;
            case kType_SolveConflict:
               PushMenuState(work, kTransfarState_ManageSaves_SolveConflict);
               break;
            default:
               ASSERT(0);
               error.mHighLevelError = kTE_Unknown;
               break;
            }
#ifdef TRANSFARRING_WARN_LEGACY
            if(work->mTransfarringType != kType_SolveConflict && work->mTransfarringType != kType_Cloudify)
#else
            if(work->mTransfarringType != kType_SolveConflict)
#endif
            {
               if(error.mHighLevelError == kTE_Success)
               {
                  PushMenuState(work, kTransfarState_ManageSaves_Transfarring);
               }
               else if(error.mHighLevelError == kTE_Unknown)
               {
                  PopMenuState(work);
                  DisplaySystemDialogErrorMessage(error);
               }
               else
               {
                  PopMenuState(work);
                  DisplayModalMessage(work, GetTransfarringStringIdForErrorCode(error.mHighLevelError), 0);
               }
            }
         }
      }
   }
}

//kTransfarState_ManageSaves_Transfarring
static void ManageSavesTransfarringStateRun(TransfarringWork *work)
{
   if(!Transfarring_IsFileDataValid(work) || work->mTransfarringType == kType_None)
   {
      PopMenuState(work);
      PopMenuState(work);
   }
   else
   {
      STransfarringError const error = Transfarring_ProcessWork();

      UpdateLoadingBar(work);

      if(error.mHighLevelError == kTE_Success)
      {
         switch(work->mTransfarringType)
         {
         case kType_Cloudify:
            DisplayModalMessage(work, kTString_Success_Cloudify, 0);
            break;
         case kType_Unlock:
            DisplayModalMessage(work, kTString_Success_Unlock, 0);
            break;
         case kType_Unlock_Vita:
            DisplayModalMessage(work, kTString_Success_Unlock_Vita, 0);
            break;
         case kType_Unlink:
            DisplayModalMessage(work, kTString_Success_Unlink, 0);
            break;
         case kType_Unlink_Vita:
            DisplayModalMessage(work, kTString_Success_Unlink_Vita, 0);
            break;
         case kType_Delete:
            DisplayModalMessage(work, kTString_Success_Delete, 0);
            break;
         case kType_UncloudifyLinked:
            DisplayModalMessage(work, kTString_Success_UncloudifyLinked, 0);
            break;
         case kType_UncloudifyOrphanedLocal:
            DisplayModalMessage(work, kTString_Success_UncloudifyOrphanedLocal, 0);
            break;
         case kType_UncloudifyOrphanedCloud:
            DisplayModalMessage(work, kTString_Success_UncloudifyOrphanedCloud, 0);
            break;
         case kType_SolveConflict:
            DisplayModalMessage(work, kTString_Success_SolveConflict, 0);
            break;
         default:
            ASSERT(0);
            break;
         }
         work->mTransfarringType = kType_None;
      }
      else if(error.mHighLevelError != kTE_Processing)
      {
         if(error.mHighLevelError == kTE_Unknown)
         {
            DisplaySystemDialogErrorMessage(error);
         }
         else
         {
            DisplayModalMessage(work, GetTransfarringStringIdForErrorCode(error.mHighLevelError), 0);
         }
         work->mTransfarringType = kType_None;
      }
   }
}

static void ManageSavesTransfarringStatePushed(TransfarringWork *work)
{
   PlayAction(work, kAction_WifiTransfar_HideYesNo);
   switch(work->mTransfarringType)
   {
   case kType_ToVita:
      DisplayStatusMessageById(work, kTString_InProgress_TransfarrToVita);
      break;
   case kType_ToVitaOrphaned:
      DisplayStatusMessageById(work, kTString_InProgress_TransfarrToVitaOrphaned);
      break;
   case kType_ToVitaLinked:
      DisplayStatusMessageById(work, kTString_InProgress_TransfarrToVitaLinked);
      break;
   case kType_FromVita:
      DisplayStatusMessageById(work, kTString_InProgress_TransfarrFromVita);
      break;
   case kType_FromVitaOrphaned:
      DisplayStatusMessageById(work, kTString_InProgress_TransfarrFromVitaOrphaned);
      break;
   case kType_FromVitaLinked:
      DisplayStatusMessageById(work, kTString_InProgress_TransfarrFromVitaLinked);
      break;
   case kType_Cloudify:
      DisplayStatusMessageById(work, kTString_InProgress_Cloudify);
      break;
   case kType_Delete:
      DisplayStatusMessageById(work, kTString_InProgress_Delete);
      break;
   case kType_UncloudifyLinked:
      DisplayStatusMessageById(work, kTString_InProgress_UncloudifyLinked);
      break;
   case kType_UncloudifyOrphanedLocal:
      DisplayStatusMessageById(work, kTString_InProgress_UncloudifyOrphanedLocal);
      break;
   case kType_UncloudifyOrphanedCloud:
      DisplayStatusMessageById(work, kTString_InProgress_UncloudifyOrphanedCloud);
      break;
   case kType_Unlock:
      DisplayStatusMessageById(work, kTString_InProgress_Unlock);
      break;
   case kType_Unlock_Vita:
      DisplayStatusMessageById(work, kTString_InProgress_Unlock_Vita);
      break;
   case kType_Unlink:
      DisplayStatusMessageById(work, kTString_InProgress_Unlink);
      break;
   case kType_Unlink_Vita:
      DisplayStatusMessageById(work, kTString_InProgress_Unlink_Vita);
      break;
   case kType_SolveConflict:
      DisplayStatusMessageById(work, kTString_InProgress_SolveConflict);
      break;
   default:
      ASSERT(0);
      break;
   }
   ShowLoadingBar(work);
}

static void ManageSavesTransfarringStatePopped(TransfarringWork *work)
{
   HideLoadingBar(work);
   LoadSaveGameTableData(work);
}

//kTransfarState_ManageSaves_SolveConflict
static void ManageSavesSolveConflictStateRun(TransfarringWork *work)
{
   if(!Transfarring_IsFileDataValid(work) || A_Pad_Cancel(&work->pad_work))
   {
      PopMenuState(work);
      PopMenuState(work);
      PlayMenuCancelSound();
   }
   else if(A_Pad_Down(&work->pad_work) || A_Pad_Up(&work->pad_work))
   {
      work->mGeneralCursor.y = 1 - work->mGeneralCursor.y;
      PlayAction(work, (EMenuAction)(kAction_WifiTransfar_SelectYes + work->mGeneralCursor.y));
      PlayMenuSelectionSound();
   }
   else if(A_Pad_OK(&work->pad_work))
   {
      TransfarringSaveUiData * const save = GetSaveForCursor(work, &work->mSaveTableCursor);
      PlayMenuOkSound();
      if(save)
      {
         STransfarringError error;
         ECloudConflictResolution resolution;

         if(work->mGeneralCursor.y)
         {
            resolution = kCCR_UseCloudSaveFile;
         }
         else
         {
            resolution = kCCR_UseLocalSaveFile;
          
         }
         error = Transfarring_PostWork_SyncSingleCloudFile(0, work->mCurrentSaveType,
            save->mSaveIndex, resolution, 0);

         if(error.mHighLevelError == kTE_Success)
         {
            PopMenuState(work);
            PushMenuState(work, kTransfarState_ManageSaves_Transfarring);
         }
         else if(error.mHighLevelError == kTE_Unknown)
         {
            PopMenuState(work);
            PopMenuState(work);
            DisplaySystemDialogErrorMessage(error);
         }
         else
         {
            PopMenuState(work);
            PopMenuState(work);
            DisplayModalMessage(work, GetTransfarringStringIdForErrorCode(error.mHighLevelError), 0);
         }
      }
      else
      {
         PopMenuState(work);
         PopMenuState(work);
      }
   }
}

static void ManageSavesSolveConflictStatePushed(TransfarringWork *work)
{
   DisplayStatusMessageById(work, kTString_Choice_SolveConflict);
   SetMenuObjectText(&gTransfarringMenuObjects[kObj_WifiTransfar_YesText], kTString_Choice_SolveConflictLocal, 0);
   SetMenuObjectText(&gTransfarringMenuObjects[kObj_WifiTransfar_NoText], kTString_Choice_SolveConflictRemote, 0);
}

static void ManageSavesSolveConflictStatePopped(TransfarringWork *work)
{
   SetMenuObjectText(&gTransfarringMenuObjects[kObj_WifiTransfar_YesText], kTString_Yes, 0);
   SetMenuObjectText(&gTransfarringMenuObjects[kObj_WifiTransfar_NoText], kTString_No, 0);
}

//kTransfarState_SystemModal_YesNo
static void SystemModalYesNoStateRun(TransfarringWork *work)
{
   if(!BP_CommonDialog_IsStillProcessing(work->mSystemModalHandle))
   {
      work->mSystemModalYesNoReply = BP_CommonDialog_GetResult(work->mSystemModalHandle) == kCDR_OK;
      PopMenuState(work);
   }
}

//kTransfarState_CheckTrophiesDisabled
static void CheckTrophiesDisabledStateRun(TransfarringWork *work)
{
   switch(work->mCheckTrophiesDisabledStep)
   {
   case kCheckTrophiesBegin:
      {
         TransfarringSaveUiData * const save = GetSaveForCursor(work, &work->mSaveTableCursor);

         if(save)
         {
            STransfarringError const error = Transfarring_PostWork_WillTrophiesBeDisabled(work->mCurrentSaveType, save->mSaveIndex);
            if(error.mHighLevelError == kTE_Success)
            {
               work->mCheckTrophiesDisabledStep = kCheckTrophiesLoading;
            }
            else
            {
               if(error.mHighLevelError == kTE_Unknown)
               {
                  DisplaySystemDialogErrorMessage(error);                     
               }
               else
               {
                  DisplayModalMessage(work, GetTransfarringStringIdForErrorCode(error.mHighLevelError), 0);
               }

               work->mCheckTrophiesDisabledStep = kCheckTrophiesDone;
            }
         }
         else
         {
            work->mCheckTrophiesDisabledStep = kCheckTrophiesDone;
         }
      }
      break;
   case kCheckTrophiesLoading:
      {
         STransfarringError const error = Transfarring_ProcessWork();

         UpdateLoadingBar(work);

         if(error.mHighLevelError == kTE_Success)
         {
            //HACK - Return value for Transfarring_PostWork_WillTrophiesBeDisabled() is sent in the low level error
            if(error.mLowLevelError)
            {
               work->mCheckTrophiesDisabledStep = kCheckTrophiesPrompt;
               DisplaySystemDialogYesNoModal(work, kTString_TrophiesWillBeLockedPrompt, 0);
            }
            else
            {
               work->mCheckTrophiesDisabledStep = kCheckTrophiesTransfarr;
            }
         }
         else if(error.mHighLevelError != kTE_Processing)
         {
            if(error.mHighLevelError == kTE_Unknown)
            {
               DisplaySystemDialogErrorMessage(error);
            }
            else
            {
               DisplayModalMessage(work, GetTransfarringStringIdForErrorCode(error.mHighLevelError), 0);
            }
            
            work->mCheckTrophiesDisabledStep = kCheckTrophiesDone;
         }
      }
      break;
   
   case kCheckTrophiesTransfarr:
      {
         TransfarringSaveUiData * const save = GetSaveForCursor(work, &work->mSaveTableCursor);

         ASSERT(work->mTransfarringType == kType_Cloudify || work->mTransfarringType == kType_ToVita);

         if(save && (work->mTransfarringType == kType_Cloudify || work->mTransfarringType == kType_ToVita))
         {
            ETransfarState nextState;
            STransfarringError error;

            if(work->mTransfarringType == kType_Cloudify)
            {
               nextState = kTransfarState_ManageSaves_Transfarring;
               error = Transfarring_PostWork_FlagCloudFile(work->mCurrentSaveType, save->mSaveIndex);

            }
            else
            {
#ifdef BP_PS3
               nextState = kTransfarState_WifiTransfar_Transfarring;
               error = Transfarring_PostNetworkThreadWork_LockLocalSaveFile(work->mCurrentSaveType, save->mSaveIndex);
#else
               ASSERT(0);
               work->mCheckTrophiesDisabledStep = kCheckTrophiesDone;
               return;
#endif
            }
            
            if(error.mHighLevelError == kTE_Success)
            {
               //Get rid of this state and back to the normal flow.
               PopMenuState(work);
               PushMenuState(work, nextState);
            }
            else
            {
               if(error.mHighLevelError == kTE_Unknown)
               {
                  DisplaySystemDialogErrorMessage(error);                     
               }
               else
               {
                  DisplayModalMessage(work, GetTransfarringStringIdForErrorCode(error.mHighLevelError), 0);
               }

               work->mCheckTrophiesDisabledStep = kCheckTrophiesDone;
            }
         }
         else
         {
            work->mCheckTrophiesDisabledStep = kCheckTrophiesDone;
         }
      }
      break;
   
   case kCheckTrophiesPrompt:
      if(work->mSystemModalYesNoReply)
      {
         work->mCheckTrophiesDisabledStep = kCheckTrophiesTransfarr;
      }
      else
      {
         work->mCheckTrophiesDisabledStep = kCheckTrophiesDone;
      }
      break;
   
   case kCheckTrophiesDone:
      work->mTransfarringType = kType_None;
      PopMenuState(work);
      PopMenuState(work);
      break;
   
   default:
      ASSERT(0);
   }
}

static void CheckTrophiesDisabledStatePushed(TransfarringWork *work)
{
   work->mCheckTrophiesDisabledStep = kCheckTrophiesBegin;

   PlayAction(work, kAction_WifiTransfar_HideYesNo);
   
   switch(work->mTransfarringType)
   {
   case kType_ToVita:
      DisplayStatusMessageById(work, kTString_InProgress_TransfarrToVita);
      break;
   case kType_Cloudify:
      DisplayStatusMessageById(work, kTString_InProgress_Cloudify);
      break;
   default:
      ASSERT(0);
      break;
   }

   ShowLoadingBar(work);
}

static void CheckTrophiesDisabledStatePopped(struct STransfarringWork *work)
{
   HideLoadingBar(work);
   DisplayStatusMessageById(work, kTString_Empty);
}

//Shared State Functions
static void SharedYesNoStatePushed(TransfarringWork* work)
{
   ETransfarringStringId textId = kTString_Empty;

   ASSERT(IsSaveTableCursorPositionValid(work, &work->mSaveTableCursor));

   work->mGeneralCursor.x = 0;
   work->mGeneralCursor.y = 0;

   switch(work->mTransfarringType)
   {
   case kType_ToVita:
      textId = kTString_Prompt_TransfarrToVita;
      break;
   case kType_ToVitaOrphaned:
      textId = kTString_Prompt_TransfarrToVitaOrphaned;
      break;
   case kType_ToVitaLinked:
      textId = kTString_Prompt_TransfarrToVitaLinked;
      break;
   case kType_FromVita:
      textId = kTString_Prompt_TransfarrFromVita;
      break;
   case kType_FromVitaOrphaned:
      textId = kTString_Prompt_TransfarrFromVitaOrphaned;
      break;
   case kType_FromVitaLinked:
      textId = kTString_Prompt_TransfarrFromVitaLinked;
      break;
   case kType_Cloudify:
      textId = kTString_Prompt_Cloudify;
      break;
   case kType_Delete:
      textId = kTString_Prompt_Delete;
      break;
   case kType_UncloudifyLinked:
      textId = kTString_Prompt_UncloudifyLinked;
      break;
   case kType_UncloudifyOrphanedLocal:
      textId = kTString_Prompt_UncloudifyOrphanedLocal;
      break;
   case kType_UncloudifyOrphanedCloud:
      textId = kTString_Prompt_UncloudifyOrphanedCloud;
      break;
   case kType_Unlock:
      textId = kTString_Prompt_Unlock;
      break;
   case kType_Unlock_Vita:
      textId = kTString_Prompt_Unlock_Vita;
      break;
   case kType_Unlink:
      textId = kTString_Prompt_Unlink;
      break;
   case kType_Unlink_Vita:
      textId = kTString_Prompt_Unlink_Vita;
      break;
   case kType_SolveConflict:
      textId = kTString_Prompt_SolveConflict;
      break;
   default:
      ASSERT(0);
      break;
   }

   DisplayStatusMessageById(work, textId);
   DisplayButtonOptions(0, 0, kTString_Empty, kTString_Empty);

   HideMenuObject(&gTransfarringMenuObjects[kObj_WifiTransfar_PsnConnectionStatus_Root]);

   PlayAction(work, kAction_WifiTransfar_DisplayYesNo);
}

static void SharedYesNoStatePopped(TransfarringWork* work)
{
   EPlatformSaveType saveType;
   int currentSaveIndex;
   int valid;

   GetSaveTypeAndIndexFromCursor(work, &saveType, &currentSaveIndex, &work->mSaveTableCursor);

   valid = SetSaveTableCursorBySaveIndex(work, saveType, currentSaveIndex, kPlayAction_No);

   if(!valid)
   {
      int const newSaveIndex = currentSaveIndex + 1;

      if(newSaveIndex < MAX_NUMBER_OF_SAVE_SLOTS)
      {
         valid = SetSaveTableCursorBySaveIndex(work, saveType, newSaveIndex, kPlayAction_No);
      }
   }

   if(!valid)
   {
      int const newSaveIndex = currentSaveIndex - 1;

      if(newSaveIndex >= 0)
      {
         valid = SetSaveTableCursorBySaveIndex(work, saveType, newSaveIndex, kPlayAction_No);
      }
   }

   if(!valid)
   {
      SetSaveGameType(work, work->mCurrentSaveType);
   }

   if(work->mSaveTableType == kSaveTable_Cloud)
   {
      DisplayMenuObject(&gTransfarringMenuObjects[kObj_WifiTransfar_PsnConnectionStatus_Root]);
   }

   PlayAction(work, kAction_WifiTransfar_HideYesNo);
}

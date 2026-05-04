#pragma once

#ifndef __TRANSFARRING_MENU_DATA__H__
#define __TRANSFARRING_MENU_DATA__H__

#include "Transfarring_UI_Strings.h"

#define MENU_OBJECT_NONE -1

#define MAX_X_VALUE 960
#define MAX_Y_VALUE 544

#define ITEM_W 395
#define ITEM_H 23

#define SAVE_TABLE_X 90
#define SAVE_TABLE_Y 99

#define PROGRESS_BAR_Y 475

#define SAVE_TABLE_WIDTH (MAX_X_VALUE - SAVE_TABLE_X * 2)
#define SAVE_TABLE_HEIGHT 300
#define SAVE_TABLE_MID_SECTION_WIDTH 68
#define SAVE_TABLE_SIDE_TABLE_WIDTH (SAVE_TABLE_WIDTH / 2 - SAVE_TABLE_MID_SECTION_WIDTH / 2)

#define SAVE_SLOT_HEIGHT 40
#define SAVE_SLOT_WIDTH (SAVE_TABLE_SIDE_TABLE_WIDTH)
#define SAVE_SLOT_OFFSET_Y 10
#define SAVE_SLOT_HIGHLIGHT_WIDTH (SAVE_SLOT_WIDTH)
#define SAVE_SLOT_HIGHLIGHT_OFFSET_X 0

#define STATUS_MESSAGE_HEIGHT 28

#define YES_BUTTON_OFFSET_X 4
#define YES_BUTTON_OFFSET_Y 8

#define CLOUD_LABEL_X 656
#define VITA_LABEL_X 616
#define PS3_LABEL_X 165
#define VITA_LABEL_LEFT_X 155

#define YES_NO_BUTTONS_HEIGHT 18 
#define YES_NO_BUTTONS_WIDTH (SAVE_TABLE_SIDE_TABLE_WIDTH / 2 - YES_BUTTON_OFFSET_X)

#define NO_BUTTON_OFFSET_X YES_BUTTON_OFFSET_X
#define NO_BUTTON_OFFSET_Y (YES_BUTTON_OFFSET_Y * 2 + YES_NO_BUTTONS_HEIGHT)

#define MAIN_MENU_BUTTON_TEXT_OFFSET_X 10
#define MAIN_MENU_BUTTON_TEXT_OFFSET_Y 3

#define CONNECTION_BOX_SPRITE_X 12
#define CONNECTION_BOX_SPRITE_Y 7
#define CONNECTION_BOX_SPRITE_WIDTH 192
#define CONNECTION_BOX_SPRITE_HEIGHT 18

#define TRANSFARRING_ICON_X 281
#define TRANSFARRING_ICON_Y 110
#define TRANSFARRING_ICON_WIDTH 399
#define TRANSFARRING_ICON_HEIGHT 342

#define BACKGROUND_ICON_PS3_X 170
#define BACKGROUND_ICON_PS3_Y 130
#define BACKGROUND_ICON_PS3_WIDTH 187
#define BACKGROUND_ICON_PS3_HEIGHT 258

#define BACKGROUND_ICON_VITA_X_LEFT 135
#define BACKGROUND_ICON_VITA_X_RIGHT 560
#define BACKGROUND_ICON_VITA_Y 180
#define BACKGROUND_ICON_VITA_WIDTH 264
#define BACKGROUND_ICON_VITA_HEIGHT 121

#define BACKGROUND_ICON_CLOUD_X 565
#define BACKGROUND_ICON_CLOUD_Y 160
#define BACKGROUND_ICON_CLOUD_WIDTH 250
#define BACKGROUND_ICON_CLOUD_HEIGHT 166

#if defined(BP_VITA)
   #define CONSOLE_IMAGE_ROOT_X 570
   #define CONSOLE_IMAGE_ROOT_Y 79

   #define CONSOLE_IMAGE_VITA_X -8
   #define CONSOLE_IMAGE_VITA_Y 195

   #define CONSOLE_IMAGE_PS3_X 221
   #define CONSOLE_IMAGE_PS3_Y 0

   #define WAVES_X 154
   #define WAVES_Y 131

   #define WAVES_WIDTH 75
   #define WAVES_HEIGHT 75
#else
   #define CONSOLE_IMAGE_ROOT_X 553
   #define CONSOLE_IMAGE_ROOT_Y (-10)

   #define CONSOLE_IMAGE_VITA_X 190
   #define CONSOLE_IMAGE_VITA_Y 95

   #define CONSOLE_IMAGE_PS3_X 10
   #define CONSOLE_IMAGE_PS3_Y 250

   #define WAVES_X 115
   #define WAVES_Y 162

   #define WAVES_WIDTH 96
   #define WAVES_HEIGHT 96
#endif

#if MGS_VERSION == 2
   #define MENU_ROOT_X 107
   #define MENU_ROOT_Y 355
   #define MORE_INFO_ROOT_X MENU_ROOT_X
   #define MORE_INFO_ROOT_Y 310
   #define SUBHEADER_X 108
   #define SUBHEADER_Y 73
   #define CONNECTION_BOX_X 667
   #define CONNECTION_BOX_Y 405
   #define SAVE_TABLE_BOTTOM_TO_BOTTOM_LINE_DISTANCE 60
   
   #define GAME_TYPE_X 125
   #define GAME_TYPE_Y 43

   #define BUTTON_L_WIDTH 31
   #define BUTTON_L_HEIGHT 14
   #define BUTTON_L_X 86
   #define BUTTON_L_Y 45
   #define BUTTON_R_DELTA_X (GAME_TYPE_X - BUTTON_L_X - BUTTON_L_WIDTH)

   #define EXPLANATION_TEXT_X 108
   #define EXPLANATION_TEXT_Y 104

   #define JP_EXP_X 115
   #define JP_EXP_Y 465

   #define JP_EXP_FLAGS 0

   #define SAVE_TYPE_TEXT_OFFSET_Y 25
#else
   #define MENU_ROOT_X 96
   #define MENU_ROOT_Y 378
   #define MORE_INFO_ROOT_X MENU_ROOT_X
   #define MORE_INFO_ROOT_Y 325
   #define SUBHEADER_X 111
   #define SUBHEADER_Y 76
   #define CONNECTION_BOX_X 667
   #define CONNECTION_BOX_Y 419
   #define SAVE_TABLE_BOTTOM_TO_BOTTOM_LINE_DISTANCE 75

   #define GAME_TYPE_X 91
   #define GAME_TYPE_Y 43

   #define EXPLANATION_TEXT_X 105
   #define EXPLANATION_TEXT_Y 110

   #define JP_EXP_X 880
   #define JP_EXP_Y 478

   #define JP_EXP_FLAGS kAlignRight
   
   #define SAVE_TYPE_TEXT_OFFSET_Y 40
#endif

// colors are ABGR
#if MGS_VERSION == 2
   #define skSaveSlotHighlightColor 0x604F5C48
   #define skSaveSlotNormalColor 0x001c1e1c
   #define skHighlightColor 0x604F5C48
   #define skTextColor 0x80aab4a0
   #define skTextColorLight 0x80ddead0
   #define skGreyedOutTextColor 0x80384343
   #define skConnectionBoxBackgroundColor 0x1a252c27
   #define skTextBackgroundColor 0x1a252c27
   #define skLineColor 0x805a6450
   #define skInvisible 0x00000000
   #define skBrokenLinkTint 0x80828e8d
   #define skWhite 0x80ffffff
   #define skLoadingBarBackgroundColor 0x80252b2b
   #define skLoadingBarForegroundColor skWhite
   #define skPS3SpriteBackgroundColor 0x401c1d1d
   #define skVitaSpriteBackgroundColor 0x401c1d1d   
   #define skConsoleSpriteOutlineColor 0x80aab4a0
   #define skLockColor 0x80aab4a0
   #define skCloudColor 0x80ffffff
   #define skCloudConflictColor 0x800000ff
   #define skLinkedSlotColor 0x601c1e1c
   #define skTransfarringIconColor 0x60728266
   #define skTransfarringIconColorFaded 0x20728266
   #define skBackgroundIconColor 0x24728266
   #define skBackgroundIconColorCloud 0x0A728266
   #define skBackgroundCoverColor 0x40000000
   #define skPsnStatusColor skTextColor
   #define skJpExpColor skWhite
#endif
#if MGS_VERSION == 3
   #define skSaveSlotHighlightColor 0x80748989
   #define skSaveSlotNormalColor 0x00748989
   #define skHighlightColor 0x80708585
   #define skTextColor 0x80272f2f
   #define skTextColorLight 0x80708585
   #define skGreyedOutTextColor 0x80384343
   #define skConnectionBoxBackgroundColor 0x1a000000
   #define skTextBackgroundColor 0x803c4848
   #define skLineColor 0x80333C3C
   #define skInvisible 0x00000000
   #define skBrokenLinkTint 0x80828e8d
   #define skWhite 0x80ffffff
   #define skLoadingBarBackgroundColor 0x80252b2b
   #define skLoadingBarForegroundColor 0x803c4747
   #if defined(BP_VITA)
      #define skPS3SpriteBackgroundColor 0x804f5e5e
      #define skVitaSpriteBackgroundColor 0x404f5e5e
   #else
      #define skPS3SpriteBackgroundColor 0x404f5e5e
      #define skVitaSpriteBackgroundColor 0x804f5e5e
   #endif
   #define skConsoleSpriteOutlineColor 0x80333c3c
   #define skLockColor 0x80272f2f
   #define skCloudColor 0x80ffffff
   #define skCloudConflictColor 0x80808080
   #define skLinkedSlotColor 0x60596a6a
   #define skTransfarringIconColor 0x30333c3c
   #define skTransfarringIconColorFaded 0x20333c3c
   #define skBackgroundIconColor 0x34333c3c
   #define skBackgroundIconColorCloud 0x0A333c3c
   #define skPsnStatusColor skWhite
   #define skJpExpColor skTextColor
#endif

typedef enum
{
   kObj_None = -1,
   kObj_Root,
#if MGS_VERSION == 2      
   kObj_BackgroundCover,
#endif
   kObj_TransfarringLogo,
   kObj_IdleStageMenuRoot,
#if MGS_VERSION == 3
   kObj_TextBackground,
#endif
   kObj_TransfarringExplanationText,
   kObj_TopBarXFarText,
   kObj_TopBarSubHeader,
#if MGS_VERSION == 2
   kObj_TopBar,
   kObj_SideBar_Right,
#endif
   kObj_SideBar,
   kObj_ConsoleImageRoot,
   kObj_VitaImage,
   kObj_VitaImageInner,
   kObj_VitaImageOutline,
   kObj_PS3Image,
   kObj_PS3ImageInner,
   kObj_PS3ImageOutline,
   kObj_WavesRoot,
   kObj_WavesOut,
   kObj_WavesIn,
   kObj_MoreInfoRoot,
   kObj_MoreInfoButton,
   kObj_MoreInfoText,
   kObj_ConnectionBox,
   kObj_ConnectionBoxBackground,
   kObj_ConnectionText,
   kObj_ConnectionNo,
   kObj_ConnectionYes,
   
   kObj_MenuRoot,
   
   kObj_Button_0_Root,
   kObj_Button_0_High,
   kObj_Button_0_Box,
   kObj_Button_0_Text_Cloud,
   kObj_Button_0_Text_Wifi,
   
   kObj_Button_1_Root,
   kObj_Button_1_High,
   kObj_Button_1_Box,
   kObj_Button_1_Text_Listen,
   kObj_Button_1_Text_Stop,
   
   kObj_Button_2_Root,
   kObj_Button_2_High,
   kObj_Button_2_Box,
   kObj_Button_2_Text,

   kObj_JpExpRoot,
   kObj_JpExpText,

   kObj_WifiTransfar_Root,
   kObj_Background_Left,
   kObj_Background_Right,
   kObj_Background_Cloud,

#if MGS_VERSION == 2
   kObj_WifiTransfar_BarL1,
   kObj_WifiTransfar_BarR1,
#endif         

   kObj_WifiTransfar_BarText,
   kObj_WifiTransfar_PsnConnectionStatus_Root,
   kObj_WifiTransfar_PsnConnectionStatus_Connected,
   kObj_WifiTransfar_PsnConnectionStatus_Disconnected,
   kObj_WifiTransfar_LabelPS3,
   kObj_WifiTransfar_LabelVita,
   kObj_WifiTransfar_BottomLineSeparator,
   kObj_WifiTransfar_CommandBox,
   kObj_WifiTransfar_CommandBoxEdge,
   kObj_WifiTransfar_YesNoRoot,
   kObj_WifiTransfar_YesRoot,
   kObj_WifiTransfar_YesHigh, 
   kObj_WifiTransfar_YesBox,
   kObj_WifiTransfar_YesText,
   kObj_WifiTransfar_NoRoot,
   kObj_WifiTransfar_NoHigh,
   kObj_WifiTransfar_NoBox,
   kObj_WifiTransfar_NoText,
   
   kObj_ButtonDisplayRoot,
   kObj_ButtonDisplay_X_Root,
   kObj_ButtonDisplay_X_Sprt,
   kObj_ButtonDisplay_X_Text,
   kObj_ButtonDisplay_SQ_Root,
   kObj_ButtonDisplay_SQ_Sprt,
   kObj_ButtonDisplay_SQ_Text,
   
   kObj_WifiTransfar_SaveTable,

#if MGS_VERSION == 2
   kObj_WifiTransfar_LeftBoxTopEdge,
   kObj_WifiTransfar_RightBoxTopEdge,
#endif

   kObj_WifiTransfar_LeftBoxLeftEdge,
   kObj_WifiTransfar_LeftBoxRightEdge,
   kObj_WifiTransfar_RightBoxLeftEdge,
   kObj_WifiTransfar_RightBoxRightEdge,

   kObj_LinkedSlot_0,
   kObj_LinkedSlotBoxFill_0,
   kObj_LinkedSlotHigh_0,
   kObj_LinkedSlotBox_0,
   kObj_LinkedSlotText_0,
   kObj_LinkedSlot_1,
   kObj_LinkedSlotBoxFill_1,
   kObj_LinkedSlotHigh_1,
   kObj_LinkedSlotBox_1,
   kObj_LinkedSlotText_1,
   kObj_LinkedSlot_2,
   kObj_LinkedSlotBoxFill_2,
   kObj_LinkedSlotHigh_2,
   kObj_LinkedSlotBox_2,
   kObj_LinkedSlotText_2,
   kObj_LinkedSlot_3,
   kObj_LinkedSlotBoxFill_3,
   kObj_LinkedSlotHigh_3, 
   kObj_LinkedSlotBox_3,
   kObj_LinkedSlotText_3,
   kObj_LinkedSlot_4,
   kObj_LinkedSlotBoxFill_4,
   kObj_LinkedSlotHigh_4, 
   kObj_LinkedSlotBox_4,
   kObj_LinkedSlotText_4,
   kObj_LinkedSlot_5,
   kObj_LinkedSlotBoxFill_5,
   kObj_LinkedSlotHigh_5,
   kObj_LinkedSlotBox_5,
   kObj_LinkedSlotText_5,

   kObj_WifiTransfar_LeftBox,
     
   kObj_LeftSlot_0,
   kObj_LeftSlotHigh_0,
   kObj_LeftSlotLineTop_0,
   kObj_LeftSlotLineBottom_0,
   kObj_LeftSlotId_0,
   kObj_LeftSlotClear_0,
   kObj_LeftSlotDiff_0,
   kObj_LeftSlotDate_0,
   kObj_LeftSlotTime_0,
   kObj_LeftSlotLock_0,
   kObj_LeftSlotCloud_0,
   kObj_LeftSlotLink_0,
   kObj_LeftSlot_1,
   kObj_LeftSlotHigh_1,
   kObj_LeftSlotLineTop_1,
   kObj_LeftSlotLineBottom_1,
   kObj_LeftSlotId_1,
   kObj_LeftSlotClear_1,
   kObj_LeftSlotDiff_1,
   kObj_LeftSlotDate_1,
   kObj_LeftSlotTime_1,
   kObj_LeftSlotLock_1,
   kObj_LeftSlotCloud_1,
   kObj_LeftSlotLink_1,
   kObj_LeftSlot_2,
   kObj_LeftSlotHigh_2,
   kObj_LeftSlotLineTop_2,
   kObj_LeftSlotLineBottom_2,
   kObj_LeftSlotId_2,
   kObj_LeftSlotClear_2,
   kObj_LeftSlotDiff_2,
   kObj_LeftSlotDate_2,
   kObj_LeftSlotTime_2,
   kObj_LeftSlotLock_2,
   kObj_LeftSlotCloud_2,
   kObj_LeftSlotLink_2,
   kObj_LeftSlot_3,
   kObj_LeftSlotHigh_3,
   kObj_LeftSlotLineTop_3,
   kObj_LeftSlotLineBottom_3,
   kObj_LeftSlotId_3,
   kObj_LeftSlotClear_3,
   kObj_LeftSlotDiff_3,
   kObj_LeftSlotDate_3,
   kObj_LeftSlotTime_3,
   kObj_LeftSlotLock_3,
   kObj_LeftSlotCloud_3,
   kObj_LeftSlotLink_3,
   kObj_LeftSlot_4,
   kObj_LeftSlotHigh_4,
   kObj_LeftSlotLineTop_4,
   kObj_LeftSlotLineBottom_4,
   kObj_LeftSlotId_4,
   kObj_LeftSlotClear_4,
   kObj_LeftSlotDiff_4,
   kObj_LeftSlotDate_4,
   kObj_LeftSlotTime_4,
   kObj_LeftSlotLock_4,
   kObj_LeftSlotCloud_4,
   kObj_LeftSlotLink_4,
   kObj_LeftSlot_5,
   kObj_LeftSlotHigh_5,
   kObj_LeftSlotLineTop_5,
   kObj_LeftSlotLineBottom_5,
   kObj_LeftSlotId_5,
   kObj_LeftSlotClear_5,
   kObj_LeftSlotDiff_5,
   kObj_LeftSlotDate_5,
   kObj_LeftSlotTime_5,
   kObj_LeftSlotLock_5,
   kObj_LeftSlotCloud_5,
   kObj_LeftSlotLink_5,

   kObj_WifiTransfar_RightBox,

   kObj_RightSlot_0,
   kObj_RightSlotHigh_0,
   kObj_RightSlotLineTop_0,
   kObj_RightSlotLineBottom_0,
   kObj_RightSlotId_0,
   kObj_RightSlotClear_0,
   kObj_RightSlotDiff_0,
   kObj_RightSlotDate_0,
   kObj_RightSlotTime_0,
   kObj_RightSlotLock_0,
   kObj_RightSlotCloud_0,
   kObj_RightSlotLink_0,
   kObj_RightSlot_1,
   kObj_RightSlotHigh_1,
   kObj_RightSlotLineTop_1,
   kObj_RightSlotLineBottom_1,
   kObj_RightSlotId_1,
   kObj_RightSlotClear_1,
   kObj_RightSlotDiff_1,
   kObj_RightSlotDate_1,
   kObj_RightSlotTime_1,
   kObj_RightSlotLock_1,
   kObj_RightSlotCloud_1,
   kObj_RightSlotLink_1,
   kObj_RightSlot_2,
   kObj_RightSlotHigh_2,
   kObj_RightSlotLineTop_2,
   kObj_RightSlotLineBottom_2,
   kObj_RightSlotId_2,
   kObj_RightSlotClear_2,
   kObj_RightSlotDiff_2,
   kObj_RightSlotDate_2,
   kObj_RightSlotTime_2,
   kObj_RightSlotLock_2,
   kObj_RightSlotCloud_2,
   kObj_RightSlotLink_2,
   kObj_RightSlot_3,
   kObj_RightSlotHigh_3,
   kObj_RightSlotLineTop_3,
   kObj_RightSlotLineBottom_3,
   kObj_RightSlotId_3,
   kObj_RightSlotClear_3,
   kObj_RightSlotDiff_3,
   kObj_RightSlotDate_3,
   kObj_RightSlotTime_3,
   kObj_RightSlotLock_3,
   kObj_RightSlotCloud_3,
   kObj_RightSlotLink_3,
   kObj_RightSlot_4,
   kObj_RightSlotHigh_4,
   kObj_RightSlotLineTop_4,
   kObj_RightSlotLineBottom_4,
   kObj_RightSlotId_4,
   kObj_RightSlotClear_4,
   kObj_RightSlotDiff_4,
   kObj_RightSlotDate_4,
   kObj_RightSlotTime_4,
   kObj_RightSlotLock_4,
   kObj_RightSlotCloud_4,
   kObj_RightSlotLink_4,
   kObj_RightSlot_5,
   kObj_RightSlotHigh_5,
   kObj_RightSlotLineTop_5,
   kObj_RightSlotLineBottom_5,
   kObj_RightSlotId_5,
   kObj_RightSlotClear_5,
   kObj_RightSlotDiff_5,
   kObj_RightSlotDate_5,
   kObj_RightSlotTime_5,
   kObj_RightSlotLock_5,
   kObj_RightSlotCloud_5,
   kObj_RightSlotLink_5,

   kObj_StatusMessageRoot,
   kObj_StatusMessageText,
   kObj_SaveTypeText,

   kObj_LoadingBarRoot,
   kObj_LoadingBarBackground,
   kObj_LoadingBarForeground,

   kObj_Count
}
EMenuObject;

typedef enum
{
   kTransfarState_Entering,
   kTransfarState_MainMenu_Disconnected_PS3,
   kTransfarState_MainMenu_Listening_PS3,
   kTransfarState_MainMenu_Connected_PS3,
   kTransfarState_MainMenu_Disconnected_VITA,
   kTransfarState_MainMenu_Connected_VITA,
   kTransfarState_Leaving,
   kTransfarState_WifiTransfar,
   kTransfarState_WifiTransfarLoading,
   kTransfarState_WifiTransfar_YesNo,
   kTransfarState_WifiTransfar_Transfarring,
   kTransfarState_ModalMessage,
   kTransfarState_ManageSaves,
   kTransfarState_ManageSavesLoading,
   kTransfarState_ManageSaves_YesNo,
   kTransfarState_ManageSaves_Transfarring,
   kTransfarState_ManageSaves_SolveConflict,
   kTransfarState_SystemModal_YesNo,
   kTransfarState_CheckTrophiesDisabled,

   kTransfarStateCount,
}
ETransfarState;

typedef enum
{
   kAction_FadeIn,
   kAction_FadeOut,
   kAction_CursorButton_0,
   kAction_CursorButton_1,
   kAction_CursorButton_2,

   kAction_DisconnectedToListening_PS3,
   kAction_DisconnectedToListening_PS3_EnableButtons,
   kAction_ListeningToConnected_PS3,
   kAction_ListeningToDisconnected_PS3,
   kAction_ConnectedToDisconnected_PS3,
   kAction_ConnectedToListening_PS3,

   kAction_DisconnectedToConnected_VITA,
   kAction_ConnectedToDisconnected_VITA,

   kAction_EnterWifiTransfarState,
   kAction_LeaveWifiTransfarState,
   kAction_EnterManageSavesState,
   kAction_LeaveManageSavesState,
   kAction_WifiTransfar_SaveSelect,
   kAction_WifiTransfar_DisplayYesNo,
   kAction_WifiTransfar_HideYesNo,
   kAction_WifiTransfar_SelectYes,
   kAction_WifiTransfar_SelectNo,
   kAction_WifiTransfar_FadeLoadingBarOut,
   kAction_WifiTransfar_FadeLoadingBarIn,

   kMenuActionCount,
}
EMenuAction;

typedef enum
{
   kNull,
   kFBox,
   kOBox,
   kText,
   kLine,
   kSprt,
   kAnim,
   kBpText,  //IMPORTANT: This type of object uses w and h to determine the size of the target texture for rendering the text and scale to scale it.
}
EMenuObjectType;

typedef enum
{
   kAlignCenter   = 0x00000001,
   kDropShadow    = 0x00000010,
   kAlignRight    = 0x00000100,
}
ETextFlags;

typedef enum
{
   kFadeOutLeftToRight = 0x00000001,
}
EBoxFlags;

typedef struct
{
   float x, y;
   unsigned int col;
   float w, h;
}
SMenuStatus;

#define MAX_ANIM_FRAMES 6

typedef struct SMenuObjectTag
{
   // constant data
   EMenuObjectType type;
   int parent;
   SMenuStatus refStatus;
   ETransfarringStringId textId;
   
   union
   {
      const char *textureName;
      const char *textOverride;
   };
   
   float scale; //Only kBpText and kText use this
   
   union
   {
      ETextFlags textFlags; //Only supported by kBpText
      EBoxFlags boxFlags;
   };

   // variable data
   SMenuStatus status; // morph target source
   SMenuStatus curStatus;
   struct SMenuObjectTag *pFirstChild;
   struct SMenuObjectTag *pSibling;
   DG_TEX *tex[MAX_ANIM_FRAMES];
   int currentFrame;
   
   void *pMiscData;
}
SMenuObject;

#define MAX_ACTION_ITEMS 5
#define MAX_ACTION_OBJECTS 15

typedef enum
{
   kSet,
   kMorph,
   kEnd
}
EMenuActionType;

typedef struct
{
   EMenuActionType type;
   int duration;
   SMenuStatus status;
}
SMenuActionItem;

typedef struct
{
   int object;
   SMenuActionItem items[MAX_ACTION_ITEMS];
}
SMenuObjectAction;

typedef struct
{
   SMenuObjectAction objActions[MAX_ACTION_OBJECTS];
}
SMenuAction;

extern SMenuObject gTransfarringMenuObjects[];
extern SMenuAction gTransfarringMenuActions[];
extern int gTransfarringMenuObjCount;

#endif

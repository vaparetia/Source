#include "StdAfx.h"

#include "transfarring_menu_data.h"

#define DEFINE_SAVE_SLOT(index, parent, sidePrefix)\
   { kNull, (parent), { 0, (index) * (SAVE_SLOT_HEIGHT + SAVE_SLOT_OFFSET_Y), skWhite } },\
      { kFBox, kObj_##sidePrefix##Slot_##index, { SAVE_SLOT_HIGHLIGHT_OFFSET_X, 0, skSaveSlotNormalColor, SAVE_SLOT_HIGHLIGHT_WIDTH, SAVE_SLOT_HEIGHT / 2} },\
      { kLine, kObj_##sidePrefix##Slot_##index, { 0, 0, skLineColor, SAVE_SLOT_WIDTH, 1 } },\
      { kLine, kObj_##sidePrefix##Slot_##index, { 0, SAVE_SLOT_HEIGHT, skLineColor, SAVE_SLOT_WIDTH, 1 } },\
      { kText, kObj_##sidePrefix##Slot_##index, { 10, 3, skTextColor }, kTString_USE_OVERRIDE, 0, 0.4f },\
      { kText, kObj_##sidePrefix##Slot_##index, { 0.32f * SAVE_SLOT_WIDTH, 3, skTextColor }, kTString_USE_OVERRIDE, 0, 0.4f },\
      { kText, kObj_##sidePrefix##Slot_##index, { 0.55f * SAVE_SLOT_WIDTH, 3, skTextColor }, kTString_USE_OVERRIDE, 0, 0.4f },\
      { kText, kObj_##sidePrefix##Slot_##index, { 10, SAVE_SLOT_HEIGHT / 2 + 3, skTextColor }, kTString_USE_OVERRIDE, 0, 0.4f },\
      { kText, kObj_##sidePrefix##Slot_##index, { 0.55f * SAVE_SLOT_WIDTH, SAVE_SLOT_HEIGHT / 2 + 3, skTextColor }, kTString_USE_OVERRIDE, 0, 0.4f },\
      { kSprt, kObj_##sidePrefix##Slot_##index, { -8, 1, skInvisible, 18, 18 }, 0, "transfarring_icon_lock" },\
      { kSprt, kObj_##sidePrefix##Slot_##index, { -8, -1, skInvisible, 18, 22 }, 0, "transfarring_icon_cloud" },\
      { kSprt, kObj_##sidePrefix##Slot_##index, { -4, 1, skInvisible, 15, 18 }, 0, "transfarring_icon_link" }

 
#define DEFINE_LINKED_SLOT(index)\
   { kNull, kObj_WifiTransfar_SaveTable, { 0, (index) * (SAVE_SLOT_HEIGHT + SAVE_SLOT_OFFSET_Y), skInvisible } },\
      { kFBox, kObj_LinkedSlot_##index, { 0, 0, skLinkedSlotColor, SAVE_TABLE_WIDTH, SAVE_SLOT_HEIGHT / 2 } },\
      { kFBox, kObj_LinkedSlot_##index, { 0, 0, skSaveSlotNormalColor, SAVE_TABLE_WIDTH, SAVE_SLOT_HEIGHT / 2 } },\
      { kOBox, kObj_LinkedSlot_##index, { 0, 0, skLineColor, SAVE_TABLE_WIDTH, SAVE_SLOT_HEIGHT / 2 } },\
      { kSprt, kObj_LinkedSlot_##index, { SAVE_TABLE_SIDE_TABLE_WIDTH + 2, 4, skWhite, 64, 12}, 0, "transfarring_linkedimage" }

#define DEFINE_LEFT_SAVE_SLOT(index) DEFINE_SAVE_SLOT(index, kObj_WifiTransfar_LeftBox, Left)
#define DEFINE_RIGHT_SAVE_SLOT(index) DEFINE_SAVE_SLOT(index, kObj_WifiTransfar_RightBox, Right)

int gTransfarringMenuObjCount = kObj_Count;

SMenuObject gTransfarringMenuObjects[] =
{
   //kObj_Root
   { kNull, kObj_None, { 0, 0, skWhite } },
#if MGS_VERSION == 2      
      //kObj_BackgroundCover
      { kFBox, kObj_WifiTransfar_Root, { 0, 0, skInvisible, MAX_X_VALUE, MAX_Y_VALUE } },
#endif
      //kObj_TransfarringLogo
      { kSprt, kObj_Root, { TRANSFARRING_ICON_X, TRANSFARRING_ICON_Y, skInvisible, TRANSFARRING_ICON_WIDTH, TRANSFARRING_ICON_HEIGHT }, 0, "transfarring_transfarringlogo" },
      //Main Menu Stage
      //kObj_IdleStageMenuRoot      
      { kNull, kObj_Root, { -20, 0, skInvisible } },
#if MGS_VERSION == 2
         //kObj_TransfarringExplanationText
   #if defined(BP_VITA)
         { kBpText, kObj_IdleStageMenuRoot, { EXPLANATION_TEXT_X, EXPLANATION_TEXT_Y, skTextColorLight, 900, 480 }, kTString_TransfarringExplanation_Disconnected_PS3, 0, 0.5f },
   #else
         { kBpText, kObj_IdleStageMenuRoot, { EXPLANATION_TEXT_X, EXPLANATION_TEXT_Y, skTextColorLight, 1000, 480 }, kTString_TransfarringExplanation_Disconnected_PS3, 0, 0.5f },
   #endif
#else
         //kObj_TextBackground      
         { kFBox, kObj_IdleStageMenuRoot, { 96, 104, skTextBackgroundColor, 420, 216 }, 0, 0, 0, kFadeOutLeftToRight },
         //kObj_TransfarringExplanationText
   #if defined(BP_VITA)
         { kBpText, kObj_IdleStageMenuRoot, { EXPLANATION_TEXT_X, EXPLANATION_TEXT_Y, skTextColorLight, 900, 480 }, kTString_TransfarringExplanation_Disconnected_PS3, 0, 0.5f, kDropShadow },
   #else
         { kBpText, kObj_IdleStageMenuRoot, { EXPLANATION_TEXT_X, EXPLANATION_TEXT_Y, skTextColorLight, 1000, 480 }, kTString_TransfarringExplanation_Disconnected_PS3, 0, 0.5f, kDropShadow },
   #endif
#endif
         //kObj_TopBarXFarText
         { kText, kObj_IdleStageMenuRoot, { 91, 43, skTextColor }, kTString_Transfarring, 0, 0.5f },
         //kObj_TopBarSubHeader
         { kBpText, kObj_IdleStageMenuRoot, { SUBHEADER_X, SUBHEADER_Y, skTextColor, 1700, 90 }, kString_SubHeader, 0, 0.38f },
#if MGS_VERSION == 2
         //kObj_TopBar
   #if defined(BP_PS3)
         { kLine, kObj_IdleStageMenuRoot, { 36, 27, skLineColor, 888, 1 } },
   #else
         { kFBox, kObj_IdleStageMenuRoot, { 36, 27, skLineColor, 888, 1.5f } },
   #endif
         //kObj_SideBar_Right
         { kFBox, kObj_IdleStageMenuRoot, { 922, 27, skLineColor, 2.25f, 430 } },
         //kObj_SideBar
         { kFBox, kObj_IdleStageMenuRoot, { 82, 27, skLineColor, 2.25f, 430 } },
#else
         //kObj_SideBar
         { kLine, kObj_IdleStageMenuRoot, { 91, 99, skLineColor, 1, 375 } },
#endif
         //kObj_ConsoleImageRoot
         { kNull, kObj_IdleStageMenuRoot, { CONSOLE_IMAGE_ROOT_X, CONSOLE_IMAGE_ROOT_Y, skWhite } },
            //kObj_VitaImage
            { kNull, kObj_ConsoleImageRoot, { CONSOLE_IMAGE_VITA_X, CONSOLE_IMAGE_VITA_Y, skWhite } },
               //kObj_VitaImageInner
               { kSprt, kObj_VitaImage, { 0, 0, skVitaSpriteBackgroundColor, 178, 82 }, 0, "transfarring_image_vita_2" },
               //kObj_VitaImageOutline
               { kSprt, kObj_VitaImage, { 0, 0, skConsoleSpriteOutlineColor, 178, 82 }, 0, "transfarring_image_vita_1" },
            //kObj_PS3Image
            { kNull, kObj_ConsoleImageRoot, { CONSOLE_IMAGE_PS3_X, CONSOLE_IMAGE_PS3_Y, skWhite } },
               //kObj_PS3ImageInner
               { kSprt, kObj_PS3Image, { 0, 0, skPS3SpriteBackgroundColor, 112, 154 }, 0, "transfarring_image_ps3_2" },
               //kObj_PS3ImageOutline
               { kSprt, kObj_PS3Image, { 0, 0, skConsoleSpriteOutlineColor, 112, 154 }, 0, "transfarring_image_ps3_1" },
            //kObj_WavesRoot
            { kNull, kObj_ConsoleImageRoot, { WAVES_X, WAVES_Y, skInvisible } },
               //kObj_WavesOut
               { kAnim, kObj_WavesRoot, { 0, 0, skConsoleSpriteOutlineColor, WAVES_WIDTH, WAVES_HEIGHT }, 0, "transfarring_wave" },
               //kObj_WavesIn
               { kSprt, kObj_WavesRoot, { 0, 0, skInvisible, 96, 96 }, 0, "transfarring_wave_00" },
         
          //kObj_MoreInfoRoot
          { kNull, kObj_IdleStageMenuRoot, { MORE_INFO_ROOT_X, MORE_INFO_ROOT_Y, skWhite } },
            //kObj_MoreInfoButton
            { kSprt, kObj_MoreInfoRoot, { 0, 1, skTextColor, 16, 16 }, 0, "transfarring_button_triangle" },
            //kObj_MoreInfoText
            { kBpText, kObj_MoreInfoRoot, { 20, 0, skTextColor, 500, 90 }, kString_TutorialLabel, 0, 0.45f },

          //kObj_ConnectionBox
         { kNull, kObj_IdleStageMenuRoot, { CONNECTION_BOX_X, CONNECTION_BOX_Y, skInvisible } },
            //kObj_ConnectionBoxBackground
            { kFBox, kObj_ConnectionBox, { 0, 0, skConnectionBoxBackgroundColor, 214, 50 } },
            //kObj_ConnectionText 
            { kText, kObj_ConnectionBox, { 28, 28, skTextColorLight }, kTString_Connection, 0, 0.4f },            //kObj_ConnectionNo  
#if defined(BP_PS3)
            { kSprt, kObj_ConnectionBox, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skInvisible, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT }, 0, "!transfarring_connectivity_no" },
#else
            { kSprt, kObj_ConnectionBox, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skWhite, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT }, 0, "!transfarring_connectivity_no" },
#endif
            //kObj_ConnectionYes   
            { kSprt, kObj_ConnectionBox, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skInvisible, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT }, 0, "!transfarring_connectivity_yes" },
         
         //kObj_MenuRoot
         { kNull, kObj_IdleStageMenuRoot, { MENU_ROOT_X, MENU_ROOT_Y, skWhite } },

            //kObj_Button_0_Root
            { kNull, kObj_MenuRoot, { 0, 0, skWhite } },
               //kObj_Button_0_High
               { kFBox, kObj_Button_0_Root, { 0, 0, skHighlightColor, ITEM_W, ITEM_H } },
               //kObj_Button_0_Box
               { kOBox, kObj_Button_0_Root, { 0, 0, skLineColor, ITEM_W, ITEM_H } },
               //kObj_Button_0_Text_Cloud
               { kText, kObj_Button_0_Root, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor }, kTString_ManageSaves, 0, 0.5 },
               //kObj_Button_0_Text_Wifi
               { kText, kObj_Button_0_Root, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skInvisible }, kTString_WifiTransfarring, 0, 0.5 },

            //kObj_Button_1_Root
            { kNull, kObj_MenuRoot, { 0, 34, skWhite } },
               //kObj_Button_1_High  
               { kFBox, kObj_Button_1_Root, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } },
               //kObj_Button_1_Box
               { kOBox, kObj_Button_1_Root, { 0, 0, skLineColor, ITEM_W, ITEM_H } },
               //kObj_Button_1_Text_Listen
#if defined(BP_PS3)
               { kText, kObj_Button_1_Root, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor }, kTString_ListenForVita, 0, 0.5 },
#else
               { kText, kObj_Button_1_Root, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor }, kTString_ConnectToPs3, 0, 0.5 },
#endif
               //kObj_Button_1_Text_Stop
#if defined(BP_PS3)
               { kText, kObj_Button_1_Root, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skInvisible }, kTString_StopListeningForVita, 0, 0.5 },
#else
               { kText, kObj_Button_1_Root, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skInvisible }, kTString_DisconnectFromPs3, 0, 0.5 },
#endif

            //kObj_Button_2_Root
            { kNull, kObj_MenuRoot, { 0, 68, skWhite } },
               //kObj_Button_2_High
               { kFBox, kObj_Button_2_Root, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } },
               //kObj_Button_2_Box
               { kOBox, kObj_Button_2_Root, { 0, 0, skLineColor, ITEM_W, ITEM_H } },
               //kObj_Button_2_Text
               { kText, kObj_Button_2_Root, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor }, kTString_Cancel, 0, 0.5 },

         //kObj_JpExpRoot
         { kNull, kObj_IdleStageMenuRoot, { JP_EXP_X, JP_EXP_Y, skInvisible } },
            //kObj_JpExpText
            { kBpText, kObj_JpExpRoot, { 0, 0, skJpExpColor, 800, 90 }, kTString_Empty, 0, 0.5f, JP_EXP_FLAGS },

      //Wifi Stage
      //kObj_WifiTransfar_Root
      { kNull, kObj_Root, { -20, 0, skInvisible } },
      //kObj_Background_Left
#if defined(BP_PS3)
         { kSprt, kObj_WifiTransfar_Root, { BACKGROUND_ICON_PS3_X, BACKGROUND_ICON_PS3_Y, skInvisible, BACKGROUND_ICON_PS3_WIDTH, BACKGROUND_ICON_PS3_HEIGHT }, 0, "transfarring_ps3icon_large" },
#else
         { kSprt, kObj_WifiTransfar_Root, { BACKGROUND_ICON_VITA_X_LEFT, BACKGROUND_ICON_VITA_Y, skInvisible, BACKGROUND_ICON_VITA_WIDTH, BACKGROUND_ICON_VITA_HEIGHT }, 0, "transfarring_vitaicon_large" },
#endif
         //kObj_Background_Right
         { kSprt, kObj_WifiTransfar_Root, { BACKGROUND_ICON_VITA_X_RIGHT, BACKGROUND_ICON_VITA_Y, skInvisible, BACKGROUND_ICON_VITA_WIDTH, BACKGROUND_ICON_VITA_HEIGHT }, 0, "transfarring_vitaicon_large" },
         //kObj_Background_Cloud
         { kSprt, kObj_WifiTransfar_Root, { BACKGROUND_ICON_CLOUD_X, BACKGROUND_ICON_CLOUD_Y, skInvisible, BACKGROUND_ICON_CLOUD_WIDTH, BACKGROUND_ICON_CLOUD_HEIGHT }, 0, "transfarring_cloudicon_large" },

#if MGS_VERSION == 2
         //kObj_WifiTransfar_BarL1
         { kSprt, kObj_WifiTransfar_Root, { BUTTON_L_X, BUTTON_L_Y, skWhite, BUTTON_L_WIDTH, BUTTON_L_HEIGHT }, 0, "transfarring_switch_l1" },
         //kObj_WifiTransfar_BarR1
         { kSprt, kObj_WifiTransfar_Root, { BUTTON_L_X, BUTTON_L_Y, skWhite, BUTTON_L_WIDTH, BUTTON_L_HEIGHT }, 0, "transfarring_switch_r1" },   
#endif         
         //kObj_WifiTransfar_BarText
         { kText, kObj_WifiTransfar_Root, { GAME_TYPE_X, GAME_TYPE_Y, skTextColor }, kTString_MainGame, 0, 0.5f },

#if MGS_VERSION == 2
         //kObj_WifiTransfar_PsnConnectionStatus_Root
         { kNull, kObj_WifiTransfar_Root, { 403, 43, skWhite } },
            //kObj_WifiTransfar_PsnConnectionStatus_Connected
            { kSprt, kObj_WifiTransfar_PsnConnectionStatus_Root, { 0, 0, skInvisible, 468, 12 }, 0, "!transfarring_psn_connected" },
            //kObj_WifiTransfar_PsnConnectionStatus_Disconnected
            { kSprt, kObj_WifiTransfar_PsnConnectionStatus_Root, { 0, 0, skInvisible, 468, 12 }, 0, "!transfarring_psn_notconnected" },
#else
         //kObj_WifiTransfar_PsnConnectionStatus_Root
          { kNull, kObj_WifiTransfar_Root, { 555, 43, skWhite } },
          //kObj_WifiTransfar_PsnConnectionStatus_Connected
          { kSprt, kObj_WifiTransfar_PsnConnectionStatus_Root, { 0, 0, skInvisible, 325, 12 }, 0, "!transfarring_psn_connected" },
          //kObj_WifiTransfar_PsnConnectionStatus_Disconnected
          { kSprt, kObj_WifiTransfar_PsnConnectionStatus_Root, { 0, 0, skInvisible, 325, 12 }, 0, "!transfarring_psn_notconnected" },
#endif
         
         //kObj_WifiTransfar_LabelPS3
         { kBpText, kObj_WifiTransfar_Root, { SAVE_TABLE_X + SAVE_TABLE_SIDE_TABLE_WIDTH / 2, 76, skTextColor, 700, 90 }, kTString_PS3, 0, 0.5f, kAlignCenter },
         //kObj_WifiTransfar_LabelVita      
         { kBpText, kObj_WifiTransfar_Root, { 692, 76, skTextColor, 700, 90 }, kTString_VITA, 0, 0.5f, kAlignCenter },
         //kObj_WifiTransfar_BottomLineSeparator
#if MGS_VERSION == 2
         { kFBox, kObj_WifiTransfar_Root, { 0, SAVE_TABLE_Y + SAVE_TABLE_HEIGHT, skLineColor, MAX_X_VALUE, 1.5f } },
#else
         { kLine, kObj_WifiTransfar_Root, { 0, SAVE_TABLE_Y + SAVE_TABLE_HEIGHT, skLineColor, MAX_X_VALUE, 1 } },
#endif
         //kObj_WifiTransfar_CommandBox
         { kNull, kObj_WifiTransfar_Root, { SAVE_TABLE_X + SAVE_TABLE_WIDTH - SAVE_TABLE_SIDE_TABLE_WIDTH / 2,
               SAVE_TABLE_Y + SAVE_TABLE_HEIGHT, skWhite} },
            //kObj_WifiTransfar_CommandBoxEdge
#if MGS_VERSION == 2
            { kFBox, kObj_WifiTransfar_CommandBox, { -1, 0, skLineColor, 1.5f, SAVE_TABLE_BOTTOM_TO_BOTTOM_LINE_DISTANCE } },
#else
            { kLine, kObj_WifiTransfar_CommandBox, { 0, 0, skLineColor, 1, SAVE_TABLE_BOTTOM_TO_BOTTOM_LINE_DISTANCE } },
#endif
            //kObj_WifiTransfar_YesNoRoot
            { kNull, kObj_WifiTransfar_CommandBox, { 0, 0, skInvisible } },
               //kObj_WifiTransfar_YesRoot
               { kNull, kObj_WifiTransfar_YesNoRoot, { YES_BUTTON_OFFSET_X, YES_BUTTON_OFFSET_Y, skWhite } },
                  //kObj_WifiTransfar_YesHigh  
                  { kFBox, kObj_WifiTransfar_YesRoot, { 0, 0, skHighlightColor, YES_NO_BUTTONS_WIDTH, YES_NO_BUTTONS_HEIGHT } },
                  //kObj_WifiTransfar_YesBox
                  { kOBox, kObj_WifiTransfar_YesRoot, { 0, 0, skLineColor, YES_NO_BUTTONS_WIDTH, YES_NO_BUTTONS_HEIGHT } },
                  //kObj_WifiTransfar_YesText
                  { kBpText, kObj_WifiTransfar_YesRoot, { 6, 2, skTextColor, 700, 90 }, kTString_Yes, 0, 0.35f },
               //kObj_WifiTransfar_NoRoot
               { kNull, kObj_WifiTransfar_YesNoRoot, { NO_BUTTON_OFFSET_X, NO_BUTTON_OFFSET_Y, skWhite } },
                  //kObj_WifiTransfar_NoHigh
                  { kFBox, kObj_WifiTransfar_NoRoot, { 0, 0, skInvisible, YES_NO_BUTTONS_WIDTH, YES_NO_BUTTONS_HEIGHT } },
                  //kObj_WifiTransfar_NoBox
                  { kOBox, kObj_WifiTransfar_NoRoot, { 0, 0, skLineColor, YES_NO_BUTTONS_WIDTH, YES_NO_BUTTONS_HEIGHT } },
                  //kObj_WifiTransfar_NoText
                  { kBpText, kObj_WifiTransfar_NoRoot, { 6, 2, skTextColor, 700, 90 }, kTString_No, 0, 0.35f },
            
            //kObj_ButtonDisplayRoot
            { kNull, kObj_WifiTransfar_CommandBox, { 0, 0, skWhite } },
               //kObj_ButtonDisplay_X_Root
               { kNull, kObj_ButtonDisplayRoot, { YES_BUTTON_OFFSET_X, YES_BUTTON_OFFSET_Y, skWhite } },
                  //kObj_ButtonDisplay_X_Sprt      
                  { kSprt, kObj_ButtonDisplay_X_Root, { 0, 0, skLockColor, 18, 18 }, 0, "transfarring_button_ex" },
                  //kObj_ButtonDisplay_X_Text
                  { kBpText, kObj_ButtonDisplay_X_Root, { 22, 1, skTextColor, 700, 90 }, kTString_Yes, 0, 0.35f },

               //kObj_ButtonDisplay_SQ_Root
               { kNull, kObj_ButtonDisplayRoot, { NO_BUTTON_OFFSET_X, NO_BUTTON_OFFSET_Y, skWhite } },
                  //kObj_ButtonDisplay_SQ_Sprt      
                  { kSprt, kObj_ButtonDisplay_SQ_Root, { 0, 0, skLockColor, 18, 18 }, 0, "transfarring_button_square" },
                  //kObj_ButtonDisplay_SQ_Text
                  { kBpText, kObj_ButtonDisplay_SQ_Root, { 22, 1, skTextColor, 700, 90 }, kTString_No, 0, 0.35f },
               
         //kObj_WifiTransfar_SaveTable
         { kNull, kObj_WifiTransfar_Root, { SAVE_TABLE_X, SAVE_TABLE_Y, skWhite } },

#if MGS_VERSION == 2
            //kObj_WifiTransfar_LeftBoxTopEdge
            { kLine, kObj_WifiTransfar_SaveTable, { 0, 0, skLineColor, SAVE_TABLE_SIDE_TABLE_WIDTH + 1, 1 } },
            //kObj_WifiTransfar_RightBoxTopEdge
            { kLine, kObj_WifiTransfar_SaveTable, { SAVE_TABLE_WIDTH / 2 + SAVE_TABLE_MID_SECTION_WIDTH / 2, 0, skLineColor, SAVE_TABLE_SIDE_TABLE_WIDTH + 1, 1 } },
#endif
            //kObj_WifiTransfar_LeftBoxLeftEdge
            { kLine, kObj_WifiTransfar_SaveTable, { 0, 0, skLineColor, 1, SAVE_TABLE_HEIGHT } },
            //kObj_WifiTransfar_LeftBoxRightEdge
            { kLine, kObj_WifiTransfar_SaveTable, { SAVE_TABLE_SIDE_TABLE_WIDTH, 0, skLineColor, 1, SAVE_TABLE_HEIGHT } },
            
            //kObj_WifiTransfar_RightBoxLeftEdge
            { kLine, kObj_WifiTransfar_SaveTable, {  SAVE_TABLE_WIDTH / 2 + SAVE_TABLE_MID_SECTION_WIDTH / 2, 0, skLineColor, 1, SAVE_TABLE_HEIGHT } },
            //kObj_WifiTransfar_RightBoxRightEdge
            { kLine, kObj_WifiTransfar_SaveTable, {  SAVE_TABLE_WIDTH / 2 + SAVE_TABLE_MID_SECTION_WIDTH / 2 + SAVE_TABLE_SIDE_TABLE_WIDTH, 0, skLineColor, 1, SAVE_TABLE_HEIGHT } },

            //kObj_LinkedSlot_0
            DEFINE_LINKED_SLOT(0),
            //kObj_LinkedSlot_1
            DEFINE_LINKED_SLOT(1),
            //kObj_LinkedSlot_2
            DEFINE_LINKED_SLOT(2),
            //kObj_LinkedSlot_3
            DEFINE_LINKED_SLOT(3),
            //kObj_LinkedSlot_4
            DEFINE_LINKED_SLOT(4),
            //kObj_LinkedSlot_5
            DEFINE_LINKED_SLOT(5),

            //kObj_WifiTransfar_LeftBox
            { kNull, kObj_WifiTransfar_SaveTable, { 0, 0, skWhite } },
               //kObj_LeftSlot_0
               DEFINE_LEFT_SAVE_SLOT(0),
               //kObj_LeftSlot_1
               DEFINE_LEFT_SAVE_SLOT(1),
               //kObj_LeftSlot_2
               DEFINE_LEFT_SAVE_SLOT(2),
               //kObj_LeftSlot_3
               DEFINE_LEFT_SAVE_SLOT(3),
               //kObj_LeftSlot_4
               DEFINE_LEFT_SAVE_SLOT(4),
               //kObj_LeftSlot_5
               DEFINE_LEFT_SAVE_SLOT(5),
      
          //kObj_WifiTransfar_RightBox
         { kNull, kObj_WifiTransfar_SaveTable, { SAVE_TABLE_WIDTH / 2 + SAVE_TABLE_MID_SECTION_WIDTH / 2, 0, skWhite } },
               //kObj_RightSlot_0
               DEFINE_RIGHT_SAVE_SLOT(0),
               //kObj_RightSlot_1
               DEFINE_RIGHT_SAVE_SLOT(1),
               //kObj_RightSlot_2
               DEFINE_RIGHT_SAVE_SLOT(2),
               //kObj_RightSlot_3
               DEFINE_RIGHT_SAVE_SLOT(3),
               //kObj_RightSlot_4
               DEFINE_RIGHT_SAVE_SLOT(4),
               //kObj_RightSlot_5
               DEFINE_RIGHT_SAVE_SLOT(5),

      //kObj_StatusMessageRoot
      { kNull, kObj_WifiTransfar_Root, { SAVE_TABLE_X, SAVE_TABLE_Y + SAVE_TABLE_HEIGHT + 5, skWhite } },
         //kObj_StatusMessageText
         { kBpText, kObj_StatusMessageRoot, { 0, 0, skTextColor, 1280, 90 },
            kTString_USE_OVERRIDE, 0, 0.4f },
         //kObj_SaveTypeText
         { kBpText, kObj_StatusMessageRoot, { 0, SAVE_TYPE_TEXT_OFFSET_Y, skTextColor, 1280, 90 },
            kTString_USE_OVERRIDE, 0, 0.4f },

      //kObj_LoadingBarRoot      
      { kNull, kObj_Root, { 158, 487, skInvisible } },
         //kObj_LoadingBarBackground      
         { kFBox, kObj_LoadingBarRoot, { 0, 0, skLoadingBarBackgroundColor, 480, 4 } },
         //kObj_LoadingBarForeground
         { kFBox, kObj_LoadingBarRoot, { 0, 0, skLoadingBarForegroundColor, 480, 4 } },
};

SMenuAction gTransfarringMenuActions[] =
{
   //kAction_FadeIn - enter screen action
   {{
      //object action 0
      { kObj_IdleStageMenuRoot, { { kMorph, 10, { 0, 0, skWhite } }, { kEnd } } },
      { kObj_TransfarringLogo, { { kMorph, 10, { TRANSFARRING_ICON_X, TRANSFARRING_ICON_Y, skTransfarringIconColor, TRANSFARRING_ICON_WIDTH, TRANSFARRING_ICON_HEIGHT } }, { kEnd } } },
      //object action 1
      { kObj_None }
   }},
      //kAction_FadeOut - leave screen action
   {{
      { kObj_IdleStageMenuRoot, { { kMorph, 10, { -20, 0, skInvisible } }, { kEnd } } },
      { kObj_TransfarringLogo, { { kMorph, 10, { TRANSFARRING_ICON_X, TRANSFARRING_ICON_Y, skInvisible, TRANSFARRING_ICON_WIDTH, TRANSFARRING_ICON_HEIGHT } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_CursorManage - select button 0 object
   {{
      { kObj_Button_0_High, { { kMorph, 10, { 0, 0, skHighlightColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_1_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_2_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_None }
   }},  
      //kAction_CursorWifi - select button 1 object
   {{
      { kObj_Button_0_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_1_High, { { kMorph, 10, { 0, 0, skHighlightColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_2_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_CursorCancel - select button 2 transfarring object
   {{
      { kObj_Button_0_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_1_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_2_High, { { kMorph, 10, { 0, 0, skHighlightColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_DisconnectedToListening_PS3
   {{
      { kObj_Button_0_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_1_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_2_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_0_Text_Cloud, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skInvisible } }, { kEnd } } },
      { kObj_Button_0_Text_Wifi, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skGreyedOutTextColor } }, { kEnd } } },
      { kObj_Button_1_Text_Listen, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skInvisible } }, { kEnd } } },
      { kObj_Button_1_Text_Stop, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skGreyedOutTextColor } }, { kEnd } } },
      { kObj_Button_2_Text, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skGreyedOutTextColor } }, { kEnd } } },
      { kObj_ConnectionBox, { { kMorph, 10, { CONNECTION_BOX_X, CONNECTION_BOX_Y, skWhite } }, { kEnd } } },
      { kObj_ConnectionNo, { { kMorph, 10, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skWhite, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT } }, { kEnd } } },
      { kObj_ConnectionYes, { { kMorph, 10, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skInvisible, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT } }, { kEnd } } },
      { kObj_WavesRoot, { { kMorph, 10, { WAVES_X, WAVES_Y, skInvisible } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_DisconnectedToListening_PS3_EnableButtons
   {{
      { kObj_Button_1_Text_Stop, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor } }, { kEnd } } },
      { kObj_Button_0_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_1_High, { { kMorph, 10, { 0, 0, skHighlightColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_2_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_ListeningToConnected_PS3
   {{
      { kObj_Button_0_Text_Wifi, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor } }, { kEnd } } },
      { kObj_Button_1_Text_Stop, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor } }, { kEnd } } },
      { kObj_ConnectionNo, { { kMorph, 10, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skInvisible, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT } }, { kEnd } } },
      { kObj_ConnectionYes, { { kMorph, 10, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skWhite, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT } }, { kEnd } } },
      { kObj_Button_0_High, { { kMorph, 10, { 0, 0, skHighlightColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_1_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_2_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_WavesRoot, { { kMorph, 10, { WAVES_X, WAVES_Y, skWhite } }, { kEnd } } },
      { kObj_None }
   }},  
      //kAction_ListeningToDisconnected_PS3
   {{
      { kObj_Button_0_Text_Wifi, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skInvisible } }, { kEnd } } },
      { kObj_Button_0_Text_Cloud, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor } }, { kEnd } } },
      { kObj_Button_1_Text_Stop, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skInvisible } }, { kEnd } } },
      { kObj_Button_1_Text_Listen, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor } }, { kEnd } } },
      { kObj_Button_2_Text, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor } }, { kEnd } } },     
      { kObj_ConnectionBox, { { kMorph, 10, { CONNECTION_BOX_X, CONNECTION_BOX_Y, skInvisible } }, { kEnd } } },
      { kObj_ConnectionNo, { { kMorph, 10, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skInvisible, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT } }, { kEnd } } },
      { kObj_ConnectionYes, { { kMorph, 10, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skInvisible, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT } }, { kEnd } } },
      { kObj_WavesRoot, { { kMorph, 10, { WAVES_X, WAVES_Y, skInvisible } }, { kEnd } } },
      { kObj_Button_0_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_1_High, { { kMorph, 10, { 0, 0, skHighlightColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_2_High, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_None }
   }},  
      //kAction_ConnectedToDisconnected_PS3
   {{
      { kObj_Button_0_Text_Wifi, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skInvisible } }, { kEnd } } },
      { kObj_Button_0_Text_Cloud, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor } }, { kEnd } } },
      { kObj_Button_1_Text_Stop, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skInvisible } }, { kEnd } } },
      { kObj_Button_1_Text_Listen, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor } }, { kEnd } } },
      { kObj_Button_2_Text, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor } }, { kEnd } } },
      { kObj_ConnectionBox, { { kMorph, 10, { CONNECTION_BOX_X, CONNECTION_BOX_Y, skInvisible } }, { kEnd } } },
      { kObj_ConnectionNo, { { kMorph, 10, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skInvisible, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT } }, { kEnd } } },
      { kObj_ConnectionYes, { { kMorph, 10, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skInvisible, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT } }, { kEnd } } },
      { kObj_WavesRoot, { { kMorph, 10, { WAVES_X, WAVES_Y, skInvisible } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_ConnectedToListening_PS3
   {{
      { kObj_Button_0_Text_Wifi, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skGreyedOutTextColor } }, { kEnd } } },
      { kObj_ConnectionNo, { { kMorph, 10, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skWhite, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT } }, { kEnd } } },
      { kObj_ConnectionYes, { { kMorph, 10, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skInvisible, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT } }, { kEnd } } },
      { kObj_WavesRoot, { { kMorph, 10, { WAVES_X, WAVES_Y, skInvisible } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_DisconnectedToConnected_VITA
   {{
      { kObj_Button_0_Text_Cloud, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skGreyedOutTextColor } }, { kEnd } } },
      { kObj_Button_1_Text_Listen, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skInvisible } }, { kEnd } } },
      { kObj_Button_1_Text_Stop, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor } }, { kEnd } } },
      { kObj_Button_2_Text, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skGreyedOutTextColor } }, { kEnd } } },     
      { kObj_ConnectionBox, { { kMorph, 10, { CONNECTION_BOX_X, CONNECTION_BOX_Y, skWhite } }, { kEnd } } },
      { kObj_ConnectionNo, { { kMorph, 10, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skInvisible, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT } }, { kEnd } } },
      { kObj_ConnectionYes, { { kMorph, 10, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skWhite, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT } }, { kEnd } } },
      { kObj_WavesRoot, { { kMorph, 10, { WAVES_X, WAVES_Y, skWhite } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_ConnectedToDisconnected_VITA
   {{
      { kObj_Button_0_Text_Cloud, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor } }, { kEnd } } },
      { kObj_Button_1_Text_Stop, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skInvisible } }, { kEnd } } },
      { kObj_Button_1_Text_Listen, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor } }, { kEnd } } },
      { kObj_Button_2_Text, { { kMorph, 10, { MAIN_MENU_BUTTON_TEXT_OFFSET_X, MAIN_MENU_BUTTON_TEXT_OFFSET_Y, skTextColor } }, { kEnd } } },     
      { kObj_ConnectionBox, { { kMorph, 10, { CONNECTION_BOX_X, CONNECTION_BOX_Y, skInvisible } }, { kEnd } } },
      { kObj_ConnectionNo, { { kMorph, 10, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skWhite, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT } }, { kEnd } } },
      { kObj_ConnectionYes, { { kMorph, 10, { CONNECTION_BOX_SPRITE_X, CONNECTION_BOX_SPRITE_Y, skInvisible, CONNECTION_BOX_SPRITE_WIDTH, CONNECTION_BOX_SPRITE_HEIGHT } }, { kEnd } } },
      { kObj_WavesRoot, { { kMorph, 10, { WAVES_X, WAVES_Y, skInvisible } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_EnterWifiTransfarState    
   {{
      { kObj_IdleStageMenuRoot, { { kMorph, 10, { 20, 0, skInvisible } }, { kEnd } } },
      { kObj_WifiTransfar_Root, { { kMorph, 10, { 0, 0, skWhite } }, { kEnd } } },
      { kObj_TransfarringLogo, { { kMorph, 10, { TRANSFARRING_ICON_X, TRANSFARRING_ICON_Y, skTransfarringIconColorFaded, TRANSFARRING_ICON_WIDTH, TRANSFARRING_ICON_HEIGHT } }, { kEnd } } },
#if defined(BP_PS3)
      { kObj_Background_Left, { { kMorph, 10, { BACKGROUND_ICON_PS3_X, BACKGROUND_ICON_PS3_Y, skBackgroundIconColor, BACKGROUND_ICON_PS3_WIDTH, BACKGROUND_ICON_PS3_HEIGHT } }, { kEnd } } },
#else
      { kObj_Background_Left, { { kMorph, 10, { BACKGROUND_ICON_VITA_X_LEFT, BACKGROUND_ICON_VITA_Y, skBackgroundIconColor, BACKGROUND_ICON_VITA_WIDTH, BACKGROUND_ICON_VITA_HEIGHT } }, { kEnd } } },
#endif
      { kObj_Background_Right, { { kMorph, 10, { BACKGROUND_ICON_VITA_X_RIGHT, BACKGROUND_ICON_VITA_Y, skBackgroundIconColor, BACKGROUND_ICON_VITA_WIDTH, BACKGROUND_ICON_VITA_HEIGHT } }, { kEnd } } },
#if MGS_VERSION == 2
      { kObj_BackgroundCover, { { kMorph, 10, { 0, 0, skBackgroundCoverColor, MAX_X_VALUE, MAX_Y_VALUE } }, { kEnd } } },
#endif      
      { kObj_None }
   }},
      //kAction_LeaveWifiTransfarState
   {{
      { kObj_WifiTransfar_Root, { { kMorph, 10, { -20, 0, skInvisible } }, { kEnd } } },
      { kObj_IdleStageMenuRoot, { { kMorph, 10, { 0, 0, skWhite } }, { kEnd } } },
      { kObj_Button_0_High, { { kSet, 0, { 0, 0, skHighlightColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_1_High, { { kSet, 0, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_2_High, { { kSet, 0, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_TransfarringLogo, { { kMorph, 10, { TRANSFARRING_ICON_X, TRANSFARRING_ICON_Y, skTransfarringIconColor, TRANSFARRING_ICON_WIDTH, TRANSFARRING_ICON_HEIGHT } }, { kEnd } } },
#if defined(BP_PS3)
      { kObj_Background_Left, { { kMorph, 10, { BACKGROUND_ICON_PS3_X, BACKGROUND_ICON_PS3_Y, skInvisible, BACKGROUND_ICON_PS3_WIDTH, BACKGROUND_ICON_PS3_HEIGHT } }, { kEnd } } },
#else
      { kObj_Background_Left, { { kMorph, 10, { BACKGROUND_ICON_VITA_X_LEFT, BACKGROUND_ICON_VITA_Y, skInvisible, BACKGROUND_ICON_VITA_WIDTH, BACKGROUND_ICON_VITA_HEIGHT } }, { kEnd } } },
#endif
      { kObj_Background_Right, { { kMorph, 10, { BACKGROUND_ICON_VITA_X_RIGHT, BACKGROUND_ICON_VITA_Y, skInvisible, BACKGROUND_ICON_VITA_WIDTH, BACKGROUND_ICON_VITA_HEIGHT } }, { kEnd } } },
#if MGS_VERSION == 2
      { kObj_BackgroundCover, { { kMorph, 10, { 0, 0, skInvisible, MAX_X_VALUE, MAX_Y_VALUE } }, { kEnd } } },
#endif      
      { kObj_None }
   }},
      //kAction_EnterManageSavesState    
   {{
      { kObj_IdleStageMenuRoot, { { kMorph, 10, { 20, 0, skInvisible } }, { kEnd } } },
      { kObj_WifiTransfar_Root, { { kMorph, 10, { 0, 0, skWhite } }, { kEnd } } },
      { kObj_TransfarringLogo, { { kMorph, 10, { TRANSFARRING_ICON_X, TRANSFARRING_ICON_Y, skTransfarringIconColorFaded, TRANSFARRING_ICON_WIDTH, TRANSFARRING_ICON_HEIGHT } }, { kEnd } } },
#if defined(BP_PS3)
      { kObj_Background_Left, { { kMorph, 10, { BACKGROUND_ICON_PS3_X, BACKGROUND_ICON_PS3_Y, skBackgroundIconColor, BACKGROUND_ICON_PS3_WIDTH, BACKGROUND_ICON_PS3_HEIGHT } }, { kEnd } } },
#else
      { kObj_Background_Left, { { kMorph, 10, { BACKGROUND_ICON_VITA_X_LEFT, BACKGROUND_ICON_VITA_Y, skBackgroundIconColor, BACKGROUND_ICON_VITA_WIDTH, BACKGROUND_ICON_VITA_HEIGHT } }, { kEnd } } },
#endif
      { kObj_Background_Cloud, { { kMorph, 10, { BACKGROUND_ICON_CLOUD_X, BACKGROUND_ICON_CLOUD_Y, skBackgroundIconColorCloud, BACKGROUND_ICON_CLOUD_WIDTH, BACKGROUND_ICON_CLOUD_HEIGHT } }, { kEnd } } },
#if MGS_VERSION == 2
      { kObj_BackgroundCover, { { kMorph, 10, { 0, 0, skBackgroundCoverColor, MAX_X_VALUE, MAX_Y_VALUE } }, { kEnd } } },
#endif      
      { kObj_None }
   }},
      //kAction_LeaveManageSavesState
   {{
      { kObj_WifiTransfar_Root, { { kMorph, 10, { -20, 0, skInvisible } }, { kEnd } } },
      { kObj_IdleStageMenuRoot, { { kMorph, 10, { 0, 0, skWhite } }, { kEnd } } },
      { kObj_Button_0_High, { { kSet, 0, { 0, 0, skHighlightColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_1_High, { { kSet, 0, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_Button_2_High, { { kSet, 0, { 0, 0, skSaveSlotNormalColor, ITEM_W, ITEM_H } }, { kEnd } } },
      { kObj_TransfarringLogo, { { kMorph, 10, { TRANSFARRING_ICON_X, TRANSFARRING_ICON_Y, skTransfarringIconColor, TRANSFARRING_ICON_WIDTH, TRANSFARRING_ICON_HEIGHT } }, { kEnd } } },
#if defined(BP_PS3)
      { kObj_Background_Left, { { kMorph, 10, { BACKGROUND_ICON_PS3_X, BACKGROUND_ICON_PS3_Y, skInvisible, BACKGROUND_ICON_PS3_WIDTH, BACKGROUND_ICON_PS3_HEIGHT } }, { kEnd } } },
#else
      { kObj_Background_Left, { { kMorph, 10, { BACKGROUND_ICON_VITA_X_LEFT, BACKGROUND_ICON_VITA_Y, skInvisible, BACKGROUND_ICON_VITA_WIDTH, BACKGROUND_ICON_VITA_HEIGHT } }, { kEnd } } },
#endif
      { kObj_Background_Cloud, { { kMorph, 10, { BACKGROUND_ICON_CLOUD_X, BACKGROUND_ICON_CLOUD_Y, skInvisible, BACKGROUND_ICON_CLOUD_WIDTH, BACKGROUND_ICON_CLOUD_HEIGHT } }, { kEnd } } },
#if MGS_VERSION == 2
      { kObj_BackgroundCover, { { kMorph, 10, { 0, 0, skInvisible, MAX_X_VALUE, MAX_Y_VALUE } }, { kEnd } } },
#endif      
      { kObj_None }
   }},
      //kAction_WifiTransfar_SaveSelect
   {{
      { kObj_RightSlotHigh_0, { { kMorph, 10, { SAVE_SLOT_HIGHLIGHT_OFFSET_X, 0, skSaveSlotNormalColor, SAVE_SLOT_HIGHLIGHT_WIDTH, SAVE_SLOT_HEIGHT / 2 } }, { kEnd } } },
      { kObj_RightSlotHigh_1, { { kMorph, 10, { SAVE_SLOT_HIGHLIGHT_OFFSET_X, 0, skSaveSlotHighlightColor, SAVE_SLOT_HIGHLIGHT_WIDTH, SAVE_SLOT_HEIGHT / 2 } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_WifiTransfar_DisplayYesNo
   {{
      { kObj_WifiTransfar_YesHigh, { { kSet, 0, { 0, 0, skHighlightColor, YES_NO_BUTTONS_WIDTH, YES_NO_BUTTONS_HEIGHT } }, { kEnd } } },
      { kObj_WifiTransfar_NoHigh, { { kSet, 0, { 0, 0, skSaveSlotNormalColor, YES_NO_BUTTONS_WIDTH, YES_NO_BUTTONS_HEIGHT } }, { kEnd } } },
      { kObj_WifiTransfar_YesNoRoot, { { kMorph, 0, { 0, 0, skWhite } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_WifiTransfar_HideYesNo
   {{
      { kObj_WifiTransfar_YesNoRoot, { { kMorph, 0, { 0, 0, skInvisible } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_WifiTransfar_SelectYes
   {{
      { kObj_WifiTransfar_YesHigh, { { kMorph, 10, { 0, 0, skHighlightColor, YES_NO_BUTTONS_WIDTH, YES_NO_BUTTONS_HEIGHT } }, { kEnd } } },
      { kObj_WifiTransfar_NoHigh, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, YES_NO_BUTTONS_WIDTH, YES_NO_BUTTONS_HEIGHT } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_WifiTransfar_SelectNo
   {{
      { kObj_WifiTransfar_YesHigh, { { kMorph, 10, { 0, 0, skSaveSlotNormalColor, YES_NO_BUTTONS_WIDTH, YES_NO_BUTTONS_HEIGHT } }, { kEnd } } },
      { kObj_WifiTransfar_NoHigh, { { kMorph, 10, { 0, 0, skHighlightColor, YES_NO_BUTTONS_WIDTH, YES_NO_BUTTONS_HEIGHT } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_WifiTransfar_FadeLoadingBarOut
   {{
      { kObj_LoadingBarForeground, { { kMorph, 20, { 0, 0, skLoadingBarBackgroundColor, 480, 4 } }, { kEnd } } },
      { kObj_None }
   }},
      //kAction_WifiTransfar_FadeLoadingBarIn
   {{
      { kObj_LoadingBarForeground, { { kMorph, 20, { 0, 0, skLoadingBarForegroundColor, 480, 4 } }, { kEnd } } },
      { kObj_None }
   }},
};

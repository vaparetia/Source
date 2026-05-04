//------------------------------------------------------------------------------------------
// BP_RichPresenceSystem.h
//
//------------------------------------------------------------------------------------------

#pragma once

// Note: if changing this match names in BP_GetActualRichPresenceString
enum ERichPresence
{
   kRP_InTitleSelect             =  0,
   kRP_MGS2_Idle                 =  1,
   kRP_MGS2_MAINMENU             =  2,
   kRP_MGS2_SNAKETALES           =  3,
   kRP_MGS2_VRMISSION            =  4,
   kRP_MGS2_ALTMISSION           =  5,
   kRP_MGS2_TANKER               =  6,
   kRP_MGS2_PLANT                =  7,
   kRP_MGS3_MAINMENU             =  8,
   kRP_MGS3_VIRTUOUS             =  9,
   kRP_MGS3_SNAKEEATER           =  10,
   kRP_MGS3_DEMOTHEATER          =  11,
   kRP_MGS3_MG1                  =  12,
   kRP_MGS3_MG2                  =  13,
   kRP_MGS3_Idle                 =  14,
};  

#ifdef __cplusplus
extern "C" {
#endif

class CColor;

ERichPresence BP_GetActualRichPresence();
const char* BP_GetActualRichPresenceStringAndColor(CColor& outRPColor);
void BP_RichPresenceSystem_SetActiveContext(ERichPresence activePresence);
void BP_RichPresenceSystem_HeartBeat();

#ifdef __cplusplus
};
#endif

//------------------------------------------------------------------------------------------

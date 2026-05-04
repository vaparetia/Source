//----------------------------------------------------------------------------
// BP_LocalizedTextByEnum.h
//----------------------------------------------------------------------------

#pragma once

#define BP_LOCALIZEDTEXT_ENUM_START (0x10000)
#define BP_LOCALIZEDTEXT_ENUM_START_PLATFORM (0x20000)

#ifdef __cplusplus
extern "C" {
#endif

//NPL - Need proper localization
//NPF - Need proper font

enum PlatformLocalizeString
{
   kBP_PLS_Saving = BP_LOCALIZEDTEXT_ENUM_START_PLATFORM,
   kBP_PLS_SavingShort,
   kBP_PLS_CheckingSlot12,
   kBP_PLS_CheckingSlot12Detail,

   kBP_PLS_NoData,

   kBP_PLS_StorageDevice,

   kBP_PLS_CreateNewSave,

   // Total
   kBP_PLS_Count,
};

enum GenericLocalizeString
{
   kBP_GLS_Options_ExplainRealTimeCutScenes = BP_LOCALIZEDTEXT_ENUM_START,
   kBP_GLS_Options_ExplainBackToTitleScreen,

   kBP_GLS_Options_ExplainDifficultyVeryEasy,
   kBP_GLS_Options_ExplainDifficultyEasy,
   kBP_GLS_Options_ExplainDifficultyNormal,
   kBP_GLS_Options_ExplainDifficultyHard,
   kBP_GLS_Options_ExplainDifficultyExtreme,
   kBP_GLS_Options_ExplainDifficultyExtremeExtreme,

   kBP_GLS_DamagedSave,

   kBP_GLS_SaveFailed,
   kBP_GLS_Overwrite_Save,
   kBP_GLS_SaveSuccessful,
   kBP_GLS_Loading,
   kBP_GLS_LoadSuccess,
   kBP_GLS_LoadFail,
   kBP_GLS_Yes,
   kBP_GLS_No,

   // Total
   kBP_GLS_Count,
};



extern char* BP_GetStringForEnum( unsigned int id );

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------
// BP_CommonDialog.h
//
// Common dialog system
//----------------------------------------------------------------------------

#pragma once

typedef enum ECommonDialogResult_s
{
   kCDR_Invalid, // Invalid result (ie dialog is still running)
   kCDR_None, // No result was given by the user
   kCDR_OK, // User pressed "OK" or "Yes"
   kCDR_No, // User pressed "No"
   kCDR_Cancel, // User canceled dialog
   kCDR_MaxNumResults,
} ECommonDialogResult;

typedef enum EMessageDialogLayout_s
{
   kMDL_YesNo, // Yes/No buttons
   kMDL_OK, // OK button
   kMDL_MaxNumLayouts,
} EMessageDialogLayout;

#ifdef __cplusplus
extern "C" {
#endif

// Functions for starting the dialog
unsigned long long BP_CommonDialog_WantsMessageDialog( char const * const pMessageString, EMessageDialogLayout const layout );
unsigned long long BP_CommonDialog_WantsErrorDialog( int const errorCode );

// You only need to call these if you care about the result, otherwise you can fire-and-forget
int BP_CommonDialog_IsStillProcessing( unsigned long long const handle );
ECommonDialogResult BP_CommonDialog_GetResult( unsigned long long const handle );

void BP_CommonDialog_Heartbeat();

#ifdef __cplusplus
};
#endif
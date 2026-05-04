//----------------------------------------------------------------------------
// BP_CommonDialogVTA.cpp
//----------------------------------------------------------------------------

#include "Engine/StdAfx.h"
#include "BP_CommonDialog.h"

#include "message_dialog.h"

namespace
{
   const unsigned long long skDialog_InvalidHandle = 0ULL;

   // Handle to the current request
   unsigned long long sDialog_CurHandle = 0ULL;

   bool sDialog_IsProcessing = false;

   // Work input
   SceChar8 sDialogWorkInput_MessageString[SCE_MSG_DIALOG_USER_MSG_SIZE];

   // Work output
   ECommonDialogResult sDialogWorkOutput_Result = kCDR_Invalid;
}

// Local functions
static unsigned long long FinalizePostedWork();
static ECommonDialogResult VitaButtonIDToResult( SceMsgDialogButtonId const buttonID );

extern "C" unsigned long long BP_CommonDialog_WantsMessageDialog( char const * const pMessageString, EMessageDialogLayout const layout )
{
   BPE_VERIFY( strlen( pMessageString ) < SCE_MSG_DIALOG_USER_MSG_SIZE - 1, false, "BP_CommonDialogVTA: User message size too large" );

   strcpy( (char*)sDialogWorkInput_MessageString, pMessageString );

   SceMsgDialogUserMessageParam messageParam;
   memset( &messageParam, 0, sizeof( messageParam ) );

   SceMsgDialogParam dialogParam;
   sceMsgDialogParamInit( &dialogParam );
   dialogParam.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
   dialogParam.userMsgParam = &messageParam;
   dialogParam.userMsgParam->msg = sDialogWorkInput_MessageString;
   dialogParam.userMsgParam->buttonType = layout == kMDL_YesNo ? SCE_MSG_DIALOG_BUTTON_TYPE_YESNO : SCE_MSG_DIALOG_BUTTON_TYPE_OK;

   int ret = sceMsgDialogInit( &dialogParam );

   if( ret == SCE_OK )
   {
      return FinalizePostedWork();
   }
   else
   {
      return skDialog_InvalidHandle;
   }
}

extern "C" unsigned long long BP_CommonDialog_WantsErrorDialog( int const errorCode )
{
   SceMsgDialogErrorCodeParam errorParam;
   memset( &errorParam, 0, sizeof( errorParam ) );

   SceMsgDialogParam dialogParam;
   sceMsgDialogParamInit( &dialogParam );
   dialogParam.mode = SCE_MSG_DIALOG_MODE_ERROR_CODE;
   dialogParam.errorCodeParam = &errorParam;
   dialogParam.errorCodeParam->errorCode = errorCode;

   int ret = sceMsgDialogInit( &dialogParam );

   if( ret == SCE_OK )
   {
      return FinalizePostedWork();
   }
   else
   {
      return skDialog_InvalidHandle;
   }
}

extern "C" int BP_CommonDialog_IsStillProcessing( unsigned long long const handle )
{
   if( handle != skDialog_InvalidHandle && sDialog_CurHandle == handle )
   {
      return sDialog_IsProcessing;
   }
   else
   {
      return 0;
   }
}

extern "C" ECommonDialogResult BP_CommonDialog_GetResult( unsigned long long const handle )
{
   ECommonDialogResult result = kCDR_Invalid;

   // Result is invalid if the handle is bad, the wrong handle was passed in, or the dialog is still processing
   if( handle != skDialog_InvalidHandle && sDialog_CurHandle == handle && !sDialog_IsProcessing )
   {
      result = sDialogWorkOutput_Result;
   }

   return result;
}

extern "C" void BP_CommonDialog_Heartbeat()
{
   if( BP_CommonDialog_IsStillProcessing( sDialog_CurHandle ) )
   {
      if( sceMsgDialogGetStatus() == SCE_COMMON_DIALOG_STATUS_FINISHED )
      {
         sDialog_IsProcessing = false;

         SceMsgDialogResult dialogResult;
         memset( &dialogResult, 0, sizeof( dialogResult ) );
         int ret = sceMsgDialogGetResult( &dialogResult );

         if( dialogResult.result == SCE_COMMON_DIALOG_RESULT_OK )
         {
            sDialogWorkOutput_Result = VitaButtonIDToResult( dialogResult.buttonId );
         }
         else
         {
            sDialogWorkOutput_Result = kCDR_Invalid;
         }

         sceMsgDialogTerm();
      }
   }
}

unsigned long long FinalizePostedWork()
{
   sDialog_IsProcessing = true;
   sDialogWorkOutput_Result = kCDR_Invalid;

   return ++sDialog_CurHandle;
}

ECommonDialogResult VitaButtonIDToResult( SceMsgDialogButtonId const buttonID )
{
   ECommonDialogResult result = kCDR_Invalid;

   switch( buttonID )
   {
   case SCE_MSG_DIALOG_BUTTON_ID_OK:
      result = kCDR_OK;
      break;
   case SCE_MSG_DIALOG_BUTTON_ID_NO:
      result = kCDR_No;
      break;
   case SCE_MSG_DIALOG_BUTTON_ID_INVALID:
      result = kCDR_None;
      break;
   }

   return result;
}
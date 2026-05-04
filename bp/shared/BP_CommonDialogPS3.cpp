//----------------------------------------------------------------------------
// BP_CommonDialogPS3.cpp
//----------------------------------------------------------------------------

#include "Engine/StdAfx.h"
#include "BP_CommonDialog.h"

#include <cell/error.h>
#include <sysutil/sysutil_msgdialog.h>

namespace
{
   const unsigned long long skDialog_InvalidHandle = 0ULL;

   // Handle to the current request
   unsigned long long sDialog_CurHandle = 0ULL;

   bool sDialog_IsProcessing = false;

   // Work input
   char sDialogWorkInput_MessageString[CELL_MSGDIALOG_STRING_SIZE];

   // Work output
   ECommonDialogResult sDialogWorkOutput_Result = kCDR_Invalid;
}

// Local functions
static unsigned long long FinalizePostedWork();
static unsigned int LayoutToPS3( EMessageDialogLayout const layout );
static void cb_error_dialog_callback( int buttonType, void* userData );
static void cb_msg_dialog_callback( int buttonType, void* userData );

extern "C" unsigned long long BP_CommonDialog_WantsMessageDialog( char const * const pMessageString, EMessageDialogLayout const layout )
{
   BPE_VERIFY( strlen( pMessageString ) < CELL_MSGDIALOG_STRING_SIZE - 1, false, "BP_CommonDialogVTA: User message size too large" );

   strcpy( sDialogWorkInput_MessageString, pMessageString );

   unsigned int dialogFlags = 0;
   dialogFlags |= CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR; // Could make the sound configurable, but we're currently only using this for errors
   dialogFlags |= CELL_MSGDIALOG_TYPE_BG_VISIBLE; // Visible background
   dialogFlags |= CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_OFF; // Keep the cancel button turned on
   dialogFlags |= CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_OK; // Default to OK
   dialogFlags |= CELL_MSGDIALOG_TYPE_PROGRESSBAR_NONE; // No progress bar
   dialogFlags |= LayoutToPS3( layout );

   int ret = cellMsgDialogOpen2(dialogFlags, sDialogWorkInput_MessageString, cb_msg_dialog_callback, NULL, NULL );

   if( ret == CELL_OK )
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
   int ret = cellMsgDialogOpenErrorCode( static_cast<unsigned int>( errorCode ), cb_error_dialog_callback, NULL, NULL );

   if( ret == CELL_OK )
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
}

unsigned long long FinalizePostedWork()
{
   sDialog_IsProcessing = true;
   sDialogWorkOutput_Result = kCDR_Invalid;

   return ++sDialog_CurHandle;
}

static void cb_error_dialog_callback( int buttonType, void* userData )
{
   switch ( buttonType ) {
   case CELL_MSGDIALOG_BUTTON_ESCAPE:
      sDialogWorkOutput_Result = kCDR_Cancel;
      break;
   case CELL_MSGDIALOG_BUTTON_OK:
      sDialogWorkOutput_Result = kCDR_OK;
      break;
   case CELL_MSGDIALOG_BUTTON_NO:
      sDialogWorkOutput_Result = kCDR_No;
      break;
   case CELL_MSGDIALOG_BUTTON_NONE:
      sDialogWorkOutput_Result = kCDR_None;
      break;
   case CELL_MSGDIALOG_BUTTON_INVALID:
   default:
      sDialogWorkOutput_Result = kCDR_Invalid;
      break;
   }

   sDialog_IsProcessing = false;
}

static void cb_msg_dialog_callback( int buttonType, void* userData )
{
   switch ( buttonType ) {
   case CELL_MSGDIALOG_BUTTON_ESCAPE:
      sDialogWorkOutput_Result = kCDR_Cancel;
      break;
   case CELL_MSGDIALOG_BUTTON_OK:
      sDialogWorkOutput_Result = kCDR_OK;
      break;
   case CELL_MSGDIALOG_BUTTON_NO:
      sDialogWorkOutput_Result = kCDR_No;
      break;
   case CELL_MSGDIALOG_BUTTON_NONE:
      sDialogWorkOutput_Result = kCDR_None;
      break;
   case CELL_MSGDIALOG_BUTTON_INVALID:
   default:
      sDialogWorkOutput_Result = kCDR_Invalid;
      break;
   }

   sDialog_IsProcessing = false;
}

static unsigned int LayoutToPS3( EMessageDialogLayout const layout )
{
   unsigned int retval = 0;

   switch( layout )
   {
   case kMDL_YesNo:
      retval = CELL_MSGDIALOG_TYPE_BUTTON_TYPE_YESNO;
      break;
   default:
      retval = CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK;
   case kMDL_OK:
      break;
   }

   return retval;
}
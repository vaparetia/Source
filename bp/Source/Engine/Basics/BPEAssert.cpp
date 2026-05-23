//------------------------------------------------------------------------------------------
// BPEAssert.cpp
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Basics/BPEAssert.h"
//------------------------------------------------------------------------------------------

#if BPE_TARGET == BPE_TARGET_WIN32
#include <windows.h>

#pragma BPE_AUTOLINKSYSLIB( user32 )

#endif

#include <stdio.h>
//#include <fcntl.h>

#include "Engine/System/COsContext.h"
#include "Engine/Basics/CEventLog.h"

//----------------------------------------------------------------------------------------------
static int const skDebugPrintBufferSize = 16385;
//----------------------------------------------------------------------------------------------

#if BPE_TARGET == BPE_TARGET_WIN32
bool const gkIsDebuggerPresent = (IsDebuggerPresent() == TRUE);
#else
bool const gkIsDebuggerPresent = true;
#endif

bool bpe_debugger_is_active( void )
{
   return gkIsDebuggerPresent;
}

//----------------------------------------------------------------------------------------------

static TDebugMsgFunc gpUserDebugMsgFunc = NULL;

TDebugMsgFunc bpe_set_debugger_print_hook( TDebugMsgFunc pUserDebugMsgFunc )
{
   TDebugMsgFunc pPrevMsgFunc = gpUserDebugMsgFunc;

   gpUserDebugMsgFunc = pUserDebugMsgFunc;

   return pPrevMsgFunc;
}

//----------------------------------------------------------------------------------------------

static TDebugMsgFunc gpUserConsoleMsgFunc = NULL;

TDebugMsgFunc bpe_set_console_print_hook( TDebugMsgFunc pUserConsoleMsgFunc )
{
   TDebugMsgFunc pPrevMsgFunc = gpUserConsoleMsgFunc;
   gpUserConsoleMsgFunc = pUserConsoleMsgFunc;

   return pPrevMsgFunc;
}


//----------------------------------------------------------------------------------------------
// Note: This function is MSVC specific ...

#if (defined(BPE_DEBUG) && (BPE_TARGET == BPE_TARGET_WIN32))

int __cdecl bpe_debug_heap_block_report( int reportType, char * pMsg, int * returnVal )
{
   bpe_debugger_printf( pMsg );
   
   switch (reportType)
   {
   case _CRT_WARN :
   case _CRT_ERROR :
      *returnVal = 0; // tell CRT to continue normal processing
      return 0;
   case _CRT_ASSERT :
      *returnVal = 1; // tell CRT to invoke the debugger)
      return 1;
   }

   return 1; // tell CRT to NOT continue processing for this block ...
}

#endif
   
//----------------------------------------------------------------------------------------------

void bpe_debug_heap_report_setup( void * const pDebugInfo )
{
#if (defined(BPE_DEBUG) && (BPE_TARGET == BPE_TARGET_WIN32))
   _CrtMemCheckpoint( reinterpret_cast< ::_CrtMemState* >(pDebugInfo) );
   _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );   _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
   _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );  _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
   _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE ); _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
#else
   BPE_UNREF(pDebugInfo);
#endif
}

//----------------------------------------------------------------------------------------------

void bpe_debug_heap_report( void * const pDebugInfo, char const * const pTitle )
{
#if (defined(BPE_DEBUG) && (BPE_TARGET == BPE_TARGET_WIN32))
   bpe_debugger_printf("\n\n*** Memory Leak Detection : start listing for %s ...\n", pTitle);
   _CrtMemDumpAllObjectsSince( reinterpret_cast< ::_CrtMemState* >(pDebugInfo) );
   bpe_debugger_printf("*** Memory Leak Detection : end of listing\n\n");
#else
   BPE_UNREF(pDebugInfo);
   BPE_UNREF(pTitle);
#endif
}

//----------------------------------------------------------------------------------------------
// Default activity is to send logging output to the system debugger and stdout. So, if you're
// running the program and this function is called the parameter will be viewable in the output
// window of MSVC.

#ifndef GOLD_VERSION
void bpe_debugger_printf( char const * const format, ... )
{
   va_list arg;
   va_start( arg, format );
   bpe_debugger_printf_va_list(format, arg);
   va_end( arg );
}
#endif

//----------------------------------------------------------------------------------------------

#ifndef GOLD_VERSION
void bpe_debugger_printf_va_list( char const * const format, va_list arg )
{
   char debuggerBuf[skDebugPrintBufferSize];
#if ( BPE_TARGET != BPE_TARGET_PS3 && BPE_TARGET != BPE_TARGET_RVL && BPE_TARGET != BPE_TARGET_VITA && BPE_TARGET != BPE_TARGET_DREAMCAST)
   vsprintf_s( debuggerBuf, format, arg );
#else
   vsprintf( debuggerBuf, format, arg );   
#endif

   BPE_ASSERT(strlen(debuggerBuf) < sizeof(debuggerBuf), "buffer overrun");

   if ( gpUserDebugMsgFunc != NULL )
   {
      if ( (*gpUserDebugMsgFunc)(debuggerBuf) == 0 )
      {
#if BPE_TARGET == BPE_TARGET_WIN32
         OutputDebugStringA(debuggerBuf);
#else
         printf("%s", debuggerBuf);
#endif
      }
   }
   else
   {
#if BPE_TARGET == BPE_TARGET_WIN32
         OutputDebugStringA(debuggerBuf);
#else
         printf("%s", debuggerBuf);
#endif
   }
}
#endif

//----------------------------------------------------------------------------------------------

#ifndef GOLD_VERSION

void bpe_console_printf( char const * const format, ... )
{
   va_list arg;
   va_start( arg, format );
   bpe_console_printf_va_list(format, arg);
   va_end( arg );
}
#endif

//----------------------------------------------------------------------------------------------

#ifndef GOLD_VERSION
void bpe_console_printf_va_list( char const * const format, va_list arg )
{
   char debuggerBuf[skDebugPrintBufferSize];

#if ( BPE_TARGET != BPE_TARGET_PS3 && BPE_TARGET != BPE_TARGET_RVL && BPE_TARGET != BPE_TARGET_VITA && BPE_TARGET != BPE_TARGET_DREAMCAST)
   vsprintf_s( debuggerBuf, format, arg );
#else
   vsprintf( debuggerBuf, format, arg );   
#endif
   BPE_ASSERT(strlen(debuggerBuf) < sizeof(debuggerBuf), "buffer overrun");
   
   if ( gpUserConsoleMsgFunc != NULL )
   {
      if ( (*gpUserConsoleMsgFunc)(debuggerBuf) == 0 )
      {
         printf( "%s", debuggerBuf );
      }
   }
   else
   {
      printf( "%s", debuggerBuf );
   }
}
#endif

//----------------------------------------------------------------------------------------------

#ifndef GOLD_VERSION
void bpe_debugger_and_console_printf( char const * const format, ... )
{
   va_list arg;
   va_start( arg, format );
   bpe_console_printf_va_list(format, arg);
   va_end( arg );

   va_start( arg, format );
   bpe_debugger_printf_va_list(format, arg);
   va_end( arg );
}
#endif

//----------------------------------------------------------------------------------------------

#ifndef GOLD_VERSION
void bpe_debugger_and_console_printf_va_list( char const * const format, va_list arg )
{
   bpe_console_printf_va_list(format, arg);
   bpe_debugger_printf_va_list(format, arg);
}
#endif
//----------------------------------------------------------------------------------------------

#ifndef GOLD_VERSION
void bpe_message_box(char const * const text, char const * const caption)
{
#if BPE_TARGET == BPE_TARGET_WIN32
   bpe_debugger_printf("MsgBox: %s - %s\n", caption, text);
   if (OsContext())
   {
      ::CloseWindow((HWND)OsContext()->mMainWindow);
   }
   ::MessageBox(NULL, text, caption, MB_OK|MB_TASKMODAL|MB_DEFAULT_DESKTOP_ONLY|MB_SETFOREGROUND|MB_TOPMOST);
#else
   printf("%s\n%s\n", caption, text);
#endif
   return;
}
#endif

//----------------------------------------------------------------------------------------------

void property_load_error_printf(char const * const pFormat, ...)
{
   va_list arg;
   va_start( arg, pFormat );
   bpe_console_printf_va_list(pFormat, arg);
   va_end( arg );

   va_start( arg, pFormat );
   bpe_debugger_printf_va_list(pFormat, arg);
   va_end( arg );
}

//----------------------------------------------------------------------------------------------
// Default activity is to present a standard Windows MessageBox to the user with Ok and Cancel
// options, and return true if the user picks Ok (which is the default).

bool bpe_default_popup_function(char const * const msg )
{
#if BPE_TARGET == BPE_TARGET_WIN32
   bool const bResultOK = (::MessageBox(NULL, msg, "Press OK to trip the debugger...", MB_OKCANCEL|MB_TASKMODAL|MB_DEFAULT_DESKTOP_ONLY|MB_SETFOREGROUND|MB_TOPMOST) == IDOK);
   return bResultOK;
#else
   printf("Press OK to trip the debugger...\n %s", msg);
   return true;
#endif
}

//----------------------------------------------------------------------------------------------
// Actual logging and query functions below utilize these function pointers
// to make the actual calls. This is what allows for user-definition of these
// actions.

static TDebugPopupFunc gpDebugPopupFunc = bpe_default_popup_function;

TDebugPopupFunc bpe_set_assert_popup_func( TDebugPopupFunc newFunc )
{
   TDebugPopupFunc oldFunc = gpDebugPopupFunc;
   gpDebugPopupFunc = newFunc;
   return oldFunc;
}

//----------------------------------------------------------------------------------------------

#pragma warning (push)
#pragma warning (disable : 4996) // 'foo' was declared depreciated

void bpe_add_string_part( char * msgString, int const maxMsgStringSize, int & msgLen, char const * const msgPart )
{
   if (msgLen < maxMsgStringSize )
   {
      const size_t partLen(strlen(msgPart));
      const size_t lenToAdd( static_cast<size_t>(maxMsgStringSize-msgLen) < partLen ? maxMsgStringSize-msgLen : partLen );
      strncat( msgString, msgPart, lenToAdd );
      msgLen += lenToAdd;
   }
}

#pragma warning (pop)

//----------------------------------------------------------------------------------------------

void bpe_log_assert_failure(char const * const fileName, unsigned long lineNum,
                       char const * const prefix, char const * const cond, char const * const msg )
{
   printf(
      "****** Assertion Failure ******\n"
      "%s(%d): %s%s%s%s%s%s\n"
      , fileName, lineNum
      , ((prefix!=NULL) && (*prefix!='\0')) ? prefix : ""
      , ((prefix!=NULL) && (*prefix!='\0')) ? " "    : ""
      , (  (cond!=NULL) &&   (*cond!='\0')) ? cond   : ""
      , (  (cond!=NULL) &&   (*cond!='\0')) ? ""     : ""
      , (   (msg!=NULL) &&    (*msg!='\0')) ? ">>> " : ""
      , (   (msg!=NULL) &&    (*msg!='\0')) ? msg    : "" );
}

//----------------------------------------------------------------------------------------------

bool bpe_query_to_halt_program(char const * const fileName, unsigned long const lineNum,
                          char const * const prefix, char const * const cond, char const * const msg)
{

   printf(
      "****** Verify Failure ******\n"
      "%s(%d): %s%s%s%s%s%s\n"
      , fileName, lineNum
      , ((prefix!=NULL) && (*prefix!='\0')) ? prefix : ""
      , ((prefix!=NULL) && (*prefix!='\0')) ? " "    : ""
      , (  (cond!=NULL) &&   (*cond!='\0')) ? cond   : ""
      , (  (cond!=NULL) &&   (*cond!='\0')) ? ""     : ""
      , (   (msg!=NULL) &&    (*msg!='\0')) ? ">>> " : ""
      , (   (msg!=NULL) &&    (*msg!='\0')) ? msg    : "" );

   if ( 
#if BPE_TARGET == BPE_TARGET_WIN32
      !IsDebuggerPresent() && 
#endif
      gpDebugPopupFunc )
   {
      (*gpDebugPopupFunc)( msg );
   }

   return true;
}

//----------------------------------------------------------------------------------------------
// Add exception handler for PS3
//
// Disabled for GoW to fix linker errors. Not exactly sure where else 
// it's defined, might need later investigation.
//
#if 0 //defined(_PS3)

#include <exception>

namespace std
{
   void std::exception::_Raise() const
   {
      BPE_ASSERTA("_Raise called?");
   }

   void std::exception::_Doraise() const
   {
      BPE_ASSERTA("_Doraise called?");
   }
}
#endif

//----------------------------------------------------------------------------------------------
// We're using BOOST_NO_EXCEPTIONS so we need to implement this function
#ifdef BOOST_NO_EXCEPTIONS

namespace boost
{
   void throw_exception(std::exception const &)
   {
      BPE_ASSERTA("Exception raised.");
   }
}

#endif
//----------------------------------------------------------------------------------------------

void bpe_log_sce_fail( char const *str, int const var )
{
   printf( "SCE FAIL: %s - %d 0x%8.8x\n", str, var );
   BPE_VERIFYA( false, "SCE FAIL" );
}
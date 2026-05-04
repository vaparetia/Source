//------------------------------------------------------------------------------------------
// BPEDebugFuncs.h
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Basics/BPEEnvironment.h"
#include <stdarg.h>

//------------------------------------------------------------------------------------------

typedef bool (*TDebugPopupFunc)(char const * const);
typedef int  (*TDebugMsgFunc)(char const * const);

ENGINE_API TDebugPopupFunc bpe_set_assert_popup_func( TDebugPopupFunc );
ENGINE_API TDebugMsgFunc   bpe_set_debugger_print_hook( TDebugMsgFunc );
ENGINE_API TDebugMsgFunc   bpe_set_console_print_hook( TDebugMsgFunc );

#ifndef GOLD_VERSION

ENGINE_API void            bpe_debugger_printf( char const * const format, ... );
ENGINE_API void            bpe_debugger_printf_va_list( char const * const format, va_list arg );
ENGINE_API void            bpe_console_printf( char const * const format, ... );
ENGINE_API void            bpe_console_printf_va_list( char const * const format, va_list arg );
ENGINE_API void            bpe_debugger_and_console_printf( char const * const format, ... );
ENGINE_API void            bpe_debugger_and_console_printf_va_list( char const * const format, va_list arg );
ENGINE_API void            bpe_message_box(char const * const text, char const * const caption);

#else

#define bpe_debugger_printf(...)
#define bpe_debugger_printf_va_list(...)
#define bpe_console_printf(...)
#define bpe_console_printf_va_list(...)
#define bpe_debugger_and_console_printf(...)
#define bpe_debugger_and_console_printf_va_list(...)
#define bpe_message_box(...)

#endif

ENGINE_API bool            bpe_debugger_is_active();

ENGINE_API void            property_load_error_printf( char const * const format, ... );

ENGINE_API void            bpe_log_assert_failure(char const * const fileName, uint32 lineNum, char const * const prefix, char const * const cond, char const * const msg );
ENGINE_API bool            bpe_query_to_halt_program(char const * const fileName, uint32 lineNum, char const * const prefix, char const * const cond, char const * const msg );
ENGINE_API void            bpe_log_sce_fail( char const *str, int const var );
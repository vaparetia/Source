/*
	KCEJ DEBUG UTILITY for EE header file

	2000/05/12	K.Uehara
	$Id: debugutil.h,v 1.2 2001/01/10 06:33:08 usr01475 Exp $
*/

#ifndef __DEBUGUTIL_H__
#define __DEBUGUTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 例外処理モジュール ( in exception.c ) */

void DEBUG_InitException( void *stacktop );
void DEBUG_InitIOPException( char *module );

void DEBUG_SetExceptionCallback( void (*func)( unsigned long *regs ) );
#define EXCEP_GETREGS( _regs, _no )	*( _regs + (_no) - 1 )
#define EXCEP_SP( _regs )	*(_regs + 29 - 1)

int cprintf( const char *format, ...  );

#if 1 //BP_PS2
#define excep_printf printf
#else
int excep_printf( const char *format, ... );
#endif

/* スタックチェックモジュール ( in stackecheck.c ) */

/* スタックを埋めつくす値 */
#define DEBUG_STACK_INVALID_VALUE	0x12345687

typedef struct _debug_stackcheck_info {
	struct _debug_stackcheck_info *next;
	char *name;
	char *top;
	int size;
} DEBUG_STACKCHECK_INFO;

void DEBUG_SetStackCheck( DEBUG_STACKCHECK_INFO *stack, char *name, void *top, int size );
DEBUG_STACKCHECK_INFO *DEBUG_ReleaseStack( void );
int  DEBUG_GetStackUseSize( void );

#ifdef __cplusplus
}
#endif

#endif /* __DEBUGUTIL_H__ */



/******************************************************************************
 * System	: DemoAct
 * Computer : DOS/V
 * OS		: Microsoft Windows
 * Compiler : Microsoft Visual C++
 * Module	: Win32 Console Application
 */

#ifndef __INC_TARGETLIST__
#define __INC_TARGETLIST__

/******************************************************************************
 * included
 */

/******************************************************************************
 * defined
 */
#ifdef _WINDOWS
#include <windows.h>
#include <SFDWIN.h>
#endif
#ifdef _UNIX
#include <SFDUNIX.h>
#endif
#ifdef _LINUX
#include <SFDLINUX.h>
#endif

/******************************************************************************
 * typedef and struct
 */
typedef struct tagTARGETLIST
{
	char szDemo[_MAX_TEXT];
	char szTarget[_MAX_TEXT];
} TARGETLIST, *PTARGETLIST, *LPTARGETLIST;

/******************************************************************************
 * globals
 */

extern TARGETLIST targetList[];

/*****************************************************************************/

#endif // __INC_TARGETLIST__

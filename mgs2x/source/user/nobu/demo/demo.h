/*
	demo.h
	    デモ定義

	2000/01/13 N.Tanaka
	$Id: demo.h,v 1.1.1.3 2002/11/19 11:46:42 Yoshizawa1 Exp $
*/

#ifndef _INC_DEMO
#define _INC_DEMO    /* #defined if demo.h has been included */

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

/******************************************************************************
 * included
 */

#include "define.h"

/******************************************************************************
 * definitions and typedefs and structures
 */

#define _DEMODEBUG
//#define _DEMOWRITEVRAM
//#define _DEMOTIMEVIEW
//#define _DEMOFRAMEOVERVIEW
//#define _DEMOMEMORYVIEW
//#define _DEMOMEMORYOVERFLOWSTOP

#ifdef DEBUG_MODE
#define DEMO_DEBUG_MODE
#endif

#define DEMO_USEMEMORY 97

#define DMT_EXECUTE       0x00000001
#define DMT_STATEVIEWLOAD 0x00000002
#define DMT_WRITEFILEAVI  0x00000004

#define MAX_STRING  256
#define LOAD_UNIT   (8196*16)

/******************************************************************************
 * exec
 */
#ifdef DEMO_DEBUG_MODE
extern void* DM_ThreadFile(DWORD dwStyle, char* lpstrFileName, int nLoop, char* lpstrDrawString);
#else
extern void* DM_ThreadFile(DWORD dwStyle, char* lpstrFileName);
#endif
/******************************************************************************
 */

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

#endif /* _INC_DEMO */

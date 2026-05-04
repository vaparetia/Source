/*
	common.h
	    汎用関数群ヘッダー

	2000/01/24 N.Tanaka
	$Id: common.h,v 1.1.1.3 2002/11/19 11:46:41 Yoshizawa1 Exp $
*/

#ifndef _INC_COMMON
#define _INC_COMMON    /* #defined if common.h has been included */

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

/******************************************************************************
 * included
 */
#include <stdio.h>

#include <libgraph.h>
#include <libdg.h>

#include "define.h"

/******************************************************************************
 * definitions and typedefs and structures
 */

#define memset(a,b,c) memset((char*)a,b,c)

/******************************************************************************
 * functions
 */

/* common.c */
extern void InitChain(void* lpAddress);
extern void NextChain(void* lpManeger, void* lpChain);
extern void DeleteChain(void* lpManeger, void* lpDelete);


/******************************************************************************
 */

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

#endif /* _INC_COMMON */

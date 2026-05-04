/*++ BUILD Version: 0000     Increment this if a change has global effects

Copyright (c) 1990-2001  Microsoft Corporation

Module Name:

    xtl.h

Abstract:

    Master include file for Xbox applications.

--*/

#ifndef _XTL_
#define _XTL_

#ifndef _INC_XTL
#define _INC_XTL

#define STRICT
#include <windows.h>
#include <mmsystem.h>

#define D3D_OVERLOADS
#include <d3d8.h>
#include <d3dx8.h>

#ifdef _DEBUG
#include <dxerr8.h>
#endif

#include <dsound.h>
#include <dmusici.h>


#include "xbox2win.h"
#include "x2w_util.h"

enum {
	// (íç) èáèòÇ™èdóv
	CLEARCODE_MODE_SONSOFLIBERTY,	// SONS OF LIBERTY
	CLEARCODE_MODE_BOSSRUSH,		// BOSS RUSH
	CLEARCODE_MODE_MISSIONS,		// MISSIONS
} ;
extern void S_MGS2SSaveRankingPassword( const int, const char* );

#endif /* _INC_XTL */
#endif /* _XTL_ */

//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	varclear.c
		gclユーティリティ関数

	2000/07/14 K.Uehara
	$Id: varclear.c,v 1.1.1.3 2002/11/19 11:51:38 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libvu0.h>
#include <libdma.h>
#include <libgraph.h>
#endif

#include "gameheader.h"
#include "libutl.h"

int NewGclVarClear( void )
{
	GCL_InitClearVar();

	return 0;
}


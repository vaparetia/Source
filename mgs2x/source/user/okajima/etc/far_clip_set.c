//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	far_clip_set.c
	ＦＡＲＣＬＩＰセット
	2000/09/06 S.Okajima
	$Id: far_clip_set.c,v 1.1.1.3 2002/11/19 11:47:40 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libmt.h"
#include "libutl.h"
#include "gameheader.h"

extern void DG_SetClipParam( float fNear, float fFar );

int NewFarClipParamSet( void )
{
	int		itemp0;
	int		itemp1;

	if ( GCL_NextStr() == NULL ) return -1 ;
	itemp0 = GCL_GetNextInt() ;
	itemp1 = GCL_GetNextInt() ;

	DG_SetClipParam( (float)itemp0, (float)itemp1 );

	return 1 ;
}

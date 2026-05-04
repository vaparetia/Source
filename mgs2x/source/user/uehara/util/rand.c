//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	rand.c
		ランダム発生

	2000/03/02 K.Uehara
	$Id: rand.c,v 1.1.1.3 2002/11/19 11:51:37 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libvu0.h>
#include <libdma.h>
#include <libgraph.h>
#endif

#include "gameheader.h"

int NewRand( void )
{
	int max;
	max = GCL_GetNextInt();

	return ( ( BP_PS2_rand() >> 8 ) % max );
}





//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	arrayset.c
		command 配列セット[NewArraySet] $r:変数 $i:値 ...

	1999/12/10 K.Uehara
	$Id: arrayset.c,v 1.1.1.3 2002/11/19 11:51:36 Yoshizawa1 Exp $
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

int NewArraySet( void )
{
	GCL_VAR_REF ref;
	int i;

	GCL_GetNextVarRef( &ref );

	for( i = 0; GCL_NextStr() != NULL; i++ ){
		GCL_SetVarRef( &ref, i, GCL_GetNextInt() );
	}
	
	return 0;
}

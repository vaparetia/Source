//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	amb_set.c
	アンビエントカラーセット
	1999/08/17 S.Okajima
	$Id: amb_set.c,v 1.1.1.3 2002/11/19 11:47:40 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"

void *NewAmbientSet( void )
{
	u_char	r;
	u_char	g;
	u_char	b;
	if ( GCL_GetOption( 'c' ) != NULL ) {
		r = ( u_char )GCL_GetNextInt();
		g = ( u_char )GCL_GetNextInt();
		b = ( u_char )GCL_GetNextInt();
		DG_SetAmbient( r,g,b );
	}
	return NewAmbientSet ;	// ＮＵＬＬでない、という意味

}

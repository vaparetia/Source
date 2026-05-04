//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	makky_rain.c
	マッキー雨音セット
	2000/11/01 S.Okajima
	$Id: makky_rain.c,v 1.1.1.3 2002/11/19 11:47:41 Yoshizawa1 Exp $
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

int NewMakkyRainSet( void )
{
	int		itemp0;

	if ( GCL_NextStr() == NULL ) return -1 ;
	itemp0 = GCL_GetNextInt() ;

	switch( itemp0 ){
	  case 0:
		GM_SdSet( SE_JOUCHUU_OFF );
		break;
	  case 1:
		GM_SdSet( SD_A_RAIN_SE1 );
		break;
	}

	return 1 ;
}

//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	eye.c
	    波 - 視線コントロール

	1999/11/10 T.Morita
	$Id: wave_eye.c,v 1.1.1.3 2002/11/19 11:46:40 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include	"libutl.h"
#include	"gameheader.h"

#include	"wave.h"

int WAV_EyeControl( Work *w )
{
    w->eyex = DG_Chanls->eye.m[2][X]*CENTER_DISTANCE + DG_Chanls->eye.m[3][X] ;
    w->eyez = DG_Chanls->eye.m[2][Z]*CENTER_DISTANCE + DG_Chanls->eye.m[3][Z] ;

    if ( DG_Chanls->eye.m[2][1] > 0.23f )
	return 0 ;
    return 1 ;
}




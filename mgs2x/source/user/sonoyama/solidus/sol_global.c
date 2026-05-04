//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   sol_global.c
   ソリダスグローバル関数 

   2001/08/12	M.Sonoyama
   $Id: sol_global.c,v 1.1.1.3 2002/11/19 11:51:06 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"rand.h"
#include	"vertex_animation.h"
#include	"sol_common.h"
#include	"solidus.h"

Work				*SOL_Work ;

/* グローバル */

/* ライフ率 */

float			SOL_LifePercentage( void )
{
	Work		*work ;
	float		v1, v2 ;

	if ( SOL_Work == NULL ) return 100.0F ;

	work = SOL_Work ;
	v1 = ( float )( ( float )work->life_gage.value / ( float )work->life_gage.max ) ;
	v2 = ( float )( ( float )work->life_gage.m9_value / ( float )work->life_gage.m9_max ) ;
	v1 = ( v1 < v2 ) ? v1 : v2 ;
	return ( v1 * 100.0F ) ;
}

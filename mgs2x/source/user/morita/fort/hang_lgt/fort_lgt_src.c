//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_lgt_src.c
   フォーチュン戦専用 中心計算

   2000/01/12 T. Morita
   $Id: fort_lgt_src.c,v 1.1.1.3 2002/11/19 11:46:12 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#define FRT_LGT_MAX_LIGHTS 5

static FVECTOR *FRT_LGT_LightSrc[FRT_LGT_MAX_LIGHTS] ;
static FVECTOR  FRT_LGT_LightDst[FRT_LGT_MAX_LIGHTS] ;
static int      FRT_LGT_n_Lights = 0 ;

FVECTOR **FRT_LGT_SearchLight( FVECTOR *light )
{
    int i, j ;

    ASSERT( light ) ;

    if ( (j = FRT_LGT_n_Lights) )
	for( i=FRT_LGT_MAX_LIGHTS ; --i>=0 ;  )
	    if ( FRT_LGT_LightSrc[i] )
	    {
		if ( FRT_LGT_LightSrc[i] == light )
		    return &FRT_LGT_LightSrc[i] ;
		if ( --j < 0 )
		    break ;
	    }
    return NULL ;
}


void FRT_LGT_GetNearestLight( FVECTOR *center, FVECTOR ***src, FVECTOR **dst, float min )
{
    FVECTOR  v    ;
    int      i, j ;
    float    t    ;

    ASSERT( center && dst && src ) ;

    /* 距離を取るのにSqrtがうざいので,二乗にする */
    min *= min ;
    if ( (j = FRT_LGT_n_Lights) )
	for( i=FRT_LGT_MAX_LIGHTS ; --i>=0 ;  )
	    if ( FRT_LGT_LightSrc[i] )
		if ( FRT_LGT_LightSrc[i]->vz < -5000.0f )
		{
		    _sceVu0SubVector( &v, center, FRT_LGT_LightSrc[i] ) ;
		    t = _sceVu0InnerProduct( &v, &v ) ;
		    if ( t < min )
		    {
			*src = &FRT_LGT_LightSrc[i] ;
			*dst = &FRT_LGT_LightDst[i] ;
			min = t ;
		    }
		    if ( --j < 0 )
			break ;
		}
}

int FRT_LGT_AddLight( FVECTOR *center )
{
    int i ;

    ASSERT( center ) ;

    if ( center->vz < -5000.0f )
	if ( !FRT_LGT_SearchLight( center ) )
	{
	    if ( FRT_LGT_n_Lights > FRT_LGT_MAX_LIGHTS )
		return 0 ;
	    for( i=FRT_LGT_MAX_LIGHTS ; --i>=0 ;  )
		if ( !FRT_LGT_LightSrc[i] )
		{
		    FRT_LGT_n_Lights++ ;
		    FRT_LGT_LightSrc[i] = center ;
		    _sceVu0CopyVector( &FRT_LGT_LightDst[i], center ) ;
		    FRT_LGT_LightDst[i].vx -= FRT_LGT_LightDst[i].vx * 0.1f ;
		    FRT_LGT_LightDst[i].vy -= 2000.0f ;
		    FRT_LGT_LightDst[i].vz += 1000.0f ;
		    return 1 ;
		}
	}
    return 0  ;
}

void FRT_LGT_RemoveLight( FVECTOR *center )
{
    FVECTOR **light ;

    if ( (light = FRT_LGT_SearchLight( center )) )
	*light = NULL, FRT_LGT_n_Lights-- ;
}

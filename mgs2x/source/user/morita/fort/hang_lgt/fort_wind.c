//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_wind.c
   フォーチュン戦専用 爆心地の計算

   2000/01/12 T. Morita
   $Id: fort_wind.c,v 1.1.1.3 2002/11/19 11:46:12 Yoshizawa1 Exp $
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

static FVECTOR FRT_LGT_Center = { 0.0f, 0.0f, 0.0f, 0.0f } ;
static float   FRT_LGT_Magnitude ;
static int     FRT_LGT_Time = 0  ;/* 同一フレームは マグニチュードを取得できる機構に必要 */

/* 爆心地を取得する。成功すると１を返し 大きさと中心位置を返戻する */
int FRT_LGT_GetMagnitudeCenter( FVECTOR *center, float *magnitude )
{
    ASSERT( center && magnitude ) ;

    if ( FRT_LGT_Time == GV_Time || FRT_LGT_Center.vw != 0.0f )
    {
	_sceVu0CopyVectorXYZ( center, &FRT_LGT_Center ) ;
	*magnitude = FRT_LGT_Magnitude ;

	FRT_LGT_Time = GV_Time ;  /* 同一フレームは,有効にする  */
	FRT_LGT_Center.vw = 0.0f ;/* マグニチュードを無効にする */
	return 1 ;
    }
    return 0  ;
}

/* 自分の *magnitudeの値より大きな爆心地を取得する。
   成功すると１を返し 大きさと中心位置を返戻する */
int FRT_LGT_GetMagnitude( FVECTOR *pos, FVECTOR *vel, float *magnitude )
{
    FVECTOR center ;
    float   current = 0.0f ;

    ASSERT( vel && pos && magnitude ) ;

    if ( FRT_LGT_GetMagnitudeCenter( &center, &current ) )
    {
	_sceVu0SubVector( &center, &center, pos ) ;
	current /= sceVu0Sqrt( _sceVu0InnerProduct( &center, &center ) ) ;
	if ( current > *magnitude )
	{
	    *magnitude = current ;
	    _sceVu0Normalize( vel, &center ) ;
	    return 1 ;
	}
    }
    return 0  ;
}

void FRT_LGT_SetMagnitude( FVECTOR *center, float magnitude )
{
    if ( center )
    {
	_sceVu0CopyVectorXYZ( &FRT_LGT_Center, center ) ;
	FRT_LGT_Magnitude = magnitude ;
	FRT_LGT_Center.vw = 1.0f ;/* マグニチュードが有効なものであることを示す */
    }
}

//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_b_gls_hzx.c
   大ガラス壊れ 当たり判定用ルーチン

   1999/12/15 T. Morita
   $Id: brk_b_gls_hzd.c,v 1.1.1.3 2002/11/19 11:45:21 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_big_glass.h"

static int HZX_BoxCheck( HZD_BOX *t, FVECTOR *pos, FVECTOR *pos_v, float bounce )
{
    FVECTOR v, p ;
    int     i ;
    float l, d, *s = (float *)pos_v ;

    _sceVu0SubVector( &p, pos, &t->center ) ;   /* 回転なしの場合 */
    _sceVu0AddVector( &v, &p, pos_v ) ;         /* pos を箱のローカル座標に変換する       */
    _sceVu0MulVector( &v, &v, &t->size_inv ) ;  /* pos を箱のローカル座標に変換する       */


    /* 箱の内側？ */
    if ( !(int)v.vx && !(int)v.vy && !(int)v.vz )
	for ( i=XYZ ; --i>=0 ; ) /* 各軸に対して平面の交点を調べる（ここからワールド座標で計算） */
	    if ( s[i] != 0.0f ) /* 0 だったら平面と直線は平行      */
	    {
		/* これで平面と交点が媒介変数lで表せる    */
		l  = (((float*)&p)[i] + (s[i]>0.0f ? t->size[i] : -t->size[i])) / s[i] ;
		if ( l >= -1.0f && l <= 1.0f )  /* 本来は 0.0 < l < 1.0だがメリ込み防止で */
		{
		    _sceVu0ScaleVector( &v, pos_v, -l ) ;
		    _sceVu0AddVector( pos, pos, &v ) ;/* posを当たった場所にする */

		    s[i] -= (bounce + 0.125f*frnd()) * s[i] ;
		    d = s[i] * 0.25f ;
		    s[i==X ? Y : i==Y ? X : X] += d * frnd() ;/*ランダムに跳ねる*/
		    s[i==X ? Z : i==Y ? Z : Y] += d * frnd() ;/*ランダムに跳ねる*/

		    return i==Y ? 1/*床*/ : 2/*壁*/ ;
		}
	    }

    return 0 ;
}

int BRK_BGLS_HzdCheck( Work *work, FVECTOR *pos, FVECTOR *pos_v, float bounce )
{
    int i, flag = 0 ;

    if ( pos->vy < 950.0f )
	for ( i=work->n_hzd ; --i>=0 ; )
	    if ( (flag=HZX_BoxCheck( &work->hzd[i], pos, pos_v, bounce )) )
		break ;
    return flag ;
}

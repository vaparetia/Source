//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_sgr.c 
   フォーチュン 武器関数(手榴弾)

   2000/01/11 T.Morita
   $Id: fort_hzd.c,v 1.4 2002/04/24 04:12:46 usr04098 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"
// for DG-LIB
#include "../../include/libdg_x.h"


#include "fort_wep.h"


#define FORT_HZX_S_SPHERE 500

int FRT_HzdSegLineCheck( HZX_SEG *seg, FVECTOR *p1, FVECTOR *p2 )
{
    FVECTOR p ;
    float sx, sz, tx, tz ;
    float r, s, rd ;

    _sceVu0SubVector( &p, p2, p1 ) ;
    sx = seg->p2.x - seg->p1.x ; sz = seg->p2.z - seg->p1.z ;
    if ( (rd = sx*p.vz - sz*p.vx) == 0.0f )                                
	return 0 ;
    tz = seg->p1.z - p1->vz ;
    tx = seg->p1.x - p1->vx ;
    r = (tz*p.vx - tx*p.vz)/rd ;
    if ( r > 1.0f || r < 0.0f )               
	return 0 ;
    s = (tz*sx - tx*sz)/rd ;
    if ( s > 1.0f || s < 0.0f )
	return  0 ;
    _sceVu0ScaleVector( &p, &p, r ) ;
    _sceVu0AddVector( &p, &p, p1 ) ;
    if ( seg->p1.y > p.vy || seg->p1.y+seg->p1.h < p.vy ||
	 seg->p2.y > p.vy || seg->p1.y+seg->p2.h < p.vy )
	return  0 ;
    return 1 ;
}

void FRT_HzdSegReactVector( HZX_SEG *seg, FVECTOR *p  )
{
    float dx, dz, f ;

    dx =  seg->p2.x - seg->p1.x ;
    dz =  seg->p2.z - seg->p1.z ;
    f = 2*( dz*p->vx - dx*p->vz )/(dx*dx + dz*dz) ;
    p->vx -= dz * f ;
    p->vz += dx * f ;
}

int FRT_HzdCheck( HZX_GROUP_ID hzx, FVECTOR *pos, FVECTOR *pos_v,
		  float min, FVECTOR *bounce, float sphere )
{
    int   atr[2] ;
    float flr[2] ;
    HZX_SEG seg[2] ;
    int flag = 0 ;
    FVECTOR new ;

    _sceVu0AddVector( &new, pos, pos_v ) ;
    if ( HZX_NearHazardCheck( hzx, pos, FORT_HZX_S_SPHERE, HZX_CHK_ALL, HZX_SEG_NO_BULLET, (int)sphere ) )
    {
        HZX_GetNearHazard( seg, atr ) ;
        if ( FRT_HzdSegLineCheck( seg, pos, &new ) )
            FRT_HzdSegReactVector( seg, pos_v ), flag |= 1 ;
    }
    if ( HZX_LevelHazardCheck( hzx, pos, HZX_CHK_ALL, HZX_FLOOR_ALL ) )
    {
        HZX_GetLevelHeight( flr ) ;
        if ( new.vy < (flr[0] -= min) )
            pos->vy = flr[0], pos_v->vy *= bounce->vy, flag |= 2 ;
    }
    return flag ;
}

int FRT_HzdOnlineCheck( HZX_GROUP_ID hzx, FVECTOR *pos, FVECTOR *pos_v,	float min, FVECTOR *bounce )
{
    int   atr[2] ;
    HZX_SEG seg[2] ;
    int flag = 0 ;
    FVECTOR new ;

    _sceVu0AddVector( &new, pos, pos_v ) ;
    if( pos_v->vy < 0.0f ) /* 床も 見る */
	flag = HZX_OnlineHazardCheck( hzx, pos, &new, HZX_CHK_ALL,
				      HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,
				      HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE ) ;
    else                   /* 床は見ない */
	flag = HZX_OnlineHazardCheck( hzx, pos, &new, HZX_CHK_F_SEGMENT| HZX_CHK_D_SEGMENT,
				      HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,
				      HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE ) ;
    if ( flag )
	HZX_GetOnlinePoint( pos ) ;/*着地点に更新*/

    if ( flag & 1 )
    {
	HZX_GetOnlineHazard( (HZX_FLR*)seg, atr ) ;
	FRT_HzdSegReactVector( seg, pos_v ) ;
	/*すこし動かさないと次のフレームでそのまま当たるから*/
	pos->vx += pos_v->vx * 0.05f ;
	pos->vz += pos_v->vz * 0.05f ;
    }
    if ( flag & 2 )
    {
	/*すこし浮かさないと次のフレームでそのまま当たるから*/
	pos->vy += min ;
	pos_v->vy *= bounce->vy ;
    }
    return flag ;
}

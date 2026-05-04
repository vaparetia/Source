//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_btl_hzx.c
   瓶壊れ 当たり判定用ルーチン

   2000/02/10 T. Morita
   $Id: brk_btl_hzd.c,v 1.1.1.3 2002/11/19 11:45:24 Yoshizawa1 Exp $
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

#include "brk_bottle.h"



static int BRK_BTL_HzxCheck( HZX_GROUP_ID hzx,
			     FVECTOR *new, FVECTOR *pos, FVECTOR *vel,
			     FVECTOR *bounce, float sphere )
{
    int flag = 0 ;
    float floor[2] ;
#if 0
    HZX_SEG seg[2] ;
    HZX_FLR flr[2] ;
    int atr[2] ;
#endif

    if ( HZX_NearHazardCheck( hzx, new, (int)sphere, HZX_CHK_ALL, HZX_SEG_NO_PLAYER, (int)sphere ) )
        HZX_GetReactVector( vel ), flag |= 0x401 ;

    if ( HZX_LevelHazardCheck( hzx, pos, HZX_CHK_ALL, HZX_FLOOR_ALL ) )
    {
	HZX_GetLevelHeight( floor ) ;
	if ( new->vy < floor[0] )
	    pos->vy = floor[0], vel->vy *= bounce->vy, flag |= 0x804 ;
    }
    return flag ;
}

static int BRK_BTL_LocalHzdCheck( Work *work,
				  FVECTOR *new, FVECTOR *pos, FVECTOR *vel,
				  FVECTOR *bounce, TARGET *t )
{
    int     i, axis, flag = 0 ;
    FVECTOR v, s, p, *h = work->hzd ;
    FVECTOR min, max ;

    while( h->vw > -5.0f )
	switch( (int)h->vw )
	{
	case BTK_BTL_HZD_BOX:
	    s.vx = (h+1)->vx + (vel->vx>0 ? -1 : 1) * (t ? t->size.vx : 0) ;
	    s.vy = (h+1)->vy + (vel->vy>0 ? -1 : 1) * (t ? t->size.vy : 0) ;
	    s.vz = (h+1)->vz + (vel->vz>0 ? -1 : 1) * (t ? t->size.vz : 0) ;
	    _sceVu0ApplyMatrix( &p, &t->world, &t->offset ) ;
	    _sceVu0AddVector( &p, &p, &t->center ) ; /* ターゲットの中心位置を計算  */
	    _sceVu0AddVector( &p, &p,  vel ) ;       /* 速度を足し,次の位置に進める */
	    _sceVu0SubVector( &p, &p,  h   ) ;       /* ハザードのローカル座標にする*/
	    _sceVu0SubVector( &s, &p, &s   ) ;       /* それぞれの軸の壁までの距離  */

#if 0
PrintVector( pos, "POS pre" ) ;
PrintVector( vel, "VEL pre" ) ;
PrintVector( &t->offset, "Trg off" ) ;
PrintVector( &t->size, "Trg siz" ) ;
PrintVector( &t->center, "Trg cen" ) ;
PrintVector(&p  , "p" ) ;
PrintVector( h  , "Thzd ps" ) ;
PrintVector(h+1, "Thzd sz" ) ;
PrintVector(&s, "Dis" ) ;
#endif

		if ( vel->vx != 0.0f ) {
			v.vw = s.vx / vel->vx ;
			if ( v.vw > 0.0f && v.vw <= 1.0f )
			  {
				  _sceVu0ScaleVector( &v, vel, v.vw ) ;
				  _sceVu0AddVector( &v, &v, &p ) ;
				  if ( !(int)(v.vy/(h+1)->vy) && !(int)(v.vz/(h+1)->vz) )
					pos->vx += s.vx, vel->vx *= bounce->vx, flag |= s.vx>0?0x101:0x102 ;
			  }
		}
		if ( vel->vy != 0.0f ) {
			v.vw = s.vy / vel->vy ;
			if ( v.vw > 0.0f && v.vw <= 1.0f )
			  {
				  _sceVu0ScaleVector( &v, vel, v.vw ) ;
				  _sceVu0AddVector( &v, &v, &p ) ;
				  if ( !(int)(v.vx/(h+1)->vx) && !(int)(v.vz/(h+1)->vz) )
					{
						vel->vy *= bounce->vy ;
						if ( s.vy>0 )
						  pos->vy = h->vy + (h+1)->vy, flag |= 0x104 ;
						else
						  pos->vy += s.vy, flag = 0x108 ;
					}
			  }
		}
		if ( vel->vz != 0.0f ) {
			v.vw = s.vz / vel->vz ;
			if ( v.vw > 0.0f && v.vw <= 1.0f )
			  {
				  //printf( "Z(%.3f) ",v.vw ) ;
				  _sceVu0ScaleVector( &v, vel, v.vw ) ;
				  _sceVu0AddVector( &v, &v, &p ) ;
				  if ( !(int)(v.vx/(h+1)->vx) && !(int)(v.vy/(h+1)->vy) )
					pos->vz += s.vz, vel->vz *= bounce->vz, flag |= s.vy>0?0x110:0x120 ;
			  }
		}
			
	    h += 2 ;
	    break ;

	case BTK_BTL_HZD_BALL:
	    _sceVu0SubVector( &s, h, pos ) ;
	    _sceVu0Normalize( &v, vel ) ;
	    _sceVu0ScaleVector( &v, &v, _sceVu0InnerProduct( &v, &s ) ) ;
	    _sceVu0AddVector( &v, &v, pos ) ;  /* Nearest point to center of the Box */
	    _sceVu0SubVector( &v, &v, h ) ;/* Coordinate to Box*/

	    if ( !(int)(v.vx/(h+1)->vx) && !(int)(v.vy/(h+1)->vy) && !(int)(v.vz/(h+1)->vz) )
		flag |= 0x20 ;
	    h += 2 ;
	    break ;

	case BTK_BTL_HZD_TUNNEL:
	    _sceVu0ApplyMatrix( &v, &t->world, &t->offset ) ;
	    _sceVu0AddVector( &v, &v, &t->center ) ;         /*ターゲットの中心位置を計算  */
	    _sceVu0AddVector( &v, &v, vel ) ;                /*速度を足し,次の位置に進める */
	    _sceVu0SubVector( &v, &v, h   ) ;                /*ハザードのローカル座標にする*/

	    min.vx = min.vy = min.vz =  60000.0f ;
	    max.vx = max.vy = max.vz = -60000.0f ;
	    s.vw = 0.0f ;
	    for ( i=8 ; --i>=0 ; )
	    {
		s.vx = i&1 ? t->size.vx : -t->size.vx ;
		s.vy = i&2 ? t->size.vy : -t->size.vy ;
		s.vz = i&4 ? t->size.vz : -t->size.vz ;
		_sceVu0ApplyMatrix( &s, &t->world, &s ) ;


#ifdef BP_PSX2_ASM
		asm volatile ( "
	    	    lqc2      vf3, 0(%2)
	    	    lqc2      vf1, 0(%0)
	    	    lqc2      vf2, 0(%1)
	    	    vmini.xyz vf1, vf1, vf3
	    	    vmax.xyz  vf2, vf2, vf3
	    	    sqc2      vf1, 0(%0)
	    	    sqc2      vf2, 0(%1)
	        ": : "r"(&min), "r"(&max), "r"(&s) ) ;
#else
		MinMaxVector( &min, &max, &s ) ;
#endif
	    }
	    _sceVu0SubVector( &s, &max, &min ) ;
	    _sceVu0ScaleVector( &s, &s, 0.5f ) ;
	    ((float*)&s)[(int)((h+1)->vw)] = 0.0f ;
	    _sceVu0SubVector( &s, h+1, &s ) ;    /* ハザードのサイズを調整 */


	    /* (h+1)->vwの軸から出てればすべての判定無効 */
	    axis  = ((int)(v.vx/s.vx) ? 1:0) | ((int)(v.vy/s.vy) ? 2:0) | ((int)(v.vz/s.vz) ? 4:0) ;
	    axis &= axis & (1 << (int)((h+1)->vw)) ? 0 : ~(1 << (int)((h+1)->vw)) ;

	    if ( axis & 1 )
	    {
		if ( v.vx < 0.0f )
		    pos->vx += vel->vx - v.vx - s.vx, flag |= 0x201 ;
		else
		    pos->vx += vel->vx - v.vx + s.vx, flag |= 0x202 ;
		vel->vx *= bounce->vx ;
	    }
	    if ( axis & 2 )
	    {
		if ( v.vy < 0.0f )
		    /*pos->vy += vel->vy - v.vy - s.vy, flag |= 0x204 ;*/
		    pos->vy = h->vy - (h+1)->vy, flag |= 0x204 ;
		else
		    pos->vy += vel->vy - v.vy + s.vy, flag |= 0x208 ;
		vel->vy *= bounce->vy ;
	    }
	    if ( axis & 4 )
	    {
		if ( v.vy < 0.0f )
		    pos->vz += vel->vz - v.vz - s.vz, flag |= 0x210 ;
		else
		    pos->vz += vel->vz - v.vz + s.vz, flag |= 0x220 ;
		vel->vz *= bounce->vz ;
	    }

	    /*
PrintVector( &t->offset, "Trg off" ) ;
PrintVector( &t->size, "Trg siz" ) ;
PrintVector( &t->center, "Trg cen" ) ;
printf( "flag%x\n", flag ) ;

PrintVector(&v  , "v" ) ;
PrintVector( h  , "Thzd ps" ) ;
PrintVector(h+1, "Thzd sz" ) ;
PrintVector(&s, "Thzd sz" ) ;
printf( "axis%d\n\n", axis ) ;
*/
	    h +=2 ;
	    break ;

	default: /* 円盤 */
	    _sceVu0SubVector( &v, new, h ) ;
	    if ( (int)(v.vz/(h+1)->vz) )
		pos->vy = (v.vy>0.0f ? h->vy : -h->vy), vel->vy *= -1, flag |= 0x10 ;
	    h++ ;
	    break ;
	}
    /*
PrintVector( pos, "new Pos" ) ;
PrintVector( vel, "new Vel" ) ;
*/
    return flag ;
}

int BRK_BTL_HzdCheck( Work *work,
		      FVECTOR *pos, FVECTOR *vel,
		      FVECTOR *bounce, float sphere, TARGET *t )
{
    int     flag ;
    FVECTOR new ;

    _sceVu0AddVector( &new, pos, vel ) ;
    if ( !(flag = BRK_BTL_LocalHzdCheck( work, &new, pos, vel, bounce, t ) ) )
	flag = BRK_BTL_HzxCheck( work->hzx, &new, pos, vel, bounce, sphere ) ;
//printf( "flag %x \n\n", flag ) ;
    return flag ;
}

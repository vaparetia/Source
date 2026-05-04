//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   blast_water.c
   水中爆発
   
   2001/06/02 M.Sonoyama
   $Id: blast_water.c,v 1.1.1.3 2002/11/19 11:49:59 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

/*------------------------------------------------------------*/

extern void 	*NewBombWaterEffect( FVECTOR *pos ) ;

/*------------------------------------------------------------*/

typedef	struct	{
    GV_ACT	actor ;
    TARGET	target ;
    TARGET	target2 ;
    POWER_TARGET	damage ;
    POWER_TARGET	faint ;
} Work ;

/*------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
    GV_DestroyActor( work ) ;
}

/*------------------------------------------------------------*/

/* コールバック */
static	void	Hit( TARGET *off, TARGET *def, void *ptr )
{
	Work		*work ;
	FVECTOR		dc ;
	FVECTOR		dsize, v ;
	HZX_GROUP_ID	hzx_id ;
	float		len ;
	int			i ;

	work = ( Work * )ptr ;

	/* サイズがでかいときや、スタンは重いのでやらない */
	if ( ( work->target.weapon_type & WP_STUNGRENADE ) ||
		off->size.vx > 4000.0F ||
		off->size.vy > 4000.0F ||
		off->size.vz > 4000.0F ) {
		return ;
	}

	DG_COPY_VEC( &dsize, &off->size ) ;

	/* 中心チェック */
	hzx_id = GM_GetHzxGroupID( off->map ) ;
	GM_TargetGetCenter( &dc, def ) ;
	if ( !HZX_OnlineHazardCheck( hzx_id, &off->center, &dc,
							     HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, 
							     HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET | HZX_FLOOR_PITFALL ) ) {
		return ;
	}

	for ( i = 0; i < 8; i ++ ) {
		v.vx = ( i & 1 ) ? dsize.vx : -dsize.vx ;
		v.vy = ( i & 2 ) ? dsize.vy : -dsize.vy ;
		v.vz = ( i & 4 ) ? dsize.vz : -dsize.vz ;
		_sceVu0AddVector( &v, &off->center, &v ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &off->center, &v, 
								    HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY,
								    HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET | HZX_FLOOR_PITFALL ) ) {
			HZX_GetOnlineVector( &v ) ;
			len = GV_VecLen3F( &v ) ;
			if ( len < 1.0F ) continue ;
			GV_LenVec3F( &v, &v, 0.0F, len - 1.0F ) ;
			_sceVu0AddVector( &v, &off->center, &v ) ;
		}
		if ( !HZX_OnlineHazardCheck( hzx_id, &v, &dc, HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY,
									 HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET | HZX_FLOOR_PITFALL ) ) {
			return ;
		}
	}
	GM_TargetHitCancel( off, def ) ;
}

/* ターゲットセット */
static	inline	void	SetTarget( work, vec, side, range1, range2, damage, faint, wp, type )
Work		*work ;
FVECTOR		*vec ;
int		side, range1, range2, damage, faint, wp, type ;
{
    TARGET	*t ;
    POWER_TARGET	*p ;
    FVECTOR	size, force ;
	int		dam ;

    t = &work->target ;
    p = &work->damage ;
	dam = damage ;
	if ( !( type & BLAST_TYPE_ONETARG ) ) dam /= 2 ;

    size.vx = size.vy = size.vz = ( float )range1 / 2.0F ;
    force.vx = size.vx / 8.0F ;
    force.vy = 0.0f;
    force.vz = 0.0f;
    force.vw = 0.0f;
    GM_SetTarget( t, TARGET_OFFENSE, 0, side, &size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, I64(1) << wp ) ;
    GM_SetPowerTarget( t, p, POWER_EXPLODE, 255, 0, dam, &force ) ;
    GM_MoveTargetMap( t, vec, GM_CurrentMap | GM_CurrentStageMap ) ;
	GM_SetTargetCallBack( t, Hit, work ) ;
    GM_PutTarget( t ) ;

	if ( type & BLAST_TYPE_ONETARG ) return ;

    t = &work->target2 ;
    p = &work->faint ;
    size.vx = size.vy = size.vz = ( float )range2 / 2.0F ;
    force.vx = size.vx / 8.0F ;
    GM_SetTarget( t, TARGET_OFFENSE, GM_CurrentStageMap, side, &size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, I64(1) << wp ) ;
    GM_SetPowerTarget( t, p, POWER_EXPLODE, 255, faint, dam, &force ) ;
    GM_MoveTargetMap( t, vec, GM_CurrentMap | GM_CurrentStageMap ) ;
	GM_SetTargetCallBack( t, Hit, work ) ;
    GM_PutTarget( t ) ;
}

static	inline	int	GetResources( work, v, side, r1, r2, damage, faint, wp, flag )
Work		*work ;
FVECTOR		*v ;
int		side, r1, r2, damage, faint, wp, flag ;
{
	int			map ;

    /* ターゲット */
    SetTarget( work, v, side, r1, r2, damage, faint, wp, flag ) ;
    /* アニメ */
	//AN_Blast_Single( v ) ;
	//C4_Explosion( v ) ;
    if ( !( flag & BLAST_TYPE_NO_FIRE ) ) NewBombWaterEffect( v );
    if ( !( flag & BLAST_TYPE_NO_NOISE ) ) {
		map = GM_GetMapIDfromPos( GM_CurrentMap, v ) ;
		GM_SetNoise( NOISE_L, v, map ) ;
	}
	if ( !( flag & BLAST_TYPE_NO_SE ) ) {
		/* 2種類の音ランダム */
		//if ( GV_Time & 1 ) GM_SeSetMode( SD_W_EXPLOS01, v, GM_SEMODE_BOMB ) ;
		//else 			     GM_SeSetMode( SD_W_EXPLOS02, v, GM_SEMODE_BOMB ) ;
		GM_SeSetMode( SD_A_EXPLOSW1, v, GM_SEMODE_BOMB ) ;
	}
    return 0 ;
}

/* 爆発 */
void		*NewBlast3Water( vec, side, range1, range2, damage, faint, wp, flag )
FVECTOR		*vec ;
int			side, range1, range2, damage, faint, wp, flag ;
{
    Work	*work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, NULL ) ;
		GetResources( work, vec, side, range1, range2, damage, faint, wp, flag ) ;
    }
    return work ;    
}

void		*NewBlastWater( vec, side, range1, range2, damage, faint, wp )
FVECTOR		*vec ;
int			side, range1, range2, damage, faint, wp ;
{
    return NewBlast3Water( vec, side, range1, range2, damage, faint, wp, 0 ) ;
}

void		*NewBlast2Water( mat, side, range1, range2, damage, faint, wp )
FMATRIX		*mat ;
int		side, range1, range2, damage, faint, wp ;
{
    FVECTOR	vec ;

    vec.vx = mat->m[ 3 ][ 0 ] ;
    vec.vy = mat->m[ 3 ][ 1 ] ;
    vec.vz = mat->m[ 3 ][ 2 ] ;
    return NewBlast3Water( &vec, side, range1, range2, damage, faint, wp, 0 ) ;
}


//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   blast.c
   汎用爆発
   
   1999/12/02 M.Sonoyama
   $Id: blast.c,v 1.2 2002/12/03 10:39:39 takaki Exp $
   */

#include <stdio.h>
#include <stdlib.h>
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


/*------------------------------------------------------------*/

extern	void	AN_Blast_Single( FVECTOR * ) ;
extern	void	*NewBombEffect( FVECTOR *, int ) ;

/*------------------------------------------------------------*/

typedef	struct	{
    GV_ACT		actor ;
	FVECTOR		noise_pos ;
	FVECTOR		chkpos[ 8 ] ;
	FVECTOR		chkpos2[ 8 ] ;
    TARGET		target ;
    TARGET		target2 ;
    POWER_TARGET	damage ;
    POWER_TARGET	faint ;
	int				type ;
	int				n_hits ;
	TARGET			*hit[ 64 ] ;
	int				time ;
	int				map ;
} Work ;

/*------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
	if ( work->time != 0 ) {
		if ( GV_Time - work->time > 2 ) {
			/* ノイズが出るのは1フレーム後 */
			GM_SetNoise( NOISE_L, &work->noise_pos, work->map ) ;
			GV_DestroyActor( work ) ;
		}
	} else {
		GV_DestroyActor( work ) ;
	}
}

/*------------------------------------------------------------*/

static	int		AlreadyHit( Work *work, TARGET *def )
{
	int			i ;

	for ( i = 0; i < work->n_hits; i ++ ) {
		if ( work->hit[ i ] == def ) return 1 ;
	}
	return 0 ;
}

/* コールバック */
static	void	Hit( TARGET *off, TARGET *def, void *ptr )
{
	Work		*work ;
	FVECTOR		dc ;
	FVECTOR		*chkpos ;
	HZX_GROUP_ID	hzx_id ;
	int			i ;

	work = ( Work * )ptr ;
	if ( work->type & BLAST_TYPE_WALLTHROUGH ) return ;

	/* 一度に６４までしか当たらない */
	if ( work->n_hits >= 64 ) {
		GM_TargetHitCancel( off, def ) ;
		return ;
	}

	/* サイズがでかいときや、スタンは重いのでやらない */
	if ( ( work->target.weapon_type & WP_STUNGRENADE ) ||
		off->size.vx > 4000.0F ||
		off->size.vy > 4000.0F ||
		off->size.vz > 4000.0F ) {
		return ;
	}

	hzx_id = GM_GetHzxGroupID( off->map ) ;

	/* 中心チェック */
	if ( off == &work->target ) {
		GM_TargetGetCenter( &dc, def ) ;
		if ( !HZX_OnlineHazardCheck( hzx_id, &off->center, &dc,
									 HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, 
									 HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET | HZX_FLOOR_PITFALL ) ) {
			//printf( "hit %x\n", def ) ;
			work->hit[ work->n_hits ] = def ;
			work->n_hits ++ ;
			return ;
		}
	} else {
		if ( AlreadyHit( work, def ) ) {
			//printf( "already hit %x\n", def ) ;
			return ;
		}
	}

	/* 40ｃｍ以下のものには頂点チェックはしない */
	if ( def->size.vx < 200.0F ||
		 def->size.vy < 200.0F ||
		 def->size.vz < 200.0F ) {
		GM_TargetHitCancel( off, def ) ;
		return ;
	}

	if ( off == &work->target ) chkpos = work->chkpos ;
	else 						chkpos = work->chkpos2 ;

	for ( i = 0; i < 8; i ++, chkpos ++ ) {
		if ( !HZX_OnlineHazardCheck( hzx_id, chkpos, &dc, HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY,
									 HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET | HZX_FLOOR_PITFALL ) ) {
			//printf( "hit %x\n", def ) ;
			work->hit[ work->n_hits ] = def ;
			work->n_hits ++ ;
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
#if 1 //BP
//#ifdef KP_WINDOWS
	size.vw = 1.0f ;
#endif
    force.vx = size.vx / 8.0F ;
    force.vy = 0.0f;/* 他の要素の初期化 */
    force.vz = 0.0f;
    force.vw = 0.0f;
    GM_SetTarget( t, TARGET_OFFENSE, GM_CurrentStageMap, side, &size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, I64(1) << wp ) ;
	GM_SetTargetCallBack( t, Hit, work ) ;
    GM_SetPowerTarget( t, p, POWER_EXPLODE, 255, 0, dam, &force ) ;
    GM_MoveTargetMap( t, vec, GM_CurrentMap | GM_CurrentStageMap ) ;

	if ( type & BLAST_TYPE_ONETARG ) {
		GM_PutTarget( t ) ;
		return ;
	}

    t = &work->target2 ;
    p = &work->faint ;
    size.vx = size.vy = size.vz = ( float )range2 / 2.0F ;
    force.vx = size.vx / 8.0F ;
    GM_SetTarget( t, TARGET_OFFENSE, GM_CurrentStageMap, side, &size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, I64(1) << wp ) ;
	if ( type & BLAST_TYPE_NO_PLAYER ) t->weapon_type |= WP_NOPLAYER ;
	GM_SetTargetCallBack( t, Hit, work ) ;
    GM_SetPowerTarget( t, p, POWER_EXPLODE, 255, faint, dam, &force ) ;
    GM_MoveTargetMap( t, vec, GM_CurrentMap | GM_CurrentStageMap ) ;

	/* 遠い方を先に登録（コールバックの順番の関係） */
    GM_PutTarget( t ) ;
    GM_PutTarget( &work->target ) ;

	/* 各爆発頂点と爆発中心のオンライン作成 */
	if ( !( type & BLAST_TYPE_WALLTHROUGH ) ) {
		TARGET			*off, *off2 ;
		FVECTOR			dsize, dsize2, v ;
		FVECTOR			*chkpos, *chkpos2 ;
		HZX_GROUP_ID	hzx_id ;
		float			len ;
		int				i ;

		off = &work->target ;
		off2 = &work->target2 ;
		chkpos = work->chkpos ;
		chkpos2 = work->chkpos2 ;
		DG_COPY_VEC( &dsize, &off->size ) ;		
		DG_COPY_VEC( &dsize2, &off2->size ) ;		
		hzx_id = GM_GetHzxGroupID( off->map ) ;

		for ( i = 0; i < 8; i ++, chkpos ++, chkpos2 ++ ) {
			v.vx = ( i & 1 ) ? dsize.vx : -dsize.vx ;
			v.vy = ( i & 2 ) ? dsize.vy : -dsize.vy ;
			v.vz = ( i & 4 ) ? dsize.vz : -dsize.vz ;
			_sceVu0AddVector( chkpos, &off->center, &v ) ;
			if ( HZX_OnlineHazardCheck( hzx_id, &off->center, chkpos, 
									    HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY,
									    HZX_SEG_NO_BULLET, 
									    HZX_FLOOR_NO_BULLET | HZX_FLOOR_PITFALL ) ) {
				HZX_GetOnlineVector( &v ) ;
				len = GV_VecLen3F( &v ) ;
				if ( len <= 1.0F ) continue ;
				GV_LenVec3F( &v, &v, 0.0F, len - 1.0F ) ;
				_sceVu0AddVector( chkpos, &off->center, &v ) ;
			}			
			v.vx = ( i & 1 ) ? dsize2.vx : -dsize2.vx ;
			v.vy = ( i & 2 ) ? dsize2.vy : -dsize2.vy ;
			v.vz = ( i & 4 ) ? dsize2.vz : -dsize2.vz ;
			_sceVu0AddVector( chkpos2, &off2->center, &v ) ;
			if ( HZX_OnlineHazardCheck( hzx_id, &off2->center, chkpos2, 
									    HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY,
									    HZX_SEG_NO_BULLET, 
									    HZX_FLOOR_NO_BULLET | HZX_FLOOR_PITFALL ) ) {
				HZX_GetOnlineVector( &v ) ;
				len = GV_VecLen3F( &v ) ;
				if ( len <= 1.0F ) continue ;
				GV_LenVec3F( &v, &v, 0.0F, len - 1.0F ) ;
				_sceVu0AddVector( chkpos2, &off2->center, &v ) ;
			}			
		}
	}
}

static	inline	int	GetResources( work, v, side, r1, r2, damage, faint, wp, flag )
Work		*work ;
FVECTOR		*v ;
int		side, r1, r2, damage, faint, wp, flag ;
{
	work->time = 0 ;
    /* ターゲット */
    SetTarget( work, v, side, r1, r2, damage, faint, wp, flag ) ;
    /* アニメ */
	//AN_Blast_Single( v ) ;
	//C4_Explosion( v ) ;
    if ( !( flag & BLAST_TYPE_NO_FIRE ) ) NewBombEffect( v, 2 );
    if ( !( flag & BLAST_TYPE_NO_NOISE ) ) {
		work->map = GM_GetMapIDfromPos( GM_CurrentMap, v ) ;
		DG_COPY_VEC( &work->noise_pos, v ) ;
		//GM_SetNoise( NOISE_L, v, map ) ;
		work->time = GV_Time ;
	}
	if ( !( flag & BLAST_TYPE_NO_SE ) ) {
		/* 2種類の音ランダム */
		if ( GV_Time & 1 ) GM_SeSetMode( SD_W_EXPLOS01, v, GM_SEMODE_BOMB ) ;
		else 			   GM_SeSetMode( SD_W_EXPLOS02, v, GM_SEMODE_BOMB ) ;
	}
	work->type = flag ;
    return 0 ;
}

/* 爆発 */
void		*NewBlast3( vec, side, range1, range2, damage, faint, wp, flag )
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

void		*NewBlast( vec, side, range1, range2, damage, faint, wp )
FVECTOR		*vec ;
int			side, range1, range2, damage, faint, wp ;
{
    return NewBlast3( vec, side, range1, range2, damage, faint, wp, 0 ) ;
}

void		*NewBlast2( mat, side, range1, range2, damage, faint, wp )
FMATRIX		*mat ;
int		side, range1, range2, damage, faint, wp ;
{
    FVECTOR	vec ;

    vec.vx = mat->m[ 3 ][ 0 ] ;
    vec.vy = mat->m[ 3 ][ 1 ] ;
    vec.vz = mat->m[ 3 ][ 2 ] ;
    return NewBlast3( &vec, side, range1, range2, damage, faint, wp, 0 ) ;
}


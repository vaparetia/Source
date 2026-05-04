//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	navi.c
	sys/libhzx/navigate.c を利用する簡易プログラム

	2000/05/18 Y.Korekado
	$Id: navi.c,v 1.1.1.3 2002/11/19 11:41:54 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include	"libgv.h"

/* ---------------------------------------------------------------- */
/*
	目標位置と同じゾーン内で距離dis以内まで移動すると
	１を返す
	*/
int		GM_NaviExtend( NAVIGATE *navi, NAVITARGET *n_trg, int range )
{
	if ( n_trg->addr == navi->ctrl->addr ) {
		navi->routes = 0 ;
		return GM_InZoneNavi( navi, n_trg, range ) ;
	} else {
		GM_ZoneNavi( navi, n_trg ) ;
		GM_ZoneNaviExtend( navi, n_trg ) ;
	}

	return 0 ;
}

int		GM_Navi( NAVIGATE *navi, NAVITARGET *n_trg, int range )
{
	if ( n_trg->addr == navi->ctrl->addr ) {
		navi->routes = 0 ;
		return GM_InZoneNavi( navi, n_trg, range ) ;
	} else {
		GM_ZoneNavi( navi, n_trg ) ;
	}

	return 0 ;
}

/*
	目標位置までの距離dis以内まで移動すると１を返す
	*/
int		GM_NaviNearExtend( NAVIGATE *navi, NAVITARGET *n_trg, int range )
{
	if( GM_InZoneNavi( navi, n_trg, range ) ) return 1 ;

	if ( n_trg->addr == navi->ctrl->addr ) {
		navi->routes = 0 ;
	} else {
		GM_ZoneNavi( navi, n_trg ) ;
		GM_ZoneNaviExtend( navi, n_trg ) ;
	}

	return 0 ;
}

int		GM_NaviNear( NAVIGATE *navi, NAVITARGET *n_trg, int range )
{
	if( GM_InZoneNavi( navi, n_trg, range ) ) return 1 ;

	if ( n_trg->addr == navi->ctrl->addr ) {
		navi->routes = 0 ;
	} else {
		GM_ZoneNavi( navi, n_trg ) ;
//		GM_ZoneNaviExtend( navi, n_trg ) ;
	}

	return 0 ;
}

/*
	同ゾーン内での追跡
	*/
int		GM_InZoneNavi( NAVIGATE *navi, NAVITARGET *n_trg, int range )
{
	FVECTOR		shift ;
	float		dx, dz ;

	shift.vx = dx = n_trg->pos.vx - navi->ctrl->mov.vx ;
	shift.vz = dz = n_trg->pos.vz - navi->ctrl->mov.vz ;

	navi->next_dir = GV_VecDir2( &shift ) ;

	if ( (int)dx > -range && (int)dx < range && (int)dz > -range && (int)dz < range ) return -1 ;

	return 0 ;
	
}

/*
	ゾーン追跡
	*/
void	GM_ZoneNavi( NAVIGATE *navi, NAVITARGET *n_trg )
{
	HZX_ZON		*zone ;
	FVECTOR		*target, floor, tmp ;
	int			this, going, next, reach ;

	going = n_trg->addr ;
	target = &n_trg->pos ;

#if 1
	/* デバッグ用フロアの高さでアドレスを再計算 */
	floor.vx = navi->ctrl->mov.vx ;
	floor.vy = navi->ctrl->levels[0] ;
	floor.vz = navi->ctrl->mov.vz ;

	this = HZX_GetAddress( navi->ctrl->hzx_id, &floor, navi->ctrl->addr ) ;
#ifdef DEBUG_MODE
	if ( this != navi->ctrl->addr ) {
		int a ;
		HZX_ZON	*zon ;

		printf("x[%f]y[%f]z[%f] \n",navi->ctrl->mov.vx,navi->ctrl->mov.vy,navi->ctrl->mov.vz);
printf("mov\n");
	this = HZX_GetAddress( navi->ctrl->hzx_id, &floor, navi->ctrl->addr ) ;
printf("floor\n");
		a = HZX_GetAddress( navi->ctrl->hzx_id, &navi->ctrl->mov, navi->ctrl->addr ) ;
		printf("navi.c:WARNING !!this[%x] addr[%x][%f] a[%x][%f]\n",
			this,navi->ctrl->addr,floor.vy,a,navi->ctrl->mov.vy) ;


		zon = HZX_GetZoneFromAdd( navi->ctrl->addr ) ;
		printf("y[%f] \n",zon->y);


//		AN_Test_Eye2( &navi->ctrl->mov, 1 );
//		AN_Test_Eye2( &floor, 1 );
	}
#endif
#else
	this = navi->ctrl->addr ;
#endif

	reach = HZX_ReachTo( this, navi->next_addr );
#if 0
printf("This=0x%4x, Next=0x%4x, Targ=0x%4x, Reach=%d \n", this, navi->next_addr, going, reach );
#endif
	if ( going != navi->going_addr 	/* 最終目標が変わった */
			|| reach <= HZX_REACH ) {	/* 一時目標に到達した */
		navi->going_addr = going ;
		if ( GM_GetBit( HZX_ZoneMapNo( n_trg->addr ) ) == navi->ctrl->hzx_id 
				&& HZX_InsideZone( navi->ctrl->hzx_id, &floor, n_trg->addr & 255 ) ) {
			/* 最終目標ゾーンに到達 */
			navi->next_zonepos = *target ;
			navi->next_addr = going ;
			_sceVu0SubVector(  &tmp, &( navi->next_zonepos ), &floor ) ;
			navi->next_dir = GV_VecDir2( &tmp ) ;
			navi->routes = 0 ;

			return ;
		} else {
			next = HZX_Navigate( this, going, &floor ) ;
			zone = HZX_GetZoneNo( HZX_ZoneMapNo(next), HZX_Zone1(next) ) ;
			navi->next_zonepos.vx = (float)zone->x ;
			navi->next_zonepos.vy = (float)zone->y ;
			navi->next_zonepos.vz = (float)zone->z ;
			navi->next_addr = next ;
			navi->routes = HZX_GetRouteCrossGroup( this, going ) ;
		}
	}
	_sceVu0SubVector(  &tmp, &( navi->next_zonepos ), &floor ) ;
	navi->next_dir = GV_VecDir2( &tmp ) ;
}

void	GM_ZoneNaviExtend( NAVIGATE *navi, NAVITARGET *n_trg )
{
	HZX_ZON		*zone ;
	FVECTOR		pos, vec ;
	int			this, going, next, tmp ;

	going = n_trg->addr ;
	this = navi->ctrl->addr ;
	next = this ;


   //printf( "navi:next_addr[%x], going[%x] this[%x]\n",navi->next_addr, going , this) ;
	while ( next != going ) {
		tmp = HZX_NextZoneCrossGroup( next, going ) ;
//printf("navi:tmp[%x] next[%x] going[%x] \n",tmp,next,going ) ;
//printf( ">[%x]",tmp ) ;
		if ( HZX_ReadOnlinInfo( tmp, this ) ) break ;
		next = tmp ;
	}
//printf( "end\n") ;

	zone = HZX_GetZoneFromAdd( next ) ;
	pos.vx = (float)zone->x ;
	pos.vy = (float)zone->y ;
	pos.vz = (float)zone->z ;

	_sceVu0SubVector( &vec, &pos, &navi->ctrl->mov ) ;
	navi->next_dir = GV_VecDir2( &vec ) ;
}


/* ---------------------------------------------------------------- */
/*
	場所指定でナビ用ターゲットセット（遅）

	園山追加機能：
	  ターゲット位置のグループを全グループ全ゾーンから検索する。
*/
void	 GM_SetNaviTargetFromPos( NAVITARGET *n_trg, FVECTOR *pos, int hzx_id )
{
	int				z, id ;

	id = HZX_GetHzxIDbyZone( 0, pos, &z ) ;
	ASSERT( id != 0 ) ;
	n_trg->pos = *pos ;
	n_trg->addr = HZX_AddressNo( GV_GetNo( id ), z, z ) ;
}

/*
	コントロールから目標セット（速）
	*/
void GM_SetNaviTargetFromCtrl( NAVITARGET *n_trg, CONTROL *ctrl )
{
	n_trg->pos = ctrl->mov ;
	n_trg->addr = ctrl->addr ;
}
/*
	ゾーンアドレスから目標セット（速）
	*/
void GM_SetNaviTargetFromZoneAddr( NAVITARGET *n_trg, HZX_ZONE_ADD zoneaddr )
{
	HZX_ZON		*hzx_zone ;

	hzx_zone = HZX_GetZoneFromAdd( zoneaddr ) ;
	n_trg->pos.vx = (float)hzx_zone->x ;
	n_trg->pos.vy = (float)hzx_zone->y ;
	n_trg->pos.vz = (float)hzx_zone->z ;

	n_trg->addr = zoneaddr ;
}

/*
	ゾーンをはみ出したり、繋がらないゾーンへ瞬間移動する場合
	この関数を呼ぶ
	*/
void GM_ReSetNavi( NAVIGATE *navi )
{
	navi->going_addr = -1 ;
}

void GM_SetNavi( NAVIGATE *navi, CONTROL *ctrl )
{
	navi->ctrl = ctrl ;
	navi->going_addr = -1 ;
}
/* ---------------------------------------------------------------- */

//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	viewer.c
	chara 安地みるみる君[NewSigZoneView] $s:名前 
	2000/04/24 K.Sigeno
	$Id: zoneviewer.c,v 1.1.1.3 2002/11/19 11:49:08 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"
#include	"korekado/enemy/enemy.h"

#define BOX_HEIGHT	(1000.0F)
#define SAFE_ZONE_VIEW_MAX (5)

extern HZX_ZON *ENE_HZX_GetZone( int ) ;

extern void SigZoneView( int ,SVECTOR * ,float) ;

static int MapAddrSet(int addr1,int mapaddr){
	int addr ;
	addr = HZX_Zone1( addr1 );
	addr = (addr|(addr<<8)|(HZX_ZoneMapNo(mapaddr)<<16));
	return addr;
}

static int RouteIndex( int z1, int z2 ,int n_zone){
	int index;

	if( z2 < z1 ){
		int tmp;
		tmp = z2;
		z2 = z1;
		z1 = tmp;
	} else if( z1 == z2 ){
		return 0;
	}

	index = ( z1 * ( 2 * n_zone - z1 - 3 ) ) / 2 + z2 - 1;
	return index ;
}


/*近接ゾーンリスト*/
#define SIG_ZONE_MAX (256)
/*nゾーン以内のゾーンを表示*/
void SigNearZone( int addr ,int n){
	HZX_HDL *hdl;
	HZX_GRP *grp_top, *grp;
	int mapno ,n_zones ,i;
	int index,check;
	int	near_zones = 0 ;
	SVECTOR		rgb;
	char list[SIG_ZONE_MAX];

	hdl = HZX_GetCurrentHzx();
	grp_top = hdl->grp;

	mapno = HZX_ZoneMapNo( addr ) ;
	grp = grp_top + mapno;
	n_zones = grp->n_zones ;

	rgb.vx = 255 ;
	rgb.vy = 0 ;
	rgb.vz = 0 ;

	if(n_zones == 0 ) return ;
	if(n_zones == 1) {
		/*
		イベントステージなどゾーン検索を使用しないステージなどは
		ゾーン1個がありえる。その場合routeが存在しないので
		指定ゾーンだけ表示して終了
		*/
		rgb.vx = 0 ;
		rgb.vy = 0 ;
		rgb.vz = 255 ;
		SigZoneView( addr, &rgb ,10.0F) ;
		return ;
	}

	/*リスト作成*/
	for(i=0;i<n_zones;i++){
		/*同一ゾーンは除外*/
//		if(HZX_Zone1(addr)== i) continue ;
		index = RouteIndex(HZX_Zone1(addr),i,n_zones) ;
#if 1
		if(grp->route[index] <= n){
#else
		if(grp->route[index] <= 1){
#endif
			/*該当ゾーン検出*/
			list[near_zones] = i ;
			near_zones ++ ;
		}
	}

	if(near_zones == 0 ) return ;

	near_zones -- ;

	while ( near_zones >= 0 ){
		check = MapAddrSet(list[near_zones] ,addr ) ;
//		if(ENE_ReadOnlinInfo(check,GM_PlayerAddress)){
		/*視界チェック*/
		if(ENE_ReadOnlinInfo(check,addr)){
		/*見えない*/
			rgb.vx = 255 ;
			rgb.vy = 0 ;
			rgb.vz = 0 ;
		}else {
			/*見える*/
			rgb.vx = 0 ;
			rgb.vy = 0 ;
			rgb.vz = 255 ;
		}
		SigZoneView( check, &rgb ,10.0F) ;
		near_zones -- ;
	}
}



#define POS_MAX (16)
typedef	struct _Work {
	GV_ACT		actor ;
	int			count ;
	int			where ;
} Work ;

static void Die(void){
}
static void Act(Work *work ){
	SVECTOR rgb ;
	rgb.vx = 255 ;
	rgb.vy = 128 ;
	rgb.vz = 0 ;

	/*近接ゾーン*/
	SigNearZone( GM_PlayerAddress ,SAFE_ZONE_VIEW_MAX);

}
/* 初期化部メイン */
//chara 安地みるみる君[NewSigZoneView] $s:名前 

void *NewSigZoneView( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
		   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		work->count = 0;
		work->where = where;
	}
	return (void *)work ;
}



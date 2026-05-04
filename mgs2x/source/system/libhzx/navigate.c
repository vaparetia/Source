//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   navigate.c
   道順ナビゲータルーチン
   
   1999/07/08 Y.Korekado
   $Id: navigate.c,v 1.1.1.3 2002/11/19 11:42:48 Yoshizawa1 Exp $			   
   
   ※ここでいう「アドレス」は、メモリアドレスではなく
   道順ナビゲート計算のための「アドレス」を意味する
   
   地形当たりデータ内のゾーン番号２つを使用して、
   「ゾーン１←→ゾーン２」といった形式で表され、
   これが下位８ビット／上位８ビットに格納される
   
   アドレスには、次の４とおりがある
   
   ２ゾーン間	zone1 = XXX	zone2 = YYY
   １ゾーン内	zone1 = XXX	zone2 = XXX
   近くのゾーン	zone1 = XXX	zone2 = 255
   初期状態	zone1 = 255	zone2 = 255
   
   ------------------------------------------------
   
   int		HZX_GetAddress( hzd, pos, address )
   HZX_HDL	*hzd ;		当たりハンドラ
   SVECTOR	*pos ;		現在の座標
   int		address ;	前回のアドレス
   
   ある座標に対するアドレスを計算する
   前回のアドレスがあれば、それを用いて計算
   
   前回のアドレスを使うので、ワークに記録しておくこと
   初期状態アドレスを与えると、全ゾーンについて再計算
   
   ------------------------------------------------
   
   int		HZX_ReachTo( hzd, addr1, addr2 )
   HZX_HDL	*hzd ;		当たりハンドラ
   int		addr1 ;		現在のアドレス
   int		addr2 ;		目標のアドレス
   
   現在のアドレスが目標のアドレスと
   一致しているかどうか、度合を計算
   
   HZX_REACH		= 0	同じ場所
   HZX_DIRECT_REACH	= 1	直接見える場所
   HZX_INDIRECT_REACH	= 2	カドを一つ曲がった場所
   HZX_UNREACH		= 3	まだまだ遠い
   
   int		HZX_Navigate( hzd, addr1, addr2, pos )
   HZX_HDL	*hzd ;		当たりハンドラ
   int		addr1 ;		現在のアドレス
   int		addr2 ;		目標のアドレス
   SVECTOR	*pos ;		現在の座標
   
   目標のアドレスへ向かうために
   次に行くべきゾーンを計算する
   pos は、判定が微妙な場合に使用
   */
#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

//#define	MAX_MIN_DISTANCE	(10000000.0F)

#if 0 //BP
//#ifdef PSX2
#define	MAX_MIN_DISTANCE	(1E+127f)
#else
#define	MAX_MIN_DISTANCE	FLOAT_MAX
#endif

/* NavigateMapToMap で使用 */
static int Min_Route_Num ;
static int Min_Map_Num ;
static int C_Route ;
static int MapRoute[ GM_MAX_MAPS ] ;
static int ZoneRoute[ GM_MAX_MAPS ] ;

static int MinMapRoute[ GM_MAX_MAPS ] ;
static int MinZoneRoute[ GM_MAX_MAPS ] ;

/* HZX_ZoneDistance で使用 */
static int Dist ;
/*----------------------------------------------------------------*/
static int RouteCourse = 0 ;
static int RouteCourseNearFlag( int rcouse, int near_flag ) 
{
	if ( rcouse == 0 ) 	return 0 ;
	if ( rcouse == 1 ) {
		if ( near_flag & HZX_ROOT_COURSE1 ) return 1 ;
		return 0 ;
	}
	if ( rcouse == 2 ) {
		if ( near_flag & HZX_ROOT_COURSE2 ) return 1 ;
		return 0 ;
	}
	if ( rcouse == 3 ) {
		if ( near_flag & (HZX_ROOT_COURSE1|HZX_ROOT_COURSE2) ) return 1 ;
		return 0 ;
	}

	return 0 ;
}

int HZX_RouteCourseNearFlag( int near_flag ) 
{
	return RouteCourseNearFlag( RouteCourse, near_flag ) ;
}

void HZX_ChangeRouteCourse( int course )
{

	if ( course == (HZX_ROOT_COURSE1|HZX_ROOT_COURSE2) ) {
		RouteCourse = 3 ;
		return ;
	}
	if ( course & HZX_ROOT_COURSE1 ) {
		RouteCourse = 1 ;
		return ;
	}
	if ( course & HZX_ROOT_COURSE2 ) {
		RouteCourse = 2 ;
		return ;
	}

	RouteCourse = 0 ;
}
void HZX_ClearRouteCourse( int course )
{
	RouteCourse = 0 ;
}
/*----------------------------------------------------------------*/
static inline void FvecToSvec( FVECTOR *fvec, SVECTOR *svec )
{
	svec->vx = (short)(fvec->vx) ;
	svec->vy = (short)(fvec->vy) ;
	svec->vz = (short)(fvec->vz) ;
}

static u_char ROUTE( u_char *route, long64 z1, long64 z2, long n_zone )
{
	long64 index;

	if( z2 < z1 ){
		long64 tmp;
		tmp = z2;
		z2 = z1;
		z1 = tmp;
	} else if( z1 == z2 ){
		return 0;
	}

	index = ( z1 * ( 2 * n_zone - z1 - 3 ) ) / 2 + z2 - 1;
	return route[ index ];
}

static u_short ROUTE_COURSE( u_short *route_course, long64 z1, long64 z2, long64 n_zone )
{
	long64 index;

	if( z2 < z1 ){
		long64 tmp;
		tmp = z2;
		z2 = z1;
		z1 = tmp;
	} else if( z1 == z2 ){
		return 0;
	}

	index = ( z1 * ( 2 * n_zone - z1 - 3 ) ) / 2 + z2 - 1;
//printf("kkkkkkkkkkkkkkkkkkkkkkkkkkk index %d !!\n",index ) ;
	return route_course[ index ];
}

/*
   座標からゾーンまでのキョリを計算
   高さは ZONE_HEIGHT 以内なら、0距離とする
   腰の位置で
   */
static	float	Distance( HZX_ZON *zone, FVECTOR *pos, float minlen )
//HZX_ZON		*zone ;		/* ゾーン構造体		*/
//FVECTOR		*pos ;		/* 座標			*/
//float		minlen;		/* 現在の最小距離 */
{
	float	sum, len ;
	float	height, height_under ;

	sum = 0;

	len = pos->vx - zone->x ;
	if ( len < 0 ) len = -len ;
	len -= zone->w ;
	if ( len > 0 ) sum += len ;
	if( sum > minlen ) return minlen;

	len = pos->vz - zone->z ;
	if ( len < 0 ) len = -len ;
	len -= zone->h ;
	if ( len > 0 ) sum += len ;
	if( sum > minlen ) return minlen;

	len = pos->vy - zone->y ;

#if 1
	if ( zone->flag & HZX_ZONE_LOW_HEIGHT ) {
		height = ZONE_LOW_HEIGHT ;
		height_under = ZONE_LOW_HEIGHT_UNDER ;
	} else {
		height = ZONE_HEIGHT ;
		height_under = ZONE_HEIGHT_UNDER ;
	}

	if ( len < 0 ) {
		if ( len < height_under ) sum -= len ;
	} else {
		if ( len >= height ) sum += len ;
	}
#else
	if ( len < 0 ) len = -len ;
	if ( len > ZONE_HEIGHT ) sum += len ;
#endif

	return sum ;
}

/*----------------------------------------------------------------*/

static inline HZX_ZON *GetZone( HZX_GRP *grp, int zone )
{
	return grp->zones + zone;
}

/*----------------------------------------------------------------*/

/*
   座標が、あるゾーン内にあるかチェック
   */
static	int	InsideZone( zone, pos )
HZX_ZON		*zone ;		/* ゾーン構造体		*/
FVECTOR		*pos ;		/* 座標			*/
{
	float		value, center, width ;
	float		height, height_under ;

	if ( zone->flag & HZX_ZONE_LOW_HEIGHT ) {
		height = ZONE_LOW_HEIGHT ;
		height_under = ZONE_LOW_HEIGHT_UNDER ;
	} else {
		height = ZONE_HEIGHT ;
		height_under = ZONE_HEIGHT_UNDER ;
	}

	value = pos->vy - zone->y ;
	/* 高さの範囲は、下は以内、上は未満 */
	if ( value < height_under || value >= height ) return 0 ;
	value = pos->vx ;
	center = zone->x ;	width = zone->w ;
	if ( value < center - width || value > center + width ) return 0 ;
	value = pos->vz ;
	center = zone->z ;	width = zone->h ;
	if ( value < center - width || value > center + width ) return 0 ;
	return 1 ;
}

/*
   座標が、２ゾーン間にあるかチェック
   */
static	int	BetweenZone( zone1, zone2, pos )
HZX_ZON		*zone1 ;	/* ゾーン構造体		*/
HZX_ZON		*zone2 ;	/* ゾーン構造体		*/
FVECTOR		*pos ;		/* 座標			*/
{
	float	value;
	float	center1, width1 ;
	float	center2, width2 ;

	/*
	   Ｙチェック
	   */
#if 1
	value = pos->vy ;
	center1 = zone1->y ;
	center2 = zone2->y ;

		/* 下方向*/
	width1 = ZONE_HEIGHT_UNDER ;
	if ( zone1->flag & HZX_ZONE_LOW_HEIGHT ) width1 = ZONE_LOW_HEIGHT_UNDER ;
	width2 = ZONE_HEIGHT_UNDER ;
	if ( zone2->flag & HZX_ZONE_LOW_HEIGHT ) width2 = ZONE_LOW_HEIGHT_UNDER ;
	if ( value < center1 + width1 && value < center2 + width2 ) return 0 ;
		/* 上方向 */
	width1 = ZONE_HEIGHT ;
	if ( zone1->flag & HZX_ZONE_LOW_HEIGHT ) width1 = ZONE_LOW_HEIGHT ;
	width2 = ZONE_HEIGHT ;
	if ( zone2->flag & HZX_ZONE_LOW_HEIGHT ) width2 = ZONE_LOW_HEIGHT ;
	if ( value > center1 + width1 && value > center2 + width2 ) return 0 ;
#else
	value = pos->vy ;
	center1 = zone1->y ;
	center2 = zone2->y ;
	value += ZONE_HEIGHT_UNDER ;
	if ( value < center1 && value < center2 ) return 0 ;
	value -= (ZONE_HEIGHT+ZONE_HEIGHT_UNDER) ;
	if ( value > center1 && value > center2 ) return 0 ;
#endif
	/*
	   Ｘチェック
	   */
	value = pos->vx ;
	center1 = zone1->x ;	width1 = zone1->w ;
	center2 = zone2->x ;	width2 = zone2->w ;
	if ( value < center1 - width1 && value < center2 - width2 ) return 0 ;
	if ( value > center1 + width1 && value > center2 + width2 ) return 0 ;
	/*
	   Ｚチェック
	   */
	value = pos->vz ;
	center1 = zone1->z ;	width1 = zone1->h ;
	center2 = zone2->z ;	width2 = zone2->h ;
	if ( value < center1 - width1 && value < center2 - width2 ) return 0 ;
	if ( value > center1 + width1 && value > center2 + width2 ) return 0 ;
	return 1 ;
}

/*
   座標が、隣接ゾーンのいずれかに
   含まれているかをチェック
   */
static	int	InsideNearZones( HZX_GRP *grp, HZX_ZON *zone, FVECTOR *pos )
//HZX_ZON		*zone ;		/* ゾーン構造体		*/
//FVECTOR		*pos ;		/* 座標			*/
{
	HZX_ZON		*zone2 ;
	u_char		*nears ;
	int		i, near ;

	nears = zone->nears ;
	zone2 = grp->zones;
	for ( i = 6 ; i > 0 ; -- i ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
		if ( InsideZone( zone2 + near, pos ) ) return near ;
	}
	return -1 ;
}

/*
   座標が、現在のゾーンと隣接ゾーンの間に
   含まれているかをチェック
   */
static	int	BetweenNearZones( HZX_GRP *grp, HZX_ZON *zone, FVECTOR *pos )
//HZX_ZON		*zone ;		/* ゾーン構造体		*/
//FVECTOR		*pos ;		/* 座標			*/
{
#if 1
	/* BetweenNearZonesの中で最小距離も計算 */
	HZX_ZON		*zone2 ;
	u_char		*nears ;
	int		i, near, min_near ;
	float	len, min_len ;

	nears = zone->nears ;
	zone2 = grp->zones;
	min_len = MAX_MIN_DISTANCE ;
	min_near = -1 ;
	for ( i = 6 ; i > 0 ; -- i ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;

		if ( BetweenZone( zone, zone2 + near, pos ) ) {
			len = Distance( zone2 + near, pos, min_len ) ;
			if ( len < min_len ) {
				min_len = len ;
				min_near = near ;
//				if ( min_len <= 1.0 ) {
//					printf("[%d]korekaaaaaaaaaaaaaaa-----[%d] len[%f]\n",GV_Time,near, min_len);
//				}
			}
		}
	}
	return min_near ;

#else
	HZX_ZON		*zone2 ;
	u_char		*nears ;
	int		i, near ;

	nears = zone->nears ;
	zone2 = grp->zones;
	for ( i = 6 ; i > 0 ; -- i ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
		if ( BetweenZone( zone, zone2 + near, pos ) ) return near ;
	}
	return -1 ;
#endif
}

/*
   座標から最も近いゾーンを検索
   */
static	int	NearestZone( HZX_GRP *grp, FVECTOR *pos )
{
	HZX_ZON		*zone ;
	int		i, min_i, n_zones;
	float	len, min_len ;

	//printf("NearestZone: mapno = [%x] \n",mapno ) ;

	zone = grp->zones ;
	n_zones = grp->n_zones ;
	min_i = -1 ;
	min_len = MAX_MIN_DISTANCE ;
	for ( i = n_zones ; i > 0 ; -- i ) {
		len = Distance( zone, pos, min_len ) ;
		if ( len < min_len ) {
			min_len = len ;
			min_i = i ;
		}
		zone ++ ;
	}
	if ( min_i < 0 ) return -1 ;
	return grp->n_zones - min_i ;
}

/*
   目標のゾーンへの直接の道があるかをチェック
   */
static	int	ExistRoute( HZX_GRP *grp, int zone1, int zone2 )
//int		zone1 ;		/* 現在のゾーン		*/
//int		zone2 ;		/* 目標のゾーン		*/
{
	HZX_ZON		*zone ;
	u_char		*nears ;
	int		i, near ;

	zone = grp->zones + zone1 ;
	nears = zone->nears ;
	for ( i = 6 ; i > 0 ; -- i ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
		if ( near == zone2 ) return 1 ;
	}
	return 0 ;
}

/*
   目標のゾーンまでの道順を計算
   */
static	int	GetRoute( HZX_GRP *grp, int zone1, int zone2 )
//int		zone1 ;		/* 現在のゾーン		*/
//int		zone2 ;		/* 目標のゾーン		*/
{
	int		n_zones ;

	if ( zone1 == HZX_NO_ZONE || zone2 == HZX_NO_ZONE ) return MAX_ROUTE ;

	n_zones = grp->n_zones ;

#ifdef ROUTE_COURSE_TEST
	return ROUTE_COURSE( grp->route_course[RouteCourse], zone1, zone2, n_zones );
#else
	return ROUTE( grp->route, zone1, zone2, n_zones );
#endif
}

/*
   隣接するゾーン
   */
static	int	NearZones( HZX_GRP *grp, int zone, int *near )
//int		zone ;			/* 現在のゾーン		*/
//int		*near ;			/* 結果格納 */
{
	u_char		*nears ;
	int			i ;

	nears = grp->zones[ zone ].nears ;
	for ( i = 0 ; i < 6 ; i++ ) {
		if ( ( near[i] = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
	}

	return i ;
}

/*
   目標のゾーンへ向かうために
   次に行くべきゾーンを計算する
   pos は、判定が微妙な場合に使用
   */
static	int	ZoneNavigate( HZX_GRP *grp, int zone1, int zone2 )
//int		zone1 ;		/* 現在のゾーン		*/
//int		zone2 ;		/* 目標のゾーン		*/
{
	u_char		*nears ;
	int		n_zones, i, near ;
	int		route, min_route, min_near ;

	if ( zone1 == zone2 ) return zone1 ;

	nears = grp->zones[ zone1 ].nears ;
	n_zones = grp->n_zones ;
	min_route = MAX_ROUTE ;
	min_near = zone1 ;
	for ( i = 6 ; i > 0 ; -- i ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
#ifdef ROUTE_COURSE_TEST
			route = ROUTE_COURSE( grp->route_course[RouteCourse], near, zone2, n_zones );
			if ( RouteCourseNearFlag( RouteCourse, grp->zones[zone1].near_flag[ 6-i ] ) ) {
				route += 256 ;
			}
#else
			route = ROUTE( grp->route, near, zone2, n_zones );
#endif
		if ( route < min_route ) {
			min_route = route ;
			min_near = near ;
		}
	}
	return min_near ;
}

#if 0
/* 最小値が同じ場合も入れ替えることで同じ距離でも上の関数とは違う結果を返す場合もある
   重なり防止に使う */
static	int	ZoneNavigate2( HZX_GRP *grp, zone1, zone2 )
HZX_HDL		*hzd ;		/* 当たりハンドラ	*/
int		zone1 ;		/* 現在のゾーン		*/
int		zone2 ;		/* 目標のゾーン		*/
{
	HZX_DEF		*def ;
	u_char		*nears ;
	int		n_zones, i, near ;
	int		route, min_route, min_near ;

	if ( zone1 == zone2 ) return zone1 ;
	def = hzd->def ;
	nears = def->groups->zones[ zone1 ].nears ;
	n_zones = def->groups->n_zones ;
	min_route = MAX_ROUTE ;
	min_near = zone1 ;
	for ( i = 6 ; i > 0 ; -- i ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
		route = ROUTE( hzd->route, near, zone2, n_zones );
		if ( route <= min_route ) {	/*  みそ */
			min_route = route ;
			min_near = near ;
		}
	}
	return min_near ;
}
#endif

static	void	NavigateMapToMap( HZX_HDL *hzd, int addr1, int addr2, int now_route_num )
{
	int	mapno1, mapno2, new_mapno, z_from, z_to ;
	int	link_zoneadd, new_addr, route ;
	int	i, j ;
    u_char	*link ;
	HZX_ZON	*link_zone ;
	HZX_GRP	*grp ;

	mapno1 = HZX_ZoneMapNo( addr1 ) ;
	z_from = HZX_Zone1( addr1 ) ;
	mapno2 = HZX_ZoneMapNo( addr2 ) ;
	z_to = HZX_Zone1( addr2 ) ;
	grp = hzd->grp + mapno1;

	/* リンク先が目標のマップだったら */
	if ( mapno1 == mapno2 ) {
#ifdef ROUTE_COURSE_TEST
		now_route_num += ROUTE_COURSE( grp->route_course[RouteCourse], z_from, z_to, grp->n_zones );
#else
		now_route_num += ROUTE( grp->route, z_from, z_to, grp->n_zones );
#endif
		if ( now_route_num < Min_Route_Num ) {
			Min_Route_Num = now_route_num ;
			Min_Map_Num = C_Route ;
			MapRoute[ C_Route+1 ] = mapno2 ;
			ZoneRoute[ C_Route+1 ] = z_to ;
			memcpy( MinMapRoute, MapRoute, sizeof(int) * (C_Route + 2) );
			memcpy( MinZoneRoute, ZoneRoute, sizeof(int) * (C_Route + 2) );
		}
		return ;
	}

	if ( C_Route + 1 >= hzd->def->n_groups ){ /* 全てのグループを検索した */
		printf("navigate.c: [0x%x][0x%x]Err NavigateMapToMap ALL GROUP !! \n",addr1,addr2) ;
		return ;
	}
	if ( grp->n_link_zones == 0 ) {	/* リンク情報がない */
		//printf( "navigate.c: Err NavigateMapToMap NO LINK !! \n") ;
		return ;
	}

    /* リンクゾーン検索 */
    link = grp->link_zone ;
	for ( i = 0; i < (int)grp->n_link_zones; i ++, link += 2 ) {
		link_zoneadd = ( int )link[ 1 ] ;
	    link_zone = grp->zones + link_zoneadd ;	

#if 1	
		/* 注！！要チェック リンクゾーンが進入禁止の場合 迂回路が有るかどうかに注意 */
		if ( link_zone->flag & HZX_ZONE_ZINTRPT ) now_route_num += 255*10 ;
#else
		if ( link_zone->flag & HZX_ZONE_ZINTRPT ) goto next ;	/* リンクゾーンが通行止め */
#endif

		new_mapno = link_zone->near_flag[ 5 ];

		j=0 ;
		while ( j<C_Route+1 ) {	/* これまでに通ったことのあるまっぷなら */
			if( new_mapno == MapRoute[ j++ ] ) goto next ;
		}
#ifdef ROUTE_COURSE_TEST
		route = ROUTE_COURSE( grp->route_course[RouteCourse], z_from, link_zoneadd, grp->n_zones ) ;
#else
		route = ROUTE( grp->route, z_from, link_zoneadd, grp->n_zones ) ;
#endif
		if ( (now_route_num + route) > Min_Route_Num ) goto next ;

		C_Route ++ ;
		/* マップルートに追加 */
		MapRoute[ C_Route ] = mapno1 ;
		ZoneRoute[ C_Route ] = link_zoneadd ;

		new_addr = HZX_AddressNo( new_mapno, link_zone->nears[ 5 ], link_zone->nears[ 5 ] ) ;
		NavigateMapToMap( hzd, new_addr, addr2, now_route_num + route ) ;
		C_Route -- ;
		
		next :
				;
	}
}

/*----------------------------------------------------------------*/

int		HZX_GetAddress( HZX_GROUP_ID id, FVECTOR *pos, int address )
//FVECTOR		*pos ;
//int			address ;
{
	HZX_ZON		*zone1, *zone2 ;
	int		z1, z2, near ;
	HZX_GRP *grp;
	int mapno, add_mapno;

	grp = HZX_GetGroup( id );
	mapno = GV_GetNo( id );
	add_mapno = HZX_ZoneMapNo( address ) ;
	z1 = HZX_Zone1( address ) ;
	z2 = HZX_Zone2( address ) ;
	
	if ( mapno != add_mapno ) {	/* 現在のマップと入力されたアドレスのマップが違ったら */
		z2 = z1 = HZX_NO_ZONE ;	/* 初期値セット */
	}

	if ( z2 == HZX_NO_ZONE ){	/* z2がノーゾーンならz1をコピーする */
		z2 = z1 ;
	}
	if ( z1 == z2 ) {
		/*
		   Zone1-NoZone 形式から更新
		   ・Zone1 に含まれていれば、Zone1-Zone1
		   ・Zone1-ZoneX 間に含まれていれば、Zone1-ZoneX
		   */
		if ( z1 != HZX_NO_ZONE ) { /* 初期値(-1)ではなければ */
			zone1 = GetZone( grp, z1 ) ;
			if ( InsideZone( zone1, pos ) ) goto end ; /* return zone1:zone1 */
			if ( ( near = BetweenNearZones( grp, zone1, pos ) ) >= 0 ) {
				z2 = near ;
				zone2 = GetZone( grp, z2 ) ;
#if 1	//japanese
				if ( InsideZone( zone2, pos ) ) {
					z1 = z2 ;
					goto end ;		/* return zone2:zone1 or zone2:zone2 */
				}
#else
				if ( InsideZone( zone2, pos ) ) z1 = z2 ;
				goto end ;		/* return zone2:zone1 or zone2:zone2 */
#endif
			}
		}
	} else {
		/*
		   Zone1-Zone2 形式から更新
		   ・Zone1 に含まれていれば、Zone1-Zone1
		   ・Zone2 に含まれていれば、Zone2-Zone2
		   ・隣接ゾーン ZoneX に含まれていれば、ZoneX-ZoneX
		   ・Zone1-Zone2 間に含まれていれば、Zone1-Zone2
		   ・Zone1-ZoneX 間に含まれていれば、Zone1-ZoneX
		   ・Zone2-ZoneX 間に含まれていれば、Zone2-ZoneX
		   */
		zone1 = GetZone( grp, z1 ) ;
		if ( InsideZone( zone1, pos ) ) { z2 = z1 ; goto end ; }
		zone2 = GetZone( grp, z2 ) ;
		if ( InsideZone( zone2, pos ) ) { z1 = z2 ; goto end ; }

		if ( ( near = InsideNearZones( grp, zone1, pos ) ) >= 0
			|| ( near = InsideNearZones( grp, zone2, pos ) ) >= 0 ) {
			z1 = z2 = near ;	goto end ;
		}
#if 0//8.19 BetweenZone以降は全検索
		if ( BetweenZone( zone1, zone2, pos ) ) return address ;

		if ( ( near = BetweenNearZones( grp, zone1, pos ) ) >= 0 ) {
			z2 = near ;	goto end ;
		}
		if ( ( near = BetweenNearZones( grp, zone2, pos ) ) >= 0 ) {
			z1 = near ;	goto end ;
		}
#endif
	}
	/*
	   以上の処理にヒットしなかったので
	   最も近いゾーンを計算しておく
	   */
	z1 = NearestZone( grp, pos ) ;
	if ( z1 >= 0 ) {
	    zone1 = GetZone( grp, z1 ) ;
	    if ( InsideZone( zone1, pos ) ) {
			z2 = z1 ;
	    } else {
			z2 = HZX_NO_ZONE ;
	    }
	} else {
	    z1 = z2 = HZX_NO_ZONE ;
	}
	end :
	return HZX_AddressNo( mapno, z1, z2 ) ;
}

int		HZX_ReachTo( int addr1, int addr2 )
{
	int		z1, z2, z3, z4 ;
	int		mapno1, mapno2, i ;
	char	*link ;
	HZX_ZON	*link_zone ;
    HZX_GRP	*grp, *grp_top ;

	mapno1 = HZX_ZoneMapNo( addr1 ) ;
	z1 = HZX_Zone1( addr1 ) ;
	z2 = HZX_Zone2( addr1 ) ;
	mapno2 = HZX_ZoneMapNo( addr2 ) ;
	z3 = HZX_Zone1( addr2 ) ;
	z4 = HZX_Zone2( addr2 ) ;
	if ( z2 == HZX_NO_ZONE ) z2 = z1 ;
	if ( z4 == HZX_NO_ZONE ) z4 = z3 ;

	grp_top = HZX_GetCurrentHzx()->grp;

	if ( mapno1 != mapno2 ) {
		if ( z1 != z2 || z3 != z4 ) return HZX_UNREACH ;

		grp = grp_top + mapno1;
	    link = grp->link_zone ;
	    for ( i = 0; i < (int)grp->n_link_zones; i ++, link += 2 ) {
			if ( link[ 1 ] == z1 ) {
				link_zone = grp->zones + ( int )z1 ;
				if ( mapno2 == link_zone->near_flag[ 5 ] ) {
					if ( link_zone->nears[ 5 ] == z3 ) {
						return HZX_INDIRECT_REACH ;
					}
				}
			}
		}

		return HZX_UNREACH ;
	}

	if ( z1 == z2 ) {
		if ( z3 == z4 ) {
			/*
			   点１と点３
			   */
			if ( z1 == z3 ) {
				return HZX_REACH ;
			} else if ( ExistRoute( grp_top + mapno1, z1, z3 ) ) {
				return HZX_INDIRECT_REACH ;
			}
		} else {
			/*
			   点１と線３－４
			   */
			if ( z1 == z3 || z1 == z4 ) return HZX_DIRECT_REACH ;
		}
	} else {
		if ( z3 == z4 ) {
			/*
			   線１－２と点３
			   */
			if ( z1 == z3 || z2 == z3 ) return HZX_DIRECT_REACH ;
		} else {
			/*
			   線１－２と線３－４
			   */
			if ( z1 == z3 && z2 == z4 ) {
				return HZX_REACH ;
			} else if ( z1 == z3 || z1 == z4 || z2 == z3 || z2 == z4 ) {
				return HZX_INDIRECT_REACH ;
			}
		}
	}
	return HZX_UNREACH ;
}

int		HZX_Navigate( int addr1, int addr2, FVECTOR *pos )
{
	HZX_HDL *hdl;
	HZX_GRP *grp_top, *grp;
	int		z1, z2, z3, z4 ;
	int		route1, route2, route3, route4 ;
	int		mapno1, mapno2, tmp ;

	hdl = HZX_GetCurrentHzx();
	grp_top = hdl->grp;

	mapno1 = HZX_ZoneMapNo( addr1 ) ;
	mapno2 = HZX_ZoneMapNo( addr2 ) ;
	tmp = addr2 ;/*printf用*/

	/* 目標が別マップの場合 */
	if ( mapno1 != mapno2 ) {
		HZX_NavigateMapToMap( addr1, addr2 ) ;
		/* 今いるマップにある目標への最短リンクゾーンへアドレスを変更 */
		addr2 =  HZX_AddressNo( MinMapRoute[ 1 ], MinZoneRoute[ 1 ], MinZoneRoute[ 1 ] ) ;
		if ( mapno1 != MinMapRoute[ 1 ] ) 	{
			printf("HZX_Navigate_M: Err Err Err !!\n");
			printf("	from[%x] to[%x(%x)] mapno1[%x] MinMapRoute[%x]\n",
					addr1, addr2,tmp, mapno1, MinMapRoute[ 1 ]);
		}
		if ( addr2 == addr1 ) {	/* リンクゾーンへ到着 */
			return HZX_LinkZoneAddress( GV_GetBit( mapno1 ),  HZX_Zone1( addr1 ) ) ;
		}
	}

	z1 = HZX_Zone1( addr1 ) ;
	z2 = HZX_Zone2( addr1 ) ;
	z3 = HZX_Zone1( addr2 ) ;
	z4 = HZX_Zone2( addr2 ) ;

	if ( z2 == HZX_NO_ZONE ) return HZX_AddressNo( mapno1, z1, z1 ) ;
	/*
	   現在、ゾーン内にいる場合
	   */
	grp = grp_top + mapno1;
	if ( z1 == z2 ) {
		/*
		   すでに到着している？
		   */
		if ( z1 == z3 || z1 == z4 ) return HZX_AddressNo( mapno1, z1, z1 ) ;
		/*
		   目標が２ゾーン間ならば、
		   現在のゾーンから近い方へ
		   */
		if ( z4 != z3 && z3 != HZX_NO_ZONE ) {
			route3 = GetRoute( grp, z1, z3 ) ;
			route4 = GetRoute( grp, z1, z4 ) ;
			if ( route4 < route3 ) z3 = z4 ;
		}
		z1 = ZoneNavigate( grp, z1, z3 ) ;
		return HZX_AddressNo( mapno1, z1, z1 ) ;
	}
	/*
	   現在、２ゾーン間にいれば、
	   目標ゾーンに近い方へ
	   */
	route1 = GetRoute( grp, z1, z3 ) ;
	route2 = GetRoute( grp, z2, z3 ) ;
	/*
	   目標が２ゾーン間ならば、
	   もう一つの目標ゾーンもチェック
	   */
	if ( z3 != z4 && z4 != HZX_NO_ZONE ) {
		route3 = GetRoute( grp, z1, z4 ) ;
		route4 = GetRoute( grp, z2, z4 ) ;
		if ( route3 < route1 ) route1 = route3 ;
		if ( route4 < route2 ) route2 = route4 ;
	}
	/*
	   判定が微妙な場合は座標でチェック
	   */
	if ( route1 == route2 ) {
		route1 = (int)Distance( GetZone( grp, z1 ), pos, 0x7f000000 ) ;
		route2 = (int)Distance( GetZone( grp, z2 ), pos, 0x7f000000 ) ;
	}

	if ( route2 < route1 ) z1 = z2 ;
	return HZX_AddressNo( mapno1, z1, z1 ) ;;
}

int		HZX_NavigateMapToMap( int addr1, int addr2 )
{
	Min_Route_Num = 100000 ;
	Min_Map_Num = 16 ;
	C_Route = 0 ;

	MapRoute[ C_Route ] = HZX_ZoneMapNo( addr1 )  ;
	ZoneRoute[ C_Route ] = HZX_Zone1( addr1 ) ;

	NavigateMapToMap( HZX_GetCurrentHzx(), addr1, addr2, C_Route ) ;
#ifdef DEBUG_MDOE
	if ( 0 ){
		int i ;
		printf( "[%s] [%x] to [%x]\n",GV_DebugMes,addr1,addr2 ) ;
		for(i=0;i<Min_Map_Num+2; i++ ) {
			printf(" [%x]z[%x]>",MapRoute[i], ZoneRoute[i]) ;
		}
		printf( " Num[%d]\n",Min_Map_Num+1 ) ;
	}
#endif
	
	return Min_Route_Num ;
}

int	HZX_LinkZoneAddress ( HZX_GROUP_ID id, int zone ) 
{
	HZX_ZON	*link_zone ;
	int		group, z1 ;

    link_zone = HZX_GetZone( id, zone ) ;

	if ( !(link_zone->flag & HZX_ZON_LINK) ) return -1 ;

    group = HZX_GetLinkGroupNo( link_zone ) ;
    z1 = HZX_GetLinkZone( link_zone ) ;

	return HZX_AddressNo( group, z1, z1 ) ;
}

//#ifdef NEW_HZD

/* 目標への次のゾーンを検索 */
int HZX_NextZone( HZX_GROUP_ID id, int zone_from, int zone_to )
{
	int n_zones;
    HZX_GRP	*grp ;
	int i, min_near, min_route ;
	HZX_ZON * zp;

#if 1	//01.10.6 必要っぽい怖いのでＰＡＬから
	if ( zone_from == zone_to ) return zone_from ;
#endif

	grp = HZX_GetGroup( id ) ;
	n_zones = grp->n_zones;
	zp = grp->zones + zone_from;

	min_near = zone_from;
	min_route = MAX_ROUTE;
	for( i = 0; i < 6; i++ ){
		int near, route;

		if( ( near = zp->nears[ i ] ) == HZX_NO_ZONE ) break;
#ifdef ROUTE_COURSE_TEST
		route = ROUTE_COURSE( grp->route_course[RouteCourse], near, zone_to, n_zones );
		if ( RouteCourseNearFlag( RouteCourse, zp->near_flag[ i ] ) ) {
			route += 256 ;
		}
#else
		route = ROUTE( grp->route, near, zone_to, n_zones );
#endif
//printf("i[%d]route%d near%d\n",i,route,near ) ;
		if( route < min_route ){
			min_route = route;
			min_near = near;
		}
	}
	return min_near ;
}

/* 目標への次のゾーンを検索、別グループ対応 */
HZX_ZONE_ADD HZX_NextZoneCrossGroup( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr )
{
    int	grpid1, grpid2, from, to, next ;

	grpid1 = GM_GetBit( HZX_ZoneMapNo( from_addr ) ) ;
	grpid2 = GM_GetBit( HZX_ZoneMapNo( to_addr ) ) ;

	if ( grpid1 == grpid2 ) {
		from = HZX_Zone1( from_addr ) ;
		to = HZX_Zone1( to_addr ) ;
		next = HZX_NextZone( grpid1, from, to ) ;
		return HZX_Address( grpid1, next, next ) ;
	}

	HZX_NavigateMapToMap( from_addr, to_addr ) ;

	grpid1 = GM_GetBit( MinMapRoute[ 0 ] ) ;
	grpid2 = GM_GetBit( MinMapRoute[ 1 ] ) ;

	if ( grpid1 != grpid2 ) {
		from = HZX_LinkZoneAddress ( grpid1, MinZoneRoute[ 0 ] ) ;
	    ASSERT( from >= 0 ) ;
		grpid1 = GM_GetBit( HZX_ZoneMapNo( from ) ) ;
	    ASSERT( grpid1 == grpid2 ) ;

printf("HZX_NextZoneCrossGroup: from[%x]\n",from);
	    return from ;
	}

	next = HZX_NextZone( grpid1, MinZoneRoute[ 0 ], MinZoneRoute[ 1 ] ) ;
printf("HZX_NextZoneCrossGroup: next[%x] from[%x] to[%x]\n",next, MinZoneRoute[ 0 ], MinZoneRoute[ 1 ] );
	return HZX_Address( grpid1, next, next ) ;
}

/* 目標への次のゾーンを検索、別グループ対応 日本版緊急バグ対処*/
HZX_ZONE_ADD HZX_NextZoneCrossGroup2( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr )
{
    int	grpid1, grpid2, from, to, next, zone1, zone2 ;

	grpid1 = GM_GetBit( HZX_ZoneMapNo( from_addr ) ) ;
	grpid2 = GM_GetBit( HZX_ZoneMapNo( to_addr ) ) ;

	if ( grpid1 == grpid2 ) {
		from = HZX_Zone1( from_addr ) ;
		to = HZX_Zone1( to_addr ) ;
		next = HZX_NextZone( grpid1, from, to ) ;
		return HZX_Address( grpid1, next, next ) ;
	}

	HZX_NavigateMapToMap( from_addr, to_addr ) ;

	grpid1 = GM_GetBit( MinMapRoute[ 0 ] ) ;
	grpid2 = GM_GetBit( MinMapRoute[ 1 ] ) ;
	zone1 = MinZoneRoute[ 0 ] ;
	zone2 = MinZoneRoute[ 1 ] ;

	if ( (MinZoneRoute[ 0 ] == MinZoneRoute[ 1 ]) && (Min_Map_Num >= 1) ) {
		/* formゾーンがリンクゾーンの場合 */
		grpid1 = GM_GetBit( MinMapRoute[ 1 ] ) ;
		grpid2 = GM_GetBit( MinMapRoute[ 2 ] ) ;
		zone1 = MinZoneRoute[ 1 ] ;
		zone2 = MinZoneRoute[ 2 ] ;
printf("HZX_NextZoneCrossGroup: from==link zon1[%x] zone2[%x]\n",zone1,zone2);
	}

	if ( grpid1 != grpid2 ) {
		from = HZX_LinkZoneAddress ( grpid1, zone1 ) ;
	    ASSERT( from >= 0 ) ;
		grpid1 = GM_GetBit( HZX_ZoneMapNo( from ) ) ;
	    ASSERT( grpid1 == grpid2 ) ;

//printf("HZX_NextZoneCrossGroup: from[%x]\n",from);
	    return from ;
	}

	next = HZX_NextZone( grpid1, zone1, zone2 ) ;
	return HZX_Address( grpid1, next, next ) ;
}



/* 同マップ内で２つのゾーンがどれだけ離れているかを計算する */
/* ちょっと重い */
int HZX_ZoneDistance( HZX_GROUP_ID id, int zone_from, int zone_to )
{
	int n_zones;
    HZX_GRP	*grp ;

	grp = HZX_GetGroup( id ) ;

	Dist = 0;

	n_zones = grp->n_zones;
	while( zone_from != zone_to ){
		int i, min_route, min_near, min_dist;
		HZX_ZON * zp;

		min_route = MAX_ROUTE;
		min_near = zone_from;
		min_dist = 0;
		zp = grp->zones + zone_from;

		for( i = 0; i < 6; i++ ){
			int near, route;

			if( ( near = zp->nears[ i ] ) == HZX_NO_ZONE ) break;
#ifdef ROUTE_COURSE_TEST
			route = ROUTE_COURSE( grp->route_course[RouteCourse], near, zone_to, n_zones );
			if ( RouteCourseNearFlag( RouteCourse, zp->near_flag[ i ] ) ) {
				route += 256 ;
			}
//printf("route[%d] near[%d] zone_to=%d, n_zones=%d \n",route,near, zone_to, n_zones ) ;
#else
			route = ROUTE( grp->route, near, zone_to, n_zones );
printf("route[%d] near[%d] zone_to=%d, n_zones=%d \n",route, near, zone_to, n_zones ) ;
/*
zone=152, sum=3, dis=3
zone=217, sum=7, dis=4
zone=214, sum=11, dis=4
zone=215, sum=14, dis=3
*/
#endif
			if( route < min_route ){
				min_route = route;
				min_near = near;
				min_dist = zp->dists[ i ];
			}
		}
		Dist += min_dist;
		if ( min_near == zone_from ) { /* ゾーンが繋がっていなかったら */
			printf(" no reach zone from %x to %x \n", zone_from, zone_to ) ;
			Dist = 100000 ;
			break ;
		}
		zone_from = min_near;
//		printf("RouteCourse[%d] zone=%d, sum=%d, dis=%d \n",RouteCourse,min_near,Dist, min_dist ) ;
	}
	return Dist;
}

/* 別マップの２つのゾーンがどれだけ離れているかを計算する */
/* 未完の大作 */
/* 遂に完結 00.9.7 */
int HZX_ZoneDistanceCrossGroup( int from_addr, int to_addr )
{
    int	grpid1, grpid2, from, to ;
    int	dist, i ;

	grpid1 = GM_GetBit( HZX_ZoneMapNo( from_addr ) ) ;
	grpid2 = GM_GetBit( HZX_ZoneMapNo( to_addr ) ) ;

	if ( grpid1 == grpid2 ) {
		from = HZX_Zone1( from_addr ) ;
		to = HZX_Zone1( to_addr ) ;
		dist = HZX_ZoneDistance( grpid1, from,  to ) ;
#ifdef DEBUG_MODE
printf(" zone[%x] to [%x] = dis[%d]\n",from_addr, to_addr, dist ) ;
#endif
		return dist ;
	}

	HZX_NavigateMapToMap( from_addr, to_addr ) ;
	dist = 0 ;
	for( i=0; i<Min_Map_Num+1; i++ ) {
		from =  HZX_AddressNo( MinMapRoute[ i ], MinZoneRoute[ i ], MinZoneRoute[ i ] ) ;
		to =  HZX_AddressNo( MinMapRoute[ i+1 ], MinZoneRoute[ i+1 ], MinZoneRoute[ i+1 ] ) ;
		grpid1 = GM_GetBit( MinMapRoute[ i ] ) ;
		grpid2 = GM_GetBit( MinMapRoute[ i+1 ] ) ;
		if ( grpid1 != grpid2 ) {
			from = HZX_LinkZoneAddress ( grpid1, MinZoneRoute[ i ] ) ;
		    ASSERT( from >= 0 ) ;
			grpid1 = GM_GetBit( HZX_ZoneMapNo( from ) ) ;
		    ASSERT( grpid1 == grpid2 ) ;

		}
		dist += HZX_ZoneDistance( grpid1, HZX_Zone1(from), HZX_Zone1(to) ) ;
	}
#ifdef DEBUG_MODE
printf(" zone[%x] to [%x] = dis[%d]\n",from_addr, to_addr, dist ) ;
#endif
	return dist ;
}

/* ルート内で開始ゾーンから指定距離を超えたゾーン番号を返す */
/* ちょっと重い */
int HZX_OverDistanceZone( HZX_GROUP_ID id, long64 zone_from, long64 zone_to,int dis_th )
{
	int n_zones;
    HZX_GRP	*grp ;

	grp = HZX_GetGroup( id ) ;

	Dist = 0;
	n_zones = grp->n_zones;
	while( zone_from != zone_to ){
		long64 i, min_route, min_near, min_dist;
		HZX_ZON * zp;

		min_route = MAX_ROUTE;
		min_near = zone_from;
		min_dist = 0;
		zp = grp->zones + zone_from;

		for( i = 0; i < 6; i++ ){
			long64 near, route;

			if( ( near = zp->nears[ i ] ) == HZX_NO_ZONE ) break;
#ifdef ROUTE_COURSE_TEST
			route = ROUTE_COURSE( grp->route_course[RouteCourse], near, zone_to, n_zones );
			if ( RouteCourseNearFlag( RouteCourse, zp->near_flag[ i ] ) ) {
				route += 256 ;
			}
#else
			route = ROUTE( grp->route, near, zone_to, n_zones );
#endif
			if( route < min_route ){
				min_route = route;
				min_near = near;
				min_dist = zp->dists[ i ];
			}
		}
		Dist += (int)min_dist;
		if ( Dist > dis_th ) return (int)zone_from ;
		if ( min_near == zone_from ) { /* ゾーンが繋がっていなかったら */
			printf("over no reach zone from %d to %d \n", zone_from, zone_to ) ;
			return (int)zone_from ;
			break ;
		}
		zone_from = min_near;
		//fprintf(1,"zone=%d, sum=%d, dis=%d \n",min_near,dist, min_dist ) ;
	}
	return (int)zone_from ;
}

/* ルート内で開始ゾーンから画面外の最初のゾーン番号を返す */
int HZX_BoundOutZone( HZX_GROUP_ID id, int zone_from, int zone_to, int dis_th )
{
	int n_zones, a;
    HZX_GRP	*grp ;
    FMATRIX	m ;
    FVECTOR	b_min, b_max ;

	grp = HZX_GetGroup( id ) ;

	Dist = 0;
	n_zones = grp->n_zones;
	a=0 ;
printf("navigate: bound out zone [%x][%d][%d] [%d]\n",id,zone_from,zone_to,dis_th) ;
	if ( zone_from == HZX_NO_ZONE || zone_to == HZX_NO_ZONE ) {
printf("navigate: ERR \n" ) ;
		return zone_from ;
	}
	while( zone_from != zone_to ){
		int i, min_route, min_near, min_dist;
		HZX_ZON * zp;

		ASSERT( a++ < n_zones ) ;

		min_route = MAX_ROUTE;
		min_near = zone_from;
		min_dist = 0;
		zp = grp->zones + zone_from;

		for( i = 0; i < 6; i++ ){
			int near, route;

			if( ( near = zp->nears[ i ] ) == HZX_NO_ZONE ) break;
#ifdef ROUTE_COURSE_TEST
			route = ROUTE_COURSE( grp->route_course[RouteCourse], near, zone_to, n_zones );
			if ( RouteCourseNearFlag( RouteCourse, zp->near_flag[ i ] ) ) {
				route += 256 ;
			}
#else
			route = ROUTE( grp->route, near, zone_to, n_zones );
#endif
			if( route < min_route ){
				min_route = route;
				min_near = near;
				min_dist = zp->dists[ i ];
			}
		}
		
		zp = grp->zones + min_near;

		Dist += min_dist;
		if ( Dist > dis_th ) {
			FVECTOR	pos ;
			
			pos.vx = zp->x ;
			pos.vy = zp->y ;
			pos.vz = zp->z ;
			
			if ( !(HZX_CurrentGroupID & id) ) {
				printf("  No Drow Map!![%x]",id) ;
#if	0
				return zone_from ;
#else
				return min_near ;
#endif
			}
			m = DG_UnitMatrix ;
			m.m[3][0] = pos.vx ;
			m.m[3][1] = pos.vy ;
			m.m[3][2] = pos.vz ;

			b_min.vx = - (float)zp->w ;
			b_min.vy = 0.0f ;
			b_min.vz = - (float)zp->h ;

			b_max.vx = (float)zp->w ;
			b_max.vy = 2000.0f ;
			b_max.vz = (float)zp->h ;

			if ( DG_BoundCheck( &m, &b_max, &b_min ) < 0 ) {	/* バウンドＢＯＸチェック */
printf("[%d]:pos[%f][%f][%f] w[%d] h[%d]\n",zone_from, pos.vx, pos.vy, pos.vz, zp->w, zp->h ) ;
#if	0
				return zone_from ;
#else
				return min_near ;
#endif
			}
		}
		if ( min_near == zone_from ) { /* ゾーンが繋がっていなかったら */
			printf("bound no reach zone from %d to %d \n", zone_from, zone_to ) ;
			return zone_from ;
			break ;
		}

		zone_from = min_near;
		//printf("zone=%d, sum=%d, dis=%d \n",min_near,dist, min_dist ) ;
	}
	return zone_from ;
}

/* ルート内で開始ゾーンから画面外の最初のゾーンアドレスを返す */
int HZX_BoundOutZoneCrossGroup( int from_addr, int to_addr, int dis_th )
{
    int	grpid1, grpid2, from, to ;
    int	BO_Zone, dist, i ;

	grpid1 = GM_GetBit( HZX_ZoneMapNo( from_addr ) ) ;
	grpid2 = GM_GetBit( HZX_ZoneMapNo( to_addr ) ) ;

	if ( grpid1 == grpid2 ) {
		from = HZX_Zone1( from_addr ) ;
		to = HZX_Zone1( to_addr ) ;
		BO_Zone = HZX_BoundOutZone( grpid1, from, to, dis_th ) ;
		return HZX_Address( grpid1, BO_Zone, BO_Zone ) ;
	}

	HZX_NavigateMapToMap( from_addr, to_addr ) ;
	dist = 0 ;
	for( i=0; i<Min_Map_Num+1; i++ ) {
		from =  HZX_AddressNo( MinMapRoute[ i ], MinZoneRoute[ i ], MinZoneRoute[ i ] ) ;
		to =  HZX_AddressNo( MinMapRoute[ i+1 ], MinZoneRoute[ i+1 ], MinZoneRoute[ i+1 ] ) ;
		grpid1 = GM_GetBit( MinMapRoute[ i ] ) ;
		grpid2 = GM_GetBit( MinMapRoute[ i+1 ] ) ;
		if ( grpid1 != grpid2 ) {
			from = HZX_LinkZoneAddress ( grpid1, MinZoneRoute[ i ] ) ;
		    ASSERT( from >= 0 ) ;
			grpid1 = GM_GetBit( HZX_ZoneMapNo( from ) ) ;
		    ASSERT( grpid1 == grpid2 ) ;
		}
		BO_Zone = HZX_BoundOutZone( grpid1, HZX_Zone1(from), HZX_Zone1(to), dis_th - dist ) ;
		if ( BO_Zone != MinZoneRoute[ i+1 ] ) {
			return HZX_Address( grpid1, BO_Zone, BO_Zone ) ;
		}
		dist += Dist ;
	}
	
	return to_addr ;
}

int	HZX_InsideZone( HZX_GROUP_ID id, FVECTOR *pos, int zone )
{
	HZX_ZON		*zp ;			/* ゾーン構造体		*/

	zp = HZX_GetZone( id, zone ) ;
	return	InsideZone( zp, pos ) ;

}

int	HZX_NearZones( HZX_GROUP_ID id, int zone, int *near )
{
	return	NearZones( HZX_GetGroup( id ), zone, near ) ;
}

/*
   目標のゾーンから遠くなるために
   次に行くべきゾーンを計算する
   */
int	HZX_FarZoneNavigate( HZX_GROUP_ID id, int zone1, int zone2, int *rout )
//int		zone1 ;		/* 現在のゾーン		*/
//int		zone2 ;		/* 目標のゾーン		*/
//int		*rout ;		/* ルート数	*/
{
	u_char		*nears ;
	int		n_zones, i, near ;
	int		route, max_route, max_near ;
    HZX_GRP	*grp ;

	grp = HZX_GetGroup( id ) ;

	if ( zone1 == zone2 ) return zone1 ;

	nears = grp->zones[ zone1 ].nears ;
	n_zones = grp->n_zones ;
	max_route = 0 ;
	max_near = zone1 ;
	for ( i = 6 ; i > 0 ; -- i ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
#ifdef ROUTE_COURSE_TEST
		route = ROUTE_COURSE( grp->route_course[RouteCourse], near, zone2, n_zones );
		if ( RouteCourseNearFlag( RouteCourse, grp->zones[zone1].near_flag[6-i] ) ) {
//			route += 256 ;
			route = 0 ;	//far なのでいけない場所は近いと判定する
		}
#else
		route = ROUTE( grp->route, near, zone2, n_zones );
#endif
		if ( route > max_route ) {
			max_route = route ;
			max_near = near ;
		}
	}
	*rout = max_route - 1 ;
	return max_near ;
}
/*重野追加 目標を二つ設定可能*/
int	HZX_FarZoneNavigate2( HZX_GROUP_ID id, int nowzone, int trgzone1, int trgzone2, int *rout )
//int		nowzone ;		/* 現在のゾーン		*/
//int		trgzone1 ;		/* 目標のゾーン1	*/
//int		trgzone2 ;		/* 目標のゾーン2	*/
//int		*rout ;		/* ルート数	*/
{
	u_char		*nears ;
	int		n_zones, i, near ;
	int		route1,route2, max_route1,max_route2, max_near ;
    HZX_GRP	*grp ;

	grp = HZX_GetGroup( id ) ;

	if(
	( nowzone == trgzone1 ) 
	||( nowzone == trgzone2 )
	 ){
		return nowzone ;
	}
	nears = grp->zones[ nowzone ].nears ;
	n_zones = grp->n_zones ;
	max_route1 = 0 ;
	max_route2 = 0 ;
	max_near = nowzone ;
	for ( i = 6 ; i > 0 ; -- i ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
#ifdef ROUTE_COURSE_TEST
		route1 = ROUTE_COURSE( grp->route_course[RouteCourse], near, trgzone1, n_zones );
		if ( RouteCourseNearFlag( RouteCourse, grp->zones[nowzone].near_flag[6-i] ) ) {
//			route1 += 256 ;
			route1 = 0 ;	//far なのでいけない場所は近いと判定する
		}
		route2 = ROUTE_COURSE( grp->route_course[RouteCourse], near, trgzone2, n_zones );
		if ( RouteCourseNearFlag( RouteCourse, grp->zones[nowzone].near_flag[6-i] ) ) {
//			route2 += 256 ;
			route2 = 0 ;	//far なのでいけない場所は近いと判定する
		}
#else
		route1 = ROUTE( grp->route, near, trgzone1, n_zones );
		route2 = ROUTE( grp->route, near, trgzone2, n_zones );
#endif
		if(
		 ( route1 >= max_route1 ) 
		 &&( route2 >= max_route2 ) 
		 ){
			max_route1 = route1 ;
			max_route2 = route2 ;
			max_near = near ;
		}
	}
	*rout = max_route1 - 1 ;
	return max_near ;
}

int	HZX_GetRoute( HZX_GROUP_ID id, int zone1, int zone2 )
//int		zone1 ;		/* 現在のゾーン		*/
//int		zone2 ;		/* 目標のゾーン		*/
{
	HZX_GRP	*grp ;
	int		route ;

	if ( zone1 == zone2 ) return 0 ;

	grp = HZX_GetGroup( id ) ;
#ifdef ROUTE_COURSE_TEST
	route = ROUTE_COURSE( grp->route_course[RouteCourse], zone1, zone2, grp->n_zones );
#else
	route = ROUTE( grp->route, zone1, zone2, grp->n_zones );
#endif
	return route ;
}

/* グループが違うアドレスまでのルート数を計算 */
int	HZX_GetRouteCrossGroup( int addr1, int addr2 )
{
	int		route, mapno1, mapno2 ;

	if ( addr1 == addr2 ) return 0 ;
	
	mapno1 = HZX_ZoneMapNo( addr1 ) ;
	mapno2 = HZX_ZoneMapNo( addr2 ) ;

	if ( mapno1 == mapno2 ) {
		route = HZX_GetRoute( GM_GetBit (mapno1), HZX_Zone1( addr1 ), HZX_Zone1( addr2 ) ) ;
		return route ;
	}
	
	route = HZX_NavigateMapToMap( addr1, addr2 ) ;
	
	return route ;
}


//#endif

/* 一番近いゾーンへの距離を返す */
static	float	NearestZoneLen( pos, n_zones, zones, zon_num )
FVECTOR		*pos ;	
int		n_zones ;
HZX_ZON		*zones ;
int		*zon_num ;
{
    int		i, min_i ;
    float	len, min_len ;

    min_i = -1 ;
    min_len = MAX_MIN_DISTANCE ;
    for ( i = 0; i < n_zones; i ++ ) {
		len = Distance( zones, pos, min_len ) ;
		if ( len < min_len ) {
			min_len = len ;
			min_i = i ;
		}
		zones ++ ;
    }
	//    ASSERT ( min_i >= 0 ) ;
    *zon_num = min_i ;
    return min_len ;
}

/* ゾーン内チェック高さ指定 */
static	int	InsideZoneEx( HZX_ZON *zone, FVECTOR *pos, 
						  float upper, float lower )
//HZX_ZON		*zone ;		/* ゾーン構造体		*/
//FVECTOR		*pos ;		/* 座標			*/
//float		upper, lower ;
{
	float		value, center, width ;

	value = pos->vy ;
	center = zone->y ;
	if ( value < center - lower || value >= center + upper ) return 0 ;
	value = pos->vx ;
	center = zone->x ;	width = ( float )zone->w ;
	//if ( value <= center - width || value > center + width ) return 0 ;
	if ( value < center - width || value >= center + width ) return 0 ;
	value = pos->vz ;
	center = zone->z ;	width = ( float )zone->h ;
	//if ( value <= center - width || value > center + width ) return 0 ;
	if ( value < center - width || value >= center + width ) return 0 ;
	return 1 ;
}
#if 0
/* ゾーン内チェック下 */
static	int	InsideZoneLower( zone, pos )
HZX_ZON		*zone ;		/* ゾーン構造体		*/
FVECTOR		*pos ;		/* 座標			*/
{
	float		value, center, width ;

	if ( pos->vy < zone->y ) return 0 ;
	value = pos->vx ;
	center = zone->x ;	width = ( float )zone->w ;
	if ( value <= center - width || value > center + width ) return 0 ;
	value = pos->vz ;
	center = zone->z ;	width = ( float )zone->h ;
	if ( value <= center - width || value > center + width ) return 0 ;
	return 1 ;
}
#endif

#define	UPPER	(2000.0F)
#define	LOWER	(1000.0F)
/* 指定座標がグループ内ゾーンに含まれる時
   ゾーン番号を返す。
   含まれるかどうかはInsideZoneExを使用し、
   最近のゾーンを検索する。 */
static	int	SearchGroupZone( FVECTOR *pos, HZX_GRP *grp, float *len )
{
    HZX_ZON	*zon ;
    float	l ;
    int		n_zones, i, min ;

    *len = MAX_MIN_DISTANCE ;
    min = -1 ;
    n_zones = grp->n_zones ;
    zon = grp->zones ;
    for ( i = 0; i < n_zones; i ++ ) {
		if ( InsideZoneEx( zon, pos, UPPER, LOWER ) ) {
			l = pos->vy - zon->y ;
			if ( l < 0.0F ) l = -l ;
			if ( l < *len ) {
				*len = l ;
				min = i ;
			}
		}
		zon ++ ;
    }
    return min ;
}

/* 指定位置より下で、一番近いゾーンを求める */
/* ゾーン外のときは-1 */
int				HZX_GetLowerZone( HZX_GROUP_ID hzx_id, FVECTOR *pos )
{
	HZX_HDL		*hdl ;
	HZX_ZON		*cur ;
	float		l, min ;
	int			n_zones, i, grpNo, res ;

	res = -1 ;
	hdl = HZX_GetCurrentHzx() ;
	grpNo = GV_GetNo( ( int )hzx_id ) ;
	min = MAX_MIN_DISTANCE ;
	cur = ( hdl->def->groups + grpNo )->zones ;
	n_zones = ( hdl->def->groups + grpNo )->n_zones ;
    for ( i = 0; i < n_zones; i ++ ) {
		if ( InsideZoneEx( cur, pos, MAX_MIN_DISTANCE, 0.0F ) ) {
			l = pos->vy - cur->y ;
			if ( l < min ) {
				min = l ;
				res = i ;
			}
		}
		cur ++ ;
    }	
	return res ;
}

/* 現在の座標がどのマップに属するかを
   ゾーンから検索する */
int	HZX_GetHzxIDbyZoneEx( HZX_GROUP_ID id, FVECTOR *pos, int *zone_num, int near_check_flag )
{
    int		bit, bit2, bit3, group, zn ;
    int		n_zones, c, z, onlyOne ;
    HZX_ZON	*zone ;
    HZX_GRP	*grp ;
    HZX_HDL	*hzd ;
    float	min_len, len ;
	
    hzd = HZX_GetCurrentHzx() ;
    grp = hzd->grp;
    ASSERT( hzd != NULL ) ;
	
//    if ( id == 0 ) id = ( 0x7FFFFFFF ) & ~( 0x7FFFFFFF >> hzd->def->n_groups );
	if ( id == 0 ) id = HZX_AllMapID ;
	
	onlyOne = 0 ;
    bit = id ;
    bit2 = 0 ;
    c = 0 ;
    /* 座標が属しているゾーンを検索 */
    min_len = MAX_MIN_DISTANCE ;
    *zone_num = HZX_NO_ZONE ;
    while( bit != 0 ) {
		int no;
		no = GV_GetNo( bit ) ;
		bit2 = GV_GetBit( no ) ;
//		if ( bit2 == id && onlyOne == 0 ) onlyOne = 1 ;
//		else onlyOne = 2 ;
		group = no ;
		if ( group >= hzd->def->n_groups ) {
			bit = 0 ;
			goto check_skip1 ;
		}
		grp = hzd->def->groups + group ;
		if ( ( z = SearchGroupZone( pos, grp, &len ) ) >= 0 ) {
			if ( min_len > len ) {
				min_len = len ;
				*zone_num = z ;
				c = bit2 ;
			}
		}
//		zone ++ ;
//		if ( onlyOne == 1 ) goto check_end ;
check_skip1 :
        bit &= ~bit2 ;
		bit2 = 0 ;
    }
    /* 属するゾーンが無い場合 */
    /* 一番近いゾーンがあるマップにする */
    if ( c == 0 ) {
//#ifdef DEBUG_MODE		
#if 0 
		printf( "HzxPos2Zone Warning : %x : %.2f %.2f %.2f\n\t no inside zone. check near zone.\n",
			    id, pos->vx, pos->vy, pos->vz ) ;
#endif
		if ( near_check_flag == 0 ) {
			bit2 = 0 ; 
			goto check_end ;
		}
		bit = id ;
		bit2 = bit3 = 0 ;
		min_len = MAX_MIN_DISTANCE ;
		while( bit != 0 ) {
			int no;
			no = GM_GetID( bit ) ;
			bit2 = GM_GetBit( no ) ;
			group = no;
			if ( group >= hzd->def->n_groups ) {
				bit = 0 ;
				goto check_skip2 ;
			}
			grp = hzd->def->groups + group ;
			n_zones = grp->n_zones ;
			zone = grp->zones ;
			len = NearestZoneLen( pos, n_zones, zone, &zn ) ;
			if ( min_len > len ) {
				min_len = len ;
				bit3 = bit2 ;
				*zone_num = zn ;
			}
check_skip2 :
            bit &= ~bit2 ;
			bit2 = 0 ;
		}	
		bit2 = bit3 ;
    } else {
		bit2 = c ;
    }
check_end :
#ifdef DEBUG_MODE
	if ( bit2 == 0 ) printf( "warning : hzx_id search failed\n" ) ;
#endif
	return bit2 ;
}

int	HZX_GetHzxIDbyZone( HZX_GROUP_ID id, FVECTOR *pos, int *zone_num )
{
	return HZX_GetHzxIDbyZoneEx( id, pos, zone_num, 1 ) ;
}

/* 位置からグループ番号とゾーン番号を返す */
void	HZX_Pos2Zone( pos, g, z )
FVECTOR		*pos ;
int		*g, *z ;
{
    int		id ;
    id = HZX_GetHzxIDbyZoneEx( 0, pos, z, 1 ) ;
    ASSERT( id != 0 ) ;
    *g = GV_GetNo( id );
}

/* 遮断された両脇のゾーン zoneadd[0] zoneadd[1] に返す　*/
void	HZX_GetInterruptZone( FVECTOR *pos, int dir, int len, HZX_ZONE_ADD *zoneadd )
{
	FVECTOR	vec, shift ;
	SVECTOR	rot ;
	int group, z ;

	rot.vx = 0 ;
	rot.vy = dir&4095 ;
	rot.vz = 0 ;
	DG_SetPos2( pos, &rot ) ;

	shift.vx = (float)len ;
	shift.vy = 0.0f ;
	shift.vz = (float)len ;
	DG_PutVector( &shift, &vec, 1 ) ;
//printf("shift0[%f][%f][%f] ",vec.vx,vec.vy,vec.vz ) ;
	HZX_Pos2Zone( &vec, &group, &z ) ;
	zoneadd[0] = HZX_AddressNo( group, z, z ) ;

	shift.vx = -(float)len ;
	DG_PutVector( &shift, &vec, 1 ) ;
	HZX_Pos2Zone( &vec, &group, &z ) ;
//printf("shift1[%f][%f][%f] \n",vec.vx,vec.vy,vec.vz ) ;
	zoneadd[1] = HZX_AddressNo( group, z, z ) ;
}

void	HZX_SetNearFlag( HZX_ZONE_ADD zadd1, HZX_ZONE_ADD zadd2, int flag )
{
	int i, z1, z2 ;
	HZX_ZON	*z ;
	
	if( (0xffff0000 & zadd1) != (0xffff0000 & zadd2) ) {
		printf(" Other Map [%x] [%x] !! \n",zadd1,zadd2 ) ;
		return ;
	}

	z1 = HZX_Zone1( zadd1 ) ;
	z2 = HZX_Zone1( zadd2 ) ;

	z = HZX_GetZoneFromAdd( zadd1 ) ;
	for ( i=0; i<6; i++ ) {
printf("i[%d] near[%x] z2[%x]\n",i,z->nears[i],z2 ) ;
		if( z->nears[i] == z2 ) {
			z->near_flag[i] |= flag ;
			break ;
		}
		if( i==5 ) {
			printf(" No Link Zone [%x] [%x]\n",zadd1,zadd2 ) ;
			return ;
		}
	}

	z = HZX_GetZoneFromAdd( zadd2 ) ;
	for ( i=0; i<6; i++ ) {
		if( z->nears[i] == z1 ) {
			z->near_flag[i] |= flag ;
			break ;
		}
		if( i==5 ) {
			printf(" No Link Zone [%x] [%x]\n",zadd1,zadd2 ) ;
			return ;
		}
	}
}

void	HZX_UnsetNearFlag( HZX_ZONE_ADD zadd1, HZX_ZONE_ADD zadd2, int flag )
{
	int i, z1, z2 ;
	HZX_ZON	*z ;
	
	if( (0xffff0000 & zadd1) != (0xffff0000 & zadd2) ) {
		printf(" Other Map [%x] [%x] !! \n",zadd1,zadd2 ) ;
		return ;
	}

	z1 = HZX_Zone1( zadd1 ) ;
	z2 = HZX_Zone1( zadd2 ) ;

	z = HZX_GetZoneFromAdd( zadd1 ) ;
	for ( i=0; i<6; i++ ) {
		if( z->nears[i] == z2 ) {
			z->near_flag[i] &= ~flag ;
			break ;
		}
		if( i==5 ) {
			printf(" No Link Zone [%x] [%x]\n",zadd1,zadd2 ) ;
			return ;
		}
	}

	z = HZX_GetZoneFromAdd( zadd2 ) ;
	for ( i=0; i<6; i++ ) {
		if( z->nears[i] == z1 ) {
			z->near_flag[i] &= ~flag ;
			break ;
		}
		if( i==5 ) {
			printf(" No Link Zone [%x] [%x]\n",zadd1,zadd2 ) ;
			return ;
		}
	}
}

int	HZX_GetNearFlag( HZX_ZONE_ADD from, HZX_ZONE_ADD to )
{
	int i, z1, z2 ;
	HZX_ZON	*z ;
	
	if( (0xffff0000 & from) != (0xffff0000 & to) ) {
		printf("HZX_GetNeagFlag: Other Map [%x] [%x] !! \n",from,to ) ;
		return -1 ;
	}

	z1 = HZX_Zone1( from ) ;
	z2 = HZX_Zone1( to ) ;

	z = HZX_GetZoneFromAdd( from ) ;
	for ( i=0; i<6; i++ ) {
		if( z->nears[i] == z2 ) {
			return z->near_flag[i] ;
		}
	}

	return -1 ;
}


#if 0		// 下請け関数を作成してGMへ移動

/* キャラクタのグループ変更関数 */
void	HZX_CheckChangeGroup( ctrl, addr )
CONTROL		*ctrl ;
int		*addr ;
{
    int		i, group ;
    HZX_HDL	*hzd ;
    HZX_GRP	*grp ;
    HZX_ZON	*link_zone, *zone ;
    u_char	*link, z, z2 ;

    z = *addr & 0xff ;
    z2 = ( *addr & 0xff00 ) >> 8 ;

    link_zone = ctrl->link_zone ;
    hzd = HZX_GetCurrentHzd( ctrl->map ) ;
    group = GM_GetHzdGroupID( ctrl->map ) ;
    grp = hzd->def->groups + group ;

    ctrl->link_zone = NULL ;
    link = grp->link_zone ;
    for ( i = 0; i < grp->n_link_zones; i ++, link += 2 ) {
		if ( z == z2 && 
			link[ 0 ] == group && link[ 1 ] == z ) {
			//printf( "inside link zone\n" ) ;
			ctrl->link_zone = grp->zones + ( int )z ;
			return ;
		}
    }
    if ( link_zone == NULL ) return ;
    /* リンクゾーンを出て、隣のグループに
	   いったのか？ */
    group = link_zone->near_flag[ 5 ] ;
    z = link_zone->nears[ 5 ] ;
    grp = hzd->def->groups + group ;
    zone = grp->zones + ( int )z ;
    if ( InsideZoneEx( zone, &( ctrl->mov ), UPPER, LOWER ) ) {
		/* 隣のグループにいっている */
		ctrl->map = GM_CurrentMap = GM_GetMapIDfromHzdGroup( group ) ;
		*addr = HZX_GetAddress_M( hzd, &ctrl->mov, HZX_Address_M( ctrl->map, z, z ), ctrl->map ) ;
		printf( "change map %x : %x\n", ctrl->map, *addr ) ;
    }
    return ;
}

#endif

HZX_ZON *HZX_CheckChangeGroup( HZX_GROUP_ID *hzx_id, FVECTOR *pos, 
							  HZX_ZON *prev_link, int *addr )
{
    u_char *link, z, z2;
    HZX_ZON *zone;
    int group_no ;
    HZX_GRP *grp;

    z = *addr & 0xff;
    z2 = ( *addr & 0xFF00 ) >> 8;

    group_no = HZX_GetGroupNo( *hzx_id );
    grp = HZX_GetGroupFromNo( group_no );

 CheckLink:
    link = grp->link_zone;

    // linkzoneにはいっているか
#if 0
		for( i = 0; i < grp->n_link_zones; i++, link += 2 ){
			if( z == z2 && link[ 0 ] == group_no && link[ 1 ] == z ){
				return grp->zones + ( int )z;
			}
		}
#endif
    if ( z == z2 && z != HZX_NO_ZONE ) {
		zone = grp->zones + z ;
		if ( zone->flag & HZX_ZON_LINK ) return zone ;
    }

    if( prev_link == NULL ) return NULL;

    // linkzoneをでて、隣のグループにいったのか
		group_no = prev_link->near_flag[ 5 ];
    z = prev_link->nears[ 5 ];
    grp = HZX_GetGroupFromNo( group_no );
    zone = grp->zones + ( int )z;

    if( InsideZoneEx( zone, pos, UPPER, LOWER ) ){
		/* となりのグループにいった */
		int id;
		id = GV_GetBit( group_no );
		*hzx_id = id;
		*addr = HZX_AddressNo( group_no, ( int )z, ( int )z ) ;
		printf( "ChangeMap %x : %x\n", id, *addr );
		prev_link = NULL;
		z2 = z;
		//隣のゾーンでlinkzoneにはいっているか
			goto CheckLink;
    }
    return NULL;
}

/* マップリンク使用グループ変更チェック */
HZX_ZON		*HZX_CheckChangeGroupMapLink( HZX_GROUP_ID *hzx_id, FVECTOR *from, FVECTOR *to,
										  HZX_ZON *prev_link, int *addr )
{
	int				i, nz ;
	HZX_MAPLINK		*maplink ;
	HZX_GRP			*ngrp ;
	HZX_ZON			*nzon ;
#if 0
	printf( "[%x:%x] check map link!! %f %f %f - %f %f %f\n",
		     *hzx_id, *addr, from->vx, from->vy, from->vz, to->vx, to->vy, to->vz ) ;
#endif
	maplink = HZX_MapLink ;
	for ( i = 0; i < HZX_N_MapLinks; i ++, maplink ++ ) {
		if ( !( ( maplink->hzx_id1 | maplink->hzx_id2 ) & *hzx_id ) ) continue ;
#if 0
		printf( "check %x : ", maplink->hzx_id1 | maplink->hzx_id2 ) ;
		DumpSegment( &maplink->segment ) ;
#endif
		if ( HZX_OnlineHazardCheckOneSegment( &maplink->segment, from, to ) ) {
			if ( maplink->hzx_id1 != *hzx_id ) {
				*hzx_id = maplink->hzx_id1 ;
				ngrp = HZX_CurrentHzx->def->groups + GV_GetNo( maplink->hzx_id1 ) ;
				nz = maplink->z1 ;
				nzon = ngrp->zones + nz ;
			} else {
				*hzx_id = maplink->hzx_id2 ;
				ngrp = HZX_CurrentHzx->def->groups + GV_GetNo( maplink->hzx_id2 ) ;
				nz = maplink->z2 ;
				nzon = ngrp->zones + nz ;
			}
			/* 新たなグループでlinkzoneに入っているか */
			if( InsideZoneEx( nzon, to, UPPER, LOWER ) ) {			
				/* 入っている */
				*addr = HZX_AddressNo( GV_GetNo( *hzx_id ), nz, nz ) ;
				prev_link = nzon ;
			} else {
				/* 入っていない */
				*addr = HZX_GetAddress( *hzx_id, to, -1 ) ;
				prev_link = NULL ;
			}
			printf( "ChangeHzxGroup %x %x : maplink\n", *hzx_id, *addr ) ;
			break ;
		}
	}
	return prev_link ;
}

/* ゾーン間に指定ステータスがたっているゾーンがあればそれを返すなければ -1 */
HZX_ZONE_ADD HZX_ZoneStatusZ2Z( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr, int status )
{
	HZX_ZONE_ADD	next_addr, tmp_addr ;
	int mapno, z ;

	mapno = HZX_ZoneMapNo( from_addr ) ;
	z = HZX_Zone1( from_addr ) ;
	from_addr = HZX_AddressNo( mapno, z, z ) ;

	mapno = HZX_ZoneMapNo( to_addr ) ;
	z = HZX_Zone1( to_addr ) ;
	to_addr = HZX_AddressNo( mapno, z, z ) ;

	if ( HZX_GetZoneFromAdd( from_addr )->flag & status ) return from_addr ; 
	tmp_addr = -1 ;
	while ( from_addr != to_addr ) {
		next_addr = HZX_NextZoneCrossGroup( from_addr, to_addr ) ;

		if ( HZX_Zone1( next_addr ) == 255 || tmp_addr == next_addr) {
			printf(" GM_GetRIntrptZ2Z Err !!!!!\n");
			return -1 ;
		}
		tmp_addr = from_addr ;

printf("next_addr=0x%x from_addr=0x%x to_addr=0x%x\n",next_addr,from_addr,to_addr);
		if ( HZX_GetZoneFromAdd( next_addr )->flag & status ) return next_addr ; 
		from_addr = next_addr ;
	}
	if ( HZX_GetZoneFromAdd( to_addr )->flag & status ) return to_addr ; 
	
	/* 無し */
	return -1 ;
}

/* ゾーンからゾーンが見えるなら０ 見えないと１
	違うマップなら１ */
int HZX_ReadOnlinInfo(int addr1,int addr2)
{
	HZX_GRP *group;
	int zone1,zone2,swap;
	u_int	*info ;	    /* grp->online_info */
	int	n_zones ;		/* グループ内ゾーン数 */
    int	add, bit, shf ;

	/*違うマップだ*/
	if(HZX_ZoneMapNo( addr1 ) != HZX_ZoneMapNo( addr2 ) ){
		return 1;
	}
	zone1 = HZX_Zone1(addr1);
	zone2 = HZX_Zone1(addr2);
	/* 同じゾーンなので見える */
	if(zone1 == zone2){
		return 0;
	}
	if(zone1 > zone2) {
		/*zone1 < zone2 にする*/
		swap = zone1 ;
		zone1 = zone2 ;
		zone2 = swap ;
	}
	group = HZX_GetGroupFromNo( HZX_ZoneMapNo( addr1 ) ) ;
	info  = group->online_info;
	n_zones = group->n_zones ;

	/*読み取り開始*/
	zone1 ++ ; zone2 ++ ;
	bit = ( ( n_zones - 1 ) + ( n_zones - ( zone1 - 1 ) ) )
		* ( zone1 - 1 ) / 2 + ( zone2 - zone1 - 1 ) ;
	add = bit / ( sizeof( u_int ) * 8 ) ;
	shf = 31 - ( bit % ( sizeof( u_int ) * 8 ) ) ;
	return ( *( info + add ) & ( 1 << shf ) ) ;
}

int	HZX_Zone2ZoneCornerCount( HZX_ZONE_ADD from, HZX_ZONE_ADD to )
{
	HZX_ZONE_ADD	nextaddr, corneraddr ;
	HZX_ZONE_ADD	prevaddr;
	int		corner ;

	nextaddr = corneraddr = prevaddr = from ;
	corner = 0 ;
	while ( HZX_ReadOnlinInfo( corneraddr, to ) ) {
		do {	// 現コーナーから見えなくなるまで隣を探す
			prevaddr = nextaddr;
			nextaddr = HZX_NextZoneCrossGroup2( nextaddr, to ) ;
		} while ( !HZX_ReadOnlinInfo( corneraddr, nextaddr ) );
		
		corneraddr = prevaddr;	// コーナーから見える限界の場所
		// (ここで減衰率を計算してかける)
		corner ++ ;
	}
	return corner ;
}

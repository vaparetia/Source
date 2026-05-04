/*
   ZoneLib.c 
   
   HZD_ZONライブラリ
   
   2000/01/21 M.Sonoyama
   
   $Id: ZoneLib.c,v 1.16 2001/07/08 07:52:37 usr02011 Exp $
*/

#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include	<sys/types.h> 
#include	<math.h>

#ifdef _WINDOWS
#include <windows.h>
#include <SFDWIN.h>
#endif
#ifdef _UNIX
#include <SFDUNIX.h>
#endif
#ifdef _LINUX
#include <SFDLINUX.h>
#endif

#include	"Scene2HZX.h"
#include	"HZXStruct2.h"
#include	"fmt_hzx.h"

#define	SAFE_NUM	HZX_MAX_SAFEZONE_NUM	/** ひとつのゾーンに対して確保する安全地帯数  **/
/** SAFEZONE.type に格納されるフラグ **/
#define	SAFE_LOW		(0x0001) /** しゃがめば隠れられる安地 **/

/*敵兵のＳｐｈｅｒｅより狭いゾーンの場合の壁方向*/
/*９０度単位で方向区別 */
#define	SAFE_BEHIND1	(0x0002) /*壁 ｄｉｒ ０方向*/
#define	SAFE_BEHIND2	(0x0004) /*壁 diｒ １０２４方向*/
#define	SAFE_BEHIND3	(0x0008) /*壁 dir ２０４８方向*/
#define	SAFE_BEHIND4	(0x0010) /*壁 dir ３０７２方向*/

/*5点チェックを行ったうち、どの点が死角だったか*/
/*注意 しゃがめば隠れられるSAFE_LOWゾーンの場合 中心のみチェックしている*/
#define	SAFE_POINT1	(0x0020) /*-x -z点*/
#define	SAFE_POINT2	(0x0040) /*+x -z点*/
#define	SAFE_POINT3	(0x0080) /*-x +z点*/
#define	SAFE_POINT4	(0x0100) /*+x +z点*/
#define	SAFE_CENTER	(0x0200) /*ゾーン中心*/

#define BEHIND_FLGS (SAFE_BEHIND1|SAFE_BEHIND2|SAFE_BEHIND3|SAFE_BEHIND4)
/*安地ゾーン距離の上限*/
#define	SAFE_MAX_DIS	15000
/*目標までのゾーンの道順による距離を調べ、上限オーバーなら対象外*/

extern	int	OnlineDebugFlag ;

/*-------------------------------------------------------------------------------*/

/* ゾーン内チェック */
/* 線上は左と上はＯＫ */
static	int	InsideZone( pos, zone, upper, lower )
FVECTOR		*pos ;
HZD_ZON		*zone ;
float		upper, lower ;
{
    float	value, center, width ;

    value = pos->vy - zone->y ;
    if ( value < -lower || value > upper ) return 0 ;
    value = pos->vx ;
    center = zone->x ;	width = zone->w ;
    if ( value < center - width || value >= center + width ) return 0 ;
    value = pos->vz ;
    center = zone->z ;	width = zone->h ;
    if ( value < center - width || value >= center + width ) return 0 ;
    return 1 ;    
}

/* グループ内ゾーン検索
   入っているゾーン番号を返す */
int	SearchGroupZone( pos, grp, len ) 
FVECTOR		*pos ;
HZD_GRP		*grp ;
float		*len ;
{
    HZD_ZON	*zon ;
    float	l ;
    int		n_zones, i, min ;

    *len = 1000000.0F ;
    min = -1 ;
    n_zones = grp->n_zones ;
    zon = grp->zones ;
    for ( i = 0; i < n_zones; i ++ ) {
		if ( InsideZone( pos, zon, 2000.0F, 2000.0F ) ) {
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

/* 一番近いゾーン番号を返す */
int	NearestGroupZone( pos, grp, len ) 
FVECTOR		*pos ;
HZD_GRP		*grp ;
float		*len ;
{
    HZD_ZON	*zon ;
    float	l ;
    int		n_zones, i, min ;

    *len = 1000000.0F ;
    min = -1 ;
    n_zones = grp->n_zones ;
    zon = grp->zones ;
    for ( i = 0; i < n_zones; i ++ ) {
		l = ( pos->vx - zon->x ) * ( pos->vx - zon->x ) ;
		l += ( pos->vy - zon->y ) * ( pos->vy - zon->y ) ;
		l += ( pos->vz - zon->z ) * ( pos->vz - zon->z ) ;
		l = sqrtf( l ) ;
		if ( l < *len ) {
			*len = l ;
			min = i ;
		}
	}
	return min ;
}

/*----------------------------------------------------------------------*/

#ifndef OLD 

/* ルートの設定 */
#define	MAX_ROUTE	(255)

/* 計算用バッファ */
typedef	struct	{
	int		n_list ;			/* 周辺集合の要素数	*/
	u_char		list[ 60 ] ;	/* 周辺集合の要素	*/
} List ;

/* ある行き先について、道順データを計算する */
static	void	MakeRoutes( zones, n_zones, to, routes )
HZD_ZON		*zones ;	/* ゾーン配列		*/
int		n_zones ;	/* ゾーン数		*/
int		to ;		/* 行き先		*/
u_char		*routes ;	/* 道順データ格納	*/
{
    List		Lists[ 2 ] ;
    List		*old, *new ;
    int			i, j, route, tmp ;
    int			n_old, n_new ;
    int			near_n ;
    u_char		*nears ;

    /*
       計算用バッファを初期化
	   */
    for ( i = 0 ; i < n_zones ; i ++ ) routes[ i ] = MAX_ROUTE ;
    routes[ to ] = 0 ;
    old = Lists ;
    new = Lists + 1 ;
    old->n_list = 1 ;
    old->list[ 0 ] = to ;
    route = 1 ;
    /*
       周辺集合がなくなるまで繰り返す
	   */
    while ( ( n_old = old->n_list ) > 0 ) {
		n_new = 0 ;
		for ( i = 0 ; i < n_old ; i ++ ) {
			/*
			   近傍の道順を検索する
			   */
			nears = zones[ old->list[ i ] ].nears ;
			for ( j = 0 ; j < 6 ; j ++ ) {
				if ( ( near_n = *( nears ++ ) ) == MAX_ROUTE ) break ;
				/* ジャンプ用のルートは計算から除外 */
//				if ( zones[ old->list[ i ] ].near_flag[j] & HZX_ROOT_JUMP ) continue ;
				if ( route < routes[ near_n ] ) {
					routes[ near_n ] = route ;
					new->list[ n_new ] = near_n ;
					n_new ++ ;
				}
			}
		}
		/*
		   ダブルバッファを入れ換える
		   */
		new->n_list = n_new ;
		tmp = 1 & route ;
		old = Lists + tmp ;
		new = Lists + ( 1 - tmp ) ;
		route ++ ;
    }
}	

/* 各グループのルート情報を作成 */
void	HZD_MakeRouteGroup( grp, routes )
HZD_GRP		*grp ;
u_char		*routes ;
{	
    HZD_ZON	*zones ;
    int		i, n_zones ;
    u_char *tmp_routes, *r;

    zones = grp->zones ;
    if( ( n_zones = grp->n_zones ) <= 0 ) return ;
    if( ( tmp_routes = ( u_char * )malloc( sizeof( u_char ) * n_zones ) ) == NULL ) return ;
    r = routes;
    for ( i = 0 ; i < n_zones ; i ++ ) {
		long j;
		MakeRoutes( zones, n_zones, i, tmp_routes ) ;
		for( j = i + 1; j < n_zones; j++ ){
			*r = tmp_routes[ j ];
			r++;
		}
    }
    free( tmp_routes );
}


/* ルートの設定 */
#define	MAX_ROUTE_COURSE	(65535)

/* 計算用バッファ */
typedef	struct	{
	int			n_list ;			/* 周辺集合の要素数	*/
//	u_short		list[ 60 ] ;	/* 周辺集合の要素	*/
//	u_short		routes[ 60 ] ;	/* 周辺集合までの経路数	*/
	u_short		list[ 120 ] ;	/* 周辺集合の要素	*/
	u_short		routes[ 120 ] ;	/* 周辺集合までの経路数	*/
} CourseList ;


/* ある行き先について、道順データを計算する */
static	void	MakeRoutesCourse( zones, n_zones, to, routes, course )
HZD_ZON		*zones ;	/* ゾーン配列		*/
int		n_zones ;	/* ゾーン数		*/
int		to ;		/* 行き先		*/
u_short		*routes ;	/* 道順データ格納	*/
int		course ;
{
    CourseList		CourseLists[ 2 ] ;
    CourseList		*old, *new ;
    int			i, j, route, tmp ;
    int			n_old, n_new ;
    int			near_n, r ;
    u_char		*nears ;

    /*
       計算用バッファを初期化
	   */
    for ( i = 0 ; i < n_zones ; i ++ ) routes[ i ] = MAX_ROUTE_COURSE ;
    routes[ to ] = 0 ;
    old = CourseLists ;
    new = CourseLists + 1 ;
    old->n_list = 1 ;
    old->list[ 0 ] = to ;
    old->routes[ 0 ] = 1 ;
    route = 1 ;
    /*
       周辺集合がなくなるまで繰り返す
	   */
    while ( ( n_old = old->n_list ) > 0 ) {
		n_new = 0 ;
		for ( i = 0 ; i < n_old ; i ++ ) {
			/*
			   近傍の道順を検索する
			   */
			nears = zones[ old->list[ i ] ].nears ;
			for ( j = 0 ; j < 6 ; j ++ ) {
				if ( ( near_n = *( nears ++ ) ) == MAX_ROUTE ) break ;

				r = old->routes[ i ] ;
				if ( zones[ old->list[ i ] ].near_flag[j] & (u_char)course ) {
//printf(" special course [%x], zones[%d] near[%d]   \n",course_flag,old->list[ i ],j ) ;
					r += MAX_ROUTE ;
				}

				if ( r < routes[ near_n ] ) {
					routes[ near_n ] = r ;
					new->list[ n_new ] = near_n ;
					new->routes[ n_new ] = r + 1 ;
					n_new ++ ;
				}
			}
		}
		/*
		   ダブルバッファを入れ換える
		   */
		new->n_list = n_new ;
		tmp = 1 & route ;
		old = CourseLists + tmp ;
		new = CourseLists + ( 1 - tmp ) ;
		route ++ ;
    }
}	
/* 各グループのルート情報を作成 */
void	HZD_MakeRouteCourseGroup( grp, routes, course )
HZD_GRP		*grp ;
u_short		*routes ;
int			course ;
{	
    HZD_ZON	*zones ;
    int		i, n_zones ;
    u_short *tmp_routes, *r;

    zones = grp->zones ;
    if( ( n_zones = grp->n_zones ) <= 0 ) return ;
    if( ( tmp_routes = ( u_short * )malloc( sizeof( u_short ) * n_zones ) ) == NULL ) return ;
    r = routes;
    for ( i = 0 ; i < n_zones ; i ++ ) {
		long j;

		MakeRoutesCourse( zones, n_zones, i, tmp_routes, course ) ;
		for( j = i + 1; j < n_zones; j++ ){
			*r = tmp_routes[ j ];
//rintf("%d:%d ",j,*r);
			r++;
		}
    }
    free( tmp_routes );
}

/* コース数を計算 */
static int CourseS( grp )
HZD_GRP		*grp ;
{
	int	n_zones = grp->n_zones ;
	u_char courses = 0 ;
    HZD_ZON	*zones = grp->zones ;
    u_char		*nears ;
    int i ;

	while ( n_zones-- > 0 ){
		nears = zones->nears ;
		for ( i = 0 ; i < 6 ; i ++ ) {
			if ( *( nears ++ ) == MAX_ROUTE ) break ;
			courses |= zones->near_flag[ i ] ;
		}
		zones++ ;
	}

	return (int)courses ;
}

/* ルート情報を作成 */
void	HZD_MakeRoute( hzd )
HZD_DEF	*hzd ;
{
    HZD_GRP	*grp ;
    int		n_groups ;
    int		n_zones ;
    int		route_size ;
    u_char 	*route ;
    u_short	*route_course ;

    n_groups = hzd->n_groups ;
    grp = hzd->groups ;
    while( -- n_groups >= 0 ) {
		n_zones = grp->n_zones ;
		if( n_zones > 1 ){
			route_size = ( ( n_zones - 1 ) * ( n_zones - 2 ) ) / 2 
				+ ( n_zones - 1 );
			route = ( u_char * )malloc( route_size );
			HZD_MakeRouteGroup( grp, route ) ;
			grp->route = route ;
		}
		grp ++ ;
	}
#ifdef ROUTE_COURSE_TEST	// コース別最短ルートテーブル
{
	int	courses, n_course ;

    n_groups = hzd->n_groups ;
    grp = hzd->groups ;
    while( -- n_groups >= 0 ) {
		n_zones = grp->n_zones ;
		courses = CourseS( grp ) ;
printf(" courses [%x] \n",courses ) ;
		grp->courses = courses ;
		if( n_zones > 1 ){
			/* ノーマルテーブル */
			route_size = ( ( n_zones - 1 ) * ( n_zones - 2 ) ) / 2 
				+ ( n_zones - 1 );
			route_course = ( u_short * )malloc( sizeof( u_short ) * route_size );
			HZD_MakeRouteCourseGroup( grp, route_course, 0 ) ;
			grp->route_course[0] = route_course ;

			/* コース１を含んだテーブル */
			if ( courses & HZX_ROOT_COURSE1 ) {
printf(" make route course 1 \n" ) ;
				route_course = ( u_short * )malloc( sizeof( u_short ) * route_size );
				HZD_MakeRouteCourseGroup( grp, route_course, HZX_ROOT_COURSE1 ) ;
				grp->route_course[1] = route_course ;
			} else {
				grp->route_course[1] = grp->route_course[0] ;
			}
			/* コース２を含んだテーブル */
			if ( courses & HZX_ROOT_COURSE2 ) {
printf(" make route course 2 \n") ;
				route_course = ( u_short * )malloc( sizeof( u_short ) * route_size );
				HZD_MakeRouteCourseGroup( grp, route_course, HZX_ROOT_COURSE2 ) ;
				grp->route_course[2] = route_course ;
			} else {
				grp->route_course[2] = grp->route_course[0] ;
			}
			/* コース１とコース２を含まないテーブル */
			if ( courses == (HZX_ROOT_COURSE1|HZX_ROOT_COURSE2) ) {
printf(" make route course 1, 2 \n" ) ;
				route_course = ( u_short * )malloc( sizeof( u_short ) * route_size );
				HZD_MakeRouteCourseGroup( grp,route_course, (HZX_ROOT_COURSE1|HZX_ROOT_COURSE2) ) ;
				grp->route_course[3] = route_course ;
			} else {
				if ( courses & HZX_ROOT_COURSE1 ) {
					grp->route_course[3] = grp->route_course[1] ;
				} else if ( courses & HZX_ROOT_COURSE2 ) {
					grp->route_course[3] = grp->route_course[2] ;
				} else {
					grp->route_course[3] = grp->route_course[0] ;
				}
			}
		}
		grp ++ ;
    }
}
#endif
}

/*----------------------------------------------------------------------*/

#define	ONLINE_RATE 0.75F	/*四隅より少し内側で判定する*/ 
#define	MIN_SEARCH	1		/*近すぎるゾーンは判定外*/
#define	MAX_SEARCH	10		/*遠すぎるゾーンも判定外*/

/*ゾーン中心近接壁 チェック半径 敵兵のｒ sphere*/
#define	S_SPHERE (500) 

#define	HZD_NO_ZONE	(255)
#define	ZONE_HEIGHT	(2000.0F)

static	inline	_sceVu0SubVector( v3, v1, v2 )
FVECTOR		*v3, *v1, *v2 ;
{
    v3->vx = v1->vx - v2->vx ;
    v3->vy = v1->vy - v2->vy ;
    v3->vz = v1->vz - v2->vz ;
}

static	inline	float	_FVecLen3( v ) 
FVECTOR		*v ;
{
    return sqrtf( v->vx * v->vx + v->vy * v->vy +
				 v->vz * v->vz ) ;
}

short		GV_VecDir2( vec )
FVECTOR		*vec ;
{
    float	dir ;
    short	res ;

    dir = atan2f( vec->vx, vec->vz ) ;
    res = 4095 & ( short )( ( dir * 2048.0F / 3.141592653589793 ) + 0.5F ) ; /* 四捨五入 */
    return res ;
}

/* ゾーン番号からゾーン構造体 */
static	inline	HZD_ZON	*HZD_GetZone( grp, n )
HZD_GRP		*grp ;
int		n ;
{
    return ( grp->zones + n ) ;
}

static	inline	int	HZD_Zone1( address )
int			address ;
{
    return 255 & address ;
}

static	inline	int	HZD_Zone2( address )
int			address ;
{
    return 255 & ( address >> 8 ) ;
}

static u_char ROUTE( u_char *route, long z1, long z2, long n_zone )
{
    long index;

    if( z2 < z1 ){
		long tmp;
		tmp = z2;
		z2 = z1;
		z1 = tmp;
    } else if( z1 == z2 ){
		return 0;
    }
    index = ( z1 * ( 2 * n_zone - z1 - 3 ) ) / 2 + z2 - 1;
    return route[ index ];
}

/* 目標のゾーンまでのルート数を返す */
int	HZD_GetRoute( grp, fromadd, toadd )
HZD_GRP			*grp ;
int			fromadd ;	/* 現在のゾーン		*/
int			toadd ;		/* 目標のゾーン		*/
{
    int	route ;

    fromadd &= 255 ;
    toadd &= 255 ;
    if ( fromadd == HZD_NO_ZONE || 
		toadd == HZD_NO_ZONE ) return MAX_ROUTE ;
    if ( fromadd == toadd ) return 0 ;
    route = ROUTE( grp->route, fromadd, toadd, grp->n_zones ) ;
    return route ;
}

/*
   座標からゾーンまでのキョリを計算
   高さは ZONE_HEIGHT 以内なら、0距離とする
   腰の位置で
   */
static	float	Distance( zone, pos, minlen )
HZD_ZON		*zone ;		/* ゾーン構造体		*/
FVECTOR		*pos ;		/* 座標			*/
float		minlen;		/* 現在の最小距離 */
{
    float	sum, len ;

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

    len = zone->y - pos->vy ;
    if ( len < 0 ) len = -len ;
    if ( len > ZONE_HEIGHT ) sum += len ;
    
    return sum ;
}

/*
   目標のゾーンへ向かうために
   次に行くべきゾーンを計算する
   pos は、判定が微妙な場合に使用
   */
static	int	ZoneNavigate( grp, zone1, zone2 )
HZD_GRP		*grp ;
int		zone1 ;		/* 現在のゾーン		*/
int		zone2 ;		/* 目標のゾーン		*/
{
    u_char	*nears ;
    int		n_zones, i, near_n ;
    int		route, min_route, min_near, group ;

    if ( zone1 == zone2 ) return zone1 ;

    nears = grp->zones[ zone1 ].nears ;
    n_zones = grp->n_zones ;
    min_route = MAX_ROUTE ;
    min_near = zone1 ;
    for ( i = 6 ; i > 0 ; -- i ) {
		if ( ( near_n = *( nears ++ ) ) == HZD_NO_ZONE ) break ;
		route = ROUTE( grp->route, near_n, zone2, n_zones );
		if ( route < min_route ) {
			min_route = route ;
			min_near = near_n ;
		}
    }
    return min_near ;
}

/* 同じ距離なら先に検索されたゾーンを選択 */
int		HZD_Navigate( grp, addr1, addr2, pos )
HZD_GRP		*grp ;
int		addr1 ;
int		addr2 ;
FVECTOR		*pos ;
{
    int		z1, z2, z3, z4 ;
    int		route1, route2, route3, route4 ;

    z1 = HZD_Zone1( addr1 ) ;
    z2 = HZD_Zone2( addr1 ) ;
    z3 = HZD_Zone1( addr2 ) ;
    z4 = HZD_Zone2( addr2 ) ;
    if ( z2 == HZD_NO_ZONE ) return z1 ;
    /*
       現在、ゾーン内にいる場合
	   */
    if ( z1 == z2 ) {
		/*
		   すでに到着している？
		   */
		if ( z1 == z3 || z1 == z4 ) return z1 ;
		/*
		   目標が２ゾーン間ならば、
		   現在のゾーンから近い方へ
		   */
		if ( z4 != z3 && z3 != HZD_NO_ZONE ) {
			route3 = HZD_GetRoute( grp, z1, z3 ) ;
			route4 = HZD_GetRoute( grp, z1, z4 ) ;
			if ( route4 < route3 ) z3 = z4 ;
		}
		return ZoneNavigate( grp, z1, z3 ) ;
    }
    /*
       現在、２ゾーン間にいれば、
       目標ゾーンに近い方へ
	   */
    route1 = HZD_GetRoute( grp, z1, z3 ) ;
    route2 = HZD_GetRoute( grp, z2, z3 ) ;
    /*
       目標が２ゾーン間ならば、
       もう一つの目標ゾーンもチェック
	   */
    if ( z3 != z4 && z4 != HZD_NO_ZONE ) {
		route3 = HZD_GetRoute( grp, z1, z4 ) ;
		route4 = HZD_GetRoute( grp, z2, z4 ) ;
		if ( route3 < route1 ) route1 = route3 ;
		if ( route4 < route2 ) route2 = route4 ;
    }
    /*
       判定が微妙な場合は座標でチェック
	   */
    if ( route1 == route2 ) {
		route1 = Distance( HZD_GetZone( grp, z1 ), pos, 0x7f000000 ) ;
		route2 = Distance( HZD_GetZone( grp, z2 ), pos, 0x7f000000 ) ;
    }
    if ( route2 < route1 ) z1 = z2 ;
    return z1 ;
}

#define _WAddr(x) ((x)|(x<<8))
/* ゾーン間の距離を求める */
static	int GetRouteDis(grp, firstpos,finalpos,firstaddr,finaladdr,max)
HZD_GRP		*grp ;
FVECTOR		*firstpos; /*スタート座標*/
FVECTOR		*finalpos; /*目標座標*/
int		firstaddr;/*スタートアドレス*/
int		finaladdr; /*目標アドレス*/
u_int		max;	/*距離がこの値をオーバーしたら計算を中断する*/
{
    FVECTOR		strpos,trgpos,goingpos,subvec;
    HZD_ZON		*zone;
    int			i,nowaddr,goingaddr,dis = 0,tmpdis;

    firstaddr &= 255 ;
    finaladdr &= 255 ;
    /*スタート座標セット*/
    if(firstpos == NULL){
		zone = HZD_GetZone( grp, firstaddr ) ;
		strpos.vx = zone->x;
		strpos.vy = zone->y;
		strpos.vz = zone->z;
    }else {
		strpos = *firstpos;
    }
    /*目標座標セット*/
    if(finalpos == NULL){
		zone = HZD_GetZone( grp, finaladdr ) ;
		trgpos.vx = zone->x;
		trgpos.vy = zone->y;
		trgpos.vz = zone->z;
    }else {
		trgpos = *finalpos;
    }
    nowaddr = firstaddr;
    /************/
    /*最初の一回目*/
    goingaddr = HZD_Navigate( grp,_WAddr(nowaddr),_WAddr(finaladdr), &strpos );
    if( goingaddr == finaladdr ){
		/*隣接あるいは同一ゾーン内なので直接座標で比較*/
		_sceVu0SubVector( &subvec, &strpos, &trgpos ) ;
		dis = _FVecLen3( &subvec ) ;
		return dis;
    }else {
		/*まだ到達していないので次のゾーン中心と比較*/
		zone = HZD_GetZone( grp, goingaddr ) ;
		goingpos.vx = zone->x;
		goingpos.vy = zone->y;
		goingpos.vz = zone->z;
		_sceVu0SubVector(  &subvec, &strpos, &goingpos ) ;
		dis = _FVecLen3( &subvec ) ;
		nowaddr = goingaddr;
		strpos = goingpos;
    }
    while(1){
		goingaddr = HZD_Navigate( grp, _WAddr(nowaddr),_WAddr(finaladdr), &strpos );
		if( goingaddr == finaladdr ){
			/*最終目標に到達した*/
			_sceVu0SubVector(  &subvec, &strpos, &trgpos ) ;
			dis += _FVecLen3( &subvec ) ;
			return dis;
		} else {
			if(goingaddr == nowaddr ){
				/*ゾーン誘導で行けない場所に目標がある*/
				_sceVu0SubVector( &subvec, &strpos, &trgpos ) ;
				dis += _FVecLen3( &subvec ) ;
				return dis;
			}
			/*まだ到達していないので次のゾーン中心と比較*/
			zone  = HZD_GetZone( grp, nowaddr ) ;
			for(i=0;i<6;i++){
				if(zone->nears[i] == goingaddr){
					/*ゾーン間距離を復元*/
					tmpdis = (int)zone->dists[i];
					dis += (tmpdis-1)*500;
					/*上限オーバー*/
					if(dis > max) return dis;
					nowaddr = goingaddr;
					zone  = HZD_GetZone( grp, goingaddr ) ;
					strpos.vx = zone->x;
					strpos.vy = zone->y;
					strpos.vz = zone->z;
					break;
				}
			}
		}
    }
}

/*--------------------------------------------------------------*/

/* 安全地帯構造体作成 */
void 	HZD_SafeZoneMake( hzd )
HZD_DEF	*hzd;
{
    HZD_GRP		*grp ;
    HZD_ZON		*zone1,*zone2 ; 
    FVECTOR		vect_ptr[2];	/*壁判定*/
    FVECTOR		pos1,pos2,w_check,hitpoint;
    int 		addr1,addr2,hit_num,hzd_dir;
    int 		i,j,k,l,m,type,group, n_groups;
    int			tmproute,num_zones,zone_dis;
    float		tmp;

    grp = hzd->groups ;
    n_groups = hzd->n_groups ;
    while( -- n_groups >= 0 ) {
		/*ゾーン数取得*/
		num_zones = grp->n_zones;
		/*ゾーン番号０からチェック開始*/
		for(addr1 = 0;addr1<num_zones;addr1++){
			if(addr1==0){
				printf("SAFEZONE CHECK START\n");
				printf("TOTAL ZONE NUM %d\n",num_zones);
				printf("NOW CHECKING. PLEASE WAIT\n");
			}
			/*仮想プレイヤゾーンセット*/
			zone1 = HZD_GetZone( grp, addr1 ) ;
			pos1.vx = (float)zone1->x;
			pos1.vy = (float)(zone1->y+1500.0F); /*目線の高さで判定*/
			pos1.vz = (float)zone1->z;
			/*格納先を初期化*/
			for(j=0;j<(SAFE_NUM);j++) {
				zone1->safes[j] = 255;
				zone1->safe_dists[j] = 255;
				zone1->safe_types[j] = 0 ;
			}
			/** ゾーン数だけループ **/
			for(addr2=0;addr2<num_zones;addr2++){
				/*同一ゾーン同士は不要*/
				if(addr1==addr2) continue;
				/*リーチ数が規定内か？*/
				tmproute = 1 + HZD_GetRoute( grp, addr1, addr2 ) ;
				if((MIN_SEARCH>=tmproute)||(MAX_SEARCH <= tmproute)) continue;
				/*ゾーン間距離が規定内か？*/
				zone_dis = GetRouteDis( grp,NULL,NULL,addr1,addr2,SAFE_MAX_DIS);
				//printf( "%d %d %d\n", addr1, addr2, zone_dis / 500 + 1 ) ;
				if(zone_dis >= SAFE_MAX_DIS) continue ;
				/*u_charに収める*/
				zone_dis = (zone_dis/500)+1;
				if(zone_dis>255) zone_dis = 255 ;
				/*チェック目標設定*/
				zone2 = HZD_GetZone( grp, addr2 ) ;

				if ( zone2->flag & HZX_ZONE_ZINTRPT ) continue ;	

				hit_num = 0;
				type = 0;
				pos2.vy = (float) (zone2->y+1500);
				/*高さが違い過ぎる場合 無効*/
				tmp = pos1.vy - pos2.vy;
				if((tmp>ZONE_HEIGHT)||(tmp< -ZONE_HEIGHT )) continue;
				/*最初に中心チェック*/
				pos2.vx = (float)zone2->x;
				pos2.vz = (float)zone2->z;
				if(HZD_OnlineHazardCheck( grp,&pos1,&pos2, 0, HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY ) ){
					hit_num++;
					type |= (SAFE_CENTER);
				}
				/*めり込み判定用に中心セット*/
				w_check.vx = (float)(zone2->x);
				w_check.vy = (float)(zone2->y+1500);
				w_check.vz = (float)(zone2->z);
				/*四隅をチェック*/
				for(i=0;i<4;i++){
					/*チェック用ベクトル生成*/
					if(i&1)	pos2.vx = (float)zone2->x+((float)zone2->w*ONLINE_RATE);
					else	pos2.vx = (float)zone2->x-((float)zone2->w*ONLINE_RATE);
					if(i<2)	pos2.vz = (float)zone2->z-((float)zone2->h*ONLINE_RATE);
					else	pos2.vz = (float)zone2->z+((float)zone2->h*ONLINE_RATE);
					/*壁めり込み判定*/
					/*ゾーンの中心からチェック*/
					if(HZD_OnlineHazardCheck(grp,&w_check,&pos2,0,HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY ) ){
						/*ゾーン内部に壁あり*/
						HZD_GetOnlinePoint(&hitpoint);
						pos2 = hitpoint;
						if(pos2.vx > w_check.vx ) pos2.vx -= 5.0F;
						else  pos2.vx += 5.0F;
						if(pos2.vz > w_check.vz ) pos2.vz -= 5.0F;
						else  pos2.vz += 5.0F;
					}
					/*オンラインチェック*/
					if(HZD_OnlineHazardCheck(grp,&pos1,&pos2,0,HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY ) )
					{
						hit_num++;
						switch (i) {
						case 0:
							type |= (SAFE_POINT1) ;
							break ;
						case 1:
							type |= (SAFE_POINT2) ;
							break ;
						case 2:
							type |= (SAFE_POINT3) ;
							break ;
						case 3:
							type |= (SAFE_POINT4) ;
							break ;
						}
					}
				}
				/* 高さ１５００で壁が見つからなかった場合低い位置で再チェック*/
				/* 3ゾーン以上離れているときのみ */
				if((hit_num==0)&&(3 < tmproute)){
					/*ゾーンの中心のみ*/
					pos2.vx = (float)zone2->x;
					pos2.vy = (float)(zone2->y+500);
					pos2.vz = (float)zone2->z;
					if(HZD_OnlineHazardCheck(grp,&pos1,&pos2,0,HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY ) )
					{
						hit_num++;
						type |= (SAFE_LOW|SAFE_CENTER);
					}
				}
				/*安地検出*/
				if(hit_num > 0)
				{
					/*低いゾーンでなければゾーン近接壁チェック*/
					if(!(type&SAFE_LOW)){
						int hzd_check;
						w_check.vx = (float)(zone2->x);
						w_check.vy = (float)(zone2->y+1500);
						w_check.vz = (float)(zone2->z);
						hzd_check = HZD_NearHazardCheck(grp,&w_check,S_SPHERE,S_SPHERE,
														0,HZX_SEG_NO_ENEMY ) ;
						/*壁検出 壁の方向によりフラグ立て*/
						if(hzd_check>0) {
							HZD_GetNearVector( vect_ptr );
							for(m=0;m<hzd_check;m++){
								hzd_dir = GV_VecDir2( &vect_ptr[i] );
								if( ( 3072+512 <= hzd_dir )||(hzd_dir < 512 )){
									type |= SAFE_BEHIND1;
								}else if( (  0+512 <=hzd_dir)&&(hzd_dir< 1024+512 )){
									type |= SAFE_BEHIND2;
								}else if( (1024+512 <= hzd_dir)&&(hzd_dir < 2048+512)){
									type |= SAFE_BEHIND3;
								}else if( (2048+512 <= hzd_dir)&&(hzd_dir < 3072+512)){ 
									type |= SAFE_BEHIND4;
								}
							}
						}
					}
					/*過去に検出したゾーンと距離比較*/
					for(k=0;k<(SAFE_NUM);k++){
						if(zone1->safe_dists[k] > zone_dis) {
							/*安地更新された*/
							/*ソート格納準備*/
							for(l=(SAFE_NUM-2);l>=k;l--){
								zone1->safes[l+1] = zone1->safes[l];
								zone1->safe_dists[l+1] = zone1->safe_dists[l];
								zone1->safe_types[l+1] = zone1->safe_types[l];
							}
							zone1->safe_dists[k] = zone_dis;
							zone1->safes[k] = addr2;
							zone1->safe_types[k] = type;
							break;
						}
					}
				}
            }
        }
		grp ++ ;
    }
    printf("SAFEZONE CHECK END\n");
}

/*------------------------------------------------------------------*/

/* １オンライン情報の書き込み */
static	void	WriteOnlineInfo( info, i, j, n )
u_int		*info ;
int		i, j ;
int		n ;
{
    int		add, bit, shf ;

    i ++ ; j ++ ;
    bit = ( ( n - 1 ) + ( n - ( i - 1 ) ) ) * ( i - 1 ) / 2 + ( j - i - 1 ) ;
    add = bit / ( sizeof( u_int ) * 8 ) ;
    shf = 31 - ( bit % ( sizeof( u_int ) * 8 ) ) ;
    *( info + add ) |= ( 1 << shf ) ;
}

#if 1
/* ゾーンｉからゾーンｊが見えるかどうかを返す。
   見える時０、見えない時１か返る。 */
int	HZX_ReadOnlineInfo( info, i, j, n_zones ) 
u_int	*info ;	/* grp->online_info */
int	i, j ;	/* ゾーン番号 ( i < j ) */
int	n_zones ; /* グループ内ゾーン数 */
{
    int		add, bit, shf ;

    i ++ ; j ++ ;
    bit = ( ( n_zones - 1 ) + ( n_zones - ( i - 1 ) ) ) * ( i - 1 ) / 2 + ( j - i - 1 ) ;
    add = bit / ( sizeof( u_int ) * 8 ) ;
    shf = 31 - ( bit % ( sizeof( u_int ) * 8 ) ) ;
    return ( *( info + add ) & ( 1 << shf ) ) ;
}
#endif

/* オンライン情報の生成 */
void	HZD_MakeOnlineInfo( hzd )
HZD_DEF	*hzd ;
{
    HZD_GRP	*grp ;
    HZD_ZON	*zon, *z1, *z2 ;
    u_int	*info, c ;
    int		n_groups ;
    int		n_zones ;
    int		i, j, size, bitsize ;
    FVECTOR	from, to ;

    n_groups = hzd->n_groups ;
    grp = hzd->groups ;
    while( -- n_groups >= 0 ) {
		n_zones = grp->n_zones ;
		grp->online_info = NULL ;
		if ( n_zones < 2 ) goto makeonlineinfo_skip ;
		zon = grp->zones ;
		bitsize = ( n_zones * n_zones - n_zones ) / 2 ; /* 総ビット数 */
		size = ( bitsize + ( sizeof( int ) * 8 - 1 ) ) / 
			( sizeof( int ) * 8 ) ; /* 総Ｉｎｔ数 */
		info = grp->online_info = ( u_int * )malloc( size * sizeof( u_int ) ) ;
		memset( info, 0x00, size * sizeof( u_int ) ) ;
		if ( info == NULL ) {
			printf( "memory allocation error : online info\n" ) ;
			return ;
		}
		z1 = zon ;
		for ( i = 0; i < n_zones - 1; i ++, z1 ++ ) {
			for ( j = i + 1, z2 = z1 + 1; j < n_zones; j ++, z2 ++ ) {
				from.vx = z1->x ;
				from.vy = z1->y + 1500.0F ;
				from.vz = z1->z ;
				to.vx = z2->x ;
				to.vy = z2->y + 1500.0F ;
				to.vz = z2->z ;

				c = HZD_OnlineHazardCheck( grp, &from, &to, 0,
										  HZX_SEG_NO_ENEMY | HZX_SEG_NO_ENEMY_EYES | HZX_SEG_RECOIL_TYPE,
										  HZX_FLOOR_NO_ENEMY | HZX_FLOOR_NO_ENEMY_EYES |
										  HZX_SEG_RECOIL_TYPE ) ;

				//HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY
					//printf( "%d", c ) ;
				if ( c ) WriteOnlineInfo( info, i, j, n_zones ) ;
			}
			//printf( "\n" ) ;
		}
		makeonlineinfo_skip :
		grp ++ ;
    }
#if 0
    {
		u_int	*buf ;
		int	n, c, x, z, b ;

		buf = info ; 
		z = n_zones - 1 ;
		n = 0 ; b = 0 ;
		for ( i = 0; i < size; i ++ ) {
			x = *( buf + i ) ;
			for ( j = 31; j >= 0; j -- ) {
				c = ( x & ( 1 << j ) ) ? 1 : 0 ;
				printf( "%d", c ) ;
				if ( ++ n == z ) {
					printf( "\n" ) ;
					n = 0 ; z -- ;
				}
				if ( ++ b == bitsize ) break ;
			}
		}
    }
#endif
}

#endif

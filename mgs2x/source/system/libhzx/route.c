//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   route.c
   道順データ初期化ルーチン

   1999/07/08 Y.Korekado
   $Id: route.c,v 1.1.1.3 2002/11/19 11:42:49 Yoshizawa1 Exp $			   

   void		HZX_MakeRoute( def, route )
   HZX_DEF	*def ;		地形当たりデータ
   u_char	*route ;	道順データ格納先

   地形当たりデータをもとに
   道順データを初期化する
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

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

	/*
		計算用バッファ
	*/
typedef	struct	{
	int		n_list ;	/* 周辺集合の要素数	*/
	u_char		list[ 60 ] ;	/* 周辺集合の要素	*/
} List ;

/*----------------------------------------------------------------*/

	/*
		ある行き先について、道順データを計算する
	*/

static	void	MakeRoutes( zones, n_zones, to, routes )
HZX_ZON		*zones ;	/* ゾーン配列		*/
int		n_zones ;	/* ゾーン数		*/
int		to ;		/* 行き先		*/
u_char		*routes ;	/* 道順データ格納	*/
{
	List		Lists[ 2 ] ;
	List		*old, *new ;
	int		i, j, route, tmp ;
	int		n_old, n_new, near ;
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
				if ( ( near = *( nears ++ ) ) == MAX_ROUTE ) break ;
				if ( route < routes[ near ] ) {
					routes[ near ] = route ;
					new->list[ n_new ] = near ;
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

/*----------------------------------------------------------------*/

void		HZX_MakeRoute( grp, routes )
HZX_GRP		*grp ;
u_char		*routes ;
{	
	HZX_ZON		*zones ;
	int		i, n_zones ;
	u_char *tmp_routes, *r;

	zones = grp->zones ;
	if( ( n_zones = grp->n_zones ) <= 0 ) return;

	if( ( tmp_routes = GV_Malloc( sizeof( u_char ) * n_zones ) ) == NULL ) return;
	r = routes;
	for ( i = 0 ; i < n_zones ; i ++ ) {
		long64 j;
		MakeRoutes( zones, n_zones, i, tmp_routes ) ;
		for( j = i + 1; j < n_zones; j++ ){
			*r = tmp_routes[ j ];
			r++;
		}
	}
	GV_Free( tmp_routes );
}

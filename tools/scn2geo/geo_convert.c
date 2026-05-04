/*
  geo_convert.c
  Scn(VRS) -> Geo(MGS2) コンバータ
   
  2002.11.6 T.Morita
   
  $Id: geo_convert.c,v 1.9 2002/12/02 06:38:13 usr04098 Exp $
  */

#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include	<sys/types.h> 
#include	<math.h>

#include	"scn2geo.h"


static int conv_search_poly( HP3DMODEL model, HP3DOBJECT obj,
							 POLYFUNC func, void *data )
{
    while( obj ) {
		HP3DOBJECT nxt ;

		/* オブジェクトからポリゴンデータを取り出す */
		if ( (*func)( obj, data ) == 0 ){
			return 0 ;
		}

		/* 子供を見る */
		if ( (nxt = P3DGetFirstObject( model, obj )) ) {
			if ( conv_search_poly( model, nxt, func, data ) == 0 ){
				return 0;
			}
		}

		/* 兄弟を見る */
		obj = P3DGetNextObject( obj ) ;
    }

    return 1;
}

#if 0
HP3DOBJECT conv_search_debug( HP3DMODEL model, HP3DOBJECT obj )
{
    while( obj )
    {
		HP3DOBJECT nxt ;

		printf( "%s\n", P3DObject(obj)->szName ) ;

		if ( (nxt = P3DGetFirstObject( model, obj )) ) {
			if ( (nxt = conv_search_debug( model, nxt )) ) {
				return nxt ;
			}
		}
		obj = P3DGetNextObject( obj ) ;
    }
    return NULL ;
}
#endif

HP3DOBJECT conv_search_object( HP3DMODEL model, HP3DOBJECT obj, char *name )
{
    while( obj )
    {
		HP3DOBJECT nxt ;

		//	printf( "%s\n", P3DObject(obj)->szName ) ;

		if ( !strcmp( P3DObject(obj)->szName, name ) ) {
			return obj ;
		}
		if ( (nxt = P3DGetFirstObject( model, obj )) ) {
			if ( (nxt = conv_search_object( model, nxt, name )) ) {
				return nxt ;
			}
		}
		obj = P3DGetNextObject( obj ) ;
    }
    return NULL ;
}

int conv_get_poly( HP3DMODEL model,
				   int grp_id, char *name,
				   POLYFUNC func, void *data )
{
    HP3DOBJECT obj ;
    char       grp_name[128] ;

    /* グループのオブジェクトを探す */
    sprintf( grp_name, "Group-%d", grp_id );
    obj = P3DFindObjectName( model, grp_name ) ;
	if ( obj == NULL ){
		printf( "cannot find group%s\n", grp_name ) ;
	}
	if ( name != NULL ) {
		obj = conv_search_object( model, obj, name ) ;
		if ( obj == NULL ){
			printf( "cannot find name%s\n", name ) ;
		}
	}
    return conv_search_poly( model, obj, func, data ) ;
}

int conv_get_n_group( HP3DMODEL model )
{
    int  i ;

    /* グループのオブジェクトを探す */
    for ( i=0 ; i<32 ; i++ ) {
		char grp_name[128] ;

		sprintf( grp_name, "Group-%d", i );

		if ( P3DFindObjectName( model, grp_name ) == NULL )
			break ;
    }
    return i ;
}

/*---------------------------------------------------------------------------


  


  ---------------------------------------------------------------------------*/

/* chank 0 の初期化 */
int convert_group( GEO_CHANK *chank, HP3DMODEL model )
{
    GEO_GROUP	*g, *new ;
    int		 i, j, k, l  ;
    int          n_group ;

    if ( core_geodef_chank_start( chank, 0 ) < 0 ) {
		printf( "cannot start chank.\n" ) ;
		return -1 ;
    }

    n_group = conv_get_n_group( model ) ;
    if ( !(new = g = core_group_make_groups( n_group )) ){
		return -1 ;
    }

    /* GEO_GROUP初期化 */
    for ( i=0 ; i<n_group ; i++ ) {
		POLYDATA data ;

		data.div.vx = g_division.vx ;
		data.div.vy = g_division.vy ;
		data.div.vz = g_division.vz ;
		data.bmax.vx = data.bmax.vy = data.bmax.vz = -10000000.0f ;
		data.bmin.vx = data.bmin.vy = data.bmin.vz =  10000000.0f ;
		conv_get_poly( model, i, NULL, poly_bound, &data ) ;
		data.max.vx = ceil( (data.bmax.vx - data.bmin.vx) / data.div.vx ) ;
		data.max.vy = ceil( (data.bmax.vy - data.bmin.vy) / data.div.vy ) ;
		data.max.vz = ceil( (data.bmax.vz - data.bmin.vz) / data.div.vz ) ;

		printf( "-------- group %d/%d max(%.0f %.0f %.0f) bmin(%.0f %.0f %.0f) bmax(%.0f %.0f %.0f)\n", i, n_group,
				data.max.vx, data.max.vy, data.max.vz,
				data.bmin.vx,data.bmin.vy,data.bmin.vz,
				data.bmax.vx,data.bmax.vy,data.bmax.vz
 ) ;

		/* GEO_GROUPの初期化 と RADIXを用意 */
		if ( core_group_add( g, &data.div, &data.max, &data.bmin ) < 0 ) {
			printf( "kakuho shippai\n" ) ;
			return -1 ;
		}

		for ( j=0 ; j<data.max.vx ; j++ ) {
			data.pos.vx = j ;
			for ( k=0 ; k<data.max.vy ; k++ ) {
				data.pos.vy = k ;
				for ( l=0 ; l<data.max.vz ; l++ ) {
					GEO_BLOCK *b       ;
					float      x, y, z ;

					data.pos.vz = l ;
					printf( "radix %d %d %d\n", j,k,l ) ;

					x = j * data.div.vx ;
					y = k * data.div.vy ;
					z = l * data.div.vz ;

					/* ハザード用ブロック */
					core_block_start( 1, x,y,z ) ;
					conv_get_poly( model, i, "Floor", poly_floor, &data ) ;
					conv_get_poly( model, i, "Hazard", poly_hazard, &data ) ;
					b = core_block_end() ;
					core_group_radix_add_block( g, b, j,k,l ) ;

					/* トラップ用ブロック */
					core_block_start( 2, x,y,z ) ;
					conv_get_poly( model, i, "Trap", poly_trap, &data ) ;
					conv_get_poly( model, i, "Radio", poly_trap, &data ) ;
					conv_get_poly( model, i, "Camera", poly_floor, &data ) ;
					b = core_block_end() ;
					core_group_radix_add_block( g, b, j,k,l ) ;
				}
			}
		}

		core_group_radix_reshape( g ) ;

		g++ ;
    }

    return core_geodef_chank_end() ;
}

GEO_DEF *convert_geodef( HP3DMODEL model )
{
    GEO_DEF   *def ;

    def = mem_alloc( sizeof(GEO_DEF) + sizeof(GEO_CHANK)*g_unit_n_types ) ;
    if ( def == NULL ) {
		return NULL ;
    }

    /* chank0 は予約チャンク */
    if ( convert_group( &def->chanks[0], model ) < 0 ) {
		return NULL ;
    }
    def->n_chanks++ ;
    def->size = mem_tell_align( 0 ) ;


    return def ;
}

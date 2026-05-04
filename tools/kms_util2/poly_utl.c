/*
	poly_utl.c
	ポリゴン関連ユーティリティ

	2000/01/22 K.Takabe
	$Id: poly_utl.c,v 1.2 2001/12/04 09:58:33 usr02774 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "fmt_kms.h"		/* FVECTOR等の定義用 */
#include "prog_utl.h"
#include "poly_utl.h"


/* ================================================================ */
#define MAX_REALLOC					(256)
#define DEFAULT_ALLOC_SIZE			(10000)
static int			n_alloc_mem = 0 ;
static POLYGON		*poly_mem[MAX_REALLOC] ;
static POLY_PACKET	system_poly ;
static int			max_polys = 0 ;
static int			use_polys = 0 ;

static int			IndexedCount = 0 ;	/* 総計頂点ＩＤ */

/* 誤差を考慮する */
#define ABOUT_CHECK

/* ================================================================ */
/*
	ポリゴンパケット関連サブルーチン
 */
/* ポリゴンパケットの初期化 */
void InitPolygonPacket( POLY_PACKET *packet )
{
	packet->start.prev = NULL ;
	packet->start.next = &packet->end ;
	packet->end.prev = &packet->start ;
	packet->end.next = NULL ;
	packet->n_polygons = 0 ;
}
/* ポリゴンパケットにポリゴンを追加 */
void AddPolygon( POLY_PACKET *packet, POLYGON *poly )
{
	POLYGON		*prev ;
	prev = packet->end.prev ;
	prev->next = poly ;
	poly->prev = prev ;
	poly->next = &packet->end ;
	packet->end.prev = poly ;
	packet->n_polygons++ ;
}
/* ポリゴンパケットから指定したポリゴンを削除 */
void DeletePolygon( POLY_PACKET *packet, POLYGON *poly )
{
	POLYGON		*prev, *next ;

	if ( ( poly->prev == NULL ) || ( poly->next == NULL ) ){
		return ;
	}
	prev = poly->prev ;
	next = poly->next ;
	prev->next = next ;
	next->prev = prev ;
	poly->prev = NULL ;
	poly->next = NULL ;

	if ( packet != NULL ){
		packet->n_polygons-- ;
		if ( packet->n_polygons < 0 ){
			packet->n_polygons = 0 ;
			printf("delete polygon error!!\n");
		}
	}
	return ;

}
/* ポリゴンパケットから先頭のポリゴンを取得する（リストからは削除しない） */
POLYGON* GetFirstPolygon( POLY_PACKET *packet )
{
	if ( packet->start.next != &packet->end ){
		return ( packet->start.next );
	}
	return ( NULL );
}
/* 次のポリゴンを取得する（リストからは削除しない） */
POLYGON* GetNextPolygon( POLYGON *poly )
{
	if ( poly->next->next != NULL ){
		return ( poly->next );
	}
	return ( NULL );
}
/* ポリゴン内容をコピーする（リンクリストの構造は変更しない） */
void CopyPolygon( POLYGON *dst_poly, POLYGON *src_poly )
{
	void *old_prev, *old_next ;
	old_prev = dst_poly->prev ;
	old_next = dst_poly->next ;
	*dst_poly = *src_poly ;
	dst_poly->prev = old_prev ;
	dst_poly->next = old_next ;
}
/* ポリゴンパケットを複製する（パケットの初期化も行なう） */
void DuplicatePolygonPacket( POLY_PACKET *dst_packet, POLY_PACKET *src_packet )
{
	POLYGON		*org_poly, *poly ;

	InitPolygonPacket( dst_packet );
	org_poly = GetFirstPolygon( src_packet ) ;
	while ( org_poly != NULL ){
		poly = AllocPolygon();
		CopyPolygon( poly, org_poly );
		AddPolygon( dst_packet, poly );
		org_poly = GetNextPolygon( org_poly );
	}
}
/* ポリゴンパケットを加える（元は残らない） */
void MovePolygonPacket( POLY_PACKET *packet, POLY_PACKET *add_packet )
{
	POLYGON	*poly ;
	while ( ( poly = GetFirstPolygon( add_packet ) ) != NULL ){
		DeletePolygon( add_packet, poly );
		AddPolygon( packet, poly );
	}
}
/* ポリゴンパケットを加える（元も残る） */
void AddPolygonPacket( POLY_PACKET *packet, POLY_PACKET *add_packet )
{
	POLYGON	*poly, *tmp_poly ;
	poly = GetFirstPolygon( add_packet );
	while ( poly != NULL ){
		tmp_poly = AllocPolygon() ;
		CopyPolygon( tmp_poly, poly );
		AddPolygon( packet, tmp_poly );
		poly = GetNextPolygon( poly );
	}
}
/* ポリゴンパケットを全て開放する（パケットの初期化も行なう） */
void FreePolygonPacket( POLY_PACKET *packet )
{
	POLYGON		*poly, *next ;

	while ( ( poly = GetFirstPolygon( packet ) ) != NULL ){
		DeletePolygon( packet, poly );
		FreePolygon( poly );
	}
	InitPolygonPacket( packet );
}
/* パケットに含まれるポリゴン数を返す */
int GetTotalPolygons( POLY_PACKET *packet )
{
	return ( packet->n_polygons );
}

/* ================================================================ */
/*
	ポリゴン管理処理
 */
/* メモリの再確保（内部使用用） */
static void ReallocPolygon( int num )
{
	POLYGON	*poly ;
	int		i ;

	max_polys += num ;
	poly = poly_mem[ n_alloc_mem ] = Malloc0( sizeof(POLYGON) * num );
	for ( i = 0 ; i < num ; i++ ){
		AddPolygon( &system_poly, poly );
		poly++ ;
	}
	n_alloc_mem++ ;
}
/* ポリゴン管理システムの初期化 */
void InitPolygonManagement( void )
{
	int		i ;
	InitPolygonPacket( &system_poly );
	n_alloc_mem = 0 ;
	max_polys = 0 ;
	use_polys = 0 ;
}
/* ポリゴン管理システムの解放 */
void EndPolygonManagement( void )
{
	int		i ;
	for ( i = 0 ; i < n_alloc_mem ; i++ ){
		Free( poly_mem[ i ] );
	}
}

/* ポリゴンデータの確保 */
POLYGON* AllocPolygon( void )
{
	POLYGON		*poly ;

	if ( use_polys >= max_polys ) ReallocPolygon( DEFAULT_ALLOC_SIZE );
	use_polys++ ;
	poly = GetFirstPolygon( &system_poly );
	DeletePolygon( &system_poly, poly );
	InitVertex( &poly->vertex[0] );
	InitVertex( &poly->vertex[1] );
	InitVertex( &poly->vertex[2] );
	poly->n_material = 0 ;
	poly->material[0].tex_id = poly->material[1].tex_id = poly->material[2].tex_id = 0 ;
	return ( poly );
}
/* ポリゴンデータの解放 */
void FreePolygon( POLYGON *poly )
{
	DeletePolygon( NULL, poly );		/* 念のため */
	AddPolygon( &system_poly, poly );
	use_polys-- ;
}

/* ================================================================ */
/*
	デバッグ関連
 */
/* 内部の情報を表示 */
void DisplayPolygonManagementStatus( void )
{
	printf("realloc count: %d\n", n_alloc_mem );
	printf("use polygos: %d\n", use_polys );
}
/* ベクトルの表示 */
void DisplayVector( char *mes, FVECTOR *vec )
{
	printf("%s %6.4f %6.4f %6.4f %6.4f\n", mes, vec->vx, vec->vy, vec->vz, vec->vw );
}
/* 重み情報の表示 */
void DisplayWeight( char *mes, WEIGHT *w )
{
	int		i ;
	printf("%s ");
	for ( i = 0 ; i < w->max_use ; i++ ) printf("(%2d, %1.5f) ", w->weight[i].skel_num, w->weight[i].weight );
	printf("\n");
	
}
/* 頂点情報の表示 */
void DisplayVertex( char *mes, VERTEX *v )
{
	static char	buff[256] ;
	printf( "%s\n", mes );
	DisplayVector( "vert:", &v->v );
	DisplayVector( "norm:", &v->n );
	DisplayVector( "uv0 :", &v->uv[0] );
	DisplayVector( "uv1 :", &v->uv[1] );
	DisplayVector( "uv2 :", &v->uv[2] );
	DisplayWeight( "weight :", &v->weight );
}
/* ポリゴン情報の表示 */
void DisplayPolygon( char *mes, POLYGON *poly )
{
	int		i ;
	printf( "%s\n", mes );
	for ( i = 0 ; i < poly->n_material ; i++ ){
		printf("texture%1d id %d \n", i, poly->material[i].tex_id );
	}
	DisplayVertex( "vertex0 ==============", &poly->vertex[0] );
	DisplayVertex( "vertex1 ==============", &poly->vertex[1] );
	DisplayVertex( "vertex2 ==============", &poly->vertex[2] );
}

/* ================================================================ */
/*
	関連ユーティリティ
 */
/* 頂点情報の初期化 */
void InitVertex( VERTEX *v )
{
	static FVECTOR		zero_v_vector = {0.0f, 0.0f, 0.0f, 1.0f};
	static FVECTOR		zero_n_vector = {0.0f, 0.0f, 0.0f, 0.0f};
	static FVECTOR		zero_uv_vector = {0.0f, 0.0f, 1.0f, 0.0f};
	static WEIGHT		zero_weight = { 0, 0.0f };
	int		i ;

	v->v = zero_v_vector ;
	v->n = zero_n_vector ;
	v->uv[0] = v->uv[1] = v->uv[2] = zero_uv_vector ;
	v->weight.max_use = 0 ;
	for ( i = 0 ; i < WEIGHT_MAX ; i++ ){
		v->weight.weight[i].skel_num = -1 ;
		v->weight.weight[i].weight = 0.0f ;
	}
	v->id = -1 ;
}
/* ２つの重み情報を揃える */
void ReorderVertexWeight( WEIGHT *new1, WEIGHT *new2, WEIGHT *we1, WEIGHT *we2 )
{
	int		skel_list[WEIGHT_MAX] ;
	int		index1, index2, max, n1, n2, i, j ;
	WEIGHT	new_we1, new_we2 ;

	/* スケルトン番号を揃える */
	max = index1 = index2 = 0 ;
	skel_list[max] = -1 ;
	while ( index1 < we1->max_use || index2 < we2->max_use ){
		n1 = we1->weight[index1].skel_num ;
		n2 = we2->weight[index2].skel_num ;
		if ( index1 >= we1->max_use ) n1 = 255 ;
		if ( index2 >= we2->max_use ) n2 = 255 ;
		if ( n1 < n2 ){
			/* スケルトン番号が未登録で重みが０以外のとき追加 */
			if ( ( ( max == 0 ) || ( skel_list[max-1] != n1 ) )
				 && we1->weight[index1].weight != 0.0f ){
				skel_list[max++] = n1 ;
			}
			index1++ ;
		} else if ( n1 > n2 ){
			/* スケルトン番号が未登録で重みが０以外のとき追加 */
			if ( ( ( max == 0 ) || ( skel_list[max-1] != n2 ) )
				 && we2->weight[index2].weight != 0.0f ){
				skel_list[max++] = n2 ;
			}
			index2++ ;
		} else {
			/* スケルトン番号が未登録でどちらかの重みが０以外のとき追加 */
			if ( ( ( max == 0 ) || ( skel_list[max-1] != n1 ) ) &&
				 ( we1->weight[index1].weight != 0.0f || we2->weight[index2].weight != 0.0f ) ){
				skel_list[max++] = n1 ;
			}
			index1++ ;
			index2++ ;
		}
		if ( max > WEIGHT_MAX ){
			fprintf(stderr,"weight over!!\n");
			//exit(-1);
		}
	}

	/* 揃えたスケルトン番号にしたがって並べ替えを行なう */
	new_we1.max_use = new_we2.max_use = max ;
	for  ( i = 0 ; i < WEIGHT_MAX ; i++ ){
		if ( i >= max ) skel_list[i] = -1 ;
		new_we1.weight[i].skel_num = skel_list[i] ;
		new_we1.weight[i].weight = 0.0f ;
		new_we2.weight[i].skel_num = skel_list[i] ;
		new_we2.weight[i].weight = 0.0f ;
		if ( skel_list[i] != -1 ){
			for ( j = 0 ; j < we1->max_use ; j++ ){
				if ( we1->weight[j].skel_num == skel_list[i] ){
					new_we1.weight[i].weight = we1->weight[j].weight ;
					break ;
				}
			}
			for ( j = 0 ; j < we2->max_use ; j++ ){
				if ( we2->weight[j].skel_num == skel_list[i] ){
					new_we2.weight[i].weight = we2->weight[j].weight ;
					break ;
				}
			}
		}
	}

	*new1 = new_we1 ;
	*new2 = new_we2 ;

}
/* スケルトン番号の若い醇に並べ替えを行なう */
void SortWeight( WEIGHT *res, WEIGHT *we )
{
	int		i, j ;
	WEIGHT	w ;
	WEIGHT_ELEMENT	ele ;

	/* バブルソート */
	w = *we ;
	for ( i = w.max_use - 1 ; i > 0  ; i-- ){
		for ( j = 0 ; j < i ; j++ ){
			if ( w.weight[j].skel_num > w.weight[j+1].skel_num ){
				ele = w.weight[j] ;
				w.weight[j] = w.weight[j+1] ;
				w.weight[j+1] = ele ;
			}
		}
	}
	*res = w ;	
}

/* ２つの実数が等しいかチェックする（誤差を考慮する） */
int IsEqualFloatAbout( float f1, float f2, float error )
{
	if ( ( ( f1 - error ) <= f2 ) && ( ( f1 + error ) >= f2 ) ) return 1 ;
	return 0 ;
}
/* ２つの頂点が等しいかチェックする（誤差を考慮する） */
int IsEqualVectorAbout( FVECTOR *vec1, FVECTOR *vec2 )
{
	if ( IsEqualFloatAbout( vec1->vx, vec2->vx, 1.0f ) &&
		IsEqualFloatAbout( vec1->vy, vec2->vy, 1.0f ) &&
		IsEqualFloatAbout( vec1->vz, vec2->vz, 1.0f ) &&
		IsEqualFloatAbout( vec1->vw, vec2->vw, 1.0f ) ){
		return ( 1 );
	}
	return ( 0 );
}
/* ２つの頂点が等しいかチェックする */
int IsEqualVector( FVECTOR *vec1, FVECTOR *vec2 )
{
	if ( ( vec1->vx == vec2->vx ) && ( vec1->vy == vec2->vy ) &&
		( vec1->vz == vec2->vz ) && ( vec1->vw == vec2->vw ) ){
		return ( 1 );
	}
	return ( 0 );
}
/* ２つの法線が等しいかチェックする（誤差を考慮する） */
int IsEqualNormalAbout( FVECTOR *vec1, FVECTOR *vec2 )
{
	if ( IsEqualFloatAbout( vec1->vx, vec2->vx, 1.0f/4096.0f ) &&
		IsEqualFloatAbout( vec1->vy, vec2->vy, 1.0f/4096.0f ) &&
		IsEqualFloatAbout( vec1->vz, vec2->vz, 1.0f/4096.0f ) ){
		return ( 1 );
	}
	return ( 0 );
}
/* ２つの法線が等しいかチェックする */
int IsEqualNormal( FVECTOR *norm1, FVECTOR *norm2 )
{
	if ( ( norm1->vx == norm2->vx ) && ( norm1->vy == norm2->vy ) && ( norm1->vz == norm2->vz ) ) {
		return ( 1 );
	}
	return ( 0 );
}
/* ２つのＵＶが等しいかチェックする（誤差を考慮する） */
int IsEqualUVAbout( FVECTOR *vec1, FVECTOR *vec2 )
{
	if ( IsEqualFloatAbout( vec1->vx, vec2->vx, 1.0f/4096.0f ) &&
		IsEqualFloatAbout( vec1->vy, vec2->vy, 1.0f/4096.0f ) &&
		IsEqualFloatAbout( vec1->vz, vec2->vz, 1.0f/4096.0f ) ){
		return ( 1 );
	}
	return ( 0 );
}
/* ２つのＵＶが等しいかチェックする */
int IsEqualUV( FVECTOR *uv1, FVECTOR *uv2 )
{
	if ( ( uv1->vx == uv2->vx ) && ( uv1->vy == uv2->vy ) && ( uv1->vz == uv2->vz ) ) {
		return ( 1 );
	}
	return ( 0 );
}
/* ２つの頂点重みが等しいかチェックする */
int IsEqualWeight( WEIGHT *we1, WEIGHT *we2 )
{
	WEIGHT	new1, new2 ;
	int		i ;

	ReorderVertexWeight( &new1, &new2, we1, we2 );
	for ( i = 0 ; i < new1.max_use ; i++ ){
		if ( new1.weight[i].weight != new2.weight[i].weight ) return ( 0 );
	}
	return ( 1 );
}

/* ２つの頂点が共有できるかチェックする */
int IsEqualVertex( VERTEX *v1, VERTEX *v2 )
{
#ifndef ABOUT_CHECK
	if ( IsEqualVector( &v1->v, &v2->v ) && IsEqualNormal( &v1->n, &v2->n ) &&
		IsEqualUV( &v1->uv[0], &v2->uv[0] ) && IsEqualUV( &v1->uv[2], &v2->uv[2] ) &&
		IsEqualUV( &v1->uv[2], &v2->uv[2] ) && IsEqualWeight( &v1->weight, &v2->weight ) ){
		return ( 1 );
	}
#else
#if 1
	if ( IsEqualVectorAbout( &v1->v, &v2->v ) && IsEqualNormal( &v1->n, &v2->n ) &&
		IsEqualUV( &v1->uv[0], &v2->uv[0] ) && IsEqualUV( &v1->uv[2], &v2->uv[2] ) &&
		IsEqualUV( &v1->uv[2], &v2->uv[2] ) && IsEqualWeight( &v1->weight, &v2->weight ) ){
		return ( 1 );
	}
#else
	if ( IsEqualVectorAbout( &v1->v, &v2->v ) && IsEqualNormalAbout( &v1->n, &v2->n ) &&
		IsEqualUVAbout( &v1->uv[0], &v2->uv[0] ) && IsEqualUVAbout( &v1->uv[2], &v2->uv[2] ) &&
		IsEqualUVAbout( &v1->uv[2], &v2->uv[2] ) && IsEqualWeight( &v1->weight, &v2->weight ) ){
		return ( 1 );
	}
#endif
#endif
	return ( 0 );
}
/* 頂点ＩＤから２つの頂点が共有できるかチェックする */
int IsEqualVertexFast( VERTEX *v1, VERTEX *v2 )
{
	if ( v1->id == v2->id ){
		return ( 1 );
	}
	return ( 0 );
}
/* ２つのポリゴンが同じかチェックする */
int IsEqualPolygon( POLYGON *poly1, POLYGON *poly2 )
{
	int		i ;
	//if ( poly1->flag != poly2->flag ) return ( 0 );
	if ( poly1->n_material != poly2->n_material ) return ( 0 );
	for ( i = 0 ; i < poly1->n_material ; i++ ){
		if ( poly1->material[i].tex_id != poly2->material[i].tex_id ) return ( 0 );
	}
	for ( i = 0 ; i < 3 ; i++ ){
		if ( !IsEqualVertex( &poly1->vertex[i], &poly2->vertex[i] ) ) return ( 0 );
	}
	return ( 1 );
}
/* ２つのポリゴンがマルチテクスチャとして合成できるかチェックする */
int IsJoinMultiTexturePolygon( POLYGON *org_poly, POLYGON *add_poly )
{
	int		i, j, count = 0 ;
	/* add_polyはシングルテクスチャであることが前提 */
	if ( ( org_poly->n_material + add_poly->n_material ) > 3 ) return ( 0 );
	for ( i = 0 ; i < 3 ; i++ ){
		for ( j = 0 ; j < 3 ; j++ ){
#ifndef ABOUT_CHECK
			if ( IsEqualVector( &org_poly->vertex[i].v, &add_poly->vertex[j].v ) &&
				IsEqualNormal( &org_poly->vertex[i].n, &add_poly->vertex[j].n ) &&
				IsEqualWeight( &org_poly->vertex[i].weight, &add_poly->vertex[j].weight ) ){
				count++ ;
			}
#else
			if ( IsEqualVectorAbout( &org_poly->vertex[i].v, &add_poly->vertex[j].v ) &&
				IsEqualNormal( &org_poly->vertex[i].n, &add_poly->vertex[j].n ) &&
				IsEqualWeight( &org_poly->vertex[i].weight, &add_poly->vertex[j].weight ) ){
				count++ ;
			}
#endif
		}
	}
	if ( count == 3 ) return ( 1 ) ;
	return ( 0 );
}
/* ２つのポリゴンをマルチテクスチャとして合成する */
void JoinMultiTexturePolygon( POLYGON *org_poly, POLYGON *add_poly )
{
	int		i, j, count = 0 ;
	/* add_polyはシングルテクスチャであることが前提 */
	for ( i = 0 ; i < 3 ; i++ ){
		for ( j = 0 ; j < 3 ; j++ ){
#ifndef ABOUT_CHECK
			if ( IsEqualVector( &org_poly->vertex[i].v, &add_poly->vertex[j].v ) &&
				IsEqualNormal( &org_poly->vertex[i].n, &add_poly->vertex[j].n ) &&
				IsEqualWeight( &org_poly->vertex[i].weight, &add_poly->vertex[j].weight ) ){
				org_poly->vertex[i].uv[org_poly->n_material] = add_poly->vertex[j].uv[0] ;
			}
#else
			if ( IsEqualVectorAbout( &org_poly->vertex[i].v, &add_poly->vertex[j].v ) &&
				IsEqualNormal( &org_poly->vertex[i].n, &add_poly->vertex[j].n ) &&
				IsEqualWeight( &org_poly->vertex[i].weight, &add_poly->vertex[j].weight ) ){
				org_poly->vertex[i].uv[org_poly->n_material] = add_poly->vertex[j].uv[0] ;
			}
#endif
		}
	}
	org_poly->material[org_poly->n_material] = add_poly->material[0] ;
	org_poly->n_material++ ;
	
}
/* ２つのポリゴンがマルチテクスチャとして合成できるかチェックする */
int IsJoinMultiTexturePolygon2( POLYGON *org_poly, POLYGON *add_poly )
{
	int		i, j, count = 0 ;
	/* add_polyはシングルテクスチャであることが前提 */
	if ( ( org_poly->n_material + add_poly->n_material ) > 3 ) return ( 0 );
	for ( i = 0 ; i < 3 ; i++ ){
		for ( j = 0 ; j < 3 ; j++ ){
#ifndef ABOUT_CHECK
			if ( IsEqualVector( &org_poly->vertex[i].v, &add_poly->vertex[j].v ) &&
				IsEqualWeight( &org_poly->vertex[i].weight, &add_poly->vertex[j].weight ) ){
				count++ ;
			}
#else
			if ( IsEqualVectorAbout( &org_poly->vertex[i].v, &add_poly->vertex[j].v ) &&
				IsEqualWeight( &org_poly->vertex[i].weight, &add_poly->vertex[j].weight ) ){
				count++ ;
			}
#endif
		}
	}
	if ( count == 3 ) return ( 1 ) ;
	return ( 0 );
}
/* ２つのポリゴンをマルチテクスチャとして合成する */
void JoinMultiTexturePolygon2( POLYGON *org_poly, POLYGON *add_poly )
{
	int		i, j, count = 0 ;
	/* add_polyはシングルテクスチャであることが前提 */
	for ( i = 0 ; i < 3 ; i++ ){
		for ( j = 0 ; j < 3 ; j++ ){
#ifndef ABOUT_CHECK
			if ( IsEqualVector( &org_poly->vertex[i].v, &add_poly->vertex[j].v ) &&
				IsEqualWeight( &org_poly->vertex[i].weight, &add_poly->vertex[j].weight ) ){
				org_poly->vertex[i].uv[org_poly->n_material] = add_poly->vertex[j].uv[0] ;
			}
#else
			if ( IsEqualVectorAbout( &org_poly->vertex[i].v, &add_poly->vertex[j].v ) &&
				IsEqualWeight( &org_poly->vertex[i].weight, &add_poly->vertex[j].weight ) ){
				org_poly->vertex[i].uv[org_poly->n_material] = add_poly->vertex[j].uv[0] ;
			}
#endif
		}
	}
	org_poly->material[org_poly->n_material] = add_poly->material[0] ;
	org_poly->n_material++ ;
	
}


/* パケットで使用しているスケルトン数を調べる（重たいかも） */
int GetUseSkeletonWeight( POLY_PACKET *packet )
{
	POLYGON	*poly ;
	int		skel_list[ 256 ], max, i, j, k, count ;

	count = 0 ;
	max = 0 ;
	poly = GetFirstPolygon( packet );
	while ( poly != NULL ){
		//DisplayPolygon( "polygon info +++++++++++++", poly );
		for ( i = 0 ; i < 3 ; i++ ){
			for ( k = 0 ; k < poly->vertex[i].weight.max_use ; k++ ){
				if ( poly->vertex[i].weight.weight[k].weight != 0.0f ){
					for ( j = 0 ; j < max ; j++ ){
						if ( poly->vertex[i].weight.weight[k].skel_num == skel_list[j] ) break ;
					}
					if ( j == max ){
						skel_list[j] = poly->vertex[i].weight.weight[k].skel_num ;
						max++ ;
					}
				}
			}
		}
		poly = GetNextPolygon( poly );
	}
	return max ;
}

/* ポリゴンパケット情報を整理する（ただし使用スケルトン数が８以内であることが保証されている場合に限る） */
void OptimizePolygonPacket( POLY_PACKET *packet )
{
	POLYGON	*poly ;
	int		skel_list[ 256 ], max, i, j, k, l, count ;
	WEIGHT	weight, tmp_we ;

	/* 使用スケルトン数をチェックする */
	count = 0 ;
	max = 0 ;
	poly = GetFirstPolygon( packet );
	while ( poly != NULL ){
		for ( i = 0 ; i < 3 ; i++ ){
			for ( k = 0 ; k < poly->vertex[i].weight.max_use ; k++ ){
				if ( poly->vertex[i].weight.weight[k].weight != 0.0f ){
					for ( l = 0 ; l < max ; l++ ){
						if ( poly->vertex[i].weight.weight[k].skel_num == skel_list[l] ) break ;
					}
					if ( l == max ){
						skel_list[l] = poly->vertex[i].weight.weight[k].skel_num ;
						max++ ;
					}
				}
			}
		}
		poly = GetNextPolygon( poly );
	}
	/* 並べ替えリストを作成 */
	max = ( max > WEIGHT_MAX ) ? WEIGHT_MAX : max ;
	weight.max_use = max ;
	for ( i = 0 ; i < WEIGHT_MAX ; i++ ){
		if ( i < max ){
			weight.weight[i].skel_num = skel_list[i] ;
		} else {
			weight.weight[i].skel_num = -1 ;
		}
	}
	SortWeight( &weight, &weight );

	/* 並べ替えを行なう */
	poly = GetFirstPolygon( packet );
	while ( poly != NULL ){
		for ( i = 0 ; i < 3 ; i++ ){
			tmp_we.max_use = max ;
			for ( j = 0 ; j < WEIGHT_MAX ; j++ ){
				tmp_we.weight[j].skel_num = weight.weight[j].skel_num ;
				tmp_we.weight[j].weight = 0.0f ;
				if ( tmp_we.weight[j].skel_num == -1 ) continue ;
				/* 一致するスケルトン番号を探し、重みをコピーする */
				for ( k = 0 ; k < poly->vertex[i].weight.max_use ; k++ ){
					if ( tmp_we.weight[j].skel_num == poly->vertex[i].weight.weight[k].skel_num ){
						tmp_we.weight[j].weight = poly->vertex[i].weight.weight[k].weight ;
						break ;
					}
				}
			}
			poly->vertex[i].weight = tmp_we ;
		}
		poly = GetNextPolygon( poly );
	}
	return ;
}
/* ================================================================ */
/*
	頂点インデックス化サポート関連サブルーチン
 */
/* 現在の頂点ＩＤ総計を初期化する */
void IndexedVertexInitialize( void )
{
	IndexedCount = 0 ;
}
/* 現在の頂点ＩＤ総計を取得する */
int GetIndexedVertexCount( void )
{
	return ( IndexedCount ) ;
}
/* パケット内のポリゴンの頂点ＩＤを初期化する */
void IndexedVertexClear( POLY_PACKET *packet )
{
	int		i ;
	POLYGON	*poly ;

	/* 各ポリゴンの頂点ＩＤを初期化する */
	poly = GetFirstPolygon( packet );
	while ( poly != NULL ){
		for ( i = 0 ; i < 3 ; i++ ){
			poly->vertex[i].id = -1 ;
		}
		poly = GetNextPolygon( poly );
	}
}
/* パケット内の頂点共有をチェックし、独立した頂点ＩＤを割り当てる */
void IndexedVertex( POLY_PACKET *packet )
{
	int		i, j, index, flush_count ;
	POLYGON	*poly, *s_poly ;

	/* 各ポリゴンの頂点ＩＤを初期化する */
	poly = GetFirstPolygon( packet );
	while ( poly != NULL ){
		for ( i = 0 ; i < 3 ; i++ ){
			poly->vertex[i].id = -1 ;
		}
		poly = GetNextPolygon( poly );
	}

	/* 頂点にＩＤを付けていく */
	index = IndexedCount ;
	flush_count = 0 ;
	poly = GetFirstPolygon( packet );
	while ( poly != NULL ){
		for ( i = 0 ; i < 3 ; i++ ){
			if ( poly->vertex[i].id == -1 ){
				/* まだＩＤが振られていない場合にはＩＤを割り当てる */
				poly->vertex[i].id = index ;
				/* 残りのポリゴンにも同じ頂点が存在する場合には同じＩＤを割り当てる */
				s_poly = GetNextPolygon( poly );
				while ( s_poly != NULL ){
					for ( j = 0 ; j < 3 ; j++ ){
						if ( s_poly->vertex[j].id != -1 ) continue ;
						if ( IsEqualVertex( &poly->vertex[i], &s_poly->vertex[j] ) ){
							s_poly->vertex[j].id = index ;
						}
					}
					s_poly = GetNextPolygon( s_poly );
				}
				index++ ;
				flush_count++ ;
#if 0
				if ( flush_count >= 10 ){
					printf("index check %6d                          %c", index, 0xd );
					fflush( stdout );
					flush_count = 0 ;
				}
#endif
			}
		}
		poly = GetNextPolygon( poly );
	}
	IndexedCount = index ;
	//printf("total_vertex : %d %06x\n", index, poly );
}

/* ================================================================ */
/*
	ストリップ化サポート関連サブルーチン
 */
/* 指定した頂点が格納されている頂点番号を検索する */
static int SearchVertexNumber( POLYGON *poly, VERTEX *v )
{
	int		i ;
	for ( i = 0 ; i < 3 ; i++ ){
		//if ( IsEqualVertex( &poly->vertex[i], v ) ) return ( i ) ;
		if ( IsEqualVertexFast( &poly->vertex[i], v ) ) return ( i ) ;
	}
	return (-1);
}
/* 指定した２点と頂点を共有しているポリゴンを検索する */
static void SearchConnectPolygon( POLYGON *trg_poly, POLY_PACKET *packet )
{
	int		i, j, checked, free ;
	VERTEX	tmp[3], *v ;
	POLYGON	*poly ;
	VERTEX	*v1, *v2 ;

	for ( i = 0 ; i < 3 ; i++ ){
		/* 各辺に対してチェックを行なう */
		switch ( i ){
		  case 0:
			v1 = &trg_poly->vertex[0] ;
			v2 = &trg_poly->vertex[1] ;
			break ;
		  case 1:
			v1 = &trg_poly->vertex[1] ;
			v2 = &trg_poly->vertex[2] ;
			break ;
		  case 2:
			v1 = &trg_poly->vertex[2] ;
			v2 = &trg_poly->vertex[0] ;
			break ;
		}
		/* すでに接続済みならスキップ */
		if ( trg_poly->link_side[i] != -1 ) continue ;

		/* パケットの全検索 */
		poly = GetFirstPolygon( packet );
		while ( poly != NULL ){

			/* チェック対象以外のポリゴンとチェックする */
			if ( poly == trg_poly ){
				poly = GetNextPolygon( poly );
				continue ;
			}

			/* 辺共有チェック */
			checked = -1 ;
			for ( j = 0 ; j < 3 ; j++ ){
				if ( IsEqualVertex( v1, &poly->vertex[j] ) || IsEqualVertex( v2, &poly->vertex[j] ) ){
					if ( checked == -1 ){
						checked = j ;
						continue ;
					} else {
						/* 相互にリンクをはる */
						trg_poly->link[i] = poly ;
						  if ( !( checked == 0 && j == 2 ) ){
							trg_poly->link_side[i] = checked ;
						} else {
							trg_poly->link_side[i] = 2 ;
						}
						poly->link[ trg_poly->link_side[i] ] = trg_poly ;
						poly->link_side[ trg_poly->link_side[i] ] = i ;
						poly = NULL ;
						break ;
					}
				}
			}
			if ( j == 3 ){
				poly = GetNextPolygon( poly );
			}
		}
	}
}
/* ストリップ化を行なうためにパケットを初期化する */
/* （以後直接触ってはいけないので注意！！！） */
void Strip_InitPolygonPacketLink( POLY_PACKET *packet )
{
	POLYGON	*poly ;

	/* フラグ、リンク情報の初期化 */
	poly = GetFirstPolygon( packet );
	while ( poly != NULL ){
		poly->flag = 0 ;
		poly->link_side[0] = poly->link_side[1] = poly->link_side[2] = -1 ;
		poly->link[0] = poly->link[1] = poly->link[2] = NULL ;
		poly = GetNextPolygon( poly );
	}

	/* フラグ、リンク情報の作成 */
	poly = GetFirstPolygon( packet );
	while ( poly != NULL ){
		SearchConnectPolygon( poly, packet );
		poly = GetNextPolygon( poly );
	}
}
/* パケットのフラグのみを初期化する */
void Strip_InitPolygonPacketFlag( POLY_PACKET *packet )
{
	POLYGON	*poly ;

	/* フラグ、リンク情報の初期化 */
	poly = GetFirstPolygon( packet );
	while ( poly != NULL ){
		poly->flag = 0 ;
		poly = GetNextPolygon( poly );
	}
}
/* 指定したポリゴンから始まるストリップの接続数を計算する */
int Strip_GetStripPolygons( POLY_PACKET *packet, POLYGON *poly, int type )
{
	static int get_side[3][3] = {{-1,0,2},{0,-1,1},{2,1,-1}};/* 頂点番号から辺番号へ */
	static int next_vertex[3] = { 2, 0, 1 };/* 次の頂点番号へ */
	VERTEX	*v1, *v2 ;
	int		index1, index2 ;
	int		count, side_no, old_side, i ;

	count = 0 ;
	Strip_InitPolygonPacketFlag( packet );

	/* スタートの頂点を決定する */
	switch ( type ){
	  case 0:
		/* ０から順方向 */
		index1 = 1 ;
		index2 = 2 ;
		break ;
	  case 1:
		/* １から順方向 */
		index1 = 2 ;
		index2 = 0 ;
		break ;
	  case 2:
		/* ２から順方向 */
		index1 = 0 ;
		index2 = 1 ;
		break ;
	  case 3:
		/* ０から逆方向 */
		index1 = 2 ;
		index2 = 1 ;
		break ;
	  case 4:
		/* １から逆方向 */
		index1 = 0 ;
		index2 = 2 ;
		break ;
	  case 5:
		/* ２から逆方向 */
		index1 = 1 ;
		index2 = 0 ;
		break ;
	}
	v1 = &poly->vertex[index1] ;
	v2 = &poly->vertex[index2] ;

	while ( poly->flag == 0 ){
		/* カウント数増加 */
		count++ ;
		/* 通過フラグを立てる */
		poly->flag |= 1 ;
		/* 接続辺番号を求める */
		side_no = get_side[ index1 ][ index2 ] ;
		if ( side_no == -1 ) break ;
		/* 接続先のポリゴンを取得する */
		if ( poly->link_side[ side_no ] == -1 ) break ;
		if ( poly->link[ side_no ] == NULL ) break ;
		old_side = side_no ;
		side_no = poly->link_side[ old_side ];/* 接続先での接続辺番号を取得する */
		poly = poly->link[ old_side ] ;
		/* 次のラスト２点のインデックス番号と頂点データのポインタを取得 */
		index1 = SearchVertexNumber( poly, v2 );
		index2 = next_vertex[ side_no ] ;
		v1 = &poly->vertex[index1] ;
		v2 = &poly->vertex[index2] ;
		
	}

	return ( count );
}

/* 指定したポリゴンから始まるストリップを切り出す */
int Strip_CutStripPolygons( POLY_PACKET *strip, POLY_PACKET *packet, POLYGON *poly, int type )
{
	static int get_side[3][3] = {{-1,0,2},{0,-1,1},{2,1,-1}};/* 頂点番号から辺番号へ */
	static int next_vertex[3] = { 2, 0, 1 };/* 次の頂点番号へ */
	VERTEX	*v1, *v2 ;
	int		index1, index2 ;
	int		count, side_no, old_side, i, flag = 0 ;
	POLYGON	*old_poly ;


	InitPolygonPacket( strip );

	count = 0 ;

	/* スタートの頂点を決定する */
	switch ( type ){
	  case 0:
		/* ０から順方向 */
		index1 = 1 ;
		index2 = 2 ;
		break ;
	  case 1:
		/* １から順方向 */
		index1 = 2 ;
		index2 = 0 ;
		break ;
	  case 2:
		/* ２から順方向 */
		index1 = 0 ;
		index2 = 1 ;
		break ;
	  case 3:
		/* ０から逆方向 */
		index1 = 2 ;
		index2 = 1 ;
		break ;
	  case 4:
		/* １から逆方向 */
		index1 = 0 ;
		index2 = 2 ;
		break ;
	  case 5:
		/* ２から逆方向 */
		index1 = 1 ;
		index2 = 0 ;
		break ;
	}
	v1 = &poly->vertex[index1] ;
	v2 = &poly->vertex[index2] ;

	while ( 1 ){
		/* カウント数増加 */
		count++ ;
		/* ポリゴンを移す */
		DeletePolygon( packet, poly );
		AddPolygon( strip, poly );
		/* 接続辺番号を求める */
		side_no = get_side[ index1 ][ index2 ] ;
		if ( side_no == -1 ) break ;
		/* 接続先のポリゴンを取得する */
		old_poly = poly ;
		old_side = side_no ;
		side_no = old_poly->link_side[ old_side ];/* 接続先での接続辺番号を取得する */
		poly = old_poly->link[ old_side ] ;
		/* 古いリンク情報を修正する */
		for ( i = 0 ; i < 3 ; i++ ){
			if ( old_poly->link_side[i] != -1 ){
				old_poly->link[i]->link[old_poly->link_side[i]] = NULL ;
				old_poly->link[i]->link_side[old_poly->link_side[i]] = -1 ;
			}
			old_poly->link[i] = NULL ;
			old_poly->link_side[i] = -1 ;
		}
		if  ( side_no == -1 ) break ;
		/* 次のラスト２点のインデックス番号と頂点データのポインタを取得 */
		index1 = SearchVertexNumber( poly, v2 );
		index2 = next_vertex[ side_no ] ;
		v1 = &poly->vertex[index1] ;
		v2 = &poly->vertex[index2] ;
		
	}

	return ( count );
}
/* 後半２つの頂点を共有しているかチェック（並べ替えも行なう） */
static int CheckPolygonConnection( POLYGON *prev, POLYGON *next )
{
	int		i, j, checked, free ;
	VERTEX	tmp[3] ;

	checked = -1 ;
	free = 2 ;
	for ( j = 0 ; j < 3 ; j++ ){
		for ( i = 1 ; i < 3 ; i++ ){
			if ( checked == i ) continue ;
			if ( IsEqualVertex( &prev->vertex[i], &next->vertex[j] ) ){
				if ( checked != -1 ){
					tmp[i-1] = next->vertex[j] ;
					tmp[2] = next->vertex[free] ;
					next->vertex[0] = tmp[0] ;
					next->vertex[1] = tmp[1] ;
					next->vertex[2] = tmp[2] ;
					return ( 1 );
				} else {
					tmp[i-1] = next->vertex[j] ;
					checked = i ;
					break ;
				}
			}
		}
		if ( i == 3 ) free = j ;
	}
	printf("polygon connection error!\n");
	return ( 0 );
}
/* 先頭のポリゴンからストリップになるように頂点を入れ替える */
/* （但し全ポリゴンのストリップ化が保証されていること） */
void Strip_OptimizeStripPacket( POLY_PACKET *strip_packet, int type )
{
	POLYGON		*poly, *next ;
	POLYGON		*tmp_poly ;			/* オリジナルデータ一時記憶用 */
	int			count = 0 ;

	tmp_poly = AllocPolygon() ;

	poly = GetFirstPolygon( strip_packet );
	CopyPolygon( tmp_poly, poly );
	switch ( type ){
	  case 0:
		/* パターン０ */
		poly->vertex[0] = tmp_poly->vertex[0] ;
		poly->vertex[1] = tmp_poly->vertex[1] ;
		poly->vertex[2] = tmp_poly->vertex[2] ;
		break ;
	  case 1:
		/* パターン１ */
		poly->vertex[0] = tmp_poly->vertex[1] ;
		poly->vertex[1] = tmp_poly->vertex[2] ;
		poly->vertex[2] = tmp_poly->vertex[0] ;
		break ;
	  case 2:
		/* パターン２ */
		poly->vertex[0] = tmp_poly->vertex[2] ;
		poly->vertex[1] = tmp_poly->vertex[0] ;
		poly->vertex[2] = tmp_poly->vertex[1] ;
		break ;
	  case 3:
		/* パターン３ */
		poly->vertex[0] = tmp_poly->vertex[0] ;
		poly->vertex[1] = tmp_poly->vertex[2] ;
		poly->vertex[2] = tmp_poly->vertex[1] ;
		break ;
	  case 4:
		/* パターン４ */
		poly->vertex[0] = tmp_poly->vertex[1] ;
		poly->vertex[1] = tmp_poly->vertex[0] ;
		poly->vertex[2] = tmp_poly->vertex[2] ;
		break ;
	  case 5:
		/* パターン５ */
		poly->vertex[0] = tmp_poly->vertex[2] ;
		poly->vertex[1] = tmp_poly->vertex[1] ;
		poly->vertex[2] = tmp_poly->vertex[0] ;
		break ;
	}

	while ( ( next = GetNextPolygon( poly ) ) != NULL ){
		count++ ;
		if ( CheckPolygonConnection( poly, next ) == 0 ){
			printf("strip optimize error !!\n");
		}
		poly = next ;
	}

	FreePolygon( tmp_poly );

}


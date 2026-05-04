/*
	convert.c
	新ライトデータ（.lt2）分割処理

	1999/10/01 K.Takabe
	$Id: convert.c,v 1.4 2000/08/25 05:09:47 usr02774 Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include "fmt_lit.h"


#define MAX( _a, _b )	( ( (_a) > (_b) ) ? (_a) : (_b) )
#define MINI( _a, _b )	( ( (_a) < (_b) ) ? (_a) : (_b) )

/* ---------------------------------------------------------------- */
static void CheckPointGroupBounding( FVECTOR *max, FVECTOR *min, LIT_POINT *lights, int n_lights );
static void CheckBlackPointGroupBounding( FVECTOR *max, FVECTOR *min, LIT_BLACKPOINT *lights, int n_lights );
static void CheckSpotGroupBounding( FVECTOR *max, FVECTOR *min, LIT_SPOT *lights, int n_lights );
static void CheckLineGroupBounding( FVECTOR *max, FVECTOR *min, LIT_LINE *lights, int n_lights );
static void VectorNormalize( FVECTOR *res, FVECTOR *vec );
static float GetColorForce( CVECTOR *col );

static int SearchPointLights( FVECTOR *min, FVECTOR *max, LIT_DEF *def, LIT_POINT *point, int type );
static int SearchSpotLights( FVECTOR *min, FVECTOR *max, LIT_DEF *def, LIT_SPOT *spot, int type );
static int SearchLineLights( FVECTOR *min, FVECTOR *max, LIT_DEF *def, LIT_LINE *line, int type );
static int SearchBlackPointLights( FVECTOR *min, FVECTOR *max, LIT_DEF *def, LIT_BLACKPOINT *black, int type );

/* ---------------------------------------------------------------- */
int DivideLt2File( char *filename, char *out_filename, int flag, int x, int y, int z )
{
	FILE	*fp, *out ;
	int		i, size, group_count, n ;
	int		total_point_size, total_spot_size, total_black_size, total_line_size ;
	void	*buffer ;
	LIT_POINT	*point_lights ;
	LIT_DEF		*org_lit_def, lit_def ;
	LIT_GRP		*lit_grp, *group ;
	FVECTOR		lit_max, lit_min ;
	int			div_n_x, div_n_y, div_n_z ;
	int			lp_x, lp_y, lp_z ;
	FVECTOR		low_base, base, offset ;
	LIT_POINT		points[1024] ;
	LIT_SPOT		spots[1024] ;
	LIT_LINE		lines[1024] ;
	LIT_BLACKPOINT	blacks[1024] ;

	if ( ( fp = fopen( filename, "rb" ) ) == NULL ){
		fprintf( stderr, "file open error !! \n" );
		return (-1);
	}

	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	buffer = malloc( size );
	fread( buffer, sizeof(char), size, fp );
	fclose( fp );
	printf("lt2 file load ... ok!\n");

	org_lit_def = buffer ;

	/* ライトのバウンディングを計算 */
	total_point_size = total_spot_size = total_line_size = total_black_size = 0 ;
	for ( i = 0 ; i < org_lit_def->n_lit_group ; i++ ){
		group = &org_lit_def->grp[i] ;
		group->lit = (void*)( (int)group->lit + (int)org_lit_def );
		if ( i == 0 ){
			lit_max = group->bound_max ;
			lit_min = group->bound_min ;
		} else {
			lit_max.vx = MAX( lit_max.vx, group->bound_max.vx );
			lit_max.vy = MAX( lit_max.vy, group->bound_max.vy );
			lit_max.vz = MAX( lit_max.vz, group->bound_max.vz );
			lit_min.vx = MINI( lit_min.vx, group->bound_min.vx );
			lit_min.vy = MINI( lit_min.vy, group->bound_min.vy );
			lit_min.vz = MINI( lit_min.vz, group->bound_min.vz );
		}
		if ( group->type & LIT_TYPE_POINT ) total_point_size += group->n_lights ;
		if ( group->type & LIT_TYPE_SPOT ) total_spot_size += group->n_lights ;
		if ( group->type & LIT_TYPE_LINE ) total_line_size += group->n_lights ;
		if ( group->type & LIT_TYPE_BLACKPOINT ) total_black_size += group->n_lights ;
	}
	printf("lt2 file bound max: %f %f %f\n", lit_max.vx, lit_max.vy, lit_max.vz );
	printf("lt2 file bound min: %f %f %f\n", lit_min.vx, lit_min.vy, lit_min.vz );
	printf("orignal total group : %d \n", org_lit_def->n_lit_group );
	printf("orignal total point ... %d\n", total_point_size );
	printf("orignal total spot ... %d\n", total_spot_size );
	printf("orignal total line ... %d\n", total_line_size );
	printf("orignal total blackpoint ... %d\n", total_black_size );

	/* 分割数計算 */
	div_n_x = div_n_y = div_n_z = 1000 ;
	while (1){
		div_n_x = ( (int)lit_max.vx + x - 1 ) / x - ( (int)lit_min.vx - x + 1 ) / x ;
		div_n_y = ( (int)lit_max.vy + y - 1 ) / y - ( (int)lit_min.vy - y + 1 ) / y ;
		div_n_z = ( (int)lit_max.vz + z - 1 ) / z - ( (int)lit_min.vz - z + 1 ) / z ;
		if( ( div_n_x * div_n_y * div_n_z ) > 4096 ){
			int *ptr ;
			ptr = ( x < y ) ? ( &x ) : ( &y ) ;
			ptr = ( *ptr < z ) ? ( ptr ) : ( &z ) ;
			*ptr *= 2 ;
			printf("retry divide ( x:%d y:%d z:%d)\n", x, y, z );
			continue ;
		}
		break ;
	}

	printf("divide ok( x:%d y:%d z:%d)(%d %d %d)\n", x, y, z, div_n_x, div_n_y, div_n_z );
	lit_grp = malloc( sizeof(LIT_GRP) * 1 );

	group_count = 0 ;
	total_point_size = total_spot_size = total_line_size = total_black_size = 0 ;
	group = lit_grp ;
	low_base.vx = ( ( (int)lit_min.vx - x + 1 ) / x ) * x ;
	low_base.vy = ( ( (int)lit_min.vy - y + 1 ) / y ) * y ;
	low_base.vz = ( ( (int)lit_min.vz - z + 1 ) / z ) * z ;
	for ( lp_y = 0, base.vy = low_base.vy ; lp_y < div_n_y ; lp_y++, base.vy += y ){
		for ( lp_z = 0, base.vz = low_base.vz ; lp_z < div_n_z ; lp_z++, base.vz += z ){
			for ( lp_x = 0, base.vx = low_base.vx ; lp_x < div_n_x ; lp_x++, base.vx += x ){
				offset.vx = base.vx + x ;
				offset.vy = base.vy + y ;
				offset.vz = base.vz + z ;
				/* 通常点光源検索 */
				n = SearchPointLights( &base, &offset, org_lit_def, points, LIT_TYPE_POINT );
				if ( n ){
					/* メモリ確保 */
					lit_grp = realloc( lit_grp, sizeof(LIT_GRP) * ( group_count + 1 ) );
					group = &lit_grp[group_count] ;
					/* 新グループ情報作成 */
					CheckPointGroupBounding( &group->bound_max, &group->bound_min, points, n );
					group->n_lights = n ;
					group->type = LIT_TYPE_POINT ;
					group->lit = malloc( sizeof(LIT_POINT) * n );
					memcpy( group->lit, points, sizeof(LIT_POINT) * n );
					group++ ;
					group_count++ ;
					total_point_size += n ;
				}
				/* 動的点光源検索 */
				n = SearchPointLights( &base, &offset, org_lit_def, points, LIT_TYPE_POINT|LIT_TYPE_DYNAMIC );
				if ( n ){
					/* メモリ確保 */
					lit_grp = realloc( lit_grp, sizeof(LIT_GRP) * ( group_count + 1 ) );
					group = &lit_grp[group_count] ;
					/* 新グループ情報作成 */
					CheckPointGroupBounding( &group->bound_max, &group->bound_min, points, n );
					group->n_lights = n ;
					group->type = LIT_TYPE_POINT|LIT_TYPE_DYNAMIC ;
					group->lit = malloc( sizeof(LIT_POINT) * n );
					memcpy( group->lit, points, sizeof(LIT_POINT) * n );
					group++ ;
					group_count++ ;
					total_point_size += n ;
				}

				/* 通常スポット光源検索 */
				n = SearchSpotLights( &base, &offset, org_lit_def, spots, LIT_TYPE_SPOT );
				if ( n ){
					/* メモリ確保 */
					lit_grp = realloc( lit_grp, sizeof(LIT_GRP) * ( group_count + 1 ) );
					group = &lit_grp[group_count] ;
					/* 新グループ情報作成 */
					CheckSpotGroupBounding( &group->bound_max, &group->bound_min, spots, n );
					group->n_lights = n ;
					group->type = LIT_TYPE_SPOT ;
					group->lit = malloc( sizeof(LIT_SPOT) * n );
					memcpy( group->lit, spots, sizeof(LIT_SPOT) * n );
					group++ ;
					group_count++ ;
					total_spot_size += n ;
				}
				/* 動的スポット光源検索 */
				n = SearchSpotLights( &base, &offset, org_lit_def, spots, LIT_TYPE_SPOT|LIT_TYPE_DYNAMIC );
				if ( n ){
					/* メモリ確保 */
					lit_grp = realloc( lit_grp, sizeof(LIT_GRP) * ( group_count + 1 ) );
					group = &lit_grp[group_count] ;
					/* 新グループ情報作成 */
					CheckSpotGroupBounding( &group->bound_max, &group->bound_min, spots, n );
					group->n_lights = n ;
					group->type = LIT_TYPE_SPOT|LIT_TYPE_DYNAMIC ;
					group->lit = malloc( sizeof(LIT_SPOT) * n );
					memcpy( group->lit, spots, sizeof(LIT_SPOT) * n );
					group++ ;
					group_count++ ;
					total_spot_size += n ;
				}

				/* 通常線光源検索 */
				n = SearchLineLights( &base, &offset, org_lit_def, lines, LIT_TYPE_LINE );
				if ( n ){
					/* メモリ確保 */
					lit_grp = realloc( lit_grp, sizeof(LIT_GRP) * ( group_count + 1 ) );
					group = &lit_grp[group_count] ;
					/* 新グループ情報作成 */
					CheckLineGroupBounding( &group->bound_max, &group->bound_min, lines, n );
					group->n_lights = n ;
					group->type = LIT_TYPE_LINE ;
					group->lit = malloc( sizeof(LIT_LINE) * n );
					memcpy( group->lit, lines, sizeof(LIT_LINE) * n );
					group++ ;
					group_count++ ;
					total_line_size += n ;
				}
				/* 動的線光源検索 */
				n = SearchLineLights( &base, &offset, org_lit_def, lines, LIT_TYPE_LINE|LIT_TYPE_DYNAMIC );
				if ( n ){
					/* メモリ確保 */
					lit_grp = realloc( lit_grp, sizeof(LIT_GRP) * ( group_count + 1 ) );
					group = &lit_grp[group_count] ;
					/* 新グループ情報作成 */
					CheckLineGroupBounding( &group->bound_max, &group->bound_min, lines, n );
					group->n_lights = n ;
					group->type = LIT_TYPE_LINE|LIT_TYPE_DYNAMIC ;
					group->lit = malloc( sizeof(LIT_LINE) * n );
					memcpy( group->lit, lines, sizeof(LIT_LINE) * n );
					group++ ;
					group_count++ ;
					total_line_size += n ;
				}

				/* 通常黒点光源検索 */
				n = SearchBlackPointLights( &base, &offset, org_lit_def, blacks, LIT_TYPE_BLACKPOINT );
				if ( n ){
					/* メモリ確保 */
					lit_grp = realloc( lit_grp, sizeof(LIT_GRP) * ( group_count + 1 ) );
					group = &lit_grp[group_count] ;
					/* 新グループ情報作成 */
					CheckBlackPointGroupBounding( &group->bound_max, &group->bound_min, blacks, n );
					group->n_lights = n ;
					group->type = LIT_TYPE_BLACKPOINT ;
					group->lit = malloc( sizeof(LIT_BLACKPOINT) * n );
					memcpy( group->lit, blacks, sizeof(LIT_BLACKPOINT) * n );
					group++ ;
					group_count++ ;
					total_black_size += n ;
				}
				/* 動的黒点光源検索 */
				n = SearchBlackPointLights( &base, &offset, org_lit_def, blacks, LIT_TYPE_BLACKPOINT|LIT_TYPE_DYNAMIC );
				if ( n ){
					/* メモリ確保 */
					lit_grp = realloc( lit_grp, sizeof(LIT_GRP) * ( group_count + 1 ) );
					group = &lit_grp[group_count] ;
					/* 新グループ情報作成 */
					CheckBlackPointGroupBounding( &group->bound_max, &group->bound_min, blacks, n );
					group->n_lights = n ;
					group->type = LIT_TYPE_BLACKPOINT|LIT_TYPE_DYNAMIC ;
					group->lit = malloc( sizeof(LIT_BLACKPOINT) * n );
					memcpy( group->lit, blacks, sizeof(LIT_BLACKPOINT) * n );
					group++ ;
					group_count++ ;
					total_black_size += n ;
				}
			}
		}
	}
	printf("total group : %d \n", group_count );
	printf("total point ... %d\n", total_point_size );
	printf("total spot ... %d\n", total_spot_size );
	printf("total line ... %d\n", total_line_size );
	printf("total blackpoint ... %d\n", total_black_size );

	lit_def = *org_lit_def ;
	lit_def.n_lit_group = group_count ;

	/* ファイルへの書き出し  */
	if ( ( out = fopen( out_filename, "wb" ) ) == NULL ){
		fprintf( stderr, "file write error !!(%s)\n", out_filename );
		return ( -1 );
	}

	fwrite( &lit_def, sizeof(LIT_DEF), 1, out );

	/* グループ情報の書き出し */
	size = sizeof(LIT_DEF) + sizeof(LIT_GRP) * lit_def.n_lit_group ;
	for ( i = 0 ; i < lit_def.n_lit_group ; i++ ){
		LIT_GRP		tmp ;
		tmp = lit_grp[i] ;
		tmp.lit = (void*)size ;
		fwrite( &tmp, sizeof(LIT_GRP), 1, out );
		if ( lit_grp[i].type & LIT_TYPE_POINT ) size += sizeof(LIT_POINT)*lit_grp[i].n_lights ;
		if ( lit_grp[i].type & LIT_TYPE_SPOT ) size += sizeof(LIT_SPOT)*lit_grp[i].n_lights ;
		if ( lit_grp[i].type & LIT_TYPE_LINE ) size += sizeof(LIT_LINE)*lit_grp[i].n_lights ;
		if ( lit_grp[i].type & LIT_TYPE_BLACKPOINT ) size += sizeof(LIT_BLACKPOINT)*lit_grp[i].n_lights ;
	}

	/* 光源情報の書き出し */
	for ( i = 0 ; i < lit_def.n_lit_group ; i++ ){
		int	w_size = 0 ;
		if ( lit_grp[i].type & LIT_TYPE_POINT ) w_size = sizeof(LIT_POINT)*lit_grp[i].n_lights ;
		if ( lit_grp[i].type & LIT_TYPE_SPOT ) w_size = sizeof(LIT_SPOT)*lit_grp[i].n_lights ;
		if ( lit_grp[i].type & LIT_TYPE_LINE ) w_size = sizeof(LIT_LINE)*lit_grp[i].n_lights ;
		if ( lit_grp[i].type & LIT_TYPE_BLACKPOINT ) w_size = sizeof(LIT_BLACKPOINT)*lit_grp[i].n_lights ;
		fwrite( lit_grp[i].lit, 1, w_size, out );
		free( lit_grp[i].lit );
	}

	fclose( out );

	free( lit_grp );
	free( buffer );

	return (0);
}

/* ---------------------------------------------------------------- */
static void CheckPointGroupBounding( FVECTOR *max, FVECTOR *min, LIT_POINT *lights, int n_lights )
{
	int		i ;

	max->vx = -1000000.0f ;
	max->vy = -1000000.0f ;
	max->vz = -1000000.0f ;
	max->vw = 0.0f ;
	min->vx = 1000000.0f ;
	min->vy = 1000000.0f ;
	min->vz = 1000000.0f ;
	min->vw = 0.0f ;

	for ( i = 0 ; i < n_lights ; i++ ){
		max->vx = MAX( lights->point.vx + lights->e_range, max->vx );
		max->vy = MAX( lights->point.vy + lights->e_range, max->vy );
		max->vz = MAX( lights->point.vz + lights->e_range, max->vz );
		min->vx = MINI( lights->point.vx - lights->e_range, min->vx );
		min->vy = MINI( lights->point.vy - lights->e_range, min->vy );
		min->vz = MINI( lights->point.vz - lights->e_range, min->vz );
		lights++ ;
	}
}

static void CheckSpotGroupBounding( FVECTOR *max, FVECTOR *min, LIT_SPOT *lights, int n_lights )
{
	int		i ;

	max->vx = -1000000.0f ;
	max->vy = -1000000.0f ;
	max->vz = -1000000.0f ;
	max->vw = 0.0f ;
	min->vx = 1000000.0f ;
	min->vy = 1000000.0f ;
	min->vz = 1000000.0f ;
	min->vw = 0.0f ;

	for ( i = 0 ; i < n_lights ; i++ ){
#if 0
		max->vx = MAX( lights->point.vx + lights->dir.vw, max->vx );
		max->vy = MAX( lights->point.vy + lights->dir.vw, max->vy );
		max->vz = MAX( lights->point.vz + lights->dir.vw, max->vz );
		min->vx = MINI( lights->point.vx - lights->dir.vw, min->vx );
		min->vy = MINI( lights->point.vy - lights->dir.vw, min->vy );
		min->vz = MINI( lights->point.vz - lights->dir.vw, min->vz );
#else
		max->vx = MAX( lights->bound_max.vx, max->vx );
		max->vy = MAX( lights->bound_max.vy, max->vy );
		max->vz = MAX( lights->bound_max.vz, max->vz );
		min->vx = MINI( lights->bound_min.vx, min->vx );
		min->vy = MINI( lights->bound_min.vy, min->vy );
		min->vz = MINI( lights->bound_min.vz, min->vz );
#endif
		lights++ ;
	}
}

static void CheckLineGroupBounding( FVECTOR *max, FVECTOR *min, LIT_LINE *lights, int n_lights )
{
	int		i ;

	max->vx = -1000000.0f ;
	max->vy = -1000000.0f ;
	max->vz = -1000000.0f ;
	max->vw = 0.0f ;
	min->vx = 1000000.0f ;
	min->vy = 1000000.0f ;
	min->vz = 1000000.0f ;
	min->vw = 0.0f ;

	for ( i = 0 ; i < n_lights ; i++ ){
		max->vx = MAX( lights->bound_max.vx, max->vx );
		max->vy = MAX( lights->bound_max.vy, max->vy );
		max->vz = MAX( lights->bound_max.vz, max->vz );
		min->vx = MINI( lights->bound_min.vx, min->vx );
		min->vy = MINI( lights->bound_min.vy, min->vy );
		min->vz = MINI( lights->bound_min.vz, min->vz );
		lights++ ;
	}
}

static void CheckBlackPointGroupBounding( FVECTOR *max, FVECTOR *min, LIT_BLACKPOINT *lights, int n_lights )
{
	int		i ;

	max->vx = -1000000.0f ;
	max->vy = -1000000.0f ;
	max->vz = -1000000.0f ;
	max->vw = 0.0f ;
	min->vx = 1000000.0f ;
	min->vy = 1000000.0f ;
	min->vz = 1000000.0f ;
	min->vw = 0.0f ;

	for ( i = 0 ; i < n_lights ; i++ ){
		max->vx = MAX( lights->bound_max.vx, max->vx );
		max->vy = MAX( lights->bound_max.vy, max->vy );
		max->vz = MAX( lights->bound_max.vz, max->vz );
		min->vx = MINI( lights->bound_min.vx, min->vx );
		min->vy = MINI( lights->bound_min.vy, min->vy );
		min->vz = MINI( lights->bound_min.vz, min->vz );
		lights++ ;
	}
}

static void VectorNormalize( FVECTOR *res, FVECTOR *vec )
{
	float	div ;
	div = 1.0f / sqrt( vec->vx * vec->vx + vec->vy * vec->vy + vec->vz * vec->vz );
	res->vx = vec->vx * div ;
	res->vy = vec->vy * div ;
	res->vz = vec->vz * div ;
}

static float GetColorForce( CVECTOR *col )
{
	float	force ;
	force = ( 0.30 * (float)col->r + 0.59 * (float)col->g + 0.11 * (float)col->b ) / 255.0 ;
	return ( force );
}

/* ---------------------------------------------------------------- */
static int SearchPointLights( FVECTOR *min, FVECTOR *max, LIT_DEF *def, LIT_POINT *point, int type )
{
	LIT_GRP	*grp ;
	LIT_POINT	*lit ;
	int		i, j, find = 0 ;
	float	e ;

	grp = def->grp ;
	for ( i = 0 ; i < def->n_lit_group ; grp++, i++ ){
		if ( grp->type != type ) continue ;
		lit = grp->lit ;
		for ( j = 0 ; j < grp->n_lights ; lit++, j++ ){
			if ( lit->flag & LIT_FLAG_DISABLE ) continue ;
			if ( lit->point.vx > max->vx ) continue ; 
			if ( lit->point.vz > max->vz ) continue ; 
			if ( lit->point.vy > max->vy ) continue ; 
			*point++ = *lit ;
			find++ ;
			lit->flag |= LIT_FLAG_DISABLE ;
		}
	}
	return ( find );
}

static int SearchSpotLights( FVECTOR *min, FVECTOR *max, LIT_DEF *def, LIT_SPOT *spot, int type )
{
	LIT_GRP	*grp ;
	LIT_SPOT	*lit ;
	int		i, j, find = 0 ;
	float	e ;
	FVECTOR	center ;

	grp = def->grp ;
	for ( i = 0 ; i < def->n_lit_group ; grp++, i++ ){
		if ( grp->type != type ) continue ;
		lit = grp->lit ;
		for ( j = 0 ; j < grp->n_lights ; lit++, j++ ){
			if ( lit->flag & LIT_FLAG_DISABLE ) continue ;
			//if ( lit->point.vx > max->vx ) continue ; 
			//if ( lit->point.vz > max->vz ) continue ; 
			//if ( lit->point.vy > max->vy ) continue ; 
			center.vx = ( lit->bound_max.vx + lit->bound_min.vx ) / 2.0f ;
			center.vy = ( lit->bound_max.vy + lit->bound_min.vy ) / 2.0f ;
			center.vz = ( lit->bound_max.vz + lit->bound_min.vz ) / 2.0f ;
			if ( center.vx > max->vx ) continue ; 
			if ( center.vz > max->vz ) continue ; 
			if ( center.vy > max->vy ) continue ; 
			*spot++ = *lit ;
			find++ ;
			lit->flag |= LIT_FLAG_DISABLE ;
		}
	}
	return ( find );
}

static int SearchLineLights( FVECTOR *min, FVECTOR *max, LIT_DEF *def, LIT_LINE *line, int type )
{
	LIT_GRP	*grp ;
	LIT_LINE	*lit ;
	int		i, j, find = 0 ;
	float	e ;
	FVECTOR	center ;

	grp = def->grp ;
	for ( i = 0 ; i < def->n_lit_group ; grp++, i++ ){
		if ( grp->type != type ) continue ;
		lit = grp->lit ;
		for ( j = 0 ; j < grp->n_lights ; lit++, j++ ){
			if ( lit->flag & LIT_FLAG_DISABLE ) continue ;
			center.vx = ( lit->bound_max.vx + lit->bound_min.vx ) / 2.0f ;
			center.vy = ( lit->bound_max.vy + lit->bound_min.vy ) / 2.0f ;
			center.vz = ( lit->bound_max.vz + lit->bound_min.vz ) / 2.0f ;
			if ( center.vx > max->vx ) continue ; 
			if ( center.vz > max->vz ) continue ; 
			if ( center.vy > max->vy ) continue ; 
			*line++ = *lit ;
			find++ ;
			lit->flag |= LIT_FLAG_DISABLE ;
		}
	}
	return ( find );
}

static int SearchBlackPointLights( FVECTOR *min, FVECTOR *max, LIT_DEF *def, LIT_BLACKPOINT *black, int type )
{
	LIT_GRP	*grp ;
	LIT_BLACKPOINT	*lit ;
	int		i, j, find = 0 ;
	float	e ;
	FVECTOR	center ;

	grp = def->grp ;
	for ( i = 0 ; i < def->n_lit_group ; grp++, i++ ){
		if ( grp->type != type ) continue ;
		lit = grp->lit ;
		for ( j = 0 ; j < grp->n_lights ; lit++, j++ ){
			if ( lit->flag & LIT_FLAG_DISABLE ) continue ;
			//if ( lit->point.vx > max->vx ) continue ; 
			//if ( lit->point.vz > max->vz ) continue ; 
			//if ( lit->point.vy > max->vy ) continue ; 
			center.vx = ( lit->bound_max.vx + lit->bound_min.vx ) / 2.0f ;
			center.vy = ( lit->bound_max.vy + lit->bound_min.vy ) / 2.0f ;
			center.vz = ( lit->bound_max.vz + lit->bound_min.vz ) / 2.0f ;
			if ( center.vx > max->vx ) continue ; 
			if ( center.vz > max->vz ) continue ; 
			if ( center.vy > max->vy ) continue ; 
			*black++ = *lit ;
			find++ ;
			lit->flag |= LIT_FLAG_DISABLE ;
		}
	}
	return ( find );
}





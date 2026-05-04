/*
	convert.c
	旧ライトデータ（.lit）＞新ライトデータ（.lt2）変換

	1999/09/27 K.Takabe
	$Id: convert.c,v 1.5 1999/10/22 05:24:11 usr02774 Exp $
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
static void VectorNormalize( FVECTOR *res, FVECTOR *vec );
static float GetColorForce( CVECTOR *col );

/* ---------------------------------------------------------------- */
int ConvertLitFile( char *filename, float scale )
{
	char	out_filename[512];
	FILE	*fp, *out ;
	int		i, size ;
	void	*buffer ;
	DG_LITS	*lits ;
	DG_LIT	*lights ;
	LIT_POINT	*point_lights ;
	LIT_DEF		lit_def ;
	LIT_GRP		lit_grp ;

	{/**/
		char	*ext ;
		strcpy( out_filename, filename );
		ext = &out_filename[ strlen( out_filename )-3 ] ;
		if ( !( ( ext[0] == 'L' || ext[0] == 'l' ) &&
				( ext[1] == 'I' || ext[1] == 'i' ) &&
				( ext[2] == 'T' || ext[2] == 't' ) ) ){
			fprintf( stderr, "file error !!\n", ext );
			return( -1 ) ;
		}
		ext[0] = 'l' ;
		ext[1] = 't' ;
		ext[2] = '2' ;
	}

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

	lits = (DG_LITS*)buffer ;
	lights = lits->lights ;

	memset( &lit_def, 0, sizeof(LIT_DEF) );
	lit_def.n_lit_group = 1 ;
	{
		extern void* readline( char * );
		char	*input ;
		FVECTOR	vec, col ;
		input = readline( "input dir ( x y z ): " );
		sscanf( input, "%f %f %f", &vec.vx, &vec.vy, &vec.vz );
		free( input );
		input = readline( "input color ( r g b ): " );
		sscanf( input, "%f %f %f", &col.vx, &col.vy, &col.vz );
		free( input );
		lit_def.color.r = col.vx ;
		lit_def.color.g = col.vy ;
		lit_def.color.b = col.vz ;
		lit_def.color.pad = 0 ;
		VectorNormalize( &lit_def.dir, &vec );
		lit_def.dir.vw = GetColorForce( &lit_def.color );
		input = readline( "input ambient ( r g b ): " );
		sscanf( input, "%f %f %f", &col.vx, &col.vy, &col.vz );
		free( input );
		lit_def.ambient.r = col.vx ;
		lit_def.ambient.g = col.vy ;
		lit_def.ambient.b = col.vz ;
		lit_def.ambient.pad = 0 ;
	}

	/* convert DG_LIT to LIT_POINT */
	point_lights = malloc( sizeof(LIT_POINT) * lits->n_lights );
	for ( i = 0 ; i < lits->n_lights ; i++ ){
		point_lights[i].point.vx = lights[i].point.vx * scale ;
		point_lights[i].point.vy = lights[i].point.vy * scale ;
		point_lights[i].point.vz = lights[i].point.vz * scale ;
		point_lights[i].color.r = lights[i].color.r ;
		point_lights[i].color.g = lights[i].color.g ;
		point_lights[i].color.b = lights[i].color.b ;
		point_lights[i].color.pad = 0 ;
		point_lights[i].r_range = lights[i].r_range * scale ;
		point_lights[i].e_range = lights[i].e_range * scale ;
		point_lights[i].point.vw = GetColorForce( &lights[i].color );
		point_lights[i].flag = LIT_FLAG_CHARAONLY|LIT_FLAG_BGONLY ;
	}

	memset( &lit_grp, 0, sizeof(LIT_GRP) );
	lit_grp.type = LIT_TYPE_POINT ;
	lit_grp.n_lights = lits->n_lights ;
	lit_grp.lit = (void*)( sizeof(LIT_DEF) + sizeof(LIT_GRP)*1 );
	CheckPointGroupBounding( &lit_grp.bound_max, &lit_grp.bound_min, point_lights, lit_grp.n_lights );

	if ( ( out = fopen( out_filename, "wb" ) ) == NULL ){
		fprintf( stderr, "file open error !! \n" );
		return ( -1 );
	}
	fwrite( &lit_def, sizeof(LIT_DEF), 1, out );
	fwrite( &lit_grp, sizeof(LIT_GRP), 1, out );
	fwrite( point_lights, sizeof(LIT_POINT), lit_grp.n_lights, out );
	fclose( out );

#if 0
	{
		FVECTOR	max, min ;
		CheckPointGroupBounding( &max, &min, point_lights, lits->n_lights );
		printf("%s\n", out_filename );
		printf("max : %f %f %f\n", max.vx, max.vy, max.vz );
		printf("min : %f %f %f\n", min.vx, min.vy, min.vz );
	}
#endif

	free( point_lights );
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


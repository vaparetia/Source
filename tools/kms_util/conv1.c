/*
	conv1.c
	convert program

	1999/10/14 K.Takabe
	$Id: conv1.c,v 1.4 2000/01/22 10:33:00 usr02774 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "fmt_kms.h"
#include "prog_utl.h"

/* ================================================================ */
/*
	Open Kms file
 */
static KMS_DEF *OpenKms( char *filename )
{
	FILE	*fp ;
	int		size, i, j ;
	KMS_DEF	*def ;
	KMS_MDL	*mdl ;
	KMS_OBJPACK	*pack ;

	if ( ( fp = fopen( filename, "rb" ) ) == NULL ){
		printf("kms file open error!(%s)\n", filename );
		return ( NULL );
	}

	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	def = Malloc0( size );
	fread( def, 1, size, fp );
	fclose( fp );

	if ( def->n_x_models == 0 ) def->n_x_models = def->n_models ;

	/* kms setup */
	mdl = def->models ;
	for ( i = def->n_x_models ; i > 0 ; i--, mdl++ ){
		mdl->packs = (KMS_OBJPACK*)( (int)mdl->packs + (int)def );
		pack = mdl->packs ;
		for ( j = mdl->n_packs ; j > 0 ; j--, pack++ ){
			pack->verts = (short*)( (int)pack->verts + (int)def );
			pack->norms = (short*)( (int)pack->norms + (int)def );
			pack->uvs = (short*)( (int)pack->uvs + (int)def );
			//pack->rgbs = (short*)( (int)pack->rgbs + (int)def );	/* 未使用なので */
		}
	}

	return ( def ) ;
}

/* ================================================================ */
/*
	同じ頂点を検索
*/
static int SearchVertex( KMS_MDL *mdl, int n_packs, int n_verts, SVECTOR *org )
{
	int		k, l, total_num ; 
	KMS_OBJPACK	*pack ;
	SVECTOR		*vec ;

	total_num = 0 ;
	pack = mdl->packs ;
	for ( k = 0 ; k <= n_packs ; k++ ){
		vec = (SVECTOR*)pack->verts ;
		if ( k == n_packs ){
			l = n_verts ;
		} else {
			l = pack->n_verts ;
		}
		for (  ; l > 0 ; l-- ){
			if ( ( vec->vx == org->vx ) && ( vec->vy == org->vy )
				 && ( vec->vz == org->vz ) && ( vec->vw == org->vw ) ){
				/* 同一頂点発見 */
				return ( total_num );
			}
			vec++ ;
			total_num++ ;
		}
		total_num += pack->n_verts & 0x01 ;
		pack++ ;
	}
	return ( -1 );
}

static int SearchNormal( KMS_MDL *mdl, int n_packs, int n_verts, SVECTOR *org )
{
	int		k, l, total_num ; 
	KMS_OBJPACK	*pack ;
	SVECTOR		*vec ;

	total_num = 0 ;
	pack = mdl->packs ;
	for ( k = 0 ; k <= n_packs ; k++ ){
		vec = (SVECTOR*)pack->norms ;
		if ( k == n_packs ){
			l = n_verts ;
		} else {
			l = pack->n_verts ;
		}
		for (  ; l > 0 ; l-- ){
			if ( ( vec->vx == org->vx ) && ( vec->vy == org->vy )
				 && ( vec->vz == org->vz ) && ( vec->vw == org->vw ) ){
				/* 同一頂点発見 */
				return ( total_num );
			}
			vec++ ;
			total_num++ ;
		}
		total_num += pack->n_verts & 0x01 ;
		pack++ ;
	}
	return ( -1 );
}

static int SearchUV( KMS_MDL *mdl, int n_packs, int n_verts, TSVECTOR *org )
{
	int		k, l, total_num ; 
	KMS_OBJPACK	*pack ;
	TSVECTOR		*vec ;

	total_num = 0 ;
	pack = mdl->packs ;
	for ( k = 0 ; k <= n_packs ; k++ ){
		vec = (TSVECTOR*)pack->uvs ;
		if ( k == n_packs ){
			l = n_verts ;
		} else {
			l = pack->n_verts ;
		}
		for (  ; l > 0 ; l-- ){
			if ( ( vec->u == org->u ) && ( vec->v == org->v ) ){
				/* 同一頂点発見 */
				return ( total_num );
			}
			vec++ ;
			total_num++ ;
		}
		if ( ( l = pack->n_verts & 0x03 ) != 0 ){
			for (  ; l < 4 ; l++ ) total_num++ ;
		}
		pack++ ;
	}
	return ( -1 );
}


/*
	共有頂点情報を作成
*/
static void CreateCommonVertex( KMS_MDL *mdl, COMMON_V_OBJ *obj )
{
	KMS_OBJPACK	*pack ;
	SVECTOR		*vec ;
	int			i, j, v_size, wk_size, index, total ;
	short		*v_index ;
	FVECTOR		*fverts ;

	/* ワークの最大を求める */
	wk_size = 0 ;
	pack = mdl->packs ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		wk_size += ( pack->n_verts + 1 ) & 0xfffe ;
		pack++ ;
	}
	obj->n_verts_index = wk_size ;
	obj->verts_index = Malloc0( sizeof(short) * wk_size );
//printf(" total index ============== %d\n", wk_size );

	/* 共有頂点にした場合の頂点数を求める（遅い！！） */
	v_size = 0 ;
	pack = mdl->packs ;
	v_index = obj->verts_index ;
	for ( i = 0 ; i < mdl->n_packs ; i++ ){
		vec = (SVECTOR*)pack->verts ;
		for ( j = 0 ; j < pack->n_verts ; j++ ){
			index = SearchVertex( mdl, i, j, vec );
			*v_index = index ;
			if ( index == -1 ) v_size++ ;
			vec++ ;
			v_index++ ;
		}
		if ( pack->n_verts & 1 ){
			*v_index = 0 ;
			v_index++ ;
		}
		pack++ ;
	}
	obj->n_verts = v_size ;
	obj->verts = (void*)Malloc0( sizeof(FVECTOR) * v_size );
	//obj->verts = (void*)( ( (int)Malloc0( sizeof(FVECTOR) * v_size + 64 ) + 63 ) & ~63 );
	//obj->verts = (int)Malloc0( sizeof(FVECTOR) * v_size + 64 ) ;
//printf(" total common verts ============== %d\n", v_size );

	total = 0 ;
	pack = mdl->packs ;
	v_index = obj->verts_index ;
	fverts = obj->verts ;
	for ( i = 0 ; i < mdl->n_packs ; i++ ){
		vec = (SVECTOR*)pack->verts ;
		for ( j = 0 ; j < pack->n_verts ; j++ ){
			index = *v_index ;
			if ( index == -1 ){
				fverts[ total ].vx = vec->vx ;
				fverts[ total ].vy = vec->vy ;
				fverts[ total ].vz = vec->vz ;
				fverts[ total ].vw = (float)vec->vw / 4096.0f ;
				*v_index = total ;
				total++ ;
			} else {
				*v_index = obj->verts_index[index] ;
			}
			vec++ ;
			v_index++ ;
		}
		if ( pack->n_verts & 1 ){
			v_index++ ;
		}
		pack++ ;
	}

}

static void CreateCommonNormal( KMS_MDL *mdl, COMMON_V_OBJ *obj )
{
	KMS_OBJPACK	*pack ;
	SVECTOR		*vec ;
	int			i, j, n_size, wk_size, index, total ;
	short		*n_index ;
	FVECTOR		*fnorms ;

	/* ワークの最大を求める */
	wk_size = 0 ;
	pack = mdl->packs ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		wk_size += ( pack->n_verts + 1 ) & 0xfffe ;
		pack++ ;
	}
	obj->n_norms_index = wk_size ;
	obj->norms_index = Malloc0( sizeof(short) * wk_size );

	/* 共有頂点にした場合の頂点数を求める（遅い！！） */
	n_size = 0 ;
	pack = mdl->packs ;
	n_index = obj->norms_index ;
	for ( i = 0 ; i < mdl->n_packs ; i++ ){
		SVECTOR	*tmp_vvec ;
		vec = (SVECTOR*)pack->norms ;
		tmp_vvec = (SVECTOR*)pack->verts ;
		for ( j = 0 ; j < pack->n_verts ; j++ ){
			vec->vw = tmp_vvec->vw ; tmp_vvec++ ;
			index = SearchNormal( mdl, i, j, vec );
			*n_index = index ;
			if ( index == -1 ) n_size++ ;
			vec++ ;
			n_index++ ;
		}
		if ( pack->n_verts & 1 ){
			*n_index = 0 ;
			n_index++ ;
		}
		pack++ ;
	}
	obj->n_norms = n_size ;
	obj->norms = (void*)Malloc0( sizeof(FVECTOR) * n_size );
	//obj->norms = (void*)( ( (int)Malloc0( sizeof(FVECTOR) * n_size + 64 ) + 63 ) & ~63 );
	//obj->norms = (int)Malloc0( sizeof(FVECTOR) * n_size + 64 ) ;
//printf(" total common norms ============== %d\n", n_size );

	total = 0 ;
	pack = mdl->packs ;
	n_index = obj->norms_index ;
	fnorms = obj->norms ;
	for ( i = 0 ; i < mdl->n_packs ; i++ ){
		vec = (SVECTOR*)pack->norms ;
		for ( j = 0 ; j < pack->n_verts ; j++ ){
			index = *n_index ;
			if ( index == -1 ){
				fnorms[ total ].vx = vec->vx / 4096.0f ;
				fnorms[ total ].vy = vec->vy / 4096.0f ;
				fnorms[ total ].vz = vec->vz / 4096.0f ;
				//fnorms[ total ].vw = 1.0f ;
				fnorms[ total ].vw = vec->vw / 4096.0f ;
				*n_index = total ;
				total++ ;
			} else {
				*n_index = obj->norms_index[index] ;
			}
			vec++ ;
			n_index++ ;
		}
		if ( pack->n_verts & 1 ){
			n_index++ ;
		}
		pack++ ;
	}

}

static void CreateCommonUV( KMS_MDL *mdl, COMMON_V_OBJ *obj )
{
	KMS_OBJPACK	*pack ;
	TSVECTOR	*vec ;
	int			i, j, uv_size, wk_size, index, total ;
	short		*uv_index ;
	FVECTOR		*fuvs ;

	/* ワークの最大を求める */
	wk_size = 0 ;
	pack = mdl->packs ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		wk_size += ( pack->n_verts + 3 ) & 0xfffc ;
		pack++ ;
	}
	obj->n_uvs_index = wk_size ;
	obj->uvs_index = Malloc0( sizeof(short) * wk_size );

	/* 共有頂点にした場合の頂点数を求める（遅い！！） */
	uv_size = 0 ;
	pack = mdl->packs ;
	uv_index = obj->uvs_index ;
	for ( i = 0 ; i < mdl->n_packs ; i++ ){
		vec = (TSVECTOR*)pack->uvs ;
		for ( j = 0 ; j < pack->n_verts ; j++ ){
			index = SearchUV( mdl, i, j, vec );
			*uv_index = index ;
			if ( index == -1 ) uv_size++ ;
			vec++ ;
			uv_index++ ;
		}
		if ( pack->n_verts & 3 ){
			for ( j = 4 - ( pack->n_verts & 3 ) ; j > 0 ; j-- ){
				*uv_index = 0 ;
				uv_index++ ;
			}
		}
		pack++ ;
	}
	obj->n_uvs = uv_size ;
	obj->uvs = (void*)Malloc0( sizeof(FVECTOR) * uv_size );
	//obj->uvs = (void*)( ( (int)Malloc0( sizeof(FVECTOR) * uv_size + 64 ) + 63 ) & ~63 );
	//obj->uvs = (int)Malloc0( sizeof(FVECTOR) * uv_size + 64 ) ;
//printf(" total common uvs ============== %d\n", uv_size );

	total = 0 ;
	pack = mdl->packs ;
	uv_index = obj->uvs_index ;
	fuvs = obj->uvs ;
	for ( i = 0 ; i < mdl->n_packs ; i++ ){
		vec = (TSVECTOR*)pack->uvs ;
		for ( j = 0 ; j < pack->n_verts ; j++ ){
			index = *uv_index ;
			if ( index == -1 ){
				fuvs[ total ].vx = vec->u / 4096.0f ;
				fuvs[ total ].vy = vec->v / 4096.0f ;
				fuvs[ total ].vz = 0.0f ;
				fuvs[ total ].vw = 1.0f ;
				*uv_index = total ;
				total++ ;
			} else {
				*uv_index = obj->uvs_index[index] ;
			}
			vec++ ;
			uv_index++ ;
		}
		if ( pack->n_verts & 3 ){
			for ( j = 4 - ( pack->n_verts & 3 ) ; j > 0 ; j-- ){
				*uv_index = 0 ;
				uv_index++ ;
			}
		}
		pack++ ;
	}

}



/* ================================================================ */
/*
	Save Data
*/
void SaveCvd( char *filename, COMMON_V_OBJS *objs )
{
	FILE	*fp ;
	int		total_n_verts, total_n_norms, total_n_uvs ;
	int		total_n_verts_index, total_n_norms_index, total_n_uvs_index ;
	int		info_size, total_size, size_count, size ;
	int		i, j ;
	void	*write_buffer ;
	COMMON_V_OBJS	*write_objs ;

	total_n_verts = total_n_norms = total_n_uvs = 0 ;
	total_n_verts_index = total_n_norms_index = total_n_uvs_index = 0 ;

	info_size = sizeof(COMMON_V_OBJS) + sizeof(COMMON_V_OBJ) * objs->n_objs ;
	for ( i = 0 ; i < objs->n_objs ; i++ ){
		total_n_verts += AlignSize16( sizeof(FVECTOR), objs->objs[ i ].n_verts ) ;
		total_n_norms += AlignSize16( sizeof(FVECTOR), objs->objs[ i ].n_norms ) ;
		total_n_uvs += AlignSize16( sizeof(FVECTOR), objs->objs[ i ].n_uvs ) ;
		total_n_verts_index += AlignSize16( sizeof(short), objs->objs[ i ].n_verts_index ) ;
		total_n_norms_index += AlignSize16( sizeof(short), objs->objs[ i ].n_norms_index ) ;
		total_n_uvs_index += AlignSize16( sizeof(short), objs->objs[ i ].n_uvs_index ) ;
	}

	total_size = info_size
		+ total_n_verts + total_n_norms + total_n_uvs
		+ total_n_verts_index + total_n_norms_index + total_n_uvs_index ;
	write_objs = write_buffer = Malloc0( total_size );

	if ( ( fp = fopen( filename, "wb" ) ) == NULL ){
		printf("file open error!!(%d)\n", filename);
	}

	size_count = 0 ;
	/* Header write */
	WriteData( write_buffer, size_count, objs, info_size );
	size_count += info_size ;

	/* Common vertex write */
	for ( i = 0 ; i < objs->n_objs ; i++ ){
		write_objs->objs[ i ].verts = (void*)size_count ;
		size = AlignSize16( sizeof(FVECTOR), objs->objs[ i ].n_verts );
		WriteData( write_buffer, size_count, objs->objs[ i ].verts, size );
		size_count += size ;
	}

	/* Common normal write */
	for ( i = 0 ; i < objs->n_objs ; i++ ){
		write_objs->objs[ i ].norms = (void*)size_count ;
		size = AlignSize16( sizeof(FVECTOR), objs->objs[ i ].n_norms );
		WriteData( write_buffer, size_count, objs->objs[ i ].norms, size );
		size_count += size ;
	}

	/* Common UV write */
	for ( i = 0 ; i < objs->n_objs ; i++ ){
		write_objs->objs[ i ].uvs = (void*)size_count ;
		size = AlignSize16( sizeof(FVECTOR), objs->objs[ i ].n_uvs );
		WriteData( write_buffer, size_count, objs->objs[ i ].uvs, size );
		size_count += size ;
	}

	/* Common vertex index write */
	for ( i = 0 ; i < objs->n_objs ; i++ ){
		write_objs->objs[ i ].verts_index = (void*)size_count ;
		size = AlignSize16( sizeof(short), objs->objs[ i ].n_verts_index );
		WriteData( write_buffer, size_count, objs->objs[ i ].verts_index, size );
		size_count += size ;
	}

	/* Common normal index write */
	for ( i = 0 ; i < objs->n_objs ; i++ ){
		write_objs->objs[ i ].norms_index = (void*)size_count ;
		size = AlignSize16( sizeof(short), objs->objs[ i ].n_norms_index );
		WriteData( write_buffer, size_count, objs->objs[ i ].norms_index, size );
		size_count += size ;
	}

	/* Common UV index write */
	for ( i = 0 ; i < objs->n_objs ; i++ ){
		write_objs->objs[ i ].uvs_index = (void*)size_count ;
		size = AlignSize16( sizeof(short), objs->objs[ i ].n_uvs_index );
		WriteData( write_buffer, size_count, objs->objs[ i ].uvs_index, size );
		size_count += size ;
	}

	fwrite( write_buffer, 1, size_count, fp );

	fclose( fp );
	free( write_buffer );
}

/* ================================================================ */
/*
	Main Routine
*/
int Convert_Kms_To_CommonVertexTabelFile( char *input_filename )
{
	char	output_filename[ 1024 ] ;
	KMS_DEF	*def ;
	COMMON_V_OBJS	*objs ;
	COMMON_V_OBJ	*obj ;
	int		i ;

	if ( ( def = OpenKms( input_filename ) ) == NULL ){
		return ( -1 );
	}

	objs = Malloc0( sizeof(COMMON_V_OBJS) + sizeof(COMMON_V_OBJ) * def->n_x_models );
	objs->id = 6754556 ; /* StrCode2("Common Vertex Data") */
	objs->n_objs = def->n_x_models ;
	objs->flag = COMMON_VERTS | COMMON_NORMS | COMMON_UVS ;

	printf("create common vertex data...\n");
	for ( i = 0 ; i < def->n_x_models ; i++ ){
		//printf("obj %d (%d)\n", i, def->models[ i ].n_packs );
		CreateCommonVertex( &def->models[ i ], &objs->objs[ i ] );
		CreateCommonNormal( &def->models[ i ], &objs->objs[ i ] );
		CreateCommonUV( &def->models[ i ], &objs->objs[ i ] );
	}

	MakeFileName( output_filename, input_filename,  input_filename, "cvd" );

	SaveCvd( output_filename, objs );

	obj = objs->objs ;
	for ( i = 0 ; i < def->n_x_models ; obj++, i++ ){
		Free( obj->verts );
		Free( obj->norms );
		Free( obj->uvs );
		Free( obj->verts_index );
		Free( obj->norms_index );
		Free( obj->uvs_index );
	}

	Free( objs );
	free( def );
}



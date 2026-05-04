/*
	conv.c
	

	2001/06/17 K.Takabe
	$Id: conv.c,v 1.3 2002/04/08 06:03:29 usr02774 Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "prog_utl.h"
#include "fmt_kms.h"

/* ---------------------------------------------------------------- */
#define MAX_KMS		(256)
/* ---------------------------------------------------------------- */
#define ZAR_FORMAT_KMS		(113171)	/*"kms"*/
typedef struct {
	int			data_offset ;
	int			data_id ;
	int			pad[ 2 ] ;
} ZAR_LIST ;
typedef struct {
	int				format_type ;	/* 113171("kms")  */
	int				version ;		/* 0 */
	int				type ;			/* 0 */
	int				n_datas ;
	ZAR_LIST	list[ 0 ];
} ZAR_HEADER ;


typedef struct {
	KMS_DEF			*def ;
	KMS_MDLPACK		*mdlpack ;
	int				total_packs ;
	int				data_id ;
} KMS_DATA ;

#define VERT_SIZE	(8)
#define NORM_SIZE	(8)
#define UV_SIZE	(4)
#define INDEX_SIZE	(2)
/* ---------------------------------------------------------------- */
/*
	グローバルワーク
 */
int	DebugMode ;
/* 各種頂点データ用スタック */
int			verts_offset ;
int			norms_offset ;
int			uvs_offset ;
int			index_offset ;
char		verts_buffer[ 1024 * 1024 ];
char		norms_buffer[ 1024 * 1024 ];
char		uvs_buffer[ 3 * 1024 * 1024 ];		/* UVは共有させない */
char		index_buffer[ 1024 * 1024 ];
int			v_hit_buffer_offset ;
int			n_hit_buffer_offset ;
int			i_hit_buffer_offset ;
char		v_hit_buffer[ 2048 ] ;
char		n_hit_buffer[ 2048 ] ;
char		i_hit_buffer[ 2048 ] ;

int			total_data_size ;
int			n_kms_datas ;
KMS_DATA		kms_data[ MAX_KMS ];
/* ---------------------------------------------------------------- */
#define BIT_LEN		24
unsigned int MGS_GetStrCode2( char *input )
{
	unsigned char c;
	unsigned char *p;
	unsigned int id, mask;
	char	filename[256], *string ;

	MakeFileName( filename, NULL, input, NULL );
	string = filename ;

	p = ( unsigned char * )string;
	id = 0;
	mask = ( 1 << BIT_LEN ) - 1;

	while( ( c = *( p++ ) ) != '\0' ){
		if ( c == ':' ) continue ;
		if ( c == '\\' ) continue ;
		if ( c == '/' ) continue ;
		if ( c == '.' ) break ;
		id = ( id << 5 ) | ( id >> (BIT_LEN-5) );
		id += c ;
		id &= mask;
	}
	if( id == 0 ) id = 1;

	return id;
}
/* ---------------------------------------------------------------- */
/* 頂点をバッファにストアし、その格納オフセット位置を返す（重複チェックつき） */
static int StoreVertsData( short *verts, int n )
{
	int		i, j, size, oft ;
	KMS_DATA	*kms ;
	KMS_MDLPACK	*mdlpack ;
	size = ( ( n + 1 ) & ~1 ) * VERT_SIZE ;
	for ( i = 0 ; i < n_kms_datas ; i++ ){
		kms = &kms_data[ i ];
		mdlpack = kms->mdlpack ;
		for ( j = 0 ; j < kms->total_packs ; j++ ){
			if ( mdlpack->n_verts == n ){
				oft = (int)mdlpack->verts ;
				if ( memcmp( verts, &verts_buffer[ oft ], size ) == 0 ){
					if ( DebugMode & 1 ) v_hit_buffer[ v_hit_buffer_offset ] = '.' ;
					return ( oft );
				}
			}
			mdlpack++ ;
		}
	}
	if ( DebugMode & 1 ) v_hit_buffer[ v_hit_buffer_offset ] = 'O' ;
	oft = verts_offset ;
	memcpy( &verts_buffer[ verts_offset ], verts, size );
	verts_offset += size ;
	return ( oft );
}
/* 法線をバッファにストアし、その格納オフセット位置を返す（重複チェックつき） */
static int StoreNormsData( short *norms, int n )
{
	int		i, j, size, oft ;
	KMS_DATA	*kms ;
	KMS_MDLPACK	*mdlpack ;
	size = ( ( n + 1 ) & ~1 ) * NORM_SIZE ;
	for ( i = 0 ; i < n_kms_datas ; i++ ){
		kms = &kms_data[ i ];
		mdlpack = kms->mdlpack ;
		for ( j = 0 ; j < kms->total_packs ; j++ ){
			if ( mdlpack->n_verts == n ){
				oft = (int)mdlpack->norms ;
				if ( memcmp( norms, &norms_buffer[ oft ], size ) == 0 ){
					if ( DebugMode & 1 ) n_hit_buffer[ n_hit_buffer_offset ] = '.' ;
					return ( oft );
				}
			}
			mdlpack++ ;
		}
	}
	if ( DebugMode & 1 ) n_hit_buffer[ n_hit_buffer_offset ] = 'O' ;
	oft = norms_offset ;
	memcpy( &norms_buffer[ norms_offset ], norms, size );
	norms_offset += size ;
	return ( oft );
}
/* UVをバッファにストアし、その格納オフセット位置を返す（重複チェックなし） */
static int StoreUvsData( short *uvs, int n )
{
	int		i, j, size, oft ;
	KMS_DATA	*kms ;
	KMS_MDLPACK	*mdlpack ;
	size = ( ( n + 3 ) & ~3 ) * UV_SIZE ;
	oft = uvs_offset ;
	memcpy( &uvs_buffer[ uvs_offset ], uvs, size );
	uvs_offset += size ;
	return ( oft );
}
/* インデックスをバッファにストアし、その格納オフセット位置を返す（重複チェックつき） */
static int StoreIndexData( short *index, int n )
{
	int		i, j, size, oft ;
	KMS_DATA	*kms ;
	KMS_MDLPACK	*mdlpack ;
	size = n * INDEX_SIZE ;
	for ( i = 0 ; i < n_kms_datas ; i++ ){
		kms = &kms_data[ i ];
		mdlpack = kms->mdlpack ;
		for ( j = 0 ; j < kms->total_packs ; j++ ){
			if ( mdlpack->n_indices == n ){
				oft = (int)mdlpack->index ;
				if ( memcmp( index, &index_buffer[ oft ], size ) == 0 ){
					if ( DebugMode & 1 ) i_hit_buffer[ i_hit_buffer_offset ] = '.' ;
					return ( oft );
				}
			}
			mdlpack++ ;
		}
	}
	if ( DebugMode & 1 ) i_hit_buffer[ i_hit_buffer_offset ] = 'O' ;
	oft = index_offset ;
	memcpy( &index_buffer[ index_offset ], index, size );
	index_offset += size ;
	return ( oft );
}
/* ---------------------------------------------------------------- */
int InitArchiveData( void )
{
	int		i ;
	for ( i = 0 ; i < MAX_KMS ; i++ ){
		if ( kms_data[ i ].def != NULL ){
			Free( kms_data[ i ].def );
			kms_data[ i ].def = NULL ;
		}
		if ( kms_data[ i ].mdlpack != NULL ){
			Free( kms_data[ i ].mdlpack );
			kms_data[ i ].mdlpack = NULL ;
		}
	}
	n_kms_datas = 0 ;
	verts_offset = 0 ;
	norms_offset = 0 ;
	uvs_offset = 0 ;
	index_offset = 0 ;
	total_data_size = 0 ;
}

/* ---------------------------------------------------------------- */
int AddKmsData( char *filename )
{
	FILE		*fp ;
	KMS_DATA	*kms ;
	KMS_DEF		*def, def_tmp ;
	KMS_MDL		*mdl ;
	KMS_MDLPACK	*mdlpack ;
	int			i, j, size, max_mdlpacks ;
	short		load_buffer[ 4096 ];

	if ( ( fp = fopen( filename, "rb" ) ) == NULL ){
		printf("file open error!!(%s)\n", filename );
		return ( -1 );
	}
	fseek( fp, 0, SEEK_END );
	total_data_size += ftell( fp );
	fseek( fp, 0, SEEK_SET );

	kms = &kms_data[ n_kms_datas ];
	/* ヘッダ及び各ユニット配列読み込み */
	fread( &def_tmp, sizeof(KMS_DEF), 1, fp );
	def = Malloc0( sizeof(KMS_DEF) + sizeof(KMS_MDL) * def_tmp.n_x_models );
	*def = def_tmp ;
	kms->def = def ;
	kms->data_id = MGS_GetStrCode2( filename );
	mdl = def->models ;
	fread( mdl, sizeof(KMS_MDL), def->n_x_models, fp );

	{/* モデルパケット読み込み */
		int		n_packs ;
		n_packs = 0 ;
		for ( i = 0 ; i < def->n_x_models ; i++ ){
			n_packs += mdl[ i ].n_packs ;
		}
		mdlpack = Malloc0( sizeof(KMS_MDLPACK) * n_packs );
		kms->mdlpack = mdlpack ;
		kms->total_packs = n_packs ;

		/* とりあえず行儀良く読み込んでみる */
		n_packs = 0 ;
		for ( i = 0 ; i < def->n_x_models ; i++ ){
			fseek( fp, (int)mdl[ i ].packs, SEEK_SET );	/* オフセット位置から正確に読む */
			fread( &mdlpack[ n_packs ], sizeof(KMS_MDLPACK), mdl[ i ].n_packs, fp );
			n_packs += mdl[ i ].n_packs ;
		}
	}

	/* 頂点／法線データ読み込み */
	v_hit_buffer_offset = 0 ;
	n_hit_buffer_offset = 0 ;
	i_hit_buffer_offset = 0 ;
	for ( i = 0 ; i < kms->total_packs ; i++ ){
		size = ( mdlpack[ i ].n_verts + 1 ) & ~1 ;

		if ( mdlpack[ i ].verts != NULL ){
			fseek( fp, (int)mdlpack[ i ].verts, SEEK_SET );
			fread( load_buffer, VERT_SIZE, size, fp );
			mdlpack[ i ].verts = (void*)StoreVertsData( load_buffer, mdlpack[i].n_verts );
		} else {
			mdlpack[ i ].verts = (void*)-1 ;
			if ( DebugMode & 1 ) v_hit_buffer[ v_hit_buffer_offset ] = '#' ;
		}

		if ( mdlpack[ i ].norms != NULL ){
			fseek( fp, (int)mdlpack[ i ].norms, SEEK_SET );
			fread( load_buffer, NORM_SIZE, size, fp );
			mdlpack[ i ].norms = (void*)StoreNormsData( load_buffer, mdlpack[i].n_verts );
		} else {
			mdlpack[ i ].verts = (void*)-1 ;
			if ( DebugMode & 1 ) n_hit_buffer[ n_hit_buffer_offset ] = '#' ;
		}

		size = mdlpack[ i ].n_indices ;	/* インデックスはアラインメントに揃える必要なし */
		if ( ( mdlpack[ i ].index != NULL ) && ( def->data_format & MGS_MODEL_FLAG_INDEX ) ){
			fseek( fp, (int)mdlpack[ i ].index, SEEK_SET );
			fread( load_buffer, INDEX_SIZE, size, fp );
			mdlpack[ i ].index = (void*)StoreIndexData( load_buffer, mdlpack[i].n_indices );
		} else {
			mdlpack[ i ].index = (void*)-1 ;
			if ( DebugMode & 1 ) i_hit_buffer[ i_hit_buffer_offset ] = '#' ;
		}

		v_hit_buffer_offset++ ;
		n_hit_buffer_offset++ ;
		i_hit_buffer_offset++ ;
	}
	if ( DebugMode & 1 ){
		v_hit_buffer[ v_hit_buffer_offset ] = '\0' ;
		n_hit_buffer[ n_hit_buffer_offset ] = '\0' ;
		i_hit_buffer[ i_hit_buffer_offset ] = '\0' ;
		printf("v = %s\n", v_hit_buffer );
		printf("n = %s\n", n_hit_buffer );
		printf("i = %s\n", i_hit_buffer );
	}

	/* UVデータ読み込み */
	for ( j = 0 ; j < 3 ; j++ ){
		for ( i = 0 ; i < kms->total_packs ; i++ ){
			if ( mdlpack[ i ].uvs[j] == NULL ){
				mdlpack[ i ].uvs[j] = -1 ;
				continue ;
			}
			fseek( fp, (int)mdlpack[ i ].uvs[j], SEEK_SET );
			size = ( mdlpack[ i ].n_verts + 3 ) & ~3 ;
			fread( load_buffer, UV_SIZE, size, fp );
			mdlpack[ i ].uvs[j] = (void*)StoreUvsData( load_buffer, mdlpack[i].n_verts );
		}
	}

	n_kms_datas++ ;

	return ( 0 );
}

int WriteZmsData( char *outfilename )
{
	FILE		*fp ;
	int			i, j, size, header_size, archive_size ;
	int			offset, v_top, n_top, u_top, i_top ;
	ZAR_HEADER	*header ;
	KMS_DATA	*kms ;

	if ( ( fp = fopen( outfilename, "wb" ) ) == NULL ){
		printf("file open error!(%s)\n", outfilename );
		return ( -1 );
	}

	header_size = sizeof(ZAR_HEADER) + sizeof(ZAR_LIST) * n_kms_datas ;
	header = Malloc0( header_size );
	memset( header, 0, header_size );

	/* ヘッダーの書きだし */
	header->format_type = ZAR_FORMAT_KMS ;
	header->version = 0 ;
	header->type = 0 ;
	header->n_datas = n_kms_datas ;
	offset = header_size ;
	for ( i = 0 ; i < n_kms_datas ; i++ ){
		header->list[ i ].data_offset = offset ;
		header->list[ i ].data_id = kms_data[ i ].data_id ;
		offset += sizeof(KMS_DEF) + sizeof(KMS_MDL) * kms_data[ i ].def->n_x_models ;
		offset += sizeof(KMS_MDLPACK) * kms_data[ i ].total_packs ;
	}
	fwrite( header, header_size, 1, fp );

	v_top = offset ;
	n_top = v_top + verts_offset ;
	u_top = n_top + norms_offset ;
	i_top = u_top + uvs_offset ;
	archive_size = i_top + index_offset ;

	offset = header_size ;
	/* 各KMSデータの書きだし */
	for ( i = 0 ; i < n_kms_datas ; i++ ){
		KMS_DEF		*def ;
		KMS_MDLPACK	*mdlpack ;
		kms = &kms_data[ i ];

		def = kms->def ;
		offset += sizeof(KMS_DEF) + sizeof(KMS_MDL) * def->n_x_models ;
		for ( j = 0 ; j < def->n_x_models ; j++ ){
			def->models[ j ].packs = (void*)offset ;
			offset += sizeof(KMS_MDLPACK) * def->models[ j ].n_packs ;
		}
		fwrite( def, sizeof(KMS_DEF) + sizeof(KMS_MDL) * def->n_x_models, 1, fp );

		mdlpack = kms->mdlpack ;
		for ( j = 0 ; j < kms->total_packs ; j++ ){
			if ( mdlpack->verts != -1 ) mdlpack->verts = (void*)( v_top + (int)mdlpack->verts );
			else mdlpack->verts = NULL ;
			if ( mdlpack->norms != -1 ) mdlpack->norms = (void*)( n_top + (int)mdlpack->norms );
			else mdlpack->norms = NULL ;
			if ( mdlpack->uvs[0] != -1 ) mdlpack->uvs[0] = (void*)( u_top + (int)mdlpack->uvs[0] );
			else mdlpack->uvs[0] = NULL ;
			if ( mdlpack->uvs[1] != -1 ) mdlpack->uvs[1] = (void*)( u_top + (int)mdlpack->uvs[1] );
			else mdlpack->uvs[1] = NULL ;
			if ( mdlpack->uvs[2] != -1 ) mdlpack->uvs[2] = (void*)( u_top + (int)mdlpack->uvs[2] );
			else mdlpack->uvs[2] = NULL ;
			if ( mdlpack->index != -1 ) mdlpack->index = (void*)( i_top + (int)mdlpack->index );
			else mdlpack->index = NULL ;
			mdlpack++ ;
		}
		fwrite( kms->mdlpack, sizeof(KMS_MDLPACK), kms->total_packs, fp );
	}

	fwrite( verts_buffer, 1, verts_offset, fp );
	fwrite( norms_buffer, 1, norms_offset, fp );
	fwrite( uvs_buffer, 1, uvs_offset, fp );
	fwrite( index_buffer, 1, index_offset, fp );

	Free( header );

	printf("archive_size %d / total_size %d = %f \n", archive_size, total_data_size, (float)archive_size / total_data_size );

	return ( 0 );
}

/* ---------------------------------------------------------------- */
#define ADJUST_POINTER( _oft, _top )	( ((void*)_oft!=NULL) ? ((int)_oft+(int)_top) : NULL )

static Analyze( void *mem )
{
	ZAR_HEADER	*header ;
	KMS_DEF		*def ;
	KMS_MDL		*mdl ;
	KMS_MDLPACK	*mdlpack ;
	int			i, j, k ;

	header = mem ;

	for ( i = 0 ; i < header->n_datas ; i++ ){
		def = (KMS_DEF*)ADJUST_POINTER( header->list[ i ].data_offset, header );
		mdl = def->models ;
		printf("data no.%d : %d  n_x_models = %d\n", i, header->list[ i ].data_id, def->n_x_models );
		for ( j = 0 ; j < def->n_x_models ; j++, mdl++ ){
			mdl->packs = (void*)ADJUST_POINTER( mdl->packs, header );
			mdlpack = mdl->packs ;
			printf("%d:%d packs %08x\n", j, mdl->n_packs, mdlpack );
			for ( k = 0 ; k < mdl->n_packs ; k++, mdlpack++ ){
				if ( !( def->data_format & MGS_MODEL_FLAG_INDEX ) ){
					printf("%d %08x %08x %08x %08x %08x\n", k,
						   mdlpack->verts, mdlpack->norms, mdlpack->uvs[0], mdlpack->uvs[1], mdlpack->uvs[2] );
				} else {
					printf("%d %08x %08x %08x %08x %08x %08x\n", k,
						   mdlpack->verts, mdlpack->norms,
						   mdlpack->uvs[0], mdlpack->uvs[1], mdlpack->uvs[2],
						   mdlpack->index );
				}
				mdlpack->verts = (void*)ADJUST_POINTER( mdlpack->verts, header );
				mdlpack->norms = (void*)ADJUST_POINTER( mdlpack->norms, header );
				mdlpack->uvs[0] = (void*)ADJUST_POINTER( mdlpack->uvs[0], header );
				mdlpack->uvs[1] = (void*)ADJUST_POINTER( mdlpack->uvs[1], header );
				mdlpack->uvs[2] = (void*)ADJUST_POINTER( mdlpack->uvs[2], header );
				mdlpack->index = (void*)ADJUST_POINTER( mdlpack->index, header );
			}
		}
	}
}

int DebugZms( char *filename )
{
	FILE		*fp ;
	int			size ;
	char		*mem ;

	if ( ( fp = fopen( filename, "rb" ) ) == NULL ){
		printf("file open error!!(%s)\n", filename );
	}

	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	printf("data size = %d \n", size );
	mem = Malloc0( size );
	fread( mem, size, 1, fp );

	Analyze( mem );

	Free( mem );
}

void SetDebugMode( int mode )
{
	DebugMode = mode ;
}


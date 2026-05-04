/*



 */

#include <stdio.h>
#include <stdlib.h>

#include "fmt_kms.h"

/* ---------------------------------------------------------------- */
static char		VertexBuffer[ 2 * 1024 * 1024 ];
static int		MaxVertexSize ;
static u_short	IndexBuffer[ 1 * 1024 * 1024 ];
static u_short	IndexConvertTable[ 32768 ];

/* ---------------------------------------------------------------- */
static void KmsDataSetup( KMS_DEF *def )
{
	KMS_MDL		*mdl ;
	KMS_MDLPACK	*pack ;
	int			i, j, n_models, tri_code ;

	mdl = def->models ;
	for ( i = def->n_x_models ; i > 0 ; i--, mdl++ ){
		mdl->packs = (KMS_MDLPACK*)( (int)mdl->packs + (int)def );
		pack = mdl->packs ;
		for ( j = mdl->n_packs ; j > 0 ; j--, pack++ ){
			pack->verts = (short*)( (int)pack->verts + (int)def );
			pack->norms = (short*)( (int)pack->norms + (int)def );
			if ( pack->uvs[0] != 0 && pack->flag & (DG_PACKFLAG_UV0))
				pack->uvs[0] = (short*)( (int)pack->uvs[0] + (int)def );
			if ( pack->uvs[1] != 0 && pack->flag & (DG_PACKFLAG_UV1|DG_PACKFLAG_BMAP))
				pack->uvs[1] = (short*)( (int)pack->uvs[1] + (int)def );
			if ( pack->uvs[2] != 0 && pack->flag & (DG_PACKFLAG_UV2|DG_PACKFLAG_BMAP|DG_PACKFLAG_SMAP))
				pack->uvs[2] = (short*)( (int)pack->uvs[2] + (int)def );
			if ( pack->rgbs != 0 ) pack->rgbs = (short*)( (int)pack->rgbs + (int)def );	/* 未使用なので */
			if ( pack->index != 0 ) pack->index = (short*)( (int)pack->index + (int)def );	/* 未使用なので */
		}
	}
}

/* ---------------------------------------------------------------- */
int RegistVertexSingle( KMS_VERTEX_SINGLE *top, int *n_verts, KMS_VERTEX_SINGLE *v )
{
	int		i, n ;
	n = *n_verts ;
	for ( i = 0 ; i < n ; i++, top++ ){
		if ( top->vx != v->vx ) continue ;
		if ( top->vy != v->vy ) continue ;
		if ( top->vz != v->vz ) continue ;
		if ( top->nx != v->nx ) continue ;
		if ( top->ny != v->ny ) continue ;
		if ( top->nz != v->nz ) continue ;
		if ( top->u0 != v->u0 ) continue ;
		if ( top->v0 != v->v0 ) continue ;
		return ( i ) ;
	}
	*top = *v ;
	*n_verts = n + 1 ;
	return ( n );
}
int RegistVertexMulti( KMS_VERTEX_MULTI *top, int *n_verts, KMS_VERTEX_MULTI *v )
{
	int		i, n ;
	n = *n_verts ;
	for ( i = 0 ; i < n ; i++, top++ ){
		if ( top->vx != v->vx ) continue ;
		if ( top->vy != v->vy ) continue ;
		if ( top->vz != v->vz ) continue ;
		if ( top->nx != v->nx ) continue ;
		if ( top->ny != v->ny ) continue ;
		if ( top->nz != v->nz ) continue ;
		if ( top->u0 != v->u0 ) continue ;
		if ( top->v0 != v->v0 ) continue ;
		if ( top->u1 != v->u1 ) continue ;
		if ( top->v1 != v->v1 ) continue ;
		if ( top->u2 != v->u2 ) continue ;
		if ( top->v2 != v->v2 ) continue ;
		return ( i ) ;
	}
	*top = *v ;
	*n_verts = n + 1 ;
	return ( n );
}
/* ---------------------------------------------------------------- */
void PackVertexDataSingle( KMS_DEF *def )
{
	KMS_MDL		*mdl ;
	KMS_MDLPACK	*pack ;
	int			i, j, k ;
	KMS_VERTEX_SINGLE	*vertex, v ;
	int					n_verts = 0, v_count ;
	short				*verts, *norms, *uvs0, *uvs1, *uvs2, *index ;

	/* オブジェクト単位で処理 */
	MaxVertexSize = 0 ;
	mdl = def->models ;
	for ( i = 0 ; i < def->n_x_models ; i++, mdl++ ){
		vertex = (void*)&VertexBuffer[ MaxVertexSize ];
		/* 共有頂点データ初期化 */
		mdl->vbuff = vertex ;
		n_verts = 0 ;
		v_count = 0 ;
		/* 頂点の共有化処理 */
		pack = mdl->packs ;
		for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
			verts = pack->verts ;
			norms = pack->norms ;
			uvs0 = pack->uvs[0] ;
			/* 共有頂点データへの登録 */
			for ( k = 0 ; k < pack->n_verts ; k++ ){
				memset( &v, 0, sizeof(v) );
				if ( verts != NULL ){
					v.vx = verts[0] ;
					v.vy = verts[1] ;
					v.vz = verts[2] ;
					v.wt = verts[3] * 32767 / 4096 ;
					verts += 4 ;
				}
				if ( norms != NULL ){
					v.nx = norms[0] * 32767 / 4096 ;
					v.ny = norms[1] * 32767 / 4096 ;
					v.nz = norms[2] * 32767 / 4096 ;
					v.f = norms[3] ;
					norms += 4 ;
				}
				if ( ( uvs0 != NULL ) && ( pack->tex_id[0] != 0 ) ){
					v.u0 = uvs0[0] * 32767 / 4096 ;
					v.v0 = uvs0[1] * 32767 / 4096 ;
					uvs0 += 2 ;
				}
				IndexConvertTable[ v_count ] = RegistVertexSingle( mdl->vbuff, &n_verts, &v );
				v_count++ ;
			}
		}
		mdl->stride = sizeof(v) ;
		mdl->n_verts = n_verts ;
		/* インデックスの変換処理 */
		pack = mdl->packs ;
		for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
			index = pack->index ;
			for ( k = 0 ; k < pack->n_indices ; k++, index++ ){
				*index = IndexConvertTable[ *index ] ;
			}
		}
		printf("v_count %4d , n_verts %4d  (%f)\n", v_count, n_verts, (float)n_verts / (float)v_count );
		MaxVertexSize += ( n_verts * sizeof(KMS_VERTEX_SINGLE) + 15 ) & ~15 ;
	}
}

void PackVertexDataMulti( KMS_DEF *def )
{
	KMS_MDL		*mdl ;
	KMS_MDLPACK	*pack ;
	int			i, j, k ;
	KMS_VERTEX_MULTI	*vertex, v ;
	int					n_verts = 0, v_count ;
	short				*verts, *norms, *uvs0, *uvs1, *uvs2, *index ;

	/* オブジェクト単位で処理 */
	MaxVertexSize = 0 ;
	mdl = def->models ;
	for ( i = 0 ; i < def->n_x_models ; i++, mdl++ ){
		vertex = (void*)&VertexBuffer[ MaxVertexSize ];
		/* 共有頂点データ初期化 */
		mdl->vbuff = vertex ;
		n_verts = 0 ;
		v_count = 0 ;
		/* 頂点の共有化処理 */
		pack = mdl->packs ;
		for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
			verts = pack->verts ;
			norms = pack->norms ;
			uvs0 = pack->uvs[0] ;
			uvs1 = pack->uvs[1] ;
			uvs2 = pack->uvs[2] ;
			/* 共有頂点データへの登録 */
			for ( k = 0 ; k < pack->n_verts ; k++ ){
				memset( &v, 0, sizeof(v) );
				if ( verts != NULL ){
					v.vx = verts[0] ;
					v.vy = verts[1] ;
					v.vz = verts[2] ;
					v.wt = verts[3] * 32767 / 4096 ;
					verts += 4 ;
				}
				if ( norms != NULL ){
					v.nx = norms[0] * 32767 / 4096 ;
					v.ny = norms[1] * 32767 / 4096 ;
					v.nz = norms[2] * 32767 / 4096 ;
					v.f = norms[3] ;
					norms += 4 ;
				}
				if ( ( uvs0 != NULL ) && ( pack->tex_id[0] != 0 ) ){
					v.u0 = uvs0[0] * 32767 / 4096 ;
					v.v0 = uvs0[1] * 32767 / 4096 ;
					uvs0 += 2 ;
				}
				if ( ( uvs1 != NULL ) && ( pack->tex_id[1] != 0 ) ){
					v.u1 = uvs1[0] * 32767 / 4096 ;
					v.v1 = uvs1[1] * 32767 / 4096 ;
					uvs1 += 2 ;
				}
				if ( ( uvs2 != NULL ) && ( pack->tex_id[2] != 0 ) ){
					v.u2 = uvs2[0] * 32767 / 4096 ;
					v.v2 = uvs2[1] * 32767 / 4096 ;
					uvs2 += 2 ;
				}
				IndexConvertTable[ v_count ] = RegistVertexMulti( mdl->vbuff, &n_verts, &v );
				v_count++ ;
			}
		}
		mdl->stride = sizeof(v) ;
		mdl->n_verts = n_verts ;
		/* インデックスの変換処理 */
		pack = mdl->packs ;
		for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
			index = pack->index ;
			for ( k = 0 ; k < pack->n_indices ; k++, index++ ){
				*index = IndexConvertTable[ *index ] ;
			}
		}
		printf("v_count %4d , n_verts %4d  (%f)\n", v_count, n_verts, (float)n_verts / (float)v_count );
		MaxVertexSize += ( n_verts * sizeof(KMS_VERTEX_MULTI) + 15 ) & ~15 ;
	}
}

void WriteKmy( KMS_DEF *def, KMS_DEF *backup_def, FILE *fp )
{
	KMS_MDL		*mdl ;
	KMS_MDLPACK	*pack ;
	int			i, j, offset, index_offset, pack_offset ;
	int			header_size, packs_size ;

	/* データのオフセット計算 */
	offset = 0 ;
	header_size = sizeof(KMS_DEF) + sizeof(KMS_MDL) * def->n_x_models ;
	packs_size = 0 ;
	mdl = def->models ;
	for ( i = 0 ; i < def->n_x_models ; i++, mdl++ ){
		packs_size += sizeof(KMS_MDLPACK) * mdl->n_packs ;
	}

	index_offset = 0 ;
	pack_offset = 0 ;
	mdl = def->models ;
	for ( i = 0 ; i < def->n_x_models ; i++, mdl++ ){
		/* 各MDLPACKのポインタ補正 */
		pack = mdl->packs ;
		for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
			pack->n_verts = 0 ;
			pack->verts = NULL ;
			pack->norms = NULL ;
			pack->uvs[0] = NULL ;
			pack->uvs[1] = NULL ;
			pack->uvs[2] = NULL ;
			pack->index = (void*)( header_size + packs_size + MaxVertexSize + index_offset ) ;
			index_offset += pack->n_indices * sizeof(short) ;
		}
		/* MDLのポインタ補正 */
		//mdl->packs = (void*)( header_size + pack_offset );
		mdl->packs = (void*)( (int)mdl->packs - (int)def );
		mdl->vbuff = (void*)( header_size + packs_size + (int)mdl->vbuff - (int)VertexBuffer );

		pack_offset += mdl->n_packs ;
	}
	/* ヘッダ部の書きだし */
	fwrite( def, header_size + packs_size, 1, fp );
	/* 頂点データの書きだし */
	fwrite( VertexBuffer, MaxVertexSize, 1, fp );
	/* インデックスデータの書きだし */
	mdl = backup_def->models ;
	for ( i = 0 ; i < backup_def->n_x_models ; i++, mdl++ ){
		/* ポインタをバックアップ用構造体から取得 */
		pack = (void*)( (int)mdl->packs - (int)def + (int)backup_def )   ;
		for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
			fwrite( pack->index, sizeof(short), pack->n_indices, fp );
		}
	}
}
/* ---------------------------------------------------------------- */
int Convert_KmxToKmy( char *in_file, char *out_file )
{
	FILE		*fp ;
	KMS_DEF		*def, *backup_def ;
	KMS_MDL		*mdl ;
	KMS_MDLPACK	*pack ;
	int			size, file_size ;
	void		*load_mem ;
	int			i, j, type ;

	if ( ( fp = fopen( in_file, "rb" ) ) == NULL ){
		printf("file open error!(%s)\n", in_file );
		return ( -1 );
	}
	fseek( fp, 0, SEEK_END );
	file_size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	load_mem = malloc( file_size );
	fread( load_mem, file_size, 1, fp );
	fclose( fp );

	def = load_mem ;
	/* フォーマットチェック */
	if ( DG_GetMdlFlag( def ) == ( MGS_MODEL_FLAG_INDEX|MGS_MODEL_FLAG_NEWXVER) ){
		printf("no support format(%s)\n", in_file );
		free( load_mem );
		return ( 0 );
	}

	KmsDataSetup( def );

	if ( DG_GetMdlFormat( def ) == MGS_MODEL_NORM ){
		type = 0 ;
	} else if ( DG_GetMdlFormat( def ) == MGS_MODEL_MULTITEX ){
		type = 1 ;
	} else {
		printf("no support type (%s)\n", in_file );
		free( load_mem );
		return ( 0 );
	}

	/* 頂点共有化 */
	if ( type == 0 ){
		PackVertexDataSingle( def );
	} else {
		PackVertexDataMulti( def );
	}

	/* 新フォーマットにて書きだし */
	if ( ( fp = fopen( out_file, "wb" ) ) == NULL ){
		printf("file open error!(%s)\n", out_file );
		free( load_mem );
		return ( -1 );
	}

	backup_def = malloc( file_size );
	memcpy( backup_def, def, file_size );
	/* 各種ポインタ情報の修正処理 */
	WriteKmy( def, backup_def, fp );

	fclose( fp );

	free( backup_def );
	free( load_mem );

}

int DumpKmy( char *in_file )
{
	FILE		*fp ;
	KMS_DEF		*def, *backup_def ;
	KMS_MDL		*mdl ;
	KMS_MDLPACK	*pack ;
	int			size, file_size ;
	void		*load_mem ;
	int			i, j, type ;

	if ( ( fp = fopen( in_file, "rb" ) ) == NULL ){
		printf("file open error!(%s)\n", in_file );
		return ( -1 );
	}
	fseek( fp, 0, SEEK_END );
	file_size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	load_mem = malloc( file_size );
	fread( load_mem, file_size, 1, fp );
	fclose( fp );

	def = load_mem ;

	KmsDataSetup( def );

	if ( DG_GetMdlFormat( def ) == MGS_MODEL_NORM ){
		type = 0 ;
	} else if ( DG_GetMdlFormat( def ) == MGS_MODEL_MULTITEX ){
		type = 1 ;
	} else {
		printf("no support type (%s)\n", in_file );
		free( load_mem );
		return ( 0 );
	}

	printf("format %d\n", def->data_format );
	printf("n_models %d\n", def->n_models );
	printf("n_x_models %d\n", def->n_x_models );
	printf("texture %d\n", def->texture );
	mdl = def->models ;
	for ( i = 0 ; i < def->n_x_models ; i++, mdl++ ){
		printf("  type %d\n", mdl->type );
		printf("  n_packs %d\n", mdl->n_packs );
		printf("  parent %d\n", mdl->parent );
		printf("  packs %p\n", (int)mdl->packs - (int)def );
		printf("  vbuff %p\n", (int)mdl->vbuff );
		printf("  stride %d\n", mdl->stride );
		printf("  n_verts %d\n", mdl->n_verts );
		pack = mdl->packs ;
		for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
			printf("    flag %08x\n", pack->flag );
			printf("    n_verts %d\n", pack->n_verts );
			printf("    n_indices %d\n", pack->n_indices );
			printf("    tex_id %d %d %d\n", pack->tex_id[0], pack->tex_id[1], pack->tex_id[2] );
			printf("    verts %p\n", (int)pack->verts - (int)def );
			printf("    norms %p\n", (int)pack->norms - (int)def );
			printf("    uvs[0] %p\n", (int)pack->uvs[0] - (int)def );
			printf("    uvs[1] %p\n", (int)pack->uvs[1] - (int)def );
			printf("    uvs[2] %p\n", (int)pack->uvs[2] - (int)def );
			printf("    index %p\n", (int)pack->index - (int)def );
		}
	}

	free( load_mem );

}




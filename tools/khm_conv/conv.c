/*



 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>



#include "fmt_kms.h"
#include "prog_utl.h"
#include "poly_utl.h"


/* ---------------------------------------------------------------- */
void KmsSetup( KMS_DEF *def )
{
	KMS_MDL		*mdl ;
	KMS_MDLPACK	*pack ;
	int			i, j, tri_code ;

	tri_code = -1 ;
	mdl = def->models ;
	for ( i = def->n_x_models ; i > 0 ; i--, mdl++ ){
		mdl->packs = (KMS_MDLPACK*)( (int)mdl->packs + (int)def );
		pack = mdl->packs ;
		for ( j = mdl->n_packs ; j > 0 ; j--, pack++ ){
			pack->verts = (short*)( (int)pack->verts + (int)def );
			pack->norms = (short*)( (int)pack->norms + (int)def );
			if ( pack->uvs[0] != 0 ) pack->uvs[0] = (short*)( (int)pack->uvs[0] + (int)def );
			if ( pack->uvs[1] != 0 ) pack->uvs[1] = (short*)( (int)pack->uvs[1] + (int)def );
			if ( pack->uvs[2] != 0 ) pack->uvs[2] = (short*)( (int)pack->uvs[2] + (int)def );
//printf("%08x %08x %08x %08x %08x \n", pack->verts, pack->norms, pack->uvs[0], pack->uvs[1], pack->uvs[2] );
			if ( pack->rgbs != 0 ) pack->rgbs = (CVECTOR*)( (int)pack->rgbs + (int)def );	/* 未使用なので */
		}
	}
	
}

#define ADJUST_POINTER( _oft_v, _top )	{ if ( (_oft_v) != NULL ) (_oft_v) = (void*)( (int)(_oft_v) + (int)(_top) ) ; }
void KhmSetup( KHM_HEADER *khm_header )
{
	int			i, j ;
	ADJUST_POINTER( khm_header->skeletons, khm_header );
	ADJUST_POINTER( khm_header->models, khm_header );
	for ( i = 0 ; i < khm_header->n_models ; i++ ){
		ADJUST_POINTER( khm_header->models[i].packet, khm_header );
		for ( j = 0 ; j < khm_header->models[i].n_packets ; j++ ){
			ADJUST_POINTER( khm_header->models[i].packet[j].verts, khm_header );
			{
				int		k ;
				KHM_VERT	*vert ;
				vert = khm_header->models[i].packet[j].verts ;
				for ( k = 0 ; k < khm_header->models[i].packet[j].n_verts ; k++, vert++ ){
#if 0
printf("loop %d %d %d\n", i, j, k );
printf("%d %d %d %d \n", vert->vx, vert->vy, vert->vz, vert->w );
printf("%d %d %d %04x \n", vert->nx, vert->ny, vert->nz, vert->flag & 0xffff );
printf("%d %d %d %d \n", vert->u0, vert->v0, vert->q0, vert->index );
printf("%d %d %d \n", vert->u1, vert->v1, vert->q1);
printf("%d %d %d \n", vert->u2, vert->v2, vert->q2);
#endif
				}
			}
		}
	}
}

/* ---------------------------------------------------------------- */
int Convert_Kms2_Khm( char *in_filename, char *out_filename )
{
	FILE			*fp ;
	KHM_HEADER		header ;
	KHM_SKEL		*skeletons ;
	KHM_MODEL		*models ;
	KHM_PACKET		*packets, *pack ;
	KHM_VERT		*vertices, *vert ;
	KMS_DEF			*kms_def ;
	KMS_MDL			*kms_mdl ;
	KMS_MDLPACK		*kms_mdlpack ;
	int				size, n_packs, n_verts, count ;
	int				i, j, k ;
	
	if ( ( fp = fopen( in_filename, "rb" ) ) == NULL ){
		printf("file open error!!(%s)\n", in_filename );
		return ( -1 );
	}
	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	kms_def = Malloc0( size );
	fread( kms_def, 1, size, fp );
	fclose( fp );

	/* ポインタの復元 */
	KmsSetup( kms_def );

	/* 各データサイズの算出 */
	n_packs = 0 ;
	for ( i = 0, kms_mdl = kms_def->models ; i < kms_def->n_x_models ; i++, kms_mdl++ ){
		n_packs += kms_mdl->n_packs ;
	}
	n_verts = 0 ;
	for ( i = 0, kms_mdl = kms_def->models ; i < kms_def->n_x_models ; i++, kms_mdl++ ){
		for ( j = 0, kms_mdlpack = kms_mdl->packs ; j < kms_mdl->n_packs ; j++, kms_mdlpack++ ){
			n_verts += ( ( kms_mdlpack->n_verts + 1 ) & ~1 ) ;
		}
	}
	printf("kms : total pack = %d, total verts = %d\n", n_packs, n_verts );

	skeletons = Malloc0( sizeof(KHM_SKEL) * kms_def->n_models );
	models = Malloc0( sizeof(KHM_MODEL) * kms_def->n_x_models );
	pack = packets = Malloc0( sizeof(KHM_PACKET) * n_packs );
	vert = vertices = Malloc0( sizeof(KHM_VERT) * n_verts );

	/* ヘッダー情報の構成 */
	header.format_type = 0 ;
	header.n_skeletons = kms_def->n_models ;
	header.n_models = kms_def->n_x_models ;
	header.texture = kms_def->texture ;
	header.bound_max.vx = kms_def->ux ;
	header.bound_max.vy = kms_def->uy ;
	header.bound_max.vz = kms_def->uz ;
	header.bound_max.vw = 1.0f ;
	header.bound_min.vx = kms_def->lx ;
	header.bound_min.vy = kms_def->ly ;
	header.bound_min.vz = kms_def->lz ;
	header.bound_min.vw = 1.0f ;
	header.skeletons = NULL ;
	header.models = NULL ;

	/* スケルトン情報の構成 */
	kms_mdl = kms_def->models ;
	for ( i = 0 ; i < kms_def->n_models ; i++, kms_mdl++ ){
		skeletons[i].id = 0 ;
		skeletons[i].flag = 0 ;
		skeletons[i].parent = kms_mdl->parent ;
		skeletons[i].trans.vx = kms_mdl->tx ;
		skeletons[i].trans.vy = kms_mdl->ty ;
		skeletons[i].trans.vz = kms_mdl->tz ;
		skeletons[i].trans.vw = 1.0f ;
		skeletons[i].bound_max.vx = kms_mdl->ux ;
		skeletons[i].bound_max.vy = kms_mdl->uy ;
		skeletons[i].bound_max.vz = kms_mdl->uz ;
		skeletons[i].bound_max.vw = 1.0f ;
		skeletons[i].bound_min.vx = kms_mdl->lx ;
		skeletons[i].bound_min.vy = kms_mdl->ly ;
		skeletons[i].bound_min.vz = kms_mdl->lz ;
		skeletons[i].bound_min.vw = 1.0f ;
		if ( skeletons->parent != -1 ){
			skeletons[i].root_trans.vx = skeletons[i].trans.vx + skeletons[ skeletons[i].parent ].root_trans.vx ;
			skeletons[i].root_trans.vy = skeletons[i].trans.vy + skeletons[ skeletons[i].parent ].root_trans.vy ;
			skeletons[i].root_trans.vz = skeletons[i].trans.vz + skeletons[ skeletons[i].parent ].root_trans.vz ;
			skeletons[i].root_trans.vw = 1.0f ;
		} else {
			skeletons[i].root_trans.vx = skeletons[i].trans.vx ;
			skeletons[i].root_trans.vy = skeletons[i].trans.vy ;
			skeletons[i].root_trans.vz = skeletons[i].trans.vz ;
			skeletons[i].root_trans.vw = 1.0f ;
		}
	}

	/* モデルデータの構成 */
	kms_mdl = kms_def->models ;
	for ( i = 0 ; i < kms_def->n_x_models ; i++, kms_mdl++ ){
		models[i].n_mats = 2 ;
		models[i].mats[0] = i ;
		models[i].mats[1] = skeletons[i].parent ;
		models[i].mats[2] = 0xff ;
		models[i].mats[3] = 0xff ;
		models[i].mats[4] = 0xff ;
		models[i].mats[5] = 0xff ;
		models[i].mats[6] = 0xff ;
		models[i].mats[7] = 0xff ;
		models[i].n_packets = kms_mdl->n_packs ;
		models[i].packet = pack ;
		models[i].bound_max.vx = kms_mdl->ux ;
		models[i].bound_max.vy = kms_mdl->uy ;
		models[i].bound_max.vz = kms_mdl->uz ;
		models[i].bound_max.vw = 1.0f ;
		models[i].bound_min.vx = kms_mdl->lx ;
		models[i].bound_min.vy = kms_mdl->ly ;
		models[i].bound_min.vz = kms_mdl->lz ;
		models[i].bound_min.vw = 1.0f ;

		/* パケット情報の構成 */
		for ( j = 0, kms_mdlpack = kms_mdl->packs ; j < kms_mdl->n_packs ; j++, kms_mdlpack++, pack++ ){
			pack->flag = kms_mdlpack->flag ;
			pack->v_size = sizeof(KHM_VERT) / ( sizeof(short) * 4 ) ;
			pack->n_verts = kms_mdlpack->n_verts ;
			pack->tex[0] = kms_mdlpack->tex_id[0] ;
			pack->tex[1] = kms_mdlpack->tex_id[1] ;
			pack->tex[2] = kms_mdlpack->tex_id[2] ;
			pack->verts = vert ;

			{/* 頂点列データ生成 */
				short		*verts, *norms, *uvs0, *uvs1, *uvs2 ;
				verts = kms_mdlpack->verts ;
				norms = kms_mdlpack->norms ;
				uvs0 = kms_mdlpack->uvs[0] ;
				uvs1 = kms_mdlpack->uvs[1] ;
				uvs2 = kms_mdlpack->uvs[2] ;
				for ( k = 0 ; k < pack->n_verts ; k++, vert++ ){
					vert->index = models[ i ].mats[1] * 4 ;
					vert->vx = verts[0] ;
					vert->vy = verts[1] ;
					vert->vz = verts[2] ;
					vert->w = verts[3] ;
					verts += 4 ;
					vert->nx = norms[0] ;
					vert->ny = norms[1] ;
					vert->nz = norms[2] ;
					vert->flag = norms[3] ;
					norms += 4 ;
					if ( uvs0 != NULL ){
						vert->u0 = uvs0[0] ;
						vert->v0 = uvs0[1] ;
						vert->q0 = 4096 ;
						uvs0 += 2 ;
					}
					if ( uvs1 != NULL ){
						vert->u1 = uvs1[0] ;
						vert->v1 = uvs1[1] ;
						vert->q1 = 4096 ;
						uvs1 += 2 ;
					}
					if ( uvs2 != NULL ){
						vert->u2 = uvs2[0] ;
						vert->v2 = uvs2[1] ;
						vert->q2 = 4096 ;
						uvs2 += 2 ;
					}
#if 0
printf("loop %d %d %d\n", i, j, k );
printf("%d %d %d %d \n", vert->vx, vert->vy, vert->vz, vert->w );
printf("%d %d %d %04x \n", vert->nx, vert->ny, vert->nz, vert->flag & 0xffff );
printf("%d %d %d %d \n", vert->u0, vert->v0, vert->q0, vert->index );
printf("%d %d %d \n", vert->u1, vert->v1, vert->q1);
printf("%d %d %d \n", vert->u2, vert->v2, vert->q2);
#endif
				}
				/* アライメント処理 */
				if ( pack->n_verts & 1 ) vert++ ;
			}
		}
	}

	/* データの書き込み */
	if ( ( fp = fopen( out_filename, "wb" ) ) == NULL ){
		printf("file open error!!(%s)\n", out_filename );
		return ( -1 );
	}

	/* 各種オフセットの設定 */
	size = 0 ;
	size += sizeof(KHM_HEADER) * 1 ;
	header.skeletons = (void*)size ;
	size += sizeof(KHM_SKEL) * header.n_skeletons ;
	header.models = (void*)size ;
	size += sizeof(KHM_MODEL) * header.n_models ;
	for ( i = 0 ; i < header.n_models ; i++ ){
		models[i].packet = (void*)size ;
		size += sizeof(KHM_PACKET) * models[i].n_packets ;
	}
	count = 0 ;
	for ( i = 0 ; i < header.n_models ; i++ ){
		for ( j = 0 ; j < models[i].n_packets ; j++, count++ ){
			packets[ count ].verts = (void*)size ;
			size += sizeof(KHM_VERT) * ( ( packets[ count ].n_verts + 1 ) & ~0x1 ) ;
		}
	}
	/* ファイルへの書きだし */
	fwrite( &header, sizeof(KHM_HEADER), 1, fp );
	fwrite( skeletons, sizeof(KHM_SKEL), header.n_skeletons, fp );
	fwrite( models, sizeof(KHM_MODEL), header.n_models, fp );
	fwrite( packets, sizeof(KHM_PACKET), n_packs, fp );
	fwrite( vertices, sizeof(KHM_VERT), n_verts, fp );

	fclose( fp );

	Free( vertices );
	Free( packets );
	Free( skeletons );
	Free( models );
}

int Dump_Khm( char *in_filename )
{
	FILE			*fp ;
	KHM_HEADER		*header ;
	int				size ;


	if ( ( fp = fopen( in_filename, "rb" ) ) == NULL ){
		printf("file open error!!(%s)\n", in_filename );
		return ( -1 );
	}
	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	header = Malloc0( size );
	fread( header, 1, size, fp );
	fclose( fp );

	KhmSetup( header );

	Free( header );
}











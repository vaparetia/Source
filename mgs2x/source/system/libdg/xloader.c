//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xloader.c
	ロード初期化ルーチン

	2002/03/12 K.Takabe
	$Id: xloader.c,v 1.1.1.3 2002/11/19 11:42:35 Yoshizawa1 Exp $

*/
/*
	ロード初期化ルーチン


*/

#ifdef KP_XBOX //BP


#ifndef KP_XBOX
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"

extern int		DG_LastLoadTexture ;

/* 将来的にはモデルデータに含まれるようになる */
static unsigned short	*IndexTop ;
//static unsigned short	LocalIndexBuffer[ 2*1024*1024 ];
//static unsigned short	LocalIndexBuffer[ 128*1204 ];
static unsigned short	LocalIndexBuffer[ 2 ];

/*----------------------------------------------------------------*/
/* KMS/EVM用の頂点バッファ情報を初期化する */
void DG_InitKmsVertexBuffer( void )
{
	IndexTop = LocalIndexBuffer ;
}
/* KMS/EVM用の頂点バッファを開放する */
void DG_ReleaseKmsVertexBuffer( void )
{
	IndexTop = LocalIndexBuffer ;
}

/*----------------------------------------------------------------*/
#define SET_ADDR( a, b ) ( ( a == NULL ) ? NULL : ((void*)( (int)a + (int)b )) )

//#define CULL_TEST		/* カリングルーチンデバッグ用 */
//#define EVM_BOUND_DEBUG

#if defined(CULL_TEST) || defined(EVM_BOUND_DEBUG) || 0
static void SVtoFV0( FVECTOR *v, SVECTOR *sv ){
	v->vx = sv->vx ;
	v->vy = sv->vy ;
	v->vz = sv->vz ;
	v->vw = 1.0f ;
}
static void SVtoFV12( FVECTOR *v, SVECTOR *sv ){
	v->vx = (float)sv->vx / 4096.0f ;
	v->vy = (float)sv->vy / 4096.0f  ;
	v->vz = (float)sv->vz / 4096.0f  ;
	v->vw = 1.0f ;
}
#endif

/*----------------------------------------------------------------*/
/* プリパケットの生成 */
static void KmsMakePrePacket( DG_DEF *def )
{
	DG_MDL		*mdl ;
	DG_MDLPACK	*pack ;
	int		i, j, k, total_verts, v_size, type ;

	/* タイプを調べる */
	if ( DG_GetMdlFormat( def ) == MGS_MODEL_MULTITEX || DG_GetMdlFormat( def ) == MGS_MODEL_MULTITEX_A ){
		type = 1 ;	/* マルチテクスチャ */
		v_size = sizeof(DG_VERTEX_KMSM);
	} else {
		type = 0 ;	/* シングルテクスチャ */
		v_size = sizeof(DG_VERTEX_KMSS);
	}

#if 0
	if ( type == 0 ){
		DG_VERTEX_KMSS	*v ;
		mdl = def->models ;
		for ( i = 0 ; i < def->n_x_models ; i++, mdl++ ){
			v = mdl->vbuff ;
			for ( j = 0 ; j < mdl->n_verts ; j++, v++ ){
				printf("%d %d %d %d %d %d %d\n", v->vx, v->vy, v->vz, v->wt, v->nx, v->ny, v->nz );
			}
		}
	} else {
		DG_VERTEX_KMSM	*v ;
		mdl = def->models ;
		for ( i = 0 ; i < def->n_x_models ; i++, mdl++ ){
			v = mdl->vbuff ;
			for ( j = 0 ; j < mdl->n_verts ; j++, v++ ){
				printf("%d %d %d %d %d %d %d\n", v->vx, v->vy, v->vz, v->wt, v->nx, v->ny, v->nz );
			}
		}
	}
	return ;
#endif

	mdl = def->models ;
	for ( i = 0 ; i < def->n_x_models ; i++, mdl++ ){
		/* 必要な頂点バッファ量を算出 */
		pack = mdl->packs ;
		total_verts = 0 ;
		for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
			total_verts += pack->n_verts ;
		}

		/* 頂点バッファの割り当て */
		mdl->vbuff = DG_AllocLocalVideoMemory( v_size * total_verts );
		mdl->stride = v_size ;

		{/* 頂点バッファへのデータ格納 */
			short		*v, *n, *uv0, *uv1, *uv2 ;
			if ( type == 0 ){
				/* シングルテクスチャ */
				DG_VERTEX_KMSS	*vbuff = (DG_VERTEX_KMSS*)mdl->vbuff ;
				pack = mdl->packs ;
				for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
					v = (short*)pack->verts ;
					n = (short*)pack->norms ;
					uv0 = (short*)pack->uvs[0] ;
					for ( k = 0 ; k < pack->n_verts ; k++, vbuff++ ){
						vbuff->vx = v[0] ;
						vbuff->vy = v[1] ;
						vbuff->vz = v[2] ;
						vbuff->wt = v[3] * 32767.0f / 4096.0f ;
						v += 4 ;
						vbuff->nx = n[0] * 32767.0f / 4096.0f ;
						vbuff->ny = n[1] * 32767.0f / 4096.0f ;
						vbuff->nz = n[2] * 32767.0f / 4096.0f ;
						vbuff->f = n[3] ;
						n += 4 ;
						if ( uv0 != NULL ){
							vbuff->u0 = uv0[0] * 32767.0f / 4096.0f ;
							vbuff->v0 = uv0[1] * 32767.0f / 4096.0f ;
							uv0 += 2 ;
						}
					}
				}
			} else {
				/* マルチテクスチャ */
				DG_VERTEX_KMSM	*vbuff = (DG_VERTEX_KMSM*)mdl->vbuff ;
				pack = mdl->packs ;
				for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
					v = (short*)pack->verts ;
					n = (short*)pack->norms ;
					uv0 = (short*)pack->uvs[0] ;
					uv1 = (short*)pack->uvs[1] ;
					uv2 = (short*)pack->uvs[2] ;
					for ( k = 0 ; k < pack->n_verts ; k++, vbuff++ ){
						vbuff->vx = v[0] ;
						vbuff->vy = v[1] ;
						vbuff->vz = v[2] ;
						vbuff->wt = v[3] * 32767.0f / 4096.0f ;
						v += 4 ;
						vbuff->nx = n[0] * 32767.0f / 4096.0f ;
						vbuff->ny = n[1] * 32767.0f / 4096.0f ;
						vbuff->nz = n[2] * 32767.0f / 4096.0f ;
						vbuff->f = n[3] ;
						n += 4 ;
						if ( uv0 != NULL ){
							vbuff->u0 = uv0[0] * 32767.0f / 4096.0f ;
							vbuff->v0 = uv0[1] * 32767.0f / 4096.0f ;
							uv0 += 2 ;
						}
						if ( uv1 != NULL ){
							vbuff->u1 = uv1[0] * 32767.0f / 4096.0f ;
							vbuff->v1 = uv1[1] * 32767.0f / 4096.0f ;
							uv1 += 2 ;
						}
						if ( uv2 != NULL ){
							vbuff->u2 = uv2[0] * 32767.0f / 4096.0f ;
							vbuff->v2 = uv2[1] * 32767.0f / 4096.0f ;
							uv2 += 2 ;
						}
					}
				}
			}
		}

		{/* インデックスバッファ作成 */
			short	*n ;
			unsigned short	*index, count ;

			pack = mdl->packs ;
			count = 0 ;
			for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
				if( def->data_format & MGS_MODEL_FLAG_INDEX ) continue;
				
				pack->index = IndexTop ;
				n = (short*)pack->norms ;

				if ( pack->n_verts == 0 ){
					pack->n_indices = 0 ;
					continue ;
				}

				if ( !( pack->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ) ){
					/* カリング無し */
					/* 念のため最初の二点はフラグを見ないで書き込んでおく */
					*IndexTop++ = count++ ;
					*IndexTop++ = count++ ;
					n += 4 * 2 ;

					/* キックフラグによるストリップ描画制御をエミュレーションする */
					for ( k = 2 ; k < pack->n_verts ; k++ ){
						if ( n[3] & 0x8000 ){
							*IndexTop++ = count - 1 ;
						}
						*IndexTop++ = count++ ;
						n += 4 ;
					}
				} else {
					/* カリング有り *//* 右回りに統一する */
					int		cull_flag = 0x00 ;
					/* 念のため最初の二点はフラグを見ないで書き込んでおく */
					*IndexTop++ = count++ ;
					*IndexTop++ = count++ ;
					n += 4 * 2 ;

					/* キックフラグによるストリップ描画制御をエミュレーションする */
					for ( k = 2 ; k < pack->n_verts ; k++ ){
						if ( n[3] & 0x8000 ){
							*IndexTop++ = count - 1 ;
							cull_flag = 0x20 - cull_flag ;
						} else if ( ( n[3] & 0xff ) == cull_flag ){
							IndexTop[ -1 ] = count - 2 ;
							*IndexTop++ = count - 1 ;
							cull_flag = 0x20 - cull_flag ;
						}
						*IndexTop++ = count++ ;
						cull_flag = 0x20 - cull_flag ;
						n += 4 ;
					}
				}

				/* インデックス数記録 */
				pack->n_indices = ( (int)IndexTop - (int)pack->index ) / sizeof(short) ;
			}
		}

	}
}

/* モデルデータセットアップ */
static void KmsDataSetup( DG_DEF *def )
{
	DG_MDL		*mdl ;
	DG_MDLPACK	*pack ;
	int			i, j, tri_code ;

	tri_code = -1 ;
	mdl = def->models ;
	for ( i = def->n_x_models ; i > 0 ; i--, mdl++ ){
		mdl->packs = (DG_MDLPACK*)( (int)mdl->packs + (int)def );
		mdl->vbuff = (void*)( (int)mdl->vbuff + (int)def );
		pack = mdl->packs ;
		for ( j = mdl->n_packs ; j > 0 ; j--, pack++ ){
#ifndef NEW_KMX_FORMAT
			pack->verts = (short*)( (int)pack->verts + (int)def );
			pack->norms = (short*)( (int)pack->norms + (int)def );
			if ( pack->uvs[0] != 0 ) pack->uvs[0] = (short*)( (int)pack->uvs[0] + (int)def );
			if ( pack->uvs[1] != 0 ) pack->uvs[1] = (short*)( (int)pack->uvs[1] + (int)def );
			if ( pack->uvs[2] != 0 ) pack->uvs[2] = (short*)( (int)pack->uvs[2] + (int)def );
			if ( pack->rgbs != 0 ) pack->rgbs = (CVECTOR*)( (int)pack->rgbs + (int)def );	/* 未使用なので */
#else
			/* DG_COMDLを使用する際に必要になるため（元々は未使用） */
			pack->rgbs = mdl ;
#endif
#if 0
			if ( (def->data_format & MGS_MODEL_FLAG_INDEX) && pack->index != NULL ) {
				pack->index = (void*)( (int)pack->index + (int)def );
			}
#else
			if ( pack->index != NULL ) {
				pack->index = (void*)( (int)pack->index + (int)def );
			}
#endif
			DG_WriteMdlPaketUV( def->texture, pack );
		}
	}
}

static void KmsDataSetup2( DG_DEF *def, void *top_addr )
{
	DG_MDL		*mdl ;
	DG_MDLPACK	*pack ;
	int			i, j, tri_code ;

	tri_code = -1 ;
	mdl = def->models ;
	for ( i = def->n_x_models ; i > 0 ; i--, mdl++ ){
		mdl->packs = (DG_MDLPACK*)( (int)mdl->packs + (int)top_addr );
		pack = mdl->packs ;
		for ( j = mdl->n_packs ; j > 0 ; j--, pack++ ){
			pack->verts = (short*)( (int)pack->verts + (int)top_addr );
			pack->norms = (short*)( (int)pack->norms + (int)top_addr );
			if ( pack->uvs[0] != 0 ) pack->uvs[0] = (short*)( (int)pack->uvs[0] + (int)top_addr );
			if ( pack->uvs[1] != 0 ) pack->uvs[1] = (short*)( (int)pack->uvs[1] + (int)top_addr );
			if ( pack->uvs[2] != 0 ) pack->uvs[2] = (short*)( (int)pack->uvs[2] + (int)top_addr );
			if ( pack->rgbs != 0 ) pack->rgbs = (CVECTOR*)( (int)pack->rgbs + (int)top_addr );	/* 未使用なので */
			if ( (def->data_format & MGS_MODEL_FLAG_INDEX) && pack->index != NULL ) {
				pack->index = (void*)( (int)pack->index + (int)top_addr );
			} 
			DG_WriteMdlPaketUV( def->texture, pack );
		}
	}
}

/*----------------------------------------------------------------*/
static void CvdDataSetup( CVD_DEF *cvd_def )
{
	CVD_MDL		*cvd_mdl ;
	int			i ;

	cvd_mdl = cvd_def->models ;
	for ( i = cvd_def->n_models ; i > 0 ; i--, cvd_mdl++ ){
		cvd_mdl->verts = (FVECTOR*)( (int)cvd_mdl->verts + (int)cvd_def );
		cvd_mdl->norms = (FVECTOR*)( (int)cvd_mdl->norms + (int)cvd_def );
		cvd_mdl->uvs = (FVECTOR*)( (int)cvd_mdl->uvs + (int)cvd_def );
		cvd_mdl->verts_index = (short*)( (int)cvd_mdl->verts_index + (int)cvd_def );
		cvd_mdl->norms_index = (short*)( (int)cvd_mdl->norms_index + (int)cvd_def );
		cvd_mdl->uvs_index = (short*)( (int)cvd_mdl->uvs_index + (int)cvd_def );
	}
}

/*----------------------------------------------------------------*/
static void Cv2DataSetup( CV2_DEF *cvd_def )
{
	CV2_MDL		*cvd_mdl ;
	int			i ;

	cvd_mdl = cvd_def->models ;
	for ( i = cvd_def->n_models ; i > 0 ; i--, cvd_mdl++ ){
		if(cvd_def->flag & COMMON_VERTS){
			cvd_mdl->verts = (FVECTOR*)( (int)cvd_mdl->verts + (int)cvd_def );
			cvd_mdl->verts_index = (short*)( (int)cvd_mdl->verts_index + (int)cvd_def );
		}
		if(cvd_def->flag & COMMON_NORMS){
			cvd_mdl->norms = (FVECTOR*)( (int)cvd_mdl->norms + (int)cvd_def );
			cvd_mdl->norms_index = (short*)( (int)cvd_mdl->norms_index + (int)cvd_def );
		}
		if(cvd_def->flag & COMMON_UVS){
			cvd_mdl->uvs = (FVECTOR*)( (int)cvd_mdl->uvs + (int)cvd_def );
			cvd_mdl->uvs_index = (short*)( (int)cvd_mdl->uvs_index + (int)cvd_def );
		}
		if(cvd_def->flag & COMMON_VERT_USRDATA){
			cvd_mdl->vert_usrdata = (VERT_USRDATA*)( (int)cvd_mdl->vert_usrdata
							    + (int)cvd_def );
		}
		if(cvd_def->flag & COMMON_NORM_USRDATA){
			cvd_mdl->norm_usrdata = (NORM_USRDATA*)( (int)cvd_mdl->norm_usrdata
							    + (int)cvd_def );
		}
	}
}

/*----------------------------------------------------------------*/
static void Lt2DataSetup( LIT_DEF *def )
{
	LIT_GRP		*grp ;
	int			i, j ;

	grp = (LIT_GRP*)&def[1] ;
	for ( i = 0 ; i < def->n_lit_group ; grp++, i++ ){
		grp->lit = (void*)( (int)grp->lit + (int)def );
		if ( grp->type & LIT_TYPE_POINT ){
			/* 点光源固有初期化 */
			LIT_POINT	*lit = (LIT_POINT*)grp->lit ;
			for ( j = grp->n_lights ; j > 0 ; lit++, j-- ){
			}
			
		} else if ( grp->type & LIT_TYPE_SPOT ){
			/* スポット光源固有初期化 */
			LIT_SPOT	*lit = (LIT_SPOT*)grp->lit ;
			for ( j = grp->n_lights ; j > 0 ; lit++, j-- ){
				//lit->umbra = cosf( lit->umbra );
				//lit->penumbra = cosf( lit->penumbra );
			}

		} else if ( grp->type & LIT_TYPE_BLACKPOINT ){
			/* 黒点光源固有初期化 */
			LIT_BLACKPOINT	*lit = (LIT_BLACKPOINT*)grp->lit ;
			for ( j = grp->n_lights ; j > 0 ; lit++, j-- ){
			}

		}
	}
}

/*----------------------------------------------------------------*/
#if 0
/* スケルトンのバウンディングを生成 */
static void MakeSkeletonBound( EVM_DEF *def )
{
	int			i, j, index ;
	EVM_PACK	*pack ;
	FVECTOR		*bound_list, *max, *min ;
	SVECTOR		*vec ;
	char	*weight ;

#if 0
	/* バウンド情報の記録（ロード用メモリとして確保する） */
	bound_list = GV_MallocLoad( sizeof(FVECTOR) * 2 * def->n_x_models );
#else
	bound_list = GV_Malloc( sizeof(FVECTOR) * 2 * def->n_x_models );
#endif
	def->packet[0].pad1  = (int)bound_list ;	/* ヘッダに空きがないのでここにポインタを記録 */
	/* バウンディングの初期化 */
	for ( i = 0 ; i < def->n_x_models ; i++ ){
		bound_list[ i * 2 + 0 ].vx = FLOAT_MAX ;
		bound_list[ i * 2 + 0 ].vy = FLOAT_MAX ;
		bound_list[ i * 2 + 0 ].vz = FLOAT_MAX ;
		bound_list[ i * 2 + 0 ].vw = 0.0f ;			/* 記録チェックフラグとして使用 */
		bound_list[ i * 2 + 1 ].vx = -FLOAT_MAX ;
		bound_list[ i * 2 + 1 ].vy = -FLOAT_MAX ;
		bound_list[ i * 2 + 1 ].vz = -FLOAT_MAX ;
		bound_list[ i * 2 + 1 ].vw = 1.0f ;
	}

	pack = def->packet ;
	for ( i = def->n_packs ; i > 0 ; i--, pack++ ){
		weight = (unsigned char*)pack->weight ;
		vec = (SVECTOR*)pack->verts ;
		if ( def->type & DG_EVMTYPE_LARGE ){/* バウンディング再計算 */
			int	k ;
			for ( k = 0 ; k < pack->n_verts ; k++ ){
				FVECTOR	v ;
				SVtoFV0( &v, vec );
				for ( j = 0 ; j < pack->n_mats ; j++ ){
					if ( weight[j] == 0 ) continue ;
					index = pack->mat_id[ weight[j+4]/4 ];
					if ( index == 0xff ) continue ;
					/* ウェイトの掛かっているスケルトンに対してバウンディングを更新 */
					min = &bound_list[ index * 2 + 0 ];
					max = &bound_list[ index * 2 + 1 ];
					max->vx = DG_MAX( max->vx, v.vx );
					max->vy = DG_MAX( max->vy, v.vy );
					max->vz = DG_MAX( max->vz, v.vz );
					min->vx = DG_MIN( min->vx, v.vx );
					min->vy = DG_MIN( min->vy, v.vy );
					min->vz = DG_MIN( min->vz, v.vz );
					min->vw = 1.0f ;	/* 記録フラグを立てる */
				}
				vec++ ;
				weight += 8 ;
			}
		} else {
			int	k ;
			for ( k = 0 ; k < pack->n_verts ; k++ ){
				FVECTOR	v ;
				SVtoFV0( &v, vec );
				v.vx /= 16.0f ;
				v.vy /= 16.0f ;
				v.vz /= 16.0f ;
				for ( j = 0 ; j < pack->n_mats ; j++ ){
					if ( weight[j] == 0 ) continue ;
					index = pack->mat_id[ weight[j+4]/4 ];
					if ( index == 0xff ) continue ;
					/* ウェイトの掛かっているスケルトンに対してバウンディングを更新 */
					min = &bound_list[ index * 2 + 0 ];
					max = &bound_list[ index * 2 + 1 ];
					max->vx = DG_MAX( max->vx, v.vx );
					max->vy = DG_MAX( max->vy, v.vy );
					max->vz = DG_MAX( max->vz, v.vz );
					min->vx = DG_MIN( min->vx, v.vx );
					min->vy = DG_MIN( min->vy, v.vy );
					min->vz = DG_MIN( min->vz, v.vz );
					min->vw = 1.0f ;	/* 記録フラグを立てる */
				}
				vec++ ;
				weight += 8 ;
			}
		}
		
	}

	for ( i = 0 ; i < def->n_x_models ; i++ ){
		/* ウェイトが掛かっていないダミースケルトンにはスケルトン中心座標をバウンディングに設定 */
		if ( bound_list[ i * 2 + 0 ].vw == 0.0f ){
			bound_list[ i * 2 + 0 ].vx = def->skeleton[ i ].rt_tx ;
			bound_list[ i * 2 + 0 ].vy = def->skeleton[ i ].rt_ty ;
			bound_list[ i * 2 + 0 ].vz = def->skeleton[ i ].rt_tz ;
			bound_list[ i * 2 + 1 ].vx = def->skeleton[ i ].rt_tx ;
			bound_list[ i * 2 + 1 ].vy = def->skeleton[ i ].rt_ty ;
			bound_list[ i * 2 + 1 ].vz = def->skeleton[ i ].rt_tz ;
			bound_list[ i * 2 + 0 ].vw = 1.0f ;
		}
	}

}
#endif
/* 頂点バッファの作成 */
static void MakeEvmVBuffer( EVM_DEF *def )
{
	DG_VERTEX_EVM	*verts ;
	EVM_PACK		*packs ;
	int				i, j, total_verts ;
	unsigned short	count = 0 ;

	/* 必要な頂点数を算出 */
	packs = def->packet ;
	total_verts = 0 ;
	for ( i = 0 ; i < def->n_packs ; i++, packs++ ){
		total_verts += packs->n_verts ;
	}

	/* 頂点バッファの確保 */
	verts = DG_AllocLocalVideoMemory( sizeof(DG_VERTEX_EVM) * total_verts );
	*(void**)&def->skeleton[ 0 ].pad0 = (int)verts ;
	packs = def->packet ;
	for ( i = 0 ; i < def->n_packs ; i++, packs++ ){
		short			*v, *n, *uv0, *uv1, *uv2 ;
		unsigned char	*weight ;
		v = packs->verts ;
		n = packs->norms ;
		uv0 = packs->uvs[0] ;
		uv1 = packs->uvs[1] ;
		uv2 = packs->uvs[2] ;
		weight = packs->weight ;
		for ( j = 0 ; j < packs->n_verts ; j++ ){
			verts->vx = v[0] ; verts->vy = v[1] ; verts->vz = v[2] ; v += 4 ;
			verts->vw = 1 ;
			verts->nx = n[0] ; verts->ny = n[1] ; verts->nz = n[2] ; n += 4 ;
			if ( uv0 != NULL ){
				verts->u0 = uv0[0] * ( 32767.0f / 4096.0f ) ;
				verts->v0 = uv0[1] * ( 32767.0f / 4096.0f ) ;
				verts->q0 = uv0[2] * ( 32767.0f / 4096.0f ) ;
				uv0 += 4 ;
			}
			if ( uv1 != NULL ){
				verts->u1 = uv1[0] * ( 32767.0f / 4096.0f ) ;
				verts->v1 = uv1[1] * ( 32767.0f / 4096.0f ) ;
				verts->q1 = uv1[2] * ( 32767.0f / 4096.0f ) ;
				uv1 += 4 ;
			}
			if ( uv2 != NULL ){
				verts->u2 = uv2[0] * ( 32767.0f / 4096.0f ) ;
				verts->v2 = uv2[1] * ( 32767.0f / 4096.0f ) ;
				verts->q2 = uv2[2] * ( 32767.0f / 4096.0f ) ;
				uv2 += 4 ;
			}
			//verts->weight[ 0 ] = weight[0] * 32767 / 128 ;
			verts->weight[ 1 ] = weight[1] * 32767 / 128 ;
			verts->weight[ 2 ] = weight[2] * 32767 / 128 ;
			verts->weight[ 3 ] = weight[3] * 32767 / 128 ;
			verts->weight[ 0 ] = 32767 - verts->weight[ 1 ] - verts->weight[ 2 ] - verts->weight[ 3 ] ;
			verts->index[ 0 ] = weight[ 4 ];
			verts->index[ 1 ] = weight[ 5 ];
			verts->index[ 2 ] = weight[ 6 ];
			verts->index[ 3 ] = weight[ 7 ];
			weight += 8 ;
			//printf("%p : %d %d %d\n", verts, verts->vx, verts->vy, verts->vz );
			verts++ ;
		}
	}

	/* インデックスデータの作成 */
	packs = def->packet ;
	for ( i = 0 ; i < def->n_packs ; i++, packs++ ){
		short			*v ;

		if( def->type & DG_EVMTYPE_INDEX ) continue;

		
		packs->index = IndexTop ;
		if ( packs->n_verts == 0 ){
			packs->n_indices = 0 ;
			continue ;
		}

		if ( !( packs->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ) ){
			/* カリング無し */
			/* 念のため最初の二点はフラグを見ないで書き込んでおく */
			*IndexTop++ = count++ ;
			*IndexTop++ = count++ ;
			v = packs->verts ;
			v += 4 * 2 ;
			for ( j = 2 ; j < packs->n_verts ; j++ ){
				if ( v[3] & 0x8000 ){
					*IndexTop++ = count - 1 ;
				}
				*IndexTop++ = count++ ;
				v += 4 ;
			}
		} else {
			/* カリング有り *//* 右回りに統一する */
			int		cull_flag = 0x00 ;
			/* 念のため最初の二点はフラグを見ないで書き込んでおく */
			*IndexTop++ = count++ ;
			*IndexTop++ = count++ ;
			v = packs->verts ;
			v += 4 * 2 ;
			for ( j = 2 ; j < packs->n_verts ; j++ ){
				if ( v[3] & 0x8000 ){
					*IndexTop++ = count - 1 ;
					cull_flag = 0x20 - cull_flag ;
				} else if ( ( v[3] & 0xff ) == cull_flag ){
					IndexTop[ -1 ] = count - 2 ;
					*IndexTop++ = count - 1 ;
					cull_flag = 0x20 - cull_flag ;
				}
				*IndexTop++ = count++ ;
				cull_flag = 0x20 - cull_flag ;
				v += 4 ;
			}
		}

		/* インデックス数記録 */
		packs->n_indices = ( (int)IndexTop - (int)packs->index ) / sizeof(short) ;
	}

}
static void EvmDataSetup( EVM_DEF *def )
{
	int			i ;
	EVM_PACK	*pack ;
#if 0
	int			count = 0, err_flag = 0 ;
#endif
#ifdef EVM_BOUND_DEBUG
	FVECTOR		max = {-FLOAT_MAX,-FLOAT_MAX,-FLOAT_MAX,1.0f} ;
	FVECTOR		min = {FLOAT_MAX,FLOAT_MAX,FLOAT_MAX,1.0f} ;
#endif

	def->packet = SET_ADDR( def->packet, def );
	def->vbuff = SET_ADDR( def->vbuff, def );
#if 0
	{
		DG_VERTEX_EVM	*verts ;
		verts = def->vbuff ;
		for ( i = 0 ; i < def->n_verts ; i++, verts++ ){
			printf("%d %d %d, ", verts->vx/16, verts->vy/16, verts->vz/16 );
			printf("%d %d %d\n", verts->nx/16, verts->ny/16, verts->nz/16 );
		}
	}
#endif
	pack = def->packet ;
	for ( i = def->n_packs ; i > 0 ; i--, pack++ ){
		pack->verts = SET_ADDR( pack->verts, def );
		pack->norms = SET_ADDR( pack->norms, def );
		pack->uvs[0] = SET_ADDR( pack->uvs[0], def );
		pack->uvs[1] = SET_ADDR( pack->uvs[1], def );
		pack->uvs[2] = SET_ADDR( pack->uvs[2], def );
		pack->weight = SET_ADDR( pack->weight, def );
		pack->index = SET_ADDR( pack->index, def );
		DG_WriteEvmMdlPaketUV( def->texture, pack );

#if 0
		pack->flag |= DG_PACKFLAG_CULLON ;
		{/* カリング用フラグ設定 */
			int	k ;
			for ( k = 2 ; k < pack->n_verts ; k++ ){
				if ( ( ((short*)pack->verts)[ k * 4 + 3 ] & 0x8000 ) == 0 ){
					FVECTOR	n, v1, v2, v3, l1, l2 ;
					FVECTOR	n1, n2, n3 ;
					SVtoFV0( &v1, (SVECTOR*)&((short*)pack->verts)[ ( k - 2 ) * 4 ] );
					SVtoFV0( &v2, (SVECTOR*)&((short*)pack->verts)[ ( k - 1 ) * 4 ] );
					SVtoFV0( &v3, (SVECTOR*)&((short*)pack->verts)[ ( k - 0 ) * 4 ] );
					SVtoFV12( &n1, (SVECTOR*)&((short*)pack->norms)[ ( k - 0 ) * 4 ] );
					SVtoFV12( &n2, (SVECTOR*)&((short*)pack->norms)[ ( k - 0 ) * 4 ] );
					SVtoFV12( &n3, (SVECTOR*)&((short*)pack->norms)[ ( k - 0 ) * 4 ] );
					_sceVu0AddVector( &n, &n1, &n2 );
					_sceVu0AddVector( &n, &n, &n3 );
					{
						float	rlen ;
						rlen = DG_RSQRT( n.vx * n.vx + n.vy * n.vy + n.vz * n.vz );
						n.vx *= rlen ;
						n.vy *= rlen ;
						n.vz *= rlen ;
					}
					_sceVu0SubVector( &l1, &v2, &v1 );
					_sceVu0SubVector( &l2, &v3, &v2 );
					_sceVu0OuterProduct( &v1, &l1, &l2 );
					if ( _sceVu0InnerProduct( &n, &v1 ) < 0.0f ){
						((short*)pack->verts)[ k * 4 + 3 ] &= 0x8000 ;
						((short*)pack->verts)[ k * 4 + 3 ] |= 0x0000 ;
					} else {
						((short*)pack->verts)[ k * 4 + 3 ] &= 0x8000 ;
						((short*)pack->verts)[ k * 4 + 3 ] |= 0x0020 ;
					}
				}
			}
		}
#endif

#if 0
		{/* マトリクスリスト整理 */
			u_char	*weight ;
			u_char	new_weight[8], index[8], *m_list ;
			int		j, k, n_matrix ;
			weight = pack->weight ;
			m_list = pack->mat_id ;
			n_matrix = 0 ;
			for ( j = 0 ; j < pack->n_verts ; j++, weight+=8 ){
				/* 重み情報のパレットインデックス化 */
				for ( k = 0 ; k < pack->n_mats ; k++ ){
					if ( weight[k] != 0 ){
						int ii ;
						for ( ii = 0 ; ii < n_matrix ; ii++ ){
							if ( index[ ii ] == m_list[ k ] ) break ;
						}
						if ( ii == n_matrix ){
							index[n_matrix] = m_list[k] ;
							n_matrix++ ;
						}
					}
				}
			}
			for ( k = 0 ; k < 8 ; k++ ){
				int		ii ;
				for ( ii = 0 ; ii < n_matrix ; ii++ ){
					if ( index[ii] == m_list[k] ) break ;
				}
				if ( ii == n_matrix && m_list[k] != 0xff ){
					printf("evm mat_id change %02x(%d) to 0xff\n", m_list[k] & 0xff, k );
					m_list[k] = 0xff ;
				}
			}
		}
#endif
#if 0
		if ( !( def->type & DG_EVMTYPE_FORMAT_V2 ) ){/* マトリクスパレット対応化 */
			char	*weight ;
			char	new_weight[8], index[8] ;
			int		j, k, n_matrix, max_matrix ;
			weight = pack->weight ;
			max_matrix = 0 ;
			for ( j = pack->n_mats ; j < 8 ; j++ ) pack->mat_id[j] = 0xff ;
			for ( j = 0 ; j < pack->n_verts ; j++, weight+=8 ){
				/* 重み情報のパレットインデックス化 */
				n_matrix = 0 ;
				for ( k = 0 ; k < pack->n_mats ; k++ ){
					if ( weight[k] != 0 ){
						new_weight[n_matrix] = weight[k] ;
						index[n_matrix] = k ;
						n_matrix++ ;
					}
				}
				if ( n_matrix > 4 ){
					printf("loader.c: evm matrix error!! (%d %d %d %d %d %d %d %d)\n",
						   weight[0], weight[1], weight[2], weight[3],
						   weight[4], weight[5], weight[6], weight[7] );
					n_matrix = 4 ;
				}
				/* 書き込み */
				for ( k = 0 ; k < n_matrix ; k++ ){
					weight[k] = new_weight[k] ;
					weight[k+4] = index[k] * 4 ;
				}
				for (  ; k < 4 ; k++ ){
					weight[k] = 0 ;
					weight[k+4] = 0 ;
				}
				/* 最大マトリクス数の調整 */
				if ( n_matrix > max_matrix ){
					max_matrix = n_matrix ;
				}
			}
			/* 最大マトリクス数補正 */
			if ( pack->n_mats != max_matrix ){
				printf("convert %d to %d\n", pack->n_mats, max_matrix );
				count++ ;
			}
			pack->n_mats = max_matrix ;
		}
#endif

#ifdef EVM_BOUND_DEBUG
		if ( def->type & DG_EVMTYPE_LARGE ){/* バウンディング再計算 */
			int	k ;
			for ( k = 0 ; k < pack->n_verts ; k++ ){
				FVECTOR	v ;
				SVtoFV0( &v, (SVECTOR*)&((short*)pack->verts)[ k * 4 ] );
				max.vx = DG_MAX( max.vx, v.vx );
				max.vy = DG_MAX( max.vy, v.vy );
				max.vz = DG_MAX( max.vz, v.vz );
				min.vx = DG_MIN( min.vx, v.vx );
				min.vy = DG_MIN( min.vy, v.vy );
				min.vz = DG_MIN( min.vz, v.vz );
			}
		} else {
			int	k ;
			for ( k = 0 ; k < pack->n_verts ; k++ ){
				FVECTOR	v ;
				SVtoFV0( &v, (SVECTOR*)&((short*)pack->verts)[ k * 4 ] );
				v.vx /= 16.0f ;
				v.vy /= 16.0f ;
				v.vz /= 16.0f ;
				max.vx = DG_MAX( max.vx, v.vx );
				max.vy = DG_MAX( max.vy, v.vy );
				max.vz = DG_MAX( max.vz, v.vz );
				min.vx = DG_MIN( min.vx, v.vx );
				min.vy = DG_MIN( min.vy, v.vy );
				min.vz = DG_MIN( min.vz, v.vz );
			}
		}
#endif

	}
#ifdef EVM_BOUND_DEBUG
	printf("max %f %f %f\n", max.vx, max.vy, max.vz );
	printf("min %f %f %f\n", min.vx, min.vy, min.vz );
	def->lx = min.vx ;
	def->ly = min.vy ;
	def->lz = min.vz ;
	def->ux = max.vx ;
	def->uy = max.vy ;
	def->uz = max.vz ;
#endif
#if 0
	printf("converted packet %d / %d\n", count, def->n_packs );
	if ( err_flag ){
		printf("err---------------------------------------------------------------------------------");
	}
#endif

}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

/* モデルの初期化 */
int		DG_LoadInitKms( void *buf, int id )
{
	int		format_id ;
	if ( id & GV_CACHEID_RESIDENT ){
#ifndef NEW_KMX_FORMAT
		KmsMakePrePacket( (DG_DEF*) buf );	/* プリパケットのみ生成しなおす必要があるため */
#endif
		return (1);
	}
	format_id = DG_GetMdlFormat( ((DG_DEF*)buf) );
	if ( format_id != MGS_MODEL_NORM && format_id != MGS_MODEL_MULTITEX &&
		format_id != MGS_MODEL_NORM_A && format_id != MGS_MODEL_MULTITEX_A ){
		printf("file format error!!(%d)\n", format_id );
		return ( 0 );
	}
	KmsDataSetup( (DG_DEF*) buf );
#ifndef NEW_KMX_FORMAT
	KmsMakePrePacket( (DG_DEF*) buf );
#endif
	//((DG_DEF*)buf)->pad[2] = id ;	/* デバッグ用 */
	return (1);
}

/*----------------------------------------------------------------*/

/* 共有頂点データの初期化 */
int		DG_LoadInitCvd( void *buf, int id )
{
	if ( id & GV_CACHEID_RESIDENT ) return (1);
	if ( ( (CVD_DEF*)buf )->id == 6754556 ){
		CvdDataSetup( (CVD_DEF*) buf );
	} else {
		Cv2DataSetup( (CV2_DEF*) buf );
	}
	return (1);
}

/*----------------------------------------------------------------*/

/* 共有頂点データの初期化 */
int		DG_LoadInitCv2( void *buf, int id )
{
	if ( id & GV_CACHEID_RESIDENT ) return (1);
	Cv2DataSetup( (CV2_DEF*) buf );
	return (1);
}

/*----------------------------------------------------------------*/

/* ライトの初期化 */
int		DG_LoadInitLt2( void *buf, int id )
{
	if ( id & GV_CACHEID_RESIDENT ) return (1);
	Lt2DataSetup( (LIT_DEF*) buf );
	return (1);
}

/*----------------------------------------------------------------*/
/* マルチウェイトエンベロープモデルの初期化 */
int		DG_LoadInitEvm( void *buf, int id )
{
	if ( id & GV_CACHEID_RESIDENT ){
		//MakeSkeletonBound( (EVM_DEF*)buf );
#ifndef NEW_EVX_FORMAT
		MakeEvmVBuffer( (EVM_DEF*)buf );
#endif
		return (1);
	}
	EvmDataSetup( (EVM_DEF*) buf );
	//MakeSkeletonBound( (EVM_DEF*)buf );
#ifndef NEW_EVX_FORMAT
	MakeEvmVBuffer( (EVM_DEF*)buf );
#endif
	return (1);
}

/*----------------------------------------------------------------*/
/* ＺＡＲデータの初期化 */
int		DG_LoadInitZar( void *buf, int id )
{
	//CACHE *GV_SetCache( int id, void *buf );
	extern void *GV_SetCache( int id, void *buf );
	ZAR_HEADER	*header = buf ;
	int			i ;

	//if ( id & GV_CACHEID_RESIDENT ) return (1);
	switch ( header->format_type ){
	  case ZAR_FORMAT_KMS:
		for ( i = 0 ; i < header->n_datas ; i++ ){
			DG_DEF	*def ;
			if ( !( id & GV_CACHEID_RESIDENT ) ){
				def = (DG_DEF*)( header->list[ i ].data_offset + (int)buf );
				KmsDataSetup2( def, buf );
			} else {
				def = (DG_DEF*)header->list[ i ].data_offset ;
			}
			KmsMakePrePacket( def );
			header->list[ i ].data_offset = (int)def ;
			/* モデルデータとして登録 */
			GV_SetCache( GV_CacheID( header->list[ i ].data_id, 'k' ), def );
		}
		break ;
	}
	return (1);
}

/* ＸＢＯＸ固有頂点バッファ開放処理 */
void DG_UnloadKms( void *buf )
{
	DG_DEF	*def = (DG_DEF*)buf ;
	DG_MDL	*mdl ;
	int		i ;
	mdl = def->models ;
	for ( i = 0 ; i < def->n_x_models ; i++, mdl++ ){
		//DG_FreeLocalVideoMemory( mdl->vbuff );
	}
}

void DG_UnloadEvm( void *buf )
{
	EVM_DEF	*def = (EVM_DEF*)buf ;
	//DG_FreeLocalVideoMemory( *(void**)&def->skeleton[ 0 ].pad0 );
}

#endif

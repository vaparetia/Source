//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	loader.c
	ロード初期化ルーチン

	1999/07/07 K.Takabe
	$Id: loader.c,v 1.1.1.3 2002/11/19 11:42:11 Yoshizawa1 Exp $

*/
/*
	ロード初期化ルーチン

	int		DG_LoadInitKmd( buf, id )
	void		*buf ;		未初期化バッファ
	int		id ;		キャッシュＩＤ

		モデルデータ（ *.kmd ）の初期化

	int		DG_LoadInitNar( buf, id )
	DG_NARS		*buf ;		未初期化バッファ
	int		id ;		キャッシュＩＤ

		モーションデータ（ *.nar ）の初期化

	int		DG_LoadInitImg( buf, id )
	long64		*buf ;		未初期化バッファ
	int		id ;		キャッシュＩＤ

	        背景データ( *.img )の初期化

	int		DG_LoadInitSgt( buf, id )
	long64		*buf ;		未初期化バッファ
	int		id ;		キャッシュＩＤ

	        賑やかしデータ( *.sgt )の初期化

	int		DG_LoadInitLit( buf, id )
	long64		*buf ;		未初期化バッファ
	int		id ;		キャッシュＩＤ

		光源データ（ *.lit ）の初期化

	int		DG_LoadInitPcx( buf, id )
	PCXDATA		*buf ;		未初期化バッファ
	int		id ;		キャッシュＩＤ

		画像データ（ *.pcx ）の初期化
*/


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

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"

#include "BP_BaseRenderer.h"
#include "BP_FileSupport.h"
#include "BP_EndianSupport.h"

extern int		DG_LastLoadTexture ;

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
#if 0
	DG_MDL		*mdl ;
	DG_MDLPACK	*pack ;
	int			i, j, n_models, tri_code, total_packet ;
	DG_KMS_PREPACKET	*pre_packet ;

	/* メモリ使用量の計算 */
	mdl = def->models ;
	total_packet = 0 ;
	for ( i = def->n_x_models ; i > 0 ; i--, mdl++ ){
		total_packet += mdl->n_packs ;
	}

	pre_packet = GV_Malloc( total_packet * sizeof(DG_KMS_PREPACKET) );
	mdl = def->models ;
	for ( i = def->n_x_models ; i > 0 ; i--, mdl++ ){
		mdl->pre_packet = pre_packet ;
		/* 描画時に使用するパケットの一部（普遍なもののみ）を予め生成しておく */
		pack = mdl->packs ;
		for ( j = mdl->n_packs ; j > 0 ; j--, pack++ ){
			/* 頂点用プリパケット生成 */
			if ( pack->verts != NULL && pack->n_verts != 0 ){
				pre_packet->verts_qwc = DMATAG_SET_QWC( DMATAG_ID_REF, ( pack->n_verts + 1 ) / 2 );
				pre_packet->verts_vif = SCE_VIF1_SET_UNPACKR( 0x100, ( pack->n_verts + 1 ) & 0xfffe,
															 VIF_VERT_PACK, 0 );
			} else {
				pre_packet->verts_qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
				pre_packet->verts_vif = SCE_VIF1_SET_NOP( 0 );
			}
			/* 法線用プリパケット生成 */
			if ( pack->norms != NULL && pack->n_verts != 0 ){
				pre_packet->norms_qwc = DMATAG_SET_QWC( DMATAG_ID_REF, ( pack->n_verts + 1 ) / 2 );
				pre_packet->norms_vif = SCE_VIF1_SET_UNPACKR( 0x101, ( pack->n_verts + 1 ) & 0xfffe,
															 VIF_NORM_PACK, 0 );
			} else {
				pre_packet->norms_qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
				pre_packet->norms_vif = SCE_VIF1_SET_NOP( 0 );
			}
			/* ＵＶ０用プリパケット生成 */
			if ( pack->uvs[0] != NULL && pack->n_verts != 0 ){
				pre_packet->uvs0_qwc = DMATAG_SET_QWC( DMATAG_ID_REF, ( pack->n_verts + 3 ) / 4 );
				pre_packet->uvs0_vif = SCE_VIF1_SET_UNPACKR( 0x102, ( pack->n_verts + 3 ) & 0xfffc,
															VIF_UV_PACK, 0 );
			} else {
				pre_packet->uvs0_qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
				pre_packet->uvs0_vif = SCE_VIF1_SET_NOP( 0 );
			}
			/* ＵＶ１用プリパケット生成 */
			if ( pack->uvs[1] != NULL && pack->n_verts != 0 ){
				pre_packet->uvs1_qwc = DMATAG_SET_QWC( DMATAG_ID_REF, ( pack->n_verts + 3 ) / 4 );
				pre_packet->uvs1_vif = SCE_VIF1_SET_UNPACKR( 0x103, ( pack->n_verts + 3 ) & 0xfffc,
															VIF_UV_PACK, 0 );
			} else {
				pre_packet->uvs1_qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
				pre_packet->uvs1_vif = SCE_VIF1_SET_NOP( 0 );
			}
			/* ＵＶ２用プリパケット生成 */
			if ( pack->uvs[2] != NULL && pack->n_verts != 0 ){
				pre_packet->uvs2_qwc = DMATAG_SET_QWC( DMATAG_ID_REF, ( pack->n_verts + 3 ) / 4 );
				pre_packet->uvs2_vif = SCE_VIF1_SET_UNPACKR( 0x104, ( pack->n_verts + 3 ) & 0xfffc,
															VIF_UV_PACK, 0 );
			} else {
				pre_packet->uvs2_qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
				pre_packet->uvs2_vif = SCE_VIF1_SET_NOP( 0 );
			}
			/* 頂点カラー用プリパケット生成 */
			if ( pack->verts != NULL && pack->n_verts != 0 ){
				pre_packet->rgbs_qwc = DMATAG_SET_QWC( DMATAG_ID_REF, ( pack->n_verts + 1 ) / 2 );
				pre_packet->rgbs_vif = SCE_VIF1_SET_UNPACKR( 0x105, ( pack->n_verts + 1 ) & 0xfffe,
															VIF_VERT_PACK, 0 );
			} else {
				pre_packet->rgbs_qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
				pre_packet->rgbs_vif = SCE_VIF1_SET_NOP( 0 );
			}
			pre_packet++ ;
		}
	}
#endif
}

static void EndianSwapShortVectors( short *pDst, unsigned int elementSize, unsigned int count )
{
   if ( pDst )
   {
      BP_LE_SwapSShortArray_Inp( pDst, elementSize * count );
   }
}

static void EndianSwapKmsData( DG_DEF *pData )
{
   BP_LE_SwapUInt_Inp( &pData->data_format );
   BP_LE_SwapUInt_Inp( &pData->n_models );
   BP_LE_SwapUInt_Inp( &pData->n_x_models );
   BP_LE_SwapUInt_Inp( &pData->texture );
   BP_LE_SwapSIntArray_Inp( &( pData->pad[0] ), 3 );
   BP_LE_SwapFloatArray_Inp( &pData->lx, 3 );
   BP_LE_SwapFloatArray_Inp( &pData->ux, 3 );
   BP_LE_SwapFloatArray_Inp( &pData->tx, 3 );
}

static void EndianSwapDgMdl( DG_MDL *pData )
{
   BP_LE_SwapUInt_Inp( &pData->type );
   BP_LE_SwapUInt_Inp( &pData->n_packs );
   BP_LE_SwapFloatArray_Inp( &pData->lx, 3 );
   BP_LE_SwapFloatArray_Inp( &pData->ux, 3 );
   BP_LE_SwapFloatArray_Inp( &pData->tx, 3 );
   BP_LE_SwapSInt_Inp( &pData->parent );
   BP_LE_SwapPtr_Inp( &pData->packs );
   BP_LE_SwapSInt_Inp( &pData->pad0 );
   BP_LE_SwapSInt_Inp( &pData->pad1 );
}

static void EndianSwapDgMdlPack( DG_MDLPACK *pData )
{
   BP_LE_SwapUInt_Inp( &pData->flag );
   BP_LE_SwapUInt_Inp( &pData->n_verts );
   BP_LE_SwapUInt_Inp( &( pData->tex_id[0] ) );
   BP_LE_SwapUInt_Inp( &( pData->tex_id[1] ) );
   BP_LE_SwapUInt_Inp( &( pData->tex_id[2] ) );
   BP_LE_SwapPtr_Inp( &pData->verts );
   BP_LE_SwapPtr_Inp( &pData->norms );
   BP_LE_SwapPtr_Inp( &( pData->uvs[0] ) );
   BP_LE_SwapPtr_Inp( &( pData->uvs[1] ) );
   BP_LE_SwapPtr_Inp( &( pData->uvs[2] ) );
   BP_LE_SwapPtr_Inp( &pData->rgbs );
   BP_LE_SwapSInt_Inp( &pData->pad );
}

static void KmsDataSetup( DG_DEF *def )
{
	DG_MDL		*mdl ;
	DG_MDLPACK	*pack ;
	int			i, j, tri_code ;

   EndianSwapKmsData( def );

	tri_code = -1 ;
	mdl = def->models ;
	for ( i = def->n_x_models ; i > 0 ; i--, mdl++ ){
      EndianSwapDgMdl( mdl );
		mdl->packs = (DG_MDLPACK*)( (int)mdl->packs + (int)def );
		pack = mdl->packs ;
		for ( j = mdl->n_packs ; j > 0 ; j--, pack++ ){
         EndianSwapDgMdlPack( pack );

			pack->verts = (short*)( (int)pack->verts + (int)def );
			pack->norms = (short*)( (int)pack->norms + (int)def );
			if ( pack->uvs[0] != 0 ) pack->uvs[0] = (short*)( (int)pack->uvs[0] + (int)def );
			if ( pack->uvs[1] != 0 ) pack->uvs[1] = (short*)( (int)pack->uvs[1] + (int)def );
			if ( pack->uvs[2] != 0 ) pack->uvs[2] = (short*)( (int)pack->uvs[2] + (int)def );
//printf("%08x %08x %08x %08x %08x \n", pack->verts, pack->norms, pack->uvs[0], pack->uvs[1], pack->uvs[2] );
			if ( pack->rgbs != 0 ) pack->rgbs = (CVECTOR*)( (int)pack->rgbs + (int)def );	/* 未使用なので */
#if 0
			if ( def->data_format == MGS_MODEL_NORM || def->data_format == MGS_MODEL_MULTITEX ){
				pack->pad = 0 ;
			} else {
				pack->pad = (short*)( (int)pack->pad + (int)def );
			}
#endif
         
         // Note that KmsDataSetup and KmsDataSetup2 have almost the same code here
         EndianSwapShortVectors( pack->verts, 4, pack->n_verts );
         EndianSwapShortVectors( pack->norms, 4, pack->n_verts );
         EndianSwapShortVectors( pack->uvs[0], 2, pack->n_verts );
         EndianSwapShortVectors( pack->uvs[1], 2, pack->n_verts );
         EndianSwapShortVectors( pack->uvs[2], 2, pack->n_verts );

         DG_WriteMdlPaketUV( def->texture, pack );
#ifdef CULL_TEST
			{/* カリング用フラグ設定 */
				int	k ;
				pack->flag |= 0x0004 ;
				for ( k = 2 ; k < pack->n_verts ; k++ ){
					if ( ( pack->norms[ k * 4 + 3 ] & 0x8000 ) == 0 ){
						FVECTOR	n, v1, v2, v3, l1, l2 ;
						SVtoFV0( &v1, (SVECTOR*)&pack->verts[ ( k - 2 ) * 4 ] );
						SVtoFV0( &v2, (SVECTOR*)&pack->verts[ ( k - 1 ) * 4 ] );
						SVtoFV0( &v3, (SVECTOR*)&pack->verts[ ( k - 0 ) * 4 ] );
						SVtoFV12( &n, (SVECTOR*)&pack->norms[ ( k - 0 ) * 4 ] );
						_sceVu0SubVector( &l1, &v2, &v1 );
						_sceVu0SubVector( &l2, &v3, &v2 );
						_sceVu0OuterProduct( &v1, &l1, &l2 );
						if ( ( _sceVu0InnerProduct( &n, &v1 ) < 0.0f ) ){
							pack->norms[ k * 4 + 3 ] &= 0x8000 ;
							pack->norms[ k * 4 + 3 ] |= 0x0000 ;
						} else {
							pack->norms[ k * 4 + 3 ] &= 0x8000 ;
							pack->norms[ k * 4 + 3 ] |= 0x0020 ;
						}
					}
				}
			}
#endif
		}
	}

}
static void KmsDataSetup2( DG_DEF *def, void *top_addr )
{
	DG_MDL		*mdl ;
	DG_MDLPACK	*pack ;
	int			i, j, tri_code ;

   EndianSwapKmsData( def );

	tri_code = -1 ;
	mdl = def->models ;
	for ( i = def->n_x_models ; i > 0 ; i--, mdl++ ){
      EndianSwapDgMdl( mdl );
		mdl->packs = (DG_MDLPACK*)( (int)mdl->packs + (int)top_addr );
		pack = mdl->packs ;
		for ( j = mdl->n_packs ; j > 0 ; j--, pack++ ){
         EndianSwapDgMdlPack( pack );

			pack->verts = (short*)( (int)pack->verts + (int)top_addr );
			pack->norms = (short*)( (int)pack->norms + (int)top_addr );
			if ( pack->uvs[0] != 0 ) pack->uvs[0] = (short*)( (int)pack->uvs[0] + (int)top_addr );
			if ( pack->uvs[1] != 0 ) pack->uvs[1] = (short*)( (int)pack->uvs[1] + (int)top_addr );
			if ( pack->uvs[2] != 0 ) pack->uvs[2] = (short*)( (int)pack->uvs[2] + (int)top_addr );
			if ( pack->rgbs != 0 ) pack->rgbs = (CVECTOR*)( (int)pack->rgbs + (int)top_addr );	/* 未使用なので */

         // Note that KmsDataSetup and KmsDataSetup2 have almost the same code here
         EndianSwapShortVectors( pack->verts, 4, pack->n_verts );
         EndianSwapShortVectors( pack->norms, 4, pack->n_verts );
         EndianSwapShortVectors( pack->uvs[0], 2, pack->n_verts );
         EndianSwapShortVectors( pack->uvs[1], 2, pack->n_verts );
         EndianSwapShortVectors( pack->uvs[2], 2, pack->n_verts );

         DG_WriteMdlPaketUV( def->texture, pack );
		}
	}
}

/*----------------------------------------------------------------*/

static void EndianSwapCVDHeader( CVD_DEF *pData )
{
   BP_LE_SwapSInt_Inp( &( pData->id ) );
   BP_LE_SwapSInt_Inp( &( pData->n_models ) );
   BP_LE_SwapSInt_Inp( &( pData->flag ) );
   BP_LE_SwapSInt_Inp( &( pData->pad1 ) );
}

/*----------------------------------------------------------------*/

static void EndianSwapCVDMDL_Outer( CVD_MDL *pData )
{
   BP_LE_SwapSInt_Inp( &pData->n_verts );
   BP_LE_SwapSInt_Inp( &pData->n_verts_index );
   BP_LE_SwapPtr_Inp( &pData->verts );
   BP_LE_SwapPtr_Inp( &pData->verts_index );

   BP_LE_SwapSInt_Inp( &pData->n_norms );
   BP_LE_SwapSInt_Inp( &pData->n_norms_index );
   BP_LE_SwapPtr_Inp( &pData->norms );
   BP_LE_SwapPtr_Inp( &pData->norms_index );

   BP_LE_SwapSInt_Inp( &pData->n_uvs );
   BP_LE_SwapSInt_Inp( &pData->n_uvs_index );
   BP_LE_SwapPtr_Inp( &pData->uvs );
   BP_LE_SwapPtr_Inp( &pData->uvs_index );
}

/*----------------------------------------------------------------*/

static void EndianSwapCVDMDL_Inner( CVD_MDL *pData )
{
   BP_LE_SwapFloatArray_Inp( &( pData->verts[0].vx ), 4 * pData->n_verts );
   BP_LE_SwapSShortArray_Inp( pData->verts_index, pData->n_verts_index );

   BP_LE_SwapFloatArray_Inp( &( pData->norms[0].vx ), 4 * pData->n_norms );
   BP_LE_SwapSShortArray_Inp( pData->norms_index, pData->n_norms_index );

   BP_LE_SwapFloatArray_Inp( &( pData->uvs[0].vx ), 4 * pData->n_uvs );
   BP_LE_SwapSShortArray_Inp( pData->uvs_index, pData->n_uvs_index );
}

/*----------------------------------------------------------------*/
static void CvdDataSetup( CVD_DEF *cvd_def )
{
	CVD_MDL		*cvd_mdl ;
	int			i ;

   EndianSwapCVDHeader( cvd_def );

	cvd_mdl = cvd_def->models ;
	for ( i = cvd_def->n_models ; i > 0 ; i--, cvd_mdl++ ){
      EndianSwapCVDMDL_Outer( cvd_mdl );
		cvd_mdl->verts = (FVECTOR*)( (int)cvd_mdl->verts + (int)cvd_def );
		cvd_mdl->norms = (FVECTOR*)( (int)cvd_mdl->norms + (int)cvd_def );
		cvd_mdl->uvs = (FVECTOR*)( (int)cvd_mdl->uvs + (int)cvd_def );
		cvd_mdl->verts_index = (short*)( (int)cvd_mdl->verts_index + (int)cvd_def );
		cvd_mdl->norms_index = (short*)( (int)cvd_mdl->norms_index + (int)cvd_def );
		cvd_mdl->uvs_index = (short*)( (int)cvd_mdl->uvs_index + (int)cvd_def );
      EndianSwapCVDMDL_Inner( cvd_mdl );
	}
}

/*----------------------------------------------------------------*/

static void EndianSwapCV2Header( CV2_DEF *pData )
{
   BP_LE_SwapSInt_Inp( &( pData->id ) );
   BP_LE_SwapSInt_Inp( &( pData->n_models ) );
   BP_LE_SwapSInt_Inp( &( pData->flag ) );
   BP_LE_SwapSInt_Inp( &( pData->pad1 ) );
}

/*----------------------------------------------------------------*/

static void EndianSwapCV2MDL_Outer( CV2_MDL *pData )
{
   BP_LE_SwapSInt_Inp( &pData->n_verts );
   BP_LE_SwapSInt_Inp( &pData->n_verts_index );
   BP_LE_SwapPtr_Inp( &pData->verts );
   BP_LE_SwapPtr_Inp( &pData->verts_index );

   BP_LE_SwapSInt_Inp( &pData->n_norms );
   BP_LE_SwapSInt_Inp( &pData->n_norms_index );
   BP_LE_SwapPtr_Inp( &pData->norms );
   BP_LE_SwapPtr_Inp( &pData->norms_index );

   BP_LE_SwapSInt_Inp( &pData->n_uvs );
   BP_LE_SwapSInt_Inp( &pData->n_uvs_index );
   BP_LE_SwapPtr_Inp( &pData->uvs );
   BP_LE_SwapPtr_Inp( &pData->uvs_index );

   BP_LE_SwapPtr_Inp( &pData->vert_usrdata );
   BP_LE_SwapPtr_Inp( &pData->norm_usrdata_le );
}

/*----------------------------------------------------------------*/
static void Cv2DataSetup( CV2_DEF *cvd_def )
{
	CV2_MDL		*cvd_mdl ;
	int			i ;

   EndianSwapCV2Header( cvd_def );

	cvd_mdl = cvd_def->models ;
	for ( i = cvd_def->n_models ; i > 0 ; i--, cvd_mdl++ ){
      EndianSwapCV2MDL_Outer( cvd_mdl );

		if(cvd_def->flag & COMMON_VERTS){
			cvd_mdl->verts = (FVECTOR*)( (int)cvd_mdl->verts + (int)cvd_def );
			cvd_mdl->verts_index = (short*)( (int)cvd_mdl->verts_index + (int)cvd_def );

         BP_LE_SwapFloatArray_Inp( &cvd_mdl->verts[0].vx, cvd_mdl->n_verts * 4 );
         BP_LE_SwapSShortArray_Inp( cvd_mdl->verts_index, cvd_mdl->n_verts_index );
		}
		if(cvd_def->flag & COMMON_NORMS){
			cvd_mdl->norms = (FVECTOR*)( (int)cvd_mdl->norms + (int)cvd_def );
			cvd_mdl->norms_index = (short*)( (int)cvd_mdl->norms_index + (int)cvd_def );

         BP_LE_SwapFloatArray_Inp( &cvd_mdl->norms[0].vx, cvd_mdl->n_norms * 4 );
         BP_LE_SwapSShortArray_Inp( cvd_mdl->norms_index, cvd_mdl->n_norms_index );
      }
		if(cvd_def->flag & COMMON_UVS){
			cvd_mdl->uvs = (FVECTOR*)( (int)cvd_mdl->uvs + (int)cvd_def );
			cvd_mdl->uvs_index = (short*)( (int)cvd_mdl->uvs_index + (int)cvd_def );

         BP_LE_SwapFloatArray_Inp( &cvd_mdl->uvs[0].vx, cvd_mdl->n_uvs * 4 );
         BP_LE_SwapSShortArray_Inp( cvd_mdl->uvs_index, cvd_mdl->n_uvs_index );
		}
		if(cvd_def->flag & COMMON_VERT_USRDATA){
         int vertIndex;

			cvd_mdl->vert_usrdata = (VERT_USRDATA*)( (int)cvd_mdl->vert_usrdata
							    + (int)cvd_def );

         // BP - there are n_verts vert_usrdata 
         for ( vertIndex = 0; vertIndex < cvd_mdl->n_verts; ++vertIndex )
         {
            BP_LE_SwapSInt_Inp( &cvd_mdl->vert_usrdata[vertIndex].nVertexSwing );
            BP_LE_SwapSShortArray_Inp( cvd_mdl->vert_usrdata[vertIndex].dWeight, 4 );
         }
		}
		if(cvd_def->flag & COMMON_NORM_USRDATA){
			cvd_mdl->norm_usrdata_le = (NORM_USRDATA*)( (int)cvd_mdl->norm_usrdata_le
							    + (int)cvd_def );
		}
	}
}

static void EndianSwapLITDef(LIT_DEF* pData)
{
   BP_LE_SwapFloatArray_Inp(&pData->dir.vx, 4);
   BP_LE_SwapSInt_Inp(&pData->n_lit_group);
   BP_LE_SwapSInt_Inp(&pData->pad);
}

static void EndianSwapLITGrp(LIT_GRP* pData)
{
   BP_LE_SwapFloatArray_Inp(&pData->bound_max.vx, 4);
   BP_LE_SwapFloatArray_Inp(&pData->bound_min.vx, 4);
   BP_LE_SwapSInt_Inp(&pData->n_lights);
   BP_LE_SwapSInt_Inp(&pData->type);
   BP_LE_SwapPtr_Inp(&pData->lit);
   BP_LE_SwapSInt_Inp(&pData->pad);
}

static void EndianSwapLITPoint(LIT_POINT* pData)
{
   BP_LE_SwapFloatArray_Inp(&pData->point.vx, 4);
   BP_LE_SwapFloat_Inp(&pData->r_range);
   BP_LE_SwapFloat_Inp(&pData->e_range);
   BP_LE_SwapSInt_Inp(&pData->flag);
}

static void EndianSwapLITSpot(LIT_SPOT* pData)
{
   BP_LE_SwapFloatArray_Inp(&pData->bound_max.vx, 4);
   BP_LE_SwapFloatArray_Inp(&pData->bound_min.vx, 4);
   BP_LE_SwapFloatArray_Inp(&pData->point.vx, 4);
   BP_LE_SwapFloatArray_Inp(&pData->dir.vx, 4);
   BP_LE_SwapFloat_Inp(&pData->umbra);
   BP_LE_SwapFloat_Inp(&pData->penumbra);
   BP_LE_SwapSInt_Inp(&pData->flag);
}

static void EndianSwapLITLine(LIT_LINE* pData)
{
   BP_LE_SwapFloatArray_Inp(&pData->bound_max.vx, 4);
   BP_LE_SwapFloatArray_Inp(&pData->bound_min.vx, 4);
   BP_LE_SwapFloatArray_Inp(&pData->point.vx, 4);
   BP_LE_SwapFloatArray_Inp(&pData->dir.vx, 4);
   BP_LE_SwapFloat_Inp(&pData->r_range);
   BP_LE_SwapSInt_Inp(&pData->pad);
   BP_LE_SwapSInt_Inp(&pData->flag);
}

static void EndianSwapLITBlackpoint(LIT_BLACKPOINT* pData)
{
   BP_LE_SwapFloatArray_Inp(&pData->bound_max.vx, 4);
   BP_LE_SwapFloatArray_Inp(&pData->bound_min.vx, 4);
   BP_LE_SwapFloatArray_Inp(&pData->point.vx, 4);
   BP_LE_SwapFloat_Inp(&pData->r_range);
   BP_LE_SwapSInt_Inp(&pData->flag);
   BP_LE_SwapSInt_Inp(&pData->pad[0]);
   BP_LE_SwapSInt_Inp(&pData->pad[1]);
}

/*----------------------------------------------------------------*/
static void Lt2DataSetup( LIT_DEF *def )
{
	LIT_GRP		*grp ;
	int			i, j ;

   EndianSwapLITDef(def);

	grp = (LIT_GRP*)&def[1] ;
	for ( i = 0 ; i < def->n_lit_group ; grp++, i++ ){
      EndianSwapLITGrp(grp);

      grp->lit = (void*)( (int)grp->lit + (int)def );
		if ( grp->type & LIT_TYPE_POINT ){
			/* 点光源固有初期化 */
			LIT_POINT	*lit = (LIT_POINT*)grp->lit ;
			for ( j = grp->n_lights ; j > 0 ; lit++, j-- ){
            EndianSwapLITPoint(lit);
			}
			
		} else if ( grp->type & LIT_TYPE_SPOT ){
			/* スポット光源固有初期化 */

			LIT_SPOT	*lit = (LIT_SPOT*)grp->lit ;
			for ( j = grp->n_lights ; j > 0 ; lit++, j-- ){
            EndianSwapLITSpot(lit);
				//lit->umbra = cosf( lit->umbra );
				//lit->penumbra = cosf( lit->penumbra );
			}
      } 

      //BP_LIGHTING - endian swap line lights
      else if ( grp->type & LIT_TYPE_LINE )
      {
         LIT_LINE	*lit = (LIT_LINE*)grp->lit ;
         for ( j = grp->n_lights ; j > 0 ; lit++, j-- )
         {
            EndianSwapLITLine(lit);
         }

		} 
      //BP_LIGHTING - endian swap line lights
      
      else if ( grp->type & LIT_TYPE_BLACKPOINT ){
			/* 黒点光源固有初期化 */
			LIT_BLACKPOINT	*lit = (LIT_BLACKPOINT*)grp->lit ;
			for ( j = grp->n_lights ; j > 0 ; lit++, j-- ){
            EndianSwapLITBlackpoint(lit);
#if 0
				printf("bound max:%f %f %f\n",
					   lit->bound_max.vx, lit->bound_max.vx, lit->bound_max.vx );
				printf( "bound min %f %f %f\n",
					   lit->bound_min.vx, lit->bound_min.vx, lit->bound_min.vx );
				printf("pos %f %f %f\n", lit->point.vx, lit->point.vy, lit->point.vz );
				printf("r_range %f flag %08x\n", lit->r_range, lit->flag );
#endif
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

static void EndianSwapEvmSkeleton( EVM_SKEL *pData )
{
   BP_LE_SwapSInt_Inp( &pData->flag );
   BP_LE_SwapSInt_Inp( &pData->parent );
   BP_LE_SwapFloatArray_Inp( &pData->tx, 3 );
   BP_LE_SwapFloatArray_Inp( &pData->rt_tx, 3 );
   BP_LE_SwapFloatArray_Inp( &pData->lx, 4 );
   BP_LE_SwapFloatArray_Inp( &pData->ux, 4 );
}

static void EndianSwapEvmDef( EVM_DEF *def )
{
   int i;
   
   BP_LE_SwapUInt_Inp( &def->n_models );
   BP_LE_SwapUInt_Inp( &def->n_x_models );
   BP_LE_SwapFloat_Inp( &def->lx );
   BP_LE_SwapFloat_Inp( &def->ly );
   BP_LE_SwapFloat_Inp( &def->lz );
   BP_LE_SwapFloat_Inp( &def->ux );
   BP_LE_SwapFloat_Inp( &def->uy );
   BP_LE_SwapFloat_Inp( &def->uz );
   BP_LE_SwapSInt_Inp( &def->type );
   BP_LE_SwapSInt_Inp( &def->texture );
   BP_LE_SwapSInt_Inp( &def->n_packs );
   BP_LE_SwapPtr_Inp( &def->packet );

   for ( i = 0; i < def->n_x_models; ++i )
   {
      EndianSwapEvmSkeleton( def->skeleton + i );
   }
}

static void EndianSwapEvmPacket( EVM_PACK *pack )
{
   BP_LE_SwapUInt_Inp( &pack->flag );
   BP_LE_SwapUInt_Inp( &pack->tex_id[0] );
   BP_LE_SwapUInt_Inp( &pack->tex_id[1] );
   BP_LE_SwapUInt_Inp( &pack->tex_id[2] );
   BP_LE_SwapUInt_Inp( &pack->n_verts );
   BP_LE_SwapUInt_Inp( &pack->n_mats );
   BP_LE_SwapPtr_Inp( &pack->verts );
   BP_LE_SwapPtr_Inp( &pack->norms );
   BP_LE_SwapPtr_Inp( &( pack->uvs[0] ) );
   BP_LE_SwapPtr_Inp( &( pack->uvs[1] ) );
   BP_LE_SwapPtr_Inp( &( pack->uvs[2] ) );
   BP_LE_SwapPtr_Inp( &pack->weight );
   BP_LE_SwapPtr_Inp( &pack->rgbs );
   BP_LE_SwapSInt_Inp( &pack->pad1 );
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

   EndianSwapEvmDef( def );

	def->packet = SET_ADDR( def->packet, def );
	pack = def->packet ;
	for ( i = def->n_packs ; i > 0 ; i--, pack++ ){
		EndianSwapEvmPacket( pack );
      
      pack->verts = SET_ADDR( pack->verts, def );
		pack->norms = SET_ADDR( pack->norms, def );
		pack->uvs[0] = SET_ADDR( pack->uvs[0], def );
		pack->uvs[1] = SET_ADDR( pack->uvs[1], def );
		pack->uvs[2] = SET_ADDR( pack->uvs[2], def );
		pack->weight = SET_ADDR( pack->weight, def );
      // BP - Note - pack->rgbs is never ptr-adjusted here and appears to be unused.
      
      EndianSwapShortVectors( pack->verts, 4, pack->n_verts );
      EndianSwapShortVectors( pack->norms, 4, pack->n_verts );
      EndianSwapShortVectors( pack->uvs[0], 4, pack->n_verts );
      EndianSwapShortVectors( pack->uvs[1], 4, pack->n_verts );
      EndianSwapShortVectors( pack->uvs[2], 4, pack->n_verts );

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
		KmsMakePrePacket( (DG_DEF*) buf );	/* プリパケットのみ生成しなおす必要があるため */
		return (1);
	}
	format_id = BP_LE_SwapUInt( ((DG_DEF*)buf)->data_format );
	if ( format_id != MGS_MODEL_NORM && format_id != MGS_MODEL_MULTITEX &&
		format_id != MGS_MODEL_NORM_A && format_id != MGS_MODEL_MULTITEX_A ){
		printf("file format error!!(%d)\n", format_id );
		return ( 0 );
	}
	KmsDataSetup( (DG_DEF*) buf );
	KmsMakePrePacket( (DG_DEF*) buf );
	//((DG_DEF*)buf)->pad[2] = id ;	/* デバッグ用 */

   BP_BindMGSMesh(BP_GetCurrentAssetCacheTag(), BP_GetCurrentLoadPath(), id & 0x00FFFFFF, (unsigned int)buf);
	return (1);
}

/*----------------------------------------------------------------*/

/* 共有頂点データの初期化 */
int		DG_LoadInitCvd( void *buf, int id )
{
	if ( id & GV_CACHEID_RESIDENT ) return (1);
	if ( BP_LE_SwapUInt( ( (CVD_DEF*)buf )->id ) == 6754556 ){
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
		return (1);
	}
	EvmDataSetup( (EVM_DEF*) buf );
	//MakeSkeletonBound( (EVM_DEF*)buf );

   BP_BindMGSMesh(BP_GetCurrentAssetCacheTag(), BP_GetCurrentLoadPath(), id & 0x00FFFFFF, (unsigned int)buf);

	return (1);
}

//----------------------------------------------------------------------------

static void EndianSwapZarHeaderAndData( ZAR_HEADER *pData )
{
   int i;
   
   BP_LE_SwapSInt_Inp( &pData->format_type );
   BP_LE_SwapSInt_Inp( &pData->version );
   BP_LE_SwapSInt_Inp( &pData->type );
   BP_LE_SwapSInt_Inp( &pData->n_datas );

   for ( i = 0; i < pData->n_datas; ++i )
   {
      BP_LE_SwapSInt_Inp( &( pData->list[i].data_id ) );
      BP_LE_SwapSInt_Inp( &( pData->list[i].data_offset ) );
      BP_LE_SwapSInt_Inp( &( pData->list[i].pad[0] ) );
      BP_LE_SwapSInt_Inp( &( pData->list[i].pad[1] ) );
   }
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
   if ( !( id & GV_CACHEID_RESIDENT ) )
   {
      EndianSwapZarHeaderAndData( header );
   }

	switch ( header->format_type ){
	  case ZAR_FORMAT_KMS:
		for ( i = 0 ; i < header->n_datas ; i++ ){
			DG_DEF	*def ;
			if ( !( id & GV_CACHEID_RESIDENT ) ){
				def = (DG_DEF*)( header->list[ i ].data_offset + (int)buf );
				KmsDataSetup2( def, buf );

            {
               char kmsLoadPath[256];
               char idName[10];

               strcpy(kmsLoadPath, BP_GetCurrentLoadPath());

               sprintf(idName, "%8.8x", id & 0x00FFFFFF);
               strcat(kmsLoadPath, idName);

               strcat(kmsLoadPath, "/");

               BP_BindMGSMesh(BP_GetCurrentAssetCacheTag(), kmsLoadPath, header->list[i].data_id & 0x00FFFFFF, (unsigned int)def);
            }

			} else {
				def = (DG_DEF*)header->list[ i ].data_offset ;
			}
			header->list[ i ].data_offset = (int)def ;
			/* モデルデータとして登録 */
			GV_SetCache( GV_CacheID( header->list[ i ].data_id, 'k' ), def );
		}
		break ;
	}
	return (1);
}



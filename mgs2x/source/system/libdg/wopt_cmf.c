/*
	xopt_cmf.c
	チャンネル処理ユニット／光学迷彩オブジェクトＤＭＡ接続ルーチン

	2002/03/27 K.Takabe
	$Id: wopt_cmf.c,v 1.19 2003/01/05 04:46:33 takaki Exp $

*/
/*

	void		DG_OptCamouflageChainChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた各オブジェクトをＤＭＡパケットに接続する

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"

#include "shader.h"

extern float	DG_BackBufferWidthAdjustCoef ;	// RendWidth/Width
extern float	DG_BackBufferHeightAdjustCoef ;	// RendHeight/Height


/* ---------------------------------------------------------------- */
	/*
		プラグイン固有設定
	*/

/* メインチャンネルプラグイン用設定 */
	/* プラグイン固有ＩＤ */
#define PLUGIN_ID		(7419565)	/* "opt_cmf" */
	/* プラグイン初期化フラグ */
#define PLUGIN_FLAG		(DG_PLUGIN_FLAG_ENABLE)
	/* プラグイン実行フェーズ指定 */
#define PLUGIN_PHASE	(DG_PLUGIN_PHASE_NORMAL)
	/* プラグインプライオリティ */
#define PLUGIN_PRIO		(DG_PLUGIN_PRIO_NORMAL)
	/* 最大使用オブジェクトキューサイズ */
#define PLUGIN_USE_QUEUE	(0)


/* ---------------------------------------------------------------- */
	/*
		構造体宣言
	*/
typedef struct	DG_VERTEX_OPTCMF_
{
	float	x, y, z ;
	float	wt ;
	DWORD	diffuse ;
	float	u0, v0, w0 ;
} DG_VERTEX_OPTCMF ;

#define	DG_VERTEX_OPTCMF_FLAG	( D3DFVF_XYZB1		\
								| D3DFVF_DIFFUSE	\
								| D3DFVF_TEX1		\
								| D3DFVF_TEXCOORDSIZE3(0))

/* ---------------------------------------------------------------- */
	/*
		プラグイン固有変数
	*/

	/* プラグイン初期化フラグ */
static int	PluginStartFlag = 0 ;
static DG_PLUGIN	DmapackPlugin ;

	/*
		プラグイン固有ワーク
	*/


/* ---------------------------------------------------------------- */
	/*
		プラグイン内部使用関数
	*/

static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status );
static void		DG_OptcmfChainChanl( DG_CHANL *cp, int which );
extern void DG_InitOptcmfVertexShader(void);

static	void	CalcKMS2OPTCMF(DG_OBJ *obj, DG_VERTEX_OPTCMF *dst,
							void *src, DWORD stride, int num) ;

/*----------------------------------------------------------------*/

static	void	ChainObj( DG_OBJ *obj ) ;
static	void	ChainObjNIdx( DG_OBJ *obj ) ;		// Index非対応版
static	void	ChainObjNVS( DG_OBJ *obj ) ;		// VertexShader非対応版
static	void	ChainObjNVSNIdx( DG_OBJ *obj ) ;	// VertexShader/Index非対応版
static	void	ChainMaskObjNVS( DG_OBJ *obj ) ;	// [Mask用]VertexShader非対応版
static	void	ChainMaskObjNVSNIdx( DG_OBJ *obj ) ;// [Mask用]VertexShader/Index非対応版
static	void	ChainObjFvf( DG_OBJ *obj ) ;		// FVF頂点版
static	void	ChainObjFvfNIdx( DG_OBJ *obj ) ;	// FVF頂点/Index非対応版

static	void	*_chain_obj_func ;
static	void	*_chain_maskobj_func ;

/*----------------------------------------------------------------*/
	/*
		プラグイン起動
	*/
void DG_AddPluginOptcmf( void )
{
	if ( PluginStartFlag != 0 ) return ;

	/* プラグイン作成 */
	DG_MakePlugin( &DmapackPlugin, PLUGIN_ID, PLUGIN_FLAG, PLUGIN_PHASE, PLUGIN_PRIO, PluginActor, PLUGIN_USE_QUEUE );
	DG_AddPlugin( &DmapackPlugin );
	/*
		プラグイン固有初期化処理
	*/
	DG_InitOptcmfVertexShader();

	PluginStartFlag = 1 ;
}

	/*
		プラグイン終了
	*/
void DG_DeletePluginOptcmf( void )
{
	while ( PluginStartFlag != 0 ){
		DG_DeletePlugin( &DmapackPlugin );
		DG_FreePlugin( &DmapackPlugin );
	}
	PluginStartFlag = 0 ;
}
/* ---------------------------------------------------------------- */
	/*
		プラグイン実行アクター
	*/
static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status )
{
	DG_OptcmfChainChanl( cp, which );
}

/* ---------------------------------------------------------------- */



/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
	/*
		各種ローカル構造体定義
	*/


/*----------------------------------------------------------------*/
	/*
		スクラッチパッドワーク定義
	*/
typedef struct {
	void		*tag ;
	u_int		z ;
	void		*obj ;
} SORT_TAG ;

/* 通常モデル描画用ワーク */
typedef struct {
	DG_OBJS		*objs ;
	int			use_tri ;
} OBJ_LIST ;

typedef struct {
	FMATRIX		eye_pers ;
	FMATRIX		eye_inv ;
	FMATRIX		rot_mat ;
	int			invisible_flag ;
	int			buffer_switch ;
	int			pad[2] ;
	u_long128		dma_buffer[128] ;	/* 汎用ＤＭＡパケット生成バッファ */
	/* オブジェクト状態管理用 */
	FMATRIX		screen ;
	FMATRIX		local_light ;
	FMATRIX		local_color ;
	FMATRIX		connection ;
	int			fog ;
	void		*texture ;

	/* ローカルワーク */
	u_long128	local_work[0] ;
} ScrpadWork ;

/* 汎用メモリワーク（主にスクラッチパッド上で構成し終わったデータなどを退避） */
extern u_long128	DG_LocalMemory[1024] ;


/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
static DG_VERTEXSHADER	DG_OptcmfVertexShader[2] ;
extern unsigned char VERTEX_SHADER_optcmf00[];
void DG_InitOptcmfVertexShader(void)
{
	BOOL	fvf_version ;

	DWORD dwObjDecl_00[] = {
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT4),		/* position, blend */
		D3DVSD_REG(2,  D3DVSDT_SHORT4),		/* normal, dummy */
		D3DVSD_END()
	};
	DWORD dwObjDecl_00_Fvf[] = {
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_FLOAT4),		/* position, blend */
		D3DVSD_REG(2,  D3DVSDT_FLOAT3),		/* normal */
		D3DVSD_END()
	};

	fvf_version = !DG_CheckObjUseVertexShader() ;	// FVF頂点判定(OBJがShader非対応)

	if( DG_CheckOptCmf00UseVertexShader() )
	{
		if( !fvf_version )
		{
			DG_MakeVertexShader( &DG_OptcmfVertexShader[0],
								VSHT_woptcmf00, dwObjDecl_00 );
		}
		else
		{
			/* FVF頂点版 */
			DG_MakeVertexShader( &DG_OptcmfVertexShader[0],
								VSHT_woptcmf00_fvf, dwObjDecl_00_Fvf );
		}
	}

	/* 描画関数の選択 */
	if( DG_CheckOptCmf00UseVertexShader() )
	{
		if( !fvf_version )
		{
			if( DG_CheckIndexPrimitiveUseable() )
			{
				_chain_obj_func     = ChainObj ;
				_chain_maskobj_func = ChainObj ;
			}
			else
			{
				_chain_obj_func     = ChainObjNIdx ;
				_chain_maskobj_func = ChainObjNIdx ;
			}
		}
		else
		{
			/* FVF頂点版 */
			if( DG_CheckIndexPrimitiveUseable() )
			{
				_chain_obj_func     = ChainObjFvf ;
				_chain_maskobj_func = ChainObjFvf ;
			}
			else
			{
				_chain_obj_func     = ChainObjFvfNIdx ;
				_chain_maskobj_func = ChainObjFvfNIdx ;
			}
		}
	}
	else
	{
		/* VertexShader非対応版 */
		if( DG_CheckIndexPrimitiveUseable() )
		{
			_chain_obj_func     = ChainObjNVS ;
			_chain_maskobj_func = ChainMaskObjNVS ;
		}
		else
		{
			_chain_obj_func     = ChainObjNVSNIdx ;
			_chain_maskobj_func = ChainMaskObjNVSNIdx ;
		}
	}

}

/*----------------------------------------------------------------*/
	/*
		パケット初期化関連
	*/



	/*
		ＤＭＡに接続する
	*/
static	void	ChainObj( DG_OBJ *obj )
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
	int           j;
#ifndef _WINDOWS
	unsigned short	*index_addr ;
#endif
#ifdef _WINDOWS
	int			index_ofs ;
#endif
	
	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	if( obj->vbuff_dirty ){ DG_FlushDGObjVertexBuffer(obj) ; }
	DG_SetStaticVertexBuffer( 0, obj->d3d_vbuff,  obj->stride );

#if !__DG_OBJ_VERTEX_WEIGHT_REV__
	if (obj->parent != -1) {
		DG_SetVertexShaderConstant(CV_WORLD1, &obj->inv_mat, 4);
	}
	DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
#else
	if (obj->parent != -1) {
		if ( !(obj->mdl_type & DG_TYPE_REVWT_WT0) ) {
			DG_SetVertexShaderConstant(CV_WORLD0, &obj->inv_mat, 4);
		} else {
			/* 計算誤差を防ぐ為の処置 */
			DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
		}
	} else {
		DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
	}
	DG_SetVertexShaderConstant(CV_WORLD1, &obj->world, 4);
#endif

	DG_SetPixelShader(0);
	DG_SelectVertexShader( &DG_OptcmfVertexShader[0] );

	if( obj->ibuff_dirty ){ DG_FlushDGObjIndexBuffer(obj) ;  }
	DG_SetStaticIndexBuffer(obj->d3d_ibuff, 0) ;
	index_ofs = 0 ;
	for (j = obj->n_packs; j > 0; j --, pkt++) {
		DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
						pkt->idx_vtx_min, pkt->idx_vtx_num,
						index_ofs, pkt->n_indices - 2) ;
		index_ofs += pkt->n_indices ;
	}
}

static	void	ChainObjNIdx( DG_OBJ *obj )
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
	int           j;
#ifndef _WINDOWS
	unsigned short	*index_addr ;
#endif
#ifdef _WINDOWS
	int			index_ofs ;
#endif
	
	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	if( obj->vbuff_dirty  ){ DG_FlushDGObjVertexBuffer(obj) ; }
	DG_SetStaticVertexBuffer( 0, obj->d3d_vbuff,  obj->stride );

#if !__DG_OBJ_VERTEX_WEIGHT_REV__
	if (obj->parent != -1) {
		DG_SetVertexShaderConstant(CV_WORLD1, &obj->inv_mat, 4);
	}
	DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
#else
	if (obj->parent != -1) {
		if ( !(obj->mdl_type & DG_TYPE_REVWT_WT0) ) {
			DG_SetVertexShaderConstant(CV_WORLD0, &obj->inv_mat, 4);
		} else {
			/* 計算誤差を防ぐ為の処置 */
			DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
		}
	} else {
		DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
	}
	DG_SetVertexShaderConstant(CV_WORLD1, &obj->world, 4);
#endif

	DG_SetPixelShader(0);
	DG_SelectVertexShader( &DG_OptcmfVertexShader[0] );

	index_ofs = 0 ;
	for (j = obj->n_packs; j > 0; j --, pkt++) {
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP,
						index_ofs, pkt->n_indices - 2) ;
		index_ofs += pkt->n_indices ;
	}
}

	/*
		ＤＭＡに接続する(FVF頂点版)
	*/
static	void	ChainObjFvf( DG_OBJ *obj )
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
	int           j;
	int			index_ofs ;
	
	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	{
		DWORD	stride ;

		if( obj->stride == sizeof(DG_VERTEX_KMSS) ){ stride = sizeof(DG_VERTEX_KMSS_FVF) ; }
		else{ stride = sizeof(DG_VERTEX_KMSM_FVF) ; }

		if( obj->vbuff_dirty ){ DG_FlushDGObjVertexBufferNVS(obj) ; }
		DG_SetStaticVertexBuffer( 0, obj->d3d_vbuff,  stride );
	}

#if !__DG_OBJ_VERTEX_WEIGHT_REV__
	if (obj->parent != -1) {
		DG_SetVertexShaderConstant(CV_WORLD1, &obj->inv_mat, 4);
	}
	DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
#else
	if (obj->parent != -1) {
		if ( !(obj->mdl_type & DG_TYPE_REVWT_WT0) ) {
			DG_SetVertexShaderConstant(CV_WORLD0, &obj->inv_mat, 4);
		} else {
			/* 計算誤差を防ぐ為の処置 */
			DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
		}
	} else {
		DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
	}
	DG_SetVertexShaderConstant(CV_WORLD1, &obj->world, 4);
#endif

	DG_SetPixelShader(0);
	DG_SelectVertexShader( &DG_OptcmfVertexShader[0] );

	if( obj->ibuff_dirty ){ DG_FlushDGObjIndexBufferNVS(obj) ;  }
	DG_SetStaticIndexBuffer(obj->d3d_ibuff, 0) ;
	index_ofs = 0 ;
	for (j = obj->n_packs; j > 0; j --, pkt++) {
		DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
						pkt->idx_vtx_min, pkt->idx_vtx_num,
						index_ofs, pkt->n_indices - 2) ;
		index_ofs += pkt->n_indices ;
	}
}

static	void	ChainObjFvfNIdx( DG_OBJ *obj )
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
	int           j;
#ifndef _WINDOWS
	unsigned short	*index_addr ;
#endif
#ifdef _WINDOWS
	int			index_ofs ;
#endif
	
	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	{
		DWORD	stride ;

		if( obj->stride == sizeof(DG_VERTEX_KMSS) ){ stride = sizeof(DG_VERTEX_KMSS_FVF) ; }
		else{ stride = sizeof(DG_VERTEX_KMSM_FVF) ; }

		if( obj->vbuff_dirty ){ DG_FlushDGObjVertexBufferNVS(obj) ; }
		DG_SetStaticVertexBuffer( 0, obj->d3d_vbuff,  stride );
	}

#if !__DG_OBJ_VERTEX_WEIGHT_REV__
	if (obj->parent != -1) {
		DG_SetVertexShaderConstant(CV_WORLD1, &obj->inv_mat, 4);
	}
	DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
#else
	if (obj->parent != -1) {
		if ( !(obj->mdl_type & DG_TYPE_REVWT_WT0) ) {
			DG_SetVertexShaderConstant(CV_WORLD0, &obj->inv_mat, 4);
		} else {
			/* 計算誤差を防ぐ為の処置 */
			DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
		}
	} else {
		DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
	}
	DG_SetVertexShaderConstant(CV_WORLD1, &obj->world, 4);
#endif

	DG_SetPixelShader(0);
	DG_SelectVertexShader( &DG_OptcmfVertexShader[0] );

	index_ofs = 0 ;
	for (j = obj->n_packs; j > 0; j --, pkt++) {
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP,
						index_ofs, pkt->n_indices - 2) ;
		index_ofs += pkt->n_indices ;
	}
}

	/*
		ChainObjNVS()系のVertexBufferを設定する
	*/
static __forceinline DWORD	ChainObjNVS_SetVertexBuffer( DG_OBJ *obj )
{
	void	*dst ;
	DWORD	n_verts ;
	DWORD	sofs ;

	n_verts = obj->n_verts ;

	ASSERT( DG_GetWorkBufferSize() > (sizeof(DG_VERTEX_OPTCMF) * n_verts) ) ;
	dst = DG_GetWorkBuffer() ;	// 作業用メモリ

	/*-- CPUで計算 --*/
	CalcKMS2OPTCMF(obj, dst, obj->vbuff, obj->stride, n_verts) ;

	/*-- Stream設定 --*/
	DG_SetVertexShader(DG_VERTEX_OPTCMF_FLAG) ;
	DG_SetDynamicVertexBuffer(dst, sizeof(DG_VERTEX_OPTCMF), n_verts, &sofs) ;

	return(sofs) ;
}

static __forceinline DWORD	ChainObjNVSNIdx_SetVertexBuffer( DG_OBJ *obj )
{
	void	*dst ;
	void	*tmp_dst ;
	DWORD	n_verts ;
	DWORD	n_indices ;
	DWORD	sofs ;

	n_verts   = obj->n_verts ;
	n_indices = obj->n_indices ;

	ASSERT( DG_GetWorkBufferSize() > (sizeof(DG_VERTEX_OPTCMF) * (n_verts + n_indices)) ) ;
	dst = DG_GetWorkBuffer() ;	// 作業用メモリ

	/*-- CPUで計算 --*/
	tmp_dst = (BYTE *)dst + (sizeof(DG_VERTEX_OPTCMF) * n_indices) ;
	CalcKMS2OPTCMF(obj, tmp_dst, obj->vbuff, obj->stride, n_verts) ;

	/*-- 並べ替え --*/
	DG_ArrangeVertexByIndex(dst, tmp_dst, sizeof(DG_VERTEX_OPTCMF),
						obj->index, n_indices) ;

	/*-- Stream設定 --*/
	DG_SetVertexShader(DG_VERTEX_OPTCMF_FLAG) ;
	DG_SetDynamicVertexBuffer(dst, sizeof(DG_VERTEX_OPTCMF), n_indices, &sofs) ;

	return(sofs) ;
}

	/*
		ＤＭＡに接続する(VertexShader非対応版)
	*/
static	void	ChainObjNVS( DG_OBJ *obj )
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
	int           j;
	DWORD		sofs ;
#ifndef _WINDOWS
	unsigned short	*index_addr ;
#endif
#ifdef _WINDOWS
	int			index_ofs ;
#endif
	
	pkt = obj->packets ;
	if (pkt == NULL) return;

	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	DG_SetPixelShader(0);

	sofs = ChainObjNVS_SetVertexBuffer(obj) ;

#if !__DG_OBJ_VERTEX_WEIGHT_REV__
	if (obj->parent != -1) {
		DG_SetTransform(D3DTS_WORLDMATRIX(1), (FMATRIX *)&obj->inv_mat) ;
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS) ;
	}
	else
	{
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE) ;
	}
	DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->world) ;
#else
	if (obj->parent != -1) {
		DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->inv_mat) ;
		DG_SetTransform(D3DTS_WORLDMATRIX(1), (FMATRIX *)&obj->world) ;
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS) ;
	}
	else
	{
		DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->world) ;
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE) ;
	}
#endif

	if( obj->ibuff_dirty ){ DG_FlushDGObjIndexBufferNVS(obj) ;  }
	DG_SetStaticIndexBuffer(obj->d3d_ibuff, sofs) ;
	index_ofs = 0 ;
	for (j = obj->n_packs; j > 0; j --, pkt++) {
		DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
						pkt->idx_vtx_min, pkt->idx_vtx_num,
						index_ofs, pkt->n_indices - 2) ;
		index_ofs += pkt->n_indices ;
	}
}

static	void	ChainObjNVSNIdx( DG_OBJ *obj )
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
	int           j;
#ifndef _WINDOWS
	unsigned short	*index_addr ;
#endif
#ifdef _WINDOWS
	int			index_ofs ;
#endif
	
	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	DG_SetPixelShader(0);

	index_ofs = ChainObjNVSNIdx_SetVertexBuffer(obj) ;

#if !__DG_OBJ_VERTEX_WEIGHT_REV__
	if (obj->parent != -1) {
		DG_SetTransform(D3DTS_WORLDMATRIX(1), (FMATRIX *)&obj->inv_mat) ;
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS) ;
	}
	else
	{
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE) ;
	}
	DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->world) ;
#else
	if (obj->parent != -1) {
		DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->inv_mat) ;
		DG_SetTransform(D3DTS_WORLDMATRIX(1), (FMATRIX *)&obj->world) ;
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS) ;
	}
	else
	{
		DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->world) ;
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE) ;
	}
#endif

	//index_ofs = 0 ;
	for (j = obj->n_packs; j > 0; j --, pkt++) {
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP,
						index_ofs, pkt->n_indices - 2) ;
		index_ofs += pkt->n_indices ;
	}
}

	/*
		ChainMaskObjNVS()系のVertexBufferを設定する
	*/
static __forceinline void	ChainMaskObjNVS_SetVertexBuffer( DG_OBJ *obj )
{
	/* VertexBuffer変更判定 */
	if( obj->cvbuff_dirty ){ DG_FlushDGObjCVertexBuffer(obj) ; }
	if( obj->vbuff_dirty  ){ DG_FlushDGObjVertexBufferNVS(obj) ; }

	if( obj->stride == sizeof(DG_VERTEX_KMSS) )
	{
		if ( obj->flag & DG_FLAG_PAINT )
		{
			/* プリシェードライティング */
			DG_SetStaticVertexBuffer(0, obj->d3d_cvbuff, sizeof(DG_VERTEX_KMSS_FVF)) ;
		}
		else
		{
			DG_SetStaticVertexBuffer(0, obj->d3d_vbuff, sizeof(DG_VERTEX_KMSS_FVF)) ;
		}

		/* FVF設定 */
		DG_SetVertexShader(D3DFVF_DG_VERTEX_KMSS_FLAG) ;
	}
	else
	{
		if ( obj->flag & DG_FLAG_PAINT )
		{
			/* プリシェードライティング */
			DG_SetStaticVertexBuffer(0, obj->d3d_cvbuff, sizeof(DG_VERTEX_KMSM_FVF)) ;
		}
		else
		{
			DG_SetStaticVertexBuffer(0, obj->d3d_vbuff, sizeof(DG_VERTEX_KMSM_FVF)) ;
		}

		/* FVF設定 */
		DG_SetVertexShader(D3DFVF_DG_VERTEX_KMSM_FLAG) ;
	}
}
	/*
		ＤＭＡに接続する(VertexShader非対応版)
	*/
static	void	ChainMaskObjNVS( DG_OBJ *obj )
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
	int           j;
#ifndef _WINDOWS
	unsigned short	*index_addr ;
#endif
#ifdef _WINDOWS
	int			index_ofs ;
#endif
	
	pkt = obj->packets ;
	if (pkt == NULL) return;

	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	DG_SetPixelShader(0);

	ChainMaskObjNVS_SetVertexBuffer(obj) ;

#if !__DG_OBJ_VERTEX_WEIGHT_REV__
	if (obj->parent != -1) {
		DG_SetTransform(D3DTS_WORLDMATRIX(1), (FMATRIX *)&obj->inv_mat) ;
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS) ;
	}
	else
	{
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE) ;
	}
	DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->world) ;
#else
	if (obj->parent != -1) {
		DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->inv_mat) ;
		DG_SetTransform(D3DTS_WORLDMATRIX(1), (FMATRIX *)&obj->world) ;
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS) ;
	}
	else
	{
		DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->world) ;
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE) ;
	}
#endif

	if( obj->ibuff_dirty ){ DG_FlushDGObjIndexBufferNVS(obj) ;  }
	DG_SetStaticIndexBuffer(obj->d3d_ibuff, 0) ;
	index_ofs = 0 ;
	for (j = obj->n_packs; j > 0; j --, pkt++) {
		DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
						pkt->idx_vtx_min, pkt->idx_vtx_num,
						index_ofs, pkt->n_indices - 2) ;
		index_ofs += pkt->n_indices ;
	}
}

static	void	ChainMaskObjNVSNIdx( DG_OBJ *obj )
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
	int           j;
#ifndef _WINDOWS
	unsigned short	*index_addr ;
#endif
#ifdef _WINDOWS
	int			index_ofs ;
#endif
	
	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	DG_SetPixelShader(0);

	ChainMaskObjNVS_SetVertexBuffer(obj) ;

#if !__DG_OBJ_VERTEX_WEIGHT_REV__
	if (obj->parent != -1) {
		DG_SetTransform(D3DTS_WORLDMATRIX(1), (FMATRIX *)&obj->inv_mat) ;
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS) ;
	}
	else
	{
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE) ;
	}
	DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->world) ;
#else
	if (obj->parent != -1) {
		DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->inv_mat) ;
		DG_SetTransform(D3DTS_WORLDMATRIX(1), (FMATRIX *)&obj->world) ;
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS) ;
	}
	else
	{
		DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->world) ;
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE) ;
	}
#endif

	index_ofs = 0 ;
	for (j = obj->n_packs; j > 0; j --, pkt++) {
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP,
						index_ofs, pkt->n_indices - 2) ;
		index_ofs += pkt->n_indices ;
	}
}
/*----------------------------------------------------------------*/
	/*
		ChainObjs初期化
	*/
static	void	InitChainObjs( DG_OBJS *objs )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;

	if ( objs->extend_data == NULL ){
		printf("opt_cmf.c:error\n");
		return ;
	}

	scrpad->local_color = *(FMATRIX*)objs->extend_data ;
	scrpad->rot_mat = ((FMATRIX*)objs->extend_data)[1] ;

	/* 光学迷彩用パラメータをＸＢＯＸ用にコンバートして頂点シェーダー定数として登録 */
	_sceVu0MulMatrix( &scrpad->local_light, &scrpad->rot_mat, &scrpad->eye_inv );
	DG_SetVertexShaderConstant(CV_OPTCMF_EYEPERS, &scrpad->local_light, 4 );
	{/* 光学迷彩化用パラメータ設定 */
#ifndef _WINDOWS
		//scrpad->local_color.m[0][0] = ( DRAW_WIDTH - 32 ) / 2.0f ;
		scrpad->local_color.m[0][1] *= -1.0f ;
		scrpad->local_color.m[1][0] += ( DISPLAY_WIDTH - DRAW_WIDTH ) / 2 ;
		scrpad->local_color.m[1][1] += ( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2 ;
		//scrpad->local_color.m[2][0] = 16.0f ;
		//scrpad->local_color.m[2][1] = 8.0f ;
#else
		float	coef_w, coef_h ;

		coef_w = DG_BackBufferWidthAdjustCoef / (float)DRAW_WIDTH ;
		coef_h = DG_BackBufferHeightAdjustCoef / (float)DRAW_HEIGHT ;
		scrpad->local_color.m[0][0] *= coef_w ;
		scrpad->local_color.m[0][1] *= -coef_h ;
		scrpad->local_color.m[1][0] *= coef_w ;
		scrpad->local_color.m[1][1] *= coef_h ;
		scrpad->local_color.m[2][0] *= coef_w ;
		scrpad->local_color.m[2][1] *= coef_h ;
#endif
		DG_SetVertexShaderConstant( CV_LIGHTCOL, &scrpad->local_color, 4 );
	}
}
	/*
		ＤＭＡに接続する
	*/
static	void	ChainObjs( DG_OBJS *objs )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJ		*obj ;
	int			i ;
	void		(*chain_obj_func)(DG_OBJ *obj) ;

	/*-- 描画ルーチン選択 ----------------------------------------*/

	chain_obj_func     = _chain_obj_func ;
	/*------------------------------------------------------------*/

	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		/* クリップアウトチェック */
		if ( obj->bound_mode & 2 ) continue ;
		if ( obj->model->type & DG_TYPE_TRANS ) continue ;
		if ( obj->flag & scrpad->invisible_flag ) continue ;

		chain_obj_func( obj );
	}

}

static	void	ChainMaskObjs( DG_OBJS *objs )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJ		*obj ;
	int			i ;
	void		(*chain_obj_func)(DG_OBJ *obj) ;

	/*-- 描画ルーチン選択 ----------------------------------------*/

	chain_obj_func     = _chain_maskobj_func ;
	/*------------------------------------------------------------*/

	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		/* クリップアウトチェック */
		if ( obj->bound_mode & 2 ) continue ;
		if ( obj->model->type & DG_TYPE_TRANS ) continue ;
		if ( obj->flag & scrpad->invisible_flag ) continue ;

		chain_obj_func( obj );
	}

}

/*----------------------------------------------------------------*/
static void DG_InitChanlRenderState( DG_CHANL *cp )
{
	static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{1.0f/4096.0f, 1.0f/255.0f, 1.0f/32767.0f, 0},
	};

	// カメラ/透視変換マトリクスの設定
	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);

	/* Vertex Processing設定 */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckObjVertexBufferSoftProcessing()) ;

	/* 頂点シェーダー用各種定数設定 */
	if( DG_CheckOptCmf00UseVertexShader() )
	{
		DG_SetVertexShaderConstant(CV_EYEPERS, &cp->eye_xpers, 4);
		//DG_SetVertexShaderConstant(CV_PERS, &cp->xpers, 4);
		DG_SetVertexShaderConstant(CV_EYE_INV, &cp->eye_inv, 4);
		DG_SetVertexShaderConstant(CV_OPTCMF_EYEPERS, &cp->eye_pers, 4);

		DG_SetVertexShaderConstant(CV_ZERO, &VS_Const[0], 1);
		DG_SetVertexShaderConstant(CV_ONE, &VS_Const[1], 1);
		DG_SetVertexShaderConstant(CV_HALF, &VS_Const[2], 1);
		DG_SetVertexShaderConstant(CV_SCALE, &VS_Const[3], 1);
		DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);

		{/* 光学迷彩化用パラメータ設定 */
			FVECTOR		tex_param[2] ;
			tex_param[0].vx = DRAW_WIDTH / 2.0f ;
			tex_param[0].vy = -DRAW_HEIGHT / 2.0f ;
			tex_param[0].vz = 0.0f ;
			tex_param[0].vw = 0.0f ;
			tex_param[1].vx = DRAW_WIDTH / 2.0f + ( DISPLAY_WIDTH - DRAW_WIDTH ) / 2 ;
			tex_param[1].vy = DRAW_HEIGHT / 2.0f + ( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2 ;
			tex_param[1].vz = 0.0f ;
			tex_param[1].vw = 0.0f ;

			DG_SetVertexShaderConstant(CV_TEX0_SCALE, tex_param, 2 );
		}

		/* Fog 初期化 */
		{
			float fog_start = 0.0f;
			float fog_end   = 1.0f;
			DG_SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog_start);
			DG_SetRenderState(D3DRS_FOGEND,   *(DWORD *)&fog_end);
		}
	}
	else
	{
		/* Vertex Shader非対応版 */
		DG_SetRenderState(D3DRS_LIGHTING, FALSE) ;
		DG_SetRenderState(D3DRS_FOGSTART, *((DWORD *)&DG_FogStart)) ;
		DG_SetRenderState(D3DRS_FOGEND,   *((DWORD *)&DG_FogEnd)) ;

		DG_SetTransform(D3DTS_PROJECTION, &cp->xpers );
		DG_SetTransform(D3DTS_VIEW,       &cp->eye_inv );

		/* Texture投影 */
		DG_SetTransformTexCoord(D3DTS_TEXTURE0, &DG_UnitMatrix) ;
		DG_SetTextureStageState(0,
				D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED|D3DTTFF_COUNT3) ;
	}
}
/*----------------------------------------------------------------*/

	/*
		キューされた各オブジェクトをＤＭＡパケットに接続する
	*/
static void		DG_OptcmfChainChanl( DG_CHANL *cp, int which )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		**oque, *objs ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		i, exec_flag = 0, max_objs ;
	int		c_gid, gid ;
	int		last_semi_trans = 0 ;
	OBJ_LIST		*obj_list, *mem_obj_list ;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;
	obj_buff = &que->objs_buffer ;

	c_gid = DG_CurrentGroupID ;
	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
	scrpad->buffer_switch = 0 ;
#ifndef _WINDOWS
	scrpad->eye_pers = cp->raise_eye_pers ;
#else
	scrpad->eye_pers = cp->raise_eye_xpers ;
#endif
	scrpad->eye_inv = cp->eye_inv ;

	/* 描画オブジェクトをスクラッチパッドへ */
	obj_list = (OBJ_LIST*)scrpad->local_work ;
	obj_buff = &que->objs_buffer ;
	oque = (DG_OBJS**)obj_buff->queue ;
	for ( max_objs = 0, i = obj_buff->n_queue ; i > 0 ; -- i, oque++ ) {
		objs = *oque ;
		if ( !( objs->flag & ( DG_FLAG_OPTCMF & DG_FLAG_PLUGINMASK ) ) ) continue ;
		if ( objs->flag & DG_FLAG_PAINT ) continue ;
		if ( objs->flag & scrpad->invisible_flag ) continue ;

		/* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
		if ( !( objs->group_id & DG_CurrentGroupID ) ) continue ;

		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		if ( objs->bound_mode == 2 ) continue ;
		obj_list[max_objs].objs = objs ;
		max_objs++ ;
	}
	if ( max_objs == 0 ) return ;

	/* スクラッチパッド上のオブジェクトリストをメモリに書き出す */
	mem_obj_list = (OBJ_LIST*)DG_LocalMemory ;
	for ( i = 0 ; i < max_objs ; i++ ) {
		mem_obj_list->objs = obj_list[i].objs ;
		mem_obj_list++ ;
	}
	mem_obj_list = (OBJ_LIST*)DG_LocalMemory ;

	/* スタティックプッシュバッファへの記録開始 */
	DG_OpenDmaTask();

	{/* ＶＲＡＭの内容待避パケット生成 */
		void	*packet, *packet_top ;
		packet_top = packet = (void*)scrpad->dma_buffer ;
		packet = DG_SetDmapackBackupFrame( packet, 0 );
		packet = DG_SetDmapackEnd( packet );
		DG_ExecAuto2DPrim( packet_top );
	}

	DG_InitChanlRenderState( cp );

	/* 描画オブジェクトをスクラッチパッドへ */
	for ( i = max_objs ; i > 0 ; -- i, mem_obj_list++ ) {
		objs = mem_obj_list->objs ;
		if ( !( objs->flag & ( DG_FLAG_OPTCMF & DG_FLAG_PLUGINMASK ) ) ) continue ;
		if ( objs->flag & DG_FLAG_PAINT ) continue ;
		if ( objs->flag & scrpad->invisible_flag ) continue ;

		/* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
		if ( !( objs->group_id & DG_CurrentGroupID ) ) continue ;

		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		if ( objs->bound_mode == 2 ) continue ;

		/* 同じモデルをテクスチャを変えて２回連続で描画 */
		InitChainObjs( objs ) ;
		DG_SetTexture( 0, NULL );
		DG_SetAlphaMode( SCE_GS_SET_ALPHA(2,2,2,1,128) );	
		ChainMaskObjs( objs );
		DG_SetTextureDirect( 0, DG_SystemTexture[ 2 ].tex_trans.ptex );
		DG_SetAlphaMode( SCE_GS_SET_ALPHA(0,1,0,1,0) );	
		ChainObjs( objs );
		exec_flag = 1 ;

#if FALSE
		if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
#endif
	}

#if 0
	/* 半透明処理無効化 */
	if ( last_semi_trans != 0 ){
		size = DG_WriteRasterMaskPacks( scrpad->dma_buffer, 0 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
	}
#endif

	/* 描画環境復帰 */
	DG_SetTextureStageState(0,
			D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2) ;

	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();


}

/*----------------------------------------------------------------*/

	/*
		頂点作成(Vertex Shader非対応版)
	*/
static	void	CalcKMS2OPTCMF(DG_OBJ *obj, DG_VERTEX_OPTCMF *dst,
							void *src, DWORD stride, int num)
{
	int					i ;
	ScrpadWork			*scrpad = (ScrpadWork *)SCRPAD_ADDR ;
	DG_VERTEX_OPTCMF	*dst_vtx ;
	DG_VERTEX_KMSS		*src_vtx ;
	short				wt ;
	FVECTOR				cpos ;
	FVECTOR				cnrm ;
	FVECTOR				pos ;
	FVECTOR				nrm ;
	FVECTOR				vec ;
	FMATRIX				pew0_mtx, pew1_mtx ;
	FMATRIX				ew0_mtx,  ew1_mtx ;
	float				coef ;
	float				u, v ;
	FMATRIX				light_col ;
	DWORD				col ;

	_sceVu0MulMatrix(&pew0_mtx, &scrpad->eye_pers, &obj->world) ;	// Screen <-- Local0
	_sceVu0MulMatrix(&pew1_mtx, &scrpad->eye_pers, &obj->inv_mat) ;	// Screen <-- Local1
	_sceVu0MulMatrix(&ew0_mtx, &scrpad->eye_inv, &obj->world) ;		// Camera <-- Local0
	_sceVu0MulMatrix(&ew1_mtx, &scrpad->eye_inv, &obj->inv_mat) ;	// Camera <-- Local1

	light_col = scrpad->local_color ;

	col = (((DWORD)light_col.m[3][3]) << 24)
		| (((DWORD)light_col.m[3][0]) << 16)
		| (((DWORD)light_col.m[3][1]) << 8)
		| ((DWORD)light_col.m[3][2]) ;			// 各数値が0.0f～255.0fであると仮定

	dst_vtx = dst ;
	src_vtx = (DG_VERTEX_KMSS *)src ;
	for(i=num; i>0; i--, dst_vtx++, (BYTE *)src_vtx+=stride)
	{
		/*-- 頂点座標計算 --------------------------------------------------*/

		pos.vx = (float)src_vtx->vx ;
		pos.vy = (float)src_vtx->vy ;
		pos.vz = (float)src_vtx->vz ;
		pos.vw = 1.0f ;

		nrm.vx = (float)src_vtx->nx ;
		nrm.vy = (float)src_vtx->ny ;
		nrm.vz = (float)src_vtx->nz ;
		nrm.vw = 0.0f ;

		dst_vtx->x = pos.vx ;
		dst_vtx->y = pos.vy ;
		dst_vtx->z = pos.vz ;

		wt = src_vtx->wt ;
		if( wt == 0 )
		{
			dst_vtx->wt = 0.0f ;
#if !__DG_OBJ_VERTEX_WEIGHT_REV__
			_sceVu0ApplyMatrix(&cpos, &pew1_mtx, &pos) ;
			_sceVu0ApplyMatrix(&cnrm, &ew1_mtx, &nrm) ;
#else
			_sceVu0ApplyMatrix(&cpos, &pew0_mtx, &pos) ;
			_sceVu0ApplyMatrix(&cnrm, &ew0_mtx, &nrm) ;
#endif
		}
		else if( wt == 32767 )
		{
			dst_vtx->wt = 1.0f ;
#if !__DG_OBJ_VERTEX_WEIGHT_REV__
			_sceVu0ApplyMatrix(&cpos, &pew0_mtx, &pos) ;
			_sceVu0ApplyMatrix(&cnrm, &ew0_mtx, &nrm) ;
#else
			_sceVu0ApplyMatrix(&cpos, &pew1_mtx, &pos) ;
			_sceVu0ApplyMatrix(&cnrm, &ew1_mtx, &nrm) ;
#endif
		}
		else
		{
			float	fwt, inv_fwt ;

			fwt     = (float)wt * (1.0f/32767.0f) ;
			inv_fwt = 1.0f - fwt ;
			dst_vtx->wt = fwt ;

#if !__DG_OBJ_VERTEX_WEIGHT_REV__
			_sceVu0ApplyMatrix(&vec,  &pew0_mtx, &pos) ;
			_sceVu0ApplyMatrix(&cpos, &pew1_mtx, &pos) ;
#else
			_sceVu0ApplyMatrix(&vec,  &pew1_mtx, &pos) ;
			_sceVu0ApplyMatrix(&cpos, &pew0_mtx, &pos) ;
#endif
			cpos.vx = (vec.vx * fwt) + (cpos.vx * inv_fwt) ;
			cpos.vy = (vec.vy * fwt) + (cpos.vy * inv_fwt) ;
			cpos.vz = (vec.vz * fwt) + (cpos.vz * inv_fwt) ;
			cpos.vw = (vec.vw * fwt) + (cpos.vw * inv_fwt) ;

#if !__DG_OBJ_VERTEX_WEIGHT_REV__
			_sceVu0ApplyMatrix(&vec,  &ew0_mtx, &nrm) ;
			_sceVu0ApplyMatrix(&cnrm, &ew1_mtx, &nrm) ;
#else
			_sceVu0ApplyMatrix(&vec,  &ew1_mtx, &nrm) ;
			_sceVu0ApplyMatrix(&cnrm, &ew0_mtx, &nrm) ;
#endif
			cnrm.vx = (vec.vx * fwt) + (cnrm.vx * inv_fwt) ;
			cnrm.vy = (vec.vy * fwt) + (cnrm.vy * inv_fwt) ;
			cnrm.vz = (vec.vz * fwt) + (cnrm.vz * inv_fwt) ;
			//cnrm.vw = (vec.vw * fwt) + (cnrm.vw * inv_fwt) ;
			cnrm.vw = 0.0f ;
		}
		_sceVu0Normalize(&cnrm, &cnrm) ;	// 法線正規化
		/*------------------------------------------------------------------*/

		/*-- Texture座標計算 -----------------------------------------------*/

		coef = 1.0f/cpos.vw ;
		
		u = cpos.vx * coef ;	// 投影
		v = cpos.vy * coef ;

		u = ((u * light_col.m[0][0]) + light_col.m[1][0]) + (cnrm.vx * light_col.m[2][0]) ;
		v = ((v * light_col.m[0][1]) + light_col.m[1][1]) + (cnrm.vy * light_col.m[2][1]) ;

		dst_vtx->u0 = u * cpos.vw ;
		dst_vtx->v0 = v * cpos.vw ;
		dst_vtx->w0 = cpos.vw ;
		/*------------------------------------------------------------------*/

		dst_vtx->diffuse = col ;
	}
}

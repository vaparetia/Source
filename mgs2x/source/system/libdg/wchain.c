/*
	wchain.c
	チャンネル処理ユニット／オブジェクトＤＭＡ接続ルーチン

	2002/02/09 K.Takabe
	$Id: wchain.c,v 1.34 2003/01/05 04:46:34 takaki Exp $

*/
/*

	void		DG_ChainChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた各オブジェクトをＤＭＡパケットに接続する


	void		DG_SortChainChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた各オブジェクトをソートしてＤＭＡパケットに接続する
	同時にソート済みのプリミティブオブジェクトも表示する

*/


#ifdef _XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"
#include "private.h"
#include "shader.h"
#include "wexebuffer.h"

#include	"def_dma.h"

//void *DG_Prim2OT[64];

#define _CopyVector( a, b ) { *(u_long128*)a = *(u_long128*)b ; }
#define _CopyMatrix( a, b ) { 0[(u_long128*)a] = 0[(u_long128*)b] ;\
								1[(u_long128*)a] = 1[(u_long128*)b] ;\
								2[(u_long128*)a] = 2[(u_long128*)b] ;\
								3[(u_long128*)a] = 3[(u_long128*)b] ; }

#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)
#define SIZEOF_WORD(_v)	(sizeof(_v)/sizeof(int))
#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))

#define GS_REGS_3(r0, r1, r2) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08)
#define GS_REGS_5(r0, r1, r2, r3, r4) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c | (r4) << 0x10)

/*----------------------------------------------------------------*/

#define NEXT_BUFF()	({ scrpad->buffer_switch = 1 - scrpad->buffer_switch ; (&scrpad->pre_packet[ scrpad->buffer_switch ] ) ;})
#ifndef MGS_MASTER
//#define	MARK(n)		( *(volatile char**)&GV_DebugMes = (char*)n )
#endif

/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
	/*
		スクラッチパッドワーク定義
	*/
/* ソート用ワーク */
typedef struct {
	void		*tag ;
	u_int		z ;
	void		*obj ;
} SORT_TAG ;
typedef struct {
	SORT_TAG	list[768] ;
	void		*ot3[64] ;
	void		*ot2[64] ;
	void		*ot1[64] ;
} SORT_WORK ;
/* 通常モデル描画用ワーク */
typedef struct {
	DG_OBJS		*objs ;
	int			use_tri ;
} OBJ_LIST ;
/* オブジェクトマトリクスストア用ワーク */
typedef struct {
	FMATRIX			world ;
	FMATRIX			screen ;
	FMATRIX			inv_mat ;
} OBJ_MATRIX ;
typedef struct {
	u_long128		*current_matrix_addr ;
	int				pad[3] ;	
	OBJ_MATRIX		buffer[2][8] ;
	u_long128		local_work[0] ;
} MATRIXSTORE_WORK ;


/* ベーススクラッチパッドワーク定義 */
typedef struct {
	/* ＤＭＡ生成用バッファ */
	u_long128		dma_buffer[128] ;	/* 汎用ＤＭＡパケット生成バッファ */
	/* オブジェクト状態管理用 */
	FMATRIX		screen ;
	FMATRIX		local_light ;
	FMATRIX		local_color ;
	FMATRIX		connection ;
	FMATRIX		ir_light_mat[2] ;
	int			fog ;
	float		fog_param1, fog_param2 ;
	short		vu_prog_list[16] ;
	FMATRIX		eye_pers ;
	FMATRIX		eye_inv ;
	/* 各種ワーク */
	int			max_num ;
	void		*now ;
	int			invisible_flag ;
	int			buffer_switch ;
	void		*last_light_addr ;		/* 最後に設定したライトマトリクスへのアドレス */
	/* StoreMatrixObjs()関数用ローカル変数 */
	u_long128	*matrix_store_addr ;
	/* ChainObj()関数用ローカル変数 */
	void		*matrix_addr ;

	/* デバッグ用 */
	DG_OBJS		*current_objs ;
#ifdef LIBDG_PERFORMANCE
	PERFORMANCE_PACKET_INFO	model[ 2 ] ;
	int			verts_count ;
	int			pack_count ;
#endif
	/* ローカルワーク */
	u_long128	local_work[0] ;
} ScrpadWork ;

/* 汎用メモリワーク（主にスクラッチパッド上で構成し終わったデータなどを退避） */
//u_long128	DG_LocalMemory[1024] ;
u_long	DG_LocalMemory[ 1024 * 2 ];

/*----------------------------------------------------------------*/

#define Vu0CopyMatrix( _a, _b ) { *(_a) = *(_b) ; }

/*----------------------------------------------------------------*/

static	void	ChainObj( DG_OBJ *obj ) ;
static	void	ChainObjNIdx( DG_OBJ *obj ) ;		// Index非対応版
static	void	ChainObjNVS( DG_OBJ *obj ) ;		// VertexShader非対応版
static	void	ChainObjNVSNIdx( DG_OBJ *obj ) ;	//	VertexShader/Index非対応版

static	void	*_chain_obj_func ;
static	void	*_write_prim_packs_list_func ;

/*----------------------------------------------------------------*/
static inline void SetLightMatrix( FMATRIX *res_light, FMATRIX *light, FMATRIX *world )
{
	FMATRIX		tmp_mat ;
	tmp_mat = *world ;
	tmp_mat.m[3][0] = 0 ;
	tmp_mat.m[3][1] = 0 ;
	tmp_mat.m[3][2] = 0 ;
	_sceVu0MulMatrix( res_light, light, &tmp_mat );
}


/*----------------------------------------------------------------*/
/*
	ピクセルシェーダー関連
*/
#if 0
DWORD DG_PixelShaderHandle[DG_PSHADER_NUM];
static char *DG_PixelShaderName[DG_PSHADER_NUM] = {
	"",
	"plight",  // ポイントライト処理シェーダ
	//"normal",  // 通常のシェーダ
	"test",	   // テスト用
};

// 常駐ピクセルシェーダ初期化/ピクセルシェーダは使うかどうか謎
void DG_InitPixelShader(void)
{
	int i;
	// 0番目はデフォルトシェーダ
	DG_PixelShaderHandle[DG_PSHADER_NONE] = 0;

	// シェーダのロード
	for (i = 1; i < DG_PSHADER_NUM; i++) {
		DG_CreatePixelShader(GV_StrCode(DG_PixelShaderName[i]),
							 &DG_PixelShaderHandle[i]);
	}
}
#else
void DG_InitPixelShader(void)
{
	PShInfoInit() ;
}

void DG_ReleasePixelShader(void)
{
	PShInfoDest() ;
}
#endif

/*
 頂点シェーダ場合分け
 分岐の要素は以下の通り。

 ・平行光源計算
 ・ウェイト計算
 ・追加光源計算
 ・点光源計算
 ・点光源処理でピクセルシェーダを使うかどうか

 まじめに全部作ると32通りになってしまうので、

 ・平行光源処理+点光源処理は同じシェーダで行う。
   点光源無効OBJの場合は点光源色を(0,0,0)にしておく

 ・点光源計算をした場合、ピクセルシェーダに渡す値も一緒に書き込む

 という制限をつける。
*/

extern DG_PIXELSHADER	DG_MultiTexPixelShader[DG_MULTITEX_MAXNUM] ;

DG_VERTEXSHADER	DG_ObjVertexShader[2];
void DG_InitObjVertexShader(void)
{
	DWORD dwObjDecl_00[] = {
		/* 通常ライティング */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT4),		/* position, blend */
		D3DVSD_REG(2,  D3DVSDT_SHORT4),		/* normal, dummy */
		D3DVSD_REG(7,  D3DVSDT_SHORT2),		/* uv0 */
//		D3DVSD_REG(8,  D3DVSDT_SHORT2),		/* uv1 */
//		D3DVSD_REG(9,  D3DVSDT_SHORT2),		/* uv2 */
		D3DVSD_END()
	};
	DWORD dwObjDecl_01[] = {
		/* プリシェードライティング */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT4),	/* position, blend */
		D3DVSD_REG(2,  D3DVSDT_SHORT4),	/* normal,dummy */
		D3DVSD_REG(7,  D3DVSDT_SHORT2),	/* uv0 */
//		D3DVSD_REG(8,  D3DVSDT_SHORT2),	/* uv1 */
//		D3DVSD_REG(9,  D3DVSDT_SHORT2),	/* uv2 */
		D3DVSD_STREAM(1),
		D3DVSD_REG(3,  D3DVSDT_D3DCOLOR),	/* diffuse */
		D3DVSD_END()
	};

	if( DG_CheckObjUseVertexShader() )
	{
		/* 通常ライティング */
		DG_MakeVertexShader( &DG_ObjVertexShader[0], VSHT_wkms_sl, dwObjDecl_00 );
		/* プリシェードライティング */
		DG_MakeVertexShader( &DG_ObjVertexShader[1], VSHT_wkms_sp, dwObjDecl_01 );
	}

	/* 描画関数選択 */
	if( DG_CheckObjUseVertexShader() )
	{
		if( DG_CheckIndexPrimitiveUseable() ){ _chain_obj_func = ChainObj ; }
		else{ _chain_obj_func = ChainObjNIdx ; }
	}
	else
	{
		/* VertexShader非対応版 */
		if( DG_CheckIndexPrimitiveUseable() ){ _chain_obj_func = ChainObjNVS ; }
		else{ _chain_obj_func = ChainObjNVSNIdx ; }
	}

	if( DG_CheckUseVertexShader() )		// 注意!! DG_CheckObjUseVertexShader()では無い
	{
		_write_prim_packs_list_func = DG_WritePrimPacksList ;
	}
	else
	{
		_write_prim_packs_list_func = DG_WritePrimPacksListNVS ;
	}
}

void DG_ReleaseObjVertexShader(void)
{
	DG_KillVertexShader( &DG_ObjVertexShader[0] );
	DG_KillVertexShader( &DG_ObjVertexShader[1] );
}

/* 共通のレンダリングステート＆頂点シェーダー定数の設定 */
static void DG_InitChanlRenderState( DG_CHANL *cp, int flag )
{
	static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{1.0f/4096.0f, 1.0f/255.0f, 1.0f/32767.0f, 0},
	};

	/* VertexProcessing設定  */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckObjVertexBufferSoftProcessing()) ;

	// カメラ/透視変換マトリクスの設定
	//DG_SetTransform(D3DTS_VIEW, &cp->eye_inv);
	//DG_SetTransform(D3DTS_PROJECTION, &cp->xpers );

	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);

	/* 頂点シェーダー用各種定数設定 */
	if( DG_CheckObjUseVertexShader() )
	{
		DG_SetVertexShaderConstant(CV_ZERO,  &VS_Const[0], 1);
		DG_SetVertexShaderConstant(CV_ONE,   &VS_Const[1], 1);
		DG_SetVertexShaderConstant(CV_HALF,  &VS_Const[2], 1);
		DG_SetVertexShaderConstant(CV_SCALE, &VS_Const[3], 1);
		DG_SetVertexShaderConstant(CV_FOG,   &DG_FogParam, 1);

		/* Fog 初期化 */
		{
			float fog_start = 0.0f;
			float fog_end = 1.0f;
			DG_SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog_start);
			DG_SetRenderState(D3DRS_FOGEND,   *(DWORD *)&fog_end);
		}
	}
	else
	{
		/*-- Fog --*/
		DG_SetRenderState(D3DRS_FOGSTART, *((DWORD *)&DG_FogStart)) ;
		DG_SetRenderState(D3DRS_FOGEND,   *((DWORD *)&DG_FogEnd)) ;
   	}

	if ( flag != 0 ){

#if FALSE
		if( DG_CheckObjUseVertexShader() || DG_CheckUseVertexShader() )
		{
			DG_SetVertexShaderConstant(CV_EYEPERS, &cp->raise_eye_xpers, 4);
			DG_SetVertexShaderConstant(CV_PERS, &cp->raise_xpers, 4);
		}

		if( !DG_CheckObjUseVertexShader() || !DG_CheckUseVertexShader() )
		{
			DG_SetTransform(D3DTS_PROJECTION, &cp->raise_xpers );
			DG_SetTransform(D3DTS_VIEW,       &cp->eye_inv );
		}
#else
		if( DG_CheckObjUseVertexShader() || DG_CheckUseVertexShader() )
		{
			DG_SetVertexShaderConstant(CV_EYEPERS, &cp->eye_xpers, 4);
			DG_SetVertexShaderConstant(CV_PERS,    &cp->raise_xpers, 4);
		}

		if( !DG_CheckObjUseVertexShader() || !DG_CheckUseVertexShader() )
		{
			DG_SetTransform(D3DTS_PROJECTION, &cp->xpers );
			DG_SetTransform(D3DTS_VIEW,       &cp->eye_inv );
		}
#endif
		/* Ｚバッファ更新中止 */
		DG_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);		

	} else {

		if( DG_CheckObjUseVertexShader() )
		{
			DG_SetVertexShaderConstant(CV_EYEPERS, &cp->eye_xpers, 4);
			DG_SetVertexShaderConstant(CV_PERS,    &cp->xpers,     4);
		}
		else
		{
			DG_SetTransform(D3DTS_PROJECTION, &cp->xpers );
			DG_SetTransform(D3DTS_VIEW,       &cp->eye_inv );
		}
	}

}

/*----------------------------------------------------------------*/
	/*
		パケット初期化関連
	*/


	/*
		ChainObj()系のVertexBufferを設定する
	*/
static __forceinline void	ChainObj_SetVertexBuffer( DG_OBJ *obj )
{
	/* VertexBuffer変更判定 */
	if( obj->ibuff_dirty ){ DG_FlushDGObjIndexBuffer(obj) ;  }
	if( obj->vbuff_dirty ){ DG_FlushDGObjVertexBuffer(obj) ; }

	if ( obj->flag & DG_FLAG_PAINT ){
		/* プリシェードライティング */
		/* 頂点ストリーム及び入力頂点フォーマットの設定 */
		//DG_SetPixelShader( NULL );
		ASSERT( obj->d3d_vbuff && obj->d3d_cvbuff )

		/* 頂点カラーをVertexBufferに設定 */
		if( obj->cvbuff_dirty ){ DG_FlushDGObjCVertexBuffer(obj) ; }

		DG_SetStaticVertexBuffer( 0, obj->d3d_vbuff,  obj->stride );
		DG_SetStaticVertexBuffer( 1, obj->d3d_cvbuff, sizeof(int) ) ;

		/* 頂点シェーダー選択 */
		DG_SelectVertexShader( &DG_ObjVertexShader[1] );

	} else {
		/* ３並行光＆１環境光ライティング */
		/* 頂点ストリーム及び入力頂点フォーマットの設定 */
		//DG_SetPixelShader( NULL );
		ASSERT( obj->d3d_vbuff )
		DG_SetStaticVertexBuffer( 0, obj->d3d_vbuff, obj->stride );

		/* 頂点シェーダー選択 */
		DG_SelectVertexShader( &DG_ObjVertexShader[0] );
	}
}

	/*
		ChainObj()系のMaterialを設定する
	*/
static __forceinline int	ChainObj_SetMaterial( DG_OBJ *obj, ScrpadWork *scrpad)
{
	int           no_tex_flag = 0 ;

	if (!(obj->flag & DG_FLAG_PAINT) ) { /* 光源計算あり */
		if ( ( DG_DisplayStatus & DG_STATE_IR_MODE ) && ( obj->flag & DG_FLAG_IRREACTION ) ){
			/* 赤外線モード表示 */
			no_tex_flag = 1 ;
			DG_SetTexture(0, NULL );
			DG_SetAlphaMode( 0 );
			DG_SetVertexShaderConstant(CV_LIGHTVEC, &scrpad->ir_light_mat[0], 4);
			DG_SetVertexShaderConstant(CV_LIGHTCOL, &scrpad->ir_light_mat[1], 4);
		} else {
			/* 通常表示 */
			if ( scrpad->last_light_addr != obj->light ){
				/* 光源計算用マトリクス設定 */
				DG_SetVertexShaderConstant(CV_LIGHTVEC, &obj->light[0], 4);
				DG_SetVertexShaderConstant(CV_LIGHTCOL, &obj->light[1], 4);
				scrpad->last_light_addr = obj->light ;
			}
		}
		if ( obj->flag & DG_FLAG_NOFOG ){
			DG_SetRenderState( D3DRS_FOGENABLE, FALSE );
		} else {
			DG_SetRenderState( D3DRS_FOGENABLE, TRUE );
		}
	} else {
		DG_SetRenderState( D3DRS_FOGENABLE, TRUE );
	}
#if !__DG_OBJ_VERTEX_WEIGHT_REV__
	if (obj->parent != -1) {
		DG_SetVertexShaderConstant(CV_WORLD1, &obj->inv_mat, 4);
	}
	DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
#else
	if ( (obj->flag & DG_FLAG_PAINT) ) { /* 光源計算あり */
		DG_SetVertexShaderConstant(CV_WORLD1, &obj->world, 4);
	} else {
		if (obj->parent != -1) {
			if ( !(obj->mdl_type & DG_TYPE_REVWT_WT0) ) {
				DG_SetVertexShaderConstant(CV_WORLD0, &obj->inv_mat, 4);
			} else {
				/* 計算誤差を防ぐ為 */
				DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
			}
		} else {
			DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
		}
		DG_SetVertexShaderConstant(CV_WORLD1, &obj->world, 4);
	}
#endif

	return( no_tex_flag ) ;
}

	/*
		ChainObj()系の終了処理
	*/
static __forceinline void	ChainObj_Restore(void)
{
	/* カリング禁止 */
#ifndef _WINDOWS
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
#endif
}

	/*
		ＤＭＡに接続する
	*/
static	void	ChainObj( DG_OBJ *obj )
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
	u_long        alpha;
	int           j, no_tex_flag = 0 ;
	int				index_ofs ;
	DWORD			cull_state ;

	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	/*-- Vertex Bufferの設定処理 -----------------------------------------------*/

	ChainObj_SetVertexBuffer(obj) ;
	/*--------------------------------------------------------------------------*/

	/*-- Material設定 ----------------------------------------------------------*/

	no_tex_flag = ChainObj_SetMaterial(obj, scrpad) ;
	/*--------------------------------------------------------------------------*/

	/*-- Packet描画 ------------------------------------------------------------*/

	cull_state = DG_RenderStateValue[D3DRS_CULLMODE] ;	// 高速化(反則気味)

	DG_SetStaticIndexBuffer(obj->d3d_ibuff, 0) ;
	index_ofs = 0 ;
	if( no_tex_flag )
	{
		/*-- Texture無し -------------------------------------------------------*/
		for (j = obj->n_packs; j > 0; j --, pkt++ ) {
			alpha = pkt->tex_ptr[0]->alpha.data;
			if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
				if( cull_state != D3DCULL_CCW )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_CCW ) ;
					cull_state = D3DCULL_CCW ;
				}
			} else {
				if( cull_state != D3DCULL_NONE )
				{
					DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ) ;
					cull_state = D3DCULL_NONE ;
				}
			}
			DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
							pkt->idx_vtx_min, pkt->idx_vtx_num,
							index_ofs, pkt->n_indices - 2) ;

			index_ofs += pkt->n_indices ;
		}
		/*----------------------------------------------------------------------*/
	}
	else
	{
		void	*last_tex ;

		/*-- Texture有り -------------------------------------------------------*/

		last_tex = pkt->tex_ptr[0] ;
		DG_SetTexture( 0, last_tex );
		for (j = obj->n_packs; j > 0; j --, pkt++ ) {
			alpha = pkt->tex_ptr[0]->alpha.data;
			if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
				if( cull_state != D3DCULL_CCW )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_CCW ) ;
					cull_state = D3DCULL_CCW ;
				}
			} else {
				if( cull_state != D3DCULL_NONE )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_NONE ) ;
					cull_state = D3DCULL_NONE ;
				}
			}

			if( last_tex != pkt->tex_ptr[0] )
			{
				last_tex = pkt->tex_ptr[0] ;
				DG_SetTexture( 0, last_tex );
			}

			DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
							pkt->idx_vtx_min, pkt->idx_vtx_num,
							index_ofs, pkt->n_indices - 2) ;
			index_ofs += pkt->n_indices ;
		}
		/*----------------------------------------------------------------------*/
	}

	DG_RenderStateValue[D3DRS_CULLMODE] = cull_state ;	// 高速化の後始末
	/*--------------------------------------------------------------------------*/

	/*-- 終了処理 --------------------------------------------------------------*/

	ChainObj_Restore() ;
	/*--------------------------------------------------------------------------*/
}

	/*
		ＤＭＡに接続する(Index頂点未対応版)
	*/
static	void	ChainObjNIdx( DG_OBJ *obj )
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
	u_long        alpha;
	int           j, no_tex_flag = 0 ;
	int				index_ofs ;
	DWORD			cull_state ;

	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	/*-- Vertex Bufferの設定処理 -----------------------------------------------*/

	ChainObj_SetVertexBuffer(obj) ;
	/*--------------------------------------------------------------------------*/

	/*-- Material設定 ----------------------------------------------------------*/

	no_tex_flag = ChainObj_SetMaterial(obj, scrpad) ;
	/*--------------------------------------------------------------------------*/

	/*-- Packet描画 ------------------------------------------------------------*/

	cull_state = DG_RenderStateValue[D3DRS_CULLMODE] ;	// 高速化(反則気味)
	index_ofs = 0 ;
	if( no_tex_flag )
	{
		/*-- Texture無し -------------------------------------------------------*/

		for (j = obj->n_packs; j > 0; j --, pkt++ ) {
			alpha = pkt->tex_ptr[0]->alpha.data;
			if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
				if( cull_state != D3DCULL_CCW )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_CCW ) ;
					cull_state = D3DCULL_CCW ;
				}
			} else {
				if( cull_state != D3DCULL_NONE )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_NONE ) ;
					cull_state = D3DCULL_NONE ;
				}
			}

			DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, index_ofs, pkt->n_indices - 2) ;
			index_ofs += pkt->n_indices ;
		}
		/*----------------------------------------------------------------------*/
	}
	else
	{
		void	*last_tex ;

		/*-- Texture有り -------------------------------------------------------*/

		last_tex = pkt->tex_ptr[0] ;
		DG_SetTexture( 0, last_tex );
		for (j = obj->n_packs; j > 0; j --, pkt++ ) {
			alpha = pkt->tex_ptr[0]->alpha.data;
			if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
				if( cull_state != D3DCULL_CCW )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_CCW ) ;
					cull_state = D3DCULL_CCW ;
				}
			} else {
				if( cull_state != D3DCULL_NONE )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_NONE ) ;
					cull_state = D3DCULL_NONE ;
				}
			}

			if( last_tex != pkt->tex_ptr[0] )
			{
				last_tex = pkt->tex_ptr[0] ;
				DG_SetTexture( 0, last_tex );
			}

			DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, index_ofs, pkt->n_indices - 2) ;
			index_ofs += pkt->n_indices ;
		}
		/*----------------------------------------------------------------------*/
	}

	DG_RenderStateValue[D3DRS_CULLMODE] = cull_state ;	// 高速化の後始末
	/*--------------------------------------------------------------------------*/

	/*-- 終了処理 --------------------------------------------------------------*/

	ChainObj_Restore() ;
	/*--------------------------------------------------------------------------*/
}

	/*
		ChainObjNVS()系のVertexBufferを設定する
	*/
static __forceinline void	ChainObjNVS_SetVertexBuffer( DG_OBJ *obj )
{
	/* VertexBuffer変更判定 */
	if( obj->ibuff_dirty ){ DG_FlushDGObjIndexBufferNVS(obj) ;  }
	if( obj->vbuff_dirty ){ DG_FlushDGObjVertexBufferNVS(obj) ; }

	if( obj->stride == sizeof(DG_VERTEX_KMSS) )
	{
		if ( obj->flag & DG_FLAG_PAINT )
		{
			/* 頂点カラーをVertexBufferに設定 */
			if( obj->cvbuff_dirty ){ DG_FlushDGObjCVertexBuffer(obj) ; }

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
			/* 頂点カラーをVertexBufferに設定 */
			if( obj->cvbuff_dirty ){ DG_FlushDGObjCVertexBuffer(obj) ; }

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
		ChainObjNVS()系のMaterialを設定する
	*/
static __forceinline int	ChainObjNVS_SetMaterial( DG_OBJ *obj, ScrpadWork *scrpad)
{
	int           no_tex_flag = 0 ;

	if (!(obj->flag & DG_FLAG_PAINT) ) { /* 光源計算あり */

		DG_SetRenderState(D3DRS_LIGHTING , TRUE) ;

		if ( ( DG_DisplayStatus & DG_STATE_IR_MODE ) && ( obj->flag & DG_FLAG_IRREACTION ) ){
			/* 赤外線モード表示 */
			no_tex_flag = 1 ;
			DG_SetTextureNVS(0, NULL );
			DG_SetAlphaMode( 0 );
			DG_SetLightMatrix2Direct3D(&scrpad->ir_light_mat[0],
									   &scrpad->ir_light_mat[1]) ;
		} else {
			/* 通常表示 */
			if ( scrpad->last_light_addr != obj->light ){
				/* 光源計算用マトリクス設定 */
				DG_SetLightMatrix2Direct3D(&obj->light[0], &obj->light[1]) ;
				scrpad->last_light_addr = obj->light ;
			}
		}
		if ( obj->flag & DG_FLAG_NOFOG ){
			DG_SetRenderState( D3DRS_FOGENABLE, FALSE );
		} else {
			DG_SetRenderState( D3DRS_FOGENABLE, TRUE );
		}
	} else {
		DG_SetRenderState(D3DRS_LIGHTING,  FALSE) ;
		DG_SetRenderState(D3DRS_FOGENABLE, TRUE) ;
	}

#if !__DG_OBJ_VERTEX_WEIGHT_REV__
	if (obj->parent != -1) {
		DG_SetTransform(D3DTS_WORLDMATRIX(1), (FMATRIX *)&obj->inv_mat) ;
		DG_SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS ) ;
	}
	else
	{
		DG_SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE ) ;
	}
	DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->world) ;
#else
	if( (obj->flag & DG_FLAG_PAINT) )
	{
		DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->world) ;
		DG_SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE ) ;
	}
	else
	{
		if (obj->parent != -1) {
			DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->inv_mat) ;
			DG_SetTransform(D3DTS_WORLDMATRIX(1), (FMATRIX *)&obj->world) ;
			DG_SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS ) ;
		}
		else
		{
			DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->world) ;
			DG_SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE ) ;
		}
	}
#endif

	return( no_tex_flag ) ;
}

	/*
		ＤＭＡに接続する(Vertex Shader未使用版)
	*/
static	void	ChainObjNVS( DG_OBJ *obj )
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
	u_long        alpha;
	int           j, no_tex_flag = 0 ;
	int				index_ofs ;
	DWORD			cull_state ;

	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	/*-- Vertex Bufferの設定処理 -----------------------------------------------*/

	ChainObjNVS_SetVertexBuffer(obj) ;
	/*--------------------------------------------------------------------------*/

	/*-- Material設定 ----------------------------------------------------------*/

	no_tex_flag = ChainObjNVS_SetMaterial(obj, scrpad) ;
	/*--------------------------------------------------------------------------*/

	/*-- Packet描画 ------------------------------------------------------------*/

	cull_state = DG_RenderStateValue[D3DRS_CULLMODE] ;	// 高速化(反則気味)

	DG_SetStaticIndexBuffer(obj->d3d_ibuff, 0) ;
	index_ofs = 0 ;
	if( no_tex_flag )
	{
		for (j = obj->n_packs; j > 0; j --, pkt++ ) {
			alpha = pkt->tex_ptr[0]->alpha.data;
			if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
				if( cull_state != D3DCULL_CCW )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_CCW ) ;
					cull_state = D3DCULL_CCW ;
				}
			} else {
				if( cull_state != D3DCULL_NONE )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_NONE ) ;
					cull_state = D3DCULL_NONE ;
				}
			}

			DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
							pkt->idx_vtx_min, pkt->idx_vtx_num,
							index_ofs, pkt->n_indices - 2) ;
			index_ofs += pkt->n_indices ;
		}
	}
	else
	{
		void	*last_tex ;

		last_tex = pkt->tex_ptr[0] ;
		DG_SetTextureNVS( 0, last_tex );

		for (j = obj->n_packs; j > 0; j --, pkt++ ) {
			alpha = pkt->tex_ptr[0]->alpha.data;
			if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
				if( cull_state != D3DCULL_CCW )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_CCW ) ;
					cull_state = D3DCULL_CCW ;
				}
			} else {
				if( cull_state != D3DCULL_NONE )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_NONE ) ;
					cull_state = D3DCULL_NONE ;
				}
			}

			if( last_tex != pkt->tex_ptr[0] )
			{
				last_tex = pkt->tex_ptr[0] ;
				DG_SetTextureNVS( 0, last_tex );
			}

			DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
							pkt->idx_vtx_min, pkt->idx_vtx_num,
							index_ofs, pkt->n_indices - 2) ;
			index_ofs += pkt->n_indices ;
		}
	}

	DG_RenderStateValue[D3DRS_CULLMODE] = cull_state ;	// 高速化の後始末
	/*--------------------------------------------------------------------------*/

	/*-- 終了処理 --------------------------------------------------------------*/

	ChainObj_Restore() ;
	/*--------------------------------------------------------------------------*/
}

	/*
		ＤＭＡに接続する(VertexShader/Index未使用版)
	*/
static	void	ChainObjNVSNIdx( DG_OBJ *obj )
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
	u_long        alpha;
	int           j, no_tex_flag = 0 ;
	int				index_ofs ;
	DWORD			cull_state ;

	pkt = obj->packets ;
	if (pkt == NULL) return;

	/*-- Vertex Bufferの設定処理 -----------------------------------------------*/

	ChainObjNVS_SetVertexBuffer(obj) ;
	/*--------------------------------------------------------------------------*/

	/*-- Material設定 ----------------------------------------------------------*/

	no_tex_flag = ChainObjNVS_SetMaterial(obj, scrpad) ;
	/*--------------------------------------------------------------------------*/

	/*-- Packet描画 ------------------------------------------------------------*/

	cull_state = DG_RenderStateValue[D3DRS_CULLMODE] ;	// 高速化(反則気味)
	index_ofs = 0 ;
	if( no_tex_flag )
	{
		for (j = obj->n_packs; j > 0; j --, pkt++ ) {
			alpha = pkt->tex_ptr[0]->alpha.data;
			if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
				if( cull_state != D3DCULL_CCW )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_CCW ) ;
					cull_state = D3DCULL_CCW ;
				}
			} else {
				if( cull_state != D3DCULL_NONE )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_NONE ) ;
					cull_state = D3DCULL_NONE ;
				}
			}

			DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, index_ofs, pkt->n_indices - 2) ;
			index_ofs += pkt->n_indices ;
		}
	}
	else
	{
		void	*last_tex ;

		last_tex = pkt->tex_ptr[0] ;
		DG_SetTextureNVS( 0, last_tex );
		for (j = obj->n_packs; j > 0; j --, pkt++ ) {
			alpha = pkt->tex_ptr[0]->alpha.data;
			if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
				if( cull_state != D3DCULL_CCW )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_CCW ) ;
					cull_state = D3DCULL_CCW ;
				}
			} else {
				if( cull_state != D3DCULL_NONE )
				{
					DG_SetRenderState_EB( D3DRS_CULLMODE, D3DCULL_NONE ) ;
					cull_state = D3DCULL_NONE ;
				}
			}

			if( last_tex != pkt->tex_ptr[0] )
			{
				last_tex = pkt->tex_ptr[0] ;
				DG_SetTextureNVS( 0, last_tex );
			}
			DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, index_ofs, pkt->n_indices - 2) ;
			index_ofs += pkt->n_indices ;
		}
	}

	DG_RenderStateValue[D3DRS_CULLMODE] = cull_state ;	// 高速化の後始末
	/*--------------------------------------------------------------------------*/

	/*-- 終了処理 --------------------------------------------------------------*/

	ChainObj_Restore() ;
	/*--------------------------------------------------------------------------*/
}

/*----------------------------------------------------------------*/

	/*
		オブジェクトのソートリストを作成する
	*/

static	void	MakeSortList( DG_OBJS *objs )
{
	ScrpadWork	*work = (ScrpadWork*)SCRPAD_ADDR ;
	SORT_WORK	*sort_work ;
	SORT_TAG	*list ;
	DG_OBJ		*obj ;
	int			i ;
	u_int		z ;
	void		**ot ;

	//sort_work = (SORT_WORK*)work->local_work ;
	sort_work = (SORT_WORK*)DG_LocalMemory ;

	list = work->now ;
	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		if ( obj->bound_mode & 2 ) continue ;
		if ( !( obj->mdl_type & DG_TYPE_TRANS ) ) continue ;
		if ( obj->flag & work->invisible_flag ) continue ;
		/* ソート用データを作成 */
		z = ( ( (u_int)obj->sort_z + 0x7fffff ) >> 6 ) ;
		list->obj = obj ;
		list->z = z ; z &= 63 ;
		ot = sort_work->ot1 + z ;
		list->tag = *ot ; *ot = list ;
		list++ ;
		work->max_num++ ;
	}
	work->now = list ;

}

	/*
		ソートを行う
	*/
static	void	SortList( void )
{
	//ScrpadWork	*work = (ScrpadWork*)SCRPAD_ADDR ;
	SORT_WORK	*sort_work ;
	int			i, z ;
	SORT_TAG	*list ;
	void		**ot2, **ot3, *next_addr ;

	//sort_work = (SORT_WORK*)work->local_work ;
	sort_work = (SORT_WORK*)DG_LocalMemory ;

	/* 大きい方からソートしていく */
	for ( i = 63 ; i >= 0  ; i-- ){
		next_addr = sort_work->ot1[i] ;
		while ( next_addr != NULL ){
			list = next_addr ;
			next_addr = list->tag ;
			z = list->z ; z = ( z >> 6 ) & 63 ;
			ot2 = sort_work->ot2 + z ;
			list->tag = *ot2 ;
			*ot2 = list ;
		}
	}

	/* 小さい方からソートしていく */
	for ( i = 0 ; i < 64   ; i++ ){
		next_addr = sort_work->ot2[i] ;
		while ( next_addr != NULL ){
			list = next_addr ;
			next_addr = list->tag ;
			z = list->z ; z = ( z >> 12 ) & 63 ;
			ot3 = sort_work->ot3 + z ;
			list->tag = *ot3 ;
			*ot3 = list ;
		}
	}

}


	/*
		ＤＭＡに接続する
	*/
static	void	SortChainObjs( DG_CHANL *cp, int which )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	SORT_WORK	*sort_work ;
	DG_OBJ		*obj ;
	int			k, /*size, */obj_flag ;
	void		*next_addr, *next_addr_prim ;
	SORT_TAG	*list ;
	DG_PRIM2_PACKET	*prim_packet ;
	DG_PRIM2_PACKET	*prim_packet_top ;
	void		(*chain_obj_func)(DG_OBJ *obj) ;
	void		(*write_prim_packs_list_func)(DG_PRIM2_PACKET*,DG_PRIM2_PACKET*,
												DG_CHANL*,int) ;
	extern void	*DG_Prim2OT[64] ;
	BOOL		prim2_use_vs ;
	BOOL		obj_use_vs ;
	BOOL		raise_cv = FALSE ;
	BOOL		raise_trans = FALSE ;

	/*-- 描画ルーチン選択 ----------------------------------------*/

	chain_obj_func             = _chain_obj_func ;
	write_prim_packs_list_func = _write_prim_packs_list_func ;

	prim2_use_vs = DG_CheckUseVertexShader() ;
	obj_use_vs   = DG_CheckObjUseVertexShader() ;

	/*------------------------------------------------------------*/

	/*-- 初期化 --------------------------------------------------*/

	DG_InitWritePrimPacks(cp) ;
	/*------------------------------------------------------------*/

	sort_work = (SORT_WORK*)DG_LocalMemory ;

	for ( k = 63 ; k >= 0  ; k-- ){
		prim_packet_top = NULL ;
		next_addr = sort_work->ot3[k] ;
		next_addr_prim = DG_Prim2OT[k];
		DG_Prim2OT[k] = NULL ;
		while ( next_addr != NULL || next_addr_prim != NULL ){
			list = next_addr ;
			prim_packet = next_addr_prim ;

			/* 処理を行うオブジェクトを決定（マージソート） */
			if ( next_addr_prim == NULL || next_addr == NULL ){
				if ( next_addr == NULL )	obj_flag = 0 ;
				else						obj_flag = 1 ;
			} else {
				if ( list->z > (unsigned int)prim_packet->sort_z )	obj_flag = 1 ;
				else												obj_flag = 0 ;
			}

			if ( obj_flag ){
				//if ( GV_PadData[1].press & PAD_A ) printf( "objs: %d\n", list->z*64-0x7fffff );
				/* ソートされたプリミティブを一気に接続 */
				if( prim_packet_top )
				{
					/* 透視変換マトリクス変更 */
					if( prim2_use_vs )
					{
						if( !raise_cv )
						{
							DG_SetVertexShaderConstant(CV_PERS, &cp->raise_xpers, 4);
							raise_cv = TRUE ;
						}
					}
					else
					{
						if( !raise_trans )
						{
							DG_SetTransform(D3DTS_PROJECTION, &cp->raise_xpers );
							raise_trans = TRUE ;
						}
					}

					write_prim_packs_list_func(prim_packet_top, prim_packet, cp, which) ;
					prim_packet_top = NULL ;

					/* VertexProcessing設定復帰  */
					DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
								DG_CheckObjVertexBufferSoftProcessing()) ;

					/* Fog設定復帰 */
					if( obj_use_vs && !prim2_use_vs )
					{
						float fog_start = 0.0f;
						float fog_end   = 1.0f;

						DG_SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog_start);
						DG_SetRenderState(D3DRS_FOGEND,   *(DWORD *)&fog_end);
					}
				}

				/* 透視変換マトリクス変更 */
				if( obj_use_vs )
				{
					if( raise_cv )
					{
						DG_SetVertexShaderConstant(CV_PERS, &cp->xpers, 4);
						raise_cv = FALSE ;
					}
				}
				else
				{
					if( raise_trans )
					{
						DG_SetTransform(D3DTS_PROJECTION, &cp->xpers );
						raise_trans = FALSE ;
					}
				}
				/* ソートしたオブジェクトを接続 */
				next_addr = list->tag ;
				obj = list->obj ;
#ifdef LIBDG_PERFORMANCE
				scrpad->verts_count = 0 ;
				scrpad->pack_count = 0 ;
#endif
				scrpad->fog = obj->fog ;
				scrpad->matrix_addr = obj->matrix_addr ;

				chain_obj_func( obj );
#ifdef LIBDG_PERFORMANCE
				scrpad->model[ obj->bound_mode ].n_obj++ ;
				scrpad->model[ obj->bound_mode ].n_packs += scrpad->pack_count ;
				scrpad->model[ obj->bound_mode ].n_verts += scrpad->verts_count ;
#endif
			} else {
				/* 直前の描画タイプがモデルなら透視変換マトリクスを優先上げ要素付きに変更 */
				//if ( GV_PadData[1].press & PAD_A ){
				//	printf( "prim: %d(%s)\n", prim_packet->sort_z*64-0x7fffff,
				//		   ((DG_PRIM2*)(prim_packet->prim))->fname );
				//}
				/* ソートしたプリミティブを接続 */
				next_addr_prim = prim_packet->next_addr;
				if( !prim_packet_top ){ prim_packet_top = prim_packet ; }

			}
#if FALSE
			if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
#endif

		}

		/* ソートされたプリミティブを一気に接続 */
		if( prim_packet_top )
		{
			/* 透視変換マトリクス変更 */
			if( prim2_use_vs )
			{
				if( !raise_cv )
				{
					DG_SetVertexShaderConstant(CV_PERS, &cp->raise_xpers, 4);
					raise_cv = TRUE ;
				}
			}
			else
			{
				if( !raise_trans )
				{
					DG_SetTransform(D3DTS_PROJECTION, &cp->raise_xpers );
					raise_trans = TRUE ;
				}
			}

			write_prim_packs_list_func(prim_packet_top, NULL, cp, which) ;

			/* VertexProcessing設定復帰  */
			DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckObjVertexBufferSoftProcessing()) ;

			/* Fog設定復帰 */
			if( obj_use_vs && !prim2_use_vs )
			{
				float fog_start = 0.0f;
				float fog_end   = 1.0f;

				DG_SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog_start);
				DG_SetRenderState(D3DRS_FOGEND,   *(DWORD *)&fog_end);
			}
		}
	}

	//DG_CurrentDmaAddr = DG_EndCacheFIFO( scrpad->local_work );

	DG_SetPixelShader( NULL );
}

/*----------------------------------------------------------------*/


	/*
		キューされた各オブジェクトをソートしてＤＭＡパケットに接続する
	*/
void		DG_SortChainChanl( DG_CHANL *cp, int which )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		**oque, *objs ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
//	DG_TEXTURE_LIST	*tex_list, **tex_list_p ;
	int		i, /*size, */gid, c_gid ;
	SORT_WORK		*sort_work ;
	int			invisible_flag ;

	que = cp->obj_queue ;

	MARK( "chain.c(sort)" );
	if ( que == NULL ) return ;
	DG_PerfStartD(DGPERF_CTGR_SORT_CHAIN) ;
	obj_buff = &que->objs_buffer ;
	c_gid = DG_CurrentGroupID ;

	/* ワーク初期化 */
	sort_work = (SORT_WORK*)DG_LocalMemory ;
	GV_ZeroMemory( sort_work->ot3, sizeof(int)*64*3 );
	scrpad->max_num = 0 ;
	scrpad->now = sort_work->list ;
	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
	scrpad->buffer_switch = 0 ;
	scrpad->fog_param1 = DG_FogParam1 ;
	scrpad->fog_param2 = DG_FogParam2 ;
	scrpad->last_light_addr = NULL ;
#ifdef LIBDG_PERFORMANCE
	scrpad->model[0] = DG_PerformanceData.trans_model[0] ;
	scrpad->model[1] = DG_PerformanceData.trans_model[1] ;
#endif
	scrpad->eye_pers = cp->eye_xpers ;
	scrpad->eye_inv = cp->eye_inv ;

	/* 本来なら半透明用テクスチャをＶＲＡＭへ転送 */
	/* しかしＸＢＯＸではテクスチャの読み込みは必要ない */

	/* リストの作成 */
	invisible_flag = DG_FLAG_MULTITEX
				   | DG_FLAG_PLUGINDRAW
				   | (DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWWRITE)
				   | scrpad->invisible_flag ;

	oque = (DG_OBJS**)obj_buff->queue ;
	for ( i = obj_buff->n_queue ; i > 0 ; -- i, oque++ ) {
		objs = *oque ;
#ifndef _WINDOWS
		if ( objs->flag & DG_FLAG_MULTITEX ) continue ;
		if ( objs->flag & scrpad->invisible_flag ) continue ;
		if ( objs->flag & DG_FLAG_PLUGINDRAW ) continue ;

		/* DG_CurrentGroupID実験 （2000/01/11 M.Sonoyama） */
		if ( !( objs->group_id & DG_CurrentGroupID ) ) continue ;

		if ( objs->flag & ( DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWWRITE ) ) continue ;

		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;

#else
		if ( objs->flag & invisible_flag ) continue ;

		/* DG_CurrentGroupID実験 （2000/01/11 M.Sonoyama） */
		if ( !( objs->group_id & c_gid ) ) continue ;

#endif
		MakeSortList( *oque );
	}

	/* ソート */
	SortList();

	/* スタティックプッシュバッファへの記録開始 */
	DG_OpenDmaTask();

	/* レンダリングステート＆頂点バッファ定数の設定 */
	DG_InitChanlRenderState( cp, 1 );

	/* 赤外線モードライトマトリクス生成 */
	{
		if( !DG_CheckIROldVersion() )
		{
			/* X-BOX版準拠演出 */
			static FMATRIX	ir_light[2] = {
				{{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
				{{{255,255,255,0},{0,0,0,0},{0,0,0,0},{128,128,128,128}}}
			};
			scrpad->ir_light_mat[0] = ir_light[0] ;
			scrpad->ir_light_mat[1] = ir_light[1] ;
			scrpad->ir_light_mat[0].m[0][0] = cp->eye.m[2][0] ;
			scrpad->ir_light_mat[0].m[1][0] = cp->eye.m[2][1] ;
			scrpad->ir_light_mat[0].m[2][0] = cp->eye.m[2][2] ;
		}
		else
		{
			/* PS2版っぽい演出 */
			static FMATRIX	ir_light[2] = {
				{{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
				{{{0,0,0,0},{0,0,0,0},{0,0,0,0},{255,255,255,255}}}
			};

			ZeroMemory(&scrpad->ir_light_mat[0], sizeof(FMATRIX)) ;
			scrpad->ir_light_mat[1] = ir_light[1] ;
		}
	}

	/* ＤＭＡ書き出し処理 */
	SortChainObjs( cp, which );

#ifdef _WINDOWS
	/* RenderStateをデフォルトに戻す */
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
#endif

	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.trans_model[0] = scrpad->model[0] ;
	DG_PerformanceData.trans_model[1] = scrpad->model[1] ;
#endif

	DG_PerfEndD(DGPERF_CTGR_SORT_CHAIN) ;
}

/*----------------------------------------------------------------*/

static	void	ChainObjs( DG_OBJS *objs )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJ		*obj ;
	int			i ;
	void		(*chain_obj_func)(DG_OBJ *obj) ;

	/*-- 描画ルーチン選択 ----------------------------------------*/

	chain_obj_func = _chain_obj_func ;
	/*------------------------------------------------------------*/

	if( DG_CheckObjUseVertexShader() )
	{
		if ( objs->flag & DG_FLAG_FOGPARAM ){
			FVECTOR		vec ;
			vec.vx = objs->fog_param[0] ;
			vec.vy = objs->fog_param[1] ;
			vec.vz = 1.0f / 255.0f ;
			vec.vw = 1.0f ;
			DG_SetVertexShaderConstant(CV_FOG, &vec, 1);
		} else {
			float fog_start = 0.0f;
			float fog_end   = 1.0f;

			DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);

			DG_SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog_start);
			DG_SetRenderState(D3DRS_FOGEND,   *(DWORD *)&fog_end);
		}
	}
	else
	{
		if ( objs->flag & DG_FLAG_FOGPARAM ){
			DG_SetRenderStateFogParam(objs->fog_param[0], objs->fog_param[1]) ;
		} else {
			DG_SetRenderState(D3DRS_FOGSTART, *((DWORD *)&DG_FogStart)) ;
			DG_SetRenderState(D3DRS_FOGEND,   *((DWORD *)&DG_FogEnd)) ;
		}
	}

	obj = objs->objs ;
	for ( i = objs->n_models ; i>0; i--, obj++ ){
		/* クリップアウトチェック */
		if ( obj->bound_mode & 2 ) continue ;
		if ( obj->mdl_type & DG_TYPE_TRANS ) continue ;
		if ( obj->flag & scrpad->invisible_flag ) continue ;

#ifdef LIBDG_PERFORMANCE
		scrpad->verts_count = 0 ;
		scrpad->pack_count = 0 ;
#endif
		scrpad->fog = obj->fog ;
		scrpad->matrix_addr = obj->matrix_addr ;
		chain_obj_func( obj );
#ifdef LIBDG_PERFORMANCE
		scrpad->model[ obj->bound_mode ].n_obj++ ;
		scrpad->model[ obj->bound_mode ].n_packs += scrpad->pack_count ;
		scrpad->model[ obj->bound_mode ].n_verts += scrpad->verts_count ;
#endif
	}
	//DG_EndSprToMem();

}

/*----------------------------------------------------------------*/

	/*
		キューされた各オブジェクトをＤＭＡパケットに接続する
	*/
void		DG_ChainChanl( DG_CHANL *cp, int which )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		**oque, *objs ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		i, j, /*size, */old_tex_code = -1, max_objs, max_mem_objs ;
	DG_TEXTURE_LIST	*tex_list, **tex_list_p ;
	int		c_gid, gid ;
	OBJ_LIST		*obj_list, *mem_obj_list ;
	int		last_semi_trans = 0, use_semi_trans = 0 ;
	int		invisible_flag ;
	que = cp->obj_queue ;
	MARK( "chain.c(normal)" );
	if ( que == NULL ) return ;
	obj_buff = &que->objs_buffer ;

	DG_PerfStartD(DGPERF_CTGR_CHAIN) ;

	/* スクラッチパッド内の必要なワークを初期化する */
	c_gid = DG_CurrentGroupID ;
	scrpad->eye_pers = cp->eye_xpers ;
	scrpad->eye_inv = cp->eye_inv ;
	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
	scrpad->buffer_switch = 0 ;
	scrpad->fog_param1 = DG_FogParam1 ;
	scrpad->fog_param2 = DG_FogParam2 ;
	scrpad->last_light_addr = NULL ;
#ifdef LIBDG_PERFORMANCE
	scrpad->model[0] = DG_PerformanceData.normal_model[0] ;
	scrpad->model[1] = DG_PerformanceData.normal_model[1] ;
#endif
	{/* 赤外線モードライトマトリクス生成 */
		if( !DG_CheckIROldVersion() )
		{
			/* X-BOX版準拠演出 */
			static FMATRIX	ir_light[2] = {
				{{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
				{{{255,255,255,0},{0,0,0,0},{0,0,0,0},{128,128,128,128}}}
			};
			scrpad->ir_light_mat[0] = ir_light[0] ;
			scrpad->ir_light_mat[1] = ir_light[1] ;
			scrpad->ir_light_mat[0].m[0][0] = cp->eye.m[2][0] ;
			scrpad->ir_light_mat[0].m[1][0] = cp->eye.m[2][1] ;
			scrpad->ir_light_mat[0].m[2][0] = cp->eye.m[2][2] ;
		}
		else
		{
			/* PS2版っぽい演出 */
			static FMATRIX	ir_light[2] = {
				{{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
				{{{0,0,0,0},{0,0,0,0},{0,0,0,0},{255,255,255,255}}}
			};

			ZeroMemory(&scrpad->ir_light_mat[0], sizeof(FMATRIX)) ;
			scrpad->ir_light_mat[1] = ir_light[1] ;
		}
	}

	/* 描画オブジェクトをスクラッチパッドへ */
	invisible_flag = DG_FLAG_MULTITEX
				   | DG_FLAG_PLUGINDRAW
				   | ( DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWWRITE )
				   | scrpad->invisible_flag ;

	obj_list = (OBJ_LIST*)scrpad->local_work ;
	oque = (DG_OBJS**)obj_buff->queue ;
	for ( max_objs = 0, i = obj_buff->n_queue ; i > 0 ; -- i, oque++ ) {
		objs = *oque ;
#ifndef _WINDOWS
		if ( objs->flag & DG_FLAG_MULTITEX ) continue ;
		if ( objs->flag & scrpad->invisible_flag ) continue ;
		if ( objs->flag & DG_FLAG_PLUGINDRAW ) continue ;

		/* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
		if ( !( objs->group_id & DG_CurrentGroupID ) ) continue ;

		//if ( objs->flag & ( DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWMAKE|DG_FLAG_SHADOWWRITE ) ) continue ;
		if ( objs->flag & ( DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWWRITE ) ) continue ;
		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		if ( objs->bound_mode == 2 ) continue ;
		if ( objs->flag & DG_FLAG_SEMITRANS ) use_semi_trans = 1 ;
#else
		if ( objs->flag & invisible_flag ) continue ;

		/* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
		if ( !( objs->group_id & c_gid ) ) continue ;

		//if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		if ( objs->bound_mode == 2 ) continue ;
		if ( objs->flag & DG_FLAG_SEMITRANS ) use_semi_trans = 1 ;
#endif
		obj_list[max_objs].objs = objs ;
		obj_list[max_objs].use_tri = objs->tri_id ;
		max_objs++ ;
	}
	if ( max_objs == 0 ) return ;

	/* スクラッチパッド上のオブジェクトリストをテクスチャ毎にソートしてメモリに書き出す */
	mem_obj_list = (OBJ_LIST*)DG_LocalMemory ;
	max_mem_objs = 0 ;
	for ( tex_list_p = DG_TextureList, j = DG_MaxTextures ; j > 0 ; tex_list_p++, j-- ){
		tex_list = *tex_list_p;
		if ( tex_list->header->compress_flag & TRI_FLAG_TRANS ) continue ;
#ifdef LIBDG_CHECK_MULTILOADTEX
		//if ( GV_Time & 2 ) if ( tex_list->flag ) continue ;
#endif

		for ( i = 0 ; i < max_objs ; i++ ) {
			if ( tex_list->code != obj_list[i].use_tri ) continue ;
			mem_obj_list->objs = obj_list[i].objs ;
			mem_obj_list->use_tri = (int)tex_list ;	/* tri_codeの替わりにDG_TEXTUER_LISTへのポインタを入れる */
			mem_obj_list++ ;
			max_mem_objs++ ;
		}

	}

	/* スタティックプッシュバッファへの記録開始 */
	DG_OpenDmaTask();

	/* 半透明を使用したオブジェクトがある場合には事前にマスクを生成 */
	if ( use_semi_trans ){
		DG_MakeStencilMask();
	}

	/* chanl 毎の RenderState を設定する */
	DG_InitChanlRenderState( cp, 0 );

	/* メモリに書き出したソート完了済みオブジェクトをテクスチャ転送を挟みながら描画する */
	mem_obj_list = (OBJ_LIST*)DG_LocalMemory ;
	//tex_list = DG_TextureList ;
	//old_tex_code = -1 ;
	for ( i = max_mem_objs ; i > 0 ; i-- ){

		/* テクスチャのチェック */
#ifndef _WINDOWS
		if ( mem_obj_list->use_tri != old_tex_code ){
			tex_list = (DG_TEXTURE_LIST*)mem_obj_list->use_tri ;
#ifdef LIBDG_PERFORMANCE
			DG_PerformanceData.use_tex_size += tex_list->tex_size ;
#endif
			tex_list->flag = 1 ;
			//size = DG_WriteTextureChangePacks2( scrpad->dma_buffer, tex_list, which );
			//DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
			old_tex_code = mem_obj_list->use_tri ;
			tex_list++ ;
		}
#endif
		/* 強制半透明描画チェック */
		objs = mem_obj_list->objs;
		if ( objs->flag & DG_FLAG_SEMITRANS ){
			if ( last_semi_trans == 0 ){
				DG_SetRenderState( D3DRS_STENCILENABLE, TRUE );
			}
			last_semi_trans = 1 ;
		} else {
			if ( last_semi_trans != 0 ){
				DG_SetRenderState( D3DRS_STENCILENABLE, FALSE );
			}
			last_semi_trans = 0 ;
		}
		// Xbox追加:ユーザ定義シェーダ使用コールバック
		if (objs->exec_func != NULL) {
			objs->exec_func(cp, which, objs, objs->extend_data);
		} else {
			ChainObjs(objs);
		}
		mem_obj_list++ ;

#if FALSE
		if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
#endif
	}

#ifdef _WINDOWS
	/* RenderStateをデフォルトに戻す */
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
#endif

	/* 半透明処理無効化 */
	if ( last_semi_trans != 0 ){
		DG_SetRenderState( D3DRS_STENCILENABLE, FALSE );
	}
	/* Ｚ圧縮を活用するためにステンシルをクリア */
	if ( use_semi_trans ){
		DG_Clear(0, NULL, D3DCLEAR_STENCIL, 0, 0.0f, 0);
	}

	DG_SetPixelShader( NULL );

	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();

	DG_PerfEndD(DGPERF_CTGR_CHAIN) ;

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.normal_model[0] = scrpad->model[0] ;
	DG_PerformanceData.normal_model[1] = scrpad->model[1] ;
#endif
}

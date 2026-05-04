/*
	xchain2.c
	チャンネル処理ユニット／マルチテクスチャオブジェクトＤＭＡ接続ルーチン

	2002/02/09 K.Takabe
	$Id: wchain2.c,v 1.29 2003/01/05 04:46:34 takaki Exp $

*/
/*

	void		DG_ChainChanl2( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた各オブジェクトをＤＭＡパケットに接続する

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

#include	"def_dma.h"

#define	__MULTITEX_ONLY_USE_VERTEXSHADER__	(TRUE)	// VertexShader使用時のみMultiTexture

#define _CopyVector( a, b ) { *(u_long128*)a = *(u_long128*)b ; }
#define _CopyMatrix( a, b ) { 0[(u_long128*)a] = 0[(u_long128*)b] ;\
								1[(u_long128*)a] = 1[(u_long128*)b] ;\
								2[(u_long128*)a] = 2[(u_long128*)b] ;\
								3[(u_long128*)a] = 3[(u_long128*)b] ; }

#define GS_REGS_1(r0) \
((r0) << 0x00)
#define GS_REGS_3(r0, r1, r2) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08)
#define GS_REGS_5(r0, r1, r2, r3, r4) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c | (r4) << 0x10)

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

static	void	ChainObj( DG_OBJ *obj ) ;
static	void	ChainObjNIdx( DG_OBJ *obj ) ;		// Index非対応版
static	void	ChainObjNVS( DG_OBJ *obj ) ;		// VertexShader非対応版
static	void	ChainObjNVSNIdx( DG_OBJ *obj ) ;	// VertexShader/Index非対応版

static	void	*_chain_obj_func ;

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

/* 通常モデル描画用ワーク */
typedef struct {
	DG_OBJS		*objs ;
	int			use_tri ;
} OBJ_LIST ;

/* ベーススクラッチパッドワーク定義 */
typedef struct {
	/* ＤＭＡ生成用バッファ */
	/* オブジェクト状態管理用 */
	FMATRIX		screen ;
	FMATRIX		local_light ;
	FMATRIX		local_color ;
	FMATRIX		connection ;
	FMATRIX		option_mat ;
	FMATRIX		ir_light_mat[2] ;
	int			fog ;
	float		fog_param1, fog_param2 ;
	short		vu_prog_list[16] ;
	int			debug_count ;
	/* 各種ワーク */
	FMATRIX		eye_inv ;
	FMATRIX		eye_pers ;
	int			max_num ;
	void		*now ;
	int			invisible_flag ;
	int			buffer_switch ;
	void		*last_light_addr ;		/* 最後に設定したライトマトリクスへのアドレス */
	/* StoreMatrixObjs()関数用ローカル変数 */
	u_long128	*matrix_store_addr ;
	/* ChainObj()関数固有ローカル変数 */
	void		*matrix_addr ;

#ifdef LIBDG_PERFORMANCE
	PERFORMANCE_PACKET_INFO	model[ 2 ] ;
	int			verts_count ;
	int			pack_count ;
#endif

	/* ローカルワーク */
	u_long128	local_work[0] ;
} ScrpadWork ;

/* 汎用メモリワーク（主にスクラッチパッド上で構成し終わったデータなどを退避） */
extern u_long128	DG_LocalMemory[1024] ;


/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
#endif

static inline void _debug_print_matrix( char *str, FMATRIX *mat )
{
	int		i ;
	printf("%s\n", str);
	for ( i = 0 ; i < 4 ; i++ ){
		printf("%10f %10f %10f %10f\n", mat->m[i][0], mat->m[i][1], mat->m[i][2], mat->m[i][3] );
	}
}

#define Vu0CopyMatrix( _a, _b ) { *(_a) = *(_b) ; }

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

static DWORD			MTVertexShaderTableNum ;
static DG_VERTEXSHADER	MTVertexShaderTable[12] ;

DG_VERTEXSHADER	DG_MultiTexObjVertexShader[12];
DG_PIXELSHADER	DG_MultiTexPixelShader[DG_MULTITEX_MAXNUM] ;
extern unsigned char VERTEX_SHADER_kms_sl[];
extern unsigned char VERTEX_SHADER_kms_ml[];
extern unsigned char VERTEX_SHADER_kms_emap[];
extern unsigned char VERTEX_SHADER_kms_smap[];
extern unsigned char PIXEL_SHADER_mtex_1tex[];	/* 通常シングルテクスチャ */
extern unsigned char PIXEL_SHADER_mtex_emap[];	/* ＭＧＳ２標準環境マップマルチテクスチャ用 */
extern unsigned char PIXEL_SHADER_mtex_smap[];	/* ＭＧＳ２標準環境マップマルチテクスチャ用 */
extern unsigned char PIXEL_SHADER_mtex_bmap[];	/* ＭＧＳ２標準バンプマップマルチテクスチャ用 */
extern unsigned char PIXEL_SHADER_mtex_emap2[];	/* ＭＧＳ２重ね用環境マップ（減算＋環境マップの２テクスチャ） */
extern unsigned char PIXEL_SHADER_mtex_emap3[];	/* ＭＧＳ２重ね用環境マップ（アルファ＋環境マップの２テクスチャ） */
extern unsigned char PIXEL_SHADER_mtex_emap4[];	/* ＭＧＳ２標準環境マップマルチテクスチャ用（２テクスチャ） */
void DG_InitMultiTexObjVertexShader(void)
{
	DWORD dwObjDecl_00[] = {
		/* 通常ライティング */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT4),	/* position, blend */
		D3DVSD_REG(2,  D3DVSDT_SHORT4),	/* normal, dummy */
		D3DVSD_REG(7,  D3DVSDT_SHORT2),	/* uv0 */
		D3DVSD_REG(8,  D3DVSDT_SHORT2),	/* uv1 */
		D3DVSD_REG(9,  D3DVSDT_SHORT2),	/* uv2 */
		D3DVSD_END()
	};
	DWORD dwObjDecl_01[] = {
		/* 頂点カラー付き */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT4),	/* position, blend */
		D3DVSD_REG(2,  D3DVSDT_SHORT4),	/* normal, dummy */
		D3DVSD_REG(7,  D3DVSDT_SHORT2),	/* uv0 */
		D3DVSD_REG(8,  D3DVSDT_SHORT2),	/* uv1 */
		D3DVSD_REG(9,  D3DVSDT_SHORT2),	/* uv2 */
		D3DVSD_STREAM(1),
		D3DVSD_REG(3,  D3DVSDT_SHORT4),	/* diffuse */
		D3DVSD_END()
	};
	DWORD dwObjKmssDecl_00[] = {
		/* 通常ライティング */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT4),	/* position, blend */
		D3DVSD_REG(2,  D3DVSDT_SHORT4),	/* normal, dummy */
		D3DVSD_REG(7,  D3DVSDT_SHORT2),	/* uv0 */
		D3DVSD_END()
	};
	DWORD dwObjKmssDecl_01[] = {
		/* 頂点カラー付き */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT4),	/* position, blend */
		D3DVSD_REG(2,  D3DVSDT_SHORT4),	/* normal, dummy */
		D3DVSD_REG(7,  D3DVSDT_SHORT2),	/* uv0 */
		D3DVSD_STREAM(1),
		D3DVSD_REG(3,  D3DVSDT_SHORT4),	/* diffuse */
		D3DVSD_END()
	};


	if( DG_CheckObjUseVertexShader() )
	{
		if( DG_GetMultiTexMax() >= 3 )
		{
			MTVertexShaderTableNum = 12 ;

			/* 頂点カラー無しシェーダー */
			DG_MakeVertexShader(&MTVertexShaderTable[0], VSHT_wkms_ml, dwObjDecl_00) ;
			DG_MakeVertexShader(&MTVertexShaderTable[1], VSHT_wkms_emap, dwObjDecl_00) ;
			DG_MakeVertexShader(&MTVertexShaderTable[2], VSHT_wkms_smap, dwObjDecl_00) ;

			/* 頂点カラー有りシェーダー */
			DG_MakeVertexShader(&MTVertexShaderTable[3], VSHT_wkms_mlvc, dwObjDecl_01)	;
			DG_MakeVertexShader(&MTVertexShaderTable[4], VSHT_wkms_emapvc, dwObjDecl_01) ;
			DG_MakeVertexShader(&MTVertexShaderTable[5], VSHT_wkms_smapvc, dwObjDecl_01) ;

			/* 頂点カラー無しシェーダー(KMSS頂点版) */
			DG_MakeVertexShader(&MTVertexShaderTable[6], VSHT_wkms_ml_kmss, dwObjKmssDecl_00) ;
			DG_MakeVertexShader(&MTVertexShaderTable[7], VSHT_wkms_emap_kmss, dwObjKmssDecl_00) ;
			DG_MakeVertexShader(&MTVertexShaderTable[8], VSHT_wkms_smap_kmss, dwObjKmssDecl_00) ;

			/* 頂点カラー有りシェーダー(KMSS頂点版) */
			DG_MakeVertexShader(&MTVertexShaderTable[9], VSHT_wkms_mlvc_kmss, dwObjKmssDecl_01)	;
			DG_MakeVertexShader(&MTVertexShaderTable[10], VSHT_wkms_emapvc_kmss, dwObjKmssDecl_01) ;
			DG_MakeVertexShader(&MTVertexShaderTable[11], VSHT_wkms_smapvc_kmss, dwObjKmssDecl_01) ;

			/* 設定 */
			DG_MultiTexObjVertexShader[0] = MTVertexShaderTable[0] ;
			DG_MultiTexObjVertexShader[1] = MTVertexShaderTable[1] ;
			DG_MultiTexObjVertexShader[2] = MTVertexShaderTable[2] ;
			DG_MultiTexObjVertexShader[3] = MTVertexShaderTable[3] ;
			DG_MultiTexObjVertexShader[4] = MTVertexShaderTable[4] ;
			DG_MultiTexObjVertexShader[5] = MTVertexShaderTable[5] ;
			DG_MultiTexObjVertexShader[6] = MTVertexShaderTable[6] ;
			DG_MultiTexObjVertexShader[7] = MTVertexShaderTable[7] ;
			DG_MultiTexObjVertexShader[8] = MTVertexShaderTable[8] ;
			DG_MultiTexObjVertexShader[9] = MTVertexShaderTable[9] ;
			DG_MultiTexObjVertexShader[10] = MTVertexShaderTable[10] ;
			DG_MultiTexObjVertexShader[11] = MTVertexShaderTable[11] ;
		}
		else
		{
			MTVertexShaderTableNum = 2 ;

			/* 頂点カラー無しシェーダー(1 Texture版) */
			DG_MakeVertexShader(&MTVertexShaderTable[0],
								VSHT_wkms_ml_1tex, dwObjKmssDecl_00) ;

			/* 頂点カラー有りシェーダー(1 Texture版) */
			DG_MakeVertexShader(&MTVertexShaderTable[1],
								VSHT_wkms_mlvc_1tex, dwObjKmssDecl_01);

			/* 設定 */
			DG_MultiTexObjVertexShader[0] = MTVertexShaderTable[0] ;
			DG_MultiTexObjVertexShader[1] = MTVertexShaderTable[0] ;
			DG_MultiTexObjVertexShader[2] = MTVertexShaderTable[0] ;
			DG_MultiTexObjVertexShader[3] = MTVertexShaderTable[1] ;
			DG_MultiTexObjVertexShader[4] = MTVertexShaderTable[1] ;
			DG_MultiTexObjVertexShader[5] = MTVertexShaderTable[1] ;
			DG_MultiTexObjVertexShader[6] = MTVertexShaderTable[0] ;
			DG_MultiTexObjVertexShader[7] = MTVertexShaderTable[0] ;
			DG_MultiTexObjVertexShader[8] = MTVertexShaderTable[0] ;
			DG_MultiTexObjVertexShader[9] = MTVertexShaderTable[1] ;
			DG_MultiTexObjVertexShader[10] = MTVertexShaderTable[1] ;
			DG_MultiTexObjVertexShader[11] = MTVertexShaderTable[1] ;
		}
	}

	/* ピクセルシェーダーも生成 */
	DG_MakePixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_NORMAL], PSHT_mtex_1tex );
	DG_MakePixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP], PSHT_wmtex_emap );
	DG_MakePixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_SMAP], PSHT_wmtex_smap );
	DG_MakePixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_BMAP], PSHT_wmtex_bmap );
	DG_MakePixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP2], PSHT_wmtex_emap2 );
	DG_MakePixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP3], PSHT_wmtex_emap3 );
	DG_MakePixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP4], PSHT_wmtex_emap4 );


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
}

void DG_ReleaseMultiTexObjVertexShader(void)
{
	{
		DWORD	i ;

		for(i=0; i<MTVertexShaderTableNum; i++)
		{
			DG_KillVertexShader(&MTVertexShaderTable[i]) ;
		}

		ZeroMemory(DG_MultiTexObjVertexShader, sizeof(DG_MultiTexObjVertexShader)) ;
	}

	DG_KillPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_NORMAL] );
	DG_KillPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP] );
	DG_KillPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_SMAP] );
	DG_KillPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_BMAP] );
	DG_KillPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP2] );
	DG_KillPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP3] );
	DG_KillPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP4] );
}

/* chanl 毎のレンダリングステートを設定する */
static void DG_ChanlRenderState(DG_CHANL *cp)
{
	static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{1.0f/4096.0f, 1.0f/255.0f, 1.0f/32767.0f, 0},
	};
	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);

	/* アルファブレンディングOFF */
	DG_AlphaBlendDisable();

	if( DG_CheckObjUseVertexShader() )
	{
		/* 頂点シェーダー用各種定数設定 */
		DG_SetVertexShaderConstant(CV_EYEPERS, &cp->eye_xpers, 4);
		DG_SetVertexShaderConstant(CV_PERS,    &cp->xpers,     4);
		DG_SetVertexShaderConstant(CV_EYE_INV, &cp->eye_inv,   4);

		DG_SetVertexShaderConstant(CV_ZERO,  &VS_Const[0], 1);
		DG_SetVertexShaderConstant(CV_ONE,   &VS_Const[1], 1);
		DG_SetVertexShaderConstant(CV_HALF,  &VS_Const[2], 1);
		DG_SetVertexShaderConstant(CV_SCALE, &VS_Const[3], 1);
		DG_SetVertexShaderConstant(CV_FOG,   &DG_FogParam, 1);

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
		DG_SetTransform(D3DTS_PROJECTION, &cp->xpers) ;
		DG_SetTransform(D3DTS_VIEW,       &cp->eye_inv) ;
	}

	/* VertexShader非対応の場合のステート初期化 */
	if( !DG_CheckObjUseVertexShader() )
	{
		DG_SetRenderState(D3DRS_LIGHTING, TRUE) ;
	}

	/* VertexProcessing設定  */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckObjVertexBufferSoftProcessing()) ;
}
/*----------------------------------------------------------------*/

typedef	struct	ChainObjVtxShaderTbl_
{
	DG_VERTEXSHADER	*normal ;
	DG_VERTEXSHADER	*emap ;
	DG_VERTEXSHADER	*smap ;
} ChainObjVtxShaderTbl ;

static FVECTOR		color_scale =
{
	1.0f/255.0f,
	1.0f/255.0f,
	1.0f/255.0f,
	1.0f
} ;

static FVECTOR	color_scale_vc =
{
	1.0f/(4096.0f * 255.0f),
	1.0f/(4096.0f * 255.0f),
	1.0f/(4096.0f * 255.0f),
	1.0f/(4096.0f)
} ;
	/*
		ChainObj()系のMaterialを設定する
	*/
static __forceinline int	ChainObj_SetMaterial( DG_OBJ *obj, ScrpadWork *scrpad,
											ChainObjVtxShaderTbl *vstbl)
{
	int           no_tex_flag = 0 ;

	/* VertexBuffer変更判定 */
	if( obj->ibuff_dirty ){ DG_FlushDGObjIndexBuffer(obj) ;  }
	if( obj->vbuff_dirty ){ DG_FlushDGObjVertexBuffer(obj) ; }

	if ( obj->rgbs == NULL ){

		/* 頂点ストリーム及び入力頂点フォーマットの設定 */
		DG_SetStaticVertexBuffer( 0, obj->d3d_vbuff,  obj->stride );

		if( obj->stride == sizeof(DG_VERTEX_KMSM) )
		{
			vstbl->normal = &DG_MultiTexObjVertexShader[0] ;
			vstbl->emap   = &DG_MultiTexObjVertexShader[1] ;
			vstbl->smap   = &DG_MultiTexObjVertexShader[2] ;
		}
		else
		{
			vstbl->normal = &DG_MultiTexObjVertexShader[6] ;
			vstbl->emap   = &DG_MultiTexObjVertexShader[7] ;
			vstbl->smap   = &DG_MultiTexObjVertexShader[8] ;
		}

		/* 頂点カラーデータに対するスケール値を設定 */
		DG_SetVertexShaderConstant(CV_CONST0, &color_scale, 1);
	} else {

		/* 頂点カラーをVertexBufferに設定 */
		if( obj->cvbuff_dirty ){ DG_FlushDGObjCVertexBuffer(obj) ; }

		/* 頂点ストリーム及び入力頂点フォーマットの設定 */
		DG_SetStaticVertexBuffer( 0, obj->d3d_vbuff,  obj->stride );
		DG_SetStaticVertexBuffer( 1, obj->d3d_cvbuff, sizeof(SVECTOR) ) ;

		if( obj->stride == sizeof(DG_VERTEX_KMSM) )
		{
			vstbl->normal = &DG_MultiTexObjVertexShader[3] ;
			vstbl->emap   = &DG_MultiTexObjVertexShader[4] ;
			vstbl->smap   = &DG_MultiTexObjVertexShader[5] ;
		}
		else
		{
			vstbl->normal = &DG_MultiTexObjVertexShader[9] ;
			vstbl->emap   = &DG_MultiTexObjVertexShader[10] ;
			vstbl->smap   = &DG_MultiTexObjVertexShader[11] ;
		}

		/* 頂点カラーデータに対するスケール値を設定 */
		DG_SetVertexShaderConstant(CV_CONST0, &color_scale_vc, 1);
	}

	if ( !( obj->flag & DG_FLAG_PAINT ) ){
		if ( ( DG_DisplayStatus & DG_STATE_IR_MODE ) && ( obj->flag & DG_FLAG_IRREACTION ) ){
			/* 赤外線モード表示 */
			no_tex_flag = 1 ;
			DG_SetPixelShader( NULL );
			DG_SetTexture(0, NULL );
			DG_SetTexture(1, NULL );
			DG_SetTexture(2, NULL );
			DG_SetAlphaMode( 0 );
			DG_SelectVertexShader( vstbl->normal );
			DG_SetVertexShaderConstant(CV_LIGHTVEC, &scrpad->ir_light_mat[0], 4);
			DG_SetVertexShaderConstant(CV_LIGHTCOL, &scrpad->ir_light_mat[1], 4);
		} else {
			/* 通常表示 */
			if ( scrpad->last_light_addr != obj->light ){
				DG_SetVertexShaderConstant(CV_LIGHTVEC, &obj->light[0], 4);
				DG_SetVertexShaderConstant(CV_LIGHTCOL, &obj->light[1], 4);
				scrpad->last_light_addr = obj->light ;
			}
		}
	}

	/* マトリクス設定 */
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
			/* 計算誤差を防ぐ為 */
			DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
		}
	} else {
		DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
	}
	DG_SetVertexShaderConstant(CV_WORLD1, &obj->world, 4);
#endif

	return( no_tex_flag ) ;
}

	/*
		ChainObj()系の終了処理
	*/
static __forceinline void	ChainObj_Restore(void)
{
#ifndef _WINDOWS
	/* カリング禁止 */
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
	int           /*i, */j, no_tex_flag = 0 ;
	ChainObjVtxShaderTbl	vstbl ;
	int			index_ofs ;
	int			pre_mtex_type ;

	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	/*-- Material設定 ----------------------------------------------------------*/

	no_tex_flag = ChainObj_SetMaterial(obj, scrpad, &vstbl) ;
	/*--------------------------------------------------------------------------*/

	/*-- Packet描画 ------------------------------------------------------------*/

	DG_SetIndices(obj->d3d_ibuff, 0) ;
	index_ofs = 0 ;
	pre_mtex_type = -1 ;
	for (j = obj->n_packs; j > 0; j --, pkt++ ) {
		if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
		} else {
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
		}
		if ( no_tex_flag == 0 ){
			DG_SetTexture(0, pkt->tex_ptr[0] );
			DG_SetTexture(1, pkt->tex_ptr[1] );
			if ( pkt->tex_ptr[2] != NULL ){
				DG_SetTexture(2, pkt->tex_ptr[2] );
#ifndef UVADJUST
				DG_SetVertexShaderConstant( CV_TEX2_SCALE, &pkt->tex_ptr[2]->vec1, 1);
				DG_SetVertexShaderConstant( CV_TEX2_OFFSET, &pkt->tex_ptr[2]->vec2, 1);
#endif
			} else {
				DG_SetTexture(2, NULL );
			}
			if( pre_mtex_type != pkt->mtex_type )
			{
				pre_mtex_type = pkt->mtex_type ;
				switch ( pkt->mtex_type ){
				  default:
				  case DG_MULTITEX_NORMAL:
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_NORMAL] );
					DG_SelectVertexShader( vstbl.normal );
					break ;
				  case DG_MULTITEX_EMAP:
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP] );
					DG_SelectVertexShader( vstbl.emap );
					break ;
				  case DG_MULTITEX_SMAP:
					///* 都合によりテクスチャの使用ステージを入れ替える */
					//DG_SetTexture( 1, pkt->tex_ptr[2] );
					//DG_SetTexture( 2, NULL );
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_SMAP] );
					DG_SelectVertexShader( vstbl.smap );
					break ;
				  case DG_MULTITEX_EMAP2:
					//DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP2] );
					//DG_SelectVertexShader( vstbl.emap );
					DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) );
					//break ;	もらったソースにはBreakが無かったので、そっちにあわせます
				  case DG_MULTITEX_EMAP4:
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP4] );
					DG_SelectVertexShader( vstbl.emap );
					break ;
				}
			}
		}

		DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
						pkt->idx_vtx_min, pkt->idx_vtx_num,
						index_ofs, pkt->n_indices - 2) ;
		index_ofs += pkt->n_indices ;
	}
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
	int           /*i, */j, no_tex_flag = 0 ;
	ChainObjVtxShaderTbl	vstbl ;
	int			index_ofs ;
	int			pre_mtex_type ;

	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	/*-- Material設定 ----------------------------------------------------------*/

	no_tex_flag = ChainObj_SetMaterial(obj, scrpad, &vstbl) ;
	/*--------------------------------------------------------------------------*/

	/*-- Packet描画 ------------------------------------------------------------*/

	index_ofs = 0 ;
	pre_mtex_type = -1 ;
	for (j = obj->n_packs; j > 0; j --, pkt++ ) {
		if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
		} else {
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
		}
		if ( no_tex_flag == 0 ){
			DG_SetTexture(0, pkt->tex_ptr[0] );
			DG_SetTexture(1, pkt->tex_ptr[1] );
			if ( pkt->tex_ptr[2] != NULL ){
				DG_SetTexture(2, pkt->tex_ptr[2] );
#ifndef UVADJUST
				DG_SetVertexShaderConstant( CV_TEX2_SCALE, &pkt->tex_ptr[2]->vec1, 1);
				DG_SetVertexShaderConstant( CV_TEX2_OFFSET, &pkt->tex_ptr[2]->vec2, 1);
#endif
			} else {
				DG_SetTexture(2, NULL );
			}
			if( pre_mtex_type != pkt->mtex_type )
			{
				pre_mtex_type = pkt->mtex_type ;
				switch ( pkt->mtex_type ){
				  default:
				  case DG_MULTITEX_NORMAL:
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_NORMAL] );
					DG_SelectVertexShader( vstbl.normal );
					break ;
				  case DG_MULTITEX_EMAP:
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP] );
					DG_SelectVertexShader( vstbl.emap );
					break ;
				  case DG_MULTITEX_SMAP:
					///* 都合によりテクスチャの使用ステージを入れ替える */
					//DG_SetTexture( 1, pkt->tex_ptr[2] );
					//DG_SetTexture( 2, NULL );
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_SMAP] );
					DG_SelectVertexShader( vstbl.smap );
					break ;
				  case DG_MULTITEX_EMAP2:
					//DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP2] );
					//DG_SelectVertexShader( vstbl.emap );
					DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) );
					//break ;	もらったソースにはBreakが無かったので、そっちにあわせます
				  case DG_MULTITEX_EMAP4:
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP4] );
					DG_SelectVertexShader( vstbl.emap );
					break ;
				}
			}
		}

		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, index_ofs, pkt->n_indices - 2) ;
		index_ofs += pkt->n_indices ;
	}
	/*--------------------------------------------------------------------------*/

	/*-- 終了処理 --------------------------------------------------------------*/

	ChainObj_Restore() ;
	/*--------------------------------------------------------------------------*/
}

	/*
		ChainObj()系のMaterialを設定する(VertexShader非対応版)
	*/
static	FMATRIX	_tex_param_mtx =
{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
} ;

static __forceinline int	ChainObj_SetMaterialNVS( DG_OBJ *obj, ScrpadWork *scrpad)
{
	int           			no_tex_flag ;
	LPDIRECT3DVERTEXBUFFER8	vbuff ;
	BOOL					painted ;
	DWORD					stride ;

	no_tex_flag = 0 ;
	painted     = (DWORD)obj->rgbs ;

	ASSERT( obj->flag & DG_FLAG_MULTITEX ) ;

	/* VertexBuffer変更判定 */
	if( obj->ibuff_dirty ){ DG_FlushDGObjIndexBufferNVS(obj) ;  }
	if( obj->vbuff_dirty ){ DG_FlushDGObjVertexBufferNVS(obj) ; }

	/* 頂点ストリーム及び入力頂点フォーマットの設定 */
	if( painted && obj->cvbuff_dirty ){ DG_FlushDGObjCVertexBuffer(obj) ; }

	if( obj->stride == sizeof(DG_VERTEX_KMSM) )
	{
		stride = sizeof(DG_VERTEX_KMSM_FVF) ;
		DG_SetVertexShader( D3DFVF_DG_VERTEX_KMSM_FLAG ) ;
	}
	else
	{
		stride = sizeof(DG_VERTEX_KMSS_FVF) ;
		DG_SetVertexShader( D3DFVF_DG_VERTEX_KMSS_FLAG ) ;
	}

	if( painted ){ vbuff = obj->d3d_cvbuff ; }
	else{ vbuff = obj->d3d_vbuff ; }

	DG_SetStaticVertexBuffer(0, vbuff, stride) ;

	/* 特殊マテリアル */
	if ( !( obj->flag & DG_FLAG_PAINT ) )
	{
		if ( ( DG_DisplayStatus & DG_STATE_IR_MODE )
			&& ( obj->flag & DG_FLAG_IRREACTION ) )
		{
			/* 赤外線モード表示 */
			no_tex_flag = 1 ;
			DG_SetPixelShader( NULL );
			DG_SetTextureNVS(0, NULL );
#if !__MULTITEX_ONLY_USE_VERTEXSHADER__
			DG_SetTextureNVS(1, NULL );
			DG_SetTextureNVS(2, NULL );
#endif
			DG_SetAlphaMode( 0 );

			DG_SetLightMatrix2Direct3D(&scrpad->ir_light_mat[0],
									   &scrpad->ir_light_mat[1]) ;
		}
		else
		{
			/* 通常表示 */
			if ( scrpad->last_light_addr != obj->light ){
				DG_SetLightMatrix2Direct3D(&obj->light[0], &obj->light[1]) ;
				scrpad->last_light_addr = obj->light ;
			}
		}
	}
	
	/* マトリクス設定 */
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
#endif

	return( no_tex_flag ) ;
}


	/*
		ＤＭＡに接続する(VertexShader非対応版)
	*/
static	void	ChainObjNVS( DG_OBJ *obj )
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
	int           /*i, */j, no_tex_flag = 0 ;
	int			index_ofs ;
#if !__MULTITEX_ONLY_USE_VERTEXSHADER__
	int			pre_mtex_type ;
#endif
	pkt = obj->packets ;
	if (pkt == NULL) return;

	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	/*-- Material設定 ----------------------------------------------------------*/

	no_tex_flag = ChainObj_SetMaterialNVS(obj, scrpad) ;
	/*--------------------------------------------------------------------------*/

	/*-- Packet描画 ------------------------------------------------------------*/

	DG_SetIndices(obj->d3d_ibuff, 0) ;
	index_ofs = 0 ;
#if !__MULTITEX_ONLY_USE_VERTEXSHADER__
	pre_mtex_type = -1 ;
#endif
	for (j = obj->n_packs; j > 0; j --, pkt++ ) {
		if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
		} else {
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
		}
		if ( no_tex_flag == 0 ){
			DG_SetTextureNVS(0, pkt->tex_ptr[0] );
#if !__MULTITEX_ONLY_USE_VERTEXSHADER__
			DG_SetTextureNVS(1, pkt->tex_ptr[1] );
			if ( pkt->tex_ptr[2] != NULL ){
				DG_SetTextureNVS(2, pkt->tex_ptr[2] );
#ifndef UVADJUST
				{
					FVECTOR	*vec ;

					vec = &pkt->tex_ptr[2]->vec1
					_tex_param_mtx.m[0][0] = vec->vx ;
					_tex_param_mtx.m[1][1] = vec->vy ;
					vec = &pkt->tex_ptr[2]->vec2
					_tex_param_mtx.m[2][0] = vec->vx ;
					_tex_param_mtx.m[2][1] = vec->vy ;

					DG_SetTransform( D3DTS_TEXTURE2, (void *)&_tex_param_mtx) ;
				}
#endif
			} else {
				DG_SetTextureNVS(2, NULL );
			}
#endif	//__MULTITEX_ONLY_USE_VERTEXSHADER__

#if !__MULTITEX_ONLY_USE_VERTEXSHADER__
			if( pre_mtex_type != pkt->mtex_type )
			{
				pre_mtex_type = pkt->mtex_type ;
				switch ( pkt->mtex_type ){
				  default:
				  case DG_MULTITEX_NORMAL:
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_NORMAL] );
					break ;
				  case DG_MULTITEX_EMAP:
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP] );
					break ;
				  case DG_MULTITEX_SMAP:
					///* 都合によりテクスチャの使用ステージを入れ替える */
					//DG_SetTextureNVS( 1, pkt->tex_ptr[2] );
					//DG_SetTextureNVS( 2, NULL );
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_SMAP] );
					break ;
				  case DG_MULTITEX_EMAP2:
					//DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP4] );
					DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) );
					//break ;
				  case DG_MULTITEX_EMAP4:
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP4] );
					break ;
				}
			}
#endif
		}
		DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
						pkt->idx_vtx_min, pkt->idx_vtx_num,
						index_ofs, pkt->n_indices - 2) ;
		index_ofs += pkt->n_indices ;
	}
	/*--------------------------------------------------------------------------*/

	/*-- 終了処理 --------------------------------------------------------------*/

	ChainObj_Restore() ;
	/*--------------------------------------------------------------------------*/
}

	/*
		ＤＭＡに接続する(VertexShader/Index非対応版)
	*/
static	void	ChainObjNVSNIdx( DG_OBJ *obj )
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
	int           /*i, */j, no_tex_flag = 0 ;
	int			index_ofs ;
#if !__MULTITEX_ONLY_USE_VERTEXSHADER__
	int			pre_mtex_type ;
#endif
	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	/*-- Material設定 ----------------------------------------------------------*/

	no_tex_flag = ChainObj_SetMaterialNVS(obj, scrpad) ;
	/*--------------------------------------------------------------------------*/

	/*-- Packet描画 ------------------------------------------------------------*/

	index_ofs = 0 ;
#if !__MULTITEX_ONLY_USE_VERTEXSHADER__
	pre_mtex_type =  -1 ;
#endif
	for (j = obj->n_packs; j > 0; j --, pkt++ ) {
		if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
		} else {
				DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
		}
		if ( no_tex_flag == 0 ){
			DG_SetTextureNVS(0, pkt->tex_ptr[0] );
#if !__MULTITEX_ONLY_USE_VERTEXSHADER__
			DG_SetTextureNVS(1, pkt->tex_ptr[1] );
			if ( pkt->tex_ptr[2] != NULL ){
				DG_SetTextureNVS(2, pkt->tex_ptr[2] );
#ifndef UVADJUST
				{
					FVECTOR	*vec ;

					vec = &pkt->tex_ptr[2]->vec1
					_tex_param_mtx.m[0][0] = vec->vx ;
					_tex_param_mtx.m[1][1] = vec->vy ;
					vec = &pkt->tex_ptr[2]->vec2
					_tex_param_mtx.m[2][0] = vec->vx ;
					_tex_param_mtx.m[2][1] = vec->vy ;

					DG_SetTransform( D3DTS_TEXTURE2, (void *)&_tex_param_mtx) ;
				}
#endif
			} else {
				DG_SetTextureNVS(2, NULL );
			}
#endif	// __MULTITEX_ONLY_USE_VERTEXSHADER__

#if !__MULTITEX_ONLY_USE_VERTEXSHADER__
			if( pre_mtex_type != pkt->mtex_type )
			{
				pre_mtex_type = pkt->mtex_type ;
				switch ( pkt->mtex_type ){
				  default:
				  case DG_MULTITEX_NORMAL:
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_NORMAL] );
					break ;
				  case DG_MULTITEX_EMAP:
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP] );
					break ;
				  case DG_MULTITEX_SMAP:
					///* 都合によりテクスチャの使用ステージを入れ替える */
					//DG_SetTextureNVS( 1, pkt->tex_ptr[2] );
					//DG_SetTextureNVS( 2, NULL );
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_SMAP] );
					break ;
				  case DG_MULTITEX_EMAP2:
					//DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP4] );
					DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) );
					//break ;
				  case DG_MULTITEX_EMAP4:
					DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP4] );
					break ;
				}
			}
#endif	// __MULTITEX_ONLY_USE_VERTEXSHADER__
		}

		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, index_ofs, pkt->n_indices - 2) ;
		index_ofs += pkt->n_indices ;
	}
	/*--------------------------------------------------------------------------*/

	/*-- 終了処理 --------------------------------------------------------------*/

	ChainObj_Restore() ;
	/*--------------------------------------------------------------------------*/
}


/*----------------------------------------------------------------*/
	/*
		ＤＭＡに接続する
	*/
static	void	ChainObjs( DG_OBJS *objs )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJ		*obj ;
	int			i ;
	void		(*chain_obj_func)(DG_OBJ *obj) ;
	DWORD		invisible_flag ;

	/*-- 描画関数選択 ----------------------------------------------------------*/

	chain_obj_func = _chain_obj_func ;
	/*--------------------------------------------------------------------------*/

	/*-- Fog設定 ---------------------------------------------------------------*/

#ifndef _WINDOWS
	if ( objs->flag & DG_FLAG_FOGPARAM ){
		FVECTOR		vec ;
		vec.vx = objs->fog_param[0] ;
		vec.vy = objs->fog_param[1] ;
		vec.vz = 1.0f / 255.0f ;
		vec.vw = 1.0f ;
		DG_SetVertexShaderConstant(CV_FOG, &vec, 1);
	} else {
		DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);
	}

	if ( objs->flag & DG_FLAG_NOFOG ){
		DG_SetRenderState( D3DRS_FOGENABLE, FALSE );
	} else {
		DG_SetRenderState( D3DRS_FOGENABLE, TRUE );
	}
#else
	if( objs->flag & DG_FLAG_NOFOG )
	{
		DG_SetRenderState( D3DRS_FOGENABLE, FALSE );
	}
	else
	{
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
				DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);
			}
		}
		else
		{
			if ( objs->flag & DG_FLAG_FOGPARAM )
			{
				DG_SetRenderStateFogParam(objs->fog_param[0], objs->fog_param[1]) ;
			}
			else
			{
				DG_SetRenderState(D3DRS_FOGSTART, *((DWORD *)&DG_FogStart)) ;
				DG_SetRenderState(D3DRS_FOGEND,   *((DWORD *)&DG_FogEnd)) ;
			}
		}
	}
#endif
	/*--------------------------------------------------------------------------*/

	invisible_flag = scrpad->invisible_flag ;
	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		/* クリップアウトチェック */
		if ( obj->bound_mode & 2 ) continue ;
		if ( obj->mdl_type & DG_TYPE_TRANS ) continue ;
		if ( obj->flag & invisible_flag ) continue ;

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

}

/*----------------------------------------------------------------*/

	/*
		キューされた各オブジェクトをＤＭＡパケットに接続する
	*/
void _DG_Chain2Chanl( DG_CHANL *cp, int which, int draw_mode )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		**oque, *objs ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		i, j, /*size, */old_tex_code = -1, max_objs, max_mem_objs ;
	DG_TEXTURE_LIST	*tex_list, **tex_list_p ;
	int		c_gid /*, gid*/ ;
	OBJ_LIST		*obj_list, *mem_obj_list ;
	int			last_semi_trans = 0, use_semi_trans = 0 ;
	DWORD		flag ;
	DWORD		invisible_flag ;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;
	MARK( "chain2.c" );
	obj_buff = &que->objs_buffer ;

	c_gid = DG_CurrentGroupID ;
	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
	scrpad->buffer_switch = 0 ;

	scrpad->eye_inv = cp->eye_inv ;
	scrpad->eye_pers = cp->eye_xpers ;
	scrpad->fog_param1 = DG_FogParam1 ;
	scrpad->fog_param2 = DG_FogParam2 ;
	scrpad->last_light_addr = NULL ;
#ifdef LIBDG_PERFORMANCE
	scrpad->model[0] = DG_PerformanceData.multex_model[0] ;
	scrpad->model[1] = DG_PerformanceData.multex_model[1] ;
#endif
	scrpad->debug_count = 0 ;
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
	invisible_flag = scrpad->invisible_flag
				   | DG_FLAG_PAINT
				   | DG_FLAG_PLUGINDRAW
				   | (DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWWRITE) ;

	obj_list = (OBJ_LIST*)scrpad->local_work ;
	oque = (DG_OBJS**)obj_buff->queue ;
	for ( max_objs = 0, i = obj_buff->n_queue ; i > 0 ; -- i, oque++ ) {
		objs = *oque ;
#ifndef _WINDOWS
		if ( !( objs->flag & DG_FLAG_MULTITEX ) ) continue ;
		if ( objs->flag & DG_FLAG_PAINT ) continue ;
		if ( objs->flag & scrpad->invisible_flag ) continue ;
		if ( objs->flag & DG_FLAG_PLUGINDRAW ) continue ;
		/* 描画フェーズコントロール */
		if ( draw_mode == 0 ){
			if ( objs->flag & DG_FLAG_LATTERDRAW ) continue ;
		} else {
			if ( !( objs->flag & DG_FLAG_LATTERDRAW ) ) continue ;
		}

		/* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
		if ( !( objs->group_id & DG_CurrentGroupID ) ) continue ;

		//if ( objs->flag & ( DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWMAKE|DG_FLAG_SHADOWWRITE ) ) continue ;
		if ( objs->flag & ( DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWWRITE ) ) continue ;
		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		if ( objs->bound_mode == 2 ) continue ;
		if ( objs->flag & DG_FLAG_SEMITRANS ) use_semi_trans = 1 ;
#else
		flag = objs->flag ;

		if ( !( flag & DG_FLAG_MULTITEX ) ) continue ;
		if ( flag & invisible_flag ) continue ;
		/* 描画フェーズコントロール */
		if ( draw_mode == 0 ){
			if ( flag & DG_FLAG_LATTERDRAW ) continue ;
		} else {
			if ( !( flag & DG_FLAG_LATTERDRAW ) ) continue ;
		}

		/* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
		if ( !( objs->group_id & c_gid ) ) continue ;

		//if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;	// 上条件とかぶる
		if ( objs->bound_mode == 2 ) continue ;
		if ( flag & DG_FLAG_SEMITRANS ) use_semi_trans = 1 ;
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
	if ( max_mem_objs == 0 ) return ;


	/* スタティックプッシュバッファへの記録開始 */
	DG_OpenDmaTask();

	/* 半透明を使用したオブジェクトがある場合には事前にマスクを生成 */
	if ( use_semi_trans ){
		DG_MakeStencilMask();
	}

	/* chanl 毎の RenderState を設定する	*/
	DG_ChanlRenderState(cp);
	//DG_BeginScene();

	/* メモリに書き出したソート完了済みオブジェクトをテクスチャ転送を挟みながら描画する */
	mem_obj_list = (OBJ_LIST*)DG_LocalMemory ;
	//tex_list = DG_TextureList ;
	old_tex_code = -1 ;
	for ( i = max_mem_objs ; i > 0 ; i-- ){
		/* テクスチャのチェック */
		if ( mem_obj_list->use_tri != old_tex_code ){
			tex_list = (DG_TEXTURE_LIST*)mem_obj_list->use_tri ;
#ifdef LIBDG_PERFORMANCE
			DG_PerformanceData.use_tex_size += tex_list->tex_size ;
#endif
			tex_list->flag = 1 ;
			//size = DG_WriteTextureChangePacks( scrpad->dma_buffer, &tex_list->tex_packet[which] );
#if 0
			size = DG_WriteTextureChangePacks2( scrpad->dma_buffer, tex_list, which );
			DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
#endif
			old_tex_code = mem_obj_list->use_tri ;
			tex_list++ ;
		}
		/* 強制半透明描画チェック */
		if ( mem_obj_list->objs->flag & DG_FLAG_SEMITRANS ){
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
		ChainObjs( mem_obj_list->objs );
		mem_obj_list++ ;
	}

#ifdef _WINDOWS
	/* RenderStateをDefault設定に */
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

	/* 念のためフォグの状態を戻しておく */
	DG_SetRenderState( D3DRS_FOGENABLE, TRUE );

	DG_SetPixelShader( NULL );
	DG_InitTextureStageState( 0 );
	DG_InitTextureStageState( 1 );
	DG_InitTextureStageState( 2 );

	//DG_EndScene();

	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.multex_model[0] = scrpad->model[0] ;
	DG_PerformanceData.multex_model[1] = scrpad->model[1] ;
#endif
}

/* ---------------------------------------------------------------- */
void DG_Chain2Chanl( DG_CHANL *cp, int which )
{
	DG_PerfStartD(DGPERF_CTGR_CHAIN2) ;
	_DG_Chain2Chanl( cp, which, 0 );
	DG_PerfEndD(DGPERF_CTGR_CHAIN2) ;
}

void DG_Chain2ChanlLatter( DG_CHANL *cp, int which )
{
	DG_PerfStartD(DGPERF_CTGR_CHAIN2_LATTER) ;
	_DG_Chain2Chanl( cp, which, 1 );
	DG_PerfEndD(DGPERF_CTGR_CHAIN2_LATTER) ;
}

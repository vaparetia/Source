//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xchain2.c
	チャンネル処理ユニット／マルチテクスチャオブジェクトＤＭＡ接続ルーチン

	2002/02/09 K.Takabe
	$Id: xchain2.c,v 1.4 2002/11/23 11:36:55 Yoshizawa1 Exp $

*/
/*

	void		DG_ChainChanl2( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた各オブジェクトをＤＭＡパケットに接続する

*/

#ifdef KP_XBOX //BP

#ifdef KP_XBOX
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
extern u_long128	DG_AS_GetLocalMemoryAddress()[1024] ;


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
DG_VERTEXSHADER	DG_MultiTexObjVertexShader[4];
DG_PIXELSHADER	DG_MultiTexPixelShader[DG_MULTITEX_MAXNUM] ;
DG_VERTEXFORMAT	DG_MultiTexObjVertexFormat[2] ;
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
#if 0 //BP_RENDER

	DWORD dwObjDecl_00[] = {
		/* 通常ライティング */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT3),		/* position */
		D3DVSD_REG(1,  D3DVSDT_NORMSHORT1),	/* blend */
		D3DVSD_REG(2,  D3DVSDT_NORMSHORT3),	/* normal */
		D3DVSD_REG(10, D3DVSDT_SHORT1),		/* dummy */
		D3DVSD_REG(7,  D3DVSDT_NORMSHORT2),	/* uv0 */
		D3DVSD_REG(8,  D3DVSDT_NORMSHORT2),	/* uv1 */
		D3DVSD_REG(9,  D3DVSDT_NORMSHORT2),	/* uv2 */
		D3DVSD_END()
	};
	DWORD dwObjDecl_01[] = {
		/* 頂点カラー付き */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT3),		/* position */
		D3DVSD_REG(1,  D3DVSDT_NORMSHORT1),	/* blend */
		D3DVSD_REG(2,  D3DVSDT_NORMSHORT3),	/* normal */
		D3DVSD_REG(10, D3DVSDT_SHORT1),		/* dummy */
		D3DVSD_REG(7,  D3DVSDT_NORMSHORT2),	/* uv0 */
		D3DVSD_REG(8,  D3DVSDT_NORMSHORT2),	/* uv1 */
		D3DVSD_REG(9,  D3DVSDT_NORMSHORT2),	/* uv2 */
		D3DVSD_STREAM(1),
		D3DVSD_REG(3,  D3DVSDT_SHORT4),	/* diffuse */
		D3DVSD_END()
	};
	/* 頂点フォーマット定義 */
	DG_MakeVertexFormat( &DG_MultiTexObjVertexFormat[0], dwObjDecl_00 );
	DG_MakeVertexFormat( &DG_MultiTexObjVertexFormat[1], dwObjDecl_01 );

	/* 頂点カラー無しシェーダー */
	DG_MakeVertexShader( &DG_MultiTexObjVertexShader[0], VERTEX_SHADER_kms_ml, dwObjDecl_00 );
	DG_MakeVertexShader( &DG_MultiTexObjVertexShader[1], VERTEX_SHADER_kms_emap, dwObjDecl_00 );
	DG_MakeVertexShader( &DG_MultiTexObjVertexShader[2], VERTEX_SHADER_kms_smap, dwObjDecl_00 );

	/* ピクセルシェーダーも生成 */
	DG_MakePixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_NORMAL], PIXEL_SHADER_mtex_1tex );
	DG_MakePixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP], PIXEL_SHADER_mtex_emap );
	DG_MakePixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_SMAP], PIXEL_SHADER_mtex_smap );
	DG_MakePixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_BMAP], PIXEL_SHADER_mtex_bmap );
	DG_MakePixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP2], PIXEL_SHADER_mtex_emap2 );
	DG_MakePixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP3], PIXEL_SHADER_mtex_emap3 );
	DG_MakePixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP4], PIXEL_SHADER_mtex_emap4 );
#endif
}
/* chanl 毎のレンダリングステートを設定する */
static void DG_ChanlRenderState(DG_CHANL *cp)
{
	static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{1.0f/4096.0f, 1.0f/255.0f, 0, 0},
	};
	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);

	/* アルファブレンディングOFF */
	DG_AlphaBlendDisable();

	/* 頂点シェーダー用各種定数設定 */
	DG_SetVertexShaderConstant(CV_EYEPERS, &cp->eye_xpers, 4);
	DG_SetVertexShaderConstant(CV_PERS, &cp->xpers, 4);
	DG_SetVertexShaderConstant(CV_EYE_INV, &cp->eye_inv, 4);

	DG_SetVertexShaderConstant(CV_ZERO, &VS_Const[0], 1);
	DG_SetVertexShaderConstant(CV_ONE, &VS_Const[1], 1);
	DG_SetVertexShaderConstant(CV_HALF, &VS_Const[2], 1);
	DG_SetVertexShaderConstant(CV_SCALE, &VS_Const[3], 1);
	DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);

	/* バーテックスシェーダーの読み込み */
	DG_ClearVertexShader();
	DG_LoadVertexShader( &DG_MultiTexObjVertexShader[0] );
	DG_LoadVertexShader( &DG_MultiTexObjVertexShader[1] );
	DG_LoadVertexShader( &DG_MultiTexObjVertexShader[2] );
}
/*----------------------------------------------------------------*/

static	void	ChainObj( DG_OBJ *obj )
{
#if 0 //BP_RENDER
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET	*pkt;
//@	u_long64        alpha;
	int           /*i, */j, no_tex_flag = 0 ;
	unsigned short	*index_addr ;
	DG_VERTEXFORMAT	*vformat ;

	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	if ( obj->rgbs == NULL ){
		static FVECTOR		color_scale = {1,1,1,1} ;
		/* 頂点ストリーム及び入力頂点フォーマットの設定 */
		DG_SetVertexBuffer( 0, obj->vbuff, obj->stride );
		DG_SetVertexStream( &DG_MultiTexObjVertexFormat[0] );
		vformat = &DG_MultiTexObjVertexFormat[0] ;	/* 頂点シェーダー指定時に必要 */
		/* 頂点カラーデータに対するスケール値を設定 */
		DG_SetVertexShaderConstant(CV_CONST0, &color_scale, 1);
		/* diffuseに対するストリームを用意していないため下の命令で予め初期化しておく */
		/* ＰＣ移植の際には頂点カラーを無視するバージョンのシェーダーを別に用意したほうがいいかも */
		//IDirect3DDevice8_Begin( g_pd3dDevice, 0 );/* PrimitiveType = 0 はXBOXではnullと思われる・・・ */
		IDirect3DDevice8_Begin( g_pd3dDevice, 1 );/* PrimitiveType = POINTLIST */
		IDirect3DDevice8_SetVertexData4ub( g_pd3dDevice, 3, 255, 255, 255, 255 );
		IDirect3DDevice8_End( g_pd3dDevice );
	} else {
		static FVECTOR		color_scale = {1.0f/4096.0f,1.0f/4096.0f,1.0f/4096.0f,1.0f/4096.0f} ;
		/* 頂点ストリーム及び入力頂点フォーマットの設定 */
		DG_SetVertexBuffer( 0, obj->vbuff, obj->stride );
		DG_SetVertexBuffer( 1, obj->rgbs, 2*4 );	/* 頂点カラー */
		DG_SetVertexStream( &DG_MultiTexObjVertexFormat[1] );
		vformat = &DG_MultiTexObjVertexFormat[1] ;	/* 頂点シェーダー指定時に必要 */
		/* 頂点カラーデータに対するスケール値を設定 */
		DG_SetVertexShaderConstant(CV_CONST0, &color_scale, 1);
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
			DG_SelectVertexShader( &DG_MultiTexObjVertexShader[0], vformat );
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
	if (obj->parent != -1) {
		DG_SetVertexShaderConstant(CV_WORLD1, &obj->inv_mat, 4);
	}
	//DG_SetPixelShader( &DG_MultiTexPixelShader[0] );
	//DG_SelectVertexShader( &DG_MultiTexObjVertexShader[0], NULL );

	/* マトリクス設定 */
	DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);

#if 0
	DG_SetAlphaMode(0);	
	/* 不透明パケットを先に描く */
	index_addr = obj->index ;
	for (j = obj->n_packs; j > 0; j --, pkt++) {
		alpha = pkt->tex_ptr[0]->alpha.data;
		if (alpha == 0) {
			if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
				DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
			} else {
				DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
			}
			DG_DrawIndexedVertices( D3DPT_TRIANGLESTRIP, pkt->n_indices, index_addr );
		}
		index_addr += pkt->n_indices ;
	}

	/* 半透明パケット */
	pkt = obj->packets;
	index_addr = obj->index ;
	for (j = obj->n_packs; j > 0; j --, pkt++ ) {
		alpha = pkt->tex_ptr[0]->alpha.data;
		if (alpha != 0) {
			if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
				DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
			} else {
				DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
			}
			DG_SetAlphaMode(alpha);
			DG_DrawIndexedVertices( D3DPT_TRIANGLESTRIP, pkt->n_indices, index_addr );
		}
		index_addr += pkt->n_indices ;
	}
#else
	index_addr = obj->index ;
	for (j = obj->n_packs; j > 0; j --, pkt++ ) {
		if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
		} else {
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
		}
		if ( no_tex_flag == 0 ){
			if ( pkt->tex_ptr[0] != NULL ){
				DG_SetTexture(0, pkt->tex_ptr[0] );
			} else {
				DG_SetTexture(0, NULL );
			}
			if ( pkt->tex_ptr[1] != NULL ){
				DG_SetTexture(1, pkt->tex_ptr[1] );
			} else {
				DG_SetTexture(1, NULL );
			}
			if ( pkt->tex_ptr[2] != NULL ){
				DG_SetTexture(2, pkt->tex_ptr[2] );
#ifndef UVADJUST
				DG_SetVertexShaderConstant( CV_TEX2_SCALE, &pkt->tex_ptr[2]->vec1, 1);
				DG_SetVertexShaderConstant( CV_TEX2_OFFSET, &pkt->tex_ptr[2]->vec2, 1);
#endif
			} else {
				DG_SetTexture(2, NULL );
			}
			switch ( pkt->mtex_type ){
			  default:
			  case DG_MULTITEX_NORMAL:
				DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_NORMAL] );
				DG_SelectVertexShader( &DG_MultiTexObjVertexShader[0], vformat );
				break ;
			  case DG_MULTITEX_EMAP:
				DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP] );
				DG_SelectVertexShader( &DG_MultiTexObjVertexShader[1], vformat );
				break ;
			  case DG_MULTITEX_SMAP:
				///* 都合によりテクスチャの使用ステージを入れ替える */
				//DG_SetTexture( 1, pkt->tex_ptr[2] );
				//DG_SetTexture( 2, NULL );
				DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_SMAP] );
				DG_SelectVertexShader( &DG_MultiTexObjVertexShader[2], vformat );
				break ;
			  case DG_MULTITEX_EMAP2:
				DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP2] );
				DG_SelectVertexShader( &DG_MultiTexObjVertexShader[1], vformat );
				DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) );
			  case DG_MULTITEX_EMAP4:
				DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP4] );
				DG_SelectVertexShader( &DG_MultiTexObjVertexShader[1], vformat );
				break ;
			}
		}
		DG_DrawIndexedVertices( D3DPT_TRIANGLESTRIP, pkt->n_indices, index_addr );
		index_addr += pkt->n_indices ;
	}
#endif

	/* カリング禁止 */
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
#endif
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

	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
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
		ChainObj( obj );
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
	int		c_gid, gid ;
	OBJ_LIST		*obj_list, *mem_obj_list ;
	int			last_semi_trans = 0, use_semi_trans = 0 ;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;
	MARK( "chain2.c" );
	obj_buff = &que->objs_buffer ;

	c_gid = cp->group_id ;
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

	/* 描画オブジェクトをスクラッチパッドへ */
	obj_list = (OBJ_LIST*)scrpad->local_work ;
	oque = (DG_OBJS**)obj_buff->queue ;
	for ( max_objs = 0, i = obj_buff->n_queue ; i > 0 ; -- i, oque++ ) {
		objs = *oque ;
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
		if ( !( objs->group_id & cp->group_id ) ) continue ;

		//if ( objs->flag & ( DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWMAKE|DG_FLAG_SHADOWWRITE ) ) continue ;
		if ( objs->flag & ( DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWWRITE ) ) continue ;
		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		if ( objs->bound_mode == 2 ) continue ;
		if ( objs->flag & DG_FLAG_SEMITRANS ) use_semi_trans = 1 ;
		obj_list[max_objs].objs = objs ;
		obj_list[max_objs].use_tri = objs->tri_id ;
		max_objs++ ;
	}
	if ( max_objs == 0 ) return ;

	/* スクラッチパッド上のオブジェクトリストをテクスチャ毎にソートしてメモリに書き出す */
	mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;
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
	mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;
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

	DG_SetVertexStream( NULL );

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
	_DG_Chain2Chanl( cp, which, 0 );
}

void DG_Chain2ChanlLatter( DG_CHANL *cp, int which )
{
	_DG_Chain2Chanl( cp, which, 1 );
}

#endif

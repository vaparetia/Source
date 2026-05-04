//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xchain.c
	チャンネル処理ユニット／オブジェクトＤＭＡ接続ルーチン

	2002/02/09 K.Takabe
	$Id: xchain.c,v 1.4 2002/11/23 11:36:55 Yoshizawa1 Exp $

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
//u_long128	DG_AS_GetLocalMemoryAddress()[1024] ;
u_long64	DG_AS_GetLocalMemoryAddress()[ 1024 * 2 ];

/*----------------------------------------------------------------*/

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

DG_VERTEXSHADER	DG_ObjVertexShader[2];
DG_VERTEXFORMAT	DG_ObjVertexFormat[2] ;
extern DG_VERTEXSHADER	DG_Prim2VertexShader[2] ;
extern unsigned char VERTEX_SHADER_kms_ml[];
extern unsigned char VERTEX_SHADER_kms_sl[];
extern unsigned char VERTEX_SHADER_kms_sp[];
void DG_InitObjVertexShader(void)
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
		/* プリシェードライティング */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT3),		/* position */
		D3DVSD_REG(1,  D3DVSDT_NORMSHORT1),	/* blend */
		D3DVSD_REG(2,  D3DVSDT_NORMSHORT3),	/* normal */
		D3DVSD_REG(10, D3DVSDT_SHORT1),		/* dummy */
		D3DVSD_REG(7,  D3DVSDT_NORMSHORT2),	/* uv0 */
		D3DVSD_REG(8,  D3DVSDT_NORMSHORT2),	/* uv1 */
		D3DVSD_REG(9,  D3DVSDT_NORMSHORT2),	/* uv2 */
		D3DVSD_STREAM(1),
		D3DVSD_REG(3,  D3DVSDT_D3DCOLOR),	/* diffuse */
		D3DVSD_END()
	};

	/* 頂点フォーマット生成 */
	DG_MakeVertexFormat( &DG_ObjVertexFormat[0], dwObjDecl_00 );
	DG_MakeVertexFormat( &DG_ObjVertexFormat[1], dwObjDecl_01 );

	/* 通常ライティング */
	DG_MakeVertexShader( &DG_ObjVertexShader[0], VERTEX_SHADER_kms_sl, dwObjDecl_00 );
	/* プリシェードライティング */
	DG_MakeVertexShader( &DG_ObjVertexShader[1], VERTEX_SHADER_kms_sp, dwObjDecl_01 );
#endif
}
/* 共通のレンダリングステート＆頂点シェーダー定数の設定 */
static void DG_InitChanlRenderState( DG_CHANL *cp, int flag )
{
	static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{1.0f/4096.0f, 1.0f/255.0f, 0, 0},
	};
//@	FVECTOR		fog_param ;

	// カメラ/透視変換マトリクスの設定
	//DG_SetTransform(D3DTS_VIEW, &cp->eye_inv);
	//DG_SetTransform(D3DTS_PROJECTION, &cp->xpers );

	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);

	/* 頂点シェーダー用各種定数設定 */
	DG_SetVertexShaderConstant(CV_EYEPERS, &cp->eye_xpers, 4);
	DG_SetVertexShaderConstant(CV_PERS, &cp->xpers, 4);

	DG_SetVertexShaderConstant(CV_ZERO, &VS_Const[0], 1);
	DG_SetVertexShaderConstant(CV_ONE, &VS_Const[1], 1);
	DG_SetVertexShaderConstant(CV_HALF, &VS_Const[2], 1);
	DG_SetVertexShaderConstant(CV_SCALE, &VS_Const[3], 1);
	DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);

	/* バーテックスシェーダー読み込み */
	DG_ClearVertexShader();
	DG_LoadVertexShader( &DG_ObjVertexShader[0] );
	DG_LoadVertexShader( &DG_ObjVertexShader[1] );
	DG_LoadVertexShader( &DG_Prim2VertexShader[0] );
	DG_LoadVertexShader( &DG_Prim2VertexShader[1] );

	if ( flag != 0 ){
		//DG_SetVertexShaderConstant(CV_EYEPERS, &cp->raise_eye_xpers, 4);
		//DG_SetVertexShaderConstant(CV_PERS, &cp->raise_xpers, 4);
		/* Ｚバッファ更新中止 */
		DG_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);		
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
//@	FMATRIX        matTmp;
	u_long64        alpha;
	int           /*i,*/ j, no_tex_flag = 0 ;
	unsigned short	*index_addr ;

	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	if ( obj->flag & DG_FLAG_PAINT ){
		/* プリシェードライティング */
		/* 頂点ストリーム及び入力頂点フォーマットの設定 */
		DG_SetPixelShader( NULL );
		DG_SetVertexBuffer( 0, obj->vbuff, obj->stride );
		//DG_SetVertexBuffer( 1, obj->rgbs, sizeof(int) );
		DG_SetVertexBuffer( 1, obj->norms, sizeof(int) );
		DG_SetVertexStream( &DG_ObjVertexFormat[ 1 ] );
		/* 頂点シェーダー選択 */
		DG_SelectVertexShader( &DG_ObjVertexShader[1], &DG_ObjVertexFormat[ 1 ] );
	} else {
		/* ３並行光＆１環境光ライティング */
		/* 頂点ストリーム及び入力頂点フォーマットの設定 */
		DG_SetPixelShader( NULL );
		DG_SetVertexBuffer( 0, obj->vbuff, obj->stride );
		DG_SetVertexStream( &DG_ObjVertexFormat[ 0 ] );
		/* 頂点シェーダー選択 */
		DG_SelectVertexShader( &DG_ObjVertexShader[0], &DG_ObjVertexFormat[ 0 ] );
	}

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
	if (obj->parent != -1) {
		DG_SetVertexShaderConstant(CV_WORLD1, &obj->inv_mat, 4);
	}

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
		alpha = pkt->tex_ptr[0]->alpha.data;
		if ( pkt->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
		} else {
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
		}
		if ( no_tex_flag == 0 ){
			DG_SetTexture( 0, pkt->tex_ptr[0] );
		}
		DG_DrawIndexedVertices( D3DPT_TRIANGLESTRIP, pkt->n_indices, index_addr );
		index_addr += pkt->n_indices ;
	}
#endif

	/* カリング禁止 */
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
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
	sort_work = (SORT_WORK*)DG_AS_GetLocalMemoryAddress() ;

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
	sort_work = (SORT_WORK*)DG_AS_GetLocalMemoryAddress() ;

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
	extern void	*DG_Prim2OT[64] ;
	int			before_type = 0 ;

	sort_work = (SORT_WORK*)DG_AS_GetLocalMemoryAddress() ;

	for ( k = 63 ; k >= 0  ; k-- ){
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
				else									obj_flag = 0 ;
			}

			if ( obj_flag ){
				/* 直前の描画タイプがプリミティブなら透視変換マトリクスを優先上げ要素なしに変更 */
				if ( before_type == 1 ){
					//DG_SetVertexShaderConstant(CV_EYEPERS, &cp->eye_xpers, 4);
					DG_SetVertexShaderConstant(CV_PERS, &cp->xpers, 4);
				}
				//if ( GV_PadData[1].press & PAD_A ) printf( "objs: %d\n", list->z*64-0x7fffff );
				/* ソートしたオブジェクトを接続 */
				next_addr = list->tag ;
				obj = list->obj ;
#ifdef LIBDG_PERFORMANCE
				scrpad->verts_count = 0 ;
				scrpad->pack_count = 0 ;
#endif
				scrpad->fog = obj->fog ;
				scrpad->matrix_addr = obj->matrix_addr ;
				DG_SetRenderState(D3DRS_FOGENABLE, TRUE);
				ChainObj( obj );
#ifdef LIBDG_PERFORMANCE
				scrpad->model[ obj->bound_mode ].n_obj++ ;
				scrpad->model[ obj->bound_mode ].n_packs += scrpad->pack_count ;
				scrpad->model[ obj->bound_mode ].n_verts += scrpad->verts_count ;
#endif
				before_type = 0 ;	/* モデル描画後フラグ設定 */
			} else {
				/* 直前の描画タイプがモデルなら透視変換マトリクスを優先上げ要素付きに変更 */
				if ( before_type == 0 ){
					//DG_SetVertexShaderConstant(CV_EYEPERS, &cp->raise_eye_xpers, 4);
					DG_SetVertexShaderConstant(CV_PERS, &cp->raise_xpers, 4);
				}
				//if ( GV_PadData[1].press & PAD_A ){
				//	printf( "prim: %d(%s)\n", prim_packet->sort_z*64-0x7fffff,
				//		   ((DG_PRIM2*)(prim_packet->prim))->fname );
				//}
				/* ソートしたプリミティブを接続 */
				next_addr_prim = prim_packet->next_addr;
				/* PRIM2描画 */
				DG_WritePrimPacks(prim_packet, cp, which);
				before_type = 1 ;	/* プリミティブ描画後フラグ設定 */
			}

			if ( DG_ISCHECK_PACKETBUFFER() ){
				k = -1 ;
				break ;	/* メモリが足りなくなったら強制終了 */
			}

		}
	}
	//DG_CurrentDmaAddr = DG_EndCacheFIFO( scrpad->local_work );

	DG_SetPixelShader( NULL );
	DG_SetVertexStream( NULL );

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

	que = cp->obj_queue ;

	MARK( "chain.c(sort)" );
	if ( que == NULL ) return ;
	obj_buff = &que->objs_buffer ;
	c_gid = cp->group_id ;

	/* ワーク初期化 */
	sort_work = (SORT_WORK*)DG_AS_GetLocalMemoryAddress() ;
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
	oque = (DG_OBJS**)obj_buff->queue ;
	for ( i = obj_buff->n_queue ; i > 0 ; -- i, oque++ ) {
		objs = *oque ;
		if ( objs->flag & DG_FLAG_MULTITEX ) continue ;
		if ( objs->flag & scrpad->invisible_flag ) continue ;
		if ( objs->flag & DG_FLAG_PLUGINDRAW ) continue ;

		/* cp->group_id実験 （2000/01/11 M.Sonoyama） */
		if ( !( objs->group_id & cp->group_id ) ) continue ;

		if ( objs->flag & ( DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWWRITE ) ) continue ;
		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		MakeSortList( *oque );
	}

	/* ソート */
	SortList();

	/* スタティックプッシュバッファへの記録開始 */
	DG_OpenDmaTask();

	/* レンダリングステート＆頂点バッファ定数の設定 */
	DG_InitChanlRenderState( cp, 1 );

	/* ＤＭＡ書き出し処理 */
	SortChainObjs( cp, which );

	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.trans_model[0] = scrpad->model[0] ;
	DG_PerformanceData.trans_model[1] = scrpad->model[1] ;
#endif
}

/*----------------------------------------------------------------*/

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

	que = cp->obj_queue ;
	MARK( "chain.c(normal)" );
	if ( que == NULL ) return ;
	obj_buff = &que->objs_buffer ;

	/* スクラッチパッド内の必要なワークを初期化する */
	c_gid = cp->group_id ;
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
		if ( objs->flag & DG_FLAG_MULTITEX ) continue ;
		if ( objs->flag & scrpad->invisible_flag ) continue ;
		if ( objs->flag & DG_FLAG_PLUGINDRAW ) continue ;

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

	/* スタティックプッシュバッファへの記録開始 */
	DG_OpenDmaTask();

	/* 半透明を使用したオブジェクトがある場合には事前にマスクを生成 */
	if ( use_semi_trans ){
		DG_MakeStencilMask();
	}

	/* chanl 毎の RenderState を設定する */
	DG_InitChanlRenderState( cp, 0 );

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
			//size = DG_WriteTextureChangePacks2( scrpad->dma_buffer, tex_list, which );
			//DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
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
		// Xbox追加:ユーザ定義シェーダ使用コールバック
		if (mem_obj_list->objs->exec_func != NULL) {
			mem_obj_list->objs->exec_func(cp, which,
										  mem_obj_list->objs,
										  mem_obj_list->objs->extend_data);
		} else {
			ChainObjs(mem_obj_list->objs);			
		}
		mem_obj_list++ ;

		if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
	}

	/* 半透明処理無効化 */
	if ( last_semi_trans != 0 ){
		DG_SetRenderState( D3DRS_STENCILENABLE, FALSE );
	}
	/* Ｚ圧縮を活用するためにステンシルをクリア */
	if ( use_semi_trans ){
		DG_Clear(0, NULL, D3DCLEAR_STENCIL, 0, 0.0f, 0);
	}

	DG_SetPixelShader( NULL );
	DG_SetVertexStream( NULL );
	//DG_EndScene();

	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();


#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.normal_model[0] = scrpad->model[0] ;
	DG_PerformanceData.normal_model[1] = scrpad->model[1] ;
#endif
}

#endif

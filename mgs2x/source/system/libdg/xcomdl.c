//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xcomdl.c
	チャンネル処理ユニット／共有モデルオブジェクト処理ルーチン

	2002/03/14 K.Takabe
	$Id: xcomdl.c,v 1.1.1.3 2002/11/19 11:42:33 Yoshizawa1 Exp $

*/
/*
	チャンネル処理ユニット／共有モデルオブジェクト処理ルーチン

	この共有モデルオブジェクトとは１ストリップで構成されている
	単一のモデル対し複数のマトリクスを用意することによって
	オブジェクトを複数表示することを可能とした特殊オブジェクトです。
	シェーディング処理は行ないませんが、オブジェクト毎にＲＧＢＡを
	指定できるようになっています。
	基本的にソートなどは行なわないため、不透明モデルと同じ扱いに
	なりますが、描画結果がソートに影響しない場合に限り半透明も
	使用可能です。


	void		DG_ComdlChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた共有モデルオブジェクトのパケット作成などを行なう


	DG_COMDL*	DG_MakeComdl( DG_MDLPACK *mdl_pack, int flag, int n_comdl, int chanl )
	DG_MDLPACK	*mdl_pack ;		１ストリップ分のモデルデータ
	int			flag ;			処理フラグ
	int			n_comdl ;		オブジェクト数
	int			chanl ;			処理チャンネル

	共有モデルオブジェクトを作成する（作成後はDG_QueueComdlObjs()で登録すること）


	void	DG_FreeComdl( DG_COMDL *comdl )
	DG_COMDL	*comdl ;		開放オブジェクト

	メモリを破棄する（これを呼ぶ前にDG_DequeueComdlObjs()でシステムから削除しておくこと）

	------------------------------------------------


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

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"
#include	"utl_dma.h"

#include "shader.h"


/* ---------------------------------------------------------------- */

/*
 * comdl用頂点シェーダ
 */
static DG_VERTEXSHADER	DG_ComdlVertexShader[1] ;
static DG_VERTEXFORMAT	DG_ComdlVertexFormat[1] ;
extern unsigned char VERTEX_SHADER_comdl[];
void DG_InitComdlVertexShader(void)
{
#if 0 //BP_RENDER
	DWORD dwObjDecl_00[] = {
		/* 通常ライティング */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT3),		/* position */
		D3DVSD_REG(7,  D3DVSDT_NORMSHORT2),	/* uv0 */
		D3DVSD_END()
	};
	/* 頂点フォーマット生成 */
	DG_MakeVertexFormat( &DG_ComdlVertexFormat[0], dwObjDecl_00 );
	/* 通常ライティング */
	DG_MakeVertexShader( &DG_ComdlVertexShader[0], VERTEX_SHADER_comdl, dwObjDecl_00);
#endif
}

typedef struct {
	short		x, y, z ;
	short		u, v ;
} DG_VERTEX_COMDL;

/* ---------------------------------------------------------------- */
	/*
		プラグイン固有設定
	*/

/* メインチャンネルプラグイン用設定 */
	/* プラグイン固有ＩＤ */
#define PLUGIN_ID		(6897906)	/* "comdl" */
	/* プラグイン初期化フラグ */
#define PLUGIN_FLAG		(DG_PLUGIN_FLAG_OBJBUFFER|DG_PLUGIN_FLAG_ENABLE)
	/* プラグイン実行フェーズ指定 */
#define PLUGIN_PHASE	(DG_PLUGIN_PHASE_NORMAL)
	/* プラグインプライオリティ */
#define PLUGIN_PRIO		(DG_PLUGIN_PRIO_NORMAL)
	/* 最大使用オブジェクトキューサイズ */
#define PLUGIN_USE_QUEUE	(128)



/* ---------------------------------------------------------------- */
	/*
		プラグイン固有変数
	*/

	/* プラグイン初期化フラグ */
static int	PluginStartFlag = 0 ;
static DG_PLUGIN	ComdlPlugin ;

/* ---------------------------------------------------------------- */
	/*
		プラグイン内部使用関数
	*/

static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status );

/* ---------------------------------------------------------------- */
	/*
		プラグイン起動
	*/
void DG_AddPluginComdl( void )
{
	if ( PluginStartFlag != 0 ) return ;

	/* プラグイン作成 */
	DG_MakePlugin( &ComdlPlugin, PLUGIN_ID, PLUGIN_FLAG, PLUGIN_PHASE, PLUGIN_PRIO, PluginActor, PLUGIN_USE_QUEUE );
	DG_AddPlugin( &ComdlPlugin );

	PluginStartFlag = 1 ;
}

	/*
		プラグイン終了
	*/
void DG_DeletePluginComdl( void )
{
	while ( PluginStartFlag != 0 ){
		DG_DeletePlugin( &ComdlPlugin );
		DG_FreePlugin( &ComdlPlugin );
	}
	PluginStartFlag = 0 ;
}

/* ---------------------------------------------------------------- */
	/*
		オブジェクト登録
	*/
int DG_QueueComdlObjs( DG_COMDL *comdl )
{
	if ( PluginStartFlag == 0 ){
		return ( -1 );
	}
	DG_QueueUserObject( ComdlPlugin.obj_buffer, comdl );
	return ( 0 );
}

	/*
		オブジェクト削除
	*/
void DG_DequeueComdlObjs( DG_COMDL *comdl )
{
	if ( PluginStartFlag == 0 ){
		return ;
	}
	DG_DequeueUserObject( ComdlPlugin.obj_buffer, comdl );
}

/* ---------------------------------------------------------------- */


/* ＶＵ１ワークパラメータ */
typedef struct _vu1_work {
	FVECTOR	clamp_min ;		/* クランプ処理用最小値（現在未使用） */
	FVECTOR	clamp_max ;		/* クランプ処理用最大値（現在未使用） */
	float	fog_param1, fog_param2, fog_param3, fog_param4 ;	/* フォグパラメータ */
	float	color_clip, poly_alpha, specular_mul, specular_clip ;	/* 各種定数 */
	FVECTOR	scale ;			/* プリミティブ座標算出用スケール値（xyz=pvec*scale+offset） */
	FVECTOR	offset ;		/* プリミティブ座標算出用オフセット */
	float	param1, param2, param3, param4 ;					/* 各種定数２ */
	FVECTOR	pad ;
	FMATRIX	pers ;			/* 透視変換マトリクス */
} Vu1Work ;

typedef struct {
	DG_COMDL	*comdl ;
	int			use_tri ;
} OBJ_LIST ;

typedef struct _scrpad_work {
	FMATRIX	mat ;
	u_long128	dma_buffer[256] ;
#ifdef LIBDG_PERFORMANCE
	PERFORMANCE_PACKET_INFO	packet_info ;
#endif

	/* ローカルワーク */
	u_long128	local_work[0] ;
} ScrpadWork ;

int			*func_list ;

/* 汎用メモリワーク（主にスクラッチパッド上で構成し終わったデータなどを退避） */
extern u_long128	DG_AS_GetLocalMemoryAddress()[1024] ;


/*----------------------------------------------------------------*/
	/*
		オブジェクトパケット用初期化パケットデータの書き出し
	*/

/*----------------------------------------------------------------*/
	/*
		初期化実行パケットの設定
	*/

	/*
		各オブジェクトのＤＭＡパケットの作成
	*/
static void WriteObjPacks( DG_COMDL *comdl )
{
	int		i ;
	DG_COMDL_POS	*pos ;
	FVECTOR			col ;
	DG_MDLPACK		*mdl_pack ;

	DG_SetTexture(0, &comdl->tex->tex_trans );
	if ( comdl->flag & DG_COMDL_SEMITRANS ){
		//DG_SetAlphaMode( comdl->tex->tex_trans.alpha.data );
	} else {
		DG_SetAlphaMode( 0 );
	}
	if ( comdl->flag & DG_COMDL_NOFOG ){
		DG_SetRenderState( D3DRS_FOGENABLE, FALSE );
	} else {
		DG_SetRenderState( D3DRS_FOGENABLE, TRUE );
	}

	DG_SetVertexBuffer( 0, comdl->vbuff, sizeof(DG_VERTEX_COMDL) );
	DG_SetVertexStream( &DG_ComdlVertexFormat[0] );

	mdl_pack = comdl->mdl_pack ;

	pos = comdl->pos ;
	for ( i = 0 ; i < comdl->n_objs ; i++ ){
		DG_SetVertexShaderConstant(CV_WORLD0, &pos->world, 4);
		col.vx = pos->color.vx ;
		col.vy = pos->color.vy ;
		col.vz = pos->color.vz ;
		col.vw = pos->color.vw ;
		DG_SetVertexShaderConstant(CV_LIGHTCOL+3, &col, 4);
		//DG_DrawIndexedVertices( D3DPT_TRIANGLESTRIP, mdl_pack->n_indices, mdl_pack->index );
		DG_DrawVertices( D3DPT_TRIANGLESTRIP, 0, mdl_pack->n_indices );
		pos++ ;
	}
}
/*----------------------------------------------------------------*/
	/*
		プラグイン実行アクター
	*/
static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status )
{
	DG_COMDL		*comdl, **que ;
	DG_COMDL_POS	*pos ;
	int		i, j, size, n, old_tex_code, invisible_flag ;
	ScrpadWork		*scrpad ;
	DG_TEXTURE_LIST	*tex_list, **tex_list_p ;
	OBJ_LIST		*obj_list, *mem_obj_list ;
	int				max_objs, max_mem_objs ;
	static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{1.0f/4096.0f, 1.0f/255.0f, 0, 0},
	};

	MARK(__FILE__);
	scrpad = SCRPAD_ADDR ;
#ifdef LIBDG_PERFORMANCE
	scrpad->packet_info = DG_PerformanceData.comodel ;
#endif

	/*
		キュー数のチェック
	*/
	if ( obj_buff->n_queue == 0 ) return ;

	invisible_flag = DG_COMDL_INVISIBLE0 << cp->chanl_num ;

	/* 描画オブジェクトをスクラッチパッドへ */
	obj_list = (OBJ_LIST*)scrpad->local_work ;
	que = (DG_COMDL**)obj_buff->queue ;
	for ( max_objs = 0, i = obj_buff->n_queue ; i > 0 ; -- i, que++ ) {
		comdl = *que ;
		if ( comdl->flag & invisible_flag ) continue ;
		/* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
		if ( !( comdl->group_id & cp->group_id ) ) continue ;
		obj_list[max_objs].comdl = comdl ;
		obj_list[max_objs].use_tri = comdl->tri_id ;
		max_objs++ ;
	}
	if ( max_objs == 0 ) return ;


	/* スタティックプッシュバッファへの記録開始 */
	DG_OpenDmaTask();

	/* 各種設定 */
	DG_SetTransform(D3DTS_VIEW, &cp->eye_inv);
#if 0
	DG_SetTransform(D3DTS_PROJECTION, &cp->xpers);
#else
	DG_SetTransform(D3DTS_PROJECTION, &cp->raise_xpers);
#endif
	DG_SetVertexShaderConstant(CV_ZERO, &VS_Const[0], 1);
	DG_SetVertexShaderConstant(CV_ONE, &VS_Const[1], 1);
	DG_SetVertexShaderConstant(CV_HALF, &VS_Const[2], 1);
	DG_SetVertexShaderConstant(CV_SCALE, &VS_Const[3], 1);
	DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);
	/* カメラ/透視変換マトリクスの設定 */
	//DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
	DG_SetVertexShaderConstant(CV_EYEPERS, &cp->eye_xpers, 4);
	/* フォグ有効化 */
	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);
	/* アルファブレンディングOFF */
	//DG_AlphaBlendDisable();
	/* 頂点シェーダー設定 */
	DG_ClearVertexShader();
	DG_LoadVertexShader( &DG_ComdlVertexShader[0] );
	DG_SelectVertexShader( &DG_ComdlVertexShader[0], &DG_ComdlVertexFormat[0] );
	/* ピクセルシェーダー設定 */
	DG_SetPixelShader( NULL );

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
			mem_obj_list->comdl = obj_list[i].comdl ;
			mem_obj_list->use_tri = (int)tex_list ;	/* tri_codeの替わりにDG_TEXTUER_LISTへのポインタを入れる */
			mem_obj_list++ ;
			max_mem_objs++ ;
		}
	}

	/* 初期化パケットの設定 */

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
		comdl = mem_obj_list->comdl ;

		/* 初期化パケット転送 */

		/* 各オブジェクトのマトリクス転送パケット設定 */
		n = comdl->n_objs ;
		pos = comdl->pos ;
#ifdef LIBDG_PERFORMANCE
		scrpad->packet_info.n_obj++ ;
		scrpad->packet_info.n_packs += comdl->n_objs ;
		scrpad->packet_info.n_verts += comdl->packet[0].dg_comdl_packet_data.n_verts * comdl->n_objs ;
#endif
		WriteObjPacks( comdl );
		mem_obj_list++ ;

		if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
	}

	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.comodel = scrpad->packet_info ;
#endif

}

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
	/*
		共有モデルオブジェクトの作成
	*/
DG_COMDL*	DG_MakeComdl( DG_MDLPACK *mdl_pack, int flag, int n_comdl, int chanl )
{
	DG_COMDL	*comdl ;
	int			size, i ;
	DG_VERTEX_COMDL	*comdl_verts ;
	short			*verts, *uvs ;

	flag |= DG_COMDL_INVISIBLE2|DG_COMDL_INVISIBLE3 ;
	ASSERT( mdl_pack != NULL )
	size = sizeof(DG_COMDL) + sizeof(DG_COMDL_POS) * n_comdl ;
	if ( ( comdl = GV_Malloc( size ) ) == NULL ){
		return NULL ;
	}
	GV_ZeroMemory( comdl, size );

	comdl->flag = flag ;
	comdl->n_objs = n_comdl ;
	comdl->chanl = chanl ;
	comdl->tri_id = DG_SearchTriFromTex( (DG_TEX*)( mdl_pack->tex_id[0] ) );

	comdl->mdl_pack = mdl_pack ;
	comdl->tex = mdl_pack->tex_id[0] ;

#ifndef NEW_KMX_FORMAT
	/* 専用頂点バッファの作成 */
	comdl->vbuff = DG_AllocLocalVideoMemory( sizeof(DG_VERTEX_COMDL) * mdl_pack->n_verts );
	verts = mdl_pack->verts ;
	uvs = mdl_pack->uvs[ 0 ];
	comdl_verts = (DG_VERTEX_COMDL*)comdl->vbuff ;
	for ( i = 0 ; i < mdl_pack->n_verts ; i++ ){
		comdl_verts->x = verts[0] ;
		comdl_verts->y = verts[1] ;
		comdl_verts->z = verts[2] ;
		comdl_verts->u = uvs[0] * ( 32767.0f / 4096.0f );
		comdl_verts->v = uvs[1] * ( 32767.0f / 4096.0f ) ;
		verts += 4 ;
		uvs += 2 ;
		comdl_verts++ ;
	}
#else
#if 0
	{/* 専用頂点バッファの作成 */
		DG_MDL			*mdl ;
		DG_VERTEX_KMSS	*verts ;
		mdl = mdl_pack->rgbs ;		/* xloader.cにて細工をしているので注意！ */
		comdl->vbuff = DG_AllocLocalVideoMemory( sizeof(DG_VERTEX_COMDL) * mdl->n_verts );
		verts = mdl->vbuff ;
		comdl_verts = (DG_VERTEX_COMDL*)comdl->vbuff ;
		for ( i = 0 ; i < mdl->n_verts ; i++ ){
			comdl_verts->x = verts->vx ;
			comdl_verts->y = verts->vy ;
			comdl_verts->z = verts->vz ;
			comdl_verts->u = verts->u0 ;
			comdl_verts->v = verts->v0 ;
			verts = (void*)( (int)verts + mdl->stride ) ;
			comdl_verts++ ;
		}
	}
#else
	{/* 専用頂点バッファの作成 */
		DG_MDL			*mdl ;
		DG_VERTEX_KMSS	*verts ;
		unsigned short	*index ;
		mdl = mdl_pack->rgbs ;		/* xloader.cにて細工をしているので注意！ */
		comdl->vbuff = DG_AllocLocalVideoMemory( sizeof(DG_VERTEX_COMDL) * mdl_pack->n_indices );
		comdl_verts = (DG_VERTEX_COMDL*)comdl->vbuff ;
		index = mdl_pack->index ;
		for ( i = 0 ; i < mdl_pack->n_indices ; i++ ){
			verts = (void*)( (int)mdl->vbuff + mdl->stride * (*index) ) ;
			comdl_verts->x = verts->vx ;
			comdl_verts->y = verts->vy ;
			comdl_verts->z = verts->vz ;
			comdl_verts->u = verts->u0 ;
			comdl_verts->v = verts->v0 ;
			comdl_verts++ ;
			index++ ;
		}
	}
#endif
#endif

	return ( comdl ) ;
}

	/*
		メモリ開放
	*/
void	DG_FreeComdl( DG_COMDL *comdl )
{
	DG_DelayedFreeLocalVideoMemory( comdl->vbuff );
	GV_DelayedFree( comdl );
}

#endif

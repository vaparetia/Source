//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xopt_cmf.c
	チャンネル処理ユニット／光学迷彩オブジェクトＤＭＡ接続ルーチン

	2002/03/27 K.Takabe
	$Id: xopt_cmf.c,v 1.1.1.3 2002/11/19 11:42:36 Yoshizawa1 Exp $

*/
/*

	void		DG_OptCamouflageChainChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた各オブジェクトをＤＭＡパケットに接続する

*/

#ifdef KP_XBOX //BP

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"

#include "shader.h"

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

/* ---------------------------------------------------------------- */
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
extern u_long128	DG_AS_GetLocalMemoryAddress()[1024] ;


/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
static DG_VERTEXSHADER	DG_OptcmfVertexShader[2] ;
static DG_VERTEXFORMAT	DG_OptcmfVertexFormat[ 1 ];
extern unsigned char VERTEX_SHADER_optcmf00[];
void DG_InitOptcmfVertexShader(void)
{
#if 0 //BP_RENDER
	DWORD dwObjDecl_00[] = {
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT3),		/* position */
		D3DVSD_REG(1,  D3DVSDT_NORMSHORT1),	/* blend */
		D3DVSD_REG(2,  D3DVSDT_NORMSHORT3),	/* normal */
		D3DVSD_REG(10, D3DVSDT_SHORT1),		/* dummy */
		D3DVSD_END()
	};
	DG_MakeVertexFormat( &DG_OptcmfVertexFormat[ 0 ], dwObjDecl_00 );
	DG_MakeVertexShader( &DG_OptcmfVertexShader[0], VERTEX_SHADER_optcmf00, dwObjDecl_00 );
#endif
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
	FMATRIX        matTmp;
	u_long64        alpha;
	int           i, j;
	unsigned short	*index_addr ;
	
	pkt = obj->packets ;
	if (pkt == NULL) return;
	
	//DG_SetTexture(0, pkt->tex_ptr[0]->ptex ); 	/* TODO: マルチテクスチャ管理 */

	DG_SetVertexBuffer( 0, obj->vbuff, obj->stride );
	DG_SetVertexStream( &DG_OptcmfVertexFormat[0] );

#if 0
	if ( scrpad->last_light_addr != obj->light ){
		DG_SetVertexShaderConstant(CV_LIGHTVEC, &obj->light[0], 4);
		DG_SetVertexShaderConstant(CV_LIGHTCOL, &obj->light[1], 4);
		scrpad->last_light_addr = obj->light ;
	}
#endif
	if (obj->parent != -1) {
		DG_SetVertexShaderConstant(CV_WORLD1, &obj->inv_mat, 4);
	}
	DG_SetPixelShader(0);
	DG_SelectVertexShader( &DG_OptcmfVertexShader[0], &DG_OptcmfVertexFormat[0] );

#if 0
	/* 法線投影マトリクスを計算 */
	_sceVu0MulMatrix( &scrpad->local_light, &scrpad->eye_inv, &obj->world );
	//scrpad->local_light = obj->world ;
	/* 法線投影マトリクスへの回転エフェクト */
	_sceVu0MulMatrix( &scrpad->local_light, &scrpad->rot_mat, &scrpad->local_light );

	/* フォグ計算 */
	if ( !( obj->flag & DG_FLAG_NOFOG ) ){
		float	f ;
		int		ifog ;
		f =  DG_FogParam1 * obj->sort_z + DG_FogParam2 ;
		f = DG_MAX( f, 0.0f );
		f = DG_MIN( f, 255.0f );
		ifog = DG_FTOI( f ) ;
		scrpad->fog = ( ifog << 4 ) | 0x8000 ;
	} else {
		scrpad->fog = ( 255 << 4 ) | 0x8000 ;
	}
#endif

	DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);

	index_addr = obj->index ;
	for (j = obj->n_packs; j > 0; j --, pkt++) {
		DG_DrawIndexedVertices( D3DPT_TRIANGLESTRIP, pkt->n_indices, index_addr );
		index_addr += pkt->n_indices ;
	}

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


#if 0
	/* 光学迷彩パラメータの設定 */
	scrpad->local_color.m[0][0] = ( DRAW_WIDTH - 32 ) / 2.0f ;
	scrpad->local_color.m[0][1] = ( DRAW_HEIGHT - 16 ) / 2.0f ;
	scrpad->local_color.m[1][0] = DRAW_WIDTH / 2.0f ;
	scrpad->local_color.m[1][1] = DRAW_HEIGHT / 2.0f ;
	scrpad->local_color.m[2][0] = 16.0f ;
	scrpad->local_color.m[2][1] = 8.0f ;

#if 0
	scrpad->local_color.m[3][0] = 120.0f ;
	scrpad->local_color.m[3][1] = 150.0f ;
	scrpad->local_color.m[3][2] = 100.0f ;
	scrpad->local_color.m[3][3] = 96.0f ;
#else
	scrpad->local_color.m[3][0] = 128.0f ;
	scrpad->local_color.m[3][1] = 128.0f ;
	scrpad->local_color.m[3][2] = 128.0f ;
	scrpad->local_color.m[3][3] = 96.0f ;
#endif

#else
	if ( objs->extend_data == NULL ){
		printf("opt_cmf.c:error\n");
		return ;
	}
	scrpad->local_color = *(FMATRIX*)objs->extend_data ;
	scrpad->rot_mat = ((FMATRIX*)objs->extend_data)[1] ;
#endif
	/* 光学迷彩用パラメータをＸＢＯＸ用にコンバートして頂点シェーダー定数として登録 */
	_sceVu0MulMatrix( &scrpad->local_light, &scrpad->rot_mat, &scrpad->eye_inv );
	DG_SetVertexShaderConstant(CV_OPTCMF_EYEPERS, &scrpad->local_light, 4 );
	{/* 光学迷彩化用パラメータ設定 */
		//scrpad->local_color.m[0][0] = ( DRAW_WIDTH - 32 ) / 2.0f ;
		scrpad->local_color.m[0][1] *= -1.0f ;
		scrpad->local_color.m[1][0] += ( DISPLAY_WIDTH - DRAW_WIDTH ) / 2 ;
		scrpad->local_color.m[1][1] += ( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2 ;
		//scrpad->local_color.m[2][0] = 16.0f ;
		//scrpad->local_color.m[2][1] = 8.0f ;
		DG_SetVertexShaderConstant( CV_LIGHTCOL, &scrpad->local_color, 4 );
	}

	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		/* クリップアウトチェック */
		if ( obj->bound_mode & 2 ) continue ;
		if ( obj->model->type & DG_TYPE_TRANS ) continue ;
		if ( obj->flag & scrpad->invisible_flag ) continue ;

		ChainObj( obj );
	}

}

/*----------------------------------------------------------------*/
static void DG_InitChanlRenderState( DG_CHANL *cp )
{
	static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{1.0f/4096.0f, 1.0f/255.0f, 0, 0},
	};

	// カメラ/透視変換マトリクスの設定
	DG_SetTransform(D3DTS_VIEW, &cp->eye_inv);
	DG_SetTransform(D3DTS_PROJECTION, &cp->xpers );

	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);

	/* 頂点シェーダー用各種定数設定 */
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


	DG_ClearVertexShader();
	DG_LoadVertexShader( &DG_OptcmfVertexShader[0] );
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
	int		i, size, exec_flag = 0, max_objs ;
	int		c_gid, gid ;
	int		last_semi_trans = 0 ;
	OBJ_LIST		*obj_list, *mem_obj_list ;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;
	obj_buff = &que->objs_buffer ;

	c_gid = cp->group_id ;
	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
	scrpad->buffer_switch = 0 ;
	scrpad->eye_pers = cp->raise_eye_pers ;
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
		if ( !( objs->group_id & cp->group_id ) ) continue ;

		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		if ( objs->bound_mode == 2 ) continue ;
		obj_list[max_objs].objs = objs ;
		max_objs++ ;
	}
	if ( max_objs == 0 ) return ;

	/* スクラッチパッド上のオブジェクトリストをメモリに書き出す */
	mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;
	for ( i = 0 ; i < max_objs ; i++ ) {
		mem_obj_list->objs = obj_list[i].objs ;
		mem_obj_list++ ;
	}
	mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;

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
		if ( !( objs->group_id & cp->group_id ) ) continue ;

		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		if ( objs->bound_mode == 2 ) continue ;

#if 0
		/* 強制半透明描画チェック */
		if ( objs->flag & DG_FLAG_SEMITRANS ){
			if ( last_semi_trans == 0 ){
				size = DG_WriteRasterMaskPacks( scrpad->dma_buffer, 1 );
				DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
			}
			last_semi_trans = 1 ;
		} else {
			if ( last_semi_trans != 0 ){
				size = DG_WriteRasterMaskPacks( scrpad->dma_buffer, 0 );
				DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
			}
			last_semi_trans = 0 ;
		}
#endif

		/* 同じモデルをテクスチャを変えて２回連続で描画 */
		DG_SetTexture( 0, NULL );
		DG_SetAlphaMode( SCE_GS_SET_ALPHA(2,2,2,1,128) );	
		ChainObjs( objs );
		DG_SetTextureDirect( 0, DG_SystemTexture[ 2 ].tex_trans.ptex );
		DG_SetAlphaMode( SCE_GS_SET_ALPHA(0,1,0,1,0) );	
		ChainObjs( objs );
		exec_flag = 1 ;

		if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
	}

#if 0
	/* 半透明処理無効化 */
	if ( last_semi_trans != 0 ){
		size = DG_WriteRasterMaskPacks( scrpad->dma_buffer, 0 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
	}
#endif


	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();


}

#endif

/*
	xcomdl.c
	チャンネル処理ユニット／共有モデルオブジェクト処理ルーチン

	2002/03/14 K.Takabe
	$Id: wcomdl.c,v 1.17 2002/12/15 15:34:42 takaki Exp $

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

#ifdef _XBOX
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
#include	"private.h"

#include "shader.h"


#define	__COMDL_USE_D3DVERTEXBUFFER__	(TRUE)	// Direct3Dの頂点バッファを作成/使用する

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
static void WriteObjPacks( DG_COMDL *comdl );
static void WriteObjPacksNVS( DG_COMDL *comdl );

static	void	*_write_obj_packs_func ;

/*----------------------------------------------------------------*/

/* ---------------------------------------------------------------- */

/*
 * comdl用頂点シェーダ
 */
static DG_VERTEXSHADER	DG_ComdlVertexShader[1] ;
extern unsigned char VERTEX_SHADER_comdl[];
void DG_InitComdlVertexShader(void)
{
	DWORD dwObjDecl_00[] = {
		/* 通常ライティング */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT4),		/* position, pad */
		D3DVSD_REG(7,  D3DVSDT_SHORT2),		/* uv0 */
		D3DVSD_END()
	};

	if( DG_CheckUseVertexShader() )
	{
		/* 通常ライティング */
		DG_MakeVertexShader( &DG_ComdlVertexShader[0], VSHT_wcomdl, dwObjDecl_00);
	}

	/* 描画方式選択 */
	if( DG_CheckUseVertexShader() )
	{
		_write_obj_packs_func = WriteObjPacks ;
	}
	else
	{
		_write_obj_packs_func = WriteObjPacksNVS ;
	}
}

void DG_ReleaseComdlVertexShader(void)
{
	DG_KillVertexShader( &DG_ComdlVertexShader[0] );
}

typedef struct {
	short		x, y, z, w ;
	short		u, v ;
} DG_VERTEX_COMDL;

typedef struct {
	float		x, y, z ;
	float		u, v ;
} DG_VERTEX_COMDL_FVF;
#define	D3DFVF_DG_VERTEX_COMDL_FLAG	( D3DFVF_XYZ		\
									| D3DFVF_TEX1		\
									| D3DFVF_TEXCOORDSIZE2(0))

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
extern u_long128	DG_LocalMemory[1024] ;


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
#if	!__COMDL_USE_D3DVERTEXBUFFER__
	DWORD			sofs ;
#endif
	int				n_indices ;
	int				n_objs ;

	/*-- 描画する必要が有るか検査 --*/
#ifdef _WINDOWS
	pos = comdl->pos ;
	for(i=comdl->n_objs; i>0; i--, pos++)
	{
		if( pos->color.vw ){ break ; }	// α検査
	}
	if( !i ){ return ; }	// (α>0)の物が無いので、描画必要無し
#endif
	/*-- --*/

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

	mdl_pack  = comdl->mdl_pack ;
	n_indices = mdl_pack->n_indices ;

#if	__COMDL_USE_D3DVERTEXBUFFER__
	{
		LPDIRECT3DVERTEXBUFFER8	d3d_vbuff ;

		d3d_vbuff = *(LPDIRECT3DVERTEXBUFFER8 *)((DG_VERTEX_COMDL*)comdl->vbuff
												+ n_indices) ;
		DG_SetStaticVertexBuffer(0, d3d_vbuff, sizeof(DG_VERTEX_COMDL)) ;
	}
#else
	DG_SetDynamicVertexBuffer(comdl->vbuff, sizeof(DG_VERTEX_COMDL), n_indices, &sofs) ;
#endif

	pos    = comdl->pos ;
	n_objs = comdl->n_objs ;
	for ( i = 0 ; i < n_objs ; i++ ){
		if( pos->color.vw )			// α値チェック
		{
			DG_SetVertexShaderConstant(CV_WORLD0, &pos->world, 4);
			col.vx = (float)pos->color.vx ;
			col.vy = (float)pos->color.vy ;
			col.vz = (float)pos->color.vz ;
			col.vw = (float)pos->color.vw ;
			DG_SetVertexShaderConstant(CV_LIGHTCOL+3, &col, 1);	// "1"が正しい気がします

#if	__COMDL_USE_D3DVERTEXBUFFER__
			DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, n_indices-2) ;
#else
			DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, n_indices-2) ;
#endif
		}
		pos++ ;
	}
}

static void WriteObjPacksNVS( DG_COMDL *comdl )
{
	int		i ;
	DG_COMDL_POS	*pos ;
	DG_MDLPACK		*mdl_pack ;
#if	!__COMDL_USE_D3DVERTEXBUFFER__
	DWORD			sofs ;
#endif
	int				n_indices ;
	int				n_objs ;

	/*-- 描画する必要が有るか検査 --*/
#ifdef _WINDOWS
	pos = comdl->pos ;
	for(i=comdl->n_objs; i>0; i--, pos++)
	{
		if( pos->color.vw ){ break ; }	// α検査
	}
	if( !i ){ return ; }	// (α>0)の物が無いので、描画必要無し
#endif
	/*-- --*/

	DG_SetTextureNVS(0, &comdl->tex->tex_trans );

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

	mdl_pack  = comdl->mdl_pack ;
	n_indices = mdl_pack->n_indices ;

#if	__COMDL_USE_D3DVERTEXBUFFER__
	{
		LPDIRECT3DVERTEXBUFFER8	d3d_vbuff ;

		d3d_vbuff = *(LPDIRECT3DVERTEXBUFFER8 *)((DG_VERTEX_COMDL_FVF*)comdl->vbuff
												+ n_indices) ;
		DG_SetStaticVertexBuffer(0, d3d_vbuff, sizeof(DG_VERTEX_COMDL_FVF)) ;
	}
#else
	DG_SetDynamicVertexBuffer(comdl->vbuff, sizeof(DG_VERTEX_COMDL_FVF), n_indices, &sofs) ;
#endif

	pos    = comdl->pos ;
	n_objs = comdl->n_objs ;
	for ( i = 0 ; i < n_objs ; i++ ){
		if( pos->color.vw )			// α値チェック
		{
			DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&pos->world) ;
			DG_SetRenderState(D3DRS_AMBIENT,  ((DWORD)pos->color.vx << 16)
											| ((DWORD)pos->color.vy << 8)
											| ((DWORD)pos->color.vz << 0) )  ;
			DG_SetRenderState( D3DRS_TEXTUREFACTOR, (DWORD)pos->color.vw << 24 );

#if	__COMDL_USE_D3DVERTEXBUFFER__
			DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, n_indices-2) ;
#else
			DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, n_indices-2) ;
#endif
		}
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
	int		i, j, n, old_tex_code, invisible_flag ;
	//int		size ;
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
	void	(*write_obj_packs_func)(DG_COMDL *) ;
	BOOL	use_vshader ;

	MARK(__FILE__);
	scrpad = (ScrpadWork *)SCRPAD_ADDR ;
#ifdef LIBDG_PERFORMANCE
	scrpad->packet_info = DG_PerformanceData.comodel ;
#endif

	/* 関数選択 */
	write_obj_packs_func = _write_obj_packs_func ;

	/* Vertex Shader使用判定 */
	use_vshader = DG_CheckUseVertexShader() ;

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
		if ( !( comdl->group_id & DG_CurrentGroupID ) ) continue ;
		obj_list[max_objs].comdl = comdl ;
		obj_list[max_objs].use_tri = comdl->tri_id ;
		max_objs++ ;
	}
	if ( max_objs == 0 ) return ;


	/* スタティックプッシュバッファへの記録開始 */
	DG_OpenDmaTask();

	/* 各種設定 */
	if( use_vshader )
	{
		/* Vertex Shader 使用版 */
		DG_SetVertexShaderConstant(CV_ZERO, &VS_Const[0], 1);
		DG_SetVertexShaderConstant(CV_ONE, &VS_Const[1], 1);
		DG_SetVertexShaderConstant(CV_HALF, &VS_Const[2], 1);
		DG_SetVertexShaderConstant(CV_SCALE, &VS_Const[3], 1);
		DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);
		/* カメラ/透視変換マトリクスの設定 */
		//DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
		DG_SetVertexShaderConstant(CV_EYEPERS, &cp->eye_xpers, 4);

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
		/* Vertex Shader 非対応版 */

		/* マトリクス */
		DG_SetTransform(D3DTS_PROJECTION, &cp->xpers);
		DG_SetTransform(D3DTS_VIEW,       &cp->eye_inv);

		/* Fog */
		DG_SetRenderState(D3DRS_FOGSTART, *((DWORD *)&DG_FogStart)) ;
		DG_SetRenderState(D3DRS_FOGEND,   *((DWORD *)&DG_FogEnd)) ;

		/* その他設定 */
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE) ;
		DG_LightEnableAll(FALSE) ;	// 平行光源計算はしない
		DG_SetRenderState(D3DRS_LIGHTING , TRUE) ;
	}

	/* フォグ有効化 */
	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);
	/* アルファブレンディングOFF */
	DG_AlphaBlendDisable();
	/* VertexProcessing設定 */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckVertexBufferSoftProcessing()) ;
	/* 頂点シェーダー設定 */
	if( use_vshader )
	{
		/* Vertex Shader 使用版 */
		DG_SelectVertexShader( &DG_ComdlVertexShader[0] );
	}
	else
	{
		/* Vertex Shader 非対応版 */
		DG_SetVertexShader( D3DFVF_DG_VERTEX_COMDL_FLAG );
		DG_SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );
	}

	/* ピクセルシェーダー設定 */
	DG_SetPixelShader( NULL );

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
			mem_obj_list->comdl = obj_list[i].comdl ;
			mem_obj_list->use_tri = (int)tex_list ;	/* tri_codeの替わりにDG_TEXTUER_LISTへのポインタを入れる */
			mem_obj_list++ ;
			max_mem_objs++ ;
		}
	}

	/* 初期化パケットの設定 */

	mem_obj_list = (OBJ_LIST*)DG_LocalMemory ;
	//tex_list = DG_TextureList ;
	old_tex_code = -1 ;
	for ( i = max_mem_objs ; i > 0 ; i-- ){

#ifndef _WINDOWS	// Windowsでは必要無い？
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
#endif	/* !defined(_WINDOWS) */

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
		write_obj_packs_func( comdl );
		mem_obj_list++ ;

#if FALSE
		if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
#endif
	}

	/* 描画環境復帰処理 */
	if( !use_vshader )
	{
		/* Vertex Shader 非対応版 */
		DG_SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT) ;
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
#if __COMDL_USE_D3DVERTEXBUFFER__
	LPDIRECT3DVERTEXBUFFER8	*d3d_vbuff ;
	void					*dst_verts ;
#endif
	//short			*verts, *uvs ;

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
	comdl->tex = (DG_TEX *)mdl_pack->tex_id[0] ;

	/* 専用頂点バッファの作成 */
	if( DG_CheckUseVertexShader() )
	{
		DG_MDL			*mdl ;
		DG_VERTEX_KMSS	*verts ;
		unsigned short	*index ;
		DG_VERTEX_COMDL	*comdl_verts ;
		int				n_indices ;
		int				size ;

		n_indices = mdl_pack->n_indices ;
		size      = (sizeof(DG_VERTEX_COMDL) * mdl_pack->n_indices) ;

		mdl = (DG_MDL *)mdl_pack->rgbs ;		/* xloader.cにて細工をしているので注意！ */
#if __COMDL_USE_D3DVERTEXBUFFER__
		comdl->vbuff = DG_AllocLocalVideoMemory( size + sizeof(LPDIRECT3DVERTEXBUFFER8) ) ;

#else
		comdl->vbuff = DG_AllocLocalVideoMemory( size );
#endif
		comdl_verts = (DG_VERTEX_COMDL*)comdl->vbuff ;
		index = mdl_pack->index ;
#ifndef _WINDOWS
		for ( i = 0 ; i < mdl_pack->n_indices ; i++ ){
#else
		for ( i = mdl_pack->n_indices ; i > 0 ; i-- ){
#endif
			verts = (void*)( (int)mdl->vbuff + mdl->stride * (*index) ) ;
			comdl_verts->x = verts->vx ;
			comdl_verts->y = verts->vy ;
			comdl_verts->z = verts->vz ;
			comdl_verts->w = 1 ;
			comdl_verts->u = verts->u0 ;
			comdl_verts->v = verts->v0 ;
			comdl_verts++ ;
			index++ ;
		}

#if __COMDL_USE_D3DVERTEXBUFFER__
		d3d_vbuff = (LPDIRECT3DVERTEXBUFFER8 *)((BYTE *)comdl->vbuff + size) ;
		DG_CreateVertexBuffer(size,
							DG_WinApp.vbuff_usage | D3DUSAGE_WRITEONLY,
							0, D3DPOOL_MANAGED, d3d_vbuff) ;

		IDirect3DVertexBuffer8_Lock(*d3d_vbuff, 0, size, (BYTE **)&dst_verts, 0) ;
		memcpy(dst_verts, comdl->vbuff, size) ;
		IDirect3DVertexBuffer8_Unlock(*d3d_vbuff) ;
#endif	// __COMDL_USE_D3DVERTEXBUFFER__
	}
	else
	{
		DG_MDL			*mdl ;
		DG_VERTEX_KMSS	*verts ;
		unsigned short	*index ;
		DG_VERTEX_COMDL_FVF	*comdl_verts ;
		int				n_indices ;
		int				size ;

		/* VertexShader非対応版 */
		n_indices = mdl_pack->n_indices ;
		size      = (sizeof(DG_VERTEX_COMDL_FVF) * mdl_pack->n_indices) ;

		mdl = (DG_MDL *)mdl_pack->rgbs ;		/* xloader.cにて細工をしているので注意！ */
#if __COMDL_USE_D3DVERTEXBUFFER__
		comdl->vbuff = DG_AllocLocalVideoMemory( size + sizeof(LPDIRECT3DVERTEXBUFFER8) ) ;

#else
		comdl->vbuff = DG_AllocLocalVideoMemory( size );
#endif
		comdl_verts = (DG_VERTEX_COMDL_FVF*)comdl->vbuff ;
		index = mdl_pack->index ;
#ifndef _WINDOWS
		for ( i = 0 ; i < mdl_pack->n_indices ; i++ ){
#else
		for ( i = mdl_pack->n_indices ; i > 0 ; i-- ){
#endif
			verts = (void*)( (int)mdl->vbuff + mdl->stride * (*index) ) ;
			comdl_verts->x = (float)verts->vx ;
			comdl_verts->y = (float)verts->vy ;
			comdl_verts->z = (float)verts->vz ;
			comdl_verts->u = (float)verts->u0 ;
			comdl_verts->v = (float)verts->v0 ;
			comdl_verts++ ;
			index++ ;
		}

#if __COMDL_USE_D3DVERTEXBUFFER__
		d3d_vbuff = (LPDIRECT3DVERTEXBUFFER8 *)((BYTE *)comdl->vbuff + size) ;
		DG_CreateVertexBuffer(size,
							DG_WinApp.vbuff_usage | D3DUSAGE_WRITEONLY,
							0, D3DPOOL_MANAGED, d3d_vbuff) ;
		IDirect3DVertexBuffer8_Lock(*d3d_vbuff, 0, size, (BYTE **)&dst_verts, 0) ;
		memcpy(dst_verts, comdl->vbuff, size) ;
		IDirect3DVertexBuffer8_Unlock(*d3d_vbuff) ;
#endif	// __COMDL_USE_D3DVERTEXBUFFER__
	}

	return ( comdl ) ;
}

	/*
		メモリ開放
	*/
void	DG_FreeComdl( DG_COMDL *comdl )
{
#if __COMDL_USE_D3DVERTEXBUFFER__

	if( comdl->vbuff )
	{
		LPDIRECT3DVERTEXBUFFER8	d3d_vbuff ;
		DG_MDLPACK				*mdl_pack ;

		mdl_pack  = comdl->mdl_pack ;
		if( DG_CheckUseVertexShader() )
		{
			d3d_vbuff = *(LPDIRECT3DVERTEXBUFFER8 *)((DG_VERTEX_COMDL*)comdl->vbuff
													+ mdl_pack->n_indices) ;
		}
		else
		{
			d3d_vbuff = *(LPDIRECT3DVERTEXBUFFER8 *)((DG_VERTEX_COMDL_FVF*)comdl->vbuff
													+ mdl_pack->n_indices) ;
		}

		if( d3d_vbuff )
		{
			DG_ExeBufferSync(FALSE) ;		// 安全策
			DG_ReleaseD3DVertexBuffer(d3d_vbuff) ;
		}
	}
#endif

	DG_DelayedFreeLocalVideoMemory( comdl->vbuff );
	GV_DelayedFree( comdl );
}

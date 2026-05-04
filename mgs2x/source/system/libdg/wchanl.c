/*
	xchanl.c
	チャンネル管理ルーチン

	2002/02/08 K.Takabe
	$Id: wchanl.c,v 1.17 2002/12/23 10:42:10 takaki Exp $

*/
/*

	void		DG_InitChanlSystem( shift )
	int		shift ;		ダブルバッファシフト幅

		モジュール初期化

	void		DG_ResetChanlSystem( flag )
	int		flag ;		フラグ

		モジュール再初期化（ステージロード時など）

	void		DG_DrawChanlSystem( which )
	int		which ;		ダブルバッファ選択

		描画開始（ DrawOTag() ）

	void		DG_ClearChanlSystem( which )
	int		which ;		ダブルバッファ選択

		フレーム開始処理（ ClearOTagR() など）

	void		DG_SortChanlSystem( which )
	int		which ;		ダブルバッファ選択

		フレーム終了処理（次のフレームのパケットを生成）

	void		DG_SetDrawEnv( chanl, env )
	int		chanl ;		チャンネル番号
	DRAWENV		*env ;		描画環境

		指定チャンネルの描画環境を変更する
		変更が有効になるのは、次のフレームから

	------------------------------------------------

	int		DG_QueueObjs( objs )
	DG_OBJS		*objs ;		物体ハンドラ

		物体ハンドラをキューに登録

	void		DG_DequeueObjs( objs )
	DG_OBJS		*objs ;		物体ハンドラ

		物体ハンドラをキューから削除

	int		DG_QueuePrim( prim )
	DG_PRIM		*prim ;		プリミティブ

		プリミティブをキューに登録

	void		DG_DequeuePrim( prim )
	DG_PRIM		*prim ;		プリミティブ

		プリミティブをキューから削除
*/

#ifndef _XBOX
#include <windows.h>
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"
#include "private.h"

#include "def_dma.h"
#include "dmapack.h"

/*----------------------------------------------------------------*/
	/*
		特権モードフラグ
	*/
int	DG_PrivilegeMode = 0 ;

	/*
		チャンネル０、１の描画順入れ替えフラグ
	*/
int	DG_ReverseOrderDrawMode = 0 ;

	/*
		チャンネルリスト
	*/
DG_CHANL		DG_Chanls[ DG_MAX_CHANLS ] ;	/* チャンネル＝カメラとして見ていいかも */
DG_OBJ_QUEUE	DG_ObjQueue ;

DG_CHANL_EX		DG_ChanlsEx[ DG_MAX_CHANLS ] ;	/* 拡張パラメータ(暫定) */

	/*
		共通オブジェクトキューバッファ
		（各オブジェクトキューに割り振られる。最終的には動的に割り振りを変更できるようにしたい）
	*/
#define MAX_QUEUE_BUFFER	(16*1024)
int		DG_UseQueueBuffer = 0 ;				/* 共通キューバッファ使用量 */
void	*DG_QueueBuffer[ MAX_QUEUE_BUFFER ];/* 共通キューバッファ */

	/*
		DG_BackBuffer2Frame抑制フラグ
	*/
extern DWORD	DG_SkipRendBackBuffer2Frame ;
extern DWORD	DG_SkipRendBackBuffer2FrameCntr ;

	/*
		画面消去パケット
	*/
static DG_DRAWENV		DG_DefaultDrawEnv[2] ;
static DG_DRAWOFFSET	DG_DefaultDrawOffset[2] ;
static struct _dg_clear_packet{
	DG_GIFTAG		giftag ;
	sceGsClear		clear ;
} DG_ClearPacket ALIGN16;


/*----------------------------------------------------------------*/
#undef DEBUG
	/*
		デバッグ表示用
	*/
#if 0
#ifdef	DEBUG
STATIC	short	N_ChanlPerfMarks ;
STATIC	u_short	ChanlDrawMark ;
STATIC	u_short	ChanlPerfMarks_Next ;
STATIC	u_short	ChanlPerfMarks[ 32 ] ;
#endif
#endif

#define DG_XBOXDEBUG_SAFETYAREA			(0x0001)
int		DG_XBoxDebugFlag = 0 ;

/*----------------------------------------------------------------*/

	/*
		チャンネル処理ユニット
	*/
void DG_DummyChanl( DG_CHANL *chanl, int which ){}	/* ダミー処理ユニット */
static void		DG_BackBuffer2Frame( DG_CHANL *cp, int which );

static	QueFunc	NormalStageList[] = {/* 標準処理ステージ */
	DG_PluginStartChanl,	/* ●プラグイン初期化ステージ */
	DG_FrameChanl,			/* 描画環境初期化＆設定ステージ */
	DG_PluginChanl,			/* ●プラグイン処理ステージ（０） */
	DG_ScreenChanl,			/* マトリクス計算＆バウンディングチェック */
	DG_Chain2Chanl,			/* マルチテクスチャオブジェクトのＤＭＡ接続ステージ */
	DG_ChainEvmChanl,		/* マルチウェイトエンベロープオブジェクト処理ステージ */
	DG_ChainChanl,			/* 不透明オブジェクトのＤＭＡ接続ステージ */
	DG_Chain2ChanlLatter,	/* マルチテクスチャオブジェクトのＤＭＡ接続ステージ */
	DG_ChainEvmChanlLatter,	/* マルチウェイトエンベロープオブジェクト処理ステージ */
	DG_ShadowChainChanl,	/* （特殊）影投影処理ステージ */
	DG_SpotChainChanl,		/* （特殊）スポットライト投影ステージ */
	//DG_ComdlChanl,		/* （特殊）共有モデルオブジェクト処理ステージ */
	DG_PluginChanl,			/* ●プラグイン処理ステージ（１） */
	DG_Prim2Chanl,			/* プリミティブソート処理ステージ */
	DG_SortChainChanl,		/* 半透明オブジェクト＆プリミティブのＤＭＡ接続ステージ */
	DG_PluginChanl,			/* ●プラグイン処理ステージ（２） */
	DG_PluginChanl,			/* ●プラグイン処理ステージ（３） */
	DG_PluginEndChanl,		/* ●プラグイン終了ステージ */
	DG_DummyChanl
} ;

static QueFunc MenuStageList[] = {
	DG_PluginStartChanl,	/* ●プラグイン初期化ステージ */
	DG_FrameChanl,			/* 描画環境初期化＆設定ステージ */
	DG_PluginChanl,			/* ●プラグイン処理ステージ（０） */
	DG_PluginChanl,			/* ●プラグイン処理ステージ（１） */
#ifdef _WINDOWS
	DG_BackBuffer2Frame,	/* BackBuffer-->FrameBuffer */
#endif
	DG_PluginChanl,			/* ●プラグイン処理ステージ（２） */
	DG_PluginChanl,			/* ●プラグイン処理ステージ（３） */
	DG_PluginEndChanl,		/* ●プラグイン終了ステージ */
	DG_DummyChanl
} ;

#ifndef	NO_PROTOTYPE
static	void	SetParam( DG_CHANL *, int ) ;
#endif

/*----------------------------------------------------------------*/

	/*
		描画環境を設定する
	*/
static void SetDefDrawEnv( DG_DRAWENV *env, DG_DRAWOFFSET *offset, int page, int x, int y, int w, int h )
{
	int		left, top, right, bottom ;

	/* パラメータの計算 */
	left = DRAW_WIDTH / 2 - w / 2 + x ;
	top = DRAW_HEIGHT / 2 - h / 2 + y ;
	right = left + w - 1 ;
	bottom = top + h - 1 ;
	if ( left < 0 ) left = 0 ;
	if ( top < 0 ) top = 0 ;
	if ( right > ( DRAW_WIDTH - 1 ) ) right = DRAW_WIDTH - 1 ;
	if ( bottom > ( DRAW_HEIGHT - 1 ) ) bottom = DRAW_HEIGHT - 1 ;

#if 0
	/* GIFTAGの設定 */
	env->gif_tag = SCE_GIF_SET_TAG(1, 1, 0, 0, 0, sizeof(env->datas)/16);
	DG_ENABLE_DRAWENV( env,
					  DG_DRAWENV_FRAME|DG_DRAWENV_ZBUF|DG_DRAWENV_XYOFFSET|DG_DRAWENV_SCISSOR
					  |DG_DRAWENV_PRMODECONT|DG_DRAWENV_COLCLAMP|DG_DRAWENV_TEST
					  |DG_DRAWENV_PABE|DG_DRAWENV_TEXA|DG_DRAWENV_DTHE|DG_DRAWENV_DIMX);

	/* 各レジスタの設定 */
	env->datas.frame1_addr = SCE_GS_FRAME_1 ;
	*(u_long*)&env->datas.frame1 = 
	  SCE_GS_SET_FRAME( BUFFER_PAGE(page)/2048, BUFFER_WIDTH/64, FRAME_BUFFER_COLOR_MODE(), 0x00000000 ) ;
	env->datas.zbuf1_addr = SCE_GS_ZBUF_1 ;
	*(u_long*)&env->datas.zbuf1 = 
	  SCE_GS_SET_ZBUF( ZBUFFER_PAGE()/2048, Z_BUFFER_COLOR_MODE(), 0 ) ;

	env->datas.xyoffset1_addr = SCE_GS_XYOFFSET_1 ;
	*(u_long*)&env->datas.xyoffset1 = 
	  SCE_GS_SET_XYOFFSET( ( ( 2048 - DRAW_WIDTH / 2 - x ) << 4 ), ( ( 2048 - DRAW_HEIGHT / 2 - y ) << 4 ) ) ;
	env->datas.scissor1_addr = SCE_GS_SCISSOR_1 ;
	*(u_long*)&env->datas.scissor1 = 
	  SCE_GS_SET_SCISSOR( left, right, top, bottom ) ;
	env->datas.prmodecont_addr = SCE_GS_PRMODECONT ;
	*(u_long*)&env->datas.prmodecont = SCE_GS_SET_PRMODECONT( 1 ) ;
	env->datas.colclamp_addr = SCE_GS_COLCLAMP ;
	*(u_long*)&env->datas.colclamp = SCE_GS_SET_COLCLAMP( 1 ) ;
	env->datas.test1_addr = SCE_GS_TEST_1 ;
	*(u_long*)&env->datas.test1 = SCE_GS_SET_TEST( 1, 5, 64, 1, 0, 0, 1, 2 ) ;
	env->datas.pabe_addr = SCE_GS_PABE ;
	*(u_long*)&env->datas.pabe = SCE_GS_SET_PABE( 0 ) ;
	env->datas.texa_addr = SCE_GS_TEXA ;
	*(u_long*)&env->datas.texa = SCE_GS_SET_TEXA( 128, 1, 128 ) ;
	env->datas.dthe_addr = SCE_GS_DTHE ;
	*(u_long*)&env->datas.dthe = 0 ;
	env->datas.dimx_addr = SCE_GS_DIMX ;
	*(u_long*)&env->datas.dimx = 0x4253061753421706 ;
	env->datas.clamp_addr = SCE_GS_CLAMP_1 ;
	*(u_long*)&env->datas.clamp = SCE_GS_SET_CLAMP(1,1,0,0,0,0) ;


	/* GIFTAGの設定 */
	offset->gif_tag.tag = SCE_GIF_SET_TAG( sizeof(offset->datas)/16, 1, 0, 0, 0, 1 );
	offset->gif_tag.regs = 0x0e ;
	/* 各レジスタの設定 */
	offset->datas.xyoffset1_addr = SCE_GS_XYOFFSET_1 ;
	*(u_long*)&offset->datas.xyoffset1 = 
	  SCE_GS_SET_XYOFFSET( ( ( 2048 - DRAW_WIDTH / 2 - x ) << 4 ), ( ( 2048 - DRAW_HEIGHT / 2 - y ) << 4 ) ) ;
#endif
}

	/*
		デフォルトの描画環境を設定
	*/
void DG_SetDrawEnv( DG_CHANL *cp, int x, int y, int w, int h )
{
	cp->width = w ;
	cp->height = h ;
	cp->offset_x = x ;
	cp->offset_y = y ;
	SetDefDrawEnv( &cp->draw_env[0], &cp->draw_offset[0], 0, x, y, w, h );
	SetDefDrawEnv( &cp->draw_env[1], &cp->draw_offset[1], 1, x, y, w, h );

}

	/*
		描画領域の制限を行う（シネマカット時の負荷軽減用なので使用には注意！）
	*/
void DG_ChangeDrawLimit( DG_CHANL *cp, int x1, int y1, int x2, int y2 )
{
	*(u_long*)&cp->draw_env[0].datas.scissor1 =  SCE_GS_SET_SCISSOR( x1, x2, y1, y2 ) ;
	*(u_long*)&cp->draw_env[1].datas.scissor1 =  SCE_GS_SET_SCISSOR( x1, x2, y1, y2 ) ;
}

	/*
		指定したチャンネルの描画環境復元ＤＭＡパケットを指定アドレスに書き込む
	*/
void *DG_PopDefaultDrawEnv( DG_CHANL *cp, void *addr )
{
	DG_DMATAG	*dmatag = addr ;
	dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, sizeof( DG_DRAWENV )/sizeof(u_long128) );
	dmatag->addr = &cp->draw_env[DG_Clock] ;
	//dmatag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0);
	dmatag->vifcode[0] = SCE_VIF1_SET_NOP(0);
	dmatag->vifcode[1] = SCE_VIF1_SET_DIRECT( sizeof( DG_DRAWENV )/sizeof(u_long128), 0 );
	dmatag++ ;
	dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, sizeof( DG_DRAWOFFSET )/sizeof(u_long128) );
	dmatag->addr = &cp->draw_offset[DG_Clock] ;
	dmatag->vifcode[0] = SCE_VIF1_SET_NOP(0);
	dmatag->vifcode[1] = SCE_VIF1_SET_DIRECT( sizeof( DG_DRAWOFFSET )/sizeof(u_long128), 0 );
	dmatag++ ;

	return ( dmatag );
}

/*----------------------------------------------------------------*/

	/*
		チャンネルにパラメータを設定
	*/
static	void	SetParam( DG_CHANL *cp, int obj_num )
{
	cp->flag = 0 ;
	cp->group_id = 1 ;
	if ( obj_num < 0 ){
		cp->obj_queue = NULL ;
	} else {
		/* オブジェクトキューの設定 */
		cp->obj_queue = &DG_ObjQueue ;
	}
}

	/*
		オブジェクトキューバッファを確保する
	*/
static void DG_AllocObjectBuffer( DG_OBJ_BUFFER *obj_buff, int max, int id )
{
	obj_buff->max_queue = max ;
	obj_buff->n_queue = 0 ;
	obj_buff->queue = &DG_QueueBuffer[ DG_UseQueueBuffer ] ;
	obj_buff->id = id ;
	DG_UseQueueBuffer += max ;
	//printf("%d %d\n", DG_UseQueueBuffer, MAX_QUEUE_BUFFER );
	ASSERT( DG_UseQueueBuffer <= MAX_QUEUE_BUFFER );
}

	/*
		プラグイン用オブジェクトキューバッファを確保する
	*/
DG_OBJ_BUFFER *DG_MakeUserObjectBuffer( int max, int id )
{
	int		i ;
	DG_OBJ_BUFFER	*obj_buff ;

	for ( i = 0 ; i < DG_MAX_USR_OBJQUEUE ; i++ ){
		obj_buff = &DG_ObjQueue.user_buffer[i] ;
		if ( obj_buff->id == 0 ){
			obj_buff->max_queue = max ;
			obj_buff->n_queue = 0 ;
			obj_buff->queue = &DG_QueueBuffer[ DG_UseQueueBuffer ] ;
			obj_buff->id = id ;
			DG_UseQueueBuffer += max ;
			ASSERT( DG_UseQueueBuffer <= MAX_QUEUE_BUFFER );
			return ( obj_buff ) ;
		}
	}
	/* 空きオブジェクトキューバッファがない場合 */
	ASSERT( 0 );
	return ( NULL );
}

	/*
		プラグイン用オブジェクトキューバッファを開放する
	*/
void DG_FreeUserObjectBuffer( DG_OBJ_BUFFER *obj_buff )
{
	obj_buff->id = 0 ;
}


/*----------------------------------------------------------------*/

void		DG_InitChanlSystem( int flag )
{
	DG_CHANL	*cp ;
	FVECTOR		from = {0,0,1,0};	/* XBOXで追加 */
	FVECTOR		to = {0,0,0,0};		/* XBOXで追加 */

	/*
		各オブジェクトキューの初期化
	*/
	/* 新初期化ルーチン（各オブジェクトキューにバッファを割り当てる） */
	DG_UseQueueBuffer = 0 ;
	GV_ZeroMemory( &DG_ObjQueue, sizeof( DG_OBJ_QUEUE ) );
	DG_AllocObjectBuffer( &DG_ObjQueue.objs_buffer, 512+256, 0 );
	DG_AllocObjectBuffer( &DG_ObjQueue.shdwwrite_buffer, 512, 0 );
	DG_AllocObjectBuffer( &DG_ObjQueue.spot_buffer, 512, 0 );
	DG_AllocObjectBuffer( &DG_ObjQueue.evmobj_buffer, 512, 0 );
	DG_AllocObjectBuffer( &DG_ObjQueue.prim2_buffer, 512, 0 );
	DG_AllocObjectBuffer( &DG_ObjQueue.particle_buffer, 512, 0 );

	/*
		画面消去用デフォルト描画環境作成
	*/
	SetDefDrawEnv( &DG_DefaultDrawEnv[0], &DG_DefaultDrawOffset[0], 0, 0, 0, DRAW_WIDTH, DRAW_HEIGHT );
	SetDefDrawEnv( &DG_DefaultDrawEnv[1], &DG_DefaultDrawOffset[1], 1, 0, 0, DRAW_WIDTH, DRAW_HEIGHT );
#if 0
	DG_ClearPacket.giftag.tag = SCE_GIF_SET_TAG( sizeof(sceGsClear) / 16, 1, 0, 0, 0, 1) ;
	DG_ClearPacket.giftag.regs = GS_REGS_AD ;
	sceGsSetDefClear( &DG_ClearPacket.clear, SCE_GS_ZGEQUAL,
					 2048-DRAW_WIDTH/2, 2048-DRAW_HEIGHT/2, DRAW_WIDTH, DRAW_HEIGHT, 0, 0, 0, 0, 0 );
#endif

   BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system

	/*
		メインチャンネル（チャンネル０）
	*/
	cp = DG_Chanls ;
	SetParam( cp, 0 );
	DG_SetDrawEnv( cp, 0, 0, DRAW_WIDTH, DRAW_HEIGHT );
	cp->bg_clear_flag = 1 ;
	cp->chanl_num = 0 ;
	cp->high_reso = 1 ;
	cp->flag = 1 ;
	cp->n_stage = sizeof( NormalStageList ) / sizeof( QueFunc ) ;
	cp->stage_list = NormalStageList ;
	DG_SetCamera2( cp, &from, &to, 2.0f );	/* XBOXで追加 */
	cp ++ ;
	/*
		チャンネル１
	*/
	SetParam( cp, 0 );
	DG_SetDrawEnv( cp, DRAW_WIDTH/4, DRAW_HEIGHT/3, DRAW_WIDTH/3, DRAW_HEIGHT/4 );
	cp->bg_clear_flag = 1 ;
	cp->chanl_num = 1 ;
	cp->high_reso = 1 ;
	cp->flag = 0 ;
	cp->n_stage = sizeof( NormalStageList ) / sizeof( QueFunc ) ;
	cp->stage_list = NormalStageList ;
	DG_SetCamera2( cp, &from, &to, 2.0f );	/* XBOXで追加 */
	cp ++ ;
	/*
		チャンネル２
	*/
	SetParam( cp, 0 ) ;
	DG_SetDrawEnv( cp, 0, 0, DRAW_WIDTH, DRAW_HEIGHT );
	cp->bg_clear_flag = 1 ;
	cp->chanl_num = 2 ;
	cp->high_reso = 1 ;
	cp->flag = 0 ;
	cp->n_stage = sizeof( NormalStageList ) / sizeof( QueFunc ) ;
	cp->stage_list = NormalStageList ;
	DG_SetCamera2( cp, &from, &to, 2.0f );	/* XBOXで追加 */
	cp ++ ;
	/*
		チャンネル３
	*/
	SetParam( cp, 0 );
	DG_SetDrawEnv( cp, 0, 0, DRAW_WIDTH, DRAW_HEIGHT );
	cp->bg_clear_flag = 1 ;
	cp->chanl_num = 3 ;
	cp->high_reso = 1 ;
	cp->flag = 0 ;
	cp->n_stage = sizeof( NormalStageList ) / sizeof( QueFunc ) ;
	cp->stage_list = NormalStageList ;
	DG_SetCamera2( cp, &from, &to, 2.0f );	/* XBOXで追加 */
	cp ++ ;
	/*
		メニュー用チャンネル（チャンネル４）
	*/
	SetParam( cp, 4 ) ;
	DG_SetDrawEnv( cp, 0, 0, DRAW_WIDTH, DRAW_HEIGHT );
	cp->draw_env[0].datas.test1.ZTE = 1 ;
	cp->draw_env[0].datas.test1.ZTST = 1 ;
	cp->draw_env[1].datas.test1.ZTE = 1 ;
	cp->draw_env[1].datas.test1.ZTST = 1 ;
	cp->bg_clear_flag = 0 ;
	cp->chanl_num = 4 ;
	cp->high_reso = 0 ;		/* メニューはデフォルトではローレゾに */
	cp->flag = 1 ;
	cp->n_stage = sizeof( MenuStageList ) / sizeof( QueFunc ) ;
	cp->stage_list = MenuStageList ;
	DG_SetCamera2( cp, &from, &to, 2.0f );	/* XBOXで追加 */
	cp ++ ;

	/*
		プラグインシステム初期化
	*/
	DG_InitPluginSystem();

	/*
		システム使用パケットの初期化
	*/
	//DG_InitSystemPacket();	/* PS2固有 */

	/*
		ＭＧＳ２デフォルト起動プラグイン組み込み
	*/
	//DG_AddPluginComdl();
	DG_AddPluginDmapack();
	DG_AddPluginOptcmf();
	/* 実験組み込み */
	//DG_AddPluginPatch();

	/* コールバック専用プラグイン(Xbox新規) */
	//DG_AddPluginCallbackObj();

}

void DG_ResetChanlSystem( int flag )
{
	DG_Chanls[0].flag = 1 ;
	DG_Chanls[1].flag = 0 ;
	DG_Chanls[2].flag = 0 ;
	DG_Chanls[3].flag = 0 ;
	DG_Chanls[4].flag = 1 ;
	DG_Chanls[0].bg_clear_flag = 1 ;
	DG_Chanls[1].bg_clear_flag = 1 ;
	DG_Chanls[2].bg_clear_flag = 1 ;
	DG_Chanls[3].bg_clear_flag = 1 ;
	DG_Chanls[4].bg_clear_flag = 0 ;
	DG_PrivilegeMode = 0 ;
	DG_ReverseOrderDrawMode = 0 ;
}

void DG_DrawChanlSystem( int which )
{
	/* 描画ＤＭＡ動作開始 */
	DG_DmaStart( which );
}

void DG_ClearChanlSystem( int which )
{
	/* 描画ＤＭＡバッファ初期化 */
	DG_DmaClear( which );
}

void DG_UnDrawChanlSystem( int which )
{
	//DG_RetryStoreChanl();
	/* 描画ＤＭＡ動作のスキップ */
	DG_DmaSkip( which );
}

static long stop_chanl_system_flag = 0;

void		DG_SortChanlSystem( int which )
{
	DG_CHANL	*cp ;
	int		i, j ;
	extern int			DG_PacketOverFlag ;				/* そのフレーム内でのバッファーオーバー通知用 */
	if( stop_chanl_system_flag ){
		return;
	}

	DG_OpenDmaTask();
	/*
		画面を消す前にイメージストアパケットを接続する
	*/
	DG_StoreChanl( NULL, 1 - which );		/* イメージストア処理 */

	/*
		とりあえず画面を消去する
	*/
	DG_Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 0.0f, 0);

	/* 描画環境の初期化 */
	DG_InitRenderState();
	DG_CloseDmaTask();

	/*
		チャンネル処理ユニットを、各チャンネルに実行する
	*/
	cp = DG_Chanls ;

	GV_PROFILE_CHANL_START();

#if 0
	for ( j = DG_MAX_CHANLS ; j > 0 ; j--, cp++ ){
		if ( cp->flag == 0 ) continue ;
		DG_CurrentGroupID = cp->group_id ;

		/* テクスチャの多重ロードチェック用初期化 */
		for ( i = 0 ; i < DG_MaxTextures ; i++ ){
			DG_TextureList[ i ]->flag = 0 ;
		}
		/* チャンネルの各描画フェースを処理 */
		for ( i = 0 ; i < cp->n_stage ; i++ ) {
			( *cp->stage_list[i] )( cp, which );
			GV_PROFILE_CHANL();
		}

	}
#else
	for ( j = 0 ; j < DG_MAX_CHANLS ; j++ ){
		cp = DG_Chanls + j ;

		/* 描画順反転モードのチェック */
		if ( DG_ReverseOrderDrawMode != 0 ){
			if ( j < 2 ) cp = DG_Chanls + ( j ^ 1 ) ;
		}

		if ( cp->flag == 0 ) continue ;

		if ( ( cp->width < 1 ) || ( cp->height < 1 ) ) continue ;	/* ＸＢＯＸで追加 */
		if ( ( j != ( DG_MAX_CHANLS - 1 ) ) && ( cp->screen == 0.0f ) ) continue ;
		if ( DG_PacketOverFlag ) continue ;

		DG_CurrentGroupID = cp->group_id ;

		/* テクスチャの多重ロードチェック用初期化 */
		for ( i = 0 ; i < DG_MaxTextures ; i++ ){
			//DG_TextureList[ i ].flag = 0 ;
			DG_TextureList[ i ]->flag = 0 ;
		}
		/* チャンネルの各描画フェースを処理 */
#ifndef DEBUG_MODE
		for ( i = 0 ; i < cp->n_stage ; i++ ) {
			if ( DG_PacketOverFlag ) break ;
			( *cp->stage_list[i] )( cp, which );
			GV_PROFILE_CHANL();
		}
#else
		{
			u_int		old_time, time, t ;
			old_time = GV_GET_PRFC_CLOCK();
			for ( i = 0 ; i < cp->n_stage ; i++ ) {
				if ( DG_PacketOverFlag ) break ;
				( *cp->stage_list[i] )( cp, which );
				time = GV_GET_PRFC_CLOCK();
				GV_PROFILE_CHANL();
				t = time - old_time ;
				if ( GV_PadData[1].press & PAD_AL ){
					printf("%-16s:%6.2f (%d)\n", GV_DebugMes, t/23270.0f, t );
				}
				old_time = time ;
			}
		}
#endif

	}
#endif

#ifdef DEBUG_MODE
	/* ＸＢＯＸ用セーフティーエリアチェック用ボックスの描画 */
	if ( DG_XBoxDebugFlag & DG_XBOXDEBUG_SAFETYAREA ){
		struct {
			DG_DMAPACK_LINE		line[4] ;
			DG_DMAPACK_PARAM		end ;
		} packet ;
		int			x0, y0, x1, y1 ;
		x0 = (640-595)/2 ;
		x1 = 640-x0 ;
		y0 = (448-408)/2 ;
		y1 = 448-y0 ;
		DG_OpenDmaTask();
		//DG_Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 0.0f, 0);
		DG_SetDmapackLine( &packet.line[0], (float)x0, (float)y0, 0x80ffffff, (float)x1, (float)y0, 0x80ffffff );
		DG_SetDmapackLine( &packet.line[1], (float)x0, (float)y1, 0x80ffffff, (float)x1, (float)y1, 0x80ffffff );
		DG_SetDmapackLine( &packet.line[2], (float)x0, (float)y0, 0x80ffffff, (float)x0, (float)y1, 0x80ffffff );
		DG_SetDmapackLine( &packet.line[3], (float)x1, (float)y0, 0x80ffffff, (float)x1, (float)y1, 0x80ffffff );
		DG_SetDmapackEnd( &packet.end );
		DG_ExecAuto2DPrim( &packet );
		DG_CloseDmaTask();
	}
#endif


}

/*----------------------------------------------------------------*/
	/*
		オブジェクト登録
	*/
static int		QueueObjs( DG_OBJ_BUFFER *obj_buff, void *objs )
{
	int		n ;

	if ( objs == NULL ) return -1 ;
	//printf("%08x,%08x %d %d %08x\n", obj_buff, obj_buff->queue, obj_buff->n_queue, obj_buff->max_queue, objs );

	/*
		登録順番をくずさないようにキューに追加する
	*/
	n = obj_buff->n_queue ;
	if ( n >= obj_buff->max_queue ){
		printf("que buffer over!!\n");
		return -1 ;
	}
	obj_buff->queue[ n ++ ] = objs ;
	obj_buff->n_queue = n ;
	return 0 ;
}

static void		DequeueObjs( DG_OBJ_BUFFER *obj_buff, void *objs )
{
	void		**oque ;
	int			n, i ;
	/*
		キュー登録位置を検索
	*/
	if ( objs == NULL ) return ;
	oque = obj_buff->queue ;
	n = obj_buff->n_queue ;
	for ( i = n ; i > 0 ; -- i ) {
		if ( *oque == objs ) goto found ;
		oque ++ ;
	}
	WARNING( "dequeued invalid objs %x\n", objs ) ;
	return ;
found :
	/*
		登録順番をくずさないようにキューから削除する
	*/
	for ( -- i ; i > 0 ; -- i ) {
		oque[ 0 ] = oque[ 1 ] ;
		oque ++ ;
	}
	obj_buff->n_queue = -- n ;
}

static void*	SearchQueueObjs( DG_OBJ_BUFFER *obj_buff, void *objs )
{
	void		**oque ;
	int			n, i ;
	/*
		キュー登録位置を検索
	*/
	if ( objs == NULL ) return ( NULL );
	oque = obj_buff->queue ;
	n = obj_buff->n_queue ;
	for ( i = n ; i > 0 ; -- i ) {
		if ( *oque == objs ) break ;
		oque ++ ;
	}
	if ( i == 0 ) return ( NULL );
	return ( oque );
}

	/*
		通常オブジェクト
	*/
int DG_QueueObjs( DG_OBJS *objs )
{
	int		ret ;
	//printf("objs:");
	//ret = QueueObjs( &DG_Chanls[ objs->chanl ].obj_queue->objs_buffer, objs );
	//if ( DG_Chanls[ objs->chanl ].obj_queue->objs_buffer.n_queue > 512 ) GV_ERROR( GV_ERROR_MAX_OBJ_QUEUE );
	ret = QueueObjs( &DG_Chanls[ 0 ].obj_queue->objs_buffer, objs );
	if ( DG_Chanls[ 0 ].obj_queue->objs_buffer.n_queue > 512 ) GV_ERROR( GV_ERROR_MAX_OBJ_QUEUE );
	if ( ret != 0 ) GV_ERROR( GV_ERROR_MAX_OBJ_QUEUE );
#if FALSE
	if( !(objs->flag & DG_FLAG_PAINT) ){ DG_NormalizeLightMatrix(objs->light) ; }	// 安全策
#endif
	return ( ret ) ;
}

void DG_DequeueObjs( DG_OBJS *objs )
{
	//return DequeueObjs( &DG_Chanls[ objs->chanl ].obj_queue->objs_buffer, objs );
	/*return */DequeueObjs( &DG_Chanls[ 0 ].obj_queue->objs_buffer, objs );
}

DG_OBJS* DG_SearchQueueObjs( DG_OBJS *objs )
{
	return SearchQueueObjs( &DG_Chanls[ 0 ].obj_queue->objs_buffer, objs );
}


	/*
		影投影オブジェクト
	*/
int DG_QueueShdwwriteObjs( DG_OBJS *objs )
{
	int		ret ;
	//printf("objs2:");
	//ret = QueueObjs( &DG_Chanls[ objs->chanl ].obj_queue->shdwwrite_buffer, objs );
	ret = QueueObjs( &DG_Chanls[ 0 ].obj_queue->shdwwrite_buffer, objs );
	if ( ret != 0 ) GV_ERROR( GV_ERROR_MAX_MISC_QUEUE );
	return ( ret ) ;
}

void DG_DequeueShdwwriteObjs( DG_OBJS *objs )
{
	//return DequeueObjs( &DG_Chanls[ objs->chanl ].obj_queue->shdwwrite_buffer, objs );
	/*return */DequeueObjs( &DG_Chanls[ 0 ].obj_queue->shdwwrite_buffer, objs );
}

	/*
		スポットライトオブジェクト
	*/
int DG_QueueSpotObjs( DG_SPOT *objs )
{
	int		ret ;
	//printf("spot:");
	/* 将来的に修正する */
	ret = QueueObjs( &DG_Chanls[ 0 ].obj_queue->spot_buffer, objs );
	if ( ret != 0 ) GV_ERROR( GV_ERROR_MAX_MISC_QUEUE );
	return ( ret );
}

void DG_DequeueSpotObjs( DG_SPOT *objs )
{
	/* 将来的に修正する */
	/*return */DequeueObjs( &DG_Chanls[ 0 ].obj_queue->spot_buffer, objs );
}


	/*
		マルチウェイトエンベロープモデルオブジェクト
	*/
int DG_QueueEvmObj( DG_EVMOBJ *evmobj )
{
	int		ret ;
	//printf("evmobj:");
	//ret = QueueObjs( &DG_Chanls[ evmobj->chanl ].obj_queue->evmobj_buffer, evmobj );
	ret = QueueObjs( &DG_Chanls[ 0 ].obj_queue->evmobj_buffer, evmobj );
	if ( ret != 0 ) GV_ERROR( GV_ERROR_MAX_MISC_QUEUE );
	return ( ret );
}

void DG_DequeueEvmObj( DG_EVMOBJ *evmobj )
{
	//return DequeueObjs( &DG_Chanls[ evmobj->chanl ].obj_queue->evmobj_buffer, evmobj );
	/*return */DequeueObjs( &DG_Chanls[ 0 ].obj_queue->evmobj_buffer, evmobj );
}

int DG_QueuePrim2( DG_PRIM2 *objs )
{
	int		ret ;
	//printf("prim2:");
	//ret = QueueObjs( &DG_Chanls[ objs->chanl ].obj_queue->prim2_buffer, objs );
	ret = QueueObjs( &DG_Chanls[ 0 ].obj_queue->prim2_buffer, objs );
	if ( ret != 0 ) GV_ERROR( GV_ERROR_MAX_PRIM_QUEUE );
	return ( ret );
}

void DG_DequeuePrim2( DG_PRIM2 *objs )
{
	//return DequeueObjs( &DG_Chanls[ objs->chanl ].obj_queue->prim2_buffer, objs );
	/*return*/ DequeueObjs( &DG_Chanls[ 0 ].obj_queue->prim2_buffer, objs );
}

int DG_QueueParticle(DG_PARTICLE *objs)
{
    int        ret;
    ret = QueueObjs(&DG_Chanls[objs->chanl].obj_queue->particle_buffer, objs);
    if (ret != 0) GV_ERROR(GV_ERROR_MAX_PRIM_QUEUE);
    return (ret);
}

void DG_DequeueParticle(DG_PARTICLE *objs)
{
    DequeueObjs(&DG_Chanls[objs->chanl].obj_queue->particle_buffer, objs);
}

/* ------------------------------------------------------ */
	/*
		プラグイン用オブジェクト
	*/
int DG_QueueUserObject( DG_OBJ_BUFFER *obj_buff, void *objs )
{
	//printf("user object:");
	return QueueObjs( obj_buff, objs );
}

void DG_DequeueUserObject( DG_OBJ_BUFFER *obj_buff, void *objs )
{
	/*return*/ DequeueObjs( obj_buff, objs );
}


/* ------------------------------------------------------ */
/* 特権オブジェクトモードの変更 */
int DG_SetPrivilegeMode( int mode )
{
	int	ret ;
	ret = DG_PrivilegeMode ;
	DG_PrivilegeMode = mode ;
	return ( ret );	/* とりあえず変更前の値を返しておく */
}

/* 描画順反転モードの変更 */
int DG_SetReverseDrawOrderMode( int mode )
{
	int	ret ;
	ret = DG_ReverseOrderDrawMode ;
	DG_ReverseOrderDrawMode = mode ;
	return ( ret );	/* とりあえず変更前の値を返しておく */
}




/* ------------------------------------------------------ */

void DG_StopMainChanlSystem( void )
{
	/*
	   メイン画面用のチャンネル処理を停止する。
	   各オブジェクト用のパケットをすべて解放。パケットエリアを使用可能にする。
	   描画が完全に終了したことを確認してからパケットエリアを使用すること。
	*/

	stop_chanl_system_flag = 1;

}

void DG_RestartMainChanlSystem( void )
{
	/*
	   メイン画面用のチャンネル処理を再開する。
	*/
	extern long DG_UnDrawFrameCount;

	stop_chanl_system_flag = 0;
	DG_UnDrawFrameCount = 1;
}

/* ------------------------------------------------------ */

/* ------------------------------------------------------ */
static void		DG_BackBuffer2Frame( DG_CHANL *cp, int which )
{
	/*
		バックバッファに描画されている物をFrameBuffer(Window内部バッファ)
		に複写します。
		これ以降の描画はFrameBuffer側にされます。
		多重呼び出しは無視されます。
	*/
	if( !DG_SkipRendBackBuffer2FrameCntr ){ DG_RendBackBuffer2Frame() ; }
}
/* ------------------------------------------------------ */

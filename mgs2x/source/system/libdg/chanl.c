//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	chanl.c
	チャンネル管理ルーチン

	1999/07/07 K.Takabe
	$Id: chanl.c,v 1.1.1.3 2002/11/19 11:42:03 Yoshizawa1 Exp $

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

#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"

#include "BP_Renderer.h"
#include "BP_RenderBuffer.h"
#include "BP_RenderBufferTypes.h"
#include "BP_Debug.h"

/*----------------------------------------------------------------*/
	/*
		特権モードフラグ
	*/
//int	DG_PrivilegeMode = 0 ;

	/*
		チャンネル０、１の描画順入れ替えフラグ
	*/
int	DG_ReverseOrderDrawMode = 0 ;

	/*
		チャンネルリスト
	*/
DG_CHANL		DG_Chanls[ DG_MAX_CHANLS ] ;	/* チャンネル＝カメラとして見ていいかも */
DG_OBJ_QUEUE	DG_ObjQueue ;

	/*
		共通オブジェクトキューバッファ
		（各オブジェクトキューに割り振られる。最終的には動的に割り振りを変更できるようにしたい）
	*/
#define MAX_QUEUE_BUFFER	(16*1024)
int		DG_UseQueueBuffer = 0 ;				/* 共通キューバッファ使用量 */
void	*DG_QueueBuffer[ MAX_QUEUE_BUFFER ];/* 共通キューバッファ */

	/*
		プリミティブオフセット設定パケット
	*/
//static DG_DRAWOFFSET	DG_DrawOffset[2] ;		/* ハイレゾ用 */
//static DG_DRAWOFFSET	DG_DrawOffsetLow[2] ;	/* ローレゾ用 */

	/*
		画面消去パケット
	*/
static DG_DRAWENV		DG_DefaultDrawEnv[2] ;
static DG_DRAWOFFSET	DG_DefaultDrawOffset[2] ;
static ALIGN16_DECL(struct) _dg_clear_packet{
	DG_GIFTAG		giftag ;
	sceGsClear		clear ;
} DG_ClearPacket ;


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

/*----------------------------------------------------------------*/

// BPRender - Hook for MSAA logic
void DG_MSAAHookChanl( DG_CHANL *chanl, int which )
{
   BP_RB_PushRegionMarker(0xFF0000FF, "AAResolve");
   BP_RB_AddCommand(kCmd_FrameMSAAEndPacket, NULL);
   BP_RB_PopRegionMarker();
}

	/*
		チャンネル処理ユニット
	*/
void DG_DummyChanl( DG_CHANL *chanl, int which ){}	/* ダミー処理ユニット */

static	QueFunc	NormalStageList[] = {/* 標準処理ステージ */
	DG_PluginStartChanl,	/* ●プラグイン初期化ステージ */
	DG_FrameChanl,			/* 描画環境初期化＆設定ステージ */
	DG_PluginChanl,			/* ●プラグイン処理ステージ（０） */
	DG_ScreenChanl,			/* マトリクス計算＆バウンディングチェック */
#if 1
	DG_Chain2Chanl,			/* マルチテクスチャオブジェクトのＤＭＡ接続ステージ */
	DG_ChainEvmChanl,		/* マルチウェイトエンベロープオブジェクト処理ステージ */
	DG_ChainChanl,			/* 不透明オブジェクトのＤＭＡ接続ステージ */
#else
	DG_ChainEvmChanl,		/* マルチウェイトエンベロープオブジェクト処理ステージ */
	DG_ChainChanl,			/* 不透明オブジェクトのＤＭＡ接続ステージ */
	DG_CoverShadowChainChanl, /* カバーシャドウ処理（実験用） */
	DG_Chain2Chanl,			/* マルチテクスチャオブジェクトのＤＭＡ接続ステージ */
#endif
	DG_Chain2ChanlLatter,	/* マルチテクスチャオブジェクトのＤＭＡ接続ステージ */
	DG_ChainEvmChanlLatter,	/* マルチウェイトエンベロープオブジェクト処理ステージ */
   DG_MSAAHookChanl, /* Hook for MSAA logic command */
	DG_ShadowChainChanl,	/* （特殊）影投影処理ステージ */
	DG_SpotChainChanl,		/* （特殊）スポットライト投影ステージ */
	//DG_ComdlChanl,			/* （特殊）共有モデルオブジェクト処理ステージ */
	DG_PluginChanl,			/* ●プラグイン処理ステージ（１） */
	DG_Prim2Chanl,			/* プリミティブソート処理ステージ */
   DG_ChainChanlLast,
   DG_Chain2ChanlLast,
   DG_Prim2Chanl,
	DG_SortChainChanl,		/* 半透明オブジェクト＆プリミティブのＤＭＡ接続ステージ */
	//DG_PrimChanl, DG_SortChanl,	/* 旧プリミティブ処理ステージ（なくなる予定） */
	DG_PluginChanl,			/* ●プラグイン処理ステージ（２） */
	DG_PluginChanl,			/* ●プラグイン処理ステージ（３） */
	DG_PluginEndChanl,		/* ●プラグイン終了ステージ */
	DG_DummyChanl
} ;

static char *skMainChannelStageNames[] =
{
   "DG_PluginStartChanl",
   "DG_FrameChanl",
   "DG_PluginChanl",
   "DG_ScreenChanl",
   "DG_Chain2Chanl",
   "DG_ChainEvmChanl",
   "DG_ChainChanl",
   "DG_Chain2ChanlLatter",
   "DG_ChainEvmChanlLatter",
   "DG_MSAAHookChanl",
   "DG_ShadowChainChanl",
   "DG_SpotChainChanl",
   "DG_PluginChanl",
   "DG_Prim2Chanl",
   "DG_ChainChanlLast",
   "DG_Chain2ChanlLast",
   "DG_SortChainChanl",
   "DG_PluginChanl",
   "DG_PluginChanl",
   "DG_PluginEndChanl",
   "DG_DummyChanl"
};

static char *skChannelNames[] =
{
   "ChannelMain",
   "ChannelSub1",
   "ChannelSub2",
   "ChannelSub3",
   "ChannelMenu",
};

static QueFunc MenuStageList[] = {
	DG_PluginStartChanl,	/* ●プラグイン初期化ステージ */
	DG_FrameChanl,			/* 描画環境初期化＆設定ステージ */
   DG_MSAAHookChanl, /* Hook for MSAA logic command */
	DG_PluginChanl,			/* ●プラグイン処理ステージ（０） */
	DG_PluginChanl,			/* ●プラグイン処理ステージ（１） */
	//DG_PrimChanl,DG_SortChanl,/* 旧プリミティブ処理ステージ */
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

	/* GIFTAGの設定 */
	env->gif_tag = SCE_GIF_SET_TAG(1, 1, 0, 0, 0, sizeof(env->datas)/16);
	DG_ENABLE_DRAWENV( env,
					  DG_DRAWENV_FRAME|DG_DRAWENV_ZBUF|DG_DRAWENV_XYOFFSET|DG_DRAWENV_SCISSOR
					  |DG_DRAWENV_PRMODECONT|DG_DRAWENV_COLCLAMP|DG_DRAWENV_TEST
					  |DG_DRAWENV_PABE|DG_DRAWENV_TEXA|DG_DRAWENV_DTHE|DG_DRAWENV_DIMX);

	/* 各レジスタの設定 */
	env->datas.frame1_addr = SCE_GS_FRAME_1 ;
	*(u_long64*)&env->datas.frame1 = 
	  SCE_GS_SET_FRAME( BUFFER_PAGE(page)/2048, BUFFER_WIDTH/64, FRAME_BUFFER_COLOR_MODE(), 0x00000000 ) ;
	env->datas.zbuf1_addr = SCE_GS_ZBUF_1 ;
	*(u_long64*)&env->datas.zbuf1 = 
	  SCE_GS_SET_ZBUF( ZBUFFER_PAGE()/2048, Z_BUFFER_COLOR_MODE(), 0 ) ;

	env->datas.xyoffset1_addr = SCE_GS_XYOFFSET_1 ;
	*(u_long64*)&env->datas.xyoffset1 = 
	  SCE_GS_SET_XYOFFSET( ( ( 2048 - DRAW_WIDTH / 2 - x ) << 4 ), ( ( 2048 - DRAW_HEIGHT / 2 - y ) << 4 ) ) ;
	env->datas.scissor1_addr = SCE_GS_SCISSOR_1 ;
	*(u_long64*)&env->datas.scissor1 = 
	  SCE_GS_SET_SCISSOR( left, right, top, bottom ) ;
	env->datas.prmodecont_addr = SCE_GS_PRMODECONT ;
	*(u_long64*)&env->datas.prmodecont = SCE_GS_SET_PRMODECONT( 1 ) ;
	env->datas.colclamp_addr = SCE_GS_COLCLAMP ;
	*(u_long64*)&env->datas.colclamp = SCE_GS_SET_COLCLAMP( 1 ) ;
	env->datas.test1_addr = SCE_GS_TEST_1 ;
	*(u_long64*)&env->datas.test1 = SCE_GS_SET_TEST( 1, 5, 64, 1, 0, 0, 1, 2 ) ;
	env->datas.pabe_addr = SCE_GS_PABE ;
	*(u_long64*)&env->datas.pabe = SCE_GS_SET_PABE( 0 ) ;
	env->datas.texa_addr = SCE_GS_TEXA ;
	*(u_long64*)&env->datas.texa = SCE_GS_SET_TEXA( 128, 1, 128 ) ;
#if COLOR_DEPTH != 32
	env->datas.dthe_addr = SCE_GS_DTHE ;
	*(u_long64*)&env->datas.dthe = 1 ;
	env->datas.dimx_addr = SCE_GS_DIMX ;
	*(u_long64*)&env->datas.dimx = 0x4253061753421706 ;
#else
	env->datas.dthe_addr = SCE_GS_DTHE ;
	*(u_long64*)&env->datas.dthe = 0 ;
	env->datas.dimx_addr = SCE_GS_DIMX ;
	*(u_long64*)&env->datas.dimx = 0x4253061753421706 ;
#endif
	env->datas.clamp_addr = SCE_GS_CLAMP_1 ;
	*(u_long64*)&env->datas.clamp = SCE_GS_SET_CLAMP(1,1,0,0,0,0) ;


	/* GIFTAGの設定 */
	offset->gif_tag.tag = SCE_GIF_SET_TAG( sizeof(offset->datas)/16, 1, 0, 0, 0, 1 );
	offset->gif_tag.regs = 0x0e ;
	/* 各レジスタの設定 */
	offset->datas.xyoffset1_addr = SCE_GS_XYOFFSET_1 ;
	*(u_long64*)&offset->datas.xyoffset1 = 
	  SCE_GS_SET_XYOFFSET( ( ( 2048 - DRAW_WIDTH / 2 - x ) << 4 ), ( ( 2048 - DRAW_HEIGHT / 2 - y ) << 4 ) ) ;

#if 0
	/* ディザテスト */
	*(u_long64*)&env->datas.frame1 = 
	  SCE_GS_SET_FRAME( BUFFER_PAGE(page)/2048, BUFFER_WIDTH/64, FRAME_BUFFER_COLOR_MODE(), 0x00070707 ) ;
	*(u_long64*)&env->datas.dthe = 1 ;
	*(u_long64*)&env->datas.dimx = 0x4253061753421706 ;/* 普通 */
	*(u_long64*)&env->datas.dimx = 0x4141373741413737 ;/* ＰＳ風 */
	*(u_long64*)&env->datas.dimx = 0x4411337744113377 ;/* ＰＳ風（ローレゾ風味） */
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
	*(u_long64*)&cp->draw_env[0].datas.scissor1 =  SCE_GS_SET_SCISSOR( x1, x2, y1, y2 ) ;
	*(u_long64*)&cp->draw_env[1].datas.scissor1 =  SCE_GS_SET_SCISSOR( x1, x2, y1, y2 ) ;
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


static DG_PLUGIN sAS_ExtraTrans_Plugin;
static int sAS_ExtraTrans_Plugin_Init = 0;

#define EXTRATRANS_PLUGIN_ID		(0x009c9a85)	/* "as_extratrans" */
#define EXTRATRANS_PLUGIN_FLAG		(DG_PLUGIN_FLAG_OBJBUFFER|DG_PLUGIN_FLAG_ENABLE)
#define EXTRATRANS_PLUGIN_PHASE	(DG_PLUGIN_PHASE_NORMAL)
#define EXTRATRANS_PLUGIN_PRIO		(DG_PLUGIN_PRIO_NORMAL)
#define EXTRATRANS_PLUGIN_USE_QUEUE	(32)

static void _DG_AS_PluginActorTransNoMSAA( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status )
{
   // AS(JM)
   // The "extra trans" plugin calls the NoMSAA transparency channels, which are 
   // Prim2First and SortChainFirst
   DG_Prim2FirstChanl( cp, which );
   DG_SortChainFirstChanl( cp, which );
}

static void _DG_AS_AddPluginTransNoMSAA()
{
   if ( sAS_ExtraTrans_Plugin_Init ) return;

   DG_MakePlugin( &sAS_ExtraTrans_Plugin, EXTRATRANS_PLUGIN_ID, EXTRATRANS_PLUGIN_FLAG, EXTRATRANS_PLUGIN_PHASE,
      EXTRATRANS_PLUGIN_PRIO, _DG_AS_PluginActorTransNoMSAA, EXTRATRANS_PLUGIN_USE_QUEUE );
   DG_AddPlugin( &sAS_ExtraTrans_Plugin );

   sAS_ExtraTrans_Plugin_Init = 1;
}

static void _DG_AS_DeletePluginTransNoMSAA()
{
   sAS_ExtraTrans_Plugin_Init = 0;
   DG_DeletePlugin( &sAS_ExtraTrans_Plugin );
   DG_FreePlugin( &sAS_ExtraTrans_Plugin );
}

/*----------------------------------------------------------------*/

void		DG_InitChanlSystem( int flag )
{
	DG_CHANL	*cp ;

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

	/*
		画面消去用デフォルト描画環境作成
	*/
	SetDefDrawEnv( &DG_DefaultDrawEnv[0], &DG_DefaultDrawOffset[0], 0, 0, 0, DRAW_WIDTH, DRAW_HEIGHT );
	SetDefDrawEnv( &DG_DefaultDrawEnv[1], &DG_DefaultDrawOffset[1], 1, 0, 0, DRAW_WIDTH, DRAW_HEIGHT );
	DG_ClearPacket.giftag.tag = SCE_GIF_SET_TAG( sizeof(sceGsClear) / 16, 1, 0, 0, 0, 1) ;
	DG_ClearPacket.giftag.regs = GS_REGS_AD ;
#if 0 //BP_PS2
	sceGsSetDefClear( &DG_ClearPacket.clear, SCE_GS_ZGEQUAL,
					 2048-DRAW_WIDTH/2, 2048-DRAW_HEIGHT/2, DRAW_WIDTH, DRAW_HEIGHT, 0, 0, 0, 0, 0 );
#endif

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
	cp ++ ;
	/*
		メニュー用チャンネル（チャンネル４）
	*/
	SetParam( cp, 4 ) ;
	DG_SetDrawEnv( cp, 0, 0, DRAW_WIDTH, DRAW_HEIGHT );
	//DG_SetDrawEnv( cp, -DRAW_WIDTH, -DRAW_HEIGHT, DRAW_WIDTH*2, DRAW_HEIGHT*2 );
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
	cp ++ ;

	/*
		プラグインシステム初期化
	*/
	DG_InitPluginSystem();

	/*
		システム使用パケットの初期化
	*/
	DG_InitSystemPacket();

	/*
		ＭＧＳ２デフォルト起動プラグイン組み込み
	*/
	DG_AddPluginComdl();
	DG_AddPluginDmapack();
	DG_AddPluginPatch();

   // AS(JM) - Transparent NoMSAA renders after patches
   _DG_AS_AddPluginTransNoMSAA();
	
   DG_AddPluginOptcmf();
	DG_AddPluginPatchTrans();
	/* 実験組み込み */
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

extern int gAllowSlowDrawFrameSkips;
int gSkipEveryOther = 0;

extern void BP_RenderMain(int currentBuffer, int shouldDrawBuffer);
extern void BP_RenderClearCommands(int currentBuffer);

void DG_DrawChanlSystem( int which )
{
   int skip = 0;
	/* 描画ＤＭＡ動作開始 */
	DG_DmaStart( which );

   if( gAllowSlowDrawFrameSkips )
   {
      if( gSkipEveryOther == 0)
      {
         skip = 1;
      }
      gSkipEveryOther = 1 - gSkipEveryOther;
   }
   if( skip == 0 )
   {
      BP_RenderMain(which, 1);
   }
   else
   {
      BP_RenderMain(which, 0);
   }
}

void DG_ClearChanlSystem( int which )
{
	/* 描画ＤＭＡバッファ初期化 */
	DG_DmaClear( which );

   BP_RenderClearCommands(which);
}

void DG_UnDrawChanlSystem( int which )
{
	DG_RetryStoreChanl();
	/* 描画ＤＭＡ動作のスキップ */
	DG_DmaSkip( which );

   BP_RenderMain(which, 0);
}

static long64 stop_chanl_system_flag = 0;

static void channel_push_markers(DG_CHANL *cp, int channelIndex)
{
   BP_Debug_PushCPUMarker(skChannelNames[channelIndex]);
#if BP_VITA
   // RB PushMarker needs a scene to work, should only push them if we're going to really render something
   // we don't really care about the viewportinfo, but leave it in anyways
   if ( cp->flag != 0 )
   {
      SBP_FrameBuffer *pCmd = (SBP_FrameBuffer *) BP_RB_Alloc( sizeof( SBP_FrameBuffer ) );

      pCmd->channel = cp->chanl_num;
      BP_RB_AddCommand( kCmd_FrameBuffer, (char*)pCmd );
   }
   if ( cp->flag != 0 )
#endif
   {
      BP_RB_PushMarker(skChannelNames[channelIndex]);
      {
         SBP_ViewportInfo * pPacket = (SBP_ViewportInfo*)BP_RB_Alloc(sizeof(SBP_ViewportInfo));
         pPacket->hasMSAAStuff = 0;
         pPacket->hasNonMSAAStuff = 0;
         BP_RB_AddCommand(kCmd_FrameNewViewportInfoPacket, pPacket);
         BP_SetViewportInfoPacket( pPacket );
      }
   }
}

static void channel_pop_markers(DG_CHANL *cp)
{
#if BP_VITA
   if (cp->flag != 0)
#endif
   {
      BP_RB_PopMarker();
   }
   BP_Debug_PopCPUMarker();
}

void		DG_SortChanlSystem( int which )
{
	DG_CHANL	*cp ;
	int		i, j, channelIndex ;

	if ( stop_chanl_system_flag )
   {
		return;
	}

	/*
		画面を消す前にイメージストアパケットを接続する
	*/
	DG_StoreChanl( NULL, 1 - which );		/* イメージストア処理 */

	/*
		とりあえず画面を消去する
	*/
	{
      //BP_RENDER TODO: Setup "draw env"?
      {
         BP_RB_AddCommand(kCmd_FrameFirstPacket, NULL);
      }
#if BP_ENABLE_TESTNODE
      BP_Debug_BeginTestNode();
#endif
      // let the channels clear themselves, so the framebuffer gets set up properly
      //{
      //   SBP_ClearViewport* pCmd = (SBP_ClearViewport*)BP_RB_Alloc(sizeof(SBP_ClearViewport));
      //   pCmd->clearColor = 0;
      //   pCmd->clearFlags = 1;
      //   BP_RB_AddCommand(kCmd_ClearViewport,  (char*)pCmd);
      //}
	}

	GV_PROFILE_CHANL_START();

#if BP_VITA
   DG_NewShadowChainSetup();
#endif

	for ( j = 0 ; j < DG_MAX_CHANLS ; j++ )
   {
      channelIndex = j;
      if (DG_ReverseOrderDrawMode && j < 2)
      {
         /* 描画順反転モードのチェック */
         // switch order for mirrors
         channelIndex = j ^ 1;
      }
	   /*
		   チャンネル処理ユニットを、各チャンネルに実行する
	   */
      cp = DG_Chanls_Buf + channelIndex;

      channel_push_markers(cp, channelIndex);

      if (cp->flag != 0)
      {
		   /* チャンネルの各描画フェースを処理 */
		   for ( i = 0 ; i < cp->n_stage ; i++ )
         {
            if (j == DG_CHANL_MAIN)
            {
               BP_Debug_PushCPUMarker(skMainChannelStageNames[i]);
            }
 			   ( *cp->stage_list[i] )( cp, which );
            if (j == DG_CHANL_MAIN)
            {
               BP_Debug_PopCPUMarker();
            }
            GV_PROFILE_CHANL();
		   }
      }

      channel_pop_markers(cp);
	}
#if BP_ENABLE_TESTNODE
   BP_Debug_EndTestNode();
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
      //printf("QueObjs: queue buffer over!!\n");
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
	return ( ret ) ;
}

void DG_DequeueObjs( DG_OBJS *objs )
{
	//return DequeueObjs( &DG_Chanls[ objs->chanl ].obj_queue->objs_buffer, objs );
	DequeueObjs( &DG_Chanls[ 0 ].obj_queue->objs_buffer, objs );
}

DG_OBJS* DG_SearchQueueObjs( DG_OBJS *objs )
{
	return SearchQueueObjs( &DG_Chanls[ 0 ].obj_queue->objs_buffer, objs );
}

#if 0
	/*
		影生成オブジェクト
	*/
int DG_QueueShadwmakeObjs( DG_OBJS *objs )
{
	int		ret ;
	//printf("objs1:");
	//ret = QueueObjs( &DG_Chanls[ objs->chanl ].obj_queue->shdwmake_buffer, objs );
	ret = QueueObjs( &DG_Chanls[ 0 ].obj_queue->shdwmake_buffer, objs );
	if ( ret != 0 ) GV_ERROR( GV_ERROR_MAX_MISC_QUEUE );
	return ( ret ) ;
}

void DG_DequeueShadowmakeObjs( DG_OBJS *objs )
{
	//return DequeueObjs( &DG_Chanls[ objs->chanl ].obj_queue->shdwmake_buffer, objs );
	return DequeueObjs( &DG_Chanls[ 0 ].obj_queue->shdwmake_buffer, objs );
}
#endif

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
	DequeueObjs( &DG_Chanls[ 0 ].obj_queue->shdwwrite_buffer, objs );
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
	DequeueObjs( &DG_Chanls[ 0 ].obj_queue->spot_buffer, objs );
}
#if 0
	/*
		共有モデルオブジェクト
	*/
int DG_QueueComdlObjs( DG_COMDL *comdl )
{
	//printf("comdl:");
	//return QueueObjs( &DG_Chanls[ comdl->chanl ].obj_queue->comdl_buffer, comdl );
	return QueueObjs( &DG_Chanls[ 0 ].obj_queue->comdl_buffer, comdl );
}

void DG_DequeueComdlObjs( DG_COMDL *comdl )
{
	//return DequeueObjs( &DG_Chanls[ comdl->chanl ].obj_queue->comdl_buffer, comdl );
	return DequeueObjs( &DG_Chanls[ 0 ].obj_queue->comdl_buffer, comdl );
}
#endif

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
	DequeueObjs( &DG_Chanls[ 0 ].obj_queue->evmobj_buffer, evmobj );
}

	/*
		プリミティブオブジェクト
	*/
int DG_QueuePrim( DG_PRIM *objs )
{
#if 0
	int		ret ;
	//printf("prim:");
	//ret = QueueObjs( &DG_Chanls[ objs->chanl ].obj_queue->prim_buffer, objs );
	ret = QueueObjs( &DG_Chanls[ 0 ].obj_queue->prim_buffer, objs );
	if ( ret != 0 ) GV_ERROR( GV_ERROR_MAX_MISC_QUEUE );
	return ( ret );
#endif
	return ( -1 );
}

void DG_DequeuePrim( DG_PRIM *objs )
{
#if 0
	//return DequeueObjs( &DG_Chanls[ objs->chanl ].obj_queue->prim_buffer, objs );
	return DequeueObjs( &DG_Chanls[ 0 ].obj_queue->prim_buffer, objs );
#endif
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
	DequeueObjs( &DG_Chanls[ 0 ].obj_queue->prim2_buffer, objs );
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
	DequeueObjs( obj_buff, objs );
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
#if 0
	DG_CHANL	*cp ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJS		**oque, *objs;
	int i,j ;
#endif

	stop_chanl_system_flag = 1;

#if 0
	cp = DG_Chanls ;		/* メインチャンネル */

	for ( j = DG_MAX_CHANLS ; j > 0 ; j-- ){
		que = cp->obj_queue;
		if ( que != NULL ){
			oque = que->objs_buffer.queue ;
			for( i = que->objs_buffer.n_queue; i > 0; i-- ){
				objs = *( oque ++ );
				//DG_FreeObjsPacket( objs, 0 );
				//DG_FreeObjsPacket( objs, 1 );
			}
		}
		cp++ ;
	}
#endif
}

void DG_RestartMainChanlSystem( void )
{
	/*
	   メイン画面用のチャンネル処理を再開する。
	*/
//	extern long64 DG_UnDrawFrameCount;

	stop_chanl_system_flag = 0;
	DG_UnDrawFrameCount = 1;
}

/* ------------------------------------------------------ */

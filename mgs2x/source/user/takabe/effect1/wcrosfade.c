/*
	wcrosfade.c
	クロスフェードエフェクト(Windows版)

	2000/02/05 K.Takabe
	2002/10/24 E.Takaki
	$Id: wcrosfade.c,v 1.3 2002/11/21 07:50:45 takaki Exp $

*/
/*
	void *NewCrossFadeEffect( int time )
	int		time ;	フェード時間（１／３００秒単位）

	キャラを起動したときのフレームを退避し、指定時間かけて
	クロスフェードを行う
	１つ起動するごとにメモリを５１２ＫＢ消費するので注意すること！
	（起動タイミングとしてはカメラが変化したタイミングで起動すると
	切り替え前の最終フレームをクロスフェード用として取り込むことができる）


	void *NewCrossFadeEffectCustom( int time, int capture_interval, int bright_time, int alpha_time, int flag )
	int		time ;				起動時間（１／３００秒単位）
	int		capture_interval ;	フレームバッファ再取り込み間隔（１／３００秒単位）
	int		bright_time ;		輝度のフェード時間（１／３００秒単位）
	int		alpha_time ;		アルファのフェード時間（１／３００秒単位）
	int		flag ;				各種フラグ（現在未使用）
								0x0001:低プライオリティモード（フェードインアウトに負ける）

	特殊な機能をつけたクロスフェードエフェクト
	フレームバッファの再取り込みを行うと自動的に輝度とアルファのフェードが
	初期値に戻る
	輝度ののフェード時間を０にすると常に輝度が１００％になる
	アルファのフェード時間を０にすると常にアルファが１２８になる
	フラグは現在使用されていない


*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"def_dma.h"
#include	"gameheader.h"
#include	"../../mode/demo/libdemo.h"


#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)

#define DRAW_DIV	(16)		/* 描画の分割数 */
#define RASTER_NUM	(112)

/* 通常設定 */
#define NORMAL_PHASE	(DG_DMAPACK_PHASE_AFTER)
#define NORMAL_PRIO		(127)
/* 低プライオリティモード設定（フェードインアウトに負けるように設定） */
#define LOW_PHASE		(DG_DMAPACK_PHASE_NORMAL)
#define LOW_PRIO		(64)

/* ---------------------------------------------------------------- */
/* 外部変数 */
#ifdef KP_XBOX //BP
extern D3DFORMAT	DG_BackBufferFormat ;		// BackBufferFormat
extern DWORD		DG_BackBufferRendWidth ;
extern DWORD		DG_BackBufferRendHeight ;
extern DWORD		DG_BackBufferWidth ;
extern DWORD		DG_BackBufferHeight ;
#endif
/* ---------------------------------------------------------------- */
/* プログラムワーク */
typedef	struct	{
#ifndef KP_WINDOWS
	GV_ACT_EX	actor ;
#else
	GV_ACT_EX_RD	actor ;	// Reset Device 処理対応版
#endif
	DG_DMAPACK	*dmapack ;
	int			time ;
	int			count ;
	int			start_flag ;
#if FALSE
	void		*texture_memory ;
#endif
	int			mode ;
	/* 拡張ワーク */
	int			capture_interval ;		/* 画像取り込み間隔 */
	int			alpha_time ;			/* アルファ減衰時間 */
	int			bright_time ;			/* 明るさ減衰時間 */
	int			capture_count ;			/*  */
	int			alpha_count ;			/*  */
	int			bright_count ;			/*  */
	int			alpha ;
	int			bright ;
	int			flag ;			/* 制御フラグ */

#if 0 //BP
#ifndef KP_WINDOWS
	DG_TEX_LIN	tex ;
#endif
#ifdef KP_WINDOWS
	DG_USERTEX	tex ;
#endif
#endif

} Work ;

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/*
	ＸＢＯＸ用ルーチン
*/
typedef struct _screen_draw {
#ifndef KP_WINDOWS
	DG_DMAPACK_PARAM	use_texlin ;
#else
	DG_DMAPACK_PARAM	use_usertex ;
#endif
	DG_DMAPACK_ALPHA	alpha ;
	DG_DMAPACK_SPRT		sprt ;
} SCREEN_DRAW ;

typedef struct {
	/* テクスチャ転送 */

	/* 初期化パケット */

	/* メイン描画パケット */
	SCREEN_DRAW	screen_draw ;

	/* 変更環境の復元パケット */
	DG_DMAPACK_PARAM	end ;
} ALL_PACKET ;

/* ---------------------------------------------------------------- */
/* ＤＭＡＰＡＣＫ用パケット初期化 */
static void InitDmaPacket( ALL_PACKET *packet, int which )
{
	DG_SetDmapackEnd( &packet->end );
}
/* スクリーン描画パケット設定 */
static void *SetScreenDrawPacket( SCREEN_DRAW *screen_draw,
						 u_long64 alpha,
						 int rgba, DG_USERTEX *tex )
{
   BP_RENDER_TODO_BREAK;
#if 1 //BP
   return NULL;
#else
	/* ピクセルテスト設定 */
	/* アルファ設定 */
	DG_SetDmapackAlpha( &screen_draw->alpha, alpha );
	/* テクスチャページ設定(flag = 0x4:rgba_tex) */
	DG_SetDmapackUserTex( &screen_draw->use_usertex, tex );
	/* 描画領域＆ＵＶ座標設定 */
	{/* ＸＢＯＸでは必ずフレームバッファを退避して描画するので重なりは気にする必要がない */
		float		x, y, w, h, u, v, uw, vh, uof, vof ;
		uof = ( DISPLAY_WIDTH - DRAW_WIDTH ) / 2 ;
		vof = ( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2 ;
		x = 0 ; w = DRAW_WIDTH ;
		y = 0 ; h = DRAW_HEIGHT ;
#ifndef KP_WINDOWS
		u = (float)( uof + 0.5f ) / (float)tex->width ;
		uw = (float)( DRAW_WIDTH ) / (float)tex->width ;
		v = (float)( vof + 0.5f ) / (float)tex->height ;
		vh = (float)( DRAW_HEIGHT ) / (float)tex->height ;
#else
		u  = (float)( uof + 0.5f ) / (float)DISPLAY_WIDTH ;
		uw = (float)( DRAW_WIDTH ) / (float)DISPLAY_WIDTH ;
		v  = (float)( vof + 0.5f ) / (float)DISPLAY_HEIGHT ;
		vh = (float)( DRAW_HEIGHT ) / (float)DISPLAY_HEIGHT ;
#if FALSE
		u  *= tex->texcoord_scale_w ;
		v  *= tex->texcoord_scale_h ;
		uw *= tex->texcoord_scale_w ;
		vh *= tex->texcoord_scale_h ;
#endif
#endif
		DG_SetDmapackSprt( &screen_draw->sprt,
						  x, y, u, v,
						  x+w, y+h, u+uw, v+vh,
						  rgba );
	}

	return ( &screen_draw[1] );
#endif
}


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	ALL_PACKET	*packet ;
	int			alpha, bright ;
	int			col ;

	/* テクスチャ取り込み要求とプリミティブの表示を同時に行う */
	if ( work->start_flag ){
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU ;
#ifndef KP_WINDOWS

      BP_RENDER_TODO_BREAK;
#if 0 //BP
		DG_SetFrameStore( work->texture_memory, 1 );
#endif

#else
		DG_SetFrameStore2UserTex( &work->tex, 1 );
#endif
		work->start_flag = 0 ;
	}

	/* 非ポーズ中のみカウンタを進ませる */
	if ( GV_PauseLevel == 0 ){
		work->count -= TIME_BASE + DM_FrameSkip * TIME_BASE ;
		if ( work->count < 0 ) work->count = 0 ;
	}

	switch ( work->mode ){
	  case 0:
		/* パラメータ設定 */
		alpha = 128 * work->count / work->time ;
		col = 0x80808080 ;
		break ;
	  default:
	  case 1:
		/* 非ポーズ中のみカウンタを進ませる */
		if ( GV_PauseLevel == 0 ){
			work->capture_count += TIME_BASE + DM_FrameSkip * TIME_BASE ;
			work->alpha_count -= TIME_BASE + DM_FrameSkip * TIME_BASE ;
			work->bright_count -= TIME_BASE + DM_FrameSkip * TIME_BASE ;
			/* フレームバッファ取り込みチェック */
			if ( work->capture_count > work->capture_interval ){
				work->capture_count -= work->capture_interval ;
				work->alpha_count = work->alpha_time ;
				work->bright_count = work->bright_time ;
				work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU ;
				work->start_flag = 1 ;
			}
			/* アルファアニメーションチェック */
			if ( work->alpha_count < 0 ){
				work->alpha_count = 0 ;
			}
			if ( work->alpha_time != 0 ){
				work->alpha = 128 * work->alpha_count / work->alpha_time ;
			} else {
				work->alpha = 128 ;
			}
			/* 輝度アニメーションチェック */
			if ( work->bright_count < 0 ){
				work->bright_count = 0 ;
			}
			if ( work->bright_time != 0 ){
				work->bright = 128 * work->bright_count / work->bright_time ;
			} else {
				work->bright = 128 ;
			}
		}
		/* パラメータ設定 */
		alpha = work->alpha ;
		bright = work->bright ;
		col = ( bright ) | ( bright << 8 ) | ( bright << 16 ) | ( bright << 24 );
		break ;
	}

	/* パケットの設定 */
	packet = work->dmapack->autopacket ;
	SetScreenDrawPacket( &packet->screen_draw,
						SCE_GS_SET_ALPHA(0,1,2,1,alpha),
						col, &work->tex );

	/* 終了チェック */
	if ( work->count == 0 ) GV_DestroyActor( work );

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	/* テクスチャ用メモリ開放 */
	DG_ReleaseUserTexture(&work->tex) ;

	/* ＤＭＡパケットオブジェクト開放 */
	if ( work->dmapack != NULL ){
		/* パケットメモリ開放 */
		if ( work->dmapack->autopacket != NULL ) GV_DelayedFree( work->dmapack->autopacket );
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
}

/* ---------------------------------------------------------------- */
static void DmyAct( Work *work ){}
static void DmyDie( Work *work ){}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int time, int flag )
{
	DG_DMAPACK	*dmapack ;
	ALL_PACKET	*packet ;
	u_int		i ;
	HRESULT		hr ;

	work->time = time ;
	work->count = time ;
	work->start_flag = 1 ;

	/* ＤＭＡパケット型オブジェクト作成 */
	if ( !( flag & 0x0001 ) ){
		work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU
												  |DG_DMAPACK_INVISIBLEMENU
												  , NORMAL_PHASE, NORMAL_PRIO );
	} else {
		work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU
												  |DG_DMAPACK_INVISIBLEMENU
												  , LOW_PHASE, LOW_PRIO );
	}
	if ( dmapack == NULL ) return ( -1 );
	DG_QueueDmapack( dmapack );

	/* ここで512KBのメモリを確保 */
#ifndef KP_WINDOWS

   BP_RENDER_TODO_BREAK;
#if 0 //BP
	work->texture_memory = GV_AllocMemory( GV_NORMAL_MEMORY, NULL,
										  ALIGNSIZE128( DISPLAY_WIDTH * DISPLAY_HEIGHT * 4 ) , 128 );
	if ( work->texture_memory == NULL ) return ( -1 );
#endif

#else
	/* Windows版ではレンダリング可能なテクスチャを作成します */
	{
		DWORD	tex_w, tex_h ;

		tex_w = DG_BackBufferRendWidth  >> DG_GetCrossFadeTexLevelW() ;
		tex_h = DG_BackBufferRendHeight >> DG_GetCrossFadeTexLevelH() ;

		if( tex_w < 32 ){ tex_w = 32 ; }
		if( tex_h < 32 ){ tex_h = 32 ; }

		hr = DG_CreateUserTexture(DG_BackBufferFormat,
								tex_w, tex_h,
								DG_CLEATE_USERTEX_RENDERTARGET,
								&work->tex) ;
		if ( FAILED(hr) ) return ( -1 );
	}
#endif

	/* パケットメモリ割り当て */
	packet = GV_Malloc( sizeof(ALL_PACKET) );
	if ( packet == NULL ) return ( -1 );
	GV_ZeroMemory( packet, sizeof(ALL_PACKET) );
	dmapack->autopacket = packet ;
	//dmapack->packet[0] = &packet[0] ;
	//dmapack->packet[1] = &packet[1] ;


	InitDmaPacket( packet, 0 );
	/*  */
#ifndef KP_WINDOWS
	SetScreenDrawPacket( &packet->screen_draw,
						SCE_GS_SET_ALPHA(0,1,2,1,64),
						0x80808080, &work->tex );
#endif

	return (0);
}

/* ---------------------------------------------------------------- */
	/*
		Reset Device 処理用関数
	*/
static	int	DeviceResetRelease(Work *work)
{
	/* UserTextureがRenderTargetとして作成されている為、処理が必要 */
	DG_ResetDeviceRelease_UserTexture(&work->tex) ;

	return(1) ;	// 正常終了
}

static	int	DeviceResetCreate(Work *work)
{
	DG_ResetDeviceCreate_UserTexture(&work->tex) ;

	return(1) ;	// 正常終了
}

/* ---------------------------------------------------------------- */
	/*
		通常クロスフェード
	*/
void *NewCrossFadeEffect( int time )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		if ( DG_CheckUseCrossFade() ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
#ifndef KP_WINDOWS
			GV_ActorEX( &work->actor );
#else
			GV_ActorEX( &work->actor.actor_ex );
			GV_ActorEXRD( &work->actor );
			GV_SetActorResetDeviceFunc( &work->actor, DeviceResetRelease, DeviceResetCreate);
#endif
			if ( GetResources( work, time, 0 ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		} else {
			GV_SetActor( &( work->actor ), DmyAct, DmyDie ) ;
#ifndef KP_WINDOWS
			GV_ActorEX( &work->actor );
#else
			GV_ActorEX( &work->actor.actor_ex );
			GV_ActorEXRD( &work->actor );
#endif
		}
	}
	return (void *)work ;
}

	/*
		特殊クロスフェード
	*/
void *NewCrossFadeEffectCustom( int time, int capture_interval, int bright_time, int alpha_time, int flag )
{
	Work		*work ;

	OPERATOR() ;

	if ( bright_time == 0 && alpha_time == 0 && flag == 0 ){
		extern void *NewFrameSkipEffect( int time, int interval );
		return NewFrameSkipEffect( time, capture_interval );
	}

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		if ( DG_CheckUseCrossFade() ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
#ifndef KP_WINDOWS
			GV_ActorEX( &work->actor );
#else
			GV_ActorEX( &work->actor.actor_ex );
			GV_ActorEXRD( &work->actor );
			GV_SetActorResetDeviceFunc( &work->actor, DeviceResetRelease, DeviceResetCreate);
#endif
			if ( GetResources( work, time, flag ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
			{
				work->alpha = 128 ;
				work->bright = 128 ;
				work->mode = 1 ;
				work->alpha_time = alpha_time ;
				work->bright_time = bright_time ;
				work->capture_interval = capture_interval ;
				work->capture_count = 0 ;
				work->alpha_count = alpha_time ;
				work->bright_count = bright_time ;
				work->flag = flag ;
			}
		} else {
			GV_SetActor( &( work->actor ), DmyAct, DmyDie ) ;
#ifndef KP_WINDOWS
			GV_ActorEX( &work->actor );
#else
			GV_ActorEX( &work->actor.actor_ex );
			GV_ActorEXRD( &work->actor );
#endif
		}
	}
	return (void *)work ;
}


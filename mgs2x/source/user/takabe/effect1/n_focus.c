//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	n_focus.c
	近景ピント暈しエフェクト

	2000/03/02 K.Takabe
	$Id: n_focus.c,v 1.5 2002/11/23 12:09:25 Yoshizawa1 Exp $

*/
/*

＜シナリオ呼び出しインターフェイス＞
chara 近景ぼかし[NewNearFocusEffectSet] $s:name \
	-plane	$b:最大描画枚数 \
	-near	$i:ぼかし最大距離 \
	-far	$i:ぼかし最小距離 \
	-enable // 起動時にオン
mesg 近景ぼかし $s:名前 off[0]
mesg 近景ぼかし $s:名前 on[1]
mesg 近景ぼかし $s:名前 set[2] $w:補間時間 $b:補間タイプ $i:ぼかし最大距離 $i:ぼかし最小距離
// 最大距離と最小距離との間を最大描画枚数で指定した段階でぼかす
// メッセージにより途中で補間しながら影響距離を変更できる
// このときの補間時間は1/60秒単位ではなく1/300秒単位なので注意すること

＜プログラム呼び出しインターフェイス＞
	void *NewNearFocusEffect( int name, int max_plane, int near, int far );
	int		name ;		メッセージを受け取るときの名前
	int		max_plane ;	最大描画枚数
	int		near ;		ぼかし最大距離
	int		far ;		ぼかし最小距離

		機能ＯＦＦメッセージ
		message[0] = 0 ;

		機能ＯＮメッセージ
		message[0] = 1 ;

		パラメータ調節メッセージ
		message[0] = 2(130で補間時間が1/1000秒単位に) ;
		message[1] = 補間時間 ;
		message[2] = 補間タイプ ;
		message[3] = ぼかし最大距離 ;
		message[4] = ぼかし最小距離 ;

※現在指定できる補間タイプは０のみである

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if 0 //BP_PS2 def PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"

#include "BP_RenderBuffer.h"

#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))

#define CLOCK_COUNT	(BP_BASE_TICK())

#define DRAW_DIV	(16)		/* 描画の分割数 */

/* ---------------------------------------------------------------- */
#define MAX_DRAW	(8)		/* ２以上 */
#define FOCUS_NEAR	(1000)
#define FOCUS_FAR	(4000)

/* ---------------------------------------------------------------- */
#if 0 //BP_PS2 def PSX2

//BP NOTE: PS2 code removed, look at version history for reference.

#else
/* ---------------------------------------------------------------- */
/*
	ＸＢＯＸ用ルーチン
*/
typedef	struct n_focus_Work {
	GV_ACT_EX	actor ;

	int			name ;
	int			map ;
	int			disable ;
	int			max_plane ;
	float		target_focus_near ;
	float		target_focus_far ;
	float		focus_near ;
	float		focus_far ;
	int			interp_time ;
	int			interp_type ;
	DG_DMAPACK	*dmapack ;
	void		*packet_mem ;

} Work ;

/* ---------------------------------------------------------------- */
typedef struct _screen_draw {
	DG_DMAPACK_PARAM	ztest_change0 ;
	DG_DMAPACK_PARAM	backup_frame ;
	DG_DMAPACK_PARAM	use_frametex ;
	DG_DMAPACK_PARAM	mode ;
	DG_DMAPACK_ALPHA	alpha ;
	DG_DMAPACK_PARAM	draw_z ;		/*  */
	DG_DMAPACK_SPRT		sprt ;
	DG_DMAPACK_PARAM	ztest_change1 ;
} SCREEN_DRAW ;

typedef struct {

	/* 初期化パケット */

	/* メイン描画パケット */
	struct _draw_packet {
		//FRAME_SET	frame_set0 ;		/* バックバッファをフレームバッファに選択 */
		SCREEN_DRAW	screen_draw0 ;		/* フレームバッファの内容をバックバッファに転送 */
		//SCREEN_DRAW	mask_draw ;			/* 特定深度以降のアルファを削除 */
		//FRAME_SET	frame_set1 ;		/* フレームバッファをメインに設定 */
		//SCREEN_DRAW	screen_draw1 ;		/* バックバッファをフレームバッファに転送 */
	} draw_packet[ MAX_DRAW ];

	/* 変更環境の復元パケット */
	DG_DMAPACK_PARAM	reset_mode ;

	DG_DMAPACK_PARAM	end ;

} ALL_PACKET ;

/* ---------------------------------------------------------------- */
static void InitDmaPacket( ALL_PACKET *packet, int which )
{
	DG_SetDmapackEnd( &packet->end );
}

static void *SetScreenDrawPacket( SCREEN_DRAW *screen_draw, int flag,
						 int test,
						 u_long64 alpha,
						 int tex_page,
						 int rgba,
						 float z, int dir )
{

	/* ピクセルテスト設定 */
	DG_SetDmapackModeEnable( &screen_draw->ztest_change0, DG_DMAPACK_MODE_ZTEST_REV );
	DG_SetDmapackModeDisable( &screen_draw->ztest_change1, DG_DMAPACK_MODE_ZTEST_REV );
	/* アルファ設定 */
	DG_SetDmapackAlpha( &screen_draw->alpha, alpha );
	/* テクスチャページ設定(flag = 0x4:rgba_tex) */
	DG_SetDmapackBackupFrame( &screen_draw->backup_frame, 0 );
	DG_SetDmapackUseFrameTex( &screen_draw->use_frametex, 2 );
	/* ブレンドの制度を向上させるためにＰＳ２エミュレーションのカラースケールを禁止する */
	/* この設定はテクスチャをセットした後に指定しないとパラメータがリセットされてしまうので注意！ */
	DG_SetDmapackModeEnable( &screen_draw->mode, DG_DMAPACK_MODE_MODULATE1X );
	/* 描画Ｚ設定 */
	DG_SetDmapackSetZ( &screen_draw->draw_z, z );
	/* 描画領域＆ＵＶ座標設定 */
	{/* ＸＢＯＸでは必ずフレームバッファを退避して描画するので重なりは気にする必要がない */
		float		x, y, w, h, u, v, uw, vh ;
		x = 0 ; w = DRAW_WIDTH ;
		y = 0 ; h = DRAW_HEIGHT ;
		/* Ｘ軸方向のプリミティブ座標、ＵＶ座標をセットアップ */
		if ( dir & 1 ){
			u = DG_FRAME_U( 1 );
			uw = DG_FRAME_U( DRAW_WIDTH + 1 );
		} else {
			u = DG_FRAME_U( 0 );
			uw = DG_FRAME_U( DRAW_WIDTH );
		}
		/* Ｙ軸方向のプリミティブ座標、ＵＶ座標をセットアップ */
		if ( dir & 2 ){
			v = DG_FRAME_V( 1 );
			vh = DG_FRAME_V( DRAW_HEIGHT + 1 );
		} else {
			v = DG_FRAME_V( 0 );
			vh = DG_FRAME_V( DRAW_HEIGHT );
		}
		DG_SetDmapackSprt( &screen_draw->sprt,
						  x, y, u, v,
						  x+w, y+h, u+uw, v+vh,
						  DG_MakeDmaPackColorFromInt( rgba ) );
	}

	return ( &screen_draw[1] );

}

/* ---------------------------------------------------------------- */
static float	CalcDepth( float z )
{
	FVECTOR	view_pos ;

	view_pos.vx = 0.0f ;
	view_pos.vy = 0.0f ;
	view_pos.vz = (float)z ;
	view_pos.vw = 1.0f ;
#if 0 //BP_XBOX
	_sceVu0ApplyMatrix( &view_pos, &( DG_Chanl( 0 )->xpers ), &view_pos );
#else
   //NOTE: Switched to different variable because "xpers" doesn't exist, investigate what the difference between "pers" and "xpers" is!
   _sceVu0ApplyMatrix( &view_pos, &( DG_Chanl( 0 )->pers ), &view_pos );
#endif
	if ( view_pos.vw != 0 ){
		z = view_pos.vz / view_pos.vw ;
	} else {
		z = 1 ;
	}
	return ( z );
}

static void SetParam( Work *work, ALL_PACKET *packet, int which, int first_flag )
{
	void	*prim ;
	int		i ;
	float	z ;

	/* 毎フレームパケットを０からせ生成する */
	prim = packet ;
	/* ニアとファーが逆の場合にはエフェクトを掛けない
	   （ＰＳ２版は整数のオーバーフローでたまたまこれと同じ挙動になっていた） */
	if ( work->focus_near <= work->focus_far ){
		DG_SetDmapackEnd( prim );
		return ;
	}
	{/* テクスチャクランプとシザー領域を対応させる */
	}

	/* パケットの初期化 */
	for ( i = 0 ; i < MAX_DRAW ; i++ ){
		z = ( work->focus_near - work->focus_far ) * i / ( work->max_plane - 1 ) + work->focus_far ;
		if ( z < 0 ) z = 0 ;
		if ( z > 1 ) z = 1 ;

		if ( i >= work->max_plane || ( z == 1.0f || z == 0.0f )){
			continue ;
		}
		/*
			沢山重ねがきをするとＰＳ２エミュレーションのカラースケールによる精度落ちが
			現われてくるのでここでは一時的にカラースケールを禁止して1.0=0xffとして指定
			しているので注意！
		*/
		prim = SetScreenDrawPacket( prim,
							0x01,
							SCE_GS_SET_TEST( 0, 7, 64, 1, 0, 0, 1, 2 ),
							SCE_GS_SET_ALPHA(0,1,2,1,255),
							which,
							0xffffffff,
							z, i & 3 );
	}
	/* 終端パケット書き込み */
	prim = DG_SetDmapackModeDisable( prim, DG_DMAPACK_MODE_MODULATE1X );
	DG_SetDmapackEnd( prim );
}


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		n_msg ;
	GV_MSG	*msg ;

	if( ( n_msg = GV_ReceiveMessage( work->name, &msg ) ) == 0 ){
		if ( work->disable ){
			GV_WaitMessage( work, work->name );
			return;
		}
	} else {
		/* メッセージチェック */
		for ( ; n_msg > 0 ; n_msg--, msg++ ){
			switch ( msg->message[0] ){
			  case 0:/* 停止 */
				work->disable = 1 ;
				break ;
			  case 1:/* 再開 */
				work->disable = 0 ;
				break ;
			  case 2:/* フォーカスセット */
			  case 130:
				work->interp_time = msg->message[1] ;	/* 補間時間 */
				work->interp_type = msg->message[2] ;	/* 補間タイプ */
				work->target_focus_near = CalcDepth( msg->message[3] ) ;/* フォーカス開始距離 */
				work->target_focus_far = CalcDepth( msg->message[4] ) ;/* フォーカス終了距離 */
				work->disable = 0 ;
				if ( msg->message[0] >= 128 ){
					work->interp_time = msg->message[1] * 300 / 1000 ;	/* 補間時間 */
				}
				break ;
			  case 3:/* プリセットフォーカス発動 */
				break ;
			}
		}
	}

	SetParam( work, work->dmapack->autopacket, DG_Clock, 0 );

	if ( work->interp_time > 0 ){
		float		f ;
		switch ( work->interp_type ){
		  case 0:/* 線形補間 */
			f = (float)CLOCK_COUNT / (float)work->interp_time ;
			work->focus_near += ( work->target_focus_near - work->focus_near ) * f ;
			work->focus_far += ( work->target_focus_far - work->focus_far ) * f ;
			break ;
		}
		work->interp_time -= CLOCK_COUNT ;
		if ( work->interp_time < 0 ) work->interp_time = 0 ;
	} else {
		work->focus_near = work->target_focus_near ;
		work->focus_far = work->target_focus_far ;
	}

	if ( work->disable ){
		work->dmapack->flag |= DG_DMAPACK_INVISIBLE0 ;
	} else {
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE0 ;
	}

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	/* パケットメモリ開放 */
	if ( work->packet_mem != NULL ) GV_DelayedFree( work->packet_mem );
	/* ＤＭＡパケットオブジェクト開放 */
	if ( work->dmapack != NULL ){
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
}

/* ---------------------------------------------------------------- */
#ifdef KP_WINDOWS
static void DmyAct( Work *work ){}
static void DmyDie( Work *work ){}
#endif

/* ---------------------------------------------------------------- */

static void BP_NearFocusCallback(void* param)
{
   Work* pWork = (Work*)param;
}

static int GetResources( Work *work, int name, int where )
{
	DG_DMAPACK	*dmapack ;
	ALL_PACKET	*packet ;

	work->name = name ;
	work->map = where ;
	work->interp_time = 0 ;					/* 補間時間 */
	work->interp_type = 0 ;					/* 補間タイプ */
	work->focus_near = work->target_focus_near ;
	work->focus_far = work->target_focus_far ;

	/* ＤＭＡパケット型オブジェクト作成 */
	work->dmapack = dmapack = DG_MakeDmapack( DG_DMAPACK_NORMAL|
											 DG_DMAPACK_INVISIBLE1|
											 DG_DMAPACK_INVISIBLE2|
											 DG_DMAPACK_INVISIBLE3
											 , DG_DMAPACK_PHASE_AFTER );
	if ( dmapack == NULL ) return ( -1 );

   DG_AS_DMAPackSetCallbacks(dmapack, BP_NearFocusCallback, NULL);
   dmapack->BP_callbackParam = work;

#if BP_DMAPACK_DEBUG_INFO
   dmapack->BP_LabelMask |= kRL_PostFx;
#endif

   
   DG_QueueDmapack( dmapack );

	/* パケットメモリ割り当て */
	work->packet_mem = packet = GV_Malloc( sizeof(ALL_PACKET) );
	if ( packet == NULL ) return ( -1 );
	dmapack->autopacket = packet ;
	//dmapack->packet[0] = &packet[0] ;
	//dmapack->packet[1] = &packet[1] ;

	//InitDmaPacket( &packet[1], 1 );

	SetParam( work, packet, 0, 1 );

	if ( work->disable ){
		work->dmapack->flag |= DG_DMAPACK_INVISIBLE0 ;
	} else {
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE0 ;
	}

	return (0);
}

static int GetResources_S( Work *work, int name, int where )
{

	work->disable = 1 ;
	work->max_plane = 4 ;
	work->target_focus_near = 0 ;	/* フォーカス開始距離 */
	work->target_focus_far = 0 ;	/* フォーカス終了距離 */

    if ( GCL_GetOption( 'e' ) != NULL ) {
		work->disable = 0 ;
	}
    if ( GCL_GetOption( 'p' ) != NULL ) {
		work->max_plane = GCL_GetNextInt();
	}
    if ( GCL_GetOption( 'n' ) != NULL ) {
		work->target_focus_near = CalcDepth( GCL_GetNextInt() );
	}
    if ( GCL_GetOption( 'f' ) != NULL ) {
		work->target_focus_far = CalcDepth( GCL_GetNextInt() );
	}

	return GetResources( work, name, where );
}

static int GetResources_P( Work *work, int name, int max_plane, int var_near, int var_far )
{
	work->disable = 0 ;
	work->max_plane = max_plane ;
	work->target_focus_near = CalcDepth( var_near ) ;
	work->target_focus_far = CalcDepth( var_far ) ;
	return GetResources( work, name, GM_CurrentMap );
}
#endif

/* ---------------------------------------------------------------- */
/* シナリオ呼び出しインターフェイス */
void *NewNearFocusEffectSet( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	//work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources_S( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
#else
		if ( DG_CheckUseFocus() ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor );
			if ( GetResources_S( work, name, where ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		} else {
			GV_SetActor( &( work->actor ), DmyAct, DmyDie ) ;
			GV_ActorEX( &work->actor );
		}
#endif
	}
	return (void *)work ;
}

/* プログラム呼び出しインターフェイス */
void *NewNearFocusEffect( int name, int max_plane, int var_near, int var_far )
{
	Work		*work ;

	OPERATOR() ;
	//work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources_P( work, name, max_plane, var_near, var_far ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
#else
		if ( DG_CheckUseFocus() ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor );
			if ( GetResources_P( work, name, max_plane, var_near, var_far ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		} else {
			GV_SetActor( &( work->actor ), DmyAct, DmyDie ) ;	// Dummy
			GV_ActorEX( &work->actor );
		}
#endif
	}
	return (void *)work ;
}

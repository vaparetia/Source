/*
  c_ffocus.c
  遠景ピント暈しエフェクト

  2000/03/02 K.Takabe(Original)
  2000/11/11 Y.Kira(Modify)
  
  $Id: c_ffocus.c,v 1.1.1.3 2002/11/19 11:45:03 Yoshizawa1 Exp $
*/
/*

＜シナリオ呼び出しインターフェイス＞
chara 遠景ぼかし[NewFarFocusEffectSet] $s:name \
	-plane	$b:最大描画枚数 \
	-near	$i:ぼかし最小距離 \
	-far	$i:ぼかし最大距離 \
	-enable // 起動時にオン
mesg 遠景ぼかし $s:名前 off[0]
mesg 遠景ぼかし $s:名前 on[1]
mesg 遠景ぼかし $s:名前 set[2] $w:補間時間 $b:補間タイプ $i:ぼかし最小距離 $i:ぼかし最大距離
// 最大距離と最小距離との間を最大描画枚数で指定した段階でぼかす
// メッセージにより途中で補間しながら影響距離を変更できる
// このときの補間時間は1/60秒単位ではなく1/300秒単位なので注意すること

＜プログラム呼び出しインターフェイス＞
	void *NewFarFocusEffect( int name, int max_plane, int near, int far );
	int		name ;		メッセージを受け取るときの名前
	int		max_plane ;	最大描画枚数
	int		near ;		ぼかし最小距離
	int		far ;		ぼかし最大距離

		機能ＯＦＦメッセージ
		message[0] = 0 ;

		機能ＯＮメッセージ
		message[0] = 1 ;

		パラメータ調節メッセージ
		message[0] = 2(130で補間時間が1/1000秒単位に) ;
		message[1] = 補間時間 ;
		message[2] = 補間タイプ ;
		message[3] = ぼかし最小距離 ;
		message[4] = ぼかし最大距離 ;

※現在指定できる補間タイプは０のみである

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
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"
#include        "codecmem.h"
#include        "codec_config.h"


#define MALLOC(size)     codecMalloc((size))
#define FREE(ptr)        codecFree((ptr))
/*
#define MALLOC(size)     GV_Malloc((size))
#define FREE(ptr)        GV_DelayedFree((ptr))
*/
#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))

#ifndef PAL
#define CLOCK_COUNT	(5)
#else
#define CLOCK_COUNT	(6)
#endif

#define DRAW_DIV	(16)		/* 描画の分割数 */

/* ---------------------------------------------------------------- */
#define MAX_DRAW	(8)		/* ２以上 */

/* ---------------------------------------------------------------- */
typedef	struct	{
  GV_ACT_EX	actor ;
  
  int			name ;
  int                   chanl;    /* 対象チャネル */
  int                   dma_flag;
  int			map ;
  int			disable ;
  int			max_plane ;
  int			target_focus_near ;
  int			target_focus_far ;
  int			focus_near ;
  int			focus_far ;
  int			interp_time ;
  int			interp_type ;
  DG_DMAPACK	*dmapack ;

  int                   ctrl[5];
} Work ;

/* ---------------------------------------------------------------- */
typedef struct _screen_draw {
	DG_GIFTAG		giftag ;
	struct _screen_draw_data{
		DG_GSREG	alpha ;
		DG_GSREG	tex0 ;
		DG_GSREG	test1 ;
		DG_GSREG	prim ;
		DG_GSREG	rgbq ;
		struct _sprt_parts{
			DG_GSREG	uv0 ;
			DG_GSREG	xyz0 ;
			DG_GSREG	uv1 ;
			DG_GSREG	xyz1 ;
		} sprt[ DRAW_DIV ];
		DG_GSREG	test2 ;
	} data ;
} SCREEN_DRAW ;

typedef struct _frame_set {
	DG_GIFTAG		giftag ;
	struct _frame_set_data {
		DG_GSREG	frame ;
	} data ;
} FRAME_SET ALIGN16;

typedef struct {
	DG_DMATAG			dmatag ;			/* ＧＩＦ接続ＤＭＡタグ */
	struct _gif_packet {

		/* 初期化パケット */
		struct _init_packet {
			DG_GIFTAG		giftag ;
			struct _init_gif_data{
				DG_GSREG	offset ;
				DG_GSREG	clamp ;
			} data ;
		} init_packet ;

		/* メイン描画パケット */
		struct _draw_packet {
			//FRAME_SET	frame_set0 ;		/* バックバッファをフレームバッファに選択 */
			SCREEN_DRAW	screen_draw0 ;		/* フレームバッファの内容をバックバッファに転送 */
			//SCREEN_DRAW	mask_draw ;			/* 特定深度以降のアルファを削除 */
			//FRAME_SET	frame_set1 ;		/* フレームバッファをメインに設定 */
			//SCREEN_DRAW	screen_draw1 ;		/* バックバッファをフレームバッファに転送 */
		} draw_packet[ MAX_DRAW ];

		/* 変更環境の復元パケット */
		struct _end_packet {
			DG_GIFTAG		giftag ;
			struct _end_gif_data{
				DG_GSREG	test ;
			} data ;
		} end_packet ;

	} gif_packet ;
	DG_DMATAG			dmatag_offset ;		/* オフセット環境復元用 */
	DG_DMATAG			dmatag_end ;		/* RETタグ */
} ALL_PACKET ALIGN16;

/* ---------------------------------------------------------------- */
static void InitDmaPacket( ALL_PACKET *packet, int which, int chanl )
{
	packet->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _gif_packet) ) ;
	packet->dmatag.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(struct _gif_packet), 0) ;
	packet->dmatag_offset.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWOFFSET) ) ;
	packet->dmatag_offset.addr = &( DG_Chanl(chanl)->draw_offset[ which ] ) ;
	packet->dmatag_offset.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_offset.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_DRAWOFFSET), 0) ;
	packet->dmatag_end.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 ) ;
	packet->dmatag_end.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_end.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
	/* 初期化パケットの初期化 */
	packet->gif_packet.init_packet.giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _init_gif_data), 1, 0, 0, 0, 1) ;
	packet->gif_packet.init_packet.giftag.regs = GS_REGS_AD ;
	packet->gif_packet.init_packet.data.offset.reg = SCE_GS_XYOFFSET_1 ;
	packet->gif_packet.init_packet.data.offset.data = 
	  SCE_GS_SET_XYOFFSET( ( ( 2048 - DRAW_WIDTH / 2 - 0 ) << 4 ), ( ( 2048 - DRAW_HEIGHT / 2 - 0 ) << 4 ) ) ;
	packet->gif_packet.init_packet.data.clamp.reg = SCE_GS_CLAMP_1 ;
	packet->gif_packet.init_packet.data.clamp.data = SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1,1,DRAW_HEIGHT-2) ;
	/* 終了パケットの初期化 */
	packet->gif_packet.end_packet.giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _end_gif_data), 1, 0, 0, 0, 1) ;
	packet->gif_packet.end_packet.giftag.regs = GS_REGS_AD ;
	packet->gif_packet.end_packet.data.test.reg = SCE_GS_TEST_1 ;
	packet->gif_packet.end_packet.data.test.data = SCE_GS_SET_TEST( 1, 7, 0, 0, 0, 0, 1, 1 ) ;
}

static void SetScreenDrawPacket( SCREEN_DRAW *screen_draw, int flag,
						 int test,
						 u_long alpha,
						 int tex_page,
						 int rgba,
						 int z, int dir )
{
	static SCREEN_DRAW	def_screen_draw = {
		.giftag = { .tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _screen_draw_data), 1, 0, 0, 0, 1),
					  .regs = GS_REGS_AD },
		.data = {
			.alpha = { .reg = SCE_GS_ALPHA_1, .data = SCE_GS_SET_ALPHA(0,1,2,1,64) },
			.tex0  = { .reg = SCE_GS_TEX0_1,  .data = SCE_GS_SET_TEX0(0,8,0,9,8,0,0,0,0,0,0,0) },
			.test1 = { .reg = SCE_GS_TEST_1,  .data = SCE_GS_SET_TEST( 1, 7, 64, 1, 0, 0, 1, 3 ) },
			.prim  = { .reg = SCE_GS_PRIM,    .data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) },
			.rgbq  = { .reg = SCE_GS_RGBAQ,   .data = SCE_GS_SET_RGBAQ(128,128,128,128,0) },
		}
	} ;
	int		i ;

	*screen_draw = def_screen_draw ;
	/* ピクセルテスト設定 */
	screen_draw->data.test1.data = test ;
	/* アルファ設定 */
	screen_draw->data.alpha.data = alpha ;
	/* テクスチャページ設定(flag = 0x4:rgba_tex) */
	screen_draw->data.tex0.data = SCE_GS_SET_TEX0( BUFFER_PAGE(tex_page) / 64 ,
							  BUFFER_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),10,10,((flag&4)!=0),0,0,0,0,0,0) ;
	/* プリミティブ属性指定（flag = 0x1:alpha disable, 0x2:tex disable） */
	screen_draw->data.prim.data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,((flag&2)==0),0,((flag&1)==0),0,1,0,0) ;
	/* 色設定 */
	screen_draw->data.rgbq.data = rgba ;
	/* 座標＆ＵＶ用レジスタ設定 */
	for ( i = 0 ; i < DRAW_DIV ; i++ ){
		screen_draw->data.sprt[ i ].uv0.reg = SCE_GS_UV ;
		screen_draw->data.sprt[ i ].uv1.reg = SCE_GS_UV ;
		screen_draw->data.sprt[ i ].xyz0.reg = SCE_GS_XYZ2 ;
		screen_draw->data.sprt[ i ].xyz1.reg = SCE_GS_XYZ2 ;
	}
	/* 描画領域＆ＵＶ座標設定 */
	{/* 描画領域が重ならないように描画方向も考慮してプリミティブ座標とＵＶ座標を設定する */
		int		x, y, w, h, u, v, uw, vh ;
		/* Ｘ軸方向のプリミティブ座標、ＵＶ座標をセットアップ */
		if ( dir & 1 ){
			x = ( 2048 - DRAW_WIDTH  / 2 ) * 16 ;
			u = 16 ;
			w =   ( DRAW_WIDTH  ) * 16 / DRAW_DIV ;
			uw =  ( DRAW_WIDTH  ) * 16 / DRAW_DIV ;
		} else {
			x = ( 2048 + DRAW_WIDTH  / 2 ) * 16 + 16 ;
			u = DRAW_WIDTH  * 16 + 16 ;
			w =  -( DRAW_WIDTH  ) * 16 / DRAW_DIV ;
			uw = -( DRAW_WIDTH  ) * 16 / DRAW_DIV ;
		}
		/* Ｙ軸方向のプリミティブ座標、ＵＶ座標をセットアップ */
		if ( dir & 2 ){
			y = ( 2048 - DRAW_HEIGHT / 2 ) * 16 ;
			v = 16 ;
			h =   ( DRAW_HEIGHT ) * 16 ;
			vh =  ( DRAW_HEIGHT ) * 16 ;
		} else {
			y = ( 2048 + DRAW_HEIGHT / 2 ) * 16 + 16 ;
			v = DRAW_HEIGHT * 16 + 16 ;
			h =  -( DRAW_HEIGHT ) * 16 ;
			vh = -( DRAW_HEIGHT ) * 16 ;
		}
		for ( i = 0 ; i < DRAW_DIV ; i++ ){
			screen_draw->data.sprt[ i ].xyz0.data = SCE_GS_SET_XYZ( x  , y  , z ) ;
			screen_draw->data.sprt[ i ].xyz1.data = SCE_GS_SET_XYZ( x+w, y+h, z ) ;
			screen_draw->data.sprt[ i ].uv0.data = SCE_GS_SET_UV( u   , v    ) ;
			screen_draw->data.sprt[ i ].uv1.data = SCE_GS_SET_UV( u+uw, v+vh ) ;
			x += w ;
			u += uw ;
		}
	}

}
static void SetFrameSettingPacket( FRAME_SET *frame_set, int page )
{
	static FRAME_SET	def_frame_set = {
		.giftag = { .tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _frame_set_data), 1, 0, 0, 0, 1), .regs = GS_REGS_AD },
		.data = {
			.frame = { .reg = SCE_GS_FRAME_1,
						 .data = SCE_GS_SET_FRAME( BUFFER_PAGE(2)/2048, BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0 ) }
		}
	};
	*frame_set = def_frame_set ;
	frame_set->data.frame.data = 
	  SCE_GS_SET_FRAME( BUFFER_PAGE(page)/2048, BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0 ) ;

}

/* ---------------------------------------------------------------- */
static int	CalcDepth( int z, int chanl )
{
	FVECTOR	view_pos ;

	view_pos.vx = 0.0f ;
	view_pos.vy = 0.0f ;
	view_pos.vz = (float)z ;
	view_pos.vw = 1.0f ;
	_sceVu0ApplyMatrix( &view_pos, &( DG_Chanl(chanl)->pers ), &view_pos );
	z = (int)( view_pos.vz / view_pos.vw * DRAW_Z_SCALE + DRAW_Z_OFFSET ) ;
	return ( z );
}

static void SetParam( Work *work, ALL_PACKET *packet, int which, int first_flag )
{
	int		i, z ;

	if ( first_flag ){
		InitDmaPacket( packet, which, work->chanl );
	}
	/* パケットの初期化 */
	for ( i = 0 ; i < MAX_DRAW ; i++ ){
		if ( first_flag ){
			//SetFrameSettingPacket( &packet->gif_packet.draw_packet[ i ].frame_set0, 2 ) ;
			//SetFrameSettingPacket( &packet->gif_packet.draw_packet[ i ].frame_set1, which ) ;
		}
		z = ( work->focus_near - work->focus_far ) * i / ( work->max_plane - 1 ) + work->focus_far ;
		if ( z < DRAW_Z_MIN ) z = DRAW_Z_MIN ;
		if ( z > DRAW_Z_MAX ) z = DRAW_Z_MAX ;

		SetScreenDrawPacket( &packet->gif_packet.draw_packet[ i ].screen_draw0,
							0x01,
							SCE_GS_SET_TEST( 0, 7, 64, 1, 0, 0, 1, 2 ),
							SCE_GS_SET_ALPHA(0,1,2,1,64),
							which,
							0x80808080,
							z, i & 3 );

		if ( i >= work->max_plane || ( z == DRAW_Z_MAX || z == DRAW_Z_MIN )){
			int		ii ;
			for ( ii = 0 ; ii < DRAW_DIV ; ii++ ){
				packet->gif_packet.draw_packet[ i ].screen_draw0.data.sprt[ ii ].xyz0.reg = SCE_GS_NOP ;
				packet->gif_packet.draw_packet[ i ].screen_draw0.data.sprt[ ii ].xyz1.reg = SCE_GS_NOP ;
			}
		}
	}
}

void ffocusSwitch(Work * work, int sw)
{
  work->ctrl[0] = (!sw) ? 0 : 1;
}

void ffocusSetFocus(Work * work, int t, int type, int near, int far)
{
  work->ctrl[0] = 2;
  work->ctrl[1] = t;
  work->ctrl[2] = type;
  work->ctrl[3] = near;
  work->ctrl[4] = far;
}



/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
  if( work->ctrl[0] == -1 ){
    if ( work->disable ) return;
  } else {
    
    /* メッセージチェック */
    switch ( work->ctrl[0] ){
    case 0:/* 停止 */
      work->disable = 1 ;
      break ;
    case 1:/* 再開 */
      work->disable = 0 ;
      break ;
    case 2:/* フォーカスセット */
    case 130:
      work->interp_time = work->ctrl[1] ;	/* 補間時間 */
      work->interp_type = work->ctrl[2] ;	/* 補間タイプ */
      work->target_focus_near = CalcDepth( work->ctrl[3], work->chanl ) ;/* フォーカス開始距離 */
      work->target_focus_far = CalcDepth( work->ctrl[4], work->chanl ) ;/* フォーカス終了距離 */

      work->disable = 0 ;
      if ( work->ctrl[0] >= 128 ){
	work->interp_time = work->ctrl[1] * 300 / 1000 ;	/* 補間時間 */
      }
      break ;
    case 3:/* プリセットフォーカス発動 */
      break ;
    }
    work->ctrl[0] = -1;
  }

  SetParam( work, work->dmapack->packet[DG_Clock], DG_Clock, 0 );

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
    work->dmapack->flag |= work->dma_flag;
  } else {
    work->dmapack->flag &= ~work->dma_flag;
  }
  
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	/* パケットメモリ開放 */
	FREE( work->dmapack->packet[0] );
	/* ＤＭＡパケットオブジェクト開放 */
	DG_DequeueDmapack( work->dmapack );
	DG_FreeDmapack( work->dmapack );
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int name, int where )
{
  DG_DMAPACK	*dmapack ;
  ALL_PACKET	*packet ;
  int		i ;
  int flag;

  work->name = name ;
  work->map = where ;
  work->interp_time = 0 ;				/* 補間時間 */
  work->interp_type = 0 ;				/* 補間タイプ */
  work->focus_near = work->target_focus_near ;
  work->focus_far = work->target_focus_far ;
  
  /* ＤＭＡパケット型オブジェクト作成 */
  if(work->chanl == 2)
    work->dma_flag = DG_DMAPACK_INVISIBLE2;
  else
    work->dma_flag = DG_DMAPACK_INVISIBLE3;

  flag = DG_DMAPACK_NORMAL| DG_DMAPACK_PRIVILEGE |
    DG_DMAPACK_INVISIBLE0| DG_DMAPACK_INVISIBLE1 |
    DG_DMAPACK_INVISIBLE2| DG_DMAPACK_INVISIBLE3;
  
  flag &= ~work->dma_flag;
  
  work->dmapack = dmapack = DG_MakeDmapack( flag, DG_DMAPACK_PHASE_AFTER );
  DG_QueueDmapack( dmapack );
  
  /* パケットメモリ割り当て */
  packet = MALLOC( sizeof(ALL_PACKET) * 2 );
  dmapack->packet[0] = &packet[0] ;
  dmapack->packet[1] = &packet[1] ;
  
  InitDmaPacket( &packet[1], 1, work->chanl );
  
  for ( i = 0 ; i < 2 ; i++ ){
    SetParam( work, &packet[i], i, 1 );
  }
  
  if ( work->disable ){
    work->dmapack->flag |= work->dma_flag;
  } else {
    work->dmapack->flag &= ~work->dma_flag;
  }
  work->ctrl[0] = -1;  /* 初期状態ではメッセージ無し */
  return (0);
}

static int GetResources_P( Work *work, int name, int chanl, int max_plane, int near, int far )
{
  work->chanl = chanl;
  work->disable = 0 ;
  work->max_plane = max_plane ;
  work->target_focus_near = CalcDepth( near, chanl ) ;
  work->target_focus_far = CalcDepth( far, chanl ) ;
  return GetResources( work, name, GM_CurrentMap );
}


/* プログラム呼び出しインターフェイス */
void *NewCodecFarFocusEffect( int name, int chanl,
			      int max_plane, int near, int far )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)codecActorPrio( ACT_MODE, sizeof( Work ), 0xf0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources_P( work, name, chanl, max_plane, near, far ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}



//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blur_timer.c
	プログラム起動＆消滅ブラー

	2000/07/11 S.Okajima
	$Id: blur_timer.c,v 1.6 2002/11/23 12:09:25 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#ifdef PSX2
#include	"utl_dma.h"
#endif
#include	"../etc/ok_util.h"
#ifdef PSX2
#include	"def_dma.h"
#endif

#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))
#define MULTIPLE_LIMIT		(1)

#define PRIORITY_PRG	(90)
#define PHASE_PRG	(DG_PLUGIN_PHASE_NORMAL)

//extern DG_DMAPACK* DG_MakeDmapack( int flag, int phase );
//extern void DG_FreeDmapack( DG_DMAPACK *dmapack );

enum {
	START=0,
	PATH,
	END
};

static int Blur_p_multiple_limit = 0;

typedef	struct	{
	GV_ACT_EX		actor ;

	DG_DMAPACK	*dmapack ;
	void		*packet_mem ;

	int			num_myself;

	int			count;
	float		intense;

	int			phase;
	float		start;
	int			time0;
	float		path;
	int			time1;
	float		end;

//	int			clock;
} Work ;

typedef struct {
	DG_DMATAG			dmatag ;
	struct _gif_packet {
		DG_GIFTAG		giftag ;
		struct _gif_data{
			DG_GSREG	alpha ;
			DG_GSREG	clamp ;
			DG_GSREG	tex0 ;
			DG_GSREG	test1 ;
			DG_GSREG	prim ;
			DG_GSREG	rgbq ;
			DG_GSREG	uv0 ;
			DG_GSREG	xyz0 ;
			DG_GSREG	uv1 ;
			DG_GSREG	xyz1 ;
			DG_GSREG	test2 ;
		} gif_data ;
	} gif_packet ;
} SCREEN_DRAW ;

#ifdef PSX2

static SCREEN_DRAW def_screen_draw = {
	/* .dmatag = */ {
		/* .qwc = */ DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_packet) ),
         NULL, // addr
		/* .vifcode = */ {
			SCE_VIF1_SET_NOP( 0 ),
			SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(struct _gif_packet), 0)
		}
	},
	/* .gif_packet = */ {
		/* .giftag = */ { /* .tag = */ SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _gif_data), 1, 0, 0, 0, 1), /* .regs = */ GS_REGS_AD },
		/* .gif_data = */ {
			/* .alpha = */ { /* .reg = */ SCE_GS_ALPHA_1, /* .data = */ SCE_GS_SET_ALPHA(0,1,0,1,64) },
			/* .clamp = */ { /* .reg = */ SCE_GS_CLAMP_1, /* .data = */ SCE_GS_SET_CLAMP(2,2,0,DRAW_WIDTH-1,1,DRAW_HEIGHT-2) },
			/* .tex0  = */ { /* .reg = */ SCE_GS_TEX0_1,  /* .data = */ SCE_GS_SET_TEX0(0,8,0,9,8,0,0,0,0,0,0,0) },
			/* .test1 = */ { /* .reg = */ SCE_GS_TEST_1,  /* .data = */ SCE_GS_SET_TEST(0,0,0,0,0,0,1,1) },
			/* .prim  = */ { /* .reg = */ SCE_GS_PRIM,    /* .data = */ SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) },
			/* .rgbq  = */ { /* .reg = */ SCE_GS_RGBAQ,   /* .data = */ SCE_GS_SET_RGBAQ(128,128,128,64,0) },
			/* .uv0   = */ { /* .reg = */ SCE_GS_UV,      /* .data = */ SCE_GS_SET_UV(0+8,0+8) },
			/* .xyz0  = */ { /* .reg = */ SCE_GS_XYZ2,
				/* .data = */ SCE_GS_SET_XYZ( ((2048-DRAW_WIDTH/2)*16+8),
									   ((2048-DRAW_HEIGHT/2)*16+8),
									   0xffffffff) },
			/* .uv1   = */ { /* .reg = */ SCE_GS_UV,      /* .data = */ SCE_GS_SET_UV(DRAW_WIDTH*16+16-8,DRAW_HEIGHT*16+16-8)},
			/* .xyz1  = */ { /* .reg = */ SCE_GS_XYZ2,
				/* .data = */ SCE_GS_SET_XYZ( ((2048+DRAW_WIDTH/2)*16-8),
									   ((2048+DRAW_HEIGHT/2)*16-8),
									   0xffffffff) },
			/* .test2 = */ { /* .reg = */ SCE_GS_TEST_1,  /* .data = */ SCE_GS_SET_TEST( 1, 5, 64, 1, 0, 0, 1, 1 ) },
		}
	}
};

#else

static SCREEN_DRAW def_screen_draw;

#endif


static void Act( Work *work )
{
	SCREEN_DRAW	*packet ;
	float	ratio;

	// 天狗兵の偽ゲームオーバー専用
	if( GM_BlurDisableFlag != 0 ){
		GV_DestroyActor( work ) ;
		return;
	}


	work->count--;
	switch( work->phase ){
	  case START:
		ratio = (float)work->count / (float)work->time0;
		work->intense = work->start * ratio + work->path * (1.0f - ratio);
		if( work->count <= 0 ){
			work->phase = PATH;
			work->count = work->time1;
		}
		break;
	  case PATH:
		ratio = (float)work->count / (float)work->time1;
		work->intense = work->path * ratio + work->end * (1.0f - ratio);
		if( work->count <= 0 ){
			work->phase = END;
		}
		break;
	  case END:
	  default:
		GV_DestroyActor( work ) ;
		break;
	}

	if( work->intense > 128.0f ){
		work->intense = 128.0f;
	}else if( work->intense < 0.0f ){
		work->intense = 0.0f;
	}


//printf("%f\n",work->intense);

#ifdef PSX2 /*yano 2002.04.02*/
//	work->clock = 1 - work->clock;
	packet = work->dmapack->packet[DG_Clock];
	packet->gif_packet.gif_data.alpha.data = 
	      SCE_GS_SET_ALPHA(0,1,2,1, (int)work->intense ) ;
#else

	{
		void			*prim ;
		unsigned int	col ;
		col = 0x40808080 ;
		prim = work->dmapack->autopacket ;
		prim = DG_SetDmapackUseFrameTex( prim, 1 );	/* フレームバッファをテクスチャとして使用する宣言 */
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0,1,2,1, (int)work->intense ) );
		/* ＵＶ値はDG_FRAME_V(),DG_FRAME_V()マクロを使用して指定すること */
		BP_RENDER_TODO_BREAK;
#ifndef KP_WINDOWS
		prim = DG_SetDmapackSprt( prim,
								 0-4  , 0-4  , DG_FRAME_U(0.5f), DG_FRAME_V(0.5f),
								 640+4, 448+4, DG_FRAME_U(641.5f), DG_FRAME_V(449.5f), col );
#else
		prim = DG_SetDmapackSprt( prim,
								 0-4  , 0-4  , DG_FRAME_U(0.5f), DG_FRAME_V(0.5f),
								 DRAW_WIDTH+4, DRAW_HEIGHT+4,
								 DG_FRAME_U((float)DRAW_WIDTH+1.5f),
								 DG_FRAME_V((float)DRAW_HEIGHT+1.5f), col );
#endif
		prim = DG_SetDmapackEnd( prim );
	}
#endif
}

static void Die( Work *work )
{
	/* パケットメモリ開放 */
	GV_DelayedFree( work->packet_mem );
	/* ＤＭＡパケットオブジェクト開放 */
	DG_DequeueDmapack( work->dmapack );
	DG_FreeDmapack( work->dmapack );
	Blur_p_multiple_limit--;
}

#ifdef KP_WINDOWS
static void DmyAct( Work *work ){ GV_DestroyActor( work ) ; }
static void DmyDie( Work *work ){}
#endif

static int GetResources( Work *work )
{
	DG_DMAPACK	*dmapack ;
	SCREEN_DRAW	*packet ;
	int		i;

//	work->clock = 0;

	/* ＤＭＡパケット型オブジェクト作成 */
	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, PHASE_PRG, PRIORITY_PRG );
	DG_QueueDmapack( dmapack );

#ifdef PSX2
	/* パケットメモリ割り当て */
	packet = work->packet_mem = GV_Malloc( sizeof(SCREEN_DRAW) * 2 );
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;

	/* パケットの初期化 */
	for ( i = 0 ; i < 2 ; i++ ){
		packet[i] = def_screen_draw ;
		packet[i].gif_packet.gif_data.tex0.data = SCE_GS_SET_TEX0(
													   BUFFER_PAGE(1-i) / 64 ,
													   BUFFER_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),10,10,0,0,0,0,0,0,0);

		packet[i].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(0,1,2,1, 0 ) ;
		packet[i].gif_packet.gif_data.xyz0.data = 
		  SCE_GS_SET_XYZ( ((2048-DRAW_WIDTH/2)*16+8-64),
						 ((2048-DRAW_HEIGHT/2)*16+8-64),
						 0xffffffff) ;
		packet[i].gif_packet.gif_data.xyz1.data = 
		  SCE_GS_SET_XYZ( ((2048+DRAW_WIDTH/2)*16-8+64),
						 ((2048+DRAW_HEIGHT/2)*16-8+64),
						 0xffffffff) ;
	}
#else
	if( !(work->packet_mem = GV_Malloc( 256 )) ){
		return -1 ;
	}
	dmapack->autopacket = work->packet_mem;
	DG_SetDmapackEnd( work->packet_mem );
#endif

	work->intense = work->start;
	work->count = work->time0;
	work->phase = START;


	return (0);
}

void *NewBlurProgTimer(
                        float start,	//最初の強さ（この強さで突然始まる 0~128）
                        int   time0,	//最初から途中までの変化時間(フレーム)
                        float path,		//途中の強さ(0~128)
                        int   time1,	//途中から最後までの変化時間(フレーム)
                        float end		//最後の強さ（この強さで突然終了する 0~128）
                      )
{
	Work		*work ;

	OPERATOR() ;

	if( Blur_p_multiple_limit >= MULTIPLE_LIMIT ) return NULL ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		Blur_p_multiple_limit++;

		work->start = start;
		work->time0 = time0;
		work->path  = path;
		work->time1 = time1;
		work->end   = end;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
#else
		if ( DG_CheckUseBlur() ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )

			Blur_p_multiple_limit++;

			work->start = start;
			work->time0 = time0;
			work->path  = path;
			work->time1 = time1;
			work->end   = end;

			if ( GetResources( work ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		} else {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )
		}
#endif
	}
	return (void *)work ;
}

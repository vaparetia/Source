//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	bokashi.c
	DMAブラー

	2000/03/14 S.Okajima
	$Id: bokashi.c,v 1.1.1.3 2002/11/19 11:47:00 Yoshizawa1 Exp $
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


#define N_PRIMS	(1)
#define N_PACKETS	(2)

#define BASE_RGB	(32)

//extern DG_DMAPACK* DG_MakeDmapack( int flag, int phase );
//extern void DG_FreeDmapack( DG_DMAPACK *dmapack );


enum{
	PHASE_ATTACK = 0,
	PHASE_CONT,
	PHASE_DECAY,
	PHASE_END
};

typedef	struct	{
	GV_ACT_EX		actor ;

	DG_DMAPACK	*dmapack ;
	void		*packet_mem ;

	int phase;
	int intense_max;
	int intense;
	int attack_max;
	int attack;
	int cont;
	int decay_max;
	int decay;
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

#if 0 //BP_GCC
static SCREEN_DRAW def_screen_draw = {
	.dmatag = {
		.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_packet) ),
		.vifcode = {
			SCE_VIF1_SET_NOP( 0 ),
			SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(struct _gif_packet), 0)
		}
	},
	.gif_packet = {
		.giftag = { .tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _gif_data), 1, 0, 0, 0, 1), .regs = GS_REGS_AD },
		.gif_data = {
			.alpha = { .reg = SCE_GS_ALPHA_1, .data = SCE_GS_SET_ALPHA(0,2,2,1,128) },
			.clamp = { .reg = SCE_GS_CLAMP_1, .data = SCE_GS_SET_CLAMP(2,2,0,DRAW_WIDTH-1,1,DRAW_HEIGHT-2) },
			.tex0  = { .reg = SCE_GS_TEX0_1,  .data = SCE_GS_SET_TEX0(0,8,0,9,8,0,0,0,0,0,0,0) },
			.test1 = { .reg = SCE_GS_TEST_1,  .data = SCE_GS_SET_TEST(0,0,0,0,0,0,1,1) },
			.prim  = { .reg = SCE_GS_PRIM,    .data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) },
			.rgbq  = { .reg = SCE_GS_RGBAQ,   .data = SCE_GS_SET_RGBAQ(128,128,128,32,0) },
			.uv0   = { .reg = SCE_GS_UV,      .data = SCE_GS_SET_UV(0+8,0+8) },
			.xyz0  = { .reg = SCE_GS_XYZ2,
				.data = SCE_GS_SET_XYZ( ((2048-DRAW_WIDTH/2)*16+8),
									   ((2048-DRAW_HEIGHT/2)*16+8),
									   0xffffffff) },
			.uv1   = { .reg = SCE_GS_UV,      .data = SCE_GS_SET_UV(DRAW_WIDTH*16+16-8,DRAW_HEIGHT*16+16-8)},
			.xyz1  = { .reg = SCE_GS_XYZ2,
				.data = SCE_GS_SET_XYZ( ((2048+DRAW_WIDTH/2)*16-8),
									   ((2048+DRAW_HEIGHT/2)*16-8),
									   0xffffffff) },
			.test2 = { .reg = SCE_GS_TEST_1,  .data = SCE_GS_SET_TEST( 1, 5, 64, 1, 0, 0, 1, 1 ) },
		}
	}
};
#endif //BP

#else

static SCREEN_DRAW def_screen_draw;

#endif


static void Act( Work *work )
{
	SCREEN_DRAW	*packet ;


	if( work->phase == PHASE_ATTACK){
		work->attack--;
		if( work->attack > 0 ){
			work->intense = work->intense_max * ( work->attack_max - work->attack ) / work->attack_max;
		}else{
			work->phase = PHASE_CONT;
		}
	}
	if( work->phase == PHASE_CONT){
		work->cont--;
		if( work->cont <= 0 ) work->phase = PHASE_DECAY;
	}
	if( work->phase == PHASE_DECAY){
		work->decay--;
		if( work->decay > 0 ){
			work->intense = work->intense_max * work->decay / work->decay_max;
		}else{
			work->phase = PHASE_END;
		}
	}
	if( work->phase == PHASE_END){
		work->intense = 0;
		GV_DestroyActor( work ) ;
	}

	packet = work->dmapack->packet[DG_Clock];
	packet->gif_packet.gif_data.rgbq.data = SCE_GS_SET_RGBAQ(BASE_RGB, BASE_RGB, BASE_RGB, work->intense, 0) ;


}

static void Die( Work *work )
{
	/* パケットメモリ開放 */
	GV_DelayedFree( work->packet_mem );
	/* ＤＭＡパケットオブジェクト開放 */
	DG_DequeueDmapack( work->dmapack );
	DG_FreeDmapack( work->dmapack );
}

static int GetResources( Work *work, int intense, int attack, int cont, int decay )
{
	DG_DMAPACK	*dmapack ;
	SCREEN_DRAW	*packet ;
	int		i ;

	work->phase    = PHASE_ATTACK;
	work->intense  = work->intense_max = intense;
	work->attack   = work->attack_max  = attack;
	work->cont     = cont;
	work->decay    = work->decay_max   = decay;


	/* ＤＭＡパケット型オブジェクト作成 */
	work->dmapack = dmapack = DG_MakeDmapack( DG_DMAPACK_NORMAL, DG_DMAPACK_PHASE_AFTER );
	DG_QueueDmapack( dmapack );

	/* パケットメモリ割り当て */
	packet = work->packet_mem = GV_Malloc( sizeof(SCREEN_DRAW) * 2 );
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;

	/* パケットの初期化 */
	for ( i = 0 ; i < 2 ; i++ ){
      BP_TODO_BREAK;
#if 0 //BP_GCC
		packet[i] = def_screen_draw ;
#endif
		packet[i].gif_packet.gif_data.tex0.data = SCE_GS_SET_TEX0(
								   BUFFER_PAGE(1-i) / 64 ,
								   BUFFER_WIDTH/64,
								   FRAME_BUFFER_COLOR_MODE(),
								   10,
								   10,
								   0,
								   0,
								   0,
								   0,
								   0,
								   0,
								   0);


		packet[i].gif_packet.gif_data.rgbq.data = SCE_GS_SET_RGBAQ(BASE_RGB, BASE_RGB, BASE_RGB ,work->intense ,0 ) ;
//		packet[i].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(0,1,2,1,16) ;
		packet[i].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(0,2,1,1,4) ;
		packet[i].gif_packet.gif_data.uv0.data = SCE_GS_SET_UV(0-8*0,0-8*0) ;
		packet[i].gif_packet.gif_data.uv1.data = SCE_GS_SET_UV(DRAW_WIDTH*16+16-8*0,DRAW_HEIGHT*16+16-8*0) ;
		packet[i].gif_packet.gif_data.xyz0.data = 
		  SCE_GS_SET_XYZ( ((2048-DRAW_WIDTH/2)*16+8- 128  ),
						 ((2048-DRAW_HEIGHT/2)*16+8- 128 ),
						 0xffffffff) ;
		packet[i].gif_packet.gif_data.xyz1.data = 
		  SCE_GS_SET_XYZ( ((2048+DRAW_WIDTH/2)*16-8+ 128  ),
						 ((2048+DRAW_HEIGHT/2)*16-8+ 128 ),
						 0xffffffff) ;

	}

	return (0);
}


void *NewDmaBlur( int intense,  /* 強さ 0-255 */
                  int attack,   /* 立ち上げ フェーズフレーム長 */
                  int cont,     /* 持続     フェーズフレーム長 */
                  int decay     /* 減衰     フェーズフレーム長 */
                )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, intense, attack, cont, decay ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

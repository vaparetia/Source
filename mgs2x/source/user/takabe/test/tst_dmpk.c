/*
	tst_dmpk.c
	ＤＧ＿ＤＭＡＰＡＣＫ使用サンプル（ブラーエフェクト）

	2000/03/01 K.Takabe
	$Id: tst_dmpk.c,v 1.1.1.3 2002/11/19 11:51:31 Yoshizawa1 Exp $

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

#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))


#define N_PRIMS	(1)
#define N_PACKETS	(2)

typedef	struct	{
	GV_ACT		actor ;

	int			name ;
	int			map ;
	DG_DMAPACK	*dmapack ;
	void		*packet_mem ;

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
			.alpha = { .reg = SCE_GS_ALPHA_1, .data = SCE_GS_SET_ALPHA(0,1,0,1,64) },
			.clamp = { .reg = SCE_GS_CLAMP_1, .data = SCE_GS_SET_CLAMP(1,0,0,0,0,0) },
			.tex0  = { .reg = SCE_GS_TEX0_1,  .data = SCE_GS_SET_TEX0(0,8,0,9,8,0,0,0,0,0,0,0) },
			.test1 = { .reg = SCE_GS_TEST_1,  .data = SCE_GS_SET_TEST(0,0,0,0,0,0,1,1) },
			.prim  = { .reg = SCE_GS_PRIM,    .data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) },
			.rgbq  = { .reg = SCE_GS_RGBAQ,   .data = SCE_GS_SET_RGBAQ(128,128,128,64,0) },
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

static void Act( Work *work )
{
//	int		n_msg ;
//	GV_MSG	*msg ;

//	if( ( n_msg = GV_ReceiveMessage( 0, &msg ) ) == 0 ){
//		GV_WaitMessage( work, 0 );
//		return;
//	}

	if ( GV_PadData[ 1 ].status & PAD_A ){
		work->dmapack->flag |= DG_DMAPACK_INVISIBLE0 ;
	} else {
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE0 ;
	}
}

static void Die( Work *work )
{
	/* パケットメモリ開放 */
	GV_DelayedFree( work->packet_mem );
	/* ＤＭＡパケットオブジェクト開放 */
	DG_DequeueDmapack( work->dmapack );
	DG_FreeDmapack( work->dmapack );
}

static int GetResources( Work *work, int name, int where )
{
	DG_DMAPACK	*dmapack ;
	SCREEN_DRAW	*packet ;
	int		i ;

	work->name = name ;
	work->map = where ;

	/* ＤＭＡパケット型オブジェクト作成 */
	work->dmapack = dmapack = DG_MakeDmapack( DG_DMAPACK_NORMAL, DG_DMAPACK_PHASE_AFTER );
	DG_QueueDmapack( dmapack );

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
#if 1
		/* バウンサーエフェクトモドキ */
		packet[i].gif_packet.gif_data.rgbq.data = SCE_GS_SET_RGBAQ(96,96,96,128,0) ;
		packet[i].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(0,2,2,1,128) ;
		packet[i].gif_packet.gif_data.uv0.data = SCE_GS_SET_UV(0-8*0,0-8*0) ;
		packet[i].gif_packet.gif_data.uv1.data = SCE_GS_SET_UV(DRAW_WIDTH*16+16-8*0,DRAW_HEIGHT*16+16-8*0) ;
		packet[i].gif_packet.gif_data.xyz0.data = 
		  SCE_GS_SET_XYZ( ((2048-DRAW_WIDTH/2)*16+8-32),
						 ((2048-DRAW_HEIGHT/2)*16+8-64),
						 0xffffffff) ;
		packet[i].gif_packet.gif_data.xyz1.data = 
		  SCE_GS_SET_XYZ( ((2048+DRAW_WIDTH/2)*16-8+32),
						 ((2048+DRAW_HEIGHT/2)*16-8+64),
						 0xffffffff) ;
#endif
	}

	return (0);
}


void *NewDmapackTestProgram( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

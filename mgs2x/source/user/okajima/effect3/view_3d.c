//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	view_3d.c
	立体視

	2001/06/14 S.Okajima
	$Id: view_3d.c,v 1.1.1.3 2002/11/19 11:47:39 Yoshizawa1 Exp $
*/
/* このプログラムは未使用 2002.04.02yano */

#ifdef PSX2 ///
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
#include	"def_dma.h"
#endif


#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"




#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))

#define N_PRIMS	(1)
#define N_PACKETS	(2)

#define PRIORITY_PRG_CONT0	(95)
#define PRIORITY_PRG_CONT1	(96)
#define PRIORITY_PRG_CONT2	(97)
#define PHASE_PRG	(DG_PLUGIN_PHASE_NORMAL)

#define PRIORITY_SCN	(250)
#define PHASE_PRG	(DG_PLUGIN_PHASE_NORMAL)
#define PHASE_SCN	(DG_PLUGIN_PHASE_LAST)

#define PRIORITY_PRG	(100)

extern void GM_SetCameraAdjust( int	chanl, FVECTOR	*adj );
extern int DM_FrameSkip ;

typedef	struct	{
	GV_ACT_EX		actor ;

	int			name;

	int			col_r;
	int			col_b;

	int			clock;
	float		cam_shift;

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
			.alpha = { .reg = SCE_GS_ALPHA_1, .data = SCE_GS_SET_ALPHA(2,0,2,1,128) },
			.clamp = { .reg = SCE_GS_CLAMP_1, .data = SCE_GS_SET_CLAMP(2,2,0,DRAW_WIDTH-1,1,DRAW_HEIGHT-2) },
			.tex0  = { .reg = SCE_GS_TEX0_1,  .data = SCE_GS_SET_TEX0(0,8,0,9,8,0,0,0,0,0,0,0) },
			.test1 = { .reg = SCE_GS_TEST_1,  .data = SCE_GS_SET_TEST(0,0,0,0,0,0,1,1) },
			.prim  = { .reg = SCE_GS_PRIM,    .data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,1,0,0) },
			.rgbq  = { .reg = SCE_GS_RGBAQ,   .data = SCE_GS_SET_RGBAQ(255,255,255,64,0) },
			.uv0   = { .reg = SCE_GS_UV,      .data = SCE_GS_SET_UV(0+8,0+8) },
			.xyz0  = { .reg = SCE_GS_XYZ2,
				.data = SCE_GS_SET_XYZ( ((2048-DRAW_WIDTH/2)*16),
									   ((2048-DRAW_HEIGHT/2)*16),
									   0xffffffff) },
			.uv1   = { .reg = SCE_GS_UV,      .data = SCE_GS_SET_UV(DRAW_WIDTH*16+16-8,DRAW_HEIGHT*16+16-8)},
			.xyz1  = { .reg = SCE_GS_XYZ2,
				.data = SCE_GS_SET_XYZ( ((2048+DRAW_WIDTH/2)*16),
									   ((2048+DRAW_HEIGHT/2)*16),
									   0xffffffff) },
			.test2 = { .reg = SCE_GS_TEST_1,  .data = SCE_GS_SET_TEST( 1, 5, 64, 1, 0, 0, 1, 1 ) },
		}
	}
};
#endif //BP

#else
static SCREEN_DRAW def_screen_draw ;
#endif


static void Act( Work *work )
{
	SCREEN_DRAW	*packet ;
	FVECTOR	fvtemp;

	if( GV_PadData[ 0 ].status & PAD_X ){
		work->cam_shift += 10.0f;
	}
	if( GV_PadData[ 0 ].status & PAD_Y ){
		work->cam_shift -= 10.0f;
	}

	if( work->clock ){
		fvtemp.vx = work->cam_shift;
	}else{
		fvtemp.vx =-work->cam_shift;
	}
	fvtemp.vy = 0.0f;
	fvtemp.vz = 0.0f;
	fvtemp.vw = 1.0f;
	GM_SetCameraAdjust( 0, &fvtemp );


#if 1
//printf("DG_Clock:%d\n",DG_Clock);

	packet = work->packet_mem;
	if( work->clock ){
		packet[DG_Clock].gif_packet.gif_data.rgbq.data = SCE_GS_SET_RGBAQ( 32,255,255,128,0);
//		work->dmapack->flag      |= DG_DMAPACK_INVISIBLEMENU ;
	}else{
		packet[DG_Clock].gif_packet.gif_data.rgbq.data = SCE_GS_SET_RGBAQ(255,255,  0,128,0);
//		work->dmapack->flag      &= ~DG_DMAPACK_INVISIBLEMENU ;
	}

/*
	packet = work->dmapack->packet[DG_Clock];
	packet->gif_packet.gif_data.alpha.data = 
	         SCE_GS_SET_ALPHA(0,1,2,1, 128 ) ;
*/

#if 0
#if 0
	work->dmapack->flag      |= DG_DMAPACK_INVISIBLEMENU ;
#else
	work->dmapack->flag      &= ~DG_DMAPACK_INVISIBLEMENU ;
#endif
#endif

#endif



	if( GV_PauseLevel == 0 ) work->clock = 1 - work->clock;



}

static void Die( Work *work )
{
	/* パケットメモリ開放 */
	GV_DelayedFree( work->packet_mem );

	/* ＤＭＡパケットオブジェクト開放 */
	DG_DequeueDmapack( work->dmapack );
	DG_FreeDmapack( work->dmapack );
}



static int GetResources( Work *work )
{
	DG_DMAPACK	*dmapack ;
	SCREEN_DRAW	*packet ;

	work->clock = 0;
	work->cam_shift = 0.0f;

	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, PHASE_PRG, PRIORITY_PRG );
	if( dmapack == NULL ) return (-1);
	DG_QueueDmapack( dmapack );
	work->packet_mem = packet = GV_Malloc( sizeof(SCREEN_DRAW) * 2 );
	if( packet == NULL ) return (-1);

	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;
   BP_TODO_BREAK;
#if 0 //BP_GCC
	packet[0] = def_screen_draw ;
	packet[1] = def_screen_draw ;
#endif
//	packet[0].gif_packet.gif_data.rgbq.data = SCE_GS_SET_RGBAQ(  0,255,255,128,0);
//	packet[1].gif_packet.gif_data.rgbq.data = SCE_GS_SET_RGBAQ(255,255,  0,128,0);

	return (0);
}

void *NewView3d( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
//	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		work->name  = name;
//		work->where = where;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

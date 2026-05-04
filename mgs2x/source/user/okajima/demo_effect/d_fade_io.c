//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_fade_io.c
	フェードイン・アウト・シナリオ起動のみ（プライオリティ高い）

	2000/03/23 S.Okajima
	$Id: d_fade_io.c,v 1.5 2002/12/05 18:42:03 takaki Exp $
*/
//dma DMA使用　放置-->XBOX対応 2002.03.18 yano
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

#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))

#define N_PRIMS	(1)
#define N_PACKETS	(2)

#define PRIORITY_PRG	(96)
#define PRIORITY_SCN	(250)
#define PHASE_PRG	(DG_PLUGIN_PHASE_NORMAL)
#define PHASE_SCN	(DG_PLUGIN_PHASE_LAST)

extern int DM_FrameSkip ;

typedef	struct	d_fade_io_Work{
	GV_ACT_EX		actor ;

	int			col_r_dest_fade;
	int			col_r_fade;
	int			col_g_dest_fade;
	int			col_g_fade;
	int			col_b_dest_fade;
	int			col_b_fade;
	int			col_a_dest_fade;
	int			col_a_fade;
	int			time_fade;
	int			time_max_fade;
	DG_DMAPACK	*dmapack_fade ;
	void		*packet_mem_fade ;

	int			keep_r_fade;
	int			keep_g_fade;
	int			keep_b_fade;
	int			keep_a_fade;

	int			proc_id;

	int			priority_sel;
	int			priority_sel_switch;
} Work ;

static Work *OK_FADE_IO_WORK = NULL;

typedef struct d_fade_io_SCREEN_DRAW{
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

#if 0 //BP_PS2 def PSX2
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
			.alpha = { .reg = SCE_GS_ALPHA_1, .data = SCE_GS_SET_ALPHA(0,2,0,1,0) },
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
#else

static SCREEN_DRAW def_screen_draw ;

#endif

#define BOTTOM_LIMIT (-4)
static void Act( Work *work )
{
	SCREEN_DRAW	*packet ;
	int	itemp;

	if( work->time_fade > BOTTOM_LIMIT ){
		work->time_fade -= DM_FrameSkip ;
		work->time_fade--;
		if( work->time_fade<= BOTTOM_LIMIT ){
			work->time_fade = BOTTOM_LIMIT;
		}
	}else{
		work->time_fade = BOTTOM_LIMIT;
	}
	itemp = work->time_fade;
	if( itemp < 0 ) itemp = 0;
	work->keep_r_fade = work->col_r_dest_fade + (work->col_r_fade - work->col_r_dest_fade) * itemp / work->time_max_fade;
	work->keep_g_fade = work->col_g_dest_fade + (work->col_g_fade - work->col_g_dest_fade) * itemp / work->time_max_fade;
	work->keep_b_fade = work->col_b_dest_fade + (work->col_b_fade - work->col_b_dest_fade) * itemp / work->time_max_fade;
	work->keep_a_fade = work->col_a_dest_fade + (work->col_a_fade - work->col_a_dest_fade) * itemp / work->time_max_fade;

#if 0 //BP_PS2 def PSX2 /*yano 2002.03.18*/
	packet = work->packet_mem_fade;
	packet[DG_Clock].gif_packet.gif_data.rgbq.data = SCE_GS_SET_RGBAQ( work->keep_r_fade, work->keep_g_fade, work->keep_b_fade, work->keep_a_fade, 0);
#else
	{
		void			*prim ;
		unsigned int	col ;
		col = 	((u_long64)(work->keep_r_fade)| ((u_long64)(work->keep_g_fade) << 8) | ((u_long64)(work->keep_b_fade) << 16) |  
				 ((u_long64)(work->keep_a_fade) << 24 ));
		prim = work->dmapack_fade->autopacket ;
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0,1,0,1,0) );
#ifndef KP_WINDOWS
		prim = DG_SetDmapackBox( prim, 0, 0, 640, 448, DG_MakeDmaPackColorFromInt(col) );
#else
		prim = DG_SetDmapackBox( prim, 0, 0, DRAW_WIDTH, DRAW_HEIGHT, col );
#endif
		prim = DG_SetDmapackEnd( prim );
	}
#endif
	if( work->keep_a_fade == 0 ){
		work->dmapack_fade->flag |= DG_DMAPACK_INVISIBLEMENU ;
	}else{
		work->dmapack_fade->flag &= ~DG_DMAPACK_INVISIBLEMENU ;
	}

	if( work->time_fade==BOTTOM_LIMIT  &&  work->proc_id !=-1 ){
		int proc = work->proc_id;	// exec中に破壊されるのを防ぐ
		work->proc_id = -1;
		GCL_ExecProc( proc, NULL );
	}

}

static void Die( Work *work )
{
	/* パケットメモリ開放 */
	GV_DelayedFree( work->packet_mem_fade );

	/* ＤＭＡパケットオブジェクト開放 */
	if( work->dmapack_fade!=NULL ){
		DG_DequeueDmapack( work->dmapack_fade );
		DG_FreeDmapack( work->dmapack_fade );
	}

	OK_FADE_IO_WORK = NULL;

	printf("NewFadeInOut_Scn():Die\n") ;
}



static int Initialize( Work *work )
{
	DG_DMAPACK	*dmapack ;
	SCREEN_DRAW	*packet ;

	if( !work->priority_sel ){
		work->dmapack_fade = dmapack = (DG_DMAPACK *)DG_MakeDmapack2( DG_DMAPACK_MENU, PHASE_SCN, PRIORITY_SCN );
	}else{
		work->dmapack_fade = dmapack = (DG_DMAPACK *)DG_MakeDmapack2( DG_DMAPACK_MENU, PHASE_PRG, PRIORITY_PRG );
	}

	if( dmapack == NULL ) return (-1);

	work->dmapack_fade->flag |= DG_DMAPACK_INVISIBLEMENU ;

	DG_QueueDmapack( dmapack );
#if 0 //BP_PS2 def PSX2 /*yano 2002.03.18*/
 	work->packet_mem_fade = packet = GV_Malloc( sizeof(SCREEN_DRAW) * 2 );
	if( packet == NULL ) return (-1);
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;
	packet[0] = def_screen_draw ;
	packet[1] = def_screen_draw ;
	packet[0].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(0,1,0,1,0);
	packet[1].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(0,1,0,1,0);
#else
	{
		void *last;
		if ( !(work->packet_mem_fade = GV_Malloc( 256 )) ) {
			return -1 ;
		}
		dmapack->autopacket = work->packet_mem_fade ;
		last = work->packet_mem_fade;
		DG_SetDmapackEnd( last );
	}
#endif


	work->col_r_dest_fade = 0;
	work->col_g_dest_fade = 0;
	work->col_b_dest_fade = 0;
	work->col_a_dest_fade = 0;

	work->time_fade       = BOTTOM_LIMIT;
	work->time_max_fade   = 1;

	return (0);
}

static void SetData( Work *work, int col_r, int col_g, int col_b, int col_a, int time )
{

	time = DIRECT_TICK( time ) ;
	if( time <= 0 ) time = 1;

	work->col_r_fade = work->keep_r_fade;
	work->col_g_fade = work->keep_g_fade;
	work->col_b_fade = work->keep_b_fade;
	work->col_a_fade = work->keep_a_fade;

	work->col_r_dest_fade = col_r;
	if( work->col_r_dest_fade > 255 ) work->col_r_dest_fade = 255;
	work->col_g_dest_fade = col_g;
	if( work->col_g_dest_fade > 255 ) work->col_g_dest_fade = 255;
	work->col_b_dest_fade = col_b;
	if( work->col_b_dest_fade > 255 ) work->col_b_dest_fade = 255;
	work->col_a_dest_fade = col_a;
	if( work->col_a_dest_fade > 255 ) work->col_a_dest_fade = 255;

	work->time_fade      = time;
	if(work->time_fade     <  0) work->time_fade     = 0;
	work->time_max_fade  = time;
	if(work->time_max_fade <= 0) work->time_max_fade = 1;

}

static int GetResources( Work *work, int col_r, int col_g, int col_b, int col_a, int time )
{
	SetData( work, col_r, col_g, col_b, col_a, time );
	return (0);
}

/* フェードインアウトシナリオ */
void *NewFadeInOut_Scn( int name, int where )
{
	Work		*work=NULL ;
	int			col_r;
	int			col_g;
	int			col_b;
	int			col_a;
	int			time;

	printf("NewFadeInOut_Scn()\n") ;
	OPERATOR() ;

	col_r = 0;
	col_g = 0;
	col_b = 0;
	col_a = 0;
	if ( GCL_GetOption( 'c' ) != NULL ){
		col_r = GCL_GetNextInt();
		col_g = GCL_GetNextInt();
		col_b = GCL_GetNextInt();
		col_a = GCL_GetNextInt();
	}

	time = 1;
	if ( GCL_GetOption( 't' ) != NULL ){
		time = GCL_GetNextInt();
	}


	if( OK_FADE_IO_WORK == NULL ){

//		work = OK_FADE_IO_WORK = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
		work = OK_FADE_IO_WORK = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
		// メニュー中でもフェードするように

		if ( work != NULL ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )


			work->priority_sel = 0;
			if ( GCL_GetOption( 's' ) != NULL ){
				work->priority_sel = 1;
			}
			work->priority_sel_switch = work->priority_sel;

			if( Initialize( work ) < 0 ){
				GV_DestroyActor( work ) ;
				return NULL ;
			}

			if ( GCL_GetOption( 'i' ) != NULL ){
				work->keep_r_fade = GCL_GetNextInt();
				work->keep_g_fade = GCL_GetNextInt();
				work->keep_b_fade = GCL_GetNextInt();
				work->keep_a_fade = GCL_GetNextInt();
			}

			work->proc_id = -1;
			if ( GCL_GetOption( 'p' ) != NULL ){
				work->proc_id = GCL_GetNextInt();
			}


			if ( GetResources( work, col_r, col_g, col_b, col_a, time ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		}
		return (void *)work ;
	}else{
		work = OK_FADE_IO_WORK;

		work->priority_sel = 0;
		if ( GCL_GetOption( 's' ) != NULL ){
			work->priority_sel = 1;
		}
		if( work->priority_sel_switch!= work->priority_sel ){
			work->priority_sel_switch = work->priority_sel;
			/* ＤＭＡパケットオブジェクト開放 */
			if( work->dmapack_fade!=NULL ){
				DG_DequeueDmapack( work->dmapack_fade );
				DG_FreeDmapack( work->dmapack_fade );
				work->dmapack_fade = NULL;
			}
			/* パケットメモリ開放 */
			if( work->packet_mem_fade!=NULL ){
				GV_DelayedFree( work->packet_mem_fade );
				work->packet_mem_fade = NULL;
			}
			if( Initialize( work ) < 0 ){
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		}

		work->proc_id = -1;
		if ( GCL_GetOption( 'p' ) != NULL ){
			work->proc_id = GCL_GetNextInt();
		}

		if ( GCL_GetOption( 'i' ) != NULL ){
			work->keep_r_fade = GCL_GetNextInt();
			work->keep_g_fade = GCL_GetNextInt();
			work->keep_b_fade = GCL_GetNextInt();
			work->keep_a_fade = GCL_GetNextInt();
		}
		SetData( work, col_r, col_g, col_b, col_a, time );
		return (void *)work ;
	}
}

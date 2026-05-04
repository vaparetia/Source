/*
        c_d_fd_io.c
	無線専用 フェードイン・アウト

	2000/03/23 S.Okajima
	2000/08/01 Y.Kira (Modify)
	$Id: c_d_fd_io.c,v 1.1.1.3 2002/11/19 11:44:56 Yoshizawa1 Exp $
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

extern DG_DMAPACK* DG_MakeDmapack( int flag, int phase );
extern void DG_FreeDmapack( DG_DMAPACK *dmapack );

typedef	struct	{
	GV_ACT		actor ;


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


	int			col_r_dest;
	int			col_r;
	int			col_g_dest;
	int			col_g;
	int			col_b_dest;
	int			col_b;
	DG_DMAPACK	*dmapack ;
	void		*packet_mem ;

	int			col_a_dest_plus;
	int			col_a_plus;
	DG_DMAPACK	*dmapack_plus ;
	void		*packet_mem_plus ;

	DG_DMAPACK	*dmapack_aft ;
	void		*packet_mem_aft ;

	int			keep_r_plus;
	int			keep_g_plus;
	int			keep_b_plus;
	int			keep_a_plus;

	int			time;
	int			time_max;

	int			nega_posi_flag;

	int			clock;

	int			proc_id;

} Work ;

Work *CODEC_OK_FADE_IO_WORK = NULL;

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

static void Act( Work *work )
{
	SCREEN_DRAW	*packet ;

	work->clock = 1 - work->clock;

	if( work->time_fade < 0 ){
		work->dmapack_fade->flag |= DG_DMAPACK_INVISIBLE0 ;
	}else{
		work->dmapack_fade->flag &= ~DG_DMAPACK_INVISIBLE0 ;
		work->keep_r_fade = work->col_r_dest_fade + (work->col_r_fade - work->col_r_dest_fade) * work->time_fade / work->time_max_fade;
		work->keep_g_fade = work->col_g_dest_fade + (work->col_g_fade - work->col_g_dest_fade) * work->time_fade / work->time_max_fade;
		work->keep_b_fade = work->col_b_dest_fade + (work->col_b_fade - work->col_b_dest_fade) * work->time_fade / work->time_max_fade;
		work->keep_a_fade = work->col_a_dest_fade + (work->col_a_fade - work->col_a_dest_fade) * work->time_fade / work->time_max_fade;
		packet = work->packet_mem_fade;
		packet[work->clock].gif_packet.gif_data.rgbq.data = SCE_GS_SET_RGBAQ( work->keep_r_fade, work->keep_g_fade, work->keep_b_fade, work->keep_a_fade, 0);
	}

	if( work->time < 0 ){
		work->dmapack_aft->flag  |= DG_DMAPACK_INVISIBLE0 ;
		work->dmapack->flag      |= DG_DMAPACK_INVISIBLE0 ;
		work->dmapack_plus->flag |= DG_DMAPACK_INVISIBLE0 ;
	}else{
		work->dmapack_aft->flag  &= ~DG_DMAPACK_INVISIBLE0 ;
		work->dmapack->flag      &= ~DG_DMAPACK_INVISIBLE0 ;
		work->dmapack_plus->flag &= ~DG_DMAPACK_INVISIBLE0 ;

		work->keep_r_plus = work->col_r_dest + (work->col_r - work->col_r_dest) * work->time / work->time_max;
		work->keep_g_plus = work->col_g_dest + (work->col_g - work->col_g_dest) * work->time / work->time_max;
		work->keep_b_plus = work->col_b_dest + (work->col_b - work->col_b_dest) * work->time / work->time_max;
		packet = work->packet_mem;
		packet[work->clock].gif_packet.gif_data.rgbq.data = SCE_GS_SET_RGBAQ( work->keep_r_plus, work->keep_g_plus, work->keep_b_plus, 128, 0);
		if( work->nega_posi_flag==0 ){
			packet[work->clock].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(1,0,2,2,128);
		}else{
			packet[work->clock].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(0,1,2,2,128);
		}


		work->keep_a_plus = work->col_a_dest_plus + (work->col_a_plus - work->col_a_dest_plus) * work->time / work->time_max;
		packet = work->packet_mem_plus;
		packet[work->clock].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(1,2,2,2,work->keep_a_plus);
		packet = work->packet_mem_aft;
		packet[work->clock].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(1,2,2,2,work->keep_a_plus);
		packet[work->clock].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(1,2,2,2,work->keep_a_plus);
	}

	if( work->time_fade > 0 ){
		work->time_fade--;
		if( work->time_fade==0  &&  work->proc_id !=-1 ){
			GCL_ExecProc( work->proc_id, NULL );
			GV_DestroyActor( work ) ;
			printf("GEGE\n");
			return;
		}
	}
	if( work->time      > 0 ) work->time--;

}

static void Die( Work *work )
{
	/* パケットメモリ開放 */
	GV_DelayedFree( work->packet_mem_fade );

	GV_DelayedFree( work->packet_mem );
	GV_DelayedFree( work->packet_mem_plus );
	GV_DelayedFree( work->packet_mem_aft );

	/* ＤＭＡパケットオブジェクト開放 */
	DG_DequeueDmapack( work->dmapack_fade );
	DG_FreeDmapack( work->dmapack_fade );

	DG_DequeueDmapack( work->dmapack );
	DG_FreeDmapack( work->dmapack );
	DG_DequeueDmapack( work->dmapack_plus );
	DG_FreeDmapack( work->dmapack_plus );
	DG_DequeueDmapack( work->dmapack_aft );
	DG_FreeDmapack( work->dmapack_aft );

	CODEC_OK_FADE_IO_WORK = NULL;
	printf("CODEC Fadeout end.\n");
}



static int Initialize( Work *work )
{
	DG_DMAPACK	*dmapack ;
	SCREEN_DRAW	*packet ;

	work->dmapack_fade = dmapack = (DG_DMAPACK *)DG_MakeDmapack( DG_DMAPACK_NORMAL | DG_DMAPACK_PRIVILEGE, DG_DMAPACK_PHASE_AFTER );
	if( dmapack == NULL ) return (-1);
	DG_QueueDmapack( dmapack );
	work->packet_mem_fade = packet = GV_Malloc( sizeof(SCREEN_DRAW) * 2 );
	if( packet == NULL ) return (-1);
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;
	packet[0] = def_screen_draw ;
	packet[1] = def_screen_draw ;
	packet[0].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(0,1,0,1,0);
	packet[1].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(0,1,0,1,0);





	work->dmapack = dmapack = (DG_DMAPACK *)DG_MakeDmapack( DG_DMAPACK_NORMAL, DG_DMAPACK_PHASE_AFTER );
	if( dmapack == NULL ) return (-1);
	DG_QueueDmapack( dmapack );
	work->packet_mem = packet = GV_Malloc( sizeof(SCREEN_DRAW) * 2 );
	if( packet == NULL ) return (-1);
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;
	packet[0] = def_screen_draw ;
	packet[1] = def_screen_draw ;
	if( work->nega_posi_flag==0 ){
		packet[0].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(1,0,2,2,128);
		packet[1].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(1,0,2,2,128);
	}else{
		packet[0].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(0,1,2,2,128);
		packet[1].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(0,1,2,2,128);
	}

	work->dmapack_plus = dmapack = (DG_DMAPACK *)DG_MakeDmapack( DG_DMAPACK_NORMAL, DG_DMAPACK_PHASE_AFTER );
	if( dmapack == NULL ) return (-1);
	DG_QueueDmapack( dmapack );
	work->packet_mem_plus = packet = GV_Malloc( sizeof(SCREEN_DRAW) * 2 );
	if( packet == NULL ) return (-1);
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;
	packet[0] = def_screen_draw ;
	packet[1] = def_screen_draw ;
	packet[0].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(1,2,2,2,0);
	packet[1].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(1,2,2,2,0);

	work->dmapack_aft = dmapack = (DG_DMAPACK *)DG_MakeDmapack( DG_DMAPACK_NORMAL, DG_DMAPACK_PHASE_AFTER );
	if( dmapack == NULL ) return (-1);
	DG_QueueDmapack( dmapack );
	work->packet_mem_aft = packet = GV_Malloc( sizeof(SCREEN_DRAW) * 2 );
	if( packet == NULL ) return (-1);
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;
	packet[0] = def_screen_draw ;
	packet[1] = def_screen_draw ;
	packet[0].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(1,2,2,2,0);
	packet[1].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(1,2,2,2,0);

	work->col_r_dest_fade = 0;
	work->col_g_dest_fade = 0;
	work->col_b_dest_fade = 0;
	work->col_a_dest_fade = 0;

	work->col_r_dest = 0;
	work->col_g_dest = 0;
	work->col_b_dest = 0;

	work->col_a_dest_plus = 128;


	work->col_r_fade = 0;
	work->col_g_fade = 0;
	work->col_b_fade = 0;
	work->col_a_fade = 0;

	work->keep_r_fade = 0;
	work->keep_g_fade = 0;
	work->keep_b_fade = 0;
	work->keep_a_fade = 0;


	work->col_r = 0;
	work->col_g = 0;
	work->col_b = 0;
	work->col_a_plus = 0;

	work->time_fade       = 0;
	work->time_max_fade   = 1;
	work->time            = 0;
	work->time_max        = 1;

	return (0);
}

static void SetData( Work *work, int col_r, int col_g, int col_b, int col_a, int time )
{

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
	work->time_max_fade  = time;
}

static int GetResources( Work *work, int col_r, int col_g, int col_b, int col_a, int time )
{
	SetData( work, col_r, col_g, col_b, col_a, time );
	return (0);
}



static void SetDataPlus( Work *work, int col_r, int col_g, int col_b, int col_a, int nega_posi_flag, int time )
{
	int	itemp;

	work->nega_posi_flag = nega_posi_flag;

	/* 同一フレームを引く（足す）ためのデータ */
	work->col_r = work->keep_r_plus;
	work->col_g = work->keep_g_plus;
	work->col_b = work->keep_b_plus;

	work->col_r_dest = col_r;
	if( work->col_r_dest > 255 ){
		work->col_r_dest = 255;
	}else if( work->col_r_dest < 0 ){
		work->col_r_dest = 0;
	}
	work->col_g_dest = col_g;
	if( work->col_g_dest > 255 ){
		work->col_g_dest = 255;
	}else if( work->col_g_dest < 0 ){
		work->col_g_dest = 0;
	}
	work->col_b_dest = col_b;
	if( work->col_b_dest > 255 ){
		work->col_b_dest = 255;
	}else if( work->col_b_dest < 0 ){
		work->col_b_dest = 0;
	}

	work->col_a_plus = work->keep_a_plus;
	work->col_a_dest_plus = col_a;
	if( work->col_a_dest_plus > 255 ){
		work->col_a_dest_plus = 255;
	}else if( work->col_a_dest_plus < 0 ){
		work->col_a_dest_plus = 0;
	}

	work->time      = time;
	work->time_max  = time;
}

static int GetResourcesPlus( Work *work, int col_r, int col_g, int col_b, int col_a, int nega_posi_flag, int time )
{
	SetDataPlus( work, col_r, col_g, col_b, col_a, nega_posi_flag, time );

	return (0);
}


/* フェードインアウト */
void *NewCodecFadeInOut_Demo( int col_r, int col_g, int col_b, int col_a, int time )
{
	Work		*work ;

	OPERATOR() ;

	if( CODEC_OK_FADE_IO_WORK == NULL ){
		work = CODEC_OK_FADE_IO_WORK = (Work *)GV_NewEffect( GV_ACTOR_MANAGER, sizeof( Work ) ) ;
		if ( work != NULL ) {
			if( Initialize( work ) < 0 ){
				GV_DestroyActor( work ) ;
				return NULL ;
			}

			work->proc_id = -1;

			GV_SetActor( &( work->actor ), Act, Die ) ;
			if ( GetResources( work, col_r, col_g, col_b, col_a, time ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
			work->clock = 0;
		}
		return (void *)work ;
	}else{
	        CODEC_OK_FADE_IO_WORK->proc_id = -1;
		SetData( CODEC_OK_FADE_IO_WORK, col_r, col_g, col_b, col_a, time );
		return (void *)CODEC_OK_FADE_IO_WORK ;
	}
}

/* フェードインアウトシナリオ */
void *NewCodecFadeInOut_Scn( int name, int where )
{
	Work		*work ;
	int			col_r;
	int			col_g;
	int			col_b;
	int			col_a;
	int			time;

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

	if( CODEC_OK_FADE_IO_WORK == NULL ){
		work = CODEC_OK_FADE_IO_WORK = (Work *)GV_NewEffect( GV_ACTOR_MANAGER, sizeof( Work ) ) ;
		if ( work != NULL ) {

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

			GV_SetActor( &( work->actor ), Act, Die ) ;
			if ( GetResources( work, col_r, col_g, col_b, col_a, time ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
			work->clock = 0;
		}
		return (void *)work ;
	}else{

		CODEC_OK_FADE_IO_WORK->proc_id = -1;
		if ( GCL_GetOption( 'p' ) != NULL ){
			CODEC_OK_FADE_IO_WORK->proc_id = GCL_GetNextInt();
		}

		if ( GCL_GetOption( 'i' ) != NULL ){
			work->keep_r_fade = GCL_GetNextInt();
			work->keep_g_fade = GCL_GetNextInt();
			work->keep_b_fade = GCL_GetNextInt();
			work->keep_a_fade = GCL_GetNextInt();
		}
		SetData( CODEC_OK_FADE_IO_WORK, col_r, col_g, col_b, col_a, time );
		return (void *)CODEC_OK_FADE_IO_WORK ;
	}
}


/* コントラスト */
/*
col_r : （ 0 ～ 255 ）この値を最初に引く
col_g : （ 0 ～ 255 ）この値を最初に引く
col_b : （ 0 ～ 255 ）この値を最初に引く
col_a : （ 0 ～ 255 ）上記値を引いた後の拡大率（128で１倍。255で２倍。）
nega_posi_flag : 0)通常 1)ネガ（col_r,col_g,col_b が 足される）
time  : 変化終了までのフレーム数
*/
void *NewCodecContrast_Demo( int col_r, int col_g, int col_b, int col_a, int nega_posi_flag, int time )
{
	Work		*work ;

	OPERATOR() ;

	if( CODEC_OK_FADE_IO_WORK == NULL ){
		work = CODEC_OK_FADE_IO_WORK = (Work *)GV_NewEffect( GV_ACTOR_MANAGER, sizeof( Work ) ) ;
		if ( work != NULL ) {

			if( Initialize( work ) < 0 ){
				GV_DestroyActor( work ) ;
				return NULL ;
			}

			work->proc_id = -1;

			GV_SetActor( &( work->actor ), Act, Die ) ;
			if ( GetResourcesPlus( work, col_r, col_g, col_b, col_a, nega_posi_flag, time ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
			work->clock = 0;
		}
		return (void *)work ;
	}else{
		CODEC_OK_FADE_IO_WORK->proc_id = -1;
		SetDataPlus( CODEC_OK_FADE_IO_WORK, col_r, col_g, col_b, col_a, nega_posi_flag, time );
		return (void *)CODEC_OK_FADE_IO_WORK ;
	}
}

/* コントラストシナリオ */
void *NewCodecContrast_Scn( int name, int where )
{
	Work		*work ;
	int col_r;
	int col_g;
	int col_b;
	int col_a;
	int nega_posi_flag;
	int time;

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

	nega_posi_flag = 0;
	if ( GCL_GetOption( 'n' ) != NULL ){
		nega_posi_flag = GCL_GetNextInt();
	}

	time = 1;
	if ( GCL_GetOption( 't' ) != NULL ){
		time = GCL_GetNextInt();
	}


	if( CODEC_OK_FADE_IO_WORK == NULL ){
		work = CODEC_OK_FADE_IO_WORK = (Work *)GV_NewEffect( GV_ACTOR_MANAGER, sizeof( Work ) ) ;
		if ( work != NULL ) {

			if( Initialize( work ) < 0 ){
				GV_DestroyActor( work ) ;
				return NULL ;
			}

			work->proc_id = -1;

			GV_SetActor( &( work->actor ), Act, Die ) ;
			if ( GetResourcesPlus( work, col_r, col_g, col_b, col_a, nega_posi_flag, time ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
			work->clock = 0;
		}
		return (void *)work ;
	}else{
		CODEC_OK_FADE_IO_WORK->proc_id = -1;
		SetDataPlus( CODEC_OK_FADE_IO_WORK, col_r, col_g, col_b, col_a, nega_posi_flag, time );
		return (void *)CODEC_OK_FADE_IO_WORK ;
	}
}

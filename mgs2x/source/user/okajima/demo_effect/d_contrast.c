//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_contrast.c
	コントラスト

	2000/03/23 S.Okajima
	$Id: d_contrast.c,v 1.1.1.3 2002/11/19 11:46:52 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
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

#define PRIORITY_PRG_CONT0	(95)
#define PRIORITY_PRG_CONT1	(96)
#define PRIORITY_PRG_CONT2	(97)
#define PHASE_PRG	(DG_PLUGIN_PHASE_NORMAL)


extern int DM_FrameSkip ;

typedef	struct	{
	GV_ACT_EX		actor ;

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

	int			keep_r_plus;
	int			keep_g_plus;
	int			keep_b_plus;
	int			keep_a_plus;

	int			time;
	int			time_max;

	int			nega_posi_flag;

} Work ;

static Work *OK_FADE_IO_WORK = NULL;

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
#if 1 //BP_GCC
   //dmatag
   {
      DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_packet) ), //qwc
      NULL, //addr
      //vifcode
      {
         SCE_VIF1_SET_NOP( 0 ),
         SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(struct _gif_packet), 0)
      }
   },
   //gif_packet
   {
      { SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _gif_data), 1, 0, 0, 0, 1), GS_REGS_AD }, //giftag
      //gif_data
      {
         { SCE_GS_SET_ALPHA(0,2,0,1,0), SCE_GS_ALPHA_1 }, //alpha
         { SCE_GS_SET_CLAMP(2,2,0,DRAW_WIDTH-1,1,DRAW_HEIGHT-2), SCE_GS_CLAMP_1 }, //clamp
         { SCE_GS_SET_TEX0(0,8,0,9,8,0,0,0,0,0,0,0), SCE_GS_TEX0_1 }, //tex0
         { SCE_GS_SET_TEST(0,0,0,0,0,0,1,1), SCE_GS_TEST_1 }, //test1
         { SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,1,0,0), SCE_GS_PRIM }, //prim
         { SCE_GS_SET_RGBAQ(255,255,255,64,0), SCE_GS_RGBAQ }, //rgbq
         { SCE_GS_SET_UV(0+8,0+8), SCE_GS_UV }, //uv0
         { SCE_GS_SET_XYZ( ((2048-DRAW_WIDTH/2)*16), ((2048-DRAW_HEIGHT/2)*16), 0xffffffff), SCE_GS_XYZ2 }, //xyz0
         { SCE_GS_SET_UV(DRAW_WIDTH*16+16-8,DRAW_HEIGHT*16+16-8), SCE_GS_UV }, //uv1
         { SCE_GS_SET_XYZ( ((2048+DRAW_WIDTH/2)*16), ((2048+DRAW_HEIGHT/2)*16), 0xffffffff), SCE_GS_XYZ2 }, //xyz1
         { SCE_GS_SET_TEST( 1, 5, 64, 1, 0, 0, 1, 1 ), SCE_GS_TEST_1 }, //test2
      }
   }
#else
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
#endif
};

#else
static SCREEN_DRAW def_screen_draw ;
#endif
static void Act( Work *work )
{
	SCREEN_DRAW	*packet ;


	work->keep_r_plus = work->col_r_dest + (work->col_r - work->col_r_dest) * work->time / work->time_max;
	work->keep_g_plus = work->col_g_dest + (work->col_g - work->col_g_dest) * work->time / work->time_max;
	work->keep_b_plus = work->col_b_dest + (work->col_b - work->col_b_dest) * work->time / work->time_max;
#ifdef PSX2
	packet = work->packet_mem;
	packet[DG_Clock].gif_packet.gif_data.rgbq.data = SCE_GS_SET_RGBAQ( work->keep_r_plus, work->keep_g_plus, work->keep_b_plus, 128, 0);
	if( work->nega_posi_flag==0 ){
		packet[DG_Clock].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(1,0,2,2,128);
	}else{
		packet[DG_Clock].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(0,1,2,2,128);
	}
#else
	{
		void			*prim ;
		unsigned int	col ;
		col = 	((u_long64)(work->keep_r_plus)| ((u_long64)(work->keep_g_plus) << 8) | 
				 ((u_long64)(work->keep_b_plus) << 16) | (u_long64)128 << 24 );
		prim = work->dmapack->autopacket;
		if( work->nega_posi_flag==0 ){
			prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(1,0,2,2,128) );
		}else{
			prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0,1,2,2,128) );
		}
		prim = DG_SetDmapackBox( prim,
								 0, 0, 
								 640, 448,  col );
		prim = DG_SetDmapackEnd( prim );
	}
#endif
	work->keep_a_plus = work->col_a_dest_plus + (work->col_a_plus - work->col_a_dest_plus) * work->time / work->time_max;
#ifdef PSX2
	packet = work->packet_mem_plus;
	packet[DG_Clock].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(1,2,2,2,work->keep_a_plus);
#else
	{
		void			*prim ;
		unsigned int	col ;		
		
		col = 	((u_long64)255 | ((u_long64)255 << 8) | 
				 ((u_long64)255 << 16) | ((u_long64)64) << 24 );
		prim = work->dmapack_plus->autopacket ;
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(1,2,2,2,work->keep_a_plus) );
		prim = DG_SetDmapackBox( prim,
								 0, 0, 
								 640, 448, col );
		prim = DG_SetDmapackEnd( prim );
	}
#endif

	if( work->keep_r_plus == 0
	 && work->keep_g_plus == 0
	 && work->keep_b_plus == 0
	 && work->keep_a_plus == 128
	 ){
		work->dmapack->flag      |= DG_DMAPACK_INVISIBLEMENU ;
		work->dmapack_plus->flag |= DG_DMAPACK_INVISIBLEMENU ;
	}else{
		work->dmapack->flag      &= ~DG_DMAPACK_INVISIBLEMENU ;
		work->dmapack_plus->flag &= ~DG_DMAPACK_INVISIBLEMENU ;
	}

	if( work->time > 0 ){
		work->time -= DM_FrameSkip ;
		work->time--;
		if( work->time < 0 ) work->time = 0;
	}else{
		work->time = 0;
	}

}

static void Die( Work *work )
{
	/* パケットメモリ開放 */
	GV_DelayedFree( work->packet_mem );
	GV_DelayedFree( work->packet_mem_plus );

	/* ＤＭＡパケットオブジェクト開放 */
	DG_DequeueDmapack( work->dmapack );
	DG_FreeDmapack( work->dmapack );
	DG_DequeueDmapack( work->dmapack_plus );
	DG_FreeDmapack( work->dmapack_plus );

	OK_FADE_IO_WORK = NULL;
}



static int Initialize( Work *work )
{
	DG_DMAPACK	*dmapack ;
	SCREEN_DRAW	*packet ;

	work->dmapack = dmapack = (DG_DMAPACK *)DG_MakeDmapack2( DG_DMAPACK_MENU, PHASE_PRG, PRIORITY_PRG_CONT0 );
	if( dmapack == NULL ) return (-1);
	DG_QueueDmapack( dmapack );

#ifdef PSX2 /*yano 2002.04.02*/
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
#else
	if( !(work->packet_mem = GV_Malloc( 256 )) ){
		return ( -1 );
	}
	dmapack->autopacket = work->packet_mem;
	DG_SetDmapackEnd( work->packet_mem );
#endif
	work->dmapack_plus = dmapack = (DG_DMAPACK *)DG_MakeDmapack2( DG_DMAPACK_MENU, PHASE_PRG, PRIORITY_PRG_CONT1 );
	if( dmapack == NULL ) return (-1);
	DG_QueueDmapack( dmapack );
#ifdef PSX2
	work->packet_mem_plus = packet = GV_Malloc( sizeof(SCREEN_DRAW) * 2 );
	if( packet == NULL ) return (-1);
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;
	packet[0] = def_screen_draw ;
	packet[1] = def_screen_draw ;
	packet[0].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(1,2,2,2,0);
	packet[1].gif_packet.gif_data.alpha.data = SCE_GS_SET_ALPHA(1,2,2,2,0);
#else
	if( !(work->packet_mem_plus = GV_Malloc( 256 )) ){
		return ( -1 );
	}
	dmapack->autopacket = work->packet_mem_plus;
	DG_SetDmapackEnd( work->packet_mem_plus );
#endif

	work->col_r_dest = 0;
	work->col_g_dest = 0;
	work->col_b_dest = 0;

	work->col_a_dest_plus = 128;

	work->col_r = 0;
	work->col_g = 0;
	work->col_b = 0;
	work->col_a_plus = 0;

	work->time            = 0;
	work->time_max        = 1;

	return (0);
}

static void SetDataPlus( Work *work, int col_r, int col_g, int col_b, int col_a, int nega_posi_flag, int time )
{

	time = DIRECT_TICK( time ) ;

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
	if( work->time_max < 1 ) work->time_max = 1;
}

static int GetResourcesPlus( Work *work, int col_r, int col_g, int col_b, int col_a, int nega_posi_flag, int time )
{
	SetDataPlus( work, col_r, col_g, col_b, col_a, nega_posi_flag, time );

	return (0);
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
void *NewContrast_Demo( int col_r, int col_g, int col_b, int col_a, int nega_posi_flag, int time )
{
	Work		*work ;

	OPERATOR() ;

	if(time > 3) time -= 3;

	if( OK_FADE_IO_WORK == NULL ){
		work = OK_FADE_IO_WORK = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
		if ( work != NULL ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )

			if( Initialize( work ) < 0 ){
				GV_DestroyActor( work ) ;
				return NULL ;
			}

			if ( GetResourcesPlus( work, col_r, col_g, col_b, col_a, nega_posi_flag, time ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		}
		return (void *)work ;
	}else{
		SetDataPlus( OK_FADE_IO_WORK, col_r, col_g, col_b, col_a, nega_posi_flag, time );
		return (void *)OK_FADE_IO_WORK ;
	}
}

void *NewContrastForce_Demo( int ini_r, int ini_g, int ini_b, int ini_a, 
                             int col_r, int col_g, int col_b, int col_a, int nega_posi_flag, int time )
{
	Work		*work ;

	OPERATOR() ;

	if(time > 3) time -= 3;

	if( OK_FADE_IO_WORK == NULL ){
		work = OK_FADE_IO_WORK = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
		if ( work != NULL ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )

			if( Initialize( work ) < 0 ){
				GV_DestroyActor( work ) ;
				return NULL ;
			}

			work->keep_r_plus = ini_r;
			work->keep_g_plus = ini_g;
			work->keep_b_plus = ini_b;
			work->keep_a_plus = ini_a;

			if ( GetResourcesPlus( work, col_r, col_g, col_b, col_a, nega_posi_flag, time ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		}
		return (void *)work ;
	}else{
		OK_FADE_IO_WORK->keep_r_plus = ini_r;
		OK_FADE_IO_WORK->keep_g_plus = ini_g;
		OK_FADE_IO_WORK->keep_b_plus = ini_b;
		OK_FADE_IO_WORK->keep_a_plus = ini_a;
		SetDataPlus( OK_FADE_IO_WORK, col_r, col_g, col_b, col_a, nega_posi_flag, time );
		return (void *)OK_FADE_IO_WORK ;
	}
}

/* コントラストシナリオ */
void *NewContrast_Scn( int name, int where )
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


	if( OK_FADE_IO_WORK == NULL ){
		work = OK_FADE_IO_WORK = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
		if ( work != NULL ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )

			if( Initialize( work ) < 0 ){
				GV_DestroyActor( work ) ;
				return NULL ;
			}

			if ( GetResourcesPlus( work, col_r, col_g, col_b, col_a, nega_posi_flag, time ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		}
		return (void *)work ;
	}else{
		SetDataPlus( OK_FADE_IO_WORK, col_r, col_g, col_b, col_a, nega_posi_flag, time );
		return (void *)OK_FADE_IO_WORK ;
	}
}



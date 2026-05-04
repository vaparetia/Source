//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blur.c
	ブラー

	1999/09/24 S.Okajima
	$Id: blur.c,v 1.6 2002/11/23 12:09:25 Yoshizawa1 Exp $

*/
#if 0 //BP_PS2 def PSX2
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
#endif

#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#if 0 //BP_PS2 def PSX2
#include	"utl_dma.h"
#endif

#include	"../etc/ok_util.h"

#if 0 //BP_PS2 def PSX2
#include	"def_dma.h"
#endif

#include "BP_RenderBuffer.h"

#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))


//extern DG_DMAPACK* DG_MakeDmapack( int flag, int phase );
//extern void DG_FreeDmapack( DG_DMAPACK *dmapack );
extern int DM_FrameSkip;


#define PRIORITY_PRG	(100)
#define PHASE_PRG	(DG_PLUGIN_PHASE_NORMAL)

enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
	REQ_PARAM,
	REQ_NO
};

typedef	struct	{
	GV_ACT_EX		actor ;

	int			name;
	int			where;

	DG_DMAPACK	*dmapack ;
	void		*packet_mem ;

	int			time;
	float		intense_data;
	float		intense;
	float		add_data;

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
			.alpha = { .reg = SCE_GS_ALPHA_1, .data = SCE_GS_SET_ALPHA(0,1,0,1,64) },
			.clamp = { .reg = SCE_GS_CLAMP_1, .data = SCE_GS_SET_CLAMP(2,2,0,DRAW_WIDTH-1,1,DRAW_HEIGHT-2) },
			.tex0  = { .reg = SCE_GS_TEX0_1,  .data = SCE_GS_SET_TEX0(0,8,0,9,8,0,0,0,0,0,0,0) },
			.test1 = { .reg = SCE_GS_TEST_1,  .data = SCE_GS_SET_TEST(0,0,0,0,0,0,1,1) },
			.prim  = { .reg = SCE_GS_PRIM,    .data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) },
			.rgbq  = { .reg = SCE_GS_RGBAQ,   .data = SCE_GS_SET_RGBAQ(128,128,128,64,0) },
			.uv0   = { .reg = SCE_GS_UV,      .data = SCE_GS_SET_UV(0+16,0+16) },
			.xyz0  = { .reg = SCE_GS_XYZ2,
				.data = SCE_GS_SET_XYZ( ((2048-DRAW_WIDTH/2)*16+8),
									   ((2048-DRAW_HEIGHT/2)*16+8),
									   0xffffffff) },
			.uv1   = { .reg = SCE_GS_UV,      .data = SCE_GS_SET_UV(DRAW_WIDTH*16+16-16,DRAW_HEIGHT*16+16-16)},
			.xyz1  = { .reg = SCE_GS_XYZ2,
				.data = SCE_GS_SET_XYZ( ((2048+DRAW_WIDTH/2)*16-8),
									   ((2048+DRAW_HEIGHT/2)*16-8),
									   0xffffffff) },
			.test2 = { .reg = SCE_GS_TEST_1,  .data = SCE_GS_SET_TEST( 1, 5, 64, 1, 0, 0, 1, 1 ) },
		}
	}
};
#else


//..PATCH_NISI
static SCREEN_DRAW def_screen_draw;

/*
 = {
	{
		DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_packet) ),
		{
			SCE_VIF1_SET_NOP( 0 ),
			SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(struct _gif_packet), 0)
		}
	},
	
	{
		{ SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _gif_data), 1, 0, 0, 0, 1), GS_REGS_AD },
		{
			{ SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,0,1,64) },
			{ SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP(2,2,0,DRAW_WIDTH-1,1,DRAW_HEIGHT-2) },
			{ SCE_GS_TEX0_1,  SCE_GS_SET_TEX0(0,8,0,9,8,0,0,0,0,0,0,0) },
			{ SCE_GS_TEST_1,  SCE_GS_SET_TEST(0,0,0,0,0,0,1,1) },
			{ SCE_GS_PRIM,    SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) },
			{ SCE_GS_RGBAQ,   SCE_GS_SET_RGBAQ(128,128,128,64,0) },
			{ SCE_GS_UV,      SCE_GS_SET_UV(0+16,0+16) },
			{ SCE_GS_XYZ2,
				SCE_GS_SET_XYZ( ((2048-DRAW_WIDTH/2)*16+8),
				((2048-DRAW_HEIGHT/2)*16+8),
				0xffffffff) },
		{ SCE_GS_UV, SCE_GS_SET_UV(DRAW_WIDTH*16+16-16,DRAW_HEIGHT*16+16-16)},
		{ SCE_GS_XYZ2,	 SCE_GS_SET_XYZ( ((2048+DRAW_WIDTH/2)*16-8),
									   ((2048+DRAW_HEIGHT/2)*16-8),
									   0xffffffff) },
			{ SCE_GS_TEST_1,SCE_GS_SET_TEST( 1, 5, 64, 1, 0, 0, 1, 1 ) },
		}
	}
};
*/


#endif


static void Act( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;
	int time_max;
	SCREEN_DRAW	*packet ;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case REQ_OFF:
			work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU ;
			break;
		  case REQ_ON:
			work->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU ;
			break;
		  case REQ_KILL:
			GV_DestroyActor( work ) ;
			break;
		  case REQ_PARAM:
			work->intense_data = (float)((int)msg->message[1] & 255);
			work->time = msg->message[2];
			work->time = DIRECT_TICK( work->time );
			if(work->time < 1) work->time=1;
			time_max = (float)work->time;
			work->add_data     = (work->intense_data - work->intense) / time_max;
			break;
			default:
			  break;
		}
		msg--;
	}

	if( work->time > 0 ){
		work->time -= DM_FrameSkip ;
		work->time--;
		if( work->time < 0 ) work->time = 0;
		work->intense += work->add_data;
		if( work->intense > 128.0f ) work->intense = 128.0f;
		if( work->intense < 0.0f   ) work->intense = 0.0f;
//printf("%f\n",work->intense);
	}

#if 0 //BP_PS2 def PSX2
	packet = work->dmapack->packet[DG_Clock];
	packet->gif_packet.gif_data.alpha.data = 
	         SCE_GS_SET_ALPHA(0,1,2,1, (int)work->intense ) ;
#else
	{
		void			*prim ;
		unsigned int	col, alpha ;
#if 1 //BP Use this original code, the xbox code below uses MODULATE1x mode which messes up since w're using the correct mode.
		col = 0x00808080 | ( (unsigned int)work->intense << 24 );
#else
		/* アルファにフレームレートを反映させる */
		alpha = work->intense ;
		//if  ( work->intense > 128 ) alpha = 64 ;
		{
			int		i, org_alpha ;
			org_alpha = alpha ;
			for ( i = 0 ; i < DM_FrameSkip ; i++ ){
				alpha = ( alpha + ( alpha * org_alpha / 128 ) ) / 2 ;
			}
			/* 処理落ちがなかったときのアルファと時間経過を考慮したアルファの平均を取る */
			//alpha = ( org_alpha + alpha ) / 2 ;
			//alpha = org_alpha + alpha ;
			alpha *= 2 ;
			if ( alpha > 255 ) alpha = 255 ;
		}
		if  ( work->intense > 128 ) alpha = 128 ;
		//col = 0x00808080 | ( alpha << 24 );
		col = 0x00ffffff | ( alpha << 24 );
		//col = 0x80ffffff ;
#endif
		prim = work->dmapack->autopacket ;
      prim = DG_SetDmapackViewMapping( prim, 0.0f, 0.0f, 960.0f, 544.0f );

		prim = DG_SetDmapackUseFrameTex( prim, 1 );	/* フレームバッファをテクスチャとして使用する宣言 */
		prim = DG_SetDmapackModeEnable( prim, DG_DMAPACK_MODE_NO_TEXALPHA );
		//BP_XBOX prim = DG_SetDmapackModeEnable( prim, DG_DMAPACK_MODE_MODULATE1X );
		//prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0,1,2,1, (int)work->intense ) );
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0,1,0,1, 0 ) );
		/* ＵＶ値はDG_FRAME_V(),DG_FRAME_V()マクロを使用して指定すること */
		
      prim = DG_SetDmapackSprt_F( prim,
								 0, 0, 0.0f / 960.0f, 0.0f / 544.0f,
								 960, 544, 960.0f / 960.0f, 544.0f / 544.0f,
                                  DG_MakeDmaPackColorFromInt(col), 0 );

		prim = DG_SetDmapackModeDisable( prim, DG_DMAPACK_MODE_NO_TEXALPHA );
		//BP_XBOX prim = DG_SetDmapackModeDisable( prim, DG_DMAPACK_MODE_MODULATE1X );
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
}

#ifdef KP_WINDOWS
static void DmyAct( Work *work ){}
static void DmyDie( Work *work ){}
#endif

/* 初期設定値を取得 */
static	void	GetOptionValue( Work *work )
{
	work->intense = 64.0f;
	if ( GCL_GetOption( 'i' ) != NULL ){
		work->intense = (float)GCL_GetNextInt();
	}
}

static int GetResources( Work *work )
{
	DG_DMAPACK	*dmapack ;
	SCREEN_DRAW	*packet ;
	int		i;

	GetOptionValue( work );

	/* ＤＭＡパケット型オブジェクト作成 */
	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, PHASE_PRG, PRIORITY_PRG );

#if BP_DMAPACK_DEBUG_INFO
   dmapack->BP_LabelMask |= kRL_PostFx;
#endif

	DG_QueueDmapack( dmapack );

	/* パケットメモリ割り当て */
#if 0 //BP_PS2 def PSX2
	packet = work->packet_mem = GV_Malloc( sizeof(SCREEN_DRAW) * 2 );
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;

	/* パケットの初期化 */
	for ( i = 0 ; i < 2 ; i++ ){
		packet[i] = def_screen_draw ;
		packet[i].gif_packet.gif_data.tex0.data = SCE_GS_SET_TEX0(
													   BUFFER_PAGE(1-i) / 64 ,
													   BUFFER_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),10,10,0,0,0,0,0,0,0);


#if 0
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

#else
	if ( !(work->packet_mem = GV_Malloc( 256 )) ) {
		return -1 ;
	}
	dmapack->autopacket = work->packet_mem ;
	DG_SetDmapackEnd( work->packet_mem );
#endif

	return (0);
}

void *NewBlur( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		work->name  = name;
		work->where = where;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
#else
		if ( DG_CheckUseBlur() ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )
			work->name  = name;
			work->where = where;
			if ( GetResources( work ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		} else {
			GV_SetActor( &( work->actor ), DmyAct, DmyDie ) ;
			GV_ActorEX( &work->actor )
			work->name  = name;
			work->where = where;
		}
#endif

	}
	return (void *)work ;
}

static int GetResources_Demo( Work *work, float intense )
{
	DG_DMAPACK	*dmapack ;
	SCREEN_DRAW	*packet ;
	int		i;

	work->intense = intense;

	/* ＤＭＡパケット型オブジェクト作成 */
	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, PHASE_PRG, PRIORITY_PRG );

#if BP_DMAPACK_DEBUG_INFO
   dmapack->BP_LabelMask |= kRL_PostFx;
#endif

	DG_QueueDmapack( dmapack );
	/* パケットメモリ割り当て */
#if 0 //BP_PS2 def PSX2
	packet = work->packet_mem = GV_Malloc( sizeof(SCREEN_DRAW) * 2 );
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;

	/* パケットの初期化 */
	for ( i = 0 ; i < 2 ; i++ ){
		packet[i] = def_screen_draw ;
		packet[i].gif_packet.gif_data.tex0.data = SCE_GS_SET_TEX0(
													   BUFFER_PAGE(1-i) / 64 ,
													   BUFFER_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),10,10,0,0,0,0,0,0,0);


#if 0
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

#else
	if ( !(work->packet_mem = GV_Malloc( 256 )) ) {
		return -1 ;
	}
	dmapack->autopacket = work->packet_mem ;
	DG_SetDmapackEnd( work->packet_mem );
#endif

	return (0);
}

void *NewBlur_Demo( int name, float intense )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		work->name  = name;
		//work->where = where;
		if ( GetResources_Demo( work, intense ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
#else
		if ( DG_CheckUseBlur() ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )
			work->name  = name;
			//work->where = where;
			if ( GetResources_Demo( work, intense ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		} else {
			GV_SetActor( &( work->actor ), DmyAct, DmyDie ) ;
			GV_ActorEX( &work->actor )
			work->name  = name;
		}
#endif
	}
	return (void *)work ;
}

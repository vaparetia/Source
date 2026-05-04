//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	cinema_scr.c
	画面上下切りフェードインアウト
	
	2000/09/05 T.Shibata
	
	$Id: cinema_scr.c,v 1.4 2002/11/23 12:16:43 Yoshizawa1 Exp $

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
#include "libutl.h"

#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"
#include	"../util/ts_util.h"
#include	"../util/dma_set.h"

#include "BP_Misc.h"
#include "BP_Camera.h"

#define CLOCK_COUNT	(BP_BASE_TICK())

#define FADE_PRIM	(SCE_GS_SET_PRIM(4,1,0,0,1,0,0,0,0))
#define FADE_TEST	(SCE_GS_SET_TEST( 0, 7, 0, 0, 0, 0, 1, 1 ))
#define FADE_ALPHA	(SCE_GS_SET_ALPHA(0,1,0,1,0x00))

#define N_VERTS		(8)

typedef struct {
	DG_DMATAG		dmatag_draw;		// ＧＩＦ接続ＤＭＡタグ
	MDL_DRAW		set_draw;			// 								描画の設定
	DG_DMATAG		dmatag_verts;		// ＧＩＦ接続ＤＭＡタグ
	DG_GIFTAG		giftag_verts;
	NOTVERT_DATA	verts[N_VERTS];		//								頂点
//	DG_DMATAG		dmatag_test;		// ＧＩＦ接続ＤＭＡタグ
//	PACKET_END		end_paket;			//								ピクセルテスト復元
//	DG_DMATAG		dmatag_offset;		// オフセット環境復元用
	DG_DMATAG		dmatag_end;			// RETタグ
}CINEMA_PACK;

typedef	struct t_Work_cinema_Scr	{  //BP_GENERAL - adding type so we can debug on PC
	GV_ACT_EX			actor ;
	
	int					name;
	int					where;
	int					top_pix;
	int					bottom_pix;
	
	int					fade_time;
	int					def_fade_time;	
	int					timer;
	int					mode;		//0-1bit 0:nop 1:フェードイン 2:フェードアウト

	float				add_rgb;
	float					rgb;
	
	DG_DMAPACK			*dmapack;	
	CINEMA_PACK			*cinema_pack[2];
	NOTVERT_DATA		*verts[2];
#if 1 //BP_XBOX ndef PSX2
	void				*packet_mem;
#endif
} Work ;

#define	MODE_CHECK		(0x03)
#define MODE_NOP		(0x00)
#define	MODE_FADEIN		(0x01)
#define	MODE_FADEOUT	(0x02)
#define	MODE_BLACK		(0x03)

enum {
	MSG_CNMSCR_FADE_OUT = 0,
	MSG_CNMSCR_FADE_IN,
	MSG_CNMSCR_KILL,
};

static void *CineWork = NULL;

static void InitDmaTags( Work *work, CINEMA_PACK *packet, int which )
{
	// --------  DMATag  ----------------------------
	// ＤＭＡタグ		描画設定
	packet->dmatag_draw.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(MDL_DRAW)) ;
	packet->dmatag_draw.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_draw.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(MDL_DRAW), 0) ;
	// ＤＭＡタグ		頂点
	packet->dmatag_verts.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(NOTVERT_DATA) * N_VERTS + SIZEOF_QWORD(DG_GIFTAG) ) ;
	packet->dmatag_verts.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_verts.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(NOTVERT_DATA) * N_VERTS + SIZEOF_QWORD(DG_GIFTAG) , 0) ;
	packet->giftag_verts.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(NOTVERT_DATA)* N_VERTS, 1, 0, 0, 0, 1);
	packet->giftag_verts.regs = GS_REGS_AD;
					  
	// ＤＭＡタグ		ピクセルテスト復元
//	packet->dmatag_test.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(PACKET_END) ) ;
//	packet->dmatag_test.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
//	packet->dmatag_test.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(PACKET_END), 0) ;
	// オフセット環境復元用
//	packet->dmatag_offset.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWOFFSET) ) ;
//	packet->dmatag_offset.addr = &( DG_Chanl( 0 )->draw_offset[ which ] ) ;
//	packet->dmatag_offset.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
//	packet->dmatag_offset.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_DRAWOFFSET), 0) ;
	// RETタグ
	packet->dmatag_end.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 ) ;
	packet->dmatag_end.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_end.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
}

static void InitVerts( Work *work )
{
	NOTVERT_DATA	*verts0,*verts1;

	verts0 = work->verts[0];
	verts1 = work->verts[1];
				
	verts0[0].xyz.reg = verts1[0].xyz.reg = SCE_GS_XYZ3;
	verts0[1].xyz.reg = verts1[1].xyz.reg = SCE_GS_XYZ3;
	verts0[2].xyz.reg = verts1[2].xyz.reg = SCE_GS_XYZ2;
	verts0[3].xyz.reg = verts1[3].xyz.reg = SCE_GS_XYZ2;
	verts0[4].xyz.reg = verts1[4].xyz.reg = SCE_GS_XYZ3;
	verts0[5].xyz.reg = verts1[5].xyz.reg = SCE_GS_XYZ3;
	verts0[6].xyz.reg = verts1[6].xyz.reg = SCE_GS_XYZ2;
	verts0[7].xyz.reg = verts1[7].xyz.reg = SCE_GS_XYZ2;
	
	verts0[0].rgbq.reg = verts1[0].rgbq.reg = SCE_GS_RGBAQ;
	verts0[1].rgbq.reg = verts1[1].rgbq.reg = SCE_GS_RGBAQ;
	verts0[2].rgbq.reg = verts1[2].rgbq.reg = SCE_GS_RGBAQ;
	verts0[3].rgbq.reg = verts1[3].rgbq.reg = SCE_GS_RGBAQ;
	verts0[4].rgbq.reg = verts1[4].rgbq.reg = SCE_GS_RGBAQ;
	verts0[5].rgbq.reg = verts1[5].rgbq.reg = SCE_GS_RGBAQ;
	verts0[6].rgbq.reg = verts1[6].rgbq.reg = SCE_GS_RGBAQ;
	verts0[7].rgbq.reg = verts1[7].rgbq.reg = SCE_GS_RGBAQ;

	verts0[0].xyz.data = verts1[0].xyz.data = SCE_GS_SET_XYZ( MIN_X, MIN_Y,0);
	verts0[1].xyz.data = verts1[1].xyz.data = SCE_GS_SET_XYZ( MAX_X, MIN_Y,0);
	verts0[2].xyz.data = verts1[2].xyz.data = SCE_GS_SET_XYZ( MIN_X, MIN_Y + (work->top_pix<<4),0);
	verts0[3].xyz.data = verts1[3].xyz.data = SCE_GS_SET_XYZ( MAX_X, MIN_Y + (work->top_pix<<4),0);
	verts0[4].xyz.data = verts1[4].xyz.data = SCE_GS_SET_XYZ( MIN_X, MAX_Y - (work->bottom_pix<<4),0);
	verts0[5].xyz.data = verts1[5].xyz.data = SCE_GS_SET_XYZ( MAX_X, MAX_Y - (work->bottom_pix<<4),0);
	verts0[6].xyz.data = verts1[6].xyz.data = SCE_GS_SET_XYZ( MIN_X, MAX_Y,0);
	verts0[7].xyz.data = verts1[7].xyz.data = SCE_GS_SET_XYZ( MAX_X, MAX_Y,0);

	verts0[0].rgbq.data = verts1[0].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,0x00,0);
	verts0[1].rgbq.data = verts1[1].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,0x00,0);
	verts0[2].rgbq.data = verts1[2].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,0x00,0);
	verts0[3].rgbq.data = verts1[3].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,0x00,0);
	verts0[4].rgbq.data = verts1[4].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,0x00,0);
	verts0[5].rgbq.data = verts1[5].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,0x00,0);
	verts0[6].rgbq.data = verts1[6].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,0x00,0);
	verts0[7].rgbq.data = verts1[7].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,0x00,0);

}

static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		case MSG_CNMSCR_FADE_OUT:
			if((work->mode&MODE_CHECK) == MODE_NOP){
				work->mode = MODE_FADEOUT;
				work->timer = -1;
				work->fade_time = DIRECT_TICK(msg->message[1]);
				//printf("FadeOut Start  time[%d]\n",work->fade_time);
			}else if((work->mode&MODE_CHECK) == MODE_FADEIN){
				work->mode = MODE_FADEOUT;
				work->fade_time = work->timer;
				work->timer = -1;
			}
			break;
		case MSG_CNMSCR_FADE_IN:
			if((work->mode&MODE_CHECK) == MODE_BLACK){
				work->mode = MODE_FADEIN;
				work->timer = -1;
				work->fade_time = DIRECT_TICK(msg->message[1]);
				//printf("FadeIn Start  time[%d]\n",work->fade_time);
				DG_SetDrawEnv( DG_Chanl(0), DG_Chanl(0)->offset_x, DG_Chanl(0)->offset_y,
							   DG_Chanl(0)->width, DG_Chanl(0)->height );
			}else if((work->mode&MODE_CHECK) == MODE_FADEOUT){
				work->mode = MODE_FADEIN;
				work->fade_time = work->timer;
				work->timer = -1;
				//printf("FadeIn Start  time[%d]\n",work->fade_time);
				DG_SetDrawEnv( DG_Chanl(0), DG_Chanl(0)->offset_x, DG_Chanl(0)->offset_y,
							   DG_Chanl(0)->width, DG_Chanl(0)->height );
			}
			break;
		case MSG_CNMSCR_KILL:
			work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
			GV_DestroyActor( work );
			return -1;
			break;
		default:
			printf("Cinema_scr Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}

	return 0;
}

static void SetVertsData( Work *work )
{
	NOTVERT_DATA	*verts;
//	int				top = 0,bottom = 0;
	int				rgb0 = 0,rgb1 = 0;
	
	verts = work->verts[DG_Clock];
	work->rgb += work->add_rgb;
	if( work->rgb > 256 ) work->rgb = 256.0f;
	if( work->rgb < 0 ) work->rgb = 0.0f;

	rgb0 = (int)work->rgb;
	
	rgb1 = rgb0 - 128;
	if( rgb0 > 128 ) rgb0 = 128;
	if( rgb1 > 128 ) rgb1 = 128;
	if( rgb1 < 0 ) rgb1 = 0;

	//printf("a0 %d:a1 %d\n",rgb0,rgb1);
#if 0 //BP_PS2 def PSX2 /*yano 2002.03.18*/
#if 0
	verts[2].xyz.data = SCE_GS_SET_XYZ( MIN_X, MIN_Y + (top<<4),0);
	verts[3].xyz.data = SCE_GS_SET_XYZ( MAX_X, MIN_Y + (top<<4),0);
	verts[4].xyz.data = SCE_GS_SET_XYZ( MIN_X, MAX_Y - (bottom<<4),0);
	verts[5].xyz.data = SCE_GS_SET_XYZ( MAX_X, MAX_Y - (bottom<<4),0);
#endif
	verts[0].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,rgb0,0);
	verts[1].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,rgb0,0);
	verts[2].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,rgb1,0);
	verts[3].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,rgb1,0);
	verts[4].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,rgb1,0);
	verts[5].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,rgb1,0);
	verts[6].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,rgb0,0);
	verts[7].rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,rgb0,0);

#else
	{
		void			*prim ;
		unsigned int	col[8] ;
		col[0] = 0x00000000 | ( (unsigned int) rgb0 << 24 );
		col[1] = 0x00000000 | ( (unsigned int) rgb0 << 24 );
		col[2] = 0x00000000 | ( (unsigned int) rgb1 << 24 );
		col[3] = 0x00000000 | ( (unsigned int) rgb1 << 24 );
		col[4] = 0x00000000 | ( (unsigned int) rgb1 << 24 );
		col[5] = 0x00000000 | ( (unsigned int) rgb1 << 24 );
		col[6] = 0x00000000 | ( (unsigned int) rgb0 << 24 );
		col[7] = 0x00000000 | ( (unsigned int) rgb0 << 24 );
		prim = work->dmapack->autopacket ;
	 	prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0,1,0,1,0x00) );
		//BP_RENDER_TODO_BREAK;
#if 0 //BP_WINDOWS ndef KP_WINDOWS
		prim = DG_SetDmapackQuad( prim,
								  0.0f  , 0.0f,                        col[0], 
								  640.0f, 0.0f, 			           col[1], 
								  640.0f, 0.0f + (float)work->top_pix, col[3],
								  0.0f,   0.0f + (float)work->top_pix, col[2] ); 
		prim = DG_SetDmapackQuad( prim,
								  0.0f  , 448.0f - (float)work->bottom_pix, col[4], 
								  640.0f, 448.0f - (float)work->bottom_pix, col[5], 
								  640.0f, 448.0f,                           col[7],
								  0.0f  , 448.0f,                           col[6] ); 
#else
		prim = DG_SetDmapackQuad( prim,
								  0.0f  , 0.0f,                        DG_MakeDmaPackColorFromInt(col[0]), 
								  DRAW_WIDTH, 0.0f, 			           DG_MakeDmaPackColorFromInt(col[1]), 
								  DRAW_WIDTH, 0.0f + (float)work->top_pix, DG_MakeDmaPackColorFromInt(col[3]),
								  0.0f,   0.0f + (float)work->top_pix, DG_MakeDmaPackColorFromInt(col[2]) ); 
		prim = DG_SetDmapackQuad( prim,
								  0.0f  , DRAW_HEIGHT - (float)work->bottom_pix, DG_MakeDmaPackColorFromInt(col[4]), 
								  DRAW_WIDTH, DRAW_HEIGHT - (float)work->bottom_pix, DG_MakeDmaPackColorFromInt(col[5]), 
								  DRAW_WIDTH, DRAW_HEIGHT,                           DG_MakeDmaPackColorFromInt(col[7]),
								  0.0f  , DRAW_HEIGHT,                           DG_MakeDmaPackColorFromInt(col[6]) ); 
#endif
		prim = DG_SetDmapackEnd( prim );
	}
#endif
}

static void Act_PS2( Work *work )   //BP_CAMERA - renamed so we can override visibility
{
	
	if(CheckMesgParam( work )) return;
	
	if(!(work->mode&MODE_CHECK))return;

	//GM_MenuStatus |= MENU_RADAR_OFF;

	if((work->mode&MODE_CHECK) == MODE_BLACK) return;
	
	if( work->timer == -1 ){
		//初期か
		work->dmapack->flag &= ~(DG_DMAPACK_INVISIBLEMENU);
		if( work->fade_time <= 0 ) work->fade_time = work->def_fade_time;
		
		if((work->mode&MODE_CHECK) == MODE_FADEIN){
			//見えるようにする
			work->add_rgb = -work->rgb / (float)work->fade_time;
		}else{
			//消えるようにする
			work->add_rgb = (256.0f - work->rgb) / (float)work->fade_time;
		}
		work->timer++;
	}
	SetVertsData( work );
	if( work->timer++ >= work->fade_time ){
		//終了処理
		if((work->mode&MODE_CHECK) == MODE_FADEOUT){
			work->mode = MODE_BLACK;
			DG_ChangeDrawLimit( DG_Chanl(0), 0, work->top_pix-2,
								DRAW_WIDTH-1, DRAW_HEIGHT - work->bottom_pix + 2 );

		}else if((work->mode&MODE_CHECK) == MODE_FADEIN){
			//DG_SetDrawEnv( DG_Chanl(0), DG_Chanl(0)->offset_x, DG_Chanl(0)->offset_y,
			//			   DG_Chanl(0)->width, DG_Chanl(0)->height );
			work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
			work->mode &= (~MODE_CHECK);
		}
		return;
	}

	//フェードインアウト
	//SetVertsData( work );
}

//BP_CAMERA - call original Act (renamed from Act to Act_PS2), then override visibility
static void Act( Work *work )
{
   // Call original Act
   Act_PS2(work);

   // Update cinema bar overrides
   BP_Camera_Cinemabars_SetState( ((work->mode & MODE_CHECK) != 0), work->top_pix, DRAW_HEIGHT - work->bottom_pix, (work->rgb / 256.0f) );

   // Update visibility
   if(work->mode & MODE_CHECK)   // Bars active?
   {
      if( BP_Camera_Cinemabars_AreEnabled() )
      {
         work->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU;
      }
      else
      {
         work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
      }
   }
}


static void Die( Work *work )
{
#if 0 //BP_XBOX ndef PSX2 /* T.morita Added 2002.03.29 */
	/* パケットメモリ開放 */
	if ( work->packet_mem ) GV_DelayedFree( work->packet_mem );
#endif

   // Update cinema bar overrides
   BP_Camera_Cinemabars_SetState( 0, work->top_pix, DRAW_HEIGHT - work->bottom_pix, 0.0f );

	if(work->dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
	
	if(work->cinema_pack[0]) GV_DelayedFree(work->cinema_pack[0]);

	DG_SetDrawEnv( DG_Chanl(0), DG_Chanl(0)->offset_x, DG_Chanl(0)->offset_y,
				   DG_Chanl(0)->width, DG_Chanl(0)->height );	
	CineWork = NULL;
}

static void GetOptions( Work *work )
{
	int		c;
    while( (c = GCL_GetNextOption()) ){
		switch(c){
		  case 't':
			work->top_pix = GCL_GetNextInt();
			work->top_pix = work->top_pix * DRAW_HEIGHT/224;
			printf("top %d\n",work->top_pix);
			break;
		  case 'b':
			work->bottom_pix = GCL_GetNextInt();
			work->bottom_pix = work->bottom_pix * DRAW_HEIGHT/224;

         if ( BP_IsPAL()==TRUE )
			   work->bottom_pix += 1;

         //BP - Extended lower cinema bar by just a bit to compensate for jimaku adjustment.
         work->bottom_pix += 12;

         printf("bottom %d\n",work->bottom_pix);
			break;
		  case 'f':
			work->def_fade_time = DIRECT_TICK(GCL_GetNextInt());
			printf("def_fade_time %d\n",work->def_fade_time);
			
			break;
		}
	}
}

static int GetResources( Work *work, int name, int where )
{
	DG_DMAPACK		*dmapack;
	CINEMA_PACK		*packet;
	int				i;

	work->name = name;
	work->where = where;
	work->mode = 0;
	work->timer = 0;
	work->fade_time = 0;
	work->def_fade_time = 0;
	work->top_pix = 0;
	work->bottom_pix = 0;
	work->rgb = 0.0f;

	GetOptions( work );

	// ＤＭＡパケット型オブジェクト作成

	work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_AFTER, 128 );
	//work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_LAST, 192 );

	if(dmapack==NULL){
		printf("ERR!! MAKE DMAPACK!!\n");
		return -1;
	}
	work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
	DG_QueueDmapack( dmapack );
#if 0 //BP_PS2 def PSX2 /*yano 2002.03.18*/
	// パケットメモリ割り当て
	packet = GV_Malloc( sizeof(CINEMA_PACK) * 2 );
	if(packet == NULL){
		printf("ERR!! MALLOC WARP_PACK!!\n");
		return -1;
	}
	work->cinema_pack[0] = dmapack->packet[0] = &packet[0] ;
	work->cinema_pack[1] = dmapack->packet[1] = &packet[1] ;
	work->verts[0] = work->cinema_pack[0]->verts;
	work->verts[1] = work->cinema_pack[1]->verts;
#else
		if ( !(work->packet_mem = GV_Malloc( 256 )) ) {
			return -1 ;
		}
		dmapack->autopacket = work->packet_mem ;
		DG_SetDmapackEnd( work->packet_mem );
#endif
	
#if 0 //BP_PS2 def PSX2 /*yano 2002.03.18*/
	for ( i = 0 ; i < 2 ; i++ ){
		InitDmaTags( work, &packet[i], i );
		InitMdlDraw( &packet[i].set_draw, FADE_TEST, FADE_ALPHA, FADE_PRIM );
		//InitEndPacket( &packet[i].end_paket );
	}
	InitVerts( work );
#endif

	return (0);
}

void *NewCinemaScreen( int name, int where )
{
	Work		*work ;
	if(CineWork){ printf("２重起動です\n"); return (void*)1; }
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
//	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
		CineWork = work;
	}
	
	return (void *)work ;
}

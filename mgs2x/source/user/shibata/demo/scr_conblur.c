//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scr_conblur.c
	集中ブラー
	
	2000/12/18 T.Shibata
	
	$Id: scr_conblur.c,v 1.6 2002/11/23 12:09:26 Yoshizawa1 Exp $

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
#include "libutl.h"

#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"
#include "../../mode/demo/eft_con.h"

#include	"../util/ts_util.h"
#include	"../util/dma_set.h"


//#define TEST

#define CLOCK_COUNT	(BP_BASE_TICK())

#define MDL_PRIM	(SCE_GS_SET_PRIM( 5, 0, 1, 0, 1, 0, 1, 0, 0 ))
#define MDL_TEST	(SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 ))
#define MDL_ALPHA	(SCE_GS_SET_ALPHA(0,1,0,1,16))

#define		N_VERTS		(6)

typedef struct {
	DG_DMATAG		dmatag_mdl;			// ＧＩＦ接続ＤＭＡタグ
	MDL_DRAW		mdl_draw;			// 								モデル描画の設定
	DG_DMATAG		dmatag_mverts;		// ＧＩＦ接続ＤＭＡタグ			モデル頂点
	DG_DMATAG		dmatag_test;		// ＧＩＦ接続ＤＭＡタグ
	PACKET_END		end_paket;			//								ピクセルテスト復元
	DG_DMATAG		dmatag_offset;		// オフセット環境復元用
	DG_DMATAG		dmatag_end;			// RETタグ
}WARP_PACK;

typedef	struct	{
	GV_ACT_EX			actor;
	int					name;
	int 				n_mverts;
	int					m_size;
	float				max_pix;
	
	WARP_PACK			*warp_pack[2];
	void				*mverts[2];

	DG_DMAPACK			*dmapack;
	FVECTOR				*mov;
	SVECTOR				*rot;
	float				alpha;

	float				diff_alpha;
	float				next_alpha;
	int					timer;
	int					flags;

#ifndef PSX2
	void				*packet_mem;
	void                *prim ;
	u_int                col  ;
#endif

} Work ;

extern int DM_FrameSkip ;

enum {
	MESG_CBLUR_VIS = 0,		//表示
	MESG_CBLUR_INV,			//非表示
	MESG_CBLUR_ALPHA,		//強度変更
};

#define		CBLUR_FLAGS_VIS		(0x0001)
#define		CBLUR_FLAGS_ALPHA	(0x0002)

#define		MEM_SCR_MPACKE	((void *)(SCRPAD_ADDR))
#define		MEM_SCR_BOTTOM	((void *)(MEM_SCR_MPACKE+sizeof(DG_GIFTAG)+sizeof(MVERT_DATA_F)*N_VERTS))



#define		CINEMA_BELT_UPPER	(16.0f*((float)DRAW_HEIGHT/224.0f))
#define		CINEMA_BELT_LOWER	(48.0f*((float)DRAW_HEIGHT/224.0f))

#define		CON_LEN_OFF_X		(4.0f)
#define		CON_LEN_SCL_X		(4.0f)
#define		CON_LEN_OFF_Y		(4.0f)
#define		CON_LEN_SCL_Y		(4.0f)

#define		CON_BLUR_MIN_X		(MIN_X)
#define		CON_BLUR_MAX_X		(MAX_X)
#define		CON_BLUR_MIN_Y		(MIN_Y + (int)(CINEMA_BELT_UPPER*SHIBATA_FSHIFT))
#define		CON_BLUR_MAX_Y		(MAX_Y - (int)(CINEMA_BELT_LOWER*SHIBATA_FSHIFT))

#define		UNIT_BOUND_MAX_X	( 1.0f)
#define		UNIT_BOUND_MIN_X	(-1.0f)
#define		UNIT_BOUND_MAX_Y	( 1.0f - (CINEMA_BELT_UPPER/((float)DRAW_HEIGHT/2.0f)))
#define		UNIT_BOUND_MIN_Y	(-1.0f + (CINEMA_BELT_LOWER/((float)DRAW_HEIGHT/2.0f)))

#ifdef PSX2  /*!!!!!!!!!!!!!!!!!!!!!!! PSX2でしか使用しない関数!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
static void InitDmaTags( Work *work, WARP_PACK *packet, int which )
{
	// --------  DMATag  ----------------------------
	// ＤＭＡタグ		モデル描画設定
	packet->dmatag_mdl.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(MDL_DRAW) ) ;
	packet->dmatag_mdl.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_mdl.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(MDL_DRAW), 0) ;
	// ＤＭＡタグ		ピクセルテスト復元
	packet->dmatag_test.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(PACKET_END) ) ;
	packet->dmatag_test.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_test.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(PACKET_END), 0) ;
	// オフセット環境復元用
	packet->dmatag_offset.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWOFFSET) ) ;
	packet->dmatag_offset.addr = &( DG_Chanl( 0 )->draw_offset[ which ] ) ;
	packet->dmatag_offset.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_offset.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_DRAWOFFSET), 0) ;
	// RETタグ
	packet->dmatag_end.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 ) ;
	packet->dmatag_end.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_end.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
	
	// ＤＭＡタグ		モデル頂点
	packet->dmatag_mverts.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, work->m_size/sizeof(u_long128) ) ;
	packet->dmatag_mverts.addr = work->mverts[which];
	packet->dmatag_mverts.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_mverts.vifcode[1] = SCE_VIF1_SET_DIRECT( work->m_size/sizeof(u_long128), 0) ;
}

static void InitMVerts( Work *work )
{
	DG_GIFTAG		*giftag = MEM_SCR_MPACKE;
	MVERT_DATA_F	*mverts = (MVERT_DATA_F*)(giftag+1);
	int				i;
	
	giftag->tag  = SCE_GIF_SET_TAG(SIZEOF_QWORD(MVERT_DATA_F)*work->n_mverts, 1, 0, 0, 0, 1);
	giftag->regs = GS_REGS_AD;

	mverts = (MVERT_DATA_F*)(giftag+1);

	mverts[0].xyz.data = SCE_GS_SET_XYZ( (2048<<4), (2048<<4), 0 );
#if 1
	mverts[1].xyz.data = SCE_GS_SET_XYZ( (2048<<4), (2048<<4), 0 );
	mverts[2].xyz.data = SCE_GS_SET_XYZ( (2048<<4), (2048<<4), 0 );
	mverts[3].xyz.data = SCE_GS_SET_XYZ( (2048<<4), (2048<<4), 0 );
	mverts[4].xyz.data = SCE_GS_SET_XYZ( (2048<<4), (2048<<4), 0 );
	mverts[5].xyz.data = SCE_GS_SET_XYZ( (2048<<4), (2048<<4), 0 );
#else
#if 1
	mverts[1].xyz.data = SCE_GS_SET_XYZ( CON_BLUR_MIN_X + (16<<4), CON_BLUR_MIN_Y + (16<<4), 0 );
	mverts[2].xyz.data = SCE_GS_SET_XYZ( CON_BLUR_MAX_X - (16<<4), CON_BLUR_MIN_Y + (16<<4), 0 );
	mverts[3].xyz.data = SCE_GS_SET_XYZ( CON_BLUR_MAX_X - (16<<4), CON_BLUR_MAX_Y - (16<<4), 0 );
	mverts[4].xyz.data = SCE_GS_SET_XYZ( CON_BLUR_MIN_X + (16<<4), CON_BLUR_MAX_Y - (16<<4), 0 );
	mverts[5].xyz.data = SCE_GS_SET_XYZ( CON_BLUR_MIN_X + (16<<4), CON_BLUR_MIN_Y + (16<<4), 0 );
#else
	mverts[1].xyz.data = SCE_GS_SET_XYZ( CON_BLUR_MIN_X + (16<<4), MIN_Y + (16<<4), 0 );
	mverts[2].xyz.data = SCE_GS_SET_XYZ( CON_BLUR_MAX_X - (16<<4), MIN_Y + (16<<4), 0 );
	mverts[3].xyz.data = SCE_GS_SET_XYZ( CON_BLUR_MAX_X - (16<<4), MAX_Y - (16<<4), 0 );
	mverts[4].xyz.data = SCE_GS_SET_XYZ( CON_BLUR_MIN_X + (16<<4), MAX_Y - (16<<4), 0 );
	mverts[5].xyz.data = SCE_GS_SET_XYZ( CON_BLUR_MIN_X + (16<<4), MIN_Y + (16<<4), 0 );
#endif
#endif
	//中心
	mverts[0].uv.data = SCE_GS_SET_UV( (DRAW_WIDTH/2)<<4,(DRAW_HEIGHT/2)<<4 );
	//右上
	mverts[1].uv.data = SCE_GS_SET_UV( MIN_UV, MIN_UV + ((int)(CINEMA_BELT_UPPER)<<4) );
	//左上
	mverts[2].uv.data = SCE_GS_SET_UV( MAX_U,  MIN_UV + ((int)(CINEMA_BELT_UPPER)<<4) );
	//左下
	mverts[3].uv.data = SCE_GS_SET_UV( MAX_U,  MAX_V  - ((int)(CINEMA_BELT_LOWER)<<4) );
	//右下
	mverts[4].uv.data = SCE_GS_SET_UV( MIN_UV, MAX_V  - ((int)(CINEMA_BELT_LOWER)<<4) );
	//右上
	mverts[5].uv.data = SCE_GS_SET_UV( MIN_UV, MIN_UV + ((int)(CINEMA_BELT_UPPER)<<4) );

	for( i = 0; i < N_VERTS; i++ ){
		mverts->xyz.reg = (i<2)?SCE_GS_XYZ3:SCE_GS_XYZ2;
		mverts->uv.reg = SCE_GS_UV;

		mverts++;
	}

	TS_Scr_Mem( work->mverts[0], MEM_SCR_MPACKE, sizeof(char), work->m_size );
	TS_Scr_Mem( work->mverts[1], MEM_SCR_MPACKE, sizeof(char), work->m_size );
}
#endif  /*!!!!!!!!!!!!!!!!!!!!!!! PSX2でしか使用しない関数!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


static inline void _RotTrans( FVECTOR *out, FMATRIX *world, FVECTOR *in )
{
#ifdef BP_PSX2_ASM
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	lqc2				vf7,0x30(%1)
	vmulax.xyzw			ACC, vf4,vf8
	vmadday.xyzw		ACC, vf5,vf8
	vmaddaz.xyzw		ACC, vf6,vf8
	vmaddw.xyzw			vf8, vf7,vf8
	sqc2				vf8,0x00(%0)
	": : "r"(out), "r"(world), "r"(in) );//:"memory" );
#else
	DG_SetPos( world ) ;
	DG_RotVectorW(in, out, 1 ) ;
#endif
}

static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;
	float dmdm;//yano 2002.03.22

	mes_num=GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case MESG_CBLUR_VIS:		//表示
			work->flags |= CBLUR_FLAGS_VIS;
			work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE;
			break;
		  case MESG_CBLUR_INV:		//非表示
			work->flags &= ~CBLUR_FLAGS_VIS;
			work->dmapack->flag |= DG_DMAPACK_INVISIBLE;
			break;
		  case MESG_CBLUR_ALPHA:	//強度変更
			work->flags |= CBLUR_FLAGS_ALPHA;
			work->next_alpha = (float)msg->message[1];
			work->timer = msg->message[2];
			//yano 2002.03.22 0DIV
			if( work->timer == 0 ) dmdm = 0.0000001f; 
			else dmdm = (float)work->timer;
			work->diff_alpha = ( work->next_alpha - work->alpha )/ dmdm;
			break;
		  default:
			printf("scr_conblur.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}

	return work->flags;
}

static void CheckDemoScrBound( FVECTOR *out, FVECTOR *check )
{
#define		CHECK_DIFF1		(UNIT_BOUND_MAX_Y/UNIT_BOUND_MAX_X)		//第一象限
#define		CHECK_DIFF2		(UNIT_BOUND_MAX_Y/UNIT_BOUND_MIN_X)		//第二象限
#define		CHECK_DIFF3		(UNIT_BOUND_MIN_Y/UNIT_BOUND_MIN_X)		//第三象限
#define		CHECK_DIFF4		(UNIT_BOUND_MIN_Y/UNIT_BOUND_MAX_X)		//第四象限

	float		a;

#ifdef PSX2
	a = check->vy/check->vx;
#else
	a = check->vx!=0.0f ? check->vy/check->vx : 0.0f ;
#endif

	if( (CHECK_DIFF4 < a && a < CHECK_DIFF1) || (CHECK_DIFF2 < a && a < CHECK_DIFF3) ){
		//ｘ軸との交差
		if( check->vx > 1.0f ){
			out->vy = a;
			out->vx = 1.0f;
		}else if( check->vx < -1.0f ){
			out->vy = -1.0f * a;
			out->vx = -1.0f;
		}
	}else{
		//ｙ軸との交差
		if( check->vy > UNIT_BOUND_MAX_Y ){
			out->vx = UNIT_BOUND_MAX_Y*check->vx/check->vy;
			out->vy = UNIT_BOUND_MAX_Y;
		}else if( check->vy < UNIT_BOUND_MIN_Y ){
			out->vx = UNIT_BOUND_MIN_Y*check->vx/check->vy;
			out->vy = UNIT_BOUND_MIN_Y;
		}
	}
}

static void Act( Work *work )
{
	DG_GIFTAG		*giftag = (DG_GIFTAG*)work->mverts[DG_Clock];
	MVERT_DATA_F	*mverts = (MVERT_DATA_F*)(giftag+1);
	
//	MVERT_DATA_F	*mverts = (MVERT_DATA_F*)(((DG_GIFTAG*)work->mverts[DG_Clock])+1);
	FVECTOR			con_pos;
	float			fcu,fcv,fcx,fcy,fx0,fy0,fx1,fy1;
#ifdef PSX2
	int				icu,icv,icx,icy,ix0,iy0,ix1,iy1;
#endif

	if( !CheckMesgParam( work ) ){
		//スリープ
		GV_WaitMessage( work, work->name );
	}

	if( work->flags & CBLUR_FLAGS_ALPHA ){
		work->alpha += work->diff_alpha * (float)CLOCK_COUNT * (float)( DM_FrameSkip + 1 );
		work->timer -= CLOCK_COUNT * ( DM_FrameSkip + 1 );
		if( work->timer < 0 ){
			work->alpha = work->next_alpha;
			work->timer = 0;
			work->flags &= ~CBLUR_FLAGS_ALPHA;
		}
	}

#ifndef PSX2
	work->prim = work->dmapack->autopacket ;
	work->prim = DG_SetDmapackUseFrameTex( work->prim, 1 ) ;
	work->prim = DG_SetDmapackAlpha( work->prim, SCE_GS_SET_ALPHA(0,1,2,1,work->col&0xff) ) ;
	_RotTrans( &con_pos, &DG_Chanls[0].eye_pers, work->mov );
	if ( con_pos.vw == 0.0f )
	  con_pos.vw = 0.00001f;
#else
	work->warp_pack[DG_Clock]->mdl_draw.data.alpha.data = SCE_GS_SET_ALPHA(0,1,2,1,((int)work->alpha));
	_RotTrans( &con_pos, &DG_Chanls[0].eye_pers, work->mov );
#endif
	con_pos.vx = con_pos.vx / con_pos.vw;
	con_pos.vy = con_pos.vy / con_pos.vw;

	//ここにチェックを入れる
	CheckDemoScrBound( &con_pos, &con_pos );

	fx0 = con_pos.vx * work->max_pix + work->max_pix;
	fx1 = con_pos.vx * work->max_pix - work->max_pix;
	fy0 = con_pos.vy * work->max_pix + work->max_pix;
	fy1 = con_pos.vy * work->max_pix - work->max_pix;

	fcu = con_pos.vx * DRAW_WIDTH_HALF + DRAW_WIDTH_HALF;
	fcv = con_pos.vy * DRAW_HEIGHT_HALF + DRAW_HEIGHT_HALF;
	
#ifdef PSX2
	fcx = 2048.0f + con_pos.vx * DRAW_WIDTH_HALF;
	fcy = 2048.0f + con_pos.vy * DRAW_HEIGHT_HALF;
	icx = (int)(fcx * SHIBATA_FSHIFT);
	ix0 = CON_BLUR_MIN_X + (int)(fx0 * SHIBATA_FSHIFT);
	ix1 = CON_BLUR_MAX_X + (int)(fx1 * SHIBATA_FSHIFT);
	icy = (int)(fcy * SHIBATA_FSHIFT);
	iy0 = CON_BLUR_MIN_Y + (int)(fy0 * SHIBATA_FSHIFT);
	iy1 = CON_BLUR_MAX_Y + (int)(fy1 * SHIBATA_FSHIFT);

	icu = (int)(fcu * SHIBATA_FSHIFT);
	icv = (int)(fcv * SHIBATA_FSHIFT);

	mverts[0].uv.data = SCE_GS_SET_UV( icu, icv );	

	mverts[0].xyz.data = SCE_GS_SET_XYZ( icx, icy, 0 );
	mverts[1].xyz.data = SCE_GS_SET_XYZ( ix0, iy0, 0 );
	mverts[2].xyz.data = SCE_GS_SET_XYZ( ix1, iy0, 0 );
	mverts[3].xyz.data = SCE_GS_SET_XYZ( ix1, iy1, 0 );
	mverts[4].xyz.data = SCE_GS_SET_XYZ( ix0, iy1, 0 );
	mverts[5].xyz.data = SCE_GS_SET_XYZ( ix0, iy0, 0 );
#else
	 
	fcx = DRAW_WIDTH_HALF  + con_pos.vx * DRAW_WIDTH_HALF  ;
	fcy = DRAW_HEIGHT_HALF + con_pos.vy * DRAW_HEIGHT_HALF ;
	fcu = DG_FRAME_U( fcu ) ;
	fcv = DG_FRAME_V( fcv ) ;

	fx0 = CON_BLUR_MIN_X + (fx0 * SHIBATA_FSHIFT);
	fx1 = CON_BLUR_MAX_X + (fx1 * SHIBATA_FSHIFT);
	fy0 = CON_BLUR_MIN_Y + (fy0 * SHIBATA_FSHIFT);
	fy1 = CON_BLUR_MAX_Y + (fy1 * SHIBATA_FSHIFT);

	work->prim = DG_SetDmapackTriangleStrip( work->prim, 4 );
	work->prim = DG_SetDmapackVertex( work->prim, fx0,fy0,
									  DG_FRAME_U(CON_BLUR_MIN_X),DG_FRAME_V(CON_BLUR_MIN_Y),
									  SCE_GS_SET_RGBAQ( ((work->col>>24)&0xff),
													    ((work->col>>16)&0xff),
													    ((work->col>> 8)&0xff), 
													    ((work->col    )&0xff), 0x3F800000 ) ) ;
	work->prim = DG_SetDmapackVertex( work->prim, fcx, fcy,
									  fcu, fcv,
									  SCE_GS_SET_RGBAQ( ((work->col>>24)&0xff),
													    ((work->col>>16)&0xff),
													    ((work->col>> 8)&0xff),
													    ((work->col    )&0xff), 0x3F800000 ) ) ;
	work->prim = DG_SetDmapackVertex( work->prim, fx1,fy0,
									  DG_FRAME_U(CON_BLUR_MAX_X),DG_FRAME_V(CON_BLUR_MIN_Y),
									  SCE_GS_SET_RGBAQ( ((work->col>>24)&0xff),
													    ((work->col>>16)&0xff),
													    ((work->col>> 8)&0xff),
													    ((work->col    )&0xff), 0x3F800000 ) ) ;
	work->prim = DG_SetDmapackVertex( work->prim, fx1,fy1,
									  DG_FRAME_U(CON_BLUR_MAX_X),DG_FRAME_V(CON_BLUR_MAX_Y),
									  SCE_GS_SET_RGBAQ( ((work->col>>24)&0xff),
													    ((work->col>>16)&0xff),
													    ((work->col>> 8)&0xff),
													    ((work->col    )&0xff), 0x3F800000 ) ) ;

	work->prim = DG_SetDmapackTriangleStrip( work->prim, 4 );
	work->prim = DG_SetDmapackVertex( work->prim, fx1,fy1,
									  DG_FRAME_U(CON_BLUR_MAX_X),DG_FRAME_V(CON_BLUR_MAX_Y),
									  SCE_GS_SET_RGBAQ( ((work->col>>24)&0xff),
													    ((work->col>>16)&0xff),
													    ((work->col>> 8)&0xff),
													    ((work->col    )&0xff), 0x3F800000 ) ) ;
	work->prim = DG_SetDmapackVertex( work->prim, fcx, fcy,
									  fcu, fcv,
									  SCE_GS_SET_RGBAQ( ((work->col>>24)&0xff),
													    ((work->col>>16)&0xff),
													    ((work->col>> 8)&0xff),
													    ((work->col    )&0xff), 0x3F800000 ) ) ;
	work->prim = DG_SetDmapackVertex( work->prim, fx0,fy1,
									  DG_FRAME_U(CON_BLUR_MIN_X),DG_FRAME_V(CON_BLUR_MAX_Y),
									  SCE_GS_SET_RGBAQ( ((work->col>>24)&0xff),
													    ((work->col>>16)&0xff),
													    ((work->col>> 8)&0xff),
													    ((work->col    )&0xff), 0x3F800000 ) ) ;
	work->prim = DG_SetDmapackVertex( work->prim, fx0,fy0,
									  DG_FRAME_U(CON_BLUR_MIN_X),DG_FRAME_V(CON_BLUR_MIN_Y),
									  SCE_GS_SET_RGBAQ( ((work->col>>24)&0xff),
													    ((work->col>>16)&0xff),
													    ((work->col>> 8)&0xff),
													    ((work->col    )&0xff), 0x3F800000 ) ) ;
	DG_SetDmapackEnd( work->prim ) ;

//printf( "scr_conblur.c need Size%d\n",  (int)work->prim-(int)work->packet_mem  ) ;

#endif

	
#if 0
	{
		FMATRIX	temp;
		DG_SetPos2( work->mov, work->rot );
		DG_GetPos( &temp );

		HZX_ViewMatrix( &temp, 500.0f );
	}
#endif
}

static void Die( Work *work )
{
	if(work->dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
	
#ifdef PSX2	
	if(work->warp_pack[0]) GV_DelayedFree(work->warp_pack[0]);
#else
	if(work->packet_mem) GV_DelayedFree( work->packet_mem );
#endif
	if(work->mverts[0]) GV_DelayedFree(work->mverts[0]);

}

#ifdef KP_WINDOWS
static void DmyAct( Work *work ){}
static void DmyDie( Work *work ){}
#endif

static int GetResources( Work *work, int con_name, u_int col, float max_pix )
{
	DG_DMAPACK		*dmapack;
	u_char			*verts;
	EFTCONTROL		*ctrl = DM_GetEftControl( con_name );
#ifdef PSX2
	WARP_PACK		*packet;
	int				i;
#endif


	if( !ctrl ) return -1;
	work->mov = &ctrl->mov;
	work->rot = &ctrl->rot;

	work->max_pix = max_pix/2.0f;
	work->n_mverts = N_VERTS;	//モデルの頂点数
	work->m_size = sizeof(DG_GIFTAG)+sizeof(MVERT_DATA_F)*work->n_mverts;
	work->alpha = (float)(col&0xff);
	work->flags = CBLUR_FLAGS_VIS;

	// 頂点データメモり確保
	verts = GV_Malloc( work->m_size*2 );
	if(verts == NULL){ printf("ERR!! MALLOC MVERTS!!\n"); return -1; }

	work->mverts[0] = (MVERT_DATA_F*)&verts[0];
	work->mverts[1] = (MVERT_DATA_F*)&verts[work->m_size];
	
	// ＤＭＡパケット型オブジェクト作成
	work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL, DG_DMAPACK_PHASE_AFTER, 144 );
	if(dmapack==NULL){ printf("ERR!! MAKE DMAPACK!!\n"); return -1; }
	DG_QueueDmapack( dmapack );

#ifdef PSX2
	// パケットメモリ割り当て
	packet = GV_Malloc( sizeof(WARP_PACK) * 2 );
	if(packet == NULL){ printf("ERR!! MALLOC WARP_PACK!!\n"); return -1; }
	
	work->warp_pack[0] = dmapack->packet[0] = &packet[0] ;
	work->warp_pack[1] = dmapack->packet[1] = &packet[1] ;

	for ( i = 0 ; i < 2 ; i++ ){
		InitDmaTags( work, &packet[i], i );
		InitMdlDraw( &packet[i].mdl_draw, MDL_TEST, MDL_ALPHA, MDL_PRIM );
		InitEndPacket( &packet[i].end_paket );

		packet[i].mdl_draw.data.alpha.data = SCE_GS_SET_ALPHA(0,1,2,1,col&0xff);
		packet[i].mdl_draw.data.rgbq.data =
			SCE_GS_SET_RGBAQ( ((col>>24)&0xff), ((col>>16)&0xff), ((col>>8)&0xff), 0x80, 0x3F800000);
#if 0
		packet[i].mdl_draw.data.alpha.data = SCE_GS_SET_ALPHA(0,1,2,1,64);
		//packet[i].mdl_draw.data.rgbq.data = SCE_GS_SET_RGBAQ(0,0,0,0x80,0x3F800000);
#endif
		//ブラーモード
		packet[i].mdl_draw.data.tex0.data = SCE_GS_SET_TEX0( BUFFER_PAGE(1-i)/64,
															 BUFFER_WIDTH/64,
															 FRAME_BUFFER_COLOR_MODE(),
															 10,10,0,0,0,0,0,0,0);
	}
	InitMVerts( work );
#else

	work->col = col ;
	if ( !(work->packet_mem = GV_Malloc( 180 )) ) {
		printf("ERR!! MALLOC scr_water_demo.c!!\n");
		return -1 ;
	}
	dmapack->autopacket = work->packet_mem ;
	DG_SetDmapackEnd( dmapack->autopacket ) ;

	//printf( "scr_water_demo.c need Size%d\n",  (int)prim-(int)work->packet_mem  ) ;

#endif

	
	return (0);
}

void *NewScrConcentrateBlur( int con_name, u_int col, float max_pix, int name )
{
	Work		*work ;

	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );

		work->name = name;
		if ( GetResources( work, con_name, col, max_pix ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
#else
	    if ( DG_CheckUseBlur() ) {
			GV_SetActor( &( work->actor ), Act, Die );
			GV_ActorEX( &work->actor );

			work->name = name;
			if ( GetResources( work, con_name, col, max_pix ) < 0 ) {
				GV_DestroyActor( work );
				return NULL;
			}
		} else {
			GV_SetActor( &( work->actor ), DmyAct, DmyDie );	// Dmy
			GV_ActorEX( &work->actor );
			work->name = name;
		}
#endif
	}
	return (void *)work ;
}

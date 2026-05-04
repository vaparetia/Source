//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scr_drop.c
	歪みモデルを使った奴(歪み変化)

	2001/08/15 T.Shibata
	
	$Id: scr_trans.c,v 1.4 2002/11/23 12:36:04 Yoshizawa1 Exp $

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
#include	"camera.h"
#include	"def_dma.h"
#include	"../util/ts_util.h"
#include	"../util/dma_set.h"


#define CLOCK_COUNT	(BP_BASE_TICK())

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

typedef struct {
	DG_DMATAG		dmatag_flush0;		// ＧＩＦ接続ＤＭＡタグ			フラッシュ
	DROW_FLUSH 		drow_flush0;		//								フラッシュ
	DG_DMATAG		dmatag_sys;			// ＧＩＦ接続ＤＭＡタグ			退避
	DG_DMATAG		dmatag_flush1;		// ＧＩＦ接続ＤＭＡタグ			フラッシュ
	DROW_FLUSH 		drow_flush1;		//								フラッシュ
	DG_DMATAG		dmatag_end;			// RETタグ
} FOR_TRANS_PACK;

typedef	struct	{
	GV_ACT_EX		actor;
	ALIGN16_PRE FOR_TRANS_PACK	*trans_pack[2] ALIGN16_POST;
	DG_DMAPACK		*dmapack;

#ifndef PSX2
	void				*packet_mem;
#endif

} Work;

extern float GM_WaterLevel;
static Work *g_work = NULL;

static void Act( Work *work )
{
#ifdef KP_XBOX
	if ( DG_Chanls[0].eye.m[3][1] > GM_WaterLevel || GM_IsGameOver() ) {
		work->dmapack->flag |=  DG_DMAPACK_INVISIBLE ;
	} else {
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE ;
	}
#endif
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	if(work->dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
	
#ifdef PSX2
	if(work->trans_pack[0]) GV_DelayedFree(work->trans_pack[0]);
#else
	if(work->packet_mem) GV_DelayedFree( work->packet_mem );
#endif
	g_work = NULL;
}


#ifdef PSX2  /*!!!!!!!!!!!!!!!!!!!!!!! PSX2でしか使用しない関数!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
static void InitDmaTags( Work *work, FOR_TRANS_PACK *packet, int which )
{
	// ＤＭＡタグ		フラッシュ
	packet->dmatag_flush0.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DROW_FLUSH) );
	packet->dmatag_flush0.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_flush0.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DROW_FLUSH), 0);
	
	// ＤＭＡタグ		フラッシュ
	packet->dmatag_flush1.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DROW_FLUSH) );
	packet->dmatag_flush1.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_flush1.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DROW_FLUSH), 0);

	// RETタグ
	packet->dmatag_end.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 );
	packet->dmatag_end.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_end.vifcode[1] = SCE_VIF1_SET_NOP( 0 );
}
#endif  /*!!!!!!!!!!!!!!!!!!!!!!! PSX2でしか使用しない関数!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

static int GetResources( Work *work )
{
	DG_DMAPACK		*dmapack;
#ifdef PSX2
	FOR_TRANS_PACK	*packet;
	int				i;
#else
	void *prim ;
#endif

	// ＤＭＡパケット型オブジェクト作成
	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL|DG_DMAPACK_INVISIBLE1|DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3,
											   DG_DMAPACK_PHASE_AFTER, 142 );
	if(dmapack == NULL){ printf("ERR!! MAKE DMAPACK!!\n"); return -1; }
	//dmapack->flag |= DG_DMAPACK_INVISIBLE0;
	DG_QueueDmapack( dmapack );

#ifdef PSX2
	// パケットメモリ割り当て
	packet = GV_Malloc( sizeof(FOR_TRANS_PACK) * 2 );
	if(packet == NULL){ printf("ERR!! MALLOC DROPS_PACK!!\n"); return -1; }
	work->trans_pack[0] = dmapack->packet[0] = &packet[0];
	work->trans_pack[1] = dmapack->packet[1] = &packet[1];

	for ( i = 0 ; i < 2 ; i++ ){
		InitDmaTags( work, &packet[i], i );
		InitDrowFlush( &packet[i].drow_flush0 );
		InitDrowFlush( &packet[i].drow_flush1 );
		DG_WritePacket_SoftImageTurnOut( &packet[i].dmatag_sys, i );		
	}
#else
	{
		int xbox_packet_size = 256;/* 下の＊＊＊のprintf文のサイズ分が必要 */		
		if ( !(work->packet_mem = GV_Malloc( xbox_packet_size )) ) {
			return -1 ;
		}
		
#define SPR_ALPHA_MODE	(SCE_GS_SET_ALPHA(0,1,2,1,0x40))
		
		dmapack->autopacket = work->packet_mem ;
		prim = dmapack->autopacket ;
		prim = DG_SetDmapackBackupFrame( prim, 3 ) ;
		prim = DG_SetDmapackUseFrameTex( prim, 2 ) ;
		prim = DG_SetDmapackAlpha( prim, SPR_ALPHA_MODE ) ;
		prim = DG_SetDmapackSprt( prim,
								 MIN_X,MIN_Y, MIN_UV, MIN_UV/* + PIX_SHIFT_V*/,
								 MAX_X,MAX_Y, MAX_U , MAX_V /* + PIX_SHIFT_V*/,
								 SCE_GS_SET_RGBAQ(0x808080,0,0,128,0)) ;
		DG_SetDmapackEnd( prim ) ;
		
		printf( "scr_trans.c need Size%d\n",  (int)prim-(int)work->packet_mem  ) ;/* ＊＊＊ */
		if( xbox_packet_size < ((int)prim-(int)work->packet_mem) ){
			printf("xbox_packet_size not enough!!\n");
			ASSERT( 0 );
		}
	}
#endif

	return (0);
}

#ifdef KP_WINDOWS
static void DmyAct( Work *work ){}
static void DmyDie( Work *work ){}
#endif

void *NewScrTrans( int name, int map )
{
	Work		*work ;

	//return (void*)1;
	if(g_work){
		return g_work;
	}

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &work->actor, Act, Die );
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
#else
		if ( DG_CheckUseBlur() ) {
			GV_SetActor( &work->actor, Act, Die );
			GV_ActorEX( &work->actor ) ;
			if ( GetResources( work ) < 0 ) {
				GV_DestroyActor( work );
				return NULL;
			}
		} else {
			GV_SetActor( &work->actor, DmyAct, DmyDie );
			GV_ActorEX( &work->actor ) ;
		}
#endif
		g_work = work;
	}

	return (void *)work ;
}

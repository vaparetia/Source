//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
  cdc_movie.c
  無線中ムービー再生機構

  2001/05/30  Y.Kira
  $Id: cdc_movie.c,v 1.5 2002/11/23 12:09:26 Yoshizawa1 Exp $
*/
#endif

#ifdef PSX2
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
#ifdef KP_XBOX
#include <xtl.h>
#endif

#include "BP_MovieSupport.h"
#include "../../../game/ipu.h"

#include "gameheader.h"
#include "font.h"
#include "libdg.h"
#include "libdg.cnf"
#include "dmapack.h"
#include "def_dma.h"
#include "ipu.h"

#define _cdc_movie_c_
#include "sprite_2d.h"
#include "cdc_movie.h"
#include "codecmem.h"
#include "codec_signal.h"

#ifdef KP_XBOX
#ifdef KP_WINDOWS
#include "winstream.h"	
#else
#include "cristream.h"	
#endif
#endif

#ifdef PSX2
#ifdef DEBUG
#define DBG(...)   printf(__VA_ARGS__)
#else
#define DBG(...)
#endif /* DEBUG */
#endif
#ifdef KP_XBOX
#define DBG
#endif

typedef struct {
   DG_DMAPACK_TEX texture;
   DG_DMAPACK_ALPHA alpha;
   DG_DMAPACK_SPRT_F sprt;
   DG_DMAPACK_TAG end;
} DRAW_PACKET;


typedef struct {
	GV_ACT_EX    actor;
	SPR_OBJ    * guide;         /* 表示位置,サイズのガイドとなる SP_BOX */

	float          x0, y0;
	float          x1, y1;

	int          finish;        /* 1 になったら終了 */  
	DG_DMAPACK * dmapack;

   DRAW_PACKET draw_packet;
   BP_Movie_Handle movie_handle;

	int           pre_disp:1;
	int           disp:1;
} Work;


static Work * now_work = NULL;


/*
 * ムービー表示タスクの終了
 */
void CDC_KillMovie(void)
{
	Work * work = now_work;

	if(NULL == work) return;
	work->finish = 1;
}

void CDC_SetMovieDispStatus(int sw)
{
	Work * work = now_work;

	if(NULL == work) return;
	work->disp = sw;
	work->disp = 1;
}

static void make_draw_packet(Work * work, DRAW_PACKET* draw)
{
   DG_TEX* dgTex = BP_Movie_Get_Texture(work->movie_handle);

	/* ガイド用オブジェクトの指す座標に従い, 表示領域を決定する。*/
	work->x0 = (work->guide->box.rect.begin.x * (float)DRAW_WIDTH / 512.0F);
	work->y0 = (work->guide->box.rect.begin.y * (float)DRAW_HEIGHT / 384.0F);
  
	work->x1 = (work->guide->box.rect.end.x * (float)DRAW_WIDTH / 512.0F) - 1;
   work->y1 = (work->guide->box.rect.end.y * (float)DRAW_HEIGHT / 384.0F) - 1;
	
   DG_SetDmapackTex(&draw->texture, dgTex);
   DG_SetDmapackAlpha( &draw->alpha, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

	
	DG_SetDmapackSprt_F( &draw->sprt,
							  work->x0, work->y0, 0.f, 0.f,
							  work->x1, work->y1, 1.f, 1.f, 
                       DG_MakeDmaPackColorFromInt(0x80808080),0);
	
   DG_SetDmapackEnd(&draw->end);
}

static void setup_dma(Work * work)
{
	DG_DMAPACK * dmapack;
	
	DBG("entering setup_dma()\n");
  
	dmapack = DG_MakeDmapack2(DG_DMAPACK_MENU | DG_DMAPACK_PRIVILEGE,
							  DG_DMAPACK_PHASE_AFTER, 200);
	dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU;

   make_draw_packet(work, &work->draw_packet);
	dmapack->autopacket = &work->draw_packet;

	work->dmapack = dmapack;
	work->disp = 0;
	work->pre_disp = 0;
}


static void Act(Work * work)
{
	if(work->pre_disp != work->disp)  /* 表示状態の切替えがあった場合 */
    {
		if(work->disp)    /* 表示 */
			DG_QueueDmapack(work->dmapack);
		else              /* 非表示 */
			DG_DequeueDmapack(work->dmapack);

		work->pre_disp = work->disp;
    }

	/* フレームデータがあれば取得する */
	if( GM_StreamGetIpu() ) {
		   GM_StreamFreeIpu();
		}
 
	/* 終了指令が出ていれば,終了する。*/
	if(work->finish)
    {
		GV_CallParentSignalFunc(work, CDC_SIGNAL_MOVIE_DIE, 0);
		GV_DestroyActor(work);
    }
}

static void Die(Work * work)
{
	DG_DequeueDmapack(work->dmapack);
	DG_FreeDmapack(work->dmapack);
	GM_StreamIpuDriverEnd();

	if(work == now_work) now_work = NULL;
}

static int ReceiveSignal(void * workp, int signal, int value)
{
	Work * work = workp;
	int ret = 0;

	switch(signal)
    {
    case CDC_SIGNAL_MOVIE_KILL:
		work->finish = 1;
		break;
    default:
		ret = GV_DefaultSignalFunc(workp, signal, value);
		break;
    }
	return ret;
}


static int GetResources(Work * work, SPR_OBJ * guide)
{
	int i;

	if(NULL != now_work) return -1;

	work->guide = guide;
	work->finish = 0;

	/* IPU ストリームドライバを起動 */
	work->movie_handle = GM_StreamIpuDriverInit(CMOVIE_WIDTH, CMOVIE_HEIGHT);

	DBG("---<2.5>\n");
	setup_dma(work);
	DBG("---<3>\n");

	/* 終了せよシグナルを受け取るための窓口が必要 */
	GV_SetActorSignalFunc(work, ReceiveSignal);
	DBG("---<4>\n");

	now_work = work;
	return 0;
}

void * NewCodecMovie(SPR_OBJ * guide)
{
	Work * work;

	if(NULL == (work = codecActorPrio(GV_ACTOR_MANAGER, sizeof(Work), 0xF0 )))
		return NULL;

	GV_SetActor(&(work->actor), Act, Die);
	GV_ActorEX(&(work->actor));

	if(GetResources(work, guide))
    {
		GV_DestroyActor(work);
		return NULL;
    }
	return work;
}

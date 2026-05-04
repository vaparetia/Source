//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  rdr_movie.c
  レーダー子画面ムービー

  2001/06/26  Y.Kira
  $id:$
*/
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

#include "BP_MovieSupport.h"
#include "../../../game/ipu.h"

#include "gameheader.h"
#include "font.h"
#include "libdg.h"
#include "libdg.cnf"

#include "g_define.h"
#include "libfs.h"
#include "dmapack.h"
#include "def_dma.h"
#include "ipu.h"
#include "strctrl.h"

#define _rdr_movie_c_
#include "radsize.h"
#include "rdr_movie.h"
#include "sprite_2d.h"

#ifdef KP_XBOX
#ifdef KP_WINDOWS
#include "winstream.h"	
#else
#include "cristream.h"	
#endif
#endif


typedef struct {
   DG_DMAPACK_TEX texture;
   DG_DMAPACK_ALPHA alpha;
   DG_DMAPACK_SPRT sprt;
   DG_DMAPACK_TAG end;
} DRAW_PACKET;


/* =========================================================================
   Work
   ========================================================================= */
typedef struct {
	GV_ACT_EX     actor;

	int           movie;         /* 再生するムービーの名称                     */
	int           handle;        /* ストリーミングハンドル                     */

	int           x0, y0;        /* ムービー表示領域左上(実画面上pixel単位)    */
	int           x1, y1;        /* ムービー表示領域右下(実画面上pixel単位)    */

	int           width;         /* ムービーサイズ 幅                          */
	int           height;        /* ムービーサイズ 高さ                        */

	DG_DMAPACK  * dmapack;       /* 転送用の dmapack                           */

   DRAW_PACKET draw_packet;
   BP_Movie_Handle movie_handle;

	int           chanl;         /* 描画チャネル指定                           */
	unsigned char r, g, b, a;    /* 描画 RGBA 値                               */

	int           proc_id;       /* 終了 proc ID                               */

	int           pre_disp:1;
	int           disp:1;
	int           finish:1;      /* 非0 になったら終了                         */

} RdrWork;


static RdrWork * now_work = NULL;

/*
 * レーダームービーの強制停止
 */
void RDR_MovieForceBreak(void)
{
	RdrWork * work = now_work;
	if(NULL == work) return;

	work->finish = 1;
}


static void make_draw_packet(RdrWork * work, DRAW_PACKET* draw)
{
   DG_TEX* dgTex = BP_Movie_Get_Texture(work->movie_handle);

   DG_SetDmapackTex(&draw->texture, dgTex);
   DG_SetDmapackAlpha( &draw->alpha, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );


   DG_SetDmapackSprt( &draw->sprt,
      (float)work->x0, (float)work->y0, 0.f, 0.f,
      (float)work->x1, (float)work->y1, 1.f, 1.f, 
      DG_MakeDmaPackColorFromInt(0x80808080));

   DG_SetDmapackEnd(&draw->end);
}

static void Act(RdrWork * work)
{
	extern int GetRadarDispStatus(void);
	int disable = 0;   /* ステータスとして表示が禁止されている場合は,
						  いかなる条件下においても表示は禁止される。 */

	/*
	 * SELECT ボタンが押されていたら,そのフレームに無線に入るものとして、
	 * 終了プロシージャを呼んだ上でムービーを終了する。
	 */
	if(!(GV_PauseLevel & (GV_PAUSE_PAUSE | GV_PAUSE_STOP)))
		if(GV_PadDataDirect[0].status & PAD_SEL)
		{
			/* 終了 proc が指定されていれば、それを call する */
			if(work->proc_id != 0)
			{
				int proc_id = work->proc_id;
				work->proc_id = 0;
				GCL_ExecProc(proc_id, NULL);
			}
			GV_DestroyActor(work);   /* 終り */
			return;
		}

	/* 基本的に子画面の表示が禁止されている場合は表示を禁止する */
	if(GM_CheckMenuStatus(MENU_SUBWIN_OFF)) disable = 1;

	/*
	 * レーダーが表示されているかを見て,ムービーを表示するかどうかを決定する
	 */
	if(!disable && GetRadarDispStatus())
    {
		/* レーダーが表示されているならば表示 */
		if(!work->disp) DG_QueueDmapack(work->dmapack);
		work->disp = 1;
    }
	else
    {
		/* 表示を禁止されるケース */
		if(work->disp) DG_DequeueDmapack(work->dmapack);
		work->disp = 0;
    }
  
	/*
	 * ストリーミングのステータスを監視し、ムービーが終了していたら
	 * 終了ステータスを有効にする
	 */
	if(GM_StreamStatus(work->handle) == GM_STREAM_STATE_END)
    {
		work->finish = 1;
		/*
		  GM_StreamStop(work->handle);
		  work->handle = GM_IpuStream(work->movie, RDR_STREAM_CH);
		*/
    }

	if(work->finish)
    {
		/* 終了 proc が指定されていれば、それを call する */
		if(work->proc_id != 0)
		{
			int proc_id = work->proc_id;
			work->proc_id = 0;
			GCL_ExecProc(proc_id, NULL);
		}
		GV_DestroyActor(work);   /* 終り */
		return;
    }

	/*
	 * ムービーの再生
	 */
	if(GM_StreamGetIpu())
    {	
		GM_StreamFreeIpu();
    }
}


static void Die(RdrWork * work)
{
	if(work->disp) 
      DG_DequeueDmapack(work->dmapack);

	/* 終りならストリーミング終了 */
	GM_StreamStop(work->handle);
	DG_FreeDmapack(work->dmapack);
	GM_StreamIpuDriverEnd();

   now_work = NULL;   /* お亡くなり。 */
}

/*
 * ムービー再生の準備をする
 */
static int setup_movie(RdrWork * work)
{
   work->movie_handle = GM_StreamIpuDriverInit(work->width, work->height);  
	return 0;
}

#define ALL_MASK (DG_DMAPACK_INVISIBLE0|DG_DMAPACK_INVISIBLE1| \
                  DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3| \
                  DG_DMAPACK_INVISIBLEMENU)

static int setup_dma(RdrWork * work)
{
	static int chanl_mask[] = {
		DG_DMAPACK_INVISIBLE0,
		DG_DMAPACK_INVISIBLE1,
		DG_DMAPACK_INVISIBLE2,
		DG_DMAPACK_INVISIBLE3,
		DG_DMAPACK_INVISIBLEMENU
	};

	DG_DMAPACK * dmapack;
	int i;
	int flag;

	if(4 == work->chanl) 
      flag = DG_DMAPACK_MENU;
	else 
      flag = DG_DMAPACK_NORMAL;

	dmapack = DG_MakeDmapack2(flag | ALL_MASK, DG_DMAPACK_PHASE_AFTER, 145);
	dmapack->flag &= ~chanl_mask[ work->chanl ];

   make_draw_packet(work, &work->draw_packet);
   dmapack->autopacket = &work->draw_packet;
  
	work->dmapack = dmapack;
	work->disp = 0;
	work->pre_disp = 0;

	return 0;
}

static int GetResources(RdrWork * work, int name, int where)
{
	int   chanl;
	float x0, y0;
	float width, height;
	int mv_w, mv_h;
	int ret;
	int r, g, b, a;
	int movie;


	if(now_work != NULL) return -1;

	work->finish = 0;   /* 初期状態では終了しない */
	work->disp = 0;     /* 初期状態では表示しない */

	chanl = RDR_DEF_CHANL;
	r = g = b = a = 128;
	movie = 0;

	/* デフォルト表示位置を設定 */
	x0 = RDR_DEF_X0;
	y0 = RDR_DEF_Y0;

	/* デフォルト表示サイズを設定 */
	width  = (float)RDR_DEF_WIDTH;
	height = (float)RDR_DEF_HEIGHT;

	/* デフォルトムービーサイズを設定 */
	mv_w = (int)RDR_DEF_MOVIE_W;
	mv_h = (int)RDR_DEF_MOVIE_H;

	/* 再生するムービーを取得 */
	if(NULL != GCL_GetOption(RDR_OPT_data))
		movie = GCL_GetNextInt();

	/* 描画チャネルを GCL から取得 */
	if(NULL != GCL_GetOption(RDR_OPT_chanl))
		chanl = GCL_GetNextInt();

	/* 描画 RGBA 値を取得 */
	if(NULL != GCL_GetOption(RDR_OPT_rgba))
    {
		r = GCL_GetNextInt();
		g = GCL_GetNextInt();
		b = GCL_GetNextInt();
		a = GCL_GetNextInt();
    }
  
	/* 終了 GCL proc を指定 */
	work->proc_id = 0;
	if(NULL != GCL_GetOption(RDR_OPT_finaly))
		work->proc_id = GCL_GetNextInt();   /* 終了 proc ID を取得 */
  
	/* 表示位置を GCL から取得 */
	if(NULL != GCL_GetOption(RDR_OPT_pos))
    {
		x0 = (float)GCL_GetNextInt();
		y0 = (float)GCL_GetNextInt();
    }

	/* 表示サイズを GCL から取得 */
	if(NULL != GCL_GetOption(RDR_OPT_size))
    {
		width  = (float)GCL_GetNextInt();
		height = (float)GCL_GetNextInt();
    }

	/* ムービーサイズを GCL から取得 */
	if(NULL != GCL_GetOption(RDR_OPT_movie))   /* 'movie' */
    {
		mv_w = GCL_GetNextInt();
		mv_h = GCL_GetNextInt();
    }

	/* 描画チャネルの設定 */
	work->chanl = chanl;

	/* 描画色の設定 */
	work->r = r;
	work->g = g;
	work->b = b;
	work->a = a;

	/* 表示関係パラメータを概念座標値から実座標値に変換 */
	{
		work->x0 = (int)x0;
		work->y0 = (int)(((y0 * (float)DRAW_HEIGHT / 384.0F)) );	// ?

		work->x1 = (int)(x0+width);
		work->y1 = (int)((((y0+height)*(float)DRAW_HEIGHT/384.0F)) );	// ?
      work->y1-= 2;
	}

	/* ムービーサイズを設定 */
	work->width  = mv_w;
	work->height = mv_h-2;
   work->movie = movie;

	if((ret = setup_movie(work))) return ret;
	if((ret = setup_dma(work))) return ret;
	work->handle = GM_IpuStream(work->movie, RDR_STREAM_CH);
	now_work = work;
	return 0;
}

void * NewRadarMovie(int name, int where)
{
	RdrWork * work = GV_NewActor(GV_ACTOR_MANAGER, sizeof(RdrWork));

	if(NULL == work) return NULL;

	GV_SetActor(&(work->actor), Act, Die);
	GV_ActorEX(&(work->actor));

	if(GetResources(work, name, where))
    {
		GV_DestroyActor(work);
		return NULL;
    }

	return work;
}

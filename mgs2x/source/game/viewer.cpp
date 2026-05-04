/*
	viewer.c
		デバッグ表示モード
	1999/08/26 K.Uehara
	$Id: viewer.cpp,v 1.1.1.3 2002/11/19 11:41:57 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "gameheader.h"
#include "menuprim.h"

// TODO: DRAW_WIDTH, DRAW_HEIGHT の定義場所
//#define DRAW_WIDTH  640
//#define DRAW_HEIGHT 480

//TODO: HSYNC_PER_VSYNC の定義場所
extern int BP_HSYNC_PER_VSYNC(void);
#define HSYNC_PER_VSYNC (BP_HSYNC_PER_VSYNC())

typedef struct {
	GV_ACT actor;
	void (**func_p)(GV_PAD *pad);
	int prof_max_act;
	int prof_max_act_time;
	int prof_max_draw;
	int prof_max_draw_time;
} Work;
static Work viewer_work ;

/* -------------------------------------------------------- */
/*
	描画用構造体宣言
*/


/* -------------------------------------------------------- */
/*
	ポリゴン数情報
*/

static void ViewPolyCount(GV_PAD *pad)
{
	DG_CHANL *cp;
	int i;
	int total_objnum = 0;
	int visible_objnum = 0;
	int divide_objnum = 0;
	int clip_verts = 0;
	int visible_verts = 0;
	int divide_verts = 0;
	int prim_packs = 0;
	int prim_prims = 0;
	int obj_queue_num = 0;
	int prim_queue_num = 0;
	
	for(i = 0; i < 1; i++){		// CHANL 0 only
		int j;
		DG_OBJ_QUEUE *que;

		cp = DG_Chanl(i);
		que = cp->obj_queue;

		// single オブジェクト
		{
			DG_OBJS **objs;
			DG_OBJ_BUFFER *objbuf;

			objbuf = &(que->objs_buffer);
			objs = (DG_OBJS **)objbuf->queue;
			obj_queue_num += objbuf->n_queue;
			for(j = objbuf->n_queue; j > 0; --j, objs++){
				DG_OBJ *op;
				int n;
				op = (*objs)->objs;
				if((*objs)->flag & DG_FLAG_INVISIBLE) continue;
				for(n = (*objs)->n_models; n > 0; --n, op++){
					total_objnum++;
					{
						DG_OBJPACK *pack;
						int n_packs, n_verts;
						n_verts = 0;
						n_packs = op->model->n_packs;
						pack = op->model->packs;
						for(; n_packs > 0; --n_packs){
							n_verts += pack->n_verts;
							pack++;
						}
						if(op->bound_mode == 0){
							visible_objnum++;
							visible_verts += n_verts;
						} else if(op->bound_mode == 1){
							divide_objnum++;
							divide_verts += n_verts;
						} else {
							clip_verts += n_verts;
						}
					}
				}
			}
		}
		// プリミティブ
		{
			DG_PRIM2 **pque;
			DG_OBJ_BUFFER *primbuf;

			primbuf = &(que->prim2_buffer);

			pque = (DG_PRIM2 **)primbuf->queue;
			j = primbuf->n_queue;

			prim_queue_num += primbuf->n_queue;

			for(; j > 0; --j){
				DG_PRIM2 *prim;

				prim = *(pque++);
				//if(prim->type & (DG_PRIM2_INVISIBLE | DG_PRIM_SORTONLY)) continue;
				if(prim->type & DG_PRIM2_INVISIBLE0) continue;

				prim_packs += prim->n_prims;
				prim_prims += prim->n_prims * prim->packet_verts;
			}
		}
	}

	DEBUG_Locate(480, DRAW_HEIGHT - 60 - 48, MENU_MODE_RIGHT);
	{
		int act_time, draw_time, w;

		w = 1 - GV_ProfileClock;

		act_time = DIFF_HSYNC_TIMER(GV_ProfileLog[ w ][ GV_PROFILE_CHANL_START
												  + GV_ChanlProfileNum[ w ] ]
									 ,  GV_ProfileLog[ w ][ GV_PROFILE_ACT_START ]);
		draw_time = DIFF_HSYNC_TIMER(GV_ProfileLog[ w ][ GV_PROFILE_DRAW_START
												   + GV_DrawProfileNum[ w ] ]
									 ,  GV_ProfileLog[ w ][ GV_PROFILE_DRAW_START ]);
		DEBUG_Printf("PERF %3d/%3d\n", act_time, draw_time);
	}
#if 1
	DEBUG_Printf("QUE %d/%d\n", obj_queue_num, prim_queue_num);
	DEBUG_Printf("PRIMS %d/%d\n", prim_packs, prim_prims);
	DEBUG_Printf("OBJS %d(%d)/%d(%d)/%d(%d)\n",
				 divide_verts, divide_objnum,
				 visible_verts, visible_objnum,
				 visible_verts + divide_verts + clip_verts, total_objnum);
#else
	printf("QUE %d/%d\n", obj_queue_num, prim_queue_num);
	printf("PRIMS %d/%d\n", prim_packs, prim_prims);
	printf("OBJS %d(%d)/%d(%d)/%d(%d)\n",
		   divide_verts, divide_objnum,
		   visible_verts, visible_objnum,
		   visible_verts + divide_verts + clip_verts, total_objnum);
#endif
}

/* -------------------------------------------------------- */
/*
	メモリ情報

	メモリリンクリストをたどって状況を表示する
*/

#define STATE_FREE	0x80000000
#define STATE_MASK	0xF8000000
#define STATE_LOAD	0x10000000

#define IS_FREE(_t)	((_t)->size & STATE_FREE)
#define IS_LOAD(_t)	((_t)->size & STATE_LOAD)

#define SET_FREE(_t)	((_t)->size |= STATE_FREE)
#define SET_USED(_t)	((_t)->size &= ~STATE_FREE)
#define SIZE(_t)		((_t)->size & ~STATE_MASK)

/*
  メモリ構造管理構造体
  base.sizeにtype情報。
*/

typedef struct _mem_tag {
	struct _mem_tag *prev;
	struct _mem_tag *next;
	void *ptr;			
	int size;
} MEM_TAG;

typedef struct _mem_info {
	MEM_TAG base;

	MEM_TAG *delayed_free_list;
	int size;
	int freesize;
} MEM_SYS;

extern "C" extern MEM_SYS memsys[ MAX_MEMSYS ];

extern "C" extern int			DG_PacketMemorySize ;			/* パケットメモリの最大サイズ */
extern "C" extern int			DG_OldUsePacketTop ;			/* 前フレームでの先頭の使用サイズ */
extern "C" extern int			DG_OldUsePacketBottom ;			/* 前フレームでの後ろの使用サイズ */

#define MEMVIEW_W	(DRAW_WIDTH - 24 * 2)
#define MEMVIEW_H	10

#define MEMVIEW_X	24
#define MEMVIEW_Y	(DRAW_HEIGHT - 24 - MEMVIEW_H * 2 - 24)

static void ViewMemorySystem(GV_PAD *pad)
{
	Work *work = &viewer_work;
	void	*prim ;

	DEBUG_Locate(24, MEMVIEW_Y - 12, 0);
	DEBUG_Printf("MEMORY\n");

	/* 目盛表示 */
	/* 目盛はライン */
	{
		int x, n;
		n = 0;
		prim = MENU_OpenPrim();
		for(x = MEMVIEW_X; x <= MEMVIEW_W + MEMVIEW_X; x += MEMVIEW_W / 4) {
			prim = DG_SetDmapackLine( prim,
									 x, MEMVIEW_Y, 0x80808080,
									 x, MEMVIEW_Y + MEMVIEW_H * 2, 0x80808080 );
			n ++;
		}
		MENU_ClosePrim( prim );
	}

	/* メモリブロック表示 */
	{
		int n;
		MEM_SYS *m;
		MEM_TAG *p;
		int max_size, use_size, unit;
		int	max_pack_size, pack_size ;

		/* パケット及びプッシュバッファ用メモリ表示 */
		prim = MENU_OpenPrim();
		{
			int x1, x2, y1, y2, y_base ;
			int col1, col2;

			x1 = 0 + MEMVIEW_X;
			x2 = x1 + (int)( (float)MEMVIEW_W * DG_OldUsePacketTop / DG_PacketMemorySize ) ;
			col1 = 0x80000000; col2 = 0x8000ff00;

			//y_base = MEMVIEW_Y + MEMVIEW_H * (2) ;
			y_base = MEMVIEW_Y ;
			y1 = y_base + 1;
			y2 = y_base + MEMVIEW_H / 1 - 1;

			prim = DG_SetDmapackQuad( prim,
									 x1, y1, col1, x2, y1, col2, x2, y2, col2, x1, y2, col1 );

			x1 = 0 + MEMVIEW_X + MEMVIEW_W ;
			x2 = x1 - (int)( (float)MEMVIEW_W * DG_OldUsePacketBottom / DG_PacketMemorySize ) ;
			col1 = 0x80000000; col2 = 0x80ff0000;

			//y_base += MEMVIEW_H / 2 ;
			y1 = y_base + 1;
			y2 = y_base + MEMVIEW_H / 1 - 1;

			prim = DG_SetDmapackQuad( prim,
									 x1, y1, col1, x2, y1, col2, x2, y2, col2, x1, y2, col1 );
		}
		MENU_ClosePrim( prim );

		n = 0;
#if 0
		/* XBOXの場合、パケットブロックは存在しない */
		m = memsys + GV_PACKET_MEMORY0 ;
		max_pack_size = max_size = m->size;
		pack_size = 0 ;
		unit = (max_size / MEMVIEW_W);

		prim = MENU_OpenPrim();
		for (p = m->base.next; p != NULL;) {
			char *addr;
			int size;
			int col1, col2;

			addr = (char *)p;
			size = 0;
			if (IS_FREE(p)) {
				p = p->next;
				continue;
			} else {
				int f;
				f = IS_LOAD(p);
				for(; p != NULL && (!IS_FREE(p)) && f == IS_LOAD(p); p = p->next){
					size += SIZE(p);
				}
				if (f) {
					col1 = 0x80000000; col2 = 0x8000ff00;
				} else {
					col1 = 0x80000000; col2 = 0x80ff0000;
				}
			}
			{
				int x1, x2, y1, y2;

				x1 = (addr - (char *)m->base.next) / unit + MEMVIEW_X;
				x2 = x1 + (size / unit);
				pack_size += size;
				if (x1 == x2) continue;

				y1 = MEMVIEW_Y + MEMVIEW_H * 0 + 1;
				y2 = MEMVIEW_Y + MEMVIEW_H * 1 - 1;

				prim = DG_SetDmapackQuad( prim,
										 x1, y1, col1, x2, y1, col2, x2, y2, col2, x1, y2, col1 );
				n += 4;
			}
		}
		MENU_ClosePrim( prim );
#endif

		/* メインブロック */
		m = memsys + GV_NORMAL_MEMORY;
		max_size = m->size;
		use_size = 0;
		unit = (max_size / MEMVIEW_W);

		prim = MENU_OpenPrim();
		for (p = m->base.next; p != NULL;) {
			char *addr;
			int size;
			int col1, col2;

			addr = (char *)p;
			size = 0;
			if (IS_FREE(p)) {
				p = p->next;
				continue;
			} else {
				int f;
				f = IS_LOAD(p);
				for(; p != NULL && (!IS_FREE(p)) && f == IS_LOAD(p); p = p->next){
					size += SIZE(p);
				}
				if (f) {
					col1 = 0x80000000; col2 = 0x8000ff00;
				} else {
					col1 = 0x80000000; col2 = 0x80ff0000;
				}
			}
			{
				int x1, x2, y1, y2;

				x1 = (addr - (char *)m->base.next) / unit + MEMVIEW_X;
				x2 = x1 + (size / unit);
				use_size += size;
				if (x1 == x2) continue;

				y1 = MEMVIEW_Y + MEMVIEW_H  + 1;
				y2 = MEMVIEW_Y + MEMVIEW_H * 2 - 1;

				prim = DG_SetDmapackQuad( prim,
										 x1, y1, col1, x2, y1, col2, x2, y2, col2, x1, y2, col1 );
				n += 4;
			}
		}
		MENU_ClosePrim( prim );
		DEBUG_Locate(452, MEMVIEW_Y + 8, 0);
//		DEBUG_Printf("%3d%%\n", pack_size * 100 / max_pack_size );
		DEBUG_Locate(452, MEMVIEW_Y + 8 + MEMVIEW_H, 0);
		DEBUG_Printf("%3d%%\n", use_size * 100 / max_size);
	}
#ifdef DEBUG_MODE
	if((pad->status & (PAD_L1 | PAD_B)) == (PAD_L1 | PAD_B)){
		GV_DumpMemory(GV_NORMAL_MEMORY);
	}
#endif
#if 0	
	if((GV_PadData[ 1 ].status & (PAD_A | PAD_B | PAD_X | PAD_Y))
		== (PAD_A | PAD_B | PAD_X | PAD_Y)){
		printf("!!! MEMORY PADDING %d !!!\n", GV_GetFreeMemorySize(GV_NORMAL_MEMORY));
		while(GV_GetFreeMemorySize(GV_NORMAL_MEMORY) > (int)((float)MEM_SIZE * 0.03F)){
			int size = GV_GetMaxFreeMemory(GV_NORMAL_MEMORY);
			printf("PADDING %d\n", size);
			GV_Malloc(size);
		}
	}
#endif
}

/* -------------------------------------------------------- */
/*
	処理時間情報
*/

#define PROFILE_W	(DRAW_WIDTH - 24 * 2)
#define PROFILE_H	10

#define PROFILE_X	24
#define PROFILE_Y	( DRAW_HEIGHT - 16 - PROFILE_H * 2 - 24)

static void *line_set( void *prim, int time1, int time2,
						 int y1, int y2, u_int col1, u_int col2, int max_time, int start_time)
{
	int x1, x2;

	x1 = DIFF_HSYNC_TIMER(time1, start_time) * PROFILE_W / max_time;
	x1 += PROFILE_X;
	if( x1 > PROFILE_W ){
		x1 = PROFILE_W;
	}
	x2 = DIFF_HSYNC_TIMER(time2, start_time) * PROFILE_W / max_time;
	x2 += PROFILE_X;
	if( x2 > PROFILE_W ){
		x2 = PROFILE_W;
	}

	prim = DG_SetDmapackQuad( prim,
							 x1, y1, col1,
							 x2, y1, col2,
							 x2, y2, col2,
							 x1, y2, col1 );

	return prim ;
}

#include <d3d8perf.h>

static int prev_act_top = 0;

//#define MAX_TIME_UNIT	( HSYNC_PER_VSYNC )

static void ViewProfileLog(GV_PAD *pad)
{
	Work *work = &viewer_work;
	int draw_time, act_time, max_time;
	int start_time;
	int i, w;
	void	*prim ;

	/*
	 * 1VSync 以下の場合は全体を1VSyncにする
	 * それ以上の場合は、全体をトータルの時間にする
	 */
	DEBUG_Locate(24, PROFILE_Y - 12, 0);
	DEBUG_Printf("PROFILE\n");

	w = 1 - GV_ProfileClock;

	start_time = GV_ProfileLog[w][GV_PROFILE_ACT_START];

	act_time = DIFF_HSYNC_TIMER(GV_ProfileLog[1-w][GV_PROFILE_ACT_START]
								, start_time );

	draw_time = DIFF_HSYNC_TIMER(GV_ProfileLog[w][GV_PROFILE_DRAW_START+ GV_DrawProfileNum[w]]
								 , start_time );
	
	max_time = (act_time > draw_time) ? act_time : draw_time;
	max_time = (max_time > HSYNC_PER_VSYNC) ? max_time : HSYNC_PER_VSYNC;

	//max_time = ( ( max_time + MAX_TIME_UNIT - 1 ) / MAX_TIME_UNIT ) * MAX_TIME_UNIT;

	if (max_time > 4 * HSYNC_PER_VSYNC) max_time = 4 * HSYNC_PER_VSYNC;

	act_time = DIFF_HSYNC_TIMER(GV_ProfileLog[w][GV_PROFILE_ACT_END]
								, start_time );

	// デバッグメーター表示
	
	// 処理棒表示
	{
		int            start;
		int            nLine = 0;
		unsigned short *p;		
		
		// act 表示
		prim = MENU_OpenPrim();
		start = GV_ProfileLog[w][GV_PROFILE_ACT_START];
		p = &GV_ProfileLog[w][GV_PROFILE_CHANL_START];
		for (i = 0; i <= GV_ChanlProfileNum[ w ]; i++, p++) {
			if (DIFF_HSYNC_TIMER(*p, start) < 2) continue;
			prim = line_set( prim, start, *p,
							PROFILE_Y + 1, PROFILE_Y + PROFILE_H - 1,
							0x80000000, 0x8000ff00, max_time, start_time );
			start = *p;
			nLine += 4;
		}
		prim = line_set( prim, start, GV_ProfileLog[w][GV_PROFILE_ACT_END],
						PROFILE_Y + 1, PROFILE_Y + PROFILE_H - 1,
						0x80000000, 0x800000ff, max_time, start_time);
		nLine += 4;
		start = GV_ProfileLog[w][GV_PROFILE_ACT_END];

		prim = line_set( prim, start, GV_ProfileLog[1-w][GV_PROFILE_ACT_START],
						PROFILE_Y + 1, PROFILE_Y + PROFILE_H - 1,
						0x80000000, 0x80FF0000, max_time, start_time);
		nLine += 4;

		start = GV_ProfileLog[w][GV_PROFILE_DRAW_START];
//printf( "%X\n", DIFF_HSYNC_TIMER( start, start_time ) );
//		start = GV_ProfileLog[w][GV_PROFILE_ACT_START];
		p = &GV_ProfileLog[w][GV_PROFILE_DRAW_START+1];
		for (i = 0; i < GV_DrawProfileNum[ w ]; i++, p++) {
			if (start == *p) continue;
//printf( "LINE %d/%d %X %X\n", i, GV_DrawProfileNum[ w ], start, *p );
			prim = line_set( prim, start, *p,
							PROFILE_Y + PROFILE_H + 1, PROFILE_Y + PROFILE_H * 2 - 1,
							0x80000000, 0x80ff0000, max_time, start_time);
			start = *p;
			nLine += 4;
		}

		MENU_ClosePrim( prim );
	}

	// 目盛り表示
	{
		int     x;
		int     step;
		int     nLine = 0;
		
		prim = MENU_OpenPrim();
		step = PROFILE_W * (HSYNC_PER_VSYNC / 4) / max_time;
		for (x = PROFILE_X; x <= PROFILE_W + PROFILE_X; x += step) {
			int ofsy;
			if (nLine % 4 == 0) {
				ofsy = 4;
			} else {
				ofsy = 0;
			}
			prim = DG_SetDmapackLine( prim,
									 x, PROFILE_Y - ofsy, 0x60808080,
									 x, PROFILE_Y + PROFILE_H * 2 + ofsy, 0x60808080 );
			nLine++;
		}
		MENU_ClosePrim( prim );
	}
	
	{
		if (work->prof_max_act_time + 60 < GV_Time) {
			work->prof_max_act = 0;
			work->prof_max_act_time = 0;
		}
		if (work->prof_max_act < act_time) {
			work->prof_max_act = act_time;
			work->prof_max_act_time = GV_Time;
		}
		if (work->prof_max_draw_time + 60 < GV_Time) {
			work->prof_max_draw = 0;
			work->prof_max_draw_time = 0;
		}
		if (work->prof_max_draw < draw_time) {
			work->prof_max_draw = draw_time;
			work->prof_max_draw_time = GV_Time;
		}
		DEBUG_Locate(428-16+96, DRAW_HEIGHT -24 - 32 - 32 - 16 - 16, 0);
		{
			int diff;
			diff = DIFF_HSYNC_TIMER( GV_ProfileLog[w][GV_PROFILE_ACT_START], prev_act_top );
			DEBUG_Printf("FRM %3d:%1.1f\n", diff, diff / 262.5F );
		}
		DEBUG_Printf("ACT %3d:%3d\n", act_time, work->prof_max_act);
		DEBUG_Printf("DRW %3d:%3d\n", draw_time, work->prof_max_draw);
	}

	prev_act_top = GV_ProfileLog[w][GV_PROFILE_ACT_START];
#if 0
	if( pad->press & PAD_Y ){
	    D3DPERF *pPerf = D3DPERF_GetStatistics();
		D3DPERF_StartPerfProfile();
        pPerf->RecordD3DPerfEvents =
            (1 << D3DPERFEvent_Header) |
            (1 << D3DPERFEvent_Present) |
            (1 << D3DPERFEvent_BlockUntilIdle) |
            (1 << D3DPERFEvent_BlockOnFence) |
            (1 << D3DPERFEvent_PushBufferWait) |
            (1 << D3DPERFEvent_ObjectLockWait) |
            (1 << D3DPERFEvent_User0) | (1 << D3DPERFEvent_User1);
		D3DPERF_StartCountingPerfEvent( 5000 );
	}
	if( pad->press & PAD_X ){
		D3DPERF_DumpPerfEvents();
		D3DPERF_Dump();
		D3DPERF_DumpPerfProfCounts();
	}
#endif
}

static void ViewNothing(GV_PAD *pad)
{
}

/* -------------------------------------------------------- */
/*
	メイン
*/

static void (*func_table[])(GV_PAD *pad) = {
	ViewNothing,
	ViewProfileLog,
	ViewMemorySystem,
	ViewPolyCount,
	NULL,
};

static void Act(Work *work)
{
	GV_PAD *pad;

	pad = &GV_PadData[1];

	if(GM_Debug2PMode == GM_DEBUG_MODE_VIEWER){
		DEBUG_Locate(32, 16, 0);
		DEBUG_Printf("PRESS 2P-L1 CHANGE MODE\n");
		if(pad->press & PAD_L1){
			work->func_p++;
			if(*work->func_p == NULL){
				work->func_p = func_table;
			}
		}
	}
	//DEBUG_SetColor(200, 200, 200);
	(**work->func_p)(pad);
	//ViewProfileLog(pad);
}

static int GetResources(Work *work)
{
	work->func_p = func_table;
	return 0;
}

void *GM_StartDebugViewer(void)
{
	Work *work;

	work = &viewer_work;

	GV_InsertActorPriority(GV_ACTOR_MANAGER, work, GM_DAEMON_PRIO + 0x10);
	GV_SetActor(work, Act, NULL);

	GetResources(work);
	return (void *)work;
}

/*
	viewer.c
		デバッグ表示モード
	1999/08/26 K.Uehara
	$Id: viewer.c,v 1.1.1.3 2002/11/19 11:41:57 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

#include	"def_dma.h"

typedef struct {
	GV_ACT actor;
	void ( **func_p )( GV_PAD *pad );
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

/* LINE_F2 */

typedef struct {
	u_long64 xy1;
	u_long64 xy2;
} LINE_F2;

static sceGifTag *set_line_f2_head( MENU_PRIM *prim )
{
	sceGifTag *giftag;

	typedef struct {
		sceGsRgbaq rgba;
		sceGsPrim prim;
	} LINE_F2_HEAD;

	LINE_F2_HEAD *init_pack;

	MENU_NEW_TAG( giftag, prim );
	DG_SET_GIFTAG( giftag, .FLG = SCE_GIF_REGLIST, .NREG = 2, .NLOOP = 1
				   , .REGS0 = GS_REGS_RGBA, GS_REGS_PRIM );

	MENU_NEW_PRIM( init_pack, prim );
	DG_SET_GS_REG( &init_pack->prim, .PRIM = SCE_GS_PRIM_LINE );
	DG_SET_GS_REG( &init_pack->rgba, 0, 255, 0, 128, 0 );

	MENU_NEW_TAG( giftag, prim );
	DG_SET_GIFTAG( giftag, .FLG = SCE_GIF_REGLIST, .NREG = 2
				   , .REGS0 = GS_REGS_XYZ2, GS_REGS_XYZ2 );
	return giftag;
}

/* LINE_G4 */

typedef struct {
	u_long64 rgb1;
	u_long64 xy1;
	u_long64 rgb2;
	u_long64 xy2;
	u_long64 rgb3;
	u_long64 xy3;
	u_long64 rgb4;
	u_long64 xy4;
} LINE_G4;

static sceGifTag *set_line_g4_head( MENU_PRIM *prim )
{
	sceGifTag *giftag;
	struct {
		sceGsPrim prim;
		u_long64 nop;
	} *head;

	MENU_NEW_TAG( giftag, prim );
	DG_SET_GIFTAG( giftag, .FLG = SCE_GIF_REGLIST, .NREG = 1, .NLOOP = 1
				   , .REGS0 = GS_REGS_PRIM );
	MENU_NEW_PRIM( head, prim );
	DG_SET_GS_REG( &head->prim, .PRIM = SCE_GS_PRIM_LINESTRIP, .IIP = 1 );

	MENU_NEW_TAG( giftag, prim );
	DG_SET_GIFTAG( giftag, .FLG = SCE_GIF_REGLIST, .NREG = 8
				   , .REGS0 = GS_REGS_RGBA, GS_REGS_XYZ3
				   , GS_REGS_RGBA, GS_REGS_XYZ2
				   , GS_REGS_RGBA, GS_REGS_XYZ2
				   , GS_REGS_RGBA, GS_REGS_XYZ2 );
	return giftag;
}

static void set_g4_box( LINE_G4 *line, int x1, int y1, int x2, int y2, int col1, int col2 )
{
	line->rgb1 = line->rgb4 = col1;
	line->rgb2 = line->rgb3 = col2;
	line->xy1 = SCE_GS_SET_XYZ( x1, y1, 0 );
	line->xy2 = SCE_GS_SET_XYZ( x2, y1, 0 );
	line->xy3 = SCE_GS_SET_XYZ( x2, y2, 0 );
	line->xy4 = SCE_GS_SET_XYZ( x1, y2, 0 );
}

/* -------------------------------------------------------- */
/*
	ポリゴン数情報
*/

static void ViewPolyCount( GV_PAD *pad )
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

	for( i = 0; i < 1; i++ ){		// CHANL 0 only
		int j;
		DG_OBJ_QUEUE *que;

		cp = DG_Chanl( i );
		que = cp->obj_queue;

		// single オブジェクト
		{
			DG_OBJS **objs;
			DG_OBJ_BUFFER *objbuf;

			objbuf = &( que->objs_buffer );
			objs = ( DG_OBJS ** )objbuf->queue;
			obj_queue_num += objbuf->n_queue;
			for( j = objbuf->n_queue; j > 0; -- j, objs ++ ){
				DG_OBJ *op;
				int n;
				op = (*objs)->objs;
				if( (*objs)->flag & DG_FLAG_INVISIBLE ) continue;
				for( n = (*objs)->n_models; n > 0; -- n, op++ ){
					total_objnum ++;
					{
						DG_OBJPACK *pack;
						int n_packs, n_verts;
						n_verts = 0;
						n_packs = op->model->n_packs;
						pack = op->model->packs;
						for( ; n_packs > 0; -- n_packs ){
							n_verts += pack->n_verts;
							pack++;
						}
						if( op->bound_mode == 0 ){
							visible_objnum ++;
							visible_verts += n_verts;
						} else if( op->bound_mode == 1 ){
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

			primbuf = &( que->prim2_buffer );

			pque = ( DG_PRIM2 ** )primbuf->queue;
			j = primbuf->n_queue;

			prim_queue_num += primbuf->n_queue;

			for( ; j > 0; --j ){
				DG_PRIM2 *prim;

				prim = *( pque ++ );
				if( prim->type & ( DG_PRIM_INVISIBLE | DG_PRIM_SORTONLY ) ) continue;

				prim_packs += prim->n_prims;
				prim_prims += prim->n_prims * prim->packet_verts;
			}
		}
	}
	DEBUG_Locate( 480, DRAW_HEIGHT - 60, MENU_MODE_RIGHT );
	{
		int act_time, draw_time, w;

		w = 1 - GV_ProfileClock;

		act_time = DIFF_HSYNC_TIMER( GV_ProfileLog[ w ][ GV_PROFILE_CHANL_START
												  + GV_ChanlProfileNum[w] ]
									 ,  GV_ProfileLog[ w ][ GV_PROFILE_ACT_START ] );
		draw_time = DIFF_HSYNC_TIMER( GV_ProfileLog[ w ][ GV_PROFILE_DRAW_START
												   + GV_DrawProfileNum[w] ]
									 ,  GV_ProfileLog[ w ][ GV_PROFILE_DRAW_START ] );
		DEBUG_Printf( "PERF %3d/%3d\n", act_time, draw_time );
	}
	DEBUG_Printf( "QUE %d/%d\n", obj_queue_num, prim_queue_num );
	DEBUG_Printf( "PRIMS %d/%d\n", prim_packs, prim_prims );
	DEBUG_Printf( "OBJS %d(%d)/%d(%d)/%d(%d)\n", divide_verts, divide_objnum, visible_verts, visible_objnum, visible_verts + divide_verts + clip_verts, total_objnum );
}

/* -------------------------------------------------------- */
/*
	メモリ情報

	メモリリンクリストをたどって状況を表示する
*/

#define STATE_FREE	0x80000000
#define STATE_MASK	0xF8000000
#define STATE_LOAD	0x10000000

#define IS_FREE( _t )	( (_t)->size & STATE_FREE )
#define IS_LOAD( _t )	( (_t)->size & STATE_LOAD )

#define SET_FREE( _t )	( (_t)->size |= STATE_FREE )
#define SET_USED( _t )	( (_t)->size &= ~STATE_FREE )
#define SIZE( _t )		( (_t)->size & ~STATE_MASK )

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

extern MEM_SYS memsys[ MAX_MEMSYS ];

#define MEMVIEW_W	( DRAW_WIDTH - 24 * 2 )
#define MEMVIEW_H	16

#define MEMVIEW_X	24
#define MEMVIEW_Y	( DRAW_HEIGHT - 24 - MEMVIEW_H * 2 )

static void ViewMemorySystem( GV_PAD *pad )
{
	MENU_PRIM *prim;

	DEBUG_Locate( 24, MEMVIEW_Y - 12, 0 );
	DEBUG_Printf( "MEMORY\n" );

	prim = MENU_OpenPrim();

	/* 目盛表示 */
	/* 目盛はライン */
	{
		int x, n;
		sceGifTag *giftag;
		LINE_F2 *line_pack;

		giftag = set_line_f2_head( prim );

		n = 0;

		for( x = DG_POS_X( MEMVIEW_X ); x <= DG_POS_X( MEMVIEW_W + MEMVIEW_X );
			 x += ( MEMVIEW_W << 4 ) / 4 ){
			MENU_NEW_PRIM( line_pack, prim );
			line_pack->xy1 = SCE_GS_SET_XYZ( x, DG_POS_Y( MEMVIEW_Y ), 0 );
			line_pack->xy2 = SCE_GS_SET_XYZ( x, DG_POS_Y( MEMVIEW_Y + MEMVIEW_H * 2 ), 0 );
			n ++;
		}
		giftag->NLOOP = n;
	}

	/* メモリブロック表示 */
	{
		int n;
		MEM_SYS *m;
		MEM_TAG *p;
		int max_size, use_size, pack_size, unit;

		sceGifTag *giftag;
		LINE_G4 *line_pack;

		giftag = set_line_g4_head( prim );
		n = 0;

		/* パケットブロック */
		{
			extern u_long128 *DG_LastDmaAddr;
			int size;
			if( ( void * )DG_LastDmaAddr < PACK_ADDR1 ){
				size = ( void * )DG_LastDmaAddr - PACK_ADDR0;
			} else {
				size = ( void * )DG_LastDmaAddr - PACK_ADDR1;
			}
			{
				int x1, x2, y1, y2;

				x1 = DG_POS_X( MEMVIEW_X );
				x2 = x1 + ( ( size * MEMVIEW_W / PACK_SIZE ) << 4);

				y1 = DG_POS_Y( MEMVIEW_Y + 1 );
				y2 = DG_POS_Y( MEMVIEW_Y + MEMVIEW_H - 1 );

				MENU_NEW_PRIM( line_pack, prim );
				set_g4_box( line_pack, x1, y1, x2, y2, 0x80000000, 0x80ff0000 );
				n++;
			}

			pack_size = size;
		}

		/* メインブロック */

		m = memsys + GV_NORMAL_MEMORY;
		max_size = m->size;
		use_size = 0;
		unit = ( max_size / MEMVIEW_W );

		for( p = m->base.next; p != NULL; ){
			void *addr;
			int size;
			int col1, col2;

			addr = p;
			size = 0;
			if( IS_FREE( p ) ){
				p = p->next;
				continue;
			} else {
				int f;
				f = IS_LOAD( p );
				for( ; p != NULL && ( !IS_FREE( p ) ) && f == IS_LOAD( p ); p = p->next ){
					size += SIZE( p );
				}
				if( f ){
					col1 = 0x80000000; col2 = 0x8000FF00;
				} else {
					col1 = 0x80000000; col2 = 0x80ff0000;
				}
			}
			{
				int x1, x2, y1, y2;

				x1 = DG_POS_X( ( addr - ( void * )m->base.next ) / unit + MEMVIEW_X );
				x2 = x1 + ( ( size / unit ) << 4 );
				use_size += size;
				if( x1 == x2 ) continue;

				y1 = DG_POS_Y( MEMVIEW_Y + MEMVIEW_H  + 1 );
				y2 = DG_POS_Y( MEMVIEW_Y + MEMVIEW_H * 2 - 1 );

				MENU_NEW_PRIM( line_pack, prim );
				set_g4_box( line_pack, x1, y1, x2, y2, col1, col2 );
				n++;
			}
		}
		giftag->NLOOP = n;

		DEBUG_Locate( 452, MEMVIEW_Y, 0 );
		DEBUG_Printf( "%3d%%\n", pack_size * 100 / PACK_SIZE );
		DEBUG_Locate( 452, MEMVIEW_Y + MEMVIEW_H, 0 );
		DEBUG_Printf( "%3d%%\n", use_size * 100 / max_size );
	}

	MENU_ClosePrim();

	if( ( pad->status & ( PAD_L1 | PAD_B ) ) == ( PAD_L1 | PAD_B ) ){
		GV_DumpMemory( GV_NORMAL_MEMORY );
	}
	if( ( GV_PadData[ 1 ].status & ( PAD_A | PAD_B | PAD_X | PAD_Y ) )
		== ( PAD_A | PAD_B | PAD_X | PAD_Y ) ){
		printf( "!!! MEMORY PADDING %d !!!\n", GV_GetFreeMemorySize( GV_NORMAL_MEMORY ) );
		while( GV_GetFreeMemorySize( GV_NORMAL_MEMORY ) > ( int )( ( float )MEM_SIZE * 0.03F ) ){
			int size = GV_GetMaxFreeMemory( GV_NORMAL_MEMORY );
			printf( "PADDING %d\n", size );
			GV_Malloc( size );
		}
	}
}

/* -------------------------------------------------------- */
/*
	処理時間情報
*/

#ifdef NTSC
#define HSYNC_PER_VSYNC 262
#endif
#ifdef PAL
#define HSYNC_PER_VSYNC 312
#endif

#define PROFILE_W	( DRAW_WIDTH - 24 * 2 )
#define PROFILE_H	16

#define PROFILE_X	24
#define PROFILE_Y	( DRAW_HEIGHT - 16 - PROFILE_H * 2 )

static void ViewProfileLog( GV_PAD *pad )
{
	/* profile log */
	MENU_PRIM *prim;
	int draw_time;
	int act_time;
	int max_time;
	int i;
	int w;

	/*
		1VSync 以下の場合は全体を1VSyncにする
		それ以上の場合は、全体をトータルの時間にする
	*/
	DEBUG_Locate( 24, PROFILE_Y - 12, 0 );
	DEBUG_Printf( "PROFILE\n" );

	w = 1 - GV_ProfileClock;

	act_time = DIFF_HSYNC_TIMER( GV_ProfileLog[ w ][ GV_PROFILE_CHANL_START
												   + GV_ChanlProfileNum[w] ]
								 ,  GV_ProfileLog[ w ][ GV_PROFILE_ACT_START ] );
	draw_time = DIFF_HSYNC_TIMER( GV_ProfileLog[ w ][ GV_PROFILE_DRAW_START
													+ GV_DrawProfileNum[w] ]
								 ,  GV_ProfileLog[ w ][ GV_PROFILE_DRAW_START ] );
//printf( "act_time %d, draw_time %d\n" );
	max_time = ( act_time > draw_time ) ? act_time : draw_time;
	max_time = ( max_time > HSYNC_PER_VSYNC ) ? max_time : HSYNC_PER_VSYNC;
	if( max_time > 4 * HSYNC_PER_VSYNC ) max_time = 4 * HSYNC_PER_VSYNC;

	prim = MENU_OpenPrim();

	/* 目盛表示 */
	/* 目盛はライン */
	{
		int x, n;
		int step;
		sceGifTag *giftag;
		LINE_F2 *line_pack;

		giftag = set_line_f2_head( prim );

		step = PROFILE_W * ( HSYNC_PER_VSYNC / 4 ) / max_time;
		n = 0;

		for( x = DG_POS_X( PROFILE_X ); x <= DG_POS_X( PROFILE_W + PROFILE_X );
			 x += step << 4 ){
			int ofsy;
			if( n % 4 == 0 ){
				ofsy = 4;
			} else {
				ofsy = 0;
			}
			MENU_NEW_PRIM( line_pack, prim );
			line_pack->xy1 = SCE_GS_SET_XYZ( x, DG_POS_Y( PROFILE_Y - ofsy ), 0 );
			line_pack->xy2 = SCE_GS_SET_XYZ( x, DG_POS_Y( PROFILE_Y + PROFILE_H * 2 + ofsy )
											 , 0 );
			n ++;
		}
		giftag->NLOOP = n;
	}
	/* 棒表示 */
	{
		sceGifTag *giftag;
		LINE_G4 *line_pack;

		int start, n;
		unsigned short *p;

		void line_set( typeof( line_pack ) line, int time1, int time2, int y1, int y2
					   , u_long64 col1, u_long64 col2 )
		{
			int x1, x2;

			x1 = DIFF_HSYNC_TIMER( time1, GV_ProfileLog[ w ][ GV_PROFILE_ACT_START ] )
				* PROFILE_W / max_time;
			x1 = DG_POS_X( x1 + PROFILE_X );

			x2 = DIFF_HSYNC_TIMER( time2, GV_ProfileLog[ w ][ GV_PROFILE_ACT_START ] )
				* PROFILE_W / max_time;
			x2 = DG_POS_X( x2 + PROFILE_X );
			line->xy1 = SCE_GS_SET_XYZ( x1, y1, 0 );
			line->xy2 = SCE_GS_SET_XYZ( x2, y1, 0 );
			line->xy3 = SCE_GS_SET_XYZ( x2, y2, 0 );
			line->xy4 = SCE_GS_SET_XYZ( x1, y2, 0 );
			line->rgb1 = line->rgb4 = col1;
			line->rgb2 = line->rgb3 = col2;
		}

		giftag = set_line_g4_head( prim );

		start = GV_ProfileLog[ w ][ GV_PROFILE_ACT_START ];
		p = &GV_ProfileLog[ w ][ GV_PROFILE_CHANL_START ];

		n = 0;
//printf( "%d\n", GV_ChanlProfileNum[w] );
		for( i = 0; i < GV_ChanlProfileNum[w]; i++, p++ ){
			if( DIFF_HSYNC_TIMER( *p, start ) < 2 ) continue;
			MENU_NEW_PRIM( line_pack, prim );
//printf( "%d %d %d\n", i, *p, start );
			line_set( line_pack, start, *p, DG_POS_Y( PROFILE_Y + 1 )
					  , DG_POS_Y( PROFILE_Y + PROFILE_H - 1 ), 0x80000000, 0x80ff0000 );
			start = *p;
			n++;
		}

		start = GV_ProfileLog[ w ][ GV_PROFILE_DRAW_START ];
		p = &GV_ProfileLog[ w ][ GV_PROFILE_DRAW_START + 1 ];
//printf( "%d\n", GV_DrawProfileNum[w] );
		for( i = 0; i < GV_DrawProfileNum[w]; i++, p++ ){
			if( start == *p ) continue;
			MENU_NEW_PRIM( line_pack, prim );
//printf( "%d %d %d\n", i, *p, start );
			line_set( line_pack, start
					  , *p
					  , DG_POS_Y( PROFILE_Y + PROFILE_H + 1 )
					  , DG_POS_Y( PROFILE_Y + PROFILE_H * 2 -1 ), 0x80000000, 0x800000ff );
			start = *p;
			n++;
		}
		giftag->NLOOP = n;
	}

	MENU_ClosePrim();

	{
		Work *work = &viewer_work;

		if( work->prof_max_act_time + 60 < GV_Time ){
			work->prof_max_act = 0;
			work->prof_max_act_time = 0;
		}
		if( work->prof_max_act < act_time ){
			work->prof_max_act = act_time;
			work->prof_max_act_time = GV_Time;
		}
		if( work->prof_max_draw_time + 60 < GV_Time ){
			work->prof_max_draw = 0;
			work->prof_max_draw_time = 0;
		}
		if( work->prof_max_draw < draw_time ){
			work->prof_max_draw = draw_time;
			work->prof_max_draw_time = GV_Time;
		}

		DEBUG_Locate( 428-16, DRAW_HEIGHT - 32 - 32 - 16, 0 );
		DEBUG_Printf( "ACT%3d:%3d\n", act_time, work->prof_max_act );
		DEBUG_Printf( "DRW%3d:%3d\n", draw_time, work->prof_max_draw );
	}
}

static void ViewNothing( GV_PAD *pad )
{
}

/* -------------------------------------------------------- */
/*
	メイン
*/

static void ( *func_table[] )( GV_PAD *pad ) = {
	ViewNothing,
	ViewProfileLog,
	ViewMemorySystem,
	ViewPolyCount,
	NULL
};

static void Act( Work *work )
{
	GV_PAD *pad;

	pad = &GV_PadData[ 1 ];

	if( GM_Debug2PMode == GM_DEBUG_MODE_VIEWER ){
		DEBUG_Locate( 32, 16, 0 );
		DEBUG_Printf( "PRESS 2P-L1 CHANGE MODE\n" );
		if( pad->press & PAD_L1 ){
			work->func_p ++;
			if( *work->func_p == NULL ){
				work->func_p = func_table;
			}
		}
	}

	DEBUG_SetColor( 200, 200, 200 );

	(**work->func_p)( pad );
}

static int GetResources( Work *work )
{
	work->func_p = func_table;
	return 0;
}

void *GM_StartDebugViewer( void )
{
	Work		*work ;

	work = &viewer_work;

	GV_InsertActorPriority( GV_ACTOR_MANAGER, work, GM_DAEMON_PRIO + 0x10 );
	GV_SetActor( work, Act, NULL );

	GetResources( work );
	return (void *)work ;
}

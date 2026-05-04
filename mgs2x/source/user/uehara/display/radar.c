/*
	radar.c
		レーダー

	1999/10/08 K.Uehara
	$Id: radar.c,v 1.1.1.3 2002/11/19 11:51:33 Yoshizawa1 Exp $
*/

#include <eekernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <math.h>

#include	"def_dma.h"
#include	"gameheader.h"

#define RADAR_VIEW_W		(20000.0F)	/* 見える範囲(1/1000m) */

#define RADAR_WINDOW_W		120
//#define RADAR_WINDOW_W		100

#if DRAW_HEIGHT == 224
#define RADAR_WINDOW_H		52
#else
#define RADAR_WINDOW_H		52*2
#endif

#define RADAR_WINDOW_X0		(DRAW_WIDTH - 16 - RADAR_WINDOW_W )
#define RADAR_WINDOW_X1		(DRAW_WIDTH - 16)
#define RADAR_WINDOW_Y0		(8)
#define RADAR_WINDOW_Y1		(RADAR_WINDOW_Y0 + RADAR_WINDOW_H)

#define RADAR_ZOOM_RATE_W	( RADAR_WINDOW_W / RADAR_VIEW_W / ASPECT_X() )	/* 横方向 */
#define RADAR_ZOOM_RATE_H	( ( RADAR_ZOOM_RATE_W * DRAW_HEIGHT / DRAW_WIDTH ) / ( ASPECT_Y() * 2.0F ) )
/* 縦方向 */

#define UPPER_RANGE			(1000.0F)
#define DOWN_RANGE			(-800.0F)

#define RADAR_ZOOM_RATE_Y	( 2.0F / ( UPPER_RANGE - DOWN_RANGE ) )

/* ---------------------------------------------------------------------- */
/*
	ワーク、転送データ
*/

typedef struct {
	unsigned int qwc;
	unsigned int addr;
	unsigned int vif1;
	unsigned int vif2;
} DMA_TAG;

typedef struct {
	u_long data;
	u_long addr;
} GIF_AD_DATA;

typedef struct {
	sceGifTag	gif_tag;
	GIF_AD_DATA alpha;
	GIF_AD_DATA	offset;
	GIF_AD_DATA zbuf;
	GIF_AD_DATA	scissor;
	GIF_AD_DATA test;
} RADAR_WINDOW_SET;

typedef struct {
	sceGifTag	gif_tag;
	sceGsPrim	prim;
	u_long			rgba;
	u_long			xy1;
	u_long			xy2;
} RADAR_TILE_SET;

typedef struct {
	sceGifTag gif_tag;
	GIF_AD_DATA test;
} RADAR_DRAWPARAM_SET;

typedef struct {
	// ソート用ヘッダ
	DG_PRIM_HEADER header;

	DG_DMATAG	dma_tag;

	// 描画環境セット
	RADAR_WINDOW_SET window;
	// タイル描画
	RADAR_TILE_SET	base_tile;
	// 描画パラメータセット
	RADAR_DRAWPARAM_SET	drparam;
	sceGifTag end_tag;
	GIF_AD_DATA	 alpha;
} PACK_HEADER;

#define MAX_ROT_TABLE	64

enum {
	CHARA_COLOR_PLAYER = 0,
	CHARA_COLOR_ENEMY_BLUE,
	CHARA_COLOR_ENEMY_RED,
	CHARA_COLOR_ENEMY_YELOW,
	MAX_CHARA_COLOR
};

typedef struct {
	int color[ 4 ];
} LCOLOR ALIGN16;

typedef struct {
	/* size max = 0xb00 = 2816 */
	float matrix[ 4 ][ 4 ];
	/* 壁描画用 */
	FVECTOR yclip;
	int bright_z, dark_z, pad1, pad2;
	LCOLOR bright_color;
	LCOLOR dark_color;
	sceGifTag giftag;
	/* 位置描画用 */
	int player_z, enemy_z, pad11, pad12;
	sceGifTag pos_giftag;
	sceGifTag sight_giftag;
	sceGifTag no_sight_giftag;
	/* 視界計算用 */
	FVECTOR rot_table[ MAX_ROT_TABLE ];
	/*視界、キャラの表示色 */
	struct {
		LCOLOR pos_bright;
		LCOLOR pos_dark;
		LCOLOR sight_bright;
		LCOLOR sight_dark;
	} chara_color[ MAX_CHARA_COLOR ];
} VU1_PARAM ALIGN16;

#define VUSETCOL( a )	( (a) * 4 )

typedef struct {
	GV_ACT actor;
	int name;
	int display_flag;
	DG_PRIM prim;
	VU1_PARAM param;
//	u_long128 buffer[ 2 ][ 40960 / sizeof( u_long128 ) ];
	u_long128 buffer[ 2 ][ 81920 / sizeof( u_long128 ) ];
} Work;

/* ---------------------------------------------------------------------- */
/*
	マトリクス
*/

extern u_long128 Vu1DrawRadarMPGTag[ 1 ] ;	// データ転送用タグ
extern int Vu1DrawRadarFunc[] ;

enum {
	SEG_DRAW_START = 0,
	SEG_DRAW_NEXT = 1,
	CHARA_SIGHT_DRAW = 2
};

static FMATRIX default_matrix = {
	{
		{ RADAR_ZOOM_RATE_W, 0.0F, 0.0F, 0.0F },
		{ 0.0F, 0.0F, RADAR_ZOOM_RATE_Y, RADAR_ZOOM_RATE_Y },
		{ 0.0F, RADAR_ZOOM_RATE_H, 0.0F, 0.0F },
		{
			2048.0F, 2048.0F
			  , ( -(UPPER_RANGE+DOWN_RANGE)/2.0F ) * RADAR_ZOOM_RATE_Y
			  , ( -(UPPER_RANGE+DOWN_RANGE)/2.0F ) * RADAR_ZOOM_RATE_Y
		},
	}
};

static unsigned char default_colors[ 4 ][ 4 ] = {
	{ 0, 160, 72, 0x80 },
	{ 0, 64, 0, 0x80 },
};

static unsigned char default_chara_colors[][ 4 ][ 4 ] = {
	{
		{ 200, 200, 200, 0x80 },
		{ 100, 100, 100, 0x80 },
		{ 0, 160, 72, 0x80 },
		{ 0, 0, 0, 0x80 },
	},
	{
		{ 255, 100, 0, 0x80 },
		{ 200, 0, 0, 0x80 },
		{ 0x00, 0x80, 0x80, 0x80 },
		{ 0x00, 0x00, 0x10, 0x80 },
	},
	{
		{ 255, 100, 0, 0x80 },
		{ 200, 0, 0, 0x80 },
		{ 0xa0, 0x00, 0x00, 0x80 },
		{ 0x10, 0x00, 0x00, 0x80 },
	},
	{
		{ 255, 100, 0, 0x80 },
		{ 200, 0, 0, 0x80 },
		{ 0xa0, 0xa0, 0x00, 0x80 },
		{ 0x08, 0x08, 0x00, 0x80 },
	},
};

/* ---------------------------------------------------------------------- */
/*
	壁描画タグの設定
*/

static DMA_TAG *make_hzd_one_segment_tag( DMA_TAG *tag, HZX_BLOCK *block, int prog, 
										  HZX_VuSEG *seg, int n_segs )
{
	int 		i ;
	IVECTOR		*center ;

	/*
		将来的にはレーダー用のセグメント列を別持ちすることによる高速化も考える。
	*/

	/* ブロック中心を加算レジスタにセット */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 1 ) ;
	tag->vif1 = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	tag->vif2 = SCE_VIF1_SET_STROW( 0 ) ;
	tag ++ ;
	center = ( IVECTOR * )tag ; 
	center->vx = block->tx ;
	center->vy = block->ty ;
	center->vz = block->tz ;
	center->vw = 0 ;
	tag ++ ;
	for ( i = 0; i < n_segs; i ++, seg ++ ) {
	    if ( seg->atr & HZX_SEG_NO_DISP_RADAR ){
			continue ;
		}
	    /* 頂点数等 */
	    tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 1 ) ;
	    tag->addr = DMATAG_SET_ADDR( seg->tag ) ;
	    tag->vif1 = SCE_VIF1_SET_STMOD( 0x00, 0 ) ; /* 加算書き込みＯＦＦ */
	    tag->vif2 = SCE_VIF1_SET_UNPACK( 0x8000, 1, VIF_DATA128, 0 ) ;
	    tag ++ ;

	    /* 頂点データ */
	    tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, seg->size ) ;
	    tag->addr = DMATAG_SET_ADDR( seg->verts ) ;
	    tag->vif1 = SCE_VIF1_SET_STMOD( 0x01, 0 ) ; /* 加算書き込み */
	    tag->vif2 = SCE_VIF1_SET_UNPACK( 0x8001, seg->size * 2, 0x0d, 0 ) ; /* V4-16 */
	    tag ++ ;

		/* ダブルバッファ切替え */
	    tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
	    tag->vif1 = SCE_VIF1_SET_FLUSHE( 0 ) ;
	    tag->vif2 = SCE_VIF1_SET_MSCAL( Vu1DrawRadarFunc[ prog ], 0 ) ;
	    tag ++ ;

	    prog = SEG_DRAW_NEXT ;
	}
	return tag;
}

static DMA_TAG *make_hzx_segment_tag( Work *work, FVECTOR *pos, DMA_TAG *tag )
{
	HZX_HDL *hzd;
	int i, n, prog;
	int *inside;
	HZX_GRP *grp;
	HZX_BLOCK *block;
	int	bit, bit2, group ;

	hzd = HZX_GetCurrentHzx() ;
	bit = HZX_CurrentGroupID ;
	prog = SEG_DRAW_START;

#if 1
	while( bit != 0 ) {
	    group = GV_GetNo( bit ) ;
	    bit2 = GV_GetBit( group ) ;
	    if ( group >= 0 && group < hzd->def->n_groups ){
		    grp = hzd->def->groups + group ;
		    inside = HZX_GetNearBlockID( grp, pos, RADAR_VIEW_W, &n ) ;
		    for( i = 0; i < n; i++ ){
				block = grp->blocks + *inside;
				inside ++ ;
				if ( block->n_segs > 0 ) {
					tag = make_hzd_one_segment_tag( tag, block, prog, 
												    block->segs, block->n_segs );
					prog = SEG_DRAW_NEXT ;
				}
#if 0
				if ( block->n_bul_segs > 0 ) {
					tag = make_hzd_one_segment_tag( tag, block, prog, 
												    block->bul_segs, block->n_bul_segs );
					prog = SEG_DRAW_NEXT ;
				}
#endif
		    }
		}
		bit &= ~bit2 ;
	}
#endif
    /* 終了タグ */
    tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
    tag->vif1 = SCE_VIF1_SET_STMOD( 0x00, 0 ) ; /* 加算書き込みをＯＦＦ */
    tag->vif2 = SCE_VIF1_SET_NOP( 0 );
    tag ++ ;

	return tag;
}

/* ---------------------------------------------------------------------- */
/*
	プレイヤーと敵兵の位置描画タグの設定
*/

static DMA_TAG *make_pos_display_tag( Work *work, FVECTOR *pos, DMA_TAG *tag )
{
	DMA_TAG *top;
	RADAR_CTRL *p;
	int n, size;

	/*
		プログラムはすでに読み込まれているので、
		データ転送してキックするだけ
	*/

	if( GV_Time % 16 < 3 ) return tag;

	top = tag;
	tag += 2;

	n = 0;
	for( p = GM_RadarControlGetTop(); p != NULL; p = p->next ){
		const int w = ( 4096 / MAX_ROT_TABLE );
		int roty, angle, width, col;
		float length;

		if( !( p->flag & RADAR_VISIBLE ) ) continue;

		*( FVECTOR * )tag = *( p->pos );
		tag ++;
		angle = p->angle;
		length = p->range;		// 敵兵の視界の長さ
		col = p->col;			// 色

		if( !( p->flag & RADAR_SIGHT ) ) col = -1;

		width = angle / 4;	// セグメント1こあたり。

		roty = p->dir - 2 * width;
		if( roty < 0 ){
			roty = ( ( roty - w / 2 ) / w + MAX_ROT_TABLE ) % MAX_ROT_TABLE;
		} else {
			roty = ( ( roty + w / 2 ) / w + MAX_ROT_TABLE ) % MAX_ROT_TABLE;
		}

		( ( unsigned int * )tag )[ 0 ] = roty;		/* 視野方向-視野角の1/2 */
		( ( unsigned int * )tag )[ 1 ] = width / w;		/* 視野角の1/4 */
		( ( unsigned int * )tag )[ 2 ] = col * 4;			/* color */
		( ( float * )tag )[ 3 ] = length;			/* 視界のながさ */
		tag ++;

		( ( float * )tag )[ 0 ] = 0.0F;
		( ( float * )tag )[ 1 ] = 0.0F;
		( ( float * )tag )[ 2 ] = p->range_zoom_rate;
		( ( float * )tag )[ 3 ] = p->range_center - pos->vy * p->range_zoom_rate;

		tag ++;
		n++;
	}

	if( n == 0 ) return top;

	// 末尾のタグの設定

	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	tag->vif1 = SCE_VIF1_SET_FLUSHE( 0 );
	tag->vif2 = SCE_VIF1_SET_MSCAL( Vu1DrawRadarFunc[ CHARA_SIGHT_DRAW ], 0 );
	tag ++;

	// 先頭のタグの設定
	size = n * 3 + 1;
	top->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, size );
	top->vif1 = SCE_VIF1_SET_STCYCL( 1, 1, 0 );
	top->vif2 = SCE_VIF1_SET_UNPACK( 0x8000, size, VIF_DATA128, 0 );
	*( unsigned int * )&( top + 1 )[ 0 ] = n;

	return tag;
}


/* ---------------------------------------------------------------------- */
/* TEST */
static void Koretest( Work *work )
{
	static int mode=ALERT_MODE_SNEAK ;
	PACK_HEADER *pack;
	RADAR_TILE_SET *tp1, *tp2;

	pack = (PACK_HEADER *)work->buffer[ 0 ] ;
	tp1 = &( pack->base_tile );
	pack = (PACK_HEADER *)work->buffer[ 1 ] ;
	tp2 = &( pack->base_tile );

	
	if ( GM_AlertMode != mode ) {
		switch ( GM_AlertMode ) {
			case ALERT_MODE_SNEAK :
				tp1->rgba = tp2->rgba = SCE_GS_SET_RGBAQ( 0, 0, 0, 0x80/8 * 4, 0 );
			break ;
			case ALERT_MODE_ALERT :
				tp1->rgba = tp2->rgba = SCE_GS_SET_RGBAQ( 100, 0, 0, 0x80/8 * 4, 0 );
			break ;
			case ALERT_MODE_AVOID :
				tp1->rgba = tp2->rgba = SCE_GS_SET_RGBAQ( 100, 100, 0, 0x80/8 * 4, 0 );

				tp1->xy1 = SCE_GS_SET_XYZ( ( 2048 - RADAR_WINDOW_W / 2 ) << 4
									  , ( 2048 - RADAR_WINDOW_H / 2 ) << 4, 50 );
				tp1->xy2 = SCE_GS_SET_XYZ( ( 2048 + RADAR_WINDOW_W / 2  + 1 ) << 4
										  , ( 2048 + RADAR_WINDOW_H / 2  + 1 ) << 4, 50 );
				tp2->xy1 = SCE_GS_SET_XYZ( ( 2048 - RADAR_WINDOW_W / 2 ) << 4
									  , ( 2048 - RADAR_WINDOW_H / 2 ) << 4, 50 );
				tp2->xy2 = SCE_GS_SET_XYZ( ( 2048 + RADAR_WINDOW_W / 2  + 1 ) << 4
										  , ( 2048 + RADAR_WINDOW_H / 2  + 1 ) << 4, 50 );
			break ;
			case ALERT_MODE_SEARCH :
				tp1->rgba = tp2->rgba = SCE_GS_SET_RGBAQ( 0, 0, 0, 0x80/8 * 4, 0 );
			break ;
		}
		mode = GM_AlertMode ;
	}

	
	if ( GM_AlertMode == ALERT_MODE_ALERT ) {
		int height, l ;

		height = RADAR_WINDOW_H * GM_AlertLevel / ALERT_LEVEL_MAX ;

//printf("GM_AlertLevel=%d height=%d\n",GM_AlertLevel, height)  ;

		tp1->xy1 = SCE_GS_SET_XYZ( ( 2048 - RADAR_WINDOW_W / 2 ) << 4
							  , ( 2048 - height / 2 ) << 4, 50 );
		tp1->xy2 = SCE_GS_SET_XYZ( ( 2048 + RADAR_WINDOW_W / 2  + 1 ) << 4
								  , ( 2048 + height / 2  + 1 ) << 4, 50 );

		tp2->xy1 = SCE_GS_SET_XYZ( ( 2048 - RADAR_WINDOW_W / 2 ) << 4
							  , ( 2048 - height / 2 ) << 4, 50 );
		tp2->xy2 = SCE_GS_SET_XYZ( ( 2048 + RADAR_WINDOW_W / 2  + 1 ) << 4
								  , ( 2048 + height / 2  + 1 ) << 4, 50 );
		if ( GM_AlertLevel == ALERT_LEVEL_MAX ) {
			l = GV_Time & 0x1f ;
			
			tp1->rgba = tp2->rgba = SCE_GS_SET_RGBAQ( 250-l, 150-l, 150-l, 0x80/8 * 4, 0 );
		} else if ( GM_AlertLevel > ALERT_LEVEL_MAX*3/4 ) {
			l = 150 * (ALERT_LEVEL_MAX - GM_AlertLevel) / (ALERT_LEVEL_MAX/4) ;
			tp1->rgba = tp2->rgba = SCE_GS_SET_RGBAQ( 250-l, 150-l, 150-l, 0x80/8 * 4, 0 );
		} else {
			tp1->rgba = tp2->rgba = SCE_GS_SET_RGBAQ( 100, 0, 0, 0x80/8 * 4, 0 );
		}
	}

}
/* ---------------------------------------------------------------------- */
/*
	ACT
*/

static inline float pos_norm( float pos )
{
	float res;

	asm( "
		cvt.w.s $f1,%1
		cvt.s.w %0,$f1
		" : "=f"(res) : "f"(pos) : "$f1" );
	return res;
}

static void Act( Work *work )
{
	PACK_HEADER *ph;
	DMA_TAG *tag;
	FVECTOR *pos;

	GM_ResetMenuStatus( MENU_RADAR_ON ) ;
	{
		GV_MSG *msg;
		int n;
		if( ( n = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
			for( ; n > 0; n-- ){
				if( ( work->display_flag = msg->message[ 0 ] ) == 0 ){
					DG_InvisiblePrim( &work->prim );
					GV_WaitMessage( work, work->name );
					return;
				}
				msg++;
			}
		}
	}

Koretest( work ) ;

#if 0
	if( GM_PlayerStatus & PLAYER_BEHIND ){
		DG_InvisiblePrim( &work->prim );
		return;
	}
#else
	if( GM_CheckMenuStatus( MENU_RADAR_OFF ) ) {
		DG_InvisiblePrim( &work->prim );
		return;
	}
#endif
	GM_SetMenuStatus( MENU_RADAR_ON ) ;

	DG_VisiblePrim( &work->prim );

	pos = GM_RadarGetCenter();

	/* マトリクスの並行移動設定 */
	work->param.matrix[ 3 ][ 0 ]
		= pos_norm( 2048.5F - pos->vx * RADAR_ZOOM_RATE_W );
	work->param.matrix[ 3 ][ 1 ]
		= pos_norm( ( 2048.5F - pos->vz * RADAR_ZOOM_RATE_H ) * 2.0F ) / 2.0F;
	work->param.matrix[ 3 ][ 2 ] = work->param.matrix[ 3 ][ 3 ]
		= ( -( (UPPER_RANGE+DOWN_RANGE)/2.0F + pos->vy ) ) * RADAR_ZOOM_RATE_Y;

	ph = ( PACK_HEADER * )work->buffer[ DG_Clock ];
	ph->dma_tag.qwc = ( ph->dma_tag.qwc & 0x8FFFFFFF ) | ( DMATAG_ID_CNT );

	tag = ( DMA_TAG * )( ph + 1 );
	
	{
		int x0, x1, y0, y1;
		x0 = RADAR_WINDOW_X0;
		y0 = RADAR_WINDOW_Y0;
		x1 = RADAR_WINDOW_X1;
		y1 = RADAR_WINDOW_Y1;
		ph->window.offset.data = SCE_GS_SET_XYOFFSET( ( 2048 - ( x0 + x1 ) / 2 ) << 4
				 , ( ( 2048 - ( y0 + y1 ) / 2 ) << 4 ) + ( DG_CurrentField ? 8 : 0 ));
	}

	/* 初期化データ転送 */

	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, sizeof( VU1_PARAM ) / sizeof( u_long128 ) );
	tag->addr = DMATAG_SET_ADDR( &work->param );
	tag->vif1 = SCE_VIF1_SET_STCYCL( 1, 1, 0 );		// 順次転送
	tag->vif2 = SCE_VIF1_SET_UNPACK( 0x0, sizeof( VU1_PARAM ) / sizeof( u_long128 )
									, VIF_DATA128, 0 );
	tag ++;

	/* ダブルバッファ設定 */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	tag->vif1 = SCE_VIF1_SET_BASE( 0x0B00/16, 0 );
	tag->vif2 = SCE_VIF1_SET_OFFSET( 0x1A00/16, 0 );
	tag ++;

	/* マイクロプログラム転送 */
	*( u_long128 * )tag = Vu1DrawRadarMPGTag[ 0 ] ;
	tag ++;

#if 1
	/* HZXブロックを検索してDMAタグを作る */
	tag = make_hzx_segment_tag( work, pos, tag );
#endif

//printf( "%d\n", ( u_int )tag - ( u_int )work->buffer[ DG_Clock ] ) ;

	/* ZBUFFER を更新しない設定 */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 2 );
	tag->vif1 = SCE_VIF1_SET_NOP( 0 );
	tag->vif2 = SCE_VIF1_SET_DIRECT( 2, 0 );
	tag++;

	DG_SET_GIFTAG( ( sceGifTag * )tag, .EOP = 1, .NLOOP = 1, .NREG = 1, .REGS0 = GS_REGS_AD );
	tag++;
	{
		GIF_AD_DATA *zbuf = ( GIF_AD_DATA * )tag;
		zbuf->addr = SCE_GS_ZBUF_1;
		zbuf->data = SCE_GS_SET_ZBUF(ZBUFFER_PAGE()/2048, Z_BUFFER_COLOR_MODE(), 1 );
		tag++;
	}


	/* プレイヤーと敵兵の表示 */
	tag = make_pos_display_tag( work, pos, tag );

	/* 終了待ち */

	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	tag->vif1 = SCE_VIF1_SET_FLUSHA( 0 );
	tag->vif2 = SCE_VIF1_SET_NOP( 0 );
	tag ++;

	/* 描画環境を戻す */
	tag = ( DMA_TAG * )DG_PopDefaultDrawEnv( DG_Chanl( DG_CHANL_MENU ), ( u_int * )tag );

	/* 終端タグ */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 );
	tag->vif1 = SCE_VIF1_SET_NOP( 0 );
	tag->vif2 = SCE_VIF1_SET_NOP( 0 );

//	printf( "size %X\n", tag - ( DMA_TAG * )( ph + 1 ) );
}

static void Die( Work *work )
{
	DG_DequeuePrim( &( work->prim ) );
}

static void set_window( PACK_HEADER *packet )
{
	RADAR_WINDOW_SET *wp;
	int x0, x1, y0, y1;

	wp = &( packet->window );

	// GIFタグの初期化
	DG_SET_GIFTAG( &wp->gif_tag
		, .FLG = SCE_GIF_PACKED, .NLOOP = 5, .NREG = 1, .REGS0 = GS_REGS_AD );

	// アルファブレンディング設定
	wp->alpha.addr = SCE_GS_ALPHA_1;
	wp->alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 );

	wp->zbuf.addr = SCE_GS_ZBUF_1;
	wp->zbuf.data = SCE_GS_SET_ZBUF(ZBUFFER_PAGE()/2048, Z_BUFFER_COLOR_MODE(), 0 );

	wp->test.addr = SCE_GS_TEST_1;
	wp->test.data = SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 );

	// ウインドウサイズの設定
	x0 = RADAR_WINDOW_X0;
	y0 = RADAR_WINDOW_Y0;
	x1 = RADAR_WINDOW_X1;
	y1 = RADAR_WINDOW_Y1;

	wp->scissor.addr = SCE_GS_SCISSOR_1;
	wp->scissor.data = SCE_GS_SET_SCISSOR( x0, x1, y0, y1 );

	// 描画オフセットの設定

	wp->offset.addr = SCE_GS_XYOFFSET_1;
	wp->offset.data = SCE_GS_SET_XYOFFSET( ( 2048 - ( x0 + x1 ) / 2 ) << 4
										   , ( 2048 - ( y0 + y1 ) / 2 ) << 4 );
}

static void set_drawparam( PACK_HEADER *packet )
{
	RADAR_DRAWPARAM_SET *pp;

	pp = &( packet->drparam );

	// GIFタグの初期化
	DG_SET_GIFTAG( &pp->gif_tag
		, .FLG = SCE_GIF_PACKED, .NLOOP = 1, .NREG = 1, .REGS0 = GS_REGS_AD );

	pp->test.addr = SCE_GS_TEST_1;
	pp->test.data = SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 2 );
}

static void set_base_tile( PACK_HEADER *packet )
{
	RADAR_TILE_SET *tp;
	int x0, x1, y0, y1;

	tp = &( packet->base_tile );

	// GIFTAGの初期化
	DG_SET_GIFTAG( &tp->gif_tag
				   , .FLG = SCE_GIF_REGLIST, .NLOOP = 1, .NREG = 4
				   , .REGS0 = GS_REGS_PRIM, GS_REGS_RGBA, GS_REGS_XYZF2, GS_REGS_XYZF2 );
	DG_SET_GS_REG( &tp->prim
				   , .PRIM = SCE_GS_PRIM_SPRITE
				   , .ABE = 1, .FST = 1 );

	tp->rgba = SCE_GS_SET_RGBAQ( 0, 0, 0, 0x80/8 * 4, 0 );

	// ウインドウサイズの設定
	x0 = RADAR_WINDOW_X0;
	y0 = RADAR_WINDOW_Y0;
	x1 = RADAR_WINDOW_X1;
	y1 = RADAR_WINDOW_Y1;

	tp->xy1 = SCE_GS_SET_XYZ( ( 2048 - RADAR_WINDOW_W / 2 ) << 4
							  , ( 2048 - RADAR_WINDOW_H / 2 ) << 4, 50 );
	tp->xy2 = SCE_GS_SET_XYZ( ( 2048 + RADAR_WINDOW_W / 2  + 1 ) << 4
							  , ( 2048 + RADAR_WINDOW_H / 2  + 1 ) << 4, 50 );
}

static void copy_color( LCOLOR *dest, unsigned char *src )
{
	int i;
	int *d;
	d = &( dest->color[ 0 ] );
	for( i = 0; i < 4; i ++ ){
		*( d ++ ) = *( src ++ );
	}
}

static void copy_chara_color( LCOLOR *dest, unsigned char table[ 4 ][ 4 ] )
{
	int i, j;
	int *d;
	for( j = 0; j < 4; j++ ){
		unsigned char *s;
		d = &( dest->color[ 0 ] );
		s = table[ j ];
		for( i = 0; i < 4; i ++ ){
			*( d ++ ) = *( s ++ );
		}
		dest ++;
	}
}

static void set_vu1_param( Work *work )
{
	VU1_PARAM *param;
	
	param = &( work->param );
	DG_COPY_MAT( param->matrix, &default_matrix	);

	param->yclip = ( FVECTOR ){
		0.0F, 0.0F, 0.0F, RADAR_ZOOM_RATE_Y
	};

	param->bright_z = 200;
	param->dark_z = 100;

	copy_color( &param->bright_color, default_colors[ 0 ] );
	copy_color( &param->dark_color, default_colors[ 1 ] );

	DG_SET_GIFTAG( &param->giftag, .FLG = SCE_GIF_PACKED, .NLOOP = 3, .PRE = 1
				   , .PRIM = SCE_GS_SET_PRIM( SCE_GS_PRIM_LINE, 0,0,0,0,0,0,0,0 )
				   , .EOP = 1, .NREG = 3
				   , .REGS0 = GS_REGS_RGBA, GS_REGS_XYZ2, GS_REGS_XYZ2 );

	DG_SET_GIFTAG( &param->pos_giftag, .FLG = SCE_GIF_PACKED, .NLOOP = 1, .PRE = 1
				   , .PRIM = SCE_GS_SET_PRIM( SCE_GS_PRIM_POINT, 0,0,0,0,0,0,0,0 )
				   , .EOP = 0, .NREG = 2
				   , .REGS0 = GS_REGS_RGBA, GS_REGS_XYZ2 );

	DG_SET_GIFTAG( &param->sight_giftag, .FLG = SCE_GIF_PACKED, .PRE = 1
				   , .PRIM = SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1,0,0,1,0,0,0,0 )
				   , .NREG = 8, .NLOOP = 1, .EOP = 1
				   , .REGS0 = GS_REGS_RGBA, GS_REGS_XYZ2, GS_REGS_RGBA, GS_REGS_XYZ2
				   , GS_REGS_XYZ2, GS_REGS_XYZ2, GS_REGS_XYZ2, GS_REGS_XYZ2 );

	DG_SET_GIFTAG( &param->no_sight_giftag, .FLG = SCE_GIF_PACKED, .PRE = 0
				   , .NREG = 1, .NLOOP = 0, .EOP = 1, .REGS0 = GS_REGS_NOP );

	copy_chara_color( ( LCOLOR * )&param->chara_color[ 0 ], default_chara_colors[ 0 ] );
	copy_chara_color( ( LCOLOR * )&param->chara_color[ 1 ], default_chara_colors[ 1 ] );
	copy_chara_color( ( LCOLOR * )&param->chara_color[ 2 ], default_chara_colors[ 2 ] );
	copy_chara_color( ( LCOLOR * )&param->chara_color[ 3 ], default_chara_colors[ 3 ] );

	/* 視界表示用ベクトルテーブル */
	{
		int i;
		FVECTOR *fv;
		fv = param->rot_table;
		for( i = 0; i < MAX_ROT_TABLE; i++ ){
			fv->vx = sinf( i * 2 * PI / MAX_ROT_TABLE ) * RADAR_ZOOM_RATE_W;
			fv->vy = cosf( i * 2 * PI / MAX_ROT_TABLE ) * RADAR_ZOOM_RATE_H;
			*( int * )&fv->vz = 255;
			*( int * )&fv->vw = 0;
			fv++;
		}
	}
}

static void InitPacket( PACK_HEADER *packet )
{
	DG_DMATAG *dt;

	// DMAタグの初期化
	dt = &( packet->dma_tag );
	dt->addr = NULL;
	dt->qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 );		// size = 0( dummy )
	dt->vifcode[ 0 ] = SCE_VIF1_SET_NOP( 0 );
	dt->vifcode[ 1 ] = SCE_VIF1_SET_DIRECT( 0, 0 );		// size = 0( dummy )

	// 描画環境セット
	set_window( packet );
	set_drawparam( packet );

	// ベースタイルセット
	set_base_tile( packet );

	// サイズ情報の確定
	/* 仮 */
	{
		int size;
		sceGifTag *end_tag;
		size = ( sizeof( PACK_HEADER ) - sizeof( DG_PRIM_HEADER ) - sizeof( DG_DMATAG ) )
			/ sizeof( u_long128 );

		dt->qwc = DMATAG_SET_QWC( DMATAG_ID_RET, size );
		dt->vifcode[ 1 ] = SCE_VIF1_SET_DIRECT( size, 0 );

		end_tag = &( packet->end_tag );
		DG_SET_GIFTAG( end_tag, .EOP = 1, .NLOOP = 1, .NREG = 1, .REGS0 = GS_REGS_AD );
		packet->alpha.addr = SCE_GS_ALPHA_1;
		packet->alpha.data = SCE_GS_SET_ALPHA( 0,2,2,1,0x80 );
	}
}

static int GetResources( Work *work, int name )
{
	DG_PRIM *prim;

	prim = &work->prim;
	GV_ZeroMemory( prim, sizeof( DG_PRIM ) );

	prim->type = DG_PRIM_SORTONLY | DG_PRIM_FREEPRIM;

	prim->chanl = DG_CHANL_MENU;
	prim->n_prims = prim->n_packet = 1;

	prim->near_z = -1;
	DG_QueuePrim( prim );

	/* VU1用パラメータテーブルの初期化 */
	set_vu1_param( work );

	/* プリミティブの初期化 */
	{
		int i;

		for( i = 0; i < 2; i ++ ){
			PACK_HEADER *pack;
			/* パケット領域の確保 */
			prim->packs[ i ] = work->buffer[ i ];
			pack = ( PACK_HEADER * )prim->packs[ i ];
			InitPacket( pack );
			pack->header.sort_z = prim->near_z + 1;
		}
	}

	/* 描画環境設定パケット */

	work->display_flag = 1;
	work->name = name;

	return 0;
}
void *NewRadar( int name, int map )
{
	/* 通常のキャラ扱い */

	Work		*work;

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) );
	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die );
		if( GetResources( work, name ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

/*
	radar.c
		レーダー

	1999/10/08 K.Uehara
	2000/01/12 K.Sigeno 引継ぎ
	$Id: radar.c,v 1.2 2002/12/05 18:41:58 takaki Exp $
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
#include <libvu0.h>


#include	"def_dma.h"
#include	"gameheader.h"

#include	"radsprite.h"

#include	"radsize.h"

extern float _RsinF( int ) ;
extern float _RcosF( int ) ;

#define CLOSE_SPEED	(SC_RATE_H*2.0F)
#define RADAR_VIEW_W		(20000.0F)	/* 見える範囲(1/1000m) */

#define RADAR_ZOOM_RATE_W	( RADAR_WINDOW_W / RADAR_VIEW_W / TARGET_ASPECT_X )	/* 横方向 */
#define RADAR_ZOOM_RATE_H	( ( RADAR_ZOOM_RATE_W * DRAW_HEIGHT / DRAW_WIDTH ) / ( ASPECT_Y() ) )
/* 縦方向 */
/*レーダー中心のスクリーン座標*/
#define RADAR_WINDOW_CX (RADAR_WINDOW_X0 + (RADAR_WINDOW_W/2))
#define RADAR_WINDOW_CY (RADAR_WINDOW_Y0 + (RADAR_WINDOW_H/2))

#define RADAR_FRAME (0)

/*キャラの写る範囲*/

#define UPPER_RANGE			( 500.0F)
#define DOWN_RANGE			( -1250.0F)

#define RADAR_ZOOM_RATE_Y	( 2.0F / ( UPPER_RANGE - DOWN_RANGE ) )

/*実験 レーダーキャラ上限*/
#define R_CTRL_MAX	(64)
/*視力と関係なく表示視界を設定*/
#define RAD_RANGE_FIX (6000.0F)

#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))


#define SET_COLOR_SPIN(_prim,_col) (*(int*)&(_prim)->spin.col = (_col))
#define SET_COLOR_SPIN2(_prim,_r,_g,_b,_a) SET_COLOR_SPIN((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))



#define BASE_ALPHA  (0x80 / 8 * 6)



#define SNK_R	(71)
#define SNK_G	(68)
#define SNK_B	(64)

#define SRC_R	(58)
#define SRC_G	(90)
#define SRC_B	(4)

#define ALT_R	(140)
#define ALT_G	(5)
#define ALT_B	(10)

#define ALT_LEVEL_R	(76)
#define ALT_LEVEL_G	(2)
#define ALT_LEVEL_B	(0)

#define ESC_R	(155)
#define ESC_G	(46)
#define ESC_B	(0)

#if 0
#define ESC_LEVEL_R	(90)
#define ESC_LEVEL_G	(24)
#define ESC_LEVEL_B	(48)
#else
#define ESC_LEVEL_R	(135)
#define ESC_LEVEL_G	(40)
#define ESC_LEVEL_B	(0)
#endif

#define JAM_R	(9)
#define JAM_G	(103)
#define JAM_B	(44)

#define JAM_LEVEL_R	(10)
#define JAM_LEVEL_G	(77)
#define JAM_LEVEL_B	(58)

#define SET_COLOR_SNK(_prim) SET_COLOR_2DPRIM2(_prim,SNK_R,SNK_G,SNK_B,0x80);
#define SET_COLOR_SRC(_prim) SET_COLOR_2DPRIM2(_prim,SRC_R,SRC_G,SRC_B,0x80);
#define SET_COLOR_ALT(_prim) SET_COLOR_2DPRIM2(_prim,ALT_R,ALT_G,ALT_B,0x80);
#define SET_COLOR_ESC(_prim) SET_COLOR_2DPRIM2(_prim,ESC_R,ESC_G,ESC_B,0x80);
#define SET_COLOR_JAM(_prim) SET_COLOR_2DPRIM2(_prim,JAM_R,JAM_G,JAM_B,0x80);
#define SET_COLOR_DEF(_prim) SET_COLOR_2DPRIM2(_prim,0x80,0x80,0x80,0x80);

#define SET_COLOR_SNK_ALP(_prim ,_alp) SET_COLOR_2DPRIM2(_prim,SNK_R,SNK_G,SNK_B,_alp);
#define SET_COLOR_SRC_ALP(_prim,_alp) SET_COLOR_2DPRIM2(_prim,SRC_R,SRC_G,SRC_B,_alp);
#define SET_COLOR_ALT_ALP(_prim,_alp) SET_COLOR_2DPRIM2(_prim,ALT_LEVEL_R,ALT_LEVEL_G,ALT_LEVEL_B,_alp);
#define SET_COLOR_ESC_ALP(_prim,_alp) SET_COLOR_2DPRIM2(_prim,ESC_LEVEL_R,ESC_LEVEL_G,ESC_LEVEL_B,_alp);
#define SET_COLOR_JAM_ALP(_prim,_alp) SET_COLOR_2DPRIM2(_prim,JAM_LEVEL_R,JAM_LEVEL_G,JAM_LEVEL_B,_alp);
#define SET_COLOR_DEF_ALP(_prim,_alp) SET_COLOR_2DPRIM2(_prim,0x80,0x80,0x80,_alp);

/* work->statusにセット */
#define RAD_DEFAULT				(0x0000) 
#define RAD_SCN_INVISIBLE	(0x0001) /*シナリオからの非表示*/

#define RAD_STATE_CHAFF (ALERT_MODE_SEARCH + 1)

#define _ABSf(x) ((x>=0.0f)?(x):(-(x)))

/*POINT*/
static void SetRGBA_Point(SPR_OBJ *obj,int r,int g,int b,int a){
	obj->point.col.r = r ;
	obj->point.col.g = g ;
	obj->point.col.b = b ;
	obj->point.col.a = a ;
}
static int RAD_Fix2Int(SPR_FIX fix){
	int res ;
	res = (int)(fix>>4) ;
	return res ;
}
/**/
static void RotVecXZ(FVECTOR *pos,FVECTOR *res ,int dir){
	SVECTOR	rot;
	FVECTOR	tmppos;
	tmppos = *pos ;
	tmppos.vy = 0.0F ;
	tmppos.vw = 0.0F ;
	rot.vx = 0; rot.vy = (dir)&4095; rot.vz = 0;
	DG_SetPos2( &DG_ZeroVector, &rot ) ;
//	DG_PutVector( pos, res, 1 ) ;
	DG_PutVector( &tmppos, &tmppos, 1 ) ;
	res->vx = tmppos.vx ;
	res->vz = tmppos.vz ;
}
#define		ALLOW_COL	(0x80)
#if 0
static void SetArrowCol(RADAR_CTRL *r_ctrl){
	int r,g,b;
	switch (r_ctrl->col){
		case RADAR_COLOR_PLAYER :
			r = ALLOW_COL; g = ALLOW_COL; b = ALLOW_COL;
			break;
		case RADAR_COLOR_BLUE :
			r = 0x40; g = 0x40; b = 0xff;
			break;
		case RADAR_COLOR_RED :
			r = ALLOW_COL; g = 0x00; b = 0x00;
			break;
		case RADAR_COLOR_YELOW :
			r = ALLOW_COL; g = ALLOW_COL; b = 0x00;
			break;
		default :
			r = ALLOW_COL; g = ALLOW_COL; b = ALLOW_COL;
			break;
	}
	SET_COLOR_SPIN2(r_ctrl->sight,r,g,b,0x80) ;
	SetRGBA_Point(r_ctrl->face,(r+(0x40))&255,(g+(0x40))&255,
	(b+(0x40))&255,0x80);
}
#endif
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
	u_long64 data;
	u_long64 addr;
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
	u_long64			rgba;
	u_long64			xy1;
	u_long64			xy2;
} RADAR_TILE_SET;

typedef struct {
	sceGifTag gif_tag;
	GIF_AD_DATA test;
} RADAR_DRAWPARAM_SET;

typedef struct {
	// ソート用ヘッダ
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

typedef ALIGN16_DECL(struct) {
	int color[ 4 ];
} LCOLOR ;

typedef ALIGN16_DECL(struct) {
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
} VU1_PARAM ;

#define VUSETCOL( a )	( (a) * 4 )

typedef struct {
	SPR_OBJ		* empty;	/* リンクのルート  */
	SPR_OBJ		* mode ;	/* alert  */
	SPR_OBJ		* use ;	/* 下部の NO USE表示  */
	SPR_OBJ		* noise ;	/* 妨害エフェクト */
	SPR_OBJ		* radio ;	/* 無線エフェクト */
	SPR_OBJ		* level ;	/* 危険値グラフ */
	SPR_OBJ		* dot ;		/* modeゲージの点滅部*/
	SPR_OBJ		* frame ;	/* 枠*/
	SPR_POS		sp_pos;
} RAD_SPR ;


#define RAD_BUF_MAX (81920 / sizeof( u_long128 ))
typedef struct {
	GV_ACT actor;
	int name;
	int display_flag;
	DG_DMAPACK *dmapack;
	VU1_PARAM param;
	RAD_SPR 	rad_spr ;
	u_short		count ;
	u_char		alert ;
	u_char		noise ;
	float		height ;
//	u_long128 buffer[ 2 ][ 40960 / sizeof( u_long128 ) ];
	u_long128 buffer[ 2 ][ RAD_BUF_MAX ];
	int		status ;
	int		nowmode ;
	u_short	nomcnt	;
	short	base_rot ;	/*表示方向*/
	int		tri_id ;
#ifdef DEBUG_MODE
	int		debug ;
#endif
} Work;



/*でばぐ用*/
#ifdef DEBUG_MODE
enum {
	RAD_DEBUG_ARROW,
	RAD_DEBUG_SIGHT,
	RAD_DEBUG_MAX
};
#endif

extern u_long128 Vu1DrawRadarMPGTag[ 1 ] ;	// データ転送用タグ
extern int Vu1DrawRadarFunc[] ;


/* ---------------------------------------------------------------------- */
/*
	マトリクス
*/


enum {
	SEG_DRAW_START = 0,
	SEG_DRAW_NEXT = 1,
	CHARA_SIGHT_DRAW = 2
};

static FMATRIX default_matrix ;
static void InitDefaultMatrix( Work *work ){
	FVECTOR	zoom_rate ;
	FMATRIX base_matrix = {
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
	} ;
	zoom_rate.vx = RADAR_ZOOM_RATE_W ;
	zoom_rate.vy = 0.0F ;
	zoom_rate.vz = RADAR_ZOOM_RATE_H ;
	zoom_rate.vw = 0.0F ;


	default_matrix = base_matrix ;
	if(work->base_rot != 0)
	{
		float rot ;
		rot = (float)(-work->base_rot) * PI / 2048.0F ;
		_sceVu0RotMatrixZ( &default_matrix,
			&default_matrix, rot );
	}
}
static unsigned char default_colors[ 4 ][ 4 ] = {
	{ 0, 160, 72, 0x80 },
	{ 10, 50, 40, 0x80 },
};
static unsigned char search_colors[ 4 ][ 4 ] = {
	{ 130, 150, 10, 0x80 },
	{ 31, 38, 10, 0x80 },
};

#if 0
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
#else

static unsigned char default_chara_colors[][ 4 ][ 4 ] = {
	{
		{ 0, 0, 0, 0x80 },
		{ 0, 0, 0, 0x80 },
		{ 0, 160, 72, 0x80 },
		{ 0, 0, 0, 0x80 },
	},
	{
		{ 0, 0, 0, 0x80 },
		{ 0, 0, 0, 0x80 },
		{ 0x00, 0x80, 0x80, 0x80 },
		{ 0x00, 0x00, 0x10, 0x80 },
	},
	{
		{ 0, 0, 0, 0x80 },
		{ 0, 0, 0, 0x80 },
		{ 0xa0, 0x00, 0x00, 0x80 },
		{ 0x10, 0x00, 0x00, 0x80 },
	},
	{
		{ 0, 0, 0, 0x80 },
		{ 0, 0, 0, 0x80 },
		{ 0xa0, 0xa0, 0x00, 0x80 },
		{ 0x08, 0x08, 0x00, 0x80 },
	},
};

#endif

#define NOISE_1 (0xce0182)
#define NOISE_2 (0xce0183)
#define NOISE_3 (0xce0184)

static u_int NoiseTexCode[3] ={
	NOISE_1,
	NOISE_2,
	NOISE_3
} ;
/* ---------------------------------------------------------------------- */
/*
	壁描画タグの設定
*/

static int	SegmentExist ;

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
	    if ( seg->atr & HZX_SEG_NO_DISP_RADAR ) {
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
		SegmentExist = 1 ;
	}
	return tag;
}

static	DMA_TAG	*make_hzd_dynamic_segment_tag( DMA_TAG *tag, int prog, HZX_D_SEGMENT *segs ) 
{
	HZX_D_SEGMENT		*seg ;

	seg = segs ;
	while( seg != NULL ) {
		if ( seg->atr & ( HZX_SEG_SKIP | HZX_SEG_NO_DISP_RADAR ) ) goto dynamic_segment_tag_skip ;
	    /* 頂点数等 */
	    tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 3 ) ;
	    tag->addr = DMATAG_SET_ADDR( seg->tag ) ;
	    tag->vif1 = SCE_VIF1_SET_STMOD( 0x00, 0 ) ; /* 加算書き込みＯＦＦ */
	    tag->vif2 = SCE_VIF1_SET_UNPACK( 0x8000, 3, VIF_DATA128, 0 ) ;
	    tag ++ ;
		/* ダブルバッファ切替え */
	    tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
	    tag->vif1 = SCE_VIF1_SET_FLUSHE( 0 ) ;
	    tag->vif2 = SCE_VIF1_SET_MSCAL( Vu1DrawRadarFunc[ prog ], 0 ) ;
	    tag ++ ;
	    prog = SEG_DRAW_NEXT ;
		SegmentExist = 1 ;
dynamic_segment_tag_skip :
		seg = seg->next ;
	}
	return tag ;
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
	bit = HZX_AddGroupID( HZX_CurrentGroupID ) ; /* 園山追加(2001.01.26) */
	prog = SEG_DRAW_START;

    SegmentExist = 0 ;
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
					if ( SegmentExist != 0 ) prog = SEG_DRAW_NEXT ;
				}
#if 0
				if ( block->n_bul_segs > 0 ) {
					tag = make_hzd_one_segment_tag( tag, block, prog, 
												    block->bul_segs, block->n_bul_segs );
					prog = SEG_DRAW_NEXT ;
				}
#endif
		    }
			/* ダイナミックハザード */
			if ( grp->dynamics->n_segs > 0 ) {
				tag = make_hzd_dynamic_segment_tag( tag, prog, grp->dynamics->segs ) ;
				if ( SegmentExist != 0 ) prog = SEG_DRAW_NEXT ;
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

		if( !( p->flag & RADAR_VISIBLE ) ) {
			continue;
		}
		*( FVECTOR * )tag = *( p->pos );
		tag ++;
		angle = p->angle;
#ifdef RAD_RANGE_FIX
		if( p->flag & RADAR_NOFIX_SIGHT){
			length = p->range;
		}else {
			length = RAD_RANGE_FIX;
		}
#else
		length = p->range;		// 敵兵の視界の長さ
#endif
		col = p->col;			// 色
		if( !( p->flag & RADAR_SIGHT ) ) {
			col = -1;
		}
		width = angle / 4;	// セグメント1こあたり。
		if((0<width)&&(width < w)
		){
			width = w ;
		}
		roty = ((p->dir+work->base_rot)&4095) - 2 * width;
//		roty = p->dir - 2 * width;
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
static void copy_color( LCOLOR *dest, unsigned char *src )
{
	int i;
	int *d;
	d = &( dest->color[ 0 ] );
	for( i = 0; i < 4; i ++ ){
		*( d ++ ) = *( src ++ );
	}
}
static void set_default_color( Work *work )
{
	VU1_PARAM *param;
	param = &( work->param );
	copy_color( &param->bright_color, default_colors[ 0 ] );
	copy_color( &param->dark_color, default_colors[ 1 ] );
}
static void set_search_color( Work *work )
{
	VU1_PARAM *param;
	param = &( work->param );
	copy_color( &param->bright_color, search_colors[ 0 ] );
	copy_color( &param->dark_color, search_colors[ 1 ] );
}

/*テクスチャWHにスプライトサイズを合わせる*/
static void SetTexSizeSpr(SPR_OBJ * sprite){
	float w,h;
	w = (float) (RAD_Fix2Int(sprite->sprite.head.tex.w)+1) ;
	h = (float) (RAD_Fix2Int(sprite->sprite.head.tex.h)+1) ;
//printf("W %f H %f \n",w,h);
	SPR_SetSizeSprite(sprite, w, h);
}
#if 0
static void SetTexSizeSpin(SPR_OBJ * sprite){
	float w,h;
	w = (float) (RAD_Fix2Int(sprite->sprite.head.tex.w)+1) ;
	h = (float) (RAD_Fix2Int(sprite->sprite.head.tex.h)+1) ;
	SPR_SetSizeSpin(sprite, w, h);
}
#endif
/*スプライトサイズにテクスチャWHを合わせる*/
static void SetNormWHSpr(SPR_OBJ * sprite){
	SPR_FIX dw, dh;      /* 表示上のサイズ     */
//	float	w;
	dw = SPR_FIXED(sprite->sprite.dw) ;
	dh = SPR_FIXED(sprite->sprite.dh) ;
	sprite->sprite.head.tex.w = dw ;
	sprite->sprite.head.tex.h = dh ;

//	w = (float) (RAD_Fix2Int(sprite->sprite.head.tex.w)+1) ;
//	h = (float) (RAD_Fix2Int(sprite->sprite.head.tex.h)+1) ;
//	SPR_SetSizeSprite(sprite, w, h);
}

static void SetRadioCol( SPR_OBJ *radio ,int r,int g,int b,int a){
	int i;
	for(i=0;i<5;i++){
		SPR_SetColorLineStrip(radio, i,r,g,b,a);
	}
}

/********
言葉の定義
レーダー 地形やキャラを表示するもの
アラート表示
********/
/*レーダーの使用禁止チェック*/
/*ゲーム状態による使用不可*/
#define RAD_STATE_NOUSE (STATE_CHAFF)
/*プレイヤ状態による使用不可*/
//#define RAD_PLAYER_NOUSE (PLAYER_INTRUDE|PLAYER_LOCKER|PLAYER_MENU_OPEN)
#define RAD_PLAYER_NOUSE (PLAYER_MENU_OPEN)
static int CheckRadarUseIntrude(void){
	if(
	(GM_Configuration & GM_CONFIG_RADAR_OFF_INTRUDE)
	&&(GM_PlayerStatus & (PLAYER_INTRUDE|PLAYER_LOCKER))
	)
	{
		return 1 ;
	}
	return 0 ;
}

static int CheckRadarUse(void)
{
	int item ;
	item = PL_GetPlayerItem() ;
	if(
	(GM_AlertMode == ALERT_MODE_ALERT )
	||(GM_AlertMode == ALERT_MODE_AVOID )
	||(GM_CheckGameStatus(RAD_STATE_NOUSE))
	||(GM_PlayerStatus & RAD_PLAYER_NOUSE)
	||(GM_Configuration & GM_CONFIG_RADAR_OFF)
	||(item == IT_Scope)
	||(GM_CheckMenuStatus ( MENU_SUBWIN_ON))
	||(CheckRadarUseIntrude())
	){
		return 1 ;
	}
	return 0;
}


static void SetBoxPos( SPR_OBJ *box ,SPR_POS *center ,float size){
	SPR_RECT rect ;
	rect.begin.x = center->x - size ;
	rect.begin.y = center->y - size*SC_RATE_H ;
	rect.end.x = center->x + size ;
	rect.end.y = center->y + size*SC_RATE_H ;
	
	SPR_SetPosBox( box, &rect) ; 
}

static void SetFramePos( Work *work ){
	SPR_RECT rect ;

	rect.begin.x = 0.0F;
	rect.begin.y = 0.0F;


	if(
	(CheckRadarUse())&&
	((work->rad_spr.mode->head.flags & SPR_FLAG_HIDDEN)
	||(work->rad_spr.empty->head.flags & SPR_FLAG_HIDDEN))
	){
		SPR_HIDE(work->rad_spr.frame);
		return ;
	}


	if(
	(work->rad_spr.mode->head.flags & SPR_FLAG_HIDDEN)
	){
		rect.begin.y += ((float)TOP_SIZE_H ) ; 
	}
//	rect.end.x = rect.begin.x + RADAR_WINDOW_W + 1.0F ;
	rect.end.x = rect.begin.x + RADAR_WINDOW_W ;
#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL 
	rect.end.y = rect.begin.y +SC_RATE_H*2;
#else
	rect.end.y = rect.begin.y +SC_RATE_H;
#endif
	if(
	(CheckRadarUseIntrude())
	||(PL_GetPlayerItem() == IT_Scope)
	){
		/*NOP*/
	}else {
		rect.end.y += work->height ;
	}
	if(
	(!(work->rad_spr.use->head.flags & SPR_FLAG_HIDDEN))
	&&(!(work->rad_spr.mode->head.flags & SPR_FLAG_HIDDEN))
	){
		rect.end.y += (BOTOM_SIZE_H-SC_RATE_H); 
	}
	if(
	(!(work->rad_spr.mode->head.flags & SPR_FLAG_HIDDEN))
	){
		rect.end.y += (float)TOP_SIZE_H;
	}
	SPR_SHOW(work->rad_spr.frame);

	rect.begin.x -= 1.0F;

	SPR_SetPosBox( work->rad_spr.frame, &rect) ; 


/***
#define RADAR_WINDOW_X0		(DRAW_WIDTH - RADAR_OFFSET_X - RADAR_WINDOW_W )
#define RADAR_WINDOW_X1		(DRAW_WIDTH - RADAR_OFFSET_X)
#define RADAR_WINDOW_Y0		(RADAR_OFFSET_Y+TOP_SIZE_H_SC)
#define RADAR_WINDOW_Y1		(RADAR_WINDOW_Y0 + RADAR_WINDOW_H)
****/


}

static void SetRadioMark( SPR_OBJ *radio ,float x ,float y ,float size){
	SPR_POS radio_pos[5] ;
	float x0,x1,y0,y1 ;
	x0 = x-size ; x1 = x+size ;
	y0 = y-size ; y1 = y+size ;

	if(x0 < RADAR_WINDOW_X0 ) x0 = RADAR_WINDOW_X0 ;
	if(x1 > RADAR_WINDOW_X1 ) x1 = RADAR_WINDOW_X1 ;
	if(y0 < (RADAR_WINDOW_Y0 * SC_RATE_H)) y0 = (RADAR_WINDOW_Y0 * SC_RATE_H) ;
	if(y1 > (RADAR_WINDOW_Y1 * SC_RATE_H)) y1 = (RADAR_WINDOW_Y1 * SC_RATE_H) ;

	radio_pos[0].x = radio_pos[3].x = x0 ;
	radio_pos[1].x = radio_pos[2].x = x1 ;

	radio_pos[0].y = radio_pos[1].y = y0 ;
	radio_pos[2].y = radio_pos[3].y = y1 ;

	radio_pos[4] = radio_pos[0] ;


	SPR_SetPosLineStrip(radio, 0, 5, radio_pos);

};

static int RadParamInit(Work *work){
	work->count = 0 ;
	work->nomcnt = 0 ;
	work->alert = GM_StartAlertMode ;
	if(GM_Configuration & GM_CONFIG_RADAR_OFF){
		work->height = 0.0F ;
	}else {
		work->height = (float)(RADAR_WINDOW_H-1) * SC_RATE_H ;
	}
	work->noise = 0 ;
	return 0 ;
}

static void set_base_color_sneak( Work *work )
{
	PACK_HEADER *packet ;
	RADAR_TILE_SET *tp;
	int i;
	for( i = 0; i < 2; i ++ ){
		/* パケット領域の確保 */
		packet = ( PACK_HEADER * )work->buffer[ i ];
		tp = &( packet->base_tile );
		tp->rgba = SCE_GS_SET_RGBAQ( 0, 28, 28, BASE_ALPHA, 0 );
	}
}

static void set_base_color_search( Work *work )
{
	PACK_HEADER *packet ;
	RADAR_TILE_SET *tp ;
	int i;
	for( i = 0; i < 2; i ++ ){
		/* パケット領域の確保 */
		packet = ( PACK_HEADER * )work->buffer[ i ];
		tp = &( packet->base_tile );
		tp->rgba = SCE_GS_SET_RGBAQ( 0, 50, 45, BASE_ALPHA, 0 );
	}
}

static void ModeChange(Work *work ,int mode )
{
//	int	code ;
//	SPR_LoadTexture(RADAR_TRI);
	switch ( mode ) {
		case ALERT_MODE_SNEAK :
			/*ハザード描画色設定*/
			set_default_color(work);
			/*背景色設定*/
			set_base_color_sneak( work ) ;
			if(work->alert == ALERT_MODE_SEARCH){
				GM_SdSet( SD_S_RADAR001 ) ;
				work->nomcnt = 60 ;
			}
			SPR_SetSizeSprite(work->rad_spr.level, 120.0F,BOTOM_SIZE_H_LEVEL);
			SPR_ObjSetTexture(work->rad_spr.use, 
				NOM_B1, work->tri_id);
			SET_COLOR_SNK(work->rad_spr.use);
			SET_COLOR_SNK(work->rad_spr.dot);
			SET_COLOR_SNK_ALP(work->rad_spr.level,0x80);
//			SET_COLOR_2DPRIM2(work->rad_spr.level,
//				SNK_R>>1, SNK_G>>1,SNK_R>>1,0x80);
			SPR_HIDE(work->rad_spr.use);
			SPR_SHOW(work->rad_spr.empty);
//			SPR_SHOW(work->rad_spr.mode);
			SPR_SHOW(work->rad_spr.noise);
			break ;
		case ALERT_MODE_ALERT :
			SPR_SetSizeSprite(work->rad_spr.level, 120.0F,BOTOM_SIZE_H_LEVEL);
			SET_COLOR_ALT(work->rad_spr.use);
			SET_COLOR_ALT(work->rad_spr.dot);
			SET_COLOR_ALT_ALP(work->rad_spr.level,0x80);
//			SET_COLOR_2DPRIM2(work->rad_spr.level,
//				ALT_R>>1, ALT_G>>1,ALT_R>>1,0x80);
			SPR_ObjSetTexture(work->rad_spr.mode,ALT_T1, work->tri_id);
			SPR_ObjSetTexture(work->rad_spr.use, 
				ALT_B1, work->tri_id);
			SPR_SHOW(work->rad_spr.use);
			SPR_SHOW(work->rad_spr.empty);
			SPR_SHOW(work->rad_spr.mode);
			SPR_SHOW(work->rad_spr.noise);
			break ;
		case ALERT_MODE_AVOID :
			SPR_SetSizeSprite(work->rad_spr.level, 120.0F,BOTOM_SIZE_H_LEVEL);
			SET_COLOR_ESC(work->rad_spr.use);
			SET_COLOR_ESC(work->rad_spr.dot);
			SET_COLOR_ESC_ALP(work->rad_spr.level,0x60);
			SPR_ObjSetTexture(work->rad_spr.mode,ESC_T1, work->tri_id);
			SPR_ObjSetTexture(work->rad_spr.use, 
				ESC_B1, work->tri_id);
			SPR_SHOW(work->rad_spr.use);
			SPR_SHOW(work->rad_spr.empty);
			SPR_SHOW(work->rad_spr.mode);
			SPR_SHOW(work->rad_spr.noise);
			break ;
		case ALERT_MODE_SEARCH :
			SPR_SetSizeSprite(work->rad_spr.level, 120.0F,BOTOM_SIZE_H_LEVEL);
			set_search_color(work);
			set_base_color_search( work ) ;
			SPR_ObjSetTexture(work->rad_spr.mode,SRC_T1, work->tri_id);
			SPR_ObjSetTexture(work->rad_spr.use, 
				NOM_B1, work->tri_id);
			SET_COLOR_SRC(work->rad_spr.use);
			SET_COLOR_SRC(work->rad_spr.dot);
			SET_COLOR_SRC_ALP(work->rad_spr.level,0x80);
//			SET_COLOR_2DPRIM2(work->rad_spr.level,
//				SRC_R>>1, SRC_G>>1,SRC_R>>1,0x80);
			SPR_SHOW(work->rad_spr.use);
			SPR_SHOW(work->rad_spr.empty);
			SPR_SHOW(work->rad_spr.mode);
			SPR_SHOW(work->rad_spr.noise);
			break ;
		case RAD_STATE_CHAFF :
			SPR_SetSizeSprite(work->rad_spr.level, 120.0F,BOTOM_SIZE_H_LEVEL);
			SPR_ObjSetTexture(work->rad_spr.mode,JAM_T1, work->tri_id);
			SPR_ObjSetTexture(work->rad_spr.use, 
				ESC_B1, work->tri_id);
			SET_COLOR_JAM(work->rad_spr.use);
			SET_COLOR_JAM(work->rad_spr.dot);
			SET_COLOR_JAM_ALP(work->rad_spr.level,0x80);
//			SET_COLOR_2DPRIM2(work->rad_spr.level,
//				JAM_R>>1, JAM_G>>1,JAM_R>>1,0x80);
			SPR_HIDE(work->rad_spr.use);
			SPR_SHOW(work->rad_spr.empty);
			SPR_SHOW(work->rad_spr.mode);
			SPR_SHOW(work->rad_spr.noise);
			break ;
	}
}

/***
DRAW_Z_MIN	(4096) Ｚ値最小 
DRAW_Z_MAX	(16777216-4096)	Ｚ値最大 
pri
*/

#define UWAGAKI (SCE_GS_SET_ALPHA(2,2,0,0,0))
#define KASAN (SCE_GS_SET_ALPHA(0,2,0,1,0))
#define KASAN2 (SCE_GS_SET_ALPHA(0,2,2,1,64))
#define ALPHA_HALF (SCE_GS_SET_ALPHA(0,1,0,1,0x80))

/*ＳＰＲ系の*/
static int RadSprInit(Work *work ){
//	int i;
	SPR_POS		b_pos;
	RAD_SPR *rad_spr ;


	rad_spr = &work->rad_spr ;
	/* 使用するTRIのロード*/
//	SPR_LoadTexture(RADAR_TRI);

	/* 操作座標となる Empty オブジェクトを作成 */
	rad_spr->empty 
		= SPR_Create_2D_Object(SP_EMPTY, DG_CHANL_MENU, NULL) ;
	if(rad_spr->empty == NULL ) return -1 ;

	/*以下、Emptyオブジェクトを最上位の親として、
	その下に各種オブジェクトを作成する。*/
	rad_spr->mode = 
		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, rad_spr->empty);
	rad_spr->dot = 
		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, rad_spr->mode);
	rad_spr->use = 
		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, rad_spr->mode);
	rad_spr->noise = 
		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, rad_spr->empty);
	rad_spr->radio = 
		SPR_Create_2D_Object( SP_LINESTRIP, DG_CHANL_MENU, NULL) ;
	rad_spr->level = 
//		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, rad_spr->mode);
		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, rad_spr->use);
	rad_spr->frame = 
		SPR_Create_2D_Object(SP_BOX,DG_CHANL_MENU,rad_spr->empty);

	SPR_SetLineStripVertexNumber(rad_spr->radio, 5) ; 
	/*無線中用*/
	SetRadioCol( rad_spr->radio ,255,255,127,127) ;
	SPR_SHOW(rad_spr->radio);
	SPR_SetPriority(rad_spr->radio , 7 ) ;
	rad_spr->radio->head.alpha = KASAN2 ;
//	rad_spr->radio->head.flags |= SPR_FLAG_ALPHA;


	/* テクスチャを指定する */
	SPR_ObjSetTexture(rad_spr->mode, ALT_T1, work->tri_id);
	SPR_ObjSetTexture(rad_spr->use, ALT_B1, work->tri_id);
	SPR_ObjSetTexture(rad_spr->noise, NoiseTexCode[0], work->tri_id);

	/*ブレンディングの各値を設定*/
#if 1
//	rad_spr->mode->head.alpha = UWAGAKI ;
	rad_spr->mode->head.alpha =	ALPHA_HALF ;
	rad_spr->mode->head.flags |= SPR_FLAG_ALPHA;

	rad_spr->use->head.alpha =	ALPHA_HALF ;
	rad_spr->use->head.flags |= SPR_FLAG_ALPHA;

	rad_spr->noise->head.alpha = ALPHA_HALF ;
	rad_spr->noise->head.flags |= SPR_FLAG_ALPHA;

	rad_spr->frame->head.alpha = UWAGAKI ;
	SPR_SetColorBox(rad_spr->frame,0x00, 0x00, 0x00,0x70);

#else
	rad_spr->mode->head.alpha = ALPHA_HALF ;
	rad_spr->use->head.alpha = ALPHA_HALF ;
	rad_spr->noise->head.alpha = UWAGAKI ;
	rad_spr->mode->head.flags |= SPR_FLAG_ALPHA;
	rad_spr->use->head.flags |= SPR_FLAG_ALPHA;
#endif


	/*発光用は加算半透明*/
//	rad_spr->use_add->head.alpha = KASAN ;
//	rad_spr->use_add->head.flags |= SPR_FLAG_ALPHA;

	rad_spr->level->head.alpha = KASAN ;
	rad_spr->level->head.flags |= SPR_FLAG_ALPHA;


//SET_COLOR_2DPRIM2(rad_spr->mode,0x80,0x80,0x80,0x80) ;

	SET_COLOR_DEF(rad_spr->use);
	SET_COLOR_DEF(rad_spr->mode);
	SET_COLOR_DEF(rad_spr->noise)

	/*作成したオブジェクトの座標値などを設定する。*/
	/*基準座標*/
	rad_spr->sp_pos.x = RADAR_WINDOW_X0 ;
	rad_spr->sp_pos.y = RADAR_OFFSET_Y * SC_RATE_H ;

	SPR_SetPosEmpty(rad_spr->empty, &rad_spr->sp_pos);
	/* スプライト表示位置の指定 */
	SPR_SetPosSprite(rad_spr->mode,&(SPR_POS){0.0F, 0.0F});
	SPR_SetPosSprite(rad_spr->dot,&(SPR_POS){8.0F, 8.0F});
	SPR_SetPosSprite(rad_spr->noise,&(SPR_POS){0.0F,(float)TOP_SIZE_H});
	SetTexSizeSpr(rad_spr->mode) ;
	SPR_SetSizeSprite(rad_spr->dot, 6.0F, 5.0F);
//	SPR_SetSizeSprite(rad_spr->mode, 120.0F, 14.0F);
	SetTexSizeSpr(rad_spr->noise) ;
	/*スプライトサイズにテクスチャをあわせる*/
	/*実験用 表示を半分にする*/
//	SPR_SetSizeSprite(rad_spr->mode, 60.0F, 9.0F);
//	SetNormWHSpr(rad_spr->mode) ;
//	SetNormWHSpr(rad_spr->noise) ;

	b_pos.x = 0.0F;
	b_pos.y = work->height;
	SPR_SetPosSprite(rad_spr->use,&b_pos);
//	SPR_SetPosSprite(rad_spr->use_add,&b_pos);
	SetTexSizeSpr(rad_spr->use) ;

#if 0
	SetTexSizeSpr(rad_spr->use_add) ;
#else
	/*直接サイズ設定*/
//	SPR_SetSizeSprite(rad_spr->use_add, 120.0F, 4.0F);
//	SPR_SetSizeSprite(rad_spr->use_add, 120.0F, 8.0F);
#endif
	SPR_SetSizeSprite(rad_spr->level, 120.0F,(float)TOP_SIZE_H);

	/*描画優先設定*/

	SPR_SetPriority(rad_spr->use,6);
	SPR_SetPriority(rad_spr->noise,5);
	SPR_SetPriority(rad_spr->mode,6);
	SPR_SetPriority(rad_spr->dot,7);
	SPR_SetPriority(rad_spr->level,7);
	SPR_SetPriority(rad_spr->frame,7);

	/* 	初期状態 */
	SPR_SHOW(rad_spr->empty);
	SPR_HIDE(rad_spr->mode);
	SPR_SHOW(rad_spr->dot);
	SPR_SHOW(rad_spr->use);
	SPR_SHOW(rad_spr->level);
	SPR_SHOW(rad_spr->frame);

	SPR_SHOW(rad_spr->noise);
	set_default_color(work);
	ModeChange(work,GM_StartAlertMode) ;
	return 0;
}

#if 0
/*スクリーン座標をSPR_POS座標に変換*/
static void (int x,int y,SPR_POS *spr_pos)
	spr_pos->x = x/DRAW_
pix_x = SPR_PIX_X(DG_CHANL_MENU, 100);

printf("PIX_X %f\n",pix_x);
pix_x = SPR_PIX_Y(DG_CHANL_MENU, 100);
printf("PIX_Y %f\n",pix_x);

printf("DRAW_HEIGHT %d\n",DRAW_HEIGHT);
printf("DRAW_WIDTH %d\n",DRAW_WIDTH);

//	DG_Chanls[(chanl)].height)

}
#endif


/*レーダ範囲か？０非表示 １表示*/
static int CheckRadarRange(RADAR_CTRL *r_ctrl){
//	RADAR_VIEW_W
	FVECTOR *center,*trg ;
	return 1 ;

	center = GM_RadarGetCenter() ;
	trg = r_ctrl->pos ;
	if(
	( abs( trg->vx - center->vx ) >= (float) RADAR_VIEW_W )
	||( abs( trg->vz - center->vz ) >= (float) RADAR_VIEW_W )
	){
		return 0 ;
	}
	return 1 ;
}

static void SetHeadColor(Work *work,RADAR_CTRL *r_ctrl,FVECTOR *cent){
	LCOLOR *col;
	float dif_y;
	unsigned char pl_color_bright[4] = { 200, 200, 200, 0x80 } ;
	unsigned char pl_color_dark[4] = { 100, 100, 100, 0x80 } ;
	unsigned char ene_color_bright[4] = { 255, 30, 0, 0x80 } ;
	unsigned char ene_color_dark[4] = { 180,64, 0, 0x80 } ;
	unsigned char *c_col ;
#ifdef DEBUG_MODE
	ASSERT(r_ctrl->col < MAX_CHARA_COLOR) ;
#endif
	dif_y = r_ctrl->pos->vy - cent->vy ;
	dif_y =  -dif_y ;
#if 0
	if(
	(r_ctrl->col == RADAR_COLOR_PLAYER)
	||((dif_y <= UPPER_RANGE)&&(DOWN_RANGE <= dif_y ))
	){
		col = &work->param.chara_color[r_ctrl->col].pos_bright ;
	}else {
		col = &work->param.chara_color[r_ctrl->col].pos_dark ;
	}
#else

	if(r_ctrl->col == RADAR_COLOR_PLAYER){
#if 1
		c_col = pl_color_bright ;
#else
		if((dif_y <= UPPER_RANGE)&&(DOWN_RANGE <= dif_y )){
			c_col = pl_color_bright ;
		}else {
			c_col = pl_color_dark ;
		}
#endif
	}else if ((dif_y <= UPPER_RANGE)&&(DOWN_RANGE <= dif_y )){
		c_col = ene_color_bright ;
	}else {
		c_col = ene_color_dark ;
	}
	SPR_SetColorBox(r_ctrl->face,c_col[0],
		c_col[1],c_col[2],c_col[3]) ;
#endif

}
static void RctrlAllHide( Work *work ){
	RADAR_CTRL *r_ctrl;
	int i;
	RAD_SPR *rad_spr ;

	rad_spr = &work->rad_spr ;
	SPR_HIDE(rad_spr->radio) ;
#if 1
	for( i=0,r_ctrl = GM_RadarControlGetTop(); 
		((r_ctrl != NULL)&&(i < R_CTRL_MAX));
		r_ctrl = r_ctrl->next,i++ ){
		SPR_HIDE(r_ctrl->face);
	}
#endif
}

/*ゲーム座標をレーダー上座標へ*/
/*RADAR_CTRLキャラ表示*/
static void SetR_CtrlsPos( Work *work )
{
	RADAR_CTRL *r_ctrl;
	int i, radicnt,radio_limit;
	SPR_POS	r_pos;
	FVECTOR center ,tmppos ,zoom_rate ;
//	float	pix_x ;
//	short	dir ;
	RAD_SPR *rad_spr ;
	rad_spr = &work->rad_spr ;
	SPR_HIDE(rad_spr->radio);
	if(work->base_rot != 0){
		RotVecXZ(GM_RadarGetCenter(),&center,(int)work->base_rot) ;
	}else {
		center = *GM_RadarGetCenter() ;
	}
	zoom_rate.vx = RADAR_ZOOM_RATE_W ;
	zoom_rate.vy = 0.0F ;
	zoom_rate.vz = RADAR_ZOOM_RATE_H ;
	zoom_rate.vw = 0.0F ;

	if(work->base_rot != 0){
		RotVecXZ(&zoom_rate,&zoom_rate,(int)work->base_rot&4095) ;
	}
//SC_RATE_H
	/*点滅表示*/
	for( i=0,r_ctrl = GM_RadarControlGetTop(); 
		((r_ctrl != NULL)&&(i < R_CTRL_MAX));
		r_ctrl = r_ctrl->next,i++ ){
		SPR_HIDE(r_ctrl->face);
		if( GV_Time % 16 < 3 ) continue ;
		if(GM_CheckMenuStatus(MENU_RADAR_OFF)) continue ;
		if(
		(GM_AlertMode == ALERT_MODE_ALERT)
			||(GM_AlertMode == ALERT_MODE_AVOID)
		){
			continue ;
		}
		/*不可視ならNOP*/
		if( !( r_ctrl->flag & RADAR_VISIBLE ) ) continue;
		if(CheckRadarRange(r_ctrl)){
			RotVecXZ(r_ctrl->pos,&tmppos,(int)work->base_rot) ;
			r_pos.x = (float)(tmppos.vx - center.vx) 
//				* (zoom_rate.vx)+(float)(RADAR_WINDOW_CX);
				* _ABSf(zoom_rate.vx)+(float)(RADAR_WINDOW_CX);
			r_pos.y = (float)(tmppos.vz - center.vz) 
//				* (zoom_rate.vz)+(float)(RADAR_WINDOW_CY);
				* _ABSf(zoom_rate.vz)+(float)(RADAR_WINDOW_CY);
			if(
			( (float) RADAR_WINDOW_X0 >= r_pos.x )
			||( (float) RADAR_WINDOW_X1 <= r_pos.x )
			||( (float) RADAR_WINDOW_Y0 >= r_pos.y )
			||( (float) RADAR_WINDOW_Y1 <= r_pos.y )
			){
				/*レーダー範囲外*/
				continue;
			}else {
				/*レーダーに入っている*/
				radio_limit = 1 ;
				/*位置表示*/
				SPR_SHOW(r_ctrl->face);
				r_pos.y *= SC_RATE_H ;
				if(r_ctrl->col == RADAR_COLOR_PLAYER){
				/*輝度で大きく見えるので小さ目に表示*/
					SetBoxPos(r_ctrl->face,&r_pos,0.5F) ;
				}else {
					SetBoxPos(r_ctrl->face,&r_pos,1.0F) ;
				}
				SetHeadColor(work,r_ctrl,&center);
			}
			/*無線*/
			if((radio_limit)&&( r_ctrl->flag & RADAR_RADIO )){
//			if((radio_limit)){
//				radicnt = (work->count & 0x3f) ;
				radicnt = (work->count & 31) ;
				radicnt /= 7 ;
				if((radicnt>1)&&(radicnt<6))
				{
					SPR_SHOW(rad_spr->radio);
					if(DG_Clock){
						SetRadioCol( rad_spr->radio ,255,255,127,127) ;
					}else {
						SetRadioCol( rad_spr->radio ,127,127,127,127) ;
					}
					SetRadioMark( rad_spr->radio ,r_pos.x ,r_pos.y ,
					((float)(radicnt)*1.5F+0.0F ) );
				}
			} 
		}
	}
}

#define TOP_CYCLE1 127
#define TOP_CYCLE2 15

#define BOTTOM_CYCLE1 63
#define BOTTOM_CYCLE2 15


static void RadTexAnim(Work *work){
	int col ;
//	int code;
//	SPR_LoadTexture(RADAR_TRI);
//	SPR_LoadTexture(RADAR_TRI);

	if((work->count&TOP_CYCLE1 ) >(TOP_CYCLE1/2) ){
		SPR_SHOW(work->rad_spr.dot) ;
	}else {
		if((work->count&TOP_CYCLE2) < (TOP_CYCLE2/2) ){
			SPR_HIDE(work->rad_spr.dot) ;
		}else {
			SPR_SHOW(work->rad_spr.dot) ;
		}
	}
	switch ( work->nowmode ) {
		case ALERT_MODE_ALERT :
			col = (BOTTOM_CYCLE1-(work->count&BOTTOM_CYCLE1))*4 ;
			SET_COLOR_2DPRIM2(work->rad_spr.use,
				(int)((col*ALT_R)/255),(int)((col*ALT_G)/255),
				(int)((col*ALT_B)/255),0x80);
			SPR_SHOW(work->rad_spr.empty);
			SPR_SHOW(work->rad_spr.level);
			SPR_SetSizeSprite(work->rad_spr.level, ((float)GM_AlertLevel/(float)ALERT_LEVEL_MAX)*120.0F,BOTOM_SIZE_H_LEVEL);
			if(GM_CheckMenuStatus ( MENU_SUBWIN_ON)){
				SPR_HIDE(work->rad_spr.use);
			}else {
				SPR_SHOW(work->rad_spr.use);
			}
			break ;
		case ALERT_MODE_AVOID :
			col = (BOTTOM_CYCLE1-(work->count&BOTTOM_CYCLE1))*4 ;
			SET_COLOR_2DPRIM2(work->rad_spr.use,
				(int)((col*ESC_R)/255),(int)((col*ESC_G)/255),
				(int)((col*ESC_B)/255),0x80);
//			SPR_SetSizeSprite(work->rad_spr.level, 120.0F,(float)TOP_SIZE_H);
			SPR_SHOW(work->rad_spr.empty);
			SPR_SHOW(work->rad_spr.level);
			if(GM_CheckMenuStatus ( MENU_SUBWIN_ON)){
				SPR_HIDE(work->rad_spr.use);
			}else {
				SPR_SHOW(work->rad_spr.use);
			}
			break ;
		case RAD_STATE_CHAFF :
			col = (BOTTOM_CYCLE1-(work->count&BOTTOM_CYCLE1))*4 ;
			SET_COLOR_2DPRIM2(work->rad_spr.use,
				(int)((col*JAM_R)/255),(int)((col*JAM_G)/255),
				(int)((col*JAM_B)/255),0x80);
			SPR_SetSizeSprite(work->rad_spr.level, 120.0F*((float)GM_JammingLevel/(float)JAMMING_LEVEL_MAX),
				(float)BOTOM_SIZE_H_LEVEL);
			SPR_SHOW(work->rad_spr.empty);
			SPR_SHOW(work->rad_spr.level);
			if(GM_CheckMenuStatus ( MENU_SUBWIN_ON)){
				SPR_HIDE(work->rad_spr.use);
			}else {
				SPR_SHOW(work->rad_spr.use);
			}
			break ;
		case ALERT_MODE_SEARCH :
			col = (BOTTOM_CYCLE1-(work->count&BOTTOM_CYCLE1))*4 ;
			SET_COLOR_2DPRIM2(work->rad_spr.use,
				(int)((col*SRC_R)/255),(int)((col*SRC_G)/255),
				(int)((col*SRC_B)/255),0x80);
#ifdef PAL
			SPR_SetSizeSprite(work->rad_spr.level, ((float)GM_CautionLevel/3000.0F)*120.0F,BOTOM_SIZE_H_LEVEL);
#else
			SPR_SetSizeSprite(work->rad_spr.level, ((float)GM_CautionLevel/3600.0F)*120.0F,BOTOM_SIZE_H_LEVEL);
#endif
			SPR_SHOW(work->rad_spr.empty);
			SPR_SHOW(work->rad_spr.level);
			SPR_SHOW(work->rad_spr.mode);
			if(GM_CheckMenuStatus ( MENU_SUBWIN_ON)){
				SPR_HIDE(work->rad_spr.use);
			}else {
				SPR_SHOW(work->rad_spr.use);
			}
				break ;
//				SPR_SetSizeSprite(work->rad_spr.level, 120.0F,(float)TOP_SIZE_H);

		case ALERT_MODE_SNEAK :
#if 0
			if(work->nomcnt){
				SPR_SHOW(work->rad_spr.mode);
			}else {
				SPR_HIDE(work->rad_spr.mode);
			}
#else
			SPR_HIDE(work->rad_spr.mode);
			SPR_HIDE(work->rad_spr.use);
#endif
			break;

	}
}
/*帰り値 ノイズ中１*/
static int ModeDisplay(Work *work){
	RAD_SPR *rad_spr ;
	SPR_POS	b_pos;
	int noise=0;

	rad_spr = &work->rad_spr ;
	SPR_HIDE(rad_spr->use);
	if(work->alert == work->nowmode ){
		RadTexAnim(work);
	}else {
		/*モードチェンジ*/
		ModeChange(work,work->nowmode);
		work->alert = work->nowmode ;
		noise = 0 ;
	}
	if(
	(GM_Configuration & GM_CONFIG_RADAR_OFF)
	||(CheckRadarUseIntrude())
	||(PL_GetPlayerItem() == IT_Scope)
	||(GM_PlayerStatus & RAD_PLAYER_NOUSE)
	||(GM_CheckMenuStatus ( MENU_SUBWIN_ON))
	){
		if(
		(GM_AlertMode == ALERT_MODE_ALERT)
		||(GM_AlertMode == ALERT_MODE_AVOID)
		||(GM_CheckGameStatus(STATE_CHAFF))
		){
			/*瞬間閉じ*/
			work->height = 0 ;
/*2000.10.28 TEST*/
//			SPR_SHOW(rad_spr->use);
		}else {
			/*瞬間開き*/
			work->height =  (float) ((RADAR_WINDOW_H-1) * SC_RATE_H)  ;
		}
		if((GM_Configuration & GM_CONFIG_RADAR_OFF)
//		||(CheckRadarUseIntrude())
		){
			work->height = 0.0F ;
		}
		noise = 0 ;
	}else{
	/**/
		if(
		(GM_AlertMode == ALERT_MODE_ALERT)
		||(GM_AlertMode == ALERT_MODE_AVOID)
		||(GM_CheckGameStatus(STATE_CHAFF))
		){
			/*閉じ*/
//			SPR_SHOW(rad_spr->use);
#if 1
			if(work->height > 0) {
				work->height -= CLOSE_SPEED ;
				noise = 1 ;
			}
			if(work->height < 0){
				work->height = 0 ;
			}
#else
			work->height =  (float) ((RADAR_WINDOW_H-1) * SC_RATE_H)  ;
			noise = 1 ;
#endif
		}else {
			/*開き*/
			if(work->height < (float) (RADAR_WINDOW_H-1) * SC_RATE_H) {
				work->height += CLOSE_SPEED ;
				noise = 1 ;
			}
			if(work->height > (float) (RADAR_WINDOW_H-1) * SC_RATE_H){
				work->height =  (float) ((RADAR_WINDOW_H-1) * SC_RATE_H)  ;
				printf("NOiSE OPEN SE\n");
				GM_SdSet( SD_S_RADAR001 ) ;
			}
		}
		if(noise){
			if(work->noise >= 3) work->noise = 0;
			SPR_ObjSetTexture(work->rad_spr.noise, 
				NoiseTexCode[work->noise], work->tri_id);
			work->noise++;
		}else {
		}
		SPR_SetSizeSprite(rad_spr->noise, 120.0F,work->height);
		SetNormWHSpr(rad_spr->noise) ;
	}
	b_pos.x = 0.0F;
/*仕様変更*/
	if(
	(GM_Configuration & GM_CONFIG_RADAR_OFF)
	||(CheckRadarUseIntrude())
	||(PL_GetPlayerItem() == IT_Scope)
	){
		b_pos.y = TOP_SIZE_H;
	}else {
		b_pos.y = work->height+TOP_SIZE_H;
	}
	SPR_SetPosSprite(rad_spr->use,&b_pos);
	if(noise){
//		SPR_SHOW(rad_spr->use);
	}
//	if(GM_CheckMenuStatus ( MENU_SUBWIN_ON)){
//		SPR_HIDE(rad_spr->use);
//	}
	return noise;
}
#if 0
static void RadDebugMode( Work *work )
{
	if ( GM_Debug2PMode == GM_DEBUG_MODE_RADAR ) {
		DEBUG_Locate( 40 + ( 22 ), 15+50, 0 );
		DEBUG_Printf( "RADAR TEST MODE");
		if( GV_PadData[ 1 ].press & PAD_L1 ){
			work->debug ++;
			if ( work->debug >= RAD_DEBUG_MAX) {
				work->debug = 0 ;
			}
		}
	}

}
#endif
/*危険レベル等の表示禁止*/
//#define MENU_MODE_OFF	(MENU_RADAR_OFF|MENU_RADIO_ON|MENU_SUBWIN_ON|MENU_WEAPON_OPEN|MENU_ITEM_OPEN|MENU_STREAM_CH_0|MENU_STREAM_CH_1)
//#define MENU_MODE_OFF	(MENU_RADAR_OFF|MENU_RADIO_ON|MENU_SUBWIN_ON|MENU_WEAPON_OPEN|MENU_ITEM_OPEN)
#define MENU_MODE_OFF	(MENU_RADAR_OFF|MENU_RADIO_ON|MENU_WEAPON_OPEN|MENU_ITEM_OPEN)
//|MENU_SUBWIN_ON
static int CheckModeUse(Work *work)
{
	if(
	(GM_CheckMenuStatus ( MENU_MODE_OFF))
	||(GV_PauseLevel & GV_LEVEL_NORMAL)
	||(work->status & RAD_SCN_INVISIBLE) 
	){
		/*非表示フラグをクリア*/
		GM_ResetMenuStatus(MENU_RADAR_OFF) ;
		return 1 ;
	}
	return 0;
}
enum{
	RAD_MSG_SCN_INVIS ,/*off*/
	RAD_MSG_SCN_VIS		/*on*/
};
static void CheckMesg(Work *work){
	GV_MSG *msg;
	int n;
	if( ( n = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
		for( ; n > 0; n-- ){
			switch( msg->message[ 0 ] ) {
				case RAD_MSG_SCN_INVIS :
					work->status |= RAD_SCN_INVISIBLE ;
					break ;
				case RAD_MSG_SCN_VIS :
					work->status &= ~RAD_SCN_INVISIBLE ;
					break ;
			}
			msg++;
		}
	}
}
static int GetNowRadarMode(void){
	if((GM_AlertMode == ALERT_MODE_ALERT )
	||(GM_AlertMode == ALERT_MODE_AVOID )){
		/*危険 回避*/
		return GM_AlertMode ;
	}
	/*チャフ*/
	if(GM_CheckGameStatus(STATE_CHAFF)) return RAD_STATE_CHAFF ;
	/*潜入 探索*/
	return GM_AlertMode ;

}

static void Act( Work *work )
{
	PACK_HEADER *ph;
	DMA_TAG *tag;
	FVECTOR *pos ,dispcent , zoom_rate ;
	int noise ;


	work->nowmode = GetNowRadarMode() ;
	/*メッセージチェック*/
	CheckMesg(work);
	/*モード表示禁止*/
	/*自動的にレーダーも使用不可*/
	/*状態チェック用*/
	GM_ResetMenuStatus(MENU_RADAR_OFF) ;
	work->count++ ;
	work->count&= 0xffff ;

	if(work->nomcnt > 0) work->nomcnt--;
	/*レーダー表示必要か？モード、ポーズ状態などチェック*/
	if(CheckModeUse(work)){
		/*全消し*/
		SPR_HIDE(work->rad_spr.empty);
		work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
		RctrlAllHide( work ) ;
		SetFramePos(work);
		return ;
	}
	/*危険レベルは表示必要らしい*/
	SPR_SHOW(work->rad_spr.empty);
	/*状態チェック用*/
	GM_SetMenuStatus(MENU_RADAR_ON) ;
	/*
	モード表示 必須
	現在のゲームモードやチャフ
	*/
	noise = ModeDisplay(work) ;

	/*開閉時ノイズエフェクト表示*/
	if((GM_PlayerStatus & RAD_PLAYER_NOUSE)
	||(GM_Configuration & GM_CONFIG_RADAR_OFF)
	||(CheckRadarUseIntrude())
	||(PL_GetPlayerItem() == IT_Scope)
	||(noise == 0)
	){
		SPR_HIDE(work->rad_spr.noise);
	}else {
		SPR_SHOW(work->rad_spr.noise);
	}
	/*本来使用可能だが、一時的に消えている状態*/
	if((GM_PlayerStatus & RAD_PLAYER_NOUSE)
	||(PL_GetPlayerItem() == IT_Scope)){
//		SPR_HIDE(work->rad_spr.use);
	}
//height
	/*レーダー使用禁止*/
	if(CheckRadarUse()||(noise)){

		/*矢印非表示*/
		RctrlAllHide( work ) ;
		/*視界とＨＺＸの非表示*/
		work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
		SetFramePos(work);
		return ;
	}
	/*光点表示*/
	SetR_CtrlsPos(work);
	SetFramePos(work);

	/**/
	work->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU;
	pos = GM_RadarGetCenter();
	/* マトリクスの並行移動設定 */
	zoom_rate.vx = RADAR_ZOOM_RATE_W ;
	zoom_rate.vy = 0.0F ;
	zoom_rate.vz = RADAR_ZOOM_RATE_H ;
	zoom_rate.vw = 0.0F ;

	if(work->base_rot != 0){
		RotVecXZ(pos,&dispcent,(int)work->base_rot&4095) ;
		RotVecXZ(&zoom_rate,&zoom_rate,(int)work->base_rot&4095) ;
	}else {
		dispcent = *pos ;
	}
	work->param.matrix[ 3 ][ 0 ]
//		= pos_norm( 2048.5F - pos->vx * RADAR_ZOOM_RATE_W );
		= pos_norm( 2048.5F - (dispcent.vx )* _ABSf(zoom_rate.vx) );
	work->param.matrix[ 3 ][ 1 ]
//		= pos_norm( ( 2048.5F - pos->vz * RADAR_ZOOM_RATE_H ) * 2.0F ) / 2.0F;
		= pos_norm( ( 2048.5F - dispcent.vz * _ABSf(zoom_rate.vz) ) * 2.0F ) / 2.0F;
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
#if 0
		ph->window.offset.data = SCE_GS_SET_XYOFFSET( ( 2048 - ( x0 + x1 ) / 2 ) << 4
				 , ( ( 2048 - ( y0 + y1 ) / 2 ) << 4 ) + ( DG_CurrentField ? 8 : 0 ));
#else
		/* HI-RESO */
		ph->window.offset.data = SCE_GS_SET_XYOFFSET( ( 2048 - ( x0 + x1 ) / 2 ) << 4
				 , ( ( 2048 - ( y0 + y1 ) / 2 ) << 4 ) );
#endif
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
	/*壁表示*/
	tag = make_hzx_segment_tag( work, pos, tag );
#endif


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
#if 1
	tag = make_pos_display_tag( work, pos, tag );
#endif
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
#if 0
	printf("START ADDR %X \n",work->buffer[ DG_Clock ]);
	printf("NOW ADDR %X \n",tag);
	printf("END ADDR %X \n",&work->buffer[ DG_Clock ][(RAD_BUF_MAX)-1]);
#endif
	/*バッファサイズチェック*/
	if( (int *) tag > (int *)&work->buffer[ DG_Clock ][(RAD_BUF_MAX)-1] ){
		printf("START ADDR %X \n",work->buffer[ DG_Clock ]);
		printf("NOW ADDR %X \n",tag);
		printf("END ADDR %X \n",&work->buffer[ DG_Clock ][(RAD_BUF_MAX)-1]);
		printf("RADAR BUF OVER!!!!!!\n");
		ASSERT(0);
	}
}

static void Die( Work *work )
{
printf("RADAR MONITOR DIE !!!!!!");
	/*親を最後に*/
	SPR_Destroy_2D_Object( work->rad_spr.empty ) ;
	/*こいつだけ子供じゃない*/
	SPR_Destroy_2D_Object( work->rad_spr.radio ) ;
	SPR_KillTexture(work->tri_id) ;
	DG_DequeueDmapack( work->dmapack );
	DG_FreeDmapack( work->dmapack );
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

	printf("/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/\n");
	tp = &( packet->base_tile );

	// GIFTAGの初期化
	DG_SET_GIFTAG( &tp->gif_tag
				   , .FLG = SCE_GIF_REGLIST, .NLOOP = 1, .NREG = 4
				   , .REGS0 = GS_REGS_PRIM, GS_REGS_RGBA, GS_REGS_XYZF2, GS_REGS_XYZF2 );
	DG_SET_GS_REG( &tp->prim
				   , .PRIM = SCE_GS_PRIM_SPRITE
				   , .ABE = 1, .FST = 1 );

//	tp->rgba = SCE_GS_SET_RGBAQ( 0, 0, 0, 0x80/8 * 4, 0 );
	tp->rgba = SCE_GS_SET_RGBAQ( 0, 28, 28, 0x80/8 * 4, 0 );

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
//			fv->vx = sinf( i * 2 * PI / MAX_ROT_TABLE ) * RADAR_ZOOM_RATE_W;
//			fv->vy = cosf( i * 2 * PI / MAX_ROT_TABLE ) * RADAR_ZOOM_RATE_H;
			fv->vx = sinf( i * 2 * PI / MAX_ROT_TABLE ) * RADAR_ZOOM_RATE_W;
			fv->vy = cosf( i * 2 * PI / MAX_ROT_TABLE ) * RADAR_ZOOM_RATE_W;

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

		size = ( sizeof( PACK_HEADER ) - sizeof( DG_DMATAG ) )
			/ sizeof( u_long128 );

		dt->qwc = DMATAG_SET_QWC( DMATAG_ID_RET, size );
		dt->vifcode[ 1 ] = SCE_VIF1_SET_DIRECT( size, 0 );

		end_tag = &( packet->end_tag );
		DG_SET_GIFTAG( end_tag, .EOP = 1, .NLOOP = 1, .NREG = 1, .REGS0 = GS_REGS_AD );
		packet->alpha.addr = SCE_GS_ALPHA_1;
//		packet->alpha.data = SCE_GS_SET_ALPHA( 0,2,2,1,0x80 );
/*TEST*/
		packet->alpha.data = KASAN ;
	}
}

static int GetResources( Work *work, int name )
{
#ifdef DEBUG_MODE
	work->debug = 0 ;
//	GM_Configuration |= GM_CONFIG_RADAR_OFF ;
//	GM_Configuration |= GM_CONFIG_RADAR_OFF_INTRUDE;
#endif
	work->tri_id = SPR_LoadTexture(RADAR_TRI);

	/*表示方向*/
	work->base_rot = (u_short)GCL_GetOptionValue( 'd', 0 ) ;

	work->base_rot &= 4095 ;
	if(work->base_rot >2048 ){
		work->base_rot = -2048 +(work->base_rot-2048) ;
	}

	/*マトリクス回転*/
	InitDefaultMatrix(work) ;

	work->status = RAD_DEFAULT ;

	{
		DG_DMAPACK *dmapack;
		int i;

		dmapack = DG_MakeDmapack( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_AFTER );
		DG_QueueDmapack( dmapack );
		work->dmapack = dmapack;

		/* VU1用パラメータテーブルの初期化 */
		set_vu1_param( work );

		for( i = 0; i < 2; i ++ ){
			PACK_HEADER *pack;
			/* パケット領域の確保 */
			dmapack->packet[ i ] = work->buffer[ i ];
			pack = ( PACK_HEADER * )work->buffer[ i ];
			InitPacket( pack );
		}
	}

	/* 描画環境設定パケット */

	work->display_flag = 1;
	work->name = name;


	/*SprInitの前にやるべし*/
	if( RadParamInit( work ) < 0) return -1 ;
	/*重野追加 スプライト*/
	if( RadSprInit( work ) < 0) return -1 ;
	return 0;
}

void *NewRadar( int name, int map )
{
	/* 通常のキャラ扱い */

	Work		*work;
//	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) );
	work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
									 sizeof( Work ), GM_RADAR_ACTOR_PRIO ) ;

//	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) );
	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die );
		if( GetResources( work, name ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}


//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	radar.c
		????

	1999/10/08 K.Uehara
	2000/01/12 K.Sigeno ???
	2001/02/27 Y.Kira   ???
	$Id: radar.c,v 1.5 2002/11/23 12:16:39 Yoshizawa1 Exp $
*/
#ifdef KP_XBOX

//#define XBOXBACK /*??????????vu??????????? 2002.04.10ynao*/
#endif

#ifdef PSX2
#include <eekernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libpkt.h>
#include <libdma.h>
#include <sifdev.h>
#include <libdev.h>
#include <math.h>
#include <libvu0.h>
#include <string.h>
#endif

#include	"gameheader.h"
#include	"def_dma.h"

#include	"radsprite.h"

#include	"radsize.h"

#include        "radar_config.h"
#include        "radar_macro.h"
#include        "bomb_sensor.h"
#include        "bomb.h"
#include        "outrange.h"
#include        "rdr_movie.h"

#include "bp_matrix.h"

//#define NEW_CLEARING (1)

#ifdef PSX2
#ifdef DEBUG
#define DBG(...)   printf(__VA_ARGS__)
#else
#define DBG(...)
#endif /* DEBUG */
#else
#define DBG
#endif

extern float _RsinF( int ) ;
extern float _RcosF( int ) ;

extern int	DEF_SEARCH_LEVEL ; /*user/korekado/enemy/command.c*/

#ifdef DEBUG_MODE
#include "debugmenu.h"

#ifdef PSX2
static int Radar_DebugMode = 0;
static GM_DEBUG_MENU Radar_debugmenu = {
	class: "RADAR", menu: "DEBUG", max: 2,
	items: ( char *[] ){ "OFF", "ON" },
	target: &Radar_DebugMode,
};
#else
static int Radar_DebugMode = 0;
static char *debug_info_items[] = { "OFF", "ON" };
static GM_DEBUG_MENU Radar_debugmenu = {
	NULL,
	"RADAR",
	"DEBUG",
	debug_info_items,
	NULL,
	&Radar_DebugMode,
	0,
	NULL,
	0,
	0,
	2,
	0,
	0
};
#endif

#endif

/* ---------------------------------------------------------------------- */
/*
	????
*/

/* ????????????????radar_config.h */

/* =========================================================================
 * ?????????
 * ========================================================================= */
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
	// ???????
	DG_DMATAG	dma_tag;
  
	// ???????
	RADAR_WINDOW_SET window;
	// ?????
	RADAR_TILE_SET	base_tile;
	// ??????????
	RADAR_DRAWPARAM_SET	drparam;
	sceGifTag end_tag;
	GIF_AD_DATA	 alpha;
} PACK_HEADER;

typedef ALIGN16_DECL(struct) {
	int color[ 4 ];
} LCOLOR ;

typedef ALIGN16_DECL(struct) {
	/* size max = 0xb00 = 2816 */
	float matrix[ 4 ][ 4 ];
	/* ???? */
	FVECTOR yclip;
	int bright_z, dark_z, pad1, pad2;
	LCOLOR bright_color;
	LCOLOR dark_color;
	LCOLOR door_color;
	sceGifTag giftag;
	/* ????? */
//	int player_z, enemy_z, pad11, pad12;
//	sceGifTag pos_giftag;
	sceGifTag sight_giftag;
	sceGifTag no_sight_giftag;
	/* ????? */
	FVECTOR rot_table[ MAX_ROT_TABLE ];
	/*?????????? */
	struct {
		LCOLOR sight_bright_top;
		LCOLOR sight_bright_end;
		LCOLOR sight_dark_top;
		LCOLOR sight_dark_end;
	} chara_color[ MAX_CHARA_COLOR ];
} VU1_PARAM ;

typedef struct {
	SPR_OBJ		* empty;	/* ???????  */
	SPR_OBJ		* mode ;	/* alert  */
	SPR_OBJ		* use ;	/* ??? NO USE??  */
	SPR_OBJ		* clearing ;	/* ??????? */
	SPR_OBJ		* noise ;	/* ??????? */
	SPR_OBJ		* radio ;	/* ??????? */
	SPR_OBJ		* level ;	/* ?????? */
	SPR_OBJ		* dot ;		/* mode???????*/

	SPR_OBJ		* frameT ;	/* ?*/
   SPR_OBJ		* frameL ;	/* ?*/
   SPR_OBJ		* frameR ;	/* ?*/
   SPR_OBJ		* frameB ;	/* ?*/
//BP JG - IF YOU ADD TO THIS STRUCTURE YOU MUST UPDATE THE SAME STRUCTURE IN XRADAR.C
	SPR_POS		sp_pos;
#ifdef XBOXBACK /*??????????vu??????????? 2002.04.10ynao*/
	SPR_OBJ		* box ;	/* ?? */
#endif
} RAD_SPR ;


typedef struct radar_work {
	GV_ACT_EX actor;
	VU1_PARAM param;
	PACK_HEADER head;
	FVECTOR zoom_rate;
	RAD_SPR		rad_spr ;
	int name;
	int display_flag;
	int prev_flag;
	int prev_frame;
	DG_DMAPACK *dmapack;
	u_short		count ;
	u_char		alert ;
	u_char		noise ;
	float		height ;
	int		status ;
	int		nowmode ;
	u_short	nomcnt	;
	short	base_rot ;	/*????*/
	int		tri_id ;
	float	zoom ;
#ifdef DEBUG_MODE
	int		debug ;
#endif
} Work;


/* ---------------------------------------------------------------------- */
/*
?????????
*/

/* =========================================================================
 * ???????
 * ========================================================================= */

// ??????
static unsigned char default_colors[ 3 ][ 4 ] = {
	{ 0, 160, 72, 0x80 },
	{ 10, 50, 40, 0x80 },
	{ 240, 32,	64, 0x80 },
};

// ?????????
static unsigned char search_colors[ 3 ][ 4 ] = {
	{ 130, 150, 10, 0x80 },
	{ 31, 38, 10, 0x80 },
	{ 240, 32,	64, 0x80 },
};

// ??????
static unsigned char default_chara_colors[][ 4 ][ 4 ] = {
	// RADAR_COLOR_PLAYER
	{
		{ 0, 160, 72, 0x80 },
		{ 0, 0, 0, 0x80 },
		{ 0, 160, 72, 0x80 },
		{ 0, 0, 0, 0x80 },
	},
	// RADAR_COLOR_BLUE
	{
		// ??????
		{ 0x00, 0x80, 0x80, 0x80 },
		{ 0x00, 0x00, 0x10, 0x80 },
		// ??????
		{ 0x00, 0x20, 0x20, 0x80 },
		{ 0x00, 0x00, 0x10, 0x80 },
	},
	// RADAR_COLOR_RED
	{
		{ 0xa0, 0x00, 0x00, 0x80 },
		{ 0x10, 0x00, 0x00, 0x80 },
		{ 0xa0/4, 0x00, 0x00, 0x80 },
		{ 0x10, 0x00, 0x00, 0x80 },
	},
	// RADAR_COLOR_YELLOW
	{
		{ 0xa0, 0xa0, 0x00, 0x80 },
		{ 0x08, 0x08, 0x00, 0x80 },
		{ 0xa0/4, 0xa0/4, 0x00, 0x80 },
		{ 0x08, 0x08, 0x00, 0x80 },
	},
};

static u_int NoiseTexCode[3] ={
	NOISE_1,
	NOISE_2,
	NOISE_3
} ;


/* =========================================================================
 * ?????
 * ========================================================================= */
#if 1 //BP_PS2
static u_long128 Vu1DrawRadarMPGTag[ 1 ] ;	// ????????
static int Vu1DrawRadarFunc[1] ;
#else
extern u_long128 Vu1DrawRadarMPGTag[ 1 ] ;	// ????????
extern int Vu1DrawRadarFunc[] ;
#endif

static FMATRIX default_matrix ;
static int	SegmentExist ;

static Work   * sys_work = NULL;

/* =========================================================================
 * ?????????????
 * ========================================================================= */

/*POINT*/
#if 0
static void SetRGBA_Point(SPR_OBJ *obj,int r,int g,int b,int a)
{
	obj->point.col.r = r ;
	obj->point.col.g = g ;
	obj->point.col.b = b ;
	obj->point.col.a = a ;
}
#endif

static int RAD_Fix2Int(SPR_FIX fix)
{
	int res ;
#if 0 //BP_PSX def PSX2
	res = (int)(fix>>4) ;
#else
	res = (int)fix ;/*XBOX???SPR_FIX?float*/
#endif
	return res ;
}

/**/
static void RotVecXZ(FVECTOR *pos,FVECTOR *res ,int dir)
{
	SVECTOR rot;
	FVECTOR tmppos;

	tmppos = *pos ;
	tmppos.vy = 0.0F ;
	tmppos.vw = 1.0F ;
	rot.vx = 0; rot.vy = (dir)&4095; rot.vz = 0;
	DG_SetPos2( &DG_ZeroVector, &rot ) ;
	//	DG_PutVector( pos, res, 1 ) ;
	DG_PutVector( &tmppos, &tmppos, 1 ) ;
	res->vx = tmppos.vx ;
	res->vy = pos->vy ;
	res->vz = tmppos.vz ;
}
/* ---------------------------------------------------------------------- */
/*
	?????
*/

static void InitDefaultMatrix( Work *work )
{
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
	};

  	default_matrix = base_matrix ;
	if(work->base_rot != 0){
		float rot ;
		rot = (float)(-work->base_rot) * PI / 2048.0F ;
		_sceVu0RotMatrixZ( &default_matrix,
						   &default_matrix, rot );
	}
}
static void InitDefaultMatrix2( Work *work, FVECTOR *center )
{
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
	};
	FMATRIX		rot_mat, trans_mat ;
	float		rot ;

	rot_mat = DG_UnitMatrix ;
	trans_mat = DG_UnitMatrix ;
	trans_mat.m[3][0] = -center->vx ;
	trans_mat.m[3][1] = -center->vy ;
	trans_mat.m[3][2] = -center->vz ;
	rot_mat.m[0][0] = work->zoom ;
	rot_mat.m[2][2] = work->zoom ;
	rot = (float)(work->base_rot) * PI / 2048.0F ;
	_sceVu0RotMatrixY( &rot_mat, &rot_mat, rot );
	_sceVu0MulMatrix( &rot_mat, &rot_mat, &trans_mat );
	_sceVu0MulMatrix( &default_matrix, &base_matrix, &rot_mat );
}
/* ---------------------------------------------------------------------- */
/*
????????
*/

static DMA_TAG *make_hzd_one_segment_tag( DMA_TAG *tag, HZX_BLOCK *block,
										  int prog, HZX_VuSEG *seg, int n_segs)
{
	int i ;
	IVECTOR * center ;

	/*
    ????????????????????????????????????
	*/

	/* ????????????????? */
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

	for ( i = 0; i < n_segs; i ++, seg ++ ){
		if ( seg->atr & HZX_SEG_NO_DISP_RADAR ) continue ;

		/* ???? */
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 1 ) ;
		tag->addr = DMATAG_SET_ADDR( seg->tag ) ;
		tag->vif1 = SCE_VIF1_SET_STMOD( 0x00, 0 ) ; /* ??????OFF */
		tag->vif2 = SCE_VIF1_SET_UNPACK( 0x8000, 1, VIF_DATA128, 0 ) ;
		tag ++ ;
      
		/* ????? */
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, seg->size ) ;
		tag->addr = DMATAG_SET_ADDR( seg->verts ) ;
		tag->vif1 = SCE_VIF1_SET_STMOD( 0x01, 0 ) ; /* ?????? */
		/* V4-16 */
		tag->vif2 = SCE_VIF1_SET_UNPACK( 0x8001, seg->size * 2, 0x0d, 0 ) ;
		tag ++ ;

		/* ?????????? */
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
		tag->vif1 = SCE_VIF1_SET_FLUSHE( 0 ) ;
		tag->vif2 = SCE_VIF1_SET_MSCAL( Vu1DrawRadarFunc[ prog ], 0 ) ;
		tag ++ ;
		prog = SEG_DRAW_NEXT ;
		SegmentExist = 1 ;
	}
	return tag;
}

static	DMA_TAG	*make_hzd_dynamic_segment_tag( DMA_TAG *tag,
											   int prog, HZX_D_SEGMENT *segs ) 
{
	HZX_D_SEGMENT		*seg ;
	int count;
	DMA_TAG *top = NULL;

	seg = segs ;
	count = 0;
	prog = SEG_DYNAMIC_DRAW_START;

	for(seg = segs; seg != NULL; seg = seg->next){
		if (seg->atr & (HZX_SEG_SKIP | HZX_SEG_NO_DISP_RADAR)) continue;

		if( count == 0 ){
			top = tag;
			tag += 2;
		}

		*( IVECTOR * )tag = seg->p1;
		if( seg->atr & HZX_SEG_DOOR ){
			( ( int * )tag )[ 3 ] = - seg->p1.vw;
		}
		tag++;
		*( IVECTOR * )tag = seg->p2;
		tag++;

		count ++;
		if( count > 63 ){
			top->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, count * 2 + 1 );
			top->vif1 = SCE_VIF1_SET_STMOD( 0x00, 0 ) ; /* ??????OFF */
			top->vif2 = SCE_VIF1_SET_UNPACK( 0x8000, count * 2 + 1, VIF_DATA128, 0 ) ;
			// ?????????????
			top ++;
			( ( int * )top )[ 0 ] = count;
			/* ?????????? */
			tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
			tag->vif1 = SCE_VIF1_SET_FLUSHE( 0 ) ;
			tag->vif2 = SCE_VIF1_SET_MSCAL( Vu1DrawRadarFunc[ prog ], 0 ) ;
			tag ++ ;
			SegmentExist = 1 ;
			prog = SEG_DYNAMIC_DRAW_NEXT;
			count = 0;
		}
    }

	if( count > 0 ){
		top->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, count * 2 + 1 );
		top->vif1 = SCE_VIF1_SET_STMOD( 0x00, 0 ) ; /* ??????OFF */
		top->vif2 = SCE_VIF1_SET_UNPACK( 0x8000, count * 2 + 1, VIF_DATA128, 0 ) ;
		// ?????????????
		top ++;
		( ( int * )top )[ 0 ] = count;
		/* ?????????? */
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
		tag->vif1 = SCE_VIF1_SET_FLUSHE( 0 ) ;
		tag->vif2 = SCE_VIF1_SET_MSCAL( Vu1DrawRadarFunc[ prog ], 0 ) ;
		tag ++ ;
		prog = SEG_DYNAMIC_DRAW_NEXT ;
		SegmentExist = 1 ;
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
	bit = HZX_AddGroupID( HZX_CurrentGroupID ) ; /* ????(2001.01.26) */
	prog = SEG_DRAW_START;
  
	while( bit != 0 ){
		group = GV_GetNo( bit ) ;
		bit2 = GV_GetBit( group ) ;
		if ( group >= 0 && group < hzd->def->n_groups ){
			grp = hzd->def->groups + group ;
			inside = HZX_GetNearBlockID( grp, pos, (int)RADAR_VIEW_W, &n ) ;
			for( i = 0; i < n; i++ ){
				block = grp->blocks + *inside;
				inside ++ ;
				if ( block->n_segs > 0 ){
					tag = make_hzd_one_segment_tag( tag, block, prog, 
													block->segs, block->n_segs );
					if ( SegmentExist != 0 ) prog = SEG_DRAW_NEXT ;
				}
			}
			/* ?????????? */
			if ( grp->dynamics->n_segs > 0 ){
				tag = make_hzd_dynamic_segment_tag( tag, prog,
													grp->dynamics->segs ) ;
				if ( SegmentExist != 0 ){
					prog = SEG_DRAW_NEXT ;
				}
			}
		}
		bit &= ~bit2 ;
	}

	/* ???? */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
	tag->vif1 = SCE_VIF1_SET_STMOD( 0x00, 0 ) ; /* ???????OFF */
	tag->vif2 = SCE_VIF1_SET_NOP( 0 );
	tag ++ ;
  
	return tag;
}

/* ---------------------------------------------------------------------- */
/*
	??????????????????
*/

static DMA_TAG *set_pos_display_tag( DMA_TAG *top, DMA_TAG *tag, int n )
{
	int size;
	int prog;

	/* ????????
     DMA ?????VU1 ? VU Mem1 ?????????????????
     ?? DMAtag ??????? */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	/* ????????????????? */
	tag->vif1 = SCE_VIF1_SET_FLUSHE( 0 );
	/* ???????????? */
	prog = CHARA_SIGHT_DRAW;
	tag->vif2 = SCE_VIF1_SET_MSCAL( Vu1DrawRadarFunc[ prog ], 0 );
	tag ++;

	// ????????
	size = n * 3 + 1;
	top->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, size );
	top->vif1 = SCE_VIF1_SET_STCYCL( 1, 1, 0 );          /* 1???????? */
	top->vif2 = SCE_VIF1_SET_UNPACK( 0x8000, size, VIF_DATA128, 0 ); /* V4-32 */

	/* VU1 ????????????????? */
	*( unsigned int * )&( top + 1 )[ 0 ] = n;

	return tag;
}


static DMA_TAG *make_pos_display_tag( Work *work, FVECTOR *pos, DMA_TAG *tag )
{
	DMA_TAG *top;
	RADAR_CTRL *p;
	int n;
  
	/*
    ????????????????????
    ??????????????
  */
  
	if(GV_Time % 16 < 3) return tag;
  
	top = tag;
	tag += 2;  /* DMATAG + ??????????? */
  
	n = 0;
	for( p = GM_RadarControlGetTop(); p != NULL; p = p->next ){
		const int w = ( 4096 / MAX_ROT_TABLE );
		int roty, angle, width, col;
		float length;

		if( !( p->flag & RADAR_VISIBLE ) ){
			continue;
		}
		if( p->col == RADAR_COLOR_PLAYER ){
			// Player
#if 0
			if( ( work->display_flag & RADAR_D_PLAYER ) == 0 ){
				continue;
			}
#endif
		} else {
			if( ( p->flag & RADAR_MINE ) ){
				if( ( work->display_flag & RADAR_D_MINE ) == 0 ){
					continue;
				}
			} else {
				if( ( work->display_flag & RADAR_D_ENEMY ) == 0 ){
					continue;
				}
			}
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
		length = p->range;		// ????????
#endif
		col = p->col;			// ?
		if( !( p->flag & RADAR_SIGHT ) ) col = -1;
		width = angle / 4;	// ?????1?????
		if((0<width)&&(width < w)) width = w ;

		roty = ((p->dir+work->base_rot)&4095) - 2 * width;
		roty = (((roty < 0) ? (roty - w / 2) : (roty + w / 2))
				/ w + MAX_ROT_TABLE) % MAX_ROT_TABLE;
    
		((unsigned int *)tag)[ 0 ] = roty;	/* ????-????1/2 */
		((unsigned int *)tag)[ 1 ] = width / w;	/* ????1/4 */
		((unsigned int *)tag)[ 2 ] = col * 4;	/* color */
		((float *)tag)[ 3 ] = length;		/* ?????? */
		tag ++;
    
		((float *)tag)[ 0 ] = 0.0F;
		((float *)tag)[ 1 ] = p->same_floor_rate;
		((float *)tag)[ 2 ] = p->range_zoom_rate;
		((float *)tag)[ 3 ] = p->range_center - pos->vy * p->range_zoom_rate;
    
		tag ++;
		n++;

		if( n >= 64 ){
			tag = set_pos_display_tag( top, tag, n );

			tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
			/* ????????????????? */
			tag->vif1 = SCE_VIF1_SET_FLUSHE( 0 );
			tag->vif2 = SCE_VIF1_SET_NOP( 0 );
			tag ++;
  
			top = tag;
			tag += 2;  /* DMATAG + ??????????? */
  
			n = 0;
		}
		/*
		????????????????????
		??????? VU1 ? GIF ?????????GS ??????

		// +0
		float  x;   // ?? x ??
		float  y;   // ?? y ??
		float  z;   // ?? z ??
		float  w;   // ?? w ??

		// +1
		int   rot;        // ????
		int   step_width; // ???? 1/4 (TriangleFan)
		int   color;      // ????(RGBA)
		int   length;     // ?????

		// +2
		int   reserved1;  // 0.0F
		int   reserved2;  // 0.0F
		int   zoom_rate;  // ??????
		int   center;     // 
		*/
	}
  
	if( n == 0 ) return top;

	tag = set_pos_display_tag( top, tag, n );

	return tag;
}


/* ---------------------------------------------------------------------- */
/*
	ACT
*/

/* ---------------------------------------------------------------------- */
/*
	?????????
*/

static inline float pos_norm( float pos )
{
  float res;
  
#ifdef BP_PSX2_ASM
  asm( "
	cvt.w.s $f1,%1
	cvt.s.w %0,$f1
	" : "=f"(res) : "f"(pos) : "$f1" );
#else
  int dmdm;
  dmdm = ( int )pos;
  res = ( float )dmdm;
#endif
  return res;
}

static void copy_color( LCOLOR *dest, unsigned char *src )
{
  int i;
  int *d;

  d = &( dest->color[ 0 ] );
  for( i = 0; i < 4; i ++ ) *( d ++ ) = *( src ++ );
}

static void set_default_color( Work *work )
{
	VU1_PARAM *param;

	param = &( work->param );
	copy_color( &param->bright_color, default_colors[ 0 ] );
	copy_color( &param->dark_color, default_colors[ 1 ] );
	copy_color( &param->door_color, default_colors[ 2 ] );
}

static void set_search_color( Work *work )
{
	VU1_PARAM *param;

	param = &( work->param );
	copy_color( &param->bright_color, search_colors[ 0 ] );
	copy_color( &param->dark_color, search_colors[ 1 ] );
	copy_color( &param->door_color, search_colors[ 2 ] );
}

/*?????WH??????????????*/
static void SetTexSizeSpr(SPR_OBJ * sprite)
{
	float w,h;

	w = (float) (RAD_Fix2Int(sprite->sprite.head.tex.w)+1) ;
	h = (float) (RAD_Fix2Int(sprite->sprite.head.tex.h)+1) ;
//printf("W %f H %f \n",w,h);
	SPR_SetSizeSprite(sprite, w, h);
}

/*
 * ??????????????WH?????
 */
static void SetNormWHSpr(SPR_OBJ * sprite)
{
	SPR_FIX dw, dh;      /* ???????     */
	//	float	w;

	dw = SPR_FIXED(sprite->sprite.dw) ;
	dh = SPR_FIXED(sprite->sprite.dh) ;
	sprite->sprite.head.tex.w = dw ;
	sprite->sprite.head.tex.h = dh ;
  
	//	w = (float) (RAD_Fix2Int(sprite->sprite.head.tex.w)+1) ;
	//	h = (float) (RAD_Fix2Int(sprite->sprite.head.tex.h)+1) ;
	//	SPR_SetSizeSprite(sprite, w, h);
}
static void SetWHSpr(SPR_OBJ * sprite,float dw,float dh)
{
//	SPR_FIX dw, dh;      /* ???????     */
	//	float	w;

	dw = SPR_FIXED(sprite->sprite.dw) ;
	dh = SPR_FIXED(sprite->sprite.dh) ;
	sprite->sprite.head.tex.w = dw ;
	sprite->sprite.head.tex.h = dh ;
  
	//	w = (float) (RAD_Fix2Int(sprite->sprite.head.tex.w)+1) ;
	//	h = (float) (RAD_Fix2Int(sprite->sprite.head.tex.h)+1) ;
	//	SPR_SetSizeSprite(sprite, w, h);
}


/* ---------------------------------------------------------------------- */
/*
	???????
*/

static int CheckRadarStatusFlag( Work *work )
{
	int flag = 0;
	if( GM_CheckMenuStatus( MENU_NODE_ACCESSED ) ){
		flag |= RADAR_STATUS_NODE;
	}
	return flag;
}

/*
	??????????????????
*/

static int CheckRadarPartDisplayFlag( Work *work )
{
	int dflag = 0;
	int item;

	if( ( GM_CheckMenuStatus( MENU_MODE_OFF )
		  || ( GV_PauseLevel != 0 )
		  || work->status & RAD_SCN_INVISIBLE )
		){
		/* ??????????????? */
		return 0;
	}

	dflag |= RADAR_D_MODE;

	if( !(
		  ( GM_Configuration & GM_CONFIG_RADAR_OFF )
		  || ( GM_PlayerStatus & ( PLAYER_MENU_OPEN ) )
		  || ( !GM_CheckMenuStatus( MENU_NODE_ACCESSED ) )
		  || ( GM_CheckMenuStatus( MENU_SUBWIN_ON ) )
		  || ((GM_Configuration & GM_CONFIG_RADAR_OFF_INTRUDE) &&
			  (GM_PlayerStatus & (PLAYER_INTRUDE|PLAYER_LOCKER)))
		  )
#ifdef DEBUG_MODE
		|| Radar_DebugMode != 0
#endif
		){
		dflag |= ( RADAR_D_HZX | RADAR_D_PLAYER | RADAR_D_ENEMY | RADAR_D_OUTDIR );
	}

	if( GM_CheckMenuStatus( MENU_SUBWIN_ON ) ){
		dflag |= RADAR_D_SUBWIN;

		return dflag;
	}

	item = PL_GetPlayerItem();

	if( item == IT_MineDetector ){
		dflag |= RADAR_D_MINE | RADAR_D_PLAYER;
	}
	if( item == IT_BombSenserA ){
		dflag |= RADAR_D_C4AREA | RADAR_D_PLAYER;
	}
	if( dflag & RADAR_D_HZX ){
		dflag |= RADAR_D_AIRAREA | RADAR_D_PLAYER;
	}
	return dflag;
}

/*
 * ????????????????????????(????)
 */
int GetRadarDispStatus(void)
{
  Work * work = sys_work;
  int flag;
  int ret;

  if(NULL == work) return 0;

  ret = flag = work->display_flag;

  if( ((( flag & (RADAR_FRAME_DISPLAY & ~RADAR_D_NOISE)) == 0) ||
       ( flag & RADAR_D_NOISE )) &&
      ((work->rad_spr.mode->head.flags & SPR_FLAG_HIDDEN) ||
       (work->rad_spr.empty->head.flags & SPR_FLAG_HIDDEN)))
    ret = 0;

  return ret;
}


static int GetNowRadarMode(void)
{
#if 0
	if((GM_AlertMode == ALERT_MODE_ALERT) || (GM_AlertMode == ALERT_MODE_AVOID)){
		/*?? ??*/
		return GM_AlertMode ;
	}
#endif

	/*???*/
	if(GM_CheckGameStatus(STATE_CHAFF)){
		return RAD_STATE_CHAFF ;
	}
	/*?? ??*/
	return GM_AlertMode ;
}


/* ---------------------------------------------------------------------- */
/*
	??????
*/

static void SetBoxPos( SPR_OBJ *box, SPR_POS *center, float size)
{
	SPR_RECT rect ;

	rect.begin.x = center->x - size*SC_RATE_W ;/*2002.04.08yano*/
	rect.begin.y = center->y - size*SC_RATE_H ;
	rect.end.x = center->x + size*SC_RATE_W ;/*2002.04.08yano*/
	rect.end.y = center->y + size*SC_RATE_H ;


	if(rect.begin.x < (float)RADAR_WINDOW_X0*SC_RATE_W ){/*2002.04.08yano*/
		rect.begin.x = (float)RADAR_WINDOW_X0*SC_RATE_W;/*2002.04.08yano*/
	}
	if(rect.begin.y < ((float)RADAR_WINDOW_Y0*SC_RATE_H)){
		rect.begin.y = (float)RADAR_WINDOW_Y0*SC_RATE_H ;
	}
	if(rect.end.x >((float)RADAR_WINDOW_X1*SC_RATE_W)){/*2002.04.08yano*/
		rect.end.x = (float)RADAR_WINDOW_X1*SC_RATE_W ;/*2002.04.08yano*/
	}
	if(rect.end.y >((float)RADAR_WINDOW_Y1*SC_RATE_H)){
		rect.end.y = (float)RADAR_WINDOW_Y1*SC_RATE_H ;
	}
 
	SPR_SetPosBox( box, &rect) ; 
}

static void SetFramePos( Work *work )
{
   SPR_POS _top;
   SPR_POS _lef;
   SPR_POS _rig;
   SPR_POS _bot;

	SPR_RECT rect ;

	rect.begin.x = 0.0F;
	rect.begin.y = 0.0F;


	if( ( work->display_flag & RADAR_FRAME_DISPLAY ) == 0 &&
	   ((work->rad_spr.mode->head.flags & SPR_FLAG_HIDDEN) ||
		(work->rad_spr.empty->head.flags & SPR_FLAG_HIDDEN)))
   {
		SPR_HIDE(work->rad_spr.frameT);
      SPR_HIDE(work->rad_spr.frameL);
      SPR_HIDE(work->rad_spr.frameR);
      SPR_HIDE(work->rad_spr.frameB);
#ifdef XBOXBACK /*??????????vu??????????? 2002.04.10ynao*/
SPR_HIDE(work->rad_spr.box);
#endif
		return ;
	}

	if(work->rad_spr.mode->head.flags & SPR_FLAG_HIDDEN)
		rect.begin.y += ((float)TOP_SIZE_H ) ; 
	rect.end.x = rect.begin.x + RADAR_WINDOW_W*SC_RATE_W ;/*2002.04.08yano*/
#ifndef PAL 
	rect.end.y = rect.begin.y +SC_RATE_H*2;
#else
	rect.end.y = rect.begin.y +SC_RATE_H;
#endif

#ifdef NEW_CLEARING
//ver 2
	if(( work->display_flag & RADAR_FRAME_DISPLAY )
	||(GM_GameStatus & STATE_CLEARING)
	){
#else
//ver 1
//	if( work->display_flag & RADAR_FRAME_DISPLAY ){
//ver 3
	if(( work->display_flag & RADAR_FRAME_DISPLAY )
	||(GM_GameStatus & STATE_CLEARING)
	){

#endif
		rect.end.y += work->height ;
	}

	if((!(work->rad_spr.use->head.flags & SPR_FLAG_HIDDEN))&&
	   (!(work->rad_spr.mode->head.flags & SPR_FLAG_HIDDEN))){
		rect.end.y += (BOTOM_SIZE_H-SC_RATE_H);
#if 0 //BP_XBOX ndef PSX2
	//XBOX
	//????
      rect.end.y -= 1.0f ;
#endif

	}

	if((!(work->rad_spr.mode->head.flags & SPR_FLAG_HIDDEN))){
		rect.end.y += (float)TOP_SIZE_H;
	}

	SPR_SHOW(work->rad_spr.frameT);
   SPR_SHOW(work->rad_spr.frameL);
   SPR_SHOW(work->rad_spr.frameR);
   SPR_SHOW(work->rad_spr.frameB);

   
#ifdef XBOXBACK /*??????????vu??????????? 2002.04.10ynao*/
SPR_SHOW(work->rad_spr.box);
#endif

	rect.end.y -= 1.0F;		// ??????(By takaki)

   _top.x = rect.begin.x-1;
   _top.y = rect.begin.y;
   _lef.x = rect.begin.x-1;
   _lef.y = rect.begin.y;
   _rig.x = rect.end.x;
   _rig.y = rect.begin.y;
   _bot.x = rect.begin.x-1;
   _bot.y = rect.end.y-1;

	SPR_SetPosSprite( work->rad_spr.frameT, &_top) ; 
   SPR_SetPosSprite( work->rad_spr.frameL, &_lef) ; 
   SPR_SetPosSprite( work->rad_spr.frameR, &_rig) ; 
   SPR_SetPosSprite( work->rad_spr.frameB, &_bot) ; 

   SPR_SetSizeSprite( work->rad_spr.frameT, (rect.end.x-rect.begin.x)+1, 1 ); 
   SPR_SetSizeSprite( work->rad_spr.frameL, 1, rect.end.y-rect.begin.y)-1; 
   SPR_SetSizeSprite( work->rad_spr.frameR, 1, rect.end.y-rect.begin.y)-1; 
   SPR_SetSizeSprite( work->rad_spr.frameB, (rect.end.x-rect.begin.x)+1+1, 1 ); 


#ifdef XBOXBACK /*??????????vu??????????? 2002.04.10ynao*/
if((!(work->rad_spr.use->head.flags & SPR_FLAG_HIDDEN))&&
   (!(work->rad_spr.mode->head.flags & SPR_FLAG_HIDDEN))){
//SPR_SetSizeSprite( work->rad_spr.box, RADAR_WINDOW_W*SC_RATE_W, rect.end.y-rect.begin.y-BOTOM_SIZE_H_LEVEL);
SPR_HIDE(work->rad_spr.box);
}else{
SPR_SetSizeSprite( work->rad_spr.box, RADAR_WINDOW_W*SC_RATE_W, rect.end.y-rect.begin.y);
}
//{ SPR_OBJ* box = work->rad_spr.box;
//printf("rect:: begin:%f,%f, end:%f,%f",rect.begin.x,rect.begin.y,rect.end.x,rect.end.y);
//printf("box::: begin:%f,%f, end:%f,%f",box->box.rect->begin.x,box->box.rect->begin.y,box->box.rect->end.x,box->box.rect->end.y);
//}
#endif

}

static void SetRadioCol( SPR_OBJ *radio ,int r,int g,int b,int a)
{
	int i;

	for(i = 0; i < 5; i++)
		SPR_SetColorLineStrip(radio, i, r, g, b, a);
}

static void SetRadioMark( SPR_OBJ *radio ,float x ,float y ,float size)
{
	SPR_POS radio_pos[5] ;
	float x0,x1,y0,y1 ;
   float toWidescreen = 4.0f/3.0f;
	x0 = x-size ; x1 = x+size ;
	y0 = y-size*toWidescreen ; y1 = y+size*toWidescreen ;
  
	if(x0 < (RADAR_WINDOW_X0 * SC_RATE_W)) x0 = RADAR_WINDOW_X0 * SC_RATE_W ;/*2002.04.08yano*/
	if(x1 > (RADAR_WINDOW_X1 * SC_RATE_W)) x1 = RADAR_WINDOW_X1 * SC_RATE_W ;/*2002.04.08yano*/
	if(y0 < (RADAR_WINDOW_Y0 * SC_RATE_H)) y0 = (RADAR_WINDOW_Y0 * SC_RATE_H) ;
	if(y1 > (RADAR_WINDOW_Y1 * SC_RATE_H)) y1 = (RADAR_WINDOW_Y1 * SC_RATE_H) ;
  
	radio_pos[0].x = radio_pos[3].x = x0 ;
	radio_pos[1].x = radio_pos[2].x = x1 ;
  
	radio_pos[0].y = radio_pos[1].y = y0 ;
	radio_pos[2].y = radio_pos[3].y = y1 ;
  
	radio_pos[4] = radio_pos[0] ;
  
	SPR_SetPosLineStrip(radio, 0, 5, radio_pos);
}

static void set_base_color_sneak( Work *work )
{
	PACK_HEADER *packet ;
	RADAR_TILE_SET *tp;

	packet = &work->head;
	tp = &( packet->base_tile );
	tp->rgba = SCE_GS_SET_RGBAQ( 0, 28, 28, BASE_ALPHA, 0 );
}

static void set_base_color_search( Work *work )
{
	PACK_HEADER *packet ;
	RADAR_TILE_SET *tp ;

	packet = &work->head;
	tp = &( packet->base_tile );
	tp->rgba = SCE_GS_SET_RGBAQ( 0, 50, 45, BASE_ALPHA, 0 );
}

static void set_mode_look_and_feel(Work * work,
								   int mode_tex, int use_tex,
								   int r, int g, int b, int a,
								   int show_flags)
{
	SPR_SetSizeSprite(work->rad_spr.level, (RADAR_WINDOW_W-2),BOTOM_SIZE_H);

	if(mode_tex >= 0)
		SPR_ObjSetTexture(work->rad_spr.mode, mode_tex, work->tri_id);
  
	if(use_tex >= 0)
		SPR_ObjSetTexture(work->rad_spr.use, use_tex, work->tri_id);
  
	SET_COLOR_2DPRIM2(work->rad_spr.use, r, g, b, 0x80);
	SET_COLOR_2DPRIM2(work->rad_spr.dot, r, g, b, 0x80);
	SET_COLOR_2DPRIM2(work->rad_spr.level, r, g, b, a);



	SPR_HIDE(work->rad_spr.use);
	SPR_HIDE(work->rad_spr.empty);
	SPR_HIDE(work->rad_spr.mode);
	SPR_HIDE(work->rad_spr.noise);

	if(show_flags & RADAR_SHOW_USE)   SPR_SHOW(work->rad_spr.use);
	if(show_flags & RADAR_SHOW_EMPTY) SPR_SHOW(work->rad_spr.empty);
	if(show_flags & RADAR_SHOW_MODE)  SPR_SHOW(work->rad_spr.mode);
	if(show_flags & RADAR_SHOW_NOISE) SPR_SHOW(work->rad_spr.noise);
}

/***
DRAW_Z_MIN	(4096) Z??? 
DRAW_Z_MAX	(16777216-4096)	Z??? 
pri
*/

/* ---------------------------------------------------------------------- */
/*
	????/??????
*/

/*???????0??? 1??*/
static int CheckRadarRange(RADAR_CTRL *r_ctrl)
{
	//	RADAR_VIEW_W
	FVECTOR *center,*trg ;

	return 1 ;
  
	center = GM_RadarGetCenter() ;
	trg = r_ctrl->pos ;
	if((fabsf( trg->vx - center->vx ) >= (float) RADAR_VIEW_W ) ||
	   ( fabsf( trg->vz - center->vz ) >= (float) RADAR_VIEW_W )){
		return 0 ;
	}

	return 1 ;
}

static void SetHeadColor(Work *work,RADAR_CTRL *r_ctrl,FVECTOR *cent)
{
	float dif_y;
	unsigned char *c_col ;
	static  unsigned char pl_color_bright[4] = { 200, 200, 200, 0x80 } ;
	static  unsigned char ene_color[ 2 ][ 4 ] = {
		{ 255, 30, 0, 0x80 }, { 120,32, 0, 0x80 }
	};
	static unsigned char mine_color[ 2 ][ 4 ] = {
		{ 0xA0, 0xA0, 0, 0x80 }, { 0xa0/4, 0xa0/4, 0, 0x80 }
	};
	static unsigned char node_color[ 2 ][ 4 ] = {
		{ 80, 80, 250, 0x80 }, { 80, 80, 250, 0x80 }
	};

#ifdef DEBUG_MODE
	ASSERT(r_ctrl->col < MAX_CHARA_COLOR) ;
#endif
	dif_y = r_ctrl->pos->vy - cent->vy ;

	if(r_ctrl->col == RADAR_COLOR_PLAYER){
		c_col = pl_color_bright ;
	} else {
		unsigned char *bc, *dc;
		if( r_ctrl->flag & RADAR_MINE ){
			bc = mine_color[ 0 ];
			dc = mine_color[ 1 ];
		} else if( r_ctrl->flag & RADAR_NODE ){
			bc = node_color[ 0 ];
			dc = node_color[ 1 ];
		} else {
			bc = ene_color[ 0 ];
			dc = ene_color[ 1 ];
		}

		dif_y = ( dif_y * r_ctrl->range_zoom_rate ) + r_ctrl->range_center;

		if( DG_FABS( dif_y ) * r_ctrl->same_floor_rate < 1.0F ){
			c_col = bc;
		} else {
			c_col = dc;
		}
#if 0
		if ((dif_y <= UPPER_RANGE)&&(DOWN_RANGE <= dif_y )){
			c_col = bc;
		} else {
			c_col = dc;
		}
#endif
	}
	SPR_SetColorBox(r_ctrl->face,c_col[0], c_col[1],c_col[2],c_col[3]) ;
}

static void RctrlAllHide( Work *work )
{
	RADAR_CTRL *r_ctrl;
	int i;
	RAD_SPR *rad_spr ;
  
	rad_spr = &work->rad_spr ;
	SPR_HIDE(rad_spr->radio) ;

	for( i = 0, r_ctrl = GM_RadarControlGetTop(); 
		 ((r_ctrl != NULL) );
		 r_ctrl = r_ctrl->next, i++){
		SPR_HIDE(r_ctrl->face);
	}
}

/*??????????????*/
/*RADAR_CTRL?????*/
static void SetR_CtrlsPos( Work *work )
{
	RADAR_CTRL *r_ctrl;
	int i, radicnt,radio_limit;
	SPR_POS	r_pos;
	FVECTOR center ,tmppos ,zoom_rate ;
	RAD_SPR *rad_spr ;

	rad_spr = &work->rad_spr ;

	SPR_HIDE(rad_spr->radio);

	if(work->base_rot != 0){
		RotVecXZ(GM_RadarGetCenter(),&center,(int)work->base_rot) ;
	} else {
		center = *GM_RadarGetCenter() ;
	}

	zoom_rate = work->zoom_rate;

	//SC_RATE_H
	/*????*/
	for( i = 0,r_ctrl = GM_RadarControlGetTop(); 
		 ((r_ctrl != NULL) );
		 r_ctrl = r_ctrl->next, i++ ){

		SPR_HIDE(r_ctrl->face);
		if(GV_Time % 16 < 3) continue ;
#if 0
		if((GM_AlertMode == ALERT_MODE_ALERT) ||
		   (GM_AlertMode == ALERT_MODE_AVOID))
			continue ;
#endif
      /*?????NOP*/
		if(!(r_ctrl->flag & RADAR_VISIBLE)) continue;

		// ?????????????
		if( r_ctrl->col == RADAR_COLOR_PLAYER ){
			// Player
#if 0
			if( ( work->display_flag & RADAR_D_PLAYER ) == 0 ){
				continue;
			}
#endif
		} else {
			if( r_ctrl->flag & RADAR_MINE ){
				// ?????
				if( ( work->display_flag & RADAR_D_MINE ) == 0 ){
					continue;
				}
			} else if( r_ctrl->flag & RADAR_NODE ){
				// NODE???:?????
			} else {
				// ??
				if( ( work->display_flag & RADAR_D_ENEMY ) == 0 ){
					continue;
				}
			}
		}
		if(CheckRadarRange(r_ctrl)){
			RotVecXZ(r_ctrl->pos,&tmppos,(int)work->base_rot) ;
			r_pos.x = (float)(tmppos.vx - center.vx) 
				* _ABSf(zoom_rate.vx)+(float)(RADAR_WINDOW_CX);
			//	* (zoom_rate.vx)+(float)(RADAR_WINDOW_CX);

			r_pos.y = (float)(tmppos.vz - center.vz) 
				* _ABSf(zoom_rate.vz)+(float)(RADAR_WINDOW_CY);
			//	* (zoom_rate.vz)+(float)(RADAR_WINDOW_CY);

			if(((float) RADAR_WINDOW_X0 >= r_pos.x ) ||
			   ((float) RADAR_WINDOW_X1 <= r_pos.x ) ||
			   ((float) RADAR_WINDOW_Y0 >= r_pos.y ) ||
			   ((float) RADAR_WINDOW_Y1 <= r_pos.y ))  /*???????*/
				continue;
			/*??????????*/
			radio_limit = 1 ;
			/*????*/
         // this is the outline around the radar blip
         SPR_HIDE(r_ctrl->face); // was SPR_SHOW
         r_pos.x +=1;
			r_pos.x *= SC_RATE_W ;/*2002.04.08yano*/
			r_pos.y *= SC_RATE_H ;
			if(r_ctrl->col == RADAR_COLOR_PLAYER){
				/* ?????????????????*/
				SetBoxPos(r_ctrl->face,&r_pos,0.5F);
			} else {
				SetBoxPos(r_ctrl->face,&r_pos,1.0F);
			}
	  
			SetHeadColor(work,r_ctrl,&center);

			/*??*/
			if(r_ctrl->flag & RADAR_RADIO ) {
				//			if((radio_limit)){
				//				radicnt = (work->count & 0x3f) ;
				radicnt = (work->count & 31) ;
				radicnt /= 7 ;
				if((radicnt > 1) && (radicnt < 6)){
					SPR_SHOW(rad_spr->radio);
		  
					if(DG_Clock)
						SetRadioCol( rad_spr->radio ,255,255,127,127) ;
					else
						SetRadioCol( rad_spr->radio ,127,127,127,127) ;
					SetRadioMark(rad_spr->radio, r_pos.x, r_pos.y,
								 ((float)(radicnt) * 1.5F + 0.0F));
				}
			}
		}
    }
}

/* ---------------------------------------------------------------------- */
/*
	????????????
*/
static void rad_tex_look_and_feel(Work * work, int col,
								  int r, int g, int b, int show_flags)
{

	if(GM_GameStatus & STATE_CLEARING) {
		SPR_ObjSetTexture(work->rad_spr.use, CLE_B1, work->tri_id);
		SET_COLOR_2DPRIM2(work->rad_spr.use,0,0,0,127);
	}else 
   {
		if( work->nowmode == ALERT_MODE_SEARCH )
      {
			SPR_ObjSetTexture(work->rad_spr.use, NOM_B1, work->tri_id);
		}else {
			SPR_ObjSetTexture(work->rad_spr.use, ALT_B1, work->tri_id);
		}
		SET_COLOR_2DPRIM2(work->rad_spr.use,
				  (int)((col * r)/255),(int)((col * g)/255),
				  (int)((col * b)/255),0x80);
	}

   SPR_SetSizeSprite(work->rad_spr.level, (RADAR_WINDOW_W-3) ,BOTOM_SIZE_H);

	if(GM_CheckGameStatus(STATE_CHAFF)){
		SET_COLOR_2DPRIM2(work->rad_spr.clearing,80,130,80,col);
	}else {
		SET_COLOR_2DPRIM2(work->rad_spr.clearing,130,130,92,col);
	} 

	SPR_SHOW(work->rad_spr.empty);
	SPR_SHOW(work->rad_spr.level);

	SPR_HIDE(work->rad_spr.use);

	if(show_flags & RADAR_SHOW_MODE) SPR_SHOW(work->rad_spr.mode);
	if(show_flags & RADAR_SHOW_USE) SPR_SHOW(work->rad_spr.use);
}

static void RadTexAnim(Work *work)
{
	int col ;
	int flags;
   SPR_POS	b_pos;
   float _BP_AlertLevel;
   float _BP_JammingLevel;
   float _BP_CautionLevel;


	if((work->count & TOP_CYCLE1) > (TOP_CYCLE1 / 2)){
		SPR_SHOW(work->rad_spr.dot) ;
	} else {
		if((work->count & TOP_CYCLE2) < (TOP_CYCLE2 / 2) ){
			SPR_HIDE(work->rad_spr.dot) ;
		} else {
			SPR_SHOW(work->rad_spr.dot) ;
		}
    }

#ifdef NEW_CLEARING
//	flags = ((GM_GameStatus & STATE_CLEARING)||( work->display_flag & RADAR_D_SUBWIN )) ? 0 : RADAR_SHOW_USE;
	/*Ver 2*/
	flags = RADAR_SHOW_USE;
#else
#if 0
	/*Ver 1*/
	flags = ( work->display_flag & RADAR_D_SUBWIN ) ? 0 : RADAR_SHOW_USE;
#else
	/*Ver 3 2001.10.01*/
//	flags = (GM_GameStatus & STATE_CLEARING) ? RADAR_SHOW_MODE : (RADAR_SHOW_USE|RADAR_SHOW_MODE);
	flags = RADAR_SHOW_USE;
#endif
#endif
	col = (BOTTOM_CYCLE1 - (work->count & BOTTOM_CYCLE1)) * 4 ;


   // move the level across a little to show the black outline.
   b_pos.x = 1.0F;
   b_pos.y = work->rad_spr.level->sprite.pos.y;
   SPR_SetPosSprite(work->rad_spr.level,&b_pos);



   //BP JG - make sure all levels are capped.
   _BP_AlertLevel = (float)GM_AlertLevel;
   _BP_JammingLevel = (float)GM_JammingLevel;
   _BP_CautionLevel = (float)GM_CautionLevel;   
   if ( _BP_AlertLevel > (float)ALERT_LEVEL_MAX )  _BP_AlertLevel = (float)ALERT_LEVEL_MAX;
   if ( _BP_JammingLevel > (float)JAMMING_LEVEL_MAX )  _BP_JammingLevel = (float)JAMMING_LEVEL_MAX;
   if ( _BP_CautionLevel > (float)CAUTION_BASE )  _BP_CautionLevel = (float)CAUTION_BASE;

	switch ( work->nowmode ){
	  case ALERT_MODE_ALERT :
		rad_tex_look_and_feel(work, col, ALT_R, ALT_G, ALT_B, flags);
      //printf("(float)GM_AlertLevel = %f   (value = %f)\n", _BP_AlertLevel, (_BP_AlertLevel / (float)ALERT_LEVEL_MAX) );
		SPR_SetSizeSprite(work->rad_spr.level, ( _BP_AlertLevel / (float)ALERT_LEVEL_MAX) * (RADAR_WINDOW_W-3), BOTOM_SIZE_H-1);   
      break ;
	  case ALERT_MODE_AVOID :
		rad_tex_look_and_feel(work, col, ESC_R, ESC_G, ESC_B, flags);
      SPR_SetSizeSprite(work->rad_spr.level, work->rad_spr.level->sprite.dw,BOTOM_SIZE_H-1);   
		break ;
	  case RAD_STATE_CHAFF :
		rad_tex_look_and_feel(work, col, JAM_R, JAM_G, JAM_B, flags);
		SPR_SetSizeSprite(work->rad_spr.level, (RADAR_WINDOW_W-3) *(_BP_JammingLevel / (float)JAMMING_LEVEL_MAX), (float)BOTOM_SIZE_H-1);
      
      break ;
	  case ALERT_MODE_SEARCH :
		rad_tex_look_and_feel(work, col, SRC_R, SRC_G, SRC_B, flags | RADAR_SHOW_MODE);
		SPR_SetSizeSprite(work->rad_spr.level, (_BP_CautionLevel/CAUTION_BASE) * (RADAR_WINDOW_W-3),BOTOM_SIZE_H);
		break ;
      
	  case ALERT_MODE_SNEAK :
		SPR_HIDE(work->rad_spr.mode);
		SPR_HIDE(work->rad_spr.use);
		break;
    }
}

static void ModeChange(Work *work ,int mode )
{
	switch ( mode ){
	  case ALERT_MODE_SNEAK :
		/*?????????*/
		set_default_color(work);
		/*?????*/
		set_base_color_sneak( work ) ;
		if(work->alert == ALERT_MODE_SEARCH)
      {
			GM_SdSet( SD_S_RADAR001 ) ;
			work->nomcnt = 60 ;
		}
		set_mode_look_and_feel(work, -1, NOM_B1,
							   SNK_R, SNK_G, SNK_B, 0x80,
							   RADAR_SHOW_EMPTY | RADAR_SHOW_NOISE);
		break ;
	  case ALERT_MODE_ALERT :
		set_mode_look_and_feel(work, ALT_T1, ALT_B1,
							   ALT_R, ALT_G, ALT_B, 0x80,
							   RADAR_SHOW_USE  | RADAR_SHOW_EMPTY |
							   RADAR_SHOW_MODE | RADAR_SHOW_NOISE);
		break ;
	  case ALERT_MODE_AVOID :
		set_mode_look_and_feel(work, ESC_T1, ESC_B1,
//		set_mode_look_and_feel(work, CLE_T1, ESC_B1,
							   ESC_R, ESC_G, ESC_B, 0x60,
							   RADAR_SHOW_USE  | RADAR_SHOW_EMPTY |
							   RADAR_SHOW_MODE | RADAR_SHOW_NOISE);
      SPR_SetSizeSprite(work->rad_spr.level, work->rad_spr.level->sprite.dw,BOTOM_SIZE_H-1);   

		break ;
	  case ALERT_MODE_SEARCH :
		set_search_color(work);
		set_base_color_search( work ) ;
		set_mode_look_and_feel(work, SRC_T1, NOM_B1,
							   SRC_R, SRC_G, SRC_B, 0x80,
							   RADAR_SHOW_USE  | RADAR_SHOW_EMPTY |
							   RADAR_SHOW_MODE | RADAR_SHOW_NOISE);
		break ;
	  case RAD_STATE_CHAFF :
		set_mode_look_and_feel(work, JAM_T1, ESC_B1,
							   JAM_R, JAM_G, JAM_B, 0x80,
							   RADAR_SHOW_EMPTY |
							   RADAR_SHOW_MODE | RADAR_SHOW_NOISE);
		break ;
	}
}

/*??? ????1*/
#define CLE_SIZE_X (88.0f)
#define CLE_SIZE_Y (8.0f)
//#define CLE_OFF_X (16.0) //( 120-88 ) / 2 
#define CLE_OFF_X (1.0) 
//#define CLE_OFF_Y (2.0f)
#define CLE_OFF_Y (2.2f)

static int ModeDisplay(Work *work)
{
	RAD_SPR *rad_spr ;
	SPR_POS	b_pos;
	int noise = 0;
	int change = 0;
	int frame;

	frame = ( work->display_flag & RADAR_FRAME_DISPLAY );
	rad_spr = &work->rad_spr ;
	SPR_HIDE(rad_spr->use);
	if(work->alert == work->nowmode ){
		/* ????? */
		RadTexAnim(work);
	} else {
		/*???????*/
		ModeChange(work,work->nowmode);
		work->alert = work->nowmode ;
		change = 1;
    }
	/*??*/
	if(
#ifdef NEW_CLEARING
	(GM_GameStatus & STATE_CLEARING)
#else
#if 0
	(work->display_flag & RADAR_D_SUBWIN)
	&&(GM_AlertMode == ALERT_MODE_AVOID) 
	&&(!(GM_CheckGameStatus(STATE_CHAFF)))
#else
//
	(GM_GameStatus & STATE_CLEARING)
#endif
#endif
	){
		SPR_SHOW(rad_spr->clearing);
	}else {
		SPR_HIDE(rad_spr->clearing);
	}

#ifdef NEW_CLEARING
	if(( work->display_flag & RADAR_D_SUBWIN )
	||(GM_GameStatus & STATE_CLEARING)
	){
#else
	if( work->display_flag & RADAR_D_SUBWIN ){
#endif

#ifdef NEW_CLEARING
// Ver 2
		if(
			(!(GM_GameStatus & STATE_CLEARING))&&
			((GM_AlertMode == ALERT_MODE_ALERT) ||
			(GM_AlertMode == ALERT_MODE_AVOID) ||
			(GM_CheckGameStatus(STATE_CHAFF)))
		){
#else
#if 0
// Ver 1
		if((GM_AlertMode == ALERT_MODE_ALERT) ||
		   (GM_AlertMode == ALERT_MODE_AVOID) ||
		   (GM_CheckGameStatus(STATE_CHAFF))){
#else
// Ver 3
//???????????
		if(0){
#endif
#endif
			/*????*/
			work->height = 0.0F ;      /*2000.10.28 TEST*/
		} else {
			/*????*/
			work->height =  (float) ((RADAR_WINDOW_H - 1) * SC_RATE_H)  ;

#ifdef NEW_CLEARING
			if(!( work->display_flag & RADAR_D_SUBWIN )){
				noise = 1 ;
				if(work->noise >= 3) work->noise = 0;
				SPR_ObjSetTexture(work->rad_spr.noise,NoiseTexCode[work->noise], work->tri_id);
				work->noise++;
				SPR_SetSizeSprite(rad_spr->noise, 120.0F,work->height);
				SetNormWHSpr(rad_spr->noise) ;
			}
#endif
		}
    } else {
		/* ???????? */
		int flag;

		flag = CheckRadarStatusFlag( work );
		if( ( ~work->prev_flag & flag ) ){
			work->height = 0.0F;
			change = 1;
		}
		work->prev_flag = flag;
#ifdef DEBUG_MODE
		if( Radar_DebugMode != 0 ){
			noise = 0;
			work->height =  (float) ((RADAR_WINDOW_H-1) * SC_RATE_H)  ;
		} else
#endif
		if((GM_AlertMode == ALERT_MODE_ALERT) ||
		   (GM_AlertMode == ALERT_MODE_AVOID) ||
		   (GM_CheckGameStatus(STATE_CHAFF))){
			/*??*/
			work->display_flag &= ~RADAR_FRAME_DISPLAY;
			if( frame && work->height > 0.0F) {
				work->height -= CLOSE_SPEED ;
				noise = 1 ;
				work->display_flag |= RADAR_D_NOISE;
			} else {
				work->height = 0.0F;
			}
			if(work->height < 0.0F) work->height = 0.0F ;
		} else {
			/*??*/
			if( ! frame ){
				if( work->height > 0.0F ){
//					GM_SdSet( SD_S_RADAR001 ) ;
				}
				work->height = 0.0F;
			} else {
				if( ( work->height == 0.0F && change == 0 )
					||  work->height > (float)(RADAR_WINDOW_H - 1) * SC_RATE_H ){
					work->height =  (float) ((RADAR_WINDOW_H-1) * SC_RATE_H)  ;
					printf("NOiSE OPEN SE\n");
					if( work->prev_frame != 0 ){
						GM_SdSet( SD_S_RADAR001 ) ;
					}
				}
				if(work->height < (float)(RADAR_WINDOW_H - 1) * SC_RATE_H){
					work->height += CLOSE_SPEED ;
					noise = 1 ;
					work->display_flag &= ~RADAR_FRAME_DISPLAY;
					work->display_flag |= RADAR_D_NOISE;
				}
			}
		}

		if(noise){
			if(work->noise >= 3) work->noise = 0;
			SPR_ObjSetTexture(work->rad_spr.noise, 
							  NoiseTexCode[work->noise], work->tri_id);
			work->noise++;
		}

      SPR_SetSizeSprite(rad_spr->noise, RADAR_WINDOW_W,work->height);
		SetNormWHSpr(rad_spr->noise) ;
    }
	work->prev_frame = frame;

	b_pos.x = 0.0F;
	b_pos.y = work->height+TOP_SIZE_H;
  
	SPR_SetPosSprite(rad_spr->use,&b_pos);
#if 1
	b_pos.x += CLE_OFF_X ;
	b_pos.y += CLE_OFF_Y ;
SPR_SetPosSprite(rad_spr->clearing,&b_pos);
#endif

	return noise;
}


/* ---------------------------------------------------------------------- */
/*
	ACT ??
*/

static void CheckMesg(Work *work)
{
	GV_MSG *msg;
	int n;

	for(n = GV_ReceiveMessage(work->name, &msg); n > 0; n--){
		switch( msg->message[ 0 ] ){
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

static void Act( Work *work )
{
	PACK_HEADER *ph;
	DMA_TAG *tag;
	FVECTOR *pos ,dispcent , zoom_rate ;
	int noise ;

   if ( GV_PauseLevel & GV_PAUSE_DEBUG )
   {
      return;
   }

#if 0
	/*???? ??*/
	if ( GV_PadData[1].status & PAD_L ){
		work->base_rot += 60 ;
		if(work->base_rot >= 2048 ){
			work->base_rot -= 4096 ;
		}
	} else if ( GV_PadData[1].status & PAD_R ){
		work->base_rot -= 60 ;
		if(work->base_rot < -2048 ){
			work->base_rot += 4096 ;
		}
	}
	/*??????????*/
	//InitDefaultMatrix(work) ;
	InitDefaultMatrix2( work, GM_RadarGetCenter() );
	/* VU1???????????? */
	//set_vu1_param( work );
	DG_COPY_MAT( work->param.matrix, &default_matrix );
#endif

	/* ??????????????? */
	InitDefaultMatrix2( work, GM_RadarGetCenter() );
	DG_COPY_MAT( work->param.matrix, &default_matrix );

	work->nowmode = GetNowRadarMode() ;
	/*?????????*/
	CheckMesg(work);
	/*???????*/
	/*?????????????*/
	/*?????? ?*/
	GM_ResetMenuStatus(MENU_RADAR_ON) ;
	work->count++ ;
	work->count&= 0xffff ;

	if(work->nomcnt > 0){
		work->nomcnt--;
	}

	work->display_flag = CheckRadarPartDisplayFlag( work );
      
	/*?????????????????????????*/
	if( work->display_flag == 0 ){
		/*???*/
		SPR_HIDE(work->rad_spr.empty);
		work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
		RctrlAllHide( work ) ;
		SetFramePos(work);
		return ;
	}
	/*?????????????*/
	SPR_SHOW(work->rad_spr.empty);

	/*???????*/
	GM_SetMenuStatus(MENU_RADAR_ON) ;

	/*
    ????? ??
    ?????????????
	*/
	noise = ModeDisplay(work) ;

	/*?????????????*/
	if( noise == 0 ){
		SPR_HIDE(work->rad_spr.noise);
	} else {
		SPR_SHOW(work->rad_spr.noise);
	}
#if 0
	if(!noise) {
		GV_CallChildSignalFunc(work, BOMB_MODE_SIGNAL, work->nowmode);
	} else {
		GV_CallChildSignalFunc(work, BOMB_MODE_SIGNAL, BOMB_FORCE_DISABLE);
	}
#else
	GV_CallChildSignalFunc(work, BOMB_MODE_SIGNAL, work->display_flag );
#endif

	/*????????*/
	if( noise || ( work->display_flag & RADAR_FRAME_DISPLAY ) == 0 ){
		/*?????*/
		RctrlAllHide( work ) ;
		/*???HZX????*/
		work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
		SetFramePos(work);

		return ;	// ??
	}

	/*????*/
	if( work->display_flag & ( RADAR_D_PLAYER | RADAR_D_ENEMY | RADAR_D_MINE ) ){
		SetR_CtrlsPos(work);
	}

	/* ?/????? */
	SetFramePos(work);
  
	/* ???????????? */
	work->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU;
	pos = GM_RadarGetCenter();

	/* ???????????? */
	zoom_rate = work->zoom_rate;

	if(work->base_rot != 0){
		RotVecXZ(pos,&dispcent,(int)work->base_rot&4095) ;
	} else {
		dispcent = *pos ;
	}

#if 0
	work->param.matrix[ 3 ][ 0 ]
		= pos_norm(2048.5F - (dispcent.vx ) * _ABSf(zoom_rate.vx));
	work->param.matrix[ 3 ][ 1 ]
		= pos_norm((2048.5F - dispcent.vz * _ABSf(zoom_rate.vz)) * 2.0F) / 2.0F;
#endif
	work->param.matrix[ 3 ][ 2 ] = work->param.matrix[ 3 ][ 3 ]
		=  -((UPPER_RANGE+DOWN_RANGE)/2.0F + pos->vy) * RADAR_ZOOM_RATE_Y;

//	ph = ( PACK_HEADER * )work->buffer[ DG_Clock ];

#if 0 //BP_PS2 def PSX2
	ph = DG_OpenDmaPacketArea();
	work->dmapack->packet[ DG_Clock ] = ph;
	memcpy( ph, &work->head, sizeof( work->head ) );
	
	ph->dma_tag.qwc = ( ph->dma_tag.qwc & 0x8FFFFFFF ) | ( DMATAG_ID_CNT );
	{
		/* ?????????? */
		int x0, x1, y0, y1;
		x0 = RADAR_WINDOW_X0;
		y0 = RADAR_WINDOW_Y0;
		x1 = RADAR_WINDOW_X1;
		y1 = RADAR_WINDOW_Y1;

		/* HI-RESO */
		ph->window.offset.data = SCE_GS_SET_XYOFFSET((2048 - (x0 + x1) / 2) << 4,
													 (2048 - (y0 + y1) / 2) << 4);
	}
  
	tag = ( DMA_TAG * )( ph + 1 );

	/* ???????? */
	tag->qwc = DMATAG_SET_QWC(DMATAG_ID_REF,
							  sizeof(VU1_PARAM) / sizeof(u_long128));
	tag->addr = DMATAG_SET_ADDR(&work->param);
	tag->vif1 = SCE_VIF1_SET_STCYCL(1, 1, 0);		// ????
	tag->vif2 = SCE_VIF1_SET_UNPACK(0x0, sizeof(VU1_PARAM) / sizeof(u_long128),
									VIF_DATA128, 0);
	tag ++;
  
	/* ????????? */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	tag->vif1 = SCE_VIF1_SET_BASE( 0x0B00/16, 0 );
	tag->vif2 = SCE_VIF1_SET_OFFSET( 0x1A00/16, 0 );
	tag ++;
  
	/* ??????????? */
	*( u_long128 * )tag = Vu1DrawRadarMPGTag[ 0 ] ;
	tag ++;
  
	SegmentExist = 0 ;

	/* HZX?????????DMA????? */

	/* ??? */
	if( work->display_flag & RADAR_D_HZX ){
		tag = make_hzx_segment_tag( work, pos, tag );
	}

	/* ???? */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	tag->vif1 = SCE_VIF1_SET_FLUSHA( 0 );
	tag->vif2 = SCE_VIF1_SET_NOP( 0 );
	tag ++;

	/* ZBUFFER ???????? */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 2 );
	tag->vif1 = SCE_VIF1_SET_NOP( 0 );
	tag->vif2 = SCE_VIF1_SET_DIRECT( 2, 0 );
	tag++;
	DG_SET_GIFTAG((sceGifTag *)tag,
				  .EOP = 1,
				  .NLOOP = 1,
				  .NREG = 1,
				  .REGS0 = GS_REGS_AD);
	tag++;
	{
		GIF_AD_DATA *zbuf = ( GIF_AD_DATA * )tag;
		zbuf->addr = SCE_GS_ZBUF_1;
		zbuf->data = SCE_GS_SET_ZBUF(ZBUFFER_PAGE()/2048,
									 Z_BUFFER_COLOR_MODE(), 1);
		tag++;
	}
  
	/* ??????????? */
	if( work->display_flag & ( RADAR_D_PLAYER | RADAR_D_MINE | RADAR_D_ENEMY ) ){
		tag = make_pos_display_tag( work, pos, tag );
	}

	/* ???? */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	tag->vif1 = SCE_VIF1_SET_FLUSHA( 0 );
	tag->vif2 = SCE_VIF1_SET_NOP( 0 );
	tag ++;

	/* ??????? */
	tag = ( DMA_TAG * )DG_PopDefaultDrawEnv( DG_Chanl( DG_CHANL_MENU ), ( u_int * )tag );
  
	/* ???? */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 );
	tag->vif1 = SCE_VIF1_SET_NOP( 0 );
	tag->vif2 = SCE_VIF1_SET_NOP( 0 );
	tag ++;
//printf( "RAD %d\n", (void *) tag - (void *)work->buffer[ DG_Clock ] );

	DG_CloseDmaPacketArea( tag );
#else
	{
		extern void RADAR_ActDrawRadar( void *addr );
		RADAR_ActDrawRadar( work );
	}
#endif


#if 0
	/*???????????*/
	if( (int *) tag > (int *)&work->buffer[ DG_Clock ][(RAD_BUF_MAX)-1] ){
		printf("START ADDR %X \n",work->buffer[ DG_Clock ]);
		printf("NOW ADDR %X \n",tag);
		printf("END ADDR %X \n",&work->buffer[ DG_Clock ][(RAD_BUF_MAX)-1]);
		printf("RADAR BUF OVER!!!!!!\n");
		ASSERT(0);
	}
#endif
}

static void Die( Work *work )
{
	printf("RADAR MONITOR DIE !!!!!!");

  /*?????*/
	SPR_Destroy_2D_Object( work->rad_spr.empty ) ;

	/*???????????*/
	SPR_Destroy_2D_Object( work->rad_spr.radio ) ;

	SPR_KillTexture(work->tri_id) ;

	DG_DequeueDmapack( work->dmapack );
	DG_FreeDmapack( work->dmapack );
	sys_work = NULL;

#if 1 //BP_XBOX ndef PSX2
	{
		extern void RADAR_EndRadar( void );
		RADAR_EndRadar();
	}
#endif
}

/* ---------------------------------------------------------------------- */
/*
	????
*/

/*SPR??*/
static int RadSprInit(Work *work )
{
	SPR_POS  b_pos;
	RAD_SPR *rad_spr ;

	rad_spr = &work->rad_spr ;

	/* ??????? Empty ????????? */
	rad_spr->empty 
		= SPR_Create_2D_Object(SP_EMPTY, DG_CHANL_MENU, NULL) ;
	if(rad_spr->empty == NULL ) return -1 ;

	/*???Empty????????????????
    ??????????????????*/
	rad_spr->mode = 
		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, rad_spr->empty);
	rad_spr->clearing = 
		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, rad_spr->mode);
	rad_spr->dot = 
		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, rad_spr->mode);
	rad_spr->use = 
		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, rad_spr->mode);
	rad_spr->noise = 
		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, rad_spr->empty);
	rad_spr->radio = 
		SPR_Create_2D_Object( SP_LINESTRIP, DG_CHANL_MENU, NULL) ;
	rad_spr->level = 
		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, rad_spr->use);
	rad_spr->frameT = 
		SPR_Create_2D_Object(SP_SPRITE,DG_CHANL_MENU,rad_spr->empty);
   rad_spr->frameL = 
      SPR_Create_2D_Object(SP_SPRITE,DG_CHANL_MENU,rad_spr->empty);
   rad_spr->frameR = 
      SPR_Create_2D_Object(SP_SPRITE,DG_CHANL_MENU,rad_spr->empty);
   rad_spr->frameB = 
      SPR_Create_2D_Object(SP_SPRITE,DG_CHANL_MENU,rad_spr->empty);
      //SPR_Create_2D_Object(SP_BOX,DG_CHANL_MENU,rad_spr->empty);
#ifdef XBOXBACK /*??????????vu??????????? 2002.04.10ynao*/
rad_spr->box = SPR_Create_2D_Object(SP_SPRITE,DG_CHANL_MENU,rad_spr->empty);
rad_spr->box->head.alpha = KASAN ;
rad_spr->box->head.flags |= SPR_FLAG_ALPHA;
//SET_COLOR_2DPRIM2(rad_spr->box,50,255,50,128);
SPR_SetColorSprite(rad_spr->box,10, 70, 30,0x70);
#endif
  
	SPR_SetLineStripVertexNumber(rad_spr->radio, 5) ; 

	/*????*/
	SetRadioCol( rad_spr->radio ,255,255,127,127) ;
//	SPR_SHOW(rad_spr->radio);
	SPR_HIDE(rad_spr->radio);

	SPR_SetPriority(rad_spr->radio , 7 ) ;
	rad_spr->radio->head.alpha = KASAN2 ;
  
	/* ?????????? */
	SPR_ObjSetTexture(rad_spr->mode, ALT_T1, work->tri_id);
	SPR_ObjSetTexture(rad_spr->clearing, CLE_T1, work->tri_id);
	SPR_ObjSetTexture(rad_spr->use, ALT_B1, work->tri_id);
	SPR_ObjSetTexture(rad_spr->noise, NoiseTexCode[0], work->tri_id);
  
	/*?????????????*/
	rad_spr->mode->head.alpha =	ALPHA_HALF ;
	rad_spr->mode->head.flags |= SPR_FLAG_ALPHA;

	rad_spr->clearing->head.alpha =	ALPHA_HALF ;
	rad_spr->clearing->head.flags |= SPR_FLAG_ALPHA;
  
	rad_spr->use->head.alpha =	ALPHA_HALF ;
	rad_spr->use->head.flags |= SPR_FLAG_ALPHA;
  
	rad_spr->noise->head.alpha = ALPHA_HALF ;
	rad_spr->noise->head.flags |= SPR_FLAG_ALPHA;
  
	rad_spr->frameT->head.alpha = UWAGAKI ;
   rad_spr->frameL->head.alpha = UWAGAKI ;
   rad_spr->frameR->head.alpha = UWAGAKI ;
   rad_spr->frameB->head.alpha = UWAGAKI ;
	SPR_SetColorSprite(rad_spr->frameT,0x00, 0x00, 0x00,0x70);
   SPR_SetColorSprite(rad_spr->frameL,0x00, 0x00, 0x00,0x70);
   SPR_SetColorSprite(rad_spr->frameR,0x00, 0x00, 0x00,0x70);
   SPR_SetColorSprite(rad_spr->frameB,0x00, 0x00, 0x00,0x70);
  
	rad_spr->level->head.alpha = KASAN ;
	rad_spr->level->head.flags |= SPR_FLAG_ALPHA;
  
	SET_COLOR_DEF(rad_spr->use);
	SET_COLOR_DEF(rad_spr->mode);
	SET_COLOR_DEF(rad_spr->clearing);
	SET_COLOR_DEF(rad_spr->noise);
    
	/*??????????????????????*/
	/*????*/
	rad_spr->sp_pos.x = RADAR_WINDOW_X0 * SC_RATE_W ;/*2002.04.08yano*/
	rad_spr->sp_pos.y = RADAR_OFFSET_Y * SC_RATE_H ;

	SPR_SetPosEmpty(rad_spr->empty, &rad_spr->sp_pos);
	/* ???????????? */
#if 0 //BP_PS2 def PSX2
	SPR_SetPosSprite(rad_spr->mode,&(SPR_POS){0.0F, 0.0F});
	SPR_SetPosSprite(rad_spr->clearing,&(SPR_POS){28.0F, (90.0F*SC_RATE_H)+(float)TOP_SIZE_H});
	SPR_SetPosSprite(rad_spr->dot,&(SPR_POS){8.0F, 8.0F});
	SPR_SetPosSprite(rad_spr->noise,&(SPR_POS){0.0F,(float)TOP_SIZE_H});
#else
	{
		SPR_POS dmdm0 = {0.0F, 0.0F},
		  		dmdm1 = {28.0F*SC_RATE_W, (90.0F*SC_RATE_H)+(float)TOP_SIZE_H},/*2002.04.08yano*/
		  		dmdm2 = {8.0F, 7.0F},
		  		dmdm3 = {0.0F,(float)TOP_SIZE_H};
		SPR_SetPosSprite(rad_spr->mode, &dmdm0 );
		SPR_SetPosSprite(rad_spr->clearing, &dmdm1 );
		SPR_SetPosSprite(rad_spr->dot, &dmdm2 );
		SPR_SetPosSprite(rad_spr->noise, &dmdm3 );
#ifdef XBOXBACK /*??????????vu??????????? 2002.04.10ynao*/
SPR_SetPosSprite(rad_spr->box, &dmdm3 );
SPR_SetPriority(rad_spr->box,7);
SPR_SHOW(rad_spr->box);
#endif
	}
#endif
	SetTexSizeSpr(rad_spr->mode) ;
   SPR_SetSizeSprite(rad_spr->mode, RADAR_WINDOW_W, TOP_SIZE_H+1);      //JG set the sprite width to be the radar size, not the texture width.
#if 0
	SetTexSizeSpr(rad_spr->clearing) ;
#else
//printf("CLEARING TEX SIZE!!!!\n");
//CLE_OFF
	SetTexSizeSpr(rad_spr->clearing) ;
	SPR_SetSizeSprite(rad_spr->clearing, CLE_SIZE_X, CLE_SIZE_Y);
#endif
	SPR_SetSizeSprite(rad_spr->dot, 4.0F, 5.0F);
	SetTexSizeSpr(rad_spr->noise) ;
   SPR_SetSizeSprite(rad_spr->noise, RADAR_WINDOW_W, RADAR_WINDOW_H);      //JG set the sprite width to be the radar size, not the texture width.
  
	b_pos.x = 0.0F;
	b_pos.y = work->height;
	SPR_SetPosSprite(rad_spr->use,&b_pos);
	SetTexSizeSpr(rad_spr->use) ;
   SPR_SetSizeSprite(rad_spr->use, RADAR_WINDOW_W, BOTOM_SIZE_H);      //JG set the sprite width to be the radar size, not the texture width.

	SPR_SetSizeSprite(rad_spr->level, RADAR_WINDOW_W,(float)TOP_SIZE_H);

  /*??????*/
  
	SPR_SetPriority(rad_spr->use,6);
	SPR_SetPriority(rad_spr->noise,5);
	SPR_SetPriority(rad_spr->mode,6);
	SPR_SetPriority(rad_spr->clearing,7);
	SPR_SetPriority(rad_spr->dot,7);
	SPR_SetPriority(rad_spr->level,7);
	SPR_SetPriority(rad_spr->frameT,7);
   SPR_SetPriority(rad_spr->frameL,7);
   SPR_SetPriority(rad_spr->frameR,7);
   SPR_SetPriority(rad_spr->frameB,7);

  /* 	???? */
	SPR_SHOW(rad_spr->empty);
	SPR_HIDE(rad_spr->mode);
//	SPR_HIDE(rad_spr->clearing);
	SPR_SHOW(rad_spr->clearing);

	SPR_SHOW(rad_spr->dot);
	SPR_SHOW(rad_spr->use);
	SPR_SHOW(rad_spr->level);

   SPR_SHOW(rad_spr->frameT);
   SPR_SHOW(rad_spr->frameL);
   SPR_SHOW(rad_spr->frameR);
   SPR_SHOW(rad_spr->frameB);

   SPR_SHOW(rad_spr->noise);
	set_default_color(work);
	ModeChange(work,GM_StartAlertMode) ;
	return 0;
}

static int RadParamInit(Work *work)
{
	work->count = 0 ;
	work->nomcnt = 0 ;
	work->alert = GM_StartAlertMode ;
	if( ( work->display_flag & RADAR_FRAME_DISPLAY ) == 0 ){
		work->height = 0.0F ;
	}else {
		work->height = (float)(RADAR_WINDOW_H-1) * SC_RATE_H ;
	}
	work->noise = 0 ;
	work->prev_flag = CheckRadarStatusFlag( work );
	return 0 ;
}

static void set_window( PACK_HEADER *packet )
{
#if 0 //BP_PS2 def PSX2
	RADAR_WINDOW_SET *wp;
	int x0, x1, y0, y1;
  
	wp = &( packet->window );
  
  // GIF??????
	DG_SET_GIFTAG(&wp->gif_tag,
				  .FLG = SCE_GIF_PACKED,
				  .NLOOP = 5,
				  .NREG = 1,
				  .REGS0 = GS_REGS_AD);
  
	// ?????????????
	wp->alpha.addr = SCE_GS_ALPHA_1;
	wp->alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 );
  
	wp->zbuf.addr = SCE_GS_ZBUF_1;
	wp->zbuf.data = SCE_GS_SET_ZBUF(ZBUFFER_PAGE()/2048,
									Z_BUFFER_COLOR_MODE(), 0);
  
	wp->test.addr = SCE_GS_TEST_1;
	wp->test.data = SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 );

	// ???????????
	x0 = RADAR_WINDOW_X0;
	y0 = RADAR_WINDOW_Y0;
	x1 = RADAR_WINDOW_X1;
	y1 = RADAR_WINDOW_Y1;

	wp->scissor.addr = SCE_GS_SCISSOR_1;
	wp->scissor.data = SCE_GS_SET_SCISSOR( x0, x1, y0, y1 );

  // ??????????
	wp->offset.addr = SCE_GS_XYOFFSET_1;
	wp->offset.data = SCE_GS_SET_XYOFFSET((2048 - (x0 + x1) / 2) << 4,
										  (2048 - (y0 + y1) / 2) << 4);
#endif
}

static void set_drawparam( PACK_HEADER *packet )
{
#if 0 //BP_PS2 def PSX2
	RADAR_DRAWPARAM_SET *pp;

	pp = &( packet->drparam );

	// GIF??????
	DG_SET_GIFTAG( &pp->gif_tag,
				   .FLG = SCE_GIF_PACKED,
				   .NLOOP = 1,
				   .NREG = 1,
				   .REGS0 = GS_REGS_AD);

	pp->test.addr = SCE_GS_TEST_1;
	pp->test.data = SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 2 );
#endif
}

static void set_base_tile( PACK_HEADER *packet )
{
#if 0 //BP_PS2 def PSX2
	RADAR_TILE_SET *tp;
	int x0, x1, y0, y1;

	tp = &( packet->base_tile );

  // GIFTAG????
	DG_SET_GIFTAG(&tp->gif_tag,
				  .FLG = SCE_GIF_REGLIST,
				  .NLOOP = 1,
				  .NREG = 4,
				  .REGS0 = GS_REGS_PRIM,
				  GS_REGS_RGBA,
				  GS_REGS_XYZF2,
				  GS_REGS_XYZF2 );

	DG_SET_GS_REG( &tp->prim,
				   .PRIM = SCE_GS_PRIM_SPRITE,
				   .ABE = 1,
				   .FST = 1 );

	//	tp->rgba = SCE_GS_SET_RGBAQ( 0, 0, 0, 0x80/8 * 4, 0 );
	tp->rgba = SCE_GS_SET_RGBAQ( 0, 28, 28, 0x80/8 * 4, 0 );

	// ???????????
	x0 = RADAR_WINDOW_X0;
	y0 = RADAR_WINDOW_Y0;
	x1 = RADAR_WINDOW_X1;
	y1 = RADAR_WINDOW_Y1;

	tp->xy1 = SCE_GS_SET_XYZ((2048 - RADAR_WINDOW_W / 2) << 4,
							 (2048 - RADAR_WINDOW_H / 2) << 4, 50);

	tp->xy2 = SCE_GS_SET_XYZ((2048 + RADAR_WINDOW_W / 2  + 1) << 4,
							 (2048 + RADAR_WINDOW_H / 2  + 1) << 4, 50 );
#endif
}

static void copy_chara_color( LCOLOR *dest, unsigned char table[ 4 ][ 4 ] )
{
	int i, j;
	int * d;

	for(j = 0; j < 4; j++){
		unsigned char *s;

		d = &(dest->color[ 0 ]);
		s = table[ j ];
		for(i = 0; i < 4; i++) *( d ++ ) = *( s ++ );
		dest++;
    }
}

static void set_vu1_param( Work *work )
{
#if 0 //BP_PS2 def PSX2
	VU1_PARAM *param;
	
	param = &( work->param );
	DG_COPY_MAT( param->matrix, &default_matrix	);
  
#if 0 //BP_PS2 def PSX2
	param->yclip = ( FVECTOR ){0.0F, 0.0F, 0.0F, RADAR_ZOOM_RATE_Y };
#else
	{
		FVECTOR dmdm = {0.0F, 0.0F, 0.0F, RADAR_ZOOM_RATE_Y };
		param->yclip = dmdm;
	}
#endif
  
	param->bright_z = 200;
	param->dark_z = 100;

	copy_color( &param->bright_color, default_colors[ 0 ] );
	copy_color( &param->dark_color, default_colors[ 1 ] );
	copy_color( &param->door_color, default_colors[ 2 ] );

	DG_SET_GIFTAG(&param->giftag,
				  .FLG = SCE_GIF_PACKED,
				  .NLOOP = 3,
				  .PRE = 1,
				  .PRIM = SCE_GS_SET_PRIM(SCE_GS_PRIM_LINE, 0,0,0,0,0,0,0,0),
				  .EOP = 1,
				  .NREG = 3,
				  .REGS0 = GS_REGS_RGBA,
				  GS_REGS_XYZ2,
				  GS_REGS_XYZ2 );
#if 0
	DG_SET_GIFTAG(&param->pos_giftag,
				  .FLG = SCE_GIF_PACKED,
				  .NLOOP = 1,
				  .PRE = 1,
				  .PRIM = SCE_GS_SET_PRIM( SCE_GS_PRIM_POINT, 0,0,0,0,0,0,0,0 ),
				  .EOP = 0,
				  .NREG = 2,
				  .REGS0 = GS_REGS_RGBA,
				  GS_REGS_XYZ2 );
#endif
	DG_SET_GIFTAG(&param->sight_giftag,
				  .FLG = SCE_GIF_PACKED,
				  .PRE = 1,
				  .PRIM = SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1,0,0,1,0,0,0,0 ),
				  .NREG = 8,
				  .NLOOP = 1,
				  .EOP = 1,
				  .REGS0 = GS_REGS_RGBA,
				  GS_REGS_XYZ2,
				  GS_REGS_RGBA,
				  GS_REGS_XYZ2,
				  GS_REGS_XYZ2,
				  GS_REGS_XYZ2,
				  GS_REGS_XYZ2,
				  GS_REGS_XYZ2);

	DG_SET_GIFTAG(&param->no_sight_giftag,
				  .FLG = SCE_GIF_PACKED,
				  .PRE = 0,
				  .NREG = 1,
				  .NLOOP = 0,
				  .EOP = 1,
				  .REGS0 = GS_REGS_NOP);

	copy_chara_color((LCOLOR *)&param->chara_color[0], default_chara_colors[0]);
	copy_chara_color((LCOLOR *)&param->chara_color[1], default_chara_colors[1]);
	copy_chara_color((LCOLOR *)&param->chara_color[2], default_chara_colors[2]);
	copy_chara_color((LCOLOR *)&param->chara_color[3], default_chara_colors[3]);

	/* ????????????? */
	{
		int i;
		FVECTOR *fv;
		fv = param->rot_table;
		for(i = 0; i < MAX_ROT_TABLE; i++){
			// fv->vx = sinf( i * 2 * PI / MAX_ROT_TABLE ) * RADAR_ZOOM_RATE_W;
			// fv->vy = cosf( i * 2 * PI / MAX_ROT_TABLE ) * RADAR_ZOOM_RATE_H;
			fv->vx = sinf(i * 2 * PI / MAX_ROT_TABLE) * RADAR_ZOOM_RATE_W;
			fv->vy = cosf(i * 2 * PI / MAX_ROT_TABLE) * RADAR_ZOOM_RATE_W;

			*(int *)&fv->vz = 255;
			*(int *)&fv->vw = 0;
			fv++;
		}
	}
#endif
}

static void InitPacket( PACK_HEADER *packet )
{
#if 0 //BP_PS2 def PSX2
	DG_DMATAG *dt;
  
  // DMA??????
	dt = &( packet->dma_tag );
	dt->addr = NULL;
	dt->qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 );   // size = 0( dummy )
	dt->vifcode[ 0 ] = SCE_VIF1_SET_NOP( 0 );
	dt->vifcode[ 1 ] = SCE_VIF1_SET_DIRECT( 0, 0 ); // size = 0( dummy )
  
	// ???????
	set_window( packet );
	set_drawparam( packet );
  
  // ?????????
	set_base_tile( packet );
  
	// ????????
	/* ? */
	{
		int size;
		sceGifTag *end_tag;
    
		size = (sizeof(PACK_HEADER) - sizeof(DG_DMATAG)) / sizeof(u_long128);
    
		dt->qwc = DMATAG_SET_QWC(DMATAG_ID_RET, size);
		dt->vifcode[1] = SCE_VIF1_SET_DIRECT(size, 0);
    
		end_tag = &(packet->end_tag);
		DG_SET_GIFTAG(end_tag,
					  .EOP = 1,
					  .NLOOP = 1,
					  .NREG = 1,
					  .REGS0 = GS_REGS_AD);

		packet->alpha.addr = SCE_GS_ALPHA_1;
		//		packet->alpha.data = SCE_GS_SET_ALPHA( 0,2,2,1,0x80 );
		/*TEST*/
		packet->alpha.data = KASAN ;
	}
#endif
}

static int GetResources( Work *work, int name )
{
#ifdef DEBUG_MODE
	work->debug = 0 ;
  //	GM_Configuration |= GM_CONFIG_RADAR_OFF ;
  //	GM_Configuration |= GM_CONFIG_RADAR_OFF_INTRUDE;
#endif
	work->tri_id = SPR_LoadTexture(RADAR_TRI);
  
  /*????*/
	work->base_rot = (u_short)GCL_GetOptionValue( 'd', 0 ) ;
	work->base_rot &= 4095 ;
	if(work->base_rot >2048 ){
		work->base_rot = -2048 +(work->base_rot-2048) ;
	}

  
	work->status = RAD_DEFAULT ;

	work->zoom = 1.0f ;
	work->zoom_rate.vx = RADAR_ZOOM_RATE_W ;
	work->zoom_rate.vy = 0.0F ;
	work->zoom_rate.vz = RADAR_ZOOM_RATE_H ;
	work->zoom_rate.vw = 0.0F ;

	/*???????*/
	InitDefaultMatrix(work) ;
	/*??????????*/
	InitDefaultMatrix2( work, GM_RadarGetCenter() );

	{
		DG_DMAPACK *dmapack;
		int i;
    
		dmapack = DG_MakeDmapack( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_AFTER );
		DG_QueueDmapack( dmapack );
		work->dmapack = dmapack;
    
#if 0 //BP_PS2 def PSX2
		/* VU1?????????????? */
		set_vu1_param( work );
    
		for( i = 0; i < 2; i ++ ){
			dmapack->packet[ i ] = &work->head;
		}
		InitPacket( &work->head );
#else
		{/* XBOX??????????? */
			extern void RADAR_InitRadar( void );
			extern void *RADAR_BufferRadarCallback( void *, DG_DMAPACK * );
         extern void RADAR_RenderRadarCallback( void * );
			DG_COPY_MAT( work->param.matrix, &default_matrix	);
			RADAR_InitRadar();
         DG_AS_DMAPackSetCallbacks(dmapack, RADAR_RenderRadarCallback, RADAR_BufferRadarCallback);
         dmapack->BP_callbackParam = work;
		}
#endif
	}
  
	/* ?????????? */
  
	work->display_flag = CheckRadarPartDisplayFlag( work );
	work->name = name;

  /*SprInit???????*/
	if( RadParamInit( work ) < 0) return -1 ;
	/*???? ?????*/
	if( RadSprInit( work ) < 0) return -1 ;

	/* ??????? or ?? Actor ?? */
	{
		void * vpt = NewRadarBomb(&(work->base_rot),
								  work->rad_spr.empty,
								  work->rad_spr.mode->sprite.dh + 1.0F);
		if(vpt != NULL) GV_SetActorChild(work, vpt);
	}
	/* ?????????? Actor ?? */
	{
		void * vpt = NewRadarOutRangeTarget(&(work->base_rot),
											&work->zoom_rate,
											work->rad_spr.empty,
											work->rad_spr.mode->sprite.dh + 1.0F);
    
		if(vpt != NULL) GV_SetActorChild(work, vpt);
	}					

	sys_work = work;

	return 0;
}

void *NewRadar( int name, int map )
{
	/* ???????? */

	Work		*work;

	work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
									 sizeof( Work ), GM_RADAR_ACTOR_PRIO ) ;
#ifdef DEBUG_MODE
	GM_AddDebugMenu( &Radar_debugmenu );
#endif
  
	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX(&(work->actor));
		if( GetResources( work, name ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}

	}
	return work ;
}

/* ---------------------------------------------------------------------- */
/*
	?????????
*/

int NewRadarZoomRateControl( void )
{
	FMATRIX mat;
	float rate;
	Work *work = sys_work;
	VU1_PARAM *param = &work->param;

	work->zoom = rate = ( GCL_GetNextInt() / 1000.0F );

	mat = DG_UnitMatrix;
	mat.m[ 0 ][ 0 ] *= rate;
	mat.m[ 2 ][ 2 ] *= rate;
	
	work->zoom_rate.vx = RADAR_ZOOM_RATE_W * rate;
	work->zoom_rate.vz = RADAR_ZOOM_RATE_H * rate;

	_sceVu0MulMatrix( &param->matrix, &default_matrix, &mat );

	/* ??????????????? */
	InitDefaultMatrix2( work, GM_RadarGetCenter() );
	/* VU1???????????? */
	set_vu1_param( work );

	/* ????????????? */
	{
		int i;
		FVECTOR *fv;
		fv = param->rot_table;
		for(i = 0; i < MAX_ROT_TABLE; i++){
			fv->vx = sinf(i * 2 * PI / MAX_ROT_TABLE) * RADAR_ZOOM_RATE_W * rate;
			fv->vy = cosf(i * 2 * PI / MAX_ROT_TABLE) * RADAR_ZOOM_RATE_W * rate;

			*(int *)&fv->vz = 255;
			*(int *)&fv->vw = 0;
			fv++;
		}
	}
	return 0;
}

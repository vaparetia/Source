//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   gaged.c
   ゲージ管理
   
   1999/12/01 M.Sonoyama
   $Id: gaged.c,v 1.2 2002/12/05 18:42:02 takaki Exp $
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include "libfs.h"

/*------------------------------------------------------------------*/

#define	GAGE_DAEMON_PRIO	PLAYER_GAGE_DAEMON_PRIO

int					GM_GageStatus = 0 ;
int					GM_N_ActiveGages = 0 ;
static	int			GM_N_Gages = 0 ;
static	GM_GageSet	*GM_GageList ;
static	GM_GageSet	MenuLifeGage ;

#define	SCROLL_TIME	(20)
#define	SCROLL_VY	(4)
#define	DEFAULT_START_Y	(16)
#define	TXT_ADJ_X	(4)
#define	TXT_ADJ_Y	(1)
#define	TXT_WIDTH	(9)
#define	TXT_HEIGHT	(11)
#define	TXT_NUM		(5)

static	int		StartY ;
static	int		ScrollCount = 0 ;
static	int		PreStatus ;
static	int		Height = 1 ;

/*------------------------------------------------------------------*/

#define	PRIM_FLAG	(DG_PRIM_POLY_G4 | DG_PRIM_SORTONLY)
#define	N_PRIMS		(8)

enum {
	GAGE_PRIM_BASE	= 0,
	GAGE_PRIM_LIFE,
	GAGE_PRIM_LIFE_DELAY,
	GAGE_PRIM_TEXT_BACK,
	GAGE_PRIM_BASE_EDGE1,
	GAGE_PRIM_BASE_EDGE2,
	GAGE_PRIM_BASE_LINE1,
	GAGE_PRIM_BASE_LINE2,

	GAGE_PRIM_BASE_LINE3,
	GAGE_PRIM_M9,
	GAGE_PRIM_M9_DELAY,
} ;

/*------------------------------------------------------------------*/

typedef	struct {
    GV_ACT		actor ;
} Work ;

/*------------------------------------------------------------------*/


/* プリミティブ初期化 */
static	void	InitPrim( gs )
GM_GageSet	*gs ;
{
    DG_MENU2_PRIM	*prim ;
    DG_MENU2_PACKET	*packs ;
    DG_MENU2_POLY_G	*poly ;
    int			i, j, m9exist, n_prims ;

	n_prims = N_PRIMS ;
	m9exist = ( gs->flag & GM_GAGE_M9EXIST ) ? 1 : 0 ;
	if ( m9exist ) n_prims += 3 ;

    prim = gs->prim = DG_MakeMenu2Prim( DG_MENU2_PRIM_POLY | DG_PRIM2_SHADE | DG_PRIM2_ALPHA,
									    n_prims, 0 ) ;
    if ( prim == NULL ) return ;
    for ( i = 0; i < 2; i ++ ) {
		packs = ( DG_MENU2_PACKET * )prim->packet[ i ] ;
		packs->alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ;
		poly = ( DG_MENU2_POLY_G * )packs->data ;
		for ( j = 0; j < n_prims; j ++ ) {
			switch ( j ) {
			case GAGE_PRIM_BASE :			/* ベース */
				poly->prim.ABE = 1 ;
				poly->prim.IIP = 0 ;
				DG_MENU2_SET_RGBA3( poly, gs->r[ 0 ], gs->g[ 0 ], gs->b[ 0 ], 64 ) ;
				DG_MENU2_SET_RGBA4( poly, gs->r[ 0 ], gs->g[ 0 ], gs->b[ 0 ], 64 ) ;
				break ;
			case GAGE_PRIM_LIFE :			/* 本体 */
				poly->prim.ABE = 0 ;
				DG_MENU2_SET_RGBA1( poly, gs->r[ 1 ], gs->g[ 1 ], gs->b[ 1 ], 128 ) ;
				DG_MENU2_SET_RGBA2( poly, gs->r[ 2 ], gs->g[ 2 ], gs->b[ 2 ], 128 ) ;
				DG_MENU2_SET_RGBA3( poly, gs->r[ 1 ], gs->g[ 1 ], gs->b[ 1 ], 128 ) ;
				DG_MENU2_SET_RGBA4( poly, gs->r[ 2 ], gs->g[ 2 ], gs->b[ 2 ], 128 ) ;
				break ;
			case GAGE_PRIM_LIFE_DELAY :			/* 減衰 */
				poly->prim.ABE = 0 ;
				poly->prim.IIP = 0 ;
            DG_MENU2_SET_RGBA1( poly, gs->r[ 3 ], gs->g[ 3 ], gs->b[ 3 ], 128 ) ;
            DG_MENU2_SET_RGBA2( poly, gs->r[ 3 ], gs->g[ 3 ], gs->b[ 3 ], 128 ) ;
            DG_MENU2_SET_RGBA3( poly, gs->r[ 3 ], gs->g[ 3 ], gs->b[ 3 ], 128 ) ;
            DG_MENU2_SET_RGBA4( poly, gs->r[ 3 ], gs->g[ 3 ], gs->b[ 3 ], 128 ) ;

            break ;
			case GAGE_PRIM_TEXT_BACK :			/* テキスト背景 */
				poly->prim.ABE = 1 ;
				poly->prim.IIP = 0 ;
				DG_MENU2_SET_RGBA3( poly, 0, 0, 0, 96 ) ;
				DG_MENU2_SET_RGBA4( poly, 0, 0, 0, 96 ) ;
				break ;
			case GAGE_PRIM_BASE_EDGE1 :
			case GAGE_PRIM_BASE_EDGE2 :
			case GAGE_PRIM_BASE_LINE1 :
			case GAGE_PRIM_BASE_LINE2 :
			case GAGE_PRIM_BASE_LINE3 :
				poly->prim.ABE = 0 ;
				poly->prim.IIP = 0 ;
				DG_MENU2_SET_RGBA3( poly, 0, 0, 0, 128 ) ;
				DG_MENU2_SET_RGBA4( poly, 0, 0, 0, 128 ) ;
				break ;
			case GAGE_PRIM_M9 :
				poly->prim.ABE = 0 ;
				poly->prim.IIP = 1 ;
				DG_MENU2_SET_RGBA1( poly, gs->m9col_left[ 0 ], gs->m9col_left[ 1 ], 
								    gs->m9col_left[ 2 ], 128 ) ;
				DG_MENU2_SET_RGBA2( poly, gs->m9col_right[ 0 ], gs->m9col_right[ 1 ], 
								    gs->m9col_right[ 2 ], 128 ) ;
				DG_MENU2_SET_RGBA3( poly, gs->m9col_left[ 0 ], gs->m9col_left[ 1 ], 
								    gs->m9col_left[ 2 ], 128 ) ;
				DG_MENU2_SET_RGBA4( poly, gs->m9col_right[ 0 ], gs->m9col_right[ 1 ], 
								    gs->m9col_right[ 2 ], 128 ) ;
				break ;
			case GAGE_PRIM_M9_DELAY :
				poly->prim.ABE = 0 ;
				DG_MENU2_SET_RGBA3( poly, gs->m9delay_col_left[ 0 ], gs->m9delay_col_left[ 1 ], 
								    gs->m9delay_col_left[ 2 ], 128 ) ;
				DG_MENU2_SET_RGBA4( poly, gs->m9delay_col_right[ 0 ], gs->m9delay_col_right[ 1 ], 
								    gs->m9delay_col_right[ 2 ], 128 ) ;
			}
			DG_MENU2_SET_XY1( poly, 0, 0 ) ;
			DG_MENU2_SET_XY2( poly, 0, 0 ) ;
			DG_MENU2_SET_XY3( poly, 0, 0 ) ;
			DG_MENU2_SET_XY4( poly, 0, 0 ) ;
			poly ++ ;
		}
    }
}

/* ゲージ更新 */
static	void	UpdateGage( gs, which )
GM_GageSet	*gs ;
int			which ;
{
    DG_MENU2_PRIM	*prim ;
    DG_MENU2_PACKET	*packs ;
    DG_MENU2_POLY_G	*poly, *polyL ;
    int			y, r, g, b ;
    int			sx, p, v, w, d, gw, vw, dw ;
    int			count, m9exist, m9adj ;

//    sx = TXT_NUM * TXT_WIDTH ;
	sx = 4 ;

	m9exist = ( gs->flag & GM_GAGE_M9EXIST ) ? 1 : 0 ;

    y = StartY ;
    if ( gs->prim == NULL ) InitPrim( gs ) ;
    prim = gs->prim ;
    if ( prim == NULL ) return ;

    if ( !( gs->flag & GM_GAGE_INVISIBLE ) && 
		( GM_GageStatus != GM_GAGE_STATE_INVISIBLE || gs->level == -1 ) &&
//		( !GM_CheckMenuStatus( MENU_GAGE_OFF ) || gs->level == -1 ) ) {
		( !GM_CheckMenuStatus( MENU_GAGE_OFF ) ) ) {
	    m9adj = ( m9exist ) ? 7 : 0 ;		
       if ( BP_Area_EU() )
       {
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
		   MENU_S_Locate( DIRECT_SCREEN_X( gs->x + 2 ),
					     DIRECT_SCREEN_Y( y + gs->h / 2 - TXT_HEIGHT * Height / 2 + 9 + m9adj ) + 1, MENU_MODE_LEFT ) ;
//#else
       }
       else
       {
		   MENU_S_Locate( DIRECT_SCREEN_X( gs->x + 2 ),
					     DIRECT_SCREEN_Y( y + gs->h / 2 - TXT_HEIGHT * Height / 2 + 9 + m9adj ), MENU_MODE_LEFT ) ;
       }
//#endif
		MENU_S_Color( 196, 196, 196, 128 ) ;
		MENU_S_Printf( "%s\n", gs->name ) ;
    }

//    w = gs->w - sx ;
    w = gs->w - sx - 2 ;
    gw = gs->max - gs->min ;
	ASSERT( gw > 0 ) ;

    v = gs->value ;
    p = gs->prev ;
    if ( v > gs->max ) v = gs->max ;
    else if ( v < gs->min ) v = gs->min ;

    if ( p > v ) {
		gs->count = gs->delay ;
		gs->dec = p ;
    } 
    if ( gs->dec < v ) {
		gs->count = 0 ;
		gs->dec = v ;
    }
    d = gs->dec ;
    gs->prev = v ;
    vw = v * w / gw ;

    packs = ( DG_MENU2_PACKET * )prim->packet[ which ] ;
    poly = ( DG_MENU2_POLY_G * )packs->data ;

	/* ベースエッジＡＮＤライン */
	m9adj = ( m9exist ) ? 4 : 0 ;

	DG_MENU2_SET_XY1( poly + GAGE_PRIM_BASE_EDGE1, DIRECT_SCREEN_X( gs->x ), DIRECT_SCREEN_Y( y ) ) ;
	DG_MENU2_SET_XY2( poly + GAGE_PRIM_BASE_EDGE1, DIRECT_SCREEN_X( gs->x + 4 ), DIRECT_SCREEN_Y( y ) ) ;
	DG_MENU2_SET_XY3( poly + GAGE_PRIM_BASE_EDGE1, DIRECT_SCREEN_X( gs->x ), DIRECT_SCREEN_Y( y + gs->h + m9adj ) ) ;
	DG_MENU2_SET_XY4( poly + GAGE_PRIM_BASE_EDGE1, DIRECT_SCREEN_X( gs->x + 4 ), DIRECT_SCREEN_Y( y + gs->h + m9adj ) ) ;

	DG_MENU2_SET_XY1( poly + GAGE_PRIM_BASE_EDGE2, DIRECT_SCREEN_X( gs->x + gs->w - 0 ), DIRECT_SCREEN_Y( y ) ) ;
	DG_MENU2_SET_XY2( poly + GAGE_PRIM_BASE_EDGE2, DIRECT_SCREEN_X( gs->x + gs->w ), DIRECT_SCREEN_Y( y ) ) ;
	DG_MENU2_SET_XY3( poly + GAGE_PRIM_BASE_EDGE2, DIRECT_SCREEN_X( gs->x + gs->w - 0  ), DIRECT_SCREEN_Y( y + gs->h + m9adj ) ) ;
	DG_MENU2_SET_XY4( poly + GAGE_PRIM_BASE_EDGE2, DIRECT_SCREEN_X( gs->x + gs->w ), DIRECT_SCREEN_Y( y + gs->h + m9adj ) ) ;

	DG_MENU2_SET_XY1( poly + GAGE_PRIM_BASE_LINE1, DIRECT_SCREEN_X( gs->x ), DIRECT_SCREEN_Y( y ) ) ;
	DG_MENU2_SET_XY2( poly + GAGE_PRIM_BASE_LINE1, DIRECT_SCREEN_X( gs->x + gs->w + 1 ), DIRECT_SCREEN_Y( y ) ) ;
	DG_MENU2_SET_XY3( poly + GAGE_PRIM_BASE_LINE1, DIRECT_SCREEN_X( gs->x ), DIRECT_SCREEN_Y( y ) ) ;
	DG_MENU2_SET_XY4( poly + GAGE_PRIM_BASE_LINE1, DIRECT_SCREEN_X( gs->x + gs->w + 1 ), DIRECT_SCREEN_Y( y ) ) ;

	DG_MENU2_SET_XY1( poly + GAGE_PRIM_BASE_LINE2, DIRECT_SCREEN_X( gs->x ), DIRECT_SCREEN_Y( y + gs->h + 1 ) ) ;
	DG_MENU2_SET_XY2( poly + GAGE_PRIM_BASE_LINE2, DIRECT_SCREEN_X( gs->x + gs->w + 1 ), DIRECT_SCREEN_Y( y + gs->h + 1 ) ) ;
	DG_MENU2_SET_XY3( poly + GAGE_PRIM_BASE_LINE2, DIRECT_SCREEN_X( gs->x ), DIRECT_SCREEN_Y( y + gs->h + 1 ) ) ;
	DG_MENU2_SET_XY4( poly + GAGE_PRIM_BASE_LINE2, DIRECT_SCREEN_X( gs->x + gs->w + 1 ), DIRECT_SCREEN_Y( y + gs->h + 1 ) ) ;

	/* ベース */
	m9adj = ( m9exist ) ? 4 + 2 : 0 ;
	DG_MENU2_SET_XY1( poly, DIRECT_SCREEN_X( gs->x ), DIRECT_SCREEN_Y( y ) ) ;
	DG_MENU2_SET_XY2( poly, DIRECT_SCREEN_X( gs->x + gs->w + 1 ), DIRECT_SCREEN_Y( y ) ) ;
	DG_MENU2_SET_XY3( poly, DIRECT_SCREEN_X( gs->x ), DIRECT_SCREEN_Y( y + gs->h + m9adj ) ) ;
	DG_MENU2_SET_XY4( poly, DIRECT_SCREEN_X( gs->x + gs->w + 1 ), DIRECT_SCREEN_Y( y + gs->h + m9adj ) ) ;
	poly ++ ;

	/* 本体 */
	if ( gs->flag & GM_GAGE_WARNING ) {
		DG_MENU2_SET_RGBA1( poly, 232, 132, 32, 96 ) ;
		DG_MENU2_SET_RGBA2( poly, 232, 132, 32, 96 ) ;
		DG_MENU2_SET_RGBA3( poly, 232, 132, 32, 96 ) ;
		DG_MENU2_SET_RGBA4( poly, 232, 132, 32, 96 ) ;
//		gs->flag &= ~GM_GAGE_WARNING ;
	} else {
		r = gs->r[ 1 ] + ( gs->r[ 2 ] - gs->r[ 1 ] ) * v / gw ;
		g = gs->g[ 1 ] + ( gs->g[ 2 ] - gs->g[ 1 ] ) * v / gw ;
		b = gs->b[ 1 ] + ( gs->b[ 2 ] - gs->b[ 1 ] ) * v / gw ;
		DG_MENU2_SET_RGBA1( poly, gs->r[ 1 ], gs->g[ 1 ], gs->b[ 1 ], 128 ) ;
		DG_MENU2_SET_RGBA3( poly, gs->r[ 1 ], gs->g[ 1 ], gs->b[ 1 ], 128 ) ;
		DG_MENU2_SET_RGBA2( poly, r, g, b, 96 ) ;
		DG_MENU2_SET_RGBA4( poly, r, g, b, 96 ) ;
	}

	DG_MENU2_SET_XY1( poly, DIRECT_SCREEN_X( gs->x + sx ), DIRECT_SCREEN_Y( y + 1 ) ) ;		
	DG_MENU2_SET_XY2( poly, DIRECT_SCREEN_X( gs->x + sx + vw ), DIRECT_SCREEN_Y( y + 1 ) ) ;		
	DG_MENU2_SET_XY3( poly, DIRECT_SCREEN_X( gs->x + sx ), DIRECT_SCREEN_Y( y + 1 + gs->h - 2 ) ) ;		
	DG_MENU2_SET_XY4( poly, DIRECT_SCREEN_X( gs->x + sx + vw ), DIRECT_SCREEN_Y( y + 1 + gs->h - 2 ) ) ;
	poly ++ ;
	/* 減衰 */
	dw = d * w / gw ;
	DG_MENU2_SET_XY1( poly, DIRECT_SCREEN_X( gs->x + sx + vw ), DIRECT_SCREEN_Y( y + 1 ) ) ;
	DG_MENU2_SET_XY2( poly, DIRECT_SCREEN_X( gs->x + sx + dw ), DIRECT_SCREEN_Y( y + 1 ) ) ;
	DG_MENU2_SET_XY3( poly, DIRECT_SCREEN_X( gs->x + sx + vw ), DIRECT_SCREEN_Y( y + 1 + gs->h - 2 ) ) ;
	DG_MENU2_SET_XY4( poly, DIRECT_SCREEN_X( gs->x + sx + dw ), DIRECT_SCREEN_Y( y + 1 + gs->h - 2 ) ) ;
	poly ++ ;

	/* テキスト背景 */
	m9adj = ( m9exist ) ? 7 : 0 ;
	DG_MENU2_SET_XY1( poly, DIRECT_SCREEN_X( gs->x + 0 ), DIRECT_SCREEN_Y( y + 4 + m9adj ) ) ;
	DG_MENU2_SET_XY2( poly, DIRECT_SCREEN_X( gs->x + 0 + gs->text_len ), DIRECT_SCREEN_Y( y + 4 + m9adj ) ) ;
	DG_MENU2_SET_XY3( poly, DIRECT_SCREEN_X( gs->x + 0 ), DIRECT_SCREEN_Y( y + 4 + TXT_HEIGHT + m9adj ) ) ;
	DG_MENU2_SET_XY4( poly, DIRECT_SCREEN_X( gs->x + 0 + gs->text_len ), DIRECT_SCREEN_Y( y + 4 + TXT_HEIGHT + m9adj ) ) ;

	count = gs->count ;
	if ( gs->count > 0 ) gs->count -- ;
	if ( count == 0 ) {
		gs->dec = v ;
	} else {
		gs->dec -= ( d - v ) / count ;
	}

	/* Ｍ９拡張 */
	if ( m9exist == 0 ) return ;

    poly = ( DG_MENU2_POLY_G * )packs->data ;

	/* ライン */
	polyL = poly + GAGE_PRIM_BASE_LINE3 ;
	DG_MENU2_SET_XY1( polyL, DIRECT_SCREEN_X( gs->x ), DIRECT_SCREEN_Y( y + gs->h + 4 ) ) ;
	DG_MENU2_SET_XY2( polyL, DIRECT_SCREEN_X( gs->x + gs->w + 1 ), DIRECT_SCREEN_Y( y + gs->h + 4 ) ) ;
	DG_MENU2_SET_XY3( polyL, DIRECT_SCREEN_X( gs->x ), DIRECT_SCREEN_Y( y + gs->h + 4 ) ) ;
	DG_MENU2_SET_XY4( polyL, DIRECT_SCREEN_X( gs->x + gs->w + 1 ), DIRECT_SCREEN_Y( y + gs->h + 4 ) ) ;

    gw = gs->m9_max - gs->m9_min ;
	ASSERT( gw > 0 ) ;

    v = gs->m9_value ;
    p = gs->m9_prev ;
    if ( v > gs->m9_max ) v = gs->m9_max ;
    else if ( v < gs->m9_min ) v = gs->m9_min ;

    if ( p > v ) {
		gs->m9_count = gs->m9_delay ;
		gs->m9_dec = p ;
    } 
    if ( gs->m9_dec < v ) {
		gs->m9_count = 0 ;
		gs->m9_dec = v ;
    }
    d = gs->m9_dec ;
    gs->m9_prev = v ;
    vw = v * w / gw ;
	dw = d * w / gw ;

	/* 本体 */
	polyL = poly + GAGE_PRIM_M9 ;
	r = gs->m9col_left[ 0 ] + ( gs->m9col_right[ 0 ] - gs->m9col_left[ 0 ] ) * v / gw ;
	g = gs->m9col_left[ 1 ] + ( gs->m9col_right[ 1 ] - gs->m9col_left[ 1 ] ) * v / gw ;
	b = gs->m9col_left[ 2 ] + ( gs->m9col_right[ 2 ] - gs->m9col_left[ 2 ] ) * v / gw ;
	DG_MENU2_SET_RGBA1( polyL, gs->m9col_left[ 0 ], gs->m9col_left[ 1 ], gs->m9col_left[ 2 ], 128 ) ;
	DG_MENU2_SET_RGBA3( polyL, gs->m9col_left[ 0 ], gs->m9col_left[ 1 ], gs->m9col_left[ 2 ], 128 ) ;
	DG_MENU2_SET_RGBA2( polyL, r, g, b, 128 ) ;
	DG_MENU2_SET_RGBA4( polyL, r, g, b, 128 ) ;

	DG_MENU2_SET_XY1( polyL, DIRECT_SCREEN_X( gs->x + sx ), DIRECT_SCREEN_Y( y + gs->h ) ) ;		
	DG_MENU2_SET_XY2( polyL, DIRECT_SCREEN_X( gs->x + sx + vw ), DIRECT_SCREEN_Y( y + gs->h ) ) ;		
	DG_MENU2_SET_XY3( polyL, DIRECT_SCREEN_X( gs->x + sx ), DIRECT_SCREEN_Y( y + gs->h + 4 ) ) ;		
	DG_MENU2_SET_XY4( polyL, DIRECT_SCREEN_X( gs->x + sx + vw ), DIRECT_SCREEN_Y( y + gs->h + 4 ) ) ;

	/* 減衰 */
	polyL = poly + GAGE_PRIM_M9_DELAY ;
	DG_MENU2_SET_XY1( polyL, DIRECT_SCREEN_X( gs->x + sx + vw ), DIRECT_SCREEN_Y( y + gs->h ) ) ;
	DG_MENU2_SET_XY2( polyL, DIRECT_SCREEN_X( gs->x + sx + dw ), DIRECT_SCREEN_Y( y + gs->h ) ) ;
	DG_MENU2_SET_XY3( polyL, DIRECT_SCREEN_X( gs->x + sx + vw ), DIRECT_SCREEN_Y( y + gs->h + 4 ) ) ;
	DG_MENU2_SET_XY4( polyL, DIRECT_SCREEN_X( gs->x + sx + dw ), DIRECT_SCREEN_Y( y + gs->h + 4 ) ) ;

	count = gs->m9_count ;
	if ( gs->m9_count > 0 ) gs->m9_count -- ;
	if ( count == 0 ) {
		gs->m9_dec = v ;
	} else {
		gs->m9_dec -= ( d - v ) / count ;
	}
}

/* メニュー用ライフゲージ */
static	void	UpdateMenuLifeGage( GM_GageSet *gs, GM_GageSet *lg )
{
	int			ybuf ;

//	if ( lg->flag & GM_GAGE_INVISIBLE ) {
//	if ( gs->flag & GM_GAGE_WARNING ) lg->flag |= GM_GAGE_WARNING ;
	lg->w = gs->w ;
	lg->h = gs->h ;
	lg->value = gs->value ;
	if ( GV_PauseLevel != 0 ) lg->value += GM_VitalityAdjust ;
	lg->prev = gs->prev ;
	lg->dec = gs->dec ;
	lg->max = gs->max ;
	lg->min = gs->min ;
	lg->delay = gs->delay ;
	if ( GM_CheckPlayerStatus( PLAYER_BLOOD_DROP ) ) {
		lg->flag |= GM_GAGE_WARNING ;
	} else {
		lg->flag &= ~GM_GAGE_WARNING ;
	}
    
	if ( ( lg->flag & GM_GAGE_INVISIBLE ) || GM_CheckMenuStatus( MENU_GAGE_OFF ) ) {
		DG_InvisibleMenu2Prim( lg->prim ) ;
		return ;
	}

	ybuf = StartY ;
    /* 状況べつ位置設定 */
    if ( GM_Configuration & GM_CONFIG_OLD_TYPE_MENU ) {
		if ( GM_CheckMenuStatus( MENU_ITEM_OPEN ) ) {
			lg->x = 128 ;
		} else {
			lg->x = 64 ;
		}
		StartY = 40 * Height ;
	} else {
		if ( GM_CheckMenuStatus( MENU_ITEM_OPEN ) ) {
			lg->x = 140 ;
		} else {
			lg->x = 52;
		}
		StartY = DEFAULT_START_Y * Height;  // BP FIX for safe zone
	}

	DG_VisibleMenu2Prim( lg->prim ) ;	
	UpdateGage( lg, DG_Clock ) ;
	StartY = ybuf ;
}

/*------------------------------------------------------------------*/

static	void	Act( work ) 
Work		*work ;
{
    GM_GageSet 	*gs ;
    int		flag ;
    int addOffsetOnce = 0;


    gs = GM_GageList ;

	GM_ResetMenuStatus( MENU_GAGE_ON ) ;
    StartY = DEFAULT_START_Y * Height ;
    if ( GM_GageStatus == GM_GAGE_STATE_APPEAR ) {
		if ( PreStatus == GM_GAGE_STATE_INVISIBLE ) {
			ScrollCount = SCROLL_TIME ;
		} 
		if ( ScrollCount > 0 ) ScrollCount -- ;
		else GM_GageStatus = GM_GAGE_STATE_VISIBLE ;
		StartY -= ScrollCount * SCROLL_VY * Height ;
    } else if ( GM_GageStatus == GM_GAGE_STATE_DISAPPEAR ) {
		if ( PreStatus == GM_GAGE_STATE_VISIBLE ) {
			ScrollCount = 0 ;
		}	
		if ( ScrollCount < SCROLL_TIME ) ScrollCount ++ ;
		else GM_GageStatus = GM_GAGE_STATE_INVISIBLE ;
		StartY -= ScrollCount * SCROLL_VY * Height ;
    }
    PreStatus = GM_GageStatus ;
    GM_N_ActiveGages = 0 ;

    while( 1 ) 
    {
		if ( gs == NULL ) break ;
		flag = gs->flag ;
		if ( gs->level == 0 ) 
      {
			/* ライフゲージ */
			UpdateMenuLifeGage( gs, &MenuLifeGage ) ;
		}
		if ( !( flag & GM_GAGE_INVISIBLE ) ) 
      {
			GM_N_ActiveGages ++ ;
		}
		if ( ( flag & GM_GAGE_INVISIBLE ) ||
#ifdef DEBUG_MODE
			( PlayerDebugMenuStatus & PDMS_DISPLAY_OFF ) ||
#endif
			GM_GageStatus == GM_GAGE_STATE_INVISIBLE ||
			GM_CheckMenuStatus( MENU_GAGE_OFF ) || 
			( GV_PauseLevel & GV_PAUSE_PAUSE ) ) {
			DG_InvisibleMenu2Prim( gs->prim ) ;
			/* 非表示になったらディレイはリセット */
			gs->prev = gs->dec = gs->value ;
			gs->m9_prev = gs->m9_dec = gs->m9_value ;
			gs->count = 0 ;
			gs->m9_count = 0 ;
		} 
      else 
      {
			GM_SetMenuStatus( MENU_GAGE_ON ) ;
			DG_VisibleMenu2Prim( gs->prim ) ;
			UpdateGage( gs, DG_Clock ) ;
			StartY += gs->h + 4 * Height ;
         if ( addOffsetOnce==0 )
         {
            StartY++;
            addOffsetOnce++;

         }

			if ( gs->flag & GM_GAGE_M9EXIST ) StartY += 8 ;
		}
		gs = gs->next ;
    }
}

static	void	Die( work )
Work		*work ;
{
	if ( MenuLifeGage.prim != NULL ) DG_FreeMenu2Prim( MenuLifeGage.prim ) ;
	MenuLifeGage.prim = NULL ;
}

/*------------------------------------------------------------------*/

static	int	GetResources( work )
Work		*work ;
{
    GM_GageStatus = PreStatus = GM_GAGE_STATE_VISIBLE ;
    GM_N_Gages = 0 ;
    GM_N_ActiveGages = 0 ;
    GM_GageList = NULL ;
    return 0 ;
}

/* ゲージ管理デーモン */
void	*NewGageDaemon( void ) 
{
    Work	*work ;

    work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
									 sizeof( Work ), GAGE_DAEMON_PRIO ) ;
    GV_SetActorKillLevel( work, GV_KILL_LEVEL_NORMAL ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
	Height = 256 ;
	if ( Height < DRAW_HEIGHT ) Height = 2 ;
	else						Height = 1 ;

	MenuLifeGage.prim = NULL ;

    return work ;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/

void	GM_AppendGageSet( gs )
GM_GageSet		*gs ;
{
    int		i ;
    GM_GageSet	*list, *prev ;

    if ( GM_N_Gages == 0 ) {
		GM_GageList = gs ;
		gs->next = NULL ;
    } else {
		list = GM_GageList ; prev = NULL ;
		for ( i = 0; i < GM_N_Gages; i ++ ) {
			if ( list->level > gs->level ) {
				if ( prev == NULL ) {
					GM_GageList = gs ;
					gs->next = list ;
				} else {
					prev->next = gs ;
					gs->next = list ;
				}
				break ;
			} else if ( list->next == NULL ) {
				list->next = gs ;
				gs->next = NULL ;
				break ;
			}
			prev = list ;
			list = list->next ;
		}
    }
    gs->prim = NULL ;
    InitPrim( gs ) ;
	if ( gs->level == 0 ) {
		InitPrim( &MenuLifeGage ) ;
	}
    GM_N_Gages ++ ;
}

void	GM_RemoveGageSet( gs ) 
GM_GageSet		*gs ;
{
    int		i, find ;
    GM_GageSet	*list ;
    
    find = 0 ;
    list = GM_GageList ;
    if ( list == gs ) {
		find = 1 ;
		GM_GageList = list->next ;
		goto remove_gageset_skip ;
    }
    for ( i = 0; i < GM_N_Gages; i ++ ) {
		if ( list->next == gs ) {
			list->next = gs->next ;
			find = 1 ;
			break ;
		}
		list = list->next ;
    }
remove_gageset_skip :
    if ( find ) {
		if ( gs->prim != NULL ) DG_FreeMenu2Prim( gs->prim ) ;
		GM_N_Gages -- ;
    }
}

/* 文字列をフォント１で表示したときの幅 */
static	int		TextWidth( char *name )
{
	extern char	font_pitch1[] ;
	u_char		c ;
	int			w, i, len ;

	w = 0 ;
	len = strlen( name ) ;
	for ( i = 0; i < len; i ++ ) {
		c = name[ i ] ;
		if ( c == '\0' || c == '\n' ) break ;
		if ( c >= ' ' && c < ( ' ' + 96 ) ) {
			c -= ' ' ;
			w += font_pitch1[ ( int )c ]*TARGET_ASPECT_X;
		}
	}
	return w + 4 ;
}

void	GM_InitGageSet( gs, name, x, w, h, value, max, min, delay, level )
GM_GageSet		*gs ;
char			*name ;
int			x, w, h, value, max, min, delay, level ;
{
    int			sx ;

    // BP WARNING - WAS strcpy
    strncpy( gs->name, name, sizeof( gs->name ) ) ;
    gs->name[ sizeof( gs->name ) - 1 ] = 0;

    sx = TXT_NUM * TXT_WIDTH ;

   if ( x <= 21 ) x+=21;      //JG make sure it stays out of the safe zone.

    gs->x = x ;
//    gs->w = w + sx ;
    gs->w = w*TARGET_ASPECT_X ;
//    gs->h = h * Height ;
	gs->h = GM_DEFAULT_GAGE_HEIGHT * Height ;
    gs->value = value ;
    gs->prev = value ;
    gs->dec = value ;
    gs->max = max ;
    gs->min = min ;
    gs->delay = delay ;
    gs->level = level ;
    gs->prim = NULL ;
    gs->next = NULL ;
    gs->flag = GM_GAGE_INVISIBLE ;
	gs->text_len = TextWidth( name ) ;
#ifdef KP_XBOX
	gs->text_len = gs->text_len * 512 / DRAW_WIDTH ;
#endif
	if ( level == 0 ) {
		/* ライフゲージ */
		MenuLifeGage = *gs ;
		MenuLifeGage.level = -1 ;
		MenuLifeGage.flag = GM_GAGE_INVISIBLE ;
	}
}

void	GM_InitGageSet2( gs, name, w, value, max, min, delay, level )
GM_GageSet		*gs ;
char			*name ;
int				w, value, max, min, delay, level ;
{
	GM_InitGageSet( gs, name, 16, w, GM_DEFAULT_GAGE_HEIGHT, value, max, min, delay, level ) ;
}


void	GM_SetGageColor( gs, r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4 ) 
GM_GageSet		*gs ;
u_char			r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4 ;
{
    gs->r[ 0 ] = r1 ;
    gs->g[ 0 ] = g1 ;
    gs->b[ 0 ] = b1 ;
    gs->r[ 1 ] = r2 ;
    gs->g[ 1 ] = g2 ;
    gs->b[ 1 ] = b2 ;
    gs->r[ 2 ] = r3 ;
    gs->g[ 2 ] = g3 ;
    gs->b[ 2 ] = b3 ;
    gs->r[ 3 ] = r4 ;
    gs->g[ 3 ] = g4 ;
    gs->b[ 3 ] = b4 ;

	if ( gs->level == 0 ) {
		/* ライフゲージ */
		memcpy( MenuLifeGage.r, gs->r, sizeof( u_char ) * 4 ) ;
		memcpy( MenuLifeGage.g, gs->g, sizeof( u_char ) * 4 ) ;
		memcpy( MenuLifeGage.b, gs->b, sizeof( u_char ) * 4 ) ;
	}
}

/* キャラ別ゲージ処理 */
void	GM_SetGageColorType( GM_GageSet *gs, int type )
{
	u_char	r1, g1, b1, r2, g2, b2 ;

	switch( type ) {
	case GM_GAGE_COLOR_TYPE_PLAYER_LIFE :
		r1 = 0 ;   g1 = 120 ; b1 = 150 ;
		r2 = 128 ; g2 = 200 ; b2 = 130 ;
		break ;
	case GM_GAGE_COLOR_TYPE_NPC_LIFE :
		r1 = 70 ; g1 = 164 ; b1 = 140 ;
		r2 = 70 ; g2 = 164 ; b2 = 140 ;
		break ;
	case GM_GAGE_COLOR_TYPE_ENEMY_LIFE :
		r1 = 128 ; g1 = 200 ; b1 = 130 ;
		r2 = 0 ;   g2 = 120 ; b2 = 150 ;
		break ;
	case GM_GAGE_COLOR_TYPE_PLAYER_O2 :
	default :
		r1 = 31 ;  g1 = 63  ; b1 = 192 ;
		r2 = 31 ;  g2 = 127 ; b2 = 255 ;
		break ;
	}
	GM_SetGageColor( gs, 0, 0, 0, r1, g1, b1, r2, g2, b2, 255, 0, 0 ) ;
}

/* Ｍ９用拡張 */
void	GM_InitGageM9( GM_GageSet *gs, int value, int max, int min, int delay )
{
	gs->flag |= GM_GAGE_M9EXIST ;
	gs->m9_value = value ;
	gs->m9_max = max ;
	gs->m9_min = min ;
	gs->m9_delay = delay ;
	GM_SetGageColorM9( gs, 72, 68, 138, 178, 165, 166, 255, 0, 0 ) ;
}

void	GM_SetGageColorM9( GM_GageSet *gs, 
						   u_char left_r, u_char left_g, u_char left_b, 
						   u_char right_r, u_char right_g, u_char right_b, 
						   u_char delay_r, u_char delay_g, u_char delay_b ) 
{
	gs->m9col_left[ 0 ] = left_r ;
	gs->m9col_left[ 1 ] = left_g ;
	gs->m9col_left[ 2 ] = left_b ;
	gs->m9col_right[ 0 ] = right_r ;
	gs->m9col_right[ 1 ] = right_g ;
	gs->m9col_right[ 2 ] = right_b ;
	gs->m9delay_col_left[ 0 ] = delay_r ;
	gs->m9delay_col_left[ 1 ] = delay_g ;
	gs->m9delay_col_left[ 2 ] = delay_b ;
	gs->m9delay_col_right[ 0 ] = delay_r ;
	gs->m9delay_col_right[ 1 ] = delay_g ;
	gs->m9delay_col_right[ 2 ] = delay_b ;
}
	
/*----------------------------------------------------------------*/

void	GM_VisibleGage( gs )
GM_GageSet		*gs ;
{
	gs->flag &= ~GM_GAGE_INVISIBLE ;
}

void	GM_InvisibleGage( gs )
GM_GageSet		*gs ;
{
	gs->flag |= GM_GAGE_INVISIBLE ;
}

void	GM_VisibleMenuLife( void )
{
	MenuLifeGage.flag &= ~GM_GAGE_INVISIBLE ;
}

void	GM_InvisibleMenuLife( void )
{
	MenuLifeGage.flag |= GM_GAGE_INVISIBLE ;
}

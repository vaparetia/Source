//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   t_sight.c
   仮サイト

   2000/10/17 M.Sonoyama
   $Id: t_sight.c,v 1.1.1.3 2002/11/19 11:50:46 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"camera.h"

extern  void	*NewFarFocusControl( GM_CameraSet *camera );

#define	ANGLE_MAX		(24.0F) 

#define	MAX_LINES		(64)
#define	MAX_LSTRIPS		(32)
#define	MAX_POLYS		(16)
#define	MAX_SPRTS		(6)

#define SPR_CHANL		(4)
typedef	struct	{
	GV_ACT_EX			actor ;
	int					n_lines ;
	int					n_lstrips ;
	int					n_polys ;
	int					n_sprts ;
	SPR_OBJ				*lines[ MAX_LINES ] ;
	SPR_OBJ				*lstrips[ MAX_LSTRIPS ] ;
	SPR_OBJ				*polys[ MAX_POLYS ] ;
	SPR_OBJ				*sprts[ MAX_SPRTS ] ;

	FVECTOR				shift ;
	FVECTOR				shift_aim ;

	SVECTOR				pre_rotate ;
	SVECTOR				rotate ;
	int					show ;
	int					flag ;
	float				t ;

	float				zoom_adj ;
	float				zoom_aim ;
	int					shift_status ;

	float				pre_angle ;
	GM_CameraSet		*camera ;
	void				*focus ;
	int					delay ;
	
	float				scale ;
} Work ;

enum {
	SGT_FLAG_NONE = 		0x0000,
	SGT_FLAG_ZOOM_IN =		0x0001,
	SGT_FLAG_ZOOM_OUT =		0x0002,
	SGT_FLAG_ANGLE_SET =	0x0004,
	SGT_FLAG_HIDE =			0x1000,
	SGT_FLAG_ACTIVE =		0x8000,
} ;

static	Work 	SightWork ;
static	int		Red = 130, Green = 200, Blue = 90 ;
static	int		Red2 = 90, Green2 = 160, Blue2 = 50 ;
static	int		Red3 = 130, Green3 = 200, Blue3 = 90 ;
static	int		Red4 = 60, Green4 = 130, Blue4 = 20 ;

typedef	struct	{
	short	x, y ;
} SGT_XY ;

typedef	struct	{
	u_char	r, g, b, a ;
} SGT_RGBA ;

typedef	struct	{
	int		texid ;
	SGT_XY	xy ;
} SGT_SPRT ;

static	SGT_XY	Lines[] = {
	{ 4, 36 }, { 4, 350 }, { 19, 42 }, { 19, 344 }, 
	{ 34, 48 }, { 34, 338 }, { 49, 54 }, { 49, 330 },
	{ 507, 36 }, { 507, 350 }, { 492, 42 }, { 492, 344 }, 
	{ 477, 48 }, { 477, 338 }, { 462, 54 }, { 462, 330 },	
	{ 45, 196 }, { 481, 196 },
	{ 71, 60 }, { 71, 326 }, { 440, 60 }, { 440, 326 },
	{ 95, 190 }, { 95, 314 }, { 110, 190 }, { 110, 314 }, 
	{ 125, 190 }, { 125, 314 }, { 140, 190 }, { 140, 314 },
	{ 371, 190 }, { 371, 314 }, { 386, 190 }, { 386, 314 }, 
	{ 401, 190 }, { 401, 314 }, { 416, 190 }, { 416, 314 },
	{ 82, 75 }, { 82, 85 }
} ;

enum {
	LINE_ZOOM_1 = 0,
	LINE_ZOOM_2,
	LINE_ZOOM_3,
	LINE_ZOOM_4,
	LINE_ZOOM_5,
	LINE_ZOOM_6,
	LINE_ZOOM_7,
	LINE_ZOOM_8,
	LINE_CENTER,
	LINE_WAKU_L,
	LINE_WAKU_R,
	LINE_GRAPH_1,
	LINE_GRAPH_2,
	LINE_GRAPH_3,
	LINE_GRAPH_4,
	LINE_GRAPH_5,
	LINE_GRAPH_6,
	LINE_GRAPH_7,
	LINE_GRAPH_8,
	LINE_ANGLE,
} ;

static	SGT_XY	LStrips[] = {
	{ 63, 60 }, { 448, 60 }, { 448, 326 }, { 390, 326 },
	{ 361, 355 }, { 150, 355 }, { 121, 326 }, { 63, 326 }, { 63, 60 }, { -1, -1 },
	{ 81, 75 }, { 158, 75 }, { 158, 85 }, { -1, -1 },
	{ 246, 333 }, { 241, 333 }, { 241, 347 }, { 246, 347 }, { -1, -1 },
	{ 265, 333 }, { 270, 333 }, { 270, 347 }, { 265, 347 }, { -1, -1 },
	{ 200, 152 }, { 311, 152 }, { 311, 241 }, { 200, 241 }, { 200, 152 }, { -1, -1 },
	{ 92, 308 }, { 92, 192 }, { 83, 192 }, { 83, 308 }, { 95, 308 }, { -1, -1 },
	{ 107, 308 }, { 107, 192 }, { 98, 192 }, { 98, 308 }, { 110, 308 }, { -1, -1 },
	{ 122, 308 }, { 122, 192 }, { 113, 192 }, { 113, 308 }, { 125, 308 }, { -1, -1 },
	{ 137, 308 }, { 137, 192 }, { 128, 192 }, { 128, 308 }, { 140, 308 }, { -1, -1 },
	{ 374, 308 }, { 374, 192 }, { 383, 192 }, { 383, 308 }, { 371, 308 }, { -1, -1 },
	{ 389, 308 }, { 389, 192 }, { 398, 192 }, { 398, 308 }, { 386, 308 }, { -1, -1 },
	{ 404, 308 }, { 404, 192 }, { 413, 192 }, { 413, 308 }, { 401, 308 }, { -1, -1 },
	{ 419, 308 }, { 419, 192 }, { 428, 192 }, { 428, 308 }, { 416, 308 }, { -1, -1 }
} ;

enum {
	STR_WAKU = 0,
	STR_ANGLE,
	STR_RY_L,
	STR_RY_R,
	STR_CENTER,
	STR_GRAPH_1,
	STR_GRAPH_2,
	STR_GRAPH_3,
	STR_GRAPH_4,
	STR_GRAPH_5,
	STR_GRAPH_6,
	STR_GRAPH_7,
	STR_GRAPH_8,
} ;
#define	N_LSTRIPS	(13)

static	SGT_XY	Polys[] = {
	{ 0, 0 }, { 512, 0 }, { 512, 23 }, { 0, 23 },
	{ 0, 404 }, { 150, 404 }, { 150, 448 }, { 0, 448 },
	{ 150, 404 }, { 173, 375 }, { 173, 448 }, { 150, 448 },
	{ 173, 375 }, { 332, 375 }, { 332, 448 }, { 173, 448 },
	{ 332, 375 }, { 361, 404 }, { 361, 448 }, { 332, 448 },
	{ 361, 404 }, { 512, 404 }, { 512, 448 }, { 361, 448 },
	{ 253, 180 }, { 259, 180 }, { 259, 194 }, { 253, 194 },
	{ 253, 197 }, { 259, 197 }, { 259, 202 }, { 253, 202 },
	{ 82, 76 }, { 83, 76 }, { 83, 85 }, { 82, 85 }
} ;

enum {
	POLY_UPPER = 0,
	POLY_LOWER_1,
	POLY_LOWER_2,
	POLY_LOWER_3,
	POLY_LOWER_4,
	POLY_LOWER_5,
	POLY_CENTER_1,
	POLY_CENTER_2,
	POLY_ANGLE,
} ;

static	SGT_RGBA PolyRgbs[] = {
	{ 0,0,0,64 }, 
	{ 0,0,0,64 }, 
	{ 0,0,0,64 }, 
	{ 0,0,0,64 }, 
	{ 0,0,0,64 }, 
	{ 0,0,0,64 }, 
	{ 50, 110, 50, 32 },
	{ 50, 110, 50, 32 },
	{ 50, 110, 50, 64 }
} ;

static	SGT_SPRT	Sprts[] = {
	{ 8493845, { 81, 62 } },	/* zoom_alp_ovl */
	{ 2073845, { 380, 62 } },	/* angle_alp_ovl */
	{ 1676436, { 316, 200 } }	/* mode_alp_ovl */
} ;

/*----------------------------------------------------------------*/

#define	FOCUS		(403770)	/* 双眼鏡フォーカス */
extern	void	PL_SendMessage( int, int *, int ) ;

/*----------------------------------------------------------------*/

#define	YADJ( y )	( ( float )( y ) * 384.0F / 448.0F ) 

static	inline	float	PALADJ( float y )
{
#ifdef PAL
	return ( y * 256.0F / 224.0F ) ;
#else
	return y ;
#endif
}

static	inline	int		FIX2INT( SPR_FIX f )
{
	int			res ;
	
#if 1 //BP
   BP_RENDER_TODO_BREAK;
   res = ((int)f) >> 4;
#else
	res = ( int )( f >> 4 ) ;
#endif
	return res ;
}

static	inline	void	XY2POS( SGT_XY *xy, SPR_POS *pos )
{
	pos->x = ( float )xy->x ;
	pos->y = YADJ( xy->y ) ;
}

static	void	SetLineXY( SPR_OBJ *sobj, SGT_XY *xy1, SGT_XY *xy2, int mode )
{
	SPR_POS		p1, p2 ;

	XY2POS( xy1, &p1 ) ;
	XY2POS( xy2, &p2 ) ;
	SPR_SetPosLine( sobj, &p1, &p2 ) ;

	if ( mode == 0 ) {
		SPR_SetColorLine( sobj, 0, Red, Green, Blue, 64 ) ;
		SPR_SetColorLine( sobj, 1, Red, Green, Blue, 64 ) ;
	} else {
		SPR_SetColorLine( sobj, 0, Red4, Green4, Blue4, 64 ) ;
		SPR_SetColorLine( sobj, 1, Red4, Green4, Blue4, 32 ) ;
	}
}

static	void	SetLStripXY( SPR_OBJ *sobj, SGT_XY *xy, int n, int mode )
{
	int			i ;
	SPR_POS		p[ 16 ] ;

	SPR_SetLineStripVertexNumber( sobj, n ) ;
	for ( i = 0; i < n; i ++, xy ++ ) XY2POS( xy, &p[ i ] ) ;
	SPR_SetPosLineStrip( sobj, 0, n, p ) ;
	for ( i = 0; i < n; i ++ ) {
		if ( mode == 0 ) SPR_SetColorLineStrip( sobj, i, Red, Green, Blue, 64 ) ;
		else if ( mode == 1 ) SPR_SetColorLineStrip( sobj, i, Red4, Green4, Blue4, 32 ) ;
		else if ( mode == 2 ) SPR_SetColorLineStrip( sobj, i, Red - 30, Green - 30, Blue - 30, 32 ) ;
	}
}

static	int		GetVertexNum( SGT_XY *xy )
{
	int			n = 0 ;

	while( xy->x != -1 || xy->y != -1 ) {
		n ++ ;
		xy ++ ;
	}
	return n ;
}

static	SGT_XY	*GetLStripStart( int n )
{
	SGT_XY		*xy ;

	xy = LStrips ;
	while( -- n >= 0 ) {
		xy += GetVertexNum( xy ) + 1 ;
	}
	return xy ;
}


static	void	SetPolyXY( SPR_OBJ *sobj, SGT_XY *xy, SGT_RGBA *rgb ) 
{
	SPR_POS		p[ 4 ] ;
	SPR_COLOR	col ;
	int			i ;
	
	col = *( SPR_COLOR * )rgb ;

	if ( col.r == 32 && col.g == 232 && col.b == 32 ) {
		col.r = Red ; col.g = Green ; col.b = Blue ;
	}

	for ( i = 0; i < 4; i ++, xy ++ ) XY2POS( xy, &p[ i ] ) ;
	SPR_SetPosPoly( sobj, p ) ;
	for ( i = 0; i < 4; i ++ ) {
		sobj->poly.col[ i ] = col ;
	}
}

static	void	SetAlpha( SPR_OBJ *sobj, int mode )
{
	switch( mode ) {
	case 0 :
		sobj->head.alpha = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ;
		break ;
	case 1 :
		sobj->head.alpha = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) ;
		break ;
	}
	sobj->head.flags |= SPR_FLAG_ALPHA ;
}

static	void	Scaling( SPR_POS *p, float t, float cx, float cy )
{
	float		dx, dy ;

	dx = p->x - cx ;
	dy = p->y - cy ;
	p->x = cx + dx * t ;
	p->y = cy + dy * t ;
}

static	void	VisibleSPR( SPR_OBJ **sobj, int n, int mode )
{
	while( -- n >= 0 ) {
		if ( mode ) SPR_SHOW( *sobj ) ;
		else		SPR_HIDE( *sobj ) ;
		sobj ++ ;
	}
}

static	void	Show( Work *work )
{
	VisibleSPR( work->lines, work->n_lines, 1 ) ;
	VisibleSPR( work->lstrips, work->n_lstrips, 1 ) ;
	VisibleSPR( work->polys, work->n_polys, 1 ) ;
	VisibleSPR( work->sprts, work->n_sprts, 1 ) ;
}

static	void	Hide( Work *work )
{
	VisibleSPR( work->lines, work->n_lines, 0 ) ;
	VisibleSPR( work->lstrips, work->n_lstrips, 0 ) ;
	VisibleSPR( work->polys, work->n_polys, 0 ) ;
	VisibleSPR( work->sprts, work->n_sprts, 0 ) ;
}

/*----------------------------------------------------------------*/

/* 数字表示 */
static	void	SetNumberPrim( Work *work )
{
	int			center, c, d, d2, a ;
	int			i, j, n, adj ;
	int			h = 256 ;
	float		y ;

#ifdef PAL
	adj = 1 ;
#else
	adj = 0 ;
#endif
	if ( work->show == 0 ) return ;
	center = work->rotate.vy & 4095 ;
	c = center / 128 * 128 ;
	d = ( c - center ) / 4 ;
	y = 334.0F * 224.0F / 448.0F ;
	if ( DRAW_HEIGHT > h ) y *= 2.0F ;
	MENU_SetAlphaMode( 0, 2, 0, 1, 0 ) ;
	for ( i = c - 128 * 4, j = -4; j <= 4; i += 128, j ++ ) {
		d2 = d + 32 * j ;
//		a = 64 - ( int )( ( float )abs( d2 ) / 1.5F ) ; if ( a < 1 ) a = 1 ;
		a = 32 - ( int )( ( float )abs( d2 ) / 3.0F ) ; if ( a < 1 ) a = 1 ;

//		MENU_Locate( 256 + d2, 332.0F * 224.0F / 448.0F, MENU_MODE_CENTER ) ;
		MENU_S_Locate( 256 + d2 + 9, ( int )PALADJ( y ) + adj, 1 ) ;
//		MENU_Color( 32, 232, 32, a ) ;
//		MENU_S_Color( Red, Green, Blue, a ) ;
		MENU_S_Color( 50, 110, 50, a ) ;
		n = i ; if ( n > 2048 ) n -= 4096 ;
		/* デモ中はでない */
		if ( !GM_CheckGameStatus( STATE_PLAY_DEMO ) ) {
			MENU_S_Printf( "%02d", abs( n / 128 ) ) ;
		}
	}
}

/* ズーム印 */
static	void	ZoomInOut( Work *work, float v )
{
	SPR_OBJ		*sobj ;
	SPR_POS		p1, p2 ;
	int			i ;
	float		f ;

	for ( i = LINE_ZOOM_1; i <= LINE_ZOOM_4; i ++ ) {
		sobj = work->lines[ i ] ;
		p1 = sobj->line.pos[ 0 ] ;
		p1.x += v ;
		if ( p1.x >= 64.0F ) p1.x = 4.0F ; 	
		if ( p1.x <= 3.0F )  p1.x = 63.0F ;
		p2.x = p1.x ;
		f = ( 60.0F - 36.0F ) * ( ( p1.x - 4.0F ) / ( 64.0F - 4.0F ) ) ;
		p1.y = YADJ( 36.0F + f ) ;
		p2.y = YADJ( 350.0F - f ) ;
		SPR_SetPosLine( sobj, &p1, &p2 ) ;		
		p1.x += ( 255.5F - p1.x ) * 2.0F ;
		p2.x = p1.x ;
		sobj = work->lines[ i + 4 ] ;
		SPR_SetPosLine( sobj, &p1, &p2 ) ;
	}
	if ( v > 0.0F ) work->zoom_aim = 0.875F ;
	else if ( v < 0.0F ) work->zoom_aim = 1.125F ;

}

/* 中心矩形 */
static	void	CenterZoom( Work *work )
{
	SPR_OBJ		*sobj ;
	SPR_POS		p[ 5 ] ;
	SGT_XY		*xy ;
	int			n, i ;
	float		f ;

	if ( work->rotate.vx < work->pre_rotate.vx ) {
		work->shift_aim.vy = -8.0F ;
	} else if ( work->rotate.vx > work->pre_rotate.vx ) {
		work->shift_aim.vy = 8.0F ;		
	}
	if ( work->rotate.vy < work->pre_rotate.vy ) {
		work->shift_aim.vx = 16.0F ;
	} else if ( work->rotate.vy > work->pre_rotate.vy ) {
		work->shift_aim.vx = -16.0F ;		
	}

	sobj = work->lstrips[ STR_CENTER ] ;
	xy = GetLStripStart( STR_CENTER ) ;
	n = GetVertexNum( xy ) ;
	for ( i = 0; i < n; i ++, xy ++ ) {
		XY2POS( xy, &p[ i ] ) ;
		Scaling( &p[ i ], work->zoom_adj, 255.5F, YADJ( 196 ) ) ;
		p[ i ].x += work->shift.vx ;
		p[ i ].y += work->shift.vy ;
	}
	SPR_SetPosLineStrip( sobj, 0, n, p ) ;

	/* 中心ライン */
	sobj = work->lines[ LINE_CENTER ] ;
	sobj->line.pos[ 0 ].y = sobj->line.pos[ 1 ].y = f 
		= YADJ( Lines[ LINE_CENTER * 2 ].y ) - work->shift.vy ;

	/* 緑びっくり */
	sobj = work->polys[ POLY_CENTER_1 ] ;
	xy = Polys + POLY_CENTER_1 * 4 ;
	for ( i = 0; i < 4; i ++, xy ++ ) {
		sobj->poly.vert[ i ].x = xy->x + work->shift.vx ;
//		sobj->poly.vert[ i ].y = YADJ( xy->y ) - work->shift.vy ;
		if ( i < 2 ) sobj->poly.vert[ i ].y = f - 14.0F ;
		else		 sobj->poly.vert[ i ].y = f - 3.0F ;
		if ( work->shift.vy > 0.1F ) sobj->poly.vert[ i ].y -= 2.0F ;
	}
	sobj = work->polys[ POLY_CENTER_2 ] ;
	xy = Polys + POLY_CENTER_2 * 4 ;
	for ( i = 0; i < 4; i ++, xy ++ ) {
		sobj->poly.vert[ i ].x = xy->x - ( work->shift.vx / 3.0F ) ;
//		sobj->poly.vert[ i ].y = YADJ( xy->y ) - work->shift.vy ;
		if ( i < 2 ) sobj->poly.vert[ i ].y = f ;
		else 		 sobj->poly.vert[ i ].y = f + 4.0F ;
	}
}

/* アングル */
static	void	AngleSet( Work *work )
{
	float		x ;
	SPR_OBJ		*sobj ;

	sobj = work->polys[ POLY_ANGLE ] ;
	x = 82.0F + work->t * ( 158.0F - 82.0F ) ;
	if ( x < 82.0F ) x = 82.0F ;
	else if ( x > 158.0F ) x = 158.0F ;
	sobj->poly.vert[ 1 ].x = x ;
	sobj->poly.vert[ 2 ].x = x ;
}

/* いんちきグラフ */
static	void	Graph( Work *work )
{
	SPR_OBJ		*sobj ;
	SVECTOR		*rot ;
	float		y, py ;
	int			i ;

	rot = &work->rotate ;
	for ( i = 0; i < 8; i ++ ) {
		sobj = work->lstrips[ STR_GRAPH_1 + i ] ;
		py = sobj->linestrip.plist[ 1 ].pos.y ;
		y = ( float )308 - ( float )( ( ( rot->vy & 4095 ) * i / ( 9 - i ) 
									   + rot->vx ) % ( 308 - 192 ) ) ;
		if ( DG_FABS( work->zoom_adj - 1.0F ) > 0.1F ) {
			y += BP_PS2_rand() % 64 ;
		}
		if ( y < 192 ) y = 192 ;
		if ( y >= 292.0F ) y = 292.0F ;
		y = YADJ( y ) ;
		y = GV_NearExp8F( py, y ) ;
		sobj->linestrip.plist[ 1 ].pos.y = y ;
		sobj->linestrip.plist[ 2 ].pos.y = y ;
	}
}

/* 色調整 */
void	PL_SetSightRGB( void )
{
	int			pad, m ;
	int			i, j ;
	SPR_OBJ		*sobj ;
	SGT_XY		*xys ;
	int			r, g, b ;
	int			r2, g2, b2 ;
	int			r3, g3, b3 ;
	Work		*work ;

	r = Red ; g = Green ; b = Blue ;
	r2 = Red2 ; g2 = Green2 ; b2 = Blue2 ;
	r3 = Red3 ; g3 = Green3 ; b3 = Blue3 ;
	pad = GV_PadData[ 1 ].status ;
	if ( pad & PAD_A ) {
		if ( pad & PAD_L1 ) {
			if ( pad & PAD_U ) Red2 ++ ;
			if ( pad & PAD_D ) Red2 -- ;
			if ( Red2 > 255 ) Red2 = 255 ;
			if ( Red2 < 0 ) Red2 = 0 ;
		} else if ( pad & PAD_L2 ) {
			if ( pad & PAD_U ) Red3 ++ ;
			if ( pad & PAD_D ) Red3 -- ;
			if ( Red3 > 255 ) Red3 = 255 ;
			if ( Red3 < 0 ) Red3 = 0 ;
		} else {
			if ( pad & PAD_U ) Red ++ ;
			if ( pad & PAD_D ) Red -- ;
			if ( Red > 255 ) Red = 255 ;
			if ( Red < 0 ) Red = 0 ;
		}
	}
	if ( pad & PAD_X ) {
		if ( pad & PAD_L1 ) {
			if ( pad & PAD_U ) Green2 ++ ;
			if ( pad & PAD_D ) Green2 -- ;
			if ( Green2 > 255 ) Green2 = 255 ;
			if ( Green2 < 0 ) Green2 = 0 ;
		} else if ( pad & PAD_L2 ) {
			if ( pad & PAD_U ) Green3 ++ ;
			if ( pad & PAD_D ) Green3 -- ;
			if ( Green3 > 255 ) Green3 = 255 ;
			if ( Green3 < 0 ) Green3 = 0 ;
		} else {
			if ( pad & PAD_U ) Green ++ ;
			if ( pad & PAD_D ) Green -- ;
			if ( Green > 255 ) Green = 255 ;
			if ( Green < 0 ) Green = 0 ;
		}
	}
	if ( pad & PAD_B ) {
		if ( pad & PAD_L1 ) {
			if ( pad & PAD_U ) Blue2 ++ ;
			if ( pad & PAD_D ) Blue2 -- ;
			if ( Blue2 > 255 ) Blue2 = 255 ;
			if ( Blue2 < 0 ) Blue2 = 0 ;
		} else if ( pad & PAD_L2 ) {
			if ( pad & PAD_U ) Blue3 ++ ;
			if ( pad & PAD_D ) Blue3 -- ;
			if ( Blue3 > 255 ) Blue3 = 255 ;
			if ( Blue3 < 0 ) Blue3 = 0 ;
		} else {
			if ( pad & PAD_U ) Blue ++ ;
			if ( pad & PAD_D ) Blue -- ;
			if ( Blue > 255 ) Blue = 255 ;
			if ( Blue < 0 ) Blue = 0 ;
		}
	}

	MENU_Locate( 8, 8, 0 ) ;
	MENU_Printf( "%d %d %d\n", Red, Green, Blue ) ;
	MENU_Printf( "%d %d %d\n", Red2, Green2, Blue2 ) ;
	MENU_Printf( "%d %d %d\n", Red3, Green3, Blue3 ) ;

	if ( r == Red && g == Green && b == Blue &&
		 r2 == Red2 && g2 == Green2 && b2 == Blue2 &&
		 r3 == Red3 && g3 == Green3 && b3 == Blue3 ) return ;

	work = &SightWork ;

	for ( i = 0; i < work->n_lines; i ++ ) {
		sobj = work->lines[ i ] ;
		SPR_SetColorLine( sobj, 0, Red, Green, Blue, 64 ) ;
		SPR_SetColorLine( sobj, 1, Red, Green, Blue, 64 ) ;
	}
	xys = LStrips ;
	for ( i = 0; i < work->n_lstrips; i ++ ) {
		sobj = work->lstrips[ i ] ;
		m = GetVertexNum( xys ) ;
		for ( j = 0; j < m; j ++ ) {
			SPR_SetColorLineStrip( sobj, j, Red, Green, Blue, 64 ) ;
		}
		xys += m + 1 ;
	}	
	for ( i = 0; i < work->n_polys; i ++ ) {
		if ( i < 6 || i > 8 ) continue ;
		sobj = work->polys[ i ] ;
		if ( i != 8 ) {
			for ( j = 0; j < 4; j ++ ) {
				sobj->poly.col[ j ].r = Red ;
				sobj->poly.col[ j ].g = Green ;
				sobj->poly.col[ j ].b = Blue ;
			}
		} else {
			for ( j = 0; j < 4; j ++ ) {
				sobj->poly.col[ j ].r = Red2 ;
				sobj->poly.col[ j ].g = Green2 ;
				sobj->poly.col[ j ].b = Blue2 ;
			}
		}
	}
	for ( i = 0; i < work->n_sprts; i ++ ) {
		sobj = work->sprts[ i ] ;
		sobj->sprite.col.r = Red3 ;
		sobj->sprite.col.g = Green3 ;
		sobj->sprite.col.b = Blue3 ;
		sobj->sprite.col.a = 128 ;
	}	

}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	int				flag ;
	int				buf[ 8 ] ;
	GM_CameraSet	*cam ;

//	PL_SetSightRGB() ;

	SetNumberPrim( work ) ;

	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ) return ;

	cam = work->camera = GM_GetCurrentCameraSet( 0 ) ;
	work->rotate = cam->rotate ;

	if ( DG_FABS( work->pre_angle - cam->angle ) > 0.01F ) {
		work->flag |= SGT_FLAG_ANGLE_SET ;
		work->t = ( cam->angle - 2.0F ) / ( ANGLE_MAX * work->scale - 2.0F ) ;
		if ( work->pre_angle < cam->angle ) work->flag |= SGT_FLAG_ZOOM_IN ;
		else								work->flag |= SGT_FLAG_ZOOM_OUT ;
#if 0
		/* フォーカス */
		if ( work->delay < 0 ) work->delay = 0 ;
		if ( work->delay < 16 ) ++ work->delay ;
#endif
	} else {
#if 0
		if ( work->delay > 0 ) {
			if ( -- work->delay == 0 ) {
				buf[ 0 ] = 0 ;
				/* フォーカスＯＦＦ */
				PL_SendMessage( FOCUS, buf, 1 ) ;
			}
		}		
#endif
	}
#if 0
	/* フォーカスにメッセージ */
    if ( work->delay != 0 ) {
		buf[ 0 ] = 2 ;
		buf[ 1 ] = 0 ;
		buf[ 2 ] = 0 ;
		buf[ 4 ] = 100000 ;
		if ( work->delay < 8 ) {
			buf[ 3 ] = 19300 - work->delay * 2400 ;
		} else {
			buf[ 3 ] = 100 + ( work->delay - 8 ) * 500 ;
		}
		PL_SendMessage( FOCUS, buf, 5 ) ;
    }
#endif
	flag = work->flag ;
	
	if ( GM_CheckGameStatus( STATE_CUT_IN ) ) flag |= SGT_FLAG_HIDE ;

	if ( work->show == 0 && !( flag & SGT_FLAG_HIDE ) ) {
		Show( work ) ;
		work->show = 1 ;
	} else if ( work->show == 1 && ( flag & SGT_FLAG_HIDE ) ) {
		Hide( work ) ;
		work->show = 0 ;
	}

	if ( flag & SGT_FLAG_ZOOM_IN ) ZoomInOut( work, 1.0F ) ;
	else if ( flag & SGT_FLAG_ZOOM_OUT ) ZoomInOut( work, -1.0F ) ;

	if ( flag & SGT_FLAG_ANGLE_SET ) AngleSet( work ) ;

	CenterZoom( work ) ;
	Graph( work ) ;

	work->flag &= ~( SGT_FLAG_ZOOM_IN | SGT_FLAG_ZOOM_OUT | SGT_FLAG_ANGLE_SET ) ;

	GV_NearExp8VF( &work->shift, &work->shift_aim, 3 ) ;
	if ( DG_FABS( work->shift.vx - work->shift_aim.vx ) < 1.0F &&
		 DG_FABS( work->shift.vy - work->shift_aim.vy ) < 1.0F ) {
		DG_COPY_VEC( &work->shift, &work->shift_aim ) ;
	}
	work->zoom_adj = GV_NearExp4F( work->zoom_adj, work->zoom_aim ) ;
	work->zoom_aim = 1.0F ;
	DG_COPY_VEC( &work->shift_aim, &DG_ZeroVector ) ;
	work->pre_rotate = work->rotate ;
	work->pre_angle = cam->angle ;
}

static	void	Die( Work *work )
{
	int			i ;

	for ( i = 0; i < work->n_lines; i ++ ) {
		if ( work->lines[ i ] != NULL ) {
			SPR_Destroy_2D_Object( work->lines[ i ] ) ;
		}
	}
	for ( i = 0; i < work->n_lstrips; i ++ ) {
		if ( work->lstrips[ i ] != NULL ) {
			SPR_Destroy_2D_Object( work->lstrips[ i ] ) ;
		}
	}
	for ( i = 0; i < work->n_polys; i ++ ) {
		if ( work->polys[ i ] != NULL ) {
			SPR_Destroy_2D_Object( work->polys[ i ] ) ;
		}
	}
	for ( i = 0; i < work->n_sprts; i ++ ) {
		if ( work->sprts[ i ] != NULL ) {
			SPR_Destroy_2D_Object( work->sprts[ i ] ) ;
		}
	}
	work->flag &= ~SGT_FLAG_ACTIVE ;
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

static	int		GetResources( Work *work )
{
	SPR_OBJ		*sobj ;
	SPR_POS		p ;
	SGT_XY		*xys, *st ;
	SGT_RGBA	*rgb ;
	SGT_SPRT	*sprt ;
	int			n, m, i, j ;

	work->n_lines = n = sizeof( Lines ) / sizeof ( SGT_XY ) / 2 ;
	xys = Lines ;
	for ( i = 0; i < n && i < MAX_LINES; i ++, xys += 2 ) {
		sobj = work->lines[ i ] = SPR_Create_2D_Object( SP_LINE, SPR_CHANL, NULL ) ;
		if ( sobj == NULL ) return -1 ;
		if ( ( i < LINE_ZOOM_1 || i > LINE_ZOOM_8 ) &&
			 ( i < LINE_GRAPH_1 || i > LINE_GRAPH_8 ) ) SetLineXY( sobj, xys, xys + 1, 0 ) ;
		else											SetLineXY( sobj, xys, xys + 1, 1 ) ;
		SetAlpha( sobj, 1 ) ;
		SPR_SHOW( sobj ) ;
	}
	work->n_lstrips = n = N_LSTRIPS ;
	xys = LStrips ;
	for ( i = 0; i < n; i ++ ) {
		st = xys ;
		sobj = work->lstrips[ i ] = SPR_Create_2D_Object( SP_LINESTRIP, SPR_CHANL, NULL ) ;
		if ( sobj == NULL ) return -1 ;
		m = GetVertexNum( st ) ;
		if ( i == STR_RY_L || i == STR_RY_R ) SetLStripXY( sobj, xys, m, 2 ) ;
		else if ( i < STR_GRAPH_1 || i > STR_GRAPH_8 ) SetLStripXY( sobj, xys, m, 0 ) ;
		else										SetLStripXY( sobj, xys, m, 1 ) ;
		SetAlpha( sobj, 1 ) ;
		SPR_SHOW( sobj ) ;
		xys += m + 1 ;
	}	
	
	work->n_polys = n = sizeof( Polys ) / sizeof( SGT_XY ) / 4 ;
	xys = Polys ;
	rgb = PolyRgbs ;
	for ( i = 0; i < n; i ++, xys += 4, rgb ++ ) {
		sobj = work->polys[ i ] = SPR_Create_2D_Object( SP_POLY, SPR_CHANL, NULL ) ;
		if ( sobj == NULL ) return -1 ;
		SetPolyXY( sobj, xys, rgb ) ;
		if ( i < 6 ) SetAlpha( sobj, 0 ) ;
		else		 SetAlpha( sobj, 1 ) ;
		SPR_SHOW( sobj ) ;		
	}

	work->n_sprts = n = sizeof( Sprts ) / sizeof( SGT_SPRT ) ;
	sprt = Sprts ;
//	SPR_LoadTexture( GV_StrCode( "radar" ) ) ;
	SPR_LoadTexture( GV_StrCode( "2D_tex" ) ) ;
	for ( i = 0; i < n; i ++, sprt ++ ) {
		sobj = work->sprts[ i ] = SPR_Create_2D_Object( SP_SPRITE, SPR_CHANL, NULL ) ;
		if ( sobj == NULL ) return -1 ;
		XY2POS( &sprt->xy, &p ) ;
		SPR_SetPosSprite( sobj, &p ) ;
		SPR_ObjSetTexture( sobj, sprt->texid, 0 ) ;
		{
			float		w, h ;

			w = ( float )FIX2INT( sobj->sprite.head.tex.w ) + 1 ;
//			h = ( float )FIX2INT( sobj->sprite.head.tex.h ) + 5 ;
			h = ( float )FIX2INT( sobj->sprite.head.tex.h ) * 1.2F ;
			SPR_SetSizeSprite( sobj, w, h ) ;
		}

		sobj->sprite.col.r = Red ;
		sobj->sprite.col.g = Green ;
		sobj->sprite.col.b = Blue ;
		sobj->sprite.col.a = 64 ;

		SetAlpha( sobj, 1 ) ;
//		sobj->head.flags |= SPR_FLAG_ALPHA ;
//		sobj->head.alpha = sobj->ex_hd.tex.dgtex->tex_trans.alpha.data ;
		SPR_SHOW( sobj ) ;		
	}

	work->flag = 0 ;
	work->show = 1 ;
	work->shift_status = 0 ;
	work->zoom_adj = work->zoom_aim = 1.0F ;


    /* ぼかし起動 */
	{
		work->focus = NewFarFocusControl( NULL ) ;
		if ( work->focus == NULL ) return 0 ;
		GV_SetActorChild( &work->actor, work->focus ) ;
	}

#if 0
	{	
		int		buf[ 2 ] ;

		work->focus = NewFarFocusEffect( FOCUS, 8, 1000, 1000 ) ;
		if ( work->focus == NULL ) {
			printf( "warning : focus effect could not start\n" ) ;
			return 0 ;
		}
		GV_SetActorChild( &work->actor, work->focus ) ;
		/* 最初はＯＦＦ */
		buf[ 0 ] = 0 ;
		PL_SendMessage( FOCUS, buf, 1 ) ;
	}
#endif

	return 0 ;
}

/*----------------------------------------------------------------*/

void			*NewScopeSight( float scale )
{
	Work		*work ;

	work = &SightWork ;
	if ( work->flag & SGT_FLAG_ACTIVE ) return NULL ;
	GV_ZeroMemory( work, sizeof( Work ) ) ;
	work->flag |= SGT_FLAG_ACTIVE ;
	GV_SetActorFreeFunc( &work->actor, NULL ) ;
	GV_SetActorClass( &work->actor, GV_CLASS_OBJECT ) ;
	GV_InsertActorPriority( GV_ACTOR_AFTER2, &work->actor, PLAYER_MENU_ACTOR_PRIO ) ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	work->scale = scale ;
	return work ;
}

void	PL_ShowScopeSight( void )
{
	SightWork.flag &= ~SGT_FLAG_HIDE ;
}

void	PL_HideScopeSight( void )
{
	SightWork.flag |= SGT_FLAG_HIDE ;
}

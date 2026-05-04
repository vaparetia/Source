/*
   m_prim2.h

   メニュー２プリミティブ汎用ヘッダ

   2000/05/01 M.Sonoyama	
   $Id: m_prim2.h,v 1.1.1.3 2002/11/19 11:42:12 Yoshizawa1 Exp $
*/

#ifndef __m_prim2_h__
#define	__m_prim2_h__

enum {
	DG_MENU2_PRIM_LINE	= DG_PRIM2_LINE,
	DG_MENU2_PRIM_POLY  = DG_PRIM2_POLY,
	DG_MENU2_PRIM_SPRT  = DG_PRIM2_SPRT,
	DG_MENU2_PRIM_POLY3 = 0x0005,
	DG_MENU2_PRIM_LINESTRIP = 0x0006,
	DG_MENU2_PRIM_POINT =	0x0007,
	DG_MENU2_PRIM_TYPEMASK = DG_PRIM2_TYPEMASK,
} ;

typedef	ALIGN16_DECL(struct)	_dg_menu2_packet {
	DG_DMATAG		setuptag ;
	DG_DMATAG		dmatag ;
	sceGifTag		primtag ;
	DG_GSREG		prim ;
	DG_GSREG		alpha ;
	sceGifTag		giftag ;
	u_long128		data[ 0 ] ;
} DG_MENU2_PACKET  ;

typedef	ALIGN16_DECL(struct) {
	DG_DMAPACK		dmapack ;
	int				flag ;
	int				n_prims ;
	int				size ;
	int				name ;
	int				priority ;
	DG_MENU2_PACKET	*packet[ 2 ] ;
} DG_MENU2_PRIM  ;

typedef	struct _dg_menu2_point {
	sceGsRgbaq		rgba1 ;
	sceGsXyzf		xy1 ;
} DG_MENU2_POINT ;

typedef struct _dg_menu2_line {
	sceGsRgbaq		rgba1 ;
	sceGsUv			uv1 ;
	sceGsXyzf		xy1 ;
	sceGsUv			uv2 ;
	sceGsXyzf		xy2 ;
} DG_MENU2_LINE ;

typedef struct _dg_menu2_line_g {
	sceGsRgbaq		rgba1 ;
	sceGsUv			uv1 ;
	sceGsXyzf		xy1 ;
	sceGsRgbaq		rgba2 ;
	sceGsUv			uv2 ;
	sceGsXyzf		xy2 ;
} DG_MENU2_LINE_G ;

typedef	struct	_dg_menu2_sprt {
	sceGsRgbaq		rgba1 ;
	sceGsUv			uv1 ;
	sceGsXyzf		xy1 ;
	sceGsUv			uv2 ;
	sceGsXyzf		xy2 ;
} DG_MENU2_SPRT ;

typedef	struct	_dg_menu2_poly {
	sceGsPrim		prim ;
	sceGsRgbaq		rgba1 ;
	sceGsUv			uv1 ;
	sceGsXyzf		xy1 ;
	sceGsUv			uv2 ;
	sceGsXyzf		xy2 ;
	sceGsUv			uv3 ;
	sceGsXyzf		xy3 ;
	sceGsUv			uv4 ;
	sceGsXyzf		xy4 ;
} DG_MENU2_POLY ;

typedef	struct	_dg_menu2_poly_g {
	sceGsPrim		prim ;
	sceGsRgbaq		rgba1 ;
	sceGsUv			uv1 ;
	sceGsXyzf		xy1 ;
	sceGsRgbaq		rgba2 ;
	sceGsUv			uv2 ;
	sceGsXyzf		xy2 ;
	sceGsRgbaq		rgba3 ;
	sceGsUv			uv3 ;
	sceGsXyzf		xy3 ;
	sceGsRgbaq		rgba4 ;
	sceGsUv			uv4 ;
	sceGsXyzf		xy4 ;
} DG_MENU2_POLY_G ;

typedef	struct	_dg_menu2_poly3 {
	sceGsPrim		prim ;
	sceGsRgbaq		rgba1 ;
	sceGsUv			uv1 ;
	sceGsXyzf		xy1 ;
	sceGsUv			uv2 ;
	sceGsXyzf		xy2 ;
	sceGsUv			uv3 ;
	sceGsXyzf		xy3 ;
} DG_MENU2_POLY3 ;

typedef	struct	_dg_menu2_poly3_g {
	sceGsPrim		prim ;
	sceGsRgbaq		rgba1 ;
	sceGsUv			uv1 ;
	sceGsXyzf		xy1 ;
	sceGsRgbaq		rgba2 ;
	sceGsUv			uv2 ;
	sceGsXyzf		xy2 ;
	sceGsRgbaq		rgba3 ;
	sceGsUv			uv3 ;
	sceGsXyzf		xy3 ;
} DG_MENU2_POLY3_G ;

/*------------------------------------------------------------------------*/

#define DRAW_Z_MAXL		(16777216ULL-4096ULL)	/* Ｚ値最大 */

#define	DG_MENU2_SET_XY1( p, x, y ) \
{ \
	  *(u_long64 * )(&((p)->xy1)) \
	  = ((DRAW_Z_MAXL << 32) | (DG_POS_Y( y ) << 16) | DG_POS_X( x )) ; \
}

#define	DG_MENU2_SET_XY2( p, x, y ) \
{ \
	  *(u_long64 * )(&((p)->xy2)) \
	  = ((DRAW_Z_MAXL << 32) | (DG_POS_Y( y ) << 16) | DG_POS_X( x )) ; \
}

#define	DG_MENU2_SET_XY3( p, x, y ) \
{ \
	  *(u_long64 * )(&((p)->xy3)) \
	  = ((DRAW_Z_MAXL << 32) | (DG_POS_Y( y ) << 16) | DG_POS_X( x )) ; \
}

#define	DG_MENU2_SET_XY4( p, x, y ) \
{ \
	  *(u_long64 * )(&((p)->xy4)) \
	  = ((DRAW_Z_MAXL << 32) | (DG_POS_Y( y ) << 16) | DG_POS_X( x )) ; \
}

#define	DG_MENU2_SET_RGBA1( p, r, g, b, a ) \
{ \
	  *(u_long64 * )(&((p)->rgba1))  = r | ( g << 8 ) | ( b << 16 ) | ( a << 24 ) ; \
}

#define	DG_MENU2_SET_RGBA2( p, r, g, b, a ) \
{ \
	  *(u_long64 * )(&((p)->rgba2))  = r | ( g << 8 ) | ( b << 16 ) | ( a << 24 ) ; \
}

#define	DG_MENU2_SET_RGBA3( p, r, g, b, a ) \
{ \
	  *(u_long64 * )(&((p)->rgba3))  = r | ( g << 8 ) | ( b << 16 ) | ( a << 24 ) ; \
}

#define	DG_MENU2_SET_RGBA4( p, r, g, b, a ) \
{ \
	  *(u_long64 * )(&((p)->rgba4))  = r | ( g << 8 ) | ( b << 16 ) | ( a << 24 ) ; \
}

#define	DG_MENU2_SET_UV1( p, u, v ) \
{ \
	  *(u_long64 * )(&((p)->uv1))  = ( x << 4 ) | ( y << 20 ) ; \
}

#define	DG_MENU2_SET_UV2( p, u, v ) \
{ \
	  *(u_long64 * )(&((p)->uv2))  = ( x << 4 ) | ( y << 20 ) ; \
}

#define	DG_MENU2_SET_UV3( p, u, v ) \
{ \
	  *(u_long64 * )(&((p)->uv3))  = ( x << 4 ) | ( y << 20 ) ; \
}

#define	DG_MENU2_SET_UV4( p, u, v ) \
{ \
	  *(u_long64 * )(&((p)->uv4))  = ( x << 4 ) | ( y << 20 ) ; \
}

/* m_prim2.c */
extern	sceGifTag	DG_GIFTAG_MENU2_LINE ;
extern	sceGifTag	DG_GIFTAG_MENU2_LINE_G ;
extern	sceGifTag	DG_GIFTAG_MENU2_POLY ;
extern	sceGifTag	DG_GIFTAG_MENU2_POLY_G ;
extern	sceGifTag	DG_GIFTAG_MENU2_SPRT ;
extern	sceGifTag	DG_GIFTAG_MENU2_POLY3 ;
extern	sceGifTag	DG_GIFTAG_MENU2_POLY3_G ;
extern	sceGifTag	DG_GIFTAG_MENU2_POINT ;
#define	DG_GIFTAG_MENU2_SPRT	DG_GIFTAG_MENU2_LINE

extern	DG_MENU2_PRIM	*DG_MakeMenu2Prim( int, int, int ) ;
extern	void			DG_FreeMenu2Prim( DG_MENU2_PRIM * ) ;
#define	GM_FreeMenu2Prim( _p )	DG_FreeMenu2Prim( _p )

extern	void	DG_VisibleMenu2Prim( DG_MENU2_PRIM * ) ;
extern	void	DG_InvisibleMenu2Prim( DG_MENU2_PRIM * ) ;

#endif /* __m_prim2_h__ */

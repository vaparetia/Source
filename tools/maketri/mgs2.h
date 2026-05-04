/*
	mgs2.h
	ＭＧＳ２用構造体定義 [[Structure definition for]]
	$Id: mgs2.h,v 1.1 2000/09/28 00:27:51 usr02774 Exp $

*/


#ifndef __MGS2_H__
#define __MGS2_H__

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------*/
/* ＜ベクトル関連定義＞ [[Vector related definitions]] */
typedef struct { unsigned char r, g, b, cd ; } CVECTOR ;
typedef struct { short vx, vy, vz, pad ; } SVECTOR ;
typedef struct { int vx, vy, vz, vw ; } IVECTOR ;
typedef struct { float vx, vy, vz, vw ; } FVECTOR ;
typedef struct { float m[4][4] ; } FMATRIX ;

/*----------------------------------------------------------------*/
/* ＜ＬＩＢＤＧ汎用定義＞ [[LIBDG generic definitions]] */

/* ＰＳ２ＤＭＡタグ構造体 [[PS2 DMA tag structure]] */
typedef struct _dg_dmatag{
	u_int	qwc ;
	void	*addr ;
	u_int	vifcode[2] ;
} DG_DMATAG ;

typedef struct _dg_gsreg {
	u_long64	data ;
	u_long64	reg ;
} DG_GSREG ;

typedef struct _dg_giftag{
	u_long64	tag ;
	u_long64	regs ;
} DG_GIFTAG ;

typedef struct _dg_vifcode {
	u_int	vifcode[4] ;
} DG_VIFCODE ;





#ifdef __cplusplus
}
#endif


#endif

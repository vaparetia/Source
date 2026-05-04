/*
	mgs2.h
	£Í£Ç£Ó£²ÍÑ¹½Â¤ÂÎÄêµÁ
	$Id: mgs2.h,v 1.1 2002/06/19 05:48:55 usr02774 Exp $

*/


#ifndef __MGS2_H__
#define __MGS2_H__

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------*/
/* ¡ã¥Ù¥¯¥È¥ë´ØÏ¢ÄêµÁ¡ä */
typedef struct { unsigned char r, g, b, cd ; } CVECTOR ;
typedef struct { short vx, vy, vz, pad ; } SVECTOR ;
typedef struct { int vx, vy, vz, vw ; } IVECTOR ;
typedef struct { float vx, vy, vz, vw ; } FVECTOR ;
typedef struct { float m[4][4] ; } FMATRIX ;

/*----------------------------------------------------------------*/
/* ¡ã£Ì£É£Â£Ä£ÇÈÆÍÑÄêµÁ¡ä */

/* £Ð£Ó£²£Ä£Í£Á¥¿¥°¹½Â¤ÂÎ */
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

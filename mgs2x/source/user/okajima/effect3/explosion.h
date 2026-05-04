/*

	explosion.h
	爆発用
	2001/07/25 S.Okajima
	$Id: explosion.h,v 1.1.1.3 2002/11/19 11:47:30 Yoshizawa1 Exp $

*/



//	 4* 4	 3* 8	  24
//	 8* 8	 7*16	 112
//	16*16	15*32	 480
//	32*32	31*64	1984

//#define	EXPLO_N_SIDES0			(16)	/*  */
//#define	EXPLO_N_SIDES0			(8)	/*  */
#define	EXPLO_N_SIDES0			(4)	/*  */

//#define	EXPLO_N_SIDES1			(32)	/* 円周分割数 */
//#define	EXPLO_N_SIDES1			(16)	/* 円周分割数 */
#define	EXPLO_N_SIDES1			(8)	/* 円周分割数 */

#define	EXPLO_N_LOOP0		(EXPLO_N_SIDES0 - 1)
#define	EXPLO_N_LOOP1		(2 * EXPLO_N_SIDES1)
#define	EXPLO_N_TOTAL		( EXPLO_N_LOOP0 * EXPLO_N_LOOP1 )
#define	EXPLO_N_VERTS		(EXPLO_N_LOOP1)
#define	EXPLO_N_PRIMS		(EXPLO_N_TOTAL/EXPLO_N_VERTS)

#define ANGLE_START	 (0.05f)
#define ANGLE_END	 (0.60f)
#define ANGLE_DIFF	 (ANGLE_END - ANGLE_START)

#define	REBIRTH_NUM		(2)

/*
	mm_orga_def.h
		mm_orga系の定数等の宣言

	2000/4/12 K.Kano
	$Id: mm_orga_def.h,v 1.1.1.3 2002/11/19 11:43:15 Yoshizawa1 Exp $
*/


#ifndef _mm_orga_def_h_
#define _mm_orga_def_h_


#define N_OBJNUM	(sizeof(objnum)/sizeof(objnum[0]))
#define N_STOCK_ARRAYS	1
#define FRAMES		4


/* SPRING_VLIMIT <= SPRING_XLIMITの方が結果が良好 */

#define SPRING_ACC	0.2f
#define SPRING_REDUCE	0.7f /* -0.3f */
#define SPRING_VLIMIT	(CVC2N(2.0f)*4.0f)
#define SPRING_XLIMIT	(CVC2N(5.0f)*4.0f)


struct _scratchpad {
    FMATRIX sm;
    FMATRIX m,tm,tm2;
    FVECTOR q;
    FVECTOR xb,xn;
};


#endif

/********************************************************************************/
/*										*/
/*	里吉汎用計算ヘッダファイル	<sato_util.h>				*/
/*					Hiroki Satoyoshi 2001/2/3		*/
/*	$Id: sato_util.h,v 1.1.1.3 2002/11/19 11:48:27 Yoshizawa1 Exp $		*/
/*										*/
/********************************************************************************/

#ifndef __def_inc_sato_util_h__
#define __def_inc_sato_util_h__

/********************************************************************************/
/*	define      								*/
/********************************************************************************/
#define M_PI 3.14159265358979323846264338327950288419716939937510f
#define		ON	(1)
#define		OFF	(0)

#if defined(PSX2)
#define RAND(_n)	( ( ( BP_PS2_rand() >> 16 ) * (_n) ) >> 15 )
#else
#define RAND(_n)	( (int)( ( (u_long64)rand()*(_n) ) / ((u_long64)RAND_MAX+1) ) )
#endif

#define DumpMATRIX( _px, _py, _m) \
{\
	MENU_Locate( (_px), (_py), 0 ); \
	MENU_Printf( "Mx(%3.5f, %3.5f, %3.5f, %3.5f)",\
		     (_m).m[0][0], (_m).m[0][1], (_m).m[0][2]); \
	MENU_Locate( (_px), (_py)+15, 0 ); \
	MENU_Printf( "My(%3.5f, %3.5f, %3.5f, %3.5f)",\
		     (_m).m[1][0], (_m).m[1][1], (_m).m[1][2]); \
	MENU_Locate( (_px), (_py)+30, 0 ); \
	MENU_Printf( "Mz(%3.5f, %3.5f, %3.5f, %3.5f)",\
		     (_m).m[2][0], (_m).m[2][1], (_m).m[2][2]); \
	MENU_Locate( (_px), (_py)+45, 0 ); \
	MENU_Printf( "Mp(%8.0f, %8.0f, %8.0f, %8.0f)",\
		     (_m).m[3][0], (_m).m[3][1], (_m).m[3][2]); \
}

#define SET_VECTOR( _vec, _vx, _vy, _vz) \
{\
     (_vec).vx = (_vx);			\
     (_vec).vy = (_vy);			\
     (_vec).vz = (_vz);			\
     (_vec).vw = 1.0f;			\
}


/********************************************************************************/
/*	extern    								*/
/********************************************************************************/
#ifdef PSX2 /* yano 2002.03.12 */
extern void View_HermiteRout( FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, float, int);
extern inline void SAT_Minus_FVECTOR(FVECTOR *, FVECTOR *, FVECTOR *);
extern inline void Fvec_from_Matrix(FVECTOR *, FMATRIX *);
extern inline void Dir_from_2Vec(FVECTOR *,FVECTOR *, SVECTOR *);
extern inline void Dir_from_2VecF(FVECTOR *,FVECTOR *, FVECTOR *);
extern void View_HermiteRout( FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, float, int);
extern inline void SetRGBA(int, int, int, int, int *);
extern void get_hermove(FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, float, float);
extern void sat_viewp(FVECTOR* , char, char, char);
extern void sat_viewp2(FVECTOR* point, float length, char r, char g, char b);
extern int setuzokuget(FVECTOR*, FVECTOR*, int);
#else
extern void View_HermiteRout( FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, float, int);
extern void SAT_Minus_FVECTOR(FVECTOR *, FVECTOR *, FVECTOR *);
extern void Fvec_from_Matrix(FVECTOR *, FMATRIX *);
extern void Dir_from_2Vec(FVECTOR *,FVECTOR *, SVECTOR *);
extern void Dir_from_2VecF(FVECTOR *,FVECTOR *, FVECTOR *);
extern void View_HermiteRout( FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, float, int);
extern void SetRGBA(int, int, int, int, int *);
extern void get_hermove(FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, float, float);
extern void sat_viewp(FVECTOR* , char, char, char);
extern void sat_viewp2(FVECTOR* point, float length, char r, char g, char b);
extern int setuzokuget(FVECTOR*, FVECTOR*, int);
#endif

#endif




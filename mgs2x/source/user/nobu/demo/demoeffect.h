/*
	demoeffect.h
	    デモエフェクト定義

	2000/04/07 N.Tanaka
	$Id: demoeffect.h,v 1.1.1.3 2002/11/19 11:46:43 Yoshizawa1 Exp $
*/

#ifndef _INC_DEMOEFFECT_
#define _INC_DEMOEFFECT_    /* #defined if demo_.h has been included */

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

/******************************************************************************
 * included
 */

/******************************************************************************
 * definitions and typedefs and structures
 */

/******************************************************************************
 * exec
 */
extern BOOL ShowEffect(LPMGSDEMOACT lpAct, LPMGSDEMOEFFECT lpEffect, LPMGSDEMOEFFECTACT lpEAct);

typedef void* (*CHARAVOID)(void*);
typedef void* (*CHARAMODEL)(void*, LPMGSDEMOMODEL lpModel);
typedef void* (*CHARAFVECT)(FVECTOR* lpfvecCenter);
typedef void* (*CHARAFMATFVECT)(FMATRIX* lpfmatWorld, FVECTOR* lpfvecCenter);
typedef void* (*CHARAFMAT)(FMATRIX* lpfmatWorld);
typedef void* (*CHARASPARK)(int nPrim, FVECTOR* lpfvectCenter, float fMinSpeed, float fWideSpeed, float fGravity, SVECTOR* lpsvectRot, SVECTOR* lpsvectRotWide, FVECTOR* lpfvectColor, float fLength, int nCount);
typedef void (*CHARACAMERABLOOD)(FVECTOR* lpfvectPos, int nWhite);
typedef void* (*CHARABLOODSPREAD)(FVECTOR* lpfvectPos, SVECTOR* lpsvectRot, int nDecay, float fSize);
typedef void (*CHARAWALLBLOOD)(FVECTOR* lpfvectPos, SVECTOR* lpsvectRot, int nWhite);
typedef void* (*CHARACIRCLELIGHT)(FMATRIX* lpfmatWorld, int* lpnFlag, int nMode);
typedef void* (*CHARAFADEINOUTDEMO)(int nColorR, int nColorG, int nColorB, int nColorA, int nCount);
typedef void* (*CHARADLOODDROPS)(FVECTOR *pos, int decay, int white, int spread_size);
typedef void* (*CHARABODYPLASMADOTDEMO)(DG_OBJS *objs, int model_id, int life);
typedef void* (*CHARACTFLUSH)(int nDecay, int nMode);
typedef void* (*CHARARAINCAMERADEMO)(int nLife);

typedef void* (*CHARASPLASHDEMO)(FVECTOR *origin, FVECTOR *center, float radius, float size, int multiple, int life);
typedef void* (*CHARASPLASHPARTSDEMO)(FVECTOR *center, SVECTOR *rot, float intense);
typedef void* (*CHARASPLASHRIPPLEDEMO)(FVECTOR *center, float radius, float direction, float angle, float size, int multiple, int life);

typedef void* (*CHARASPRITEFOGWORLDDEMO)(FVECTOR *bound0, FVECTOR *bound1, FVECTOR *vec, FVECTOR *col, int tex_name, int size, int life, int mode);
typedef void* (*CHARASPRITEFOGPERSDEMO)(FVECTOR *bound0, FVECTOR *bound1, FVECTOR *vec, FVECTOR *col, int tex_name, int size, int life, int mode);
typedef void* (*CHARASPRITEFOGONCAMERADEMO)(FVECTOR *vec, FVECTOR *col, int tex_name, int size, int life, int mode);

typedef void* (*CHARAOPTICALCAMOUFLAGEBREAKDEMO)(DG_OBJS *objs, int type, int start_time, int end_time, int color);
typedef void* (*CHARACONTRASTDEMO)(int col_u_r, int col_u_g, int col_u_b, int col_d_r, int col_d_g, int col_d_b, int nega_posi_flag, int time);

typedef void* (*CHARATHUNDERDEMO)(float sx, float sy);
typedef void* (*CHARAFOGSETDEMO)(int col_r, int col_g, int col_b, float near, float far, int time);
typedef void* (*CHARASPLASHMOTIONDEMO)(DG_OBJS *objs, float step_limit, int life);

typedef void* (*CHARASPARKDEMO)(FMATRIX *d_world, FVECTOR *d_color);

/******************************************************************************
 */

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

#endif /* _INC_DEMOEFFECT_ */

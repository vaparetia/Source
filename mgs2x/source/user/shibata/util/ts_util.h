/*
	ts_util.h
	いろいろ
	2000/05/17 T.Shibata
	$Id: ts_util.h,v 1.1.1.3 2002/11/19 11:48:55 Yoshizawa1 Exp $
*/

#ifndef TS_UTIL_H
#define	TS_UTIL_H


#define	VERT_KICK_CODE	(0x8fff)
#define	DRAW_KICK_CODE	(0x0fff)

#define	SYS_SCR_BOTTOM	((void*)0x70004000)

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
//extern void	*AN_Test_Eye2( FVECTOR *mov, int size );

extern void TS_Mem_Scr( void *dst, void *src, int size, int num );
extern void TS_Scr_Mem( void *dst, void *src, int size, int num );

extern void TS_RotMatrixZXY( FMATRIX *out_mat, FMATRIX *in_mat, FVECTOR *rot );
extern void TS_RotMatrixZ( FMATRIX *out_mat, FMATRIX *in_mat, float rot );
extern void TS_RotMatrixX( FMATRIX *out_mat, FMATRIX *in_mat, float rot );
extern void TS_RotMatrixY( FMATRIX *out_mat, FMATRIX *in_mat, float rot );

extern void TS_MakeMatrix( FMATRIX *out_mat, FVECTOR *from_vec ,FVECTOR *pos );
extern void TS_MakeMatrix2( FMATRIX *out_mat, FVECTOR *z_vec, FVECTOR *pouter, FVECTOR *pos );
extern void TS_ScaleMatrix( FMATRIX *out_mat, FMATRIX *in_mat, FVECTOR *scale);

extern void vu0_MulVec( FVECTOR *r, FVECTOR *s, FVECTOR *t );
//内分点
//比 s:t
//extern void TS_DividingPoint( FVECTOR *p, FVECTOR *a, FVECTOR *b, float s, float t );

extern void Tra_SvecToFvec(FVECTOR *r,SVECTOR *a);

//マトリクスからＲＯＴを求める
//ＲＯＴ（０から４０９６）
extern int TS_MatToRot( SVECTOR *rot, FMATRIX *mat );
//ベクトルからＲＯＴを求める
//ＲＯＴ（０から４０９６）
extern void TS_VecToRot( SVECTOR *rot, FVECTOR *vec );

extern float _TS_Sin( int );
#define	TS_SINs(_s) _TS_Sin((_s))
#define	TS_COSs(_s) _TS_Sin((_s)+1024)

//ポスとライトからＲＧＢを求める
extern int GetRGBFromLightPos( FVECTOR *pos, FMATRIX *light );
extern int GetRGBFromLightNorm( FVECTOR *norm, FMATRIX *light );
extern DG_PRIM2 *InitStripPrim( int tex_code, int n_strip, short *uv_pix, FVECTOR *init_pos );


#ifndef BP_PS2
#define _sceVu0MulVectorXYZ(_v0,_v1,_v2) \
	{((FVECTOR*)(_v0))->x = ((FVECTOR*)(_v1))->x * ((FVECTOR*)(_v2))->x; \
	 ((FVECTOR*)(_v0))->y = ((FVECTOR*)(_v1))->y * ((FVECTOR*)(_v2))->y; \
	 ((FVECTOR*)(_v0))->z = ((FVECTOR*)(_v1))->z * ((FVECTOR*)(_v2))->z; }
#endif

#endif

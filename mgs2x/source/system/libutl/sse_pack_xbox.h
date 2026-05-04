/*
   sse_pack.h
		XBOX用ベクトル／マトリクス演算ライブラリ (FPU使用)

	1999/08/04 K.Kano
	$Id: sse_pack_xbox.h,v 1.1.1.3 2002/11/19 11:42:59 Yoshizawa1 Exp $
*/



#ifndef _sse_pack_xbox_h_
#define _sse_pack_xbox_h_

extern VECTOR REG_vf1 ;
extern VECTOR REG_vf2 ;
extern VECTOR REG_vf3 ;
extern MATRIX REG_m0 ;
extern MATRIX REG_m1 ;
extern MATRIX REG_m2 ;
extern IVECTOR REG_vi1 ;
extern IVECTOR REG_vi2 ;
extern IVECTOR REG_vi3 ;
extern float REG_Q ;

/* ロード/ストア */
static inline void vu0_Ldv0(const VECTOR * const a)
{
	REG_vf1 = *a ;
}

static inline void vu0_Ldv1(const VECTOR * const a)
{
	REG_vf2 = *a ;
}

static inline void vu0_Ldv2(const VECTOR * const a)
{
	REG_vf3 = *a ;
}

static inline void vu0_LdSVv0(const SVECTOR * const a)
{
	*(int *)&REG_vf1.vx = (int)a->vx ;
	*(int *)&REG_vf1.vy = (int)a->vy ;
	*(int *)&REG_vf1.vz = (int)a->vz ;
}

static inline void vu0_LdSVv1(const SVECTOR * const a)
{
	*(int *)&REG_vf2.vx = (int)a->vx ;
	*(int *)&REG_vf2.vy = (int)a->vy ;
	*(int *)&REG_vf2.vz = (int)a->vz ;
}

static inline void vu0_LdSVv2(const SVECTOR * const a)
{
	*(int *)&REG_vf3.vx = (int)a->vx ;
	*(int *)&REG_vf3.vy = (int)a->vy ;
	*(int *)&REG_vf3.vz = (int)a->vz ;
}

static inline void vu0_Stv0(VECTOR *a)
{
	*a = REG_vf1 ;
}

static inline void vu0_Stv1(VECTOR *a)
{
	*a = REG_vf2 ;
}

static inline void vu0_Stv2(VECTOR *a)
{
	*a = REG_vf3 ;
}

static inline void vu0_StSVv0(SVECTOR *a)
{
	a->vx = (short)*(int*)&REG_vf1.vx ;
	a->vy = (short)*(int*)&REG_vf1.vy ;
	a->vz = (short)*(int*)&REG_vf1.vz ;
}

static inline void vu0_StSVv1(SVECTOR *a)
{
	a->vx = (short)*(int*)&REG_vf2.vx ;
	a->vy = (short)*(int*)&REG_vf2.vy ;
	a->vz = (short)*(int*)&REG_vf2.vz ;
}

static inline void vu0_StSVv2(SVECTOR *a)
{
	a->vx = (short)*(int*)&REG_vf3.vx ;
	a->vy = (short)*(int*)&REG_vf3.vy ;
	a->vz = (short)*(int*)&REG_vf3.vz ;
}

static inline void vu0_Ldm0(const MATRIX * const a)
{
	REG_m0 = *a ;
}

static inline void vu0_Ldm1(const MATRIX * const a)
{
	REG_m1 = *a ;
}

static inline void vu0_Ldm2(const MATRIX * const a)
{
	REG_m2 = *a ;
}


/* new XBOX util */
static inline MATRIX *vu0_Ptrm0( void )
{
	return &REG_m0 ;
}
static inline MATRIX *vu0_Ptrm1( void )
{
	return &REG_m1 ;
}
static inline MATRIX *vu0_Ptrm2( void )
{
	return &REG_m2 ;
}

static inline void vu0_Stm0(MATRIX *a)
{
	*a = REG_m0 ;
}

static inline void vu0_Stm1(MATRIX *a)
{
	*a = REG_m1 ;
}

static inline void vu0_Stm2(MATRIX *a)
{
	*a = REG_m2 ;
}

/* ベクトルを０クリアする */
static inline void vu0_Clrv0(void)
{
	REG_vf1.vx = 0 ;
	REG_vf1.vy = 0 ;
	REG_vf1.vz = 0 ;
	REG_vf1.vw = 0 ;
}

static inline void vu0_Clrv1(void)
{
	REG_vf2.vx = 0 ;
	REG_vf2.vy = 0 ;
	REG_vf2.vz = 0 ;
	REG_vf2.vw = 0 ;
}

static inline void vu0_Clrv2(void)
{
	REG_vf3.vx = 0 ;
	REG_vf3.vy = 0 ;
	REG_vf3.vz = 0 ;
	REG_vf3.vw = 0 ;
}

/* ベクトルの特定要素値を0.0にする */
static inline void vu0_Setv0x0(void)
{
	REG_vf1.vx = 0 ;
}

static inline void vu0_Setv1x0(void)
{
	REG_vf2.vx = 0 ;
}

static inline void vu0_Setv2x0(void)
{
	REG_vf3.vx = 0 ;
}

static inline void vu0_Setv0y0(void)
{
	REG_vf1.vy = 0 ;
}

static inline void vu0_Setv1y0(void)
{
	REG_vf2.vy = 0 ;
}

static inline void vu0_Setv2y0(void)
{
	REG_vf3.vy = 0 ;
}

static inline void vu0_Setv0z0(void)
{
	REG_vf1.vz = 0 ;
}

static inline void vu0_Setv1z0(void)
{
	REG_vf2.vz = 0 ;
}

static inline void vu0_Setv2z0(void)
{
	REG_vf3.vz = 0 ;
}

/* ベクトルのｗ値を、0.0にする */
static inline void vu0_Setv0w0(void)
{
	REG_vf1.vw = 0 ;
}

static inline void vu0_Setv1w0(void)
{
	REG_vf2.vw = 0 ;
}

static inline void vu0_Setv2w0(void)
{
	REG_vf3.vw = 0 ;
}

/* ベクトルのｗ値を、1.0にする */
static inline void vu0_Setv0w1(void)
{
	REG_vf1.vw = 1.0f ;
}

static inline void vu0_Setv1w1(void)
{
	REG_vf2.vw = 1.0f ;
}

static inline void vu0_Setv2w1(void)
{
	REG_vf3.vw = 1.0f ;
}


/* コピー */
static inline void vu0_Cpv0v1(void)
{
	REG_vf2 = REG_vf1 ;
}

static inline void vu0_Cpv0v2(void)
{
	REG_vf3 = REG_vf1 ;
}

static inline void vu0_Cpv1v2(void)
{
	REG_vf3 = REG_vf2 ;
}

static inline void vu0_Cpv1v0(void)
{
	REG_vf1 = REG_vf2 ;
}

static inline void vu0_Cpv2v0(void)
{
	REG_vf1 = REG_vf3 ;
}

static inline void vu0_Cpv2v1(void)
{
	REG_vf2 = REG_vf3 ;
}

static inline void vu0_Cpm0m1(void)
{
	REG_m1 = REG_m0 ;
}

static inline void vu0_Cpm0m2(void)
{
	REG_m2 = REG_m0 ;
}

static inline void vu0_Cpm1m2(void)
{
	REG_m2 = REG_m1 ;
}

static inline void vu0_Cpm1m0(void)
{
	REG_m0 = REG_m1 ;
}

static inline void vu0_Cpm2m0(void)
{
	REG_m0 = REG_m2 ;
}

static inline void vu0_Cpm2m1(void)
{
	REG_m1 = REG_m2 ;
}

/* 演算 */

/* 加算 */
static inline void vu0_Addv0v1(void)
{
	REG_vf1.vx = REG_vf1.vx + REG_vf2.vx ;
	REG_vf1.vy = REG_vf1.vy + REG_vf2.vy ;
	REG_vf1.vz = REG_vf1.vz + REG_vf2.vz ;
}

static inline void vu0_Addv0v2(void)
{
	REG_vf1.vx = REG_vf1.vx + REG_vf3.vx ;
	REG_vf1.vy = REG_vf1.vy + REG_vf3.vy ;
	REG_vf1.vz = REG_vf1.vz + REG_vf3.vz ;
}

static inline void vu0_Addv1v2(void)
{
	REG_vf2.vx = REG_vf2.vx + REG_vf3.vx ;
	REG_vf2.vy = REG_vf2.vy + REG_vf3.vy ;
	REG_vf2.vz = REG_vf2.vz + REG_vf3.vz ;
}

static inline void vu0_Addv1v0(void)
{
	REG_vf2.vx = REG_vf2.vx + REG_vf1.vx ;
	REG_vf2.vy = REG_vf2.vy + REG_vf1.vy ;
	REG_vf2.vz = REG_vf2.vz + REG_vf1.vz ;
}

static inline void vu0_Addv2v0(void)
{
	REG_vf3.vx = REG_vf3.vx + REG_vf1.vx ;
	REG_vf3.vy = REG_vf3.vy + REG_vf1.vy ;
	REG_vf3.vz = REG_vf3.vz + REG_vf1.vz ;
}

static inline void vu0_Addv2v1(void)
{
	REG_vf3.vx = REG_vf3.vx + REG_vf2.vx ;
	REG_vf3.vy = REG_vf3.vy + REG_vf2.vy ;
	REG_vf3.vz = REG_vf3.vz + REG_vf2.vz ;
}

static inline void vu0_Addv2v0v1(void)
{
	REG_vf3.vx = REG_vf1.vx + REG_vf2.vx ;
	REG_vf3.vy = REG_vf1.vy + REG_vf2.vy ;
	REG_vf3.vz = REG_vf1.vz + REG_vf2.vz ;
}

static inline void vu0_Addv1v0v1(void)
{
	REG_vf2.vx = REG_vf1.vx + REG_vf2.vx ;
	REG_vf2.vy = REG_vf1.vy + REG_vf2.vy ;
	REG_vf2.vz = REG_vf1.vz + REG_vf2.vz ;
}

static inline void vu0_Addv1v0v2(void)
{
	REG_vf2.vx = REG_vf1.vx + REG_vf3.vx ;
	REG_vf2.vy = REG_vf1.vy + REG_vf3.vy ;
	REG_vf2.vz = REG_vf1.vz + REG_vf3.vz ;
}

static inline void vu0_Addv2v0v2(void)
{
	REG_vf3.vx = REG_vf1.vx + REG_vf3.vx ;
	REG_vf3.vy = REG_vf1.vy + REG_vf3.vy ;
	REG_vf3.vz = REG_vf1.vz + REG_vf3.vz ;
}

static inline void vu0_Addv0v1v2(void)
{
	REG_vf1.vx = REG_vf2.vx + REG_vf3.vx ;
	REG_vf1.vy = REG_vf2.vy + REG_vf3.vy ;
	REG_vf1.vz = REG_vf2.vz + REG_vf3.vz ;
}

static inline void vu0_Addv2v1v2(void)
{
	REG_vf3.vx = REG_vf2.vx + REG_vf3.vx ;
	REG_vf3.vy = REG_vf2.vy + REG_vf3.vy ;
	REG_vf3.vz = REG_vf2.vz + REG_vf3.vz ;
}

static inline void vu0_Addv2v1v0(void)
{
	REG_vf3.vx = REG_vf2.vx + REG_vf1.vx ;
	REG_vf3.vy = REG_vf2.vy + REG_vf1.vy ;
	REG_vf3.vz = REG_vf2.vz + REG_vf1.vz ;
}

static inline void vu0_Addv0v1v0(void)
{
	REG_vf1.vx = REG_vf2.vx + REG_vf1.vx ;
	REG_vf1.vy = REG_vf2.vy + REG_vf1.vy ;
	REG_vf1.vz = REG_vf2.vz + REG_vf1.vz ;
}

static inline void vu0_Addv1v2v0(void)
{
	REG_vf2.vx = REG_vf3.vx + REG_vf1.vx ;
	REG_vf2.vy = REG_vf3.vy + REG_vf1.vy ;
	REG_vf2.vz = REG_vf3.vz + REG_vf1.vz ;
}

static inline void vu0_Addv0v2v0(void)
{
	REG_vf1.vx = REG_vf3.vx + REG_vf1.vx ;
	REG_vf1.vy = REG_vf3.vy + REG_vf1.vy ;
	REG_vf1.vz = REG_vf3.vz + REG_vf1.vz ;
}

static inline void vu0_Addv0v2v1(void)
{
	REG_vf1.vx = REG_vf3.vx + REG_vf2.vx ;
	REG_vf1.vy = REG_vf3.vy + REG_vf2.vy ;
	REG_vf1.vz = REG_vf3.vz + REG_vf2.vz ;
}

static inline void vu0_Addv1v2v1(void)
{
	REG_vf2.vx = REG_vf3.vx + REG_vf2.vx ;
	REG_vf2.vy = REG_vf3.vy + REG_vf2.vy ;
	REG_vf2.vz = REG_vf3.vz + REG_vf2.vz ;
}

/* 減算 */
static inline void vu0_Subv0v1(void)
{
	REG_vf1.vx = REG_vf1.vx - REG_vf2.vx ;
	REG_vf1.vy = REG_vf1.vy - REG_vf2.vy ;
	REG_vf1.vz = REG_vf1.vz - REG_vf2.vz ;
}

static inline void vu0_Subv0v2(void)
{
	REG_vf1.vx = REG_vf1.vx - REG_vf3.vx ;
	REG_vf1.vy = REG_vf1.vy - REG_vf3.vy ;
	REG_vf1.vz = REG_vf1.vz - REG_vf3.vz ;
}

static inline void vu0_Subv1v2(void)
{
	REG_vf2.vx = REG_vf2.vx - REG_vf3.vx ;
	REG_vf2.vy = REG_vf2.vy - REG_vf3.vy ;
	REG_vf2.vz = REG_vf2.vz - REG_vf3.vz ;
}

static inline void vu0_Subv1v0(void)
{
	REG_vf2.vx = REG_vf2.vx - REG_vf1.vx ;
	REG_vf2.vy = REG_vf2.vy - REG_vf1.vy ;
	REG_vf2.vz = REG_vf2.vz - REG_vf1.vz ;
}

static inline void vu0_Subv2v0(void)
{
	REG_vf3.vx = REG_vf3.vx - REG_vf1.vx ;
	REG_vf3.vy = REG_vf3.vy - REG_vf1.vy ;
	REG_vf3.vz = REG_vf3.vz - REG_vf1.vz ;
}

static inline void vu0_Subv2v1(void)
{
	REG_vf3.vx = REG_vf3.vx - REG_vf2.vx ;
	REG_vf3.vy = REG_vf3.vy - REG_vf2.vy ;
	REG_vf3.vz = REG_vf3.vz - REG_vf2.vz ;
}

static inline void vu0_Subv2v0v1(void)
{
	REG_vf3.vx = REG_vf1.vx - REG_vf2.vx ;
	REG_vf3.vy = REG_vf1.vy - REG_vf2.vy ;
	REG_vf3.vz = REG_vf1.vz - REG_vf2.vz ;
}

static inline void vu0_Subv1v0v1(void)
{
	REG_vf2.vx = REG_vf1.vx - REG_vf2.vx ;
	REG_vf2.vy = REG_vf1.vy - REG_vf2.vy ;
	REG_vf2.vz = REG_vf1.vz - REG_vf2.vz ;
}

static inline void vu0_Subv1v0v2(void)
{
	REG_vf2.vx = REG_vf1.vx - REG_vf3.vx ;
	REG_vf2.vy = REG_vf1.vy - REG_vf3.vy ;
	REG_vf2.vz = REG_vf1.vz - REG_vf3.vz ;
}

static inline void vu0_Subv2v0v2(void)
{
	REG_vf3.vx = REG_vf1.vx - REG_vf3.vx ;
	REG_vf3.vy = REG_vf1.vy - REG_vf3.vy ;
	REG_vf3.vz = REG_vf1.vz - REG_vf3.vz ;
}

static inline void vu0_Subv0v1v2(void)
{
	REG_vf1.vx = REG_vf2.vx - REG_vf3.vx ;
	REG_vf1.vy = REG_vf2.vy - REG_vf3.vy ;
	REG_vf1.vz = REG_vf2.vz - REG_vf3.vz ;
}

static inline void vu0_Subv2v1v2(void)
{
	REG_vf3.vx = REG_vf2.vx - REG_vf3.vx ;
	REG_vf3.vy = REG_vf2.vy - REG_vf3.vy ;
	REG_vf3.vz = REG_vf2.vz - REG_vf3.vz ;
}

static inline void vu0_Subv2v1v0(void)
{
	REG_vf3.vx = REG_vf2.vx - REG_vf1.vx ;
	REG_vf3.vy = REG_vf2.vy - REG_vf1.vy ;
	REG_vf3.vz = REG_vf2.vz - REG_vf1.vz ;
}

static inline void vu0_Subv0v1v0(void)
{
	REG_vf1.vx = REG_vf2.vx - REG_vf1.vx ;
	REG_vf1.vy = REG_vf2.vy - REG_vf1.vy ;
	REG_vf1.vz = REG_vf2.vz - REG_vf1.vz ;
}

static inline void vu0_Subv1v2v0(void)
{
	REG_vf2.vx = REG_vf3.vx - REG_vf1.vx ;
	REG_vf2.vy = REG_vf3.vy - REG_vf1.vy ;
	REG_vf2.vz = REG_vf3.vz - REG_vf1.vz ;
}

static inline void vu0_Subv0v2v0(void)
{
	REG_vf1.vx = REG_vf3.vx - REG_vf1.vx ;
	REG_vf1.vy = REG_vf3.vy - REG_vf1.vy ;
	REG_vf1.vz = REG_vf3.vz - REG_vf1.vz ;
}

static inline void vu0_Subv0v2v1(void)
{
	REG_vf1.vx = REG_vf3.vx - REG_vf2.vx ;
	REG_vf1.vy = REG_vf3.vy - REG_vf2.vy ;
	REG_vf1.vz = REG_vf3.vz - REG_vf2.vz ;
}

static inline void vu0_Subv1v2v1(void)
{
	REG_vf2.vx = REG_vf3.vx - REG_vf2.vx ;
	REG_vf2.vy = REG_vf3.vy - REG_vf2.vy ;
	REG_vf2.vz = REG_vf3.vz - REG_vf2.vz ;
}

/* 乗算 */
static inline void vu0_Mulv0a(const float a)
{
	REG_vf1.vx = REG_vf1.vx * a ;
	REG_vf1.vy = REG_vf1.vy * a ;
	REG_vf1.vz = REG_vf1.vz * a ;
}

static inline void vu0_Mulv1a(const float a)
{
	REG_vf2.vx = REG_vf2.vx * a ;
	REG_vf2.vy = REG_vf2.vy * a ;
	REG_vf2.vz = REG_vf2.vz * a ;
}

static inline void vu0_Mulv2a(const float a)
{
	REG_vf3.vx = REG_vf3.vx * a ;
	REG_vf3.vy = REG_vf3.vy * a ;
	REG_vf3.vz = REG_vf3.vz * a ;
}

static inline void vu0_Mulv1v0a(const float a)
{
	REG_vf2.vx = REG_vf1.vx * a ;
	REG_vf2.vy = REG_vf1.vy * a ;
	REG_vf2.vz = REG_vf1.vz * a ;
}

static inline void vu0_Mulv2v0a(const float a)
{
	REG_vf3.vx = REG_vf1.vx * a ;
	REG_vf3.vy = REG_vf1.vy * a ;
	REG_vf3.vz = REG_vf1.vz * a ;
}

static inline void vu0_Mulv0v1a(const float a)
{
	REG_vf1.vx = REG_vf2.vx * a ;
	REG_vf1.vy = REG_vf2.vy * a ;
	REG_vf1.vz = REG_vf2.vz * a ;
}

static inline void vu0_Mulv2v1a(const float a)
{
	REG_vf3.vx = REG_vf2.vx * a ;
	REG_vf3.vy = REG_vf2.vy * a ;
	REG_vf3.vz = REG_vf2.vz * a ;
}

static inline void vu0_Mulv0v2a(const float a)
{
	REG_vf1.vx = REG_vf3.vx * a ;
	REG_vf1.vy = REG_vf3.vy * a ;
	REG_vf1.vz = REG_vf3.vz * a ;
}

static inline void vu0_Mulv1v2a(const float a)
{
	REG_vf2.vx = REG_vf3.vx * a ;
	REG_vf2.vy = REG_vf3.vy * a ;
	REG_vf2.vz = REG_vf3.vz * a ;
}

static inline void vu0_Mulv0Q(void)
{
	REG_vf1.vx = REG_vf1.vx * REG_Q ;
	REG_vf1.vy = REG_vf1.vy * REG_Q ;
	REG_vf1.vz = REG_vf1.vz * REG_Q ;
}

static inline void vu0_Mulv1Q(void)
{
	REG_vf2.vx = REG_vf2.vx * REG_Q ;
	REG_vf2.vy = REG_vf2.vy * REG_Q ;
	REG_vf2.vz = REG_vf2.vz * REG_Q ;
}

static inline void vu0_Mulv2Q(void)
{
	REG_vf3.vx = REG_vf3.vx * REG_Q ;
	REG_vf3.vy = REG_vf3.vy * REG_Q ;
	REG_vf3.vz = REG_vf3.vz * REG_Q ;
}

static inline void vu0_Mulv1v0Q(void)
{
	REG_vf2.vx = REG_vf1.vx * REG_Q ;
	REG_vf2.vy = REG_vf1.vy * REG_Q ;
	REG_vf2.vz = REG_vf1.vz * REG_Q ;
}

static inline void vu0_Mulv2v0Q(void)
{
	REG_vf3.vx = REG_vf1.vx * REG_Q ;
	REG_vf3.vy = REG_vf1.vy * REG_Q ;
	REG_vf3.vz = REG_vf1.vz * REG_Q ;
}

static inline void vu0_Mulv0v1Q(void)
{
	REG_vf1.vx = REG_vf2.vx * REG_Q ;
	REG_vf1.vy = REG_vf2.vy * REG_Q ;
	REG_vf1.vz = REG_vf2.vz * REG_Q ;
}

static inline void vu0_Mulv2v1Q(void)
{
	REG_vf3.vx = REG_vf2.vx * REG_Q ;
	REG_vf3.vy = REG_vf2.vy * REG_Q ;
	REG_vf3.vz = REG_vf2.vz * REG_Q ;
}

static inline void vu0_Mulv0v2Q(void)
{
	REG_vf1.vx = REG_vf3.vx * REG_Q ;
	REG_vf1.vy = REG_vf3.vy * REG_Q ;
	REG_vf1.vz = REG_vf3.vz * REG_Q ;
}

static inline void vu0_Mulv1v2Q(void)
{
	REG_vf2.vx = REG_vf3.vx * REG_Q ;
	REG_vf2.vy = REG_vf3.vy * REG_Q ;
	REG_vf2.vz = REG_vf3.vz * REG_Q ;
}

/* 除算 */
static inline void vu0_DivQab(const float a,const float b)
{
	ASSERT( b== 0.0f ) ;
	REG_Q = a / b ;
}

/* 平方根 */
static inline void vu0_SqrtQ(const float a)
{
	REG_Q = bp_sqrtf( a ) ; //BP_MATH - emulate PS2 sqrtf
}

/* 平方根の逆数  q = b/sqrt(a) */
static inline void vu0_RsqrtQ(const float a,const float b)
{
   float sqr = bp_sqrtf( a ) ;   //BP_MATH - emulate PS2 sqrtf

	if( sqr == 0.0f ) sqr = 0.0000001f;
	REG_Q = b / sqr ;
}

static inline void vu0_WaitQ(void)
{
}

/* 内積 */
static inline float vu0_InnerProductv0v1(void)
{
    return REG_vf1.vx * REG_vf2.vx + REG_vf1.vy * REG_vf2.vy + REG_vf1.vz * REG_vf2.vz ;
}

static inline float vu0_InnerProductv0v2(void)
{
    return REG_vf1.vx * REG_vf3.vx + REG_vf1.vy * REG_vf3.vy + REG_vf1.vz * REG_vf3.vz ;
}

static inline float vu0_InnerProductv1v2(void)
{
    return REG_vf2.vx * REG_vf3.vx + REG_vf2.vy * REG_vf3.vy + REG_vf2.vz * REG_vf3.vz ;
}

/* ベクトルの長さの平方 */
static inline float vu0_VectorLength2v0(void)
{
    return REG_vf1.vx * REG_vf1.vx + REG_vf1.vy * REG_vf1.vy + REG_vf1.vz * REG_vf1.vz ;
}

static inline float vu0_VectorLength2v1(void)
{
    return REG_vf2.vx * REG_vf2.vx + REG_vf2.vy * REG_vf2.vy + REG_vf2.vz * REG_vf2.vz ;
}

static inline float vu0_VectorLength2v2(void)
{
    return REG_vf3.vx * REG_vf3.vx + REG_vf3.vy * REG_vf3.vy + REG_vf3.vz * REG_vf3.vz ;
}

/* ベクトルの正規化する */
static inline void vu0_VectorNormalv0(void)
{
	float len ;

	len = vu0_VectorLength2v0() ;
	len = 1.0f / bp_sqrtf( len ) ;   //BP_MATH - emulate PS2 sqrtf
    REG_vf1.vx = REG_vf1.vx * len ;
    REG_vf1.vy = REG_vf1.vy * len ;
    REG_vf1.vz = REG_vf1.vz * len ;
}

static inline void vu0_VectorNormalv1(void)
{
	float len ;

	len = vu0_VectorLength2v1() ;
	len = 1.0f / bp_sqrtf( len ) ;   //BP_MATH - emulate PS2 sqrtf
    REG_vf1.vx = REG_vf1.vx * len ;
    REG_vf1.vy = REG_vf1.vy * len ;
    REG_vf1.vz = REG_vf1.vz * len ;
}

static inline void vu0_VectorNormalv2(void)
{
	float len ;

	len = vu0_VectorLength2v2() ;
	len = 1.0f / bp_sqrtf( len ) ;   //BP_MATH - emulate PS2 sqrtf
    REG_vf1.vx = REG_vf1.vx * len ;
    REG_vf1.vy = REG_vf1.vy * len ;
    REG_vf1.vz = REG_vf1.vz * len ;
}

/* 外積 */
static inline void vu0_OuterProductv0v1(void)
{
	VECTOR v ;
    v.vx =  REG_vf1.vy * REG_vf2.vz -  REG_vf1.vz * REG_vf2.vy ;
    v.vy =  REG_vf1.vz * REG_vf2.vx -  REG_vf1.vx * REG_vf2.vz ;
    v.vz =  REG_vf1.vx * REG_vf2.vy -  REG_vf1.vy * REG_vf2.vx ;
	REG_vf1 = v ;
}

static inline void vu0_OuterProductv0v2(void)
{
	VECTOR v ;
    v.vx =  REG_vf1.vy * REG_vf3.vz -  REG_vf1.vz * REG_vf3.vy ;
    v.vy =  REG_vf1.vz * REG_vf3.vx -  REG_vf1.vx * REG_vf3.vz ;
    v.vz =  REG_vf1.vx * REG_vf3.vy -  REG_vf1.vy * REG_vf3.vx ;
	REG_vf1 = v ;
}

static inline void vu0_OuterProductv1v2(void)
{
	VECTOR v ;
    v.vx =  REG_vf2.vy * REG_vf3.vz -  REG_vf2.vz * REG_vf3.vy ;
    v.vy =  REG_vf2.vz * REG_vf3.vx -  REG_vf2.vx * REG_vf3.vz ;
    v.vz =  REG_vf2.vx * REG_vf3.vy -  REG_vf2.vy * REG_vf3.vx ;
	REG_vf2 = v ;
}

static inline void vu0_OuterProductv1v0(void)
{
	VECTOR v ;
    v.vx =  REG_vf2.vy * REG_vf1.vz -  REG_vf2.vz * REG_vf1.vy ;
    v.vy =  REG_vf2.vz * REG_vf1.vx -  REG_vf2.vx * REG_vf1.vz ;
    v.vz =  REG_vf2.vx * REG_vf1.vy -  REG_vf2.vy * REG_vf1.vx ;
	REG_vf2 = v ;
}

static inline void vu0_OuterProductv2v0(void)
{
	VECTOR v ;
    v.vx =  REG_vf3.vy * REG_vf1.vz -  REG_vf3.vz * REG_vf1.vy ;
    v.vy =  REG_vf3.vz * REG_vf1.vx -  REG_vf3.vx * REG_vf1.vz ;
    v.vz =  REG_vf3.vx * REG_vf1.vy -  REG_vf3.vy * REG_vf1.vx ;
	REG_vf3 = v ;
}

static inline void vu0_OuterProductv2v1(void)
{
	VECTOR v ;
    v.vx =  REG_vf3.vy * REG_vf2.vz -  REG_vf3.vz * REG_vf2.vy ;
    v.vy =  REG_vf3.vz * REG_vf2.vx -  REG_vf3.vx * REG_vf2.vz ;
    v.vz =  REG_vf3.vx * REG_vf2.vy -  REG_vf3.vy * REG_vf2.vx ;
	REG_vf3 = v ;
}

static inline void vu0_OuterProductv2v0v1(void)
{
	VECTOR v ;
    v.vx =  REG_vf1.vy * REG_vf2.vz -  REG_vf1.vz * REG_vf2.vy ;
    v.vy =  REG_vf1.vz * REG_vf2.vx -  REG_vf1.vx * REG_vf2.vz ;
    v.vz =  REG_vf1.vx * REG_vf2.vy -  REG_vf1.vy * REG_vf2.vx ;
	REG_vf3 = v ;
}

static inline void vu0_OuterProductv1v0v1(void)
{
	VECTOR v ;
    v.vx =  REG_vf1.vy * REG_vf2.vz -  REG_vf1.vz * REG_vf2.vy ;
    v.vy =  REG_vf1.vz * REG_vf2.vx -  REG_vf1.vx * REG_vf2.vz ;
    v.vz =  REG_vf1.vx * REG_vf2.vy -  REG_vf1.vy * REG_vf2.vx ;
	REG_vf2 = v ;
}

static inline void vu0_OuterProductv1v0v2(void)
{
	VECTOR v ;
    v.vx =  REG_vf1.vy * REG_vf3.vz -  REG_vf1.vz * REG_vf3.vy ;
    v.vy =  REG_vf1.vz * REG_vf3.vx -  REG_vf1.vx * REG_vf3.vz ;
    v.vz =  REG_vf1.vx * REG_vf3.vy -  REG_vf1.vy * REG_vf3.vx ;
	REG_vf2 = v ;
}

static inline void vu0_OuterProductv2v0v2(void)
{
	VECTOR v ;
    v.vx =  REG_vf1.vy * REG_vf2.vz -  REG_vf1.vz * REG_vf2.vy ;
    v.vy =  REG_vf1.vz * REG_vf2.vx -  REG_vf1.vx * REG_vf2.vz ;
    v.vz =  REG_vf1.vx * REG_vf2.vy -  REG_vf1.vy * REG_vf2.vx ;
	REG_vf3 = v ;
}

static inline void vu0_OuterProductv0v1v2(void)
{
	VECTOR v ;
    v.vx =  REG_vf2.vy * REG_vf3.vz -  REG_vf2.vz * REG_vf3.vy ;
    v.vy =  REG_vf2.vz * REG_vf3.vx -  REG_vf2.vx * REG_vf3.vz ;
    v.vz =  REG_vf2.vx * REG_vf3.vy -  REG_vf2.vy * REG_vf3.vx ;
	REG_vf1 = v ;
}

static inline void vu0_OuterProductv2v1v2(void)
{
	VECTOR v ;
    v.vx =  REG_vf2.vy * REG_vf3.vz -  REG_vf2.vz * REG_vf3.vy ;
    v.vy =  REG_vf2.vz * REG_vf3.vx -  REG_vf2.vx * REG_vf3.vz ;
    v.vz =  REG_vf2.vx * REG_vf3.vy -  REG_vf2.vy * REG_vf3.vx ;
	REG_vf3 = v ;
}

static inline void vu0_OuterProductv2v1v0(void)
{
	VECTOR v ;
    v.vx =  REG_vf2.vy * REG_vf1.vz -  REG_vf2.vz * REG_vf1.vy ;
    v.vy =  REG_vf2.vz * REG_vf1.vx -  REG_vf2.vx * REG_vf1.vz ;
    v.vz =  REG_vf2.vx * REG_vf1.vy -  REG_vf2.vy * REG_vf1.vx ;
	REG_vf3 = v ;
}

static inline void vu0_OuterProductv0v1v0(void)
{
	VECTOR v ;
    v.vx =  REG_vf2.vy * REG_vf1.vz -  REG_vf2.vz * REG_vf1.vy ;
    v.vy =  REG_vf2.vz * REG_vf1.vx -  REG_vf2.vx * REG_vf1.vz ;
    v.vz =  REG_vf2.vx * REG_vf1.vy -  REG_vf2.vy * REG_vf1.vx ;
	REG_vf1 = v ;
}

static inline void vu0_OuterProductv1v2v0(void)
{
	VECTOR v ;
    v.vx =  REG_vf3.vy * REG_vf1.vz -  REG_vf3.vz * REG_vf1.vy ;
    v.vy =  REG_vf3.vz * REG_vf1.vx -  REG_vf3.vx * REG_vf1.vz ;
    v.vz =  REG_vf3.vx * REG_vf1.vy -  REG_vf3.vy * REG_vf1.vx ;
	REG_vf2 = v ;
}

static inline void vu0_OuterProductv0v2v0(void)
{
	VECTOR v ;
    v.vx =  REG_vf3.vy * REG_vf1.vz -  REG_vf3.vz * REG_vf1.vy ;
    v.vy =  REG_vf3.vz * REG_vf1.vx -  REG_vf3.vx * REG_vf1.vz ;
    v.vz =  REG_vf3.vx * REG_vf1.vy -  REG_vf3.vy * REG_vf1.vx ;
	REG_vf1 = v ;
}

static inline void vu0_OuterProductv0v2v1(void)
{
	VECTOR v ;
    v.vx =  REG_vf3.vy * REG_vf2.vz -  REG_vf3.vz * REG_vf2.vy ;
    v.vy =  REG_vf3.vz * REG_vf2.vx -  REG_vf3.vx * REG_vf2.vz ;
    v.vz =  REG_vf3.vx * REG_vf2.vy -  REG_vf3.vy * REG_vf2.vx ;
	REG_vf1 = v ;
}

static inline void vu0_OuterProductv1v2v1(void)
{
	VECTOR v ;
    v.vx =  REG_vf3.vy * REG_vf2.vz -  REG_vf3.vz * REG_vf2.vy ;
    v.vy =  REG_vf3.vz * REG_vf2.vx -  REG_vf3.vx * REG_vf2.vz ;
    v.vz =  REG_vf3.vx * REG_vf2.vy -  REG_vf3.vy * REG_vf2.vx ;
	REG_vf2 = v ;
}

/* 各要素を自乗したベクトルを出す */
static inline void vu0_VectorPow2v0(void)
{
    REG_vf1.vx =  REG_vf1.vx * REG_vf1.vx ;
    REG_vf1.vy =  REG_vf1.vy * REG_vf1.vy ;
    REG_vf1.vz =  REG_vf1.vz * REG_vf1.vz ;
}

static inline void vu0_VectorPow2v1(void)
{
    REG_vf2.vx =  REG_vf2.vx * REG_vf2.vx ;
    REG_vf2.vy =  REG_vf2.vy * REG_vf2.vy ;
    REG_vf2.vz =  REG_vf2.vz * REG_vf2.vz ;
}

static inline void vu0_VectorPow2v2(void)
{
    REG_vf3.vx =  REG_vf3.vx * REG_vf3.vx ;
    REG_vf3.vy =  REG_vf3.vy * REG_vf3.vy ;
    REG_vf3.vz =  REG_vf3.vz * REG_vf3.vz ;
}

/* 楕円球の当たり用のパラメータを算出する
   (a, b, c, 1.0) -> (bc, ca, ab, abc) */
static inline void vu0_MakeOvalParamsv0(void)
{
	FVECTOR v ;

	v.vx = REG_vf1.vy*REG_vf1.vz ;
	v.vy = REG_vf1.vx*REG_vf1.vz ;
	v.vz = REG_vf1.vx*REG_vf1.vy ;
	v.vw = REG_vf1.vx*REG_vf1.vy*REG_vf1.vz ;

	REG_vf1 = v ;
}

static inline void vu0_MakeOvalParamsv1(void)
{
	FVECTOR v ;

	v.vx = REG_vf2.vy*REG_vf2.vz ;
	v.vy = REG_vf2.vx*REG_vf2.vz ;
	v.vz = REG_vf2.vx*REG_vf2.vy ;
	v.vw = REG_vf2.vx*REG_vf2.vy*REG_vf2.vz ;

	REG_vf2 = v ;
}

static inline void vu0_MakeOvalParamsv2(void)
{
	FVECTOR v ;

	v.vx = REG_vf3.vy*REG_vf3.vz ;
	v.vy = REG_vf3.vx*REG_vf3.vz ;
	v.vz = REG_vf3.vx*REG_vf3.vy ;
	v.vw = REG_vf3.vx*REG_vf3.vy*REG_vf3.vz ;

	REG_vf3 = v ;
}

/* マトリクス 掛け合わせる a = m*b */
static inline void vu0_ApplyMatrix( VECTOR *a, MATRIX *m, VECTOR *b )
{
	_sceVu0ApplyMatrix( a, m, b ) ;
}
static inline void vu0_ApplyMatrixXYZ( VECTOR *a, MATRIX *m, VECTOR *b )
{
	VECTOR v = *b ;

	v.vw = 1.0f ;
	_sceVu0ApplyMatrix( a, m, &v ) ;
}
static inline void vu0_ApplyMatrixROT( VECTOR *a, MATRIX *m, VECTOR *b )
{
	VECTOR v = *b ;

	v.vw = 0.0f ;
	_sceVu0ApplyMatrix( a, m, &v ) ;
}

/* マトリクス同士を掛け合わせる */
static inline void vu0_Mulm2m0m1(void)
{
#if 1
	_sceVu0MulMatrix( &REG_m2, &REG_m0, &REG_m1 ) ;
#else
	int  i ;

	for ( i=4 ; --i>=0 ; )
	{
		vu0_ApplyMatrix( (VECTOR *)REG_m2.m[i], &REG_m0, (VECTOR *)REG_m1.m[i] ) ;
	}
#endif
}

static inline void vu0_Mulm2m1m0(void)
{
#if 1
	_sceVu0MulMatrix( &REG_m2, &REG_m1, &REG_m0 ) ;
#else
	int  i ;

	for ( i=4 ; --i>=0 ; )
	{
		vu0_ApplyMatrix( (VECTOR *)REG_m2.m[i], &REG_m1, (VECTOR *)REG_m0.m[i] ) ;
	}
#endif
}

static inline void vu0_Mulm1m0m2(void)
{
#if 1
	_sceVu0MulMatrix( &REG_m1, &REG_m0, &REG_m2 ) ;
#else
	int  i ;

	for ( i=4 ; --i>=0 ; )
	{
		vu0_ApplyMatrix( (VECTOR *)REG_m1.m[i], &REG_m0, (VECTOR *)REG_m2.m[i] ) ;
	}
#endif
}

static inline void vu0_Mulm1m2m0(void)
{
#if 1
	_sceVu0MulMatrix( &REG_m1, &REG_m2, &REG_m0 ) ;
#else
	int  i ;

	for ( i=4 ; --i>=0 ; )
	{
		vu0_ApplyMatrix( (VECTOR *)REG_m1.m[i], &REG_m2, (VECTOR *)REG_m0.m[i] ) ;
	}
#endif
}

static inline void vu0_Mulm0m1m2(void)
{
#if 1
	_sceVu0MulMatrix( &REG_m0, &REG_m1, &REG_m2 ) ;
#else
	int  i ;

	for ( i=4 ; --i>=0 ; )
	{
		vu0_ApplyMatrix( (VECTOR *)REG_m0.m[i], &REG_m1, (VECTOR *)REG_m2.m[i] ) ;
	}
#endif
}

static inline void vu0_Mulm0m2m1(void)
{
#if 1
	_sceVu0MulMatrix( &REG_m0, &REG_m2, &REG_m1 ) ;
#else
	int  i ;

	for ( i=4 ; --i>=0 ; )
	{
		vu0_ApplyMatrix( (VECTOR *)REG_m0.m[i], &REG_m2, (VECTOR *)REG_m1.m[i] ) ;
	}
#endif
}

/* マトリクスとベクトルを掛け合わせる */
static inline void vu0_Mulv0m0v0(void)
{
	vu0_ApplyMatrix( &REG_vf1, &REG_m0, &REG_vf1 ) ;
}

static inline void vu0_Mulv1m0v0(void)
{
	vu0_ApplyMatrix( &REG_vf2, &REG_m0, &REG_vf1 ) ;
}

static inline void vu0_Mulv2m0v0(void)
{
	vu0_ApplyMatrix( &REG_vf3, &REG_m0, &REG_vf1 ) ;
}

static inline void vu0_Mulv0m0v1(void)
{
	vu0_ApplyMatrix( &REG_vf1, &REG_m0, &REG_vf2 ) ;
}

static inline void vu0_Mulv1m0v1(void)
{
	vu0_ApplyMatrix( &REG_vf2, &REG_m0, &REG_vf2 ) ;
}

static inline void vu0_Mulv2m0v1(void)
{
	vu0_ApplyMatrix( &REG_vf3, &REG_m0, &REG_vf2 ) ;
}

static inline void vu0_Mulv0m0v2(void)
{
	vu0_ApplyMatrix( &REG_vf1, &REG_m0, &REG_vf3 ) ;
}

static inline void vu0_Mulv1m0v2(void)
{
	vu0_ApplyMatrix( &REG_vf2, &REG_m0, &REG_vf3 ) ;
}

static inline void vu0_Mulv2m0v2(void)
{
	vu0_ApplyMatrix( &REG_vf3, &REG_m0, &REG_vf3 ) ;
}

static inline void vu0_Mulv0m1v0(void)
{
	vu0_ApplyMatrix( &REG_vf1, &REG_m1, &REG_vf1 ) ;
}

static inline void vu0_Mulv1m1v0(void)
{
	vu0_ApplyMatrix( &REG_vf2, &REG_m1, &REG_vf1 ) ;
}

static inline void vu0_Mulv2m1v0(void)
{
	vu0_ApplyMatrix( &REG_vf3, &REG_m1, &REG_vf1 ) ;
}

static inline void vu0_Mulv0m1v1(void)
{
	vu0_ApplyMatrix( &REG_vf1, &REG_m1, &REG_vf2 ) ;
}

static inline void vu0_Mulv1m1v1(void)
{
	vu0_ApplyMatrix( &REG_vf2, &REG_m1, &REG_vf2 ) ;
}

static inline void vu0_Mulv2m1v1(void)
{
	vu0_ApplyMatrix( &REG_vf3, &REG_m1, &REG_vf2 ) ;
}

static inline void vu0_Mulv0m1v2(void)
{
	vu0_ApplyMatrix( &REG_vf1, &REG_m1, &REG_vf3 ) ;
}

static inline void vu0_Mulv1m1v2(void)
{
	vu0_ApplyMatrix( &REG_vf2, &REG_m1, &REG_vf3 ) ;
}

static inline void vu0_Mulv2m1v2(void)
{
	vu0_ApplyMatrix( &REG_vf3, &REG_m1, &REG_vf3 ) ;
}

static inline void vu0_Mulv0m2v0(void)
{
	vu0_ApplyMatrix( &REG_vf1, &REG_m2, &REG_vf1 ) ;
}

static inline void vu0_Mulv1m2v0(void)
{
	vu0_ApplyMatrix( &REG_vf2, &REG_m2, &REG_vf1 ) ;
}

static inline void vu0_Mulv2m2v0(void)
{
	vu0_ApplyMatrix( &REG_vf3, &REG_m2, &REG_vf1 ) ;
}

static inline void vu0_Mulv0m2v1(void)
{
	vu0_ApplyMatrix( &REG_vf1, &REG_m2, &REG_vf2 ) ;
}

static inline void vu0_Mulv1m2v1(void)
{
	vu0_ApplyMatrix( &REG_vf2, &REG_m2, &REG_vf2 ) ;
}

static inline void vu0_Mulv2m2v1(void)
{
	vu0_ApplyMatrix( &REG_vf3, &REG_m2, &REG_vf2 ) ;
}

static inline void vu0_Mulv0m2v2(void)
{
	vu0_ApplyMatrix( &REG_vf1, &REG_m2, &REG_vf3 ) ;
}

static inline void vu0_Mulv1m2v2(void)
{
	vu0_ApplyMatrix( &REG_vf2, &REG_m2, &REG_vf3 ) ;
}

static inline void vu0_Mulv2m2v2(void)
{
	vu0_ApplyMatrix( &REG_vf3, &REG_m2, &REG_vf3 ) ;
}

static inline float vu0_Sin(float x)
{
    while(x>(float)M_PI) x-=(float)M_PI*2.0f;
    while(x<-(float)M_PI) x+=(float)M_PI*2.0f;
    if(x>(float)M_PI/2.0f) x=(float)M_PI-x;
    if(x<-(float)M_PI/2.0f) x=-(float)M_PI-x;

	return sinf( x ) ;
}

static inline float vu0_Cos(const float x)
{
    return vu0_Sin(x+(float)M_PI/2.0f);
}

static inline float vu0_SinS(int x)
{
    x=(x<<(32-12))>>(32-12);
    if(x>0x400) x=0x800-x;
    if(x<-0x400) x=-0x800-x;

    return sinf( x*(float)M_PI/2048 ) ;
}

static inline float vu0_CosS(const int x)
{
    return vu0_SinS(x+0x400);
}

static inline void vu0_Sin4(const float * const x,float *ans)
{
    float y[4];
    int i;

    for(i=0;i<4;i++){
	y[i]=x[i];
	while(y[i]>(float)M_PI) y[i]-=(float)M_PI*2.0f;
	while(y[i]<-(float)M_PI) y[i]+=(float)M_PI*2.0f;
	if(y[i]>(float)M_PI/2.0f) y[i]=(float)M_PI-y[i];
	if(y[i]<-(float)M_PI/2.0f) y[i]=-(float)M_PI-y[i];
    ans[i] = sinf( y[i] ) ;
    }
}

static inline void vu0_Cos4(const float * const x,float *ans)
{
    float y[4];
    int i;
    for(i=0;i<4;i++) y[i]=x[i]+(float)M_PI/2.0f;
    vu0_Sin4(y,ans);
}

static inline void vu0_SinS4(const short * const x,float *ans)
{
    short y[4];
    int i;

    for(i=0;i<4;i++){
	y[i]=x[i];
	y[i]=((int)(y[i])<<(32-12))>>(32-12);
	if(y[i]>0x400) y[i]=0x800-y[i];
	if(y[i]<-0x400) y[i]=-0x800-y[i];
    ans[i] = sinf( y[i]/2048*(float)M_PI ) ;
    }
}

static inline void vu0_CosS4(const short * const x,float *ans)
{
    short y[4];
    int i;
    for(i=0;i<4;i++) y[i]=x[i]+0x400;
    vu0_SinS4(y,ans);
}

/* ランダムの出力 */
static inline void vu0_RandInit(const float x)
{
}

static inline float vu0_Rand(void)
{
	return rnd() ;
}

/* PS系の角度を、ラジアンに変換 */
static inline void vu0_PSDegV2RadV(const SVECTOR * const s,FVECTOR *f)
{
	f->x = (float)s->vx * ( 2.f * M_PI / 4096.f);
	f->y = (float)s->vy * ( 2.f * M_PI / 4096.f);
	f->z = (float)s->vz * ( 2.f * M_PI / 4096.f);
}

static inline void vu0_RadV2PSDegV(const FVECTOR * const f,SVECTOR *s)
{
	s->vx = (short)(f->vx * 4096.f /( 2.f * M_PI )) ;
	s->vy = (short)(f->vy * 4096.f /( 2.f * M_PI )) ;
	s->vz = (short)(f->vz * 4096.f /( 2.f * M_PI )) ;
}


/* ベクトルの変換 */
static inline void vu0_IV0toFVv0(void)
{
	REG_vf1.vx = (float)(*(int *)&REG_vf1.vx) ;
	REG_vf1.vy = (float)(*(int *)&REG_vf1.vy) ;
	REG_vf1.vz = (float)(*(int *)&REG_vf1.vz) ;
	REG_vf1.vw = (float)(*(int *)&REG_vf1.vw) ;
}

static inline void vu0_IV0toFVv1(void)
{
	REG_vf2.vx = (float)(*(int *)&REG_vf2.vx) ;
	REG_vf2.vy = (float)(*(int *)&REG_vf2.vy) ;
	REG_vf2.vz = (float)(*(int *)&REG_vf2.vz) ;
	REG_vf2.vw = (float)(*(int *)&REG_vf2.vw) ;
}

static inline void vu0_IV0toFVv2(void)
{
	REG_vf3.vx = (float)(*(int *)&REG_vf3.vx) ;
	REG_vf3.vy = (float)(*(int *)&REG_vf3.vy) ;
	REG_vf3.vz = (float)(*(int *)&REG_vf3.vz) ;
	REG_vf3.vw = (float)(*(int *)&REG_vf3.vw) ;
}

static inline void vu0_IV0toFV(const IVECTOR * const iv,FVECTOR *fv)
{
	fv->vx = (float)iv->vx ;
	fv->vy = (float)iv->vy ;
	fv->vz = (float)iv->vz ;
	fv->vw = (float)iv->vw ;
}

static inline void vu0_SV0toFV(const SVECTOR * const iv,FVECTOR *fv)
{
	fv->vx = (float)iv->vx ;
	fv->vy = (float)iv->vy ;
	fv->vz = (float)iv->vz ;
	fv->vw = (float)iv->pad ;
}


static inline void vu0_IV12toFVv0(void)
{
	REG_vf1.vx = (float)(*(int *)&REG_vf1.vx)/4096.0f ;
	REG_vf1.vy = (float)(*(int *)&REG_vf1.vy)/4096.0f ;
	REG_vf1.vz = (float)(*(int *)&REG_vf1.vz)/4096.0f ;
	REG_vf1.vw = (float)(*(int *)&REG_vf1.vw)/4096.0f ;
}

static inline void vu0_IV12toFVv1(void)
{
	REG_vf2.vx = (float)(*(int *)&REG_vf2.vx)/4096.0f ;
	REG_vf2.vy = (float)(*(int *)&REG_vf2.vy)/4096.0f ;
	REG_vf2.vz = (float)(*(int *)&REG_vf2.vz)/4096.0f ;
	REG_vf2.vw = (float)(*(int *)&REG_vf2.vw)/4096.0f ;
}

static inline void vu0_IV12toFVv2(void)
{
	REG_vf3.vx = (float)(*(int *)&REG_vf3.vx)/4096.0f ;
	REG_vf3.vy = (float)(*(int *)&REG_vf3.vy)/4096.0f ;
	REG_vf3.vz = (float)(*(int *)&REG_vf3.vz)/4096.0f ;
	REG_vf3.vw = (float)(*(int *)&REG_vf3.vw)/4096.0f ;
}

static inline void vu0_IV12toFV(const IVECTOR * const iv,FVECTOR *fv)
{
	fv->vx = (float)iv->vx/4096.0f ;
	fv->vy = (float)iv->vy/4096.0f ;
	fv->vz = (float)iv->vz/4096.0f ;
	fv->vw = (float)iv->vw/4096.0f ;
}

static inline void vu0_SV12toFV(const SVECTOR * const iv,FVECTOR *fv)
{
	fv->vx = (float)iv->vx/4096.0f ;
	fv->vy = (float)iv->vy/4096.0f ;
	fv->vz = (float)iv->vz/4096.0f ;
	fv->vw = (float)iv->pad/4096.0f ;
}


static inline void vu0_FVtoIV0v0(void)
{
	*(int *)&REG_vf1.vx = (int)REG_vf1.vx ;
	*(int *)&REG_vf1.vy = (int)REG_vf1.vy ;
	*(int *)&REG_vf1.vz = (int)REG_vf1.vz ;
	*(int *)&REG_vf1.vw = (int)REG_vf1.vw ;
}

static inline void vu0_FVtoIV0v1(void)
{
	*(int *)&REG_vf2.vx = (int)REG_vf2.vx ;
	*(int *)&REG_vf2.vy = (int)REG_vf2.vy ;
	*(int *)&REG_vf2.vz = (int)REG_vf2.vz ;
	*(int *)&REG_vf2.vw = (int)REG_vf2.vw ;
}

static inline void vu0_FVtoIV0v2(void)
{
	*(int *)&REG_vf3.vx = (int)REG_vf3.vx ;
	*(int *)&REG_vf3.vy = (int)REG_vf3.vy ;
	*(int *)&REG_vf3.vz = (int)REG_vf3.vz ;
	*(int *)&REG_vf3.vw = (int)REG_vf3.vw ;
}

static inline void vu0_FVtoIV0(const FVECTOR * const fv,IVECTOR *iv)
{
	iv->vx  = (int)fv->vx ;
	iv->vy  = (int)fv->vy ;
	iv->vz  = (int)fv->vz ;
	iv->vw  = (int)fv->vw ;
}

static inline void vu0_FVtoSV0(const FVECTOR * const fv,SVECTOR *iv)
{
	iv->vx  = (short)fv->vx ;
	iv->vy  = (short)fv->vy ;
	iv->vz  = (short)fv->vz ;
	iv->pad = (short)fv->vw ;
}


static inline void vu0_FVtoIV12v0(void)
{
	*(int *)&REG_vf1.vx = (int)(REG_vf1.vx*4096.0f) ;
	*(int *)&REG_vf1.vy = (int)(REG_vf1.vy*4096.0f) ;
	*(int *)&REG_vf1.vz = (int)(REG_vf1.vz*4096.0f) ;
	*(int *)&REG_vf1.vw = (int)(REG_vf1.vw*4096.0f) ;
}

static inline void vu0_FVtoIV12v1(void)
{
	*(int *)&REG_vf2.vx = (int)(REG_vf2.vx*4096.0f) ;
	*(int *)&REG_vf2.vy = (int)(REG_vf2.vy*4096.0f) ;
	*(int *)&REG_vf2.vz = (int)(REG_vf2.vz*4096.0f) ;
	*(int *)&REG_vf2.vw = (int)(REG_vf2.vw*4096.0f) ;
}

static inline void vu0_FVtoIV12v2(void)
{
	*(int *)&REG_vf3.vx = (int)(REG_vf3.vx*4096.0f) ;
	*(int *)&REG_vf3.vy = (int)(REG_vf3.vy*4096.0f) ;
	*(int *)&REG_vf3.vz = (int)(REG_vf3.vz*4096.0f) ;
	*(int *)&REG_vf3.vw = (int)(REG_vf3.vw*4096.0f) ;
}

static inline void vu0_FVtoIV12(const FVECTOR * const fv,IVECTOR *iv)
{
	iv->vx  = (int)(fv->vx * 4096.0f) ;
	iv->vy  = (int)(fv->vy * 4096.0f) ;
	iv->vz  = (int)(fv->vz * 4096.0f) ;
	iv->vw  = (int)(fv->vw * 4096.0f) ;
}

static inline void vu0_FVtoSV12(const FVECTOR * const fv,SVECTOR *iv)
{
	iv->vx  = (short)(fv->vx * 4096.0f) ;
	iv->vy  = (short)(fv->vy * 4096.0f) ;
	iv->vz  = (short)(fv->vz * 4096.0f) ;
	iv->pad = (short)(fv->vw * 4096.0f) ;
}


/* バウンディングボックス内に、ある点が存在するかどうかのチェック
   バウンディングボックス内に存在すれば１、しなければ０を返す。*/
static inline int vu0_CheckBoundingBox(const VECTOR * const vertex,
									   const VECTOR * const v0,
									   const VECTOR * const v1)
{
	VECTOR	vecDiff0;
	VECTOR	vecDiff1;

	D3DXVec3Subtract((D3DXVECTOR3*)&vecDiff0
					 , (const D3DXVECTOR3*)v0, (const D3DXVECTOR3*)vertex);
	D3DXVec3Subtract((D3DXVECTOR3*)&vecDiff1
					 , (const D3DXVECTOR3*)v1, (const D3DXVECTOR3*)vertex);

	if( vecDiff0.x * vecDiff1.x < 0.f
		&& vecDiff0.y * vecDiff1.y < 0.f
		&& vecDiff0.z * vecDiff1.z < 0.f ) return 1;
	return 0;
}


/* ある点を中心とした指定範囲の中に、その点が含まれているかどうかを判別
   範囲内に存在すれば１、しなければ０を返す。*/
static inline int vu0_CheckRange(const FVECTOR * const vertex,
								 const FVECTOR * const v0,
								 const FVECTOR * const range)
{
	FVECTOR	vecDiff0;
	FVECTOR	vecDiff1;
	
	_sceVu0SubVector( &vecDiff0, v0, vertex);

	_sceVu0SubVector( &vecDiff1, &vecDiff0, range);
	_sceVu0AddVector( &vecDiff0, &vecDiff0, range);

	if( vecDiff0.x * vecDiff1.x < 0.f &&
		vecDiff0.y * vecDiff1.y < 0.f &&
		vecDiff0.z * vecDiff1.z < 0.f ) return 1;
	return 0;
}

#endif /* _sse_pack_xbox_h_ */

/*
	vu0_pack.h
		PS2用ベクトル／マトリクス演算ライブラリ (VU0使用)

	1999/08/04 K.Kano
	$Id: vu0_pack.h,v 1.1.1.3 2002/11/19 11:43:01 Yoshizawa1 Exp $
*/

/* -----------------------------------------------------------------
   Title    : PS2用ベクトル／マトリクス演算ライブラリ (VU0使用)
   Author   : 狩野  賢一郎 ( Kenichiro Kano ) KCEJ 制作一部
   Date     : 8/4/99
   Histtory :
   ----------------------------------------------------------------- */

#ifndef _vu0_pack_h_
#define _vu0_pack_h_

#ifdef KP_XBOX  /**************XBOX用の専用関数(書換え版)***********************  T.Morita 2002.02.01 */

#include "sse_pack_xbox.h"

#else        /************** PS2用の専用関数(オリジナル)***********************  T.Morita 2002.02.01 */
//#include <math.h>

#include "bp_matrix.h"

/* VU0で、マトリクスやベクトル演算する場合のマクロ */


/*
  VUレジスタの使用目的
  ベクトルレジスタ(128bit)
  vf0
  vf1	ベクトル０
  vf2	ベクトル１
  vf3	ベクトル２
  vf4	マトリクス０
  vf5	マトリクス０
  vf6	マトリクス０
  vf7	マトリクス０

  vf8	マトリクス１
  vf9	マトリクス１
  vf10	マトリクス１
  vf11	マトリクス１
  vf12	マトリクス２
  vf13	マトリクス２
  vf14	マトリクス２
  vf15	マトリクス２

  vf16	テンポラリ
  vf17	テンポラリ
  vf18	テンポラリ
  vf19	テンポラリ
  vf20
  vf21
  vf22
  vf23

  vf24
  vf25
  vf26
  vf27
  vf28
  vf29
  vf30
  vf31

  マトリクスの形態は、
   | m[0][0]  m[1][0]  m[2][0]  m[3][0] |
   | m[0][1]  m[1][1]  m[2][1]  m[3][1] |
   | m[0][2]  m[1][2]  m[2][2]  m[3][2] |
   | m[0][3]  m[1][3]  m[2][3]  m[3][3] |
   を使用する。


  整数レジスタ
  vi0
  vi1
  vi2
  vi3
  vi4
  vi5
  vi6
  vi7

  vi8
  vi9
  vi10
  vi11
  vi12
  vi13
  vi14
  vi15

  */

#if 1 //BP_ASM
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
   return BP_Sin(x);
#if 0 // ARMATURE OPTIMIZATION
   while(x>(float)M_PI) x-=(float)M_PI*2.0f;
   while(x<-(float)M_PI) x+=(float)M_PI*2.0f;
   if(x>(float)M_PI/2.0f) x=(float)M_PI-x;
   if(x<-(float)M_PI/2.0f) x=-(float)M_PI-x;

   return sinf( x ) ;
#endif
}

static inline float vu0_Cos(const float x)
{
   return BP_Cos(x);
#if 0 // ARMATURE OPTIMIZATION
   return vu0_Sin(x+(float)M_PI/2.0f);
#endif
}

static inline float vu0_SinS(int x)
{
   x=(x<<(32-12))>>(32-12);
   if(x>0x400) x=0x800-x;
   if(x<-0x400) x=-0x800-x;

   return BP_Sin(x*(float)M_PI/2048);
   // ARMATURE OPTIMIZATION
//   return sinf( x*(float)M_PI/2048 ) ;
}

static inline float vu0_CosS(const int x)
{
   return vu0_SinS(x+0x400);
}

static inline void vu0_Sin4(const float * const x,float *ans)
{
   ans[0] = BP_Sin(x[0]);
   ans[1] = BP_Sin(x[1]);
   ans[2] = BP_Sin(x[2]);
   ans[3] = BP_Sin(x[3]);
   // ARMATURE OPTIMIZATION
#if 0
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
#endif
}

static inline void vu0_Cos4(const float * const x,float *ans)
{
   ans[0] = BP_Cos(x[0]);
   ans[1] = BP_Cos(x[1]);
   ans[2] = BP_Cos(x[2]);
   ans[3] = BP_Cos(x[3]);
   // ARMATURE OPTIMIZATION
#if 0
   float y[4];
   int i;
   for(i=0;i<4;i++) y[i]=x[i]+(float)M_PI/2.0f;
   vu0_Sin4(y,ans);
#endif
}

static inline void vu0_SinS4(const short * const x,float *ans)
{
   ans[0] = vu0_SinS(x[0]);
   ans[1] = vu0_SinS(x[1]);
   ans[2] = vu0_SinS(x[2]);
   ans[3] = vu0_SinS(x[3]);
   // ARMATURE OPTIMIZATION
#if 0
   short y[4];
   int i;

   for(i=0;i<4;i++){
      y[i]=x[i];
      y[i]=((int)(y[i])<<(32-12))>>(32-12);
      if(y[i]>0x400) y[i]=0x800-y[i];
      if(y[i]<-0x400) y[i]=-0x800-y[i];
      ans[i] = sinf( y[i]/2048*(float)M_PI ) ;
   }
#endif
}

static inline void vu0_CosS4(const short * const x,float *ans)
{
   ans[0] = vu0_CosS(x[0]);
   ans[1] = vu0_CosS(x[1]);
   ans[2] = vu0_CosS(x[2]);
   ans[3] = vu0_CosS(x[3]);
   // ARMATURE OPTIMIZATION
#if 0
   short y[4];
   int i;
   for(i=0;i<4;i++) y[i]=x[i]+0x400;
   vu0_SinS4(y,ans);
#endif
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
   f->vx = (float)s->vx * ( 2.f * M_PI / 4096.f);
   f->vy = (float)s->vy * ( 2.f * M_PI / 4096.f);
   f->vz = (float)s->vz * ( 2.f * M_PI / 4096.f);
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

   BP_Vec3_SubVec( &vecDiff0, v0, vertex );
   BP_Vec3_SubVec( &vecDiff1, v1, vertex );

   if( vecDiff0.vx * vecDiff1.vx < 0.f
      && vecDiff0.vy * vecDiff1.vy < 0.f
      && vecDiff0.vz * vecDiff1.vz < 0.f ) return 1;
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

   if( vecDiff0.vx * vecDiff1.vx < 0.f &&
      vecDiff0.vy * vecDiff1.vy < 0.f &&
      vecDiff0.vz * vecDiff1.vz < 0.f ) return 1;
   return 0;
}

#else //BP_ASM

/* ロード/ストア */
static inline void vu0_Ldv0(const VECTOR * const a)
{
    asm volatile ("
    lqc2	vf1,0x00(%0)
    " : : "r"(a) );
}

static inline void vu0_Ldv1(const VECTOR * const a)
{
    asm volatile ("
    lqc2	vf2,0x00(%0)
    " : : "r"(a) );
}

static inline void vu0_Ldv2(const VECTOR * const a)
{
    asm volatile ("
    lqc2	vf3,0x00(%0)
    " : : "r"(a) );
}

static inline void vu0_LdSVv0(const SVECTOR * const a)
{
    asm volatile ("
    lh		$8,2(%0)
    prot3w	$8,$8
    lh		$8,0(%0)
    lh		$9,4(%0)
    ppacw	$8,$9,$8
    qmtc2.ni	$8,vf1
    " : : "r"(a) : "$8","$9");
}

static inline void vu0_LdSVv1(const SVECTOR * const a)
{
    asm volatile ("
    lh		$8,2(%0)
    prot3w	$8,$8
    lh		$8,0(%0)
    lh		$9,4(%0)
    ppacw	$8,$9,$8
    qmtc2.ni	$8,vf2
    " : : "r"(a) : "$8","$9");
}

static inline void vu0_LdSVv2(const SVECTOR * const a)
{
    asm volatile ("
    lh		$8,2(%0)
    prot3w	$8,$8
    lh		$8,0(%0)
    lh		$9,4(%0)
    ppacw	$8,$9,$8
    qmtc2.ni	$8,vf3
    " : : "r"(a) : "$8","$9");
}

static inline void vu0_Stv0(VECTOR *a)
{
    asm volatile ("
    sqc2	vf1,0x00(%0)
    " : : "r"(a) : "memory" );
}

static inline void vu0_Stv1(VECTOR *a)
{
    asm volatile ("
    sqc2	vf2,0x00(%0)
    " : : "r"(a) : "memory" );
}

static inline void vu0_Stv2(VECTOR *a)
{
    asm volatile ("
    sqc2	vf3,0x00(%0)
    " : : "r"(a) : "memory" );
}

static inline void vu0_StSVv0(SVECTOR *a)
{
    asm volatile ("
    qmfc2.i	$8,vf1
    sh		$8,0(%0)
    prot3w	$8,$8
    sh		$8,2(%0)
    prot3w	$8,$8
    sh		$8,4(%0)
    ": : "r"(a) : "$8","memory");
}

static inline void vu0_StSVv1(SVECTOR *a)
{
    asm volatile ("
    qmfc2.i	$8,vf2
    sh		$8,0(%0)
    prot3w	$8,$8
    sh		$8,2(%0)
    prot3w	$8,$8
    sh		$8,4(%0)
    ": : "r"(a) : "$8","memory");
}

static inline void vu0_StSVv2(SVECTOR *a)
{
    asm volatile ("
    qmfc2.i	$8,vf3
    sh		$8,0(%0)
    prot3w	$8,$8
    sh		$8,2(%0)
    prot3w	$8,$8
    sh		$8,4(%0)
    ": : "r"(a) : "$8","memory");
}

static inline void vu0_Ldm0(const MATRIX * const a)
{
    asm volatile ("
    lqc2	vf4,0x00(%0)
    lqc2	vf5,0x10(%0)
    lqc2	vf6,0x20(%0)
    lqc2	vf7,0x30(%0)
    " : : "r"(a) );
}

static inline void vu0_Ldm1(const MATRIX * const a)
{
    asm volatile ("
    lqc2	vf8,0x00(%0)
    lqc2	vf9,0x10(%0)
    lqc2	vf10,0x20(%0)
    lqc2	vf11,0x30(%0)
    " : : "r"(a) );
}

static inline void vu0_Ldm2(const MATRIX * const a)
{
    asm volatile ("
    lqc2	vf12,0x00(%0)
    lqc2	vf13,0x10(%0)
    lqc2	vf14,0x20(%0)
    lqc2	vf15,0x30(%0)
    " : : "r"(a) );
}

static inline void vu0_Stm0(MATRIX *a)
{
    asm volatile ("
    sqc2	vf4,0x00(%0)
    sqc2	vf5,0x10(%0)
    sqc2	vf6,0x20(%0)
    sqc2	vf7,0x30(%0)
    " : : "r"(a) : "memory" );
}

static inline void vu0_Stm1(MATRIX *a)
{
    asm volatile ("
    sqc2	vf8,0x00(%0)
    sqc2	vf9,0x10(%0)
    sqc2	vf10,0x20(%0)
    sqc2	vf11,0x30(%0)
    " : : "r"(a) : "memory" );
}

static inline void vu0_Stm2(MATRIX *a)
{
    asm volatile ("
    sqc2	vf12,0x00(%0)
    sqc2	vf13,0x10(%0)
    sqc2	vf14,0x20(%0)
    sqc2	vf15,0x30(%0)
    " : : "r"(a) : "memory" );
}

/* ベクトルを０クリアする */
static inline void vu0_Clrv0(void)
{
    asm volatile ("
    qmtc2.ni		$0,vf1
    ");
}

static inline void vu0_Clrv1(void)
{
    asm volatile ("
    qmtc2.ni		$0,vf2
    ");
}

static inline void vu0_Clrv2(void)
{
    asm volatile ("
    qmtc2.ni		$0,vf3
    ");
}

/* ベクトルの特定要素値を0.0にする */
static inline void vu0_Setv0x0(void)
{
    asm volatile ("
    vmulx.x	vf1,vf1,vf0
    ");
}

static inline void vu0_Setv1x0(void)
{
    asm volatile ("
    vmulx.x	vf2,vf2,vf0
    ");
}

static inline void vu0_Setv2x0(void)
{
    asm volatile ("
    vmulx.x	vf3,vf3,vf0
    ");
}

static inline void vu0_Setv0y0(void)
{
    asm volatile ("
    vmulx.y	vf1,vf1,vf0
    ");
}

static inline void vu0_Setv1y0(void)
{
    asm volatile ("
    vmulx.y	vf2,vf2,vf0
    ");
}

static inline void vu0_Setv2y0(void)
{
    asm volatile ("
    vmulx.y	vf3,vf3,vf0
    ");
}

static inline void vu0_Setv0z0(void)
{
    asm volatile ("
    vmulx.z	vf1,vf1,vf0
    ");
}

static inline void vu0_Setv1z0(void)
{
    asm volatile ("
    vmulx.z	vf2,vf2,vf0
    ");
}

static inline void vu0_Setv2z0(void)
{
    asm volatile ("
    vmulx.z	vf3,vf3,vf0
    ");
}

/* ベクトルのｗ値を、0.0にする */
static inline void vu0_Setv0w0(void)
{
    asm volatile ("
    vmulx.w	vf1,vf1,vf0
    ");
}

static inline void vu0_Setv1w0(void)
{
    asm volatile ("
    vmulx.w	vf2,vf2,vf0
    ");
}

static inline void vu0_Setv2w0(void)
{
    asm volatile ("
    vmulx.w	vf3,vf3,vf0
    ");
}

/* ベクトルのｗ値を、1.0にする */
static inline void vu0_Setv0w1(void)
{
    asm volatile ("
    vmove.w	vf1,vf0
    ");
}

static inline void vu0_Setv1w1(void)
{
    asm volatile ("
    vmove.w	vf2,vf0
    ");
}

static inline void vu0_Setv2w1(void)
{
    asm volatile ("
    vmove.w	vf3,vf0
    ");
}


/* コピー */
static inline void vu0_Cpv0v1(void)
{
    asm volatile ("
    vmove.xyzw	vf2,vf1
    ");
}

static inline void vu0_Cpv0v2(void)
{
    asm volatile ("
    vmove.xyzw	vf3,vf1
    ");
}

static inline void vu0_Cpv1v2(void)
{
    asm volatile ("
    vmove.xyzw	vf3,vf2
    ");
}

static inline void vu0_Cpv1v0(void)
{
    asm volatile ("
    vmove.xyzw	vf1,vf2
    ");
}

static inline void vu0_Cpv2v0(void)
{
    asm volatile ("
    vmove.xyzw	vf1,vf3
    ");
}

static inline void vu0_Cpv2v1(void)
{
    asm volatile ("
    vmove.xyzw	vf2,vf3
    ");
}

static inline void vu0_Cpm0m1(void)
{
    asm volatile ("
    vmove.xyzw	vf8,vf4
    vmove.xyzw	vf9,vf5
    vmove.xyzw	vf10,vf6
    vmove.xyzw	vf11,vf7
    ");
}

static inline void vu0_Cpm0m2(void)
{
    asm volatile ("
    vmove.xyzw	vf12,vf4
    vmove.xyzw	vf13,vf5
    vmove.xyzw	vf14,vf6
    vmove.xyzw	vf15,vf7
    ");
}

static inline void vu0_Cpm1m2(void)
{
    asm volatile ("
    vmove.xyzw	vf12,vf8
    vmove.xyzw	vf13,vf9
    vmove.xyzw	vf14,vf10
    vmove.xyzw	vf15,vf11
    ");
}

static inline void vu0_Cpm1m0(void)
{
    asm volatile ("
    vmove.xyzw	vf4,vf8
    vmove.xyzw	vf5,vf9
    vmove.xyzw	vf6,vf10
    vmove.xyzw	vf7,vf11
    ");
}

static inline void vu0_Cpm2m0(void)
{
    asm volatile ("
    vmove.xyzw	vf4,vf12
    vmove.xyzw	vf5,vf13
    vmove.xyzw	vf6,vf14
    vmove.xyzw	vf7,vf15
    ");
}

static inline void vu0_Cpm2m1(void)
{
    asm volatile ("
    vmove.xyzw	vf8,vf12
    vmove.xyzw	vf9,vf13
    vmove.xyzw	vf10,vf14
    vmove.xyzw	vf11,vf15
    ");
}

/* 演算 */

/* 加算 */
static inline void vu0_Addv0v1(void)
{
    asm volatile ("
    vadd.xyz	vf1,vf1,vf2
    ");
}

static inline void vu0_Addv0v2(void)
{
    asm volatile ("
    vadd.xyz	vf1,vf1,vf3
    ");
}

static inline void vu0_Addv1v2(void)
{
    asm volatile ("
    vadd.xyz	vf2,vf2,vf3
    ");
}

static inline void vu0_Addv1v0(void)
{
    asm volatile ("
    vadd.xyz	vf2,vf2,vf1
    ");
}

static inline void vu0_Addv2v0(void)
{
    asm volatile ("
    vadd.xyz	vf3,vf3,vf1
    ");
}

static inline void vu0_Addv2v1(void)
{
    asm volatile ("
    vadd.xyz	vf3,vf3,vf2
    ");
}

static inline void vu0_Addv2v0v1(void)
{
    asm volatile ("
    vadd.xyz	vf3,vf1,vf2
    ");
}

static inline void vu0_Addv1v0v1(void)
{
    asm volatile ("
    vadd.xyz	vf2,vf1,vf2
    ");
}

static inline void vu0_Addv1v0v2(void)
{
    asm volatile ("
    vadd.xyz	vf2,vf1,vf3
    ");
}

static inline void vu0_Addv2v0v2(void)
{
    asm volatile ("
    vadd.xyz	vf3,vf1,vf3
    ");
}

static inline void vu0_Addv0v1v2(void)
{
    asm volatile ("
    vadd.xyz	vf1,vf2,vf3
    ");
}

static inline void vu0_Addv2v1v2(void)
{
    asm volatile ("
    vadd.xyz	vf3,vf2,vf3
    ");
}

static inline void vu0_Addv2v1v0(void)
{
    asm volatile ("
    vadd.xyz	vf3,vf2,vf1
    ");
}

static inline void vu0_Addv0v1v0(void)
{
    asm volatile ("
    vadd.xyz	vf1,vf2,vf1
    ");
}

static inline void vu0_Addv1v2v0(void)
{
    asm volatile ("
    vadd.xyz	vf2,vf3,vf1
    ");
}

static inline void vu0_Addv0v2v0(void)
{
    asm volatile ("
    vadd.xyz	vf1,vf3,vf1
    ");
}

static inline void vu0_Addv0v2v1(void)
{
    asm volatile ("
    vadd.xyz	vf1,vf3,vf2
    ");
}

static inline void vu0_Addv1v2v1(void)
{
    asm volatile ("
    vadd.xyz	vf2,vf3,vf2
    ");
}

/* 減算 */
static inline void vu0_Subv0v1(void)
{
    asm volatile ("
    vsub.xyz	vf1,vf1,vf2
    ");
}

static inline void vu0_Subv0v2(void)
{
    asm volatile ("
    vsub.xyz	vf1,vf1,vf3
    ");
}

static inline void vu0_Subv1v2(void)
{
    asm volatile ("
    vsub.xyz	vf2,vf2,vf3
    ");
}

static inline void vu0_Subv1v0(void)
{
    asm volatile ("
    vsub.xyz	vf2,vf2,vf1
    ");
}

static inline void vu0_Subv2v0(void)
{
    asm volatile ("
    vsub.xyz	vf3,vf3,vf1
    ");
}

static inline void vu0_Subv2v1(void)
{
    asm volatile ("
    vsub.xyz	vf3,vf3,vf2
    ");
}

static inline void vu0_Subv2v0v1(void)
{
    asm volatile ("
    vsub.xyz	vf3,vf1,vf2
    ");
}

static inline void vu0_Subv1v0v1(void)
{
    asm volatile ("
    vsub.xyz	vf2,vf1,vf2
    ");
}

static inline void vu0_Subv1v0v2(void)
{
    asm volatile ("
    vsub.xyz	vf2,vf1,vf3
    ");
}

static inline void vu0_Subv2v0v2(void)
{
    asm volatile ("
    vsub.xyz	vf3,vf1,vf3
    ");
}

static inline void vu0_Subv0v1v2(void)
{
    asm volatile ("
    vsub.xyz	vf1,vf2,vf3
    ");
}

static inline void vu0_Subv2v1v2(void)
{
    asm volatile ("
    vsub.xyz	vf3,vf2,vf3
    ");
}

static inline void vu0_Subv2v1v0(void)
{
    asm volatile ("
    vsub.xyz	vf3,vf2,vf1
    ");
}

static inline void vu0_Subv0v1v0(void)
{
    asm volatile ("
    vsub.xyz	vf1,vf2,vf1
    ");
}

static inline void vu0_Subv1v2v0(void)
{
    asm volatile ("
    vsub.xyz	vf2,vf3,vf1
    ");
}

static inline void vu0_Subv0v2v0(void)
{
    asm volatile ("
    vsub.xyz	vf1,vf3,vf1
    ");
}

static inline void vu0_Subv0v2v1(void)
{
    asm volatile ("
    vsub.xyz	vf1,vf3,vf2
    ");
}

static inline void vu0_Subv1v2v1(void)
{
    asm volatile ("
    vsub.xyz	vf2,vf3,vf2
    ");
}

/* 乗算 */
static inline void vu0_Mulv0a(const float a)
{
    asm volatile ("
    qmtc2.ni	%0,vf16
    vmulx.xyz	vf1,vf1,vf16x
    " : : "r"(a) );
}

static inline void vu0_Mulv1a(const float a)
{
    asm volatile ("
    qmtc2.ni	%0,vf16
    vmulx.xyz	vf2,vf2,vf16x
    " : : "r"(a) );
}

static inline void vu0_Mulv2a(const float a)
{
    asm volatile ("
    qmtc2.ni	%0,vf16
    vmulx.xyz	vf3,vf3,vf16x
    " : : "r"(a) );
}

static inline void vu0_Mulv1v0a(const float a)
{
    asm volatile ("
    qmtc2.ni	%0,vf16
    vmulx.xyz	vf2,vf1,vf16x
    " : : "r"(a) );
}

static inline void vu0_Mulv2v0a(const float a)
{
    asm volatile ("
    qmtc2.ni	%0,vf16
    vmulx.xyz	vf3,vf1,vf16x
    " : : "r"(a) );
}

static inline void vu0_Mulv0v1a(const float a)
{
    asm volatile ("
    qmtc2.ni	%0,vf16
    vmulx.xyz	vf1,vf2,vf16x
    " : : "r"(a) );
}

static inline void vu0_Mulv2v1a(const float a)
{
    asm volatile ("
    qmtc2.ni	%0,vf16
    vmulx.xyz	vf3,vf2,vf16x
    " : : "r"(a) );
}

static inline void vu0_Mulv0v2a(const float a)
{
    asm volatile ("
    qmtc2.ni	%0,vf16
    vmulx.xyz	vf1,vf3,vf16x
    " : : "r"(a) );
}

static inline void vu0_Mulv1v2a(const float a)
{
    asm volatile ("
    qmtc2.ni	%0,vf16
    vmulx.xyz	vf2,vf3,vf16x
    " : : "r"(a) );
}

static inline void vu0_Mulv0Q(void)
{
    asm volatile ("
    vmulq.xyz	vf1,vf1,Q
    ");
}

static inline void vu0_Mulv1Q(void)
{
    asm volatile ("
    vmulq.xyz	vf2,vf2,Q
    ");
}

static inline void vu0_Mulv2Q(void)
{
    asm volatile ("
    vmulq.xyz	vf3,vf3,Q
    ");
}

static inline void vu0_Mulv1v0Q(void)
{
    asm volatile ("
    vmulq.xyz	vf2,vf1,Q
    ");
}

static inline void vu0_Mulv2v0Q(void)
{
    asm volatile ("
    vmulq.xyz	vf3,vf1,Q
    ");
}

static inline void vu0_Mulv0v1Q(void)
{
    asm volatile ("
    vmulq.xyz	vf1,vf2,Q
    ");
}

static inline void vu0_Mulv2v1Q(void)
{
    asm volatile ("
    vmulq.xyz	vf3,vf2,Q
    ");
}

static inline void vu0_Mulv0v2Q(void)
{
    asm volatile ("
    vmulq.xyz	vf1,vf3,Q
    ");
}

static inline void vu0_Mulv1v2Q(void)
{
    asm volatile ("
    vmulq.xyz	vf2,vf3,Q
    ");
}

/* 除算 */
static inline void vu0_DivQab(const float a,const float b)
{
    asm volatile ("
    qmtc2.ni	%0,vf16
    qmtc2.ni	%1,vf17
    vdiv	Q,vf16x,vf17x
    " : : "r"(a), "r"(b) );
}

/* 平方根 */
static inline void vu0_SqrtQ(const float a)
{
    asm volatile ("
    qmtc2.ni	%0,vf16
    vsqrt	Q,vf16x
    " : : "r"(a) );
}

/* 平方根の逆数  q = b/sqrt(a) */
static inline void vu0_RsqrtQ(const float a,const float b)
{
    asm volatile ("
    qmtc2.ni	%1,vf16
    qmtc2.ni	%0,vf17
    vrsqrt	Q,vf16x,vf17x
    " : : "r"(a), "r"(b) );
}

static inline void vu0_WaitQ(void)
{
    asm volatile ("
    vwaitq
    ");
}

/* 内積 */
static inline float vu0_InnerProductv0v1(void)
{
    float ans;
#if 0
    asm volatile ("
    qmtc2.i	%1,vf17
    vmul.xyz	vf16,vf1,vf2
    vmulax.x	ACC,vf17,vf16x
    vmadday.x	ACC,vf17,vf16y
    vmaddz.x	vf16,vf17,vf16z
    qmfc2.i	$8,vf16
    mtc1	$8,%0
    " : "=f"(ans) : "r"(1.0f) : "$8" );
#else
    /* 本当は、レジスタ渡しにしておきたいのだが、
       qmfc命令によるレジスタ渡しは、まともに動かなかったので、
       下のように、メモリに展開するようにした */
    asm volatile ("
    qmtc2.i	%1,vf17
    vmul.xyz	vf16,vf1,vf2
    vmulax.x	ACC,vf17,vf16x
    vmadday.x	ACC,vf17,vf16y
    vmaddz.x	vf16,vf17,vf16z
    qmfc2.i	$8,vf16
    sw		$8,0(%0)
    " : : "r"(&ans),"r"(1.0f) : "$8", "memory" );
#endif
    return ans;
}

static inline float vu0_InnerProductv0v2(void)
{
    float ans;
#if 0
    asm volatile ("
    qmtc2.i	%1,vf17
    vmul.xyz	vf16,vf1,vf3
    vmulax.x	ACC,vf17,vf16x
    vmadday.x	ACC,vf17,vf16y
    vmaddz.x	vf16,vf17,vf16z
    qmfc2.i	$8,vf16
    mtc1	$8,%0
    " : "=f"(ans) : "r"(1.0f) : "$8" );
#else
    /* 本当は、レジスタ渡しにしておきたいのだが、
       qmfc命令によるレジスタ渡しは、まともに動かなかったので、
       下のように、メモリに展開するようにした */
    asm volatile ("
    qmtc2.i	%1,vf17
    vmul.xyz	vf16,vf1,vf3
    vmulax.x	ACC,vf17,vf16x
    vmadday.x	ACC,vf17,vf16y
    vmaddz.x	vf16,vf17,vf16z
    qmfc2.i	$8,vf16
    sw		$8,0(%0)
    " : : "r"(&ans),"r"(1.0f) : "$8", "memory" );
#endif
    return ans;
}

static inline float vu0_InnerProductv1v2(void)
{
    float ans;
#if 0
    asm volatile ("
    qmtc2.i	%1,vf17
    vmul.xyz	vf16,vf2,vf3
    vmulax.x	ACC,vf17,vf16x
    vmadday.x	ACC,vf17,vf16y
    vmaddz.x	vf16,vf17,vf16z
    qmfc2.i	$8,vf16
    mtc1	$8,%0
    " : "=f"(ans) : "r"(1.0f) : "$8" );
#else
    /* 本当は、レジスタ渡しにしておきたいのだが、
       qmfc命令によるレジスタ渡しは、まともに動かなかったので、
       下のように、メモリに展開するようにした */
    asm volatile ("
    qmtc2.i	%1,vf17
    vmul.xyz	vf16,vf2,vf3
    vmulax.x	ACC,vf17,vf16x
    vmadday.x	ACC,vf17,vf16y
    vmaddz.x	vf16,vf17,vf16z
    qmfc2.i	$8,vf16
    sw		$8,0(%0)
    " : : "r"(&ans),"r"(1.0f) : "$8", "memory" );
#endif
    return ans;
}

/* ベクトルの長さの平方 */
static inline float vu0_VectorLength2v0(void)
{
    float ans;
#if 0
    asm volatile ("
    vmul.xyz	vf16,vf1,vf1
    vmulaw.x	ACC,vf0,vf16
    vmaddaw.y	ACC,vf0,vf16
    vmaddw.z	vf16,vf0,vf16
    qmfc2.i	%0,vf16
    mtsah	$0,6
    qfsrv	%0,%0,%0
    " : "=r"(ans) );
#else
    /* 本当は、レジスタ渡しにしておきたいのだが、
       qmfc命令によるレジスタ渡しは、まともに動かなかったので、
       下のように、メモリに展開するようにした */
    asm volatile ("
    qmtc2.i	%1,vf17
    vmul.xyz	vf16,vf1,vf1
    vmulax.x	ACC,vf17,vf16x
    vmadday.x	ACC,vf17,vf16y
    vmaddz.x	vf16,vf17,vf16z
    qmfc2.i	$8,vf16
    sw		$8,0(%0)
    " : : "r"(&ans),"r"(1.0f) : "$8", "memory" );
#endif
    return ans;
}

static inline float vu0_VectorLength2v1(void)
{
    float ans;
#if 0
    asm volatile ("
    vmul.xyz	vf16,vf2,vf2
    vmulaw.x	ACC,vf0,vf16
    vmaddaw.y	ACC,vf0,vf16
    vmaddw.z	vf16,vf0,vf16
    qmfc2.i	%0,vf16
    mtsah	$0,6
    qfsrv	%0,%0,%0
    " : "=r"(ans) );
#else
    /* 本当は、レジスタ渡しにしておきたいのだが、
       qmfc命令によるレジスタ渡しは、まともに動かなかったので、
       下のように、メモリに展開するようにした */
    asm volatile ("
    qmtc2.i	%1,vf17
    vmul.xyz	vf16,vf2,vf2
    vmulax.x	ACC,vf17,vf16x
    vmadday.x	ACC,vf17,vf16y
    vmaddz.x	vf16,vf17,vf16z
    qmfc2.i	$8,vf16
    sw		$8,0(%0)
    " : : "r"(&ans),"r"(1.0f) : "$8", "memory" );
#endif
    return ans;
}

static inline float vu0_VectorLength2v2(void)
{
    float ans;
#if 0
    asm volatile ("
    vmul.xyz	vf16,vf3,vf3
    vmulaw.x	ACC,vf0,vf16
    vmaddaw.y	ACC,vf0,vf16
    vmaddw.z	vf16,vf0,vf16
    qmfc2.i	%0,vf16
    mtsah	$0,6
    qfsrv	%0,%0,%0
    " : "=r"(ans) );
#else
    /* 本当は、レジスタ渡しにしておきたいのだが、
       qmfc命令によるレジスタ渡しは、まともに動かなかったので、
       下のように、メモリに展開するようにした */
    asm volatile ("
    qmtc2.i	%1,vf17
    vmul.xyz	vf16,vf3,vf3
    vmulax.x	ACC,vf17,vf16x
    vmadday.x	ACC,vf17,vf16y
    vmaddz.x	vf16,vf17,vf16z
    qmfc2.i	$8,vf16
    sw		$8,0(%0)
    " : : "r"(&ans),"r"(1.0f) : "$8", "memory" );
#endif
    return ans;
}

/* ベクトルの正規化する */
static inline void vu0_VectorNormalv0(void)
{
    asm volatile ("
    vmul.xyz	vf16,vf1,vf1
    vmulax.w	ACC,vf0,vf16
    vmadday.w	ACC,vf0,vf16
    vmaddz.w	vf16,vf0,vf16
    vrsqrt	Q,vf0w,vf16w
    vwaitq
    vmulq.xyz	vf1,vf1,Q
    ");
}

static inline void vu0_VectorNormalv1(void)
{
    asm volatile ("
    vmul.xyz	vf16,vf2,vf2
    vmulax.w	ACC,vf0,vf16
    vmadday.w	ACC,vf0,vf16
    vmaddz.w	vf16,vf0,vf16
    vrsqrt	Q,vf0w,vf16w
    vwaitq
    vmulq.xyz	vf2,vf2,Q
    ");
}

static inline void vu0_VectorNormalv2(void)
{
    asm volatile ("
    vmul.xyz	vf16,vf3,vf3
    vmulax.w	ACC,vf0,vf16
    vmadday.w	ACC,vf0,vf16
    vmaddz.w	vf16,vf0,vf16
    vrsqrt	Q,vf0w,vf16w
    vwaitq
    vmulq.xyz	vf3,vf3,Q
    ");
}

/* 外積 */
static inline void vu0_OuterProductv0v1(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf1,vf2
    vopmsub.xyz	vf1,vf2,vf1
    ");
}

static inline void vu0_OuterProductv0v2(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf1,vf3
    vopmsub.xyz	vf1,vf3,vf1
    ");
}

static inline void vu0_OuterProductv1v2(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf2,vf3
    vopmsub.xyz	vf2,vf3,vf2
    ");
}

static inline void vu0_OuterProductv1v0(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf2,vf1
    vopmsub.xyz	vf2,vf1,vf2
    ");
}

static inline void vu0_OuterProductv2v0(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf3,vf1
    vopmsub.xyz	vf3,vf1,vf3
    ");
}

static inline void vu0_OuterProductv2v1(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf3,vf2
    vopmsub.xyz	vf3,vf2,vf3
    ");
}

static inline void vu0_OuterProductv2v0v1(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf1,vf2
    vopmsub.xyz	vf3,vf2,vf1
    ");
}

static inline void vu0_OuterProductv1v0v1(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf1,vf2
    vopmsub.xyz	vf2,vf2,vf1
    ");
}

static inline void vu0_OuterProductv1v0v2(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf1,vf3
    vopmsub.xyz	vf2,vf3,vf1
    ");
}

static inline void vu0_OuterProductv2v0v2(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf1,vf3
    vopmsub.xyz	vf3,vf3,vf1
    ");
}

static inline void vu0_OuterProductv0v1v2(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf2,vf3
    vopmsub.xyz	vf1,vf3,vf2
    ");
}

static inline void vu0_OuterProductv2v1v2(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf2,vf3
    vopmsub.xyz	vf3,vf3,vf2
    ");
}

static inline void vu0_OuterProductv2v1v0(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf2,vf1
    vopmsub.xyz	vf3,vf1,vf2
    ");
}

static inline void vu0_OuterProductv0v1v0(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf2,vf1
    vopmsub.xyz	vf1,vf1,vf2
    ");
}

static inline void vu0_OuterProductv1v2v0(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf3,vf1
    vopmsub.xyz	vf2,vf1,vf3
    ");
}

static inline void vu0_OuterProductv0v2v0(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf3,vf1
    vopmsub.xyz	vf1,vf1,vf3
    ");
}

static inline void vu0_OuterProductv0v2v1(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf3,vf2
    vopmsub.xyz	vf1,vf2,vf3
    ");
}

static inline void vu0_OuterProductv1v2v1(void)
{
    asm volatile ("
    vopmula.xyz	ACC,vf3,vf2
    vopmsub.xyz	vf2,vf2,vf3
    ");
}

/* 各要素を自乗したベクトルを出す */
static inline void vu0_VectorPow2v0(void)
{
    asm volatile ("
    vmul.xyz	vf1,vf1,vf1
    ");
}

static inline void vu0_VectorPow2v1(void)
{
    asm volatile ("
    vmul.xyz	vf2,vf2,vf2
    ");
}

static inline void vu0_VectorPow2v2(void)
{
    asm volatile ("
    vmul.xyz	vf3,vf3,vf3
    ");
}

/* 楕円球の当たり用のパラメータを算出する
   (a, b, c, 1.0) -> (bc, ca, ab, abc) */
static inline void vu0_MakeOvalParamsv0(void)
{
    vu0_Setv0w1();
    asm volatile ("
    vmr32.xyzw	vf16,vf1
    vmr32.xyzw	vf17,vf16
    vmul.xyzw	vf18,vf1,vf16
    vmul.xyzw	vf1,vf18,vf17
    vmr32.xyzw	vf1,vf1
    ");
}

static inline void vu0_MakeOvalParamsv1(void)
{
    vu0_Setv1w1();
    asm volatile ("
    vmr32.xyzw	vf16,vf2
    vmr32.xyzw	vf17,vf16
    vmul.xyzw	vf18,vf2,vf16
    vmul.xyzw	vf2,vf18,vf17
    vmr32.xyzw	vf2,vf2
    ");
}

static inline void vu0_MakeOvalParamsv2(void)
{
    vu0_Setv2w1();
    asm volatile ("
    vmr32.xyzw	vf16,vf3
    vmr32.xyzw	vf17,vf16
    vmul.xyzw	vf18,vf3,vf16
    vmul.xyzw	vf3,vf18,vf17
    vmr32.xyzw	vf3,vf3
    ");
}

/* マトリクス同士を掛け合わせる */
static inline void vu0_Mulm2m0m1(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf4,vf8x
    vmadday.xyzw	ACC,vf5,vf8y
    vmaddaz.xyzw	ACC,vf6,vf8z
    vmaddw.xyzw		vf12,vf7,vf8w
    vmulax.xyzw		ACC,vf4,vf9x
    vmadday.xyzw	ACC,vf5,vf9y
    vmaddaz.xyzw	ACC,vf6,vf9z
    vmaddw.xyzw		vf13,vf7,vf9w
    vmulax.xyzw		ACC,vf4,vf10x
    vmadday.xyzw	ACC,vf5,vf10y
    vmaddaz.xyzw	ACC,vf6,vf10z
    vmaddw.xyzw		vf14,vf7,vf10w
    vmulax.xyzw		ACC,vf4,vf11x
    vmadday.xyzw	ACC,vf5,vf11y
    vmaddaz.xyzw	ACC,vf6,vf11z
    vmaddw.xyzw		vf15,vf7,vf11w
    ");
}

static inline void vu0_Mulm2m1m0(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf8,vf4x
    vmadday.xyzw	ACC,vf9,vf4y
    vmaddaz.xyzw	ACC,vf10,vf4z
    vmaddw.xyzw		vf12,vf11,vf4w
    vmulax.xyzw		ACC,vf8,vf5x
    vmadday.xyzw	ACC,vf9,vf5y
    vmaddaz.xyzw	ACC,vf10,vf5z
    vmaddw.xyzw		vf13,vf11,vf5w
    vmulax.xyzw		ACC,vf8,vf6x
    vmadday.xyzw	ACC,vf9,vf6y
    vmaddaz.xyzw	ACC,vf10,vf6z
    vmaddw.xyzw		vf14,vf11,vf6w
    vmulax.xyzw		ACC,vf8,vf7x
    vmadday.xyzw	ACC,vf9,vf7y
    vmaddaz.xyzw	ACC,vf10,vf7z
    vmaddw.xyzw		vf15,vf11,vf7w
    ");
}

static inline void vu0_Mulm1m0m2(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf4,vf12x
    vmadday.xyzw	ACC,vf5,vf12y
    vmaddaz.xyzw	ACC,vf6,vf12z
    vmaddw.xyzw		vf8,vf7,vf12w
    vmulax.xyzw		ACC,vf4,vf13x
    vmadday.xyzw	ACC,vf5,vf13y
    vmaddaz.xyzw	ACC,vf6,vf13z
    vmaddw.xyzw		vf9,vf7,vf13w
    vmulax.xyzw		ACC,vf4,vf14x
    vmadday.xyzw	ACC,vf5,vf14y
    vmaddaz.xyzw	ACC,vf6,vf14z
    vmaddw.xyzw		vf10,vf7,vf14w
    vmulax.xyzw		ACC,vf4,vf15x
    vmadday.xyzw	ACC,vf5,vf15y
    vmaddaz.xyzw	ACC,vf6,vf15z
    vmaddw.xyzw		vf11,vf7,vf15w
    ");
}

static inline void vu0_Mulm1m2m0(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf12,vf4x
    vmadday.xyzw	ACC,vf13,vf4y
    vmaddaz.xyzw	ACC,vf14,vf4z
    vmaddw.xyzw		vf8,vf15,vf4w
    vmulax.xyzw		ACC,vf12,vf5x
    vmadday.xyzw	ACC,vf13,vf5y
    vmaddaz.xyzw	ACC,vf14,vf5z
    vmaddw.xyzw		vf9,vf15,vf5w
    vmulax.xyzw		ACC,vf12,vf6x
    vmadday.xyzw	ACC,vf13,vf6y
    vmaddaz.xyzw	ACC,vf14,vf6z
    vmaddw.xyzw		vf10,vf15,vf6w
    vmulax.xyzw		ACC,vf12,vf7x
    vmadday.xyzw	ACC,vf13,vf7y
    vmaddaz.xyzw	ACC,vf14,vf7z
    vmaddw.xyzw		vf11,vf15,vf7w
    ");
}

static inline void vu0_Mulm0m1m2(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf8,vf12x
    vmadday.xyzw	ACC,vf9,vf12y
    vmaddaz.xyzw	ACC,vf10,vf12z
    vmaddw.xyzw		vf4,vf11,vf12w
    vmulax.xyzw		ACC,vf8,vf13x
    vmadday.xyzw	ACC,vf9,vf13y
    vmaddaz.xyzw	ACC,vf10,vf13z
    vmaddw.xyzw		vf5,vf11,vf13w
    vmulax.xyzw		ACC,vf8,vf14x
    vmadday.xyzw	ACC,vf9,vf14y
    vmaddaz.xyzw	ACC,vf10,vf14z
    vmaddw.xyzw		vf6,vf11,vf14w
    vmulax.xyzw		ACC,vf8,vf15x
    vmadday.xyzw	ACC,vf9,vf15y
    vmaddaz.xyzw	ACC,vf10,vf15z
    vmaddw.xyzw		vf7,vf11,vf15w
    ");
}

static inline void vu0_Mulm0m2m1(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf12,vf8x
    vmadday.xyzw	ACC,vf13,vf8y
    vmaddaz.xyzw	ACC,vf14,vf8z
    vmaddw.xyzw		vf4,vf15,vf8w
    vmulax.xyzw		ACC,vf12,vf9x
    vmadday.xyzw	ACC,vf13,vf9y
    vmaddaz.xyzw	ACC,vf14,vf9z
    vmaddw.xyzw		vf5,vf15,vf9w
    vmulax.xyzw		ACC,vf12,vf10x
    vmadday.xyzw	ACC,vf13,vf10y
    vmaddaz.xyzw	ACC,vf14,vf10z
    vmaddw.xyzw		vf6,vf15,vf10w
    vmulax.xyzw		ACC,vf12,vf11x
    vmadday.xyzw	ACC,vf13,vf11y
    vmaddaz.xyzw	ACC,vf14,vf11z
    vmaddw.xyzw		vf7,vf15,vf11w
    ");
}

/* マトリクスとベクトルを掛け合わせる */
static inline void vu0_Mulv0m0v0(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf4,vf1x
    vmadday.xyzw	ACC,vf5,vf1y
    vmaddaz.xyzw	ACC,vf6,vf1z
    vmaddw.xyzw		vf1,vf7,vf1w
    ");
}

static inline void vu0_Mulv1m0v0(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf4,vf1x
    vmadday.xyzw	ACC,vf5,vf1y
    vmaddaz.xyzw	ACC,vf6,vf1z
    vmaddw.xyzw		vf2,vf7,vf1w
    ");
}

static inline void vu0_Mulv2m0v0(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf4,vf1x
    vmadday.xyzw	ACC,vf5,vf1y
    vmaddaz.xyzw	ACC,vf6,vf1z
    vmaddw.xyzw		vf3,vf7,vf1w
    ");
}

static inline void vu0_Mulv0m0v1(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf4,vf2x
    vmadday.xyzw	ACC,vf5,vf2y
    vmaddaz.xyzw	ACC,vf6,vf2z
    vmaddw.xyzw		vf1,vf7,vf2w
    ");
}

static inline void vu0_Mulv1m0v1(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf4,vf2x
    vmadday.xyzw	ACC,vf5,vf2y
    vmaddaz.xyzw	ACC,vf6,vf2z
    vmaddw.xyzw		vf2,vf7,vf2w
    ");
}

static inline void vu0_Mulv2m0v1(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf4,vf2x
    vmadday.xyzw	ACC,vf5,vf2y
    vmaddaz.xyzw	ACC,vf6,vf2z
    vmaddw.xyzw		vf3,vf7,vf2w
    ");
}

static inline void vu0_Mulv0m0v2(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf4,vf3x
    vmadday.xyzw	ACC,vf5,vf3y
    vmaddaz.xyzw	ACC,vf6,vf3z
    vmaddw.xyzw		vf1,vf7,vf3w
    ");
}

static inline void vu0_Mulv1m0v2(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf4,vf3x
    vmadday.xyzw	ACC,vf5,vf3y
    vmaddaz.xyzw	ACC,vf6,vf3z
    vmaddw.xyzw		vf2,vf7,vf3w
    ");
}

static inline void vu0_Mulv2m0v2(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf4,vf3x
    vmadday.xyzw	ACC,vf5,vf3y
    vmaddaz.xyzw	ACC,vf6,vf3z
    vmaddw.xyzw		vf3,vf7,vf3w
    ");
}

static inline void vu0_Mulv0m1v0(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf8,vf1x
    vmadday.xyzw	ACC,vf9,vf1y
    vmaddaz.xyzw	ACC,vf10,vf1z
    vmaddw.xyzw		vf1,vf11,vf1w
    ");
}

static inline void vu0_Mulv1m1v0(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf8,vf1x
    vmadday.xyzw	ACC,vf9,vf1y
    vmaddaz.xyzw	ACC,vf10,vf1z
    vmaddw.xyzw		vf2,vf11,vf1w
    ");
}

static inline void vu0_Mulv2m1v0(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf8,vf1x
    vmadday.xyzw	ACC,vf9,vf1y
    vmaddaz.xyzw	ACC,vf10,vf1z
    vmaddw.xyzw		vf3,vf11,vf1w
    ");
}

static inline void vu0_Mulv0m1v1(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf8,vf2x
    vmadday.xyzw	ACC,vf9,vf2y
    vmaddaz.xyzw	ACC,vf10,vf2z
    vmaddw.xyzw		vf1,vf11,vf2w
    ");
}

static inline void vu0_Mulv1m1v1(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf8,vf2x
    vmadday.xyzw	ACC,vf9,vf2y
    vmaddaz.xyzw	ACC,vf10,vf2z
    vmaddw.xyzw		vf2,vf11,vf2w
    ");
}

static inline void vu0_Mulv2m1v1(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf8,vf2x
    vmadday.xyzw	ACC,vf9,vf2y
    vmaddaz.xyzw	ACC,vf10,vf2z
    vmaddw.xyzw		vf3,vf11,vf2w
    ");
}

static inline void vu0_Mulv0m1v2(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf8,vf3x
    vmadday.xyzw	ACC,vf9,vf3y
    vmaddaz.xyzw	ACC,vf10,vf3z
    vmaddw.xyzw		vf1,vf11,vf3w
    ");
}

static inline void vu0_Mulv1m1v2(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf8,vf3x
    vmadday.xyzw	ACC,vf9,vf3y
    vmaddaz.xyzw	ACC,vf10,vf3z
    vmaddw.xyzw		vf2,vf11,vf3w
    ");
}

static inline void vu0_Mulv2m1v2(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf8,vf3x
    vmadday.xyzw	ACC,vf9,vf3y
    vmaddaz.xyzw	ACC,vf10,vf3z
    vmaddw.xyzw		vf3,vf11,vf3w
    ");
}

static inline void vu0_Mulv0m2v0(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf12,vf1x
    vmadday.xyzw	ACC,vf13,vf1y
    vmaddaz.xyzw	ACC,vf14,vf1z
    vmaddw.xyzw		vf1,vf15,vf1w
    ");
}

static inline void vu0_Mulv1m2v0(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf12,vf1x
    vmadday.xyzw	ACC,vf13,vf1y
    vmaddaz.xyzw	ACC,vf14,vf1z
    vmaddw.xyzw		vf2,vf15,vf1w
    ");
}

static inline void vu0_Mulv2m2v0(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf12,vf1x
    vmadday.xyzw	ACC,vf13,vf1y
    vmaddaz.xyzw	ACC,vf14,vf1z
    vmaddw.xyzw		vf3,vf15,vf1w
    ");
}

static inline void vu0_Mulv0m2v1(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf12,vf2x
    vmadday.xyzw	ACC,vf13,vf2y
    vmaddaz.xyzw	ACC,vf14,vf2z
    vmaddw.xyzw		vf1,vf15,vf2w
    ");
}

static inline void vu0_Mulv1m2v1(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf12,vf2x
    vmadday.xyzw	ACC,vf13,vf2y
    vmaddaz.xyzw	ACC,vf14,vf2z
    vmaddw.xyzw		vf2,vf15,vf2w
    ");
}

static inline void vu0_Mulv2m2v1(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf12,vf2x
    vmadday.xyzw	ACC,vf13,vf2y
    vmaddaz.xyzw	ACC,vf14,vf2z
    vmaddw.xyzw		vf3,vf15,vf2w
    ");
}

static inline void vu0_Mulv0m2v2(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf12,vf3x
    vmadday.xyzw	ACC,vf13,vf3y
    vmaddaz.xyzw	ACC,vf14,vf3z
    vmaddw.xyzw		vf1,vf15,vf3w
    ");
}

static inline void vu0_Mulv1m2v2(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf12,vf3x
    vmadday.xyzw	ACC,vf13,vf3y
    vmaddaz.xyzw	ACC,vf14,vf3z
    vmaddw.xyzw		vf2,vf15,vf3w
    ");
}

static inline void vu0_Mulv2m2v2(void)
{
    asm volatile  ("
    vmulax.xyzw		ACC,vf12,vf3x
    vmadday.xyzw	ACC,vf13,vf3y
    vmaddaz.xyzw	ACC,vf14,vf3z
    vmaddw.xyzw		vf3,vf15,vf3w
    ");
}

static inline float vu0_Sin(float x)
{
    float ans;

    while(x>(float)M_PI) x-=(float)M_PI*2.0f;
    while(x<-(float)M_PI) x+=(float)M_PI*2.0f;
    if(x>(float)M_PI/2.0f) x=(float)M_PI-x;
    if(x<-(float)M_PI/2.0f) x=-(float)M_PI-x;

#if 0
    asm volatile ("
    qmtc2.ni		%1,vf16
    vmul.xyzw		vf17,vf16,vf16
    qmtc2.ni		%2,vf20
    vmulaw.xyzw		ACC,vf16,vf00
    vmul.xyzw		vf18,vf16,vf17
    qmtc2.ni		%3,vf21
    vmaddax.xyzw	ACC,vf18,vf20
    vmul.xyzw		vf16,vf18,vf17
    qmtc2.ni		%4,vf22
    vmaddax.xyzw	ACC,vf16,vf21
    vmul.xyzw		vf18,vf16,vf17
    qmtc2.ni		%5,vf23
    vmaddax.xyzw	ACC,vf18,vf22
    vmul.xyzw		vf16,vf18,vf17
    vmaddx.xyzw		vf16,vf16,vf23
    qmfc2.i		$8,vf16
    mtc1		$8,%0
    " : "=f"(ans) : "r"(x),
    "r"(-1.0f/(3.0f*2.0f)),"r"(1.0f/(5.0f*4.0f*3.0f*2.0f)),
    "r"(-1.0f/(7.0f*6.0f*5.0f*4.0f*3.0f*2.0f)),
    "r"(1.0f/(9.0f*8.0f*7.0f*6.0f*5.0f*4.0f*3.0f*2.0f)) : "$8","memory" );
#else
    asm volatile ("
    qmtc2.ni		%1,vf16
    vmul.xyzw		vf17,vf16,vf16
    qmtc2.ni		%2,vf20
    vmulaw.xyzw		ACC,vf16,vf00
    vmul.xyzw		vf18,vf16,vf17
    qmtc2.ni		%3,vf21
    vmaddax.xyzw	ACC,vf18,vf20
    vmul.xyzw		vf16,vf18,vf17
    qmtc2.ni		%4,vf22
    vmaddax.xyzw	ACC,vf16,vf21
    vmul.xyzw		vf18,vf16,vf17
    qmtc2.ni		%5,vf23
    vmaddax.xyzw	ACC,vf18,vf22
    vmul.xyzw		vf16,vf18,vf17
    vmaddx.xyzw		vf16,vf16,vf23
    qmfc2.i		$8,vf16
    sw			$8,0(%0)
    " :  : "r"(&ans), "r"(x),
    "r"(-1.0f/(3.0f*2.0f)),"r"(1.0f/(5.0f*4.0f*3.0f*2.0f)),
    "r"(-1.0f/(7.0f*6.0f*5.0f*4.0f*3.0f*2.0f)),
    "r"(1.0f/(9.0f*8.0f*7.0f*6.0f*5.0f*4.0f*3.0f*2.0f)) : "$8","memory" );
#endif

    return ans;
}

static inline float vu0_Cos(const float x)
{
    return vu0_Sin(x+(float)M_PI/2.0f);
}

static inline float vu0_SinS(int x)
{
    float ans;

    x=(x<<(32-12))>>(32-12);
    if(x>0x400) x=0x800-x;
    if(x<-0x400) x=-0x800-x;

#if 0
    asm volatile ("
    qmtc2.ni		%1,vf16

    qmtc2.ni		%6,vf17
    vitof12.xyzw	vf16,vf16
    vmulx.xyzw		vf16,vf16,vf17

    vmul.xyzw		vf17,vf16,vf16
    qmtc2.ni		%2,vf20
    vmulaw.xyzw		ACC,vf16,vf00
    vmul.xyzw		vf18,vf16,vf17
    qmtc2.ni		%3,vf21
    vmaddax.xyzw	ACC,vf18,vf20
    vmul.xyzw		vf16,vf18,vf17
    qmtc2.ni		%4,vf22
    vmaddax.xyzw	ACC,vf16,vf21
    vmul.xyzw		vf18,vf16,vf17
    qmtc2.ni		%5,vf23
    vmaddax.xyzw	ACC,vf18,vf22
    vmul.xyzw		vf16,vf18,vf17
    vmaddx.xyzw		vf16,vf16,vf23
    qmfc2.i		$8,vf16
    mtc1		$8,%0
    " : "=f"(ans) : "r"(x),
    "r"(-1.0f/(3.0f*2.0f)),"r"(1.0f/(5.0f*4.0f*3.0f*2.0f)),
    "r"(-1.0f/(7.0f*6.0f*5.0f*4.0f*3.0f*2.0f)),
    "r"(1.0f/(9.0f*8.0f*7.0f*6.0f*5.0f*4.0f*3.0f*2.0f)), "r"((float)M_PI*2.0f)
    : "$8","memory" );
#else
    asm volatile ("
    qmtc2.ni		%1,vf16

    qmtc2.ni		%6,vf17
    vitof12.xyzw	vf16,vf16
    vmulx.xyzw		vf16,vf16,vf17

    vmul.xyzw		vf17,vf16,vf16
    qmtc2.ni		%2,vf20
    vmulaw.xyzw		ACC,vf16,vf00
    vmul.xyzw		vf18,vf16,vf17
    qmtc2.ni		%3,vf21
    vmaddax.xyzw	ACC,vf18,vf20
    vmul.xyzw		vf16,vf18,vf17
    qmtc2.ni		%4,vf22
    vmaddax.xyzw	ACC,vf16,vf21
    vmul.xyzw		vf18,vf16,vf17
    qmtc2.ni		%5,vf23
    vmaddax.xyzw	ACC,vf18,vf22
    vmul.xyzw		vf16,vf18,vf17
    vmaddx.xyzw		vf16,vf16,vf23
    qmfc2.i		$8,vf16
    sw			$8,0(%0)
    " :  : "r"(&ans), "r"(x),
    "r"(-1.0f/(3.0f*2.0f)),"r"(1.0f/(5.0f*4.0f*3.0f*2.0f)),
    "r"(-1.0f/(7.0f*6.0f*5.0f*4.0f*3.0f*2.0f)),
    "r"(1.0f/(9.0f*8.0f*7.0f*6.0f*5.0f*4.0f*3.0f*2.0f)), "r"((float)M_PI*2.0f)
    : "$8","memory" );
#endif

    return ans;
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
    }

    asm volatile ("
    lqc2		vf16,0(%1)
    vmul.xyzw		vf17,vf16,vf16
    qmtc2.ni		%2,vf20
    vmulaw.xyzw		ACC,vf16,vf00
    vmul.xyzw		vf18,vf16,vf17
    qmtc2.ni		%3,vf21
    vmaddax.xyzw	ACC,vf18,vf20
    vmul.xyzw		vf16,vf18,vf17
    qmtc2.ni		%4,vf22
    vmaddax.xyzw	ACC,vf16,vf21
    vmul.xyzw		vf18,vf16,vf17
    qmtc2.ni		%5,vf23
    vmaddax.xyzw	ACC,vf18,vf22
    vmul.xyzw		vf16,vf18,vf17
    vmaddx.xyzw		vf16,vf16,vf23
    sqc2		vf16,0(%0)
    " : : "r"(ans),"r"(y),
    "r"(-1.0f/(3.0f*2.0f)),"r"(1.0f/(5.0f*4.0f*3.0f*2.0f)),
    "r"(-1.0f/(7.0f*6.0f*5.0f*4.0f*3.0f*2.0f)),
    "r"(1.0f/(9.0f*8.0f*7.0f*6.0f*5.0f*4.0f*3.0f*2.0f)) : "memory" );
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
    }

    asm volatile ("
    lh			$8,2(%1)
    lh			$9,6(%1)
    prot3w		$8,$8
    prot3w		$9,$9
    lh			$8,0(%1)
    lh			$9,4(%1)
    ppacw		$8,$9,$8
    qmtc2.ni		$8,vf16

    qmtc2.ni		%6,vf17
    vitof12.xyzw	vf16,vf16
    vmulx.xyzw		vf16,vf16,vf17

    vmul.xyzw		vf17,vf16,vf16
    qmtc2.ni		%2,vf20
    vmulaw.xyzw		ACC,vf16,vf00
    vmul.xyzw		vf18,vf16,vf17
    qmtc2.ni		%3,vf21
    vmaddax.xyzw	ACC,vf18,vf20
    vmul.xyzw		vf16,vf18,vf17
    qmtc2.ni		%4,vf22
    vmaddax.xyzw	ACC,vf16,vf21
    vmul.xyzw		vf18,vf16,vf17
    qmtc2.ni		%5,vf23
    vmaddax.xyzw	ACC,vf18,vf22
    vmul.xyzw		vf16,vf18,vf17
    vmaddx.xyzw		vf16,vf16,vf23
    sqc2		vf16,0(%0)
    " : : "r"(ans),"r"(y),
    "r"(-1.0f/(3.0f*2.0f)),"r"(1.0f/(5.0f*4.0f*3.0f*2.0f)),
    "r"(-1.0f/(7.0f*6.0f*5.0f*4.0f*3.0f*2.0f)),
    "r"(1.0f/(9.0f*8.0f*7.0f*6.0f*5.0f*4.0f*3.0f*2.0f)), "r"((float)M_PI*2.0f)
    : "$8","$9","memory" );
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
    asm volatile ("
    li			$8,0x007fffff
    and			$8,$8,%0
    bnez		$8,ljump
    nop
    addiu		%0,%0,1
ljump:
    qmtc2.ni		%0,vf16
    vrinit		R,vf16x
    " : : "r"(x) : "$8" );
}

static inline float vu0_Rand(void)
{
    float ans;

#if 0
    asm volatile ("
    vrnext.x		vf16,R
    qmfc2.i		$8,vf16
    mtc1		$8,%0
    " : "=f"(ans) : : "$8" );
#else
    asm volatile ("
    vrnext.x		vf16,R
    qmfc2.i		$8,vf16
    sw			$8,0(%0)
    " : : "r"(&ans) : "$8","memory" );
#endif

    return ans-1.0f;
}

/* PS系の角度を、ラジアンに変換 */
static inline void vu0_PSDegV2RadV(const SVECTOR * const s,FVECTOR *f)
{
    asm volatile ("
    lh		$8,2(%0)
    prot3w	$8,$8
    lh		$8,0(%0)
    lh		$9,4(%0)
    ppacw	$8,$9,$8
    psllw	$8,$8,32-12
    psraw	$8,$8,32-12
    qmtc2.ni	$8,vf16
    qmtc2.ni	%2,vf17
    vitof12.xyz	vf16,vf16
    vmulx.xyz	vf16,vf16,vf17
    sqc2	vf16,0(%1)
    " : : "r"(s), "r"(f), "r"((float)M_PI*2.0f) : "$8","$9","memory");
}

static inline void vu0_RadV2PSDegV(const FVECTOR * const f,SVECTOR *s)
{
    asm volatile ("
    lqc2	vf16,0(%0)
    qmtc2.ni	%2,vf17
    vmulx.xyz	vf16,vf16,vf17
    vftoi12.xyz	vf16,vf16
    qmfc2.i	$8,vf16
    sh		$8,0(%1)
    prot3w	$8,$8
    sh		$8,2(%1)
    prot3w	$8,$8
    sh		$8,4(%1)
    " : : "r"(f), "r"(s), "r"(1.0f/((float)M_PI*2.0f)) : "$8","memory");
}


/* ベクトルの変換 */
static inline void vu0_IV0toFVv0(void)
{
    asm volatile ("
    vitof0.xyzw	vf1,vf1
    ");
}

static inline void vu0_IV0toFVv1(void)
{
    asm volatile ("
    vitof0.xyzw	vf2,vf2
    ");
}

static inline void vu0_IV0toFVv2(void)
{
    asm volatile ("
    vitof0.xyzw	vf3,vf3
    ");
}

static inline void vu0_IV0toFV(const IVECTOR * const iv,FVECTOR *fv)
{
    vu0_Ldv0((FVECTOR *)iv);
    vu0_IV0toFVv0();
    vu0_Stv0(fv);
}

static inline void vu0_SV0toFV(const SVECTOR * const iv,FVECTOR *fv)
{
    vu0_LdSVv0(iv);
    vu0_IV0toFVv0();
    vu0_Stv0(fv);
}


static inline void vu0_IV12toFVv0(void)
{
    asm volatile ("
    vitof12.xyzw	vf1,vf1
    ");
}

static inline void vu0_IV12toFVv1(void)
{
    asm volatile ("
    vitof12.xyzw	vf2,vf2
    ");
}

static inline void vu0_IV12toFVv2(void)
{
    asm volatile ("
    vitof12.xyzw	vf3,vf3
    ");
}

static inline void vu0_IV12toFV(const IVECTOR * const iv,FVECTOR *fv)
{
    vu0_Ldv0((FVECTOR *)iv);
    vu0_IV12toFVv1();
    vu0_Stv0(fv);
}

static inline void vu0_SV12toFV(const SVECTOR * const iv,FVECTOR *fv)
{
    vu0_LdSVv0(iv);
    vu0_IV12toFVv1();
    vu0_Stv0(fv);
}


static inline void vu0_FVtoIV0v0(void)
{
    asm volatile ("
    vftoi0.xyzw	vf1,vf1
    ");
}

static inline void vu0_FVtoIV0v1(void)
{
    asm volatile ("
    vftoi0.xyzw	vf2,vf2
    ");
}

static inline void vu0_FVtoIV0v2(void)
{
    asm volatile ("
    vftoi0.xyzw	vf3,vf3
    ");
}

static inline void vu0_FVtoIV0(const FVECTOR * const fv,IVECTOR *iv)
{
    vu0_Ldv0(fv);
    vu0_FVtoIV0v0();
    vu0_Stv0((FVECTOR *)iv);
}

static inline void vu0_FVtoSV0(const FVECTOR * const fv,SVECTOR *iv)
{
    vu0_Ldv0(fv);
    vu0_FVtoIV0v0();
    vu0_StSVv0(iv);
}


static inline void vu0_FVtoIV12v0(void)
{
    asm volatile ("
    vftoi12.xyzw	vf1,vf1
    ");
}

static inline void vu0_FVtoIV12v1(void)
{
    asm volatile ("
    vftoi12.xyzw	vf2,vf2
    ");
}

static inline void vu0_FVtoIV12v2(void)
{
    asm volatile ("
    vftoi12.xyzw	vf3,vf3
    ");
}

static inline void vu0_FVtoIV12(const FVECTOR * const fv,IVECTOR *iv)
{
    vu0_Ldv0(fv);
    vu0_FVtoIV12v0();
    vu0_Stv0((FVECTOR *)iv);
}

static inline void vu0_FVtoSV12(const FVECTOR * const fv,SVECTOR *iv)
{
    vu0_Ldv0(fv);
    vu0_FVtoIV12v0();
    vu0_StSVv0(iv);
}


/* バウンディングボックス内に、ある点が存在するかどうかのチェック
   バウンディングボックス内に存在すれば１、しなければ０を返す。*/
static inline int vu0_CheckBoundingBox(const VECTOR * const vertex,
				       const VECTOR * const v0,const VECTOR * const v1)
{
    int ans;

    vu0_Ldv0(v0);
    vu0_Ldv1(v1);
    vu0_Addv2v0v1();
    vu0_Subv1v0v1();
    vu0_Ldv0(vertex);
    vu0_Mulv2a(1.0f/2.0f);
    vu0_Mulv1a(1.0f/2.0f);
    vu0_Subv0v2();

#if 0
    asm volatile ("
    vmulx.w	vf16,vf0,vf2
    vmuly.w	vf17,vf0,vf2
    vmulz.w	vf18,vf0,vf2
    vclipw.xyz	vf1,vf16
    vclipw.xyz	vf1,vf17
    vclipw.xyz	vf1,vf18
    vnop
    vnop
    vnop
    vnop
    cfc2	%0,$18
    " : "=r"(ans) );
#else
    asm volatile ("
    vmulx.w	vf16,vf0,vf2
    vmuly.w	vf17,vf0,vf2
    vmulz.w	vf18,vf0,vf2
    vclipw.xyz	vf1,vf16
    vclipw.xyz	vf1,vf17
    vclipw.xyz	vf1,vf18
    vnop
    vnop
    vnop
    vnop
    cfc2	$8,$vi18
    sw		$8,0(%0)
    " : : "r"(&ans) : "$8","memory" );
#endif

    return !(ans & ((1<<13)|(1<<12)|(1<<9)|(1<<8)|(1<<5)|(1<<4)));
}


/* ある点を中心とした指定範囲の中に、その点が含まれているかどうかを判別
   範囲内に存在すれば１、しなければ０を返す。*/
static inline int vu0_CheckRange(const VECTOR * const vertex,
				 const VECTOR * const v0,const VECTOR * const range)
{
    int ans;

    vu0_Ldv2(v0);
    vu0_Ldv0(vertex);
    vu0_Ldv1(range);
    vu0_Subv0v2();

#if 0
    asm volatile ("
    vmulx.w	vf16,vf0,vf2
    vmuly.w	vf17,vf0,vf2
    vmulz.w	vf18,vf0,vf2
    vclipw.xyz	vf1,vf16
    vclipw.xyz	vf1,vf17
    vclipw.xyz	vf1,vf18
    vnop
    vnop
    vnop
    vnop
    cfc2	%0,$18
    " : "=r"(ans) );
#else
    asm volatile ("
    vmulx.w	vf16,vf0,vf2
    vmuly.w	vf17,vf0,vf2
    vmulz.w	vf18,vf0,vf2
    vclipw.xyz	vf1,vf16
    vclipw.xyz	vf1,vf17
    vclipw.xyz	vf1,vf18
    vnop
    vnop
    vnop
    vnop
    cfc2	$8,$vi18
    sw		$8,0(%0)
    " : : "r"(&ans) : "$8","memory" );
#endif

    return !(ans & ((1<<13)|(1<<12)|(1<<9)|(1<<8)|(1<<5)|(1<<4)));
}

#endif //BP_ASM

#endif /* KP_XBOX ************** PS2用の専用関数(オリジナル)ここまで***********************  T.Morita 2002.02.01 */


#endif

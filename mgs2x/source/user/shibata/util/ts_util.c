//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ts_util.c
	いろいろ
	2000/05/17 T.Shibata
	$Id: ts_util.c,v 1.1.1.3 2002/11/19 11:48:54 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"
#include	"ts_util.h"
#include	"sin_table.h"

/* メインメモリからスクラッチパッドへ転送 */
void TS_Mem_Scr( void *dst, void *src, int size, int num )
{
	UTL_StartMemToSpr( dst, src, (size * num + (sizeof(u_long128)-1)) / sizeof(u_long128) );
	UTL_EndMemToSpr() ;
}

/* スクラッチパッドからメインメモリへ転送 */
void TS_Scr_Mem( void *dst, void *src, int size, int num )
{
	UTL_StartSprToMem( dst, src, (size * num + (sizeof(u_long128)-1)) / sizeof(u_long128) );
	UTL_EndSprToMem() ;
}

void TS_ScaleMatrix( FMATRIX *out_mat, FMATRIX *in_mat, FVECTOR *scale)
{
	//fpu_CopyMatrix(out_mat,in_mat);
	DG_COPY_VEC((FVECTOR*)&out_mat->m[3], (FVECTOR*)&in_mat->m[3]) ;

	out_mat->m[0][0] = in_mat->m[0][0] * scale->vx;
	out_mat->m[1][0] = in_mat->m[1][0] * scale->vx;
	out_mat->m[2][0] = in_mat->m[2][0] * scale->vx;	

	out_mat->m[0][1] = in_mat->m[0][1] * scale->vy;
	out_mat->m[1][1] = in_mat->m[1][1] * scale->vy;
	out_mat->m[2][1] = in_mat->m[2][1] * scale->vy;

	out_mat->m[0][2] = in_mat->m[0][2] * scale->vz;
	out_mat->m[1][2] = in_mat->m[1][2] * scale->vz;
	out_mat->m[2][2] = in_mat->m[2][2] * scale->vz;
}

void TS_RotMatrixX( FMATRIX *out_mat, FMATRIX *in_mat, float rot )
{
	FMATRIX fmtemp = {
		{
			{ 1.0f,      0.0f,      0.0f, 0.0f },
			{ 0.0f, cosf(rot),-sinf(rot), 0.0f },
			{ 0.0f, sinf(rot), cosf(rot), 0.0f },
			{ 0.0f,      0.0f,      0.0f, 1.0f },
		}
	};

	fpu_MulMatrices( out_mat, &fmtemp, in_mat );
	
}

void TS_RotMatrixY( FMATRIX *out_mat, FMATRIX *in_mat, float rot )
{
	FMATRIX fmtemp = {
		{
			{  cosf(rot), 0.0f, sinf(rot), 0.0f },
			{  		0.0f, 1.0f, 	 0.0f, 0.0f },
			{ -sinf(rot), 0.0f, cosf(rot), 0.0f },
			{       0.0f, 0.0f, 	 0.0f, 1.0f },
		}
	};

	fpu_MulMatrices( out_mat, &fmtemp, in_mat );
	
}

void TS_RotMatrixZ( FMATRIX *out_mat, FMATRIX *in_mat, float rot )
{
	FMATRIX fmtemp = {
		{
			{ cosf(rot),-sinf(rot), 0.0f, 0.0f },
			{ sinf(rot), cosf(rot), 0.0f, 0.0f },
			{      0.0f,      0.0f, 1.0f, 0.0f },
			{      0.0f,      0.0f, 0.0f, 1.0f }
		}
	};

	fpu_MulMatrices( out_mat, &fmtemp, in_mat );
}

void TS_RotMatrixZXY( FMATRIX *out_mat, FMATRIX *in_mat, FVECTOR *rot )
{
	TS_RotMatrixZ( out_mat,  in_mat, rot->vz );
	TS_RotMatrixX( out_mat, out_mat, rot->vx );
	TS_RotMatrixY( out_mat, out_mat, rot->vy );
}

//
void TS_MakeMatrix( FMATRIX *out_mat, FVECTOR *from_vec ,FVECTOR *pos )
{
	FVECTOR	outer = {0.0f,1.0f,0.0f,0.0f};
	FVECTOR *fvtemp = (FVECTOR*)out_mat;

	fpu_CopyVector(&fvtemp[3],pos);
	fvtemp[3].vw = 1.0f;
	
	fpu_CopyVector(&fvtemp[2],from_vec);
	fpu_VectorNormal(&fvtemp[2]);
	fvtemp[2].vw = 0.0f;

    _sceVu0OuterProduct(&fvtemp[0],&outer, from_vec );
	//fpu_OuterProduct(&fvtemp[0], &outer, from_vec );
	fpu_VectorNormal(&fvtemp[0]);
	fvtemp[0].vw = 0.0f;

	fpu_OuterProduct(&fvtemp[1], from_vec, &fvtemp[0] );
	fpu_VectorNormal(&fvtemp[1]);
	fvtemp[1].vw = 0.0f;

}
//vwの値に注意して使用してーん
//errのときは視点のＹ方向を使って作る
void TS_MakeMatrix2( FMATRIX *out_mat, FVECTOR *z_vec, FVECTOR *pouter, FVECTOR *pos )
{
	FVECTOR	outer = { 0.0f, 1.0f, 0.0f, 0.0f };
#ifndef BP_PS2
	FVECTOR *fvtemp = (FVECTOR*)out_mat;
#endif
	float	inner;

	if(pouter) DG_COPY_VEC( &outer, pouter );
	_sceVu0Normalize( &outer, &outer );
	inner = _sceVu0InnerProduct( &outer, z_vec );
	inner = (inner < 0.0f)?-inner:inner;
	if( (0.999999f < inner) ||
		(outer.vx == 0.0f && outer.vy == 0.0f && outer.vz == 0.0f)){
		//printf("Mega Hit Err!!<inner = %f>\n",inner);
		//DG_COPY_VEC( &outer, (FVECTOR*)DG_Chanls[0].eye.m[1] );

		DG_COPY_VEC(out_mat->m[0],DG_UnitMatrix.m[0]);
		DG_COPY_VEC(out_mat->m[1],DG_UnitMatrix.m[2]);out_mat->m[1][2] = -1.0f;
		DG_COPY_VEC(out_mat->m[2],DG_UnitMatrix.m[1]);
		DG_COPY_VEC(out_mat->m[3],pos);

      bp_math_assert( BP_Mat_Check(out_mat) );

		return;
	}
#ifndef BP_PS2
	DG_COPY_VEC( &fvtemp[3], pos );
	DG_COPY_VEC( &fvtemp[2], z_vec );
    _sceVu0OuterProduct( &fvtemp[0], &outer, &fvtemp[2] );
    _sceVu0OuterProduct( &fvtemp[1], &fvtemp[2], &fvtemp[0] );
	_sceVu0Normalize( &fvtemp[0], &fvtemp[0] );
	_sceVu0Normalize( &fvtemp[1], &fvtemp[1] );
	_sceVu0Normalize( &fvtemp[2], &fvtemp[2] );

   bp_math_assert( BP_Mat_Check(out_mat) );

#else
//  vf0	
//  vf1	
//  vf2	
//  vf3	outer
//  vf4	out_mat x
//  vf5	out_mat y
//  vf6	out_mat z
//  vf7	out_mat w
	  
	asm volatile ("
    lqc2		vf6 ,0x00(%1)
    lqc2		vf3 ,0x00(%2)
    lqc2		vf7 ,0x00(%0)
    vopmula.xyz	ACC,vf3,vf6
    vopmsub.xyz	vf4,vf6,vf3
    vopmula.xyz	ACC,vf6,vf4
    vopmsub.xyz	vf5,vf4,vf6

    vmul.xyz	vf1,vf4,vf4
    vmulax.w	ACC,vf0,vf1
    vmadday.w	ACC,vf0,vf1
    vmaddz.w	vf1,vf0,vf1
    vrsqrt	Q,vf0w,vf1w
    vwaitq
    vmulq.xyz	vf4,vf4,Q

    vmul.xyz	vf1,vf5,vf5
    vmulax.w	ACC,vf0,vf1
    vmadday.w	ACC,vf0,vf1
    vmaddz.w	vf1,vf0,vf1
    vrsqrt	Q,vf0w,vf1w
    vwaitq
    vmulq.xyz	vf5,vf5,Q

    vmul.xyz	vf1,vf6,vf6
    vmulax.w	ACC,vf0,vf1
    vmadday.w	ACC,vf0,vf1
    vmaddz.w	vf1,vf0,vf1
    vrsqrt	Q,vf0w,vf1w
    vwaitq
    vmulq.xyz	vf6,vf6,Q

	sqc2		vf4 ,0x00(%3)
	sqc2		vf5 ,0x10(%3)
	sqc2		vf6 ,0x20(%3)
	sqc2		vf7 ,0x30(%3)
	": : "r"(pos), "r"(z_vec), "r"(&outer), "r"(out_mat) );
#endif
//	fvtemp[0].vw = 0.0f;
//	fvtemp[1].vw = 0.0f;
//	fvtemp[2].vw = 0.0f;
//	fvtemp[3].vw = 1.0f;	
}


//r.vx = s.vx * t.vx
//r.vy = s.vy * t.vy
//r.vz = s.vz * t.vz
void vu0_MulVec( FVECTOR *r, FVECTOR *s, FVECTOR *t )
{
#ifdef BP_PSX2_ASM
	asm volatile ("
    lqc2		vf9 ,0x00(%1)
    lqc2		vf10,0x00(%2)
    vmul        vf8 ,vf9, vf10
	sqc2		vf8 ,0x00(%0)
	": : "r"(r), "r"(s), "r"(t) );
#else
	_sceVu0MulVector( &r, &s, &t );
#endif
}

//内分点
//比 s:t
#if 0
void TS_DividingPoint( FVECTOR *p, FVECTOR *a, FVECTOR *b, float s, float t )
{
#if 1
	p->vx = (t * a->vx + s * b->vx)/(s+t);
	p->vy = (t * a->vy + s * b->vy)/(s+t);
	p->vz = (t * a->vz + s * b->vz)/(s+t);
#else
	FVECTOR ta,sb;
	
	fpu_MulVectorScaler( &ta, a, t);
	fpu_MulVectorScaler( &sb, b, s);
	fpu_AddVectors( p, &ta, &sb);
	fpu_DivVectorScaler( p, p, s+t);
#endif
}
#endif

// SVECTOR(1:3:12)->FVECTOR
void Tra_SvecToFvec(FVECTOR *r,SVECTOR *a)
{
	r->vx = (float)a->vx/4096.0f;
	r->vy = (float)a->vy/4096.0f;
	r->vz = (float)a->vz/4096.0f;
}

//マトリクスからＲＯＴを求める
//ＲＯＴ（０から４０９６）
int TS_MatToRot( SVECTOR *rot, FMATRIX *mat )
{
	FMATRIX	fmat;

	rot->vx = 0;
	rot->vy = 0;
	rot->vz = 0;
	
	_sceVu0Normalize( fmat.m[0], mat->m[0]);
	_sceVu0Normalize( fmat.m[1], mat->m[1]);
	_sceVu0Normalize( fmat.m[2], mat->m[2]);
	
//	FastInverseMatrix( &fmat, &fmat );

	if( (fmat.m[0][1] == 0.0f && fmat.m[1][1] == 0.0f) ||
		(fmat.m[2][0] == 0.0f && fmat.m[2][2] == 0.0f)){
		printf("mega hit err!!");
		return -2;
	}

	if(-fmat.m[2][1] <= -1.0f){
		rot->vz = 0;
		rot->vx = -2048;
		rot->vy = (short)(2048.0f * atan2f(-fmat.m[2][0],fmat.m[2][2]) / PI);
		return -1;
	}else if(-fmat.m[2][1] >= 1.0f){
		rot->vz = 0;
		rot->vx = 2048;
		rot->vy = -(short)(2048.0f * atan2f(-fmat.m[2][0],fmat.m[2][2]) / PI);
		return 1;
	}

	rot->vx = (short)(2048.0f * asinf( -fmat.m[2][1]) / PI);
	rot->vy = (short)(2048.0f * atan2f(fmat.m[2][0],fmat.m[2][2]) / PI);
	rot->vz = (short)(2048.0f * atan2f(fmat.m[0][1],fmat.m[1][1]) / PI);

//	rot->pad = rot->vz;
//	rot->vz = rot->vx;
//	rot->vx = rot->vy;
//	rot->vy = rot->pad;
//	rot x 200:y -600: z 800 -> rot x 799:y 200: z -599
//									z		x     y
	
	return 0;
}
//ベクトルからＲＯＴを求める
//ＲＯＴ（０から４０９６）
void TS_VecToRot( SVECTOR *rot, FVECTOR *vec )
{
	rot->vx = (short)(2048.0f * asinf( -vec->vy ) / PI);
	rot->vy = (short)(2048.0f * atan2f( vec->vx , vec->vz ) / PI);
	rot->vz = 0;
}

float _TS_Sin( int s )
{
	u_int	temp = (u_int)s;
	float	ret;

//	if(temp >= 4096) temp %= 4096;
//	while(temp < 0){ temp += 4096; }
	temp &= 0x0fff;
	if(temp > 3072){
		//第４
		temp = 4096 - temp;
		ret = -(float)SinCosTable[temp]/TS_ONE;
	}else if(temp > 2048){
		//第３
		temp = temp - 2048;
		ret = -(float)SinCosTable[temp]/TS_ONE;
	}else if(temp > 1024){
		//第２
		temp = 2048 - temp;
		ret = (float)SinCosTable[temp]/TS_ONE;
	}else{
		//第１
		ret = (float)SinCosTable[temp]/TS_ONE;
	}
	return ret;
}

//pos:法線生成のためポジション三つ使います
int GetRGBFromLightPos( FVECTOR *pos, FMATRIX *light )
{
	//int col;
	FVECTOR	color;
	static FVECTOR max_col = { 255.0f, 255.0f, 255.0f, 0.0f };

#ifdef BP_PSX2_ASM
	asm ("
		lqc2		vf04,	0x00(%2)	#pos
		lqc2		vf05,	0x10(%2)
		lqc2		vf06,	0x20(%2)

		lqc2		vf08,	0x00(%1)	#vec L0	0
		lqc2		vf09,	0x10(%1)	#vec L0	1
		lqc2		vf10,	0x20(%1)	#vec L0	2

		lqc2		vf12,	0x40(%1)	#col L1 0
		lqc2		vf13,	0x50(%1)	#col L1 1
		lqc2		vf14,	0x60(%1)	#col L1 2
		lqc2		vf15,	0x70(%1)	#col L1 3

		lqc2		vf01,	0x00(%3)	#vmin用

		#norm生成
		vsub.xyzw	vf16, vf05, vf04
		vsub.xyzw	vf17, vf06, vf04

		vopmula.xyz	ACC,  vf16, vf17
		vopmsub.xyz	vf18, vf17, vf16

		vmul.xyz	vf19, vf18, vf18
		vmulax.w	ACC,  vf00, vf19
		vmadday.w	ACC,  vf00, vf19
		vmaddz.w	vf01, vf00, vf19
		vrsqrt		Q,   vf00w, vf01w
		vwaitq
		vmulq.xyz	vf03, vf18, Q		#vf03:norm

		vmulax.xyzw		ACC,  vf08, vf03x
		vmadday.xyzw	ACC,  vf09, vf03y
		vmaddz.xyzw		vf20, vf10, vf03z

		vmax.xyzw		vf21, vf20, vf00

		vmulax.xyzw		ACC,  vf12, vf21x
		vmadday.xyzw	ACC,  vf13, vf21y
		vmaddaz.xyzw	ACC,  vf14, vf21z
		vmaddw.xyzw		vf22, vf15, vf00w

		vmini.xyzw		vf23, vf22, vf1
			
		sqc2			vf23,0x00(%0)

	": : "r"(&color), "r"(light), "r"(pos), "r"(&max_col) : "memory" );
#else
	FVECTOR	vec1, vec2, vec3 ;
	float q ;

	_sceVu0SubVector( &vec1, (pos+1), pos ) ;
	_sceVu0SubVector( &vec2, (pos+2), pos ) ;

	_sceVu0OuterProduct( &vec2, &vec1, &vec2 );
	_sceVu0MulVectorXYZ( &vec1, &vec2, &vec2 );
	q = 1.0f/bp_sqrtf( vec1.vx + vec1.vy + vec1.vz ) ; //BP_MATH - emulate PS2 sqrtf
	_sceVu0ScaleVector( &vec3, &vec2, q ) ;

	DG_SetPos( light ) ;
	DG_PutVector( &vec3, &vec1, 1 ) ;

	vec1.vx = ( vec1.vx > 0.0f ) ? vec1.vx : 0.0f ;
	vec1.vy = ( vec1.vy > 0.0f ) ? vec1.vy : 0.0f ;
	vec1.vz = ( vec1.vz > 0.0f ) ? vec1.vz : 0.0f ;
	vec1.vw = ( vec1.vw > 1.0f ) ? vec1.vw : 1.0f ;

	DG_SetPos( light+1 ) ;
	DG_PutVector( &vec1, &vec2, 1 ) ;
	vec2.vx += (light+1)->m[3][0] ;
	vec2.vy += (light+1)->m[3][1] ;
	vec2.vz += (light+1)->m[3][2] ;
	vec2.vw += (light+1)->m[3][3] ;

	color.vx = ( vec2.vx < max_col.vx ) ? vec2.vx : max_col.vx ;
	color.vy = ( vec2.vy < max_col.vy ) ? vec2.vy : max_col.vy ;
	color.vz = ( vec2.vz < max_col.vz ) ? vec2.vz : max_col.vz ;
	color.vw = ( vec2.vw < max_col.vw ) ? vec2.vw : max_col.vw ;
#endif

	return ((int)color.vx | (((int)color.vy<<8)) | (((int)color.vz<<16)) );
}
//
int GetRGBFromLightNorm( FVECTOR *norm, FMATRIX *light )
{
	//int col;
	FVECTOR	color;
	static FVECTOR max_col = { 255.0f, 255.0f, 255.0f, 0.0f };

#ifdef BP_PSX2_ASM
	asm ("
		lqc2		vf03,	0x00(%2)	#norm

		lqc2		vf08,	0x00(%1)	#vec L0	0
		lqc2		vf09,	0x10(%1)	#vec L0	1
		lqc2		vf10,	0x20(%1)	#vec L0	2

		lqc2		vf12,	0x40(%1)	#col L1 0
		lqc2		vf13,	0x50(%1)	#col L1 1
		lqc2		vf14,	0x60(%1)	#col L1 2
		lqc2		vf15,	0x70(%1)	#col L1 3

		lqc2		vf01,	0x00(%3)	#vmin用

		vmulax.xyzw		ACC,  vf08, vf03x
		vmadday.xyzw	ACC,  vf09, vf03y
		vmaddz.xyzw		vf20, vf10, vf03z

		vmax.xyzw		vf21, vf20, vf00

		vmulax.xyzw		ACC,  vf12, vf21x
		vmadday.xyzw	ACC,  vf13, vf21y
		vmaddaz.xyzw	ACC,  vf14, vf21z
		vmaddw.xyzw		vf22, vf15, vf00w

		vmini.xyzw		vf23, vf22, vf1
			
		sqc2			vf23,0x00(%0)

	": : "r"(&color), "r"(light), "r"(norm), "r"(&max_col) : "memory" );
#else
	FVECTOR vec1, vec2 ;

	DG_SetPos( light ) ;
	DG_RotVector( norm, &vec1, 1 ) ;

	vec1.vx = ( vec1.vx > 0.0f ) ? vec1.vx : 0.0f ;
	vec1.vy = ( vec1.vy > 0.0f ) ? vec1.vy : 0.0f ;
	vec1.vz = ( vec1.vz > 0.0f ) ? vec1.vz : 0.0f ;
	vec1.vw = ( vec1.vw > 1.0f ) ? vec1.vw : 1.0f ;

	DG_SetPos( light+1 ) ;
	DG_PutVector( &vec1, &vec2, 1 ) ;

	color.vx = ( vec2.vx < max_col.vx ) ? vec2.vx : max_col.vx ;
	color.vy = ( vec2.vy < max_col.vy ) ? vec2.vy : max_col.vy ;
	color.vz = ( vec2.vz < max_col.vz ) ? vec2.vz : max_col.vz ;
	color.vw = ( vec2.vw < max_col.vw ) ? vec2.vw : max_col.vw ;
#endif

	return ((int)color.vx | (((int)color.vy<<8)) | (((int)color.vz<<16)) );
}

//64頂点まで
//DG_SetPrim2Alphaは取りあえずアルファです
DG_PRIM2 *InitStripPrim( int tex_code, int n_strip, short *uv_pix, FVECTOR *init_pos )
{
	DG_PRIM2 			*prim;
	DG_TEX				*tex;
	FVECTOR				*pos0, *pos1;
	DG_PRIM2_UVRGB		*uvrgb0, *uvrgb1;
	short				*uv = uv_pix;
	int					n_verts = 2*n_strip+2;
	int					i,th,tw,off_u,off_v;

	tex = DG_GetTexture(tex_code);
	if(!tex){ printf("ERR!! NO TEX!! ts_util.c\n"); return (NULL); }

	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
						 1,
						 n_verts );
	if(!prim){ printf("ERR!! MAKE PRIM2!! ts_util.c\n"); return (NULL); }

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	DG_GetTexelInfo( NULL, NULL, &off_u, &off_v, tex );
	tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f ;
	th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
	tw = 1 << tw ;
	th = 1 << th ;

	pos0 = prim->pos[0];
	pos1 = prim->pos[1];
	uvrgb0 = prim->uvrgb[0];
	uvrgb1 = prim->uvrgb[1];

	for( i = 0; i < n_verts; i++ ){
		DG_COPY_VEC( pos0, &init_pos[i] );
		DG_COPY_VEC( pos1, &init_pos[i] );

		uvrgb0->r = 0x80;
		uvrgb0->g = 0x80;
		uvrgb0->b = 0x80;
		uvrgb0->a = 0x80;

		uvrgb0->q = 4096;
		uvrgb0->f = (i<2)?VERT_KICK_CODE:DRAW_KICK_CODE;

		uvrgb0->u = FTOI12( ((float)(off_u + uv[0]))/(float)tw );
		uvrgb0->v = FTOI12( ((float)(off_v + uv[1]))/(float)th );

		//uvrgb0->u = (i&1)?u1:u0;
		//uvrgb0->v = FTOI12((float)(i/2) * 1.0f/32.0f * tex->v_scale + tex->v_offset );

		*uvrgb1 = *uvrgb0;

		uv+=2;
		pos0++; pos1++;
		uvrgb0++; uvrgb1++;
	}
	
	return prim;
}

typedef struct {
	short	vx;
	short	vy;
	short	vz;
	short	u;
	short	v;
	short	flags;
	short	count;
	short	nx;
	short	ny;
	short	nz;
}VERTS_DATA;

//ライト計算付
#pragma optimize("",off)
void TS_PolyVertsLight( FVECTOR *ppos, DG_PRIM2_UVRGB *puv, int n_verts, int alpha, VERTS_DATA *vdata, FMATRIX *light, FMATRIX *root )
{
	int 			i;
	DG_PRIM2_UVRGB	*uvrgb = puv;
	FVECTOR			*pos = ppos;
	VERTS_DATA 		*data = vdata;
	static FVECTOR	max_col = { 255.0f, 255.0f, 255.0f, 0.0f };
	
	//最初にＬ０とワールドだけ計算してしまう。
	//vf16<=>vf19に入れとく
#ifdef BP_PSX2_ASM
	asm ("
		lqc2		vf4,	0x00(%0)	#方向
		lqc2		vf5,	0x10(%0)
		lqc2		vf6,	0x20(%0)
		lqc2		vf7,	0x30(%0)

		lqc2		vf8,	0x00(%1)	#world
		lqc2		vf9,	0x10(%1)
		lqc2		vf10,	0x20(%1)
		lqc2		vf11,	0x30(%1)

		lqc2		vf12,	0x40(%0)	#col
		lqc2		vf13,	0x50(%0)
		lqc2		vf14,	0x60(%0)
		lqc2		vf15,	0x70(%0)

		lqc2		vf1,	0x00(%2)	#vmin用

		vmulax.xyzw		ACC,  vf4, vf8x
		vmadday.xyzw	ACC,  vf5, vf8y
		vmaddaz.xyzw	ACC,  vf6, vf8z
		vmaddw.xyzw		vf16, vf7, vf8w

		vmulax.xyzw		ACC,  vf4, vf9x
		vmadday.xyzw	ACC,  vf5, vf9y
		vmaddaz.xyzw	ACC,  vf6, vf9z
		vmaddw.xyzw		vf17, vf7, vf9w

		vmulax.xyzw		ACC,  vf4, vf10x
		vmadday.xyzw	ACC,  vf5, vf10y
		vmaddaz.xyzw	ACC,  vf6, vf10z
		vmaddw.xyzw		vf18, vf7, vf10w

		vmulax.xyzw		ACC,  vf4, vf0x
		vmadday.xyzw	ACC,  vf5, vf0y
		vmaddaz.xyzw	ACC,  vf6, vf0z
		vmaddw.xyzw		vf19, vf7, vf0w

	": : "r"(light), "r"(root), "r"(&max_col) : "memory" );
#else
	FVECTOR	vec1, vec2 ;
	FMATRIX	mat ;
//yano 2002.03.04
  	_sceVu0MulMatrix( &mat, light, root );
	_sceVu0ApplyMatrix( (FVECTOR *)mat.m[3], light, &DG_ZeroVector );
#endif

	for( i = 0; i < n_verts; i++ ){
		FVECTOR		norm;
		FVECTOR		temp;

		temp.vx = (float)data->vx ;
		temp.vy = (float)data->vy ;
		temp.vz = (float)data->vz ;
		temp.vw = 1.0f;

		norm.vx = (float)data->nx/4096.0f;
		norm.vy = (float)data->ny/4096.0f;
		norm.vz = (float)data->nz/4096.0f;
		norm.vw = 0.0f;

#ifdef BP_PSX2_ASM
		asm ("
			lqc2			vf3,	0x00(%1)
			lqc2			vf4,	0x00(%2)

			#pos
			vmulax.xyzw			ACC,  vf08, vf04
			vmadday.xyzw		ACC,  vf09, vf04
			vmaddaz.xyzw		ACC,  vf10, vf04
			vmaddw.xyzw			vf24, vf11, vf04
			sqc2				vf24, 0x00(%0)

			#light
			vmulax.xyzw		ACC,  vf16, vf3x
			vmadday.xyzw	ACC,  vf17, vf3y
			vmaddaz.xyzw	ACC,  vf18, vf3z
			vmaddw.xyzw		vf20, vf19, vf3w

			vmax.xyzw		vf21, vf20, vf0

			vmulax.xyzw		ACC,  vf12, vf21x
			vmadday.xyzw	ACC,  vf13, vf21y
			vmaddaz.xyzw	ACC,  vf14, vf21z
			vmaddw.xyzw		vf22, vf15, vf21w

			vmini.xyzw		vf23, vf22, vf1
			
			sqc2			vf23, 0x00(%1)
		": : "r"(pos), "r"(&norm), "r"(&temp): "memory" );
#else
		_sceVu0ApplyMatrix( pos , root, &temp );
		_sceVu0ApplyMatrix( &vec1, &mat , &norm );

		vec1.vx = ( vec1.vx > 0.0f ) ? vec1.vx : 0.0f ;
		vec1.vy = ( vec1.vy > 0.0f ) ? vec1.vy : 0.0f ;
		vec1.vz = ( vec1.vz > 0.0f ) ? vec1.vz : 0.0f ;
		vec1.vw = ( vec1.vw > 1.0f ) ? vec1.vw : 1.0f ;

		_sceVu0ApplyMatrix( &vec2, (light + 1), &vec1 );

		norm.vx = ( vec2.vx < max_col.vx ) ? vec2.vx : max_col.vx ;
		norm.vy = ( vec2.vy < max_col.vy ) ? vec2.vy : max_col.vy ;
		norm.vz = ( vec2.vz < max_col.vz ) ? vec2.vz : max_col.vz ;
		norm.vw = ( vec2.vw < max_col.vw ) ? vec2.vw : max_col.vw ;
#endif
		uvrgb->a = alpha ;

#ifdef PSX2
		uvrgb->r = (int)norm.vx;
		uvrgb->g = (int)norm.vy;
		uvrgb->b = (int)norm.vz;
#else
		uvrgb->r = (u_short)norm.vx ;
		uvrgb->g = (u_short)norm.vy ;
		uvrgb->b = (u_short)norm.vz ;
#endif

		pos++;
		data++;
		uvrgb++;
	}
}
#pragma optimize("",on)


#if 0

typedef struct
{
	GV_ACT_EX		actor;
	DG_PRIM2		*prim;
	int				life;
} Work ;

static void Act(Work *work)
{
	if( --work->life < 0 ){
		DG_InvisiblePrim2(work->prim) ;
		GV_DestroyActor(work) ;
	}
}

static void Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim);
}

static int GetResources( Work *work, FVECTOR *verts, int n_verts, int color )
{
	DG_PRIM2		*prim = NULL;
	DG_TEX			*tex = NULL;
	FVECTOR			*pos0,*pos1,*wpos = verts;
	DG_PRIM2_UVRGB	*uvrgb0,*uvrgb1;
	int				i,r,g,b,a;

	r = (color>>24)&0xff;
	g = (color>>16)&0xff;
	b = (color>> 8)&0xff;
	a = color&0xff;
	
	tex = DG_GetTexture(6715088);	
	prim = work->prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_ANTIALIASING,
									  1,
									  n_verts );
	if(!prim){
		printf("null prim\n");
		return (-1);
	}
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	// DataInit
	work->life = 0;

	pos0 = prim->pos[0];
	pos1 = prim->pos[1];
	uvrgb0 = prim->uvrgb[0];
	uvrgb1 = prim->uvrgb[1];
	
	for( i = 0; i < n_verts; i++ ){
		DG_COPY_VEC( pos0, wpos );
		DG_COPY_VEC( pos1, wpos );

		uvrgb0->r = r;
		uvrgb0->g = g;
		uvrgb0->b = b;
		uvrgb0->a = a;
		uvrgb0->q = 4096;
		uvrgb0->f = (i==0)?VERT_KICK_CODE:DRAW_KICK_CODE;
		uvrgb0->u = 0;
		uvrgb0->v = 0;
		
		*uvrgb1 = *uvrgb0;
		wpos++;
		pos0++; pos1++;
		uvrgb0++; uvrgb1++;
	}
	return (0);
}
/*
color 0xrrggbbaa
*/
void *NewTsDebugLine( FVECTOR *verts, int n_verts, int color )
{
	Work *work = NULL;
	
	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));

	if(!work) return NULL;
	
	GV_SetActor(&(work->actor),Act,Die) ;
	GV_ActorEX( &work->actor ) ;
	if(GetResources( work, verts, n_verts, color ) < 0){
		GV_DestroyActor(work) ;
		return NULL ;
	}

	return work;
}
#endif

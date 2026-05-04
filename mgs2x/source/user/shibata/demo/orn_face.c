//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    org_face.c
	オルガマスクエフェクト

	2001/04/01 T.Shibata

	$Id: orn_face.c,v 1.1.1.3 2002/11/19 11:48:32 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

#include	"../util/ts_util.h"

#define CLOCK_COUNT	(BP_BASE_TICK())


//static void ActScrPrim( DG_PRIM2 *prim, int n_verts, int alpha );

extern void *NewGeneralSprite2( int name, int tex_code, FMATRIX *world, FVECTOR *pos, float shift,
								int init_rgba, int w,
								int life, int mode, int act_time, int sleep_time );

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

#define		GET_COL_R(_rgba)	(((_rgba)>>24)&0xff)
#define		GET_COL_G(_rgba)	(((_rgba)>>16)&0xff)
#define		GET_COL_B(_rgba)	(((_rgba)>> 8)&0xff)
#define		GET_COL_A(_rgba)	(((_rgba)>> 0)&0xff)
#define		SET_COL(_r,_g,_b,_a)	(((_r)<<24)|((_g)<<16)|((_b)<<8)|(_a))
//#define 	TS_DEBUG

#define MEM_SCR_POS		((void*)(SCRPAD_ADDR))
#define MEM_SCR_UV		((void*)(SCRPAD_ADDR+sizeof(FVECTOR)*256))

typedef struct {
	GV_ACT_EX		actor;
	int				name;
	OBJECT			*object;
	FVECTOR			*mov;
	int				flags;

	FMATRIX			lights[2];
	FMATRIX			root;

	int				console_timer;
	DG_OBJS			*objs;
	int				fadein_time;
	int				fadeout_time;
	float			diff_alpha0;
	float			diff_alpha1;
	float			to_alpha;
	//メット関係
	DG_PRIM2		*met_prim;
	DG_PRIM2		*cover_prim;
	DG_PRIM2		*noiz_prim;

	int				timer;
	int				msk_step;
	
	int				wave_time;
	float			add_v_off;

	float			met_alp;
	float			cover_alp;
	float			noiz_alp;

	float			v_offset;

	float			tex_v_off;
	float			tex_v_scl;

	DG_PRIM2		*txt_r_prim;
	DG_PRIM2		*txt_l_prim;
	DG_TEX			*txt_r_tex;
	DG_TEX			*txt_l_tex;

	
	DG_PRIM2		*mitsu_prim;
	DG_PRIM2		*square_prim;

} Work;

extern int DM_FrameSkip ;

#define		VOFF_MAX	(112.0f)
#define		NOIZ_SIZE	(224.0f)
#define		VOFF_MIN	(0.0f)
#define		SCALE_SIZE	(2.0f)

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

#define N_MASK_VERTS	(sizeof(MaskVerts)/sizeof(MaskVerts[0]))
//4/4 copy
static VERTS_DATA MaskVerts[] = {
	{   66,   69,   81, 2389, 1700, 0x8000,  486, -3709, -110, -1733, },
	{   57,   15,   99, 2116, 2614, 0x8000,  468, -3541,  738, -1920, },
	{   44,   47,  125, 1615, 2102, 0x0020,  442, -2950, -693, -2755, },
	{   31,   -3,  127, 1479, 2846, 0x0000,  440, -3132,  891, -2484, },
	{   17,   40,  145, 1206, 2176, 0x0020,  422, -1421, -1325, -3605, },
	{   16,  -18,  141, 1166, 3151, 0x0000,  426, -1139, 1090, -3780, },
	{  -17,   40,  145,  482, 2176, 0x0020,  422, 1216, -1087, -3756, },
	{  -16,  -18,  141,  522, 3151, 0x0000,  426, 1725, 1026, -3570, },
	{  -31,   -3,  127,  209, 2840, 0x0020,  440, 3132,  891, -2484, },

	{  -44,   47,  125,   72, 2102, 0x8000,  442, 2950, -693, -2755, },
	{  -38,   61,  120,   72, 1810, 0x8000,  447, 1499, -2402, -2959, },
	{  -17,   40,  145,  482, 2176, 0x0020,  422, 1216, -1087, -3756, },
	{    0,   55,  129,  847, 1921, 0x0000,  438,    0, -3043, -2741, },
	{   17,   40,  145, 1206, 2176, 0x0020,  422, -1421, -1325, -3605, },
	{   38,   61,  120, 1615, 1810, 0x0000,  447, -1499, -2402, -2959, },
	{   44,   47,  125, 1615, 2102, 0x0020,  442, -2950, -693, -2755, },
	{   51,   73,  100, 1888, 1627, 0x0000,  467, -2792, -1652, -2500, },
	{   66,   69,   81, 2389, 1700, 0x0020,  486, -3709, -110, -1733, },

	{   45,  -26,   93, 1979, 3200, 0x8000,  474, -3158, 1177, -2327, },
	{   41,  -31,   97, 1843, 3273, 0x8000,  470, -3270, 1034, -2239, },
	{   31,   -3,  127, 1479, 2846, 0x0020,  440, -3132,  891, -2484, },
	{   16,  -55,  121, 1160, 3821, 0x0000,  446, -2615, 1408, -2820, },
	{   16,  -18,  141, 1166, 3151, 0x0020,  426, -1139, 1090, -3780, },
	{  -16,  -55,  121,  527, 3821, 0x0000,  446, 2142, 1585, -3109, },
	{  -16,  -18,  141,  522, 3151, 0x0020,  426, 1725, 1026, -3570, },

	{   74,   59,   60, 2753, 1846, 0x8000,  507, -3696,  557, -1673, },
	{   74,   47,   56, 2798, 2029, 0x8000,  511, -3877,  277, -1290, },
	{   66,   69,   81, 2389, 1700, 0x0020,  486, -3709, -110, -1733, },
	{   71,   24,   64, 2707, 2474, 0x0000,  503, -3897,  406, -1191, },
	{   57,   15,   99, 2116, 2614, 0x0020,  468, -3541,  738, -1920, },
	{   61,    2,   78, 2434, 2761, 0x0000,  489, -3798,  883, -1250, },
	{   53,  -25,   85, 2207, 3200, 0x0020,  482, -3413, 1044, -2007, },

	{   31,   -3,  127, 1479, 2846, 0x8000,  440, -3132,  891, -2484, },
	{   57,   15,   99, 2116, 2614, 0x8000,  468, -3541,  738, -1920, },
	{   45,  -26,   93, 1979, 3200, 0x0020,  474, -3158, 1177, -2327, },
	{   53,  -25,   85, 2207, 3200, 0x0000,  482, -3413, 1044, -2007, },

	{   74,   47,   56, 2798, 2029, 0x8000,  511, -3877,  277, -1290, },
	{   76,   43,   46, 3026, 2102, 0x8000,  521, -3990,  194, -903, },
	{   71,   24,   64, 2707, 2474, 0x0020,  503, -3897,  406, -1191, },
	{   75,   15,   45, 3026, 2651, 0x0000,  522, -3909,  557, -1087, },
	{   61,    2,   78, 2434, 2761, 0x0020,  489, -3798,  883, -1250, },

//切れ目41頂点の
	{  -16,  -55,  121, 1160, 3821, 0x8000,  446, 2142, 1585, -3109, },
	{  -41,  -31,   97, 1843, 3273, 0x8000,  470, 3270, 1034, -2239, },
	{  -31,   -3,  127, 1479, 2846, 0x0020,  440, 3132,  891, -2484, },
	{  -45,  -26,   93, 1979, 3200, 0x0000,  474, 3158, 1177, -2327, },
	{  -57,   15,   99, 2116, 2614, 0x0020,  468, 3541,  738, -1920, },
	{  -53,  -25,   85, 2207, 3200, 0x0000,  482, 3413, 1044, -2007, },
	{  -61,    2,   78, 2434, 2761, 0x0020,  489, 3798,  883, -1250, },

	{  -74,   59,   60, 2753, 1846, 0x8000,  507, 3696,  557, -1673, },
	{  -66,   69,   81, 2389, 1700, 0x8000,  486, 3709, -110, -1733, },
	{  -74,   47,   56, 2798, 2029, 0x0020,  511, 3877,  277, -1290, },
	{  -71,   24,   64, 2707, 2474, 0x0000,  503, 3897,  406, -1191, },
	{  -76,   43,   46, 3026, 2102, 0x0020,  521, 3990,  194, -903, },
	{  -75,   15,   45, 3026, 2651, 0x0000,  522, 3909,  557, -1087, },

	{  -61,    2,   78, 2434, 2761, 0x8000,  489, 3798,  883, -1250, },
	{  -71,   24,   64, 2707, 2474, 0x8000,  503, 3897,  406, -1191, },
	{  -57,   15,   99, 2116, 2614, 0x0020,  468, 3541,  738, -1920, },
	{  -66,   69,   81, 2389, 1700, 0x0000,  486, 3709, -110, -1733, },
	{  -44,   47,  125, 1615, 2102, 0x0020,  442, 2950, -693, -2755, },

	{  -66,   69,   81, 2389, 1700, 0x8000,  486, 3709, -110, -1733, },
	{  -51,   73,  100, 1888, 1627, 0x8000,  467, 2792, -1652, -2500, },
	{  -44,   47,  125, 1615, 2102, 0x0020,  442, 2950, -693, -2755, },
	{  -38,   61,  120, 1615, 1810, 0x0000,  447, 1499, -2402, -2959, },

	{  -31,   -3,  127,  209, 2840, 0x8000,  440, 3132,  891, -2484, },
	{  -44,   47,  125,   72, 2102, 0x8000,  442, 2950, -693, -2755, },
	{  -17,   40,  145,  482, 2176, 0x0020,  422, 1216, -1087, -3756, },

	{   41,  -31,   97, 1843, 3273, 0x8000,  470, -3270, 1034, -2239, },
	{   39,  -41,   96, 1843, 3456, 0x8000,  471, -3177,  878, -2431, },
	{   16,  -55,  121, 1160, 3821, 0x0020,  446, -2615, 1408, -2820, },

	{  -31,   -3,  127, 1479, 2846, 0x8000,  440, 3132,  891, -2484, },
	{  -57,   15,   99, 2116, 2614, 0x8000,  468, 3541,  738, -1920, },
	{  -44,   47,  125, 1615, 2102, 0x0020,  442, 2950, -693, -2755, },

	{  -61,    2,   78, 2434, 2761, 0x8000,  489, 3798,  883, -1250, },
	{  -75,   15,   45, 3026, 2651, 0x8000,  522, 3909,  557, -1087, },
	{  -71,   24,   64, 2707, 2474, 0x0020,  503, 3897,  406, -1191, },

	{  -16,  -55,  121, 1160, 3821, 0x8000,  446, 2142, 1585, -3109, },
	{  -39,  -41,   96, 1843, 3456, 0x8000,  471, 3177,  878, -2431, },
	{  -41,  -31,   97, 1843, 3273, 0x0020,  470, 3270, 1034, -2239, },

	{  -16,  -55,  121, 1160, 3821, 0x8000,  446, 2142, 1585, -3109, },
	{  -31,   -3,  127, 1479, 2846, 0x8000,  440, 3132,  891, -2484, },
	{  -16,  -18,  141, 1166, 3151, 0x0020,  426, 1725, 1026, -3570, },
	{  -16,  -18,  141, 1166, 3151, 0x0020,  426, 1725, 1026, -3570, },
//１頂点ダミー
};

static FVECTOR OrgEftTxt_R_Pos[] = {
	{ -65.0f, 49.0f,  66.0f, 1.0f },
	{ -45.0f, 49.0f, 101.0f, 1.0f },
	{ -65.0f, 15.0f,  66.0f, 1.0f },
	{ -45.0f, 15.0f, 101.0f, 1.0f },
};

static FVECTOR OrgEftTxt_L_Pos[] = {
	{ 44.0f, 49.0f, 101.0f, 1.0f },
	{ 64.0f, 49.0f,  66.0f, 1.0f },
	{ 44.0f, 15.0f, 101.0f, 1.0f },
	{ 64.0f, 15.0f,  66.0f, 1.0f },
};

static FVECTOR OrgEftMitsu_Pos[] = {
	{ -33.0f, 22.0f, 121.0f, 1.0f },
	{  -9.0f, 22.0f, 139.0f, 1.0f },
	{ -33.0f, -5.0f, 121.0f, 1.0f },
	{  -9.0f, -5.0f, 139.0f, 1.0f },
};

static FVECTOR OrgEftSquare_Pos[] = {
	{  8.0f, 44.0f, 139.0f, 1.0f },
	{ 37.0f, 44.0f, 118.0f, 1.0f },
	{  8.0f, -4.0f, 139.0f, 1.0f },
	{ 37.0f, -4.0f, 118.0f, 1.0f },
};

enum {
	ORGFAEC_STEP_INIT = 0,
	ORGFAEC_STEP_FADEIN,
	ORGFAEC_STEP_FADEOUT,
	ORGFAEC_STEP_NOIZ_FADEIN,		//ノイズ見え
	ORGFAEC_STEP_MET_FADEOUT,		//ここから中身が見える（分岐：元に戻るか、ノイズも消えるか)

	ORGFAEC_STEP_MET_FADEIN,		//中身消えーの
	ORGFAEC_STEP_NOIZ_FADEOUT,		//ノイズ消えーの

	//ノイズ消え追加する
	ORGFAEC_STEP_FADE_FIX,			//半透明固定
	
};

void BP_DG_PutVectorW(FVECTOR *from, FVECTOR *to, int n)
{
   FMATRIX world;
   DG_GetPos(&world);
   while ( -- n >= 0 ) {
      BP_Mat_TransformVec4( to, &world, from );
      from ++ ;
      to ++ ;
   }
}

void BP_DG_RotVectorW(FVECTOR *from, FVECTOR *to, int n)
{
   FMATRIX world;
   DG_GetPos(&world);
   while ( -- n >= 0 ) {
      BP_Mat_RotateVec4( to, &world, from );
      from ++ ;
      to ++ ;
   }
}

void BP_DG_RotVector(FVECTOR *from, FVECTOR *to, int n)
{
   FMATRIX world;
   DG_GetPos(&world);
   while ( -- n >= 0 ) {
      BP_Mat_RotateVec3( to, &world, from );
      from ++ ;
      to ++ ;
   }
}

static inline void _RotTrans( FVECTOR *out, FMATRIX *world, FVECTOR *in )
{
#ifdef BP_PSX2_ASM
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	lqc2				vf7,0x30(%1)
	vmulax.xyzw			ACC, vf4,vf8
	vmadday.xyzw		ACC, vf5,vf8
	vmaddaz.xyzw		ACC, vf6,vf8
	vmaddw.xyzw			vf8, vf7,vf8
	sqc2				vf8,0x00(%0)
	": : "r"(out), "r"(world), "r"(in) );//:"memory" );
#else
	DG_SetPos( world ) ;
	BP_DG_RotVectorW(in, out, 1 ) ;
#endif
}

static inline void _RotVector( FVECTOR *out, FMATRIX *world, FVECTOR *in )
{
#ifdef BP_PSX2_ASM
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	vmulax.xyzw			ACC, vf4,vf8
	vmadday.xyzw		ACC, vf5,vf8
	vmaddz.xyzw			vf8, vf6,vf8
	sqc2				vf8,0x00(%0)
	": : "r"(out), "r"(world), "r"(in) );//:"memory" );
#else
	DG_SetPos( world ) ;
	BP_DG_RotVector(in, out, 1 ) ;
#endif
}
/*
//アルファ変えるのみ
static void ActScrPrim( DG_PRIM2 *prim, int n_verts, int alpha )
{
	int 			clock,i;
	DG_PRIM2_UVRGB	*uvrgb;

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	uvrgb = prim->uvrgb[clock];

	for( i = 0; i < n_verts; i++ ){
		uvrgb->a = alpha;
		uvrgb++;
	}
}
*/

//ライト計算付
static void ActScrPrimLight( DG_PRIM2 *prim, int n_verts, int alpha, VERTS_DATA *vdata, FMATRIX *light, FMATRIX *root )
{
	int 			clock,i;
	DG_PRIM2_UVRGB	*uvrgb;
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
		#lqc2		vf11,	0x30(%1)

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
	FVECTOR	vec1 ;
	FMATRIX	mat ;

	DG_SetPos( light ) ;
	BP_DG_PutVectorW( (FVECTOR *)root->m[0], (FVECTOR *)mat.m[0], 3 ) ;
	BP_DG_PutVectorW( &DG_ZeroVector, (FVECTOR *)mat.m[3], 1 ) ;
#endif

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	uvrgb = prim->uvrgb[clock];

	for( i = 0; i < n_verts; i++ ){
		FVECTOR		norm;
//		int			r,g,b;
		norm.vx = (float)data->nx/4096.0f;
		norm.vy = (float)data->ny/4096.0f;
		norm.vz = (float)data->nz/4096.0f;
		norm.vw = 0.0f;

#ifdef BP_PSX2_ASM
		asm ("
			lqc2			vf3,	0x00(%0)

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
			
			sqc2			vf23,0x00(%0)
		": : "r"(&norm) : "memory" );
#else
		DG_SetPos( &mat ) ;
		BP_DG_PutVectorW( &norm, &vec1, 1 ) ;
		vec1.vx = ( vec1.vx > 0.0f ) ? vec1.vx : 0.0f ;
		vec1.vy = ( vec1.vy > 0.0f ) ? vec1.vy : 0.0f ;
		vec1.vz = ( vec1.vz > 0.0f ) ? vec1.vz : 0.0f ;
		vec1.vw = ( vec1.vw > 1.0f ) ? vec1.vw : 1.0f ;

		DG_SetPos( light+1 ) ;
		BP_DG_PutVectorW( &vec1, &norm, 1 ) ;
		norm.vx = ( norm.vx < max_col.vx ) ? norm.vx : max_col.vx ;
		norm.vy = ( norm.vy < max_col.vy ) ? norm.vy : max_col.vy ;
		norm.vz = ( norm.vz < max_col.vz ) ? norm.vz : max_col.vz ;
		norm.vw = ( norm.vw < max_col.vw ) ? norm.vw : max_col.vw ;
#endif

		uvrgb->a = alpha;
		
		uvrgb->r = (int)norm.vx;
		uvrgb->g = (int)norm.vy;
		uvrgb->b = (int)norm.vz;
		
//		if( i == 0 ){
//			PRINT_PFVEC(i,&norm);
//		}
		
		data++;
		uvrgb++;
	}
}

//texスクロール縦付
static void ActScrPrimVoff( DG_PRIM2 *prim, int n_verts, int alpha, VERTS_DATA *vdata,
							float v_off, float tv_off, float tv_scl )
{
	int 			clock,i;
	DG_PRIM2_UVRGB	*uvrgb;
	VERTS_DATA 		*data = vdata;
	float			off_set = v_off/NOIZ_SIZE;
	//_sceVu0MulMatrix( &l, &light[1], &light[0]);

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	uvrgb = prim->uvrgb[clock];

	for( i = 0; i < n_verts; i++ ){
		float	v = (float)data->v/(4096.0f*SCALE_SIZE) + off_set;
		uvrgb->a = alpha;

		uvrgb->v = FTOI12( v * tv_scl + tv_off );
		data++;
		uvrgb++;
	}
}

static void ActTxtLine( DG_PRIM2 *prim, DG_TEX *tex, int n_verts )
{
	int 			clock,th,off_v,v[2],k = (irnd()>>9)&0x07;
	DG_PRIM2_UVRGB	*uvrgb0,*uvrgb1;

	
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;
	uvrgb0 = prim->uvrgb[clock];
	uvrgb1 = prim->uvrgb[1-clock];

	memcpy( &uvrgb0[4], &uvrgb1[0], (n_verts-4)*sizeof(DG_PRIM2_UVRGB) );
	
	th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
	th = 1 << th ;
	DG_GetTexelInfo( NULL, NULL, NULL, &off_v, tex );
	
	v[0] = FTOI12( ((float)(off_v + 4*k )+0.5f)/(float)th );
	v[1] = FTOI12( ((float)(off_v + 4*k + 4)-0.5f)/(float)th );

	uvrgb0[0].v = v[0];
	uvrgb0[1].v = v[0];
	uvrgb0[2].v = v[1];
	uvrgb0[3].v = v[1];
}
enum {
	MESG_ORG_FACE_INV = 0,
	MESG_ORG_FACE_NOIZ,
	MESG_ORG_FACE_FIX,
};

static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case MESG_ORG_FACE_INV:
			//垣間見えモード
			work->flags = 0;
			
			ActScrPrimLight( work->met_prim, N_MASK_VERTS, 255, MaskVerts, work->lights, &work->root );
			work->msk_step = ORGFAEC_STEP_NOIZ_FADEIN;
			work->add_v_off = 8.0f;
			work->timer = 0;
			work->diff_alpha0 = (240.0f - work->noiz_alp)/16.0f;
//printf("blur %f\n",work->diff_alpha0);
			DG_VisiblePrim2(work->met_prim);
			DG_VisiblePrim2(work->cover_prim);
			DG_VisiblePrim2(work->noiz_prim);
			
			DG_InvisibleObjs( work->objs );
			
			break;
		  case MESG_ORG_FACE_NOIZ:
			work->msk_step = ORGFAEC_STEP_FADEIN;
			work->fadein_time = msg->message[1];
			work->fadeout_time = msg->message[2];
			work->to_alpha = (float)msg->message[3];
			
			work->diff_alpha0 = (work->to_alpha - work->noiz_alp)/(float)(work->fadein_time<<1);
			work->diff_alpha1 = -work->to_alpha/(float)(work->fadeout_time<<1);
			work->fadein_time *= 10;
			work->fadeout_time *= 10;
			ActScrPrimLight( work->met_prim, N_MASK_VERTS, 255, MaskVerts, work->lights, &work->root );
//printf( "blur %d %d %d %f %f\n",msg->message[1],msg->message[2],msg->message[3],work->diff_alpha0,work->diff_alpha1);
			DG_VisiblePrim2(work->met_prim);
			DG_VisiblePrim2(work->cover_prim);
			DG_VisiblePrim2(work->noiz_prim);
			
			DG_InvisibleObjs( work->objs );
			
			break;
		  case MESG_ORG_FACE_FIX:
			DG_VisiblePrim2(work->met_prim);
			DG_VisiblePrim2(work->cover_prim);
			DG_VisiblePrim2(work->noiz_prim);
			DG_InvisibleObjs( work->objs );
			work->msk_step = ORGFAEC_STEP_FADE_FIX;
			break;
		  default:
			printf("orn_face.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}

	return 0;
}
//760～825
static void Act(Work *work)
{
#ifdef TS_DEBUG
	DM_FrameSkip = 0;
#endif
	//work->wave_time += 16;

	DG_COPY_MAT( &work->root, &work->object->objs->objs[12].world );
	_sceVu0ScaleVector( work->root.m[0], work->root.m[0], 1.002f );
	_sceVu0ScaleVector( work->root.m[1], work->root.m[1], 1.004f );
	_sceVu0ScaleVector( work->root.m[2], work->root.m[2], 0.999f );

	DG_GetLightMatrix( work->mov, work->lights );
	//DG_GetLightMatrix( work->root.m[3], work->lights );
#if 1
	CheckMesgParam( work );
#endif
	switch(work->msk_step){
	  case ORGFAEC_STEP_INIT:
		//ウェイトなんかでつかいませう
#ifdef TS_DEBUG
		if( GV_PadData[0].press & PAD_A ){
			ActScrPrimLight( work->met_prim, N_MASK_VERTS, 255, MaskVerts, work->lights, &work->root );
			work->msk_step++;
			work->add_v_off = 8.0f;
			DG_VisiblePrim2(work->met_prim);
			DG_VisiblePrim2(work->cover_prim);
			DG_VisiblePrim2(work->noiz_prim);
			
			DG_InvisibleObjs( work->objs );
		}
#else
		//CheckMesgParam( work );
#endif
		break;
	  case ORGFAEC_STEP_FADEIN:		//ノイズ見え
		work->v_offset += work->add_v_off * (float)(DM_FrameSkip+1);
		if( work->v_offset > VOFF_MAX ) work->v_offset -= VOFF_MAX;
		else if( work->v_offset < VOFF_MIN ) work->v_offset += VOFF_MAX;
		
		work->timer += CLOCK_COUNT * (DM_FrameSkip+1);
		work->noiz_alp += work->diff_alpha0 * (float)(DM_FrameSkip+1);

		if( work->timer >= work->fadein_time ){
			work->msk_step++;
			work->noiz_alp = work->to_alpha;
			work->timer = 0;
		}
		ActScrPrimLight( work->met_prim, N_MASK_VERTS, 255, MaskVerts, work->lights, &work->root );
		ActScrPrimVoff( work->noiz_prim, N_MASK_VERTS, work->noiz_alp, MaskVerts,
						work->v_offset, work->tex_v_off, work->tex_v_scl );
		//CheckMesgParam( work );
		break;
	  case ORGFAEC_STEP_FADEOUT:		//ノイズ消え
		work->v_offset += work->add_v_off * (float)(DM_FrameSkip+1);
		if( work->v_offset > VOFF_MAX ) work->v_offset -= VOFF_MAX;
		else if( work->v_offset < VOFF_MIN ) work->v_offset += VOFF_MAX;
		
		work->timer += CLOCK_COUNT * (DM_FrameSkip+1);
		work->noiz_alp += work->diff_alpha1 * (float)(DM_FrameSkip+1);
		if( work->noiz_alp < 0.0f ) work->noiz_alp = 0.0f;
		if( work->timer >= work->fadeout_time ){
			work->msk_step = 0;
			work->noiz_alp = 0.0f;
			work->timer = 0;
			
			DG_InvisiblePrim2(work->met_prim);
			DG_InvisiblePrim2(work->cover_prim);
			DG_InvisiblePrim2(work->noiz_prim);
			DG_VisibleObjs( work->objs );
		}
		ActScrPrimLight( work->met_prim, N_MASK_VERTS, 255, MaskVerts, work->lights, &work->root );
		ActScrPrimVoff( work->noiz_prim, N_MASK_VERTS, work->noiz_alp, MaskVerts,
						work->v_offset, work->tex_v_off, work->tex_v_scl );

		CheckMesgParam( work );
		break;
	  case ORGFAEC_STEP_NOIZ_FADEIN:		//ノイズ見え
//		if( --work->wave_time < 0 ){
//			work->wave_time = irnd()%128 + 64;
//			work->add_v_off = 0.5f + frnd()*2.0f;
//		}
		work->v_offset += work->add_v_off * (float)(DM_FrameSkip+1);
		if( work->v_offset > VOFF_MAX ) work->v_offset -= VOFF_MAX;
		else if( work->v_offset < VOFF_MIN ) work->v_offset += VOFF_MAX;

		work->timer += CLOCK_COUNT * (DM_FrameSkip+1);
		work->noiz_alp += work->diff_alpha0 * (float)(DM_FrameSkip+1);
		if( work->noiz_alp > 240.0f ) work->noiz_alp = 240.0f;

		if( work->timer >= 16*5 ){
			work->msk_step++;
			work->noiz_alp = 240.0f;
			work->cover_alp = 0.0f;
			work->timer = 0;
		}

		ActScrPrimLight( work->met_prim, N_MASK_VERTS, 255, MaskVerts, work->lights, &work->root );
		ActScrPrimVoff( work->noiz_prim, N_MASK_VERTS, work->noiz_alp, MaskVerts,
						work->v_offset, work->tex_v_off, work->tex_v_scl );
		ActScrPrimLight( work->cover_prim, N_MASK_VERTS, work->cover_alp, MaskVerts, work->lights, &work->root );
		break;
	  case ORGFAEC_STEP_MET_FADEOUT:		//ここから中身が見える（この後分岐：元に戻るか、ノイズも消えるか)
		//if( --work->wave_time < 0 ){
		//	work->wave_time = irnd()%128 + 64;
		//	work->add_v_off = 0.5f + frnd()*2.0f;
		//}

		work->v_offset += work->add_v_off * (float)(DM_FrameSkip+1);
		if( work->v_offset > VOFF_MAX ) work->v_offset -= VOFF_MAX;
		else if( work->v_offset < VOFF_MIN ) work->v_offset += VOFF_MAX;

		work->timer += CLOCK_COUNT * (DM_FrameSkip+1);
		work->met_alp -= 64.0f/8.0f * (float)(DM_FrameSkip+1);
		work->cover_alp += 96.0f/8.0f * (float)(DM_FrameSkip+1);

		if( work->met_alp < 64.0f ) work->met_alp = 64.0f;
		if( work->cover_alp > 96.0f ) work->cover_alp = 96.0f;
		
		if( work->timer < 5*5 ){
			work->noiz_alp -= 80.0f/5.0f * (float)(DM_FrameSkip+1);
			work->add_v_off -= 4.0f/5.0f * (float)(DM_FrameSkip+1);

			if( work->noiz_alp < 160.0f ) work->noiz_alp = 160.0f;
			if( work->add_v_off < 4.0f ) work->add_v_off = 4.0f;
		}else{
			work->noiz_alp = 160.0f;
			work->add_v_off = 4.0f;
		}
		
		if( work->timer >= 8*5 ){
			work->msk_step++;
			work->met_alp = 255.0f - 64.0f;
			work->cover_alp = 96.0f;
			work->timer = 0;
		}

		ActScrPrimVoff( work->noiz_prim, N_MASK_VERTS, work->noiz_alp, MaskVerts,
						work->v_offset, work->tex_v_off, work->tex_v_scl );
		ActScrPrimLight( work->met_prim, N_MASK_VERTS, work->met_alp, MaskVerts, work->lights, &work->root );
		ActScrPrimLight( work->cover_prim, N_MASK_VERTS, work->cover_alp, MaskVerts, work->lights, &work->root );
		//ActScrPrim( work->cover_prim, N_MASK_VERTS, work->cover_alp );
		break;
	  case ORGFAEC_STEP_MET_FADEIN:			//中身消えーの

		work->v_offset += work->add_v_off * (float)(DM_FrameSkip+1);
		if( work->v_offset > VOFF_MAX ) work->v_offset -= VOFF_MAX;
		else if( work->v_offset < VOFF_MIN ) work->v_offset += VOFF_MAX;
		
		work->timer += CLOCK_COUNT * (DM_FrameSkip+1);
		work->met_alp += 64.0f/8.0f * (float)(DM_FrameSkip+1);
		work->cover_alp -= 96.0f/8.0f * (float)(DM_FrameSkip+1);
		
		work->noiz_alp += 80.0f/8.0f * (float)(DM_FrameSkip+1);
		
		if( work->met_alp > 255.0f ) work->met_alp = 255.0f;
		if( work->cover_alp < 0.0f ) work->cover_alp = 0.0f;
		if( work->noiz_alp > 240.0f ) work->noiz_alp = 240.0f;

		if( work->timer >= 8*5 ){
			work->msk_step++;
			work->met_alp = 255.0f;
			work->cover_alp = 0.0f;
			work->noiz_alp = 240.0f;
			work->timer = 0;
		}
//printf(" ma %f: ca %f: na %f\n",work->met_alp,work->cover_alp,work->noiz_alp);
		ActScrPrimVoff( work->noiz_prim, N_MASK_VERTS, work->noiz_alp, MaskVerts,
						work->v_offset, work->tex_v_off, work->tex_v_scl );
		ActScrPrimLight( work->met_prim, N_MASK_VERTS, work->met_alp, MaskVerts, work->lights, &work->root );
		ActScrPrimLight( work->cover_prim, N_MASK_VERTS, work->cover_alp, MaskVerts, work->lights, &work->root );
		//ActScrPrim( work->cover_prim, N_MASK_VERTS, work->cover_alp );
		break;

	  case ORGFAEC_STEP_NOIZ_FADEOUT:		//ノイズ消えーの

		work->v_offset += work->add_v_off * (float)(DM_FrameSkip+1);
		if( work->v_offset > VOFF_MAX ) work->v_offset -= VOFF_MAX;
		else if( work->v_offset < VOFF_MIN ) work->v_offset += VOFF_MAX;
		
		work->timer += CLOCK_COUNT * (DM_FrameSkip+1);
		work->noiz_alp -= 240.0f/16.0f * (float)(DM_FrameSkip+1);
		//work->add_v_off -= 4.0f/16.0f * (float)(DM_FrameSkip+1);
		
		if( work->noiz_alp < 0.0f ) work->noiz_alp = 0.0f;
		if( work->add_v_off < 0.0f ) work->add_v_off = 0.0f;
		
		if( work->timer >= 16*5 ){
#ifdef TS_DEBUG
			work->msk_step = 0;
#else
			work->msk_step = 0;
#endif
			work->noiz_alp = 0.0f;
			work->timer = 0;
			DG_InvisiblePrim2(work->met_prim);
			DG_InvisiblePrim2(work->cover_prim);
			DG_InvisiblePrim2(work->noiz_prim);
			DG_VisibleObjs( work->objs );
			//work->add_v_off = 0.0f;
		}
		ActScrPrimLight( work->cover_prim, N_MASK_VERTS, work->cover_alp, MaskVerts, work->lights, &work->root );
		ActScrPrimLight( work->met_prim, N_MASK_VERTS, 255, MaskVerts, work->lights, &work->root );
		ActScrPrimVoff( work->noiz_prim, N_MASK_VERTS, work->noiz_alp, MaskVerts,
						work->v_offset, work->tex_v_off, work->tex_v_scl );
		break;
	  case ORGFAEC_STEP_FADE_FIX:		
		ActScrPrimVoff( work->noiz_prim, N_MASK_VERTS, 0.0f, MaskVerts,
						work->v_offset, work->tex_v_off, work->tex_v_scl );
		ActScrPrimLight( work->met_prim, N_MASK_VERTS, 255.0f - 128.0f, MaskVerts, work->lights, &work->root );
		ActScrPrimLight( work->cover_prim, N_MASK_VERTS, 96.0f, MaskVerts, work->lights, &work->root );
		break;
	  default:
		work->timer += CLOCK_COUNT;
		if( work->timer > 60*5 ){
			work->msk_step = 0;
			work->met_alp = 255.0f;
			work->timer = 0;
		}
		
		break;
	}
	if( (GV_Time % 4)==0 )ActTxtLine( work->txt_r_prim, work->txt_r_tex, 32 );
	if( (GV_Time % 6)==0 )ActTxtLine( work->txt_l_prim, work->txt_l_tex, 32 );
//printf("blur  ma %f: ca %f: na %f\n",work->met_alp,work->cover_alp,work->noiz_alp);
}

static void Die(Work *work)
{
	if( work->met_prim ) GM_FreePrim2( work->met_prim );
	if( work->cover_prim ) GM_FreePrim2( work->cover_prim );
	if( work->noiz_prim ) GM_FreePrim2( work->noiz_prim );

	if( work->txt_r_prim ) GM_FreePrim2( work->txt_r_prim );
	if( work->txt_l_prim ) GM_FreePrim2( work->txt_l_prim );

	if( work->mitsu_prim ) GM_FreePrim2( work->mitsu_prim );
	if( work->square_prim ) GM_FreePrim2( work->square_prim );

	if( work->objs ){
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}
}

#ifdef TS_DEBUG
static CONTROL* SearchControl( int name, int map )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		//if ( ( control->map & map ) == 0 ) continue ;
		return ( control ) ;
	}
	printf("org_face.c: search faild control !!\n");
	return ( NULL );
}
#endif

static DG_PRIM2 *InitMaskPrim( int n_prims, int n_verts, int tex_code, VERTS_DATA *vdata, int color, int mode,
							   int flag, FMATRIX *root )
{
	DG_PRIM2		*prim;	
	DG_TEX 			*tex;
	int				i;
	VERTS_DATA 		*data = vdata;
	FVECTOR			*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB	*uvrgb = MEM_SCR_UV;

//	int				max = 0x80000000;
//	int				min = 0x7f000000;
	
	tex = DG_GetTexture(tex_code);
	if(!tex) { return(NULL); }
	prim = GM_MakePrim2( mode, n_prims, n_verts );
	if(!prim){ printf("ERR!! MAKE PRIM!!orn_face.c\n"); return (NULL); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	DG_ConfigPrim2Tex( prim, tex );

	prim->root = root;
	for( i = 0; i < n_verts*n_prims; i++ ){
		pos->vx = (float)data->vx;
		pos->vy = (float)data->vy;
		pos->vz = (float)data->vz;
		pos->vw = 1.0f;

		uvrgb->r = GET_COL_R(color);
		uvrgb->g = GET_COL_G(color);
		uvrgb->b = GET_COL_B(color);
		uvrgb->a = GET_COL_A(color);
		if(flag)
			uvrgb->f = data->flags;
		else
			uvrgb->f = data->flags|0x0fff;

		uvrgb->q = 4096;
		uvrgb->u = FTOI12((float)data->u/4096.0f * tex->u_scale + tex->u_offset);
		uvrgb->v = FTOI12((float)data->v/4096.0f * tex->v_scale + tex->v_offset);

		//PRINT_PFVEC(i,pos);
		uvrgb++; pos++; data++;
	}

	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_verts*n_prims );

	//printf("max %d: min %d\n",max,min);
	return prim;
}

#define PRIM_FLAG (DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_CCW)
static DG_PRIM2 *InitTxtPrim( int tex_code, int n_prims, int n_verts, int n_poly, FMATRIX *root, FVECTOR *init_pos )
{
	DG_PRIM2		*prim;	
	DG_TEX 			*tex;
	int				i,j,off_v,th;
	FVECTOR			*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB	*uvrgb = MEM_SCR_UV;
	FVECTOR			fvtemp[4];
	FVECTOR			add[2];
	int				u[2],v[2];
	
	tex = DG_GetTexture(tex_code);
	if(!tex) { return(NULL); }
	prim = GM_MakePrim2( PRIM_FLAG, n_prims, n_verts );
	if(!prim){ return (NULL); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	DG_ConfigPrim2Tex( prim, tex );
	prim->root = root;
	
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	
	DG_COPY_VEC( &fvtemp[0], &init_pos[0] );
	DG_COPY_VEC( &fvtemp[1], &init_pos[1] );
	_sceVu0SubVector( &add[0], &init_pos[2], &init_pos[0] );
	_sceVu0SubVector( &add[1], &init_pos[3], &init_pos[1] );
	_sceVu0ScaleVector( &add[0], &add[0], (1.0f/8.0f) );
	_sceVu0ScaleVector( &add[1], &add[1], (1.0f/8.0f) );
	_sceVu0AddVector( &fvtemp[2], &init_pos[0], &add[0] );
	_sceVu0AddVector( &fvtemp[3], &init_pos[1], &add[1] );

	th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
	th = 1 << th ;
	
	DG_GetTexelInfo( NULL, NULL, NULL, &off_v, tex );
	
	u[0] = FTOI12(0.0f * tex->u_scale + tex->u_offset );
	u[1] = FTOI12(1.0f * tex->u_scale + tex->u_offset );
	
	for( i = 0; i < n_poly; i++ ){
		v[0] = FTOI12( ((float)(off_v + 4*i )+0.5f)/(float)th );
		v[1] = FTOI12( ((float)(off_v + 4*i + 4)-0.5f)/(float)th );

		//v[0] = FTOI12(0.0f * tex->v_scale + tex->v_offset );
		//v[1] = FTOI12(1.0f * tex->v_scale + tex->v_offset );
		
		for( j = 0; j < 4; j++ ){
			DG_COPY_VEC( pos, &fvtemp[j] );
			pos->vw = 1.0f;

			uvrgb->r = 128;
			uvrgb->g = 128;
			uvrgb->b = 128;
			uvrgb->a = 64;
			uvrgb->f = (j<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
			uvrgb->q = 4096;

			uvrgb->u = u[j&1];
			uvrgb->v = v[j>>1];
			//NewGeneralSprite2( 0, 6715088, root, pos, 0.0f,
			//				   0x80808080, 1, 0x7fffffff, 0x10000, 100, 0 );
			
			pos++;
			uvrgb++;
			
		}
		
		DG_COPY_VEC( &fvtemp[0], &fvtemp[2] );
		DG_COPY_VEC( &fvtemp[1], &fvtemp[3] );
		_sceVu0AddVector( &fvtemp[2], &fvtemp[2], &add[0] );
		_sceVu0AddVector( &fvtemp[3], &fvtemp[3], &add[1] );
	}

	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_verts*n_prims );
	
	return prim;
}
static DG_PRIM2 *InitPolyPrim( int tex_code, int n_prims, int n_verts, int n_poly, FMATRIX *root, FVECTOR *init_pos )
{
	DG_PRIM2		*prim;	
	DG_TEX 			*tex;
	int				i,j;
	FVECTOR			*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB	*uvrgb = MEM_SCR_UV;
	int				u[2],v[2];
	
	tex = DG_GetTexture(tex_code);
	if(!tex) { return(NULL); }
	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX,
						 n_prims, n_verts );
	if(!prim){ return (NULL); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	DG_ConfigPrim2Tex( prim, tex );
	prim->root = root;
	
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	u[0] = FTOI12(0.0f * tex->u_scale + tex->u_offset );
	u[1] = FTOI12(1.0f * tex->u_scale + tex->u_offset );
	v[0] = FTOI12(0.0f * tex->v_scale + tex->v_offset );
	v[1] = FTOI12(1.0f * tex->v_scale + tex->v_offset );
	
	for( i = 0; i < n_poly; i++ ){
		for( j = 0; j < 4; j++ ){
			DG_COPY_VEC( pos, &init_pos[j] );
			pos->vw = 1.0f;

			uvrgb->r = 128;
			uvrgb->g = 128;
			uvrgb->b = 128;
			uvrgb->a = 128;
			uvrgb->f = (j<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
			uvrgb->q = 4096;

			uvrgb->u = u[j&1];
			uvrgb->v = v[j>>1];
			pos++;
			uvrgb++;
			
		}
	}

	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_verts*n_prims );
	
	return prim;
}


static int InitMask( Work *work )
{
	DG_PRIM2	*prim;
	//ノイズ
	prim = work->noiz_prim = InitMaskPrim( 2,41,
										   GV_StrCode("orn_sld_noiz_alp"),
										   MaskVerts,
										   SET_COL(100,150,140,0),
										   DG_PRIM2_CULLPOLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX,1,
										   &work->root );
	if(!prim) return -1;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise = 2000;

	//不透明メット
	prim = work->met_prim = InitMaskPrim( 2,41,
										  GV_StrCode("orn_head_met_alp"),
										  MaskVerts,
										  SET_COL(128,128,128,0),
										  DG_PRIM2_CULLPOLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_FOG,1,
										  &work->root );
	if(!prim) return -1;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	//prim->raise = 10000;

	//半透明メット
	prim = work->cover_prim = InitMaskPrim( 2,41,
											GV_StrCode("orn_sld_cover_alp"),
											MaskVerts,
											SET_COL(0,0,0,0),
											DG_PRIM2_CULLPOLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX,1,
											&work->root );
	if(!prim) return -1;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	prim->raise = 1000;

	{
		DG_TEX	*tex;
		tex = DG_GetTexture(GV_StrCode("orn_sld_noiz_alp"));
		if(!tex) { return -1; }
		work->tex_v_off = tex->v_offset;
		work->tex_v_scl = tex->v_scale;
	}
	

	
	return 0;
}

static DG_OBJS *InitItemObjs( int code, FMATRIX *root, FMATRIX *light )
{
	DG_DEF		*def;
	DG_OBJS		*objs;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( code, 'k' ) ) ;
	if(!def){ printf("ERR!! NO MODEL!!\n"); return NULL; }
	objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE, 0 );
	if(!objs) return NULL;
	if(DG_QueueObjs( objs )<0) return NULL;
	if(root) objs->root = root;
	
	if(light) DG_SetLightMatrix( objs, light );
	return objs;
}


static int GetResources( Work *work )
{
#ifdef TS_DEBUG
	CONTROL	*control;
	
	control = SearchControl( GV_StrCode("オルガ"), GM_CurrentStageMap );
	if(!control) return -1;
	work->mov = &control->mov;
	work->object = control->object;
#endif
	if( !work->object ) return -1;
	if( InitMask( work ) ) return -1;

	DG_InvisiblePrim2(work->met_prim);
	DG_InvisiblePrim2(work->cover_prim);
	DG_InvisiblePrim2(work->noiz_prim);

	work->met_alp = 255.0f;
	work->cover_alp = 0.0f;
	work->noiz_alp = 0.0f;
	work->add_v_off = 8.0f;

	//work->msk_step = ORGFAEC_STEP_FADE_FIX;
	
	work->txt_r_prim = InitTxtPrim( GV_StrCode("orn_sld_text_R_alp"), 1, 32,
									8, &work->object->objs->objs[12].world, OrgEftTxt_R_Pos );
	if(!work->txt_r_prim) return -1;
	work->txt_r_tex = DG_GetTexture(GV_StrCode("orn_sld_text_R_alp"));
	work->txt_r_prim->raise = -1000;
	
	work->txt_l_prim = InitTxtPrim( GV_StrCode("orn_sld_text_L_alp"), 1, 32,
									8, &work->object->objs->objs[12].world, OrgEftTxt_L_Pos );
	if(!work->txt_l_prim) return -1;
	work->txt_l_tex = DG_GetTexture(GV_StrCode("orn_sld_text_L_alp"));
	work->txt_l_prim->raise = -1000;

	work->mitsu_prim = InitPolyPrim( GV_StrCode("orn_sld_mitsubishi_alp"), 1, 4,
									1, &work->object->objs->objs[12].world, OrgEftMitsu_Pos );
	if(!work->mitsu_prim) return -1;
	work->mitsu_prim->raise = -1000;

	
	work->square_prim = InitPolyPrim( GV_StrCode("orn_sld_square_alp"), 1, 4,
									1, &work->object->objs->objs[12].world, OrgEftSquare_Pos );
	if(!work->square_prim) return -1;
	work->square_prim->raise = -1000;

	work->objs = InitItemObjs( GV_StrCode("org_tng_sld_mt"), &work->root, work->lights );
	if( !work->objs ) return -1;
	DG_GetLightMatrix( work->mov, work->lights );
	return 0;
}

void *NewOrgFaceEft( int name, CONTROL *ctrl, int mode )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor( &(work->actor), Act, Die );
		GV_ActorEX( &work->actor );		
		work->name = name;
		work->object = ctrl->object;
		work->mov = &ctrl->mov;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work) ;
			return NULL;
		}
	}

	return (void *)work ;
}

void *NewOrnFaceEft_debug( int name, int map )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor( &(work->actor), Act, Die );
		GV_ActorEX( &work->actor );
		
		work->name = name;
		//work->object = object;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work) ;
			return NULL;
		}
		//printf("オルガマスクですので\n");
	}

	return (void *)work ;
}


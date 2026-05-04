//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    ray_waterpollute.c
    レイ汚水
	2001/05/22 T.Shibata
	
	$Id: ray_laser.c,v 1.1.1.3 2002/11/19 11:48:39 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
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

#include "../../mode/demo/eft_con.h"
#include	"../util/ts_util.h"

//#define		TS_DEBUG

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

#define		MEM_SCR_POS		((void*)(SCRPAD_ADDR))
#define		MEM_SCR_UV		((void*)(SCRPAD_ADDR+sizeof(FVECTOR)*320))

#define		GET_COL_R(_rgba)	(((_rgba)>>24)&0xff)
#define		GET_COL_G(_rgba)	(((_rgba)>>16)&0xff)
#define		GET_COL_B(_rgba)	(((_rgba)>> 8)&0xff)
#define		GET_COL_A(_rgba)	(((_rgba)>> 0)&0xff)
#define		SET_COL(_r,_g,_b,_a)	(((_r)<<24)|((_g)<<16)|((_b)<<8)|(_a))

#define TEX_CODE2		(GV_StrCode("drop01_msk"))
#define TEX_CODE		(GV_StrCode("drop01_msk"))
//#define TEX_CODE		(GV_StrCode("blood_1c_msk"))

extern float BP_AdjustTick5(float);
#define		FADE_TIME	(BP_AdjustTick5(32.0f))

#define		N_PRIMS	(16)
#define		N_VERTS	(16)

#define		N_PARTS	(32)
#define		N_TURNS	(8)

#define		N_PRIMS2	(16)
#define		N_VERTS2	(12)

#define		N_PARTS2	(32)
#define		N_TURNS2	(6)

#define		N_PRIMS4	(4)
#define		N_VERTS4	(16)

#define		N_PARTS4	(8)
#define		N_TURNS4	(8)

#define		INIT_ALPHA	(16)
#define		INIT_ALPHA2	(32)

typedef struct {
	GV_ACT_EX	actor;
	FMATRIX		root;
	DG_PRIM2	*prim;
	DG_PRIM2	*prim2;
	DG_PRIM2	*prim3;
	DG_PRIM2	*prim4;
	
	FMATRIX		*ray_head;
	int			name;
	FVECTOR		*to_pos;
	SVECTOR		*rot;
	u_int		timer;
	FVECTOR		vec[N_PRIMS*N_VERTS];
	FVECTOR		vec2[N_PRIMS2*N_VERTS2];
	FVECTOR		vec4[N_PRIMS4*N_VERTS4];
	FVECTOR		side_pos[2];
	
} Work;

//メタルの口座標です。
//上：±200, 300, 2230
//下：±200, -250, 2230

#define INIT_DVEC	(500.0f)
#define DEBUG_LEN	(12000.0f)

#define	SIZE_OFF	(32)
#define	SIZE_SCL	(32)

#define	SIZE_OFF2	(80)
#define	SIZE_SCL2	(80)

static FVECTOR RayMouthSize = { 200.0f, 275.0f, 0.0f, 0.0f };
static FVECTOR RayMouthCenter = { 0.0f, 25.0f, 2230.0f - 1000.0f, 1.0f };
//static FVECTOR RayMouthCenterCs = { 0.0f, 25.0f, 2230.0f, 1.0f };
//static FVECTOR RayMouthTop = { 0.0f, 25.0f, 2230.0f + DEBUG_LEN, 1.0f };


/*
・world はずっと参照する！！他は直値
・world の X方向を軸として intense の強さの水飛沫を 出す。
・angle は 球座標系の角度で、0.0f でＸ方向のみ、1.0f で最大開放
*/
extern void *NewCircleSplush( FMATRIX *world, float intense, float angle, int num, int life );



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
	": : "r"(out), "r"(world), "r"(in) :"memory" );
#else
	DG_SetPos( world ) ;
	DG_RotVectorW(in, out, 1 ) ;
#endif

}


static inline void _MakeNormMatrix( FMATRIX *world, FVECTOR *from, FVECTOR *to, FVECTOR *pos )
{
#ifdef BP_PSX2_ASM
	//vf4->vf7にマトリックスはいります
	asm volatile ("
		lqc2		vf07, 0x00(%1)	#from（mat[3]）
		lqc2		vf08, 0x00(%2)	#to
		lqc2		vf09, 0x00(%3)	#pos

		vsub.xyzw	vf10, vf08,	vf07	#to  - from
		vsub.xyzw	vf11, vf09,	vf07	#pos - from
		
		vmul.xyzw	vf16, vf10, vf10	#正規化
		vmulax.w	ACC,  vf00, vf16x
		vmadday.w	ACC,  vf00, vf16y
		vmaddz.w	vf02,  vf00, vf16z
		vrsqrt		Q, vf00w, vf02w
		vwaitq
		vmulq.xyz	vf04, vf10, Q			#正規化終了（X軸）

		vmul.xyzw	vf16, vf04, vf11		#vf04とvf11の内積
		vmulax.w	ACC,  vf00, vf16x
		vmadday.w	ACC,  vf00, vf16y
		vmaddz.w	vf17, vf00, vf16z

		vmulw.xyzw	vf18, vf04, vf17w		#内積で倍して最寄のvec上の点を求める
		vadd.xyzw	vf19, vf18, vf07		#
		vsub.xyzw	vf20, vf19, vf09		#↑-pos

		vmul.xyzw	vf16, vf20, vf20		#正規化
		vmulax.w	ACC,  vf00, vf16x
		vmadday.w	ACC,  vf00, vf16y
		vmaddz.w	vf02,  vf00, vf16z
		vrsqrt		Q, vf00w, vf02w
		vwaitq
		vmulq.xyz	vf06, vf20, Q			#正規化終了（Z軸）

		vopmula.xyz	ACC,  vf04, vf06		#外積でY軸を求める
		vopmsub.xyz	vf05, vf06, vf04
		vmulx.w		vf05, vf00, vf00
		
		sqc2		vf4,0x00(%0)
		sqc2		vf5,0x10(%0)
		sqc2		vf6,0x20(%0)
		sqc2		vf7,0x30(%0)
		
	": : "r"(world), "r"(from), "r"(to), "r"(pos) : "memory" );

//	PRINT_PFVEC(0,(FVECTOR*)world->m[0]);
//	PRINT_PFVEC(1,(FVECTOR*)world->m[1]);
//	PRINT_PFVEC(2,(FVECTOR*)world->m[2]);
//	PRINT_PFVEC(3,(FVECTOR*)world->m[3]);
//	printf("\n");
#else
	FVECTOR		vtmp;
	float		len;

	_sceVu0SubVector( (FVECTOR*)world->m[0], to, from );
	_sceVu0SubVector( &vtmp, pos, from );
	_sceVu0Normalize( (FVECTOR*)world->m[0], (FVECTOR*)world->m[0] );

	len = _sceVu0InnerProduct( (FVECTOR*)world->m[0], &vtmp );
	_sceVu0ScaleVector( &vtmp, (FVECTOR*)world->m[0], len );
	_sceVu0AddVector( &vtmp, &vtmp, from );
	_sceVu0SubVector( (FVECTOR*)world->m[2], &vtmp, pos );
	_sceVu0Normalize( (FVECTOR*)world->m[2], (FVECTOR*)world->m[2] );
	_sceVu0OuterProduct( (FVECTOR*)world->m[1], (FVECTOR*)world->m[0], (FVECTOR*)world->m[2] );
	DG_COPY_VEC( (FVECTOR*)world->m[3], from );
#endif
}

static void MakePrimRootMatrix( FMATRIX	*root, FVECTOR *from, FVECTOR *to )
{
	FVECTOR	fvtemp;
	_sceVu0SubVector( &fvtemp, to, from );
	TS_MakeMatrix( root, &fvtemp, from );
}


// --------------------------------------------------------------------------

static void RsprtPrimAct( DG_PRIM2 *prim, int verts, int parts, int turns,
						  FVECTOR *pvec, int timer,
						  float off_speed, float scl_speed,
						  int off_size, int scl_size, int add_size, int scl_rot, int alpha, int sub_alpha )
{
	int					i,par_off,clock;
	FVECTOR				*pos,*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh,*pre_uvrgbwh;

   DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	par_off = (timer%turns)*parts;

	TS_Mem_Scr( MEM_SCR_POS, prim->pos[1-clock], sizeof(FVECTOR), verts);


	pos = MEM_SCR_POS;
	vec = pvec;
	uvrgbwh = prim->uvrgb[clock];
	pre_uvrgbwh = prim->uvrgb[1-clock];
	//printf("memememememememem = %p\n",uvrgbwh);
	for( i = 0; i < verts; i++ ){
		//移動
		
#if 0
		if( uvrgbwh->pad1 ){
			if( uvrgbwh->a != alpha ){
				if( pre_uvrgbwh->a < sub_alpha ){
					uvrgbwh->a = 0;
				}else{
					uvrgbwh->a = (short)(pre_uvrgbwh->a - sub_alpha);
				}
			}
			uvrgbwh->pad1 = (short)(add_size + pre_uvrgbwh->pad1);

			uvrgbwh->w = (short)((float)uvrgbwh->pad1 * TS_COSs(uvrgbwh->pad0));
			uvrgbwh->h = (short)((float)uvrgbwh->pad1 * TS_SINs(uvrgbwh->pad0));
		}
#endif
			_sceVu0AddVector( pos, pos, vec );
			uvrgbwh->pad1 = (short)(add_size + pre_uvrgbwh->pad1);
			uvrgbwh->w = (short)((float)uvrgbwh->pad1 * TS_COSs(uvrgbwh->pad0));
			uvrgbwh->h = (short)((float)uvrgbwh->pad1 * TS_SINs(uvrgbwh->pad0));
			
			if( pre_uvrgbwh->a < sub_alpha ){
				uvrgbwh->a = 0; 
			}else{
				uvrgbwh->a = (short)(pre_uvrgbwh->a - sub_alpha);
			}
			
		//printf("<%d>wh[%d,%d][%d,%d]\n",i,uvrgbwh->w,uvrgbwh->h,uvrgbwh->pad1,pre_uvrgbwh->pad1);
		//printf("<%d>a[%d]\n",i,uvrgbwh->a );
		pos++;
		vec++;
		uvrgbwh++;
		pre_uvrgbwh++;
	}

	
	pos = MEM_SCR_POS;
	uvrgbwh = prim->uvrgb[clock];
	pre_uvrgbwh = prim->uvrgb[1-clock];

	pos += par_off;
	uvrgbwh += par_off;
	pre_uvrgbwh += par_off;
	vec = &pvec[par_off];
	
	for( i = 0; i < parts; i++ ){
		//pos初期化
		FVECTOR		fvtemp;
		SVECTOR		rot = { (irnd()>>16)%(scl_rot+1)-(scl_rot>>1), (irnd()>>8)%(scl_rot+1)-(scl_rot>>1), 0, 0 };
		fvtemp.vx = RayMouthSize.vx * frnd();
		fvtemp.vy = RayMouthSize.vy * frnd();
		fvtemp.vz = RayMouthSize.vz;
		fvtemp.vw = RayMouthSize.vw;

		DG_COPY_VEC( pos, &fvtemp );
		//vec初期化		
		fvtemp.vx = 0.0f;
		fvtemp.vy = 0.0f;
		fvtemp.vz = off_speed + scl_speed * rnd();
		fvtemp.vw = 0.0f;
		//DG_SetPos( work->ray_head );
		DG_SetPos( &DG_UnitMatrix );
		DG_RotatePos( &rot );
		DG_RotVector( &fvtemp, vec, 1 );

		uvrgbwh->pad0 = pre_uvrgbwh->pad0 = (irnd()>>16)%2048;
		uvrgbwh->pad1 = (pre_uvrgbwh->pad1 = off_size + (irnd()>>12)%scl_size);

		//printf("<%d>wh[%d,%d][%d,%d]\n",i,uvrgbwh->w,uvrgbwh->h,uvrgbwh->pad1,pre_uvrgbwh->pad1);
		
		uvrgbwh->w = uvrgbwh->h = 0;
		uvrgbwh->a = alpha;

		_sceVu0AddVector( pos, pos, vec );
		pos++;
		vec++;
		uvrgbwh++;
		pre_uvrgbwh++;
	}

	//printf("\n");
	TS_Scr_Mem( prim->pos[clock], MEM_SCR_POS, sizeof(FVECTOR), verts );
//	TS_Scr_Mem( prim->uvrgb[clock], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS );

}

static void PolyPrimAct( DG_PRIM2 *prim, float len )
{
	FVECTOR			*pos;
	int				clock;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	pos = prim->pos[clock];

	pos[1].vx = len;
	pos[3].vx = len;
	pos[5].vx = len;
}
static inline float _Vu0VecLenXYZ( FVECTOR *vec )
{
	float	ans;

#ifdef BP_PSX2_ASM
	asm volatile ("
    lqc2		vf4, 0x00(%1)
    vmul.xyz    vf5, vf4, vf4
    vmulax.w	ACC, vf0, vf5x
    vmadday.w	ACC, vf0, vf5y
    vmaddz.w	vf5, vf0, vf5z
	vsqrt		Q, vf5w
	vwaitq
	vaddq.x		vf6, vf0, Q
    qmfc2.i		$8,vf6
    sw			$8,0(%0)
	": : "r"(&ans), "r"(vec) : "$8", "memory" );
#else
	ans = GV_VecLen3F( vec ) ;
#endif
	return (ans);
}

static void Act(Work *work)
{
	FVECTOR			fvtemp;
	float			len;
	
   //AR_PARTICLE_HALF
   if( !AS_WillPrimBuffSwitch() )
   {
      return;
   }

	_RotTrans( &fvtemp, work->ray_head, &RayMouthCenter );
	MakePrimRootMatrix( &work->root, &fvtemp, work->to_pos );
	_MakeNormMatrix( &work->prim3->as_world, &fvtemp, work->to_pos, (FVECTOR*)DG_Chanls[0].eye.m[3] );

	_sceVu0SubVector( &fvtemp, work->to_pos, &fvtemp );
	len = _Vu0VecLenXYZ( &fvtemp );
	//HZX_ViewMatrix( &work->prim3->world, 8000.0f );
/*
	RsprtPrimAct(	DG_PRIM2 *prim, int verts, int parts, int turns,
					FVECTOR *pvec, int timer,
					float off_speed, float scl_speed,
					int off_size, int scl_size, int add_size, int scl_rot, int alpha、int sub_alpha )
*/
	PolyPrimAct( work->prim3, len );
	//周り
	RsprtPrimAct( work->prim, N_PRIMS*N_VERTS, N_PARTS, N_TURNS,
				  work->vec, work->timer,
				  400.0f, 800.0f, 150, 150, 150, 160, INIT_ALPHA, 0 );
	//中心部分
	RsprtPrimAct( work->prim2, N_PRIMS2*N_VERTS2, N_PARTS2, N_TURNS2,
				  work->vec2, work->timer,
				  500.0f, 1000.0f, 150, 1, 0, 96, 64, 64/N_TURNS2 );
	//口付近
	RsprtPrimAct( work->prim4, N_PRIMS4*N_VERTS4, N_PARTS4, N_TURNS4,
				  work->vec4, work->timer,
				  400.0f, 200.0f, 100, 1, 400, 512+256, INIT_ALPHA2, INIT_ALPHA2/N_TURNS4 );


#if 0
	{
		FVECTOR fvtemp;

		_sceVu0AddVector( &fvtemp, &RayMouthCenter, &RayMouthSize );
		_RotTrans( &fvtemp, work->ray_head, &fvtemp );
		_RotTrans( &work->side_pos[0], &DG_Chanls[DG_CHANL_MAIN].eye_pers, &fvtemp );
		_sceVu0SubVector( &fvtemp, &RayMouthCenter, &RayMouthSize );
		_RotTrans( &fvtemp, work->ray_head, &fvtemp );
		_RotTrans( &work->side_pos[1], &DG_Chanls[DG_CHANL_MAIN].eye_pers, &fvtemp );
		
	}
#endif
#if 0
	{
		FVECTOR		debug_line[2];
		FVECTOR		debug_sqr[4];
		static FVECTOR debug_init_sqr[4] = {
			{ 0.0f - 1000.0f, 25.0f + 1000.0f, 2230.0f + DEBUG_LEN, 1.0f },
			{ 0.0f + 1000.0f, 25.0f + 1000.0f, 2230.0f + DEBUG_LEN, 1.0f },
			{ 0.0f - 1000.0f, 25.0f - 1000.0f, 2230.0f + DEBUG_LEN, 1.0f },
			{ 0.0f + 1000.0f, 25.0f - 1000.0f, 2230.0f + DEBUG_LEN, 1.0f },
		};

		_RotTrans( &debug_line[0], work->ray_head, &RayMouthCenter );
		DG_COPY_VEC( &debug_line[1], work->to_pos );
		//_RotTrans( &debug_line[1], work->ray_head, &RayMouthTop );
		NewLineView( debug_line, 1, 160, 32, 140 );
		
		_RotTrans( &debug_sqr[0], work->ray_head, &debug_init_sqr[0] );
		_RotTrans( &debug_sqr[1], work->ray_head, &debug_init_sqr[1] );
		_RotTrans( &debug_sqr[2], work->ray_head, &debug_init_sqr[2] );
		_RotTrans( &debug_sqr[3], work->ray_head, &debug_init_sqr[3] );
		NewSquareView( debug_sqr, 1, 16, 16, 16 );
	}
#endif
	work->timer++;
}

static void Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim);
	if(work->prim2) GM_FreePrim2(work->prim2);
	if(work->prim3) GM_FreePrim2(work->prim3);
	if(work->prim4) GM_FreePrim2(work->prim4);
}

static DG_PRIM2 *InitRsprtPrim2( int n_prims, int n_verts, int tex_code, int col )
{
	DG_PRIM2		*prim;
	DG_TEX			*tex;
	int				i;
	int				u[2],v[2];
	FVECTOR				*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGBWH	*uvrgbwh0,*uvrgbwh1;

	tex = DG_GetTexture(tex_code);
	if(!tex) { printf("ERR!! NO TEX!! <ray_waterpollute.c>\n"); return(NULL); }
	prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
						 n_prims, n_verts);
	if(!prim){ printf("ERR!! MAKE PRIM!! <ray_waterpollute.c>\n"); return (NULL); }
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	u[0] = FTOI12(0.0f * tex->u_scale + tex->u_offset);
	v[0] = FTOI12(0.0f * tex->v_scale + tex->v_offset);
	u[1] = FTOI12(1.0f * tex->u_scale + tex->u_offset);
	v[1] = FTOI12(1.0f * tex->v_scale + tex->v_offset);

	uvrgbwh0 = prim->uvrgb[0];
	uvrgbwh1 = prim->uvrgb[1];

	for( i = 0; i < n_prims*n_verts; i++ ){
		//u_short rot,size;
		DG_COPY_VEC( pos, &DG_ZeroVector );
		uvrgbwh0->r = GET_COL_R(col);
		uvrgbwh0->g = GET_COL_G(col);
		uvrgbwh0->b = GET_COL_B(col);
		uvrgbwh0->a = GET_COL_A(col);
#if 0
		uvrgbwh0->r = 180;
		uvrgbwh0->g = 180;
		uvrgbwh0->b = 240;
		uvrgbwh0->a = 32;
#endif
		uvrgbwh0->f0 = VERT_KICK_CODE;
		uvrgbwh0->f1 = DRAW_KICK_CODE;
		
		uvrgbwh0->q0 = 4096;
		uvrgbwh0->q1 = 4096;
		
		uvrgbwh0->u0 = u[0];
		uvrgbwh0->v0 = v[0];
		uvrgbwh0->u1 = u[1];
		uvrgbwh0->v1 = v[1];

		//rot = (irnd()>>16)%2048;//PI * frnd();
		//size = (SIZE_OFF + SIZE_SCL*frnd());
		
		uvrgbwh0->pad0 = 0;//(irnd()>>16)%2048;
		uvrgbwh0->pad1 = 0;//SIZE_OFF + (irnd()>>12)%SIZE_SCL;

		uvrgbwh0->w = 0;//(int)(size * cosf( rot ) );
		uvrgbwh0->h = 0;//(int)(size * sinf( rot ) );
		uvrgbwh1[0] = uvrgbwh0[0];
		pos++;
		uvrgbwh0++;
		uvrgbwh1++;
	}

	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
//	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), n_verts*n_prims );
//	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), n_verts*n_prims );

	return prim;
}

static DG_PRIM2 *InitPolyPrim2( int n_prims, int n_verts, int tex_code, int	col )
{
	DG_PRIM2		*prim;
	DG_TEX			*tex;
	int				i,j;
	int				u[3],v[2];
	FVECTOR			*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB	*uvrgb0,*uvrgb1;

	tex = DG_GetTexture(tex_code);
	if(!tex) { printf("ERR!! NO TEX!! <ray_waterpollute.c>\n"); return(NULL); }
	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
						 n_prims, n_verts);
	if(!prim){ printf("ERR!! MAKE PRIM!! <ray_waterpollute.c>\n"); return (NULL); }
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	u[0] = FTOI12(0.0f * tex->u_scale + tex->u_offset);
	v[0] = FTOI12(0.0f * tex->v_scale + tex->v_offset);
	u[1] = FTOI12(0.5f * tex->u_scale + tex->u_offset);
	v[1] = FTOI12(0.5f * tex->v_scale + tex->v_offset);
	u[2] = FTOI12(1.0f * tex->u_scale + tex->u_offset);

	uvrgb0 = prim->uvrgb[0];
	uvrgb1 = prim->uvrgb[1];

	for( i = 0; i < n_prims; i++ ){
		for( j = 0; j < n_verts; j++ ){
			static FVECTOR init_pos[] = {
				{ 0.0f, -150.0f, 0.0f, 1.0f },
				{ 0.0f, -150.0f, 0.0f, 1.0f },
				{ 0.0f,  0.0f, 0.0f, 1.0f },
				{ 0.0f,  0.0f, 0.0f, 1.0f },
				{ 0.0f,  150.0f, 0.0f, 1.0f },
				{ 0.0f,  150.0f, 0.0f, 1.0f },
			};

//			DG_COPY_VEC( pos, &DG_ZeroVector );
			DG_COPY_VEC( pos, &init_pos[j] );
			uvrgb0->r = GET_COL_R(col);
			uvrgb0->g = GET_COL_G(col);
			uvrgb0->b = GET_COL_B(col);
			uvrgb0->a = (j/2==1)?GET_COL_A(col):0;

			uvrgb0->f = (j<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
		
			uvrgb0->q = 4096;

			uvrgb0->u = u[j/2];
			uvrgb0->v = v[j&1];

			uvrgb1[0] = uvrgb0[0];
			pos++;
			uvrgb0++;
			uvrgb1++;
		}
	}

	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
//	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), n_verts*n_prims );
//	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), n_verts*n_prims );

	return prim;
}

static int GetResources( Work *work )
{
	FVECTOR	fvtemp;

	_RotTrans( &fvtemp, work->ray_head, &RayMouthCenter );
	MakePrimRootMatrix( &work->root, &fvtemp, work->to_pos );

	work->prim = InitRsprtPrim2( N_PRIMS, N_VERTS, TEX_CODE, SET_COL(54,72,100,0) );
//	work->prim = InitRsprtPrim2( N_PRIMS, N_VERTS, TEX_CODE, SET_COL(120,120,120,8) );
	if(!work->prim) return -1;
	work->prim->root = &work->root;

	work->prim2 = InitRsprtPrim2( N_PRIMS2, N_VERTS2, TEX_CODE2, SET_COL(100,100,130,0) );
//	work->prim2 = InitRsprtPrim2( N_PRIMS2, N_VERTS2, TEX_CODE2, SET_COL(64,96,160,16) );
	if(!work->prim2) return -1;
	work->prim2->root = &work->root;

	work->prim3 = InitPolyPrim2( 1, 6, GV_StrCode("wave12_alp_ovl"), SET_COL(100,100,130,60) );
	if(!work->prim3) return -1;
   work->prim3->flag |= DG_PRIM_AS_CUSTOMWORLD;

	work->prim4 = InitRsprtPrim2( N_PRIMS4, N_VERTS4, TEX_CODE2, SET_COL(100,100,130,0) );
//	work->prim4 = InitRsprtPrim2( N_PRIMS4, N_VERTS4, TEX_CODE2, SET_COL(45,96,120,16) );
	if(!work->prim4) return -1;
	work->prim4->root = &work->root;

	_RotTrans( &fvtemp, work->ray_head, &RayMouthCenter );
	MakePrimRootMatrix( &work->root, &fvtemp, work->to_pos );
	_MakeNormMatrix( &work->prim3->as_world, &fvtemp, work->to_pos, (FVECTOR*)DG_Chanls[0].eye.m[3] );
#if 0
	{
		extern void *NewSideFlareWidth(
			FVECTOR *pos_scr0,	// 死ぬまで参照し続ける
			FVECTOR *pos_scr1,	// 死ぬまで参照し続ける
			u_char	*alpha		// 死ぬまで参照し続ける
		);
		static u_char side_alpha = 64;
		void *ray_eft1;
		ray_eft1 = NewSideFlareWidth( &work->side_pos[0], &work->side_pos[1], &side_alpha );
		GV_SetActorChild( work, ray_eft1 );
	}
#endif

	return (0);
}

void *NewWaterPollute_game( FMATRIX *root, FVECTOR *to_pos )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		work->ray_head = root;
		work->to_pos = to_pos;
		
		if(GetResources( work ) < 0){
			GV_DestroyActor(work);
			return NULL ;
		}
		printf("--------------あははははは\n");

	}

	return (void *)work;
}



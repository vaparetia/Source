//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    ray_colwater.c
    レイ汚水跳ね返り
	2001/04/16 T.Shibata

	$Id: ray_colwater.c,v 1.1.1.3 2002/11/19 11:48:33 Yoshizawa1 Exp $

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

#define TEX_CODE		(GV_StrCode("drop01_msk"))
#define TEX_CODE2		(GV_StrCode("drop01_msk"))
#define TEX_CODE3		(GV_StrCode("blood_1c_msk"))

//#define		FADE_TIME	(BP_AdjustTick3(26.6f))

#define		N_PRIMS	(4)
#define		N_VERTS	(16)

#define		N_PARTS	(8)
#define		N_TURNS	(8)

#define		N_PRIMS2	(4)
#define		N_VERTS2	(16)

#define		N_PARTS2	(8)
#define		N_TURNS2	(8)

#define		N_PRIMS3	(8)
#define		N_VERTS3	(16)

#define		N_PARTS3	(4)
#define		N_TURNS3	(32)

#define		INIT_ALPHA	(32)
#define		INIT_ALPHA2	(32)

#define GRAVITATION	(P_GRAVITY)

typedef struct {
	GV_ACT_EX	actor;
	DG_PRIM2	*prim;
	DG_PRIM2	*prim2;
	DG_PRIM2	*prim3;
	
	FMATRIX		*ray_col;
	u_int		timer;
	FVECTOR		vec[N_PRIMS*N_VERTS];
	FVECTOR		vec2[N_PRIMS2*N_VERTS2];
	FVECTOR		vec3[N_PRIMS3*N_VERTS3];
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

static FVECTOR RayColWaterSize = { 500.0f, 500.0f, 0.0f, 0.0f };

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


// --------------------------------------------------------------------------

static void RsprtPrimAct( DG_PRIM2 *prim, FMATRIX *root, int verts, int parts, int turns,
						  FVECTOR *pvec, int timer,
						  float off_speed, float scl_speed,
						  int off_size, int scl_size, int add_size, int scl_rot, int alpha, int sub_alpha,
						  int mode )
{
	int					i,par_off,clock;
	FVECTOR				*pos,*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh,*pre_uvrgbwh;

   DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	par_off = (timer%turns)*parts;
	
	TS_Mem_Scr( MEM_SCR_POS, prim->pos[1-clock], sizeof(FVECTOR), verts);
	
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
#if 0
		SVECTOR		rot = { (irnd()>>16)%(scl_rot+1)-(scl_rot>>1), (irnd()>>8)%(scl_rot+1)-(scl_rot>>1), 0, 0 };
#else
		float a = off_speed + scl_speed * rnd();
		short	rot_z = (irnd()>>8)%4096, rot_x = (irnd()>>16)%128;
#endif
		fvtemp.vx = RayColWaterSize.vx * frnd();
		fvtemp.vy = RayColWaterSize.vy * frnd();
		fvtemp.vz = RayColWaterSize.vz;
		fvtemp.vw = 1.0f;
		//DG_COPY_VEC( pos, &fvtemp );
		_RotTrans( pos, root, &fvtemp );
		//vec初期化
#if 0
		fvtemp.vx = 0.0f;
		fvtemp.vy = off_speed + scl_speed * rnd();
		fvtemp.vz = 0.0f;
		fvtemp.vw = 0.0f;
		DG_SetPos( root );
		//DG_SetPos( &DG_UnitMatrix );
		DG_RotatePos( &rot );
		DG_RotVector( &fvtemp, vec, 1 );
#else
		fvtemp.vx = a * TS_COSs(rot_x) * TS_SINs(rot_z);
		fvtemp.vy = a * TS_SINs(rot_x);
		fvtemp.vz = a * TS_COSs(rot_x) * TS_COSs(rot_z);
		fvtemp.vw = 0.0f;
		_RotTrans( vec, root, &fvtemp );
#endif
		uvrgbwh->pad0 = pre_uvrgbwh->pad0 = (irnd()>>16)%2048;
		uvrgbwh->pad1 = pre_uvrgbwh->pad1 = off_size + (irnd()>>12)%scl_size;

		uvrgbwh->w = uvrgbwh->h = 0;
		uvrgbwh->a = alpha;
		pos++;
		vec++;
		uvrgbwh++;
	}

	pos = MEM_SCR_POS;
	vec = pvec;
	uvrgbwh = prim->uvrgb[clock];
	pre_uvrgbwh = prim->uvrgb[1-clock];

	for( i = 0; i < verts; i++ ){
		//移動
		_sceVu0AddVector( pos, pos, vec );
		if(uvrgbwh->w){
			uvrgbwh->pad1 = add_size + pre_uvrgbwh->pad1;
			uvrgbwh->a = pre_uvrgbwh->a - sub_alpha;
		} else {
			uvrgbwh->pad1 = add_size + uvrgbwh->pad1;
		}

		uvrgbwh->w = (int)((float)uvrgbwh->pad1 * TS_COSs(uvrgbwh->pad0));
		uvrgbwh->h = (int)((float)uvrgbwh->pad1 * TS_SINs(uvrgbwh->pad0));

		pos++;
		vec++;
		uvrgbwh++;
		pre_uvrgbwh++;
	}

	TS_Scr_Mem( prim->pos[clock], MEM_SCR_POS, sizeof(FVECTOR), verts );
//	TS_Scr_Mem( prim->uvrgb[clock], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS );

}

static void Act(Work *work)
{
   //AR_PARTICLE_HALF
   if( !AS_WillPrimBuffSwitch() )
   {
      return;
   }
//	FVECTOR			fvtemp;

/*
	RsprtPrimAct(	DG_PRIM2 *prim, int verts, int parts, int turns,
					FVECTOR *pvec, int timer,
					float off_speed, float scl_speed,
					int off_size, int scl_size, int add_size, int scl_rot, int alpha、int sub_alpha )
*/
	//HZX_ViewMatrix( work->ray_col, 1000.0f );
	//周り
	/*
	RsprtPrimAct( work->prim, work->ray_col, N_PRIMS*N_VERTS, N_PARTS, N_TURNS,
				  work->vec, work->timer,
				  400.0f, 200.0f,
				  150, 150, 150,
				  1024-128, INIT_ALPHA2, INIT_ALPHA2/N_TURNS, 0 );
	//中心部分
	RsprtPrimAct( work->prim2, work->ray_col, N_PRIMS2*N_VERTS2, N_PARTS2, N_TURNS2,
				  work->vec2, work->timer,
				  400.0f, 200.0f,
				  150, 150, 150,
				  1024-64, INIT_ALPHA2, INIT_ALPHA2/N_TURNS2, 0 );
	*/
	//口付近
	RsprtPrimAct( work->prim3, work->ray_col, N_PRIMS3*N_VERTS3, N_PARTS3, N_TURNS3,
				  work->vec3, work->timer,
				  50.0f, 50.0f,
				  50, 50, 50,
				  1024-64, INIT_ALPHA2, INIT_ALPHA2/N_TURNS3, 1 );

	work->timer++;
}

static void Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim);
	if(work->prim2) GM_FreePrim2(work->prim2);
	if(work->prim3) GM_FreePrim2(work->prim3);
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
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	u[0] = FTOI12(0.0f * tex->u_scale + tex->u_offset);
	v[0] = FTOI12(0.0f * tex->v_scale + tex->v_offset);
	u[1] = FTOI12(1.0f * tex->u_scale + tex->u_offset);
	v[1] = FTOI12(1.0f * tex->v_scale + tex->v_offset);

	uvrgbwh0 = prim->uvrgb[0];
	uvrgbwh1 = prim->uvrgb[1];

	for( i = 0; i < n_prims*n_verts; i++ ){
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

static int GetResources( Work *work )
{
	
	//work->prim = InitRsprtPrim2( N_PRIMS, N_VERTS, TEX_CODE, SET_COL(54,72,100,0) );
	//if(!work->prim) return -1;
	
	//work->prim2 = InitRsprtPrim2( N_PRIMS2, N_VERTS2, TEX_CODE2, SET_COL(100,100,130,0) );
	//if(!work->prim2) return -1;
	
	work->prim3 = InitRsprtPrim2( N_PRIMS3, N_VERTS3, TEX_CODE3, SET_COL(160,160,160,0) );
	if(!work->prim3) return -1;

	//NewCircleSplush( &work->cs_root0, 100.0f, 1.0f, 2, -1 );

//		DG_InvisiblePrim2(work->prim);
//		DG_InvisiblePrim2(work->prim2);
	return (0);
}

void *NewColWater( FMATRIX *root )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		work->ray_col = root;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work);
			return NULL ;
		}
	}

	return (void *)work;
}

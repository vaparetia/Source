//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    cam_lamp.c
    カメラランプ
	2000/02/08 T.Shibata
	
	$Id: cam_lamp.c,v 1.1.1.3 2002/11/19 11:48:36 Yoshizawa1 Exp $

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

#define		GET_COL_R(_rgba)	(((_rgba)>>24)&0xff)
#define		GET_COL_G(_rgba)	(((_rgba)>>16)&0xff)
#define		GET_COL_B(_rgba)	(((_rgba)>> 8)&0xff)
#define		GET_COL_A(_rgba)	(((_rgba)>> 0)&0xff)
#define		SET_COL(_r,_g,_b,_a)	(((_r)<<24)|((_g)<<16)|((_b)<<8)|(_a))

#define		MEM_SCR_POS		((void*)(SCRPAD_ADDR))
#define		MEM_SCR_UV		((void*)(SCRPAD_ADDR+sizeof(FVECTOR)*256))

#define		N_PRIMS_LAMP		(1)
#define		N_VERTS_LAMP		(1)
#define		N_PRIMS_BASE		(1)
#define		N_VERTS_BASE		(4)

#define		ACTV_COL_R	(210.0f)
#define		ACTV_COL_G	(100.0f)
#define		ACTV_COL_B	(80.0f)
#define		ACTV_COL_A	(128.0f)

#define		SLEEP_COL_R	(63.0f*1.5f)
#define		SLEEP_COL_G	(35.0f*1.2f)
#define		SLEEP_COL_B	(17.0f*1.2f)
#define		SLEEP_COL_A	(0.0f)

extern float BP_AdjustTick5(float);
#define		FADE_TIME	(BP_AdjustTick5(32.0f))

#define		DIFF_COL_R	((SLEEP_COL_R-ACTV_COL_R)/FADE_TIME)
#define		DIFF_COL_G	((SLEEP_COL_G-ACTV_COL_G)/FADE_TIME)
#define		DIFF_COL_B	((SLEEP_COL_B-ACTV_COL_B)/FADE_TIME)
#define		DIFF_COL_A	((SLEEP_COL_A-ACTV_COL_A)/FADE_TIME)
#if 0
static int DebugFlags[2] = {1,1};
static int count = 0;
#endif

//extern void *NewCmeraLamp( FMATRIX *world, int *flags )
#define		CM_LAMP_FLAGS_ACTV		(0x80000001)
#define		CM_LAMP_FLAGS_SLEEP		(0x80000002)


#define		CM_LAMP_FLAGS_ON		(0x80000010)
#define		CM_LAMP_FLAGS_OFF		(0x80000020)

#define		CM_LAMP_MODE_ACT		(0x0001)
#define		CM_LAMP_MODE_SLEEP		(0x0002)

#define		CM_LAMP_MODE_ON			(0x0010)
#define		CM_LAMP_MODE_OFF		(0x0020)

typedef struct {
	GV_ACT_EX	actor;
	DG_PRIM2	*prim_lamp;
	DG_PRIM2	*prim_base;
	DG_OBJS		*objs;
	FMATRIX		*world;
	FMATRIX		root;
	FMATRIX		light[2];
	FVECTOR		color;
	int			name;
	int			mode;
	int			*flags;
	int			count;
} Work ;

static FVECTOR LampBase_Pos[] = {
	{  150.0f, 1.0f,  100.0f, 1.0f },
	{ -150.0f, 1.0f,  100.0f, 1.0f },
	{  150.0f, 1.0f, -100.0f, 1.0f },
	{ -150.0f, 1.0f, -100.0f, 1.0f },
};

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
	DG_RotVectorW(in, out, 1 ) ;
#endif
}

static void SetPolyRGBA( DG_PRIM2 *prim, int n_verts, FVECTOR *color )
{
	int 			clock,i;
	DG_PRIM2_UVRGB	*uvrgb;

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	uvrgb = prim->uvrgb[clock];

	for( i = 0; i < n_verts; i++ ){		
		uvrgb->r = (int)color->vx;
		uvrgb->g = (int)color->vy;
		uvrgb->b = (int)color->vz;
		uvrgb->a = (int)(color->vw * 0.5f);
		uvrgb++;
	}
}

static void SetLampPosRGBA( DG_PRIM2 *prim, int n_verts, FVECTOR *color, FVECTOR *center )
{
	int 				clock,i;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	FVECTOR				*pos,fvtemp;
	FVECTOR				*eye_pos = (FVECTOR*)DG_Chanls[0].eye.m[3];
	
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	pos = prim->pos[clock];
	uvrgbwh = prim->uvrgb[clock];

	_sceVu0SubVector( &fvtemp, eye_pos, center );
	_sceVu0Normalize( &fvtemp, &fvtemp );
	_sceVu0ScaleVector( &fvtemp, &fvtemp, 100.0f );
	fvtemp.vy += 50.0f;
	for( i = 0; i < n_verts; i++ ){
		_sceVu0AddVector( pos, &fvtemp, center );
		uvrgbwh->r = (int)color->vx;
		uvrgbwh->g = (int)color->vy;
		uvrgbwh->b = (int)color->vz;
		uvrgbwh->a = (int)(color->vw * 0.5f);
		pos++;
		uvrgbwh++;
	}
}

static void Act(Work *work)
{
	static FVECTOR offset = { 0.0f, 450.0f, 130.0f, 1.0f };
//	static FVECTOR offset = { 0.0f, 450.0f, 260.0f, 1.0f };
	static FVECTOR diff_col = { 0, 0, 0, 0 };

   int		check = 0;
   int		flags = *work->flags;

   diff_col.x = DIFF_COL_R;
   diff_col.y = DIFF_COL_G;
   diff_col.z = DIFF_COL_B;
   diff_col.w = DIFF_COL_A;

#if 0
	if((GV_Time % 300) == 0){
		DebugFlags[work->count] <<= 1;
		if(DebugFlags[work->count] == 0x0004) DebugFlags[work->count] = 1;
		DebugFlags[work->count] |= 0x80000000;
	}
	//printf("%x\n",DebugFlags[work->count]);
#endif
	DG_COPY_MAT( &work->root, work->world );
	_RotTrans( (FVECTOR*)work->root.m[3], work->world, &offset );

	if(work->mode & CM_LAMP_MODE_ACT){
		//printf("act\n");
		_sceVu0SubVector( &work->color, &work->color, &diff_col );
		if(work->color.vx > ACTV_COL_R){
			work->color.vx = ACTV_COL_R;
			check++;
		}
		if(work->color.vy > ACTV_COL_G){
			work->color.vy = ACTV_COL_G;
			check++;
		}
		if(work->color.vz > ACTV_COL_B){
			work->color.vz = ACTV_COL_B;
			check++;
		}
		if(work->color.vw > ACTV_COL_A){
			work->color.vw = ACTV_COL_A;
			check++;
		}
		if(check == 4) work->mode &= ~CM_LAMP_MODE_ACT;		
	}else if(work->mode & CM_LAMP_MODE_SLEEP){
		//printf("sleep\n");
		_sceVu0AddVector( &work->color, &work->color, &diff_col );
		if(work->color.vx < SLEEP_COL_R){
			work->color.vx = SLEEP_COL_R;
			check++;
		}
		if(work->color.vy < SLEEP_COL_G){
			work->color.vy = SLEEP_COL_G;
			check++;
		}
		if(work->color.vz < SLEEP_COL_B){
			work->color.vz = SLEEP_COL_B;
			check++;
		}
		if(work->color.vw < SLEEP_COL_A){
			work->color.vw = SLEEP_COL_A;
			check++;
		}
		if(check == 4) work->mode &= ~CM_LAMP_MODE_SLEEP;
	}
	
	SetPolyRGBA( work->prim_base, 4, &work->color );
	SetLampPosRGBA( work->prim_lamp, 1, &work->color, (FVECTOR*)work->root.m[3] );
	DG_COPY_VEC( work->light[1].m[3], &work->color );
	if(!(flags & 0x80000000)) return;

	*work->flags &= 0x7fffffff;
	flags &= 0x7fffffff;

	if(flags&CM_LAMP_FLAGS_SLEEP){
		work->mode &= ~(0x00000003);
		work->mode |= CM_LAMP_MODE_SLEEP;
		//printf("ねーむれ\n");
	}else if(flags&CM_LAMP_FLAGS_ACTV){
		work->mode &= ~(0x00000003);
		work->mode |= CM_LAMP_MODE_ACT;
		//printf("ひかれー\n");
	}

}

static void Die(Work *work)
{
	if(work->objs) DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs );
	if(work->prim_lamp) GM_FreePrim2(work->prim_lamp);
	if(work->prim_base) GM_FreePrim2(work->prim_base);
	//printf("die\n");
}

static DG_OBJS *InitLampObj( FMATRIX *root, FVECTOR *pos, FMATRIX *light, int color )
{
	DG_DEF		*def;
	DG_OBJS		*objs;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("field_camera_light"), 'k' ) ) ;
	if(!def){ printf("ERR!! NO MODEL!!\n"); return NULL; }
	objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE, 0 );
	if(!objs) return NULL;
	if(DG_QueueObjs( objs )<0) return NULL;
	objs->root = root;
    GM_GroupObjs( objs, GM_CurrentMap ) ;
//	DG_InvisibleObjs( objs );

	
//	DG_GetLightMatrix( pos, light );
	DG_SetLightMatrix( objs, light );

	light[1].m[3][0] = (float)GET_COL_R(color);
	light[1].m[3][1] = (float)GET_COL_G(color);
	light[1].m[3][2] = (float)GET_COL_B(color);
	return objs;
}

static DG_PRIM2 *InitBasePrim2( FMATRIX *root, int n_prims, int n_verts, int tex_code,
								FVECTOR *verts, int color )
{
	DG_PRIM2	*prim;
	DG_TEX		*tex;
	int				i,j;
	int				u[2],v[2];
	FVECTOR			*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB	*uvrgb = MEM_SCR_UV;

	tex = DG_GetTexture(tex_code);
	if(!tex) { printf("ERR!! NO TEX!! <cam_lamp.c>\n"); return(NULL); }
	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX,
						 n_prims, n_verts);
	if(!prim){ printf("ERR!! MAKE PRIM!! <cam_lamp.c>\n"); return (NULL); }
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	prim->root = root;

	u[0] = FTOI12(0.0f * tex->u_scale + tex->u_offset);
	v[0] = FTOI12(0.0f * tex->v_scale + tex->v_offset);
	u[1] = FTOI12(1.0f * tex->u_scale + tex->u_offset);
	v[1] = FTOI12(1.0f * tex->v_scale + tex->v_offset);
	
	for( i = 0; i < n_prims; i++ ){
		for( j = 0; j < n_verts; j++ ){
			DG_COPY_VEC( pos, &verts[j] );

			uvrgb->r = GET_COL_R(color);
			uvrgb->g = GET_COL_G(color);
			uvrgb->b = GET_COL_B(color);
			uvrgb->a = GET_COL_A(color);
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

static DG_PRIM2 *InitLampPrim2( int n_prims, int n_verts, int tex_code, int size )
{
	DG_PRIM2		*prim;
	DG_TEX			*tex;
	int				i;
	int				u[2],v[2];
	FVECTOR				*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGBWH	*uvrgbwh = MEM_SCR_UV;

	tex = DG_GetTexture(tex_code);
	if(!tex) { printf("ERR!! NO TEX!! <cam_lamp.c>\n"); return(NULL); }
	prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
						 n_prims, n_verts);
	if(!prim){ printf("ERR!! MAKE PRIM!! <cam_lamp.c>\n"); return (NULL); }
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	u[0] = FTOI12(0.0f * tex->u_scale + tex->u_offset);
	v[0] = FTOI12(0.0f * tex->v_scale + tex->v_offset);
	u[1] = FTOI12(1.0f * tex->u_scale + tex->u_offset);
	v[1] = FTOI12(1.0f * tex->v_scale + tex->v_offset);
	
	for( i = 0; i < n_prims*n_verts; i++ ){
		DG_COPY_VEC( pos, &DG_ZeroVector );
		uvrgbwh->r = 0;
		uvrgbwh->g = 0;
		uvrgbwh->b = 0;
		uvrgbwh->a = 0;
		uvrgbwh->f0 = VERT_KICK_CODE;
		uvrgbwh->f1 = DRAW_KICK_CODE;
		uvrgbwh->q0 = 4096;
		uvrgbwh->q1 = 4096;
		uvrgbwh->u0 = u[0];
		uvrgbwh->v0 = v[0];
		uvrgbwh->u1 = u[1];
		uvrgbwh->v1 = v[1];
		uvrgbwh->w = size;
		uvrgbwh->h = size;

		pos++;
		uvrgbwh++;
	}

	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), n_verts*n_prims );

	return prim;
}

static int GetResources( Work *work )
{
	work->objs = InitLampObj( &work->root, (FVECTOR*)work->world->m[3], work->light,
	                          SET_COL((int)ACTV_COL_R,(int)ACTV_COL_G,(int)ACTV_COL_B,128) );
	if(!work->objs) return -1;
	work->prim_base = InitBasePrim2( &work->root, N_PRIMS_BASE, N_VERTS_BASE, 3594043,
	                                 LampBase_Pos, SET_COL((int)ACTV_COL_R,(int)ACTV_COL_G,(int)ACTV_COL_B,128) );
	if(!work->prim_base) return -1;
	work->prim_lamp = InitLampPrim2( N_PRIMS_LAMP, N_VERTS_LAMP, 3594043, 150 );
	if(!work->prim_lamp) return -1;

	work->color.vx = ACTV_COL_R;
	work->color.vy = ACTV_COL_G;
	work->color.vz = ACTV_COL_B;
	work->color.vw = ACTV_COL_A;
#if 0
	work->flags = &DebugFlags[count];
	work->count = count;
	count++;
#endif
	return (0);
}

void *NewCmeraLamp( FMATRIX *world, int *flags )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		work->world = world;
		work->flags = flags;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work);
			return NULL ;
		}
	}

	return (void *)work ;
}

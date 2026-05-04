//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    ts_min_smoke.c
    極小規模煙
	2000/05/18 T.Shibata
	
	$Id: ts_min_smoke.c,v 1.1.1.3 2002/11/19 11:48:47 Yoshizawa1 Exp $

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

#include	"def_dma.h"
#include	"utl_dma.h"

#include	"../util/ts_util.h"

#define		N_PRIMS		(1)
#define		N_VERTS		(16)

#define		COLOR_R		(0xff)
#define		COLOR_G		(0xff)
#define		COLOR_B		(0xff)
#define		ALPHA		(24)

#define		SUB_ALPHA	(8)

#define		LIFE_TIME 	((ALPHA*2)/SUB_ALPHA)

#define		SIZE		(20.0f)

#define		SPEED_SCALE	(30.0f)
#define		SPEED_MIN	(0.0f)

#define		ANGLE_SCALE		(192)
#define 	ANGLE_OFFSET	(ANGLE_SCALE/2)

#define		C_OFFSET	(50.0f)
#define		RESISTANCE	(0.8f)

#ifdef BP_PS2
#define		MEM_SCR_POS		((void *)( SCRPAD_ADDR))
#define		MEM_SCR_UV		((void *)( MEM_SCR_POS ) + sizeof(FVECTOR)*N_PRIMS*N_VERTS)
#define		MEM_SCR_VEC		((void *)( MEM_SCR_UV  ) + sizeof(DG_PRIM2_UVRGBWH)*N_PRIMS*N_VERTS)
#define		MEM_SCR_BPOS	((void *)( MEM_SCR_VEC ) + sizeof(FVECTOR)*N_PRIMS*N_VERTS)
#else
#define		MEM_SCR_POS		((void *)( SCRPAD_ADDR))
#define		MEM_SCR_UV		((void *)( (char *)MEM_SCR_POS + sizeof(FVECTOR)*N_PRIMS*N_VERTS))
#define		MEM_SCR_VEC		((void *)( (char *)MEM_SCR_UV + sizeof(DG_PRIM2_UVRGBWH)*N_PRIMS*N_VERTS))
#define		MEM_SCR_BPOS	((void *)( (char *)MEM_SCR_VEC + sizeof(FVECTOR)*N_PRIMS*N_VERTS))
#endif

typedef struct
{
	GV_ACT_EX		actor ;
	DG_PRIM2	*prim;
	
	int			timer;
	FVECTOR		dvec[N_PRIMS*N_VERTS];
} Work ;


static void Act(Work *work)
{
	int					i,j,clock;
	FVECTOR 			*pos,*dvec,*b_pos;
	DG_PRIM2_UVRGBWH 	*uvrgbwh;


	if( ++work->timer > LIFE_TIME ){
		DG_InvisiblePrim2(work->prim) ;		
		GV_DestroyActor( work );
	}

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	pos = MEM_SCR_POS;
	uvrgbwh = MEM_SCR_UV;
	b_pos = MEM_SCR_BPOS;
	dvec = MEM_SCR_VEC;
	
	TS_Scr_Mem( MEM_SCR_POS,work->prim->pos[clock], sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( MEM_SCR_BPOS,work->prim->pos[1-clock], sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( MEM_SCR_VEC,work->dvec, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( MEM_SCR_UV,work->prim->uvrgb[clock], sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS );

	for( i = 0; i < N_PRIMS; i++ ){
		for( j = 0; j < N_VERTS; j++ ){
			fpu_AddVectors(pos,b_pos,dvec);
			fpu_MulVectorScaler(dvec,dvec,RESISTANCE);
			//fpu_DivVectorScaler(dvec,dvec,RESISTANCE);
			
			if(uvrgbwh->a > SUB_ALPHA) uvrgbwh->a -= SUB_ALPHA;

			pos++;
			b_pos++;
			dvec++;
			uvrgbwh++;		
		}
	}
	TS_Scr_Mem( work->prim->pos[clock], MEM_SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->dvec, MEM_SCR_VEC, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->prim->uvrgb[clock], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS );
}

static void Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim);
}

static void InitWorkData( Work *work, DG_TEX *tex, FVECTOR *center, FVECTOR *force )
{
	int 					i,j;
	FVECTOR					*pos,*dvec;
	DG_PRIM2_UVRGBWH		*uvrgbwh;
	SVECTOR					svtemp;
	FMATRIX					fmtemp;
	short					u0,v0,u1,v1;
	float					angle;
	
	pos = MEM_SCR_POS;
	uvrgbwh = MEM_SCR_UV;
	dvec = MEM_SCR_VEC;

	//センターをカメラ方向にずらす
//	DG_COPY_VEC( &eye,&DG_Chanls[DG_CHANL_MAIN].eye.m[3] );
//	fpu_SubVectors(&eye,&eye,center);
//	fpu_VectorNormal(&eye);
//	fpu_DivVectorScaler(&eye,&eye,C_OFFSET);
//	fpu_AddVectors(&w_center,&eye,center);

	//
	TS_MakeMatrix(&fmtemp,force,center);
	
	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	for(i = 0; i < N_PRIMS; i++ ){
		for(j = 0; j < N_VERTS; j++ ){
			// init pos
			DG_COPY_VEC(pos,center);

			// init dvec
			dvec->vx = 0.0f;
			dvec->vy = 0.0f;
			dvec->vz = rnd()*SPEED_SCALE + SPEED_MIN;
			dvec->vw = 0.0f;

			svtemp.vx = (irnd()%ANGLE_SCALE)-ANGLE_OFFSET;
			svtemp.vy = (irnd()%ANGLE_SCALE)-ANGLE_OFFSET;
			svtemp.vz = 0;
		
			DG_SetPos(&fmtemp);
			DG_RotatePos(&svtemp) ;
			DG_RotVector(dvec,dvec,1);
			
			// init uvrgbwh
			uvrgbwh->u0 = u0;
			uvrgbwh->v0 = v0;
			uvrgbwh->u1 = u1;
			uvrgbwh->v1 = v1;
			uvrgbwh->q0 = 4096;
			uvrgbwh->q1 = 4096;
			uvrgbwh->f0 = 0x0fff;
			uvrgbwh->f1 = 0x0fff;
		
			uvrgbwh->r = COLOR_R;
			uvrgbwh->g = COLOR_G;
			uvrgbwh->b = COLOR_B;
			uvrgbwh->a = ALPHA;

			angle = rnd()*2*PI;

	
			uvrgbwh->w = SIZE * cosf(angle);
			uvrgbwh->h = SIZE * sinf(angle);
		
			pos++;
			dvec++;
			uvrgbwh++;
		}	
	}
	
	TS_Scr_Mem( work->prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->dvec, MEM_SCR_VEC, sizeof(FVECTOR), N_PRIMS*N_VERTS );

}


static int GetResources( Work *work, FVECTOR *center, FVECTOR *force )
{
	DG_PRIM2	*prim = NULL;
	DG_TEX		*tex = NULL;

//	tex = DG_GetTexture(GV_StrCode("chi01_msk"));
	tex = DG_GetTexture(10984814);
	
	if(!tex){
		printf(" not texture int flour_Down.c \n");
		return(-1);
	}
	prim = work->prim = GM_MakePrim2(DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
									  N_PRIMS,
									  N_VERTS );
	if(!prim){
		printf("null prim\n");
		return (-1);
	}

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	
	// DataInit
	work->timer = 0;

	InitWorkData( work, tex, center, force );
	
	return (0);
}

/*
	center:	発生ポイント
	force : 発生方向
*/

void *NewTs_Min_Smoke( FVECTOR *center, FVECTOR *force )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		if(GetResources(work,center,force) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

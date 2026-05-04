//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    ts_min_fog.c
    極小規模もわもわ
	2000/05/19 T.Shibata
	
	$Id: ts_min_fog.c,v 1.1.1.3 2002/11/19 11:48:47 Yoshizawa1 Exp $

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
#define		N_VERTS		(12)

#define		COLOR_R		(0x20)
#define		COLOR_G		(0x20)
#define		COLOR_B		(0x20)
#define		ALPHA		(24)

#define		SUB_ALPHA	(2)

#define		LIFE_TIME 	((ALPHA*2)/SUB_ALPHA)

#define		SIZE		(20)
#define		MIN_SIZE	(20)
#define		MAX_SIZE	(400)
#define		ADD_SIZE	(MAX_SIZE/LIFE_TIME)

#define		SPEED_SCALE	(16.0f)
#define		SPEED_MIN	(0.0f)

#define		ANGLE_SCALE		(2048)
#define 	ANGLE_OFFSET	(ANGLE_SCALE/2)

#define		C_OFFSET	(500.0f)
#define		RESISTANCE	(0.8f)

#ifdef BP_PS2
#define		MEM_SCR_POS		((void *)( SCRPAD_ADDR))
#define		MEM_SCR_UV		((void *)( MEM_SCR_POS ) + sizeof(FVECTOR)*N_PRIMS*N_VERTS)
#define		MEM_SCR_VEC		((void *)( MEM_SCR_UV  ) + sizeof(DG_PRIM2_UVRGBWH)*N_PRIMS*N_VERTS)
#define		MEM_SCR_WPOS	((void *)( MEM_SCR_VEC ) + sizeof(FVECTOR)*N_PRIMS*N_VERTS)
#else
#define		MEM_SCR_POS		((void *)( SCRPAD_ADDR))
#define		MEM_SCR_UV		((void *)( (char *)MEM_SCR_POS + sizeof(FVECTOR)*N_PRIMS*N_VERTS) )
#define		MEM_SCR_VEC		((void *)( (char *)MEM_SCR_UV  + sizeof(DG_PRIM2_UVRGBWH)*N_PRIMS*N_VERTS) )
#define		MEM_SCR_WPOS	((void *)( (char *)MEM_SCR_VEC + sizeof(FVECTOR)*N_PRIMS*N_VERTS) )
#endif

typedef struct
{
	GV_ACT_EX		actor ;
	DG_PRIM2	*prim;
	
	int			timer;
	
	FVECTOR		*center;
	
	FVECTOR		pos[N_PRIMS*N_VERTS];
	FVECTOR		dvec[N_PRIMS*N_VERTS];
	float		angle[N_PRIMS*N_VERTS];
	
} Work ;


static void Act(Work *work)
{
	int					i,j,clock;
	FVECTOR 			*pos,*dvec,*w_pos;
	FVECTOR				fvtemp,center;
	DG_PRIM2_UVRGBWH 	*uvrgbwh;
	float				scale,*angle;

	if( ++work->timer > LIFE_TIME+4 ){
		DG_InvisiblePrim2(work->prim) ;		
		GV_DestroyActor( work );
	}

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	pos = MEM_SCR_POS;
	uvrgbwh = MEM_SCR_UV;
	w_pos = MEM_SCR_WPOS;
	dvec = MEM_SCR_VEC;
	angle = work->angle;
	
	TS_Scr_Mem( MEM_SCR_POS,work->prim->pos[clock], sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( MEM_SCR_WPOS,work->pos, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( MEM_SCR_VEC,work->dvec, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( MEM_SCR_UV,work->prim->uvrgb[clock], sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS );

	DG_COPY_VEC( &fvtemp, &DG_Chanls[DG_CHANL_MAIN].eye.m[3] );
	fpu_SubVectors( &fvtemp, &fvtemp, work->center);
	fpu_VectorNormal( &fvtemp );
	fpu_MulVectorScaler( &fvtemp,&fvtemp,C_OFFSET);
	fpu_AddVectors( &center, work->center, &fvtemp );
	
	for( i = 0; i < N_PRIMS; i++ ){
		for( j = 0; j < N_VERTS; j++ ){
			fpu_AddVectors(w_pos,w_pos,dvec);
			fpu_MulVectorScaler(dvec,dvec,RESISTANCE);

			if(uvrgbwh->a > SUB_ALPHA) uvrgbwh->a -= SUB_ALPHA;
			if(MIN_SIZE + work->timer*ADD_SIZE < MAX_SIZE){ 
				scale = MIN_SIZE + work->timer*ADD_SIZE;
				uvrgbwh->w = scale * cosf(*angle);
				uvrgbwh->h = scale * sinf(*angle);
			}
			fpu_AddVectors(pos,w_pos,&center);

			angle++;
			pos++;
			w_pos++;
			dvec++;
			uvrgbwh++;		
		}
	}
	
	TS_Scr_Mem( work->prim->pos[clock], MEM_SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->pos, MEM_SCR_WPOS, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->dvec, MEM_SCR_VEC, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->prim->uvrgb[clock], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS );
}

static void Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim);
}

static void InitWorkData( Work *work, DG_TEX *tex, FVECTOR *center )
{
	int 					i,j;
	FVECTOR					*pos,*dvec;
//	FVECTOR		  			w_force,w_center,eye;
	DG_PRIM2_UVRGBWH		*uvrgbwh;
//	SVECTOR					svtemp;
//	FMATRIX					fmtemp;
//	FVECTOR					fvtemp;
	short					u0,v0,u1,v1;
	float					*angle;
	
	pos = MEM_SCR_POS;
	uvrgbwh = MEM_SCR_UV;
	dvec = MEM_SCR_VEC;
	angle = work->angle;
//	TS_MakeMatrix(&fmtemp,force,center);
	
	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	for(i = 0; i < N_PRIMS; i++ ){
		for(j = 0; j < N_VERTS; j++ ){
			// init pos
			//DG_COPY_VEC(pos,&center);
			DG_COPY_VEC(pos,&DG_ZeroVector);

			// init dvec
			dvec->vx = frnd()*2.0f;
			dvec->vy = frnd()*2.0f;
			dvec->vz = frnd()*2.0f;
			dvec->vw = 0.0f;

			fpu_VectorNormal( dvec );
			fpu_MulVectorScaler( dvec,dvec,rnd()*SPEED_SCALE + SPEED_MIN);
				
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

			*angle = rnd()*2*PI;
	
			uvrgbwh->w = MIN_SIZE * cosf(*angle);
			uvrgbwh->h = MIN_SIZE * sinf(*angle);
		
			pos++;
			dvec++;
			uvrgbwh++;
			angle++;
		}	
	}

	TS_Scr_Mem( work->prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->pos, MEM_SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->dvec, MEM_SCR_VEC, sizeof(FVECTOR), N_PRIMS*N_VERTS );

}


static int GetResources( Work *work, FVECTOR *center )
{
	DG_PRIM2	*prim = NULL;
	DG_TEX		*tex = NULL;

//	tex = DG_GetTexture(GV_StrCode("chi01_msk"));
//	tex = DG_GetTexture(GV_StrCode("blood_1e_msk"));
	tex = DG_GetTexture(15638496);
	
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
	work->center = center;

	//test
//	work->center = &work->test_center;
//	DG_COPY_VEC(&work->test_center,center);
	
	InitWorkData( work, tex, center );
	
	return (0);
}

/*
	center:	発生ポイント
*/

void *NewTs_Min_Fog( FVECTOR *center )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		if(GetResources(work,center) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
//	printf("new min fog\n");
	}

	return (void *)work ;
}

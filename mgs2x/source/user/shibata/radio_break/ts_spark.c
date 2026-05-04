//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   ts_spark.c  
   火花
   2000/05/17 T.Shibata

   $Id: ts_spark.c,v 1.1.1.3 2002/11/19 11:48:48 Yoshizawa1 Exp $
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


#define		N_PRIMS		(16)
#define		N_VERTS		(3)

#define		COLOR_R		(0xb0)
#define		COLOR_G		(0x50)
#define		COLOR_B		(0x50)
#define		ALPHA		(0x28)//アンチエイリアスを使用するときには１２８にする


#define		COLOR_G_OFF		(0x60)
#define		COLOR_G_MOD		(0x40)

#define		SPEED_SCALE		(16.0f)
#define		SPEED_MIN		(24.0f)
#define		ANGLE_SCALE		(PI/18)

#define		ANGLE_SCALE_S	(682)
#define		ANGLE_OFFSET_S	(ANGLE_SCALE_S/2)

#define		LIFE_TIME		(ALPHA)

#define		GRAVITATION		(1.0f)
#define		RESISTANCE		(0.8f)


#ifdef BP_PS2
#define		MEM_SCR_POS		((void *)(SCRPAD_ADDR))
#define		MEM_SCR_UV		((void*)(MEM_SCR_POS + sizeof(FVECTOR)       *N_PRIMS*N_VERTS))
#define		MEM_SCR_VEC		((void*)(MEM_SCR_UV  + sizeof(DG_PRIM2_UVRGB)*N_PRIMS*N_VERTS))
#define		MEM_SCR_BPOS	((void*)(MEM_SCR_VEC + sizeof(FVECTOR)       *N_PRIMS))
#else
#define		MEM_SCR_POS		((void *)(SCRPAD_ADDR))
#define		MEM_SCR_UV		((void*)((char *)MEM_SCR_POS + sizeof(FVECTOR)       *N_PRIMS*N_VERTS))
#define		MEM_SCR_VEC		((void*)((char *)MEM_SCR_UV  + sizeof(DG_PRIM2_UVRGB)*N_PRIMS*N_VERTS))
#define		MEM_SCR_BPOS	((void*)((char *)MEM_SCR_VEC + sizeof(FVECTOR)       *N_PRIMS))
#endif

typedef struct
{
	GV_ACT_EX	 	actor ;
	DG_PRIM2   	*prim ;
	int			timer;
	FVECTOR		dvec[N_PRIMS];
} Work ;

static   void  Act(Work *work)
{
	FVECTOR 		*pos,*b_pos,*dvec;
	DG_PRIM2_UVRGB	*uvrgb;
	int				i,j,clock;

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
	b_pos = MEM_SCR_BPOS;
	uvrgb = MEM_SCR_UV;
	dvec = MEM_SCR_VEC;
	
	TS_Mem_Scr( MEM_SCR_BPOS, work->prim->pos[1-clock], sizeof(FVECTOR)       , N_VERTS*N_PRIMS );
	TS_Mem_Scr( MEM_SCR_UV  , work->prim->uvrgb[clock], sizeof(DG_PRIM2_UVRGB), N_VERTS*N_PRIMS );
	TS_Mem_Scr( MEM_SCR_VEC , work->dvec              , sizeof(FVECTOR)       , N_PRIMS );

	for( i = 0; i < N_PRIMS; i++ ){
		
		for( j = 0; j < N_VERTS; j++ ){
			if(uvrgb->a) uvrgb->a -= 2;
			if(uvrgb->g) uvrgb->g -= 2;
			uvrgb++;
		}

		memcpy(&pos[1],b_pos,sizeof(FVECTOR)*(N_VERTS-1));
		fpu_AddVectors(pos,b_pos,dvec);

		dvec->vx *= RESISTANCE;
		dvec->vy = ( dvec->vy - GRAVITATION ) * RESISTANCE;
		dvec->vz *= RESISTANCE;

		b_pos+=N_VERTS;
		pos+=N_VERTS;
		dvec++;
	}
	
	TS_Scr_Mem( work->prim->pos[clock]  , MEM_SCR_POS, sizeof(FVECTOR)       , N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->prim->uvrgb[clock], MEM_SCR_UV , sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->dvec              , MEM_SCR_VEC, sizeof(FVECTOR)       , N_PRIMS );
	
//void TS_Mem_Scr( void *dst, void *src, int size, int num )
}

static   void  Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim) ;
}

static void InitWorkData( Work *work, DG_TEX *tex, FVECTOR *center, FVECTOR *vec, float pow)
{
	FVECTOR 		fvtemp;
	FVECTOR 		*pos,*dvec;
	DG_PRIM2_UVRGB	*uvrgb;
	FMATRIX			fmtemp;
//	float			ftemp_0,ftemp_1,scale;
	int				i,j;
	short			u0,v0,u1,v1;
	SVECTOR			svtemp;

	//tex uv set
	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	//出る方向
	if(vec){
		_sceVu0Normalize(&fvtemp,vec);
	}else{
		fvtemp.vx = frnd();
		fvtemp.vy = frnd();
		fvtemp.vz = frnd();
	}
	TS_MakeMatrix( &fmtemp, &fvtemp ,center );
#if 0
	printf("m[0] vx %2.6f:vy %2.6f:xz %2.6f\n"  , fmtemp.m[0][0], fmtemp.m[0][1], fmtemp.m[0][2]);
	printf("m[1] vx %2.6f:vy %2.6f:xz %2.6f\n"  , fmtemp.m[1][0], fmtemp.m[1][1], fmtemp.m[1][2]);
	printf("m[2] vx %2.6f:vy %2.6f:xz %2.6f\n\n", fmtemp.m[2][0], fmtemp.m[2][1], fmtemp.m[2][2]);
#endif
//	DG_SetPos(&fmtemp);
//	DG_RotVector(MEM_SCR_VEC,MEM_SCR_VEC,N_PRIMS);
	
	pos = MEM_SCR_POS;
	uvrgb = MEM_SCR_UV;
	dvec = MEM_SCR_VEC;

	for( i = 0; i < N_PRIMS; i++ ){
		
#if 0
		scale = rnd()*SPEED_SCALE + SPEED_MIN;
		ftemp_0 = frnd()*ANGLE_SCALE;
		ftemp_1 = frnd()*ANGLE_SCALE;

		dvec->vx = scale * -sinf(ftemp_0) * -sinf(ftemp_1);
		dvec->vy = scale * -sinf(ftemp_0) *  cosf(ftemp_1);
		dvec->vz = scale *  cosf(ftemp_0);	
		dvec->vw = 0.0f;
#else
		dvec->vx = 0.0f;
		dvec->vy = 0.0f;
		if( pow <= 0.0f ){
			dvec->vz = rnd()*SPEED_SCALE + SPEED_MIN;
		}else{
			dvec->vz = pow*0.5f*(rnd()+1.0f);
		}
		dvec->vw = 0.0f;

		svtemp.vx = (irnd()%ANGLE_SCALE_S)-ANGLE_OFFSET_S;
		svtemp.vy = (irnd()%ANGLE_SCALE_S)-ANGLE_OFFSET_S;
		svtemp.vz = 0;
		
		DG_SetPos(&fmtemp);
		DG_RotatePos(&svtemp) ;
		DG_RotVector(dvec,dvec,1);
#endif
	
	//	printf("vec vx %f: vy %f: vz %f\n",dvec->vx,dvec->vy,dvec->vz );
		for( j = 0; j < N_VERTS; j++ ){
			DG_COPY_VEC( pos, center );
			
			if( j & 1 ){
				uvrgb->u = u1;
				uvrgb->v = v1;
			}else{
				uvrgb->u = u0;
				uvrgb->v = v0;
			}

			if(j){
				uvrgb->f = DRAW_KICK_CODE;
			}else{
				uvrgb->f = VERT_KICK_CODE;
			}
			
			uvrgb->q = 4096;
			uvrgb->r = COLOR_R;
			uvrgb->g = COLOR_G_OFF + (int)(COLOR_G_MOD*frnd());
			uvrgb->b = COLOR_B;
			uvrgb->a = ALPHA;

			pos++;
			uvrgb++;
			
		}
		dvec++;
	}

	
    // スクラッチパッドからメインメモリへ転送	
	TS_Scr_Mem( work->prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS );
	TS_Scr_Mem( work->dvec, MEM_SCR_VEC, sizeof(FVECTOR), N_PRIMS );
/*
	int		DG_SetTmpLight( point, r_range, e_range )
	FVECTOR		*point ;	光源中心位置
	float		r_range ;	明度基準キョリ
	float		e_range ;	最大到達キョリ

	int		DG_SetTmpLight2( point, r_range, e_range, color, flag )
	FVECTOR		*point ;	光源中心位置
	float		r_range ;	明度基準キョリ（r_range*2で明るさ０、r_rangeで明るさが０．７５になる）
	float		e_range ;	光源有効ボックスの大きさ（r_range*2が理想）
	int			color ;		色
	int			flag ;		光源フラグ（LIT_FLAG_BGONLY、LIT_FLAG_CHARAONLY）
*/
}
static   int  GetResources( Work *work, FVECTOR *center, FVECTOR *vec, float pow )
{
	DG_PRIM2	  *prim ;
	DG_TEX		*tex ;

	prim = work->prim = GM_MakePrim2(DG_PRIM2_LINE|DG_PRIM2_TEX|DG_PRIM2_SHADE|DG_PRIM2_ALPHA,//|DG_PRIM2_ANTIALIASING,
									 N_PRIMS,
									 N_VERTS );
	if(!prim){
		printf("null prim\n");
		return (-1);
	}
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	

//	tex = DG_GetTexture(GV_StrCode("col256_bld100"));
	tex = DG_GetTexture(7338993);
	if(!tex){
		printf(" not texture int flour_Down.c \n");
		return(-1);
	}

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	
	// DataInit
	work->timer = 0;

	InitWorkData( work, tex, center, vec, pow );

	return (0);
}


void *NewTs_Spark( FVECTOR *center, FVECTOR *vec, float pow )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		if(GetResources( work, center,vec,pow ) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;

}

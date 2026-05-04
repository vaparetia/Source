//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    ab_mind.c
	ヘッドマーク壊れたあと
	2000/07/17 T.Shibata

	$Id: ab_mind.c,v 1.1.1.3 2002/11/19 11:48:46 Yoshizawa1 Exp $
	
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"
#include	"utl_dma.h"

#include	"../util/ts_util.h"

#define		N_POOS		(3)
#define		N_PANS		(8)

#define		N_PRIMS0	(1)
#define		N_VERTS0	(24)

#define		N_PRIMS1	(1)
#define		N_VERTS1	(24)

#define		COLOR_R		(0x80)
#define		COLOR_G		(0x80)
#define		COLOR_B		(0x80)
#define		ALPHA		(16)

#define		SUB_ALPHA	(2)

#define		MAX_SIZE		(40)
#define		MIN_SIZE		(20)

#define		MAX_Y		(400.0f)
#define		MIN_Y		(200.0f)

#define		SCL_SPEED		(2.0f)
#define		OFF_SPEED		(5.0f)

#define		RADI			(M_PI/4)

#define		FLAGS_CHECK		(0x000f)
#define		FLAGS_NOP		(0)
#define		FLAGS_INIT0		(1)
#define		FLAGS_POO		(2)
#define		FLAGS_INIT2		(3)
#define		FLAGS_PAN		(4)
#define		FLAGS_CLEAR0	(5)
#define		FLAGS_CLEAR1	(6)

#ifdef BP_PS2
#define		MEM_SCR_POS0	((void *)( SCRPAD_ADDR))
#define		MEM_SCR_UV0		((void *)( MEM_SCR_POS0 + sizeof(FVECTOR)*N_PRIMS0*N_VERTS0))
#define		MEM_SCR_POS1	((void *)( MEM_SCR_UV0  + sizeof(DG_PRIM2_UVRGBWH)*N_PRIMS0*N_VERTS0))
#define		MEM_SCR_UV1		((void *)( MEM_SCR_POS1 + sizeof(FVECTOR)*N_PRIMS1*N_VERTS1))
#define		MEM_SCR_DVEC	((void *)( MEM_SCR_UV1 + sizeof(DG_PRIM2_UVRGB)*N_PRIMS1*N_VERTS1))
#define		MEM_SCR_WPOS	((void *)( MEM_SCR_DVEC + sizeof(FVECTOR)*N_POOS*N_PANS))
#define		MEM_SCR_BOTTOM	((void *)( MEM_SCR_WPOS + sizeof(FVECTOR)*N_POOS*N_PANS))
#else
#define		MEM_SCR_POS0	((void *)( SCRPAD_ADDR))
#define		MEM_SCR_UV0		((void *)( (char *)MEM_SCR_POS0 + sizeof(FVECTOR)*N_PRIMS0*N_VERTS0))
#define		MEM_SCR_POS1	((void *)( (char *)MEM_SCR_UV0  + sizeof(DG_PRIM2_UVRGBWH)*N_PRIMS0*N_VERTS0))
#define		MEM_SCR_UV1		((void *)( (char *)MEM_SCR_POS1 + sizeof(FVECTOR)*N_PRIMS1*N_VERTS1))
#define		MEM_SCR_DVEC	((void *)( (char *)MEM_SCR_UV1 + sizeof(DG_PRIM2_UVRGB)*N_PRIMS1*N_VERTS1))
#define		MEM_SCR_WPOS	((void *)( (char *)MEM_SCR_DVEC + sizeof(FVECTOR)*N_POOS*N_PANS))
#define		MEM_SCR_BOTTOM	((void *)( (char *)MEM_SCR_WPOS + sizeof(FVECTOR)*N_POOS*N_PANS))
#endif

typedef struct
{
	GV_ACT_EX	actor ;
	FVECTOR		pos[N_POOS*N_PANS];
	FVECTOR		dvec[N_POOS*N_PANS];
	int			flags[N_POOS];
	short		wait[N_POOS];
	short		life[N_POOS];
	DG_PRIM2	*prim0;
	DG_PRIM2	*prim1;
	FMATRIX		*world;
	int			timer;
} Work ;

static void Act(Work *work)
{
	int 				i,j,clock;
	FVECTOR				*pos0,*pos1;
	FVECTOR				*wpos,*dvec;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGB		*uvrgb;
	FVECTOR				*eye = (FVECTOR*)DG_Chanls[0].eye.m[3],force;
	FMATRIX				mat;
//		DG_InvisiblePrim2(work->prim0);
//		DG_InvisiblePrim2(work->prim1);
//		GV_DestroyActor( work );
	
	pos0 = MEM_SCR_POS0;
	pos1 = MEM_SCR_POS1;
	dvec = MEM_SCR_DVEC;
	wpos = MEM_SCR_WPOS;
	uvrgbwh = MEM_SCR_UV0;
	uvrgb = MEM_SCR_UV1;
	
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim0 ) )
   {
      return;
   }
	DG_SwitchBuffPrim2( work->prim1 );
	clock = work->prim0->buffer_clock;

	GM_GroupPrim2( work->prim0, GM_CurrentStageMap );
	GM_GroupPrim2( work->prim1, GM_CurrentStageMap );

	TS_Mem_Scr( MEM_SCR_POS0, work->prim0->pos[1-clock],sizeof(FVECTOR),N_VERTS0*N_PRIMS0);
	TS_Mem_Scr( MEM_SCR_POS1, work->prim1->pos[1-clock],sizeof(FVECTOR),N_VERTS1*N_PRIMS1);
	TS_Mem_Scr( MEM_SCR_UV0,  work->prim0->uvrgb[1-clock],sizeof(DG_PRIM2_UVRGBWH),N_VERTS0*N_PRIMS0);
	TS_Mem_Scr( MEM_SCR_UV1,  work->prim1->uvrgb[1-clock],sizeof(DG_PRIM2_UVRGB),N_VERTS1*N_PRIMS1);
	TS_Mem_Scr( MEM_SCR_WPOS, work->pos,sizeof(FVECTOR),N_POOS*N_PANS);
	TS_Mem_Scr( MEM_SCR_DVEC, work->dvec,sizeof(FVECTOR),N_POOS*N_PANS);
	
	for( i = 0; i < N_POOS; i++ ){
		switch(work->flags[i] & FLAGS_CHECK){
		  case FLAGS_NOP:
			if(work->wait[i] < work->timer) work->flags[i]++;
			break;
		  case FLAGS_INIT0:
			dvec[1].vx = 50.0f*frnd();
			dvec[1].vy = MIN_Y;
			dvec[1].vz = 50.0f*frnd();
			dvec[1].vw = 0.0f;

			dvec[0].vx = 10.0f * frnd();
			dvec[0].vy = OFF_SPEED;
			dvec[0].vz = frnd()/(6.0f*M_PI);
			dvec[0].vw = frnd()/(6.0f*M_PI);

			//DG_COPY_VEC( &dvec[1], &fvtemp0 );
			
			uvrgbwh->w = MAX_SIZE>>1;
			uvrgbwh->h = MAX_SIZE>>1;
			
			for( j = 0; j < 8; j++ ) DG_COPY_VEC( &wpos[j], &dvec[1] );

			DG_SetPos(work->world);
			DG_PutVector(wpos,pos0,1);
			DG_PutVector(wpos,pos1,8);
			
			work->flags[i]++;
			break;
		  case FLAGS_POO:
			if( !(work->timer % 3) ){
				memmove( &wpos[1], &wpos[0],sizeof(FVECTOR)*7 );
			}
			
			wpos[0].vy += dvec[0].vy;
			wpos[0].vx = dvec[0].vx * cosf(dvec[0].vz*wpos[0].vy) + dvec[1].vx;
			wpos[0].vz = dvec[0].vx * sinf(dvec[0].vw*wpos[0].vy) + dvec[1].vz;
			dvec[0].vy *= 0.95f;
			//fpu_AddVectors(&wpos[0],&dvec[1],&wpos[0]);
			//fpu_AddVectors(&wpos[0],&dvec[0],&wpos[0]);

			DG_SetPos(work->world);
			DG_PutVector(wpos,pos0,1);
			DG_PutVector(wpos,pos1,8);

            if( --work->life[i] < 0 )work->flags[i] = work->flags[i]++;
			break;
		  case FLAGS_INIT2:	
			for( j = 0; j < 8; j++ ){
				DG_COPY_VEC( &pos0[j], &pos0[0] );
	//			DG_COPY_VEC( &wpos[j], &wpos[0] );
				DG_COPY_VEC( &wpos[j], &DG_ZeroVector );
#if 0
				dvec[j].vx = 5.0f * frnd();
				dvec[j].vy = 5.0f * frnd();
				dvec[j].vz = 5.0f * frnd();
				dvec[j].vw = 0.0f;
#else
				dvec[j].vx = 2.0f *	cosf( RADI*j );
				dvec[j].vy = 2.0f * sinf( RADI*j );
				dvec[j].vz = 0.0f;
				dvec[j].vw = 0.0f;
#endif
				uvrgbwh[j].a = 64;
				uvrgbwh[j].w = MIN_SIZE>>1;
				uvrgbwh[j].h = MIN_SIZE>>1;
			}
			work->life[i] = 32;
			work->flags[i]++;
			break;
		  case FLAGS_PAN:
			for( j = 0; j < 8; j++ ){
				fpu_AddVectors(&wpos[j],&dvec[j],&wpos[j]);
				fpu_MulVectorScaler(&dvec[j],&dvec[j],0.95f);
				if(uvrgbwh[j].a) uvrgbwh[j].a -= 2;
			}
			fpu_SubVectors(&force,eye,&pos1[0]);
			TS_MakeMatrix(&mat,&force,&pos1[0]);
			DG_SetPos(&mat);
			DG_PutVector(wpos,pos0,8);
			memmove(&pos1[1],&pos1[0],sizeof(FVECTOR)*7);
            if( --work->life[i] < 0 )work->flags[i] = work->flags[i]++;
			break;
		  case FLAGS_CLEAR1:
			work->life[i] = 64;
			work->wait[i] = work->timer + 49;
		  case FLAGS_CLEAR0:
			for( j = 0; j < 8; j++ ){
				DG_COPY_VEC(&pos0[j],&DG_ZeroVector);
				DG_COPY_VEC(&pos1[j],&DG_ZeroVector);
				DG_COPY_VEC(&dvec[j],&DG_ZeroVector);
				DG_COPY_VEC(&wpos[j],&DG_ZeroVector);
				
				uvrgbwh[j].w = 0;
				uvrgbwh[j].h = 0;
				uvrgbwh[j].a = 128;
			}
			work->flags[i]++;// = FLAGS_NOP;
			break;
		  default:
			//printf("init (%d)\n",work->flags[i] & FLAGS_CHECK);
			work->flags[i] = FLAGS_NOP;
			break;
		}
		dvec+=8; wpos+=8;
		pos0+=8; pos1+=8;
		uvrgb+=8; uvrgbwh+=8;
#if 0
		if(0){//!i){
			pos1-=8;wpos-=8;
			for( j = 0; j < 8; j++ ){
				printf("[%d] <%d> pos1 vx %f: vy %f: vz %f\n",j,work->flags[i] & FLAGS_CHECK,pos1->vx,pos1->vy,pos1->vz);
				printf("[%d] <%d> wpos vx %f: vy %f: vz %f\n",j,work->flags[i] & FLAGS_CHECK,wpos->vx,wpos->vy,wpos->vz);
				pos1++;wpos++;
			}
			printf("\n");
		}
#endif
	}
	work->timer++;
	
	TS_Scr_Mem( work->prim0->pos[clock],MEM_SCR_POS0,sizeof(FVECTOR),N_VERTS0*N_PRIMS0);
	TS_Scr_Mem( work->prim1->pos[clock],MEM_SCR_POS1,sizeof(FVECTOR),N_VERTS1*N_PRIMS1);
	TS_Scr_Mem( work->prim0->uvrgb[clock],MEM_SCR_UV0,sizeof(DG_PRIM2_UVRGBWH),N_VERTS0*N_PRIMS0);
	TS_Scr_Mem( work->prim1->uvrgb[clock],MEM_SCR_UV1,sizeof(DG_PRIM2_UVRGB),N_VERTS1*N_PRIMS1);
	TS_Scr_Mem( work->pos,MEM_SCR_WPOS,sizeof(FVECTOR),N_POOS*N_PANS);
	TS_Scr_Mem( work->dvec,MEM_SCR_DVEC,sizeof(FVECTOR),N_POOS*N_PANS);
}

static void Die(Work *work)
{
	if(work->prim0) GM_FreePrim2(work->prim0);
	if(work->prim1) GM_FreePrim2(work->prim1);
}

static void InitWorkData( Work *work, DG_TEX *tex0, DG_TEX *tex1 )
{
	int					i,j;
	short				u0[2],v0[2];
	short				u1[2],v1[2];
	FVECTOR				*pos0,*dvec;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGB		*uvrgb;

	u0[0] = FTOI12( 0.0f * tex0->u_scale + tex0->u_offset );
	v0[0] = FTOI12( 0.0f * tex0->v_scale + tex0->v_offset );
	u0[1] = FTOI12( 1.0f * tex0->u_scale + tex0->u_offset );
	v0[1] = FTOI12( 1.0f * tex0->v_scale + tex0->v_offset );
	u1[0] = FTOI12( 0.0f * tex1->u_scale + tex1->u_offset );
	v1[0] = FTOI12( 0.0f * tex1->v_scale + tex1->v_offset );
	u1[1] = FTOI12( 1.0f * tex1->u_scale + tex1->u_offset );
	v1[1] = FTOI12( 1.0f * tex1->v_scale + tex1->v_offset );

	pos0 = MEM_SCR_POS0;
	dvec = MEM_SCR_DVEC;
	uvrgbwh = MEM_SCR_UV0;
	uvrgb = MEM_SCR_UV1;

	for( i = 0; i < N_POOS; i++ ){
		work->flags[i] = 0;
		work->wait[i] = i * 49;
		work->life[i] = 64;
		for( j = 0; j < 8; j++ ){
			DG_COPY_VEC(pos0,&DG_ZeroVector);
			DG_COPY_VEC(dvec,&DG_ZeroVector);

			uvrgb->u = u1[j&1];
			uvrgb->v = v1[j&1];
			uvrgb->q = 4094;
			uvrgb->f = (j)?DRAW_KICK_CODE:VERT_KICK_CODE;
	
			uvrgbwh->u0 = u0[0];
			uvrgbwh->v0 = v0[0];
			uvrgbwh->q0 = 4096;
			uvrgbwh->f0 = 0;
			uvrgbwh->u1 = u0[1];
			uvrgbwh->v1 = v0[1];
			uvrgbwh->q1 = 4096;
			uvrgbwh->f1 = 0;
			uvrgbwh->w = 0;//MAX_SIZE>>1;
			uvrgbwh->h = 0;//MAX_SIZE>>1;
			
			uvrgbwh->r = uvrgb->r = COLOR_R;
			uvrgbwh->g = uvrgb->g = COLOR_G;
			uvrgbwh->b = uvrgb->b = COLOR_B;
			uvrgbwh->a = 128;
			uvrgb->a = 128 - 16*j;
			pos0++;
			uvrgbwh++;
			uvrgb++;
		}
	}

	TS_Scr_Mem( work->prim0->pos[0],MEM_SCR_POS0,sizeof(FVECTOR),N_VERTS0*N_PRIMS0);
	TS_Scr_Mem( work->prim0->pos[1],MEM_SCR_POS0,sizeof(FVECTOR),N_VERTS0*N_PRIMS0);
	TS_Scr_Mem( work->prim0->uvrgb[0],MEM_SCR_UV0,sizeof(DG_PRIM2_UVRGBWH),N_VERTS0*N_PRIMS0);
	TS_Scr_Mem( work->prim0->uvrgb[1],MEM_SCR_UV0,sizeof(DG_PRIM2_UVRGBWH),N_VERTS0*N_PRIMS0);
	TS_Scr_Mem( work->prim1->pos[0],MEM_SCR_POS0,sizeof(FVECTOR),N_VERTS1*N_PRIMS1);
	TS_Scr_Mem( work->prim1->pos[1],MEM_SCR_POS0,sizeof(FVECTOR),N_VERTS1*N_PRIMS1);
	TS_Scr_Mem( work->prim1->uvrgb[0],MEM_SCR_UV1,sizeof(DG_PRIM2_UVRGB),N_VERTS1*N_PRIMS1);
	TS_Scr_Mem( work->prim1->uvrgb[1],MEM_SCR_UV1,sizeof(DG_PRIM2_UVRGB),N_VERTS1*N_PRIMS1);
	
	TS_Scr_Mem( work->dvec,MEM_SCR_DVEC,sizeof(FVECTOR),N_POOS*N_PANS);
	TS_Scr_Mem( work->pos,MEM_SCR_DVEC,sizeof(FVECTOR),N_POOS*N_PANS);
	
}


static int GetResources( Work *work, FMATRIX *world )
{
	DG_PRIM2	*prim = NULL;
	DG_TEX		*tex0 = NULL;
	DG_TEX		*tex1 = NULL;

//	tex = DG_GetTexture(GV_StrCode("blood_2bw_msk"));
//	tex0 = DG_GetTexture(GV_StrCode("drop01_msk"));
	tex0 = DG_GetTexture(12359931);
	if(!tex0){
		printf(" not texture int flour_Down.c \n");
		return(-1);
	}

	prim = work->prim0 = GM_MakePrim2(DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
									  N_PRIMS0, N_VERTS0 );
	if(!prim){
		printf("null prim\n");
		return (-1);
	}
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	DG_ConfigPrim2Tex( prim, tex0 );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x80 ) );

	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	
//	tex = DG_GetTexture(GV_StrCode("blood_2bw_msk"));
//	tex1 = DG_GetTexture(GV_StrCode("col128_alp"));
	tex1 = DG_GetTexture(8617636);
	if(!tex1){
		printf(" not texture int flour_Down.c \n");
		return(-1);
	}
	prim = work->prim1 = GM_MakePrim2(DG_PRIM2_LINE|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING,
									  N_PRIMS1, N_VERTS1 );
	if(!prim){
		printf("null prim\n");
		return (-1);
	}
	//DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 ) );
	DG_ConfigPrim2Tex( prim, tex1 );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 ) );
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	
	
	// DataInit
	work->timer = 0;
	work->world = world;
	
	InitWorkData( work, tex0, tex1 );

	GM_GroupPrim2( work->prim0, GM_CurrentStageMap ) ;
	GM_GroupPrim2( work->prim1, GM_CurrentStageMap ) ;

	return (0);
}

/*
	world: 発生マトリクス
*/

void *NewAbsentMind( FMATRIX *world )
{
	Work *work = NULL;
	
	work = (Work*)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die);
		GV_ActorEX( &work->actor );
		if(GetResources(work,world) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
//	printf( "SCR_BOTTOM = %p\n", MEM_SCR_BOTTOM );
	return (void *)work ;
}

//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   radio_comdl.c  
   ラジオの破片
   2000/05/18 T.Shibata

   $Id: radio_comdl.c,v 1.1.1.3 2002/11/19 11:48:47 Yoshizawa1 Exp $
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


#define		N_OBJS		(32)

#define		COLOR_R		(0x80)
#define		COLOR_G		(0x80)
#define		COLOR_B		(0x80)

#define		ALPHA		(0x80)
#define		SUB_ALPHA	(0x04)

#define		SPEED_SCALE		(48.0f)
#define		SPEED_MIN		( 1.0f)

#define		ANGLE_SCALE_X	(512)
#define		ANGLE_OFFSET_X	(ANGLE_SCALE_X/2)
#define		ANGLE_SCALE_Y	(1024)
#define		ANGLE_OFFSET_Y	(ANGLE_SCALE_Y/2)

#define		LIFE_TIME		(ALPHA)

#define		GRAVITATION		(-P_GRAVITY)
#define		RESISTANCE		(1.0f)//(0.88f)

#define		FRAGMENT_MODEL	(GV_StrCode( "rad_frg1" ))
//#define		FRAGMENT_MODEL	(GV_StrCode( "shl_frg2" ))
//#define		FRAGMENT_MODEL	(GV_StrCode( "shl_frg3" ))


#ifdef BP_PS2
#define		MEM_SCR_POS		((void *)(SCRPAD_ADDR))
#define		MEM_SCR_VEC		((void*)(MEM_SCR_POS + sizeof(FVECTOR)*N_OBJS))
#define		MEM_SCR_SCL		((void*)(MEM_SCR_VEC + sizeof(FVECTOR)*N_OBJS))
#else
#define		MEM_SCR_POS		((void *)(SCRPAD_ADDR))
#define		MEM_SCR_VEC		((void*)((char *)MEM_SCR_POS + sizeof(FVECTOR)*N_OBJS))
#define		MEM_SCR_SCL		((void*)((char *)MEM_SCR_VEC + sizeof(FVECTOR)*N_OBJS))
#endif

//extern
extern int  DG_QueueComdlObjs( DG_COMDL *comdl );
extern void DG_DequeueComdlObjs( DG_COMDL *comdl );

typedef struct
{
	GV_ACT_EX	 	actor ;
	DG_COMDL	*comdl ;

	int			timer;

	SVECTOR		rot[N_OBJS];
	
	FVECTOR		scale[N_OBJS];
	FVECTOR		dvec[N_OBJS];
	FVECTOR		pos[N_OBJS];
	
} Work ;

static   void  Act(Work *work)
{
	FVECTOR 		*pos,*dvec,*scl;
	DG_COMDL_POS 	*comdl_pos;
	SVECTOR			*rot;
	FMATRIX			fmtemp;
	int				i,cnt=0;

#if 0
	if( ++work->timer > LIFE_TIME ){
		GV_DestroyActor( work );
	}
#endif
	pos = MEM_SCR_POS;
	dvec = MEM_SCR_VEC;
	scl = MEM_SCR_SCL;
	rot = work->rot;//MEM_SCR_ROT;
	comdl_pos = work->comdl->pos;
	
	TS_Mem_Scr( MEM_SCR_POS, work->pos , sizeof(FVECTOR), N_OBJS );
	TS_Mem_Scr( MEM_SCR_VEC, work->dvec, sizeof(FVECTOR), N_OBJS );
	TS_Mem_Scr( MEM_SCR_SCL, work->scale , sizeof(FVECTOR), N_OBJS );

	for( i = 0; i < N_OBJS; i++ ){
		rot->vx += 40;
		rot->vy += 70;

		dvec->vx *= RESISTANCE;
		dvec->vy = ( dvec->vy - GRAVITATION ) * RESISTANCE;
		dvec->vz *= RESISTANCE;

		fpu_AddVectors(pos,pos,dvec);
		
		//AN_Test_Eye2( pos, 2 );
		DG_SetPos2( pos, rot );
		DG_GetPos( &fmtemp );
		TS_ScaleMatrix(&comdl_pos->world,&fmtemp,scl);
//		DG_GetPos( &comdl_pos->world );

		if(dvec->vy < 0.0f){
			if(comdl_pos->color.vw >= SUB_ALPHA){
				comdl_pos->color.vw -= SUB_ALPHA;
			}else{
				if(++cnt >= N_OBJS) GV_DestroyActor( work );
			}
		}

		scl++;
		pos++;
		dvec++;
		rot++;
		comdl_pos++;
	}
	
	TS_Scr_Mem( work->pos , MEM_SCR_POS, sizeof(FVECTOR), N_OBJS );
	TS_Scr_Mem( work->dvec, MEM_SCR_VEC, sizeof(FVECTOR), N_OBJS );
//	TS_Scr_Mem( work->scale , MEM_SCR_SCL, sizeof(FVECTOR), N_OBJS );
	
}

static   void  Die(Work *work)
{
	if(work->comdl){
		DG_DequeueComdlObjs( work->comdl );
		DG_FreeComdl( work->comdl );
	}
}

static void InitWorkData( Work *work, DG_COMDL *comdl, FVECTOR *center, FVECTOR *force )
{
	FVECTOR 		*pos,*dvec,*scl;
	DG_COMDL_POS 	*comdl_pos;
	SVECTOR			*rot;
	FMATRIX			fmtemp;
	SVECTOR			svtemp;
	int				i;


	//出る方向
	TS_MakeMatrix( &fmtemp, force, center );
	svtemp.vx = -512;
	svtemp.vy = 0;
	svtemp.vz = 0;

	DG_SetPos(&fmtemp);
	DG_RotatePos(&svtemp);
	DG_GetPos(&fmtemp);


#if 0
	printf("m[0] vx %2.6f:vy %2.6f:xz %2.6f\n"  , fmtemp.m[0][0], fmtemp.m[0][1], fmtemp.m[0][2]);
	printf("m[1] vx %2.6f:vy %2.6f:xz %2.6f\n"  , fmtemp.m[1][0], fmtemp.m[1][1], fmtemp.m[1][2]);
	printf("m[2] vx %2.6f:vy %2.6f:xz %2.6f\n\n", fmtemp.m[2][0], fmtemp.m[2][1], fmtemp.m[2][2]);
#endif
	//printf("comdl_pos size = %d\n",sizeof(DG_COMDL_POS));
	
	pos = MEM_SCR_POS;
	dvec = MEM_SCR_VEC;
	rot = work->rot;
	comdl_pos = comdl->pos;
	scl = MEM_SCR_SCL;
	
	//printf("before init &comdl_pos = %p\n",comdl_pos);
	//printf("before init &comdl = %p\n",comdl);

	for( i = 0; i < N_OBJS; i++ ){
		//init pos
		DG_COPY_VEC(pos,center);
		
		//init dvec
		dvec->vx = 0.0f;
		dvec->vy = 0.0f;
		dvec->vz = rnd()*SPEED_SCALE + SPEED_MIN;
		dvec->vw =  0.0f;

		svtemp.vx = (irnd()%ANGLE_SCALE_X)-ANGLE_OFFSET_X;
		svtemp.vy = (irnd()%ANGLE_SCALE_Y)-ANGLE_OFFSET_Y;
		svtemp.vz = 0;

		DG_SetPos(&fmtemp);
		DG_RotatePos(&svtemp);
		DG_RotVector(dvec,dvec,1);

		//init rot
		rot->vx = (short)(irnd());
		rot->vy = (short)(irnd());
		rot->vz = (short)(irnd());

		//init comdel
		DG_SetPos2( pos, rot ) ;
		DG_GetPos( &comdl_pos->world );
		
		// 整数型なので注意！
		comdl_pos->color.vx = COLOR_R;
		comdl_pos->color.vy = COLOR_G;
		comdl_pos->color.vz = COLOR_B;
		comdl_pos->color.vw = ALPHA;
#define	SCALE	(0.8f)
		scl->vx = SCALE;//rnd()/2;
		scl->vy = SCALE;//rnd()/2;
		scl->vz = SCALE;//rnd()/2;
		
		pos++;
		dvec++;
		scl++;
		rot++;
		comdl_pos++;
	}

    // スクラッチパッドからメインメモリへ転送	
	TS_Scr_Mem( work->pos , MEM_SCR_POS, sizeof(FVECTOR), N_OBJS );
	TS_Scr_Mem( work->dvec, MEM_SCR_VEC, sizeof(FVECTOR), N_OBJS );
	TS_Scr_Mem( work->scale , MEM_SCR_SCL, sizeof(FVECTOR), N_OBJS );

}
static   int  GetResources( Work *work, FVECTOR *center, FVECTOR *force )
{	
	DG_DEF		*def ;
	DG_COMDL	*comdl ;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( FRAGMENT_MODEL, 'k' ) ) ;

	comdl = work->comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, N_OBJS, 0 );
	if( !comdl ) return -1;
		
	DG_QueueComdlObjs( comdl );
	
	// DataInit
	work->timer = 0;

	InitWorkData( work, comdl , center, force);

	return (0);
}


void *NewRadio_Comdl( FVECTOR *center ,FVECTOR *force )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		if(GetResources( work, center, force ) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;

}

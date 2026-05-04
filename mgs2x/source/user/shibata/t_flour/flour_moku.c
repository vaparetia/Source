//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

   メリケン粉 サラサラが床に当たってでる奴
	2000/04/26 T.Shibata

	$Id: flour_moku.c,v 1.1.1.3 2002/11/19 11:48:51 Yoshizawa1 Exp $

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

#define		N_PRIMS		(8)
#define		N_VERTS		(12)

#define		COLOR_R		(0x80)
#define		COLOR_G		(0x80)
#define		COLOR_B		(0x80)
#define		ALPHA		(0x20)

#define		LIFE_TIME 		(24 * 4 + 44)

#define		SIZE			(64.0f)
#define		SPEED			(8.0f)
#define		WAIT_TIME		(8)
#define		TAME			(16)

#define		FLAGS_INIT		(0x0001)

#ifdef BP_PS2
#define		MEM_SCR0	((void *)( SCRPAD_ADDR))
#define		MEM_SCR1	((void *)( MEM_SCR0 ) + sizeof(FVECTOR)*N_PRIMS*N_VERTS)
#define		MEM_SCR2	((void *)( MEM_SCR1 ) + sizeof(DG_PRIM2_UVRGBWH)*N_PRIMS*N_VERTS)
#define		MEM_SCR3	((void *)( MEM_SCR2 ) + sizeof(FVECTOR)*N_PRIMS*N_VERTS)
#define		MEM_SCR4	((void *)( MEM_SCR3 ) + sizeof(DG_PRIM2_UVRGBWH)*N_PRIMS*N_VERTS)
#else
#define		MEM_SCR0	((void *)( SCRPAD_ADDR))
#define		MEM_SCR1	((void *)( (char *)MEM_SCR0 + sizeof(FVECTOR)*N_PRIMS*N_VERTS) )
#define		MEM_SCR2	((void *)( (char *)MEM_SCR1 + sizeof(DG_PRIM2_UVRGBWH)*N_PRIMS*N_VERTS) )
#define		MEM_SCR3	((void *)( (char *)MEM_SCR2 + sizeof(FVECTOR)*N_PRIMS*N_VERTS) )
#define		MEM_SCR4	((void *)( (char *)MEM_SCR3 + sizeof(DG_PRIM2_UVRGBWH)*N_PRIMS*N_VERTS) )
#endif

/* extern */
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
extern  void  _BigScrCopy( void *dat, void *src, int size, int num ) ;
extern  void  _BigMemCopy( void *dst, void *src, int size, int num ) ;

//static int TS_flour_counter = 0;

typedef struct
{
	GV_ACT_EX		actor ;
	DG_PRIM2	*prim;
	
	int			timer;

	FVECTOR		center;
	FVECTOR		dvec[N_PRIMS*N_VERTS];
	short		flags[N_PRIMS*N_VERTS];
	
} Work ;


static void Act(Work *work)
{
	int					i,j,clock;
	FVECTOR 			*pos,*dvec,*before_pos;
	DG_PRIM2_UVRGBWH 	*uvrgbwh,*b_uvrgbwh;
	float				speed,angle;
	short				*flags;


	if( ++work->timer > LIFE_TIME ){
		DG_InvisiblePrim2(work->prim) ;
		
		GV_DestroyActor( work );
	}

	if( work->timer < TAME )return;
	
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	pos = MEM_SCR0;
	uvrgbwh = MEM_SCR1;
	before_pos = MEM_SCR2;
	b_uvrgbwh = MEM_SCR3;
	dvec = MEM_SCR4;
	flags = work->flags;
	
	_BigScrCopy( MEM_SCR1,work->prim->uvrgb[clock], sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS );
	_BigScrCopy( MEM_SCR2,work->prim->pos[1-clock], sizeof(FVECTOR), N_PRIMS*N_VERTS );
	_BigScrCopy( MEM_SCR3,work->prim->uvrgb[1-clock], sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS );
	_BigScrCopy( MEM_SCR4,work->dvec, sizeof(FVECTOR), N_PRIMS*N_VERTS );

	for( i = 0; i < N_PRIMS; i++ ){
		if( i*WAIT_TIME >= work->timer )break;
		//printf("[%d]",i);
		for( j = 0; j < N_VERTS; j++ ){
			if(uvrgbwh->a){
				_sceVu0AddVector( pos, before_pos, dvec);
				dvec->vy += 0.25f;
				uvrgbwh->a -= 2;
			}
			if(!uvrgbwh->a){
				if(*flags & FLAGS_INIT){
					uvrgbwh->a = ALPHA;
					_sceVu0AddVector( pos, before_pos, dvec);
					dvec->vy += 0.25f;
					uvrgbwh->a -= 2;
					*flags &= ~(FLAGS_INIT);
				}else{
					if(LIFE_TIME - work->timer > ALPHA){
						angle = rnd()*2*PI;
						speed = 12.0f*rnd();
						DG_COPY_VEC(pos,&work->center);
						dvec->vx = speed * cosf(angle);
						dvec->vy = 0.0f;
						dvec->vz = speed * sinf(angle);
						*flags |= FLAGS_INIT;
						uvrgbwh->a = ALPHA;
					}
				}
			}
			pos++;
			before_pos++;
			uvrgbwh++;
			b_uvrgbwh++;
			dvec++;
			flags++;
		}
	}
	//printf("\n");
	_BigMemCopy( work->prim->pos[clock], MEM_SCR0, sizeof(FVECTOR), i * N_VERTS );
	_BigMemCopy( work->prim->uvrgb[clock], MEM_SCR1, sizeof(DG_PRIM2_UVRGBWH), i * N_VERTS );
	_BigMemCopy( work->dvec, MEM_SCR4, sizeof(FVECTOR), i * N_VERTS );

}

static void Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim);
}

static void InitWorkData( Work *work, DG_TEX *tex )
{
	int 					i,j;
	FVECTOR					*pos;
	FVECTOR		  			*dvec;
	DG_PRIM2_UVRGBWH		*uvrgbwh;
	short					u0,v0,u1,v1,*flags;
	
	pos = MEM_SCR0;
	uvrgbwh = MEM_SCR1;
	dvec = MEM_SCR2;
	flags = work->flags;

	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	for(i = 0; i < N_PRIMS; i++ ){
		for( j = 0; j < N_VERTS; j++ ){
			*flags = 0;
			DG_COPY_VEC(pos,&work->center);

			dvec->vx = 0.0f;
			dvec->vy = 0.0f;
			dvec->vz = 0.0f;

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
			uvrgbwh->a = 0;//ALPHA;
			
			uvrgbwh->w = 0;
			uvrgbwh->h = SIZE;
			
			pos++;
			dvec++;
			uvrgbwh++;
			flags++;
		}
	}

	_BigMemCopy( work->prim->pos[0],MEM_SCR0,sizeof(FVECTOR),N_PRIMS*N_VERTS);
	_BigMemCopy( work->prim->uvrgb[0],MEM_SCR1,sizeof(DG_PRIM2_UVRGBWH),N_PRIMS*N_VERTS);
	_BigMemCopy( work->prim->pos[1],MEM_SCR0,sizeof(FVECTOR),N_PRIMS*N_VERTS);
	_BigMemCopy( work->prim->uvrgb[1],MEM_SCR1,sizeof(DG_PRIM2_UVRGBWH),N_PRIMS*N_VERTS);
	_BigMemCopy( work->dvec,MEM_SCR3,sizeof(FVECTOR),N_PRIMS*N_VERTS);

}


static int GetResources( Work *work, FVECTOR *center )
{
	DG_PRIM2	*prim = NULL;
	DG_TEX		*tex = NULL;

	//tex = DG_GetTexture(GV_StrCode("powder04_alp"));
	tex = DG_GetTexture(12095646);
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
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	/* まだチャンネル１～３は無いが取りあえず フラグ立て */
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	
	/* DataInit */
	work->timer = 0;
	DG_COPY_VEC(&work->center,center);

	InitWorkData( work, tex );

	
	
	return (0);
}

/*
    map:
    name:
	center:	中心
*/

void *NewFlour_Moku( FVECTOR *center )
{
	Work *work = NULL;

	work = (Work*)GV_NewActor(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(GetResources(work,center) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

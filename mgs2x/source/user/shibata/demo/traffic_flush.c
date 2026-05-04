//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    traffic_flush.c
	交通の光（移動する奴）
	2000/05/31 T.Shibata

	$Id: traffic_flush.c,v 1.1.1.3 2002/11/19 11:48:35 Yoshizawa1 Exp $
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

//#include	"def_dma.h"
//#include	"utl_dma.h"

#include	"../util/ts_util.h"

#define		N_PRIMS	 	(1)
#define		N_VERTS		(16)

#define		COLOR_R	(0x80)
#define		COLOR_G	(0x80)
#define		COLOR_B	(0x80)
#define		ALPHA	(0x10)

#define 	MAX_ALPHA	(64)
#define		SUB_ALPHA	(1)

#ifdef BP_PS2
#define		MEM_SCR_POS		((void *)(SCRPAD_ADDR))
#define		MEM_SCR_UV		((void *)(MEM_SCR_POS + sizeof(FVECTOR) * N_PRIMS * N_VERTS))
#define		MEM_SCR_WPOS	((void *)(MEM_SCR_UV + sizeof(DG_PRIM2_UVRGBWH) * N_PRIMS * N_VERTS))
#else
#define		MEM_SCR_POS		((void *)(SCRPAD_ADDR))
#define		MEM_SCR_UV		((void *)((char *)MEM_SCR_POS + sizeof(FVECTOR) * N_PRIMS * N_VERTS))
#define		MEM_SCR_WPOS	((void *)((char *)MEM_SCR_UV + sizeof(DG_PRIM2_UVRGBWH) * N_PRIMS * N_VERTS))
#endif

// extern

//----------------------------------------------------
//		呼出の方
//----------------------------------------------------

typedef struct
{
	GV_ACT_EX			actor;
	DG_PRIM2			*prim;
	FVECTOR				*m_pos;
	float				offset_z;
	int					timer;
	int					life;
	float 				k;
} Work ;


static void Act(Work *work)
{
	FVECTOR				*pos,*b_pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh,*b_uvrgbwh;
	int 				clock,cnt;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;
	
	pos = work->prim->pos[clock];
	b_pos = work->prim->pos[1-clock];
	uvrgbwh = work->prim->uvrgb[clock];
	b_uvrgbwh = work->prim->uvrgb[1-clock];

	if( --work->timer < 16 ){
		if( work->timer < 0 ){
			GV_CallParentSignalFunc( work, 0, 0 );
			DG_InvisiblePrim2(work->prim) ;
			GV_DestroyActor( work );
			return;
		}
		memmove( uvrgbwh, &b_uvrgbwh[1], sizeof(DG_PRIM2_UVRGBWH) * (N_VERTS-1) );
		uvrgbwh += N_VERTS-1;
		uvrgbwh->w = 0;
		uvrgbwh->h = 0;
		uvrgbwh->a = 0;
	}else{
		if( (cnt = work->life - work->timer) < 16 ){
			memmove( &uvrgbwh[1], b_uvrgbwh, sizeof(DG_PRIM2_UVRGBWH) * (N_VERTS-1) );
			uvrgbwh->a = (short)((float)cnt * work->k );
		}
	
	}
	memmove( &pos[1], b_pos, sizeof(FVECTOR) * (N_VERTS-1) );
	DG_COPY_VEC(pos,work->m_pos);
	pos->vz += work->offset_z;
	//	AN_Test_Eye2( pos, 1);
}

static void Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim);
		//GV_CallParentSignalFunc( work, 0, 0 );//printf("kill\n");
}

static void InitWorkData( Work *work, DG_TEX *tex, FVECTOR *m_pos, CVECTOR *rgba )
{
	
	int					j;
	short 				u0,v0,u1,v1;
	FVECTOR				*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	
	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	work->offset_z = 800.0f * frnd();
	
	pos = MEM_SCR_POS;
	uvrgbwh = MEM_SCR_UV;
		
	for( j = 0; j < N_VERTS; j++ ){		
		DG_COPY_VEC(pos,m_pos);
		pos->vz += work->offset_z;
		
		uvrgbwh->u0 = u0;
		uvrgbwh->v0 = v0;
		uvrgbwh->u1 = u1;
		uvrgbwh->v1 = v1;
		uvrgbwh->r = rgba->r;
		uvrgbwh->g = rgba->g;
		uvrgbwh->b = rgba->b;
		uvrgbwh->a = 0;//MAX_ALPHA - j*4;

		uvrgbwh->q0 = 4096;
		uvrgbwh->q1 = 4096;
		uvrgbwh->f0 = 0;
		uvrgbwh->f1 = 0;

		uvrgbwh->w = uvrgbwh->h = 2400 + (int)1200*rnd();
		
		pos++;
		uvrgbwh++;
	}

	TS_Scr_Mem( work->prim->pos[0],MEM_SCR_POS,sizeof(FVECTOR),N_VERTS*N_PRIMS);
	TS_Scr_Mem( work->prim->pos[1],MEM_SCR_POS,sizeof(FVECTOR),N_VERTS*N_PRIMS);
	TS_Scr_Mem( work->prim->uvrgb[0],MEM_SCR_UV,sizeof(DG_PRIM2_UVRGBWH),N_VERTS*N_PRIMS);
	TS_Scr_Mem( work->prim->uvrgb[1],MEM_SCR_UV,sizeof(DG_PRIM2_UVRGBWH),N_VERTS*N_PRIMS);
//	TS_Scr_Mem( work->pos,MEM_SCR_WPOS,sizeof(FVECTOR),N_PRIMS);
	
}

static int ReceiveSignal( void *pwork, int signal, int value )
{
	Work *work = pwork;
	int	ret = 0;
	switch(signal){
	  case 0:
		if(work->prim)DG_VisiblePrim2(work->prim) ;
		break;
	  case 1:
		if(work->prim)DG_InvisiblePrim2(work->prim) ;
		break;
	  default:
		ret = GV_DefaultSignalFunc( pwork, signal, value );
		break;
	}
	return ret;
}

static int GetResources( Work *work, FVECTOR *m_pos, CVECTOR *rgba )
{
	DG_PRIM2	*prim = NULL;
	DG_TEX		*tex = NULL;

	tex = DG_GetTexture(3594043);//(GV_StrCode("drop01_msk"));
	if(!tex){
		printf(" not texture \n");
		return(-1);
	}
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
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
	work->timer = work->life = 64 + irnd()%32;
	work->m_pos = m_pos;
	work->k = (float)rgba->cd/6.0f;
	if( work->k < 0.0f ) work->k = 0.0f;
	InitWorkData( work, tex, m_pos, rgba );

	GV_SetActorSignalFunc( work, ReceiveSignal );

//	printf("bottom = %x\n",MEM_SCR_WPOS);
	return (0);
}

/*
    *m_pos:
*/

void *NewTraffic_Flush( FVECTOR *m_pos, CVECTOR *rgba )
{
	Work *work = NULL;
	
	work = (Work*)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor )
		if(GetResources( work, m_pos, rgba ) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}

	return (void *)work ;
}

//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

   メリケン粉 ガス（カメラ透視変換）
	2000/04/26 T.Shibata

	$Id: flour_gas3.c,v 1.1.1.3 2002/11/19 11:48:51 Yoshizawa1 Exp $

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

#define		N_PRIMS		(1)
//#define		N_VERTS		(24)
#define		N_VERTS		(4)

/* 初期化の値 */
#define		COLOR_R		(255)
#define		COLOR_G		(255)
#define		COLOR_B		(255)
#define		ALPHA		(0x00)

#define		MAX_ALPHA	(32.0f)
#define		SUB_ALPHA	(1.0f)
#define		SUB_ALPHA1	(1.0f)
#define		ADD_ALPHA	(1.0f)

#define		MAX_SPEED		(8.0f)
#define		MIN_SPEED		(6.0f)

#define		MAX_SIZE_W		(128.0f)
#define		MAX_SIZE_H		(MAX_SIZE_W)

#define		SCREEN_Z		(51.0f)

#define		BOUND_R			(64.0f)
#define		BOUND_W			(2000.0f)
#define		BOUND_H			(1500.0f)


#define		WAIT			(32)
#define		LIFE_TIME 		(420+WAIT)
#define		KILL_WAIT0		(LIFE_TIME-256)
#define		KILL_WAIT1		(420+WAIT)

#ifdef BP_PS2
#define		MEM_SCR_POS		((void *)( SCRPAD_ADDR))
#define		MEM_SCR_UV		((void *)( MEM_SCR_POS ) + sizeof(FVECTOR)*N_PRIMS*N_VERTS)
#define		MEM_SCR_WPOS	((void *)( MEM_SCR_UV ) + sizeof(DG_PRIM2_UVRGBWH)*N_PRIMS*N_VERTS)
#define		MEM_SCR_DVEC	((void *)( MEM_SCR_WPOS ) + sizeof(FVECTOR)*N_PRIMS*N_VERTS)
#else
#define		MEM_SCR_POS		((void *)( SCRPAD_ADDR))
#define		MEM_SCR_UV		((void *)( (char *)MEM_SCR_POS + sizeof(FVECTOR)*N_PRIMS*N_VERTS) )
#define		MEM_SCR_WPOS	((void *)( (char *)MEM_SCR_UV + sizeof(DG_PRIM2_UVRGBWH)*N_PRIMS*N_VERTS) )
#define		MEM_SCR_DVEC	((void *)( (char *)MEM_SCR_WPOS + sizeof(FVECTOR)*N_PRIMS*N_VERTS) )
#endif

/* extern */
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
extern  void  _BigScrCopy( void *dat, void *src, int size, int num ) ;
extern  void  _BigMemCopy( void *dst, void *src, int size, int num ) ;

typedef struct
{
	GV_ACT_EX		actor ;
	DG_PRIM2	*prim;
	DG_TEX		*tex;
	
	int			timer;
	
	FVECTOR		bound_0;
	FVECTOR		bound_1;
	
	float		alpha[N_PRIMS*N_VERTS];		/* アルファ */
	FVECTOR		center;						/* 発生ポイント */
	FVECTOR		dvec[N_PRIMS*N_VERTS];		/* 移動ベクトル */
	FVECTOR		w_pos[N_PRIMS*N_VERTS];		/* ワーク位置 */
	
} Work ;

static Work *Gas_Work = NULL;

static void Act(Work *work)
{
	int					i,j,clock;
	FVECTOR 			*pos,*dvec,*w_pos;
	DG_PRIM2_UVRGBWH 	*uvrgbwh;
	float				screen_near_x;
	float				screen_near_y;
	FVECTOR				bound_0,bound_1;
	FVECTOR				fvtemp;
	float				len;//,scale,angle_a,angle_b;
	float				*alpha;

	if( ++work->timer > LIFE_TIME ){
		//printf("kill\n");
		DG_InvisiblePrim2(work->prim) ;
		
		GV_DestroyActor( work );
	}

	DG_COPY_VEC(&bound_0,&work->bound_0);
	DG_COPY_VEC(&bound_1,&work->bound_1);
	if( vu0_CheckBoundingBox( &GM_PlayerPosition, &bound_0, &bound_1 ) ){
		GM_SetPlayerStatusEX( I64(0), PLAYER2_POWDER ) ;
	}



	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	pos = MEM_SCR_POS;
	w_pos = MEM_SCR_WPOS;
	dvec = MEM_SCR_DVEC;
	uvrgbwh = MEM_SCR_UV;
	alpha = work->alpha;

	_BigScrCopy( MEM_SCR_WPOS,work->w_pos, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	_BigScrCopy( MEM_SCR_DVEC,work->dvec, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	_BigScrCopy( MEM_SCR_UV,work->prim->uvrgb[clock], sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS );

	screen_near_x = SCREEN_Z / (ASPECT_X()                                           ) / DG_Chanls[0].screen;
	screen_near_y = SCREEN_Z / (ASPECT_Y() * DG_Chanls[0].width / DG_Chanls[0].height) / DG_Chanls[0].screen;

	// 発生ポイントからカメラまでの距離
	fpu_SubVectors( &fvtemp,(FVECTOR*)&(DG_Chanls[0].eye.m[3]),&work->center );
	len = fpu_VectorLength2(&fvtemp);
	len = 1.0f - fpu_Sqrt(len)/14000.0f;
	if(len < 0.0f)len = 0.0f;

	for( i = 0; i < N_PRIMS; i++ ){
		for( j = 0; j < N_VERTS; j++ ){
			if( work->timer > WAIT ){
				if( work->timer > KILL_WAIT0 ) {
					*alpha -= SUB_ALPHA1;
					if(*alpha < 0.0f ) *alpha = 0.0f;
				}else{
					// バンダリチェック
					if( vu0_CheckBoundingBox( w_pos, &bound_0, &bound_1 ) ){
						// バンダリ内
						if( *alpha < MAX_ALPHA ) *alpha += ADD_ALPHA;
					} else {
						if(*alpha > SUB_ALPHA ){
							*alpha -= SUB_ALPHA;	
						} else {
							w_pos->vx = frnd() * BOUND_W;
							w_pos->vy = frnd() * BOUND_H;
							w_pos->vz = frnd() * BOUND_W;
							fpu_AddVectors( w_pos,w_pos,&work->center);
							*alpha = 0.0f;
						}
					}
				}
			}

			fpu_AddVectors( w_pos,w_pos,dvec);
//			AN_Test_Eye2( w_pos, 2 );

			pos++;
			uvrgbwh++;
			dvec++;
			w_pos++;
			alpha++;
		}
	}

	// 視点透視変換
	DG_SetPos( &DG_Chanls[0].eye_pers );
	DG_PutVector( MEM_SCR_WPOS, MEM_SCR_POS, N_VERTS * N_PRIMS );

	pos = MEM_SCR_POS;
//	w_pos = MEM_SCR_WPOS;
	dvec = MEM_SCR_DVEC;
	uvrgbwh = MEM_SCR_UV;
	alpha = work->alpha;

	for( i = 0; i < N_PRIMS; i++ ){
		for( j = 0; j < N_VERTS; j++ ){
			if( pos->vz < 2.0f * pos->vw ){
				pos->vw = fpu_Abs(pos->vw);
				uvrgbwh->a = (u_short)(*alpha * len);
				uvrgbwh->h = uvrgbwh->w = 72;
				pos->vz = SCREEN_Z;
				pos->vx = screen_near_x * pos->vx / pos->vw;
				pos->vy = screen_near_y * pos->vy / pos->vw;


			}else{
				uvrgbwh->h = uvrgbwh->w = 0 ;
				uvrgbwh->a = 0;
				DG_COPY_VEC( pos, &DG_ZeroVector );
			}
			pos++;
			uvrgbwh++;
			dvec++;
	//		w_pos++;
			alpha++;
		}
	}

	_BigMemCopy( work->prim->pos[clock], MEM_SCR_POS, sizeof(FVECTOR), N_PRIMS * N_VERTS );
	_BigMemCopy( work->prim->uvrgb[clock], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
	_BigMemCopy( work->dvec, MEM_SCR_DVEC, sizeof(FVECTOR), N_PRIMS * N_VERTS );
	_BigMemCopy( work->w_pos, MEM_SCR_WPOS, sizeof(FVECTOR), N_PRIMS * N_VERTS );

}

static void Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim);
	Gas_Work = NULL;
}


static void InitWorkData( Work *work, DG_TEX *tex )
{
	int 					i,j,u_index,v_index;
	FVECTOR		  			*dvec,*pos;
//	FVECTOR					fvtemp;
	DG_PRIM2_UVRGBWH		*uvrgbwh;
	short					u[2],v[2];
	float					angle_a,angle_b,scale;
	
	pos = MEM_SCR_POS;
	uvrgbwh = MEM_SCR_UV;
	dvec = MEM_SCR_DVEC;

	u[0] = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v[0] = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u[1] = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v[1] = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	work->bound_0.vx = BOUND_W;
	work->bound_0.vy = BOUND_H;
	work->bound_0.vz = BOUND_W;
	work->bound_1.vx = -BOUND_W;
	work->bound_1.vy = -BOUND_H;
	work->bound_1.vz = -BOUND_W;

	fpu_AddVectors( &work->bound_0,&work->center,&work->bound_0);
	fpu_AddVectors( &work->bound_1,&work->center,&work->bound_1);

	for(i = 0; i < N_PRIMS; i++ ){
		for( j = 0; j < N_VERTS; j++ ){
			angle_b = 2*PI*rnd();
			angle_a = 2*PI*rnd();
			scale = rnd() * MAX_SPEED + MIN_SPEED;

			dvec->vx = scale * cosf(angle_a) * cosf(angle_b);
			dvec->vy = scale * sinf(angle_a);
			dvec->vz = scale * cosf(angle_a) * sinf(angle_b);

			pos->vx = frnd() * BOUND_W;
			pos->vy = frnd() * BOUND_H;
			pos->vz = frnd() * BOUND_W;

			fpu_AddVectors( pos, &work->center, pos );

			u_index = irnd()%2;
			v_index = irnd()%2;

			uvrgbwh->u0 = u[u_index];
			uvrgbwh->v0 = v[v_index];
			uvrgbwh->u1 = u[1-u_index];
			uvrgbwh->v1 = v[1-v_index];

			uvrgbwh->q0 = 4096;
			uvrgbwh->q1 = 4096;
			uvrgbwh->f0 = 0x0fff;
			uvrgbwh->f1 = 0x0fff;

			uvrgbwh->r = COLOR_R;
			uvrgbwh->g = COLOR_G;
			uvrgbwh->b = COLOR_B;

			uvrgbwh->w = 0;
			uvrgbwh->h = 0;

			work->alpha[ i*N_VERTS+j ] = ADD_ALPHA * (j%8);
			uvrgbwh->a = ADD_ALPHA * (j%8);//ALPHA;

			pos++;
			dvec++;
			uvrgbwh++;

		}
	}

	_BigMemCopy( work->w_pos,MEM_SCR_POS,sizeof(FVECTOR),N_PRIMS*N_VERTS);
	_BigMemCopy( work->prim->pos[0],MEM_SCR_POS,sizeof(FVECTOR),N_PRIMS*N_VERTS);
	_BigMemCopy( work->prim->uvrgb[0],MEM_SCR_UV,sizeof(DG_PRIM2_UVRGBWH),N_PRIMS*N_VERTS);
	_BigMemCopy( work->prim->pos[1],MEM_SCR_POS,sizeof(FVECTOR),N_PRIMS*N_VERTS);
	_BigMemCopy( work->prim->uvrgb[1],MEM_SCR_UV,sizeof(DG_PRIM2_UVRGBWH),N_PRIMS*N_VERTS);
	_BigMemCopy( work->dvec,MEM_SCR_DVEC,sizeof(FVECTOR),N_PRIMS*N_VERTS);

}


static int GetResources( Work *work, FVECTOR *center )
{
	DG_PRIM2	*prim = NULL;
	DG_TEX		*tex = NULL;

	//tex = work->tex = DG_GetTexture(GV_StrCode("powder01_alp"));
	tex = DG_GetTexture(8949918);
	
	if(!tex){
		printf(" not texture int flour_Down.c \n");
		return(-1);
	}

	prim = work->prim = GM_MakePrim2(DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA,
									  N_PRIMS,
									  N_VERTS );
	if(!prim){
		printf("null prim\n");
		return (-1);
	}

	DG_ConfigPrim2Tex( prim, tex );

	prim->raise = 4095;
	

//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	/* まだチャンネル１～３は無いが取りあえず フラグ立て */
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	
	/* DataInit */
	work->timer = 0;

	DG_COPY_VEC(&work->center,center);

	InitWorkData( work, tex );
	Gas_Work = work;
//printf("ahoaho man\n");
	return (0);
}

/*
	center:	中心
*/

void *NewFlour_Gas3( FVECTOR *center )
{
	Work *work = NULL;
	
	if(Gas_Work){
		//printf("reset_gas\n");
		Gas_Work->timer = 0;
		DG_COPY_VEC(&Gas_Work->center,center);
		Gas_Work->bound_0.vx = BOUND_W;
		Gas_Work->bound_0.vy = BOUND_H;
		Gas_Work->bound_0.vz = BOUND_W;
		Gas_Work->bound_1.vx = -BOUND_W;
		Gas_Work->bound_1.vy = -BOUND_H;
		Gas_Work->bound_1.vz = -BOUND_W;

		fpu_AddVectors( &Gas_Work->bound_0,center,&Gas_Work->bound_0);
		fpu_AddVectors( &Gas_Work->bound_1,center,&Gas_Work->bound_1);
		
		return (Gas_Work);
	}
	work = (Work*)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
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

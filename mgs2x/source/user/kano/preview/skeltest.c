//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	skeltest.c
	バウンディング表示デバッグキャラ

	1999/11/02 K.Takabe
	$Id: skeltest.c,v 1.1.1.3 2002/11/19 11:43:31 Yoshizawa1 Exp $

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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"

#include	"libutl.h"


/* ---------------------------------------------------------------- */
/*
  補助マクロ
  */

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define _RND(n)	( ( (BP_PS2_rand()>>16) * n ) >> 15 )

/* ---------------------------------------------------------------- */
/*
  プログラム使用定数の定義
  */

#define N_PRIMS	(20)
#define N_PACKETS	(1)

/* ---------------------------------------------------------------- */
/*
  プログラムワーク
  */

typedef	struct	{
    GV_ACT_EX	actor ;

    DG_PRIM2 *prim ;
    FVECTOR	*pos;
} Work ;


/* ---------------------------------------------------------------- */
/*
  プログラムメイン処理
  */
static void Act( Work *work )
{
    GV_DestroyActor( work );
}

static void Die( Work *work )
{
    if ( work->prim != NULL ){
		DG_DequeuePrim2( work->prim );
		DG_FreePrim2( work->prim );
    }
}



static void PutPos(FVECTOR *pos,DG_PRIM2_UVRGB *uvrgb,
				   FVECTOR *p,FVECTOR *q,FVECTOR *x,FVECTOR *y,FVECTOR *z,int color)
{
	int k;

	fpu_CopyVector(&(pos[0]),p);
	pos[0].vw=1.0f;
	fpu_AddVectors(&(pos[1]),q,x);
	pos[1].vw=1.0f;
	fpu_CopyVector(&(pos[2]),p);
	pos[2].vw=1.0f;
	fpu_AddVectors(&(pos[3]),q,y);
	pos[3].vw=1.0f;
	fpu_CopyVector(&(pos[4]),p);
	pos[4].vw=1.0f;
	fpu_SubVectors(&(pos[5]),q,x);
	pos[5].vw=1.0f;
	fpu_CopyVector(&(pos[6]),p);
	pos[6].vw=1.0f;
	fpu_SubVectors(&(pos[7]),q,y);
	pos[7].vw=1.0f;

	fpu_SubVectors(&(pos[8]),q,z);
	pos[8].vw=1.0f;
	fpu_AddVectors(&(pos[9]),q,x);
	pos[9].vw=1.0f;
	fpu_SubVectors(&(pos[10]),q,z);
	pos[10].vw=1.0f;
	fpu_AddVectors(&(pos[11]),q,y);
	pos[11].vw=1.0f;
	fpu_SubVectors(&(pos[12]),q,z);
	pos[12].vw=1.0f;
	fpu_SubVectors(&(pos[13]),q,x);
	pos[13].vw=1.0f;
	fpu_SubVectors(&(pos[14]),q,z);
	pos[14].vw=1.0f;
	fpu_SubVectors(&(pos[15]),q,y);
	pos[15].vw=1.0f;

	fpu_AddVectors(&(pos[16]),q,x);
	pos[16].vw=1.0f;
	fpu_AddVectors(&(pos[17]),q,y);
	pos[17].vw=1.0f;
	fpu_AddVectors(&(pos[18]),q,y);
	pos[18].vw=1.0f;
	fpu_SubVectors(&(pos[19]),q,x);
	pos[19].vw=1.0f;
	fpu_SubVectors(&(pos[20]),q,x);
	pos[20].vw=1.0f;
	fpu_SubVectors(&(pos[21]),q,y);
	pos[21].vw=1.0f;
	fpu_SubVectors(&(pos[22]),q,y);
	pos[22].vw=1.0f;
	fpu_AddVectors(&(pos[23]),q,x);
	pos[23].vw=1.0f;

	for(k=0;k<24;k++){
#if 1
		if(k & 1) uvrgb[k].f = 0x0fff ;
		else uvrgb[k].f = 0x8fff ;
#else
		uvrgb[k].f = 0x0fff ;
#endif
		uvrgb[k].q = 4096 ;

		uvrgb[k].r = ( color ) & 255 ;
		uvrgb[k].g = ( color >> 8 ) & 255 ;
		uvrgb[k].b = ( color >> 16 ) & 255 ;
		uvrgb[k].a = 128 ;/* アンチエイリアスを使用するときには１２８にする */
	}
}



#define BONE_LEN		50.0f

static int KmsSetLine(Work *work,DG_OBJS *objs,int color)
{
	DG_MDL *mdl;
    DG_PRIM2		*prim;
    DG_PRIM2_UVRGB	*uvrgb;
	FVECTOR *pos;
	int n_models;
    int		i;
	FVECTOR world_v[128];

	n_models=objs->def->n_models;
	mdl=objs->def->models;

    prim = work->prim = 
		GM_MakePrim2(DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_ANTIALIASING|DG_PRIM2_SINGLEBUFF,
					 n_models,24);
    if(prim==NULL) return 0;


    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
    //prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) ;
    fpu_CopyUnitMatrix(&(prim->as_world));
    prim->group_id = 0x7fffffff ;

    for (i=0;i<n_models;i++){
		fpu_CopyVector(&(world_v[i]),(FVECTOR *)&(objs->objs[i].world.m[3][0]));
	}

    for (i=0;i<n_models;i++){
		FVECTOR x,y,z,p;
		int parent;


		parent=(mdl+i)->parent;

		if(parent!=-1){
			fpu_CopyVector(&p,&(world_v[parent]));
		}
		else{
			fpu_MulVectorScaler(&p,(FVECTOR *)&(objs->objs[i].world.m[2][0]),BONE_LEN);
			fpu_AddVectors(&p,&(world_v[i]),&p);
			p.vw=1.0f;
		}

		fpu_SubVectors(&z,&p,&(world_v[i]));
#if 1
		fpu_VectorNormal(&z);
#else
		{
			float l=fpu_VectorLength2(&z);
			l=fpu_Rsqrt(l,1.0f);
			fpu_MulVectorScaler(&z,&z,l);

			// printf("%f\n",l);
			//printf("%f %f %f %f\n",z.vx,z.vy,z.vz,z.vw);
		}
#endif

		fpu_ClearVector(&y);
		if(z.vx==0.0f && z.vz==0.0f){
			y.vx=1.0f;
		}
		else{
			y.vy=1.0f;
		}
		fpu_OuterProduct(&x,&y,&z);
		fpu_VectorNormal(&x);
		fpu_OuterProduct(&y,&z,&x);

		fpu_MulVectorScaler(&x,&x,BONE_LEN);
		fpu_MulVectorScaler(&y,&y,BONE_LEN);
		fpu_MulVectorScaler(&z,&z,BONE_LEN);


		pos=prim->pos[0]+i*24;
		uvrgb=(DG_PRIM2_UVRGB *)(prim->uvrgb[0])+i*24;


		PutPos(pos,uvrgb,&p,&(world_v[i]),&x,&y,&z,color);
	}

    return 1;
}

void *NewKmsSkeletonTest(DG_OBJS *objs,int color)
{
    Work *work ;

    OPERATOR() ;
    work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;

    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX(&(work->actor));

		if(!KmsSetLine(work,objs,color)){
			GV_DestroyActor(work);
			return NULL;
		}
    }
    return (void *)work;
}

static int EvmSetLine(Work *work,DG_EVMOBJ *obj,int color)
{
	EVM_SKEL *skel;
    DG_PRIM2		*prim;
    DG_PRIM2_UVRGB	*uvrgb;
	FVECTOR *pos;
	FMATRIX *mat;
	int n_models;
    int		i;
	FVECTOR world_v[128];

	n_models=obj->def->n_x_models;
	skel=obj->def->skeleton;
	mat=obj->matrix[obj->use_buffer];

    prim = work->prim = 
		GM_MakePrim2(DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_ANTIALIASING|DG_PRIM2_SINGLEBUFF,
					 n_models,24);
    if(prim==NULL) return 0;


    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
    //prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) ;
    fpu_CopyUnitMatrix(&(prim->as_world));
    prim->group_id = 0x7fffffff ;

    for (i=0;i<n_models;i++){
		FVECTOR vec;

		vec.vx=(skel+i)->rt_tx;
		vec.vy=(skel+i)->rt_ty;
		vec.vz=(skel+i)->rt_tz;
		vec.vw=1.0f;
		_sceVu0ApplyMatrix(&(world_v[i]),mat+i,&vec);
	}

    for (i=0;i<n_models;i++){
		FVECTOR x,y,z,p;
		int parent;


		parent=(skel+i)->parent;

		if(parent!=-1){
			fpu_CopyVector(&p,&(world_v[parent]));
		}
		else{
			fpu_MulVectorScaler(&p,(FVECTOR *)&(mat[i].m[2][0]),BONE_LEN);
			fpu_AddVectors(&p,&(world_v[i]),&p);
			p.vw=1.0f;
		}

		fpu_SubVectors(&z,&p,&(world_v[i]));
#if 1
		fpu_VectorNormal(&z);
#else
		{
			float l=fpu_VectorLength2(&z);
			l=fpu_Rsqrt(l,1.0f);
			fpu_MulVectorScaler(&z,&z,l);

			// printf("%f\n",l);
			//printf("%f %f %f %f\n",z.vx,z.vy,z.vz,z.vw);
		}
#endif

		fpu_ClearVector(&y);
		if(z.vx==0.0f && z.vz==0.0f){
			y.vx=1.0f;
		}
		else{
			y.vy=1.0f;
		}
		fpu_OuterProduct(&x,&y,&z);
		fpu_VectorNormal(&x);
		fpu_OuterProduct(&y,&z,&x);

		fpu_MulVectorScaler(&x,&x,BONE_LEN);
		fpu_MulVectorScaler(&y,&y,BONE_LEN);
		fpu_MulVectorScaler(&z,&z,BONE_LEN);


		pos=prim->pos[0]+i*24;
		uvrgb=(DG_PRIM2_UVRGB *)(prim->uvrgb[0])+i*24;


		PutPos(pos,uvrgb,&p,&(world_v[i]),&x,&y,&z,color);
	}

    return 1;
}

void *NewEvmSkeletonTest(DG_EVMOBJ *obj,int color)
{
    Work *work ;

    OPERATOR() ;
    work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;

    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX(&(work->actor));

		if(!EvmSetLine(work,obj,color)){
			GV_DestroyActor(work);
			return NULL;
		}
    }
    return (void *)work;
}

/*
	particle.c
		パーティクル汎用制御ルーチン

	1999/08/26 K.Kano
	$Id: particle.c,v 1.1.1.3 2002/11/19 11:43:26 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"


typedef struct _Work {
    GV_ACT		actor ;		/* リンク */
    CONTROL_PARTICLES	cparts;
} Work;


/* ------------------------------------------------------------------------ */
/* メイン処理 */

static void Act(Work *work)
{
    ParticleSub(&(work->cparts));
}

static void Die(Work *work)
{
    CONTROL_PARTICLES *cparts=&(work->cparts);
    extern void DG_FreePrim(DG_PRIM *prim);

    if(cparts->partp!=NULL) GV_Free(cparts->partp);
    if(cparts->pos!=NULL) GV_Free(cparts->pos);
    DG_FreePrim((DG_PRIM *)(cparts->prim));
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */


static void CPU_ParticleExtraSub(CONTROL_PARTICLES *cparts,PARTICLE_PARAMETER *partp,
				 FVECTOR *pos,DG_PRIM_PACKET *packet,int size)
{
    int i;

    for(i=size;i>0;i--){
	switch(cparts->display_type){
	case PARTICLE_DISP_TYPE_SPR:
	    break;
	case PARTICLE_DISP_TYPE_LINE:
	    if(cparts->gour_flag){
		DG_LINE_G2 *prim=(DG_LINE_G2 *)(packet->prim_top);
		if(partp->count==0){
		    prim->rgba1.a=0;
		    prim->rgba2.a=0;
		}
		else{
		    prim->rgba1.a=128;
		    prim->rgba2.a=128;
		}
		packet=(DG_PRIM_PACKET *)(prim+1);
	    }
	    else{
		DG_LINE_R2 *prim=(DG_LINE_R2 *)(packet->prim_top);
		if(partp->count==0) prim->rgba1.a=0;
		else prim->rgba1.a=128;
		packet=(DG_PRIM_PACKET *)(prim+1);
	    }
	    break;
	case PARTICLE_DISP_TYPE_POLY:
	    if(cparts->gour_flag){
		DG_POLY_G3 *prim=(DG_POLY_G3 *)(packet->prim_top);
		if(partp->count==0){
		    prim->rgba1.a=0;
		    prim->rgba2.a=0;
		    prim->rgba3.a=0;
		}
		else{
		    prim->rgba1.a=128;
		    prim->rgba2.a=128;
		    prim->rgba3.a=128;
		}
		packet=(DG_PRIM_PACKET *)(prim+1);
	    }
	    else{
		DG_POLY_R3 *prim=(DG_POLY_R3 *)(packet->prim_top);
		if(partp->count==0) prim->rgba1.a=0;
		else prim->rgba1.a=128;
		packet=(DG_PRIM_PACKET *)(prim+1);
	    }
	    break;
	}
    }
}


/* 初期設定値を取得 */
static void GetOptionValue(Work *work)
{
    CONTROL_PARTICLES *cparts=&(work->cparts);
    int	buf[ 3 ];

#if 0
    int stance ;
    float level ;
#endif

    /* 初期位置 */
    if ( GCL_GetOption( 'p' ) != NULL ) {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &( cparts->bounding_box[0] ) ) ;
    }
    if ( GCL_GetOption( 'q' ) != NULL ) {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &( cparts->bounding_box[1] ) ) ;
    }
    if ( GCL_GetOption( 's' ) != NULL ) {
	cparts->size=GCL_GetNextInt();
    }


    /* TEST CODE */
    //cparts->bounding_type=PARTICLE_BOUNDING_TYPE_REFLECT;
    cparts->bounding_type=PARTICLE_BOUNDING_TYPE_OVERLAP;
    //cparts->bounding_type=PARTICLE_BOUNDING_TYPE_ERASE;

    cparts->display_type=PARTICLE_DISP_TYPE_LINE;
    //cparts->display_type=PARTICLE_DISP_TYPE_POLY;
    cparts->gour_flag=1;

    cparts->display_size=40.0f;

    cparts->CPU_OriginalFunc
	=(void (*)(CONTROL_PARTICLES *cparts,PARTICLE_PARAMETER *partp,
		   FVECTOR *pos,void *packet,int size))CPU_ParticleExtraSub;

    /* TEST CODE END */

}

static void InitParticlePackets(Work *work)
{
    CONTROL_PARTICLES *cparts=&(work->cparts);
    int j,k;
    DG_PRIM *prim=cparts->prim;

    switch(cparts->display_type){
    case PARTICLE_DISP_TYPE_LINE:
	if(cparts->gour_flag){
	    for(j=0;j<2;j++){
		DG_PRIM_PACKET *packet=(DG_PRIM_PACKET *)(prim->packs[j]);
		for(k=0;k<prim->n_packet;k++){
		    DG_LINE_G2 *prim=(DG_LINE_G2 *)(packet->prim_top);
		    DG_SET_RGBA1(prim,  0,  0,  0,128);
		    DG_SET_RGBA2(prim,255,255,255,128);
		    packet=(DG_PRIM_PACKET *)(prim+1);
		}
	    }
	}
	else{
	    for(j=0;j<2;j++){
		DG_PRIM_PACKET *packet=(DG_PRIM_PACKET *)(prim->packs[j]);
		for(k=0;k<prim->n_packet;k++){
		    DG_LINE_R2 *prim=(DG_LINE_R2 *)(packet->prim_top);
		    DG_SET_RGBA1(prim,255,255,255,128);
		    packet=(DG_PRIM_PACKET *)(prim+1);
		}
	    }
	}
	break;
    case PARTICLE_DISP_TYPE_POLY:
	if(cparts->gour_flag){
	    for(j=0;j<2;j++){
		DG_PRIM_PACKET *packet=(DG_PRIM_PACKET *)(prim->packs[j]);
		for(k=0;k<prim->n_packet;k++){
		    DG_POLY_G3 *prim=(DG_POLY_G3 *)(packet->prim_top);
		    DG_SET_RGBA1(prim,255,  0,  0,128);
		    DG_SET_RGBA2(prim,  0,255,  0,128);
		    DG_SET_RGBA3(prim,  0,  0,255,128);
		    packet=(DG_PRIM_PACKET *)(prim+1);
		}
	    }
	}
	else{
	    for(j=0;j<2;j++){
		DG_PRIM_PACKET *packet=(DG_PRIM_PACKET *)(prim->packs[j]);
		for(k=0;k<prim->n_packet;k++){
		    DG_POLY_R3 *prim=(DG_POLY_R3 *)(packet->prim_top);
		    DG_SET_RGBA1(prim,255,255,255,128);
		    packet=(DG_PRIM_PACKET *)(prim+1);
		}
	    }
	}
	break;
    case PARTICLE_DISP_TYPE_SPR:
	break;
    }
}


#define INITV	100.0f

static void InitParticleParameters(Work *work)
{
    CONTROL_PARTICLES *cparts=&(work->cparts);
    PARTICLE_PARAMETER *partp=cparts->partp;
    FVECTOR sv;
    int i;

    vu0_Ldv0(&(cparts->bounding_box[0]));
    vu0_Ldv1(&(cparts->bounding_box[1]));
    vu0_Subv0v1();
    vu0_Stv0(&sv);

    for(i=0;i<cparts->size;i++,partp++){
	partp->pos.vx=(float)rand()/(float)RAND_MAX;
	partp->pos.vy=(float)rand()/(float)RAND_MAX;
	partp->pos.vz=(float)rand()/(float)RAND_MAX;
	partp->pos.vw=cparts->display_size;

	partp->pos.vx*=sv.vx;
	partp->pos.vy*=sv.vy;
	partp->pos.vz*=sv.vz;

	vu0_Ldv0(&(partp->pos));
	vu0_Addv0v1();
	vu0_Stv0(&(partp->pos));
	vu0_Stv0(&(partp->old_pos));

	partp->v.vx=(float)rand()/(float)RAND_MAX*INITV;
	partp->v.vy=(float)rand()/(float)RAND_MAX*INITV;
	//partp->v.vz=(float)rand()/(float)RAND_MAX*INITV;
	partp->v.vz=fpu_Sqrt(INITV*INITV-partp->v.vx*partp->v.vx-partp->v.vy*partp->v.vy);

	fpu_ClearVector(&(partp->dv));

	partp->count=-1;
    }
}


/* 資源を獲得 */
static int GetResources(Work *work)
{
    GetOptionValue(work);
    if(!InitParticle(&(work->cparts))) return 0;
    InitParticleParameters(work);
    InitParticlePackets(work);
    return 1;
}

/* 初期化部メイン */
void *NewBasicParticle(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof( Work )) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Act,Die) ;
	
	if(!GetResources(work)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }

    return (void *)work ;
}

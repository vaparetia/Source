//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	va_janken.c
		頂点アニメーション

	1999/11/09 K.Kano
	$Id: va_janken.c,v 1.1.1.3 2002/11/19 11:43:43 Yoshizawa1 Exp $
*/

#if defined(DEBUG) || defined(DEBUG_MODE)
//#define L_DEBUG_MODE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"


/* ワーク */
typedef	struct _Work {
    GV_ACT		actor;		/* リンク */
    OBJECT		body;

    VERTEX_ANIME_WORK	*va;

    FMATRIX		lights[2];
    FVECTOR		pos;
    SVECTOR		rot;

} Work ;


#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_ONEPIECE|DG_FLAG_FINISHCALC)


/* ------------------------------------------------------------------------ */
/* メイン処理 */

#define COUNT 60

static void Act(Work *work)
{
    SimpleVertexAnimation(work->va);

    if(work->va->count==0){
	float p;
	p=work->va->p[2];
	work->va->p[2]=work->va->p[0];
	work->va->p[0]=p;
	work->va->count=COUNT;
    }

    DG_GetLightMatrix(&(work->pos),work->lights);
    DG_SetPos2(&(work->pos),&(work->rot));
    GM_ActObject(&(work->body));
}

static void Die(Work *work)
{
    ExitVertexAnimation(work->va);
    GM_FreeObject(&(work->body));
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */

/* 初期設定値を取得 */
static void GetOptionValue(Work *work)
{
    work->pos=DG_ZeroVector;
    work->rot=DG_ZeroSVector;

    if(GCL_GetOption('x')!=NULL){
	int buf[3];
	GCL_GetIV(GCL_NextStr(),buf);
	vu0_IV0toFV((IVECTOR *)buf,&(work->pos));
    }
    if(GCL_GetOption('r')!=NULL){
	int buf[3];
	GCL_GetIV(GCL_NextStr(),buf);
	work->rot.vx=buf[0];
	work->rot.vy=buf[1];
	work->rot.vz=buf[2];
    }
}



/* 資源を獲得 */
static int GetResources(Work *work)
{
    CV2_DEF *ptn0,*ptn1,*ptn2,*ptn3;


    ptn0=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("dbg_hand"),'c'));
    if(ptn0==NULL) return 0;

    ptn1=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("dbg_guu"),'c'));
    if(ptn1==NULL) return 0;

    ptn2=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("dbg_cyki"),'c'));
    if(ptn2==NULL) return 0;

    ptn3=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("dbg_par"),'c'));
    if(ptn3==NULL) return 0;

    GM_InitObject(&(work->body),GV_StrCode("dbg_hand"),OBJECT_FLAG);
    if((work->va=InitVertexAnimation(work->body.objs->objs+0,&(ptn0->models[0]),
				     DG_VANIME_VERTS|DG_VANIME_NORMS,4))==NULL){
	return 0;
    }
    GM_ConfigObjectLight(&(work->body),work->lights) ;

    work->va->key[0]=ptn0->models+0;
    work->va->key[1]=ptn1->models+0;
    work->va->key[2]=ptn2->models+0;
    work->va->key[3]=ptn3->models+0;
    work->va->p[0]=1.0f;
    work->va->p[1]=0.0f;
    work->va->p[2]=0.0f;
    work->va->p[3]=0.0f;
    work->va->count=0;

#if 0
    printf("SRC : Vertex = %d , Vertex Indexes = %d\n",
	   src->models[HUMAN21_HIDARI_ASHI1].n_verts,
	   src->models[HUMAN21_HIDARI_ASHI1].n_verts_index);
    printf("DIS : Vertex = %d , Vertex Indexes = %d\n",
	   dis->models[0].n_verts,
	   dis->models[0].n_verts_index);
#endif

    GetOptionValue(work);

    return 1;
}

/* 初期化部メイン */
void *NewVATestJanken(void)
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

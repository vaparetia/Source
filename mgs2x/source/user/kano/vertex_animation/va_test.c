//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	va_test.c
		頂点アニメーション

	1999/11/09 K.Kano
	$Id: va_test.c,v 1.1.1.3 2002/11/19 11:43:44 Yoshizawa1 Exp $
*/

#if defined(DEBUG) || defined(DEBUG_MODE)
//#define L_DEBUG_MODE
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
    GV_ACT		actor ;		/* リンク */

    VERTEX_ANIME_WORK	*va;
    VERTEX_ANIME_WORK	*va2;
} Work ;


#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)


/* ------------------------------------------------------------------------ */
/* メイン処理 */

#define COUNT 3

static void Act(Work *work)
{
    SimpleVertexAnimation(work->va);
    SimpleVertexAnimation(work->va2);

    if(work->va->count==0){
	float p;
	p=work->va->p[1];
	work->va->p[1]=work->va->p[0];
	work->va->p[0]=p;
	work->va->count=COUNT;
    }

    if(work->va2->count==0){
	float p;
	p=work->va->p[1];
	work->va->p[1]=work->va->p[0];
	work->va->p[0]=p;
	work->va->count=COUNT;
    }
}

static void Die(Work *work)
{
    ExitVertexAnimation(work->va);
    ExitVertexAnimation(work->va2);
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */


#if 0

/* 初期設定値を取得 */
static void GetOptionValue(Work *work)
{
    work->mp.m=0.001f;
    work->mp.inv_m=1.0f/work->mp.m;
    work->mp.pa=0.001f;
    work->mp.k=-0.0001f;
    work->mp.we=-1.0f;
    work->mp.wl=(int)CVC2N(10);

    work->mp.param_oval=1.2f;

    work->mp.collision_flag=0;

    work->mp.tmat=NULL;
    fpu_CopyUnitMatrix(&(work->mp.root));
    fpu_ClearVector(&(work->mp.x[0]));

    work->mp.target=NULL;
    work->mp.tobjnum=0;

    if(GCL_GetOption('d')!=NULL){
	static const SAMPLE_MODEL_PARAMETER sample[]={
	    { /* ワイヤー */
		1000*0.000001f,	/* -p */
		100*0.001f,	/* -m */
		10000*0.000001f,/* -k */
		100*0.001f+1.0f,/* -e */
		200,		/* -l */
	    },
	    { /* 紐 */
		1000*0.000001f,	/* -p */
		10*0.001f,	/* -m */
		200*0.000001f,	/* -k */
		100*0.001f+1.0f,/* -e */
		200,		/* -l */
	    },
	};
	int d=GCL_GetNextInt();

	work->mp.m=sample[d].m;
	work->mp.inv_m=1.0f/work->mp.m;
	work->mp.pa=sample[d].pa;
	work->mp.k=-sample[d].k;
	work->mp.we=-sample[d].we;
	work->mp.wl=sample[d].wl;
    }

    /* 初期位置 */
    if( GCL_GetOption( 'x' ) != NULL ){
	int buf[ 3 ];

	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &( work->control.mov ) ) ;
	fpu_CopyVector(&(work->mp.x[0]),&(work->control.mov));
	work->mp.x[0].vw=0.0f;
    } 

    /* 特定charaの特定objに付いていく */
    if( GCL_GetOption( 'y' ) != NULL ){
	CONTROL *ctrl;
	int name;

	name=GCL_GetNextInt();
	ctrl=GM_SearchWhere(name);
	if(ctrl!=NULL) work->mp.target=(OBJECT *)(ctrl+1);
    }
    if( GCL_GetOption( 'z' ) != NULL ){
	work->mp.tobjnum=GCL_GetNextInt();
    }
    if(work->mp.target!=NULL){
	work->mp.tmat=&(work->mp.target->objs->objs[work->mp.tobjnum].world);
	work->body.objs->root=&(work->mp.root);
	DG_SetLightMatrix(work->body.objs,work->mp.target->objs->light);
    }

    /* 質量 */
    if( GCL_GetOption('m') != NULL ){
	int m=GCL_GetNextInt();
	float fm,inv_fm;

	/* グラム単位 */
	fm=(float)m*0.001f;
	inv_fm=1.0f/fm;

	work->mp.m=fm;
	work->mp.inv_m=inv_fm;
    }

    /* 風によって受ける力 */
    if( GCL_GetOption('p') != NULL ){
	int p=GCL_GetNextInt();
	float fp;

	fp=(float)p*0.000001f;

	work->mp.pa=fp;
    }

    /* 速度に対する抵抗値 */
    if( GCL_GetOption('k') != NULL ){
	int k=GCL_GetNextInt();
	float fk;

	fk=(float)k*0.000001f;
	work->mp.k=-fk;
    }

    /* 壁に対する反発係数 */
    if( GCL_GetOption('e') != NULL ){
	int e=GCL_GetNextInt();
	float fe;

	fe=(float)e*0.001f+1.0f;
	work->mp.we=-fe;
    }

    /* 壁検出距離 */
    if( GCL_GetOption('l') != NULL ){
	int l=GCL_GetNextInt();
	work->mp.wl=l;
    }

    /* 楕円球当たりのパラメータ */
    if( GCL_GetOption('o') != NULL ){
	int o=GCL_GetNextInt();
	float fo;

	fo=(float)o*0.01f;
	work->mp.param_oval=fo;
    }

    /* 当たり判定フラグ */
    if( GCL_GetOption('f') != NULL ){
	int f=GCL_GetNextInt();
        work->mp.collision_flag=(f!=0);
    }

    if( GCL_GetOption( 'r' ) != NULL ){
	int buf[ 3 ];

	GCL_GetIV(GCL_NextStr(),buf);
	work->control.turn.vx=work->control.rot.vx=buf[0];
	work->control.turn.vy=work->control.rot.vy=buf[1];
	work->control.turn.vz=work->control.rot.vz=buf[2];
    }
}

#endif


/* 資源を獲得 */
static int GetResources(Work *work)
{
    CONTROL *ctrl;
    OBJECT *object;
    DG_OBJ *obj;
    CV2_DEF *src,*dis;

    ctrl=GM_SearchWhere(GV_StrCode("スネーク"));
    if(ctrl==NULL) return 0;

    object=(OBJECT *)(ctrl+1);
    obj=&(object->objs->objs[HUMAN21_HIDARI_ASHI1]);

    src=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_def"),'a'));
    if(src==NULL) return 0;

#if 0
    dis=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_a0"),'a'));
    if(dis==NULL) return 0;

    if((work->va=InitVertexAnimation(obj,&(dis->models[HUMAN21_HIDARI_ASHI1]),
				     DG_VANIME_VERTS,2))==NULL) return 0;

    work->va->key[0]=&(src->models[HUMAN21_HIDARI_ASHI1]);
    work->va->key[1]=&(dis->models[HUMAN21_HIDARI_ASHI1]);
    work->va->p[0]=1.0f;
    work->va->p[1]=0.0f;
    work->va->count=0;

    printf("SRC : Vertex = %d , Vertex Indexes = %d\n",
	   src->models[HUMAN21_HIDARI_ASHI1].n_verts,
	   src->models[HUMAN21_HIDARI_ASHI1].n_verts_index);
    printf("DIS : Vertex = %d , Vertex Indexes = %d\n",
	   dis->models[HUMAN21_HIDARI_ASHI1].n_verts,
	   dis->models[HUMAN21_HIDARI_ASHI1].n_verts_index);
#else
    dis=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_a1"),'a'));
    if(dis==NULL) return 0;

    if((work->va=InitVertexAnimation(obj,&(dis->models[0]),
				     DG_VANIME_VERTS|DG_VANIME_NORMS,2))==NULL) return 0;

    work->va->key[0]=&(src->models[HUMAN21_HIDARI_ASHI1]);
    work->va->key[1]=&(dis->models[0]);
    work->va->p[0]=1.0f;
    work->va->p[1]=0.0f;
    work->va->count=0;

    printf("SRC : Vertex = %d , Vertex Indexes = %d\n",
	   src->models[HUMAN21_HIDARI_ASHI1].n_verts,
	   src->models[HUMAN21_HIDARI_ASHI1].n_verts_index);
    printf("DIS : Vertex = %d , Vertex Indexes = %d\n",
	   dis->models[0].n_verts,
	   dis->models[0].n_verts_index);
#endif

    obj=&(object->objs->objs[HUMAN21_HIDARI_ASHI2]);

#if 0
    if((work->va2=InitVertexAnimation(obj,&(dis->models[HUMAN21_HIDARI_ASHI2]),
				      DG_VANIME_VERTS,2))==NULL){
	ExitVertexAnimation(work->va);
	return 0;
    }

    work->va2->key[0]=&(src->models[HUMAN21_HIDARI_ASHI2]);
    work->va2->key[1]=&(dis->models[HUMAN21_HIDARI_ASHI2]);
    work->va2->p[0]=1.0f;
    work->va2->p[1]=0.0f;
    work->va2->count=0;

    printf("SRC : Vertex = %d , Vertex Indexes = %d\n",
	   src->models[HUMAN21_HIDARI_ASHI2].n_verts,
	   src->models[HUMAN21_HIDARI_ASHI2].n_verts_index);
    printf("DIS : Vertex = %d , Vertex Indexes = %d\n",
	   dis->models[HUMAN21_HIDARI_ASHI2].n_verts,
	   dis->models[HUMAN21_HIDARI_ASHI2].n_verts_index);
#else
    dis=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_a2"),'a'));
    if(dis==NULL) return 0;

    if((work->va2=InitVertexAnimation(obj,&(dis->models[0]),
				      DG_VANIME_VERTS|DG_VANIME_NORMS,2))==NULL){
	ExitVertexAnimation(work->va);
	return 0;
    }

    work->va2->key[0]=&(src->models[HUMAN21_HIDARI_ASHI2]);
    work->va2->key[1]=&(dis->models[0]);
    work->va2->p[0]=1.0f;
    work->va2->p[1]=0.0f;
    work->va2->count=0;

    printf("SRC : Vertex = %d , Vertex Indexes = %d\n",
	   src->models[HUMAN21_HIDARI_ASHI2].n_verts,
	   src->models[HUMAN21_HIDARI_ASHI2].n_verts_index);
    printf("DIS : Vertex = %d , Vertex Indexes = %d\n",
	   dis->models[0].n_verts,
	   dis->models[0].n_verts_index);
#endif

    //GetOptionValue(work);

    return 1;
}

/* 初期化部メイン */
void *NewVATest(void)
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

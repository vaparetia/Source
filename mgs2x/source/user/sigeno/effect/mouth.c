//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mouth.c
	くちパク *NewMouthAnim( &work->body );
	2000/07/05 K.Sigeno
	$Id: mouth.c,v 1.1.1.3 2002/11/19 11:49:48 Yoshizawa1 Exp $
*/

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

#include "mouth.h"



/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
    OBJECT		body;		/*合成モデル*/
	OBJECT		*human ;	/* 接続先モデル*/
    VERTEX_ANIME_WORK	*va;
    FMATRIX		lights[2];
	int		now_mode;	/*現在の目標形状番号*/
	int		*task;		/*アクターの動作状況*/
	int		*req_mode;	/*リクエスト*/
	int		*count;		/*補完時間*/
} Work ;

//#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_ONEPIECE|DG_FLAG_FINISHCALC)
#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION)
/* ------------------------------------------------------- */

/* メイン処理 */

/************************************
#define HUMAN21_KUBI             (11)
#define HUMAN21_ATAMA            (12)
HUMAN21_HIDARI_UDE2
************************************/
static void Act(Work *work)
{
	int i ;

	work->human->objs->objs[HUMAN21_ATAMA].flag
		|= DG_FLAG_INVISIBLE ;
	/*終了処理*/
	if((work->now_mode == 0)
	&&(*work->req_mode == 0)
	&&(work->va->count==0)
	){
		*work->task = ACT_END ;
		work->human->objs->objs[HUMAN21_ATAMA].flag
			&= ~DG_FLAG_INVISIBLE ;
		work->body.objs->objs[1].flag |= DG_FLAG_INVISIBLE ;

		GV_DestroyActor(work) ;
		return ;
	}
	*work->task = ACTIVE ;
	if(work->now_mode != *work->req_mode ) {
		/*新規アニメセット*/
		for(i=0;i<FACE_ANIM_NUM ; i++) {
			work->va->p[i]= 0.0F;
		}
//printf("MOUTH ACT \n") ;
//printf("SW == %d\n",*work->req_mode) ;
//printf("CONT == %d\n",*work->count) ;
		work->now_mode = *work->req_mode ;
		work->va->p[work->now_mode]= 1.0F;
		work->va->count = *work->count ;
	}
	if(work->va->count > 0 ) {
		/*変形処理 本体*/
		SimpleVertexAnimation(work->va);
	}
	/*ライトをコピー*/
	work->lights[0] = work->human->objs->light[0];
	work->lights[1] = work->human->objs->light[1];

	/*本体のマトリクスに合わせる*/
	work->body.objs->objs[0].world 
		= work->human->objs->objs[HUMAN21_KUBI].world ;
	work->body.objs->objs[1].world 
		= work->human->objs->objs[HUMAN21_ATAMA].world ;
	work->body.objs->world 
		= work->human->objs->objs[HUMAN21_KUBI].world ;
	work->body.map_name = work->human->map_name ; /*兵士のマップ*/
	GM_GroupObjs( work->body.objs, work->human->map_name ) ;

#if 0
printf("NOW MOUTH ACTIVE\n") ;

	{
		SVECTOR rgb ;
		FVECTOR viewpos;
	extern void PosBox(FVECTOR * ,float ,SVECTOR * );

		GV_MatToVec(&work->body.objs->objs[1].world ,&viewpos);

		rgb.vx = 255;
		rgb.vy =   0;
		rgb.vz =   0;
//		PosBox(&rgb );
		viewpos.vy += 100.0F ;
		PosBox(&GM_PlayerPosition,100.0F,&rgb );
	}
#endif

}

static void Die(Work *work)
{
//printf("MOUTH DIE \n") ;
	if(work->va != NULL) ExitVertexAnimation(work->va);
    GM_FreeObject(&(work->body));
}

/* ------------------------------------------------------------------------ */

/* 資源を獲得 */
static int GetResources(Work *work,OBJECT *human)
{
	CV2_DEF *ptn[FACE_ANIM_NUM];
	CV2_DEF *base;
	int i,part_no,cv2_num;

	work->human = human;

	if(
	(*work->req_mode >= FACE_ANIM_NUM)
	||(*work->req_mode < 0)
	){
		return 0;
	}
	if( human == NULL ) {
		return 0;
	}
	base =(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode(
		"gbs_face_def"),'c'));

	ptn[0]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode(
	"gbs_face0"),'c'));
	if(ptn[0]==NULL){
		printf("MOUTH C %d\n",__LINE__);
		return 0;
	}
	ptn[1]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode(
	"gbs_face1"),'c'));
    if(ptn[1]==NULL) {
		printf("MOUTH C %d\n",__LINE__);
		return 0;
	}

	GM_InitObject(&(work->body),GV_StrCode("gbs_face_def"),OBJECT_FLAG);

    GM_ConfigObjectLight(&(work->body),work->lights) ;

	/*変形させる関節番号を指定*/
	/*表示用の切り出しモデルのどちらが親か？*/
	part_no = 1; /*表示用KMSの関節何番を変形させるか*/
	cv2_num = 0; /*CV2中の関節番号*/

	if((work->va=InitVertexAnimation(work->body.objs->objs+part_no,
	&(base->models[part_no]),DG_VANIME_VERTS|DG_VANIME_NORMS,
	FACE_ANIM_NUM))==NULL){
		printf("MOUTH C %d\n",__LINE__);
		return 0;
	}

	work->va->key[0] = &(base->models[1]); 
	work->va->p[0]=0.0f;	/*合成比率*/

	for(i=1;i<(FACE_ANIM_NUM);i++){
		work->va->key[i]=&(ptn[i-1]->models[0]); 
		work->va->p[i]=0.0f;	/*合成比率*/
	}

    work->va->count = 0;
	work->now_mode = 0;
    work->va->p[work->now_mode]=1.0f;
/*人体を不可視*/
#if 0
	/*act内に移行*/
	work->human->objs->objs[HUMAN21_ATAMA].flag
		|= DG_FLAG_INVISIBLE ;
#endif
/*付随モデルの根元を不可視*/
	work->body.objs->objs[0].flag |= DG_FLAG_INVISIBLE ;
	/*初期状態にするため、変形処理を一回行う*/
	SimpleVertexAnimation(work->va);
    return 1;
}


/* 初期化部メイン */

#define PRIO	0x40	/* 敵兵より後に処理 */

	int		now_mode;	/*現在の目標形状番号*/
	int		*task;		/*アクターの動作状況*/
	int		*req_mode;	/*リクエスト*/
	int		*count;		/*補完時間*/
void *NewMouthAnim( human ,task,req_mode,count)
OBJECT	*human ;	/* 人体オブジェクト */
int		*task;
int		*req_mode;
int		*count;
{
    Work *work ;
	*task = OFF ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if(work!=NULL) {
		work->task = task;
		work->req_mode = req_mode ;
		work->count = count ;
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,human)){
			printf("MOUTH C %d\n",__LINE__);
			GV_DestroyActor(work) ;
			return NULL ;
		}else {
			*task = GET_RES ;
		}
	}
	return (void *)work ;
}


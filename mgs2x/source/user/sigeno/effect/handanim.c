//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	handanim.c
	クリアリング時のハンドシグナル *NewFingerSign( &work->body );
	2000/03/13 K.Sigeno
	$Id: handanim.c,v 1.1.1.3 2002/11/19 11:49:46 Yoshizawa1 Exp $
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

#ifndef DEMO_GBS_HAND
#include "korekado/enemy/enemy.h"
#endif
#define	HAND_ANIM_NUM	(6)
//#define	HAND_ANIM_NUM	(5)

/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
    OBJECT		body;
	OBJECT	*human ;		/* 接続先モデル*/
    VERTEX_ANIME_WORK	*va;
    FMATRIX		lights[2];
	int	*sw ;
	int	mode;
	int name;
} Work ;


//#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_ONEPIECE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION)
#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION)
/* ------------------------------------------------------- */

/* メイン処理 */

enum {
	GBS_MDL,	/*茶色迷彩*/
	URBAN_MDL,	/*都市迷彩*/
	HITECH_MDL	/*ハイテク兵*/
} ;


/***
#define	SW_FLAG_SWITCH1	0x00000002
#define	SW_FLAG_SWITCH2	0x00000004
#define	SW_FLAG_SWITCH3	0x00000008
#define	SW_FLAG_SWITCH4	0x00000010
０ デフォルト
１ グー
２ パー
３ 人差し指
４ 親指

***/
#ifndef DEMO_GBS_HAND
static int SW_2_VAL(int sw){
	if(sw < 0 ) return -1;
	if(sw & SW_FLAG_SWITCH1 ) return 1 ;
	if(sw & SW_FLAG_SWITCH2 ) return 2 ;
	if(sw & SW_FLAG_SWITCH3 ) return 3 ;
	if(sw & SW_FLAG_SWITCH4 ) return 4 ;
	if(sw & SW_FLAG_SWITCH5 ) return 5 ;
	return 0 ;
}
#endif
#ifdef DEMO_GBS_HAND
static void MSG_Check(Work *work,int *sw, int *cnt){
	GV_MSG *msg;
	int n_msg,code;
	float	time;
	*sw = work->mode ;
	n_msg = GV_ReceiveMessage( work->name, &msg );
	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case 1 :
				*sw = msg->message[ 1 ] ;
				time = (float) msg->message[ 2 ] ;
				time *= (6.0F/100.0F);
				*cnt = (int) time ;
#ifdef DEMO_GBS_HAND
printf("HAND MSG %d %d \n",*sw ,*cnt );
#endif

			break ;
		}
		msg++ ;
	}
}
#endif

static void Act(Work *work)
{
	int sw,i ;

#ifdef DEMO_GBS_HAND
	int cnt ;
#else
	/*補完時間*/
	u_char time_lst[8] = {18,6,18,6,6,1,0,0 };
#endif

#ifdef DEMO_GBS_HAND
	MSG_Check(work,&sw,&cnt);
#else
	sw = SW_2_VAL(*work->sw);
//	sw = *work->sw ;
#endif

	work->human->objs->objs[HUMAN21_HIDARI_TE].flag
		|= DG_FLAG_INVISIBLE ;

	if(sw >= HAND_ANIM_NUM ) sw = 0;
	if(sw != work->mode ) {
		if(sw < 0){
//printf("HAND ANIM DESTROY \n");
			/*強制終了*/
			/*人体の手首を可視*/
			work->human->objs->objs[HUMAN21_HIDARI_TE].flag
				&= ~DG_FLAG_INVISIBLE ;
			GV_DestroyActor(work) ;
			return ;
		}
		/*新規アニメセット*/
		for(i=0;i<HAND_ANIM_NUM ; i++) {
//printf("HAND ANIM SET!!!! \n");
			work->va->p[i]= 0.0F;
		}
		work->mode = sw ;
		work->va->p[work->mode]= 1.0F;
#ifdef DEMO_GBS_HAND
		work->va->count = cnt ;
#else
		work->va->count = DIRECT_TICK(time_lst[work->mode]) ;
#endif
	}
	if(work->va->count > 0 ) {
		/*変形処理 本体*/
		SimpleVertexAnimation(work->va);
	}
//	DG_COPY_MAT( &work->body.objs->objs[0].world , &work->human->objs->objs[HUMAN21_HIDARI_UDE2].world );

	/*ライトをコピー*/
//	work->lights[0] = work->human->objs->light[0];
//	work->lights[1] = work->human->objs->light[1];

	/*本体のマトリクスに合わせる*/
	work->body.objs->objs[0].world 
		= work->human->objs->objs[HUMAN21_HIDARI_UDE2].world ;
	work->body.objs->objs[1].world 
		= work->human->objs->objs[HUMAN21_HIDARI_TE].world ;

	work->body.objs->world 
		= work->human->objs->objs[HUMAN21_HIDARI_TE].world ;
//		= work->human->objs->world ;
	work->body.map_name = work->human->map_name ; /*兵士のマップ*/

	/*親の非表示に対応*/
	if(work->human->objs->flag & DG_FLAG_INVISIBLE ){
		DG_InvisibleObjs( work->body.objs );
	}else {
		DG_VisibleObjs( work->body.objs );
	}


	GM_GroupObjs( work->body.objs, work->human->map_name ) ;
}

static void Die(Work *work)
{
	extern void DG_DisconnectObjs( DG_OBJS *, DG_OBJS * ) ;

	if(work->va != NULL) ExitVertexAnimation(work->va);
#if 0
	if ( !(GV_IsStageDestroy( work )) ) {
		DG_DisconnectObjs( work->human->objs, work->body.objs ) ;
	}
#else
	DG_DisconnectObjs( work->human->objs, work->body.objs ) ;
#endif

    GM_FreeObject(&(work->body));
}

/* ------------------------------------------------------------------------ */

/* 資源を獲得 */
#ifdef DEMO_GBS_HAND
static int GetResources(Work *work,OBJECT *human ,int name)
#else
static int GetResources(Work *work,OBJECT *human,int *sw,int type)
#endif
{
	extern void DG_ConnectObjs( DG_OBJS *, DG_OBJS * ) ;

	CV2_DEF *ptn[HAND_ANIM_NUM];
//	CV2_DEF *def_ptn;

	int i,part_no,cv2_num;

printf("ENEMY HAND ANIMATION INIT!!\n");
	work->human = human;
#ifdef DEMO_GBS_HAND
    GM_InitObject(&(work->body),GV_StrCode("gbs_hand_def"),OBJECT_FLAG);
	work->name = name ;
#else
	switch(type){
		case GBS_MDL :
		    GM_InitObject(&(work->body),GV_StrCode("gbs_hand_def"),OBJECT_FLAG);
			break;
		case URBAN_MDL :
		    GM_InitObject(&(work->body),GV_StrCode("gps_hand_def"),OBJECT_FLAG);
			break;
		case HITECH_MDL :
		    GM_InitObject(&(work->body),GV_StrCode("htc_hand_def"),OBJECT_FLAG);
			break;
	}
	work->sw = sw ;
#endif
	if( human == NULL ) {
		printf("GBS HAND PARENT OBJ NULL !\n");
		return 0;
	}
	GM_ConfigObjectLight(&(work->body),work->human->objs->light) ;
	/*本体に関連付け*/
	DG_ConnectObjs( work->human->objs, work->body.objs ) ;

#ifdef DEMO_GBS_HAND
	
//	def_ptn=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand_def"),'c'));
//	ptn[0]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand0"),'c'));
	ptn[0]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand_def"),'c'));
	ptn[1]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand1"),'c'));
	ptn[2]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand2"),'c'));
	ptn[3]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand3"),'c'));
	ptn[4]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand4"),'c'));
	ptn[5]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand5"),'c'));
#else 


	switch(type){
		case GBS_MDL :
//	def_ptn=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand_def"),'c'));
//			ptn[0]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand0"),'c'));

			ptn[0]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand_def"),'c'));
		    ptn[1]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand1"),'c'));
			ptn[2]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand2"),'c'));
			ptn[3]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand3"),'c'));
			ptn[4]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand4"),'c'));
			ptn[5]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gbs_hand5"),'c'));
			break;
		case URBAN_MDL :
printf("URBAN_CV2 INIT !!\n");
//			def_ptn=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gps_hand_def"),'c'));
//			ptn[0]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gps_hand0"),'c'));

			ptn[0]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gps_hand_def"),'c'));
			ptn[1]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gps_hand1"),'c'));
			ptn[2]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gps_hand2"),'c'));
			ptn[3]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gps_hand3"),'c'));
			ptn[4]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gps_hand4"),'c'));
			ptn[5]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("gps_hand5"),'c'));
			break;
		case HITECH_MDL :
			ptn[0]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("htc_hand_def"),'c'));
			ptn[1]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("htc_hand_1"),'c'));
			ptn[2]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("htc_hand_2"),'c'));
			ptn[3]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("htc_hand_3"),'c'));
			ptn[4]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("htc_hand_4"),'c'));
			ptn[5]=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode("htc_hand_5"),'c'));
			break;
	}
#endif

    if((ptn[0]==NULL)||(ptn[1]==NULL)||(ptn[2]==NULL)||(ptn[3]==NULL)||(ptn[4]==NULL)||(ptn[5]==NULL)) {
		ASSERT(0);
	}
	/*変形させる関節番号を指定*/
	part_no = 1; /*元のモデルの関節何番を変形させるか*/
	cv2_num = 0; /*CV2中の関節番号*/

	if((work->va=InitVertexAnimation(work->body.objs->objs+part_no,
//	&(ptn[0]->models[cv2_num]),DG_VANIME_VERTS|DG_VANIME_NORMS,
	&(ptn[0]->models[part_no]),DG_VANIME_VERTS|DG_VANIME_NORMS,
	HAND_ANIM_NUM))==NULL){
		printf("VertexAnimation Work Failed !\n");
		return 0;
	}

    work->va->key[0]=&(ptn[0]->models[part_no]);
	for(i=1;i<HAND_ANIM_NUM;i++){
	    work->va->key[i]=&(ptn[i]->models[cv2_num]);
	    work->va->p[i]=0.0f;	/*合成比率*/
	}

    work->va->p[0]=1.0f;
    work->va->count=0;

	work->mode = 0;
/*人体の手首を不可視*/
	work->human->objs->objs[HUMAN21_HIDARI_TE].flag
		|= DG_FLAG_INVISIBLE ;
//	work->human->objs->objs[HUMAN21_HIDARI_UDE2].flag
//		|= DG_FLAG_INVISIBLE ;
/*付随モデルの下腕を不可視*/
	/*腕*/
	work->body.objs->objs[0].flag |= DG_FLAG_INVISIBLE ;
	/*初期状態にするため、変形処理を一回行う*/
	SimpleVertexAnimation(work->va);
#ifdef DEMO_GBS_HAND
printf("GBS HAND INIT!!\n");
#endif

    return 1;
}


/* 初期化部メイン */

#define PRIO	0x40	/* 敵兵より後に処理 */

#ifdef DEMO_GBS_HAND
//デモ呼び出し用ハンドアニメ *NewDemoGbsHand( OBJECT * );
void *NewGbsHandDemo( human , name )
OBJECT	*human ;	/* 人体オブジェクト */
int		name ;
{
    Work *work ;

	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,human,name)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
#else
void *NewFingerSign( human ,sw )
OBJECT	*human ;	/* 人体オブジェクト */
int		*sw ; 		/* 制御スイッチ */
{
    Work *work ;

	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,human,sw,GBS_MDL)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
void *NewFingerSignUrban( human ,sw )
OBJECT	*human ;	/* 人体オブジェクト */
int		*sw ; 		/* 制御スイッチ */
{
    Work *work ;

	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,human,sw,URBAN_MDL)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewFingerSignHiTech( human ,sw )
OBJECT	*human ;	/* 人体オブジェクト */
int		*sw ; 		/* 制御スイッチ */
{
    Work *work ;

	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,human,sw,HITECH_MDL)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


#endif


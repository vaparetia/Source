//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	play_disap.c
	プレイヤを縮小消去 前作ＶＲ風
	2002/06/07 K.Sigeno
	$Id: play_disap.c,v 1.1.1.3 2002/11/19 11:49:48 Yoshizawa1 Exp $
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

#include "../effect/sig_vanim.h"


#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif

extern DG_OBJS *GM_GetMapObjs(int) ;
extern int NewKillInvisibleChara(void) ;
extern int NewInvisibleChara(void) ;


/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	DG_OBJS		*objs ;
	FMATRIX		lights[2] ;
	int			cnt ;
} Work ;

#define NORMAL_MODE			(0x00000000)
#define AMB_MODE			(0x00000001)
#define BUILD_Y				(0x00000002)
#define BUILD_UP			(0x00000004)	/*足から変形開始*/
#define NO_CHECK_SW			(0x00000008)
#define DISAPPEAR_MODE		(0x00000010) /*実体から消えへ デフォルトは無から実体へ*/
#define BODY_MODE			(0x00000020)
#define BUILD_X				(0x00000040)
#define BUILD_ROLL			(0x00000080)
#define BUILD_REV			(0x00000100) /*Y軸下から飛んでくる*/
#define MOTION_TR			(0x00000200) /*モーション追随*/
#define BUILD_RND			(0x00000400)	/*ノイジー*/


#define TIME_A		(15)
#define TIME_B		(15)
#define DELAY_ITEM	(15)
#define FLY_LEN		(0.0f)
//#define MODE		(BUILD_Y|DISAPPEAR_MODE|BUILD_UP|BUILD_REV)
#define MODE		(DISAPPEAR_MODE|BUILD_UP|MOTION_TR)
static void CallBreakBody(Work *work){
extern void *NewSigBreakBody(DG_OBJS *,float ,CVECTOR *,int,int,int,int,void *) ;
extern void *NewSigBreakObj3(int mdlcode,FMATRIX *world,int mode ,float len,int time_a ,
		int delay,int *sw,FVECTOR *shift,SVECTOR *rot) ;

//work->objs
	NewSigBreakBody(GM_PlayerBody->objs,FLY_LEN, NULL,TIME_A,TIME_B,0,MODE,NULL) ;
//	NewSigBreakBody(work->objs,FLY_LEN, NULL,TIME_A,TIME_B,0,MODE,NULL) ;
	if(GM_AnotherPlayer == GM_ANOTHER_PLAYER_RAIDEN ) {
		NewSigBreakObj3(GV_StrCode("rai_hair_shadow_mt"),
			&(GM_PlayerBody->objs->objs[HUMAN21_ATAMA].world),
			MODE|NO_CHECK_SW ,FLY_LEN,TIME_A ,DELAY_ITEM,NULL,&DG_ZeroVector, 
			&DG_ZeroSVector) ;
	}

}

#define DISAP_TIME	(30)
static void Act(Work *work)
{
	int i ;
	FVECTOR scale ,base_scale;
	
#if 1
	if(work->cnt==DISAP_TIME){
		CallBreakBody(work) ;
	}
#else
	if(work->cnt >(DISAP_TIME/2)){
		if((work->cnt==DISAP_TIME)||((work->cnt &7 )==0)){
			CallBreakBody(work) ;
		}
	}
#endif

	if(work->cnt==DISAP_TIME){
		//SD_E_STEALTH2,	//ステルス兵ステルス化//stealth2 1499
		GM_SeSetMode(SD_E_STEALTH2,&GM_PlayerPosition ,GM_SEMODE_BOMB) ;
	}

	if(work->cnt ==(DISAP_TIME/2)){
//		NewInvisibleChara() ;
	}
	work->cnt-- ;
	if(work->cnt<-60){
//		work->cnt = 127 ;
//		GV_DestroyActor(work) ;
	}
}
static void Die(Work *work)
{
	NewKillInvisibleChara() ;
	if(work->objs != NULL ) {
//		DG_DequeueObjs( work->objs ) ;
		DG_FreeObjs( work->objs );
	}
}

static int GetResources(Work *work,int name , int where )
{



#if 1


#else
	DG_DEF	*def ;
	int i;
	def = (DG_DEF*) GM_PlayerBody->objs->def ;
	work->objs = DG_MakeObjs( def, DG_FLAG_FINISHCALC, DG_CHANL_MAIN ) ;
	if(work->objs == NULL ) return 0 ;
//	DG_QueueObjs( work->objs ) ;
	DG_SetPos( &GM_PlayerBody->objs->world ) ;
	DG_PutObjs( work->objs );
	for(i= 0 ;i<GM_PlayerBody->objs->n_models;i++){
		work->objs->objs[i].world = GM_PlayerBody->objs->objs[i].world ;
	}
//	DG_SetLightMatrix( work->objs, work->lights );
//	DG_GetLightMatrix( (FVECTOR *) &work->objs->world.m[3] , work->lights );
#endif

	NewInvisibleChara() ;

	work->cnt = DISAP_TIME ;
	return 1;
}

void *NewPlayerDisap( int name , int where )
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,name,where)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


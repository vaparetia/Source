//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scn_break.c
	2002/08/04 K.Sigeno
	$Id: scn_break.c,v 1.3 2002/11/23 12:46:55 Yoshizawa1 Exp $
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
#include "camera.h"

#include "../effect/sig_vanim.h"



/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	CONTROL		*p_ctrl ;
	OBJECT		*p_body ;
	int			name ;
	int			mode ;
	int			flag ;
	int			delay ;
	int			cnt ;
	int			in_flag ;
	int			proc_id ;
	int			first_se_tick ;
	int			last_se_tick ;
	int			time_a ;
	int			time_b ;
} Work ;

#define	DEFAULT_MODE	(5)
#define	NO_XZ_MOVE			(0x0001) /*モーションの水平移動を無視*/
#define	NO_GRAVITY			(0x0002) /*重力接地無し*/
#define	NO_HAZARD			(0x0004) /*壁アタリ無し*/
#define	PAUSE_MOT			(0x0008) /*静止*/
#define	INIT_POS			(0x0010) /*再生開始時に座標リセット*/
#define	NO_INTERP			(0x0020) /*モーション補完無し*/
#define	PRESHADE			(0x0040) /*プリシェード使用*/
#define	INVISIBLE_START		(0x0080) /*非表示で起動*/
#define	PUT_MOT_THERMAL		(0x0100)	//赤外線ゴーグル反応
#define	PUT_MOT_BOUND		(0x0200)	//デバッグ用表示
#define	MT_ANIM				(0x0400)	//ストリーム口パク
#define	RAD_POINT			(0x0800)	//ストリーム口パク



#define TIME_A	DIRECT_TICK(30)
#define TIME_B	DIRECT_TICK(50)


enum {
	MSG_TR_START = 1,
};

#define TR_START	(0x0001)


#define HANKEI (750.0f)
//#define HANKEI (10000.0f)
#define BREAK_RANGE	(300.0f)
#define INTERP   6*(300/BP_FRAMES_PER_SEC())

extern void *NewSigBreakBody(DG_OBJS *,float ,CVECTOR *,int,int,int,int,void *) ;
extern void *NewSigBreakObj5(DG_OBJS *,int ,float ,int  ,int ,int *) ;



static	void	CheckMessage( work )
Work	*work ;
{
    GV_MSG	*msg ;
	int n_msg, code ;
	n_msg = GV_ReceiveMessage( work->name, &msg );

	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case MSG_TR_START :
printf("MESG REC!!!!!!!!!!!!!!!!!\n");
				if(!(work->in_flag & TR_START)){
					work->in_flag |= TR_START ;
					NewSigBreakBody(work->p_body->objs,BREAK_RANGE, NULL,work->time_a,work->time_b,
						0,work->flag,NULL) ;
					if(work->p_body->objs->next_dgobjs != NULL){
						NewSigBreakObj5(work->p_body->objs->next_dgobjs,work->flag,BREAK_RANGE,work->time_a,
							work->time_b,NULL) ;
					}
				}
				break;
		}
		msg++ ;
	}
}

#if 0

static void visiblemdl(work)
Work	*work ;
{
//printf("putmodel visible set!!\n");

	if(work->body.evmobj != NULL){
		work->body.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
	}else {
		DG_VisibleObjs( work->body.objs ) ;
	}
	if(work->in_flag & ITEM_SET){
		DG_VisibleObjs( work->item.objs ) ;
	}
	if(work->status & RAD_POINT){
		work->rctrl.flag |= RADAR_VISIBLE ;
	}
}
#endif

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



static CONTROL* SearchControl( int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		return ( control ) ;
	}

	return ( NULL );
}


static void Act(Work *work)
{
	int mode=0,cnt=0 ,se_tick;

	CheckMessage( work ) ;
	DG_InvisibleObjs(work->p_body->objs) ;
	if(!(work->in_flag & TR_START)){
		if(work->p_body->objs->next_dgobjs != NULL){
			DG_InvisibleObjs(work->p_body->objs->next_dgobjs) ;
		}
		return ;
	}else {
		if(work->p_body->objs->next_dgobjs != NULL){
			DG_VisibleObjs(work->p_body->objs->next_dgobjs) ;
		}
	}

	GM_CurrentMap = GM_CurrentStageMap ;


	if(work->cnt >= DIRECT_TICK(work->time_a+work->time_b)){
		if(work->proc_id != 0){
			GM_ExecProc( work->proc_id, NULL );
		}
		DG_VisibleObjs(work->p_body->objs) ;
		if(work->p_body->objs->next_dgobjs != NULL){
			DG_VisibleObjs(work->p_body->objs->next_dgobjs) ;
		}
		GV_DestroyActor(work) ;
	}


/*SE CALL*/
	/*VR以外ではプレイヤエフェクトで音を鳴らす*/
//	if(!(GM_GameStatus & STATE_VR_ONLY)){
	if(0){
		se_tick = DG_TickCount - work->first_se_tick ;
	//printf("TICK[%d] ACT[%d] \n",se_tick,work->cnt) ;
		if(work->cnt == 0){
			GM_SdSet(SD_A_V_START1);
			work->first_se_tick = DG_TickCount ;
		}else if((DIRECT_TICK(36) <=se_tick)&&(work->cnt<(work->time_a+work->time_b))){
			if( (DG_TickCount - work->last_se_tick) >DIRECT_TICK(12) ){
				GM_SdSet(SD_A_V_START2);
				work->last_se_tick = DG_TickCount;
			}else {
			}
		}else if(work->cnt == (work->time_a+work->time_b)) {
			GM_SdSet(SD_A_V_START3 );
		}
	}

	work->cnt++ ;
}
static void Die(Work *work)
{
}
static int GetResources(Work *work, int name, int where)
{
extern void *NewSigBreakBody(DG_OBJS *,float ,CVECTOR *,int,int,int,int,void *) ;
extern void *NewSigBreakObj3(int mdlcode,FMATRIX *world,int mode ,float len,int time_a ,
		int delay,int *sw,FVECTOR *shift,SVECTOR *rot) ;

	int mode ,delay;

	GM_CurrentMap = GM_CurrentStageMap ;

	if ( GCL_GetOption( 'p' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			work->proc_id  = GCL_GetNextInt();
		}else {
			work->proc_id  = 0;
		}
	}

	work->p_ctrl = NULL ;
	if ( GCL_GetOption( 'n' ) != NULL ){
		work->p_ctrl = SearchControl( GCL_GetNextInt() ) ;
	}
	if(work->p_ctrl == NULL ){
		printf("BREAK BODY CHARACTOR NOT FOUND !!!\n");
		ASSERT(0) ;
	}
	work->p_body = work->p_ctrl->object ;
	if(work->p_body == NULL ){
		printf("PARENT OBJECT IS NOT LINKED !!!\n");
		ASSERT(0) ;
	}

	DG_InvisibleObjs(work->p_body->objs) ;
	work->name = name ;

	work->time_a = TIME_A;
	work->time_b = TIME_B;

	if ( GCL_GetOption( 't' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			work->time_a  = DIRECT_TICK(GCL_GetNextInt());
		}
		if( GCL_NextStr() != NULL ){
			work->time_b  = DIRECT_TICK(GCL_GetNextInt());
		}
	}

	work->mode = DEFAULT_MODE ;

	mode = BUILD_UP|BUILD_Y|MOTION_TR ;;

	work->flag = mode ;
	work->first_se_tick = 0;
	work->last_se_tick = 0;
	work->in_flag = 0 ;
	return 1;
}


/* 初期化部メイン */
void *NewScnBreakBody( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_AFTER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources( work,name,where )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


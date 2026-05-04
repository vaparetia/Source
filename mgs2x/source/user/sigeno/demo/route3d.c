//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	route3d.c
	３Ｄルートを巡回するプットオブジェ
	chara	プットルートオブジェ[NewRoute3d] 
	2000/12/15 K.Sigeno
	$Id: route3d.c,v 1.1.1.3 2002/11/19 11:49:27 Yoshizawa1 Exp $
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
#include	"korekado/enemy/enemy.h"
#include "korekado/enemy/enemy.x"

#define		CYP_SPEED_LOW_LIMIT (0.005F)
#define		CYP_ACCELE	(2.0F)
#define		DEF_MAX_SPEED	(80) /*最高速*/

/*ポイントタイムの値がこの時、mesg受信まで待機*/
#define		P_TIME_WAIT	(-3)	
//#define		P_TIME_WAIT	(150)	
/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	int			name ;
	DG_OBJS		*objs ;
	FMATRIX		lights[2] ;
	CONTROL		control;
//    CONTROL_NOEVT		ctrl ;
	ROUTENAVI	rnavi;	/*ルート誘導*/
	TRGPOINT	trg;	/*移動目標情報*/
	int			route ; /*ルート番号*/
	short		mode ;
	short			status ;
	float		speed ; 
} Work ;
enum {
	MOVE_MODE,ACTION_MODE
};
enum {
	MSG_ROUTE_CHANGE = 1
};
enum {
	ST_NORMAL = 0x00,
	ST_MULTI = 0x01,
	ST_WAIT = 0x02,
};

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern void RouteView(ROUTENAVI *) ;

#endif

#define	ONE_PIECE_FLAG	(DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
#define	MULTI_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

//#define	OBJECT_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)
//#define	OBJECT_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

static void SetPointAction( Work *work ){
	ROUTENAVI *rnavi ;
	rnavi = &work->rnavi ;
	rnavi->p_action = rnavi->pa_action[(int)rnavi->next_node];
	rnavi->p_acttime = DIRECT_TICK(rnavi->pa_time[(int)rnavi->next_node]) ;
	rnavi->p_dir = rnavi->pa_dir[(int)rnavi->next_node];
	rnavi->p_con = rnavi->pa_con[(int)rnavi->next_node];
	rnavi->p_actstatus = rnavi->pa_flag[(int)rnavi->next_node];

	if(rnavi->p_acttime == P_TIME_WAIT){
		work->status |= ST_WAIT ;
	}

}

static void CYP_SetNextnode( rnavi, trgp )
ROUTENAVI	*rnavi ;
TRGPOINT	*trgp ;
{
	ENE_SetNextnode( rnavi ) ;
	trgp->pos = rnavi->nodes[ (short)rnavi->next_node ] ;
	trgp->map = rnavi->mapbit[ (short)rnavi->next_node ] ;
//printf("IN ENE_SetTrgpNextnode \n");
//printf("next_node %d \n",(short)rnavi->next_node);
//printf("trgp->map %X \n",trgp->map);
/*サイファの目標にアドレス不要なので一時削除*/
//	trgp->addr = HZX_GetAddress( trgp->map, &trgp->pos, -1 ) ;
}

static int DirectTrace3D(Work *work, float range){

	FVECTOR		shift ,*pos ;
	float		dx,dy, dz ;
	TRGPOINT *trgp ;
	CONTROL *ctrl ;

	trgp = &work->trg ;
	ctrl = &work->control ;

	pos = &ctrl->mov;
	shift.vx = dx = trgp->pos.vx - pos->vx ;
	shift.vy = dy = trgp->pos.vy - pos->vy ;
	shift.vz = dz = trgp->pos.vz - pos->vz ;

	if(
		(dx > -range)&&(dx < range) 
		&&(dy > -range)&&(dy < range) 
		&&(dz > -range)&&(dz < range)
	){
		/*到達*/
		trgp->dir = _FVecDir2( &shift ) ;
		ctrl->step = shift ;
		return -1 ;
	}
	trgp->dir = _FVecDir2( &shift ) ;
	GV_LenVec3F( &shift , &shift, 0.0F, work->speed ) ;
	ctrl->step = shift ;
	return 0 ;

}

static void MoveRoute(Work *work){
	ROUTENAVI *rnavi ;
	rnavi = &work->rnavi ;
	/*移動制御と到達チェック*/
	if(DirectTrace3D(work,work->speed) < 0 ){
		/*ポイント到達*/
		if(rnavi->p_acttime == 0 ){
	/*待機時間無しなのですぐに次ポイントへ*/
//			ENE_SetTrgpNextnode( rnavi, &work->trg ) ;
			CYP_SetNextnode( rnavi, &work->trg ) ;
			SetPointAction( work );
		}else {
	/*待機モードへ*/
			work->mode = ACTION_MODE ;
		}
	}else {
		/*進行方向*/
		work->control.turn.vy = work->trg.dir ;
		work->control.turn.vx = 0 ;
	}
}
static void ActionSeq(Work *work)
{
	if(work->rnavi.p_dir >=0 ) {
		work->control.turn.vy = work->rnavi.p_dir ;
		work->control.turn.vx = 0 ;
	}
	if(
	(work->rnavi.p_acttime>0)
	&&(work->rnavi.p_acttime != P_TIME_WAIT)
	){
/*待機時間中*/
		work->rnavi.p_acttime--;
	}else if(
	(work->rnavi.p_acttime <= 0)
	&&(work->rnavi.p_acttime != P_TIME_WAIT)
	){
/*待機終了 次のポイントセット*/
//		work->nowpos = work->trg.pos ;
		CYP_SetNextnode( &work->rnavi, &work->trg ) ;
		SetPointAction( work );
		work->mode = MOVE_MODE ;
	}else if (work->rnavi.p_acttime == P_TIME_WAIT){
		/*メッセージ受信するまで待機*/	
		if(!(work->status & ST_WAIT )){
			/*待機終了 次のポイントセット*/
//			work->nowpos = work->trg.pos ;
//			ENE_SetTrgpNextnode( &work->rnavi, &work->trg ) ;
			CYP_SetNextnode( &work->rnavi, &work->trg ) ;
			SetPointAction( work );
			work->mode = MOVE_MODE ;
		}
	}
}
/********************************/


static void SetObjs(Work *work){
//	SVECTOR rot ;
//	FVECTOR	pos;
	FMATRIX	world;
	int i,parent;

	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	DG_GetPos( &world ) ;
	DG_PutObjs( work->objs );
//	GM_GroupObjs( work->body.objs, work->control.map ) ;
	/*別パーツ*/
	if(work->status & ST_MULTI){
		for(i=0;i<work->objs->n_models; i++){
			parent = work->objs->objs[i].parent ;
			if(parent < 0){
				DG_SetPos( &world ) ;
			}else{
				DG_SetPos( &work->objs->objs[parent].world ) ;
			}
			DG_MovePos( &work->objs->objs[i].trans ) ;
		//	DG_RotatePos( &work->control.rot ) ;
			DG_GetPos( &work->objs->objs[i].world  ) ;
		}
	}
}
static void Act(Work *work)
{
	/*巡回中*/
	switch (work->mode){
		case MOVE_MODE :
			MoveRoute(work) ;
			break;
		case ACTION_MODE :
			work->control.step = DG_ZeroVector ;
			ActionSeq(work);
			break;
	}
//	work->control.step = DG_ZeroVector ;

	work->control.interp = 8 ;
	GM_ActControl(&work->control) ;
	DG_GetLightMatrix( &work->control.mov, work->lights );
	SetObjs(work);
//	RouteView(&work->rnavi) ;
}
static void Die(Work *work)
{
	HZX_FlashTrap( work->control.hzx_id, &work->control.evt ) ;
	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs );
	GM_FreeControl( &work->control);
}

static int GetResources(Work *work, int name, int where)
{
	DG_DEF  *def ;
	int model,flag;
//	SVECTOR	range ;
//	float	length ;
	work->name = name ;
	GM_InitControl( &work->control, name, where );
	GM_ConfigControlMessageCheck( &work->control ) ;
//	work->control.interp = 16 ;
	/*CONTROLフラグ*/
	work->control.skip_flag =
	(CTRL_SKIP_FLR_CHECK|CTRL_SKIP_SEG_CHECK
	|CTRL_SKIP_GET_ADDRESS
	|CTRL_SKIP_NEAR_CHECK|CTRL_SKIP_ONLINE_CHECK
	|CTRL_SKIP_TRAP
	); 
	if ( GCL_GetOption( 'k' ) != NULL ) {
		model = GCL_GetNextInt() ;
	}else {
		return 0 ;
	}
	/*ルート設定*/
	if ( GCL_GetOption( 'r' ) != NULL ) {
		work->route = GCL_GetNextInt() ;
		if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
			work->route += GM_RouteOffset ; 
		}

		ENE_InitRouteNavi( &work->rnavi, work->route, 0 );
		SetPointAction( work );
	}else {
		return 0 ;
	}

	if ( GCL_GetOption( 's' ) != NULL ) {
		work->speed = (float) GCL_GetNextInt() ;
	}
	if ( GCL_GetOption( 'f' ) != NULL ) {
		work->status = GCL_GetNextInt() ;
		if(work->status & ST_MULTI){
			flag = MULTI_FLAG ;
		}else {
			flag = ONE_PIECE_FLAG ;
		}
	}else {
		flag = ONE_PIECE_FLAG ;
	}
	/*ルートポイントに配置*/
	work->control.mov = work->rnavi.nodes[ 0 ] ;

	def = (DG_DEF*) GV_GetCache( GV_CacheID( model, 'k' ) ) ;
	work->objs = DG_MakeObjs( def, flag, DG_CHANL_MAIN ) ;
	DG_QueueObjs( work->objs ) ;
	DG_SetLightMatrix( work->objs, work->lights );
//	GM_ConfigControlMapID( &work->control ) ;
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	DG_PutObjs( work->objs );
	GM_GroupObjs( work->objs, where ) ;
	work->mode = ACTION_MODE ;

	return 1;
}

/* 初期化部メイン */
//chara	プットルートオブジェ[NewRoute3d] 
void *NewRoute3d( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
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

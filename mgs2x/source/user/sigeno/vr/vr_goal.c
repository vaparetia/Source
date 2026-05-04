//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vr_goal.c
	ＶＲステージゴール
	chara	ＶＲゴール
	2002/01/31 K.Sigeno
	$Id: vr_goal.c,v 1.1.1.3 2002/11/19 11:49:57 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"
#include "korekado/enemy/enemy.h"
#include "korekado/enemy/enemy.x"
#include	"vr.h"

//#define		SCALE_SPEED		(0.15f)
//#define		SCALE_SPEED		(0.015f)
//#define		SCALE_SPEED		(0.050f)
//#define		SCALE_SPEED		(0.025f)
//#define		SCALE_SPEED		(0.040f)
//#define		SCALE_SPEED		(0.10f)


extern float BP_AdjustTick3(float);
#define		SCALE_SPEED		( BP_AdjustTick3(0.02f) )
//#define		NULL_OBJ	(1)

/*ポイントタイムの値がこの時、mesg受信まで待機*/
#define		P_TIME_WAIT	(-3)	
//#define		P_TIME_WAIT	(150)	
#define		BODY_SIZE_X		(250.0f)
#define		BODY_SIZE_Y		(250.0f)
#define		BODY_SIZE_Z		(250.0f)
#define		HEAD_OFFSET		(500.0f)
/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	DG_OBJS		*objs ;
	FMATRIX		lights[2] ;
	FMATRIX		amb_lights[2] ;
	CONTROL		control;
	ROUTENAVI	rnavi;	/*ルート誘導*/
	TRGPOINT	trg;	/*移動目標情報*/
	FVECTOR		pos ;
	int			route ; /*ルート番号*/
	int			name ;
	int			check ;
	float		speed ; 
	RADAR_CTRL	rctrl ;
	float		scale ;

	short		mode ;
	short		status ;
	short		rot ;
	short		pad ;
} Work ;
enum {
	MOVE_MODE,ACTION_MODE
};
enum {
	MSG_ROUTE_CHANGE = 1
};
enum {
	ST_NORMAL = 0x00,
	ST_WAIT = 0x02,
	ST_MOVE = 0x04,
};



static int	CheckPL_inside(Work *) ;
static void	pl_hit(Work *) ;


#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern void RouteView(ROUTENAVI *) ;

#endif

#define	OBJ_FLAG		(DG_FLAG_SHADE|DG_FLAG_NOFOG|DG_FLAG_FINISHCALC|DG_FLAG_LATTERDRAW|DG_FLAG_IRREACTION)


static void SetAmb( Work *work ,float amb){
	work->amb_lights[1].m[ 3 ][ 0 ] = amb ;
	work->amb_lights[1].m[ 3 ][ 1 ] = amb ;
	work->amb_lights[1].m[ 3 ][ 2 ] = amb ;
}
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
	if(GM_VRStatus & GM_VR_IDLE){
		work->control.step = DG_ZeroVector ;
		return ;
	}
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

#define DEF_AMB (127.0f)
static void SetObjs(Work *work){
//	SVECTOR rot ;
	FVECTOR	scale ;
	FMATRIX	world;
	int i,parent;

	scale.vx = scale.vy = scale.vz = work->scale ;
	scale.vw = 1.0f;

	DG_SetPos2( &work->control.mov, &DG_ZeroSVector ) ;

	DG_GetPos( &world ) ;
	DG_PutObjs( work->objs );
//	GM_GroupObjs( work->body.objs, work->control.map ) ;
	/*別パーツ*/
	if(1){
		for(i=0;i<work->objs->n_models; i++){
			parent = work->objs->objs[i].parent ;
			if(parent < 0){
				DG_SetPos( &world ) ;
			}else{
				DG_SetPos( &work->objs->objs[parent].world ) ;
			}
			DG_MovePos( &work->objs->objs[i].trans ) ;

			if(i>0){
//				if((VR_GOAL_FLAG & VR_GOAL_OPEN )&&(!(VR_GOAL_FLAG & VR_GOAL_IN))){
				if(VR_GOAL_FLAG & VR_GOAL_OPEN ){
					GM_RadarSetFlag( &work->rctrl, RADAR_VISIBLE ) ;
					if(work->scale > 0.0f){
						work->objs->objs[i].flag &= ~DG_FLAG_INVISIBLE ;
					}else {
						work->objs->objs[i].flag |= DG_FLAG_INVISIBLE ;
					}
//					DG_ScalePos(&scale);
					if(VR_GOAL_FLAG & VR_GOAL_IN){
					/*アンビエント減衰でフェードアウト*/
						SetAmb(work,work->scale * DEF_AMB) ;
//						SetAmb(work,work->scale * 127.0f) ;
					}else {
						SetAmb(work,DEF_AMB) ;
					}
				}else {
					work->objs->objs[i].flag |= DG_FLAG_INVISIBLE ;
				}
			}
			if(i==1){
//			if(1){
				SVECTOR rot ;
				rot.vx = 0 ;
				rot.vy = work->rot ;
				rot.vz = 0 ;
				DG_RotatePos( &rot ) ;
			}
			DG_GetPos( &work->objs->objs[i].world  ) ;
		}
	}
}
static void Act(Work *work)
{
	FVECTOR		t_pos ;
//	SVECTOR	rgb ;
	/*巡回中*/

	if(work->status & ST_MOVE){
		switch (work->mode){
			case MOVE_MODE :
				MoveRoute(work) ;
				break;
			case ACTION_MODE :
				work->control.step = DG_ZeroVector ;
				ActionSeq(work);
				break;
		}
		work->control.interp = 8 ;
		GM_ActControl(&work->control) ;
	}
#if 0
	if(VR_GOAL_FLAG & VR_GOAL_IN){
		/*アンビエント減衰で非表示にするため マトリックス切り替え*/
//		DG_SetLightMatrix( work->objs, work->amb_lights );
		work->objs->objs[1].light = work->amb_lights;
		work->objs->objs[2].light = work->amb_lights;
	}else {
		DG_GetLightMatrix( &work->control.mov, work->lights );
	}
#else
	/*常に独自シェード*/
	/*土台*/
	work->objs->objs[0].light = work->lights;
	/*回転部分*/
	work->objs->objs[1].light = work->amb_lights;
	work->objs->objs[2].light = work->amb_lights;
#endif
	if((VR_GOAL_FLAG & VR_GOAL_IN)&&(work->scale>0.0f)){
		work->scale -= SCALE_SPEED ;
		if(work->scale <0.0f){
			work->scale = 0.0f ;
		}
#if 0
	}else if((VR_GOAL_FLAG & VR_GOAL_OPEN )&&(work->scale<1.0f)){
		work->scale += SCALE_SPEED ;
		if(work->scale >1.0f){
			work->scale = 1.0f ;
		}
#endif
	}
	SetObjs(work);

	t_pos = work->control.mov ;
	t_pos.vy += 250.0f ;
	if(VR_GOAL_FLAG & VR_GOAL_OPEN ){
#if 0
		{
			rgb.vx = 0 ;
			rgb.vy = 0 ;
			rgb.vz = 127 ;
			PosBox(&t_pos ,250.0f , &rgb );
		}
#endif
		if(CheckPL_inside(work)){
			pl_hit(work) ;
		}
	}else {
#if 0
		{
			rgb.vx = 127 ;
			rgb.vy = 0 ;
			rgb.vz = 0 ;
			PosBox(&t_pos ,250.0f , &rgb );
		}
#endif
	}
	work->rot += 16 ;
	work->rot &= 4095 ;
//	RouteView(&work->rnavi) ;
}
static void Die(Work *work)
{
	HZX_FlashTrap( work->control.hzx_id, &work->control.evt ) ;
#ifndef NULL_OBJ
	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs );
#endif
    GM_FreeRadarControl( & work->rctrl ) ;
	GM_FreeControl( &work->control);
}
#define GOAL_WIDE	(500.0f)
#define GOAL_HIGH	(1750.0f)

static int	checkplpos(Work *work){
	FVECTOR	gpos,ppos;
	gpos = work->control.mov ;
	ppos = GM_PlayerPosition ;
	if((ppos.vx >(gpos.vx+GOAL_WIDE))
		||(ppos.vx <(gpos.vx-GOAL_WIDE))){
		return 0;
	}
	if((ppos.vz >(gpos.vz+GOAL_WIDE))
		||(ppos.vz <(gpos.vz-GOAL_WIDE))){
		return 0;
	}
	if((ppos.vy >(gpos.vy+GOAL_HIGH))
		||(ppos.vy <(gpos.vy))){
		return 0;
	}
	return 1 ;
}
static int	CheckPL_inside(Work *work){
	if((GM_IsGameOver())||(GM_PlayerStatus&(PLAYER_DEAD|PLAYER_DAMAGED|PLAYER_BEYOND|PLAYER_FORCE)) ){
		return 0 ;
	}
	if(checkplpos(work)){
		if(work->check){
			return 0 ;
		}else {
			work->check = 1 ;
			return 1 ;
		}
	}else {
		work->check = 0 ;
		return 0 ;
	}
}
static void	pl_hit(Work *work)
{
//	char str[16] ;
//	str = "goal" ;
//	sprintf(str,"hitpos %d",(int)def->hit.vy) ;
	VR_GOAL_FLAG |= VR_GOAL_IN ;
}
static int GetResources(Work *work, int name, int where)
{
	DG_DEF  *def ;
	int model,buf[3];
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
		model = GV_StrCode("ration_ibox_sh") ;
//		return 0 ;
	}
	/*ルート設定*/
	if ( GCL_GetOption( 'r' ) != NULL ) {
		work->route = GCL_GetNextInt() ;
		if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
			work->route += GM_RouteOffset ; 
		}
		ENE_InitRouteNavi( &work->rnavi, work->route, 0 );
		SetPointAction( work );
		work->status |= ST_MOVE ;
	}else {
		if ( GCL_GetOption( 'p' ) != NULL ) {
			GCL_GetNextIV( buf ) ;
			work->pos.vx = (float) buf[0] ;
			work->pos.vy = (float) buf[1] ;
			work->pos.vz = (float) buf[2] ;
		}
	}

	if ( GCL_GetOption( 's' ) != NULL ) {
		work->speed = (float) (DIRECT_TICK(GCL_GetNextInt())) ;
	}
	if ( GCL_GetOption( 'f' ) != NULL ) {
		work->status = GCL_GetNextInt() ;
	}

	/*ルートポイントに配置*/
	if(work->status & ST_MOVE){
		work->control.mov = work->rnavi.nodes[ 0 ] ;
	}else {
		work->control.mov = work->pos ;
	}

	def = (DG_DEF*) GV_GetCache( GV_CacheID( MDL_VR_GOAL, 'k' ) ) ;
	work->objs = DG_MakeObjs( def, OBJ_FLAG, DG_CHANL_MAIN ) ;


	DG_QueueObjs( work->objs ) ;

#if 0
	DG_SetLightMatrix( work->objs, work->lights );
#else
	work->lights[1].m[ 3 ][ 0 ] = 160 ;
	work->lights[1].m[ 3 ][ 1 ] = 160 ;
	work->lights[1].m[ 3 ][ 2 ] = 160 ;
#endif
	GM_ConfigControlMapID( &work->control ) ;
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
#ifndef NULL_OBJ
	DG_PutObjs( work->objs );
	GM_GroupObjs( work->objs, where ) ;
#endif
	work->mode = ACTION_MODE ;
	work->check = 0 ;
//	work->scale = 0.0f ;
	work->scale = 1.0f ;

	GM_InitRadarControl( &work->rctrl, &work->control.mov, RADAR_VISIBLE|RADAR_NODE, -1 );
	work->rctrl.col = RADAR_COLOR_BLUE ;
	GM_RadarSetVRange( &work->rctrl, RADAR_VRANGE_UPPER , RADAR_VRANGE_LOWER );
	GM_RadarResetFlag( &work->rctrl, RADAR_VISIBLE ) ;

	return 1;
}

/* 初期化部メイン */
//chara	プットルートオブジェ[NewRoute3d] 
void *NewVRGoal( name , where )
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

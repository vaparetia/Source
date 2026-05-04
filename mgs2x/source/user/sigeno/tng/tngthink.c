//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	tngthink.c
	2001/02/06 K.Sigeno
	$Id: tngthink.c,v 1.1.1.3 2002/11/19 11:49:54 Yoshizawa1 Exp $
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
#include	"korekado/conv/korekado.x"
#include "korekado/enemy/enemy.h"
#include	"libutl.h"

#include	"../attacker/attacker.h"

#include	"../attacker/sig_conv.x"

#include	"tng.h"

#include	"korekado/enemy/motion.h"


#define		NEAR_SHOOT_DELAY	30
//#define		WAIT_SHOOT_DELAY	90
#define		WAIT_SHOOT_DELAY	30
//#define		RUN_SHOOT_DELAY		90
#define		RUN_SHOOT_DELAY		15
//#define HANG_ATTACK_ON
extern int TNG_AlertCheckPad( ACTION * ) ;

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern void SigZoneView( int , SVECTOR * ,float ) ; 

#endif

extern void CheckAttackTarget(ENETHINK * ) ;


#if 0
#define RANK_DIS 1500 /* 車間距離*/
#else
#define RANK_DIS 1000 /* 盾兵実験用*/
#endif

#if 0
#define MUKA_DIS 480 /* 車間距離*/
#define MUKA_LDIS 450 /* 車間距離*/
#define MUKA_STOP 200 /* 車間距離*/
#else
#define MUKA_DIS 550 /* 車間距離*/
#define MUKA_LDIS 500 /* 車間距離*/
#define MUKA_STOP 400 /* 車間距離*/
#endif

#define TEST_LEN 400 /*視界チェックの左右幅*/
#define ATTACK_TIME (AT_THK_RATE*20) /*安全地帯から突撃開始の時間*/

#define AT_SET_POS (0) /* 座標指定配置実験*/

#define TURN_WAIT (AT_THK_RATE*2)  /*振り向き時間*/

/*対ビヨンド*/
//#define STOMP_DIS (1400)
#define STOMP_DIS (1000.0F)
#define BEYOND_NEAR (STOMP_DIS + 500.0F)


#include "../attacker/check_at.c"


static int CheckJumpAttackCondition(ENETHINK *entk){
	TNG_COM		*tng_com ;
	ENTK_TENG_A		*tng_a;
	
	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

	if(
	(entk->act->bodyp.type & ENE_TYPE_KATANA )&&
	( entk->pl_eyei.dis > KATANA_JUMP_ATTACK_DIS )
	&&( entk->pl_eyei.dis < KATANA_JUMP_ATTACK_DIS+1000 )
	&&(entk->ctrl->mov.vy < (tng_com->base_floor +2000.0f))
	&&(tng_com->jump_attack==0)
	) {
		return 1 ;
	}
	return 0 ;
}
static void GoHangAttack(ENETHINK *entk){
	entk->think2 = TH2_ATTACK ; 
	entk->think3 = TH3_ATTACK_HANG ;
	entk->count3 = 0 ;
}
static void GoJumpAttack(ENETHINK *entk){
	entk->think2 = TH2_ATTACK;
	entk->think3 = TH3_JUMP_ATTACK ; 
	entk->count3 = 0 ;
}
static void GoWaitMode(ENETHINK *entk){
	entk->think2 = TH2_ATTACK ;
	entk->think3 = TH3_WAIT ;
}

static int CheckX_FireTng(ENETHINK *entk){
	AT_THK *at_thk ;
	E_UNIT *e_unit ;
	ENETHINK	*trgentk;
	int i,subdir;

	at_thk = (AT_THK *)entk->character ;
#if 0
	if(
	(at_thk->zone_dis >= (entk->at_com->chasedis + 1000) )
	) {
		return 0 ;
	}
#endif
	if(at_thk->dis_rank>0){
		e_unit = entk->com->enemys.
		group[entk->g_id]->unit[entk->u_id] ;
		for(i=0;i<e_unit->enemy_num;i++){
			if(i==entk->id) continue ;
			trgentk = e_unit->entk[i];
			if(entk->pl_eyei.dis < trgentk->pl_eyei.dis){
				continue ;
			}
			subdir = SIG_CheckDirSub(entk->pl_eyei.dir,trgentk->pl_eyei.dir) ;
			if( X_FIRE_DIR*2 > abs(subdir) ){
				return 1;
			}

		}
	}
	return 0 ;
}

static int CheckTngPrudence(ENETHINK *entk){
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;


	if(
	(entk->count3 > (AT_THK_RATE*2))&&(at_thk->dis_rank>0)
	&&(CheckX_FireTng(entk))
	){
//		printf("Go Deploy\n");
		GoDeploy(entk);
		return 1;
	}
//	printf("No Deploy\n");
	return 0;
}

//#define CENT_LIM (128)
#define CENT_LIM (200)

static int CheckPlayerSightTng( ENETHINK *entk ,int mode ,int p_sight)
{
	short p_dir,eye_dir,sub;
	int pl_wp ;
/*TEST*/
	return 0 ;

/*******************/
	pl_wp = PL_GetPlayerWeapon( ) ;
	if(GM_PlayerStatus &PLAYER_DEAD) return 0;
	if((GM_PlayerStatus & PLAYER_HOLD)
		&&(pl_wp)
	){
		/*視界外なら無効*/
		if((mode)
		&&
//		( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN )
		(
		(ENE_ReadOnlinInfo(entk->znavi->next_addr,GM_PlayerAddress))
		||( entk->pl_eyei.dis > entk->sense.eye_s )
		)
		){
			return 0;
		}
		eye_dir = entk->pl_eyei.dir&4095;
		p_dir = GM_PlayerControl->turn.vy&4095;
		sub = SIG_CheckDirSub( p_dir,eye_dir);
		if ( (sub > (2048-p_sight) ) || (sub < (-2048+p_sight) ) ){
		/*範囲内*/
			if((pl_wp!=WP_Rgb6)&&(pl_wp!=WP_Stinger)
			&&(entk->act->bodyp.type & ENE_TYPE_KATANA )
			){
				if ( (sub > (2048-p_sight+CENT_LIM) ) || (sub < (-2048+p_sight-CENT_LIM) ) ){
					return 0 ; 
				}
			}
			if(sub>0) {
				return RIGHT_WALL; /*左へ行け*/
			}else {
				return LEFT_WALL; 
			}
		}
	}
	return 0;
}

#define TNG_DEF_LIMIT	(700000.0f)
static int CheckTngDefPosMode(ENETHINK *entk){
	float len ;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;
	if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
		/*プレイヤが見えないが守備位置が近ければ守備展開*/
		if(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN){
			if(at_thk->at_status & AT_ST_DEFENSE ){
#if 1
				return 1 ;
#else
				len = GV_VecLen3F2( &entk->def_pos, &GM_PlayerPosition ) ;
				if(len < TNG_DEF_LIMIT){
					return 1 ;
				}
#endif
			}
		}
		return 0 ;
	}else {
		return (at_thk->at_status & AT_ST_DEFENSE );
	}
}

/*補助思考 近接での思考分岐*/
static int ThinkSubNear( entk )
ENETHINK	*entk ;
{
	int reach,rank_dis = 2000;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;
	/*段差近くでは車間距離を広くする*/
	if(CheckFlrAtr(entk))
	{
		rank_dis += 500;
	}
#if 1
	if(entk->count3 < AT_THK_RATE ){
		entk->act->pad = SP_READYGUN ;
		return 0;
	}
#endif
/*****/
	/*手前が近い*/
	if((at_thk->dis_dif < rank_dis  )&&(at_thk->dis_rank > 0))
	{
		if(entk->think3 == TH3_WAIT_CHASE) return 0;
		/*待機モード*/
		entk->think2 = TH2_CHASE ; 
		entk->think3 = TH3_WAIT_CHASE ;
		entk->count3 = 0 ;
		return 1;
	}

	/*追跡再開 プレイヤがゾーン外の時注意*/
	if((at_thk->dis_rank == 0 )&&(at_thk->in_sight < AT_THK_RATE*2)
//	&&(HZX_Zone1(entk->com->plzone_in_zone[0]) != HZX_Zone1(entk->ctrl->addr) )
//	&&(entk->com->plzone_in_zone[0]  != entk->ctrl->addr )
	&&(*entk->pl_eyei.addr  != entk->ctrl->addr )
	){
		entk->think2 = TH2_CHASE ; 
//		entk->think3 = TH3_DIRECT_CHASE ;
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return 1;
	}
	/*手前が遠い*/
	/*06.01後方待機実験*/
	if(
	( at_thk->dis_dif >= (rank_dis+500 )  )
	&&(!((at_thk->dis_rank > 1)&&(at_thk->at_status & AT_ST_NEXT)))
	){
/**/
		/*追跡モード*/
#if 1 
/*最後にいたゾーン*/
reach = HZX_ReachTo( entk->ctrl->addr, *entk->pl_eyei.addr );
#else
/**/
reach = HZX_ReachTo( entk->ctrl->addr, GM_PlayerAddress );
#endif
		if(reach < HZX_UNREACH){
			if(entk->think3 == TH3_DIRECT_CHASE) return 0;
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_DIRECT_CHASE ;
			entk->count3 = 0 ;
			return 1;
		}else {
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_ZONE_CHASE ;
			entk->count3 = 0 ;
			return 1;
		}
	}
	/*現状維持*/
	entk->act->pad = SP_READYGUN ;
	return 0;
}
static int ThinkBeyond(entk)
ENETHINK *entk ;
{
	if((entk->pl_eyei.
	sight != EYE_INFO_SIGHT_IN)
	&&(GM_PlayerStatus & PLAYER_BEYOND ) )
	{
		{
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_WAIT_CHASE ; 
		}
		entk->count3 = 0 ;
		entk->act->move_s = MoveAttackRun ;
		return 1;
	}
	return 0;
}
/*----- 思考ルーチン --------------------------------------------------*/
/*----- 低レベル思考モード --------------------------------------------*/

static void Think3_ZoneChasePlayer( entk )
ENETHINK	*entk ;
{
//	int reach,rank_dis = 1000,aim_check = 0;
	int rank_dis = 1000;
	AT_THK *at_thk;
	TNG_COM		*tng_com ;
	ENTK_TENG_A		*tng_a;
	
	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

//printf("Think3_ZoneChasePlayer ID [%d]\n",entk->id);
#if 0
	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
#else
	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));
#endif
	entk->act->dir = entk->ctrl->turn.vy ;

/*床の状態によって距離を変化*/
	if(CheckFlrAtr(entk)){
		rank_dis += 500;
	}
	if(at_thk->dis_rank > 1){
		rank_dis += 1000;
	}
/*
	if(at_thk->dis_rank == 2){
		rank_dis += 2000;
	}
*/

#if 0
	SetAimPosPlayer(entk,0);
#else
	SetAimPosEyei(entk) ;
#endif
#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			/*めくら時の処理*/
			if( at_thk->zone_dis > entk->at_com->chasedis+1000)
			{
				entk->act->pad = SP_READYGUN;
				entk->count3 =0;
				entk->think2 =TH2_CHASE;
				entk->think3 =TH3_WAIT_CHASE;
				entk->act->dir = entk->ctrl->turn.vy ;
				return ;
			}
		}
	}
#endif

	entk->act->pad = SP_MOVE_RUN ;
	entk->act->dir = entk->ctrl->turn.vy ;
	at_thk->at_status &= (~AT_ST_SQUAT);

#if 0
	/*眠り味方救助*/
	if(CheckLastAT(entk)){
		if(CheckRescueAll(entk)){
			return ;
		}
	}
#endif







	/*現在地アドレス更新*/
	if(!(entk->count3%(AT_THK_RATE*5))) {
		entk->znavi->this_addr = 
		HZX_GetAddress( entk->ctrl->hzx_id, 
		&( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;
	}
	/*手前が近けりゃ待機状態*/
	/*06.01 後方待機*/
	if(
	(( at_thk->dis_dif < rank_dis  )&&(at_thk->dis_rank > 0))
//	||((at_thk->dis_rank > 1)&&(at_thk->at_status & AT_ST_NEXT))
	){
		/*対イントルード*/
		if(
		(entk->at_com->watch_status & AT_COM_WATCH_INTRUDE )
		){
			if(at_thk->dis_rank ==0){
				GoIntZoneChase(entk);
			}else {
				GoIntWait(entk);
			} 
			return ;
		}

		entk->think3 = TH3_WAIT_CHASE ; 
		entk->count3 = 0 ;
		return ;
	}


	/*目標が近けりゃ攻撃 */
	/*先頭兵はつねにゾーン距離を詰める*/
	if(
	((at_thk->in_sight>(AT_THK_RATE*2))||(at_thk->at_status & AT_ST_FEEL))
	&&
	(
	( 
//	(at_thk->zone_dis < entk->at_com->chasedis )
	(entk->pl_eyei.dis < entk->at_com->chasedis )
	&&(!(entk->at_com->watch_status & AT_COM_WATCH_BEYOND))
	)
/**2000.08.07**/
//	||
//	((at_thk->dis_rank>0)&&(entk->pl_eyei.dis < entk->at_com->chasedis )) 
	)
	){
		/*おしゃべり実験*/
		/*追跡から停止*/
		if((irnd()>>8)%2){
#if 0
			int voice ;
			int rnd ;
			rnd = ((irnd()>>8)%3);
			voice = EV_STOP_1 ;
		if(entk->com->enemys.group[entk->g_id]->
			unit[entk->u_id]->enemy_num>0){
			COM_SetSpeak( voice, entk ) ;
		}
#endif
		}
#if 0
		SetAimPosPlayer(entk,0);
#else
		SetAimPosEyei(entk) ;
#endif
		GoNearAttack(entk);
		return ;
	}

	/*飛びつき攻撃*/
	if(CheckJumpAttackCondition(entk)) {
		tng_com->jump_attack = TNG_JUMP_ATTACK_CNT;
		GoJumpAttack(entk);
		return ;
	}
/*コーナー判定*/
#if 1
	if((entk->count3>AT_THK_RATE))
	{
		if(
		(!(entk->act->bodyp.type & ENE_TYPE_TNG_A))
		){
			if(Checkout(entk)){
				 return;
			}
		}
	}
#endif
#if 0
	if(
	(at_thk->dis_rank > 1)&&(entk->count3%(AT_THK_RATE*3) == 0 )
	&&( entk->pl_eyei.dis < entk->sense.eye_s-1000 )
	){
		if(!HZX_OnlineHazardCheck(entk->ctrl->hzx_id,&entk->ctrl->mov,
		&GM_PlayerFindPos ,
		HZX_CHK_ALL,HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY ))
		{
			/*向きを変えれば見える位置なら向きを変える*/
			entk->act->dir = entk->pl_eyei.dir ;
			entk->think3 = TH3_WAIT_CHASE ; 
			entk->count3 = 0 ;
			return ;
		}
	}
#endif
//CheckJumpPos
/*ゾーン追跡*/
//intrp
	if(CheckJumpPosENE(entk)){

//printf("ZONE CHASE PLAYER TRG Y [%f]\n",entk->act->target_pos.vy);
		if((tng_a->type >= TNG_TYPE_C)&&(entk->act->target_pos.vy > entk->ctrl->mov.vy)){
			if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
				SIG_SetChasePos(entk ,&entk->tmp_pos) ;
				entk->think2 = TH2_CHASE ;
				entk->think3 = TH3_DIRECT_CHASE_JUMP ;
				entk->count3 = 0 ;
				return ;
			}
		}else {
			SIG_SetChasePos(entk ,&entk->tmp_pos) ;
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_DIRECT_CHASE_JUMP ;
			entk->count3 = 0 ;
			return ;
		}
	}
	if ( 
//	(ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) < 0 )
	(ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) < 0 )
	||(SIG_CheckRIntrpt(entk))
	){
/*ダイレクト追跡に移行*/
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		return ;
	}
	/*走りながらも攻撃*/
	if(!(entk->act->bodyp.type & ENE_TYPE_KATANA )){
		if(
		( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )
		&&(entk->count3>DIRECT_TICK(RUN_SHOOT_DELAY))
		){
			entk->bullet = 0;
			if(tng_com->now_hang_num == 0){
				RandShoot(entk);
			}
		}
	}
	
	if((entk->count3%(AT_THK_RATE*2)==0)
//	||(entk->at_com->Pl_StayTime > 60 )
	){
		if(
		(entk->count3 < (AT_THK_RATE*5)) /*動きはじめはゆっくり*/
		&&(at_thk->zone_dis <= entk->sense.eye_s )
//		&&(at_thk->zone_dis <= entk->sense.eye_s*2))
		)
		{
			SetMoveMode(entk);
		} else {
			/*****
			後退時にはアクションを前進に切り替えた瞬間の
			方向切り替えの補完による遅れから一瞬後退してしまい
			追跡モード切り替えしきい値前後を振動する現象が起きる
			これを防止するため、後退から前進への変化する時のみ
			しきい値を大き目に取る
			******/
			if((entk->act->move_s == MoveBack) 
//			&&(at_thk->zone_dis <= (entk->sense.eye_s+2000))
			){
				SetMoveMode(entk);
			}else {
				if(entk->act->move_s != MoveAttackRun ){
					entk->act->tmp_time = 12 ;
				}
				entk->act->move_s = MoveAttackRun ;
			}
		}
	}

/*目標に近くなれば 強制的にゆっくり歩き*/
	if(entk->act->move_s == MoveAttackRun) {
		entk->act->dir = entk->trgpoint.dir ;
//		if( at_thk->dis_dif < rank_dis+1000 ) {
		if( at_thk->dis_dif < rank_dis ) {
			entk->act->move_s = MoveCautionWalk ;
		}
	} else {
	/*体はプレイヤ方向移動は目的ゾーン方向*/
		entk->act->tmp_dir = entk->trgpoint.dir ;
		entk->act->dir = entk->pl_eyei.dir ;
	}
	if(CheckDeathBed(entk) ){
		return ;
	}
	entk->count3 ++ ;
}

static void Think3_ZoneChasePlayerHang( entk )
ENETHINK	*entk ;
{
//	int reach,rank_dis = 1000,aim_check = 0;
//	int rank_dis = 1000;
	AT_THK *at_thk;
	ENTK_TENG_A *tng_a ;
	TNG_COM		*tng_com ;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

//hang_num
#if 0
	if(tng_a->def_level > tng_com->hang_level){
		/*待機*/
		entk->act->dir = entk->ctrl->turn.vy ;
		entk->act->pad = SP_SQUATHIDE ;
		return ;
	}
#endif
	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));
	SetAimPosEyei(entk) ;
	entk->act->dir = entk->ctrl->turn.vy ;
	entk->act->pad = SP_MOVE_RUN ;
	at_thk->at_status &= (~AT_ST_SQUAT);
	/*現在地アドレス更新*/
	if(!(entk->count3%(AT_THK_RATE*5))) {
		entk->znavi->this_addr = 
		HZX_GetAddress( entk->ctrl->hzx_id, 
		&( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;
	}
	/*目標が近けりゃ攻撃 */
	if(entk->pl_eyei.dis < HANG_DIS ){
		GoHangAttack(entk) ;
		return ;
	}
/*ゾーン追跡*/
	if(CheckJumpPosENE(entk)){
		SIG_SetChasePos(entk ,&entk->tmp_pos) ;
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_DIRECT_CHASE_JUMP ;
		entk->count3 = 0 ;
		return ;
	}
	if ( 
	(ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) < 0 )
	||(SIG_CheckRIntrpt(entk))
	){
/*ダイレクト追跡に移行*/
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		return ;
	}
	entk->act->move_s = MoveAttackRun ;
	entk->act->tmp_dir 
		= entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}


/*座標追跡 プレイヤ直接*/
static	void	Think3_DirectChasePlayer( entk )
ENETHINK	*entk ;
{
	int		reach,rank_dis = 1000 ;
	TNG_COM		*tng_com ;
	ENTK_TENG_A	*tng_a ;
	AT_THK *at_thk;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

	entk->act->dir = entk->ctrl->turn.vy ;
#if 0
	SetAimPosPlayer(entk,0);
	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
#else
	SetAimPosEyei(entk) ;
	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));
#endif
	if( CheckFlrAtr(entk) ){
		rank_dis += 500;
	}
//	if(at_thk->dis_rank > 1){
//		rank_dis += 1000;
//	}


#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			/*めくら時の処理*/
			if( at_thk->zone_dis > entk->at_com->chasedis+1000)
			{
				entk->act->pad = SP_READYGUN;
				entk->count3 =0;
				entk->think2 =TH2_CHASE;
				entk->think3 =TH3_WAIT_CHASE;
				entk->act->dir = entk->ctrl->turn.vy ;
				return ;
			}
		}
	}
#endif

	entk->act->dir = entk->ctrl->turn.vy ;


	/*対ビヨンド思考に移行*/
	if(entk->at_com->watch_status & AT_COM_WATCH_BEYOND)
	{
		/*見失ってない*/
		if((entk->at_com->alert_time%(AT_THK_RATE*2)==0)&&(at_thk->dis_rank==0)){
			SetMoveMode(entk);
			entk->act->dir = entk->trgpoint.dir ;
			entk->act->pad = SP_MOVE_RUN ;
			entk->think2 = TH2_CHASE;
			entk->think3 = TH3_CHASE_BEYOND ;
			entk->count3 = 0 ;
			return ;
		}
	}
	/*対ロッカー停止条件*/
	if(
	(entk->at_com->watch_status & AT_COM_WATCH_LOCKER)
	&&(!(GM_PlayerStatus & PLAYER_FORCE))
	&&(at_thk->dis_rank == 0 )
	&&(at_thk->zone_dis < entk->at_com->chasedis )
	){
#if 0
		GoEasyAttack(entk,DIRECT_TICK(180));
#else
		if(GoAttackLocker(entk) ){
			return ;
		}
#endif
	}


	at_thk->at_status &= (~AT_ST_SQUAT);
	//PLAYER_BEYOND	/*ビヨンドモード中*/
	if(ThinkBeyond(entk)) return ;



	/*手前が近けりゃ*/

//	if(( at_thk->dis_dif < rank_dis  )&&(entk->reach < HZX_UNREACH ) )
	if(( at_thk->dis_dif < rank_dis  )
	||(entk->at_com->watch_status & AT_COM_WATCH_INTRUDE )
	){
		/*対イントルード*/
		if(
		(entk->at_com->watch_status & AT_COM_WATCH_INTRUDE )
		){
			if(at_thk->dis_rank ==0){
				GoIntZoneChase(entk);
			}else {
				GoIntWait(entk);
			} 
			return ;
		}
		if(at_thk->dis_rank > 0){
			entk->think3 = TH3_WAIT_CHASE ; 
			entk->count3 = 0 ;
			entk->act->move_s = MoveAttackRun ;
			return ;
		}
	}



	if(CheckJumpPosENE(entk)){
/*ここで目標距離調べて直接ジャンプすればいいかな？*/
		SIG_SetChasePos(entk ,&entk->tmp_pos) ;
		if(GV_VecLen3F2( &entk->znavi->flore_pos, &entk->trgpoint.pos )< 250.0F){
			/*直接ジャンプ*/
			if(entk->tmp_pos.vy == entk->act->target_pos.vy){
				entk->act->pad = SP_TNG_JUMP_LEVEL ;
			}else {
				entk->act->pad = SP_TNG_JUMP ;
			}
			entk->act->dir = (int) entk->tmp_pos.vw ;
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_START_JUMP ;
			entk->count3 = 0 ;
			return ;
		}else {
			/*踏み切り位置まで移動*/
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_DIRECT_CHASE_JUMP ;
			entk->count3 = 0 ;
			return ;
		}
	}

	/*追跡のための方向設定と距離判定*/
	ENE_DirectTrace( &(entk->trgpoint), &(entk->znavi->flore_pos), (0) ) ;
/*STOP*/
	if(
	( entk->pl_eyei.dis < entk->at_com->chasedis )
	&&(!(GM_PlayerStatus & PLAYER_BEYOND ) )
	)
	{
	/*攻撃距離内*/
		GoNearAttack(entk);
		entk->act->dir = entk->ctrl->turn.vy ;
		return ;
	}

	/*距離外*/
	/*32フレON/OFF 100000*/
//	if( entk->count3 & 0x20){
	if( entk->count3 ==0){
		/*現在値 計算し直し*/
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), -1 ) ;
	}
	/* いつまでも直線じゃいられない */
	entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;

	reach = HZX_ReachTo( entk->znavi->this_addr, entk->trgpoint.addr );


	if(
	( reach > HZX_INDIRECT_REACH )
	){
	/*ゾーン到達してない*/
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
//		entk->act->dir = entk->ctrl->turn.vy ;
		return ;
	}

	/*走りながらも攻撃*/
	if(at_thk->dis_rank < 2){
		if( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ){
#if 0
			SetAimPosPlayer(entk,0);
#else
	SetAimPosEyei(entk) ;
#endif
			if(
			(!(entk->act->bodyp.type & ENE_TYPE_KATANA ))
			&&(entk->count3>DIRECT_TICK(120))
			){
				if(tng_com->now_hang_num == 0){
					RandShoot(entk);
				}
			}
		}
	}	

	if(HZX_Zone2(*entk->pl_eyei.addr) !=255){
		/*追跡可能*/
		entk->act->pad = SP_MOVE_RUN ;
		SetMoveMode(entk);
		entk->act->dir = entk->trgpoint.dir ;
	}else {
		entk->act->pad = SP_READYGUN ;
		entk->act->dir = entk->pl_eyei.dir ;
//		entk->act->dir = entk->trgpoint.dir ;
	}
	entk->count3 ++ ;
}

static	void	Think3_DirectChasePlayerHang( entk )
ENETHINK	*entk ;
{
	int		reach ;
	AT_THK *at_thk;
	at_thk = (AT_THK *)entk->character ;
	entk->act->dir = entk->pl_eyei.dir ;
	SetAimPosEyei(entk) ;
	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));
	entk->act->dir = entk->ctrl->turn.vy ;
	/*追跡のための方向設定と距離判定*/
	ENE_DirectTrace( &(entk->trgpoint), &(entk->znavi->flore_pos), (0) ) ;
	if( entk->pl_eyei.dis < HANG_DIS ){
		/*攻撃距離内*/
		GoHangAttack(entk);
		return ;
	}
	/*距離外*/
	/*32フレON/OFF 100000*/
//	if( entk->count3 & 0x20){
	if( entk->count3 ==0){
		/*現在値 計算し直し*/
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), -1 ) ;
	}
	/* いつまでも直線じゃいられない */
	entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;
	reach = HZX_ReachTo( entk->znavi->this_addr, entk->trgpoint.addr );
	if(
	( reach > HZX_INDIRECT_REACH )
	){
	/*ゾーン到達してない*/
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}
	entk->act->pad = SP_MOVE_RUN ;
	entk->act->move_s = MoveCautionWalk ;
	entk->act->dir = entk->pl_eyei.dir ;
	entk->act->tmp_dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}

static void Think3_ZoneChaseNearAttack( entk )
ENETHINK	*entk ;
{
	int rank_dis = 1000;
	AT_THK *at_thk;
	TNG_COM		*tng_com ;
	ENTK_TENG_A	*	tng_a ;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

//	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));
	entk->act->dir = entk->ctrl->turn.vy ;

#if 0
	SetAimPosPlayer(entk,0);
#else
	SetAimPosEyei(entk) ;
#endif

#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			/*めくら時の処理*/
			if( at_thk->zone_dis > entk->at_com->chasedis+1000)
			{
				entk->act->pad = SP_READYGUN;
				entk->count3 =0;
				entk->think2 =TH2_CHASE;
				entk->think3 =TH3_WAIT_CHASE;
				entk->act->dir = entk->ctrl->turn.vy ;
				return ;
			}
		}
	}
#endif

	entk->act->pad = SP_MOVE_RUN ;
	entk->act->dir = entk->ctrl->turn.vy ;
	at_thk->at_status &= (~AT_ST_SQUAT);

	/*現在地アドレス更新*/
	if(!(entk->count3%(AT_THK_RATE*5))) {
		entk->znavi->this_addr = 
		HZX_GetAddress( entk->ctrl->hzx_id, 
		&( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;
	}
	/*手前が近けりゃ待機状態*/
	/*06.01 後方待機*/
	if(
	(( at_thk->dis_dif < rank_dis  )&&(at_thk->dis_rank > 0))
	||(tng_com->now_hang_num != 0)
	){
		/*対イントルード*/
		if(
		(entk->at_com->watch_status & AT_COM_WATCH_INTRUDE )
		){
			if(at_thk->dis_rank ==0){
				GoIntZoneChase(entk);
			}else {
				GoIntWait(entk);
			} 
			return ;
		}

		entk->think3 = TH3_WAIT_CHASE ; 
		entk->count3 = 0 ;
		return ;
	}
	/*目標が近けりゃ攻撃 */
	/*先頭兵はつねにゾーン距離を詰める*/

	if(
	( entk->pl_eyei.dis <= (int)KATANA_DIS )
//	&&(Sig_GetRoute( entk->ctrl->hzx_id,*entk->pl_eyei.addr,
//	entk->ctrl->addr)<=HZX_INDIRECT_REACH)
	) {
		entk->think2 = TH2_ATTACK;
		entk->think3 = TH3_ATTACK_BEAT ; 
		entk->count3 = 0 ;
		return ;
	}

	if(CheckJumpPosENE(entk)){
		SIG_SetChasePos(entk ,&entk->tmp_pos) ;
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_DIRECT_CHASE_JUMP ;
		entk->count3 = 0 ;
		return ;
	}
	if ( 
	(ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) < 0 )
	||(SIG_CheckRIntrpt(entk))
	){
/*ダイレクト追跡に移行*/
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_DIRECT_CHASE_NEAR_ATTACK ;
		entk->count3 = 0 ;
		return ;
	}
	if((GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE )==0)){
		if(
		( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )
		&&(at_thk->dis_rank > (KATANA_ATTACK_NUM-1))
		){
			switch(CheckPlayerSightTng( entk ,0,256) ){
				case LEFT_WALL :
					/*右へ行け*/
					entk->act->pad = SP_ROLL_R ;
					break;
				case RIGHT_WALL:
					 /*左へ行け*/
					entk->act->pad = SP_ROLL_L ;
					break;
			}
		}
	}

	if(CheckJumpAttackCondition(entk)) {
		tng_com->jump_attack = TNG_JUMP_ATTACK_CNT;
		GoJumpAttack(entk);
		return ;
	}

	entk->act->move_s = MoveAttackRun ;
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}


/*座標追跡 プレイヤ直接*/
static	void	Think3_DirectChaseNearAttack( entk )
ENETHINK	*entk ;
{
	int		reach,rank_dis = 1000 ;
	TNG_COM		*tng_com ;
	ENTK_TENG_A	*	tng_a ;
	AT_THK *at_thk;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

#if 0
	SetAimPosPlayer(entk,0);
#else
	SetAimPosEyei(entk) ;
#endif

	entk->act->dir = entk->ctrl->turn.vy ;
//	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));


#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			/*めくら時の処理*/
			if( at_thk->zone_dis > entk->at_com->chasedis+1000)
			{
				entk->act->pad = SP_READYGUN;
				entk->count3 =0;
				entk->think2 =TH2_CHASE;
				entk->think3 =TH3_WAIT_CHASE;
				entk->act->dir = entk->ctrl->turn.vy ;
				return ;
			}
		}
	}
#endif

	entk->act->dir = entk->ctrl->turn.vy ;


	/*対ビヨンド思考に移行*/
	if(entk->at_com->watch_status & AT_COM_WATCH_BEYOND)
	{
		/*見失ってない*/
		if((entk->at_com->alert_time%(AT_THK_RATE*2)==0)&&(at_thk->dis_rank==0)){
			SetMoveMode(entk);
			entk->act->dir = entk->trgpoint.dir ;
			entk->act->pad = SP_MOVE_RUN ;
			entk->think2 = TH2_CHASE;
			entk->think3 = TH3_CHASE_BEYOND ;
			entk->count3 = 0 ;
			return ;
		}
	}

	at_thk->at_status &= (~AT_ST_SQUAT);
	//PLAYER_BEYOND	/*ビヨンドモード中*/
	if(ThinkBeyond(entk)) return ;
	/*首絞めフェイズなので攻撃中止*/
	if(tng_com->now_hang_num != 0){
		entk->think3 = TH3_WAIT_CHASE ; 
		entk->count3 = 0 ;
		entk->act->move_s = MoveAttackRun ;
		return ;
	}
	/*手前が近けりゃ*/
	if(( at_thk->dis_dif < rank_dis  )
	&&(entk->at_com->watch_status & AT_COM_WATCH_INTRUDE )
	){
		/*対イントルード*/
		if(
		(entk->at_com->watch_status & AT_COM_WATCH_INTRUDE )
		){
			if(at_thk->dis_rank ==0){
				GoIntZoneChase(entk);
			}else {
				GoIntWait(entk);
			} 
			return ;
		}
		if(at_thk->dis_rank > 0){
			entk->think3 = TH3_WAIT_CHASE ; 
			entk->count3 = 0 ;
			entk->act->move_s = MoveAttackRun ;
			return ;
		}
	}



	if(CheckJumpPosENE(entk)){
/*ここで目標距離調べて直接ジャンプすればいいかな？*/
		SIG_SetChasePos(entk ,&entk->tmp_pos) ;
		if(GV_VecLen3F2( &entk->znavi->flore_pos, &entk->trgpoint.pos )< 250.0F){
			/*直接ジャンプ*/
			if(entk->tmp_pos.vy == entk->act->target_pos.vy){
				entk->act->pad = SP_TNG_JUMP_LEVEL ;
			}else {
				entk->act->pad = SP_TNG_JUMP ;
			}
			entk->act->dir = (int) entk->tmp_pos.vw ;
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_START_JUMP ;
			entk->count3 = 0 ;
			return ;
		}else {
			/*踏み切り位置まで移動*/
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_DIRECT_CHASE_JUMP ;
			entk->count3 = 0 ;
			return ;
		}
	}

	/*追跡のための方向設定と距離判定*/
	ENE_DirectTrace( &(entk->trgpoint), &(entk->znavi->flore_pos), (0) ) ;
	/*距離外*/
	/*32フレON/OFF 100000*/
//	if( entk->count3 & 0x20){
	if( entk->count3 ==0){
		/*現在値 計算し直し*/
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), -1 ) ;
	}
	/* いつまでも直線じゃいられない */
	entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;

	reach = HZX_ReachTo( entk->znavi->this_addr, entk->trgpoint.addr );


	if(
	( reach > HZX_INDIRECT_REACH )
	){
	/*ゾーン到達してない*/
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
//		entk->act->dir = entk->ctrl->turn.vy ;
		return ;
	}

	if(
	( entk->pl_eyei.dis <= KATANA_DIS )
//	&&(Sig_GetRoute( entk->ctrl->hzx_id,*entk->pl_eyei.addr,
//	entk->ctrl->addr)<=HZX_INDIRECT_REACH)
	) {
		entk->think2 = TH2_ATTACK;
		entk->think3 = TH3_ATTACK_BEAT ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->act->move_s = MoveAttackRun ;
	if(HZX_Zone2(*entk->pl_eyei.addr) !=255){
		/*追跡可能*/
		entk->act->pad = SP_MOVE_RUN ;
		entk->act->dir = entk->trgpoint.dir ;
	}else {
		entk->act->pad = SP_READYGUN ;
		entk->act->dir = entk->pl_eyei.dir ;
//		entk->act->dir = entk->trgpoint.dir ;
	}
	entk->count3 ++ ;
}



//#define MUKADE_YOSOMI


/*安全地帯に避難*/
//#define BEHIND_FLGS (SAFE_BEHIND1|SAFE_BEHIND2|SAFE_BEHIND3|SAFE_BEHIND4)
static void Think3_ZoneChaseSafe( entk )
	ENETHINK	*entk ;
{
	HZX_ZON		*safezone; /*ゾーン*/
	FVECTOR		pos;
	int			type;

//	ENETHINK	*nearentk;
//	FVECTOR		nearsub ;
//	int			neardir ;
	AT_THK *at_thk ;
	ENTK_TENG_A	*	tng_a ;

	at_thk = (AT_THK *) entk->character ;
	tng_a = (ENTK_TENG_A*) entk->eve_a ;

	entk->act->dir = entk->ctrl->turn.vy ;

#if 0
	SetAimPosPlayer(entk,0);
#else
//	SetAimPosEyei(entk) ;
#endif
//printf("NOW Think3_ZoneChaseSafe ID[%d]!!!!!!!\n",entk->id) ;


	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->pad = SP_MOVE_RUN ;
	entk->act->dir = entk->ctrl->turn.vy ;
/*ゾーン追跡*/
	if( entk->count3 == 0 ) {
		/*目標地点を安地に設定*/
		if(at_thk->safeaddr == HZX_NO_ZONE) {
			ResetUseZone(entk);
			/*安地が異常なら近接処理やり直し*/
		if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
		GoNearAttack(entk);
//			entk->act->dir = entk->ctrl->turn.vy ;
			return ;
		}
//		safezone = HZX_GetZone(entk->ctrl->hzx_id, at_thk->safeaddr ) ;
		safezone = ENE_HZX_GetZone(at_thk->safeaddr ) ;
		pos.vx =(float)safezone->x ;
		pos.vy =(float)safezone->y ;
		pos.vz =(float)safezone->z ;
		entk->trgpoint.pos = pos;
		entk->trgpoint.addr = at_thk->safeaddr ;
	}
#if 0
	/*ゾーン追跡＆終了判定*/
	/*盾兵の背後に隠れる*/
	if(entk->count3%(AT_THK_RATE*2) == 0){
		if(CheckShlBehind(entk)){
			if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
			ResetUseZone(entk);
			entk->think2 = TH2_ATTACK ; 
			entk->think3 = TH3_WAIT_LOWSAFE ;
			entk->count3 = 0 ;
			entk->act->dir = entk->ctrl->turn.vy ;
			return ;
		}
	}
#endif
	if(
	(ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) < 0 )
	||(SIG_CheckRIntrpt(entk))
	){
		if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
			GoSafeDirect(entk);
		return ;
#if 1
		if(CheckEnePlOnline(entk))
		{
			ResetUseZone(entk);
			GoReload(entk);
			return ;
	
			ResetUseZone(entk);
			/*安地に到着*/
			type = at_thk->safetype;
		if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
			if(type& SAFE_LOW){
				/**低いゾーン**/
				entk->think2 = TH2_ATTACK ; 
				entk->think3 = TH3_WAIT_LOWSAFE ;
			}else if(type & BEHIND_FLGS){
				/*張り付き*/
				entk->think2 = TH2_ATTACK ; 
				entk->think3 = TH3_BEHIND ;
			}else {
				GoReload(entk);
				return ;
			}
			entk->count3 = 0 ;
			return ;
		}
#endif
	}
	/*重なり防止*/
	if(CheckNearAT2Wait(entk)) {
		if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
		GoChaseWait(entk);
		return ;
	}

	if(CheckJumpPosENE(entk)){
		if((tng_a->type >= TNG_TYPE_C)&&(entk->act->target_pos.vy > entk->ctrl->mov.vy)){
		}else {
			SIG_SetChasePos(entk ,&entk->tmp_pos) ;
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_DIRECT_CHASE_JUMP ;
			entk->count3 = 0 ;
			return ;
		}
	}


#if 0
	/*ビックリマーク*/
	if((at_thk->sight_time > 0 )
	&&(entk->count3 == AT_THK_RATE*2)
	){
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
	}
#endif
#if 1
	/*正面向きまま後退*/
	/*しばらくすると通常走りで移動*/
	if(entk->count3 < AT_THK_RATE*20){
		if(entk->count3%(AT_THK_RATE*2)==0){
			SetMoveMode(entk);
		}
	}else if(entk->count3 < AT_THK_RATE*21) {
		entk->act->move_s = MoveAttackRun ;
		entk->act->pad = SP_READYGUN ;
	}else {
		entk->act->move_s = MoveAttackRun ;
	}
#else
	if(entk->count3%(AT_THK_RATE*2)==0){
		SetMoveMode(entk);
	}
#endif
	if((GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE )==0)){
		if((entk->act->move_s == MoveSideR )
		||(entk->act->move_s == MoveSideL )){
			if(
			(at_thk->sight_time > (AT_THK_RATE*10) )
			&&(!(CheckFlrAtr(entk)))
			){
				if(Sig_InsideZone( ENE_HZX_GetZone(entk->ctrl->addr),
					&entk->ctrl->mov ,-400.0F )){

					if(CheckRollTime(entk)){
						if(entk->act->move_s == MoveSideR )
							entk->act->pad = SP_ROLL_R ;
						if(entk->act->move_s == MoveSideL )
							entk->act->pad = SP_ROLL_L ;
					}
				}
			}
		}
	}

	if(
	( entk->pl_eyei.dis <= entk->sense.eye_s) 
//	&&(entk->status2 & ENE_STATUS2_AIM_GUN)
//	&&(entk->status2 & ENE_STATUS2_AIM_GUNSHOOT)
	){
		if(!(entk->act->bodyp.type & ENE_TYPE_KATANA )){
//			RandShoot(entk);
		}
	}

#if 1
	if( entk->act->move_s == MoveAttackRun) {
		entk->act->dir = entk->trgpoint.dir ;
	} else {
		SetAimPosEyei(entk) ;
		entk->act->dir = entk->pl_eyei.dir ;
	}
#else
	entk->act->dir = entk->trgpoint.dir ;
#endif

	if(EscCancel(entk)){
		ResetUseZone(entk);
		return ;
	}
	if(CheckBeat(entk)){
		return ;
	} 
	entk->count3 ++ ;
}

static void Think3_DirectChaseSafe( entk )
	ENETHINK	*entk ;
{
//	SVECTOR rgb;
	entk->act->pad = SP_MOVE_RUN ;

#if 0
	{
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&entk->trgpoint.pos ,250.0F ,&rgb );

		rgb.vx =   0;	rgb.vy =   0;rgb.vz =  255;
		PosBox(&(entk->znavi->flore_pos) ,250.0F ,&rgb );
	}
#endif

	entk->act->dir = entk->pl_eyei.dir ;
	/*追跡のための方向設定と距離判定*/
	if(
	(ENE_DirectTrace(&(entk->trgpoint),&(entk->znavi->flore_pos),
//	350)<0)
//	50)<0)
	80)<0)
//	||(CheckEnePlOnline(entk))
	||(CheckToucheWallDir(entk,entk->trgpoint.dir ,512 ) )
	){
//		if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
		entk->act->pad = SP_READYGUN ;
		entk->act->mot_speed_correct = 0.0F;
//		entk->ctrl->mov.vx = entk->trgpoint.pos.vx ;
//		entk->ctrl->mov.vz = entk->trgpoint.pos.vz ;

		ResetUseZone(entk);
#if 1
		if(entk->bullet >= entk->max_bullet){
			GoReload(entk);
		}else {
			GoEasyAttack(entk,AT_THK_RATE*6);
		}
#else
		GoReload(entk);
#endif
		return ;
	}

	if(CheckNearAT2Wait(entk)){
		if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
		GoChaseWait(entk);
		return ;
	}

/*debug*/
	/*方向補正は毎フレーム行う*/
	/*進行方向*/
	entk->act->tmp_dir = entk->trgpoint.dir ;
	if(entk->count3%(AT_THK_RATE*2)==0){
		SetMoveMode(entk);
	}
	/*体の向き*/
	entk->act->dir = entk->pl_eyei.dir ;
//	entk->act->dir = entk->trgpoint.dir ;
	/**/
	if(EscCancel(entk)){
		if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
		ResetUseZone(entk);
		return ;
	}
	entk->count3 ++ ;
}


/*プレイヤから離れる*/
/******
マップまたぎには非対応
******/
/*後退ゾーン数*/
#define FAR_NUM (10)
static void Think3_ZoneChaseEscape( entk )
ENETHINK	*entk ;
{
	HZX_ZON		*esczone; /*ゾーン*/
//	FVECTOR		pos;
//	int		i;
	TNG_COM		*tng_com ;
	ENTK_TENG_A	*	tng_a ;
	AT_THK *at_thk;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

	at_thk->at_status &= (~AT_ST_SQUAT);



	entk->act->pad = SP_MOVE_RUN ;
	entk->act->dir = entk->ctrl->turn.vy ;

#if 0
printf("NOW ZoneEscape [%d]\n",entk->id);
	{
		SVECTOR rgb;
		extern void SigZoneView( int ,SVECTOR * ,float size);

		rgb.vx = 255 ;
		rgb.vy = 0 ;
		rgb.vz = 0 ;
		SigZoneView(at_thk->escaddr,&rgb,100.0F) ; 
		printf("ESC TRG MAP %d\n",entk->trgpoint.map);
		printf("NOW AT MAP %d\n",HZX_ZoneMapNo(entk->ctrl->addr));
	}
#endif

#if 0
	SetAimPosPlayer(entk,0);
#else
	SetAimPosEyei(entk) ;
#endif


	/*別マップに逃げられたら終了*/
#if 0
	if(HZX_ZoneMapNo(GM_PlayerAddress) != HZX_ZoneMapNo(entk->ctrl->addr) ){
		if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
		GoEasyAttack(entk,AT_THK_RATE*10);
		return ;
	}
#endif


/*ゾーン追跡*/
	if( entk->count3 == 0 ) {

		/*後退ゾーンを目標地点に設定*/
//		if(at_thk->escaddr == HZX_NO_ZONE) {
		if(HZX_Zone1(at_thk->escaddr) == HZX_NO_ZONE) {
#if 0
			for(i=FAR_NUM;i>=0;i--){

				at_thk->escaddr = 
					GetFarZone(GM_PlayerAddress,i,entk);
/*他の兵士がいるゾーンには行かない*/
//				if(CheckUseZone(entk ,at_thk->escaddr)==0){
//					break;
//				}
			}
#else
//mapaddr
			at_thk->escaddr = 
				GetFarZone(*entk->pl_eyei.addr,2000,entk);
#endif
		}else {
		}
		if(at_thk->escaddr == entk->ctrl->addr) {
			if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
			GoEasyAttack(entk,AT_THK_RATE*10);
			return ;
		}

		if(HZX_Zone1(at_thk->escaddr) == HZX_NO_ZONE ){
			printf("ESC ZONE NOT FOUND!!!\n");
			ASSERT(0);
		}
#if 0
		esczone = HZX_GetZone(entk->ctrl->hzx_id, 
		HZX_Zone1(at_thk->escaddr) ) ;
#else
		esczone = ENE_HZX_GetZone(at_thk->escaddr ) ;
#endif

#if 0
printf("SET ZONE X %d Z %d\n",esczone->x,esczone->z);
printf("SET ADDR1== %d\n",HZX_Zone1(at_thk->escaddr));
printf("SET ADDR2== %d\n",HZX_Zone2(at_thk->escaddr));
printf("SET MAP %d\n",HZX_ZoneMapNo(at_thk->escaddr));
#endif

#if 1
		ENE_SetTrgpZone( &entk->trgpoint,HZX_Zone1(at_thk->escaddr),entk->ctrl->hzx_id );
#else
		pos.vx =(float)esczone->x ;
		pos.vy =(float)esczone->y ;
		pos.vz =(float)esczone->z ;
		entk->trgpoint.pos = pos;
		entk->trgpoint.addr = at_thk->escaddr;
		entk->trgpoint.map = HZX_ZoneMapNo(at_thk->escaddr);
#endif
	}

#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&entk->trgpoint.pos ,250.0F ,&rgb );
		rgb.vx =   0;	rgb.vy =   0;rgb.vz =  255;
		PosBox(&(entk->znavi->flore_pos) ,250.0F ,&rgb );
	}
#endif

	if(entk->count3%(AT_THK_RATE*2) == 0){
		if(CheckShlBehind(entk)){
			if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
			entk->think2 = TH2_ATTACK ; 
			entk->think3 = TH3_WAIT_LOWSAFE ;
			entk->count3 = 0 ;
//			entk->act->dir = entk->ctrl->turn.vy ;
			return ;
		}
	}
//	if(entk->count3%30 ==0 ) NewZoneViewer( at_thk->escaddr,30,0);
/*ゾーン追跡＆終了判定*/
	if(
	(ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) < 0 )
	||(SIG_CheckRIntrpt(entk))
	){
/*正常到達*/
		if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
		GoSafeDirect(entk);
		return ;
	}
	if((0)
//	||(at_thk->zone_dis > entk->at_com->esc_dis+(at_thk->dis_rank*1000))
	||(entk->count3 >(AT_THK_RATE*40))
//	||(ENE_ReadOnlinInfo(entk->ctrl->addr,GM_PlayerAddress))
	||((HZX_Zone1(entk->trgpoint.addr)
	==HZX_Zone1(entk->ctrl->addr))
	&&(CheckUseZone(entk ,entk->trgpoint.addr)))
	||(CheckOnlyRIntrpt(entk))
	){
		/*中断処理*/
		if(ENE_ReadOnlinInfo(entk->ctrl->addr,*entk->pl_eyei.addr)){
		/*安全なので少し休む*/
			if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
			if(entk->bullet == 0) {
				GoEasyAttack(entk,AT_THK_RATE*10);
			}else {
				entk->think2 = TH2_ATTACK ; 
				entk->think3 = TH3_WAIT_LOWSAFE ;
				entk->count3 = 0 ;
			}
			return ;
		}else {
		/*隠れてないので反撃*/
			GoEasyAttack(entk,AT_THK_RATE*10);
			return ;
		}
	}
	if(CheckNearAT2Wait(entk)) {
		if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
		GoChaseWait(entk);
		return ;
	}
	
/*2000.06.19 コーナー確保から射撃*/
	if(at_thk->at_status & AT_ST_NEXT) {
		/*角の向こうにいる*/
		if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_WAIT_LOWSAFE ;
		entk->count3 = 0 ;
		return ;
	}

#if 0
	if(entk->count3 < AT_THK_RATE*20 ){
		if(entk->count3%(AT_THK_RATE*2)==0){
			SetMoveMode(entk);
		}
	}else if(entk->count3 < (AT_THK_RATE*22)) {
		entk->act->move_s = MoveAttackRun ;
		entk->act->pad = SP_READYGUN ;
	}else {
		entk->act->move_s = MoveAttackRun ;
	}
#else
	if(entk->count3%(AT_THK_RATE*2)==0){
		SetMoveMode(entk);
	}
#endif
/*2000.07.14転がり逃げ実験*/
	if((GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE )==0)){
		if((Sig_GetRoute( entk->ctrl->hzx_id,entk->trgpoint.addr,
		entk->ctrl->addr)<=HZX_INDIRECT_REACH)
		&&(ENE_ReadOnlinInfo(entk->trgpoint.addr,*entk->pl_eyei.addr))
		){
			if((entk->act->move_s == MoveSideR )
			||(entk->act->move_s == MoveSideL )
			){
				if(
				(at_thk->sight_time > (AT_THK_RATE*5) )&&(!(CheckFlrAtr(entk)))
				){
					if(Sig_InsideZone( ENE_HZX_GetZone(entk->ctrl->addr),
						&entk->ctrl->mov ,-400.0F )){
						if(CheckRollTime(entk)){
							if(entk->act->move_s == MoveSideR )
								entk->act->pad = SP_ROLL_R ;
							if(entk->act->move_s == MoveSideL )
								entk->act->pad = SP_ROLL_L ;
						}
					}
				}
			}
		}
	}
	if(
	( entk->pl_eyei.dis <= entk->sense.eye_s) 
	&&(entk->act->pad == SP_MOVE_RUN)
	&&(!(entk->act->bodyp.type & ENE_TYPE_KATANA ))
	&&(entk->count3 > DIRECT_TICK(120))
	){
/*逃げながら*/
		if(tng_com->now_hang_num == 0){
			RandShoot(entk);
		}
	}
	if(CheckBeat(entk)){
		return ;
	} 

#if 1
	if( entk->act->move_s == MoveAttackRun) {
		entk->act->dir = entk->trgpoint.dir ;
	} else {
		entk->act->dir = entk->pl_eyei.dir ;
	}
#else
	entk->act->dir = entk->trgpoint.dir ;
#endif
#if 0
	/*追跡条件変更につき廃止*/
	if(EscCancel(entk)) return ;
#endif
	/*殴り*/
	if(CheckBeat(entk)){
		return ;
	} 
	/*逃げアクション変更*/
	if((GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE )==0)){
		if(entk->count3 == 0){
			if( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ){
//			if(CheckPlayerSightTng( entk ,0,256) != 0 ){
				switch( entk->act->move_s){
					case MoveSideR :
						entk->act->pad = SP_ROLL_R ;
						break;
					case MoveSideL :
						entk->act->pad = SP_ROLL_L ;
						break;
					case MoveBack :
					/*刀兵はバク転無し*/
						if(!(entk->act->bodyp.type & ENE_TYPE_KATANA )){
							entk->act->pad = SP_BACK_TURN ;
						}
						break;
				}
			}
		}
	}
	entk->count3 ++ ;
}
//intr
//jump
/*指定ポイントまで移動*/
static void Think3_ZoneChasePos( entk )
ENETHINK	*entk ;
{
//	HZX_ZON		*esczone; /*ゾーン*/
//	FVECTOR		pos;
	AT_THK *at_thk;

	ENTK_TENG_A	*	tng_a ;

	at_thk = (AT_THK *) entk->character ;
	tng_a = (ENTK_TENG_A*) entk->eve_a ;

	if(entk->count3==0){
		/*目標をセット*/
	}
	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->pad = SP_MOVE_RUN ;
	entk->act->dir = entk->ctrl->turn.vy ;
/*ゾーン追跡*/
/*ゾーン追跡＆終了判定*/
	if
	((ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) < 0 )
	||(SIG_CheckRIntrpt(entk)))
	{
		if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
		/*ゾーン内の座標に近づく*/
		entk->think3 = TH3_DIRECT_POS ;
		entk->count3 = 0 ;
		return ;
	}
	/*殴り*/
	if(CheckBeat(entk)){
		return ;
	} 


	if(CheckJumpPosENE(entk)){

//printf("ZONE CHASE PLAYER TRG Y [%f]\n",entk->act->target_pos.vy);
		if((tng_a->type >= TNG_TYPE_C)&&(entk->act->target_pos.vy > entk->ctrl->mov.vy)){
		}else {
			SIG_SetChasePos(entk ,&entk->tmp_pos) ;
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_DIRECT_CHASE_JUMP ;
			entk->count3 = 0 ;
			return ;
		}
	}


	entk->act->move_s = MoveAttackRun ;

	if( entk->act->move_s == MoveAttackRun) {

		entk->act->dir = entk->trgpoint.dir ;
	} else {
		entk->act->dir = entk->pl_eyei.dir ;
	}

	entk->count3 ++ ;
}



/*座標追跡 ゾーン内座標*/
static	void	Think3_DirectChasePos( entk )
ENETHINK	*entk ;
{
	int		reach ;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->pad = SP_MOVE_RUN ;
	entk->act->dir = entk->ctrl->turn.vy ;


	/*追跡のための方向設定と距離判定*/
	/*座標追跡*/
#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&entk->trgpoint.pos ,250.0F ,&rgb );
		rgb.vx =   0;	rgb.vy =   0;rgb.vz =  255;
		PosBox(&(entk->znavi->flore_pos) ,250.0F ,&rgb );
	}
#endif

	if( entk->count3%(AT_THK_RATE*5) ==0){
		/*現在値 計算し直し*/
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), -1 ) ;
		/* いつまでも直線じゃいられない */
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;
		reach = HZX_ReachTo( entk->znavi->this_addr, entk->trgpoint.addr );
		if(
		( reach > HZX_INDIRECT_REACH )
		){
		/*ゾーン到達してない*/
			entk->act->pad = SP_READYGUN ;
			entk->think3 = TH3_ZONE_POS ;
			entk->count3 = 0 ;
//		entk->act->dir = entk->ctrl->turn.vy ;
			return ;
		}
	}
	if(
		( ENE_DirectTrace( &(entk->trgpoint), &(entk->znavi->flore_pos), 75 ) < 0 ) 
//		||(CheckToucheWallDir(entk,entk->trgpoint.dir ,256 ))
		){
		/*到着後*/
		/*目標座標に移動*/
		entk->act->mot_speed_correct = 0.0F;
//		entk->ctrl->mov.vx = entk->trgpoint.pos.vx ;
//		entk->ctrl->mov.vz = entk->trgpoint.pos.vz ;
		/*目標修正*/
		entk->act->pad = SP_READYGUN ;
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));

		if(at_thk->th2_buf >= 0){
			SetReturnThink( entk ) ;
		}else {
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_WAIT_CHASE ; 
		}

		/*実験 撤退*/
//		ResetWaitStatus(entk);
		entk->count3 = 0 ;
		return ;
	}
	/*SYMでは壁際への移動が多いのでr_sphere内でＯＫ*/
	/*r_sphere 以内に入れば到着*/
	if((entk->think2 == TH2_SYMMETRY)
	&&(entk->trgpoint.h_dis < entk->ctrl->r_sphere)
	&&(entk->ctrl->n_touches)
	){
		/*到着後*/
		entk->act->mot_speed_correct = 0.0F;
		entk->act->pad = SP_READYGUN ;
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));

		if(at_thk->th2_buf >= 0){
			SetReturnThink( entk ) ;
		}else {
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_WAIT_CHASE ; 
		}
		/*実験 撤退*/
//		ResetWaitStatus(entk);
		entk->count3 = 0 ;
		return ;
	}
	if(!(entk->act->bodyp.type & ENE_TYPE_KATANA )){
//		RandShoot( entk );
	}
	SetMoveMode(entk);
	if( entk->act->move_s == MoveAttackRun) {
		entk->act->dir = entk->trgpoint.dir ;
	} else {
		entk->act->dir = entk->pl_eyei.dir ;
	}
	entk->count3 ++ ;
}

static	void	Think3_DirectChaseJump( entk )
ENETHINK	*entk ;
{
//	int		reach ;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->pad = SP_MOVE_RUN ;
	entk->act->move_s = MoveCautionWalk ;
	entk->act->dir = entk->trgpoint.dir ;
	entk->act->tmp_dir= entk->trgpoint.dir ;
	/*追跡のための方向設定と距離判定*/
	/*座標追跡*/
#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&entk->trgpoint.pos ,250.0F ,&rgb );
		rgb.vx =   0;	rgb.vy =   0;rgb.vz =  255;
		PosBox(&(entk->znavi->flore_pos) ,250.0F ,&rgb );
	}
#endif
	if(
		( ENE_DirectTrace( &(entk->trgpoint), &(entk->znavi->flore_pos), 250 ) < 0 ) 
		){
		CheckJumpPosENE(entk) ;
		/*到着後*/
		/*目標座標に移動*/
		entk->act->mot_speed_correct = 0.0F;
		/*目標修正*/
		if(entk->tmp_pos.vy == entk->act->target_pos.vy){
			entk->act->pad = SP_TNG_JUMP_LEVEL ;
		}else {
			entk->act->pad = SP_TNG_JUMP ;
		}
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		if(CheckTngDefPosMode(entk)){
			ENE_SetTrgpPosMap( &(entk->trgpoint),&entk->def_pos, entk->def_mapbit ) ;
		}else {
			ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));
		}
		entk->act->dir = (int) entk->tmp_pos.vw ;
		entk->think3 = TH3_START_JUMP ;
		entk->count3 = 0 ;
		return ;
	}
	entk->count3 ++ ;
}

static	void	Think3_StartJump( entk )
ENETHINK	*entk ;
{
//	int		reach ;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

#if 0
	{
	SVECTOR rgb ;
	rgb.vx = 255 ;	rgb.vy = 0 ;	rgb.vz = 0 ;
	PosBox( &entk->trgpoint.pos ,100.0F ,&rgb );
	rgb.vx = 0 ;	rgb.vy = 0 ;	rgb.vz = 255 ;
	SigZoneView( entk->znavi->next_addr , NULL , 1000.0F ) ; 
	}
#endif

	if(entk->count3 ==0 ){
		CheckJumpPosENE( entk) ;
	}
	at_thk->at_status &= (~AT_ST_SQUAT);
	if(entk->tmp_pos.vy == entk->act->target_pos.vy){
		entk->act->pad = SP_TNG_JUMP_LEVEL ;
	}else {
		entk->act->pad = SP_TNG_JUMP ;
	}
	entk->act->dir = (int) entk->tmp_pos.vw ;
	/*追跡のための方向設定と距離判定*/
	/*座標追跡*/
#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&entk->trgpoint.pos ,250.0F ,&rgb );
		rgb.vx =   0;	rgb.vy =   0;rgb.vz =  255;
		PosBox(&(entk->znavi->flore_pos) ,250.0F ,&rgb );
	}
#endif
	if(entk->act->act_end == 1){
		/*到着後*/
		/*目標座標に移動*/
		entk->act->pad = SP_READYGUN ;
		if(CheckTngDefPosMode(entk)){
			/*守備位置到着後の思考を予約*/
			at_thk->th2_buf = TH2_ATTACK ;
			at_thk->th3_buf = TH3_DEFENCE ;
			/*守備位置へ向かう*/
			entk->think2 = TH2_CHASE ; 
			if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
				entk->think3 = TH3_ZONE_POS ;
			}else {
				entk->think3 = TH3_ZONE_CHASE ;
			}
			ENE_SetTrgpPosMap( &(entk->trgpoint),&entk->def_pos, entk->def_mapbit ) ;
		}else {
			/*プレイヤ追跡守備位置へ向かう*/
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_ZONE_CHASE ;
			ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));
		}
/*追跡*/
		entk->count3 = 0 ;
		return ;
	}
	entk->count3 ++ ;
}


/*格闘*/
static	void	Think3_AttackBeat( entk )
	ENETHINK	*entk ;
{
//	int sw ;
	AT_THK *at_thk ;
//	FVECTOR debug[2] ;
	TNG_COM		*tng_com ;
	ENTK_TENG_A *tng_a ;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;


	at_thk = (AT_THK *) entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	if(entk->count3 == 0){
		if(GM_PlayerStatus & (PLAYER_GROUND|PLAYER_CB_BOX)) {
			entk->act->pad = SP_STOMP ;
		}else {
/*通常攻撃*/
			entk->act->pad = SP_ATTACK_PUNCH_L ;
		}
		at_thk->at_tmptime = 0;
	}
#if 1
	if(
	(entk->act->act_end)
	||(entk->count3 >= AT_THK_RATE*20)
	){
		/*連続殴り防止*/
//		GoEasyAttack(entk,AT_THK_RATE*5);
		if(CheckBeat(entk)){
			return ;
		} 
		GoNearAttack(entk);
		return ;
	}
#endif
	/*プレイヤ方向を向かせる*/
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}
static	void	Think3_AttackHang( entk )
ENETHINK	*entk ;
{
//	int sw ;
//	FVECTOR debug[2] ;
	AT_THK *at_thk ;
	TNG_COM		*tng_com ;
	ENTK_TENG_A *tng_a ;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;


	at_thk = (AT_THK *) entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

	at_thk->at_status &= (~AT_ST_SQUAT);
//	if((entk->count3 >= 0)&&(entk->count3 == 0)){
	if(entk->count3 == 0){
		entk->act->pad = SP_HANG_START ;
		at_thk->at_tmptime = 0;
	}
#if 0
	else if((entk->count3 >= 4)&&(entk->count3 <= 7)){
		entk->act->pad = SP_READYGUN ;
	}else if(entk->count3 == 8){
		entk->act->pad = SP_HANG_START ;
	}
#endif
	if(
	(entk->act->act_end)
//	||(entk->count3 >= 960)
	){
		tng_a->status &= ~EVE_TNG_HANG ;
		/*連続殴り防止*/
		GoEasyAttack(entk,DIRECT_TICK(60));
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}


static	void Think3_KatanaJumpAttack( entk )
ENETHINK	*entk ;
{

	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	if(entk->count3 == 0){
		entk->act->pad = SP_JUMP_ATTACK ;
		at_thk->at_tmptime = 0;
	}

	if(
	(entk->act->act_end)
	||(entk->count3 >= AT_THK_RATE*25)
	){
		if(CheckBeat(entk)){
			return ;
		} 
		GoNearAttack(entk);
		return ;
	}
	/*プレイヤ方向を向かせる*/
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}


/*踏みつけ  SP_STOMP*/
static	void	Think3_AttackStomp( entk )
	ENETHINK	*entk ;
{
	FVECTOR	aimpos,e_pos ;
	float	trglen ;
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;
	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->dir = entk->ctrl->turn.vy ;

/*TEST 2000.09.05 */
/*毎フレームパッド更新*/
#if 0
	entk->act->pad = SP_STOMP ;
	if(entk->count3 == 0){
		at_thk->at_tmptime = 0;
	}
#else
	if(entk->count3 == 0){
		entk->act->pad = SP_STOMP ;
		at_thk->at_tmptime = 0;
	}
#endif
#if 1
	if(GM_PlayerStatus & PLAYER_FORCE){
		entk->act->pad = SP_BACKWALK ;
	}
	if((GM_PlayerStatus & PLAYER_FORCE)
	&&(entk->pl_eyei.dis < AT_DIS_MIN))
	{
		entk->act->pad = SP_BACKWALK ;
	}
#endif

	if(entk->act->act_end){
		/*踏めるかチェック*/
		/*足の位置*/
		e_pos = entk->znavi->flore_pos ;
		aimpos = GM_PlayerPosition ;
		aimpos.vy += 1200.0F ;	/* 手の高さ */
		trglen = GV_VecLen3F2( &aimpos, &e_pos ) ;
		if(
		(trglen <= STOMP_DIS)
		&&(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN)
		&&(ENE_EyeOnlineCheck( entk->ctrl->hzx_id,
			&entk->ctrl->mov, &aimpos )
		)){
			/*その場で踏み直し*/
			entk->count3 = 0;
		}else{
#if 1
//			entk->act->pad = SP_READYGUN ;
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_WAIT_CHASE ;
			entk->count3 = 0;
#endif
		}
		return ;
	}
	/*プレイヤ方向を向かせる*/
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}



static	void	Think3_AttackReload( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

	/*リロード中断*/
	if(
	(0)
//	( entk->at_com->com_sight < 2)
	||
	(( entk->at_com->Pl_StayTime == 0 )
	&&( at_thk->zone_dis > (entk->at_com->chasedis+2000) ))
	)
	{
		entk->act->act_end = 1;
	}
	
	if(at_thk->at_status & AT_ST_SQUAT){
		/*しゃがみ状態*/
		entk->act->pad = SP_RELOAD_SQUAT ;
	}else {
		entk->act->pad = SP_RELOAD ;
	}
	if ( entk->act->act_end ) {
		/*覗き込み実験*/
#if 0
printf("RELOAD ACT END\n");
printf("RELOAD PAD %d\n",entk->act->pad);
printf("KEEP PAD %d\n",entk->act->keep_pad);
#endif
		if((at_thk->at_status & AT_ST_NEXT)
//		&&( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN )
		&&(entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR)
		){
			entk->act->move_s = MoveCautionWalk ;
			SetMoveMode(entk);
			if(
			(entk->act->move_s == MoveSideR )
			||(entk->act->move_s == MoveSideL )
			){
				/*プレイヤが構えていれば覗かない*/
				if(!CheckPlayerSight( entk ,1 ,128)){
					GoAttackPeek(entk,entk->act->move_s);
					return;
				}
			}
		}
		GoNearAttack(entk);
		return ;
	}


	/*リロードキャンセル逃亡実験*/

	if((GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE )==0)){
		if(
		(ENE_AlertGameLevel >= AT_ESC_LEVEL)
		&&(at_thk->sight_time > entk->at_com->esctime )
		){
//			ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
			GoDanger( entk ) ;
			return;
		}
	}
#if 0
	if(entk->count3 == 0){
		entk->act->dir = entk->pl_eyei.dir ;
	}
#else
	entk->act->dir = entk->pl_eyei.dir ;
#endif
	entk->count3 ++ ;
}




/*横歩き登場*/
static void Think3_SideMove( entk )
ENETHINK	*entk ;
{
	int side;
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->dir = entk->ctrl->turn.vy ;


#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&entk->ctrl->mov ,350.0F ,&rgb );
//printf("Think3_SideMove\n");
	}
#endif
//setmovemode

#if 0
	SetAimPosPlayer(entk,0);
#else
	SetAimPosEyei(entk) ;
#endif

	entk->act->tmp_dir = entk->trgpoint.dir ;
	entk->act->pad = SP_MOVE_RUN ;
	SetMoveMode(entk);

#if 0
	/*方向調べ*/
	if(entk->count3==0) {

		if(CheckTurnSide(entk)) {
			entk->act->move_s = MoveSideR ;
		}else {
			entk->act->move_s = MoveSideL ; 
		}
	}
#endif
	/*重なりチェック*/
	if(entk->count3%(AT_THK_RATE*2)==0){
		side = CheckSideAT(entk,500.0F);
		side |= CheckWallDir(entk);
		switch (entk->act->move_s){
			case MoveSideR:
				if(side&RIGHT_WALL){
//					GoNearAttack(entk);
					GoChaseWait(entk);
					return;
				}
				break;
			case MoveSideL :
				if(side&LEFT_WALL){
//					GoNearAttack(entk);
					GoChaseWait(entk);
					return;
				}
				break;
		}
	}

	/*回避*/
	if(entk->count3%(AT_THK_RATE*2)==0){
		if(at_thk->sight_time > entk->at_com->esctime )
		{
			side = CheckSideSafe(entk,1000.0F);
			if(side & RIGHT_WALL){
				entk->act->move_s = MoveSideR ; 
			}else if(side & LEFT_WALL){
				entk->act->move_s = MoveSideL ; 
			}
			entk->count3 = 1;
			at_thk->sight_time = 0;
		}
	}
	/*ここの時間が過ぎれば壁ヒットなどなくても停止*/
	if(
	( entk->count3 > (AT_THK_RATE*20 - at_thk->dis_rank*AT_THK_RATE*5) ) 
//	||( entk->at_com->Pl_StayTime == 0 )
	){
		entk->act->pad = 0;
		entk->act->dir = -1 ;
//		GoNearAttack(entk);
		GoChaseWait(entk);
		return ;
	}
	if(!(entk->act->bodyp.type & ENE_TYPE_KATANA )){
//		RandShoot(entk);
	}
	entk->act->dir = entk->pl_eyei.dir;

	if(
	(at_thk->at_status & AT_ST_NEXT) 
	&&
	((entk->act->move_s == MoveSideR )
	||(entk->act->move_s == MoveSideL ))

	){
		GoAttackPeek(entk,entk->act->move_s);
		return;
	}


	entk->count3 ++ ;
}


static void Think3_GrdAttackHigh(entk)
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

	if(entk->count3 == 0){
		entk->act->target_pos = GM_PlayerPosition ;
		at_thk->at_tmptime = 0;
		entk->act->pad = SP_GRD_HIGH;
#if 0
/*act内発声にしてみる 「くらえ」*/
		if(entk->com->enemys.group[entk->g_id]->
			unit[entk->u_id]->enemy_num>1){
				COM_SetSpeak( EV_GRENADE_1, entk ) ;
		}else {
			COM_SetSpeak( EV_EAT_1, entk ) ;
		}
#endif
#if 0
//printf("SE GRD CALLED!!\n");
	/*グレネードを見てみんなビックリ*/
		SetSurpMode(entk);
#endif
	}
//	entk->sw_gun |= SW_FLAG_SWITCH2 ;

	if(entk->act->act_end == 2) at_thk->at_tmptime = 1;

	if(
	(entk->act->act_end==1)
	||(entk->count3 > AT_THK_RATE*50)
	){
		if(at_thk->at_tmptime == 1){
			printf("GRD THROW FAILED!!!!\n");
			/*プレイヤを目標に設定*/
//			ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));

			if(SIG_CheckRIntrpt(entk)){
				/*接近不能なら*/
			}else {
				/*接近可能なら突撃*/
				SetBerserkTime(entk,AT_THK_RATE*30) ;
			}
		}
		GoChaseWait(entk);
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir;
	entk->count3++;
}


/*角から覗く*/
static void Think3_Peek(entk) 
ENETHINK	*entk ;
{
	TNG_COM		*tng_com ;
	ENTK_TENG_A	*	tng_a ;
	AT_THK *at_thk;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;
	entk->act->dir = entk->ctrl->turn.vy ;

	if(((entk->count3%(AT_THK_RATE*10))==0)||(entk->at_com->Pl_StayTime ==0)){
printf(" THREAT CONDITON CHECK!!\n ");
//		if(CheckThreatCondition(entk->ctrl->addr,GM_PlayerAddress)){
		if(CheckThreatCondition(entk->ctrl->addr,entk->com->plzone_in_zone[0])){
			/*自動ドア発見*/
printf(" SLIDE DOOR FOUND!!\n");
			entk->act->dir = entk->ctrl->turn.vy ;
			entk->act->pad = SP_READYGUN;
			SetBerserkTime(entk,AT_THK_RATE*30) ;
//			GoNearAttack(entk) ;
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_ZONE_CHASE ;
			entk->count3 = 0 ;
			return  ;
		}else {
printf(" THREAT OK!!!!!\n");
		}
	}


/*０開始 １静止 ２戻り*/
	if(entk->count3 == 0){
		at_thk->at_tmptime = 0; /*シーケンス管理*/
		entk->tmp_time = 0;
		if(at_thk->th2_buf == MoveSideR ){
			entk->act->pad = SP_PEEK_HIGH_R ;
		}else {
			entk->act->pad = SP_PEEK_HIGH_L ;
		}
	}
	switch(at_thk->at_tmptime){
		case 0 :
			if(at_thk->th2_buf == MoveSideR ){
				entk->act->pad = SP_PEEK_HIGH_R ;
			}else {
				entk->act->pad = SP_PEEK_HIGH_L ;
			}
			/*覗き開始*/
			if(entk->act->act_end){
				at_thk->at_tmptime++;
			}
		break;
		case 1 :
			/*攻撃中*/
			if(at_thk->th2_buf == MoveSideR ){
				entk->act->pad = SP_PEEK_HIGH_IDLE_R ;
			}else {
				entk->act->pad = SP_PEEK_HIGH_IDLE_L ;
			}
#if 0
			SetAimPosPlayer(entk,0);
#else
	SetAimPosEyei(entk) ;
#endif

#if 0
			if(
			( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN )
			||(at_thk->sight_time > entk->at_com->esctime )
			){
				at_thk->at_tmptime++;
				break;
			}
#endif
			if(!(entk->act->bodyp.type & ENE_TYPE_KATANA )){
				if(tng_com->now_hang_num == 0){
					ThreatShot(entk,&entk->act->aim_pos);
				}
			}
			if(entk->tmp_time > AT_THK_RATE*20){
				at_thk->at_tmptime++;
				/*
				覗き込み終了まで見えないままだったら
				攻撃位置を変更
				*/
				if( at_thk->sight_time == 0 ){
					SetBerserkTime(entk,AT_THK_RATE*30) ;
				}
			}
			entk->tmp_time++;
		break;
		case 2 :
			/*戻り開始*/
			if(at_thk->th2_buf == MoveSideR )
				entk->act->pad = SP_PEEK_HIGH_END_R ;
			else 
				entk->act->pad = SP_PEEK_HIGH_END_L ;
			at_thk->at_tmptime++;
		break;
		case 3 :
			/*元の姿勢*/
			if(entk->act->act_end){
				if(
				(at_thk->at_status & AT_ST_NEXT) 
				&&( entk->pl_eyei.dis <= entk->sense.eye_s) 
				){
					GoReload(entk);
				}else {
					GoNearAttack(entk);
				}
				return ;
			}
		break;
	}
	if(!(at_thk->at_status & AT_ST_NEXT))
	{
		GoNearAttack(entk);
	}
	entk->act->dir = entk->pl_eyei.dir;
	entk->count3++;
}


/*手前の動きを見つつ待機*/
static void Think3_ChaseWait( entk )
ENETHINK	*entk ;
{
	FVECTOR npos,ppos;
	int subdir ;
	int rank_dis = 1000 ;
	TNG_COM		*tng_com ;
	ENTK_TENG_A	*	tng_a ;
	AT_THK *at_thk;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

	entk->act->dir = entk->ctrl->turn.vy ;

#if 0
	SetAimPosPlayer(entk,0);
#else
	SetAimPosEyei(entk) ;
#endif


	if( CheckFlrAtr(entk) ){
		rank_dis += 500;
	}
	if(at_thk->dis_rank > 1){
		rank_dis += 1000;
	}
	entk->act->pad = SP_READYGUN;
//	entk->act->pad = SP_GRD_LOW;


	if((entk->count3 % (AT_THK_RATE*10))==0){
		if(CheckTngDefPosMode(entk)){
		/*拠点防衛に出発*/
			GoDefPosChase(entk);
			return ;
		}
	}

	/*対イントルード*/
	if(
	(entk->at_com->watch_status & AT_COM_WATCH_INTRUDE )
	&&(at_thk->zone_dis < 5000 )
	){
		if(at_thk->dis_rank ==0){
			GoIntZoneChase(entk);
		}else {
			GoIntWait(entk);
		} 
		return ;
	}

	/*接近モード*/
#if 1
	if( at_thk->dis_rank>0)
	{
		if(CheckWaitStatus(entk)){
			GoApproachWait( entk );
			return ;
		}
	}
#endif

#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			/*めくら時の処理*/
			if(
			(at_thk->dis_rank>0)&&( at_thk->dis_dif >= rank_dis+500)
			&&( at_thk->zone_dis > entk->at_com->chasedis+2000)
			){
				/*通常行動続行*/
			}else {
				/*停止*/
				entk->status2 = 0 ;
				entk->act->dir = entk->ctrl->turn.vy ;
				entk->count3 ++ ;
				return;
			}
		}
	}
#endif


	/*攻撃モードに移行*/
	if(
	( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )
//	&&( entk->pl_eyei.dis < entk->at_com->chasedis )
	){
		/*強制モーション中は思考進行しない*/
		if(!(GM_PlayerStatus & PLAYER_FORCE)){
			GoNearAttack(entk);
			return ;
		}
	}

	/*プレイヤを見失ってなければ進行*/
	if(!(GM_PlayerStatus & PLAYER_BEYOND) )
	{
		if(ThinkSubNear( entk )) {
//		entk->act->dir = entk->ctrl->turn.vy ;
			return;
		}
	}else {
		/*ビヨンド中*/

		if(entk->at_com->watch_status & AT_COM_WATCH_BEYOND){
			/*飛び越え途中は待機*/
			if(GM_PlayerStatus & PLAYER_FORCE){
				if(entk->pl_eyei.dis < AT_DIS_MIN){
					entk->act->pad = SP_BACKWALK ;
				}
			}else {
				/*ぶら下がり中は距離詰め*/
				if((entk->at_com->alert_time%(AT_THK_RATE*2)==0)&&(at_thk->dis_rank==0)){
					entk->think2 = TH2_CHASE;
					entk->think3 = TH3_CHASE_BEYOND ;
					entk->count3 = 0 ;
					return ;
				}
			}
		}
	}


#if 0
	if(ENE_AlertGameLevel >= AT_ESC_LEVEL){
		/*リロードするときは隠れてから*/
		if((at_thk->sight_time > entk->at_com->esctime )
		||
		(( entk->bullet >= entk->max_bullet ))
		){
		GoDanger( entk ) ;
			return;
		}
	}
#endif
	if( entk->bullet >= entk->max_bullet ){
		/*ATACK NEAR*/
		/*その場でリロード*/
		if(
		(entk->act->bodyp.type & ENE_TYPE_SHOTGUN)
		){
			if(entk->act->keep_pad == SP_READYGUN) {
				GoReload( entk );
				return ;
			}
		} else {
			GoReload( entk );
			return ;
		}
	}

	/*飛びつき攻撃*/
	if(CheckJumpAttackCondition(entk)) {
		tng_com->jump_attack = TNG_JUMP_ATTACK_CNT;
		GoJumpAttack(entk);
		return ;
	}
	/*以下 待機処理*/
	/*プレイヤ方向を向かせる*/
	if(
//	(entk->count3 >= 30)
	(entk->count3 >= AT_THK_RATE*2)
	||(at_thk->at_status & AT_ST_NEXT)
	||( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR )
	){
		entk->act->dir = entk->pl_eyei.dir ;
	}else if(
		(entk->count3%(AT_THK_RATE*10) == 10)
		&&(entk->pl_eyei.dis < (entk->sense.eye_s -1000))
	){
		/*とりあえずCTRLからPL_posで判定*/
		npos = entk->ctrl->mov;
		npos.vy += 1000.0F;
		ppos = GM_PlayerFindPos ;

		if(!HZX_OnlineHazardCheck(entk->ctrl->hzx_id,&npos,&ppos,
		HZX_CHK_ALL,HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY ))
		{
			/*向きを変えれば見える位置なら向きを変える*/
			entk->act->dir = entk->pl_eyei.dir ;
		}else {
			/*みえなくても角度差がすくなければ転回*/
			subdir = abs(SIG_CheckDirSub(entk->ctrl->turn.vy,entk->pl_eyei.dir));
			if(subdir<1024){
				entk->act->dir = entk->pl_eyei.dir;
			}
		}
	}else {
		entk->act->dir = entk->ctrl->turn.vy ;
	}

	if(at_thk->at_status & AT_ST_NEXT) {
		/*
		移動はさせないが、覗き方向判定のため
		SetMoveMode()を使用
		*/
		entk->act->move_s = MoveCautionWalk ;
		SetMoveMode(entk);
		if(
		(entk->act->move_s == MoveSideR )
		||(entk->act->move_s == MoveSideL )
		)
		{
			GoAttackPeek(entk,entk->act->move_s);
			return;
		}
	}
	if(CheckDeathBed(entk) ){
		return ;
	}
	/*2000.06.19 後ろでうろうろ*/
	if(!(entk->at_com->watch_status)){
		if(CheckTngPrudence(entk)) return ;
	}
	if(!(entk->act->bodyp.type & ENE_TYPE_KATANA )){
		if(at_thk->sight_time > DIRECT_TICK(WAIT_SHOOT_DELAY)){
			if(tng_com->now_hang_num == 0){
				SIG_BasicShoot(entk,&GM_PlayerFindPos,0);
			}
//			RandShoot(entk);
		}
	}
	entk->count3 ++ ;
}
#define SHL_TIME AT_THK_RATE*10
#define DEF_TIME  AT_THK_RATE*5

// GM_PlayerStatus & PLAYER_DARK_AREA /* 暗闇 */
static void Think3_AttackNear( entk )
ENETHINK	*entk ;
{
/*beyond*/
	int fl_atr ;
	FVECTOR		testpos;
	TNG_COM		*tng_com ;
	ENTK_TENG_A	*	tng_a ;
	AT_THK *at_thk;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

	entk->act->dir = entk->ctrl->turn.vy ;

#if 0
	SetAimPosPlayer(entk,0);
#else
	SetAimPosEyei(entk) ;
#endif

	entk->act->dir = entk->ctrl->turn.vy ;


#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			/*めくら時の処理*/
			if( at_thk->zone_dis < entk->at_com->chasedis+1000)
			{
				entk->act->pad = SP_READYGUN;
				entk->count3 =0;
				entk->think2 =TH2_CHASE;
				entk->think3 =TH3_WAIT_CHASE;
				entk->act->dir = entk->ctrl->turn.vy ;
				return ;
			}
		}
	}
#endif


	if((entk->count3 % (AT_THK_RATE*10))==0){
		if(CheckTngDefPosMode(entk)){
		/*拠点防衛に出発*/
			GoDefPosChase(entk);
			return ;
		}
	}

	if(
	(entk->at_com->watch_status & AT_COM_WATCH_INTRUDE )
	&&(at_thk->zone_dis < 5000)
	){
		if(at_thk->dis_rank ==0){
			GoIntZoneChase(entk);
		}else {
			GoIntWait(entk);
		} 
		return ;
	}

	if(
	(entk->at_com->watch_status & AT_COM_WATCH_BEYOND)
	&&(!(GM_PlayerStatus & PLAYER_FORCE))
	){
		/*見失ってない*/
		if((entk->at_com->alert_time%(AT_THK_RATE*2)==0)&&(at_thk->dis_rank==0)){
			entk->think2 = TH2_CHASE;
			entk->think3 = TH3_CHASE_BEYOND ;
			entk->count3 = 0 ;
			return ;
		}
	}

	/*フロア属性チェック*/
	fl_atr = CheckFlrAtr(entk);
	/*割り込みアクション終了時にthinkをリスタート*/
	if ( entk->act->act_end ) {
//printf("THINK NEAR RESET!!!\n");
		entk->count3 = 0 ;
		return ;
	}
	/*先頭兵は常にゾーン距離で追跡*/

	/* 目標が見えなくてもチーム内の誰かが捕捉中なら */
	/* あわてて追跡しない */
/*イントルード中はここで振動する*/
//	if(at_thk->dis_rank==0){
	if(0){
/*暗闇中は特別*/
		if(
		(GM_PlayerStatus & PLAYER_DARK_AREA)&&
//		(at_thk->in_sight < AT_THK_RATE*2)
		(at_thk->in_sight == 0 )
		){
			GoDirectChase(entk);
			return ;
		}
		if(
		( at_thk->zone_dis > (entk->at_com->chasedis+1000))
		||(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN)
		){
			if(ThinkSubNear( entk )){
				return;
			}
		}
	}else if(
	(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN )
//	&&(entk->count3>AT_THK_RATE*10)
	){
		if(
		( (entk->at_com->com_sight <= entk->at_com->siege_num)
		&&(at_thk->dis_rank <= entk->at_com->siege_num) )
		||( at_thk->zone_dis > entk->at_com->chasedis+1000 ) 
		)
		{
/** NEAR 2 ZONE **/
/*****/
/*******/
			if(ThinkSubNear( entk )){
				return;
			}
		}
	}

//printf("ESC TESET  %d! !!!!!!!!!!!!!\n",at_thk->sight_time);
	/*距離保ち*/
	if(
	((entk->pl_eyei.dis < (entk->sense.eye_s*3/4) )
	||
	(at_thk->sight_time > (AT_THK_RATE*2) ))
	&&(at_thk->in_sight > (AT_THK_RATE*10))
	&&(!(GM_PlayerStatus & PLAYER_DARK_AREA))
	&&(!(GM_PlayerStatus & AT_PL_HIDE))
	&&(!( entk->act->bodyp.type & ENE_TYPE_TNG_A))
	&&((entk->count3 %(AT_THK_RATE*5))==((AT_THK_RATE*5)-1))
	&&(entk->at_com->berserk==0)
	){
		ENETHINK	*back_entk;
		AT_THK		*back_at_thk;
		int rout,back_addr ,goback = 0 ,side_addr ;
//printf("ESC CHECK!!!!!!!!!!!!!!\n");
		if(
		( HZX_Zone1(entk->ctrl->addr) == HZX_Zone1(GM_PlayerAddress))
		){
//printf("AT THINK LINE %d\n",__LINE__);
			/*プレイヤと同じゾーンの場合
			自分に座標の近い隣接ゾーンを目標とする*/
			side_addr = back_addr = CheckPosNearZone( 
			&entk->ctrl->mov, entk->ctrl->addr);
			if(HZX_Zone1(back_addr) != HZX_NO_ZONE) {
				goback = 1 ;
			}
		}else {
//printf("AT THINK LINE %d\n",__LINE__);
			/*逃げゾーン取得*/
			side_addr = SIG_GetFarDirZone( entk->ctrl->hzx_id,
				entk->ctrl->addr,GM_PlayerAddress ) ;

			back_addr = HZX_FarZoneNavigate( entk->ctrl->hzx_id,
				HZX_Zone1(entk->ctrl->addr),HZX_Zone1(GM_PlayerAddress),&rout);
			if(
			(Sig_GetRoute(entk->ctrl->hzx_id,GM_PlayerAddress,entk->ctrl->addr))
			>= (Sig_GetRoute(entk->ctrl->hzx_id,GM_PlayerAddress,side_addr))
			){
				/*近いので横ゾーンは禁止*/
				side_addr = back_addr ;
			}
			goback = 1 ;
			if(GM_GetRIntrptZ2Z(back_addr,entk->ctrl->addr) != NULL){
				goback = 0 ;
			}
		}
		/*逃げ予定地が後方と重ならないか？*/
		back_entk = (ENETHINK *)GetAtRanking(entk,1) ;
		if(back_entk != NULL ){
//printf("AT THINK LINE %d\n",__LINE__);
			back_at_thk = (AT_THK *) back_entk->character ;
			if(
			(HZX_Zone1(back_addr) == HZX_Zone1(back_entk->ctrl->addr) )
			&&(HZX_Zone1(side_addr) == HZX_Zone1(back_entk->ctrl->addr) )
//			&&(1500.0F > back_at_thk->dis_dif )
			){
//printf("AT THINK LINE %d\n",__LINE__);
				/*プレイヤと後方兵両方にぶつからない
				ゾーンを調べる*/
				back_addr = HZX_FarZoneNavigate2( entk->ctrl->hzx_id,
					HZX_Zone1(entk->ctrl->addr),HZX_Zone1(GM_PlayerAddress),
					HZX_Zone1(back_entk->ctrl->addr), &rout ) ;
				if((HZX_Zone1(back_addr) != HZX_Zone1(entk->ctrl->addr))){
//printf("AT THINK LINE %d\n",__LINE__);
					GoEscape(entk);
					at_thk->escaddr = back_addr ;
					return ;
				}
			}else {
//printf("AT THINK LINE %d\n",__LINE__);
				GoEscape(entk);
				if(goback == 1){
					if(side_addr == entk->ctrl->addr){
						at_thk->escaddr = back_addr ;
					}else {
						at_thk->escaddr = side_addr ;
					}
				}
				return ;
			}
		}else {
		/*一人*/
			GoEscape(entk);
			if(goback == 1){
				if(side_addr == entk->ctrl->addr){
					at_thk->escaddr = back_addr ;
				}else {
					at_thk->escaddr = side_addr ;
				}
			}
			return ;
		}
	}
	/*殴りチェック*/
	if(CheckBeat(entk)){
		return ;
	} 


/*回避*/
/*2000.09.05*/
	if( entk->bullet >= entk->max_bullet ){
		/*ATACK NEAR*/
		/*その場でリロード*/
		{
			GoReload( entk );
			return ;
		}
	}

	/*基本攻撃姿勢*/
	testpos = entk->znavi->flore_pos;
	testpos.vy = (float) entk->ctrl->levels[0] + 563.0F ;
	/*しゃがみチェックは一回だけ*/
#if 0
	if((entk->count3 == (AT_THK_RATE*5))&&(fl_atr==0)
		&&(at_thk->dis_rank<2)&&(entk->pl_eyei.dis>2000)
		&&(entk->at_com->siege[entk->u_id]==0)
		&&( GM_PlayerControl->levels[0]== entk->ctrl->levels[0])
		&&(!HZX_OnlineHazardCheck(entk->ctrl->hzx_id,
//			&testpos,&GM_PlayerPosition,
			&testpos,&GM_PlayerFindPos ,
			(HZX_CHK_F_SEGMENT|HZX_CHK_D_SEGMENT),
			HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET ) )
	) {
		at_thk->at_status |= (AT_ST_SQUAT);
	}
#endif
	if(((entk->count3&63)==60)){
		if((entk->pl_eyei.dis <2000)) {
			at_thk->at_status &= (~AT_ST_SQUAT);
		}
	}
	/*防御姿勢チェック*/
	if(entk->act->bodyp.type & ENE_TYPE_TNG_A)
	{
		if(CheckPlayerSight_Shl(entk)==1){
//			at_thk->at_tmptime = 30;
			if(((irnd()>>8)%2)&&(at_thk->at_tmptime < SHL_TIME)) 
				at_thk->at_tmptime++;
		}
		else if(((irnd()>>8)%2)&&(at_thk->at_tmptime>0)) at_thk->at_tmptime--;
		/*行動不能状態ならすぐ盾下げ*/
		if(GM_PlayerStatus & PL_NO_SHOOT) {
			at_thk->at_tmptime = 0;
		}
	}else at_thk->at_tmptime = 0;

	if(at_thk->at_status & AT_ST_SQUAT){
		entk->act->pad = SP_SQUATGUN;
	}else {
		entk->act->pad = SP_READYGUN;
	}

	/*味方死にチェック*/
	if(CheckDeathBed(entk) ){
		return ;
	}
	/*後ろでうごきまわりチェック*/
	if(CheckTngPrudence(entk)) return ;
	/*逃げアクション変更*/
//	if(entk->count3 == 0){
	if(1){
		if(
		( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )
//		&&(at_thk->dis_rank > (KATANA_ATTACK_NUM-1))
		){
			switch(CheckPlayerSightTng( entk ,0,256) ){
				case LEFT_WALL :
					/*右へ行け*/
					entk->act->pad = SP_ROLL_R ;
					break;
				case RIGHT_WALL:
					 /*左へ行け*/
					entk->act->pad = SP_ROLL_L ;
					break;
			}
		}
	}
#if 0
/* For E3 Video*/
#else
/*一定時間で突撃*/
	if(
	((entk->count3 > DIRECT_TICK(180))
	||
	((entk->act->bodyp.type & ENE_TYPE_KATANA )
	&&(entk->count3 > DIRECT_TICK(120))
	)
	)
	&&(at_thk->dis_rank <= (KATANA_ATTACK_NUM-1))
	){
		if(tng_com->now_hang_num == 0){

			if(CheckJumpAttackCondition(entk)) {
				tng_com->jump_attack = TNG_JUMP_ATTACK_CNT;
				GoJumpAttack(entk);
				return ;
			}else {
				entk->think2 = TH2_CHASE ;
				entk->think3 = TH3_ZONE_CHASE_NEAR_ATTACK ;
				entk->count3 = 0;
				return ;
			}
		}
	}
#endif

	if(entk->count3 > DIRECT_TICK(NEAR_SHOOT_DELAY)){
		if(!(entk->act->bodyp.type & ENE_TYPE_KATANA )){
			if(tng_com->now_hang_num == 0){
				StillShoot( entk  ,AT_SHT_NO_RELOAD);
			}
		}
	}
	/*プレイヤ方向を向かせる*/
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}
/*
角の向こうにいるのを察知
威嚇を行う
*/
#define THREAT_DELAY	(AT_THK_RATE*40)

//ENE_AlertGameLevel


/*攻撃状態簡易版 追跡 回避をしない*/
/*呼び出し前に tmptimeに終了時間を設定すること */
static void Think3_AttackStand( entk )
ENETHINK	*entk ;
{
	TNG_COM		*tng_com ;
	ENTK_TENG_A	*	tng_a ;
	AT_THK *at_thk;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

	entk->act->dir = entk->ctrl->turn.vy ;

	if( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR ){
		SetAimPosEyei(entk) ;
	}
	/*終了時間チェック*/
	if((entk->count3 > at_thk->at_tmptime)
	){
		GoNearAttack(entk);
		return;
	}
	/*殴りチェック*/
	if(!(tng_a->status & EVE_TNG_HANG)){
		if(CheckBeat(entk)){
			return ;
		} 
	}
	/*基本攻撃姿勢*/
	at_thk->at_status &= ~AT_ST_SQUAT;
	entk->act->pad = SP_READYGUN;

	if( entk->bullet >= entk->max_bullet ){
		GoReload(entk);
		return ;
	}

#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&entk->ctrl->mov ,250.0F ,&rgb );
	}
#endif

	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
	if(CheckDeathBed(entk) ){
		return ;
	}
	if(
	(!(tng_a->status & EVE_TNG_HANG))
	&&(!(entk->act->bodyp.type & ENE_TYPE_KATANA ))
	){
		/*2000.12.13 視界チェック*/
		if(entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ){
			if(tng_com->now_hang_num == 0){
				RandShoot_TNG( entk );
			}
		}
	}
}


/* ライデンorスネーク死亡時になにもしなくなる */
static void Think3_Stop( entk )
ENETHINK	*entk ;
{
	TNG_COM		*tng_com ;
	ENTK_TENG_A	*	tng_a ;
	AT_THK *at_thk;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

	entk->act->dir = entk->ctrl->turn.vy ;

	if( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR ){
		SetAimPosEyei(entk) ;
	}
	/*基本攻撃姿勢*/
	entk->act->pad = SP_READYGUN;
	entk->count3 ++ ;
}


#if 1
static void Think3_AttackLocker( entk )
ENETHINK	*entk ;
{
	FVECTOR		pos;
	int			dir ;
//	HZX_ZON		*zone ;
	R_INTRPT	*r_intrpt ;

	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

	{
		/*ロッカーあける*/
		if(entk->count3==0){
			r_intrpt = GetLockerInfo(&pos,&dir);
			if(r_intrpt !=NULL){
				GM_RIntrptCallBack( r_intrpt, 100, 5, RINTRP_ORDER_OPEN ) ;
				entk->act->tmp_dir = dir ;
			}else {
				GoNearAttack(entk);
				entk->act->pad = SP_READYGUN;
				return ;
			}
		}
		/*基本攻撃姿勢*/
		at_thk->at_status &= ~AT_ST_SQUAT;
		entk->act->pad = SP_OPEN_LOCKER;
	}
//	entk->think2 = TH2_ATTACK ;
//	entk->think3 = TH3_ATTACK_LOCKER ;

//		SetAimPosPlayer(entk,0);
	if(
	(!(entk->at_com->watch_status & AT_COM_WATCH_LOCKER))
	||(GM_PlayerStatus & PLAYER_FORCE)
	||(entk->act->act_end)
	){
		/*その後の行動*/
#if 1
		GoNearAttack(entk);
//		GoEasyAttack(entk,120);
//		GoEscape(entk);
#else
		escaddr = HZX_FarZoneNavigate( entk->ctrl->hzx_id,
			HZX_Zone1(entk->ctrl->addr),HZX_Zone1(GM_PlayerAddress),&rout);
		zone = HZX_GetZone(entk->ctrl->hzx_id, HZX_Zone1(escaddr) ) ;
		SetZonePos2Fvec(zone ,&pos) ;
		GoPosChase(entk ,&pos) ;
#endif
		return;
	}


	if(entk->pl_eyei.sight == EYE_INFO_SIGHT_IN){
		entk->act->dir = entk->pl_eyei.dir ;
	}else {
		entk->act->dir = entk->act->tmp_dir ;
	}
	entk->count3 ++ ;
}
#endif
/*指定ポイントで防衛*/
/*相当離れるまで攻撃 */
static void Think3_Defence( entk )
ENETHINK	*entk ;
{
	TNG_COM		*tng_com ;
	ENTK_TENG_A	*	tng_a ;
	AT_THK *at_thk;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

	entk->act->dir = entk->ctrl->turn.vy ;



	/*基本攻撃姿勢*/
	/*立ち*/
	at_thk->at_status &= ~AT_ST_SQUAT;
	entk->act->pad = SP_READYGUN;

/**/
	if( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR ){
		SetAimPosEyei(entk) ;
	}

#if 0
	if(at_thk->zone_dis > (entk->sense.eye_s - 1000)){
		GoNearAttack(entk);
		return;
	}
#endif
	/*殴りチェック*/
#if 1
	if(CheckBeat(entk)){
		return ;
	} 
#endif


	if( entk->bullet >= entk->max_bullet ){
		GoReload(entk);
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
	if(CheckDeathBed(entk) ){
		return ;
	}
	if(!(entk->act->bodyp.type & ENE_TYPE_KATANA )){
		if(tng_com->now_hang_num == 0){
			RandShoot( entk );
		}
	}
}

static void Think3_Defence2( entk )
ENETHINK	*entk ;
{
	TNG_COM		*tng_com ;
	ENTK_TENG_A	*	tng_a ;
	AT_THK *at_thk;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

	entk->act->dir = entk->ctrl->turn.vy ;

	/*基本攻撃姿勢*/
	/*しゃがみ*/
#if 0
		at_thk->at_status |= AT_ST_SQUAT;
		entk->act->pad = SP_SQUATGUN;
#else
		/*立ち*/
		at_thk->at_status &= ~AT_ST_SQUAT ;
		entk->act->pad = SP_READYGUN ;
#endif

/**/
	{
		/*戻れる*/
		GoPosChase(entk ,&entk->def_pos);
		at_thk->th2_buf = TH2_ATTACK ;
		at_thk->th3_buf = TH3_DEFENCE ;
		return  ;
	}
	if( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR ){
#if 0
		SetAimPosPlayer(entk,0);
#else
	SetAimPosEyei(entk) ;
#endif

	}
//	if(at_thk->zone_dis > AT_DEF_START_DIS_PL){
	if(at_thk->zone_dis > (entk->sense.eye_s - 1000)){
		GoNearAttack(entk);
//		entk->act->dir = entk->ctrl->turn.vy ;
		return;
	}
	/*殴りチェック*/
#if 1
	if(CheckBeat(entk)){
		return ;
	} 
#endif


	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
	if(CheckDeathBed(entk) ){
		return ;
	}
}



static void Think3_Beyond( entk )
ENETHINK	*entk ;
{
	FVECTOR aimpos,sub,e_pos;
	CONTROL	*ctrl ;
	float trglen,edgelen;

	TNG_COM		*tng_com ;
	ENTK_TENG_A	*	tng_a ;
	AT_THK *at_thk;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

	entk->act->dir = entk->ctrl->turn.vy ;

#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&GM_PlayerFindPos ,250.0F ,&rgb );
	}
#endif
	entk->act->move_s = MoveCautionWalk;
	entk->act->pad = SP_MOVE_RUN ;
	ctrl = entk->ctrl ;
/*2000.09.04 ビヨンドからの戻り中は後退*/
#if 0
	if(GM_PlayerStatus & PLAYER_FORCE){
		GoEscape(entk);
		return ;
	}
#else
	if(
	(GM_PlayerStatus & PLAYER_FORCE)
	&&(entk->pl_eyei.dis < AT_DIS_MIN)
	){
		aimpos = GM_PlayerFindPos ;
		entk->act->pad = SP_BACKWALK ;
		entk->count3 ++ ;
		entk->act->dir = entk->pl_eyei.dir ;
		/*SetMoveModeを使わない場合自分で補正方向セット*/
		entk->act->tmp_dir = entk->pl_eyei.dir ;
		return ;
	}else {
		aimpos = GM_PlayerPosition ;
		aimpos.vy += 1200.0F ;	/* 手の高さ */
	}
#endif

	/*威嚇射撃*/
	/*踏めない地形が多いので 当てるようにしてごまかすかも*/
	entk->bullet = 0 ;
	if(!(entk->act->bodyp.type & ENE_TYPE_KATANA )){
		if(tng_com->now_hang_num == 0){
			ThreatShot( entk ,&aimpos) ;
		}
	}
	if(aimpos.vy > (entk->ctrl->mov.vy+500.0F)){
		aimpos.vy = entk->ctrl->mov.vy+500.0F ;
	}

	if(entk->count3 ==0){
		entk->tmp_time = 0;
	}


	entk->act->aim_pos = aimpos;
//	entk->status2 = ENE_STATUS2_AIM_GUN ;
	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;

//	e_pos = entk->ctrl->mov ;
	/*足の位置*/
	e_pos = entk->znavi->flore_pos ;
//	_sceVu0SubVector(  &sub, &aimpos, &e_pos ) ;
//	trglen = _FVecLen3( &sub ) ;

	trglen = GV_VecLen3F2( &aimpos, &e_pos ) ;

	if(!(entk->at_com->watch_status & AT_COM_WATCH_BEYOND)){
		GoNearAttack(entk);
		return;
	}
	if( HZX_Zone1(entk->ctrl->addr)
		!= HZX_Zone1(entk->com->plzone_in_zone[0]) )
	{
		/*再追跡*/
		entk->think2 = TH2_CHASE;
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0;
		return ;
//		ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), 
//			entk->ctrl, entk->count3 );
//		entk->act->pad = SP_MOVE_RUN ;
//		SetMoveMode(entk) ;
	} else if(entk->tmp_time > 0 ){
		entk->act->pad = SP_READYGUN ;
	}else {
		if( trglen > STOMP_DIS ){
			/*位置ぎめ*/
			/*ゾーン縁までの距離*/
//			GetNearPosInZone( ctrl->hzx_id,entk->com->plzone_in_zone[0],
			GetNearPosInZone( ctrl->hzx_id,ctrl->addr,
			&aimpos,&sub ) ;
			edgelen = GV_VecLen3F2( &sub, &e_pos ) ;
#if 0
	{
		PosBox(&sub ,250.0F ,NULL );
		SVECTOR rgb;
		extern void SigZoneView( int ,SVECTOR * ,float size);

		rgb.vx = 255 ;
		rgb.vy = 0 ;
		rgb.vz = 0 ;
		SigZoneView(ctrl->addr,&rgb,100.0F) ; 
	}
#endif

			if(edgelen < BEYOND_NEAR){
				/*端には到達している*/
				/*踏めるかチェック*/
				if(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN){
					/*手首までオンラインチェック*/
					if(ENE_EyeOnlineCheck( entk->ctrl->hzx_id,
						/*威嚇のみ*/
						&entk->ctrl->mov, &aimpos )){
						entk->act->pad = SP_READYGUN ;
						entk->tmp_time = (AT_THK_RATE*10);
						entk->act->dir = entk->pl_eyei.dir ;
						entk->count3 ++ ;
						return ;
					}
				}
			}
			/*踏むために前進*/
			entk->act->move_s = MoveCautionWalk;
			entk->act->pad = SP_MOVE_RUN ;
			entk->act->dir = entk->pl_eyei.dir ;
			/*SetMoveModeを使わない場合自分で補正方向セット*/
			entk->act->tmp_dir = entk->pl_eyei.dir ;
		} else {
			/*踏む*/
			/*踏めるかチェック*/
			if(
			(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN)
			&&(ENE_EyeOnlineCheck( entk->ctrl->hzx_id,
					&entk->ctrl->mov, &aimpos )
			)){
					entk->tmp_time = (AT_THK_RATE*10);
			}else {
				entk->think2 = TH2_ATTACK;
				entk->think3 = TH3_ATTACK_STOMP ; 
				entk->count3 = 0 ;
				return;
			}
		}
	}

	if((entk->at_com->alert_time%(AT_THK_RATE*2)==0)
	&&(at_thk->dis_rank>0)){
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_WAIT_CHASE ;
		entk->count3 = 0;
		return ;
	}
//dir
	if(entk->tmp_time > 0) entk->tmp_time-- ;
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}
/*************/

#define WAIT_TIME		(AT_THK_RATE*10)
#define SIDE_IN_TIME	(AT_THK_RATE*10)

/*危険なので逃げるか？よけるか？*/
static void Think3_Danger( entk )
ENETHINK	*entk ;
{
	int reach,side=0,sideaddr,side_at,i,tmpaddr; 
	FVECTOR	trgpos[2];
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

	if(entk->count3 == 0){
#if 0
/*TEST*/
	FVECTOR tmptrg;
	tmptrg = GM_PlayerPosition ;
	tmptrg.vx -= 1000.0F ;
	tmptrg.vy -= 1000.0F ;
	tmptrg.vz -= 1000.0F ;
	GoPosChase(entk,&tmptrg) ;
	return ;
#endif

		entk->act->pad = SP_READYGUN ;
		at_thk->safetype = 0;
		/*あいてる安地があるか？*/
		if(CheckSafeZone( entk )) {
		/* 安地あり*/
			reach = HZX_ReachTo( entk->ctrl->addr,at_thk->safeaddr );
			if( reach <= HZX_INDIRECT_REACH ) {
				/*安地が隣接してる*/
				sideaddr = CheckSideNum(entk,1500.0F,trgpos);
				/*安地が右どなりか左どなり*/
				if(HZX_Zone1(sideaddr)==HZX_Zone1(at_thk->safeaddr)){
					side = 1;
				}else if(HZX_Zone2(sideaddr)==
					HZX_Zone2(at_thk->safeaddr)){
					side = 2;
				}
			}
			side_at = CheckSideAT(entk,2100.0F);
			if((side ==1)||(side ==2)){
				if(((side ==1)&&(side_at&RIGHT_WALL))
				||((side ==2)&&(side_at&LEFT_WALL)))
				{
/*横に逃げたいが、隣に他の兵士がいてぶつかる*/
					ResetUseZone(entk);
					if(
					( entk->bullet >= entk->max_bullet )
					){
						/*ATACK NEAR*/
						/*その場でリロード*/
						entk->bullet = 0 ;
						GoReload( entk );
						entk->count3 = 0 ;
						return ;
					}else {
					/*その場で攻撃姿勢で待機*/
						GoEasyAttack(entk,(AT_THK_RATE*10));
						return ;
					}
/*ここに到達した場合、DANGER思考のままでアクション起動*/
//Think3_Danger

					if(side ==1) {
						/*右転がり*/
						at_thk->at_status |= AT_ST_SQUAT;
printf("Think3_Danger\n");
						entk->at_com->rollout = AT_ROLL_COUNT ;
						entk->act->pad = SP_ROLL_R ;
					}else { 
						/*左転がり*/
printf("Think3_Danger\n");
						at_thk->at_status |= AT_ST_SQUAT;
						entk->at_com->rollout = AT_ROLL_COUNT ;
						entk->act->pad = SP_ROLL_L ;
					}
					return ;
				}
			}else {
				/* side == 0 安地が隣接していない*/
				/*安地までゾーン誘導*/
				entk->think2 = TH2_CHASE;
				entk->think3 = TH3_SAFEZONE_CHASE ;
				entk->count3 = 0 ;
//		entk->act->dir = entk->ctrl->turn.vy ;
				return ;
			}
		} else {
/*安地無しの処理*/
			{
#if 1
				/*左右のゾーンを調べる*/
				sideaddr = CheckSideNum(entk,2000.0F,trgpos);
				for(i=0;i<2;i++){
					if(i==0) tmpaddr = HZX_Zone1(sideaddr);
					else tmpaddr = HZX_Zone2(sideaddr);
					reach = Sig_GetRoute( entk->ctrl->hzx_id ,
						(entk->ctrl->addr),AddrSet(tmpaddr));
					if((reach>0)&&(reach<=2)) {
						/*横のゾーンへ誘導*/
						entk->think2 = TH2_CHASE;
						at_thk->escaddr = AddrSet(tmpaddr) ;
						entk->think3 = TH3_ESC_ZONECHASE;
						entk->count3 = 0 ;
						ResetUseZone(entk);
						return;
					}
				}
#endif
			}
			
			/*マップまたぎでなければ逃亡*/
			if(
#if 0
				(HZX_ZoneMapNo( entk->ctrl->addr) == 
				HZX_ZoneMapNo( GM_PlayerAddress ) )
				&&(!(ENE_HZX_GetZone(entk->ctrl->addr)->flag 
				& HZX_ZON_LINK))
#else
	/*2000.12.20 マップ間でも対応*/
			(1)
#endif
			){
				entk->think2 = TH2_CHASE;
				at_thk->escaddr = HZX_NO_ZONE ;
				entk->think3 = TH3_ESC_ZONECHASE;
				entk->count3 = 0 ;
				return;
			}
		}
	}
	GoEasyAttack(entk,(AT_THK_RATE*10));
	return ;
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}
static void Think3_Behind(entk)
	ENETHINK	*entk ;
{
/*TEST*/
	float speed;
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;
	entk->act->dir = entk->ctrl->turn.vy ;

	if(entk->count3 ==0){
		entk->bullet = 0 ;
		at_thk->at_tmptime = 0;
	}
	if(at_thk->at_tmptime>0){
		if(at_thk->safetype&SAFE_BEHIND1){
			entk->act->dir = 0;
		}else if(at_thk->safetype&SAFE_BEHIND2){
			entk->act->dir = 1024;
		}else if(at_thk->safetype&SAFE_BEHIND3){
			entk->act->dir = 2048;
		}else if(at_thk->safetype&SAFE_BEHIND4){
			entk->act->dir = 3073;
		}
	}
	switch (at_thk->at_tmptime){
		case 0:
			entk->act->pad = SP_MOVE_RUN ;
			if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->znavi->flore_pos), 200 ) < 0 ) {
			/*目標到着*/
				at_thk->at_tmptime++;
			}
			entk->act->dir = entk->trgpoint.dir ;
			break;
		case 1:
			entk->act->pad = SP_MOVE_RUN ;
			if(entk->ctrl->n_touches) {
				at_thk->at_tmptime++;
			}
			break;
		case 2:
			entk->act->pad = SP_BEHIND ;
			entk->act->dir +=2048;
			if((ENE_BEHIND_TIME-entk->count3)<AT_THK_RATE){
//				entk->act->dir +=2048;
				speed = (50.0F);
			}else {
//				entk->act->dir +=2048;
				speed = (-25.0F);
			}
			entk->ctrl->step.vx = (speed) * _RsinF( (int)entk->act->dir ) ;
			entk->ctrl->step.vz = (speed) * _RcosF( (int)entk->act->dir ) ;
		break;
	}
	at_thk->at_status &= (~AT_ST_SQUAT);
//	if(!(at_thk->safetype&BEHIND_FLGS)||(entk->count3>ENE_BEHIND_TIME)||(entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )) {
	if(!(at_thk->safetype&BEHIND_FLGS)||(entk->count3>ENE_BEHIND_TIME)){
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));

		GoNearAttack(entk);
//		entk->act->dir = entk->ctrl->turn.vy ;
		return;
	}
	entk->count3 ++;
}

/*安地到着後の行動*/
static void Think3_WaitSafezone( entk )
	ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;
	entk->act->dir = entk->ctrl->turn.vy ;

	/*プレイヤを目標に設定*/
	ResetUseZone(entk);
	at_thk->at_status &= (~AT_ST_SQUAT);

//	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));

	if(entk->count3 == 0){
		/*体の向きをゾーン進行方向へ*/
		ENE_ZoneTrace( entk->znavi, &(entk->trgpoint),entk->ctrl, entk->count3 );
//		if(entk->bullet != 0 ){
		if(1){
			/* その場リロード*/
			/*WAIT SAFE*/
			GoReload(entk);
//		entk->act->dir = entk->ctrl->turn.vy ;
			return ;
		}
	}
	/*ゾーンに到達後*/
	/*一定時間後、再攻撃*/
	entk->act->act_end = 0;
	if((entk->count3 > (AT_THK_RATE*10))&&(entk->count3%(AT_THK_RATE*2) ==0)){
		if(!CheckPlayerSight(entk,0,64)){
			/* 見られてない */
			entk->act->act_end = 1;
		}
	}
	if (entk->act->act_end) {
/*安地待機終了後の行動*/
		entk->think2 = TH2_CHASE ;
#if 1
		entk->think3 = TH3_WAIT_CHASE ; 
#else
		entk->think3 = TH3_ZONE_CHASE ;
#endif
		entk->count3 = 0 ;
//		entk->act->dir = entk->ctrl->turn.vy ;
		return ;
	}
	entk->act->pad = SP_READYGUN ;
	if(EscCancel(entk)) return ;
	entk->act->ctrl->turn.vy = entk->pl_eyei.dir;
	entk->count3 ++;
}
/*低い所に隠れてからの行動*/
/* 立ったりしゃがんだり撃ったり*/
#define	SQUAT_HIDE_TIME (AT_THK_RATE*2)
#define	SQUAT_END_TIME (AT_THK_RATE*20)

static void Think3_WaitLowSafe( entk )
	ENETHINK	*entk ;
{
	int sight ;
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;
	sight = CheckPlayerSight(entk,0,64);
	ResetUseZone(entk);
	at_thk->at_status |= (AT_ST_SQUAT);

	/*プレイヤを目標に設定*/
//	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));

	if(entk->count3 == 0) {
		at_thk->at_tmptime = 0;
		ENE_ZoneTrace( entk->znavi, &(entk->trgpoint),entk->ctrl, entk->count3 );
	}
	if(entk->act->act_end) at_thk->at_tmptime = 1;

	if(entk->count3< SQUAT_HIDE_TIME ) entk->act->pad = SP_READYGUN ;
	else if(entk->count3< (SQUAT_HIDE_TIME+1) ){
		/*弾丸少なければしゃがみリロード*/
		/*それ以外しゃがみ待機*/
		if(entk->bullet >(entk->max_bullet) ){
//		if(entk->bullet != 0 ){
			entk->act->pad = SP_RELOAD_SQUAT ;
			entk->bullet = 0;
		}else {
			entk->act->pad = SP_SQUATHIDE ;
			at_thk->at_tmptime = 1;
		}
		if(at_thk->in_sight) GoEasyAttack(entk,(AT_THK_RATE*10));
	}else if(at_thk->at_tmptime) {
		/*リロード後*/
		/*みられてる間隠れ*/
		if((entk->count3> SQUAT_END_TIME )){
			/*ここでwaitの管轄外に移行*/
			GoEasyAttack(entk,(AT_THK_RATE*10));
//		entk->act->dir = entk->ctrl->turn.vy ;
			return ;
		}else {
			entk->act->pad = SP_SQUATHIDE ;
		}
	}
#if 0
	else {
		if((sight==0)||(entk->count3> SQUAT_END_TIME)){
		GoNearAttack(entk);
			return ;
		}else {
			entk->act->pad = SP_SQUATHIDE ;
		}
	}
#endif
	if(EscCancel(entk)) return ;
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++;
}

static void Think3_KatanaGuard( entk )
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->dir = entk->ctrl->turn.vy ;


	if(entk->count3 == 0){
		entk->act->pad = SP_KATANA_GUARD ;
	}
	if(entk->count3 > DIRECT_TICK(60)){
		entk->act->act_end = 1;
	}
	if (entk->act->act_end) {
//		GoNearAttack(entk);
		GoChaseWait(entk);
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++;
}


#if 0
暗視ゴーグル
act->sw->n_sight = 2 ; 暗視ゴーグルON 
TH3_NSIGHT_ON
#endif

/*----- 中レベル思考モード --------------------------------------------*/
	/*追跡*/
static	void	Think2_Chase( entk )
ENETHINK	*entk ;
{
	ENTK_TENG_A *tng_a ;
	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	switch ( entk->think3 ) {
	    case TH3_ZONE_CHASE :			/* 目標ゾーンまで行く */
			if(tng_a->status & EVE_TNG_HANG){
				Think3_ZoneChasePlayerHang( entk ) ;
			}else {
				Think3_ZoneChasePlayer( entk ) ;
			}
		break ;
		case TH3_DIRECT_CHASE :			/* 同じゾーン内で追いかける */
			if(tng_a->status & EVE_TNG_HANG){
				Think3_DirectChasePlayerHang( entk ) ;
			}else {
				Think3_DirectChasePlayer( entk ) ;
			}
		break ;
		case TH3_SAFEZONE_CHASE :			/* 安全ゾーンへ逃げる */
			Think3_ZoneChaseSafe( entk );
		break ;
		case TH3_SAFEDIRECT_CHASE :			/* 安全ゾーンへ逃げる */
			Think3_DirectChaseSafe( entk );
		break ;
		case TH3_WAIT_CHASE : /*手前の味方を見つつ待機*/
			Think3_ChaseWait( entk );
		break ;
//		case TH3_ATTACK_SIEGE :
//			Think3_AttackSiege( entk );
//		break;
		case TH3_SIDE_MOVE :
			Think3_SideMove( entk );
		break;
#if 0
		case TH3_TURN_CHASE :
			Think3_ChaseTurn(entk);
		break;
#endif
		case TH3_GOSIGN :
//			Think3_GoSign( entk );
		break;
#if 0
		case TH3_ROLL_OUT :
			Think3_ChaseRoll(entk);
		break;
		case TH3_SIDE_STEP :
			Think3_SideStep(entk);
		break;
#endif
//		case TH3_MUKA_ZONE:
//	    	Think3_ZoneChasePlayerM( entk ) ;
//		break ;
//		case TH3_MUKA_DIRECT:
//	    	Think3_DirectChasePlayerM( entk ) ;
//		break;
		case TH3_ESC_ZONECHASE:
			Think3_ZoneChaseEscape(entk);
		break;
		case TH3_ZONE_POS:
			Think3_ZoneChasePos(entk);
		break;
		case TH3_DIRECT_POS:
			Think3_DirectChasePos(entk);
		break;
		case TH3_CHASE_BEYOND :
			Think3_Beyond(entk);
		break;
		case TH3_DEPLOY :
			Think3_ZoneChaseEscape(entk);
			if(CheckX_FireTng(entk) == 0){
				/*散開終了*/
				GoChaseWait(entk);
			}
		break;
		case TH3_DIRECT_CHASE_JUMP  :
			Think3_DirectChaseJump(entk);
		break;
		case TH3_START_JUMP :
			Think3_StartJump(entk);
		break;
		case TH3_ZONE_CHASE_NEAR_ATTACK :
				Think3_ZoneChaseNearAttack( entk ) ;
		break;
		case TH3_DIRECT_CHASE_NEAR_ATTACK :
				Think3_DirectChaseNearAttack( entk ) ;
		break;
	}
}

	/*攻撃*/
static	void	Think2_Attack( entk )
ENETHINK	*entk ;
{
	ENTK_TENG_A *tng_a ;
	TNG_COM		*tng_com ;

	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

	if(entk->think3 != TH3_KATANA_GURAD){
		tng_a->guard_n = tng_com->guard_max ;
	}
	switch ( entk->think3 ) {
	    case TH3_ATTACK_NEAR :
			/*静止して攻撃*/
			Think3_AttackNear( entk ) ;
		break;
		case TH3_ATTACK_BEAT :
			/*格闘*/
	    	Think3_AttackBeat( entk ) ;
		break ;
		case TH3_ATTACK_HANG :
			Think3_AttackHang(entk) ;
		break;
		case TH3_WAIT_SAFE :
			Think3_WaitSafezone( entk );
		break;
		case TH3_WAIT_LOWSAFE :
			Think3_WaitLowSafe( entk );
		break;
	    case TH3_ATTACK_RELOAD :
	    	Think3_AttackReload( entk ) ;
		break;
//		case TH3_ATTACK_SIEGE :
//			Think3_AttackSiege( entk );
//		break;
		case TH3_DANGER :
			Think3_Danger( entk );
		break;
		case TH3_BEHIND:
			Think3_Behind(entk);
		break;
		case TH3_ATTACK_EASY:
			Think3_AttackStand(entk);
		break;
		case TH3_ATTACK_STOMP:
			Think3_AttackStomp(entk);
		break;
		case TH3_ATTACK_GRD_HIGH :
			Think3_GrdAttackHigh(entk);
		break;
		case TH3_THREAT :
			printf("THINK ERR IN SET THREAT!!\n");
//			Think3_Threat(entk) ;
		break;
		case TH3_ATTACK_PEEK :
			Think3_Peek(entk) ;
		break;
		case TH3_DEFENCE :
			Think3_Defence( entk );
		break;
		case TH3_DEFENCE2 :
			Think3_Defence2( entk );
		break;
#if 1
		case TH3_ATTACK_LOCKER :
			Think3_AttackLocker( entk ) ;
		break;
#endif
		case TH3_KATANA_GURAD :
			Think3_KatanaGuard( entk );
		break;
		case TH3_JUMP_ATTACK :
			Think3_KatanaJumpAttack( entk );
		break ;
		case TH3_WAIT :
			Think3_Stop( entk ) ;
		break ;
	}
}
#include "../attacker/discovery_at.c"


/*
威嚇モード中レベル思考 TH2_Threat();
*/
#include "../attacker/threat.c"
/*接近警戒 中レベル思考TH2_Approach()*/
#include "../attacker/approach.c"
/*イントルード 中レベル思考TH2_Intrude()*/
#include "../attacker/intrude.c"

/*対称配置モード 中レベル思考TH2_Symmetry()*/
#include "../attacker/symmetry.c"

/*眠り兵救出モード TH2_Rescue()*/
#include "../attacker/rescue.c"

/*刀防御ターゲットに弾丸ヒットしたら防御姿勢コール*/
static void CheckKanataGuard(ENETHINK *entk){
	ENTK_TENG_A *tng_a ;
//	SVECTOR rgb ;
	tng_a = (ENTK_TENG_A*)entk->eve_a ;

	if(tng_a->katana & TNG_KATANA_BULLET_HIT){
		if((entk->think3 == TH3_KATANA_GURAD)
//		&&(entk->count3 < DIRECT_TICK(KATANA_GUARD_INTR))
		&&(entk->count3 < entk->act->tmp_time)
		){
#if 0
			printf("BULLET HIT !! But count3 is [%d]\n ",entk->count3);
			printf("tmp_time is[%d]\n ",entk->act->tmp_time);
			rgb.vy = 127 ; 
			PosBox(&entk->ctrl->mov ,1250.0F ,&rgb );
#endif
			tng_a->katana &= ~TNG_KATANA_BULLET_HIT ;
			return ;
		}
#if 0
		rgb.vx = 127 ; 
		PosBox(&entk->ctrl->mov ,1250.0F ,&rgb );
		printf("GUARD THINK START !!!\n");
#endif
		entk->think2 = TH2_ATTACK ;
		entk->think3 = TH3_KATANA_GURAD ;
		entk->count3 = 0 ;
	}
	tng_a->katana &= ~TNG_KATANA_BULLET_HIT ;
//	SP_KATANA_GUARD
}
/*----- 高レベル思考モード --------------------------------------------*/

	/*
		危険
	*/
void	Tng_Think1_Alert( entk )
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	ENTK_TENG_A *tng_a ;
	TNG_COM		*tng_com ;

	at_thk = (AT_THK *) entk->character ;
	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

	if(tng_com->phase == TNG_PH_END){
		GoWaitMode(entk);
	}


#ifdef DEBUG_MODE
	if(tng_com->mode & TNG_COM_DEBUG){
		if(tng_a->status & EVE_TNG_HANG){
			PosBox( &(entk->ctrl->mov) ,2000.0F ,NULL );
		}
	}
#endif

	if(entk->act->bodyp.type & ENE_TYPE_KATANA) {
		CheckKanataGuard(entk) ;
	}
#if 0
	{
		SVECTOR rgb;
		int sideaddr ;
		extern void SigZoneView( int ,SVECTOR * ,float size);

		rgb.vx = 255 ;
		rgb.vy = 0 ;
		rgb.vz = 0 ;

		sideaddr = SIG_GetFarDirZone( entk->ctrl->hzx_id,
			entk->ctrl->addr,GM_PlayerAddress ) ;
		SigZoneView(sideaddr,&rgb,100.0F) ; 

	}
#endif
#if 0
	{
		SVECTOR rgb;
		int sideaddr ;
		extern void SigZoneView( int ,SVECTOR * ,float size);

		rgb.vx = 255 ;
		rgb.vy = 0 ;
		rgb.vz = 0 ;

		SigZoneView(entk->znavi->next_addr,&rgb,100.0F) ; 
	}
#endif
#if 0
	{
		extern void PosBox(FVECTOR *,float ,SVECTOR *) ;
		GM_CurrentMap = GM_PlayerMap ;
		PosBox(&entk->act->target_pos,250.0F ,NULL) ;
printf("ACT TARGET VX %f \n",entk->act->target_pos.vx);
	}
#endif
	CheckAttackTarget(entk) ;

	{
		/*コーナーの向こう察知*/
		/*視界外なら不可*/
#if 1
	/*刀は待機*/
	if(!(entk->act->bodyp.type & ENE_TYPE_KATANA )){
		SIG_CheckCorner(entk);
	}else {
		/*気配系フラグを寝かせる*/
		at_thk->at_status &= (~(AT_ST_FEEL|AT_ST_NEXT|AT_ST_FEEL_LOW)) ;
	}
#endif
		/*気配を感じている場合*/
		if(
		(!(SIG_CheckStealthStatus(entk)))&&
		((at_thk->at_status & AT_ST_FEEL)
		||(at_thk->at_status & AT_ST_NEXT)
		||(at_thk->at_status & AT_ST_IN_COVER)
		||(at_thk->found_addr == entk->com->plzone_in_zone[0])
		||(entk->at_com->watch_status & 
		(AT_COM_WATCH_BEYOND|AT_COM_WATCH_INTRUDE|AT_COM_WATCH_LOCKER))
		)
		){
			/*maxやめて値維持*/
			if(entk->alert <= (ALERT_LEVEL_MAX/4)){
				entk->alert = (ALERT_LEVEL_MAX/4) ;
			}
			at_thk->found_addr = entk->com->plzone_in_zone[0] ;
		}else {
		}

	}
/*割り込み的処理はここで*/
	/*優先の低いものから*/
	/*囲みモード*/
//	CheckSymmetry(entk ) ;
	/*人質取られビックリ*/
	AT_CheckHostage(entk) ;
	/*仲間が殺された*/
//	if(entk->think2 == TH2_ATTACK) CheckDeathBed(entk) ;
//
	/*盾壊れチェック*/
//	CheckShield(entk) ;

	switch ( entk->think2 ) {
	    case TH2_CHASE :		/* 追跡 */
			Think2_Chase( entk ) ;
			tng_a->guard_n = tng_com->guard_max ;
		break ;
	    case TH2_ATTACK :		/* 攻撃 */
			Think2_Attack( entk ) ;
		break ;
	    case TH2_DISCOVERY :	/* ビックリして中断 */
			Think2_Discovery( entk ) ;
		break ;
	    case TH2_THREAT :	/* 威嚇状態 */
			Think2_Threat( entk ) ;
		break ;
	    case TH2_APPROACH :	/* 威嚇状態 */
			Think2_Approach( entk ) ;
		break ;
	    case TH2_INTRUDE :	/* イントルード */
			Think2_Intrude( entk ) ;
		break ;
	    case TH2_SYMMETRY :	/* 対称展開 */
			Think2_Symmetry( entk ) ;
		break ;
	    case TH2_RESCUE :	
			Think2_Rescue( entk ) ;
		break ;

	}
	/*発見プロセス*/
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->alert = ALERT_LEVEL_MAX ;
		at_thk->found_addr = entk->com->plzone_in_zone[0] ;
	}
	CheckSightTime(entk);

/*DEBUG*/
#if 0
	{
		int dir1,dir2,subdir,idebug,jdebug;
		if((entk->id ==0)&&(entk->count3==0)){
			for(jdebug=0;jdebug<4;jdebug++){
				dir1 = 1024 * jdebug;
				dir1 &= 4095; 
				for(idebug=-5;idebug<5;idebug++){
					dir2 = dir1+idebug+2048;
					dir2 &= 4095;
					subdir = SIG_CheckDirSub(dir2,dir1);
				}
			}
		}
	}
#endif

}
void TngAlertModeStart( ENETHINK *entk)
{

	AT_THK *at_thk ;
	ENTK_TENG_A *tng_a ;
	BODYPARAM	*bodyp ;

	bodyp = &entk->act->bodyp ;
	tng_a = (ENTK_TENG_A*) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;

#if 0	//by kore
	if ( entk->act->status & ACT_STATUS_UNREAL ) {
		/* 待機所に控えていたら出現場所チェックしてワープ */
		ENE_RouteWarp( entk ) ;
	}
#endif


	ASSERT( entk->at_com != NULL ) ;
	entk->act->CheckPad = TNG_AlertCheckPad ;
	entk->act->keep_pad = -1 ;
	entk->act->dir = -1 ;
	entk->act->pad = 0 ;


	tng_a->status |= EVE_TNG_NO_DEF ;

	ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerFindPos, 
	&GM_PlayerAddress, NULL, &GM_PlayerMap ) ;
//	entk->think_mode = ENE_TH1_ALERT ; 
	entk->think1 = ENE_TH1_ALERT ; 
	if(tng_a->status & EVE_TNG_HANG){
		entk->think2 = TH2_CHASE ; 
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
printf("EVE_TNG_HANG 2 CHASE\n");
	}else {
		GoNearAttack(entk);
	}
	entk->count3 = 0 ;
	entk->act->move_s = MoveAttackRun ;
	entk->act->pad = 0 ;
	at_thk->safetype = 0;
	at_thk->sight_time =0; 
	at_thk->at_status &= (~AT_ST_SQUAT);
	at_thk->in_sight = 0;
	at_thk->safeaddr = HZX_NO_ZONE;
	at_thk->found_addr = HZX_NO_ZONE ;

	if(CheckTngDefPosMode(entk)){
		ENE_SetTrgpPosMap( &(entk->trgpoint),&entk->def_pos, entk->def_mapbit ) ;
	}else {
		ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint));
	}
	ResetUseZone(entk);
	entk->sw_gun &= ~SW_FLAG_SWITCH1 ;


}

void TngAlertModeStartDamage( ENETHINK *entk)
{

	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;
	ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerFindPos, 
	&GM_PlayerAddress, NULL, &GM_PlayerMap ) ;

	at_thk->in_sight = (AT_THK_RATE*20);
	at_thk->sight_time =(AT_THK_RATE*20); 
	GoEscape(entk) ;
}


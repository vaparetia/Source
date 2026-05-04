//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	atthink.c
	attackerの思考制御

	1999/07/06 K.Sigeno
	$Id: atthink.c,v 1.1.1.3 2002/11/19 11:49:01 Yoshizawa1 Exp $
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
#include	"korekado/enemy/enemy.h"
#include	"libutl.h"

#include	"attacker.h"

#include	"../defender/eve_a.h"

#include	"sig_conv.x"
#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif

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

#define	GUN_HAZARD_DIS (1200.0f)
/*対ビヨンド*/
//#define STOMP_DIS (1400)
#define STOMP_DIS (1000.0F)
#define BEYOND_NEAR (STOMP_DIS + 500.0F)

//extern void SIG_CheckCorner(ENETHINK *);
extern void SIG_SetChasePos(ENETHINK *,FVECTOR *) ;
extern int CheckLastAT(ENETHINK *) ;
extern int SIG_CheckFrontSeg(CONTROL *,float) ;


#include "check_at.c"
/*攻撃兵死亡時にコールされる関数*/


/*
リロードチェック
ショットガンモーションのため、PAD状態でリロード許可を調べる
*/
static int CheckStompCondition(ENETHINK *entk){
	FVECTOR	aimpos ,e_pos,sub;
	float trglen ;
	aimpos = GM_PlayerPosition ;
	aimpos.vy = entk->znavi->flore_pos.vy ;	/* 手の高さ */
	/*足の位置*/
	e_pos = entk->znavi->flore_pos ;
	trglen = GV_VecLen3F2( &aimpos, &e_pos ) ;
	if( trglen <= STOMP_DIS ){
		return 1 ;
	}
	/*位置ぎめ*/
	/*ゾーン縁までの距離*/
	GetNearPosInZone( entk->ctrl->hzx_id,entk->ctrl->addr,&aimpos,&sub ) ;
	GetNearPosInZone( GM_PlayerControl->hzx_id,GM_PlayerAddress,&aimpos,&sub ) ;

	trglen = GV_VecLen3F2( &sub, &e_pos ) ;
	if(trglen < BEYOND_NEAR){
		/*端には到達している*/
		return 1 ;
	}
	return 0;
}
static int CheckNarrowAttack(ENETHINK *entk){
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	if(
	(GM_PlayerStatus & PLAYER_NARROW)
//	(GM_PlayerStatus2 & PLAYER2_NARROW_HIDDEN)
//	(GM_CheckPlayerStatusEX(0,PLAYER2_NARROW_HIDDEN))
	&&(at_thk->dis_rank==0)
	&&(entk->ctrl->addr == entk->com->plzone_in_zone[0])
	){
		return 1 ;
	}
	return 0 ;
}
static int AT_CheckReload(ENETHINK *entk){
	if(( entk->act->keep_pad == SP_READYGUN)
	&&( entk->bullet >= entk->max_bullet )
	){
		return 1 ;
	}
	return 0 ;
}

/*盾が破壊された*/
void SIG_CheckShieldBroken(ENETHINK *entk){
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	if((entk->act->bodyp.type &ENE_TYPE_SHIELD)&&(entk->sw.shield == SHL_ST_NOP)) {
		entk->act->bodyp.type &= (~ENE_TYPE_SHIELD) ;
		entk->act->sw->sub_weapon = 0;
		entk->max_bullet = ENE_GetAmmoMax(entk->name_id.weapon) ;
		entk->act->CheckPad = AT_AlertCheckPad ;
	}
}

/*補助思考 近接での思考分岐*/
static int ThinkSubNear( entk )
ENETHINK	*entk ;
{
	int reach,rank_dis = 2000 ,zone_ch,trg_addr;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

/* test */
//COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
/*********/
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		trg_addr = entk->com->plzone_in_zone[0]  ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		trg_addr = *(entk->npc_eyei.addr) ;
	}else {
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
		trg_addr = entk->trgpoint.addr ;
	}
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


	zone_ch = ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint),entk->ctrl, 0 ) ;
#if 0
printf("rank_dis \[%d]\n",rank_dis);
printf("trg_addr [%x] entk->ctrl->addr [%x] \n",entk->trgpoint.addr,entk->ctrl->addr);
printf(" ID [%d] dis_rank [%d] in_sight [%d] dis_dif [%d]\n",entk->id ,at_thk->dis_rank, at_thk->in_sight,at_thk->dis_dif );
printf("this_addr [%x] going_addr [%x] next_addr [%x] zone_ch[%d]\n",entk->znavi->this_addr,entk->znavi->going_addr ,entk->znavi->next_addr,zone_ch) ;
#endif
	if(zone_ch ==1){
		if(CheckOnlyRIntrpt(entk)){
			if((entk->at_com->watch_status & AT_COM_WATCH_LOCKER)
			&&(entk->znavi->next_addr==GM_PlayerAddress)
			){
				if(at_thk->dis_rank == 0 ){
					/*ロッカーモードなので扉直前まで止まらない*/
				}else {
					/*開ける人の邪魔にならないように下がる*/
					GoEscape(entk);
					zone_ch = -2 ;
				}
			}else {
				zone_ch = -2 ;
			}
		}else {
		}
	}
	/*追跡再開 プレイヤがゾーン外の時注意*/
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		if((at_thk->dis_rank == 0 )&&(at_thk->in_sight < AT_THK_RATE*2)
		&&(trg_addr  != entk->ctrl->addr )
		&&((zone_ch != -2))
		){
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_ZONE_CHASE ;
			entk->count3 = 0 ;
			return 1;
		}else {
		}
	}else {

		if((at_thk->dis_rank == 0 )
		&&(trg_addr  != entk->ctrl->addr )
		&&((zone_ch != -2))
		){
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_ZONE_CHASE ;
			entk->count3 = 0 ;
			return 1;
		}else {
		}
	}
	/*手前が遠い*/
	if(
	( at_thk->dis_dif >= (rank_dis+500 )  )
	&&(!((at_thk->dis_rank > 1)&&(at_thk->at_status & AT_ST_NEXT)))
	&&((zone_ch != -2))
	){
/**/
		/*追跡モード*/
/*最後にいたゾーン*/
reach = HZX_ReachTo( entk->ctrl->addr, trg_addr );
		if(reach < HZX_UNREACH){
			if(entk->think3 == TH3_DIRECT_CHASE) return 0;
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_DIRECT_CHASE ;
			entk->count3 = 0 ;
			return 1;
		}else {
			/* 号令 誰が出そう？*/
			/*後ろに2人いる時*/
			/*二番手*/
			if((at_thk->dis_rank == 1)
			&&(entk->com->enemys.group[entk->g_id]->
			unit[entk->u_id]->enemy_num>3)
			&&(!entk->at_com->gosign)
			)
			{
				entk->at_com->gosign = GOSIGN_TIME;
				entk->think2 = TH2_CHASE;
				entk->think3 = TH3_GOSIGN;
				entk->count3 = 0;
				return 1;
			}
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
	&&(GM_PlayerStatus & PLAYER_BEYOND ) 
	&&(GM_PlayerPosition.vy < (entk->ctrl->mov.vy-BEYOND_DIS))
	)
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
#if 0
static void ThinkSubMukade(ENETHINK *entk){
	if(entk->act->mukade_time){
		if(entk->id>0) entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
//		if(entk->id>0) entk->status2 |= ENE_STATUS2_AIM_RHAND ;
		switch(entk->id){
			case 1 :
				entk->act->aim_dir = 
				(entk->trgpoint.dir + 600-entk->act->mukade_time*10)&4095;
				break ;
			case 2 :
				entk->act->aim_dir =
				(entk->trgpoint.dir - 600 + entk->act->mukade_time*10)&4095;
				break ;
			case 3 :
				entk->act->aim_dir =
				(entk->trgpoint.dir + 600 -entk->act->mukade_time*10)&4095;
					break ;
		}
	}
}
#endif
static void SetMukadeRhand(entk)
ENETHINK *entk;
{
	FVECTOR	shift[3];
	int i ,set_id;

	
	for(i=0;i<3;i++){
		shift[i].vx = 0.0F; 
		shift[i].vy = 0.0F; 
		shift[i].vz = 1000.0F; 
	}	

	shift[0].vx = -1000.0F ;
	shift[1].vx = -1500.0F ;
	shift[2].vx = -2000.0F ;

	shift[0].vy = 0.0F ;
	shift[1].vy = -500.0F ;
	shift[2].vy = -1000.0F ;

	set_id = entk->id-1 ;
	if(set_id > 2 ) set_id = 2 ;
	if(entk->id ==0 ) return ;
	DG_SetPos( &BODYWORLD( entk->act->body, HUMAN21_KOSHI ) );
	DG_PutVector( &shift[set_id],&shift[set_id], 1 );

	entk->status2 |= ENE_STATUS2_AIM_RHAND ;
//		entk->act->aim_pos = entk->buddy->ctrl->mov ;
//		entk->act->aim_pos = GM_PlayerPosition;
		entk->act->aim_pos = shift[set_id] ;
//		entk->act->aim_pos.vy -= 1000 ;
}
/*----- 思考ルーチン --------------------------------------------------*/
/*----- 低レベル思考モード --------------------------------------------*/

static void Think3_ZoneChasePlayer( entk )
ENETHINK	*entk ;
{
	EYEINFO *trg_eye ;
//	int reach,rank_dis = 1000,aim_check = 0;
	int rank_dis = 1000, zone_ch,trg_addr;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;
#if 0
printf("TOP OF ZONE CHESE \n");
printf("I AM ID [%d] RANK [%d] DIS_dif [%d] rank_dis[%d]\n",entk->id,at_thk->dis_rank,at_thk->dis_dif,rank_dis);
#endif
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
#if 0
/***2001.09.18**/
		SetAimPosPlayer(entk,0);
/*****/
#endif
		trg_eye = &(entk->pl_eyei) ;
		trg_addr = entk->com->plzone_in_zone[0]  ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
//		SetAimPosNPC_Eyei(entk) ;
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
		trg_eye = &(entk->npc_eyei) ;
		trg_addr = *(entk->npc_eyei.addr) ;
	}else {
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
//		ENE_SetTrgpZone( &(entk->trgpoint), HZX_Zone1(entk->trgpoint.addr), entk->ctrl->hzx_id ) ;
//		SetAimPosTrgp(entk);
		/*未発見時にはtrg_eyeを参照しないのでplをいれてても問題なし*/
		trg_eye = &(entk->pl_eyei) ;
		trg_addr = entk->trgpoint.addr ;
	}



	entk->act->dir = entk->ctrl->turn.vy ;


/*床の状態によって距離を変化*/
	if(CheckFlrAtr(entk)){
		rank_dis += 500;
	}
	if(at_thk->dis_rank > 1){
		rank_dis += 1000;
	}


//printf("ALT ZONE MOVE\n");
//printf("I AM ID [%d] RANK [%d] DIS_dif [%d] rank_dis[%d]\n",entk->id,at_thk->dis_rank,at_thk->dis_dif,rank_dis);

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

	if(CheckLastAT(entk)){
		if(CheckRescueAll(entk)){
			return ;
		}
	}

	/*現在地アドレス更新*/
	if(!(entk->count3%(AT_THK_RATE*5))) {
		entk->znavi->this_addr = 
		HZX_GetAddress( entk->ctrl->hzx_id, 
		&( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;
	}


	/*手前が近けりゃ待機状態*/
	/*06.01 後方待機*/
	if(COM_AlertStatus()&(COM_ALERT_PLAYER_DETECT|COM_ALERT_NPC_DETECT)){
		if(
		(( at_thk->dis_dif < rank_dis  )&&(at_thk->dis_rank > 0))
//		||(HZX_GetZone(entk->ctrl->hzx_id,HZX_Zone1(entk->znavi->next_addr))->flag & HZX_ZONE_INTRUDE)
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
	}else {
		/*未発見時処理*/
//		if( at_thk->dis_dif < rank_dis  ){
		if(( at_thk->dis_dif < rank_dis  )&&(at_thk->dis_rank > 0)){
			GoChaseWait(entk);
			return ;
		}
	}

	/*対ロッカー停止条件*/
	if(entk->at_com->watch_status & AT_COM_WATCH_LOCKER){
		if(at_thk->dis_rank == 0 ){
//			printf("zone_dis [%d] chasedis [%d]\n",at_thk->zone_dis ,entk->at_com->chasedis ) ;
		}
		if((!(GM_PlayerStatus & PLAYER_FORCE))
		&&(at_thk->dis_rank == 0 )
		&&(at_thk->zone_dis < 2000 )
		){
			if(GoAttackLocker(entk) ){

				return ;
			}
		}
		if(at_thk->dis_rank != 0 ){
			if( entk->pl_eyei.dis <= (4000) ){
//				GoEscape(entk);
				GoEasyAttack(entk,DIRECT_TICK(60));
				return ;
			}
		}
	}
	/*目標が近けりゃ攻撃 */
	/*先頭兵はつねにゾーン距離を詰める*/
	if(
	(COM_AlertStatus()&(COM_ALERT_PLAYER_DETECT|COM_ALERT_NPC_DETECT))
	&&
	((at_thk->in_sight>(AT_THK_RATE*2))||(at_thk->at_status & AT_ST_FEEL)||(SIG_CheckStealthStatus(entk)))
	&&(( 
//	(trg_eye->dis < entk->at_com->chasedis )
	(at_thk->zone_dis < entk->at_com->chasedis )
	&&(!(entk->at_com->watch_status & AT_COM_WATCH_BEYOND))))
	){
		/*おしゃべり実験*/
		/*追跡から停止*/
//		if((irnd()>>8)%2){
//		}
//		SIG_AT_VoiceCall(entk,SD_V_C07MAKI,AT_V_TOMARE) ;
		SetAimPosPlayer(entk,0);
		GoNearAttack(entk);
		return ;
	}else {
		if((entk->ctrl->addr == trg_addr)
		){
			GoChaseWait(entk);
			return ;
		}
	}

/*コーナー判定*/
	if((entk->count3>AT_THK_RATE))
	{
		if(
		(!(entk->act->bodyp.type & ENE_TYPE_EVENT_A))
		){
			if(Checkout(entk)){
				 return;
			}
		}
	}

/*ゾーン追跡*/
//intrp
/*
return -1 ; 到達
return -2 進入禁止
return 1 ルート遮断
*/
	zone_ch =
		ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint),
		entk->ctrl, entk->count3 ) ;
	if((zone_ch ==1)&&(!(entk->at_com->watch_status & AT_COM_WATCH_LOCKER))){
		if(CheckOnlyRIntrpt(entk)){
			zone_ch = -2 ;
		}else {
		}
	}

	if(
	(zone_ch == -2)
	){
		entk->act->pad = SP_READYGUN;
		entk->count3 =0;
		entk->think2 =TH2_CHASE;
		entk->think3 =TH3_WAIT_CHASE;
		entk->act->dir = entk->ctrl->turn.vy ;
		return ;
	}
	if ( 
//	( zone_ch != 0 )
//	( zone_ch < 0 )
	( zone_ch == -1 )
	||( zone_ch == -2 )
	||(CheckOnlyRIntrpt(entk))
	){
/*ダイレクト追跡に移行*/
//printf("THIS [%d] NEXT [%d] \n",entk->znavi->this_addr,entk->znavi->next_addr ) ; 
//printf("CheckOnlyRIntrpt[%d]\n",(CheckOnlyRIntrpt(entk)));
//printf("zone_ch[%d]\n",zone_ch);
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		return ;
	}
	/*走りながらも攻撃*/
	if(COM_AlertStatus()&(COM_ALERT_PLAYER_DETECT|COM_ALERT_NPC_DETECT)){

		if( trg_eye->sight >= EYE_INFO_SIGHT_BLURR ){
			RandShoot(entk);
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



/*座標追跡 プレイヤ直接*/
static	void	Think3_DirectChasePlayer( entk )
ENETHINK	*entk ;
{
	EYEINFO *trg_eye ;
	int		reach,rank_dis = 1000 ,trg_addr ;
	AT_THK *at_thk;

	at_thk = (AT_THK *)entk->character ;

	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		SetAimPosPlayer(entk,0);
		trg_eye = &(entk->pl_eyei) ;
//		trg_addr = entk->com->plzone_in_zone[0]  ;
		trg_addr = GM_PlayerAddress  ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		SetAimPosNPC_Eyei(entk) ;
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
		trg_eye = &(entk->npc_eyei) ;
		trg_addr = *(entk->npc_eyei.addr) ;
	}else {
#if 1
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
#else
		ENE_SetTrgpZone( &(entk->trgpoint), HZX_Zone1(entk->trgpoint.addr), entk->ctrl->hzx_id ) ;
#endif
		SetAimPosTrgp(entk);
		/*未発見時にはtrg_eyeを参照しないのでplをいれてても問題なし*/
		trg_eye = &(entk->pl_eyei) ;
		trg_addr = entk->trgpoint.addr ;
	}





	entk->act->dir = entk->ctrl->turn.vy ;
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
	}else {
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
	}

	if( CheckFlrAtr(entk) ){
		rank_dis += 500;
	}
//	if(at_thk->dis_rank > 1){
//		rank_dis += 1000;
//	}

//printf("ALT DIRECT MOVE\n");
//printf("I AM ID [%d] RANK [%d] DIS_dif [%d] rank_dis[%d]\n",entk->id,at_thk->dis_rank,at_thk->dis_dif,rank_dis);


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
	if(
	(entk->at_com->watch_status & AT_COM_WATCH_BEYOND)
	&&(GM_PlayerPosition.vy < (entk->ctrl->mov.vy-BEYOND_DIS))
	){
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
	if(entk->at_com->watch_status & AT_COM_WATCH_LOCKER){
		if((!(GM_PlayerStatus & PLAYER_FORCE))
		&&(at_thk->dis_rank == 0 )
		&&(at_thk->zone_dis < entk->at_com->chasedis )
		){
			if(GoAttackLocker(entk) ){
				return ;
			}
		}
		if(at_thk->dis_rank != 0 ){
			if( entk->pl_eyei.dis <= (3000) ){
				GoEscape(entk);
				return ;
			}
		}
	}


	at_thk->at_status &= (~AT_ST_SQUAT);
	//PLAYER_BEYOND	/*ビヨンドモード中*/
	if(ThinkBeyond(entk)) return ;



	/*手前が近けりゃ*/
	if(COM_AlertStatus()&(COM_ALERT_PLAYER_DETECT|COM_ALERT_NPC_DETECT)){
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
	}else {
		if( at_thk->dis_dif < rank_dis  ){
			GoChaseWait(entk);
			return ;
		}
	}
	/*追跡のための方向設定と距離判定*/
	ENE_DirectTrace( &(entk->trgpoint), &(entk->znavi->flore_pos), (0) ) ;
/*STOP*/


	if(COM_AlertStatus()&(COM_ALERT_PLAYER_DETECT|COM_ALERT_NPC_DETECT)){
		if(
		( trg_eye->dis < entk->at_com->chasedis )
		&&(!(GM_PlayerStatus & PLAYER_BEYOND ) )
		)
		{
		/*攻撃距離内*/
			GoNearAttack(entk);
			entk->act->dir = entk->ctrl->turn.vy ;
			return ;
		}
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
	//	if( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR ){
	//	if( entk->pl_eyei.dis <= (entk->sense.eye_s*2) ){
		if( trg_eye->sight == EYE_INFO_SIGHT_IN ){

#if 0
			switch(entk->pl_eyei.sight){
				case EYE_INFO_SIGHT_IN:
					SetAimPosPlayer(entk,0);
					break;
				case EYE_INFO_SIGHT_BLURR:
					SetAimPosPlayer(entk,1);
					break;
				default :
					SetAimPosPlayer(entk,2);
			}
#else
			SetAimPosPlayer(entk,0);
#endif
			RandShoot(entk);
		}
	}	

//	if(HZX_Zone2(GM_PlayerAddress) !=255){
	if(HZX_Zone2(trg_addr) !=255){
		/*追跡可能*/
		entk->act->pad = SP_MOVE_RUN ;
		SetMoveMode(entk);
		entk->act->dir = entk->trgpoint.dir ;
	}else {
		entk->act->pad = SP_READYGUN ;
		entk->act->dir = trg_eye->dir ;
	}
	entk->count3 ++ ;


}
//#define MUKADE_YOSOMI
/*むかで状態*/
static void Think3_ZoneChasePlayerM( entk )
ENETHINK	*entk ;
{
//	int reach;
	FVECTOR pos;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			/*めくら時の処理*/
			if((at_thk->dis_rank==0)&&( at_thk->zone_dis > entk->at_com->chasedis+1000))
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

	at_thk->at_status &= (~AT_ST_SQUAT);

	/*現在地アドレス更新*/
//	if(!(entk->count3%30)) {
	if(1){
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;
	}
	/*プレイヤを目標に設定*/
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
	}else {
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
	}
	/*相棒を目標設定*/
//	if((entk->at_com->level==0)&&(at_thk->dis_rank>0)){
	if((entk->at_com->level==0)&&(entk->id>0)){
		pos.vx =(float)entk->buddy->ctrl->mov.vx ;
		pos.vy =(float)entk->buddy->ctrl->mov.vy ;
		pos.vz =(float)entk->buddy->ctrl->mov.vz ;
		entk->trgpoint.pos = pos;
		entk->trgpoint.addr = entk->buddy->znavi->this_addr;
	}
	/*ゾーン追跡*/
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		/*ダイレクト追跡に移行*/
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_MUKA_DIRECT ;
		entk->count3 = 0 ;
//		entk->act->dir = entk->ctrl->turn.vy ;
		return ;
	}
	if(entk->at_com->level==0){
		/*中速*/
		entk->act->pad = SP_MUKADE_WALK_LOW ;
		entk->act->move_s = MoveRun ;
		if(at_thk->dis_rank == 0) {
		/*先頭兵は銃構え姿勢*/
			entk->act->move_s = MoveWalkGun ;
		}
		if((at_thk->dis_dif>MUKA_DIS)&&(entk->id)) {
		/* 高速 */
			entk->act->move_s = MoveAttackRun ;
		}else if((at_thk->dis_dif<MUKA_LDIS)&&(entk->id)) {
		/* 低速 */
			/*二番手以降の兵 手前が停止中なら停止*/
			if(entk->buddy->act->status & ACT_STATUS_MOVE){
				entk->act->move_s = MoveWalk;
			}else {
				/*停止*/
				entk->act->pad = SP_READYGUN ;
				/*とりあえず 銃構え状態で待機*/
			}
		}

		else if((entk->id==0)
		&&(entk->pl_eyei.dis < 2000))
		{
			/*デバッグ用 ムカデの先頭*/
			entk->act->pad = SP_READYGUN ;
		}
#ifdef MUKADE_YOSOMI
		ThinkSubMukade(entk);
#else
		SetMukadeRhand(entk);
#endif
	}else {
		/**/
		if(entk->id==0){
			GoNearAttack(entk);
		}else {
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_WAIT_CHASE ; 
		}
		entk->act->move_s = MoveAttackRun ;
		entk->count3 = 0 ;
//		entk->act->dir = entk->ctrl->turn.vy ;
		return ;
	}
	/*体に移動命令*/
	if(
	(entk->trgpoint.dir==0)
	||(entk->trgpoint.dir== -1)
	){
	}
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

/*座標追跡 プレイヤ直接*/
static	void	Think3_DirectChasePlayerM( entk )
ENETHINK	*entk ;
{
	int		reach ;
	FVECTOR pos;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;
	
	entk->act->dir = entk->ctrl->turn.vy ;
	
#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			/*めくら時の処理*/
			if((entk->id==0)&&( at_thk->zone_dis > entk->at_com->chasedis+1000))
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

	at_thk->at_status &= (~AT_ST_SQUAT);
/*ビヨンド中*/
	/*将来的には個別処理必要*/
	if(
	(entk->at_com->level==0)&&
	(entk->id==0)&& ((GM_PlayerStatus & (PLAYER_BEYOND|PLAYER_INTRUDE))||(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE))
	)
	{
		/*先頭が最終目標にまで到達したので、ムカデ状態解除*/
		entk->at_com->level = 1;
		entk->think3 = TH3_WAIT_CHASE ; 
		entk->count3 = 0 ;
		entk->act->move_s = MoveAttackRun ;
//		entk->act->dir = entk->ctrl->turn.vy ;

		return ;
	}

	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
	}else {
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
	}

	if((entk->at_com->level==0)&&(entk->id>0)){
		pos.vx =(float)entk->buddy->ctrl->mov.vx ;
		pos.vy =(float)entk->buddy->ctrl->mov.vy ;
		pos.vz =(float)entk->buddy->ctrl->mov.vz ;
		entk->trgpoint.pos = pos;
		entk->trgpoint.addr = entk->buddy->ctrl->addr;
	}

	/*追跡のための方向設定と距離判定*/
	ENE_DirectTrace( &(entk->trgpoint), &(entk->znavi->flore_pos), (0) ) ;
	/*距離外*/
	/*32フレON/OFF 100000*/
	if( entk->count3 & 0x20){
//	if( entk->count3 == 0){
		/*現在値 計算し直し*/
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), -1 ) ;
	}
	entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id,
	 &( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;

	reach = HZX_ReachTo( entk->znavi->this_addr, entk->trgpoint.addr );


	if(entk->at_com->level==0){
		/*中速*/
		entk->act->pad = SP_MUKADE_WALK_LOW ;
		entk->act->move_s = MoveRun ;
		if(entk->id == 0) {
		/*先頭兵は銃構え姿勢*/
			entk->act->move_s = MoveWalkGun ;
		}
		if((at_thk->dis_dif>MUKA_DIS)&&(entk->id)) {
		/* 高速 */
			entk->act->move_s = MoveAttackRun ;
		}else if((at_thk->dis_dif<MUKA_LDIS)&&(entk->id)) {
		/* 低速 */
			/*二番手以降の兵 手前が停止中なら停止*/
			if(
			(entk->buddy->act->status & ACT_STATUS_MOVE)
			&&(at_thk->dis_dif>MUKA_STOP)
			){
				entk->act->move_s = MoveWalk;
			}else {
				/*停止*/
				entk->act->pad = SP_READYGUN ;
				/*とりあえず 銃構え状態で待機*/
			}
		}else if((entk->id==0)
		&&(entk->pl_eyei.dis < 2000))
		{
			/*デバッグ用 ムカデの先頭*/
			entk->act->pad = SP_READYGUN ;
		}
#ifdef MUKADE_YOSOMI
		ThinkSubMukade(entk);
#else
		SetMukadeRhand(entk);
#endif

	}else {
		if(entk->id==0){
			GoNearAttack(entk);
		}else {
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_WAIT_CHASE ; 
		}
		entk->act->move_s = MoveAttackRun ;
		entk->count3 = 0 ;
//		entk->act->dir = entk->ctrl->turn.vy ;
		return ;
	}
	/* 体に移動命令 */
	if( reach > HZX_INDIRECT_REACH ) {
	/*ゾーン到達してない*/
		entk->think3 = TH3_MUKA_ZONE ;
		entk->count3 = 0 ;
		entk->act->dir = entk->ctrl->turn.vy ;
	}
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;

//dis_rank
}

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
	
	at_thk = (AT_THK *) entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

	if( entk->act->move_s != MoveAttackRun) {
		SetAimPosPlayer(entk,0);
	}

	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->pad = SP_MOVE_RUN ;
	entk->act->dir = entk->ctrl->turn.vy ;
/*ゾーン追跡*/
	if( entk->count3 == 0 ) {
		/*目標地点を安地に設定*/
		if( HZX_Zone1(at_thk->safeaddr) == HZX_NO_ZONE) {
			ResetUseZone(entk);
			/*安地が異常なら近接処理やり直し*/
			if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
			GoNearAttack(entk);
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
	if(
	(ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) < 0 )
	||(SIG_CheckRIntrpt(entk)
	||(SIG_CheckStealthStatus(entk))
	)
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


	if(
	( entk->pl_eyei.dis <= entk->sense.eye_s) 
	&&(entk->pl_eyei.sight != EYE_INFO_SIGHT_OUT_HZD ) 
//	&&(entk->status2 & ENE_STATUS2_AIM_GUN)
//	&&(entk->status2 & ENE_STATUS2_AIM_GUNSHOOT)
	){
		RandShoot(entk);
//		entk->bullet = 0;
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
	if( entk->act->move_s != MoveAttackRun) {
		SetAimPosPlayer(entk,0);
	}
	entk->act->dir = entk->pl_eyei.dir ;
	/*追跡のための方向設定と距離判定*/
	/*ステルス中は逃げない */
	if(
	(ENE_DirectTrace(&(entk->trgpoint),&(entk->znavi->flore_pos),
//	350)<0)
//	50)<0)
	80)<0)
//	||(CheckEnePlOnline(entk))
	||(CheckToucheWallDir(entk,entk->trgpoint.dir ,512 ) )
	||(SIG_CheckStealthStatus(entk))
	){
//		if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
		entk->act->pad = SP_READYGUN ;
		entk->act->mot_speed_correct = 0.0F;
//		entk->ctrl->mov.vx = entk->trgpoint.pos.vx ;
//		entk->ctrl->mov.vz = entk->trgpoint.pos.vz ;

		ResetUseZone(entk);
#if 1
//		if(entk->bullet >= entk->max_bullet){
		if(AT_CheckReload(entk)){

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
//	int		shl_chk=0,i;
	AT_THK *at_thk ;

	at_thk = (AT_THK *) entk->character ;
	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->pad = SP_MOVE_RUN ;
	entk->act->dir = entk->ctrl->turn.vy ;

#if 0
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
	SetAimPosPlayer(entk,0);

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
				GetFarZone(GM_PlayerAddress,2000,entk);
#endif
		}else {
		}
		if((at_thk->escaddr == entk->ctrl->addr) 
		||( ENE_ZoneIntrptCheck( at_thk->escaddr))
		){
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
		if(ENE_ReadOnlinInfo(entk->ctrl->addr,GM_PlayerAddress)){
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


	if(at_thk->at_status & AT_ST_ABS_ESCAPE){
		entk->act->move_s = MoveRun ;
	}else if(entk->count3%(AT_THK_RATE*2)==0){
		SetMoveMode(entk);
	}
#endif
/*2000.07.14転がり逃げ実験*/
	if((Sig_GetRoute( entk->ctrl->hzx_id,entk->trgpoint.addr,
	entk->ctrl->addr)<=HZX_INDIRECT_REACH)
	&&(ENE_ReadOnlinInfo(entk->trgpoint.addr,GM_PlayerAddress))
	&&(at_thk->at_status & AT_ST_ABS_ESCAPE)
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
	if(
	( entk->pl_eyei.dis <= entk->sense.eye_s) 
	&&(entk->pl_eyei.sight != EYE_INFO_SIGHT_OUT_HZD ) 
	&&(entk->act->pad == SP_MOVE_RUN)
	&&(!(entk->act->bodyp.type & ENE_TYPE_SHOTGUN))
//	&&(entk->status2 & ENE_STATUS2_AIM_GUN)
//	&&(entk->status2 & ENE_STATUS2_AIM_GUNSHOOT)
	){
		RandShoot(entk);
//		entk->bullet = 0;
	}
	if(CheckBeat(entk)){
		return ;
	} 

#if 1
	if(
	( entk->act->move_s == MoveAttackRun) 
	||( entk->act->move_s == MoveRun) 
	){
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
	entk->count3 ++ ;
}
//intr
/*指定ポイントまで移動*/
static void Think3_ZoneChasePos( entk )
ENETHINK	*entk ;
{
//	HZX_ZON		*esczone; /*ゾーン*/
//	FVECTOR		pos;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;
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


	if( (GM_GameStatus & STATE_VR_ANOTHER)&&(at_thk->at_status & AT_ST_DEFENSE)  ){
		/*通常歩き*/
		entk->act->move_s = MoveAttackRun ;
		entk->act->dir = entk->trgpoint.dir ;
	}else {
		RandShoot( entk );
		if(entk->count3%(AT_THK_RATE*2)==0){
			SetMoveMode(entk);
		}
	}
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

//printf("Think3_DirectChasePos\n");

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
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
	}else {
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
	}
		if(at_thk->th2_buf >= 0){
			SetReturnThink( entk ) ;
		}else {
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_WAIT_CHASE ; 
		}

		/*実験 撤退*/
		ResetWaitStatus(entk);
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
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
	}else {
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
	}
		if(at_thk->th2_buf >= 0){
			SetReturnThink( entk ) ;
		}else {
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_WAIT_CHASE ; 
		}
		/*実験 撤退*/
		ResetWaitStatus(entk);
		entk->count3 = 0 ;
		return ;
	}
#if 0
	/*SYM用にいろいろ追加 2000.07.27*/
	RandShoot( entk );
	if(entk->count3 < 120){
		SetMoveMode(entk);
	}else {
		entk->act->move_s = MoveAttackRun ;
	}
#else
	RandShoot( entk );
	SetMoveMode(entk);
#endif
	if( entk->act->move_s == MoveAttackRun) {
		entk->act->dir = entk->trgpoint.dir ;
	} else {
		entk->act->dir = entk->pl_eyei.dir ;
	}
	entk->count3 ++ ;
}


/*格闘*/
static	void	Think3_AttackBeat( entk )
	ENETHINK	*entk ;
{
	int sw ;
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	if(entk->count3 == 0){
//		if(GM_PlayerStatus & (PLAYER_GROUND|PLAYER_SQUAT|PLAYER_CB_BOX)) {
		if(GM_PlayerStatus & (PLAYER_GROUND|PLAYER_CB_BOX)) {
			entk->act->pad = SP_STOMP ;
		}else {
			if(entk->act->bodyp.type & ENE_TYPE_SHIELD){
			/*盾兵*/
				if(((irnd()>>8)&1)){
					/*盾殴り*/
					entk->act->pad = SP_SHL_PUSH ;
				}else {
					entk->act->pad = SP_ATTACK_KICK ;
				}
			}else {
				/*通常装備*/
				sw = ((irnd()>>8)&3) ;
				switch(sw) {
					case 0 :
						entk->act->pad = SP_ATTACK_PUNCH_L ;
						break ;
					case 1 :
						entk->act->pad = SP_ATTACK_PUNCH_R ;
						break ;
					case 2 :
						entk->act->pad = SP_ATTACK_KICK_L ;
						break ;
					case 3 :
						entk->act->pad = SP_ATTACK_KICK_R ;
						break ;
				}
			}
		}
		at_thk->at_tmptime = 0;
	}

#if 1
	if(
	(entk->act->act_end)
	||(entk->count3 >= AT_THK_RATE*20)
	){
//printf("BEAT ACT END\n");
		/*連続殴り防止*/
//		GoEasyAttack(entk,AT_THK_RATE*5);
		GoNearAttack(entk);
		return ;
	}
#else
	/*プレイヤとの距離判定でatacknearへ*/
	/*モーション終わりで*/
	if(entk->act->act_end){
		if(at_thk->at_tmptime==0) at_thk->at_tmptime = 1;
	}
	if(at_thk->at_tmptime){
		at_thk->at_tmptime++;
		entk->act->pad = SP_READYGUN;
		/*attackNearと同形 逃げられたら*/
		if(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ) {
			if(
			((entk->at_com->com_sight <= entk->at_com->siege_num)
			&&(at_thk->dis_rank <= entk->at_com->siege_num))
			||( entk->pl_eyei.dis > (entk->at_com->chasedis+2000)   ) 
		/*でもあまりに離れたら追跡に参加*/
			)
			{
				if(ThinkSubNear( entk )){
//					entk->act->dir = entk->ctrl->turn.vy ;
					return;
				}
			}
		}

		if(at_thk->at_tmptime>(AT_THK_RATE*20)){
			GoNearAttack(entk);
			return ;
		}
		StillShoot( entk  ,AT_SHT_NO_RELOAD);
	}
#endif
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
//	entk->act->dir = entk->ctrl->turn.vy ;
	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	entk->act->dir = GV_VecDir2FromTo( &entk->ctrl->mov,&GM_PlayerPosition ) ;
	if(entk->count3 == 0){
		entk->act->pad = SP_STOMP ;
		at_thk->at_tmptime = 0;
	}
	if((GM_PlayerStatus & PLAYER_FORCE)
	&&(entk->pl_eyei.dis < AT_DIS_MIN))
	{
		entk->act->pad = SP_BACKWALK ;
	}

	if(entk->act->act_end){
		/*踏めるかチェック*/
		/*足の位置*/
		e_pos = entk->znavi->flore_pos ;
		aimpos = GM_PlayerPosition ;
		aimpos.vy += 1200.0F ;	/* 手の高さ */
		trglen = GV_VecLen3F2( &aimpos, &e_pos ) ;
		if(
#if 1
		(CheckStompCondition(entk))
#else
		(trglen <= STOMP_DIS)
		&&(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN)
		&&(ENE_EyeOnlineCheck( entk->ctrl->hzx_id,
			&entk->ctrl->mov, &aimpos )
		)
#endif
		){
			/*その場で踏み直し*/
			entk->count3 = 0;
			entk->act->pad = SP_STOMP ;
//printf("STOMP2 STOMP\n");
		}else{
#if 1
//printf("STOMP2 CHASE\n");
//			entk->act->pad = SP_READYGUN ;
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_WAIT_CHASE ;
			entk->count3 = 0;
#endif
		}
		return ;
	}
	/*プレイヤ方向を向かせる*/
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
	
	if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
		at_thk->at_status &= ~AT_ST_SQUAT ;
		entk->act->pad = SP_RELOAD ;
	}else if(at_thk->at_status & AT_ST_SQUAT){
		/*しゃがみ状態*/
		entk->act->pad = SP_RELOAD_SQUAT ;
	}else {
		entk->act->pad = SP_RELOAD ;
	}
	if ( entk->act->act_end ) {
#if 0
printf("RELOAD ACT END\n");
printf("RELOAD PAD %d\n",entk->act->pad);
printf("KEEP PAD %d\n",entk->act->keep_pad);
#endif
		if((at_thk->at_status & AT_ST_NEXT)
//		&&( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN )
//		&&(entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR)
		&&(entk->pl_eyei.sight >= EYE_INFO_SIGHT_OUT_HZD)
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
//		GoNearAttack(entk);
		GoChaseWait(entk);
		return ;
	}
	/*リロードキャンセル逃亡実験*/
	if(
	(ENE_AlertGameLevel >= AT_ESC_LEVEL)
	&&(at_thk->sight_time > entk->at_com->esctime )
	){
printf("RELOAD 2 ESCAPE\n");
//		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		GoDanger( entk ) ;
		return;
	}

#if 0
	if(entk->count3 == 0){
		entk->act->dir = entk->pl_eyei.dir ;
	}
#else
	if((COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)&&(!(SIG_CheckStealthStatus(entk)))){
		entk->act->dir = entk->pl_eyei.dir ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		entk->act->dir = entk->npc_eyei.dir ;
	}
#endif
	entk->count3 ++ ;
}





/*転がり飛び出し*/
static void Think3_ChaseRoll( entk )
ENETHINK	*entk ;
{
	int side = 0;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

	at_thk->at_status |= AT_ST_SQUAT;
	/*方向調べ*/
	if(entk->count3==0) {
		/*転がり発生条件を満たしていたらさらに壁チェックなど*/
		if(
		(at_thk->dis_rank == 0)
		&&(!(CheckFlrAtr(entk)))
#if 0
		&&( at_thk->zone_dis > 3000)
#endif
		){
			/*ころがり空間があるかチェック*/
#if 0
			side = CheckSideSafe(entk,2000.0F);
			side |= CheckSideAT(entk,2000.0F);
#else 
//			side = CheckSideAT(entk,2000.0F);
#endif
		}else {
			/*ころがり禁止*/
			side = 
				(RIGHT_NOZONE |RIGHT_WALL|LEFT_NOZONE |LEFT_WALL) ;
		}
		if(
		(CheckTurnSide(entk)) 
		){
			if((side&RIGHT_NOZONE)||(side&RIGHT_WALL)){
				entk->act->pad = SP_STEP_R;
			}else {
				if(CheckRollTime(entk)){
					entk->act->pad = SP_ROLL_R;
				}else {
					entk->act->pad = SP_STEP_R;
				}
			}
		}else {
			if((side&LEFT_NOZONE)||(side&LEFT_WALL)){
				entk->act->pad = SP_STEP_L;
			}else {
				if(CheckRollTime(entk)){
					entk->act->pad = SP_ROLL_L;
				}else {
					entk->act->pad = SP_STEP_L;
				}
			}
		}
		entk->ctrl->turn.vy = entk->pl_eyei.dir;
/*転がり出の変わりに覗き実験*/
#if 1
		if(at_thk->at_status & AT_ST_NEXT) {
			if(entk->act->pad == SP_STEP_R){
				GoAttackPeek(entk,MoveSideR);
				return;
			}else if(entk->act->pad == SP_STEP_L){
				GoAttackPeek(entk,MoveSideL);
				return;
			}
		}
#endif
	}

	if ( entk->act->act_end ) {
		entk->act->pad = -1;
		entk->act->dir = -1 ;
		GoNearAttack(entk);
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir;

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

	SetAimPosPlayer(entk,0);
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
		int test ;
		test = CheckSideAT(entk,500.0F);
		side = test ;
		test = CheckWallDir(entk); 
		side |= test;
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
	if((!(entk->act->bodyp.type & ENE_TYPE_SHIELD))
	&&(entk->count3%(AT_THK_RATE*2)==0)
	){
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
	RandShoot(entk);
	entk->act->dir = entk->pl_eyei.dir;

	/*AK装備以外は覗けないからノーチェック*/
	if(
	(!(entk->act->bodyp.type & ENE_TYPE_SHIELD))
	&&(!(entk->act->bodyp.type & ENE_TYPE_SHOTGUN))
	){
		if(
		(at_thk->at_status & AT_ST_NEXT) 
		&&
		((entk->act->move_s == MoveSideR )
		||(entk->act->move_s == MoveSideL ))
		){
			GoAttackPeek(entk,entk->act->move_s);
			return;
		}
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
		entk->act->target_pos.vy = GM_PlayerControl->levels[0] ;

		at_thk->at_tmptime = 0;
		entk->act->pad = SP_GRD_HIGH;
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
			if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
				ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
			}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
				ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
			}else {
//				ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
				SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
			}
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
//TH3_GRD_M4_SHOOT
static void Think3_GrdM4Shoot(entk)
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;
//printf("Think3_GrdM4Shoot\n");
	entk->act->dir = entk->ctrl->turn.vy ;
	entk->act->pad = SP_GRD_M4_SHOOT ;
#if 1
//	if((entk->count3 == 0)||( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )){
	if((entk->count3 == 0)||( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR)){
		entk->act->target_pos = GM_PlayerPosition ;
		at_thk->at_tmptime = 0;
	}
#else
	if(entk->count3 == 0){
		entk->act->target_pos = GM_PlayerPosition ;
		at_thk->at_tmptime = 0;
	}
#endif
	if(entk->count3 >= DIRECT_TICK(110)){
		at_thk->at_status |= AT_ST_NO_SHOT ;
	}
	if(
	(entk->act->act_end==1)
	||(entk->count3 > AT_THK_RATE*50)
	){
//printf("GRD_2 RELOAD![%d]\n",entk->count3);
		entk->act->pad = SP_RELOAD ;
		GoReload(entk);
//		GoChaseWait(entk);
		return ;
	}
/*殴りチェック*/
#if 0
	if(CheckBeat(entk)){
		return ;
	} 
#endif

#if 1
	entk->act->dir = GV_VecDir2FromTo( &entk->ctrl->mov,&entk->act->target_pos ) ;
#else 
	entk->act->dir = entk->pl_eyei.dir;
#endif	
	entk->count3++;
}


/*角から覗く*/
static void Think3_Peek(entk) 
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;
	entk->act->dir = entk->ctrl->turn.vy ;

	if(((entk->count3%(AT_THK_RATE*10))==0)||(entk->at_com->Pl_StayTime ==0)){
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
			SetAimPosPlayer(entk,0);
#if 0
			if(
			( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN )
			||(at_thk->sight_time > entk->at_com->esctime )
			){
				at_thk->at_tmptime++;
				break;
			}
#endif
			ThreatShot(entk,&entk->act->aim_pos);
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
	int subdir ,zone_ch;
	int rank_dis = 1000 ;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

//printf("WAIT START ID [%d]\n",entk->id);


	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
//		SetAimPosPlayer(entk,0);
		SetAbsAimPosPlayer(entk);
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
		SetAimPosNPC_Eyei(entk) ;
	}else {
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
		SetAimPosTrgp(entk);
	}

//	entk->status2 |= (ENE_STATUS2_AIM_FACE|ENE_STATUS2_AIM_GUNSHOOT) ;
//	entk->act->aim_pos.vy +=3000.0f ;


	entk->act->dir = entk->ctrl->turn.vy ;


	if( CheckFlrAtr(entk) ){
		rank_dis += 500;
	}
	if(at_thk->dis_rank > 1){
		rank_dis += 1000;
	}
	
#if 1
	if((GM_PlayerStatus & PLAYER_GROUND )&&(at_thk->dis_rank==0)){
		at_thk->at_status |= AT_ST_SQUAT ;
		entk->act->pad = SP_SQUATGUN;
//printf("WAIT SQUAT ID [%d]\n",entk->id);
	}else {
		at_thk->at_status &= (~AT_ST_SQUAT);
		entk->act->pad = SP_READYGUN;

//printf("WAIT STAND ID [%d]\n",entk->id);
	}
#else
	if(at_thk->in_sight > entk->at_com->shoot_delay){
		if((GM_PlayerStatus & PLAYER_GROUND )&&(at_thk->dis_rank==0)){
			at_thk->at_status |= AT_ST_SQUAT ;
			entk->act->pad = SP_SQUATGUN;
		}else {
			at_thk->at_status &= (~AT_ST_SQUAT);
			entk->act->pad = SP_READYGUN;
		}
	}else {
//			if(SIG_CheckFrontSeg(entk->ctrl,GUN_HAZARD_DIS)){

		entk->act->pad = SP_WAIT ;
		entk->status2 &= ~ENE_STATUS2_AIM_GUN ;

	}
#endif

	if((entk->count3 % (AT_THK_RATE*10))==0){
		if(CheckDefPosMode(entk)){
		/*拠点防衛に出発*/
			GoDefPosChase(entk);
			return ;
		}
	}

	/*対イントルード*/
	if(entk->at_com->watch_status & AT_COM_WATCH_INTRUDE ){
		if(at_thk->zone_dis < 5000 ){
			if(at_thk->dis_rank ==0){
				GoIntZoneChase(entk);
			}else {
				GoIntWait(entk);
			} 
			return ;
		}else {
			if((at_thk->dis_rank ==0)&&(entk->count3> DIRECT_TICK(60))
			&&(HZX_GetZone(entk->ctrl->hzx_id,HZX_Zone1(entk->znavi->next_addr))->flag & HZX_ZONE_ZINTRPT)
			){
				entk->at_com->watch_status &= ~AT_COM_WATCH_INTRUDE ;
			}
		}
	}
	/*対ロッカー*/
#if 1
	if(
	(entk->at_com->watch_status & AT_COM_WATCH_LOCKER)
	&&(!(GM_PlayerStatus & PLAYER_FORCE))
	&&(at_thk->dis_rank == 0 )
	){
		entk->think2 = TH2_CHASE ; 
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}
#endif
	zone_ch =
		ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint),
		entk->ctrl, entk->count3 ) ;

	if(zone_ch ==1){
		if(CheckOnlyRIntrpt(entk)){
			zone_ch = -2 ;
		}else {
		}
	}


	/*接近モード*/
#if 1
	if(
	( at_thk->dis_rank>0)&&(zone_ch != -2)
	){
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
#if 1
	if(
	( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )
	&&(!(GM_PlayerStatus & PLAYER_BEYOND) )
	){
		/*強制モーション中は思考進行しない*/
		if(!(GM_PlayerStatus & PLAYER_FORCE)){
			GoNearAttack(entk);
			return ;
		}
	}
#else
	if(
	( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )
	&&(!(entk->act->bodyp.type & ENE_TYPE_SHOTGUN))
	){
		StillShoot( entk  ,AT_SHT_NO_RELOAD);
	}
#endif
	/*プレイヤを見失ってなければ進行*/
	if(!(GM_PlayerStatus & PLAYER_BEYOND) )
	{
		if(ThinkSubNear( entk )) {
			return;
		}
#if 0
		if(COM_AlertStatus()&(COM_ALERT_PLAYER_DETECT|COM_ALERT_NPC_DETECT)){
		}else {
			/*未発見時はその場で停止*/
			entk->act->dir = entk->trgpoint.dir ;
			return ;
		}
#endif
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
				if((entk->at_com->alert_time%(AT_THK_RATE*2)==0)&&(at_thk->dis_rank==0)
				&&(GM_PlayerPosition.vy < (entk->ctrl->mov.vy-BEYOND_DIS))
				){
					/*追跡可能なところなら*/
					ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint),entk->ctrl, entk->count3 ) ;
					if(HZX_GetZone(entk->ctrl->hzx_id,HZX_Zone1(entk->znavi->next_addr))->flag & HZX_ZONE_ZINTRPT){
#if 0
						/*攻撃不能なところは見失う*/
						if((at_thk->dis_rank == 0)
						&&(entk->count3> DIRECT_TICK(120)))
						{
							entk->at_com->watch_status &= ~AT_COM_WATCH_BEYOND ;
						}
#endif
					}else {
						entk->think2 = TH2_CHASE;
						entk->think3 = TH3_CHASE_BEYOND ;
						entk->count3 = 0 ;
						return ;
					}
				}else {
					if(at_thk->dis_rank!=0){
						if(ThinkSubNear( entk )) {
							return;
						}
					}
				}
			}
		}
	}
	/*行けないけど追い詰めてる*/
	/*2001.06.26　ポンプ室モード*/
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		HZX_ZON		*zone; /*ゾーン*/
		zone = HZX_GetZone(GM_PlayerHzxID,HZX_Zone1(GM_PlayerAddress) ) ;
		if(entk->count3 > DIRECT_TICK(90)){
			if((at_thk->dis_rank == 0)
//			&&(zone->flag & HZX_ZONE_NO_AVOID)
			&&(entk->at_com->watch_status & AT_COM_WATCH_NO_AVOID)
			){
				GoGrdHigh(entk);
				return ;
			}
			if(
//			(at_thk->dis_rank == 0)&&
			(at_thk->at_status & AT_ST_FEEL)
			&&(!(zone->flag & HZX_ZONE_NO_AVOID))
			){
				GoThreat(entk);
				return ;
			}
		}
	}
//	if( entk->bullet >= entk->max_bullet ){
	if(AT_CheckReload(entk)){
		/*ATACK NEAR*/
		GoReload( entk );
		return ;
	}

	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		/*プレイヤ方向を向かせる*/
		if(
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
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
		entk->act->dir = entk->npc_eyei.dir;
	}else {
//		entk->act->dir = entk->trgpoint.dir ;
		entk->act->dir = GV_VecDir2FromTo( &entk->ctrl->mov,&entk->trgpoint.pos ) ;
	}
	if(CheckDeathBed(entk) ){
		return ;
	}
	/*2000.06.19 後ろでうろうろ*/
	if(!(entk->at_com->watch_status)){
		if(CheckPrudence(entk)) {
			return ;
		}
	}

	if(entk->count3 > DIRECT_TICK(60)){
		if(CheckNarrowAttack(entk)){
			GoGrdHigh(entk);
			return ;
		}
	}

	entk->count3 ++ ;
	if(
	(entk->at_com->watch_status & AT_COM_WATCH_LOCKER)
	&&(at_thk->dis_rank > 0)
	){
		return ;
	}

	if(!(entk->act->bodyp.type & ENE_TYPE_SHOTGUN)){
		if(AT_CheckReload(entk)){
			GoReload( entk );
			return ;
		}
		if(at_thk->in_sight > entk->at_com->shoot_delay){
			RandShoot(entk);
			return ;
		}
	}
#if 0
	if(entk->act->bodyp.type & ENE_TYPE_EVENT_A){
		SIG_BasicShoot(entk,&GM_PlayerFindPos,0);
	}
#endif
}
#define SHL_TIME AT_THK_RATE*10
#define DEF_TIME  AT_THK_RATE*5

// GM_PlayerStatus & PLAYER_DARK_AREA /* 暗闇 */
static void Think3_AttackNear( entk )
ENETHINK	*entk ;
{
/*beyond*/
	int shl_dif,fl_atr,count3_buf;
	FVECTOR		testpos;
//	SVECTOR rgb;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;
	count3_buf = entk->count3 ;

	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
//		SetAimPosPlayer(entk,0);
		SetAbsAimPosPlayer(entk);
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
		SetAimPosNPC_Eyei(entk) ;
	}else {
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
		SetAimPosTrgp(entk);
	}



	entk->act->dir = entk->ctrl->turn.vy ;
	/*ショットガンは立って撃て*/
	if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
		at_thk->at_status &= ~(AT_ST_SQUAT);
	}


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
		if(CheckDefPosMode(entk)){
		/*拠点防衛に出発*/
			GoDefPosChase(entk);
			return ;
		}
	}

#if 0
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

#else
	if(entk->at_com->watch_status & AT_COM_WATCH_INTRUDE ){
		if(at_thk->zone_dis < 5000 ){
			if(at_thk->dis_rank ==0){
				GoIntZoneChase(entk);
			}else {
				GoIntWait(entk);
			} 
			return ;
		}else {
			if((at_thk->dis_rank ==0)&&(entk->count3> DIRECT_TICK(60))
			&&(HZX_GetZone(entk->ctrl->hzx_id,HZX_Zone1(entk->znavi->next_addr))->flag & HZX_ZONE_ZINTRPT)
			){
				entk->at_com->watch_status &= ~AT_COM_WATCH_INTRUDE ;
			}
		}
	}

#endif

	if(
	(entk->at_com->watch_status & AT_COM_WATCH_BEYOND)
	&&(!(GM_PlayerStatus & PLAYER_FORCE))
	&&(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN )
	){
		/*見失ってない*/
		if((entk->at_com->alert_time%(AT_THK_RATE*2)==0)&&(at_thk->dis_rank==0)
		&&(GM_PlayerPosition.vy < (entk->ctrl->mov.vy-BEYOND_DIS))
		){
			entk->think2 = TH2_CHASE;
			entk->think3 = TH3_CHASE_BEYOND ;
			entk->count3 = 0 ;
			return ;
		}
	}

	/*フロア属性チェック*/
	fl_atr = CheckFlrAtr(entk);

/*割り込みアクション終了時にthinkをリスタート*/
	/*先頭兵は常にゾーン距離で追跡*/
	/* 目標が見えなくてもチーム内の誰かが捕捉中なら */
	/* あわてて追跡しない */
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		if((entk->pl_eyei.sight != EYE_INFO_SIGHT_IN )){
			if(
			( (entk->at_com->com_sight <= entk->at_com->siege_num)
			&&(at_thk->dis_rank <= entk->at_com->siege_num) )
			||( at_thk->zone_dis > entk->at_com->chasedis+1000 ) 
			)
			{
				if(ThinkSubNear( entk )){
					if((count3_buf > DIRECT_TICK(60))
					&&((entk->think3 == TH3_ZONE_CHASE)||(entk->think3 == TH3_DIRECT_CHASE))
					){
						SIG_AT_VoiceCall(entk,SD_V_ATKO03,AT_V_NIGETAZO) ;
					}
					return;
				}
			}
		}
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		if((entk->npc_eyei.sight != EYE_INFO_SIGHT_IN )){
			{
				if(ThinkSubNear( entk )){
					return;
				}
			}
		}
	}else {
		if(entk->ctrl->addr != entk->at_com->at_trg_addr){
			if(ThinkSubNear( entk )){
				return;
			}
		}
	}

	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		/*距離保ち*/
		if(
		((entk->pl_eyei.dis < (entk->sense.eye_s*3/4) )
		||
		(at_thk->sight_time > (AT_THK_RATE*2) ))
		&&(at_thk->in_sight > (AT_THK_RATE*10))
		&&(!(GM_PlayerStatus & PLAYER_DARK_AREA))
		&&(!(GM_PlayerStatus & AT_PL_HIDE))
//	&&(!( entk->act->bodyp.type & ENE_TYPE_SHIELD))
//	&&(!( entk->act->bodyp.type & ENE_TYPE_HITECH))
		&&((entk->count3 %(AT_THK_RATE*5))==((AT_THK_RATE*5)-1))
		&&(entk->at_com->berserk==0)
		){
			ENETHINK	*back_entk;
			AT_THK		*back_at_thk;
			int rout,back_addr ,goback = 0 ,side_addr ;
			if(
			( HZX_Zone1(entk->ctrl->addr) == HZX_Zone1(GM_PlayerAddress))
			){
				/*プレイヤと同じゾーンの場合
				自分に座標の近い隣接ゾーンを目標とする*/
				side_addr = back_addr = CheckPosNearZone( 
				&entk->ctrl->mov, entk->ctrl->addr);
				if(HZX_Zone1(back_addr) != HZX_NO_ZONE) {
					goback = 1 ;
				}
			}else {
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
				}else {
					if(ENE_ZoneIntrptCheck( back_addr ) ){ 
						goback = 0 ;
					}
				}
			}
			/*逃げ予定地が後方と重ならないか？*/
			back_entk = GetAtRanking(entk,1) ;
			if(back_entk != NULL ){
				back_at_thk = (AT_THK *) back_entk->character ;
				if(
				(HZX_Zone1(back_addr) == HZX_Zone1(back_entk->ctrl->addr) )
				&&(HZX_Zone1(side_addr) == HZX_Zone1(back_entk->ctrl->addr) )
				){
					/*プレイヤと後方兵両方にぶつからない
					ゾーンを調べる*/
					back_addr = HZX_FarZoneNavigate2( entk->ctrl->hzx_id,
						HZX_Zone1(entk->ctrl->addr),HZX_Zone1(GM_PlayerAddress),
						HZX_Zone1(back_entk->ctrl->addr), &rout ) ;
					if((HZX_Zone1(back_addr) != HZX_Zone1(entk->ctrl->addr))){
						GoEscape(entk);
						at_thk->escaddr = back_addr ;
						return ;
					}
				}else {
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
	}
	/*殴りチェック*/
	if(CheckBeat(entk)){
		return ;
	} 


/*回避*/
/*2000.09.05*/
	if(AT_CheckReload(entk)){
		/*ATACK NEAR*/
		/*その場でリロード*/
		/*ハイテク兵から見えるとこにいる*/
		if((entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )&&(entk->act->bodyp.type & ENE_TYPE_HITECH)){
			GoM4GrdShoot( entk );
			return ;
		}else {
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
	if(entk->act->bodyp.type & ENE_TYPE_SHIELD)
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

	
	if((at_thk->at_tmptime>DEF_TIME) 
	&&(entk->at_com->berserk==0)
	){
		shl_dif =1;
	}else {
		shl_dif = 0;
	}

	if(at_thk->at_status & AT_ST_SQUAT){
		/*しゃがみ状態*/
		if(shl_dif ==1 ) {
			entk->act->pad = SP_SQUAT_DEFENCE;
		}else entk->act->pad = SP_SQUATGUN;
	}else {
		if(shl_dif ==1) {
			entk->act->pad = SP_STAND_DEFENCE;
		}else {
			entk->act->pad = SP_READYGUN;
		}
	}


	/*盾防御で膠着防止*/
	if(
	(shl_dif ==1)
	&&(entk->count3 >= DIRECT_TICK(180))
	){
		SetBerserkTime(entk,DIRECT_TICK(120)) ;
	}

	/*味方死にチェック*/
	if(CheckDeathBed(entk) ){
		return ;
	}
	/*後ろでうごきまわりチェック*/
	if(CheckPrudence(entk)) return ;

	/*プレイヤ方向を向かせる*/
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		entk->act->dir = entk->pl_eyei.dir;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
		entk->act->dir = entk->npc_eyei.dir;
	}else {
		entk->act->dir = entk->trgpoint.dir ;
	}

#if 1
	if( entk->act->act_end ){
		entk->act->pad = SP_READYGUN;
		entk->count3 = 0 ;
		return ;
	}
#endif
	entk->count3 ++ ;
	/*射撃 リロード分岐無し*/
	if(
	(entk->at_com->watch_status & AT_COM_WATCH_LOCKER)
	&&(at_thk->dis_rank > 0)
	){
		return ;
	}

	/*攻撃兵がプレイヤに攻撃しましたフラグ*/
	if(at_thk->in_sight>DIRECT_TICK(60)){
		COM_SetAlertStatus( COM_ALERT_ATK_ATTACK ) ;
	}
	/*ショットガンは連射不可*/
	if(
	(entk->act->bodyp.type & ENE_TYPE_SHOTGUN)
	){
		if(entk->act->keep_pad == SP_SHOOTGUN) {
			return ;
		}
	}

	if(
	(shl_dif != 1) &&(entk->act->pad != SP_WAIT)
	){
		StillShoot( entk  ,AT_SHT_NO_RELOAD);
	}

	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		HZX_ZON		*zone; /*ゾーン*/
		zone = HZX_GetZone(GM_PlayerHzxID,HZX_Zone1(GM_PlayerAddress) ) ;
		if(
		(entk->count3 > DIRECT_TICK(90))
		&&(at_thk->dis_rank == 0)
//		&&(zone->flag & HZX_ZONE_NO_AVOID)
		&&(entk->at_com->watch_status & AT_COM_WATCH_NO_AVOID)
		){
			GoGrdHigh(entk);
			return ;
		}
	}

}


#define TURN_TIME			(DIRECT_TICK(120))
#define TOTAL_TURN_TIME		(DIRECT_TICK(480))

static void SetLookAroundDir(ENETHINK *entk)
{
	int count ;
	count = entk->count3%TOTAL_TURN_TIME ;
	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;

	if ( count  < TURN_TIME ) {
		entk->act->aim_dir = entk->ctrl->turn.vy + 256 ;
	} else if ( count < (TURN_TIME*2) ) {
		entk->act->aim_dir = entk->ctrl->turn.vy - 256 ;
	}
}

/*対NPC　先頭キャラ*/
static void Think3_WaitNearNPC( entk )
ENETHINK	*entk ;
{
/*beyond*/
	int fl_atr;
//	SVECTOR rgb;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

//	SetAimPosNPC_Eyei(entk) ;
	ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
#if 0
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
	}else {
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	}
#endif

	entk->act->dir = entk->ctrl->turn.vy ;
	at_thk->at_status &= ~(AT_ST_SQUAT);

	/*フロア属性チェック*/
	fl_atr = CheckFlrAtr(entk);
#if 1
	/*割り込みアクション終了時にthinkをリスタート*/
	if ( entk->act->act_end ) {
		entk->count3 = 0 ;
		return ;
	}
#endif

	/* 目標が見えなくてもチーム内の誰かが捕捉中なら */
	/* あわてて追跡しない */
	if(
	(entk->npc_eyei.sight != EYE_INFO_SIGHT_IN )
	){
		if( at_thk->zone_dis > entk->at_com->chasedis+1000 ) {
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_ZONE_CHASE ;
			entk->count3 = 0 ;
			return ;
		}
	}
	if((at_thk->dis_rank==0)&&((entk->count3%DIRECT_TICK(300))> DIRECT_TICK(180))){
		entk->act->pad = SP_READYGUN;
		entk->act->dir = entk->npc_eyei.dir ;
		if(
		(entk->act->bodyp.type & ENE_TYPE_SHOTGUN)
		&&(entk->act->keep_pad == SP_SHOOTGUN) 
		){
		}else {
//printf("StillShoot\n");
			SetAimPosNPC_Eyei(entk) ;
			entk->act->aim_dir = entk->act->dir ;
			StillShoot( entk  , AT_SHT_BLIND);
		}
	}else {
		entk->act->pad = SP_AIM_POS ;
		/*キョロキョロ*/
		SetLookAroundDir(entk) ;
		entk->act->dir = entk->npc_eyei.dir ;
	}
//printf("Think3_WaitNearNPC rank [%d]count[%d]\n",at_thk->dis_rank,entk->count3);
	/*味方死にチェック*/
	if(CheckDeathBed(entk) ){
		return ;
	}
	entk->count3 ++ ;
}



/*対NPC　後方キャラ*/
static void Think3_ChaseWaitNPC( entk )
ENETHINK	*entk ;
{
//	FVECTOR npos,ppos;
	int zone_ch;
//	int rank_dis = 1000 ;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;
//	entk->act->pad = SP_READYGUN;
	entk->act->pad = SP_AIM_POS ;
//	SetAimPosNPC_Eyei(entk) ;

	zone_ch =
		ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint),
		entk->ctrl, entk->count3 ) ;

	if(zone_ch ==1){
		if(CheckOnlyRIntrpt(entk)){
			zone_ch = -2 ;
		}else {
		}
	}

	/*攻撃モードに移行*/
	if(
	( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )
	){
		/*強制モーション中は思考進行しない*/
		if(!(GM_PlayerStatus & PLAYER_FORCE)){
			GoNearAttack(entk);
			return ;
		}
	}
	if(ThinkSubNear( entk )) {
		return;
	}
	if(CheckDeathBed(entk) ){
		return ;
	}

	if((at_thk->dis_rank==0)&&((entk->count3%DIRECT_TICK(300))> DIRECT_TICK(180))){
		entk->act->pad = SP_READYGUN;
		entk->act->dir = entk->npc_eyei.dir ;
		if(
		(entk->act->bodyp.type & ENE_TYPE_SHOTGUN)
		&&(entk->act->keep_pad == SP_SHOOTGUN) 
		){
		}else {
//printf("StillShoot\n");
			SetAimPosNPC_Eyei(entk) ;
			entk->act->aim_dir = entk->act->dir ;
			if((entk->count3%DIRECT_TICK(300))> DIRECT_TICK(210)){
				StillShoot( entk  , AT_SHT_BLIND);
			}
		}
	}else {
		entk->act->pad = SP_AIM_POS ;
		/*キョロキョロ*/
		SetLookAroundDir(entk) ;
		entk->act->dir = entk->npc_eyei.dir ;
	}



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
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

	if( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR )
		SetAimPosPlayer(entk,0);
	/*終了時間チェック*/
	if((entk->count3 > at_thk->at_tmptime)
//	||(entk->at_com->com_sight == 0 )
	){
		GoNearAttack(entk);
		entk->act->pad = SP_READYGUN;
//		entk->act->dir = entk->ctrl->turn.vy ;
		return;
	}
	/*殴りチェック*/
#if 1
	if(CheckBeat(entk)){
		return ;
	} 
#else
	if((entk->pl_eyei.sight == EYE_INFO_SIGHT_IN)
		&&(entk->count3>(AT_THK_RATE*2))
		&&( entk->pl_eyei.dis <= BEAT_DIS )) {
		entk->think2 = TH2_ATTACK;
		entk->think3 = TH3_ATTACK_BEAT ; 
		entk->count3 = 0 ;
		return;
	}
#endif

	/*基本攻撃姿勢*/
	at_thk->at_status &= ~AT_ST_SQUAT;

	entk->act->pad = SP_READYGUN;

	if(AT_CheckReload(entk)){
		GoReload(entk);
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
	if(CheckDeathBed(entk) ){
		return ;
	}
	/*2000.12.13 視界チェック*/
	if(entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ){
		RandShoot( entk );
	}
}
/*呼び出し前に tmptimeに終了時間を設定すること */
static void Think3_AttackLie( entk )
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;
	entk->act->dir = entk->ctrl->turn.vy ;
#if 0
	if( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR )
		SetAimPosPlayer(entk,0);
#endif
	/*終了時間チェック*/
	if((entk->count3 > at_thk->at_tmptime)
	){
		GoDeploy(entk ) ;
		return;
	}
	/*殴りチェック*/
	if(CheckBeat(entk)){
		return ;
	} 
	/*基本攻撃姿勢*/
	at_thk->at_status &= ~AT_ST_SQUAT;
	entk->act->pad = SP_INTRUDE_FIRE;

	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
//	if(CheckDeathBed(entk) ){
//		return ;
//	}
	/*2000.12.13 視界チェック*/
	if(entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ){
		RandShoot( entk );
	}
}
/*伏せ姿勢でニキータを見つめる*/
static void Think3_DodgeNikita( entk )
ENETHINK	*entk ;
{
	FVECTOR sub;
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;
	entk->act->dir = entk->ctrl->turn.vy ;
	/*基本攻撃姿勢*/
	at_thk->surprised = DIRECT_TICK(10) ;
	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
	entk->act->aim_pos = GM_NikitaPosition[ 0 ];
	at_thk->at_status &= ~AT_ST_SQUAT;
	entk->act->pad = SP_INTRUDE_FIRE;
	if((!SIG_NktCheck(entk))&&(entk->count3 > DIRECT_TICK(90))){
		GoNearAttack(entk);
		return  ;
	}
/*体の向き　ニキータ方向*/
#if 1
	_sceVu0SubVector( &sub, &GM_NikitaPosition[ 0 ], &entk->ctrl->mov ) ;
	entk->act->dir = GV_VecDir2( &sub );
#else
	entk->act->dir = entk->pl_eyei.dir ;
#endif
	entk->count3 ++ ;
}

static void Think3_AttackLocker( entk )
ENETHINK	*entk ;
{
	FVECTOR		pos;
	int			dir ;
//	HZX_ZON		*zone ;
	R_INTRPT	*r_intrpt ;

	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

//ASSERT(at_thk->dis_rank == 0)  ;

	entk->act->dir = entk->ctrl->turn.vy ;

	if(entk->act->bodyp.type & ENE_TYPE_SHIELD){
		/*ロッカー蹴り壊す*/
		if(entk->count3==0){
			at_thk->at_status &= ~AT_ST_SQUAT;
			r_intrpt = GetLockerInfo(&pos,&dir);
			if(r_intrpt != NULL ){
				entk->act->pad = SP_KICK_LOCKER ;
				entk->act->tmp_dir = dir ;
			}else {
				GoNearAttack(entk);
				entk->act->pad = SP_READYGUN;
				return ;
			}
		}
//		entk->act->pad = SP_ATTACK_KICK ;
	}else {
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
//		SetAimPosPlayer(entk,0);
	if(
	(!(entk->at_com->watch_status & AT_COM_WATCH_LOCKER))
	||(GM_PlayerStatus & PLAYER_FORCE)
	||(entk->act->act_end)
	){
		/*その後の行動*/
#if 1
		/*ロッカー攻撃続行*/
		if((entk->at_com->watch_status & AT_COM_WATCH_LOCKER)
		&&(!(GM_PlayerStatus & PLAYER_FORCE))
		&&(at_thk->dis_rank == 0 )
		){
			if(GoAttackLocker(entk) ){
				return ;
			}
		}
		GoNearAttack(entk);
#else
		/*いったん下がる*/
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

			
/*指定ポイントで防衛*/
/*相当離れるまで攻撃 */
static void Think3_Defence( entk )
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	SVECTOR	rgb;
	at_thk = (AT_THK *)entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

	rgb.vx = 0;
	rgb.vy = 255;
	rgb.vz = 0;

	if( (GM_GameStatus & STATE_VR_ANOTHER)&&(at_thk->at_status & AT_ST_DEFENSE)  ){
		at_thk->at_status &= ~AT_ST_DEFENSE ;
	}

	/*基本攻撃姿勢*/
//	if(at_thk->dis_rank <=1 ){
	if(0){
		/*しゃがみ*/
		at_thk->at_status |= AT_ST_SQUAT;
		entk->act->pad = SP_SQUATGUN;
	}else{
		/*立ち*/
		at_thk->at_status &= ~AT_ST_SQUAT;
		entk->act->pad = SP_READYGUN;
	}

/**/
	if(CheckDefPosEne(entk)) {
		/*道を空ける*/
		GoPosChase(entk ,&at_thk->def_pos2);
		at_thk->th2_buf = TH2_ATTACK ;
		at_thk->th3_buf = TH3_DEFENCE2 ;
		return  ;
	}
	if( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR ){
		SetAimPosPlayer(entk,0);
	}
#if 0
	/*終了時間チェック*/
	if((entk->count3 > at_thk->at_tmptime)
	||(entk->at_com->com_sight == 0 )){
		GoNearAttack(entk);
//		entk->act->dir = entk->ctrl->turn.vy ;
		return;
	}
#else
//	if(at_thk->zone_dis > AT_DEF_START_DIS_PL){
	if(at_thk->zone_dis > (entk->sense.eye_s - 1000)){
printf("DEF MODE 2 GoNearAttack\n");
		GoNearAttack(entk);
//		entk->act->dir = entk->ctrl->turn.vy ;
		return;
	}
#endif
	/*殴りチェック*/
#if 1
	if(CheckBeat(entk)){
		return ;
	} 
#else
	if((entk->pl_eyei.sight == EYE_INFO_SIGHT_IN)
	&&(entk->count3>(AT_THK_RATE*2))
	&&( entk->pl_eyei.dis <= BEAT_DIS )) {
		entk->think2 = TH2_ATTACK;
		entk->think3 = TH3_ATTACK_BEAT ; 
		entk->count3 = 0 ;
		return;
	}
#endif


	if(AT_CheckReload(entk)){
		GoReload(entk);
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
	if(CheckDeathBed(entk) ){
		return ;
	}
	RandShoot( entk );
}

static void Think3_Defence2( entk )
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
//	SVECTOR	rgb;
	at_thk = (AT_THK *)entk->character ;

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
	if(CheckDefPosEne(entk)) {
		/*まだふさがってる*/
		/*道を空ける*/
	}else {
		/*戻れる*/
		GoPosChase(entk ,&entk->def_pos);
		at_thk->th2_buf = TH2_ATTACK ;
		at_thk->th3_buf = TH3_DEFENCE ;
		return  ;
	}
	if( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR ){
		SetAimPosPlayer(entk,0);
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
#else
	if((entk->pl_eyei.sight == EYE_INFO_SIGHT_IN)
	&&(entk->count3>(AT_THK_RATE*2))
	&&( entk->pl_eyei.dis <= BEAT_DIS )) {
		entk->think2 = TH2_ATTACK;
		entk->think3 = TH3_ATTACK_BEAT ; 
		entk->count3 = 0 ;
		return;
	}
#endif


	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
	if(CheckDeathBed(entk) ){
		return ;
	}
}


static void Think3_AttackSiege( entk )
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

//	if(at_thk->at_status & AT_ST_NEXT) 

	entk->act->dir = entk->ctrl->turn.vy ;
	at_thk->at_status &= (~AT_ST_SQUAT);
	if(CheckFlrAtr(entk)){
		GoNearAttack(entk);
		return ;
	}

	SetAimPosPlayer(entk,0);
	/*割り込みアクション終了時にthinkをリスタート*/
	if ( entk->act->act_end ) {
		entk->count3 = 0 ;
//		entk->act->dir = entk->ctrl->turn.vy ;
		return ;
	}
	/*回避*/
	if(
	(ENE_AlertGameLevel >= AT_ESC_LEVEL)&&(entk->count3>(AT_THK_RATE*5))
	&&(at_thk->sight_time > entk->at_com->esctime ))
	{
		GoDanger( entk ) ;
			return;
	}
/*START*/
	/*回り込み中 判定甘くする*/
	if( at_thk->zone_dis > (entk->at_com->chasedis + 2000) )
	{
		if(ThinkSubNear( entk ) ){
			return;
		}
	}

/**移動＆終了処理**/
#if 0
	if(entk->at_com->siege[entk->u_id]){
	/*安地チェックは10F毎*/
		entk->act->pad = CheckForm( entk,(!(entk->count3 % (AT_THK_RATE*2))) );
		if(entk->act->pad == SP_READYGUN){
			GoEasyAttack(entk,(AT_THK_RATE*10));
//		entk->act->dir = entk->ctrl->turn.vy ;
			return;
		}
	}else {
		entk->act->pad = SP_READYGUN;
	}
#endif

/* ばたつき防止 アクション変化タイミングに遊び*/
	if ( entk->count3 == 0 ) {
		at_thk->at_tmptime =0;
	}else {
		if(entk->act->pad != entk->act->keep_pad){
			if(at_thk->at_tmptime > (AT_THK_RATE*2)){
				at_thk->at_tmptime = 0;
			}else{
				at_thk->at_tmptime++;
				entk->act->pad = entk->act->keep_pad;
			}
		}else {
			at_thk->at_tmptime = 0;
		}
	}

	if(entk->count3<(AT_THK_RATE*2)) {
		/* 静止期間*/
		entk->act->pad = SP_READYGUN;
	}else if(entk->count3>(AT_THK_RATE*5)){
		/*停止許可*/
		if(entk->act->pad == SP_READYGUN ) {
			if(entk->think2 == TH2_ATTACK){
				GoNearAttack(entk);
			}else {
				entk->think3 = TH3_WAIT_CHASE ;
			}
			entk->count3 = 0 ;
//		entk->act->dir = entk->ctrl->turn.vy ;
			return ;
		}
	}

	/*プレイヤ方向を向かせる*/
	/*射撃 リロード分岐付き*/
	RandShoot(entk);
	entk->act->dir = entk->pl_eyei.dir ;


	if(
	(at_thk->at_status & AT_ST_NEXT) 
	&&
	((entk->act->move_s == MoveSideR )
	||(entk->act->move_s == MoveSideL))
	){
		GoAttackPeek(entk,entk->act->move_s);
		return;
	}

	if(CheckDeathBed(entk) ){
		return ;
	}

	entk->count3 ++ ;

}

static void Think3_Beyond( entk )
ENETHINK	*entk ;
{
	FVECTOR aimpos,sub,e_pos;
	CONTROL	*ctrl ;
	float trglen,edgelen;
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&GM_PlayerFindPos ,250.0F ,&rgb );
	}
#endif
	/*基本パラメータ初期化*/
	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	entk->act->move_s = MoveCautionWalk;
	entk->act->pad = SP_MOVE_RUN ;
	if(entk->count3==0) entk->tmp_time = 0;

	ctrl = entk->ctrl ;
	/*別モードへの切り替えチェック*/
	/*2000.09.04 ビヨンドからの戻り中は後退*/
	if((GM_PlayerStatus & PLAYER_FORCE)&&(entk->pl_eyei.dis < AT_DIS_MIN)){
		aimpos = GM_PlayerFindPos ;
		entk->act->pad = SP_BACKWALK ;
		entk->count3 ++ ;
		/*SetMoveModeを使わない場合自分で補正方向セット*/
		entk->act->dir = GV_VecDir2FromTo( &entk->ctrl->mov,&aimpos ) ;
		entk->act->tmp_dir = entk->act->dir ;
		return ;
	}else {
		/*aimposに攻撃目標セット*/
		aimpos = GM_PlayerPosition ;
		aimpos.vy += 1200.0F ;	/* 手の高さ */
	}
	/*以下　接近および踏み*/
	if(aimpos.vy > (entk->ctrl->mov.vy+500.0F)){
		aimpos.vy = entk->ctrl->mov.vy+500.0F ;
	}
	entk->act->aim_pos = aimpos;
	/*威嚇射撃*/
	entk->bullet = 0 ;
	ThreatShot( entk ,&aimpos) ;
#if 1
/*old*/
	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
#else
	entk->status2 = ENE_STATUS2_AIM_GUN ;
#endif
	/*足の位置*/
	e_pos = entk->znavi->flore_pos ;
	/*最終目標までの距離*/
	trglen = GV_VecLen3F2( &aimpos, &e_pos ) ;
#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&aimpos ,250.0F ,&rgb );
	}
	{
		SVECTOR	rgb;
		rgb.vx = 0;	rgb.vy =   0;	rgb.vz =   255;
		PosBox(&(entk->trgpoint.pos),250.0F ,&rgb );
	}
#endif
	/*通常攻撃に復帰*/
	if((!(entk->at_com->watch_status & AT_COM_WATCH_BEYOND))||(!(GM_PlayerPosition.vy < (entk->ctrl->mov.vy-BEYOND_DIS)))
	){
		GoNearAttack(entk);
		return;
	}
	if( HZX_Zone1(entk->ctrl->addr)!= HZX_Zone1(entk->com->plzone_in_zone[0]) ) /*最後に入ったゾーン*/
//		!= HZX_Zone1(GM_PlayerAddress) )
	{
		/*再ゾーン追跡*/
		entk->think2 = TH2_CHASE;
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0;
		return ;
	} else if(entk->tmp_time > 0 ){
		/*ちょっと止まって様子見*/
		entk->act->pad = SP_READYGUN ;
	}else {
		if( trglen > STOMP_DIS ){
			/*位置ぎめ*/
			/*ゾーン縁までの距離*/
//			GetNearPosInZone( ctrl->hzx_id,entk->com->plzone_in_zone[0],
			GetNearPosInZone( ctrl->hzx_id,ctrl->addr,&aimpos,&sub ) ;
			edgelen = GV_VecLen3F2( &sub, &e_pos ) ;
			/*ここでsubにはゾーン内での目標が入る目論見*/
#if 0
			{
				SVECTOR	rgb;
				rgb.vx = 0;	rgb.vy =   255;	rgb.vz =   0;
				PosBox(&sub ,250.0F ,&rgb );
			}
#endif
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
					if(ENE_EyeOnlineCheck( entk->ctrl->hzx_id,&entk->ctrl->mov, &aimpos )){
						entk->act->pad = SP_READYGUN ;
						entk->tmp_time = (AT_THK_RATE*10);
						entk->act->dir = GV_VecDir2FromTo( &entk->ctrl->mov,&sub ) ;
						entk->count3 ++ ;
						return ;
					}
				}
			}
			/*踏むために前進*/
			printf("HUMUTAMENI SENSIN\n");
			entk->act->move_s = MoveCautionWalk;
			entk->act->pad = SP_MOVE_RUN ;
			entk->act->dir = GV_VecDir2FromTo( &entk->ctrl->mov,&aimpos ) ;
			/*SetMoveModeを使わない場合自分で補正方向セット*/
		} else {
			/*踏む*/
			/*踏めるかチェック*/
			if((entk->pl_eyei.sight != EYE_INFO_SIGHT_IN)&&(ENE_EyeOnlineCheck( entk->ctrl->hzx_id,&entk->ctrl->mov, &aimpos ))){
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
	if(entk->tmp_time > 0) entk->tmp_time-- ;
//	entk->act->dir = entk->pl_eyei.dir ;
	entk->act->tmp_dir = entk->act->dir ;
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
					if(AT_CheckReload(entk)){
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
						entk->at_com->rollout = AT_ROLL_COUNT ;
						entk->act->pad = SP_ROLL_R ;
					}else { 
						/*左転がり*/
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
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
	}else {
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
	}

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

	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
	}else {
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
	}

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
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
	}else {
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
	}

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


static void Think3_GoSign( entk )
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->dir = entk->ctrl->turn.vy ;

	if((entk->count3 > (AT_THK_RATE*5))&&( entk->at_com->com_sight == 0)){
		entk->act->act_end = 1;
	}

	if(entk->count3 == 0){
/*おしゃべり実験 ＧＯサインに合わせる*/
#if 0
		if( at_thk->zone_dis > (entk->at_com->chasedis + 2000))
		{
			int voice ,rnd ;
			rnd = ((irnd()>>8)%2);
			switch (rnd){
				case 0:
					voice = EV_GO_1 ;
					break;
				case 1:
					voice = EV_CAMON_1 ;
					break;
				default :
					voice = EV_GO_1 ;
			}
			if(entk->com->enemys.group[entk->g_id]->
				unit[entk->u_id]->enemy_num>0){
				COM_SetSpeak( voice, entk ) ;
			}
		}
#endif
//		SIG_AT_VoiceCall(entk,SD_V_ATKO03,AT_V_NIGETAZO) ;
		entk->at_com->gosign = GOSIGN_TIME;
		entk->act->pad = SP_CLE_GOSIGN ;
	}

	if (entk->act->act_end) {
		GoNearAttack(entk);
		entk->count3 = 0 ;
//		entk->act->dir = entk->ctrl->turn.vy ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++;
}
static void Think3_Shl_Break(entk)
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	entk->act->dir = entk->ctrl->turn.vy ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	if(entk->count3 == 0){
		entk->act->pad = SP_SHL_BREAK ;
	}
	if ((entk->count3>(AT_THK_RATE*10))||(entk->act->act_end)) {
//		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		entk->act->sw->sub_weapon = 0;
		GoDanger( entk ) ;
//		entk->act->dir = entk->ctrl->turn.vy ;
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
#if 1
static void Think3_NSight_ON( entk )
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;
	entk->act->dir = entk->ctrl->turn.vy ;

	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_NSIGHT_ON ;
	}
	if ( entk->act->act_end ) {
		GoNearAttack(entk);
		return ;
	}
	entk->count3 ++ ;
}
#endif

/*----- 中レベル思考モード --------------------------------------------*/
	/*追跡*/
static	void	Think2_Chase( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ZONE_CHASE :			/* 目標ゾーンまで行く */
			Think3_ZoneChasePlayer( entk ) ;
		break ;
		case TH3_DIRECT_CHASE :			/* 同じゾーン内で追いかける */
			Think3_DirectChasePlayer( entk ) ;
		break ;
		case TH3_SAFEZONE_CHASE :			/* 安全ゾーンへ逃げる */
			Think3_ZoneChaseSafe( entk );
		break ;
		case TH3_SAFEDIRECT_CHASE :			/* 安全ゾーンへ逃げる */
			Think3_DirectChaseSafe( entk );
		break ;
		case TH3_WAIT_CHASE : /*手前の味方を見つつ待機*/

//			if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT&&(!(SIG_CheckStealthStatus(entk)))){
			if((COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)&&((entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ))){
				Think3_ChaseWait( entk );
			}else if((COM_AlertStatus()&COM_ALERT_NPC_DETECT)&&(entk->npc_eyei.sight == EYE_INFO_SIGHT_IN )){
				Think3_ChaseWaitNPC( entk ) ;
			}else {
				Think3_ChaseWait( entk );
			}

		break ;
		case TH3_ATTACK_SIEGE :
			Think3_AttackSiege( entk );
		break;
		case TH3_SIDE_MOVE :
			Think3_SideMove( entk );
		break;
#if 0
		case TH3_TURN_CHASE :
			Think3_ChaseTurn(entk);
		break;
#endif
		case TH3_GOSIGN :
			Think3_GoSign( entk );
		break;
		case TH3_ROLL_OUT :
			Think3_ChaseRoll(entk);
		break;
#if 0
		case TH3_SIDE_STEP :
			Think3_SideStep(entk);
		break;
#endif
		case TH3_MUKA_ZONE:
	    	Think3_ZoneChasePlayerM( entk ) ;
		break ;
		case TH3_MUKA_DIRECT:
	    	Think3_DirectChasePlayerM( entk ) ;
		break;
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
			if(CheckX_Fire(entk) == 0){
				/*散開終了*/
				GoChaseWait(entk);
			}
		break;
	}

}

	/*攻撃*/
static	void	Think2_Attack( entk )
ENETHINK	*entk ;
{

/*FACE TEST*/
#ifdef FACE_TEST
	if((entk->at_com->alert_time&HZX_NO_ZONE)<60) entk->animflag =3;
	else entk->animflag =7;
#endif
//TEST_Locate( 72 , 12, 0 );

	switch ( entk->think3 ) {
	    case TH3_ATTACK_NEAR :
			/*静止して攻撃*/
//			if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
//			if((COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)&&(!(SIG_CheckStealthStatus(entk)))){
			if((COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)&&((entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ))){
		    	Think3_AttackNear( entk ) ;
			}else if((COM_AlertStatus()&COM_ALERT_NPC_DETECT)&&(entk->npc_eyei.sight == EYE_INFO_SIGHT_IN )){
				Think3_WaitNearNPC( entk ) ;
			}else {
		    	Think3_AttackNear( entk ) ;
			}
		break;
		case TH3_ATTACK_BEAT :
			/*格闘*/
	    	Think3_AttackBeat( entk ) ;
		break ;
		case TH3_WAIT_SAFE :
			Think3_WaitSafezone( entk );
		break;
		case TH3_WAIT_LOWSAFE :
			Think3_WaitLowSafe( entk );
		break;
	    case TH3_ATTACK_RELOAD :
	    	Think3_AttackReload( entk ) ;
		break;
		case TH3_ATTACK_SIEGE :
			Think3_AttackSiege( entk );
		break;
		case TH3_DANGER :
			Think3_Danger( entk );
		break;
		case TH3_BEHIND:
			Think3_Behind(entk);
		break;
		case TH3_SHL_BREAK:
			Think3_Shl_Break(entk);
		break;
		case TH3_ATTACK_EASY:
			Think3_AttackStand(entk);
		break;
		case TH3_ATTACK_EASY_LIE :
			Think3_AttackLie(entk);
		break;
		case TH3_DODGE_NIKITA :
			Think3_DodgeNikita(entk);
		break ;
		case TH3_ATTACK_STOMP:
			Think3_AttackStomp(entk);
		break;
		case TH3_ATTACK_GRD_HIGH :
			Think3_GrdAttackHigh(entk);
		break;
		case TH3_NSIGHT_ON :
			Think3_NSight_ON(entk) ;
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
		case TH3_ATTACK_LOCKER :
			Think3_AttackLocker( entk ) ;
		break;
#if 0
		case TH3_GRD_M4_RELOAD :
			Think3_GrdM4Reload( entk ) ;
		break;
#endif
		case TH3_GRD_M4_SHOOT :
			Think3_GrdM4Shoot(entk) ;
		break;
	}
}
#include "discovery_at.c"


/*
威嚇モード中レベル思考 TH2_Threat();
*/
#include "threat.c"
/*接近警戒 中レベル思考TH2_Approach()*/
#include "approach.c"
/*イントルード 中レベル思考TH2_Intrude()*/
#include "intrude.c"

/*対称配置モード 中レベル思考TH2_Symmetry()*/
#include "symmetry.c"

/*眠り兵救出モード TH2_Rescue()*/
#include "rescue.c"

/*負傷モード Think2_PBreak()*/
#include "pbreak_at.c"


/*----- 高レベル思考モード --------------------------------------------*/

	/*
		危険
	*/
void	Think1_Alert( entk )
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *) entk->character ;

	entk->iknow_flag &= ~IKNOW_HIDDEN_PLAYER ;

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
	{
		extern void PosBox(FVECTOR *,float ,SVECTOR *) ;
		GM_CurrentMap = GM_PlayerMap ;
		PosBox(entk->pl_eyei.pos,150.0F ,NULL) ;
	}

#endif

	{
		/*コーナーの向こう察知*/
		/*視界外なら不可*/
		if(entk->act->bodyp.type & ENE_TYPE_EVENT_A){
			ENTK_TYPE_A *eve_a ;
			eve_a = ( ENTK_TYPE_A * ) entk->eve_a ;
			if(eve_a->type <=1){
				SIG_CheckCorner(entk);
			}
		}else {
			SIG_CheckCorner(entk);
		}
		/*包囲位置にいる？*/
		CheckInCover(entk);
		/*気配を感じている場合*/
		if(
		(( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN)&&(GM_PlayerStatus & PLAYER_IN_THE_WATER))
		){
			/*水中に逃げられた*/
		}else {

			int zone_flag = HZX_ZONE_ZINTRPT ;
//printf("entk->com->plzone_in_zone [%x]\n",entk->com->plzone_in_zone[0]);
//			ASSERT(HZX_Zone1(entk->com->plzone_in_zone[0]) != HZX_NO_ZONE)
//			&&(!(ENE_HZX_GetZone(at_thk->found_addr)->flag & HZX_ZONE_ZINTRPT)))
			if(HZX_Zone1(at_thk->found_addr) != HZX_NO_ZONE ){
				zone_flag = ENE_HZX_GetZone(at_thk->found_addr)->flag ;
			}
			/*威嚇中*/
			if(
			(!(SIG_CheckStealthStatus(entk)))
			&&(
			(at_thk->at_status & AT_ST_FEEL)
			||(at_thk->at_status & AT_ST_NEXT)
			||(at_thk->at_status & AT_ST_IN_COVER)
			||((at_thk->found_addr == entk->com->plzone_in_zone[0])&&(!(zone_flag & HZX_ZONE_ZINTRPT)))
			||(entk->at_com->watch_status & 
			(AT_COM_WATCH_BEYOND|AT_COM_WATCH_INTRUDE|AT_COM_WATCH_LOCKER|AT_COM_WATCH_NO_AVOID))
			)
			){

				/*maxやめて値維持*/
#if 0
				if(entk->alert < (ALERT_LEVEL_MAX/4)){
					entk->alert = (ALERT_LEVEL_MAX/4) ;
				}
#else
				/*maxやめて値維持*/
				if(entk->alert < (ALERT_LEVEL_MAX/4)){
					entk->alert += 16 ;
				}
#endif
				at_thk->found_addr = entk->com->plzone_in_zone[0] ;

			}else {
			}
		}
	}
	at_thk->at_status &= ~AT_ST_NO_SHOT ;

/*割り込み的処理はここで*/
	/*優先の低いものから*/
	/*囲みモード*/
	CheckSymmetry(entk ) ;
	/*人質取られビックリ*/
	AT_CheckHostage(entk) ;
	/*プレイヤがすごい武器を持っている*/
	AT_CheckDanger(entk) ;
	/*プレイヤを初発見*/
	AT_CheckPlayer(entk) ;
	/*仲間が殺された*/
//	if(entk->think2 == TH2_ATTACK) CheckDeathBed(entk) ;
//
	/*盾壊れチェック*/
//	CheckShield(entk) ;

	switch ( entk->think2 ) {
	    case TH2_CHASE :		/* 追跡 */
			Think2_Chase( entk ) ;
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
	    case TH2_RESCUE :	/* 対称展開 */
			Think2_Rescue( entk ) ;
		break ;
	    case TH2_PBREAK :	/* 負傷中 */
			Think2_PBreak( entk ) ;
		break ;
	}
	/*発見プロセス*/
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->alert = ALERT_LEVEL_MAX ;
		at_thk->found_addr = entk->com->plzone_in_zone[0] ;
	}

	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
			entk->alert = ALERT_LEVEL_MAX ;
		}
	}
	
	if(1){
		CheckSightTime(entk);
	}
	if(at_thk->surprised > 0) {
		at_thk->surprised-- ;
	}
	/*前フレームでの目標位置待避*/
//	( (AT_THK * ) entk->character )->old_trgpos = entk->trgpoint.pos;


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

//	SIG_CheckRIntrpt(entk) ;

}

void AlertModeStart( ENETHINK *entk)
{

	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;


printf("AlertModeStart\n") ;

#if 0
/*----- user/korekado/enemy/enemy.h line 1655 alertstatus --------------------------------------------------*/
#define COM_ALERT_PLAYER_DETECT		0x00000001	/* プレイヤーを見つけている */
#define COM_ALERT_NPC_DETECT		0x00000002	/* NPC(エマ)を見つけている */
#define COM_ALERT_ATK_ATTACK		0x00000004	/* 攻撃兵、プレイヤーに攻撃開始 */
#define COM_ALERT_ATK_NPC			0x00000008	/* NPC担当兵、NPCに攻撃開始 */
	COM_AlertStatus( ) ;
#endif

#if 1	//by kore
	if ( entk->act->status & ACT_STATUS_UNREAL ) {
		/* 待機所に控えていたら出現場所チェックしてワープ */
		ENE_RouteWarp( entk ) ;
	}
#endif

	ASSERT( entk->at_com != NULL ) ;
	if(entk->act->bodyp.type & ENE_TYPE_SHIELD){
		entk->act->CheckPad = AT_AlertPadShl ;
	} else {
		entk->act->CheckPad = AT_AlertCheckPad ;
	}
	entk->act->keep_pad = -1 ;
	entk->act->pad = 0 ;
	entk->act->dir = -1 ;

//	entk->think_mode = ENE_TH1_ALERT ; 
	entk->think1 = ENE_TH1_ALERT ; 
	GoNearAttack(entk);


	if(entk->at_com->level==0) {
		entk->think2 = TH2_CHASE ; 
		entk->think3 = TH3_MUKA_ZONE ; 
	}else {
	}
	//暗視ゴーグル
	if(
	(entk->act->sw->n_sight == 1)
	&&(entk->think3 != TH3_MUKA_ZONE )
	){
		/* 暗視ゴーグルON */
		if(entk->act->bodyp.type & ENE_TYPE_EVENT_A){
			ENTK_TYPE_A *eve_a ;
			/*長廊下兵*/
			eve_a = ( ENTK_TYPE_A * ) entk->eve_a ;
			if(eve_a->type <=1){
				/*左舷*/
				entk->think3 = TH3_NSIGHT_ON ;
			}else {
				/*右舷*/
				entk->act->sw->n_sight = 2 ;
			}
		}else {
			entk->think3 = TH3_NSIGHT_ON ;
		}
	}

	entk->count3 = 0 ;

	entk->act->move_s = MoveAttackRun ;

	at_thk->safetype = 0;
	at_thk->sight_time =0; 
	at_thk->at_status &= (~AT_ST_SQUAT);
	at_thk->in_sight = 0;
	at_thk->safeaddr = HZX_NO_ZONE ;
	at_thk->found_addr = HZX_NO_ZONE ;
	at_thk->surprised = 0 ;

#if 1
/*NPC対応*/
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
printf("SetTrgpPlayer\n");
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
printf("SetTrgpNPC\n");
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
	}else {
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
printf("SetTrgpPlayerLastpos\n");
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), entk->com ) ;
	}
#else
	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
#endif
	ResetUseZone(entk);
	entk->sw_gun &= ~SW_FLAG_SWITCH1 ;

	if ( entk->act->bodyp.pbreak ) {
		GoPbreak(entk) ;
		printf("GoPbreak!!!!!!!\n");
	}

}

void AlertModeStartDamage( ENETHINK *entk)
{

	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	at_thk->in_sight = (AT_THK_RATE*20);
	at_thk->sight_time =(AT_THK_RATE*20); 
	at_thk->surprised = 0 ;

#if 1
	if ( entk->act->bodyp.pbreak ) {
		GoPbreak(entk) ;
		printf("GoPbreak!!!!!!!\n");
	}else {
//		if(entk->act->bodyp.type & ENE_TYPE_HITECH){
		if(0){
			/*ハイテク兵は逃げない*/
			GoChaseWait(entk) ;
		}else {
			/*ダメージ後は隠れようとする*/
			GoEscape(entk) ;
		}
	}
#else
//	if(entk->act->bodyp.type & ENE_TYPE_HITECH){
	if(0){
		/*ハイテク兵は逃げない*/
		GoChaseWait(entk) ;
	}else {
		/*ダメージ後は隠れようとする*/
		GoEscape(entk) ;
	}
#endif
}


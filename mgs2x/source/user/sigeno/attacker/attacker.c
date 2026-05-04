//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	attcker.c
	攻撃兵 本体

	1999/07/06 K.Sigeno
	$Id: attacker.c,v 1.1.1.3 2002/11/19 11:49:01 Yoshizawa1 Exp $
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

#include	"korekado/enemy/enemy.h"
#include	"attacker.h"


static void CheckATDmg(ENETHINK *) ;

/*user/korekadoからのextern*/
#include	"korekado/conv/korekado.x"
#include	"fromkore.x"

/*user/sigenoからのextern*/
#include	"atthink.x"
#include	"atress.x"
#include	"sigavoid.x"

extern void *NewSIG_BlurPoint(FMATRIX *world,FVECTOR *shift,CVECTOR *col ,int *sw);

extern void AT_AnesThink(ENETHINK * ) ;
extern int CheckAnesAT(ENETHINK * ) ;





#include	"atprepro.c"

//extern void Check_AttackerStartModeAvoid( ENETHINK *);
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern	void	*NewInterPoly_Gr( FVECTOR *, float , CVECTOR );



extern void	*NewAT_Shield( OBJECT* ,ENETHINK* ,OBJECT**);

extern void	ENE_AttackerStartModeSearch(ENETHINK*);
extern void	ENE_WatcherStartModeAlert(ENETHINK*);
extern void	ENE_WatcherStartModeAvoid(ENETHINK*);
extern void	ENE_AttackerStartModeSearchToSneak(ENETHINK*);
extern void	ENE_AttackerStartModeAvoidToSearch(ENETHINK*);
extern void	ENE_AttackerStartModeSneakDamage(ENETHINK*);
extern void	ENE_AttackerStartModeSearchDamage( ENETHINK*);
extern void ENE_SupportStageStartModeToSearch(ENETHINK *) ;
extern void ENE_AttackerStageStartModeToSneak(ENETHINK *) ;
extern void ENE_AttackerStageStartModeSearchWarp(ENETHINK *) ;


extern void	AlertModeStartDamage(ENETHINK*);
extern void	Check_AttackerStartModeAvoidDamage(ENETHINK*);
extern short	ENE_GetAmmoMax(int ) ;
extern void	ENE_Watcher_Think1_Sneak(ENETHINK*);
extern void	ENE_Attacker_Think1_Search(ENETHINK*);
extern CONTROL	*EMA_CommandGetControl(void) ;
extern void	ENE_SupportStartModeAvoidToSearch( ENETHINK* ) ;
extern void	SIG_CheckShieldBroken(ENETHINK* ) ;


/*画面外チェック 他でも使いそうなら移動する*/
/*pos が底面中心 size は 各辺の長さ*/
static int AT_BoundCheck(FVECTOR *pos,FVECTOR *size){
	FMATRIX m;
	FVECTOR	b_min,b_max ;
	
	m = DG_UnitMatrix ;
	m.m[3][0] = pos->vx ;
	m.m[3][1] = pos->vy ;
	m.m[3][2] = pos->vz ;

	b_min.vx = - (size->vx/2.0F) ;
	b_min.vy = 0.0F ;
	b_min.vz = - (size->vz/2.0F) ;

	b_max.vx = (size->vx/2.0F) ;
	b_max.vy = size->vy ;
	b_max.vz = (size->vz/2.0F) ;
	return DG_BoundCheck( &m, &b_max, &b_min )  ;
}
static void AT_AlertWarp(ENETHINK *entk){
	FVECTOR size = { 2000.0F,2000.0F,2000.0F,0.0F};
	AT_THK *at_thk;

	at_thk = (AT_THK *) entk->character ;

printf("AT_BoundCheck res %d\n",AT_BoundCheck(&entk->znavi->flore_pos,&size));
//printf("AT_BoundCheck flore_pos X[%f]Y[%f]Z[%f]\n",
//entk->znavi->flore_pos.vx,entk->znavi->flore_pos.vy,entk->znavi->flore_pos.vz);
/*起動直後はflore_pos初期値なのでワープしないことがある*/
/*本編に影響しそうなので注意*/
#if 1
	if(GM_GameStatus & STATE_VR_ANOTHER){
		if(entk->znavi->flore_pos.vx == 0.0f){
			entk->znavi->flore_pos = entk->ctrl->mov ;
		}
	}
#endif

	if(AT_BoundCheck(&entk->znavi->flore_pos,&size)<0 ) {
		if(entk->at_com->scn_status & AT_COM_SCN_STATUS_WARP){
printf("AT_AlertWarp");
			at_thk->blur_sw = -60;
			ENE_WarpNearPos( entk->ctrl, entk->trgpoint.addr, AT_WARP_DIS ) ;
			if(entk->at_com->level ==0)
				entk->at_com->level = 1;
		}
	}
}
static	void	SneakModeCheck( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	switch( GM_AlertMode ) {
		case ALERT_MODE_ALERT :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
ASSERT(entk->at_com != NULL) ;
			if(entk->at_com->wait_cnt == 0)
			{
				AlertModeStart(entk);
				AT_AlertWarp(entk);
				at_thk->blur_sw = -60;
			}
		break ;
		case ALERT_MODE_AVOID :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			if(entk->at_com->avoid_wait_cnt == 0){
				Check_AttackerStartModeAvoid( entk ) ;
			}
		break ;
		case ALERT_MODE_SEARCH :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			ENE_AttackerStartModeSearch( entk ) ;
		break ;
	}
	if ( ENE_DamageCheck( entk ) ) {
		if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
		ENE_EnemyStartModeDamage( entk ) ;
	}
}

/*----- 高レベルモード移行チェック --------------------------------------------------*/
static	void	ConvertSneakModeCheack( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;
	switch( GM_AlertMode ) {
		case ALERT_MODE_ALERT :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
ASSERT(entk->at_com != NULL) ;
			if(entk->at_com->wait_cnt == 0)
			{
				AlertModeStart(entk);
				AT_AlertWarp(entk);
				at_thk->blur_sw = -60;
			}
		break ;
		case ALERT_MODE_AVOID :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			if(entk->at_com->avoid_wait_cnt == 0){
				Check_AttackerStartModeAvoid( entk ) ;
			}
		break ;
		case ALERT_MODE_SEARCH :
#if 0		
			/* 回避モードを攻撃兵の思考 */
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			ENE_AttackerStartModeSearch( entk ) ;
		break ;
#else
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			if(entk->status & ENE_STATUS_CONVERT2){
				entk->sense.status = RADAR_COLOR_BLUE ;
				ENE_SupportStartModeCaution( entk ) ;
			}else {
				entk->think1 = ENE_TH1_SEARCH ; 
			}
		break ;
#endif
	}
	if ( ENE_DamageCheck( entk ) ) {
		if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
		ENE_EnemyStartModeDamage( entk ) ;
	}
}


static	void	SearchModeCheck( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			ENE_AttackerStartModeSearchToSneak( entk ) ;
		break ;
		case ALERT_MODE_ALERT :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			AlertModeStart(entk);
			AT_AlertWarp(entk);
				at_thk->blur_sw = -60;
		break ;
		case ALERT_MODE_AVOID :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			if(entk->at_com->avoid_wait_cnt == 0)
			{
				Check_AttackerStartModeAvoid( entk ) ;
			}
		break ;
	}

	if ( ENE_DamageCheck( entk ) ) {
		if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
		ENE_EnemyStartModeDamage( entk ) ;
	}
}




static	void	Convert2SearchModeCheck( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
//			ENE_WatcherStartModeSneak( entk ) ;
			ENE_AttackerStartModeSneak( entk ) ;
		break ;
		case ALERT_MODE_ALERT :
			AlertModeStart(entk);
				at_thk->blur_sw = -60;
			entk->bd_eyei.flag |= EYE_INFO_FLAG_SKIP ;
		break ;
		case ALERT_MODE_AVOID :
			if(entk->at_com->avoid_wait_cnt == 0)
			{
				Check_AttackerStartModeAvoid( entk ) ;
				entk->bd_eyei.flag |= EYE_INFO_FLAG_SKIP ;
			}
		break ;
	}
	if ( ENE_DamageCheck( entk ) ) {
		ENE_EnemyStartModeDamage( entk ) ;
		entk->bd_eyei.flag |= EYE_INFO_FLAG_SKIP ;
	}
}
static	void	ConvertSearchModeCheck( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
			/*コンバート兵 巡回復帰*/
			ENE_WatcherStartModeSneak( entk ) ;
		break ;
		case ALERT_MODE_ALERT :
			AlertModeStart(entk);
			at_thk->blur_sw = -60;
			entk->bd_eyei.flag |= EYE_INFO_FLAG_SKIP ;
		break ;
		case ALERT_MODE_AVOID :
			if(entk->at_com->avoid_wait_cnt == 0)
			{
				Check_AttackerStartModeAvoid( entk ) ;
				entk->bd_eyei.flag |= EYE_INFO_FLAG_SKIP ;
			}
		break ;
	}
	if ( ENE_DamageCheck( entk ) ) {
		ENE_EnemyStartModeDamage( entk ) ;
		entk->bd_eyei.flag |= EYE_INFO_FLAG_SKIP ;
	}
}




static	void	AlertModeCheck( entk )
ENETHINK	*entk ;
{
	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
			if(entk->status & ENE_STATUS_CONVERT){
				ENE_WatcherStartModeSneak( entk ) ;
			}else if(entk->status & ENE_STATUS_CONVERT2){
//				ENE_WatcherStartModeSneak( entk ) ;
				ENE_AttackerStartModeSneak( entk ) ;
			}else {
				ENE_AttackerStartModeSneak( entk ) ;
			}
		break ;
		case ALERT_MODE_AVOID :
			if(entk->at_com->avoid_wait_cnt == 0)
			{
				Check_AttackerStartModeAvoid( entk ) ;
			}
		break ;
		case ALERT_MODE_SEARCH :
			if(entk->status & ENE_STATUS_CONVERT){
				ENE_WatcherStartModeSneak( entk ) ;
				entk->think1 = ENE_TH1_SEARCH ; 
			}else if(entk->status & ENE_STATUS_CONVERT2){
				entk->sense.status = RADAR_COLOR_BLUE ;
				ENE_SupportStartModeCaution( entk ) ;
			}else {
				ENE_AttackerStartModeSearch( entk ) ;
			}
		break ;
	}

	if ( ENE_DamageCheck( entk ) ) {
		ENE_EnemyStartModeDamage( entk ) ;

	}
	/*2000.11.09危険モード中に大きな音を聞いた*/
	if ( entk->notice & (ENE_NOTICE_NOISE_M|ENE_NOTICE_NOISE_L) ) {
		entk->alert = ALERT_LEVEL_MAX ;
	}

	/*2001.06.11 危険モードのアラートレベル制御*/
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
			entk->alert = ALERT_LEVEL_MAX ;
			COM_SetAlertStatus( COM_ALERT_ATK_NPC ) ;
			COM_SetAlertStatus(COM_ALERT_NPC_DETECT) ;
		}
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
//		if(!(SIG_CheckStealthStatus(entk))){
		if(1){
			COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
		}
	}
}



static	void	AvoidModeCheck( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			if(entk->status & ENE_STATUS_CONVERT){
				ENE_WatcherStartModeSneak( entk ) ;
			}else if(entk->status & ENE_STATUS_CONVERT2){
//				ENE_WatcherStartModeSneak( entk ) ;
				ENE_AttackerStartModeSneak( entk ) ;
			}else {
				ENE_AttackerStartModeSneak( entk ) ;
			}
		break ;
		case ALERT_MODE_ALERT :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			AlertModeStart(entk);
			at_thk->blur_sw = -60;
			AT_AlertWarp(entk);
		break ;
		case ALERT_MODE_SEARCH :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;

			if(entk->status & ENE_STATUS_CONVERT){
				ENE_WatcherStartModeSneak( entk ) ;
				entk->think1 = ENE_TH1_SEARCH ; 
			}else if(entk->status & ENE_STATUS_CONVERT2){
				ENE_SupportStartModeAvoidToSearch( entk ) ;
			}else {
				ENE_AttackerStartModeAvoidToSearch( entk ) ;
			}

		break ;
	}

	if ( ENE_DamageCheck( entk ) ) {
		if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
		ENE_EnemyStartModeDamage( entk ) ;
	}

}

static	void	DamageModeCheck( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

//printf("Iam [%d] status [%x]\n",entk->id,entk->status);

	/*ダメージ後の復帰処理*/
	/*死んでないので思考セット後 再始動*/
	if ( entk->notice & ENE_NOTICE_DAMAGE ) {
		entk->sw_gun &= ~SW_FLAG_SWITCH2 ;
//		SIG_CheckShieldBroken(entk) ;
		switch( GM_AlertMode ) {
			case ALERT_MODE_SNEAK :
				if(entk->status & ENE_STATUS_CONVERT){
					ENE_WatcherStartModeSneakDamage( entk ) ;
				}else if(entk->status & ENE_STATUS_CONVERT2){
//					ENE_WatcherStartModeSneakDamage( entk ) ;
					ENE_AttackerStartModeSneakDamage( entk ) ;
				}else {
					ENE_AttackerStartModeSneakDamage( entk ) ;
				}
			break ;
			case ALERT_MODE_SEARCH :
//printf("SEARCH CHECK \n");
				if(entk->status & ENE_STATUS_CONVERT){
					ENE_WatcherStartModeSneakDamage( entk ) ;
					entk->think1 = ENE_TH1_SEARCH ; 
				}else if(entk->status & ENE_STATUS_CONVERT2){
					ENE_SupportStartModeCaution( entk ) ;
					ENE_SupportStartModeCautionDamage( entk ) ;
				}else {
					ENE_AttackerStartModeSearchDamage( entk ) ;
				}
			break ;
			case ALERT_MODE_ALERT :
				at_thk->blur_sw = -60;
				AlertModeStart(entk);
				/*ダメージ復帰*/
				AlertModeStartDamage(entk);
			break ;
			case ALERT_MODE_AVOID :
				Check_AttackerStartModeAvoidDamage( entk ) ;
			break ;
		}
		if ( ENE_DamageCheck( entk ) ) {
			ENE_EnemyStartModeDamage( entk ) ;
		}
	}

	if ( entk->notice & ENE_NOTICE_RES ) {
		/*死んでた*/
		/*CheckPad設定へ*/
		entk->sw_gun &= ~SW_FLAG_SWITCH2 ;
		ENE_AttackerResurrectionMode( entk ) ;
	}

//printf("Iam [%d] th1 [%d] th2 [%d] th3 [%d] \n",entk->id,entk->think1,entk->think2,entk->think3);

}

/*再発生時に装備品を再設定*/
static void EquipCheck(Work *work){
	ENETHINK *entk;
	AT_THK *at_thk;
	entk = &work->enethink;
	at_thk = (AT_THK *) entk->character ;

	entk->act->sw->sub_weapon = 0;
	entk->act->bodyp.type &= (~ENE_TYPE_SHIELD) ;
//	at_thk->shield = 0;
	entk->sw.finger = 0 ;
	if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){ 
		entk->sw_gun |= SW_FLAG_VISIBLE ;
	}
	if((at_thk->equip_req == AT_EQUIP_SHIELD)
	||(at_thk->equip_req == AT_EQUIP_LIGHT_SHIELD)
	){
//		printf("SHIELD REQUEST RECIEVE\n"); 
#if 1
		entk->sw.shield = SHL_ST_MAKE_NEW;
		printf("SHIELD FLAG SET\n"); 
		AT_SetType( entk->act, ENE_TYPE_SHIELD );
		entk->act->sw->sub_weapon = 1;
		entk->sw.finger = SW_FLAG_SWITCH5 ;
#else
		GV_SetActorChild( entk->w, NewAT_Shield( &work->body ,entk ,&entk->act->sub_obj) );
		if(entk->sw.shield==SHL_ST_ACTIVE) {
			printf("SHIELD FLAG SET\n"); 
			AT_SetType( entk->act, ENE_TYPE_SHIELD );
			entk->act->sw->sub_weapon = 1;
			entk->sw.finger = SW_FLAG_SWITCH5 ;
		}else {
			printf("SHIELD MAKE FAILED\n");
		}
#endif
	}
	if(entk->act->sw->sub_weapon == 1){
		/*さぶを使用*/
		entk->max_bullet = ENE_GetAmmoMax(entk->name_id.sub_weapon) ;
	}else {
		/*めいんを使用*/
		entk->max_bullet = ENE_GetAmmoMax(entk->name_id.weapon) ;
	}
}

static	void	RessModeCheck( work )
Work *work ;
{
	ENETHINK *entk;
	AT_THK *at_thk;

	entk = &work->enethink;
	at_thk = (AT_THK *) entk->character ;

	if ( entk->notice & ENE_NOTICE_RES ) {
		/*装備品設定*/
		EquipCheck( work );
		switch( GM_AlertMode ) {
			case ALERT_MODE_SNEAK :
				if(entk->status & ENE_STATUS_CONVERT){
					ENE_WatcherStartModeSneak( entk ) ;
				}else if(entk->status & ENE_STATUS_CONVERT2){
//					ENE_WatcherStartModeSneak( entk ) ;
					ENE_AttackerStartModeSneak( entk ) ;
				}else {
					ENE_AttackerStartModeSneak( entk ) ;
				}
			break ;
			case ALERT_MODE_ALERT :
				at_thk->blur_sw = -60;
				AlertModeStart(entk);
			break ;
			case ALERT_MODE_AVOID :
				Check_AttackerStartModeAvoid( entk ) ;
			break ;
			case ALERT_MODE_SEARCH :
				if(entk->status & ENE_STATUS_CONVERT){
					ENE_WatcherStartModeSneak( entk ) ;
					entk->think1 = ENE_TH1_SEARCH ; 
				}else if(entk->status & ENE_STATUS_CONVERT2){
					entk->sense.status = RADAR_COLOR_BLUE ;
					ENE_SupportStartModeCaution( entk ) ;
					entk->think1 = ENE_TH1_SEARCH ; 
				}else {
					ENE_AttackerStartModeSneak( entk ) ;
				}
			break ;
		}
	}
	/*サブスタンス追加処理*/
	if(GM_GameStatus & STATE_VR_ANOTHER){
		if(at_thk->at_status & AT_ST_TMP_TRG){
			at_thk->at_status |= AT_ST_DEFENSE ;
		}
	}

}

static void InitThinkParam( work )
Work	*work ;
{
	/*ENE_ACT内で if(dir>=0) で分岐に使っている*/
	work->action.dir = -1 ;
	work->action.body_dir = -1 ;
	work->action.pad = 0 ;
	work->enethink.status2 = 0;
	work->enethink.thk_status = 0 ;

}

static void SetSurpMode( ENETHINK *entk){
	/*周りをビックリさせる処理*/
	if(entk->at_com->surprised > 0) {
		return ;
	}
	entk->at_com->watch_status_req |= AT_COM_WATCH_DEATHBED ;
#ifdef BP_PSX2_GCC
	(FVECTOR) entk->at_com->tmptrg 	= (FVECTOR) entk->ctrl->mov ;
#else
	entk->at_com->tmptrg = entk->ctrl->mov ;
#endif
	/*プレイヤからビックリ発生地点までの距離*/
	entk->at_com->tmptrg.vw 
		= (float) entk->pl_eyei.dis ;
	entk->at_com->surprised = AT_SURP_TIME ;
}

static void SIG_AttackerDeath( ENETHINK *entk)
{
	/*周りをビックリさせる処理*/
	if(ENE_AlertGameLevel == 0) SetSurpMode(entk);
	if(ENE_AlertGameLevel < AT_MAX_LEVEL){
		ENE_AlertGameLevel++ ;
	}
	/**/
	entk->act->sw->sub_weapon = 0 ;
	if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){ 
		entk->sw_gun &= ~SW_FLAG_VISIBLE ;
	}
	if(entk->act->bodyp.type & ENE_TYPE_SHIELD){ 
		entk->sw.shield = SHL_ST_ENE_DEATH; /*盾消し*/
	}
}

/*ライフ減少時の攻撃兵処理*/
static void CheckATDmg(ENETHINK *entk){ 
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	if( at_thk->life_buf > entk->act->bodyp.life ) 
	{
		/*ダメージを受けたとき*/
		/*待機フラグリセット*/
#if 0
		ResetWaitStatus( entk ) ;
#else
	entk->at_com->watch_status &= ~AT_COM_WATCH_APPROACH ;
#endif

		/*ダメージ後は最大警戒*/
		at_thk->sight_time = 255;

		if( entk->act->bodyp.life <= 0 ) 
		{ 
			/*死亡処理いろいろ*/
			SIG_AttackerDeath( entk ) ;
		}
	}
	at_thk->life_buf = entk->act->bodyp.life ;
}

static void Think( work )
Work	*work ;
{
	ENETHINK	*entk ;
	AT_THK *at_thk ;

	
	if ( GM_VRStatus & (GM_VR_IDLE) ) return ;
#if 0
//	if ( GM_VRStatus & (GM_VR_IDLE) ) return ;
	if ( GM_VRStatus & GM_VR_CLEAR ) {
		ENE_ActStatusCheck ( &work->enethink ) ;
		work->action.old_status = work->action.status ;
		return ;
	}
#endif



	entk = &work->enethink;
	at_thk = (AT_THK *) entk->character ;

	entk = &work->enethink ;
#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			at_thk->found_addr = HZX_NO_ZONE ;
		}
	}
#endif

	InitThinkParam( work ) ;
/****************************
各ModeCheck関数内で現在のアラートレベルと自身の思考モードを比較
切り替え必要なら各ModeStart関数で別思考開始のための初期化
*****************************/


	if(GM_GameStatus & STATE_VR_ONLY){
		HZX_ChangeRouteCourse( HZX_ROOT_COURSE1 ) ;
	}
	switch( entk->think1 ){
		case ENE_TH1_SNEAK : /*通常*/
			if(entk->status & ENE_STATUS_CONVERT){
				ENE_Watcher_Think1_Sneak( entk ) ;
				ConvertSneakModeCheack( entk ) ;
			}else if(entk->status & ENE_STATUS_CONVERT2){
				ENE_Attacker_Think1_Normal( entk ) ;
				ConvertSneakModeCheack( entk ) ;
			}else {
				ENE_Attacker_Think1_Normal( entk ) ;
				SneakModeCheck( entk ) ;
			}
		break ;
		case ENE_TH1_SEARCH : /*探索*/
			if(entk->status & ENE_STATUS_CONVERT){
				ENE_Watcher_Think1_Sneak( entk ) ;
				ConvertSearchModeCheck( entk ) ;
			}else if(entk->status & ENE_STATUS_CONVERT2){
				/*************************/
				ENE_Support_Think1_Caution( entk ) ;
				Convert2SearchModeCheck( entk ) ;
				/*************************/
			}else {
				ENE_Attacker_Think1_Search( entk ) ;
				SearchModeCheck( entk ) ;
			}
		break ;
		case ENE_TH1_ALERT : /*危険*/
			Think1_Alert( entk ) ;
			AlertModeCheck( entk ) ;
		break ;
		case ENE_TH1_AVOID : /*回避*/
			Check_Attacker_Think1_Avoid( entk );
			AvoidModeCheck( entk ) ;
		break ;
		case ENE_TH1_DAMAGE :
			ENE_Enemy_Think1_Damage( entk ) ;
			/*********************
			麻酔寝中 追加処理
			thinkに影響は与えない。
			他の兵士が参照する全体フラグを操作
			*********************/
			if(CheckAnesAT(entk)){
				AT_AnesThink(entk);
			}
			DamageModeCheck( entk ) ;
			if ( entk->notice & ENE_NOTICE_RES ) {
				entk->sw.shield = SHL_ST_ENE_DEATH; /*盾消し*/
			}
			break ;
		case ENE_TH1_RESURRECT :
			ENE_AttackerResurrect( entk ) ;
			RessModeCheck( work ) ;
			break ;
		default : 
			printf("THINK1 ERR!! [%d]\n",entk->think1);
	}
	HZX_ClearRouteCourse( 0 ) ;
	/*重野追加 攻撃兵死亡時の追加処理*/
	CheckATDmg(entk) ;



}


/*--------------------------------------------------------------------*/
static void Action( work )
Work	*work ;
{

	ENETHINK	*entk ;
	CONTROL		*ctrl ;
	OBJECT		*body ;

	entk = &work->enethink ;
	ctrl = &work->control ;
	body = &work->body ;

//	entk->sense.facedir = ctrl->rot.vy ;

	ENE_ActInit( entk ) ;


	AT_Action( &work->action ) ;

	ENE_ActStatusCheck( entk ) ;

	ENE_Gravitation( entk ) ;
}


static void AttackerMain( Work *work )
{
	/***** atprepro.c *****/
	PreProcess( work ) ;
	/* スーパーアンリアルでも個別メッセージだけはうけとっておく */
	if ( work->enethink.status & ENE_STATUS_EVER_UNREAL ) return ;
	Think( work ) ;
	Action( work ) ;

}


static void Act( Work *work )
{
//	static	FVECTOR	shift = {80.0F,   0.0F,-120.0F, 0.0F};

	ENETHINK *entk;
	AT_THK *at_thk ;



	entk = &work->enethink;
	at_thk = (AT_THK *) entk->character ;

#if 0
//SIG_NumPrint(&entk->ctrl->mov ,entk->act->body->m_ctrl->mt3_ctrl->motion_num) ;
//SIG_NumPrint(&entk->ctrl->mov ,at_thk->dis_rank) ;
//SIG_NumPrint(&entk->ctrl->mov , entk->pl_eyei.sight) ;
	if(entk->iknow_flag & IKNOW_HIDDEN_PLAYER ){
		SIG_NumPrint(&entk->ctrl->mov , at_thk->dis_rank) ;
	}
#endif



#if 0
	{
		SVECTOR	rgb ;
		rgb.vx = 127 ;
		rgb.vy = 0 ;
		rgb.vz = 0 ;
//		SigZoneView(entk->at_com->at_trg_addr , &rgb, 100.0f) ; 
//		SigZoneView(entk->trgpoint.addr , &rgb, 100.0f) ; 
		rgb.vx = 0 ;
		rgb.vy = 127 ;
		rgb.vz = 0 ;
		SigZoneView(entk->znavi->next_addr , &rgb, 200.0f) ; 
		rgb.vx = 0 ;
		rgb.vy = 0 ;
		rgb.vz = 127 ;
//		SigZoneView(entk->ctrl->addr , &rgb, 300.0f) ; 
//		SigZoneView(entk->ctrl->addr , &rgb, 100.0f) ; 
//		trg_addr = entk->trgpoint.addr ;
//		PosBox(&entk->com->player_lastpos ,250.0f , &rgb );
	}
#endif


	/*ENE_GM_Act内でモーション再生とstep補正*/
	ENE_GM_Act( &work->enethink ) ;

//	if ( GM_VRStatus & (GM_VR_IDLE) ) return ;
	if ( GM_VRStatus & GM_VR_CLEAR ) {
		ENE_ActStatusCheck ( &work->enethink ) ;
		work->action.old_status = work->action.status ;
		if(work->at_thk.blur_sw > 0){
			work->at_thk.blur_sw = 0;
		}
		return ;
	}

	/*EYE_INFOセット*/
	ENE_PreProcess( &work->enethink ) ;
	AttackerMain( work ) ;
	ENE_AftProcess( &work->enethink ) ;

	/*付随エフェクト制御*/
	if(( entk->act->bodyp.life <=0)
	||(work->body.objs->flag & DG_FLAG_INVISIBLE)
	||(!(GM_CurrentStageMap & work->body.objs->group_id))
	){ 
		if(work->at_thk.blur_sw > 0){
			work->at_thk.blur_sw = 0;
		}
	}else {
		if(work->at_thk.blur_sw < 1){
			work->at_thk.blur_sw++;
		}
	}

	SIG_CheckShieldBroken(entk) ;
}

static void Die( Work *work )
{
	ENE_FreeResources( &(work->enethink) ) ;

#ifdef DEBUG_PRIM
	GM_FreePrim( work->prim );
#endif 

}

/*---------------------------------------------------------------*/
static	void	DefTargCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;
    work = ( Work * )ptr ;

//	printf("HiT !! [%d] \n",work->enethink.id ) ;
	ENE_DefTargCallBack( off, def, &work->enethink ) ;

}
static int SIG_ENE_GetResources( Work *work, int name ,int *mt_array)
{

	ENETHINK	*entk ;
	CONTROL		*ctrl ;
	ACTION		*act ;
	OBJECT		*body, *weapon, *sub_weapon  ;
	int		root, node, bodyname, wpname ,sub_wpname ,life, faint, value ,hearing;
	char	*d_pos_opt, *opt ;
	AT_THK *at_thk ;
	entk = &work->enethink ;
	entk->character = &work->at_thk ;
	at_thk = &work->at_thk ;
//	at_thk->ef_pos = &work->ef_pos ;
	ctrl = &work->control ;
	act = &work->action ;
	body = &work->body ;
	weapon = &work->weapon ;
	sub_weapon = NULL ;

	COM_SetIDNumber( entk ) ;

	entk->w = act->w = ( void * )work ;	/* 親子関係用にワークのポインタをセット */

	/*2002.06.07 終了proc追加*/
	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		entk->death_proc.argc = ENE_GclGetProc( &(entk->death_proc.proc), &entk->death_proc.argv[0] ) ;
		if ( entk->death_proc.argc > (GCL_MAX_ARGS/2) ) return -1 ;
	} else {
		entk->death_proc.proc = 0 ;
	}

	if ( (entk->status = GCL_GetOptionValue( 's', -1 )) < 0  ) {
		entk->status = 0 ;
	}
	if ( (entk->status_status = GCL_GetOptionValue( 't', -1 )) < 0  ) {
		entk->status_status = 0 ;
	}
	if((COM_GetCommander())->stage_kind & ENE_STAGE_GPS ){
		entk->status |= ENE_STATUS_URBAN ;
	}
	if ( (root = GCL_GetOptionValue( 'r', -1 )) < 0  ) {
		return -1 ;
	}
	if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
		root += GM_RouteOffset ; 
	}

	if ( (node = GCL_GetOptionValue( 'n', -1 )) < 0  ) {
		node = 0 ;
	}
	if ( (life = GCL_GetOptionValue( 'l', -1 )) < 0  ) {
		life = DEF_LIFE ;
	}
	if ( (faint = GCL_GetOptionValue( 'f', -1 )) < 0  ) {
		faint = DEF_FAINT ;
	}
	if ( (hearing  = GCL_GetOptionValue( 'h', -1 )) < 0  ) {
		hearing = DEF_HEARING ;
	}

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		entk->waiting_pos_num = GCL_GetOptionValue( 'm', -1 ) ;
	} else {
		entk->waiting_pos_num = -1 ;
	}

	/*視力設定*/
	entk->sense.eye_s_s[ALERT_MODE_SNEAK] = DEF_AT_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_ALERT] = DEF_AT_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_AVOID] = DEF_AT_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_SEARCH] = -1 ;
	if ( ( opt = GCL_GetOption( 'i' ) ) != NULL ){
		ENE_GclGetInt( opt, &entk->sense.eye_s_s[0] ) ;
	}
	if ( entk->sense.eye_s_s[ALERT_MODE_SEARCH] < 0 ) {
		entk->sense.eye_s_s[ALERT_MODE_SEARCH] = entk->sense.eye_s_s[ALERT_MODE_SNEAK] ;
	}
	if ( (value = GCL_GetOptionValue( 'v', -1 )) < 0  ) {
		entk->voice_chara = entk->id%4 ;
	} else {
		entk->voice_chara = value ;
	}
	at_thk->equip_req = GCL_GetOptionValue( 'e', AT_EQUIP_NORMAL );
	act->bodyp.type = (ENE_TYPE_ATTACKER|ENE_TYPE_DMG_HAND_LEG) ;

	at_thk->at_status = 0 ;
	/* 守備位置セット */
	d_pos_opt = GCL_GetOption( 'd' ) ;
	if ( d_pos_opt  != NULL ){
		ENE_GCL_GetFV( d_pos_opt, &entk->def_pos ) ;
		entk->def_mapbit = GM_GetHzxGroupID( GM_GetMapID( GCL_GetNextInt( ) ) );
		at_thk->def_addr = HZX_GetAddress( entk->def_mapbit, &entk->def_pos, -1 );
		at_thk->at_status |= AT_ST_DEFENSE ;
		if(GM_GameStatus & STATE_VR_ANOTHER){
			at_thk->at_status |= AT_ST_TMP_TRG ;
		}
	} else {
		entk->def_pos = ctrl->mov ;
		entk->def_mapbit = 0 ;
	}
	/*守備位置からの待避位置*/
	d_pos_opt = GCL_GetOption( 'o' ) ;
	if ( d_pos_opt  != NULL ){
		ENE_GCL_GetFV( d_pos_opt, &at_thk->def_pos2 ) ;
		at_thk->at_status |= AT_ST_DODGE ;
	}

	if ( ( opt = GCL_GetOption( 'c' ) ) != NULL ){
		entk->search_route = GCL_GetNextInt( ) ;
		if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
			entk->search_route += GM_RouteOffset ; 
		}
	}else {
		entk->search_route = root ;
	}

	/*モデル 装備設定*/
	entk->sw.shield = SHL_ST_NOP;
	entk->sw.sub_weapon = 0;
	sub_wpname = 0 ;

	if(entk->status & ENE_STATUS_CONVERT){
		if(entk->status & ENE_STATUS_CONVERT2){
			printf("ATTACKER STATUS SET ERROR!!!\n");
			return -1 ;
		}
		act->bodyp.type |= ENE_TYPE_CONVERT ;
		act->bodyp.type |= ENE_TYPE_WATCHER ;
	}
	if(entk->status & ENE_STATUS_CONVERT2){
		int			buddy ;
		act->bodyp.type |= (ENE_TYPE_SUPPORT|ENE_TYPE_CONVERT2) ;
		if ( (buddy = GCL_GetOptionValue( 'b', -1 )) < 0  ) {
			printf("Please Set Surport target name\n") ;
			return -1 ;
		}
		if ( (entk->spbudy = COM_SetSupportEnemy( buddy )) == NULL ) {
			printf("attacker.c: Err No Budy[%d]\n",buddy ) ;
			return -1 ;
		}
		entk->bd_uniq_id  = entk->spbudy->uniq_id ;
		entk->bd_eyei.flag = EYE_INFO_FLAG_SKIP ;
	}
	if(at_thk->equip_req == AT_EQUIP_HITECH_1){
		bodyname = ENE_MDL_NAME_HTC ;
		wpname = E_WP_M4_NM ;
		/*TYPE 設定*/
		act->bodyp.type |=(ENE_TYPE_HITECH|ENE_TYPE_M4) ;
	}else {
		if(entk->status & ENE_STATUS_CONVERT){
			if(entk->status & ENE_STATUS_URBAN){
				bodyname = ENE_MDL_NAME_GPS ;
			}else {
				bodyname = ENE_MDL_NAME_GBS ;
			}
		}else {
			if(entk->status & ENE_STATUS_URBAN){
				bodyname = ENE_MDL_NAME_GPA ;
			}else {
				bodyname = ENE_MDL_NAME_GBA ;
			}
		}
		switch(at_thk->equip_req){
			case AT_EQUIP_SHOTGUN :
				wpname = E_WP_SPS ;
				act->bodyp.type |= ENE_TYPE_SHOTGUN ;
				break ;
			case AT_EQUIP_SHIELD :
			case AT_EQUIP_LIGHT_SHIELD :
				if(entk->status & ENE_STATUS_URBAN){
					wpname = E_WP_ABAKAN ;
					act->bodyp.type |= ENE_TYPE_ABAKAN ;
				}else {
					wpname = E_WP_AKS ;
				}
				sub_wpname = E_WP_MKR ;
				sub_weapon = &work->sub_weapon ;
				act->bodyp.type |= ENE_TYPE_SHIELD ;
				break ;
			default :
				if(entk->status & ENE_STATUS_URBAN){
					wpname = E_WP_ABAKAN ;
					act->bodyp.type |= ENE_TYPE_ABAKAN ;
				}else {
					wpname = E_WP_AKS ;
				}
				break ;
		}
	}
	ENE_SetSenseParam( &(entk->sense), 0, DEF_EYE_RANGE,entk->sense.eye_s_s[0], hearing, DEF_SMELL ) ;
/*ここを変えれば目標変更可能*/
	ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerFindPos, &GM_PlayerAddress, NULL, &GM_PlayerMap ) ;
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		CONTROL	*ema_ctrl ;
		
//		ema_ctrl = EMA_CommandGetPosition() ;
		ema_ctrl = EMA_CommandGetControl() ;

		ENE_SetEyeInfo( &(entk->npc_eyei), &ema_ctrl->mov, 
				&ema_ctrl->addr, NULL, &ema_ctrl->hzx_id ) ;
	}


	ENE_InitControl( entk,ctrl, name ) ;

	/* モーション名セット */
	ENE_SetMotionName( entk, ENE_MOT_BASE, ENE_MOT_STAGE ) ;

#if 0
	SET_FLAG(entk->status, ENE_STATUS_NO_FINGER) ;
	SET_FLAG(entk->status, ENE_STATUS_NO_EYEANIM);
//	SET_FLAG(entk->status, ENE_STATUS_NO_VANIM)  ;
#endif

	/*VR用モデル*/
	if ( entk->status_status & ENE_STST_VRBODY ) {
		bodyname = ENE_MDL_NAME_VR_ATK ;
		AT_SetType( act, ENE_TYPE_VR ) ;
	}

	ENE_InitObject( entk, body, bodyname, ctrl, work->lights ) ;
	/* 防御ターゲットの初期化(装備より先にする)*/
	ENE_InitTarget( entk, ctrl, body, act ) ;
    GM_SetTargetCallBack( &act->bodyp.deftrg, DefTargCallBack, work ) ;
	
	ENE_InitWeapon( entk, body, weapon, wpname, NULL ) ;
	ENE_InitSubWeapon( entk, body, sub_weapon, sub_wpname ) ;
	ENE_InitAction( entk, act, ctrl, body, life, faint, DEF_BLOOD ,NULL) ;

	if(act->bodyp.type & ENE_TYPE_SHOTGUN){
		if ( !(entk->status_status & ENE_STST_OPTCAMOUFLAGE) ) {
			if ( entk->attachment != NULL ) {
				GV_DestroyOtherActor( entk->attachment ) ;
				entk->attachment = NULL ;
			}
			entk->attachment = ENE_SetAttachmentGPA_SPS( (void *)entk->w, body ) ;
		}else {
			entk->attachment = NULL ;
		}
		entk->sw.magg = 0 ;
	}


	if(act->bodyp.type & ENE_TYPE_SHIELD){
GV_SetActorChild( entk->w,(void *) NewAT_Shield( &work->body ,
entk ,&act->sub_obj) );
		ASSERT(entk->sw.shield == SHL_ST_ACTIVE) ;
		entk->sw.sub_weapon = 1;
		/*盾を握る*/
		entk->sw.finger = SW_FLAG_SWITCH5 ;
	}else {
		entk->sw.sub_weapon = 0 ;
	}
	/*装備系設定後 装弾数セット*/
	if(entk->sw.sub_weapon == 1){
		/*さぶを使用*/
		entk->max_bullet = ENE_GetAmmoMax(entk->name_id.sub_weapon) ;

	}else {
		/*めいんを使用*/
		entk->max_bullet = ENE_GetAmmoMax(entk->name_id.weapon) ;
	}
	/* 耐久力セット */
#if 0
	DURABLE_AREA0,	/* 頭部 */
	DURABLE_AREA1,	/* 胸、腰部 */
	DURABLE_AREA2,	/* 四肢 */
	DURABLE_AREA3,	/* 未使用 */
#endif

	if ( GM_GameLevel < GM_LEVEL_EASY ) {
		AT_SetDurable( &act->bodyp, 0, 0 , 0, 0 ) ;
	}else {
		if(act->bodyp.type & ENE_TYPE_HITECH){
			/*ハイテク兵は固い*/
			AT_SetDurable( &act->bodyp, 0, 10, 2, 0 ) ;
		}else if(act->bodyp.type & ENE_TYPE_CONVERT){
			/*警備兵と同じ耐久力*/
			AT_SetDurable( &act->bodyp, 0, 0, 0, 0 ) ;
		}else {
			/*防弾チョッキ以外は耐久なし*/
			AT_SetDurable( &act->bodyp, 0, 5, 0, 0 ) ;
		}
	}

#if 0
	if(act->bodyp.type & ENE_TYPE_HITECH){
		SET_FLAG(entk->status,ENE_STATUS_NO_CAPTURE) ;
	}
#endif
	at_thk->life_buf = life ;

	if ( ENE_InitThink( entk, ctrl, &work->routenavi, 
		&work->zonenavi, act, root, node ) < 0 ) {
		return -1 ;
	}

//	ENE_InitRoute( &work->cl_route ) ;
	entk->rnavi2 = &work->cl_route ;

	if(entk->status & ENE_STATUS_CONVERT){
		ENE_WatcherStartModeSneak( entk ) ;
	}else if(entk->status & ENE_STATUS_CONVERT2){
//		ENE_AttackerStartModeSneak( entk ) ;
		if( GM_AlertMode == ALERT_MODE_SEARCH ) {
			ENE_SupportStageStartModeToSearch( entk ) ;
		}else {
			ENE_AttackerStageStartModeToSneak( entk ) ;
		}
	}else {
		if( GM_AlertMode == ALERT_MODE_SEARCH ) {
//			ENE_AttackerStartModeSearchWarp( entk ) ;
			ENE_AttackerStageStartModeSearchWarp( entk ) ;
			at_thk->blur_sw = -60;
		} else {
//			ENE_AttackerStartModeSneak( entk ) ;
			ENE_AttackerStageStartModeToSneak( entk ) ;
		}
	}

	work->action.CheckDamage = ENE_EnemyDamagePad ;

	at_thk->found_addr = HZX_NO_ZONE ;


#if 1
	/*ハイテクぼんぼり*/
	if(act->bodyp.type & ENE_TYPE_HITECH){
		FVECTOR shift ={-111.0f, 101.0f,52.0f,0.0f};
		CVECTOR col ;
		col.r = 80 ;
		col.g = 20 ;
		col.b = 0 ;
		col.cd = 127 ;
		at_thk->blur_sw = -60;
		GV_SetActorChild( work , NewSIG_BlurPoint(&work->body.objs->objs[HUMAN21_ATAMA].world,
			&shift,&col ,&at_thk->blur_sw)) ;
	}
#endif
	/* ステージ情報当てはめ */
	ENE_LoadEneMemory( entk ) ;
	/* 武器位置最後に初期位置へリセット */
	ENE_ResetWeaponPosition( entk ) ;

/*待機場所から1フレームだけ見つけちゃう問題*/
/*視界off状態で起動*/
	work->action.status |= ACT_STATUS_EYE_CLOSE ;

	return (0);
}
void *NewSigAttacker( int name, int where )
{
	Work		*work ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
	if ( work != NULL ) {
printf("Attacker Get Res\n");
		work->name =name;
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( SIG_ENE_GetResources( work, name,NULL ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
printf("Attacker Get Res End\n");
	}
	return (void *)work ;
}


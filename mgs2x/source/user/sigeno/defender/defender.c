//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	defender.c
	拠点防御イベント兵
	2000/03/01 K.Sigeno
	$Id: defender.c,v 1.2 2002/12/26 11:04:53 Yoshizawa1 Exp $
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
#include	"korekado/enemy/enemy.x"

#include "../attacker/at_thk.h"
#include	"eve_a.h"
#include	"defender.h"
#include	"../attacker/sig_conv.x"

//#define DEF_POS_TEST
extern void AT_SetDurable( BODYPARAM *,int,int,int,int);
extern void ENE_AttackerResurrect( ENETHINK * );
extern void ENE_NoticeCheck( ENETHINK * );
extern void AlertModeStart( ENETHINK * ) ;
extern void ENE_EnemyStartModeDamage( ENETHINK * ) ;
extern void Think1_Alert( ENETHINK * );
extern void AlertModeStartDamage(ENETHINK *) ;
extern void ENE_Gravitation( ENETHINK * );
extern void SIG_AttackerStartModeAvoidDamage( ENETHINK * );



extern void Think1_Defense( ENETHINK * );
extern void ENE_Watcher_Think1_Sneak( ENETHINK * );
extern void ENE_Enemy_Think1_Damage( ENETHINK * );
extern void Sig_DefenseModeStart( ENETHINK * ) ;
extern void GetDefPos( ENETHINK *) ;
extern void GetDefPos2( ENETHINK *) ;
extern void GetDefPos3( ENETHINK *) ;
extern void GetDefPos4( ENETHINK *) ;
extern void DefPosFree(ENETHINK *) ;
extern void DEF_AT2DEF_Check(ENETHINK *) ;
extern void Sig_DefenseModeStartDam( ENETHINK * ) ;
extern void COM_GetResPos(FVECTOR * ,int *,int) ;

static void SIG_DefRetireCheck(Work	* ) ;


/*sigeno/attacker/sigavoid.c*/
extern void SIG_Attacker_Think1_Avoid( ENETHINK *entk ) ;
extern void SIG_AttackerStartModeAvoid( ENETHINK *entk ) ;

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern void SigZoneView( int , SVECTOR * ,float ) ; 
#endif


extern void SetDefKillFlag(void) ;


/*mesg*/
enum {
	EVENT_MSG_NO_WARP_ROUTE = 7
};
void	SIG_DefenderResurrect( ENETHINK * ) ;










static void PreProcess( Work *work )
{
	ENETHINK	*entk ;
	ENTK_TYPE_A *eve_a ;

	entk = &work->enethink ;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;

	/* スーパーアンリアルでも個別メッセージだけはうけとっておく */
	if ( work->enethink.status & ENE_STATUS_EVER_UNREAL ) return ;

	ENE_EyeInfoCheck( entk, &entk->pl_eyei ) ;
	ENE_NoticeCheck( entk ) ;
//printf("entk->notice %d\n",entk->notice);
#if 0
	if(entk->think1 == ENE_TH1_SNEAK) {
	/*巡回中は通常の警備兵システムに準拠*/
	/*ここで 視覚を危険値に反映*/
		ENE_SetAlertLevel( entk ) ;
	}else {
		entk->alert = 0 ;
	}
#else
	ENE_SetAlertLevel( entk ) ;
#endif

	eve_a->status &= ~EVE_A_ATTAK_END ;
}

/*********************
危険レベルではなく個別のモード指定で思考切り替え
*********************/
static	void	ThinkModeCheck( entk )
ENETHINK	*entk ;
{
	DEF_COM		*def_com ;


	ENTK_TYPE_A *eve_a ;
	eve_a = entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;



//	switch( entk->think_mode ) {
//	switch( entk->think1 ) {
	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
		case ALERT_MODE_SEARCH :
			if(entk->think1 != ENE_TH1_SNEAK) {
				ENE_WatcherStartModeSneak( entk ) ;
			}
		break ;
		case ALERT_MODE_ALERT :
			if(
			(entk->think1 != ENE_TH1_DEFENSE)
			&&(entk->think1 != ENE_TH1_ALERT)
			){
				if(eve_a->type <= 1){
					if(eve_a->type==1){
						GetDefPos(entk) ;
					}else{
		/*順番に使用*/
						if(eve_a->def_pos_req == -1){
							GetDefPos4( entk) ;
printf("###########SET DEF4 POS ID %d NUM %d\n",entk->id,eve_a->def_pos_num);
						}else {
printf("###########SET DEF3 POS ID %d NUM %d\n",entk->id,eve_a->def_pos_num);
							GetDefPos3( entk) ;
						}
					}
				}else {
					if(def_com->def_mode != AT_COM_DEF_DYNAMIC )
						GetDefPos3( entk) ;
				}
			}
			/*攻撃兵から守備兵に戻すチェック*/
			if((entk->think1 == ENE_TH1_ALERT)
			&&(eve_a->type <= 1)
			){
				DEF_AT2DEF_Check(entk) ;
			}
			if(eve_a->status & EVE_A_NO_DEF){
				if(entk->think1 != ENE_TH1_ALERT) {
					printf("NOW TH1 %d\n",entk->think1);
					AlertModeStart(entk);
				}
			}else {
				if(entk->think1 != ENE_TH1_DEFENSE) {
					Sig_DefenseModeStart( entk ) ;
				}
			}
		break ;
		case ALERT_MODE_AVOID :
			if(entk->think1 != ENE_TH1_AVOID) {
				SIG_AttackerStartModeAvoid( entk ) ;
			}
		break ;
	}
	if ( ENE_DamageCheck( entk ) ) {
		ENE_EnemyStartModeDamage( entk ) ;
	}
}






static	void	RessModeCheck( work )
Work *work ;
{
	ENETHINK *entk;

	entk = &work->enethink;

	if ( entk->notice & ENE_NOTICE_RES ) {
		switch( GM_AlertMode ) {
			case ALERT_MODE_SNEAK :
			case ALERT_MODE_SEARCH :
//				ENE_ResetRevMotion(entk->act);
//				SIG_DefenderStartModeSneak( entk ) ;
				ENE_WatcherStartModeSneak( entk ) ;
			break ;
			case ALERT_MODE_ALERT :
//				ENE_ResetRevMotion(entk->act);
				AlertModeStart(entk);
			break ;
		}
	}
}
//				eve_a->status |= EVE_A_NO_DEF ;

static	void	DamageModeCheck( entk )
ENETHINK	*entk ;
{
//if(entk->id == 1) printf("DMG MODE CHECK!!  %d\n",entk->think_mode ) ;
	ENTK_TYPE_A *eve_a ;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;

/*2000.01.12 */
	if((entk->act->bodyp.life <= 0)||(CheckNoActive(entk))){
		eve_a->status &= ~EVE_A_OBSERVE ;
	}
/*****************/
	/*ダメージ後の復帰処理*/
	/*死んでないので思考セット後 再始動*/
	if ( entk->notice & ENE_NOTICE_DAMAGE ) {
printf("NOTICE\n");
		eve_a->status |= EVE_A_ATTAK_END ;
		switch( GM_AlertMode ) {
			case ALERT_MODE_SNEAK :
			case ALERT_MODE_SEARCH :
				ENE_WatcherStartModeSneakDamage( entk ) ;
			break ;
			case ALERT_MODE_ALERT :
				if(eve_a->status & EVE_A_NO_DEF){
					if(entk->think1 != ENE_TH1_ALERT) {
						AlertModeStart(entk);
					/*ダメージ復帰*/
						AlertModeStartDamage(entk);
					}
				}else {
					if(entk->think1 != ENE_TH1_DEFENSE) {
						Sig_DefenseModeStart( entk ) ;
						/*ダメージ時の追加処理*/
						Sig_DefenseModeStartDam( entk ) ;
printf("DAM 2 DEF!!\n");
					}
				}
			break ;
			case ALERT_MODE_AVOID :
				SIG_AttackerStartModeAvoidDamage( entk ) ;
			if ( entk->act->bodyp.pbreak != 0 ) {
				entk->act->bodyp.pbreak = 0 ; /* 超暫定！！アクトでやり */
			}
			break ;
		}
		if ( ENE_DamageCheck( entk ) ) {
			ENE_EnemyStartModeDamage( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_RES ) {
printf("DEATH !!!!!!!!!\n");
		eve_a->status |= EVE_A_ATTAK_END ;
		/*死んでた*/
		/*目撃者から除外*/
		eve_a->status &= ~EVE_A_OBSERVE ;
		eve_a->status |= EVE_A_DEATH ;
		if(eve_a->type==0){
			/*2001.01.15 死亡後は新POS*/
			DefPosFree(entk);
		}
#if 1
		/*復活*/
		SIG_DefenderResurrect( entk ) ;
#endif
	}
}

void	SIG_DefenderResurrect( entk )
ENETHINK	*entk ;
{
	FVECTOR	pos ;
	int		hzx_id ;
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;

	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;
printf("DEFENDER RESURE!\n");
	entk->act->dir = -1 ;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;


	if(def_com->max_res_num >= 0){
		if(def_com->max_res_num <= def_com->res_num){
			return ;
		}
	}
	def_com->res_num++ ;



	ENE_RefreshParam( entk ) ;
//	DefRefreshParam( entk ) ;
	if(eve_a->type >1){
		GetDefPos2( entk) ;
	}else {
		if(eve_a->type ==1){
			/*test 交代処理廃止 シナリオ指定ｐｏｓへ*/
			GetDefPos3( entk) ;
		}else {
			GetDefPos4( entk) ;
		}

	}

/*2001.01.17*/
#if 0
	COM_ResurrectionPosition( &pos, &hzx_id ) ;
#else
	COM_GetResPos( &pos, &hzx_id ,(int)def_com->res_pos_num ) ;
#endif
	HZX_FlashTrap( entk->ctrl->hzx_id, &entk->ctrl->evt ) ;
	GM_ResetControlPositionAndGroup( entk->ctrl, &pos, hzx_id ) ;/* 再発生場所へ移動 */
	Sig_DefenseModeStart( entk ) ;
	/*2001.02.01 左舷突入モード時の再発生は別思考へ*/


#if 0
	/*コマンダーまかせだと1フレーム遅れるので*/
	if(def_com->def_mode == AT_COM_DEF_WAIT ){
#else
	/*自前で判断*/
	if(
	( eve_a->type >=2 )
	&&(entk->com->res_count_in_alert > (entk->com->max_res_in_alert-entk->at_com->unit[0].enemy_num))
	&&(entk->com->res_count_in_alert != entk->com->max_res_in_alert)
	){
#endif
		entk->think1 = ENE_TH1_DEFENSE ; 
		entk->think2 = TH2_MOVE ; 
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
	}
	eve_a->status &= ~EVE_A_NO_DEF ;

#if 0
	/*コマンダーまかせだと1フレーム遅れるので*/
	if(def_com->def_mode == AT_COM_DEF_DYNAMIC )
#else
	/*自前で判断*/
	/*再発生処理 通常長廊下は再発生数MAXまできたら突撃モードで開始*/
	if((entk->com->res_count_in_alert == entk->com->max_res_in_alert)
		&&(!(eve_a->another_flag & ANOTHER_DARK_CAMP )))
#endif
	{
		eve_a->status |= EVE_A_NO_DEF ;
		DefPosFree(entk);
	}
	entk->count3 = 0 ;
}
#if 0
void	SIG_DefenderResurrect2( entk )
ENETHINK	*entk ;
{
	FVECTOR	pos ;
	int		hzx_id ;
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;

	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;
	entk->act->dir = -1 ;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	ENE_RefreshParam( entk ) ;
//	DefRefreshParam( entk ) ;
	if(eve_a->type >1){
		GetDefPos2( entk) ;
	}else {
		if(eve_a->type ==1){
			/*test 交代処理廃止 シナリオ指定ｐｏｓへ*/
			GetDefPos3( entk) ;
		}else {
			GetDefPos4( entk) ;
		}

	}

/*2001.01.17*/
#if 0
	COM_ResurrectionPosition( &pos, &hzx_id ) ;
#else
	COM_GetResPos( &pos, &hzx_id ,def_com->res_pos_num ) ;
#endif
	HZX_FlashTrap( entk->ctrl->hzx_id, &entk->ctrl->evt ) ;
	GM_ResetControlPositionAndGroup( entk->ctrl, &pos, hzx_id ) ;/* 再発生場所へ移動 */
}
#endif






#define DEF_FAINT_TIME (120)
static void SIG_DefRetireCheck(Work	*work )
{
extern int SIG_BoundCheck(FVECTOR * , float ,float ) ;

	int retire = 0 ;
	ENETHINK *entk ;
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;



	entk = &work->enethink ;
	eve_a = entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;

	if(eve_a->type < 2) return  ;

#if 0
	/*気絶退出処理 独自だったのを廃止して共通のFAINT_EXITを使用*/
	if(( entk->act->bodyp.life >0)&&(entk->act->bodyp.faint_time > DEF_FAINT_TIME )){
		/*行動不能フラグ*/
		entk->act->status |= ACT_STATUS_TARGET_SKIP ;
		retire = 1 ;
		if(entk->com->max_res_in_alert == entk->com->res_count_in_alert){
			def_com->retire_num++;
		}
	}
	if((retire)
	&&(eve_a->type >= 2)	//ｗ03b
	&&(entk->com->max_res_in_alert > entk->com->res_count_in_alert)
	&&(entk->count3 > DIRECT_TICK(RETIRE_NO_CHECK_TIME))
	){
		int test ;
		if(
		(entk->count3 > DIRECT_TICK(RETIRE_CHECK_TIME_ABS-RETIRE_CHECK_HALH))
		&&(entk->count3 <= DIRECT_TICK(RETIRE_CHECK_TIME_ABS))){
			/*点滅*/
			if(entk->count3 & 1){
				SIG_AllObjectInvisible(&work->body) ;
			}else {
				SIG_AllObjectVisible(&work->body) ;
			}
		}
		if(entk->count3 > DIRECT_TICK(RETIRE_CHECK_TIME_ABS)){
			test = -1 ;
		}else {
			test = SIG_BoundCheck(&entk->znavi->flore_pos,1000.0F,2000.0F) ;
		}
		if(
		(test  < 0)
		){
			/*再発生*/
			/*resをみて再発生後のモード切替を行うので先にインクリしてから*/
			entk->com->max_res_in_alert--;
/*暗視ゴーグルOFF*/
			entk->act->sw->n_sight = 1 ;

			SIG_DefenderResurrect( entk ) ;
			SIG_AllObjectVisible(&work->body) ;
		}
	}else {
		SIG_AllObjectVisible(&work->body) ;
	}
#endif
}


static void DEF_DeathProc(ENETHINK *entk ){ 
	DEF_COM		*def_com ;
	ENTK_TYPE_A *eve_a ;

	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;

	def_com->destroy++;
}


#if 0
static void InitThinkParam( work )
Work	*work ;
{
	/*ENE_ACT内で if(dir>=0) で分岐に使っている*/
	work->action.dir = -1 ;
	work->action.pad = 0 ;
	work->enethink.status2 = 0;
	/*回避モードには参加しない*/
	work->enethink.avoid = 0 ;
	work->enethink.scene = CLE_ENE_WAIT_COMPLETE ;

}
#else 
static void InitThinkParam( work )
Work	*work ;
{
	work->action.dir = -1 ;
	work->action.body_dir = -1 ;
	work->action.pad = 0 ;
	work->enethink.status2 = 0 ;
	work->enethink.thk_status = 0 ;
	/*回避モードには参加しない*/
	work->enethink.avoid = 0 ;
	work->enethink.scene = CLE_ENE_WAIT_COMPLETE ;

}

#endif

static void Think( work )
Work	*work ;
{
	ENETHINK	*entk ;
	ENTK_TYPE_A *eve_a ;
	entk = &work->enethink ;
	eve_a = entk->eve_a ;

#if 0
	if ( GM_VRStatus & (GM_VR_IDLE) ) return ;
	if ( GM_VRStatus & GM_VR_CLEAR ) {
		ENE_ActStatusCheck ( &work->enethink ) ;
		work->action.old_status = work->action.status ;
		return ;
	}
#endif
	InitThinkParam( work ) ;

/****************************
各ModeCheck関数内で現在のアラートレベルと自身の思考モードを比較
切り替え必要なら各ModeStart関数で別思考開始のための初期化
*****************************/

	/* 暗視ゴーグルON */
	if(entk->act->bodyp.type & ENE_TYPE_EVENT_A){
		ENTK_TYPE_A *eve_a ;
		/*長廊下兵*/
		eve_a = ( ENTK_TYPE_A * ) entk->eve_a ;
		if(eve_a->type >1){
			/*右舷*/
			entk->act->sw->n_sight = 2 ;
		}
	}

	if(
	(eve_a->type <= 1 )&&(~ eve_a->status & EVE_A_OBSERVE)
	)
	{
//		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		if(( entk->alert == ALERT_LEVEL_MAX )&&(entk->act->bodyp.life >0)){
			if((SIG_CheckStealthStatus(entk))||(CheckNoActive(entk))){
			}else {
				eve_a->status |= EVE_A_OBSERVE;
				COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
			}
		}
	}

	if(GM_GameStatus & STATE_VR_ONLY){
		HZX_ChangeRouteCourse( HZX_ROOT_COURSE1 ) ;
	}
	switch( entk->think1 ){
		case ENE_TH1_DAMAGE :
			/*ダメージ中及び死亡から再発生まで*/
			ENE_Enemy_Think1_Damage( entk ) ;
			/*麻酔チェック*/
			DamageModeCheck( entk );
			break ;
		case ENE_TH1_RESURRECT :
			ENE_AttackerResurrect( entk ) ;
			RessModeCheck( work ) ;
			break ;
		case ENE_TH1_DEFENSE : /*銃撃戦モード*/
			if(SIG_CheckStealthStatus(entk)||(CheckNoActive(entk))){
			}else {
				eve_a->status |= EVE_A_OBSERVE;
			}
			Think1_Defense( entk ) ;
			ThinkModeCheck( entk ) ;
			break ;
		case ENE_TH1_ALERT : /*危険*/
			Think1_Alert( entk ) ;
			ThinkModeCheck( entk ) ;
			break ;
		case ENE_TH1_SNEAK : /*通常*/
			CLEAR_FLAG( entk->iknow_flag ) ;
			ENE_Watcher_Think1_Sneak( entk ) ;
			ThinkModeCheck( entk ) ;
			break ;
		case ENE_TH1_AVOID : /*回避*/
			CLEAR_FLAG( entk->iknow_flag ) ;
//printf("THK_MODE %d\n",entk->think_mode);

//			Check_Attacker_Think1_Avoid( entk ) ;
			SIG_Attacker_Think1_Avoid( entk ) ;
			ThinkModeCheck( entk ) ;
			break ;
	}
	HZX_ClearRouteCourse( 0 ) ;
	/*一度発見したら死亡まで危険レベルを維持*/
	if(SIG_CheckStealthStatus(entk)||(CheckNoActive(entk))){
		eve_a->status &= ~EVE_A_OBSERVE ;
	}
	if(eve_a->status & EVE_A_OBSERVE){
		entk->alert = ALERT_LEVEL_MAX ;
	}

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
/*************************/
	SIG_DefRetireCheck( work ) ;
/*************************/
	ENE_ActStatusCheck( entk ) ;
	ENE_Gravitation( entk ) ;
}


static void DefenderMain( Work *work )
{
	ENETHINK *entk;

	entk = &work->enethink ;
	PreProcess( work ) ;
	Think( work ) ;

	Action( work ) ;
}


static void	DEF_CheckMessage( entk )
ENETHINK	*entk ;
{
    GV_MSG	*msg ;
	int n_msg, code ;
	FVECTOR		pos;
	ENTK_TYPE_A	*eve_a ;


	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;
	
    n_msg = entk->ctrl->n_msg ;
	msg = entk->ctrl->msg ;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;



	if  ( n_msg > 0 ) {
		printf("message get id=[%d]\n",entk->id ) ;
	}

	if(entk->act->bodyp.type & ENE_TYPE_EVENT_A){
		while ( n_msg-- > 0 ) {
			code = msg->message[ 0 ] ;
			switch( code ) {
				case EVENT_MSG_DEFENSE :
					eve_a->status  &= ~EVE_A_SUPORT_WAIT ;
//					entk->think_mode = ENE_TH1_DEFENSE ;
					entk->think1 = ENE_TH1_DEFENSE ;
					break;
				case EVENT_MSG_ALERT :
					eve_a->status  &= ~EVE_A_SUPORT_WAIT ;
//					entk->think_mode = ENE_TH1_ALERT ;
					entk->think1 = ENE_TH1_ALERT ;
					break;
				case EVENT_MSG_WARP :
#if 1
					/*画面内チェック*/
					if(SIG_BoundCheck(&entk->znavi->flore_pos,1500.0F,2000.0F)  < 0){
						pos.vx = (float)msg->message[ 1 ] ;
						pos.vy = (float)msg->message[ 2 ]+1000 ;
						pos.vz = (float)msg->message[ 3 ] ;
	HZX_FlashTrap( entk->ctrl->hzx_id, &entk->ctrl->evt ) ;
						GM_ResetControlPositionAndGroup( entk->ctrl, &pos, GM_GetMapID( msg->message[ 4 ] ) );
/*ワープ後自動的に追跡モード*/
						eve_a->status  &= ~EVE_A_SUPORT_WAIT ;
					}
#endif
					break;
				case EVENT_MSG_ROOTCHANGE :
					entk->rnavi->next_route = (short)msg->message[ 1 ] ;
					if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
//						entk->rnavi->next_route += COM_GetRootOfset( ) ;
						entk->rnavi->next_route += GM_RouteOffset ; 
					}
					if ( msg->message_len >= 4 ) {
						entk->def_pos.vx = (float)msg->message[ 2 ] ;
						entk->def_pos.vy = (float)msg->message[ 3 ] ;
						entk->def_pos.vz = (float)msg->message[ 4 ] ;
						entk->def_mapbit = 
							GM_GetMapID( msg->message[ 5 ] ) ;
					}
					if ( entk->act->status & ACT_STATUS_UNREAL ) {
						ENE_RouteWarp( entk ) ;
					}
#if 1
printf("w03a enemy deffender:change route [%d]->[%d] \n",entk->rnavi->c_route, entk->rnavi->next_route ) ;
#endif
					break ;
				case EVENT_MSG_NO_WARP_ROUTE :
					entk->rnavi->next_route = (short)msg->message[ 1 ] ;
					if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
//						entk->rnavi->next_route += COM_GetRootOfset( ) ;
						entk->rnavi->next_route += GM_RouteOffset ; 
					}
					break ;
				/*ビデオ用モード切り替え*/
				case EVENT_MSG_MODE :
					eve_a->video = (int) msg->message[ 1 ] ;
					break;
				case EVENT_MSG_RESET :
					/**/
					eve_a->video = 1 ;
					eve_a->seq_index = 0 ;
					eve_a->seq_time = 0;
					eve_a->det_time = 0;
					at_thk->at_tmptime = 0;
					entk->count3 = 0 ;
					break;
#if 0
				case EVENT_MSG_POSSET :
					eve_a->def_pos_req = (int) msg->message[ 1 ] ;
					break;
#endif
			}
			msg++ ;
		}
	}
}

static void Act( Work *work )
{

	ENETHINK *entk;
	AT_THK *at_thk ;

	at_thk = &work->at_thk ;
	entk = &work->enethink;




/*TEST*/
#if 0

SIG_NumPrint(&entk->ctrl->mov ,entk->pl_eyei.sight) ;
	if(entk->iknow_flag & IKNOW_ZZZ){
		printf("I'm sleeping ! %d\n",entk->id);
	}
#endif
#if 0
	if(entk->status & ENE_STATUS_EVER_ZZZ ){
		printf("ENE_STATUS_EVER_ZZZ %d\n",entk->id);
	}
#endif

	if(
	( entk->act->bodyp.dammode == DAM_MODE_MASUI_DOWN )
	&&( entk->status & ENE_STATUS_EVER_ZZZ )
	){
//		printf("I'm sleeping ! %d\n",entk->id);
	}


	ENE_GM_Act( entk ) ;

	if ( GM_VRStatus & (GM_VR_IDLE) ) return ;
	if ( GM_VRStatus & GM_VR_CLEAR ) {
		ENE_ActStatusCheck ( &work->enethink ) ;
		work->action.old_status = work->action.status ;
		return ;
	}

	DEF_CheckMessage( entk );

	/*EYE_INFOセット*/
	ENE_PreProcess( entk ) ;


	DefenderMain( work ) ;
	ENE_AftProcess( entk ) ;

	if(( entk->act->bodyp.life <=0)
	&&(at_thk->life_buf >0)
	){ 
		DEF_DeathProc(entk);
		SetDefKillFlag() ;
	}
	at_thk->life_buf = entk->act->bodyp.life ;

#if 0
	if( entk->id ==0 ) {
		if(work->enethink.act->sw->n_sight == 2){
			printf("SIGHT ON!\n");
		}
		if(work->enethink.act->sw->n_sight == 1){
			printf("SIGHT OFFFFFFF!\n");
		}
	}
#endif

}

static void Die( Work *work )
{
printf("DEFENDER DIE!!\n");
	ENE_FreeResources( &(work->enethink) ) ;
}


/*---------------------------------------------------------------*/
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
static int SIG_ENE_GetResources( Work *work, int name, int type ,int *mt_array)
{

	ENETHINK	*entk ;
	CONTROL		*ctrl ;
	ACTION		*act ;
	OBJECT		*body, *weapon, *sub_weapon  ;
	int		root, node, bodyname, wpname ,sub_wpname ,life, faint, value,equip;
//	FVECTOR	d_pos;
	char	*d_pos_opt, *opt ;
	AT_THK *at_thk ;

#if 0
	equip = GCL_GetOptionValue( 'e', AT_EQUIP_NORMAL );
#else
	equip = AT_EQUIP_NORMAL ;
#endif
	entk = &work->enethink ;
	/*entkの中身を可視*/
	entk->character = &work->at_thk ;
	at_thk = &work->at_thk ;
	ctrl = &work->control ;
	act = &work->action ;
	body = &work->body ;
	weapon = &work->weapon ;
//	sub_weapon = &work->sub_weapon ;
	sub_weapon = NULL ;

	entk->w = act->w = ( void * )work ;	/* 親子関係用にワークのポインタをセット */
	if ( (entk->status = GCL_GetOptionValue( 's', -1 )) < 0  ) {
		entk->status = 0 ;
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

	/*視力設定*/
//	eye_s = GCL_GetOptionValue( 'i', DEF_AT_EYE_SIGHT );
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
	act->bodyp.type = ENE_TYPE_ATTACKER;

	if ( entk->status_status & ENE_STST_VRBODY ) {
		bodyname = ENE_MDL_NAME_VR ;
		act->bodyp.type |= ENE_TYPE_VR ;
	} else {
		bodyname = ENE_MDL_NAME_GBS ;
	}

	/*装備品*/
	/*こっちは兵士の種類を決定*/
	at_thk->equip_req = equip ;
	/*ショットガン装備兵*/
	wpname = E_WP_AKS ;
//	sub_wpname = E_WP_MKR ;
	sub_wpname = 0 ;

printf("ATTACKER COMSET Id NUMBER!!!!!!!!!!!!!!\n") ;
	ENE_SetSenseParam( &(entk->sense), 0, DEF_EYE_RANGE,
	entk->sense.eye_s_s[0], DEF_HEARING, DEF_SMELL ) ;
/*ここを変えれば目標変更可能*/
	ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerFindPos, 
	&GM_PlayerAddress, NULL, &GM_PlayerMap ) ;
	ENE_InitControl( entk,ctrl, name ) ;

	/* モーション名セット */
	ENE_SetMotionName( entk, ENE_MOT_BASE, ENE_MOT_STAGE ) ;
	ENE_InitObject( entk, body, bodyname, ctrl, work->lights ) ;
	/* 防御ターゲットの初期化(装備より先にする)*/
	ENE_InitTarget( entk, ctrl, body, act ) ;
    GM_SetTargetCallBack( &act->bodyp.deftrg, DefTargCallBack, work ) ;
	
	ENE_InitWeapon( entk, body, weapon, wpname, NULL ) ;
	ENE_InitSubWeapon( entk, body, sub_weapon, sub_wpname ) ;

	ENE_InitAction( entk, act, ctrl, body, life, faint, DEF_BLOOD ,NULL) ;
#if 0
	/*
	現在 麻酔値はbloodの半分の値が自動セットされている。
	麻酔を個別設定するには
	BODYPARAM	*bodyp ;
	bodyp->anesthesia = bodyp->m_anesthesia = 麻酔耐久度 
	*/

#endif
	/* 耐久力セット */
	AT_SetDurable( &act->bodyp, 0, 5, 3, 0 ) ;

	at_thk->life_buf = life ;

	if ( (value = GCL_GetOptionValue( 'v', -1 )) < 0  ) {
		entk->voice_chara = entk->id%4 ;
	} else {
		entk->voice_chara = value ;
	}

	if ( ENE_InitThink( entk, ctrl, &work->routenavi, 
		&work->zonenavi, act, root, node ) < 0 ) {
		return -1 ;
	}
	entk->rnavi2 = &work->cl_route ;
	if( GM_AlertMode == ALERT_MODE_SEARCH ) {
		ENE_AttackerStartModeSearchWarp( &work->enethink ) ;
	} else {
		ENE_AttackerStartModeSneak( &work->enethink ) ;
	}

	work->action.CheckDamage = ENE_EnemyDamagePad ;

	at_thk->at_status = 0 ;
	/* 守備位置セット */
	d_pos_opt = GCL_GetOption( 'd' ) ;
	if ( d_pos_opt  != NULL ){
		ENE_GCL_GetFV( d_pos_opt, &entk->def_pos ) ;
		entk->def_mapbit = 
			GM_GetHzxGroupID( GM_GetMapID( GCL_GetNextInt( ) ) );
		at_thk->def_addr = 
//		HZX_GetAddress( GV_GetBit( int no ), &entk->def_pos, -1 );
	HZX_GetAddress( entk->def_mapbit, &entk->def_pos, -1 );

		at_thk->at_status |= AT_ST_DEFENSE ;

printf("Scen Set def_map = %x \n",entk->def_mapbit );
	} else {
		entk->def_pos = ctrl->mov ;
		entk->def_mapbit = 0 ;
	}
	entk->search_route = GCL_GetOptionValue( 'c', root ) ;

	/*盾壊れた時連絡*/
	/*装備系 設定初期化*/
	entk->sw.shield = 0;
	act->sw->sub_weapon = 0;
	/*装備系設定後 装弾数セット*/
	/*めいんを使用*/
	entk->max_bullet = ENE_GetAmmoMax(entk->name_id.weapon) ;
	at_thk->found_addr = HZX_NO_ZONE ;

	return (0);
}
static int GetResources( Work *work, int name )
{

	ENETHINK	*entk ;
	ENTK_TYPE_A *eve_a ;

	ACTION		*act ;
//	OBJECT		*body, *weapon, *sub_weapon  ;
//	FVECTOR	d_pos;
	char	*opt ;
//	int	i,mode;

	entk = &work->enethink ;
	COM_SetIDNumber( entk ) ;

	if ( (entk->status_status = GCL_GetOptionValue( 'k', -1 )) < 0  ) {
		entk->status_status = 0 ;
	}

	SIG_ENE_GetResources(work ,name ,ENE_TYPE_EVENT_A,NULL) ;
	eve_a = entk->eve_a = &work->entk_a ;
	act = &work->action ;

	eve_a->another_flag = GCL_GetOptionValue( 'v', ANOTHER_NORMAL );

	/*守備兵に必須の非通報フラグ*/
	SET_FLAG(entk->status, (ENE_STATUS_NO_FINGER|ENE_STATUS_LONELY_FIGHT)) ;

	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		entk->death_proc.argc = 
		ENE_GclGetProc( &(entk->death_proc.proc),
		&entk->death_proc.argv[0] ) ;
		if ( entk->death_proc.argc > (GCL_MAX_ARGS/2) ) return -1 ;
	} else {
		entk->death_proc.proc = 0 ;
	}
	/*視力上書き*/
#if 1
	entk->sense.eye_s_s[ALERT_MODE_SNEAK] = DEF_DEF_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_ALERT] = DEF_DEF_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_AVOID] = DEF_DEF_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_SEARCH] = DEF_DEF_EYE_SIGHT ;
#endif
	/* 耐久力セット */
//	AT_SetDurable( &act->bodyp, 0, 5, 3, 0 ) ;
	/*弱め*/
	AT_SetDurable( &act->bodyp, 0, 0, 0, 0 ) ;

	if ( (entk->act->bgm_track = GCL_GetOptionValue( 'b', -1 )) < 0  ) {
		entk->act->bgm_track = 0 ;
	} else {
#ifdef KP_WINDOWS		
		//-------------------------------------------------
		//	ＢＧＭ＿ＳＥの再生トラックを0x10以降に全て面した為
		
		if(entk->act->bgm_track < 0x10){
			entk->act->bgm_track += 0x10;
		}
#endif
		/* 個別ＢＧＭトラックを初期化する */
		GM_MixConvFader( entk->act->bgm_track, 0x20, 0 ) ;
	}
	/*照準ぶれ幅*/
//	eve_a->shot_rnd = GCL_GetOptionValue( 'a',0);
	eve_a->shot_rnd = 2000 ;

//	ENE_InitRoute( &work->cl_route ) ;
	/*防御兵である*/
	act->bodyp.type |= ENE_TYPE_EVENT_A;


	entk->def_mapbit = GM_CurrentMap ;
	entk->def_pos = DG_ZeroVector ;
	/*起動時のモードセレクト*/
	eve_a->type = GCL_GetOptionValue( 't', 0 ) ;
	if(eve_a->type > 1){
		/*左舷兵*/
		/*気絶退出処理 独自だったのを廃止して共通のFAINT_EXITを使用*/
		entk->status_status |= ENE_STST_FAINT_EXIT ;
		AT_SetType( act, ENE_TYPE_FAINT_EXIT ) ;
	}

	eve_a->status = 0;

	eve_a->def_pos_num = -1; /*守備位置番号*/
	eve_a->def_pos_req = -1;

	eve_a->seq_index = 0 ;
	eve_a->det_time = 0 ; /** 援護呼びまでのカウンタ **/
	eve_a->video = 0;
	eve_a->level = 0;
	eve_a->shuffle_time = 0 ;
	entk->think1 = ENE_TH1_SNEAK ;
/*手足ダメージでライフ減る*/
	act->bodyp.type |= ENE_TYPE_DMG_HAND_LEG ;



	SET_FLAG(act->bodyp.type,ENE_TYPE_NO_PBREAK);

	switch( eve_a->type ){
// type は０左舷巡回１左舷追加 ２右舷守備 ３右舷後方
		//巡回
		case 0 :
		case 1 :
printf("TYPE 0 ENTK ID %d\n",entk->id);
			entk->think1 = ENE_TH1_SNEAK ;
			ENE_WatcherStartModeSneak( entk ) ;
		break;
/*以下右舷*/
		case 2 :
printf("TYPE 2 ENTK ID %d\n",entk->id);
		entk->status |= (ENE_STATUS_EVER_ZZZ|ENE_STATUS_NO_STEALTH) ;

		break;
		case 3 :
printf("TYPE 3 ENTK ID %d\n",entk->id);
		entk->status |= (ENE_STATUS_EVER_ZZZ|ENE_STATUS_NO_STEALTH) ;
		break;
	}
	/* 武器位置最後に初期位置へリセット */
	ENE_ResetWeaponPosition( entk ) ;
	return (0);
}

void *NewSigDefender( int name, int where )
{
	Work		*work ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
	if ( work != NULL ) {
		work->name =name;
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, name ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


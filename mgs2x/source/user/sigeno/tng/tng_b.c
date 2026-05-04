//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	defender.c
	拠点防御イベント兵
	2000/03/01 K.Sigeno
	$Id: tng_b.c,v 1.1.1.3 2002/11/19 11:49:53 Yoshizawa1 Exp $
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
#include "korekado/enemy/enemy.x"

//#include	"motion.h"

//#include	"tng_motion.h"
//#include	"tng_array.h"

#include	"../attacker/at_thk.h"
#include	"tng.h"
#include	"tng_a.h"
#include	"tngthink.x"
#include	"../attacker/sig_conv.x"


//#define DEF_POS_TEST
extern void AT_SetDurable( BODYPARAM *,int,int,int,int);

extern void ENE_AttackerResurrect( ENETHINK * );
extern void ENE_NoticeCheck( ENETHINK * );
extern void TngAlertModeStart( ENETHINK * ) ;
extern void ENE_EnemyStartModeDamage( ENETHINK * ) ;
extern void Tng_Think1_Alert( ENETHINK * );
extern void ENE_Gravitation( ENETHINK * );
extern void TngReadNode( ENETHINK *) ;
extern int CheckNoActive(ENETHINK * ) ;

//extern void ENE_Watcher_Think1_Sneak( ENETHINK * );
extern void ENE_Enemy_Think1_Damage( ENETHINK * );

//extern int SIG_ZoneBoundCheck(HZX_ZON * ,float ) ;
extern int SIG_BoundCheck(FVECTOR * , float ,float ) ;

extern void Sig_TngDefenseModeStart( ENETHINK *) ;
extern void Sig_TngDefenseModeStart2( ENETHINK *) ;


extern void TNG_COM_GetResPos(TNG_COM * ,FVECTOR *,int *,int) ;

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern void SigZoneView( int , SVECTOR * ,float ) ; 
#endif
extern void	*NewTngKatana( OBJECT *body,short *flag,ENETHINK *entk ) ;
extern void TngKatanaOn(ENETHINK *) ;
extern void TngKatanaOff(ENETHINK *) ;
extern void TngKatanaBlurOff(ENETHINK *) ;

extern void *NewSIG_BlurPoint(FMATRIX *,FVECTOR *,CVECTOR * ,int *) ;


extern void Think1_TngDefense(ENETHINK *) ;
extern void SetTngKillFlag(void) ;


static void	TNG_DefenderResurrect( ENETHINK * ) ;
static void TNG_RetireCheck( Work * ) ;


static void ResetHomingStatus(ENETHINK *entk,int flag){
	entk->hom.status &= ~flag ;
}
static void TNG_UnSetUnrealStatus(ENETHINK *entk){
	/*行動可能*/
	DG_VisibleObjs( entk->act->body->objs ) ;
	AT_UnSetTargetClass( entk->act, TARGET_SKIP ) ;
	SET_FLAG( entk->act->bodyp.deftrg.class, TARGET_LOCKON ) ;
}
static void TNG_SetUnrealStatus(ENETHINK *entk){
	entk->act->dir = -1 ;
	DG_InvisibleObjs( entk->act->body->objs ) ;
	AT_SetTargetClass( entk->act, TARGET_SKIP ) ;
	UNSET_FLAG( entk->act->bodyp.deftrg.class, TARGET_LOCKON ) ;
}
static void TNG_CheckTrgStatus(ENETHINK *entk){
	/*行動可能*/
	if(entk->act->bodyp.deftrg.class & TARGET_SKIP ){
		entk->act->bodyp.deftrg.class &= ~TARGET_LOCKON ;
	}
}


/*マルチテクスチャの有効条件チェック*/
static int CheckTexMode(void){
	if(GM_PlayerStatus & PLAYER_WATCH){
		return 1 ;
	}
	return 0 ;
}
/*死亡した場合の処理をまとめる*/
static void Tng_DeathProc(ENETHINK *entk ){ 
	TNG_COM		*tng_com ;
	ENTK_TENG_A *eve_a ;

	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;
	tng_com->destroy++;
}
static void PreProcess( Work *work )
{
	ENETHINK	*entk ;

	entk = &work->enethink ;


	/* スーパーアンリアルでも個別メッセージだけはうけとっておく */
	if ( work->enethink.status & ENE_STATUS_EVER_UNREAL ) return ;

	ENE_NoticeCheck( entk ) ;
	ENE_EyeInfoCheck( entk, &entk->pl_eyei ) ;
#if 0
	if(entk->think1 == ENE_TH1_SNEAK) {
	/*巡回中は通常の警備兵システムに準拠*/
	/*ここで 視覚を危険値に反映*/
		ENE_SetAlertLevel( entk ) ;
	}else {
		entk->alert = 0 ;
	}
#else
/*発見すると危険レベル変更*/
	ENE_SetAlertLevel( entk ) ;
#endif

}

/*********************
危険レベルではなく個別のモード指定で思考切り替え
*********************/
static	void	ThinkModeCheck( entk )
ENETHINK	*entk ;
{
	TNG_COM		*tng_com ;
	ENTK_TENG_A *eve_a ;

	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
		case ALERT_MODE_SEARCH :
			if(entk->think1 != ENE_TH1_SNEAK) {
//				ENE_WatcherStartModeSneak( entk ) ;
			}
		break ;
		case ALERT_MODE_ALERT :
			if(entk->think1 != ENE_TH1_ALERT){
				if(
				(entk->act->bodyp.type & ENE_TYPE_KATANA)
				||(eve_a->status & EVE_TNG_NO_DEF )
				||(tng_com->phase == TNG_PH_END)
				){
					TngAlertModeStart( entk ) ;
				}
			}
			if(
			(entk->think1 != ENE_TH1_ALERT) 
			&&(entk->think1 != ENE_TH1_DEFENSE) 
			){
//				TngAlertModeStart(entk);
				Sig_TngDefenseModeStart(entk);
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
//				TngAlertModeStart(entk);
				Sig_TngDefenseModeStart(entk);
			break ;
#if 0
			case ENE_TH1_DEFENSE :
//				ENE_ResetRevMotion(entk->act);
				Sig_DefenseModeStart( entk ) ;
			break ;
#endif
		}
	}
}

static	void	DamageModeCheck( entk )
ENETHINK	*entk ;
{

	TNG_COM		*tng_com ;
	ENTK_TENG_A *eve_a ;

	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

//if(entk->id == 1) printf("DMG MODE CHECK!!  %d\n",entk->think_mode ) ;

/*2000.01.12 */
/*****************/
	/*ダメージ後の復帰処理*/
	/*死んでないので思考セット後 再始動*/
	if ( entk->notice & ENE_NOTICE_DAMAGE ) {
		switch( GM_AlertMode ) {
			case ALERT_MODE_SNEAK :
			case ALERT_MODE_SEARCH :
/*ダメージ後 巡回続行*/
				ENE_WatcherStartModeSneak( entk ) ;
			break ;
			case ALERT_MODE_ALERT :
				eve_a->status &= ~EVE_TNG_HANG ;
				if(
				((entk->act->bodyp.type & ENE_TYPE_KATANA)&&(eve_a->def_level <= tng_com->now_level))
				||(eve_a->status & EVE_TNG_NO_DEF )
				){
					TngAlertModeStart( entk ) ;
				}else {
					Sig_TngDefenseModeStart(entk);
				}
			break ;
		}
		if ( ENE_DamageCheck( entk ) ) {
			ENE_EnemyStartModeDamage( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_RES ) {
printf("DEATH !!!!!!!!!\n");
//		eve_a->status |= EVE_A_ATTAK_END ;
		/*死んでた*/
		/*目撃者から除外*/
//		eve_a->status &= ~EVE_A_OBSERVE ;
//		eve_a->status |= EVE_A_DEATH ;
//		if(eve_a->type==0){
//			/*2001.01.15 死亡後は新POS*/
//			DefPosFree(entk);
//		}
		/*復活*/
		TNG_DefenderResurrect( entk ) ;
		ResetHomingStatus(entk,HOMING_TNG_FAINT) ;
		TngKatanaOff(entk);
	}
}


static void	TNG_DefenderResurrect( entk )
ENETHINK	*entk ;
{
	FVECTOR	pos ;
	int		hzx_id ;
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;
	BODYPARAM	*bodyp ;
	bodyp = &entk->act->bodyp ;

	UNSET_FLAG( bodyp->deftrg.class, TARGET_LOCKON ) ;

	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

	entk->act->dir = -1 ;

	if(tng_com->max_res_num >= 0){
		if(tng_com->max_res_num <= tng_com->res_num){
			return ;
		}
	}
	tng_com->res_num++ ;


	ENE_RefreshParam( entk ) ;
#if 0
	TngGetDefPos4(entk);
#else
/*守備座標を巡回ルートに変更*/
	TngReadNode(entk) ;
#endif
printf("respos_num %d\n",eve_a->res_pos);
	TNG_COM_GetResPos( tng_com,&pos, &hzx_id ,eve_a->res_pos ) ;
printf("TENG RES POS %x \n",hzx_id);
	HZX_FlashTrap( entk->ctrl->hzx_id, &entk->ctrl->evt ) ;
	GM_ResetControlPositionAndGroup( entk->ctrl, &pos, hzx_id ) ;/* 再発生場所へ移動 */
	eve_a->blur_sw= -60 ;

	Sig_TngDefenseModeStart(entk);
#if 0
	if(def_com->def_mode == AT_COM_DEF_WAIT ){
		entk->think1 = ENE_TH1_DEFENSE ; 
		entk->think2 = TH2_MOVE ; 
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
	}
#endif

	entk->count3 = 0 ;
}

static void RefreshParam( entk )
ENETHINK	*entk ;
{
	BODYPARAM	*bodyp ;

//	entk->uniq_id = entk->name_id.uniq_id = COM_GetUniqID( ) ;

	bodyp = &entk->act->bodyp ;

	bodyp->life = bodyp->m_life ;
	bodyp->faint = bodyp->m_faint ;
	bodyp->blood = bodyp->m_blood ;
	bodyp->anesthesia = bodyp->m_anesthesia ;

	bodyp->pbreak = 0 ;
	bodyp->loss_blood = 0 ;
	bodyp->faint_time = 0 ;
	bodyp->dam_level_num[0]=0 ;
	bodyp->dam_level_num[1]=0 ;
	bodyp->dam_level_num[2]=0 ;
	bodyp->dam_level_num[3]=0 ;
	bodyp->ane_level_num[0]=0 ;
	bodyp->ane_level_num[1]=0 ;
	bodyp->ane_level_num[2]=0 ;
	bodyp->ane_level_num[3]=0 ;

}

static void	TNG_DefenderResurrect2( entk )
ENETHINK	*entk ;
{
	FVECTOR	pos ;
	int		hzx_id ;
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;
	BODYPARAM	*bodyp ;
	bodyp = &entk->act->bodyp ;
	entk->act->dir = -1 ;

	UNSET_FLAG( bodyp->deftrg.class, TARGET_LOCKON ) ;

	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

	RefreshParam( entk ) ;
	TngReadNode(entk) ;
//	UNSET_FLAG(entk->status, ENE_STATUS_EVER_ZZZ) ;
	TNG_COM_GetResPos( tng_com,&pos, &hzx_id ,eve_a->res_pos ) ;
//	pos.vy += 1000.0F ;
	HZX_FlashTrap( entk->ctrl->hzx_id, &entk->ctrl->evt ) ;
	GM_ResetControlPositionAndGroup( entk->ctrl, &pos, hzx_id ) ;/* 再発生場所へ移動 */
	eve_a->blur_sw= -60 ;

#if 1
	Sig_TngDefenseModeStart2(entk);
//	entk->count3 = 0 ;
#endif
}

/*落下兵救助用*/
static void	TNG_DefenderResurrect3( entk )
ENETHINK	*entk ;
{
	FVECTOR	pos ;
	int		hzx_id ;
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;
	BODYPARAM	*bodyp ;
	bodyp = &entk->act->bodyp ;

	UNSET_FLAG( bodyp->deftrg.class, TARGET_LOCKON ) ;
	entk->act->dir = -1 ;

	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

//	entk->act->bodyp.faint_time = 0  ;
	ENE_RefreshParam( entk ) ;
//	TngReadNode(entk) ;
//	UNSET_FLAG(entk->status, ENE_STATUS_EVER_ZZZ) ;
	TNG_COM_GetResPos( tng_com,&pos, &hzx_id ,eve_a->res_pos ) ;
//	pos.vy += 1000.0F ;
	HZX_FlashTrap( entk->ctrl->hzx_id, &entk->ctrl->evt ) ;
	GM_ResetControlPositionAndGroup( entk->ctrl, &pos, hzx_id ) ;/* 再発生場所へ移動 */
	eve_a->blur_sw= -60 ;
printf("ENE WARP!!!!!!!!\n");
#if 1
	Sig_TngDefenseModeStart2(entk);
//	entk->count3 = 0 ;
#endif
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
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;
	int test= 0;
	BODYPARAM	*bodyp ;

	entk = &work->enethink ;
	bodyp = &entk->act->bodyp ;
	eve_a = (ENTK_TENG_A*)entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;


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

	switch( entk->think1 ){
		case ENE_TH1_DAMAGE :
			/*ダメージ中及び死亡から再発生まで*/
			ENE_Enemy_Think1_Damage( entk ) ;
			DamageModeCheck( entk );
			if( entk->act->bodyp.life <=0){ 
#if 0
				TngKatanaOff(entk);
#else
				TngKatanaBlurOff(entk);
//				TngKatanaFall(entk) ;
#endif
				SetTngKillFlag() ;
			}
			break ;
		case ENE_TH1_RESURRECT :
			ENE_AttackerResurrect( entk ) ;
			ResetHomingStatus(entk,HOMING_TNG_FAINT) ;
			RessModeCheck( work ) ;
			break ;
		case ENE_TH1_DEFENSE : /*銃撃戦モード*/

			if(GM_GameStatus & STATE_VR_ONLY){
				HZX_ChangeRouteCourse( HZX_ROOT_COURSE2 ) ;
			}
/*
強行突破された時のワープ処理
廃止かも、、
*/
			if(
			(eve_a->type ==1)
			&&(eve_a->def_level < tng_com->now_level)
			&&((entk->ctrl->mov.vz) > (GM_PlayerPosition.vz+3000.0f))
			){
				test = SIG_BoundCheck(&entk->znavi->flore_pos,1000.0F,2000.0F) ;
				if(test  < 0){
/*ワープ*/
//					TngKatanaOff(entk);
				TngKatanaBlurOff(entk);
					TNG_DefenderResurrect( entk ) ;
					ResetHomingStatus(entk,HOMING_TNG_FAINT) ;
					entk->act->dir = -1 ;
					break ;
				}
			}
#if 1
			/*ｗ４５以外では行動許可待ちで待機*/
			if(eve_a->type < 2){
				if(eve_a->def_level <= tng_com->now_level){
					/*行動可能*/
					TNG_UnSetUnrealStatus(entk) ;
				}else {
					/*待機*/
					TNG_SetUnrealStatus(entk) ;
					break ;
				}
			}
			eve_a->status |= EVE_TNG_OBSERVE;
#endif
			Think1_TngDefense( entk ) ;
			ThinkModeCheck( entk ) ;

			HZX_ClearRouteCourse( 0 ) ;
			break ;
		case ENE_TH1_ALERT : /*危険*/
			/*遊撃モード*/
			if(GM_GameStatus & STATE_VR_ONLY){
				HZX_ChangeRouteCourse( HZX_ROOT_COURSE2 ) ;
			}

			Tng_Think1_Alert( entk ) ;

			if(
			(eve_a->type ==1)
			&&(eve_a->def_level < tng_com->now_level)
//			&&( eve_a->def_level< tng_com->max_level)
			){
				test = SIG_BoundCheck(&entk->znavi->flore_pos,1000.0F,2000.0F) ;
				if(test  < 0){
#if 1
/*ワープ*/
//					TngKatanaOff(entk);
					TngKatanaBlurOff(entk);
					TNG_DefenderResurrect( entk ) ;
					ResetHomingStatus(entk,HOMING_TNG_FAINT) ;
					entk->act->dir = -1 ;
#endif
					break ;
				}
			}

			ThinkModeCheck( entk ) ;

			HZX_ClearRouteCourse( 0 ) ;

			break ;
		case ENE_TH1_AVOID : /*回避*/
			if(GM_GameStatus & STATE_VR_ONLY){
				HZX_ChangeRouteCourse( HZX_ROOT_COURSE2 ) ;
			}

#if 1
			if(eve_a->def_level <= tng_com->now_level){
				/*行動可能*/
				TNG_UnSetUnrealStatus(entk) ;
			}else {
				/*待機*/
				TNG_SetUnrealStatus(entk) ;
				break ;
			}
#endif
			Think1_TngDefense( entk ) ;
			ThinkModeCheck( entk ) ;
			entk->alert  = 0 ;
			HZX_ClearRouteCourse( 0 ) ;
			break ;
		case ENE_TH1_SNEAK : /*通常*/
			CLEAR_FLAG( entk->iknow_flag ) ;
			entk->alert  = 0 ;
//			ENE_Watcher_Think1_Sneak( entk ) ;
			ThinkModeCheck( entk ) ;
			break ;
	}

}


/*--------------------------------------------------------------------*/
static void Action ( work )
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
/******************************/
	TNG_RetireCheck( work ) ;
/******************************/
	ENE_ActStatusCheck( entk ) ;
	ENE_Gravitation( entk ) ;

}

#define DEF_FAINT_TIME (120)
static void TNG_RetireCheck( Work *work )
{
	ENETHINK *entk;
	AT_THK *at_thk ;
	ENTK_TENG_A *tng_a ;

	TNG_COM		*tng_com ;

	entk = &work->enethink;
	at_thk = (AT_THK *) entk->character ;
	tng_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;


	/*保険判定 ダメージ状態から復帰していない*/
	if(
	(entk->think1 == ENE_TH1_DAMAGE)
	&&(entk->think2 == 0)
	&&(entk->think3 == 0)
	&&(!(entk->act->status_status & ACT_STST_HANG))
	&&(entk->count3 > DIRECT_TICK(60*60))
	&&( entk->act->bodyp.life >0)
	&&(	entk->act->bodyp.faint_time < DEF_FAINT_TIME )
	){
printf("EVE_TNG_DAM_WAIT\n");
		tng_a->status |= EVE_TNG_DAM_WAIT ;
	}else {
		tng_a->status &= ~EVE_TNG_DAM_WAIT ;
	}

	if(tng_a->type == 1){
		if(tng_a->def_level <= tng_com->now_level){
		/*行動可能*/
//			entk->act->status &= ~ACT_STATUS_TARGET_SKIP ;
			SET_FLAG( entk->act->bodyp.deftrg.class, TARGET_LOCKON ) ;
			SIG_AllObjectVisible(&work->body) ;
		}else {
			/*待機*/
//printf("TAIKI!!!!!!![%d]\n",tng_a->def_level);
			tng_a->blur_sw = -60 ;
			entk->act->status |= ACT_STATUS_TARGET_SKIP ;
			UNSET_FLAG( entk->act->bodyp.deftrg.class, TARGET_LOCKON ) ;
			SIG_AllObjectInvisible(&work->body) ;
			/*VRでは見えないとこに待機してるので念のため現状維持*/
			if(!(GM_GameStatus & STATE_VR_ONLY)){
				entk->act->status |= ACT_STATUS_INVISIBLE ;
			}
			return ;
		}
	}


	if(( entk->act->bodyp.life >0)&&(entk->act->bodyp.faint_time < DEF_FAINT_TIME )){
		tng_a->status |= EVE_TNG_ACTIVE;
	}
	if( entk->status_status & (ENE_STST_FAINT_EXIT|ENE_STST_HOLD_EXIT) ) {
/*ザコサバイバル用 気絶退出処理をenemy共通部におまかせ*/
		tng_a->status &= ~EVE_TNG_RETIRE;
		tng_a->status &= ~EVE_TNG_RES_REQ ;
	}else if(( entk->act->bodyp.life >0)&&(entk->act->bodyp.faint_time > DEF_FAINT_TIME )){
/*生きてるけど行動不能フラグ*/
		if(!(tng_a->status & EVE_TNG_RETIRE)){
			tng_com->destroy++;
			if(entk->com->max_res_in_alert > entk->com->res_count_in_alert){
				tng_a->status |= EVE_TNG_RES_REQ ;
				entk->com->res_count_in_alert++ ;
			}
		}
		tng_a->status |= EVE_TNG_RETIRE;
		TngKatanaBlurOff(entk);
		entk->hom.status |= HOMING_TNG_FAINT ;
		entk->act->status |= ACT_STATUS_TARGET_SKIP ;
//		if(entk->com->max_res_in_alert <= entk->com->res_count_in_alert){
		if(!(tng_a->status & EVE_TNG_RES_REQ)){
			tng_com->retire_num++;
		}
		tng_a->blur_sw = -60 ;
	}else {
		tng_a->status &= ~EVE_TNG_RETIRE;
		tng_a->status &= ~EVE_TNG_RES_REQ ;
	}

	if(
	(tng_a->status & EVE_TNG_RES_REQ)
	&&(tng_a->type >= 1)	//ｗ44とw45
	&&(entk->count3 > DIRECT_TICK(RETIRE_NO_CHECK_TIME))
	){
		int test = 0 ;

		if(
		(entk->count3 > DIRECT_TICK(RETIRE_CHECK_TIME_ABS-RETIRE_CHECK_HALH))
		&&(entk->count3 <= DIRECT_TICK(RETIRE_CHECK_TIME_ABS))){
			/*点滅*/
			if(entk->count3 & 1){
				SIG_AllObjectInvisible(&work->body) ;
//				TNG_UnSetUnrealStatus(entk) ;
			}else {
				SIG_AllObjectVisible(&work->body) ;
//				TNG_SetUnrealStatus(entk) ;
			}
		}
#if 0
		if(entk->count3 > DIRECT_TICK(RETIRE_CHECK_TIME_ABS)){
			test = -1 ;
		}else {
			test = SIG_BoundCheck(&entk->znavi->flore_pos,1000.0F,2000.0F) ;
		}
#else
		if(entk->count3 > DIRECT_TICK(RETIRE_CHECK_TIME_ABS)){
			test = -1 ;
		}
#endif
		if(
		(test  < 0)
		){
			/*再発生*/
			/*刀消し*/
//			TngKatanaOff(entk);
			TngKatanaBlurOff(entk);
			TNG_DefenderResurrect2( entk ) ;
			tng_a->blur_sw = -60 ;
			ResetHomingStatus(entk,HOMING_TNG_FAINT) ;
			SIG_AllObjectVisible(&work->body) ;
		}
	}else {
		/*無敵で寝たまま放置*/
//暫定　ありえない
//		SIG_AllObjectVisible(&work->body) ;
	}
}


static void DefenderMain( Work *work )
{
	ENETHINK *entk;
	AT_THK *at_thk ;
	ENTK_TENG_A *tng_a ;

	TNG_COM		*tng_com ;

	entk = &work->enethink;
	at_thk = (AT_THK *) entk->character ;
	tng_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

	PreProcess( work ) ;
	Think( work ) ;

	Action( work ) ;

}



static void Act( Work *work )
{

	ENETHINK *entk;
	AT_THK *at_thk ;
	ENTK_TENG_A *tng_a ;

	TNG_COM		*tng_com ;

	entk = &work->enethink;
	at_thk = (AT_THK *) entk->character ;
	tng_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

	tng_a->status &= ~EVE_TNG_ACTIVE ;

	if(entk->com->max_res_in_alert <= entk->com->res_count_in_alert){
//	if(1){
		entk->act->bodyp.type |= ENE_TYPE_NO_ROTTEN ;
	}




	if(tng_com->mode & TNG_COM_MULTITEX_AUTO){
		if(CheckTexMode()){
			/*ON*/
			SIG_MultiTexOn(&work->body);
		}else {
			/*OFF*/
			SIG_MultiTexOff(&work->body);
		}
	}
	work->hex_info.flags &= ~HEX_FLAG_UNDRAW ;


#if 0
	{
		extern inline void SIG_NumPrint(FVECTOR * ,int) ;
		SIG_NumPrint(&entk->ctrl->mov ,entk->rnavi->pa_action[(int)entk->rnavi->next_node]) ;
	}

	if(entk->iknow_flag & IKNOW_ZZZ){
		printf("I'm sleeping ! %d\n",entk->id);
	}
	if(
	( entk->act->bodyp.dammode == DAM_MODE_MASUI_DOWN )
	&&( entk->status & ENE_STATUS_EVER_ZZZ )
	){
//		printf("I'm sleeping ! %d\n",entk->id);
	}

#endif




	ENE_GM_Act( entk ) ;

	if ( GM_VRStatus & (GM_VR_IDLE) ) return ;
	if ( GM_VRStatus & GM_VR_CLEAR ) {
		ENE_ActStatusCheck ( &work->enethink ) ;
		work->action.old_status = work->action.status ;
		if(tng_a->blur_sw > 0){
			tng_a->blur_sw = 0;
		}
		work->hex_info.flags |= HEX_FLAG_UNDRAW ;
		return ;
	}

	/*EYE_INFOセット*/
	ENE_PreProcess( entk ) ;


	DefenderMain( work ) ;

	ENE_AftProcess( entk ) ;
	if(( entk->act->bodyp.life <=0)
	&&(at_thk->life_buf >0)
	){ 
		Tng_DeathProc(entk);
	}

	at_thk->life_buf = entk->act->bodyp.life ;
	tng_a->katana &= ~TNG_KATANA_BULLET_HIT ;


//	bodyp = &entk->act->bodyp ;
//	UNSET_FLAG( bodyp->deftrg.class, TARGET_LOCKON ) ;
//	if(entk->act->bodyp.deftrg.class & TARGET_LOCKON ) {
//printf("LOCKON MODE!! [%d]\n",entk->id);
//	}else {
//	}

	if(tng_com->res_trap){
		if(SIG_CheckTrap( entk->ctrl ,tng_com->res_trap )){
			if(
				(entk->think1 != ENE_TH1_DAMAGE)
				&&(entk->act->bodyp.life>0)
			){
				/*ステージから零れてる*/
printf("FALL TNG RESCUE!!!!\n");
				TNG_DefenderResurrect3(entk);
			}
		} 
	}
#if 1
/*なぞの落下兵救助 保険措置*/
	if(entk->ctrl->mov.vy < (tng_com->base_floor - 55000.0f)){
printf("FALL TNG !!!!LIFE [%d]\n",entk->act->bodyp.life);
		printf("max[%d]res[%d]destroy[%d]retire[%d]\n",
			entk->com->max_res_in_alert,
			entk->com->res_count_in_alert,
			tng_com->destroy,
			tng_com->retire_num
		) ;
		TNG_DefenderResurrect3(entk);
	}
#endif

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
//		entk->alert = ALERT_LEVEL_MAX ;
		COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
	}

	/*付随エフェクト制御*/
	if(( entk->act->bodyp.life <=0)
	||(work->body.objs->flag & DG_FLAG_INVISIBLE)
	){ 
		if(tng_a->blur_sw > 0){
			tng_a->blur_sw	= 0;
		}
	}else {
		if(tng_a->blur_sw < 1){
			tng_a->blur_sw++;
		}
	}
/***************debug**********************/
#if 0
	if(entk->hom.status & HOMING_TNG_FAINT){
		printf("HOMING_TNG_FAINT SET!!!!!!!\n");
	}
	if(entk->hom.status & HOMING_SKIP){
		printf("HOMING SKIP ID[%d]!!!! \n",entk->id);
	}
/*******************************************/
#endif
TNG_CheckTrgStatus(entk);

	if ( entk->act->ctrl->skip_flag & CTRL_SKIP_FLR_CHECK ) {
		work->hex_info.flags |= HEX_FLAG_UNDRAW ;
	}

#ifdef DEBUG_MODE
	if(tng_com->mode & TNG_COM_DEBUG){
extern void SIG_NumPrint(FVECTOR * ,int) ;

//		SIG_NumPrint(&entk->ctrl->mov ,entk->act->body->m_ctrl->mt3_ctrl->motion_num) ;
//		SIG_NumPrint(&entk->ctrl->mov ,tng_a->guard_n) ;
//		SIG_NumPrint(&entk->ctrl->mov ,entk->count3) ;
		if(tng_a->status & EVE_TNG_ACTIVE){
			SIG_NumPrint(&entk->ctrl->mov ,1) ;
		}else {
			SIG_NumPrint(&entk->ctrl->mov ,0) ;
		}
	}
#endif


}

static void Die( Work *work )
{
printf("DEFENDER DIE!!\n");
	ENE_FreeResources( &(work->enethink) ) ;
	HEX_RemoveControlInfo(&work->hex_info) ;

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
	OBJECT		*body, *weapon ;
	int		root, node, bodyname, wpname ,sub_wpname ,life, faint, value,equip;
//	FVECTOR	d_pos;
	char	*d_pos_opt, *opt ;
	AT_THK *at_thk ;

	equip = GCL_GetOptionValue( 'e', AT_EQUIP_NORMAL );

	entk = &work->enethink ;


	entk->character = &work->at_thk ;
	at_thk = &work->at_thk ;
	ctrl = &work->control ;
	act = &work->action ;
	body = &work->body ;
	weapon = &work->weapon ;
//	sub_weapon = &work->sub_weapon ;

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
	if(equip == AT_EQUIP_HITECH_1){
		type = ENE_TYPE_HITECH ;
printf("AT_EQUIP_HITECH_1\n");
//		act->bodyp.type |= ENE_TYPE_HITECH;
	}

//	bodyname = GV_StrCode( "tng_def" ) ;

	switch (type){
		case ENE_TYPE_ATTACKER :
			bodyname = ENE_MDL_NAME_GBA ;
			break ;
		case ENE_TYPE_EVENT_A :
			bodyname = ENE_MDL_NAME_GBS ;
			break ;
		case 	ENE_TYPE_TNG_A :
			bodyname = ENE_MDL_NAME_TNG ;
			break ;
		case	ENE_TYPE_HITECH :
			bodyname = ENE_MDL_NAME_HTC ;
printf("bodyname = ENE_MDL_NAME_HTC\n");
			break ;
		default :
			bodyname = ENE_MDL_NAME_GBA ;
			break ;
	}
	/*装備品*/
	/*こっちは兵士の種類を決定*/
//	at_thk->equip_req = GCL_GetOptionValue( 'e', AT_EQUIP_NORMAL );
	at_thk->equip_req = equip ;

	/*ショットガン装備兵*/
	if(at_thk->equip_req == AT_EQUIP_SHOTGUN){
		wpname = E_WP_SPS ;
		act->bodyp.type |= ENE_TYPE_SHOTGUN ;
	}else if(at_thk->equip_req == AT_EQUIP_ABAKAN){
		wpname = E_WP_ABAKAN ;
		act->bodyp.type |= ENE_TYPE_ABAKAN ;
	}else if(type & ENE_TYPE_TNG_A){
		wpname = E_WP_P90 ;
	}else{
printf("wpname = E_WP_AKS\n");
		wpname = E_WP_AKS ;
	}

//	sub_wpname = E_WP_MKR ;
	sub_wpname = 0 ;


	ENE_SetSenseParam( &(entk->sense), 0, DEF_EYE_RANGE,
	entk->sense.eye_s_s[0], DEF_HEARING, DEF_SMELL ) ;
/*ここを変えれば目標変更可能*/
	ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerFindPos, 
	&GM_PlayerAddress, NULL, &GM_PlayerMap ) ;
	ENE_InitControl( entk,ctrl, name ) ;

	/* モーション名セット */
	ENE_SetMotionName( entk, ENE_MOT_BASE, ENE_MOT_STAGE ) ;

	SET_FLAG(entk->status, ENE_STATUS_NO_FINGER) ;
	SET_FLAG(entk->status, ENE_STATUS_NO_EYEANIM);
	SET_FLAG(entk->status, ENE_STATUS_NO_VANIM)  ;
	SET_FLAG(entk->status, ENE_STATUS_EVER_ZZZ) ;
	ENE_InitObject( entk, body, bodyname, ctrl, work->lights ) ;
	/*マルチテクスチャ解除*/
//		SIG_MultiTexOff(body);

	/* 防御ターゲットの初期化(装備より先にする)*/
	ENE_InitTarget( entk, ctrl, body, act ) ;
    GM_SetTargetCallBack( &act->bodyp.deftrg, DefTargCallBack, work ) ;
	
	ENE_InitWeapon( entk, body, weapon, wpname, NULL ) ;
	ENE_InitSubWeapon( entk, body, NULL, sub_wpname ) ;

	act->bodyp.type |= ENE_TYPE_TNG_A ;
	act->bodyp.type = (ENE_TYPE_ATTACKER|ENE_TYPE_TNG_A);

//	ENE_InitAction( entk, act, ctrl, body, life, faint, DEF_BLOOD ,mt_array) ;
	ENE_InitAction( entk, act, ctrl, body, life, faint, DEF_BLOOD ,NULL) ;

	/* 耐久力セット */
	AT_SetDurable( &act->bodyp, 0, 5, 3, 0 ) ;

//	at_thk->life_buf = entk->act->bodyp.life ;
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
	/*守備位置からの待避位置*/
	d_pos_opt = GCL_GetOption( 'o' ) ;
	if ( d_pos_opt  != NULL ){
		ENE_GCL_GetFV( d_pos_opt, &at_thk->def_pos2 ) ;
		at_thk->at_status |= AT_ST_DODGE ;
	}
	entk->search_route = GCL_GetOptionValue( 'c', root ) ;


	/*盾壊れた時連絡*/
	/*装備系 設定初期化*/
	entk->sw.shield = 0;
	act->sw->sub_weapon = 0;
	/*装備系設定後 装弾数セット*/
	if(entk->act->sw->sub_weapon == 1){
		/*さぶを使用*/
		entk->max_bullet = ENE_GetAmmoMax(entk->name_id.sub_weapon) ;
	}else {
		/*めいんを使用*/
		entk->max_bullet = ENE_GetAmmoMax(entk->name_id.weapon) ;
	}


	at_thk->found_addr = HZX_NO_ZONE ;


#ifdef DEBUG_PRIM
	InitPrim( work ,0,0) ;
#endif
	return (0);
}
static int GetResources( Work *work, int name )
{

	ENETHINK	*entk ;
	ENTK_TENG_A *tng_a ;
	BODYPARAM	*bodyp ;
	ACTION		*act ;
	char	*opt ;

	entk = &work->enethink ;
	tng_a = entk->eve_a = &work->tng_a ;
	act = &work->action ;

printf("ATTACKER COMSET Id NUMBER!!!!!!!!!!!!!!\n") ;
	COM_SetIDNumber( entk ) ;

	if ( (entk->status_status = GCL_GetOptionValue( 'k', -1 )) < 0  ) {
		entk->status_status = 0 ;
	}
//	AT_GetResources(work ,name ,ENE_TYPE_TNG_A,NULL) ;

	act->bodyp.type = (ENE_TYPE_ATTACKER|ENE_TYPE_TNG_A);
	SIG_ENE_GetResources(work ,name ,ENE_TYPE_TNG_A,NULL) ;
	if(act->bodyp.type & ENE_TYPE_HOLD_EXIT){
		act->bodyp.type |= ENE_TYPE_CONVERT ;
	}
/*手足ダメージでライフ減る*/
	act->bodyp.type |= ENE_TYPE_DMG_HAND_LEG ;
/*部位ダメージ無し*/
	SET_FLAG(act->bodyp.type,ENE_TYPE_NO_PBREAK);

	ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerFindPos, 
	&GM_PlayerAddress, NULL, &GM_PlayerMap ) ;

	/*忍者は首絞めるのでつかみターゲットを確保*/
	act->cap_attack = &work->capture ;

	tng_a->status = 0;
	tng_a->katana = 0;
	tng_a->guard_n = 0 ;
	/*守備兵に必須の非通報フラグ*/
	entk->status |= (ENE_STATUS_LONELY_FIGHT|ENE_STATUS_NO_STEALTH) ;

	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		entk->death_proc.argc = 
		ENE_GclGetProc( &(entk->death_proc.proc),
		&entk->death_proc.argv[0] ) ;
		if ( entk->death_proc.argc > (GCL_MAX_ARGS/2) ) return -1 ;
	} else {
		entk->death_proc.proc = 0 ;
	}
	/*視力上書き*/
#if 0
no_def
	entk->sense.eye_s_s[ALERT_MODE_SNEAK] = 180000 ;
	entk->sense.eye_s_s[ALERT_MODE_ALERT] = 180000 ;
	entk->sense.eye_s_s[ALERT_MODE_AVOID] = 180000 ;
	entk->sense.eye_s_s[ALERT_MODE_SEARCH] = 180000 ;
#endif
	/* 耐久力セット */
//	AT_SetDurable( &act->bodyp, 0, 3, 3, 0 ) ;
	/*弱め*/
	AT_SetDurable( &act->bodyp, 0, 0, 0, 0 ) ;
	entk->act->bgm_track = 0 ;
	tng_a->type = GCL_GetOptionValue( 't', 0 ) ;
	entk->def_mapbit = GM_CurrentMap ;
	entk->def_pos = DG_ZeroVector ;
	/*起動時のモードセレクト*/
	entk->think1 = ENE_TH1_SNEAK ;

	ENE_WatcherStartModeSneak( entk ) ;


	GV_SetActorChild( work, NewTngKatana( &work->body,&tng_a->katana ,entk)  );
//	GV_SetActorChild( work,NewEyeAnimTypeSetDemo(&work->body,work->name,(entk->id)%6));
	/*暫定使用 タイプで装備固定*/
	if(tng_a->type == TNG_TYPE_D){
		TngKatanaOn(entk);
	}
	HEX_RegistControlInfo(&work->hex_info, &work->control) ;

//	NewTargetView2( &(act->offense), 32, 232, 186 ) ;

	bodyp = &entk->act->bodyp ;
	UNSET_FLAG( bodyp->deftrg.class, TARGET_LOCKON ) ;


#if 1
	/*ぼんぼり*/
	if(act->bodyp.type & ENE_TYPE_TNG_A){
		FVECTOR shift ={-100.0f, 88.0f,105.0f,0.0f};
		CVECTOR col ;
		col.r = 80 ;
		col.g = 30 ;
		col.b = 0 ;
		col.cd = 127 ;
		tng_a->blur_sw = 0;
		GV_SetActorChild( work , (void *)NewSIG_BlurPoint(&work->body.objs->objs[HUMAN21_ATAMA].world,
			&shift,&col ,&tng_a->blur_sw)) ;
	}
#endif
	/* 武器位置最後に初期位置へリセット */
	ENE_ResetWeaponPosition( entk ) ;

	return (0);
}

void *NewSigTengB( int name, int where )
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


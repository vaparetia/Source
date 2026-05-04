//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	tng_a.c
	巡回警備モード付き天狗　ｗ41　ｗ42用
	2000/03/01 K.Sigeno
	$Id: tng_a.c,v 1.1.1.3 2002/11/19 11:49:52 Yoshizawa1 Exp $
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

#include	"korekado/enemy/motion.h"

//#include	"tng_motion.h"
//#include	"tng_array.h"

#include	"../attacker/at_thk.h"
#include	"tng.h"
#include	"tng_a.h"
#include	"tngthink.x"
#include	"../attacker/sigavoid.x"
#include	"../attacker/sig_conv.x"

//#define DEF_POS_TEST
extern void AT_SetDurable( BODYPARAM *,int,int,int,int);

extern void ENE_NoticeCheck( ENETHINK * );
extern void TngAlertModeStart( ENETHINK * ) ;
extern void ENE_EnemyStartModeDamage( ENETHINK * ) ;
extern void Tng_Think1_Alert( ENETHINK * );
extern void ENE_Gravitation( ENETHINK * );
extern void ENE_Watcher_Think1_Sneak( ENETHINK * );
extern void ENE_Enemy_Think1_Damage( ENETHINK * );
extern void ENE_AttackerResurrect( ENETHINK * );
extern void ENE_AttackerResurrectionMode( ENETHINK * );

extern void AT_AnesThink(ENETHINK *) ;


extern void *NewSIG_BlurPoint(FMATRIX *world,FVECTOR *shift,CVECTOR *col ,int *sw);


#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern void SigZoneView( int , SVECTOR * ,float ) ; 
#endif


/*マルチテクスチャの有効条件チェック*/
static int CheckTexMode(void){
	if(GM_PlayerStatus & PLAYER_WATCH){
		return 1 ;
	}
	return 0 ;
}

#if 1
static void	TNG_CheckMessage( entk )
ENETHINK	*entk ;
{
    GV_MSG	*msg ;
	int n_msg, code ;
	FVECTOR		pos;
//	CONTROL	ctrl ;
	AT_THK *at_thk ;
	ENTK_TENG_A *tng_a ;

	tng_a = (ENTK_TENG_A *) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;
	
    n_msg = entk->ctrl->n_msg ;
	msg = entk->ctrl->msg ;



	if  ( n_msg > 0 ) {
		printf("message get id=[%d]\n",entk->id ) ;
	}

//	if(entk->act->bodyp.type & ENE_TYPE_TNG_A){
	if(1){
		while ( n_msg-- > 0 ) {
			code = msg->message[ 0 ] ;
			switch( code ) {
				case TNG_A_MSG_DEFENSE :
//					entk->think_mode = ENE_TH1_DEFENSE ;
					entk->think1 = ENE_TH1_DEFENSE ;
					break;
				case TNG_A_MSG_ALERT :
					entk->think1 = ENE_TH1_ALERT ;
					break;
				case TNG_A_MSG_WARP :
#if 1
printf("TNG_A_MSG_WARP  get id=[%d]\n",entk->id ) ;
					pos.vx = (float)msg->message[ 1 ] ;
					pos.vy = (float)msg->message[ 2 ]+1000 ;
					pos.vz = (float)msg->message[ 3 ] ;
	HZX_FlashTrap( entk->ctrl->hzx_id, &entk->ctrl->evt ) ;
					GM_ResetControlPositionAndGroup( entk->ctrl, &pos,GM_GetMapID( msg->message[ 4 ] ) );
					tng_a->blur_sw =  -60 ;
/*ワープ後自動的に追跡モード*/
//					entk->think_mode = ENE_TH1_ALERT ;
//					entk->think1 = ENE_TH1_ALERT ;
#endif
					break;
				case TNG_A_MSG_ROOTCHANGE :
printf("TNG_A_MSG_ROOTCHANGE  get id=[%d]\n",entk->id ) ;
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
				break ;
				/*ビデオ用モード切り替え*/
				case TNG_A_MSG_MODE :
					break;
				case TNG_A_MSG_RESET :
					/**/
					at_thk->at_tmptime = 0;
					entk->count3 = 0 ;
					break;
#if 0
				case TNG_A_MSG_POSSET :
					eve_a->def_pos_req = (int) msg->message[ 1 ] ;
					break;
#endif
			}
			msg++ ;
		}
	}
}
#endif

static void PreProcess( Work *work )
{
	ENETHINK	*entk ;

	entk = &work->enethink ;

	TNG_CheckMessage( entk ) ;

	/* スーパーアンリアルでも個別メッセージだけはうけとっておく */
	if ( work->enethink.status & ENE_STATUS_EVER_UNREAL ) return ;

	ENE_EyeInfoCheck( entk, &entk->pl_eyei ) ;
	ENE_NoticeCheck( entk ) ;
	ENE_SetAlertLevel( entk ) ;

}















/****************************************************/

#if 0
static	void	SneakModeCheck( entk )
ENETHINK	*entk ;
{
	switch( GM_AlertMode ) {
		case ALERT_MODE_ALERT :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			ASSERT(entk->at_com != NULL) ;
			if(entk->at_com->wait_cnt == 0)
			{
//				AlertModeStart(entk);
				TngAlertModeStart( entk) ;
//				AT_AlertWarp(entk);
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
#endif
/*----- 高レベルモード移行チェック --------------------------------------------------*/

#if 0
static	void	SearchModeCheck( entk )
ENETHINK	*entk ;
{
	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			ENE_AttackerStartModeSearchToSneak( entk ) ;
		break ;
		case ALERT_MODE_ALERT :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
//			AlertModeStart(entk);
			TngAlertModeStart( entk) ;

//			AT_AlertWarp(entk);
		break ;
		case ALERT_MODE_AVOID :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			if(entk->at_com->avoid_wait_cnt == 0){
				Check_AttackerStartModeAvoid( entk ) ;
			}
		break ;
	}

	if ( ENE_DamageCheck( entk ) ) {
		if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
		ENE_EnemyStartModeDamage( entk ) ;
	}
}
#endif
static	void	AlertModeCheck( entk )
ENETHINK	*entk ;
{
	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
			if(entk->status & ENE_STATUS_CONVERT){
				ENE_WatcherStartModeSneak( entk ) ;
			}else {
				ENE_AttackerStartModeSneak( entk ) ;
			}
		break ;
		case ALERT_MODE_AVOID :
			if(entk->at_com->avoid_wait_cnt == 0){
				Check_AttackerStartModeAvoid( entk ) ;
			}
		break ;
		case ALERT_MODE_SEARCH :
			if(entk->status & ENE_STATUS_CONVERT){
				ENE_WatcherStartModeSneak( entk ) ;
				entk->think1 = ENE_TH1_SEARCH ; 
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
		}
	}


	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
//		entk->alert = ALERT_LEVEL_MAX ;
		COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
	}
}



static	void	AvoidModeCheck( entk )
ENETHINK	*entk ;
{
	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			if(entk->status & ENE_STATUS_CONVERT){
				ENE_WatcherStartModeSneak( entk ) ;
			}else {
				ENE_AttackerStartModeSneak( entk ) ;
			}
		break ;
		case ALERT_MODE_ALERT :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
//			AlertModeStart(entk);
			TngAlertModeStart( entk) ;
//			AT_AlertWarp(entk);
		break ;
		case ALERT_MODE_SEARCH :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;

			if(entk->status & ENE_STATUS_CONVERT){
				ENE_WatcherStartModeSneak( entk ) ;
				entk->think1 = ENE_TH1_SEARCH ; 
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

	/*ダメージ後の復帰処理*/
	/*死んでないので思考セット後 再始動*/
	if ( entk->notice & ENE_NOTICE_DAMAGE ) {
		entk->sw_gun &= ~SW_FLAG_SWITCH2 ;

		switch( GM_AlertMode ) {
			case ALERT_MODE_SNEAK :
				if(entk->status & ENE_STATUS_CONVERT){
					ENE_WatcherStartModeSneakDamage( entk ) ;
				}else {
					ENE_AttackerStartModeSneakDamage( entk ) ;
				}
			break ;
			case ALERT_MODE_ALERT :
//				AlertModeStart(entk);
				TngAlertModeStart( entk) ;
				/*ダメージ復帰*/
//				AlertModeStartDamage(entk);
				TngAlertModeStartDamage(entk) ;
			break ;
			case ALERT_MODE_AVOID :
				if(entk->at_com->avoid_wait_cnt == 0){
					Check_AttackerStartModeAvoidDamage( entk ) ;
				}
			break ;
			case ALERT_MODE_SEARCH :
				if(entk->status & ENE_STATUS_CONVERT){
					ENE_WatcherStartModeSneakDamage( entk ) ;
					entk->think1 = ENE_TH1_SEARCH ; 
				}else {
					ENE_AttackerStartModeSearchDamage( entk ) ;
				}
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
}

static	void	ConvertSneakModeCheack( entk )
ENETHINK	*entk ;
{
	switch( GM_AlertMode ) {
		case ALERT_MODE_ALERT :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
ASSERT(entk->at_com != NULL) ;
			if(entk->at_com->wait_cnt == 0)
			{
//				AlertModeStart(entk);
				TngAlertModeStart( entk) ;
//				AT_AlertWarp(entk);
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
#else
			/* 回避モードを警備兵の思考 */
			entk->think1 = ENE_TH1_SEARCH ; 
#endif
		break ;
	}
	if ( ENE_DamageCheck( entk ) ) {
		if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
		ENE_EnemyStartModeDamage( entk ) ;
	}
}

/****************************************************/



static	void	RessModeCheck( work )
Work *work ;
{
	ENETHINK *entk;

	entk = &work->enethink;

	if ( entk->notice & ENE_NOTICE_RES ) {
		switch( GM_AlertMode ) {
			case ALERT_MODE_SNEAK :
			case ALERT_MODE_SEARCH :
			case ALERT_MODE_AVOID :
//				ENE_ResetRevMotion(entk->act);
//				SIG_DefenderStartModeSneak( entk ) ;
				ENE_WatcherStartModeSneak( entk ) ;
			break ;
			case ALERT_MODE_ALERT :
//				ENE_ResetRevMotion(entk->act);
				TngAlertModeStart(entk);
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

#if 0
void	SIG_DefenderResurrect( entk )
ENETHINK	*entk ;
{
	FVECTOR	pos ;
	int		hzx_id ;
	ENTK_TYPE_A *eve_a ;
	TNG_COM		*tng_com ;

	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

	entk->act->dir = -1 ;
//	entk->act->pad = 0 ;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;

	ENE_RefreshParam( entk ) ;
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
	COM_GetResPos( &pos, &hzx_id ,tng_com->res_pos_num ) ;
//TNG_COM_GetResPos
#endif
	HZX_FlashTrap( entk->ctrl->hzx_id, &entk->ctrl->evt ) ;
	GM_ResetControlPositionAndGroup( entk->ctrl, &pos, hzx_id ) ;/* 再発生場所へ移動 */
	eve_a->blur_sw= -60 ;

	Sig_DefenseModeStart( entk ) ;
	/*2001.02.01 左舷突入モード時の再発生は別思考へ*/


	if(tng_com->def_mode == AT_COM_DEF_WAIT ){
		entk->think1 = ENE_TH1_DEFENSE ; 
		entk->think2 = TH2_MOVE ; 
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
	}
	eve_a->status &= ~EVE_A_NO_DEF ;
	if(tng_com->def_mode == AT_COM_DEF_DYNAMIC )
	{
		eve_a->status |= EVE_A_NO_DEF ;
		DefPosFree(entk);
	}
	entk->count3 = 0 ;
}

#endif
#if 0
static void InitThinkParam( work )
Work	*work ;
{
	/*ENE_ACT内で if(dir>=0) で分岐に使っている*/
	work->action.dir = -1 ;
	work->action.pad = 0 ;
	work->enethink.status2 = 0;
	/*回避モードには参加しない*/

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

}
#endif

static void Think( work )
Work	*work ;
{
	ENETHINK	*entk ;
	AT_THK *at_thk ;

#if 0
	if ( GM_VRStatus & (GM_VR_IDLE) ) return ;
	if ( GM_VRStatus & GM_VR_CLEAR ) {
		ENE_ActStatusCheck ( &work->enethink ) ;
		work->action.old_status = work->action.status ;
		return ;
	}
#endif
	entk = &work->enethink ;
	at_thk = (AT_THK *) entk->character ;



	InitThinkParam( work ) ;

/****************************
各ModeCheck関数内で現在のアラートレベルと自身の思考モードを比較
切り替え必要なら各ModeStart関数で別思考開始のための初期化
*****************************/

	switch( entk->think1 ){
		case ENE_TH1_SNEAK : /*通常*/
			HZX_ChangeRouteCourse( HZX_ROOT_COURSE1 ) ;
			ENE_Watcher_Think1_Sneak( entk ) ;
			HZX_ClearRouteCourse( 0 ) ;
			ConvertSneakModeCheack( entk ) ;
			break ;
		case ENE_TH1_SEARCH : /*通常*/
			HZX_ChangeRouteCourse( HZX_ROOT_COURSE1 ) ;
			ENE_Watcher_Think1_Sneak( entk ) ;
			HZX_ClearRouteCourse( 0 ) ;
			ConvertSneakModeCheack( entk ) ;
			break ;
		case ENE_TH1_ALERT : /*危険*/
			Tng_Think1_Alert( entk ) ;
			AlertModeCheck( entk ) ;
			break ;
		case ENE_TH1_AVOID : /*回避*/
			HZX_ChangeRouteCourse( HZX_ROOT_COURSE1 ) ;
			Check_Attacker_Think1_Avoid( entk );
			AvoidModeCheck( entk ) ;
			HZX_ClearRouteCourse( 0 ) ;
			break ;
		case ENE_TH1_DAMAGE :
			/*ダメージ中及び死亡から再発生まで*/
			ENE_Enemy_Think1_Damage( entk ) ;
			/*麻酔チェック*/
			if(CheckAnesAT(entk)){
				AT_AnesThink(entk);
			}
			DamageModeCheck( entk ) ;
			break ;
		case ENE_TH1_RESURRECT :
			ENE_AttackerResurrect( entk ) ;
			RessModeCheck( work ) ;
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

	ENE_ActInit( entk ) ;
	AT_Action( &work->action ) ;
	ENE_ActStatusCheck( entk ) ;
	ENE_Gravitation( entk ) ;


}


static void TngMain( Work *work )
{
	PreProcess( work ) ;
	/* スーパーアンリアルでも個別メッセージだけはうけとっておく */
	if ( work->enethink.status & ENE_STATUS_EVER_UNREAL ) return ;
	Think( work ) ;
	Action( work ) ;

}

static void Act( Work *work )
{
//	SVECTOR rgb;

	ENETHINK *entk;
	ENTK_TENG_A *tng_a ;
	TNG_COM		*tng_com ;

	entk = &work->enethink;
	tng_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) tng_a->tng_com ;

//	PosBox(&entk->def_pos,250.0f , NULL );

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

	ENE_PreProcess( entk ) ;
	TngMain( work ) ;

	ENE_AftProcess( entk ) ;



	/*付随エフェクト制御*/
	if(( entk->act->bodyp.life <=0)
	||(work->body.objs->flag & DG_FLAG_INVISIBLE)
	){ 
		if(tng_a->blur_sw > 0){
			tng_a->blur_sw = 0;
		}
	}else {
		if(tng_a->blur_sw < 1){
			tng_a->blur_sw++;
		}
	}
	if ( entk->act->ctrl->skip_flag & CTRL_SKIP_FLR_CHECK ) {
		work->hex_info.flags |= HEX_FLAG_UNDRAW ;
	}
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
//tng_a->type  = TNG_TYPE_A ;

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
	ENE_InitObject( entk, body, bodyname, ctrl, work->lights ) ;

	/*マルチテクスチャ解除*/
//	SIG_MultiTexOff(body);
	/* 防御ターゲットの初期化(装備より先にする)*/
	ENE_InitTarget( entk, ctrl, body, act ) ;
    GM_SetTargetCallBack( &act->bodyp.deftrg, DefTargCallBack, work ) ;
	
	ENE_InitWeapon( entk, body, weapon, wpname, NULL ) ;
	ENE_InitSubWeapon( entk, body, NULL, sub_wpname ) ;

	act->bodyp.type |= (ENE_TYPE_ATTACKER|ENE_TYPE_TNG_A);
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


//	ENE_InitRoute( &work->cl_route ) ;
	entk->rnavi2 = &work->cl_route ;


	if( GM_AlertMode == ALERT_MODE_SEARCH ) {
		ENE_AttackerStartModeSearchWarp( &work->enethink ) ;
	} else {
		ENE_AttackerStartModeSneak( &work->enethink ) ;
	}
	HEX_RegistControlInfo(&work->hex_info, &work->control) ;
	work->action.CheckDamage = ENE_EnemyDamagePad ;

	at_thk->at_status = 0 ;
	/* 守備位置セット */
	d_pos_opt = GCL_GetOption( 'd' ) ;
	if ( d_pos_opt  != NULL ){
		CONTROL ctrl ;
		entk->def_pos.vx = (float)(GCL_GetNextInt()); 
		entk->def_pos.vy = (float)(GCL_GetNextInt()); 
		entk->def_pos.vz = (float)(GCL_GetNextInt()); 
		ctrl.mov = entk->def_pos ;
		GM_ConfigControlMapID( &ctrl ) ;
		entk->def_mapbit = ctrl.map ;
		at_thk->at_status |= AT_ST_DEFENSE ;
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

	if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
		entk->search_route += GM_RouteOffset ;
	}


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
	ACTION		*act ;
	char	*opt ;


	entk = &work->enethink ;

	entk->eve_a = &work->tng_a ;
	act = &work->action ;
	tng_a = (ENTK_TENG_A *) entk->eve_a ;
	entk->rnavi =&work->routenavi ;

printf("ATTACKER COMSET Id NUMBER!!!!!!!!!!!!!!\n") ;
	COM_SetIDNumber( entk ) ;
	if ( (entk->status_status = GCL_GetOptionValue( 't', -1 )) < 0  ) {
		entk->status_status = 0 ;
	}
	SIG_ENE_GetResources(work ,name ,ENE_TYPE_TNG_A,NULL) ;
	if(act->bodyp.type & ENE_TYPE_HOLD_EXIT){
		act->bodyp.type |= ENE_TYPE_CONVERT ;
	}
#if 1
	act->bodyp.type |= (ENE_TYPE_ATTACKER|ENE_TYPE_TNG_A);
#else
	act->bodyp.type = (ENE_TYPE_ATTACKER|ENE_TYPE_TNG_A);
#endif
/*手足ダメージでライフ減る*/
	act->bodyp.type |= ENE_TYPE_DMG_HAND_LEG ;
	SET_FLAG(act->bodyp.type,ENE_TYPE_NO_PBREAK);



	ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerFindPos, &GM_PlayerAddress, NULL, &GM_PlayerMap ) ;

	/*守備兵に必須の非通報フラグ*/
	entk->status |= ENE_STATUS_LONELY_FIGHT|ENE_STATUS_CONVERT ;


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
	entk->sense.eye_s_s[ALERT_MODE_SNEAK] = 3000.0F ;
	entk->sense.eye_s_s[ALERT_MODE_ALERT] = 3000.0F ;
	entk->sense.eye_s_s[ALERT_MODE_AVOID] = 3000.0F ;
	entk->sense.eye_s_s[ALERT_MODE_SEARCH] = 3000.0F ;
#endif
	/* 耐久力セット */
//	AT_SetDurable( &act->bodyp, 0, 3, 3, 0 ) ;
	/*弱め*/
	AT_SetDurable( &act->bodyp, 0, 0, 0, 0 ) ;

#if 1
	entk->act->bgm_track = 0 ;
#else
	if ( (entk->act->bgm_track = GCL_GetOptionValue( 'b', -1 )) < 0  ) {
		entk->act->bgm_track = 0 ;
	} else {
		/* 個別ＢＧＭトラックを初期化する */
		GM_MixConvFader( entk->act->bgm_track, 0x20, 0 ) ;
	}
#endif
	/*起動時のモードセレクト*/
	entk->think1 = ENE_TH1_SNEAK ;

	ENE_WatcherStartModeSneak( entk ) ;

	work->tng_a.type = TNG_TYPE_A ;

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
		GV_SetActorChild( work , NewSIG_BlurPoint(&work->body.objs->objs[HUMAN21_ATAMA].world,
			&shift,&col ,&tng_a->blur_sw)) ;
	}
#endif


/*待機場所から1フレームだけ見つけちゃう問題*/
/*視界off状態で起動*/
	work->action.status |= ACT_STATUS_EYE_CLOSE ;

	/* ステージ情報当てはめ */
	ENE_LoadEneMemory( entk ) ;

	/* 武器位置最後に初期位置へリセット */
	ENE_ResetWeaponPosition( entk ) ;


	return (0);
}

void *NewSigTengA( int name, int where )
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


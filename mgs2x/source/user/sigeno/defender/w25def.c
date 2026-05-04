//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	w25def.c
	タイトロープイベント兵
	海上巡回兵の拡張型
	2001/04/17 K.Sigeno
	$Id: w25def.c,v 1.1.1.3 2002/11/19 11:49:10 Yoshizawa1 Exp $
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

#include	"eve_w32.h"
#include	"../attacker/sigavoid.x"
#include	"../attacker/sig_conv.x"

//#include	"../../morita/emma/include/emma_com.h"

/*----- ワーク定義 -----*/
typedef	struct	{
	GV_ACT_EX	actor ;
	CONTROL		control ;
	OBJECT		body ;
	OBJECT		weapon ;
	OBJECT		sub_weapon ;

	FMATRIX		lights[2] ;

	int			name;
	int			id ;

	/* 暫定モーションデータ */
//	int			motion_data[ MOTION_MAX ];
	/* アクション */
	ACTION		action ;
	/* ルートナビ */
	ROUTENAVI	routenavi ;
	/* クリアリングルートナビ */
	ROUTENAVI	cl_route ;
	/* ゾーンナビ */
	ZONENAVI	zonenavi ;
	/* 敵兵思考 */
	ENETHINK		enethink ;
	EVENT_W32	eve_w ;
//	AT_THK			at_thk ;
//	ENTK_TYPE_A		entk_a ;
	/*実験用Prim*/
} Work ;

//#define DEF_POS_TEST
extern void AT_SetDurable( BODYPARAM *,int,int,int,int);

extern void ENE_NoticeCheck( ENETHINK * );
//extern void AlertModeStart( ENETHINK * ) ;
extern void ENE_EnemyStartModeDamage( ENETHINK * ) ;
//extern void Think1_Alert( ENETHINK * );
extern void ENE_Gravitation( ENETHINK * );
//extern void Think1_Defense( ENETHINK * );
extern void ENE_Watcher_Think1_Sneak( ENETHINK * );
extern void ENE_Enemy_Think1_Damage( ENETHINK * );
//extern void Sig_DefenseModeStart( ENETHINK * ) ;
extern void ENE_WatcherStartModeAvoidDamage(ENETHINK *) ;
extern void ENE_AttackerResurrect(ENETHINK *) ;
extern void ENE_AttackerStartModeSneak(ENETHINK *) ;

/*sigeno/attacker/sigavoid.c*/
//extern void SIG_Attacker_Think1_Avoid( ENETHINK *entk ) ;
//extern void SIG_AttackerStartModeAvoid( ENETHINK *entk ) ;

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern void SigZoneView( int , SVECTOR * ,float ) ; 
extern void SIG_RouteView(ROUTENAVI * ) ;
#endif

extern void W32AlertStart(ENETHINK *) ;
extern void W32thinkAlert(ENETHINK *) ;
extern void W32EscapeStart(ENETHINK *) ;

//void	SIG_DefenderResurrect( ENETHINK * ) ;


#if 0
static	void	DamageModeCheack( entk )
ENETHINK	*entk ;
{

	EVENT_W32 *eve_w ;

	eve_w = (EVENT_W32 *) entk->character;

	if ( entk->notice & ENE_NOTICE_DAMAGE ) {
		switch( GM_AlertMode ) {
			case ALERT_MODE_SNEAK :
				ENE_WatcherStartModeSneakDamage( entk ) ;
			break ;
			case ALERT_MODE_ALERT :
//				ENE_WatcherStartModeAlert( entk ) ;
				if(eve_w->find_flag & W32_ESC_POS){
					W32EscapeStart(entk) ;
				}else {
					W32AlertStart(entk) ;
				}
			break ;
			case ALERT_MODE_SEARCH :
				ENE_WatcherStartModeSneakDamage( entk ) ;
			break ;
			default :
				ENE_WatcherStartModeAvoidDamage( entk ) ;
			break ;
		}
	}
#if 0
	if ( entk->notice & ENE_NOTICE_RES ) {
		ENE_WatcherResurrectionMode( entk ) ;
	}
#endif
}
#endif

static void UnSetHomingStatusEmaSight(ENETHINK *entk){
	entk->hom.status &= ~HOMING_EMA_IN_SIGHT ;
}
/*チームのメンバーが発見済みだったら攻撃参加*/
#if 0
static void CheckTeamMemberFind( ENETHINK *entk ){
	EVENT_W32 *eve_w ,*target;
	COMMANDER	*com ;
	E_GROUP		*group ;
	E_UNIT		*unit ;
	int i ;
	
	eve_w = (EVENT_W32 *) entk->character;
	com = entk->com ;
	if(eve_w->team){
		group = com->enemys.group[0] ;
		unit = group->unit[0] ;
		for ( i=0; i<unit->enemy_num; i++ ) {
			entk = unit->entk[ i ] ;
			target = (EVENT_W32 *) entk->character ;
			if(eve_w->team == target->team){
				if(target->find_flag & W32_FIND){
					eve_w->find_flag |= (W32_ALERT|W32_FIND) ; 
					break;
				}
				if(((target->find_flag & W32_ALERT )
				&&(target->st_flag & W32_ST_CONTACT))
				){
					eve_w->find_flag |= W32_ALERT ; 
					break;
				}
			}
		}
	}
}

#endif
/*チームのメンバーに連絡*/
static void SetTeamMemberFind( ENETHINK *entk ){
	EVENT_W32 *eve_w ,*target;
	COMMANDER	*com ;
	E_GROUP		*group ;
	E_UNIT		*unit ;
	int i ;
	
	eve_w = (EVENT_W32 *) entk->character;
	com = entk->com ;
	if(eve_w->team){
		group = com->enemys.group[0] ;
		unit = group->unit[0] ;
		for ( i=0; i<unit->enemy_num; i++ ) {
			entk = unit->entk[ i ] ;
			target = (EVENT_W32 *) entk->character ;
			if(eve_w->team == target->team){
				target->find_flag |= (W32_ALERT|W32_FIND) ;
//				if(target->find_flag & W32_FIND){
//					eve_w->find_flag |= (W32_ALERT|W32_FIND) ; 
//					break;
//				}
			}
		}
	}
}

static void PreProcess( Work *work )
{
	ENETHINK	*entk ;
	EVENT_W32 *eve_w;

	entk = &work->enethink ;
	eve_w = (EVENT_W32 *) entk->character;


	/* スーパーアンリアルでも個別メッセージだけはうけとっておく */
	if ( work->enethink.status & ENE_STATUS_EVER_UNREAL ) return ;

	ENE_EyeInfoCheck( entk, &entk->pl_eyei ) ;
	ENE_NoticeCheck( entk ) ;
#if 1
	ENE_SetAlertLevel( entk ) ;
#else
	if(entk->think1 == ENE_TH1_SNEAK) {
	}else {
		ENE_SetAlertLevel( entk ) ;
	}
#endif
}

/*********************
危険レベルではなく個別のモード指定で思考切り替え
*********************/
static	void	ThinkModeCheck( entk )
ENETHINK	*entk ;
{
	EVENT_W32 *eve_w ;


	eve_w = (EVENT_W32 *) entk->character ;
	if(
	(entk->think1 != ENE_TH1_ALERT)
	&&((eve_w->find_flag & W32_ALERT)
	||( GM_AlertMode == ALERT_MODE_ALERT ))
	){
printf("AlertStart\n");
		entk->think1 = ENE_TH1_ALERT ;
		W32AlertStart(entk) ;
	}else if(
//	(GM_AlertMode != ALERT_MODE_ALERT)
//	&&(entk->think1 == ENE_TH1_ALERT)
	(!(eve_w->find_flag & W32_ALERT))){
		switch (GM_AlertMode){
			case ALERT_MODE_SNEAK :
			case ALERT_MODE_SEARCH :
				if(entk->think1 != ENE_TH1_SNEAK){
printf("ENE_WatcherStartModeSneak\n");
					ENE_WatcherStartModeSneak( entk ) ;
					ENE_SetTrgpEnemy( entk, &(entk->trgpoint) ) ;
				}
				break;
			case ALERT_MODE_AVOID :
				if(entk->think1 != ENE_TH1_AVOID){
printf("SIG_AttackerStartModeAvoid\n");
					SIG_AttackerStartModeAvoid( entk ) ;
				}
				break;
			}
	}
	if ( ENE_DamageCheck( entk ) ) {
		if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
printf("ENE_EnemyStartModeDamage\n");
		ENE_EnemyStartModeDamage( entk ) ;
	}
}



static	void	RessModeCheck( work )
Work *work ;
{
	ENETHINK *entk;

	entk = &work->enethink;
	if ( entk->notice & ENE_NOTICE_RES ) {
	}
}

static	void	DamageModeCheck( entk )
ENETHINK	*entk ;
{
	EVENT_W32 *eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;

	/*死んでないので思考セット後 再始動*/
	if ( entk->notice & ENE_NOTICE_DAMAGE ) {

/*2001.06.05 ダメージ後　攻撃モードに行かない*/

		entk->sw_gun &= ~SW_FLAG_SWITCH2 ;
		switch( GM_AlertMode ) {
			case ALERT_MODE_SNEAK :
//				ENE_AttackerStartModeSneakDamage( entk ) ;
				ENE_WatcherStartModeSneakDamage( entk ) ;
			break ;
			case ALERT_MODE_ALERT :
			if(eve_w->find_flag & W32_ESC_POS){
				W32EscapeStart(entk) ;
			}else {
				W32AlertStart(entk) ;
			}
			break ;
#if 1
			case ALERT_MODE_AVOID :
//				Check_AttackerStartModeAvoidDamage( entk ) ;
				SIG_AttackerStartModeAvoidDamage( entk ) ;
			break ;
			default :
				ENE_WatcherStartModeSneakDamage( entk ) ;
			break;
#else
			case ALERT_MODE_AVOID :
//				Check_AttackerStartModeAvoidDamage( entk ) ;
				SIG_AttackerStartModeAvoid( entk ) ;
			break ;
			case ALERT_MODE_SEARCH :
				ENE_AttackerStartModeSearchDamage( entk ) ;
			break ;
#endif
		}
		if ( ENE_DamageCheck( entk ) ) {
			ENE_EnemyStartModeDamage( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_RES ) {
printf("DEATH !!!!!!!!!\n");
#if 0
		/*復活*/
		ENE_AttackerResurrectionMode( entk ) ;
#endif
	}
}
#if 0
static void	SIG_W32DefenderResurrect( entk )
ENETHINK	*entk ;
{
	FVECTOR	pos ;
	int		hzx_id ;


	entk->act->dir = -1 ;
//	entk->act->pad = 0 ;

	ENE_RefreshParam( entk ) ;

/*2001.01.17*/
#if 0
	COM_ResurrectionPosition( &pos, &hzx_id ) ;
#endif

	HZX_FlashTrap( entk->ctrl->hzx_id, &entk->ctrl->evt ) ;
	GM_ResetControlPositionAndGroup( entk->ctrl, &pos, hzx_id ) ;/* 再発生場所へ移動 */
//	Sig_DefenseModeStart( entk ) ;
	/*2001.02.01 左舷突入モード時の再発生は別思考へ*/
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
	EVENT_W32 *eve_w ;

	entk = &work->enethink ;
	eve_w = (EVENT_W32 *) entk->character ;

	if((!(eve_w->find_flag & W32_FIND))&&(entk->think1 != ENE_TH1_ALERT)){
//		CheckTeamMemberFind(entk) ;
	}
	InitThinkParam( work ) ;

/****************************
各ModeCheck関数内で現在のアラートレベルと自身の思考モードを比較
切り替え必要なら各ModeStart関数で別思考開始のための初期化
*****************************/


	switch( entk->think1 ){
		case ENE_TH1_DAMAGE :
			/*ダメージ中及び死亡から再発生まで*/
			ENE_Enemy_Think1_Damage( entk ) ;
			DamageModeCheck( entk );
			break ;
		case ENE_TH1_RESURRECT :
			ENE_AttackerResurrect( entk ) ;
			RessModeCheck( work ) ;
			break ;
		case ENE_TH1_ALERT : /*危険*/
			/*危険モード中に大きな音を聞いた*/
			if ( entk->notice & (ENE_NOTICE_NOISE_M|ENE_NOTICE_NOISE_L) ) {
				entk->alert = ALERT_LEVEL_MAX ;
			}
			if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
				entk->alert = ALERT_LEVEL_MAX ;
				eve_w->find_flag |= (W32_FIND|W32_ALERT); 
			}
			W32thinkAlert(entk) ;

			if((entk->alert >= ALERT_LEVEL_MAX)&&(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)){
				if(( eve_w->find_flag &(W32_ALERT|W32_FIND)) == (W32_ALERT|W32_FIND)){
					if(entk->count3 > DIRECT_TICK(60)){
						SetTeamMemberFind(entk) ;
					}
				}
			}

			ThinkModeCheck( entk ) ;
/*koko*/
#if 0
			if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
				entk->alert = ALERT_LEVEL_MAX ;
			}
#else
			if(eve_w->find_flag & W32_FIND){
				entk->alert = ALERT_LEVEL_MAX ;
			}
#endif
			break ;
		case ENE_TH1_AVOID : /*回避*/
//			SIG_AttackerStartModeAvoid( entk ) ;
			SIG_Attacker_Think1_Avoid( entk ) ;
			ThinkModeCheck( entk ) ;
//			AvoidModeCheck( entk ) ;
			break ;
		case ENE_TH1_SNEAK : /*通常*/
		case ENE_TH1_SEARCH : /*探索*/
			CLEAR_FLAG( entk->iknow_flag ) ;
			ENE_Watcher_Think1_Sneak( entk ) ;
			/*ビックリポーズ後に攻撃*/
/*koko*/
			if((entk->alert >= ALERT_LEVEL_MAX)&&(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)){
				eve_w->find_flag |= (W32_FIND|W32_ALERT); 
				SetTeamMemberFind(entk) ;
			}
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


	ENE_ActInit( entk ) ;
/*2001.03.29 ロックオンしない*/
/*2001.06.21 ロックオンしする*/
//	entk->act->bodyp.deftrg.class &= ~TARGET_LOCKON ;

/*statusフラグのチェック*/
	AT_Action( &work->action ) ;

	ENE_ActStatusCheck( entk ) ;

#if 1
	ENE_Gravitation( entk ) ;
#endif

#if 0
	ctrl->height = body->height ;
	if ( ctrl->grounded & 1 ) ctrl->step.vy = 0.0F ;
	ctrl->step.vy -= 64.0F ;
#endif


}


static void DefenderMain( Work *work )
{
	PreProcess( work ) ;
	if ( work->enethink.status & ENE_STATUS_EVER_UNREAL ) return ;
	Think( work ) ;
	Action( work ) ;

}

#define W25_MSG_ROUTE_CHANGE (1)
#define W25_MSG_ATTACK_MODE (2)
#define W25_MSG_HOUNYOU_HOM_ON (3)
#define W25_MSG_HOUNYOU_HOM_OFF (4)
#define W25_MSG_DETECT_OFF		(5)

static void	DEF_CheckMessage( entk )
ENETHINK	*entk ;
{
    GV_MSG	*msg ;
	int n_msg, code ;
	EVENT_W32 *eve_w  ;

	eve_w = (EVENT_W32 *) entk->character ;
	
    n_msg = entk->ctrl->n_msg ;
	msg = entk->ctrl->msg ;


	if  ( n_msg > 0 ) {
		printf("message get id=[%d]\n",entk->id ) ;
	}

	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case W25_MSG_ROUTE_CHANGE :
				entk->rnavi->next_route = (short)msg->message[ 1 ] ;
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
//					entk->rnavi->next_route += COM_GetRootOfset( ) ;
					entk->rnavi->next_route += GM_RouteOffset ; 
				}
				entk->rnavi->next_node = (u_char)msg->message[ 2 ] ;
			break ;
			case W25_MSG_ATTACK_MODE :
				eve_w->find_flag |= W32_ALERT; 
			break ;
			case W25_MSG_HOUNYOU_HOM_ON :
				SET_FLAG( entk->sw.hounyou, 0x02 ) ;
			break ;
			case W25_MSG_HOUNYOU_HOM_OFF :
				UNSET_FLAG( entk->sw.hounyou, 0x02 ) ;
			break ;
			case W25_MSG_DETECT_OFF :
			printf("message DETECT_OFF get \n" ) ;
				UNSET_FLAG( eve_w->find_flag,(W32_ALERT|W32_FIND)) ; 
			break ;
		}
		msg++ ;
	}
}

static void Act( Work *work )
{

	ENETHINK *entk;
	EVENT_W32 *eve_w  ;

	entk = &work->enethink;
	eve_w = (EVENT_W32 *) entk->character ;

//SIG_RouteView(entk->rnavi) ;
/***/
//	SIG_NumPrint(&entk->ctrl->mov ,entk->alert) ;
/***/
	if(eve_w->OldPlAddr == GM_PlayerAddress){
		eve_w->Pl_StayTime++ ;
	}else {
		eve_w->Pl_StayTime = 0 ;
	}

	if(SIG_CheckStealthStatus(entk)){
		UNSET_FLAG( eve_w->find_flag,(W32_ALERT|W32_FIND)) ; 
	}

	ENE_GM_Act( entk ) ;

	DEF_CheckMessage( entk );
	/*EYE_INFOセット*/
	ENE_PreProcess( entk ) ;
	UnSetHomingStatusEmaSight(entk) ;

	DefenderMain( work ) ;

	ENE_AftProcess( entk ) ;
	work->eve_w.old_find_flag = work->eve_w.find_flag ; 
	eve_w->OldPlAddr = GM_PlayerAddress ;

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
	}

//printf("entk->pl_eyei.sight[%d]\n",entk->pl_eyei.sight) ;


}

static void Die( Work *work )
{
printf("DEFENDER DIE!!\n");
	ENE_FreeResources( &(work->enethink) ) ;
}


#if 1
static	void	DefTargCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;
    work = ( Work * )ptr ;

	ENE_DefTargCallBack( off, def, &work->enethink ) ;

}
#endif
/*---------------------------------------------------------------*/
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

static int w25_GetResources( Work *work, int name, int type ,int *mt_array)
{

	ENETHINK	*entk ;
	CONTROL		*ctrl ;
	ACTION		*act ;
	OBJECT		*body, *weapon, *sub_weapon  ;
	int		root, node, bodyname, wpname ,sub_wpname ,life, faint, value,hearing;
	int	flag ;
//	FVECTOR	d_pos;
	char	*opt ;

	EVENT_W32 *eve_w ;


	entk = &work->enethink ;
	entk->character = eve_w =&work->eve_w ;


	bodyname = ENE_MDL_NAME_GPS ;

	ctrl = &work->control ;
	act = &work->action ;
	body = &work->body ;
	weapon = &work->weapon ;
	sub_weapon = &work->sub_weapon ;

	entk->w = act->w = ( void * )work ;	/* 親子関係用にワークのポインタをセット */
	entk->status = 0 ;



	act->bodyp.type |= (ENE_TYPE_DMG_HAND_LEG|ENE_TYPE_ABAKAN|ENE_TYPE_NO_BLURR) ;
	SET_FLAG(act->bodyp.type,ENE_TYPE_NO_PBREAK);

	flag =	(ENE_STATUS_NO_FINGER|
			ENE_STATUS_NO_EYEANIM|ENE_STATUS_NO_VANIM|
			ENE_STATUS_SHADOW_OFF|
			ENE_STATUS_NO_CAPTURE) ;
//			|ENE_STATUS_GUNLIGHT
		 //			ENE_STATUS_EVER_ZZZ|ENE_STATUS_NO_CAPTURE) ;

	SET_FLAG(entk->status,flag ) ;
printf(" TIRHT LOPE ENE MODE [%d]\n",eve_w->mode);
	if(eve_w->mode == DEF_MODE_W25_C ){
		SET_FLAG(entk->status, ENE_STATUS_HOUNYOU );
printf("ENE_STATUS_HOUNYOU!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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


//	bodyname = GV_StrCode( "tng_def" ) ;
	/*装備品*/
	/*こっちは兵士の種類を決定*/
	{
//		wpname = E_WP_AKS ;
		wpname = E_WP_ABAKAN ;
	}

//	sub_wpname = E_WP_MKR ;
	sub_wpname = 0 ;



	if(eve_w->mode == DEF_MODE_W25_B ){
		ENE_SetSenseParam( &(entk->sense), 0, W25_DEF_EYE_RANGE,
		entk->sense.eye_s_s[0], hearing, DEF_SMELL ) ;
	}else {
		ENE_SetSenseParam( &(entk->sense), 0, DEF_EYE_RANGE,
		entk->sense.eye_s_s[0], hearing, DEF_SMELL ) ;
	}

/*ここを変えれば目標変更可能*/
	ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerFindPos, 
	&GM_PlayerAddress, NULL, &GM_PlayerMap ) ;
	ENE_InitControl( entk,ctrl, name ) ;

	ENE_InitObject( entk, body, bodyname, ctrl, work->lights ) ;

	/* 防御ターゲットの初期化(装備より先にする)*/

	ENE_InitTarget( entk, ctrl, body, act ) ;
    GM_SetTargetCallBack( &act->bodyp.deftrg, DefTargCallBack, work ) ;
	
	ENE_InitWeapon( entk, body, weapon, wpname, NULL ) ;
//	ENE_InitSubWeapon( entk, body, sub_weapon, sub_wpname ) ;

	ENE_InitAction( entk, act, ctrl, body, life, faint, DEF_BLOOD ,NULL) ;

	/* 耐久力セット */
	AT_SetDurable( &act->bodyp, 0, 5, 3, 0 ) ;

	if ( (value = GCL_GetOptionValue( 'v', -1 )) < 0  ) {
		entk->voice_chara = entk->id%4 ;
	} else {
		entk->voice_chara = value ;
	}

	if ( ENE_InitThink( entk, ctrl, &work->routenavi, 
		&work->zonenavi, act, root, node ) < 0 ) {
		return -1 ;
	}


//	entk->rnavi2 = &work->cl_route ;

	ENE_AttackerStartModeSneak( &work->enethink ) ;

	work->action.CheckDamage = ENE_EnemyDamagePad ;

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
	return (0);
}

static int GetResources( Work *work, int name )
{

	ENETHINK	*entk ;
	EVENT_W32 *eve_w ;

	CONTROL		*t_ctrl , esc;
	ACTION		*act ;
	int			t_name, i;

	char	*opt ;

	entk = &work->enethink ;
	COM_SetIDNumber( entk ) ;

	entk->character = eve_w =&work->eve_w ;
	act = &work->action ;

	eve_w->mode = GCL_GetOptionValue( 'm', DEF_MODE_W32_A ) ;
	eve_w->fix_dir = GCL_GetOptionValue( 'c', -1 ) ;
	
	w25_GetResources(work ,name ,ENE_TYPE_EVENT_A,NULL) ;
	work->eve_w.find_flag = 0; 

	if ( ( opt = GCL_GetOption( 't' ) ) != NULL ){
		t_name = GCL_GetNextInt() ;
		t_ctrl = SearchControl( t_name ) ; 
	} else {
		t_ctrl = NULL ;
	}

	if ( ( opt = GCL_GetOption( 's' ) ) != NULL ){
		eve_w->st_flag = GCL_GetNextInt() ;
	}else {
		eve_w->st_flag = 0 ;
	}
	if(eve_w->st_flag & W32_ST_SKIP_DMG){
		act->bodyp.type |= ENE_TYPE_DMG_MUTEKI ;
	}

//ENE_STATUS_HOUNYOU

	/*守備兵に必須の非通報フラグ*/
	entk->status |= ENE_STATUS_LONELY_FIGHT ;
#if 0
	entk->sense.eye_s_s[ALERT_MODE_SNEAK] = 10000 ;
	entk->sense.eye_s_s[ALERT_MODE_ALERT] = 10000 ;
	entk->sense.eye_s_s[ALERT_MODE_AVOID] =  10000 ;
	entk->sense.eye_s_s[ALERT_MODE_SEARCH] = 10000 ;
#endif
	/* 耐久力セット */
//	AT_SetDurable( &act->bodyp, 0, 5, 3, 0 ) ;
	/*弱め*/
	AT_SetDurable( &act->bodyp, 0, 0, 0, 0 ) ;

/*チーム番号*/
	if ( ( opt = GCL_GetOption( 'b' ) ) != NULL ){
		eve_w->team = GCL_GetNextInt() ;
	} else {
		eve_w->team = 0 ;
	}

/*逃げ場所*/
	eve_w->esc_cnt = 0 ;
	if ( ( opt = GCL_GetOption( 'd' ) ) != NULL ){
		eve_w->esc_pos.vx = (float)GCL_GetNextInt() ;
		eve_w->esc_pos.vy = (float)GCL_GetNextInt() ;
		eve_w->esc_pos.vz = (float)GCL_GetNextInt() ;
		esc.mov = eve_w->esc_pos ;
		GM_ConfigControlMapID( &esc ) ;
		eve_w->esc_map = esc.map;
		eve_w->find_flag |= W32_ESC_POS ;
	}

/*攻撃場所*/
#if 0
	FVECTOR		attack_pos[2] ; /*攻撃場所*/
	int			attack_map[2] ;
	short		attack_max ;
	short		attack_num ;
#endif
	eve_w->attack_num = 0;
	eve_w->attack_max = 0;
	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		for(i=0;i<W32_ATTACKPOS_MAX;i++){
			if(GCL_NextStr() == NULL) break;
			eve_w->attack_pos[i].vx = (float)GCL_GetNextInt() ;
			eve_w->attack_pos[i].vy = (float)GCL_GetNextInt() ;
			eve_w->attack_pos[i].vz = (float)GCL_GetNextInt() ;
			esc.mov = eve_w->attack_pos[i] ;
			GM_ConfigControlMapID( &esc ) ;
			eve_w->attack_map[i] = esc.map;
			eve_w->find_flag |= W32_ATTACK_POS ;
			eve_w->attack_max = i ;
		}
	}
/*死亡時プロック*/
	if ( ( opt = GCL_GetOption( 'e' ) ) != NULL ){
		entk->death_proc.argc = 
		ENE_GclGetProc( &(entk->death_proc.proc),
		&entk->death_proc.argv[0] ) ;
		if ( entk->death_proc.argc > (GCL_MAX_ARGS/2) ) return -1 ;
	} else {
		entk->death_proc.proc = 0 ;
	}



	entk->act->bgm_track = 0 ;
	entk->think1 = ENE_TH1_SNEAK ;
	eve_w->Pl_StayTime = 0;
	eve_w->OldPlAddr = -1;

/*手足ダメージでライフ減る*/
	ENE_WatcherStartModeSneak( entk ) ;
	ENE_SetTrgpEnemy( entk, &(entk->trgpoint) ) ;
	work->eve_w.old_find_flag = work->eve_w.find_flag ; 

	/* ステージ情報当てはめ */
	ENE_LoadEneMemory( entk ) ;

	/* 武器位置最後に初期位置へリセット */
	ENE_ResetWeaponPosition( entk ) ;

	return (0);
}

void *NewW25Defender( int name, int where )
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


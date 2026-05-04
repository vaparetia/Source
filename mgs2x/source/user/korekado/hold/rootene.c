//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	rootene.c
	ルート兵（船倉用）

	2000/12/20 Y.Korekado
	$Id: rootene.c,v 1.1.1.2 2002/11/03 17:07:58 Yoshizawa1 Exp $
	
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

#include	"holdene.h"
#include	"rootene.h"
#include	"rtthink.c"

#include "../../mode/demo/libdemo.h"
//#include	"fmt_demo.h"

extern void GM_MouthAnimation( int talk_name, DG_EVMOBJ *evmobj  ) ;

/*--- ------------------------------------------------------------*/
/* メッセージ処理 */
static	void	CheckMessage( entk )
ENETHINK	*entk ;
{
    GV_MSG	*msg ;
	int n_msg, code ;

    n_msg = entk->ctrl->n_msg ;
	msg = entk->ctrl->msg ;

	while ( n_msg-- > 0 ) {
printf("rootene: message [%d]n",msg->message[ 0 ] ) ;
		code = msg->message[ 0 ] ;
		switch( code ) {
			case ENE_MSG_ROOTCHANGE :
printf("change route [%d]->[%d] deffende[%d][%d][%d]\n",
		entk->rnavi->c_route, msg->message[ 1 ],msg->message[ 2 ],msg->message[ 3 ],msg->message[ 4 ] ) ;
				entk->rnavi->next_route = (short)msg->message[ 1 ] ;
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					entk->rnavi->next_route += COM_GetRootOfset( ) ;
				}
				if ( msg->message_len >= 3 ) {
						entk->rnavi->chang_node = (u_char)msg->message[ 2 ] ;
//					printf(" Message Group ID [%x] \n",entk->def_mapbit ) ;
				}
			break ;
		}
		msg++ ;
	}
}

static void PreProcess( Work *work )
{
	ENETHINK	*entk ;

	entk = &work->enethink ;

	CheckMessage( entk ) ;

	ENE_EyeInfoCheck( entk, &entk->pl_eyei ) ;
	ENE_NoticeCheck( entk ) ;
	ENE_SetAlertLevel( entk ) ;
}

static	void	Action ( work )
Work	*work ;
{
	ENETHINK	*entk ;

	entk = &work->enethink ;

	ENE_ActInit( entk ) ;
	AT_Action( &work->action ) ;
	ENE_ActStatusCheck( entk ) ;

	ENE_Gravitation( entk ) ;

}

static void RootEneMain( Work *work )
{
	/* 前処理 */
	PreProcess( work ) ;
	/* 思考処理 */
	Think( work ) ;
	/* 行動処理 */
	Action( work ) ;
}

static void Act( Work *work )
{
	ENETHINK	*entk ;
	
	entk = &work->enethink ;

	ENE_GM_Act( entk ) ;

	ENE_PreProcess( entk ) ;
	RootEneMain( work ) ;
	ENE_AftProcess( entk ) ;
	/* べーやんデバッグ後影切る */
	UNSET_FLAG( entk->act->body->objs->flag, DG_FLAG_SHADOWMAKE ) ;

	if (*(work->flag) & HLD_STATUS_SCOT ) {
		SET_FLAG( entk->act->body->objs->flag, DG_FLAG_INVISIBLE ) ;
//		SET_FLAG( entk->act->body->evmobj->flag, DG_EVMOBJ_INVISIBLE ) ;
		/* スコットは銃消す */
		SET_FLAG( entk->weapon->objs->flag, DG_FLAG_INVISIBLE )  ;
		GM_MouthAnimation( GV_StrCode( "スコット" ), entk->act->body->evmobj ) ;
	}

	if ( *work->flag & (HLD_STATUS_NORMAL_DETECT) ) {
		work->gameover_delay ++ ;
		if( work->gameover_delay > COUNT_VMODE(60) ) {
			/* 発見中フラグＯＮ */
			COM_SetFlameFlag( CMFLAG_DETECT ) ;
		}
	}

	if ( entk->act->status & ACT_STATUS_DEATH ) {
		SET_FLAG ( *work->flag, HLD_STATUS_DEATH ) ;
	}

	work->hold->dogtag_sw = entk->act->sw->dogtag ;
	work->hold->dogtag_item.c_proc =  entk->act->dogtag_item.c_proc ;
	entk->hom.status |= HOMING_SKIP ;	/* ホーミングなし */
}

static void Die( Work *work )
{
	ENETHINK	*entk ;
	extern void ExitCorpseIK(void *work);


	entk = &work->enethink ;
	if ( entk->vanime_head != NULL )  ExitHumanMA( entk->vanime_head ) ;
	if ( entk->sub_weapon != NULL ) GM_FreeObject( entk->sub_weapon ) ;
	TAKABE_FreePuppetIK( entk->act->new_ik ) ;

	if ( !(GV_IsStageDestroy( work )) ) {
		if ( entk->act->oozeblood != NULL ) GV_DestroyActorQuick( entk->act->oozeblood ) ;
		/* 補完用に現在のrotsをコピー */
	//	KR_MemCopy( work->abs_rots, entk->act->body->m_ctrl->abs_rots, sizeof(FVECTOR), 21 ) ;
//		memcpy( work->abs_rots, entk->act->body->m_ctrl->abs_rots, sizeof(FVECTOR)*21 ) ;
	}

	GM_FreeZoneIntrpt( &(entk->z_intrpt) ) ;
	GM_FreeHomingTrg( &(entk->hom) ) ;
    GM_FreeRadarControl( &( entk->sense.rctrl ) ) ;
	entk->act->bodyp.capture.flag = 0 ;	/* C4 落とす */
    GM_FreeTarget( &entk->act->bodyp.deftrg ) ;	/* 親だけで良い */
    GM_FreeTarget( &entk->act->bodyp.pushtrg ) ;
printf(" Check Ene free target[%x]\n",&entk->act->bodyp.deftrg);

/*
    GM_SetTarget( &entk->act->bodyp.deftrg, TARGET_DEFENSE|TARGET_SEEK, 1, ENEMY_SIDE,  &EneBodyTrgSize[ ENE_BODYTRGSIZE_STAND ], &DG_ZeroVector ) ;
    GM_PutTarget( &entk->act->bodyp.deftrg ) ;
*/

	COM_UnsetEnethinkWork( entk ) ;

printf(" Check Ene Die\n");
}
/*--- ------------------------------------------------------------*/
/* スコットダメージ処理 */
static int	ScotDamagePad( act )
ACTION	*act ;
{
	return 0 ;
}


/* ターゲットコールバック関数 */
static	void	DefTargCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;
    work = ( Work * )ptr ;

	ENE_DefTargCallBack( off, def, &work->enethink ) ;
}

/*	
	シナリオオプション使用状況
	s:ステータス r:ルート n:ノード d:ディフェンス位置
*/
static int GetResources( Work *work, OBJECT *body, OBJECT *weapon, 
	FVECTOR *pos, int dir, FVECTOR *mt_rots, int name, int *flag, int mode,
	CONTROL *ctrl, HOLD_ENE *hold )
{
	extern void *NewDogtagFlash( OBJECT *object, int *flag ) ;
	ENETHINK	*entk ;
	ACTION		*act ;
	int			i, life, faint, zone ;
	int		act_num ;
	int eye_len, ear ;

	work->hold = hold ;
	work->return_pos = *pos ;
	work->return_dir = dir ;
	work->rtwork.flag = work->flag = flag ;
	work->gameover_delay = 0 ;

	entk = &work->enethink ;
	act = &work->action ;
	entk->character = &work->rtwork ;

	entk->w = act->w = ( void * )work ;	/* 親子関係用にワークのポインタをセット */

	entk->sense.eye_s_s[ALERT_MODE_SNEAK] = DEF_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_ALERT] = DEF_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_AVOID] = DEF_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_SEARCH] = DEF_EYE_SIGHT ;

	life = DEF_LIFE ;
	faint = DEF_FAINT ;

/* 敵兵標準初期化 */

	/***コマンダへの登録****/
	COM_SetIDNumber( entk ) ;

	/* パラメータセット */
	eye_len = entk->sense.eye_s_s[0] ;
	ear = DEF_HEARING ;

	if (*(work->flag) & HLD_STATUS_SCOT ) {
		entk->sense.eye_s_s[ALERT_MODE_SNEAK] = 1 ;
		entk->sense.eye_s_s[ALERT_MODE_ALERT] = 1 ;
		entk->sense.eye_s_s[ALERT_MODE_AVOID] = 1 ;
		entk->sense.eye_s_s[ALERT_MODE_SEARCH] = 1 ;
		eye_len = 2 ;
		ear = 1 ;
	}

	ENE_SetSenseParam( &(entk->sense), 0, DEF_EYE_RANGE, eye_len, ear, DEF_SMELL ) ;
	ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerFindPos, &GM_PlayerAddress, NULL, &GM_PlayerHzxID ) ;

	/* コントロールの初期化 */
	entk->ctrl = ctrl ;
	GM_InitRadarControl( &entk->sense.rctrl, &entk->ctrl->mov, RADAR_VISIBLE|RADAR_SIGHT, -1 );
	GM_RadarSetVRange( &entk->sense.rctrl, RADAR_VRANGE_UPPER , RADAR_VRANGE_LOWER );
	GM_SetZoneIntrpt( &entk->z_intrpt, ctrl, 0, 0, ZONE_INTRPT_ENEMY ) ;
	GM_PutZoneIntrpt( &entk->z_intrpt ) ;

	/* オブジェクトの初期化 */
#if 0
	ENE_InitObject( entk, body, bodyname, ctrl, work->lights ) ;
	entk->name_id.body = data ; 死ぬ処理は後回し
#else
	entk->lights = body->objs->light ;
	entk->vanime_head = NULL ; /* cv2 ないので頂点アニメ無し */

	/* 関節型モデルの表示 */
	GM_ConfigObjectMotion( body, 3, GV_StrCode("ushold"), MT_FLAG_HUMAN2 ) ;
	GM_ConfigObjectStep( body, &ctrl->step ) ;
	GM_ConfigObjectAction( body, 0, 0, 0, 0xfffff, 0 ) ;

	/* object control の関連付け */
	GM_ConfigControlObject( ctrl, body ) ;

	/* 頂点アニメ */
	entk->vanime_flag = 0 ;
	entk->vanime_head = NULL ;

#endif

	/* 防御ターゲットの初期化(装備より先にする)*/
	ENE_InitTarget( entk, ctrl, body, act ) ;
    GM_SetTargetCallBack( &act->bodyp.deftrg, DefTargCallBack, work ) ;

	/* ヘッドマーク */
	GV_SetActorChild( entk->w, (act->headmarkwork = NewControl_Headmark3( 
			&(BODYWORLD( body, HUMAN21_ATAMA )), &act->bodyp.deftrg, ctrl ) ) ) ;

	/* 装備関係初期化 */
#if 0
	ENE_InitWeapon( entk, body, weapon, wpname, &act->bodyp.deftrg ) ;
#else
//	entk->name_id.weapon = data ; 死ぬ処理は後回し
	entk->weapon = weapon ;
	entk->sw.magg = 1 ;
	entk->sw.radio = 0 ;
	entk->sw.sub_weapon = 0 ;
	entk->sw.bullet = &entk->bullet ;
	entk->sw_gun = 0 ;
	entk->sw_light = 0 ;
	entk->sw.n_sight = 0 ;

#endif
	/* サブウェポン無し */
	ENE_InitSubWeapon( entk, body, NULL, 0 ) ;

	/* アクション初期化 */
	AT_InitAction( act, ctrl, body ) ;
	act->ene_status = &entk->status ;
	act->thk_status = &entk->thk_status ;
	act->name_id = &entk->name_id ;
	act->sw = &entk->sw ;
	AT_SetBodyParam( &(act->bodyp), life, faint, DEF_BLOOD ) ;


	if (*(work->flag) & HLD_STATUS_SCOT ) {
		for ( i = 0 ; i < EM_STANDARD_MAX ; i++ ){
#ifdef HOLD_MOT_NEW
			ScotMotionArray[ i ] = MAX_USHOLD_MOTION+i ;
#else
			ScotMotionArray[ i ] = -1 ;
#endif
		}
		i=0 ;
		while ( use_standard_motion_num[i][0] >= 0 ){
			ScotMotionArray[ use_standard_motion_num[i][0] ] = use_standard_motion_num[i][1] ;

#ifndef HOLD_MOT_NEW
			if (*(work->flag) & HLD_STATUS_SCOT ) {
				if ( use_standard_motion_num[i][1] == USHOLD_gbs_ak_nom_idle_atk ) {
					ScotMotionArray[ use_standard_motion_num[i][0] ] = SCOT_speech_idle_1 ;
				}
				if ( use_standard_motion_num[i][1] == USHOLD_gbs_ak_nom_walk ) {
					ScotMotionArray[ use_standard_motion_num[i][0] ] = SCOT_walk ;
				}
				if ( use_standard_motion_num[i][1] == USHOLD_gbs_ak_nom_find_ply ) {
					ScotMotionArray[ use_standard_motion_num[i][0] ] = SCOT_speech_detect_sna ;
				}
				if ( use_standard_motion_num[i][1] == USHOLD_gbs_ak_nom_fire_p_high ) {
					ScotMotionArray[ use_standard_motion_num[i][0] ] = SCOT_speech_detect_sna_idle ;
				}
			}
#endif
	//printf("%d: [%d]->[%d]\n",i,use_standard_motion_num[i][0],use_standard_motion_num[i][1] ) ;
			i++ ;
		}
#ifdef HOLD_MOT_NEW
		if (*(work->flag) & HLD_STATUS_SCOT ) {
			ScotMotionArray[ EM_stand ] = SCOT_speech_idle_1 ;
			ScotMotionArray[ EM_walk ] = SCOT_walk ;
			ScotMotionArray[ EM_find_ply ] = SCOT_speech_detect_sna ;
			ScotMotionArray[ EM_kamae_gun_high ] = SCOT_speech_detect_sna_idle ;
		}
#endif
		AT_SetActionMotion( act, NULL, (int *)ScotMotionArray ) ;
	} else {
		for ( i = 0 ; i < EM_STANDARD_MAX ; i++ ){
			CheckEneMotionArray[ i ] = MAX_USHOLD_MOTION+i ;
		}
		i=0 ;
		while ( use_standard_motion_num[i][0] >= 0 ){
			CheckEneMotionArray[ use_standard_motion_num[i][0] ] = use_standard_motion_num[i][1] ;
			i++ ;
		}
		AT_SetActionMotion( act, NULL, (int *)CheckEneMotionArray ) ;
	}

	AT_SetMode( act, ENE_ActStandStill ) ;

	act_num = 0 ;
	act->c_motion_num[0] = act_num ;
//	GM_ConfigObjectAction( act->body, 0, act->motion_table[act_num], 0, MOTION_MASK_FULL, 0 );
//	GM_ConfigObjectAction( act->body, 0, 0, 0, MOTION_MASK_FULL, ACT_INTERP_DEF );
	if (*(work->flag) & HLD_STATUS_SCOT ) {
		GM_ConfigObjectAction( act->body, 0, SCOT_speech_idle_1, 0, MOTION_MASK_FULL, 0 );
	} else {
		GM_ConfigObjectAction( act->body, 0, USHOLD_idle_4, 0, MOTION_MASK_FULL, 0 );
	} 
/*
	old_abs = body->m_ctrl->old_abs_rots ;
	for ( i = 0 ; i < 21 ; i++ ){
		*old_abs = *mt_rots ;
	}
*/
	work->abs_rots = mt_rots ;

	/* ＩＫ */
	act->new_ik = TAKABE_MakePuppetIK( ctrl, body );
	act->ik_time = 0 ;

	/* ドッグタグ */
	entk->sw.dogtag = 0 ;
	entk->d_name = NULL ;
	act->dogtag_id = hold->dogtag_id ;
	act->dogtag_item = hold->dogtag_item ;
	if ( act->dogtag_id >= 0 ) {
		GV_SetActorChild( work, NewDogtagFlash( act->body, &entk->sw.dogtag ) ) ;
	}

	/* 思考パラメータ初期化 */
//	if ( ENE_InitThink( entk, ctrl, &work->routenavi, &work->zonenavi, act, root, node ) < 0 ) {

	ENE_InitRouteNavi( &work->routenavi, mode, 0 ) ;
	ENE_InitZoneNavi( &work->zonenavi ) ;
	ENE_InitTargPoint( &entk->trgpoint ) ;
	ENE_InitEneThink( entk, ctrl, &work->routenavi, &work->zonenavi, act ) ;
	ctrl->mov = work->routenavi.nodes[ 0 ] ;
	entk->sense.facedir = ctrl->rot.vy = ctrl->turn.vy = work->routenavi.pa_dir[ 0 ] ;
    GM_ConfigControlHzxHeight( ctrl, 750.0F, pos->vy ) ;
	GM_ConfigControlMapID( ctrl ) ;
/* 床の高さ決め打ち */
//	ctrl->hzx_base = -17000 ;
	ctrl->hzx_base = ctrl->mov.vy ;

//entk->act->body->height = ctrl->mov.vy - pos->vy ;

	ENE_SetTrgpPoint( &(entk->trgpoint), &(ctrl->mov), ctrl->hzx_id) ;
	zone = HZX_GetAddress( ctrl->hzx_id, &ctrl->mov, -1 ) ;
	for( i=0; i<4; i++ ) {
		entk->before_inzone[i] = ctrl->addr ;
	}
	entk->name_id.g_id = entk->g_id ;
	entk->name_id.u_id = entk->u_id ;
	entk->name_id.id = entk->id ;
	act->name_id->motion = GV_StrCode("ushold") ;

#if 1
	entk->def_pos = work->routenavi.nodes[ 0 ] ;	/* 戻る位置 */
#else
	entk->def_pos = *pos ;	/* 戻る位置 */
#endif
	entk->tmp_buff2[0] = dir ;/* 戻る方向 */

	/*思考開始場所セット */
	RootStart( entk, mode ) ;
	if (*(work->flag) & HLD_STATUS_SCOT ) {
		work->action.CheckDamage = ScotDamagePad ;
	} else {
		work->action.CheckDamage = ENE_EnemyDamagePad ;
	}

	/* 死体になっても腐らない */
	/* 聴力幅せまい */
	AT_SetType( act, (ENE_TYPE_USHOLD|ENE_TYPE_NO_PBREAK) ) ;

printf("Root Ene Ctrl Name[%d]\n",ctrl->name ) ;
	return (0);
}

void *NewRootEnemy( body, weapon, pos, dir, mt_rots, name, flag, mode, ctrl, hold )
OBJECT	*body ;
OBJECT	*weapon ;
FVECTOR	*pos ;
int		dir ;
FVECTOR	*mt_rots ;
int		name ;
int		*flag ;
int		mode ;
CONTROL	*ctrl ;
HOLD_ENE *hold ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body, weapon, pos, dir, mt_rots, name, flag, mode, ctrl, hold ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

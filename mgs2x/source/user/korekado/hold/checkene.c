//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	checkene.c
	確認兵（船倉用）

	2000/03/28 Y.Korekado
	$Id: checkene.c,v 1.1.1.3 2002/11/19 11:44:15 Yoshizawa1 Exp $

	2001/10/22 PAL
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
#include	"checkene.h"
#include	"chthink.c"


/*--- ------------------------------------------------------------*/
static void PreProcess( Work *work )
{
	ENETHINK	*entk ;

	entk = &work->enethink ;
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

static void CheckEneMain( Work *work )
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
	CheckEneMain( work ) ;
	ENE_AftProcess( entk ) ;
	/* べーやんデバッグ後影切るライター兵でいる */
	if ( entk->act->keep_mot != EM_ijiiji ) {
		UNSET_FLAG( entk->act->body->objs->flag, DG_FLAG_SHADOWMAKE ) ;
	}
/*
	if ( entk->think3 == TH3_ALLEND ) {
		SET_FLAG ( *work->flag, HLD_STATUS_ACTIVE_END ) ;
	}
	if ( entk->act->pad == SP_DISCOVERY_STOP ) {
		SET_FLAG ( *work->flag, HLD_STATUS_DOWN_DETECT ) ;
	}
	if ( entk->act->pad == SP_DISCOVERY ) {
		SET_FLAG ( *work->flag, HLD_STATUS_NORMAL_DETECT ) ;
	}
*/
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

	if( entk->act->status_status & ACT_STST_KAITAIC4_BOMB ) {
		if ( work->hold->items & 0x0200 /*ITEM_KAITAIC4*/ ) {
			work->hold->status2 |= HLD_STATUS2_C4_BOMB ;
			work->hold->bodyparam.capture.flag &= ~CAPTURE_C4EXIST ;
		}
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
#if 0
    TARGET			deftrg ;	/* 防御ターゲット */
	CAPTURE_TARGET	capture ;	/* 捕まり情報 */
    POWER_TARGET	power ;	/* 防御属性 */
    TARGET			def_child1[ PTARGET_LEVEL1_NUM ] ;
    POWER_TARGET	power_child1[ PTARGET_LEVEL1_NUM ] ;	/* 防御属性 */
    TARGET			def_child2[ PTARGET_LEVEL2_NUM ] ;
    POWER_TARGET	power_child2[ PTARGET_LEVEL2_NUM ] ;	/* 防御属性 */
    TARGET			def_child3[ PTARGET_LEVEL3_NUM ] ;
    POWER_TARGET	power_child3[ PTARGET_LEVEL3_NUM ] ;	/* 防御属性 */


    TARGET			deftrg ;	/* 防御ターゲット */
	CAPTURE_TARGET	capture ;	/* 捕まり情報 */
    POWER_TARGET	power ;	/* 防御属性 */
    TARGET			def_child0[ PTARGET_LEVEL0_NUM ] ;
    POWER_TARGET	power_child0[ PTARGET_LEVEL0_NUM ] ;	/* 防御属性 */
    TARGET			def_child1[ PTARGET_LEVEL1_NUM ] ;
    POWER_TARGET	power_child1[ PTARGET_LEVEL1_NUM ] ;	/* 防御属性 */
    TARGET			def_child2[ PTARGET_LEVEL2_NUM ] ;
    POWER_TARGET	power_child2[ PTARGET_LEVEL2_NUM ] ;	/* 防御属性 */
    TARGET			def_child3[ PTARGET_LEVEL3_NUM ] ;
    POWER_TARGET	power_child3[ PTARGET_LEVEL3_NUM ] ;	/* 防御属性 */
#endif
static	void	TargetParamCopy( TARGET *to, TARGET *from )
{
	to->class = from->class ;
	to->map = from->map ;
	to->side = from->side ;

	to->world = from->world ;
	to->center = from->center ;
	to->offset = from->offset ;
	to->size = from->size ;

	to->weapon_type = from->weapon_type ;
	to->damaged = from->damaged ;
}

static	void	TargetPowerParamCopy( POWER_TARGET *to, POWER_TARGET *from )
{
	to->force = from->force ;
	to->type = from->type ;
	to->vital = from->vital ;
	to->damage = from->damage ;
	to->faint = from->faint ;
}

static	void	TargetCaptureParamCopy( CAPTURE_TARGET *to, CAPTURE_TARGET *from )
{
	to->capture = from->capture ;
	to->flag = from->flag ;
	if( from->capture != NULL ) {
		/* 相手先のつかみ相手を自分に換える */
		from->capture->capture = to ;
	}
}

static	void	TargetCopy( ENETHINK *entk, HOLDBODYPARAM *bodyp )
{
	int	i ;
	ACTION	*act ;

	act = entk->act ;
	TargetParamCopy( &act->bodyp.deftrg, &bodyp->deftrg ) ;
	TargetPowerParamCopy( &act->bodyp.power, &bodyp->power ) ;
	TargetCaptureParamCopy( &act->bodyp.capture, &bodyp->capture ) ;
	act->bodyp.off_center = bodyp->off_center ;

	for ( i=0; i<PTARGET_LEVEL1_NUM; i++ ) {
		TargetParamCopy( &act->bodyp.def_child1[i], &bodyp->def_child1[i] ) ;
		TargetPowerParamCopy( &act->bodyp.power_child1[i], &bodyp->power_child1[i] ) ;
	}
	for ( i=0; i<PTARGET_LEVEL2_NUM; i++ ) {
		TargetParamCopy( &act->bodyp.def_child2[i], &bodyp->def_child2[i] ) ;
		TargetPowerParamCopy( &act->bodyp.power_child2[i], &bodyp->power_child2[i] ) ;
	}
	for ( i=0; i<PTARGET_LEVEL3_NUM; i++ ) {
		TargetParamCopy( &act->bodyp.def_child3[i], &bodyp->def_child3[i] ) ;
		TargetPowerParamCopy( &act->bodyp.power_child3[i], &bodyp->power_child3[i] ) ;
	}
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
	int event, FVECTOR *trg_pos, CNCTOBJ *cnct, CONTROL *ctrl, HOLDBODYPARAM *bodyp,
	HOLD_ENE *hold )
{
	extern void *NewDogtagFlash( OBJECT *object, int *flag ) ;
	ENETHINK	*entk ;
	ACTION		*act ;
	int			i, life, faint, zone ;
	int		act_num ;

	work->hold = hold ;
	work->return_pos = *pos ;
	work->return_dir = dir ;
	work->flag = flag ;
	work->gameover_delay = 0 ;

	entk = &work->enethink ;
	act = &work->action ;

	entk->character = &work->chkwork ;
	
	work->chkwork.cnct = cnct ;

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
	ENE_SetSenseParam( &(entk->sense), 0, DEF_EYE_RANGE, entk->sense.eye_s_s[0], DEF_HEARING, DEF_SMELL ) ;
	ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerFindPos, &GM_PlayerAddress, NULL, &GM_PlayerHzxID ) ;

	/* コントロールの初期化 */
	entk->ctrl = ctrl ;
	GM_InitRadarControl( &entk->sense.rctrl, &entk->ctrl->mov, RADAR_VISIBLE|RADAR_SIGHT, -1 );
	GM_RadarSetVRange( &entk->sense.rctrl, RADAR_VRANGE_UPPER , RADAR_VRANGE_LOWER );
	GM_SetZoneIntrpt( &entk->z_intrpt, ctrl, 0, 0, ZONE_INTRPT_ENEMY ) ;
	GM_PutZoneIntrpt( &entk->z_intrpt ) ;

	/* オブジェクトの初期化 */
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

	for ( i = 0 ; i < EM_STANDARD_MAX ; i++ ){
		CheckEneMotionArray[ i ] = MAX_USHOLD_MOTION+i ;
	}
	i=0 ;
	while ( use_standard_motion_num[i][0] >= 0 ){
		CheckEneMotionArray[ use_standard_motion_num[i][0] ] = use_standard_motion_num[i][1] ;
//printf("%d: [%d]->[%d]\n",i,use_standard_motion_num[i][0],use_standard_motion_num[i][1] ) ;
		i++ ;
	}
	AT_SetActionMotion( act, NULL, (int *)CheckEneMotionArray ) ;

	AT_SetMode( act, ENE_ActStandStill ) ;

	act_num = 0 ;
	act->c_motion_num[0] = act_num ;
	GM_ConfigObjectAction( act->body, 0, USHOLD_idle_4, 0, MOTION_MASK_FULL, 0 );
/*
{
	FVECTOR		*old_abs ;
	
	old_abs = body->m_ctrl->old_abs_rots ;
	for ( i = 0 ; i < 21 ; i++ ){
		*old_abs = *mt_rots ;
	}
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
//	ENE_InitRouteNavi( NULL, 0, 0 ) ;
	ENE_InitZoneNavi( &work->zonenavi ) ;
	ENE_InitTargPoint( &entk->trgpoint ) ;
	ENE_InitEneThink( entk, ctrl, NULL, &work->zonenavi, act ) ;
	KR_FMatToFvec( &(BODYWORLD( body, HUMAN21_KOSHI )), &ctrl->mov ) ;
	ctrl->rot.vy = dir ;
	ctrl->turn.vy = ctrl->rot.vy ;

//	ctrl->turn.vy = _FVecTrgDir2( &ctrl->mov, &GM_PlayerPosition ) ; ;

    GM_ConfigControlHzxHeight( ctrl, 750.0F, pos->vy ) ;
	GM_ConfigControlMapID( ctrl ) ;

	/* 床の高さ */
	{
		float	levels[2] ;
		int levelflag ;

	    levelflag = HZX_LevelHazardCheck( (HZX_GROUP_ID)ctrl->hzx_id, &ctrl->mov,
					ctrl->hzx_check_type, ctrl->flr_flag ) ;
		if ( levelflag != 0 ) {
			HZX_GetLevelHeight( levels ) ; 
			if ( !( 1 & levelflag ) ) levels[ 0 ] = 0.0F ;
			ctrl->hzx_base = levels[ 0 ] ;
printf("checkene:ctrl->hzx_base[%f]\n",ctrl->hzx_base);
		} else {
			ctrl->hzx_base = -17000 ;
printf("WARNING: No Floor [%f]\n",ctrl->hzx_base);
		}
	}

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

	entk->def_pos = *pos ;	/* 戻る位置 */
	entk->def_mapbit = ctrl->hzx_id ;	/* 戻る位置 */
	entk->tmp_buff2[0] = dir ;/* 戻る方向 */

	/* 死体にならない */
	AT_SetType( act, (ENE_TYPE_USHOLD|ENE_TYPE_NO_PBREAK) ) ;

	if ( *work->flag & (HLD_STATUS_CAMERAMN) ) {
		AT_SetType( act, (ENE_TYPE_CAMERA_USHOLD) ) ;
	}

	/*思考開始場所セット */
	ENE_CheckStart( entk, mode, event, trg_pos ) ;
	work->action.CheckDamage = ENE_EnemyDamagePad ;

#ifdef DEBUG_MODE
if(0){
	extern void *NewEyeView( FMATRIX *world,
			int len, int range, int upper, int under, COMMANDER *com, int *status2 ) ;
	NewEyeView( &BODYWORLD( entk->act->body, HUMAN21_ATAMA ), entk->sense.eye_s, 
		entk->sense.eye_r, -UPPER_EYE_SIGHT, -UNDER_EYE_SIGHT, entk->com, &entk->status2 ) ;
}
#endif

	if ( event & (HLD_EVENT_DAMAGE|HLD_EVENT_CAPTURE) ) {
		TargetCopy( entk, bodyp ) ;
		work->action.status |= ACT_STATUS_EYE_CLOSE ;
		Act( work );
	}

	return (0);
}

void *NewCheckEnemy( body, weapon, pos, dir, mt_rots, name, flag, mode, event, trg_pos, cnct, ctrl, bodyp, hold )
OBJECT	*body ;
OBJECT	*weapon ;
FVECTOR	*pos ;
int		dir ;
FVECTOR	*mt_rots ;
int		name ;
int		*flag ;
int		mode ;
int		event ;
FVECTOR	*trg_pos ;
CNCTOBJ	*cnct ;
CONTROL *ctrl ;
HOLDBODYPARAM *bodyp ;
HOLD_ENE *hold ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body, weapon, pos, dir, mt_rots, name, flag, mode,
				event, trg_pos, cnct, ctrl, bodyp, hold ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

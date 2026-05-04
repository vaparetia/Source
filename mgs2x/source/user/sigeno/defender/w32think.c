//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	w32think.c
	拠点防御イベント兵
	w25 w32 で使用
	2001/04/01 K.Sigeno
	$Id: w32think.c,v 1.1.1.3 2002/11/19 11:49:11 Yoshizawa1 Exp $
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
#include	"../attacker/sig_conv.x"
#include	"../conv/conv_act.x"

#include	"eve_w32.h"
#include	"../../morita/emma/include/emma_com.h"

#include "BP_Misc.h"

//#define SUPER_PEEK (1)
enum {
	SP_NONE,
	SP_READYGUN ,
	SP_SHOOTGUN ,
	SP_MOVE_RUN ,
	SP_SQUATHIDE,
	SP_DISCOVERY,
	SP_RELOAD,
	SP_RELOAD_WINDOW, /*窓際でリロード*/
	SP_GRD_HIGH,
	SP_GRD_LOW,
	SP_PEEP_WINDOW,
	SP_PEEP_R,
	SP_PEEP_L,
	SP_SUPER_PEEP_L ,
	SP_MEDICATION
};
//EM_gbs_ak_nom_fire_peep_w25
enum {
	TH3_ATTACK_NEAR,TH3_WAIT_CHASE,TH3_ZONE_CHASE,
	TH3_DIRECT_CHASE,TH3_ESC_ZONECHASE,TH3_ESC_DIRECT,
	TH3_ESC_WAIT,TH3_DEFENSE,TH3_DISCOVERY,TH3_ATTACK_RELOAD,
	TH3_GRD_HIGH,TH3_MEDICATION
	};

#define AK_INTER 3
#define SIG_AT_SetMode(x,y) AT_SetMode( x,(ACTIONMODE) y ) 

extern int	ENE_ZoneTraceIntrpt( ZONENAVI *,TRGPOINT *,CONTROL *,int);
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern void *NewMagicGrd( FVECTOR * ,FVECTOR * ,float ,int,int ,short);
extern void ENE_ActMedication( ACTION * , int) ;

void W32AlertStart(ENETHINK * ) ;
void W32EscapeStartInThink(ENETHINK *) ;
void W32EscapeStart(ENETHINK *) ;


static void w25_ActPeekEnd( ACTION *, int  );

static void SetHomingStatusEmaSight(ENETHINK *entk){
	entk->hom.status |= HOMING_EMA_IN_SIGHT ;
}


#define W32_TURN_TIME (DIRECT_TICK(60))
static void w32_SetLookAroundDir(ENETHINK *entk)
{
	int count ;
	count = entk->count3%(W32_TURN_TIME*3) ;
	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;

	if ( count  < W32_TURN_TIME ) {
		entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
	} else if ( count < (W32_TURN_TIME*2) ) {
		entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
	}
}


#if 0
static void SetAimPosTrgp(ENETHINK *entk)
{
	entk->status2 |= ENE_STATUS2_AIM_GUN ;
	entk->act->aim_pos = entk->trgpoint.pos ;
}
#endif
#define LEG_BOTH	(MOTION_MASK_LEG_R|MOTION_MASK_LEG_L)

static void MotLayer_Leg(ACTION *act){
	AT_SetAction( act, 1, EM_idle_holdup, 0, (LEG_BOTH), ACT_INTERP_DEF ) ;
	act->body->m_ctrl->mt3_ctrl[ 1 ].flag |= MT3_ACTIVE ;
}

/*銃ゆらし付きＰＡＤ射撃*/
/*静止しての攻撃なので狙いが正確*/

static void StillShoot( ENETHINK *entk ,int mode)
{

#ifdef DEBUG_NO_SHOT
	return ;
#endif
	if(SIG_CheckStealthStatus(entk)){
		return ;
	}


	/*乱数で発砲*/
	if ( !(entk->count3 % AK_INTER) ) {
		if ( (irnd()>>8)%4  ) {
			return;
		}
		entk->act->pad = SP_SHOOTGUN ;
	}
}



static void GoNearAttack(ENETHINK *entk)
{
	entk->think3 = TH3_ATTACK_NEAR ;
	entk->count3 = 0 ;
}

static void SetNextAttackPos(ENETHINK *entk){
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;

	eve_w->attack_num++;
	if(eve_w->attack_num>eve_w->attack_max) eve_w->attack_num = 0;
	ENE_SetTrgpPosMap( &(entk->trgpoint),&eve_w->attack_pos[eve_w->attack_num], eve_w->attack_map[eve_w->attack_num] );
}
static int CheckNearAttackPos(ENETHINK *entk){
	float dis = 10000000.0f,tmp;
	int i,near_num ;
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;
	near_num = eve_w->attack_max+1 ;
	for(i=0;i<=eve_w->attack_max;i++){
		tmp = GV_VecLen3F2(&eve_w->attack_pos[i],entk->pl_eyei.pos);
		if(tmp < dis){
			dis = tmp ;
			near_num = i;
		}
	}
	return near_num ;
}

static float VecLen2d(FVECTOR *vec1 ,FVECTOR *vec2){
	FVECTOR tmp1,tmp2 ;
	
	tmp1 = *vec1 ;
	tmp2 = *vec2 ;

	tmp1.vy = tmp2.vy = 0.0f ;	
	return GV_VecLen3F2(&tmp1,&tmp2);
}


static void SetNearAttackPos(ENETHINK *entk){
	int res ;
	EVENT_W32	*eve_w ;

	eve_w = (EVENT_W32 *) entk->character ;
	res = CheckNearAttackPos(entk) ;

	if(res <= eve_w->attack_max){
		eve_w->attack_num = res ;
	}
	ENE_SetTrgpPosMap( &(entk->trgpoint),&eve_w->attack_pos[eve_w->attack_num], eve_w->attack_map[eve_w->attack_num] );
}
static void SetAttackPos(ENETHINK *entk){
	EVENT_W32	*eve_w ;

	eve_w = (EVENT_W32 *) entk->character ;
	if(eve_w->mode <= DEF_MODE_W32_C ){
		SetNextAttackPos(entk);
	}else {
		SetNearAttackPos(entk);
	}
}
/* 覗き姿勢用 特種アジャスト*/
static void w25_ActReadyGun( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	/*アジャスト*/

	AT_SetActStatus( act, ACT_STATUS_STAND| ACT_STATUS_ADJ_X ) ;
//	AT_SetActStatus( act, ACT_STATUS_SPHERE_100 ) ;

//	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;
	}

	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
	}
}
#define CORRECT_SPEED	(20.0F)
static void PosCorrect(ACTION *act){
extern float _RsinF( int ) ;
extern float _RcosF( int ) ;
	if ( act->tmp_dir >= 0 )
	{
		act->ctrl->step.vx +=
			CORRECT_SPEED * _RsinF( (int)act->tmp_dir ) ;
		act->ctrl->step.vz += 
			CORRECT_SPEED * _RcosF( (int)act->tmp_dir ) ;
	}
	act->tmp_dir = -1 ;
}
static void w25_ActPeekStart( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	/*アジャスト*/
	AT_SetActStatus( act, ACT_STATUS_STAND| ACT_STATUS_ADJ_X ) ;
	AT_SetActStatus( act, ACT_STATUS_SPHERE_100 ) ;

//	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
		ACT_INTERP_M ) ;
		MotLayer_Leg(act) ;
	}

	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
	}

	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		switch(act->keep_mot){
			case EM_gbs_ak_nom_peek_r_start : 
				act->keep_mot = EM_gbs_ak_nom_peek_r_fire ;
				break ;
			case EM_gbs_ak_nom_peek_l_start : 
				act->keep_mot = EM_gbs_ak_nom_peek_l_fire ;
				break ;
			case EM_gbs_ak_nom_peek_l_start_w25 : 
				act->keep_mot = EM_gbs_ak_nom_peek_l_fire_w25 ;
				break ;
//			act->keep_mot = EM_gbs_ak_nom_fire_peep_w25 ;
		}
		SIG_AT_SetMode( act, w25_ActPeekEnd ) ;
		return ;
	}
	PosCorrect(act);
}

static void w25_ActPeekEnd( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	/*アジャスト*/
	AT_SetActStatus( act, ACT_STATUS_STAND| ACT_STATUS_ADJ_X ) ;
	AT_SetActStatus( act, ACT_STATUS_SPHERE_100 ) ;
	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}

	if(act->keep_mot == EM_gbs_ak_nom_peek_l_fire_w25){
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_W25_SUPER_PEEP ) ;
	}else {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
	}

	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
		ACT_INTERP_M ) ;
		MotLayer_Leg(act) ;
	}

	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
			return ;
		}
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
	}
	PosCorrect(act);
}

static void w32_ActReadyGun( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	/*アジャスト*/
//	AT_SetActStatus( act, ACT_STATUS_STAND| ACT_STATUS_ADJ_X ) ;
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	if ( act->CheckDamage( act ) ) {
		return ;
	}


	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;
	}

	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
	}
}
#define	GRD_HIGH_THR_TIME	DIRECT_TICK(96)

static void ActGrdThrHighW32( act, time )
ACTION	*act ;
int		time ;
{
//	/*モーションの移動量が大きすぎるのでリセット*/
//	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
	FVECTOR pos;
	act->mot_speed_correct = 0.2F;
	act->ctrl->step = DG_ZeroVector ;

	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

   if ( ((BP_IsPAL()!=TRUE) && ((time > 5 ) &&( time < 149)) ) ||
        ((BP_IsPAL()==TRUE) && ((time > 5 ) &&( time < 124)) ) )
   {
		AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_GUN_FREE ) ;
	}
	if ( act->CheckDamage( act ) ) {
//		act->act_end = 1 ;
		return ;
	}
	if ( act->dir >= 0 ) {
//		act->ctrl->turn.vy = act->dir ;
	}else	{
		//printf("DIR -1 TO STILL\n");
		//printf("AT_ACT LINE %d\n",__LINE__);
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( time == 0 ) {
//printf("GRD VOICE CALL\n");
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),
			ENE_BODYTRGSIZE_STAND );
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;
//		MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE*1.5f ) ;

	}

	if ( time == 12 ) {
//		GM_SeSetMode( SD_V_C04MAKI+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
		SIG_SetSpeak( SD_V_C04MAKI, act ) ;
	}
#if 1
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->act_end = 1 ;
			return ;
		}
	}
#endif

//	if ( time == 198 ) {
//	if ( time == 80 ) {
	if ( time == GRD_HIGH_THR_TIME ) {
		GV_MatToVec(&BODYWORLD( act->body, HUMAN21_MIGI_TE ),&pos);
		NewMagicGrd( &pos ,&act->target_pos ,4000.0f ,PLAYER_SIDE,-1 ,0);
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
}
/*リロードを特別にするかも*/
static void ActReloadW25( act, time )
ACTION	*act ;
int		time ;
{

//	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	AT_SetActStatus( act, ACT_STATUS_SPHERE_100 ) ;

	if ( act->CheckDamage( act ) ) {
		act->sw->magg = 1 ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		//printf("DIR -1 TO STILL\n");
		//printf("AT_ACT LINE %d\n",__LINE__);
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
		ACT_INTERP_M ) ;
		MotLayer_Leg(act) ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->magg = 1 ;
			return ;
		}
	}

	if ( time == DIRECT_TICK(19) ) act->sw->magg = 4 ; /*sute*/
	if ( time == DIRECT_TICK(66) ) act->sw->magg = 2 ;	/*temoti*/
	if ( time == DIRECT_TICK(112) ) act->sw->magg = 1 ;	/*juutuke*/
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->sw->magg = 1 ;
		act->act_end = 1 ;
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	PosCorrect(act);

}
static void ActGrdThrLowW32( act, time )
ACTION	*act ;
int		time ;
{
	FVECTOR pos;
	act->mot_speed_correct = 0.2F;
	act->ctrl->step = DG_ZeroVector ;
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if(( time > DIRECT_TICK(26) ) &&( time < DIRECT_TICK(347) )){
		AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_GUN_FREE ) ;
	}
	if ( act->CheckDamage( act ) ) {
//		act->act_end = 1 ;
		return ;
	}
	if ( act->dir >= 0 ) {
//		act->ctrl->turn.vy = act->dir ;
	}else	{
		//printf("DIR -1 TO STILL\n");
		//printf("AT_ACT LINE %d\n",__LINE__);
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),
			ENE_BODYTRGSIZE_STAND );
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;
//		MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE*1.5f ) ;
		MotLayer_Leg(act) ;
	}

	if ( time == DIRECT_TICK(112) ) {
printf("GRD VOICE CALL\n");
//		GM_SeSetMode( SD_V_C04MAKI+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
		SIG_SetSpeak( SD_V_C04MAKI, act ) ;

	}
#if 1
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->act_end = 1 ;
			return ;
		}
	}
#endif

//	if ( time == 198 ) {
//	if ( time == 80 ) {
	if ( time == DIRECT_TICK(205) ) {
		GV_MatToVec(&BODYWORLD( act->body, HUMAN21_MIGI_TE ),&pos);
		NewMagicGrd( &pos ,&act->target_pos ,800.0f ,PLAYER_SIDE,-1 ,0);
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	PosCorrect(act);
}

/*静止して銃上段発射*/
static void w32_ActShootGun( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;



	/*アジャスト*/
//	AT_SetActStatus( act, ACT_STATUS_SHOT_PIKU | ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	AT_SetActStatus( act, ACT_STATUS_SHOT_PIKU | ACT_STATUS_STAND  ) ;

	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		//printf("DIR -1 TO STILL\n");
		//printf("AT_ACT LINE %d\n",__LINE__);
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
#if 0
	if ( !((irnd()>>8)%8)  ) {
		ENE_ShootBullet( act,ENE_BULLET_NOWALL ,NULL ) ;
	}else {
		ENE_ShootBullet( act,ENE_BULLET_NORMAL ,NULL ) ;
	}
#else
		ENE_ShootBullet( act,ENE_BULLET_NORMAL ,NULL ) ;
#endif

	*act->sw->bullet = *act->sw->bullet+1 ;
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}
	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
}
/*しゃがみ待機*/
static void w32_ActSquatReady( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	/*アジャスト*/
//	AT_SetActStatus( act, ACT_STATUS_STAND| ACT_STATUS_ADJ_X ) ;
	AT_SetActStatus( act, ACT_STATUS_SHAGAMI ) ;
	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;
	}
	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
	}
}

static int W32_AlertCheckPad( act )
ACTION	*act ;
{
	switch ( act->pad ) {
		case SP_READYGUN :
//printf("SP_READYGUN \n");
			act->keep_mot = EM_kamae_gun_high ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, w32_ActReadyGun ) ;
			return 1 ;
		break ;
		case SP_SHOOTGUN :
			act->keep_mot = EM_kamae_gun_high ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, w32_ActShootGun ) ;
			return 1 ;
		break ;
		case SP_SQUATHIDE :
			act->keep_mot = EM_squat_hide_idle ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, w32_ActSquatReady ) ;
			return 1 ;
		break ;
		case SP_DISCOVERY :
			act->keep_mot = ENE_DiscoveryMotion(act) ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, SIG_ActDiscovery ) ;
			return 1 ;
		break ;
		case SP_RELOAD :
			act->keep_mot = EM_reload ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, ActStandReload ) ;
//			SIG_AT_SetMode( act, ActReloadW25 ) ;
			return 1 ;
		break ;
		case SP_RELOAD_WINDOW :
			act->keep_mot = EM_reload ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, ActReloadW25 ) ;
			return 1 ;
		break ;


		case SP_MOVE_RUN :
			act->keep_mot = EM_run_atk;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, AT_ActMoveRun);
			return 1 ;
		break ;
		case SP_GRD_HIGH :
			act->keep_mot = EM_gm_fire_high;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, ActGrdThrHighW32);
			return 1 ;
		break ;
		case SP_GRD_LOW :
			act->keep_mot = EM_gm_fire_low;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, ActGrdThrLowW32);
			return 1 ;
		break ;
		case SP_PEEP_WINDOW :
			act->keep_mot = EM_gbs_ak_nom_fire_peep_w25 ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, w25_ActReadyGun ) ;
			return 1 ;
		break ;
		case SP_PEEP_R :
			act->keep_mot = EM_gbs_ak_nom_peek_r_start ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, w25_ActReadyGun ) ;
			SIG_AT_SetMode( act, w25_ActPeekStart ) ;
			return 1 ;
		break ;
		case SP_PEEP_L :
			act->keep_mot = EM_gbs_ak_nom_peek_l_start ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, w25_ActPeekStart ) ;
			return 1 ;
		break ;
		case SP_SUPER_PEEP_L :
			act->keep_mot = EM_gbs_ak_nom_peek_l_start_w25 ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, w25_ActPeekStart ) ;
			return 1 ;
		break ;
		case SP_MEDICATION :
			act->keep_mot = EM_legl_morph_legl ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, ENE_ActMedication ) ;
			return 1 ;
		break ;

#if 0
		case SP_STOMP :
			act->keep_mot = EM_tng_p90_nom_stamp ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActNomStomp);
			SIG_AT_SetMode( act, AT_ActAcrobat ) ;
			return 1 ;
		break ;
#endif
	}
	return 0 ;
}

/**/


static void SetAimPosEyei(ENETHINK *entk ,int mode)
{
//	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
	entk->status2 |= ENE_STATUS2_AIM_GUN ;
	entk->act->aim_pos = *entk->pl_eyei.pos ;
}


static int SetCoorectDir(ENETHINK *entk,FVECTOR *trg){
	FVECTOR	testpos,subvec;
	float len ;
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;

	testpos = *trg ;
	testpos.vy = entk->ctrl->mov.vy;
	_sceVu0SubVector( &subvec, &testpos, &entk->ctrl->mov ) ;
	len = GV_VecLen3F( &subvec );
	if( len < 25.0F) {
		return -1;
	}else {
		return GV_VecDir2( &subvec );
	}
}



static	void	Think3_Medical( entk )
ENETHINK	*entk ;
{
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_MEDICATION ;
//		UNSET_FLAG( entk->sw.radio, SW_FLAG_BREAK ) ;
	}
	if ( entk->act->act_end ) {
#if 0
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_DAMAGE ) ;
		CLEAR_FLAG( entk->iknow_flag ) ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
#else
		if(eve_w->find_flag & W32_ESC_POS){
			W32EscapeStart(entk) ;
		}else {
			W32AlertStart(entk) ;
		}
#endif
		return ;
	}
	entk->act->dir = entk->ctrl->turn.vy ;
	entk->count3 ++ ;
	return ;
}

static	void Think3_AttackReload( entk )
ENETHINK	*entk ;
{
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;


	if(eve_w->fix_dir < 0){
		entk->act->dir = entk->ctrl->turn.vy ;
	}else {
		entk->act->dir = eve_w->fix_dir ;
	}

	if(eve_w->mode == DEF_MODE_W25_B){
		entk->act->pad = SP_RELOAD_WINDOW ;
		entk->act->tmp_dir = SetCoorectDir(entk,&entk->trgpoint.pos);
	}else {
		entk->act->pad = SP_RELOAD ;
		entk->act->tmp_dir = -1;
	}

	if(entk->count3 > DIRECT_TICK(180)){
		entk->act->act_end = 1;
	}

	if ( entk->act->act_end ) {
		/*攻撃モード*/
		entk->think3 = TH3_DEFENSE ; 
		entk->count3 = 0 ;
		return ;
	}
//	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}
static	void Think3_GrdAttack( entk )
ENETHINK	*entk ;
{
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;


	entk->act->dir = entk->ctrl->turn.vy ;
	entk->act->tmp_dir = 
		SetCoorectDir(entk,&entk->trgpoint.pos);
	if(entk->count3 ==0){
		entk->act->target_pos = GM_PlayerPosition ;
	}

	if(entk->count3 > DIRECT_TICK(400)){
		entk->act->act_end = 1;
	}
	if(
	(eve_w->mode == DEF_MODE_W25_B)
	||(eve_w->mode == DEF_MODE_W25_C)
	){
		/*後半は狙いを正確にする*/
		entk->act->target_pos = GM_PlayerPosition ;
		entk->act->pad = SP_GRD_LOW ;
	}else {
		entk->act->pad = SP_GRD_HIGH ;
	}

	if ( entk->act->act_end ) {
		/*攻撃モード*/
		entk->think3 = TH3_DEFENSE ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}

static void Think3_ZoneChaseEyei( entk )
ENETHINK	*entk ;
{
//	int reach,rank_dis = 1000,aim_check = 0;
	int zone_ch;
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;


	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint)) ;

	entk->act->dir = entk->ctrl->turn.vy ;
	SetAimPosEyei(entk,0);

	if(entk->count3 == 0){
		if(eve_w->st_flag & W32_ST_CONTACT){
			/*通報フラグオンなら*/
//			eve_w->find_flag |= W32_FIND ;
		}
	}
#if 0
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			/*めくら時の処理*/
			if( at_thk->zone_dis > entk->at_com->chasedis+1000)
			{
				entk->act->pad = SP_READYGUN;
				entk->count3 =0;
				entk->think3 =TH3_WAIT_CHASE;
				entk->act->dir = entk->ctrl->turn.vy ;
				return ;
			}
		}
	}
#endif
	entk->act->pad = SP_MOVE_RUN ;
	/*現在地アドレス更新*/
	if(!(entk->count3%(DIRECT_TICK(30)))) {
		entk->znavi->this_addr = 
		HZX_GetAddress( entk->ctrl->hzx_id, 
			&( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;
	}
	/*手前が近けりゃ待機状態*/
	/*目標が近けりゃ攻撃 */
	if(
	( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )
	&&(entk->pl_eyei.dis < (entk->sense.eye_s-1000))
	){
		SetAimPosEyei(entk,0);
		GoNearAttack(entk);
		return ;
	}
/*ゾーン追跡*/
//intrp
/*
return 0 ; ゾーン追跡中
return -1 ; 到達
return -2 進入禁止
return 1 ルート遮断
*/
	zone_ch =
		ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint),
		entk->ctrl, entk->count3 ) ;
	if(
	(zone_ch == -2)||(zone_ch == 1)
	){
		entk->act->pad = SP_READYGUN;
		entk->count3 =0;
//		entk->think3 =TH3_WAIT_CHASE;
		entk->think3 =TH3_ATTACK_NEAR;
		entk->act->dir = entk->ctrl->turn.vy ;
		return ;
	}else if(zone_ch == -1 ){
//	}else if(0){
		if(100.0f < VecLen2d(&entk->znavi->flore_pos, &entk->trgpoint.pos) ){
//dis direct
			entk->count3 =0;
			entk->think3 = TH3_DIRECT_CHASE;
			entk->act->dir = entk->ctrl->turn.vy ;
			return ;
		}else {
/*既に到達*/
			entk->act->pad = SP_READYGUN;
			entk->count3 =0;
	//		entk->think3 =TH3_WAIT_CHASE;
			entk->think3 =TH3_ATTACK_NEAR;
			entk->act->dir = entk->ctrl->turn.vy ;
			return ;
		}
	}
	if((entk->count3%(DIRECT_TICK(12))==0)
	){
		if(
		(entk->count3 < DIRECT_TICK(30)) /*動きはじめはゆっくり*/
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
			){
				SetMoveMode(entk);
			}else {
				if(entk->act->move_s != MoveAttackRun ){
					entk->act->tmp_time = 12 ;
				}
				entk->act->move_s = MoveRun ;
			}
		}
	}
/*目標に近くなれば 強制的にゆっくり歩き*/
	if(entk->act->move_s == MoveAttackRun) {
		entk->act->dir = entk->trgpoint.dir ;
	} else {
	/*体はプレイヤ方向移動は目的ゾーン方向*/
		entk->act->tmp_dir = entk->trgpoint.dir ;
		entk->act->dir = entk->pl_eyei.dir ;
	}

	if(
	(eve_w->mode >= DEF_MODE_W25_A)
	&&(eve_w->mode <= DEF_MODE_W25_C)
	){
//		SIG_RandShoot( entk ) ;
		SIG_BasicShoot(entk,&GM_PlayerFindPos,ENE_BULLET_NORANDAM);
	}
	entk->count3 ++ ;
}



/*座標追跡 プレイヤ直接*/
static	void	Think3_DirectChaseEyei( entk )
ENETHINK	*entk ;
{
	int		reach ;
	int	zone_ch ;
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;

	SetAimPosEyei(entk,0);
	entk->act->dir = entk->ctrl->turn.vy ;
#if 0
	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
#else
	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint)) ;
#endif

#if 0
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


	/*追跡のための方向設定と距離判定*/
	ENE_DirectTrace( &(entk->trgpoint), &(entk->znavi->flore_pos), (0) ) ;
	/*目標が近けりゃ攻撃 */
	if(
	( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )
	&&(entk->pl_eyei.dis < (entk->sense.eye_s-1000))
	){
		SetAimPosEyei(entk,0);
		entk->act->dir = entk->ctrl->turn.vy ;
		GoNearAttack(entk);
		return ;
	}
	/*距離外*/
	/*32フレON/OFF 100000*/
	if( entk->count3 & 0x20){
		/*現在値 計算し直し*/
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), -1 ) ;
	}
	/* いつまでも直線じゃいられない */
	entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;
	reach = HZX_ReachTo( entk->znavi->this_addr, entk->trgpoint.addr );
/*
return 0 ; ゾーン追跡中
return -1 ; 到達
return -2 進入禁止
return 1 ルート遮断
*/

	if(
	( reach > HZX_INDIRECT_REACH )
	){
	/*ゾーン到達してない*/
		zone_ch =
			ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint),
			entk->ctrl, entk->count3 ) ;
		if(zone_ch == 0){
			entk->think3 = TH3_ZONE_CHASE ;
			entk->count3 = 0 ;
			return ;
		}
	}
	if(HZX_Zone2( *entk->pl_eyei.addr) !=255){
		/*追跡可能*/
		entk->act->pad = SP_MOVE_RUN ;
		SetMoveMode(entk);
		entk->act->dir = entk->trgpoint.dir ;
	}else {
		entk->act->pad = SP_READYGUN ;
		entk->act->dir = entk->pl_eyei.dir ;
	}
	if((eve_w->mode >= DEF_MODE_W25_A)
	&&(eve_w->mode <= DEF_MODE_W25_C)){
//		SIG_RandShoot( entk ) ;
	SIG_BasicShoot(entk,&GM_PlayerFindPos,ENE_BULLET_NORANDAM);
	}

	entk->count3 ++ ;
}

static void Think3_AttackNear( entk )
ENETHINK	*entk ;
{
	int zone_ch;

//	FVECTOR		testpos;

	entk->act->pad = SP_READYGUN;
	SetHomingStatusEmaSight(entk) ;

	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint)) ;
	SetAimPosEyei(entk,0);
//	entk->act->dir = entk->pl_eyei.dir ;
//	entk->act->dir = entk->trgpoint.dir ;
	entk->act->dir = entk->pl_eyei.dir ;

//	printf("entk->count3 %d sight %d\n",
//		entk->count3,entk->pl_eyei.sight);

	/*割り込みアクション終了時にthinkをリスタート*/
	if ( entk->act->act_end ) {
printf("THINK NEAR RESET!!!\n");
		entk->count3 = 0 ;
		return ;
	}
	/* 目標が見えなければ追跡*/
	/* あわてて追跡しない */
	if(
	(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN )
//	(0)
	){
/*
return	 0 ; ゾーン追跡中
return	-1 ; 到達
return	-2 進入禁止
return	 1 ルート遮断
*/
		zone_ch =
			ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint),
			entk->ctrl, entk->count3 ) ;

		if( zone_ch == 0 )
		{
			/*ゾーン到達してない*/
			entk->think3 = TH3_ZONE_CHASE ;
			entk->count3 = 0 ;
			return;
		}
	}
	StillShoot(entk , 0) ;
//	entk->bullet = 0 ;
	entk->count3 ++ ;
//	StillShoot( entk  ,AT_SHT_NO_RELOAD);
}


#define	DEF_POS_SHIFT	(200.0F)
#define	SUPER_PEEK_DIS	(3000)

static void Think3_AttackStand( entk ) 
ENETHINK	*entk ;
{
	FVECTOR		testpos;
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;

//2001.04.18
	if((eve_w->mode >= DEF_MODE_W32_A)&&(eve_w->mode <= DEF_MODE_W32_C)){
		/*エマ狙撃イベント*/
		SetHomingStatusEmaSight(entk) ;
	}else {
		/*タイトロープ*/
//		if((COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)){
		if(eve_w->find_flag & W32_FIND ){
			/*プレイヤ目撃状態*/
		}else {
			FVECTOR checkpos;
			checkpos = entk->com->player_lastpos;
//			PosBox(&(testpos),250.0f ,NULL );
			/*爆発などにより、未確認での危険モード*/
			/*怪しい方向を向いてきょろきょろ*/
			if ( (entk->count3 ) > 0 && (entk->count3 ) < (DIRECT_TICK(60))  ) {
				entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
				entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
			} else if ( (entk->count3 > (DIRECT_TICK(60))) && (entk->count3  < DIRECT_TICK(120))) {
				entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
				entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
			}
//			entk->act->dir = entk->pl_eyei.dir ;
			entk->act->dir = GV_VecDir2FromTo( &entk->ctrl->mov, &checkpos ) ;
			entk->act->pad = SP_READYGUN;
			entk->count3++;
			return ;
		}
	}


	if(eve_w->mode == DEF_MODE_W25_B){
		/*小窓部屋*/
		testpos = entk->trgpoint.pos ;
		if(entk->pl_eyei.pos->vx > entk->ctrl->mov.vx){
			entk->act->pad = SP_PEEP_R ;
			testpos.vx -= DEF_POS_SHIFT ;
		}else {
			if(entk->count3 == 0){
				if(
				(entk->at_com->Pl_StayTime > DIRECT_TICK(60*3))
				&&(entk->pl_eyei.dis>SUPER_PEEK_DIS)
				){
					entk->act->pad = SP_SUPER_PEEP_L ;
				}else {
					entk->act->pad = SP_PEEP_L ;
				}
			}else {
				if((entk->act->keep_pad == SP_SUPER_PEEP_L)||(entk->act->keep_pad == SP_PEEP_L)){
					entk->act->pad = entk->act->keep_pad ;
				}else {
					if(
					(entk->at_com->Pl_StayTime > DIRECT_TICK(60*3))
					&&(entk->pl_eyei.dis>SUPER_PEEK_DIS)
					){
						entk->act->pad = SP_SUPER_PEEP_L ;
					}else {
						entk->act->pad = SP_PEEP_L ;
					}
				}
			}
			testpos.vx += DEF_POS_SHIFT ;
		}
		entk->act->tmp_dir = 
			SetCoorectDir(entk,&testpos);
	}else if(eve_w->mode == DEF_MODE_W25_C){
		entk->act->pad = SP_PEEP_WINDOW ;
	}else {
		entk->act->pad = SP_READYGUN;
	}

	if(eve_w->mode == DEF_MODE_W25_C){
		SET_FLAG( entk->status2, ENE_STATUS2_AIM_NO_LIMIT);
	}

//	if(entk->count3 ==10){
//2001.04.18
	if(eve_w->mode <= DEF_MODE_W32_C){
		if(eve_w->find_flag & W32_FIND ){
			ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint)) ;
			SetAimPosEyei(entk,0);
#if 0
			EMA_CommandEneAttack( entk->ctrl->name ) ;
#else
/* For Japan */
/* 気絶時のサウンドフェード問題対処版 */
			if((entk->act->bodyp.life<=0)||(CheckNoActive(entk))){
				/*行動不能なのでNOP*/
			}else {
				/*攻撃中 エマに通報*/
				EMA_CommandEneAttack( entk->ctrl->name ) ;
			}
#endif
		}else {
			/*警戒しているが見つけていない*/
			ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
			SET_FLAG( entk->status2, ENE_STATUS2_AIM_GUN);
			entk->act->aim_pos = GM_PlayerFindPos ;
//			entk->act->pad = SP_AIM_POS ;
			/*キョロキョロ*/
			w32_SetLookAroundDir(entk) ;
			entk->act->dir = entk->trgpoint.dir ;
		}
	}else {
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		entk->act->aim_pos = GM_PlayerPosition ;
		if((eve_w->mode != DEF_MODE_W25_B)
		&&(eve_w->mode != DEF_MODE_W25_C)
		){
//printf("AIM POS SET!!!!!!!!!\n");
			SET_FLAG( entk->status2, ENE_STATUS2_AIM_GUN);
		}
	}
	ENE_DirectTrace( &(entk->trgpoint), &(entk->znavi->flore_pos), (0) ) ;

	if(eve_w->mode <= DEF_MODE_W32_C){
		entk->act->dir = entk->trgpoint.dir ;
	}else {
#if 0
		if(eve_w->fix_dir < 0){
#else
		if(1){
#endif
			entk->act->dir = entk->pl_eyei.dir ;
		}else {
			entk->act->dir = eve_w->fix_dir ;
		}
	}
	if(entk->count3 > DIRECT_TICK(30)){
		if((eve_w->mode >= DEF_MODE_W25_A)){
			if((COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)){
				int flag = ENE_BULLET_NORANDAM ;
/*壁貫通弾*/
				if((eve_w->mode == DEF_MODE_W25_B)){
#if 0
					if ( ((irnd()>>8)%10)==0  ) {
						flag |= ENE_BULLET_NOWALL ;
					}
#endif
					if(
//					(entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )
//					||((GM_PlayerStatus & PLAYER_BEYOND )&&(entk->pl_eyei.sight == EYE_INFO_SIGHT_OUT_HZD) )
					(1)
					){
						if((GM_PlayerStatus & PLAYER_BEYOND )&&(entk->pl_eyei.sight == EYE_INFO_SIGHT_OUT_HZD) ){
							if ( ((irnd()>>8)%8)==0  ) {
								flag |= ENE_BULLET_NOWALL ;
							}
						}
						SIG_BasicShoot(entk,&GM_PlayerFindPos,flag);
					}
				}else {
					SIG_BasicShoot(entk,&GM_PlayerFindPos,flag);
				}
			}
		}else if(eve_w->mode <= DEF_MODE_W32_C){
			/*エマイベント*/
			if(eve_w->find_flag & W32_FIND ){
				StillShoot(entk , 0) ;
			}
		}
	}
	if ( entk->bullet >= entk->max_bullet ) {

		entk->bullet = 0 ;
		entk->count3 = 0 ;
#if 1
		if((eve_w->mode >= DEF_MODE_W25_A)
		&&(entk->pl_eyei.dis < (entk->sense.eye_s+1000))
//		&&(eve_w->dis_rank == 0)
		){
#endif
			if(
			(eve_w->mode == DEF_MODE_W25_A)
//			||(eve_w->mode == DEF_MODE_W25_C)
//			||(Sig_GetRoute(entk->ctrl->hzx_id,entk->ctrl->addr,*entk->pl_eyei.addr)<=1)
			){
				entk->think3 = TH3_GRD_HIGH ; 
			}
			else {
				entk->think3 = TH3_ATTACK_RELOAD ; 
			}
		}else {
			entk->think3 = TH3_ATTACK_RELOAD ; 
		}
		return  ;
	}
	if(eve_w->mode <= DEF_MODE_W32_C){
		if(
		(!(eve_w->find_flag & W32_FIND ))&&
		(entk->count3> DIRECT_TICK(180))
		)
		{
			W32EscapeStartInThink(entk) ;
			return ;
		}
	}else {
#if 1
		if(
		((entk->count3 & 127)==60)
		&&(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN )
		){
			if(eve_w->attack_num != CheckNearAttackPos(entk)){
				entk->think3 = TH3_ESC_ZONECHASE ;
				if(eve_w->mode == DEF_MODE_W25_C){
					/*放尿兵はゾーン無視で追跡*/
					entk->think3 = TH3_ESC_DIRECT ;
				}

				SetAttackPos(entk) ;
				entk->count3 = 0 ;
				return ;
			}
		}
#else
		if(entk->count3> DIRECT_TICK(180)){
			W32EscapeStartInThink(entk) ;
			return ;
		}
#endif
	}
	entk->count3 ++ ;
}

static void Think3_Discovery( entk ) 
ENETHINK	*entk ;
{
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;

	entk->act->pad = SP_DISCOVERY;
	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint)) ;
	SetAimPosEyei(entk,0);
	entk->act->dir = entk->pl_eyei.dir ;
	if(( entk->act->act_end ) 
	||(entk->count3> DIRECT_TICK(120))
	)
	{

//		entk->act->pad = SP_MOVE_RUN ;
		entk->act->pad = SP_READYGUN;
		entk->act->move_s = MoveRun ;
		if(eve_w->find_flag & W32_ATTACK_POS){
			entk->think3 = TH3_ESC_ZONECHASE ;
			if(eve_w->mode == DEF_MODE_W25_C){
				/*放尿兵はゾーン無視で追跡*/
				entk->think3 = TH3_ESC_DIRECT ;
			}

			ENE_SetTrgpPosMap( &(entk->trgpoint),&eve_w->attack_pos[eve_w->attack_num], eve_w->attack_map[eve_w->attack_num] );
			eve_w->attack_num++;
			if(eve_w->attack_num>eve_w->attack_max) eve_w->attack_num = 0;
		}else {
			entk->think3 = TH3_ZONE_CHASE ;
			ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint)) ;
		}
		entk->count3 = 0 ;
		return ;
	}
	entk->count3 ++ ;
}
static void Think3_ZoneChaseEscape( entk )
ENETHINK	*entk ;
{
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;
#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&entk->trgpoint.pos ,250.0F ,&rgb );
	}
#endif
	if(entk->count3==0){
		/*目標をセット*/
	}
	entk->act->pad = SP_MOVE_RUN ;
	entk->act->dir = entk->ctrl->turn.vy ;

	/*目標位置からプレイヤが見えそうになければチェック*/
	if(eve_w->mode >= DEF_MODE_W25_A ){
		SetCoorectDir(entk,&entk->trgpoint.pos);
		if((entk->count3 & 15)==0){
			FVECTOR	sub ;
			float len;
			_sceVu0SubVector( &sub, &entk->trgpoint.pos,entk->pl_eyei.pos ) ;
			len = GV_VecLen3F( &sub );
			if(len > 4000.0F){
//(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN )
				if(eve_w->attack_num != CheckNearAttackPos(entk)){
					entk->think3 = TH3_ESC_ZONECHASE ;
					if(eve_w->mode == DEF_MODE_W25_C){
						/*放尿兵はゾーン無視で追跡*/
						entk->think3 = TH3_ESC_DIRECT ;
					}
					SetAttackPos(entk) ;
					entk->count3 = 0 ;
					return  ;
				}
			}
		}
	}
/*ゾーン追跡*/
/*ゾーン追跡＆終了判定*/
	if
	((ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) < 0 )
	||(SIG_CheckRIntrpt(entk)))
	{
		if(100.0f < VecLen2d(&entk->znavi->flore_pos, &entk->trgpoint.pos) ){
			if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
			/*ゾーン内の座標に近づく*/
			entk->think3 = TH3_ESC_DIRECT ;
			entk->count3 = 0 ;
			return ;
		}else {
/*既に到達*/
			entk->count3 = 0 ;
			entk->act->pad = SP_READYGUN ;
			entk->act->dir = entk->pl_eyei.dir ;
			entk->act->tmp_dir = entk->pl_eyei.dir ;
			if(eve_w->esc_cnt > 0){
				/*隠れモード*/
				ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint)) ;
				entk->think3 = TH3_ESC_WAIT ; 
			}else {
				/*攻撃モード*/
				entk->think3 = TH3_DEFENSE ; 
			}
			return ;
		}
	}

	entk->act->move_s = MoveRun ;
	entk->act->dir = entk->trgpoint.dir ;
//	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}

static int CheckDiscovery(ENETHINK *entk){
	EVENT_W32	*eve_w ;
//printf("CheckDiscovery\n");
	eve_w = (EVENT_W32 *) entk->character ;
	/*目撃した本人のみびっくり*/
	if((COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)){
		return 0 ;
	}

#if 1
	if((!(eve_w->old_find_flag & W32_FAKE))
	&&(eve_w->find_flag & W32_FAKE)
	){
		entk->think3 = TH3_DISCOVERY ;
		ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint)) ;
		entk->act->dir = entk->pl_eyei.dir ;
		entk->count3 = 0 ;
		return 1;
	}
#endif
	if(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ){
		return 0 ;
	}
	if(
	(!(eve_w->old_find_flag & W32_FIND))
	&&(eve_w->find_flag & W32_FIND)
	){
//ASSERT(0);
		entk->think3 = TH3_DISCOVERY ;
		ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint)) ;
		entk->act->dir = entk->pl_eyei.dir ;
		entk->count3 = 0 ;
		return 1;
	}
	return 0 ;
}

/*座標追跡 ゾーン内座標*/
static	void	Think3_DirectChasePos( entk )
ENETHINK	*entk ;
{
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;

	entk->act->pad = SP_MOVE_RUN ;
	entk->act->dir = entk->ctrl->turn.vy ;


//entk->trgpoint.h_dis
#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&entk->trgpoint.pos ,250.0F ,&rgb );
	}
#endif


#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&entk->trgpoint.pos ,250.0F ,&rgb );
		rgb.vx =   0;	rgb.vy =   0;rgb.vz =  255;
		PosBox(&(entk->znavi->flore_pos) ,250.0F ,&rgb );
	}
#endif

#if 0
	if( entk->count3%(30) ==0){
		int		reach ;
		/*現在値 計算し直し*/
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), -1 ) ;
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;
		reach = HZX_ReachTo( entk->znavi->this_addr, entk->trgpoint.addr );
		if(
		( reach > HZX_INDIRECT_REACH )
		){
		/*ゾーン到達してない*/
			entk->act->pad = SP_READYGUN ;
			entk->think3 = TH3_ESC_ZONECHASE ;
			entk->count3 = 0 ;
			return ;
		}
	}
#endif
	if(
	( ENE_DirectTrace( &(entk->trgpoint), 
//	&(entk->znavi->flore_pos), 100 ) < 0 ) 
	&(entk->znavi->flore_pos), 250 ) < 0 ) 
//	||(CheckToucheWallDir(entk,entk->trgpoint.dir ,256 ))
	){
		/*到着後*/
		/*目標座標に移動*/
		entk->act->mot_speed_correct = 0.0F;
		if(eve_w->mode == DEF_MODE_W25_B){
			entk->ctrl->mov.vx = entk->trgpoint.pos.vx ;
			entk->ctrl->mov.vz = entk->trgpoint.pos.vz ;
		}
		/*目標修正*/

		entk->act->pad = SP_READYGUN ;
		entk->act->dir = entk->pl_eyei.dir ;
		entk->act->tmp_dir = entk->pl_eyei.dir ;
		if(eve_w->esc_cnt > 0){
			/*隠れモード*/
			ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint)) ;
			entk->think3 = TH3_ESC_WAIT ; 
		}else {
			/*攻撃モード*/
			entk->think3 = TH3_DEFENSE ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	if(entk->trgpoint.h_dis > 600 ){
		entk->act->move_s = MoveRun ;
	}else {
		entk->act->move_s = MoveWalkGun ;
	}
//	entk->act->move_s = MoveCautionWalk ;
//	entk->act->move_s = MoveWalkGun ;
//EM_walk_light
	entk->act->dir = entk->trgpoint.dir ;
	entk->act->tmp_dir = entk->trgpoint.dir ;
//	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}
static void Think3_WaitEscape( entk ) 
ENETHINK	*entk ;
{
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;

	entk->act->pad = SP_SQUATHIDE;
//	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint)) ;
/*プレイヤの方向を向く*/
	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;

	SetAimPosEyei(entk,0);
	entk->act->dir = entk->pl_eyei.dir ;
	if(eve_w->esc_cnt <= 0){

		if(eve_w->find_flag & W32_ATTACK_POS){
			entk->think3 = TH3_ESC_ZONECHASE ;
			if(eve_w->mode == DEF_MODE_W25_C){
				/*放尿兵はゾーン無視で追跡*/
				entk->think3 = TH3_ESC_DIRECT ;
			}
//			SetNextAttackPos(entk) ;
			SetAttackPos(entk) ;
		}else {
			entk->think3 = TH3_ZONE_CHASE ;
			ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint)) ;
		}
		entk->count3 = 0 ;
		return ;
	}
	eve_w->esc_cnt-- ;
	entk->count3 ++ ;
}



void W32AlertStart(ENETHINK *entk){
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;
//contact
	entk->think1 = ENE_TH1_ALERT ;
	entk->act->CheckPad =  W32_AlertCheckPad ;
	entk->act->keep_pad = -1 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveRun ;
	entk->act->pad = 0 ;
	entk->act->dir = -1 ;
	entk->sw_gun &= ~SW_FLAG_SWITCH1 ;

	{
		if(eve_w->find_flag & W32_ATTACK_POS){
			entk->think3 = TH3_ESC_ZONECHASE ;
			if(eve_w->mode == DEF_MODE_W25_C){
			/*放尿兵はゾーン無視で追跡*/
				entk->think3 = TH3_ESC_DIRECT ;
			}
//			SetNextAttackPos(entk) ;
			SetAttackPos(entk) ;
		}else {
			entk->think3 = TH3_ZONE_CHASE ;
			ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint)) ;
//			SIG_SetTrgpPlayerLastpos( &(entk->pl_eyei), entk->com ) ;
		}
	}
}
void W32EscapeStart(ENETHINK *entk){
	EVENT_W32	*eve_w ;

	entk->think1 = ENE_TH1_ALERT ;

	eve_w = (EVENT_W32 *) entk->character ;
	eve_w->find_flag |= W32_ALERT ;
	eve_w->esc_cnt = DIRECT_TICK(W32_ESC_TIME) ;
	entk->act->CheckPad = W32_AlertCheckPad ;
	entk->act->keep_pad = -1 ;
	entk->think3 = TH3_ESC_ZONECHASE ;
	if(eve_w->mode == DEF_MODE_W25_C){
		/*放尿兵はゾーン無視で追跡*/
		entk->think3 = TH3_ESC_DIRECT ;
	}

	entk->count3 = 0 ;
	entk->act->move_s = MoveRun ;
	entk->act->pad = 0 ;
	entk->act->dir = -1 ;
//	ENE_SetTrgpEyei(&(entk->pl_eyei),&(entk->trgpoint)) ;
	ENE_SetTrgpPosMap( &(entk->trgpoint),&eve_w->esc_pos, eve_w->esc_map );
	entk->sw_gun &= ~SW_FLAG_SWITCH1 ;
}

void W32MedicationStart(ENETHINK *entk){
	EVENT_W32	*eve_w ;
	eve_w = (EVENT_W32 *) entk->character ;
	entk->act->CheckPad = W32_AlertCheckPad ;
	entk->act->keep_pad = -1 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveRun ;
	entk->act->pad = 0 ;
	entk->act->dir = -1 ;
	entk->sw_gun &= ~SW_FLAG_SWITCH1 ;
	entk->think3 = TH3_MEDICATION ;
	entk->count3 = 0 ;
}

void W32EscapeStartInThink(ENETHINK *entk){
	EVENT_W32	*eve_w ;

	eve_w = (EVENT_W32 *) entk->character ;

	eve_w->esc_cnt = DIRECT_TICK(W32_ESC_TIME) ;
	entk->think3 = TH3_ESC_ZONECHASE ;
	if(eve_w->mode == DEF_MODE_W25_C){
		/*放尿兵はゾーン無視で追跡*/
		entk->think3 = TH3_ESC_DIRECT ;
	}
	entk->count3 = 0 ;
	entk->act->move_s = MoveRun ;
	ENE_SetTrgpPosMap( &(entk->trgpoint),&eve_w->esc_pos, eve_w->esc_map );
}

void W32thinkAlert(ENETHINK *entk){
//	Think3_AttackNear( entk ) ;
	/*発見プロセス*/
	CheckDiscovery(entk) ;
	switch ( entk->think3 ) {
		case TH3_ATTACK_NEAR :
			/*静止して攻撃*/
			Think3_AttackNear( entk ) ;
		break ;
		case TH3_WAIT_CHASE :
//			Think3_Wait( entk ) ;
			Think3_AttackNear( entk ) ;
		break ;
		case TH3_ZONE_CHASE :
			Think3_ZoneChaseEyei( entk ) ;
		break ;
		case TH3_DIRECT_CHASE :
			Think3_DirectChaseEyei( entk ) ;
		break ;
		case TH3_ESC_ZONECHASE :
			Think3_ZoneChaseEscape( entk ) ;
		break;
		case TH3_ESC_DIRECT :
			Think3_DirectChasePos( entk ) ;
		break ;
		case TH3_ESC_WAIT :
			Think3_WaitEscape( entk ) ;
		break;
		case TH3_DEFENSE :
			Think3_AttackStand( entk ) ;
		break;
		case TH3_DISCOVERY :
			Think3_Discovery( entk ) ;
		break ;
		case TH3_ATTACK_RELOAD :
			Think3_AttackReload( entk ) ;
		break ;
		case TH3_GRD_HIGH :
			Think3_GrdAttack( entk ) ;
		break ;
		case TH3_MEDICATION :
			Think3_Medical( entk ) ;
		break;
	}
}


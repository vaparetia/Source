//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	tng_act.c
	天狗アクション関数 主にジャンプ系
	2001/02/06 K.Sigeno
	$Id: tng_act.c,v 1.1.1.3 2002/11/19 11:49:53 Yoshizawa1 Exp $
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

#include "korekado/enemy/enemy.h"
#include	"korekado/enemy/motion.h"
//#include	"tng_motion.h"
#include	"tng.h"

#if 0
memo

#define	ENE_MOT_FLAG_REVERSAL_U	(0x40000000)	/* 上半身 */
#define	ENE_MOT_FLAG_REVERSAL_D	(0x80000000)	/* 下半身 */
#endif

//#define TNG_LAND_ATTACK
static	void ActTngJumpGain( ACTION	* ,int ) ;
static	void ActTngJumpRoll( ACTION	* ,int ) ;
static	void ActTngJumpFall( ACTION	* ,int ) ;
static	void ActTngJumpLanding( ACTION	* ,int ) ;

static	void ActTngJumpRollLevel( ACTION	* ,int ) ;
//static	void ActTngJumpGainLevel( ACTION	* ,int ) ;
//static	void ActTngJumpFallLevel( ACTION	* ,int ) ;
static	void ActTngJumpLandingLevel( ACTION	* ,int ) ;
static	void TNG_ActReady( ACTION	* ,int ) ;

#if 0
	act->ctrl->step.vy += TEST_TNG_GAIN ;
	act->ctrl->grounded &= ~1 ; /* 床チェックを解除するので０にしておく */
#endif
//define 	TEST_TNG_FALL	(30.0F)

//#define TNG_SPEED_RATE (2.0F)	/*モーション加工*/
//#define TNG_SPEED_START (0.4F)	/*モーション加工*/
#define TNG_SPEED_START (1.0F)	/*モーション加工*/

//#define TNG_SPEED_START_LEVEL (0.3F)	/*モーション加工*/
#define TNG_SPEED_START_LEVEL (1.0F)	/*モーション加工*/

#define TNG_SPEED_RATE (1.0F)	/*モーション加工*/

#define TNG_LAND_RATE (2.0F)	/*着地モーション*/
#define TNG_LAND_RATE_LEVEL (3.0F)	/*着地モーション*/

//#define TNG_GAIN_START (200.0F)
//#define TNG_GAIN_START (170.0F)
//#define TNG_GAIN_START (250.0F)
//#define TNG_GAIN_START (280.0F)
//#define TNG_GAIN_START (200.0F)
#define TNG_GAIN_START (290.0F)
//#define TNG_FALL_AG (12.0F) /*反重力の大きさ*/
#define TNG_FALL_AG (8.0F)

#define TNG_GAIN_SPEED (260.0F)

#define ROLL_LEVEL_RATE (0.45F)

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
//extern void SigZoneView( int , SVECTOR * ,float ) ; 
#endif

//extern void *NewLandingSmoke( FVECTOR * , float ) ;

static void tngmotspeed(ACTION	*act ,float rate){
//	return ;
	MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE*rate ) ;
}

void TNG_ActSquatNoAdj( act, time )
ACTION	*act ;
int		time ;
{

	AT_SetActStatus( act, ACT_STATUS_SHAGAMI ) ;
	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_LIE ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,ACT_INTERP_M ) ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
//		AT_SetMode( act, ENE_ActStandStill ) ;
	}
	

}



//ACT_STATUS_SEG_OFF
void	ActTngJumpStart( act, time )
ACTION	*act ;
int		time ;
{
//dir
//printf("VART START !!\n");
	AT_SetActStatus( act,ACT_STATUS_TARGET_SKIP) ;
//	AT_SetActStatus( act, ACT_STATUS_SEG_OFF|ACT_STATUS_FLR_OFF) ;

	if(GM_GameStatus & STATE_VR_ONLY){
		act->mot_speed_correct = 0.0001f ;
	}

	if ( time == 0 ) {
		AT_ReSetAction( act, 0, EM_tng_p90_nom_jump_start, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		tngmotspeed(act,TNG_SPEED_START);
	}

	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else {
		AT_SetMode( act, ENE_ActStandStill ) ;
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		if(GM_GameStatus & STATE_VR_ONLY){
			act->ctrl->rot.vy = act->ctrl->turn.vy ;
		}
		AT_SetMode( act, ActTngJumpGain ) ;
		return ;
	}
}
static	void ActTngJumpGain( act, time )
ACTION	*act ;
int		time ;
{
	float tenjo ,rise;


	/*柵を越えるための高さ VRでは少し高めにしておく*/
	if(GM_GameStatus & STATE_VR_ONLY){
		rise = 2500.0f ;
	}else {
		rise = 2000.0f ;
	}
	AT_SetActStatus( act,ACT_STATUS_TARGET_SKIP) ;

#if 0
	printf("ctrl->levels[0] %f\n",act->ctrl->levels[0]);
	printf("ctrl->levels[1] %f\n",act->ctrl->levels[1]);
	printf("ctrl->mov.vy %f\n",act->ctrl->mov.vy);
#endif

	tenjo = act->ctrl->levels[1] - act->ctrl->mov.vy ;
	act->ctrl->grounded &= ~1 ; /* 床チェックを解除するので０にしておく */
	/*重力落下を弱める*/
//	act->ctrl->step.vy += TNG_FALL_AG  ;
	act->ctrl->step.vy = TNG_GAIN_SPEED ;
	if ( time == 0 ) {
//		act->ctrl->step.vy = TNG_GAIN_START ;
		AT_ReSetAction( act, 0, EM_tng_p90_nom_jump_up, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		tngmotspeed(act,TNG_SPEED_RATE);
	}
	if(
	 ( act->ctrl->step.vy <= 0.0f ) 
	||(tenjo < 1000.0F)
	||
	(
	(act->ctrl->mov.vy >= (act->target_pos.vy + rise))
	&&((act->ctrl->mov.vy - act->ctrl->levels[0])>rise )
	)
	 ){
		AT_SetMode( act, ActTngJumpRoll ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else {
		AT_SetMode( act, ENE_ActStandStill ) ;
	}
}
static	void ActTngJumpRoll( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act,ACT_STATUS_TARGET_SKIP|ACT_STATUS_FLR_OFF) ;

//	act->ctrl->step.vy = 0.0F ;
//	act->ctrl->grounded &= ~1 ; /* 床チェックを解除するので０にしておく */


	if ( time == 0 ) {
		AT_ReSetAction( act, 0, EM_tng_p90_nom_jump_peak, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
//		AT_ReSetAction( act, 0, EM_tng_p90_nom_leap, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		tngmotspeed(act,ROLL_LEVEL_RATE);
	}

	if(GM_GameStatus & STATE_VR_ONLY){
		act->mot_speed_correct = (act->target_pos.vw/ TNG_JUNP_DIS_VR ) ;
	}else {
		act->mot_speed_correct = (act->target_pos.vw/ TNG_JUNP_DIS );
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
//printf("ROLL END TIME %d\n",time);
		AT_SetMode( act, ActTngJumpFall ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else {
		AT_SetMode( act, ENE_ActStandStill ) ;
	}

}
static	void ActTngJumpFall( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act,ACT_STATUS_TARGET_SKIP) ;

/*TEST 落下を遅くする*/
	act->ctrl->step.vy += TNG_FALL_AG  ;

	if(GM_GameStatus & STATE_VR_ONLY){
		act->mot_speed_correct = 0.0001f ;
	}


	if ( time == 0 ) {
		AT_ReSetAction( act, 0, EM_tng_p90_nom_jump_down, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		tngmotspeed(act,TNG_SPEED_RATE);
	}
//	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
	if ( act->ctrl->grounded & 1 ) {
#ifdef TNG_LAND_ATTACK
		FVECTOR force = { 0.0F,-200.0F,100.0F } ;
		FVECTOR size = { 500.0F, 500.0F,500.0F } ;
		ENE_SetOffenseTarget3( act,WP_PUNCH ,
			HUMAN21_MIGI_KAKATO,&size ,&force) ;
#endif
		AT_SetMode( act, ActTngJumpLanding ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else {
		AT_SetMode( act, ENE_ActStandStill ) ;
	}
}

static	void ActTngJumpLanding( act, time )
ACTION	*act ;
int		time ;
{
	FVECTOR pos;
	AT_SetActStatus( act,ACT_STATUS_TARGET_SKIP) ;

	if(GM_GameStatus & STATE_VR_ONLY){
		act->mot_speed_correct = 0.0001f ;
	}

	if ( time == 0 ) {
		AT_ReSetAction( act, 0, EM_tng_p90_nom_jump_end, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		tngmotspeed(act,TNG_LAND_RATE);
		pos = act->ctrl->mov ;
		pos.vy = act->ctrl->levels[0] ;
//		NewLandingSmoke( &pos, 800.0f ) ;
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else {
		AT_SetMode( act, ENE_ActStandStill ) ;
	}

}
void	ActTngJumpStartLevel( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act,ACT_STATUS_TARGET_SKIP|ACT_STATUS_SEG_OFF) ;

	if ( time == 0 ) {
		AT_ReSetAction( act, 0, EM_tng_p90_nom_jump_start, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		tngmotspeed(act,TNG_SPEED_START_LEVEL);
	}

	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
//		AT_SetMode( act, ActTngJumpGainLevel ) ;
		AT_SetMode( act, ActTngJumpRollLevel ) ;
		if(GM_GameStatus & STATE_VR_ONLY){
			act->ctrl->rot.vy = act->ctrl->turn.vy ;
		}
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else {
		AT_SetMode( act, ENE_ActStandStill ) ;
	}
}
#if 0
static	void ActTngJumpGainLevel( act, time )
ACTION	*act ;
int		time ;
{
	float tenjo ;
	AT_SetActStatus( act,ACT_STATUS_TARGET_SKIP|ACT_STATUS_SEG_OFF) ;

#if 0
	printf("ctrl->levels[0] %f\n",act->ctrl->levels[0]);
	printf("ctrl->levels[1] %f\n",act->ctrl->levels[1]);
	printf("ctrl->mov.vy %f\n",act->ctrl->mov.vy);
#endif

	tenjo = act->ctrl->levels[1] - act->ctrl->mov.vy ;
	act->ctrl->grounded &= ~1 ; /* 床チェックを解除するので０にしておく */
	/*重力落下を弱める*/
//	act->ctrl->step.vy += TNG_FALL_AG  ;
	act->ctrl->step.vy = TNG_GAIN_SPEED ;
	if ( time == 0 ) {
//		act->ctrl->step.vy = TNG_GAIN_START ;
		AT_ReSetAction( act, 0, EM_tng_p90_nom_jump_up, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		tngmotspeed(act,TNG_SPEED_RATE);
	}
	if(
	 ( act->ctrl->step.vy <= 0.0f ) 
	||(tenjo < 1000.0F)
	||
	(
	(act->ctrl->mov.vy >= (act->target_pos.vy + 2000.0F))
	&&((act->ctrl->mov.vy - act->ctrl->levels[0])>2000.0F )
	)
	 ){
		AT_SetMode( act, ActTngJumpRollLevel ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else {
		AT_SetMode( act, ENE_ActStandStill ) ;
	}
}
#endif
static	void ActTngJumpRollLevel( act, time )
ACTION	*act ;
int		time ;
{

//	AT_SetActStatus( act,ACT_STATUS_TARGET_SKIP|ACT_STATUS_FLR_OFF) ;
	AT_SetActStatus( act,ACT_STATUS_TARGET_SKIP|ACT_STATUS_FLR_OFF|ACT_STATUS_SEG_OFF) ;

//	act->ctrl->step.vy = 0.0F ;
//	act->ctrl->grounded &= ~1 ; /* 床チェックを解除するので０にしておく */

	if ( time == 0 ) {
//		AT_ReSetAction( act, 0, EM_tng_p90_nom_jump_peak, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		AT_ReSetAction( act, 0, EM_tng_p90_nom_leap, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
//		tngmotspeed(act,TNG_SPEED_RATE);
		tngmotspeed(act,ROLL_LEVEL_RATE);
//printf("ROLL MOTION CORRECT 10.0 !!\n ") ;
	}

	act->mot_speed_correct = (act->target_pos.vw/ (TNG_JUNP_DIS_LEVEL) );

	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
//printf("ROLL END TIME %d\n",time);
//		AT_SetMode( act, ActTngJumpFallLevel ) ;
		AT_SetMode( act, ActTngJumpLandingLevel ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else {
		AT_SetMode( act, ENE_ActStandStill ) ;
	}

}
static	void ActTngJumpLandingLevel( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act,ACT_STATUS_TARGET_SKIP) ;


	if ( time == 0 ) {
		AT_ReSetAction( act, 0, EM_tng_p90_nom_jump_end, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		tngmotspeed(act,TNG_LAND_RATE_LEVEL);
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else {
		AT_SetMode( act, ENE_ActStandStill ) ;
	}

}


void TNG_ActNearAtk( act, time )
ACTION	*act ;
int		time ;
{
	FVECTOR pos;

	static FVECTOR force = { 0.0F,-20.0F,10.0F } ;
	static FVECTOR size = { 500.0F, 500.0F,500.0F } ;


//printf("NOW TNG_ActNearAtk!!!\n");
//	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_PUSHT_SKIP ) ;
//	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_PUSHT_SKIP | ACT_STATUS_ADJ_X) ;
	act->ctrl->step = DG_ZeroVector ;
	if ( time == 0 ) {
		if ( act->dir >= 0 ) {
			act->ctrl->turn.vy = act->dir ;
		}
//		AT_SetAction( act, 0, EM_ak_attack_near_kick_l, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		/*気合声*/
//		GM_SeSetMode( SD_V_GBSFUN01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
//		GM_SeSetMode( SD_V_GBSFUN01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

/*
	tng_p90_nom_attack_near_spinkick_high,
	tng_p90_nom_attack_near_shoutei,
	tng_p90_nom_attack_near_spiraluper,
	tng_p90_nom_attack_near_kneekick,
	tng_p90_nom_attack_near_spinkick_low,
	tng_p90_nom_attack_near_elbow,
	tng_p90_nom_attack_near_somersault,

	static FVECTOR force = { 0.0F,-200.0F,400.0F } ;

*/
	if(!(act->bodyp.type & ENE_TYPE_KATANA )){
		switch( act->keep_mot ) {
			/* スピンキック */
			case EM_tng_p90_nom_attack_near_spinkick_high :	
				if(time == DIRECT_TICK(4)){
					ENE_SetOffenseTarget3( act,WP_PUNCH ,
						HUMAN21_MIGI_KAKATO,&size ,&force) ;
				}
				if(time == DIRECT_TICK(17)){
					ENE_SetOffenseTarget3( act,WP_PUNCH ,
						HUMAN21_HIDARI_KAKATO,&size ,&force) ;
				}
//			if ( time > COUNT_VMODE(4) && time < COUNT_VMODE(76) )
//				AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
			break ;
			case EM_tng_p90_nom_attack_near_shoutei :	/* パンチ */
				if(time == DIRECT_TICK(4)){
					ENE_SetOffenseTarget3( act,WP_PUNCH ,
						HUMAN21_HIDARI_TE,&size ,&force) ;
				}
			break ;
			case EM_tng_p90_nom_attack_near_somersault :	/* キック */
				if(time == DIRECT_TICK(4)){
					ENE_SetOffenseTarget3( act,WP_PUNCH ,
						HUMAN21_HIDARI_KAKATO,&size ,&force) ;
				}
			break ;
			case EM_tng_p90_nom_attack_near_elbow :
				if(time == DIRECT_TICK(4)){
					ENE_SetOffenseTarget3( act,WP_PUNCH ,
						HUMAN21_HIDARI_UDE2,&size ,&force) ;
				}
			break ;
			case EM_tng_p90_nom_attack_near_kneekick :
				if(time == DIRECT_TICK(5)){
					ENE_SetOffenseTarget3( act,WP_PUNCH ,
						HUMAN21_MIGI_ASHI2,&size ,&force) ;
				}
				if(time == DIRECT_TICK(29)){
					ENE_SetOffenseTarget3( act,WP_PUNCH ,
						HUMAN21_MIGI_KAKATO,&size ,&force) ;
				}
			break ;
			case EM_tng_p90_nom_attack_near_spinkick_low :
				if((time >= DIRECT_TICK(4))&& (time <= DIRECT_TICK(19))){
					ENE_SetOffenseTarget3( act,WP_PUNCH ,
						HUMAN21_MIGI_KAKATO,&size ,&force) ;
				}
			break ;
			case EM_tng_p90_nom_attack_near_spiraluper :
				if(time == DIRECT_TICK(4)){
					ENE_SetOffenseTarget3( act,WP_PUNCH ,
						HUMAN21_HIDARI_TE,&size ,&force) ;
				}
			break ;
			case EM_tng_p90_nom_sliding :
//printf("NOW SLIDING ATK!!!\n");
				if( time > DIRECT_TICK(18) && time < DIRECT_TICK(38) ) {
					ENE_SetOffenseTarget3( act,WP_PUNCH ,
						HUMAN21_MIGI_KAKATO,&size ,&force) ;
				}
			break ;
			case EM_tng_p90_nom_stamp :
				if((time >= DIRECT_TICK(22))&&(time <= DIRECT_TICK(30))){
					ENE_SetOffenseTarget3( act,WP_PUNCH ,
						HUMAN21_MIGI_KAKATO,&size ,&force) ;
				}
			break;
		}
	}else {
		/*刀*/
		switch( act->keep_mot ) {
			/*ジャンプ斬り*/
			case EM_tng_tbl_nom_leap_attack :
				if( time >= COUNT_VMODE(58) && time <= COUNT_VMODE(61) ) {
					ENE_SetOffenseTarget3( act,WP_BLADE ,
						HUMAN21_MIGI_TE,&size ,&force) ;
				}
				if( time == COUNT_VMODE(60)) {
//					GM_SeSetMode( SD_A_SWORDHAD, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
					GM_SeSetMode( SD_A_SWORDHAD, &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
		pos = act->ctrl->mov ;
		pos.vy = act->ctrl->levels[0] ;
//					NewLandingSmoke( &pos, 800.0f ) ;
				}
				break ;
			case EM_tng_tbl_nom_attack1 :
			case EM_tng_tbl_nom_attack2 :
				if( time == DIRECT_TICK(13)  ) {
					ENE_SetOffenseTarget3( act,WP_BLADE ,
						HUMAN21_MIGI_TE,&size ,&force) ;
				}
				break ;
			/*突き*/
			case EM_tng_tbl_nom_attack3 :
				if( time == DIRECT_TICK(13) ) {
					ENE_SetOffenseTarget3( act,WP_BLADESTAB ,
						HUMAN21_MIGI_TE,&size ,&force) ;
				}
			break ;
			case EM_tng_p90_nom_stamp :
				if((time >= DIRECT_TICK(22))&&(time <= DIRECT_TICK(30))){
					ENE_SetOffenseTarget3( act,WP_PUNCH ,
						HUMAN21_MIGI_KAKATO,&size ,&force) ;
				}
			break;
		}
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( act->dir >= 0 ) {
//		act->ctrl->turn.vy = act->dir ;
	}else {
		AT_SetMode( act, ENE_ActStandStill ) ;
	}

}

//	EM_tng_tbl_guard_1,
//	EM_tng_tbl_guard_2,
//	EM_tng_tbl_guard_3,
void TNG_ActBladeGuard( act, time )
ACTION	*act ;
int		time ;
{

	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_PUSHT_SKIP|ACT_STATUS_GURAD ) ;

	if ( time == 0 ) {
//		AT_ReSetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		AT_ReSetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, 0 ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else {
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( act->CheckPad( act ) ) {
		return ;
	}


	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
#if 0
		AT_SetMode( act, ENE_ActStandStill ) ;
#else
/*ガード時に1フレームの隙も発生させない*/
		act->keep_mot = EM_tng_tbl_nom_idle ;
		act->tmp_time = 0 ;
		AT_SetMode( act, TNG_ActReady ) ;
#endif
		return ;
	}

}

/*静止して上段構え*/
static void TNG_ActReady( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	/*アジャスト*/
//	AT_SetActStatus( act, ACT_STATUS_STAND| ACT_STATUS_ADJ_X ) ;
	AT_SetActStatus( act, ACT_STATUS_STAND) ;
	if( act->bodyp.type & ENE_TYPE_KATANA)
	{
		AT_SetActStatus( act, ACT_STATUS_GURAD);
	}
	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
		ACT_INTERP_M ) ;
	}

//	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
			return ;
		}
//	}

	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
	}
}


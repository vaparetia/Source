/*
	at_act.c
	アタッカー アクション

	1999/07/09 K.Sigeno
	$Id: at_act.c,v 1.1.1.3 2002/11/19 11:48:59 Yoshizawa1 Exp $
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
#include	"libmt.h"
#include	"libdg.cnf"
#include	"libutl.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"korekado/enemy/enemy.h"
#include	"attacker.h"
//#include	"ataction.c"
//#include	"../defender/eve_a.h"

#include "BP_Misc.h"

#if 0
action.hに定義された補完値
ACT_INTERP_DEF		8*5	/* 1/300単位 */
ACT_INTERP_M		60*1 /* 1/300単位 */
ACT_INTERP_SLOWLY	60*5 /* 1/300単位 */
ACT_INTERP_VERY_SLOWLY	60*30 /* 1/300単位 */
#endif

#define AT_GRD_MAX_DIS (4000.0F)
/*グレネード発射*/

/****
		case SP_STAND_DEFENCE :
		case SP_SQUAT_DEFENCE :
****/
/*ターゲットサイズ変更*/
/*基本装備*/
static void TrgSizeSetNormal(ACTION *act)
{
//	ENE_BODYTRGSIZE_STAND_SHIELD,
//	ENE_BODYTRGSIZE_SQUAT_SHIELD
	if(act->pad == SP_SQUATGUN){
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_SQUAT ) ;
	}else {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
	}

}
/*盾装備*/
static void TrgSizeSetShield(ACTION *act)
{
	if((act->pad == SP_SQUATGUN)
	||(act->pad == SP_SQUAT_DEFENCE)){
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_SQUAT_SHIELD ) ;
	}else {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND_SHIELD ) ;
	}
}

//#define LEFT_ARM_MASK (0x0780) /*肩含む*/
//#define LEFT_ARM_MASK (0x0700) /*肩含まない*/
#define LEFT_ARM_MASK (0x0600) /*手首と下の腕*/
static void MotLayer_Shield(ACTION *act){
	AT_SetAction( act, 1, EM_shl_nom_override, 0, (MOTION_MASK_UPPER), ACT_INTERP_DEF ) ;
	act->body->m_ctrl->mt3_ctrl[ 1 ].flag |= MT3_ACTIVE ;
}

#if 0
static void MotLayer_StandWait(ACTION *act){
	AT_SetAction( act, 1, EM_gbs_ak_nom_idle_lower, 0, (MOTION_MASK_UPPER), ACT_INTERP_DEF ) ;
	act->body->m_ctrl->mt3_ctrl[ 1 ].flag |= MT3_ACTIVE ;
}
static void MotLayer_Mukade(ACTION *act){
	AT_SetAction( act, 1, EM_cau_mukade_idle, 0, (LEFT_ARM_MASK), ACT_INTERP_DEF ) ;
	act->body->m_ctrl->mt3_ctrl[ 1 ].flag |= MT3_ACTIVE ;
}
#endif

static void Layer_Off(ACTION *act,int layer){
	act->body->m_ctrl->mt3_ctrl[ layer ].flag &= ~MT3_ACTIVE ;
}



/*静止して銃上段構え*/
void AT_ActReadyGun( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	/*アジャスト*/

	if((COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)&&(!(GM_PlayerStatus & PLAYER_STEALTH))
	&&(!(act->bodyp.type & ENE_TYPE_TNG_A ))
	){
		AT_SetActStatus( act, ACT_STATUS_STAND| ACT_STATUS_ADJ_X ) ;
	}else {
		AT_SetActStatus( act, ACT_STATUS_STAND) ;
	}


	if( act->bodyp.type & ENE_TYPE_KATANA)
	{
		AT_SetActStatus( act, ACT_STATUS_GURAD);
	}
	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,ACT_INTERP_M ) ;

		TrgSizeSetNormal(act);
		Layer_Off(act,1);
	}

	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
	}
}

void AT_ActReadyGunPbreak( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND  ) ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !act->CheckPad( act ) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;

}
/*静止して銃上段発射*/
void AT_ActShootGun( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;


	/*アジャスト*/
	if((COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)&&(!(GM_PlayerStatus & PLAYER_STEALTH))
	&&(!(act->bodyp.type & ENE_TYPE_TNG_A ))
	){
		AT_SetActStatus( act, ACT_STATUS_SHOT_PIKU | ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	}else {
		AT_SetActStatus( act, ACT_STATUS_SHOT_PIKU | ACT_STATUS_STAND  ) ;
	}

	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if((!( act->bodyp.type & ENE_TYPE_TNG_A))&&( !((irnd()>>8)%8)  ) ){
		ENE_ShootBullet( act,ENE_BULLET_NOWALL ,NULL ) ;
	}else {
		ENE_ShootBullet( act,ENE_BULLET_NORMAL ,NULL ) ;
	}
	*act->sw->bullet = *act->sw->bullet+1 ;
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
#if 1
		TrgSizeSetNormal(act);
#endif
		Layer_Off(act,1);
	}

	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
}
static void AT_ActReadyGunShl( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	/*アジャスト*/
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT&&(!(GM_PlayerStatus & PLAYER_STEALTH))){
		AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	}else {
		AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	}
	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}

	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		TrgSizeSetShield(act);
	}

	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
}

/*静止して銃上段発射*/
static void AT_ActShootGunShl( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	/*アジャスト*/



	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT&&(!(GM_PlayerStatus & PLAYER_STEALTH))){
		AT_SetActStatus( act, ACT_STATUS_SHOT_PIKU |ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	}else {
		AT_SetActStatus( act, ACT_STATUS_SHOT_PIKU | ACT_STATUS_STAND  ) ;
	}



	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if((!( act->bodyp.type & ENE_TYPE_TNG_A))&&( !((irnd()>>8)%8)  )) {
		ENE_ShootBullet( act,ENE_BULLET_NOWALL ,NULL ) ;
	}else {
		ENE_ShootBullet( act,ENE_BULLET_NORMAL ,NULL ) ;
	}
	*act->sw->bullet = *act->sw->bullet+1 ;

/*TEST*/
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
#if 1
		TrgSizeSetShield(act);
#endif
	}

	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
}



void AT_ActSquatReadyGun( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	/*アジャスト*/
	AT_SetActStatus( act, ACT_STATUS_ADJ_X |ACT_STATUS_SHAGAMI);
//	AT_SetActStatus( act, ACT_STATUS_SHAGAMI);

	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
/*TEST*/
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, 
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_SQUAT ) ;
		Layer_Off(act,1);
	}

	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
			return ;
		}
	}

}

/*静止して銃下段発射*/
void AT_ActSquatShootGun( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	/*アジャスト*/
	AT_SetActStatus( act, ACT_STATUS_SHOT_PIKU | ACT_STATUS_ADJ_X |ACT_STATUS_SHAGAMI ) ;
//	AT_SetActStatus( act, ACT_STATUS_SHOT_PIKU | ACT_STATUS_SHAGAMI ) ;

	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if((!( act->bodyp.type & ENE_TYPE_TNG_A))&& ( !((irnd()>>8)%2)  ) ){
		ENE_ShootBullet( act,ENE_BULLET_NOWALL ,NULL ) ;
	}else {
		ENE_ShootBullet( act,ENE_BULLET_NORMAL ,NULL ) ;
	}
	*act->sw->bullet = *act->sw->bullet+1 ;

/*TEST*/
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_SQUAT ) ;
		Layer_Off(act,1);
	}

	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
			ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
			return ;
		}
	}
}
static void AT_ActSquatReadyGunShl( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	AT_SetActStatus( act, ACT_STATUS_ADJ_X |ACT_STATUS_SHAGAMI ) ;
//	AT_SetActStatus( act, ACT_STATUS_SHAGAMI ) ;
	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
/*TEST*/
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		TrgSizeSetShield(act);
	}

	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
			return ;
		}
	}
}
//	act->keep_mot = EM_gbs_ak_nom_idle_lower ;

void AT_ActStandWait( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	/*アジャスト*/
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	if( act->bodyp.type & ENE_TYPE_KATANA)
	{
		AT_SetActStatus( act, ACT_STATUS_GURAD);
	}
	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}

//	act->sw->finger = SW_FLAG_SWITCH1 ;


	if ( time == 0 ) {
		TrgSizeSetNormal(act);
#if 0
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,ACT_INTERP_M ) ;
		if(!( act->bodyp.type & ENE_TYPE_KATANA)){
			MotLayer_StandWait(act);
		}
#else
		if(!( act->bodyp.type & ENE_TYPE_KATANA)){
			AT_SetAction( act, 0, EM_gbs_ak_nom_idle_lower, 0, MOTION_MASK_FULL,ACT_INTERP_M ) ;
		}else {
			AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,ACT_INTERP_M ) ;
		}
#endif

	}

	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
	}
}


/*静止して銃下段発射*/
static void AT_ActSquatShootGunShl( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	/*アジャスト*/

//	AT_SetActStatus( act, ACT_STATUS_SHOT_PIKU |ACT_STATUS_ADJ_X | ACT_STATUS_SHAGAMI ) ;
	AT_SetActStatus( act, ACT_STATUS_SHOT_PIKU  | ACT_STATUS_SHAGAMI ) ;

	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
//	ENE_Shoot( &(BODYWORLD( act->body, HUMAN21_MIGI_TE )), act ) ;
	ENE_ShootBullet( act,ENE_BULLET_NORMAL ,NULL ) ;
	*act->sw->bullet = *act->sw->bullet+1 ;
/*TEST*/
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
#if 1
		TrgSizeSetShield(act);
#endif
	}

	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
			return ;
		}
	}
}

/*接近 殴り攻撃*/
static void AT_ActPunchFAMAS( act, time )
ACTION	*act ;
int		time ;
{
	int hit_time = (AT_THK_RATE*2) ;

	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( act->CheckDamage( act ) ) {
//		act->act_end = 1 ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if (( time==0) ) {
		AT_SetAction( act, 0, (act->keep_mot), 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	/*攻撃あたり*/
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
//		ENE_SetOffenseTarget( act, 0 ) ;
		Layer_Off(act,1);

	}
	

	switch(act->keep_mot){
		case EM_attack_near : /*回し蹴り*/
		case EM_ak_attack_near_kick_r :
		case EM_ak_attack_near_kick_l :
		case EM_htc_m4_nom_attack_near : //ハイテク暫定

			hit_time = (AT_THK_RATE) ;
			break;
		case EM_ak_attack_near_punch_r :
		case EM_ak_attack_near_punch_l :
			/*1で銃放し 49で復帰*/

         if ( ( BP_IsPAL()!=TRUE && ( time >= 0 ) &&( time < 49 ) ) ||
               (BP_IsPAL()==TRUE && ( time >= 0 ) &&( time < 41 ) ) )
         {
				AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_GUN_FREE ) ;
			}
			hit_time = (AT_THK_RATE) ;
			break;
	}
	if (time == hit_time) {
	/*攻撃あたり*/
//		ENE_SetOffenseTarget( act, 0 ) ;
		ENE_SetOffenseTarget( act, (act->keep_mot) ) ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		if(act->bodyp.type & ENE_TYPE_SHOTGUN){
			act->keep_mot = EM_gbs_sps_nom_fire_p ;
		}else {
			act->keep_mot = EM_kamae_gun_high ;
		}
		AT_SetMode( act, AT_ActReadyGun ) ;
		return ;
	}

	act->ctrl->step = DG_ZeroVector ;
}

static void AT_ActKickLocker( act, time )
ACTION	*act ;
int		time ;
{
	int hit_time = (AT_THK_RATE*2) ,hit_pos = 0;
	static FVECTOR locker_size = { 750.0F,750.0F,750.0F } ;
	if ( act->CheckDamage( act ) ) {
//		act->act_end = 1 ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
//		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if (( time==0) ) {
		AT_SetAction( act, 0, (act->keep_mot), 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	/*攻撃あたり*/
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),
		ENE_BODYTRGSIZE_STAND ) ;
//		ENE_SetOffenseTarget( act, 0 ) ;

		Layer_Off(act,1);

	}

	switch(act->keep_mot){
		case EM_shl_nom_attack_near : /*蹴り*/
		default :
			/*ENE内部でモーション番号から攻撃部位を判別*/
			hit_time = (AT_THK_RATE*3) ;
			hit_pos = EM_attack_near ; 
			break;
	}
	if (time==hit_time) {
	/*攻撃あたり*/
//		ENE_SetOffenseTarget( act, hit_pos ) ;
		ENE_SetOffenseTarget2( act,WP_KICK,HUMAN21_MIGI_KAKATO,
		&locker_size ) ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		act->keep_mot = EM_shl_nom_idle ;
		AT_SetMode( act, AT_ActReadyGunShl ) ;
		return ;
	}
	if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) {
//		AT_SetMode( act, ENE_ActStandStill ) ;
//		AT_SetMode( act, AT_ActReadyGun ) ;
//		return ;
	}


	act->ctrl->step = DG_ZeroVector ;
}
static void AT_ActPunchShl( act, time )
ACTION	*act ;
int		time ;
{

	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
//		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if (( time==0) ) {
		AT_SetAction( act, 0, (act->keep_mot), 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	/*攻撃あたり*/
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),
		ENE_BODYTRGSIZE_STAND ) ;
//		ENE_SetOffenseTarget( act, 0 ) ;
		Layer_Off(act,1);

	}

	switch(act->keep_mot){
		case EM_shl_nom_attack_near : /*蹴り*/
			/*ENE内部でモーション番号から攻撃部位を判別*/


         if ( (BP_IsPAL()!=TRUE && (time > 9)&&(time <= 18)) ||
              (BP_IsPAL()==TRUE && (time > 8)&&(time <= 15)) )
         {
				ENE_SetOffenseTarget( act, act->keep_mot ) ;
			}
			break;
		case EM_shl_nom_attack_near_shl : 

         if ( (BP_IsPAL()!=TRUE && (time > 7)&&(time <= 12)) ||
              (BP_IsPAL()==TRUE && (time > 6)&&(time <= 10)) )
         {
            ENE_SetOffenseTarget( act, act->keep_mot ) ;
			}
			break;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		act->keep_mot = EM_shl_nom_idle ;
		AT_SetMode( act, AT_ActReadyGunShl ) ;
		return ;
	}
	if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) {
//		AT_SetMode( act, ENE_ActStandStill ) ;
//		AT_SetMode( act, AT_ActReadyGun ) ;
//		return ;
	}

	act->ctrl->step = DG_ZeroVector ;
}



/*踏みつけ 基本装備兵*/
//	static FVECTOR size = { 500.0F,500.0F,500.0F } ;
// shiled 30f
#if 0
static void AT_ActNomStomp( act, time )
ACTION	*act ;
int		time ;
{
	FVECTOR size = { 500.0F,500.0F,500.0F } ;
//	AT_SetActStatus( act, ACT_STATUS_ADJ_X | ACT_STATUS_STAND ) ;
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
	if ( time==0 ) {
		AT_SetAction( act, 0, (act->keep_mot), 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		Layer_Off(act,1);
	}
	/*基本踏みつけ*/
#ifndef PAL       //BP JG - unused
	if (time==22) {
#else
	if (time==18) {
#endif
		/*攻撃あたり*/
		ENE_SetOffenseTarget2(act,WP_STAMP,HUMAN21_HIDARI_TSUMASAKI,&size );
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		if(act->bodyp.type & ENE_TYPE_SHOTGUN){
			act->keep_mot = EM_gbs_sps_nom_fire_p ;
		}else {
			act->keep_mot = EM_kamae_gun_high ;
		}
		AT_SetMode( act, AT_ActReadyGun ) ;
	}

	act->ctrl->step = DG_ZeroVector ;
}
#endif
#if 0
static void AT_ActShlStomp( act, time )
ACTION	*act ;
int		time ;
{
	FVECTOR	size = { 1000.0F,1000.0F,1000.0F } ;
//	FVECTOR	effectpos;
//	FMATRIX	*body ;


//	AT_SetActStatus( act, ACT_STATUS_ADJ_X | ACT_STATUS_STAND ) ;
	AT_SetActStatus( act,  ACT_STATUS_STAND ) ;

	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
//		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( time==0 ) {
		AT_SetAction( act, 0, (act->keep_mot), 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	/*攻撃あたり*/
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		Layer_Off(act,1);
	}
	/*盾 踏みつけ*/
#ifndef PAL       //BP JG - unused
	if ( time == 30 ) {
#else
	if ( time == 25 ) {
#endif
#if 0
		extern void *NewDustCloud( FVECTOR * );
		body = &( BODYWORLD(act->body, HUMAN21_HIDARI_TSUMASAKI) );
		effectpos.vx = body->m[ 3 ][ 0 ] ;
		effectpos.vy = body->m[ 3 ][ 1 ] ;
		effectpos.vz = body->m[ 3 ][ 2 ] ;
//		NewBlood_2D(&effectpos);
		NewDustCloud(&effectpos);
#endif
		ENE_SetOffenseTarget2(act,WP_STAMP,HUMAN21_HIDARI_TSUMASAKI,
		&size );
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
	if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) {
		act->keep_mot = EM_shl_nom_idle ;
		AT_SetMode( act, AT_ActReadyGunShl ) ;
		return ;
	}

	act->ctrl->step = DG_ZeroVector ;
}
#endif
/*對エルード攻撃*/
static void AT_ActBeyondAtk( act, time )
ACTION	*act ;
int		time ;
{
	FVECTOR	size = { 1000.0F,1000.0F,1000.0F } ;

//	AT_SetActStatus( act, ACT_STATUS_ADJ_X | ACT_STATUS_STAND ) ;
	AT_SetActStatus( act,  ACT_STATUS_STAND ) ;

	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( time==0 ) {
		AT_SetAction( act, 0, (act->keep_mot), 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	/*攻撃あたり*/
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		Layer_Off(act,1);
	}
	/*盾 踏みつけ*/


   if ( (BP_IsPAL()!=TRUE && (time==30)) ||
        (BP_IsPAL()==TRUE && (time==25)) )
   {

#if 0
		extern void *NewDustCloud( FVECTOR * );
		body = &( BODYWORLD(act->body, HUMAN21_HIDARI_TSUMASAKI) );
		effectpos.vx = body->m[ 3 ][ 0 ] ;
		effectpos.vy = body->m[ 3 ][ 1 ] ;
		effectpos.vz = body->m[ 3 ][ 2 ] ;
//		NewBlood_2D(&effectpos);
		NewDustCloud(&effectpos);
#endif
//		if(act->bodyp.type & ENE_TYPE_SHIELD){}

		ENE_SetOffenseTarget2(act,WP_STAMP,HUMAN21_HIDARI_TSUMASAKI,
		&size );
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
	if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) {
		act->keep_mot = EM_shl_nom_idle ;
		if(act->bodyp.type & ENE_TYPE_SHIELD){
			AT_SetMode( act, AT_ActReadyGunShl ) ;
		}else {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		return ;
	}

	act->ctrl->step = DG_ZeroVector ;
}


/*味方起こし踏み*/
static void AT_ActWakeStomp( act, time )
ACTION	*act ;
int		time ;
{
//	FVECTOR size = { 500.0F,500.0F,500.0F } ;
//	AT_SetActStatus( act, ACT_STATUS_ADJ_X | ACT_STATUS_STAND ) ;
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
	if ( time==0 ) {
		AT_SetAction( act, 0, (act->keep_mot), 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		Layer_Off(act,1);
	}
	/*基本踏みつけ*/


   if ( (BP_IsPAL()!=TRUE && (time==22)) ||
        (BP_IsPAL()==TRUE && (time==18)) )
   {
		/*攻撃あたり*/
		ENE_SetTargetEneDirect( act,act->the_target, 0 );
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		if(act->bodyp.type & ENE_TYPE_SHOTGUN){
			act->keep_mot = EM_gbs_sps_nom_fire_p ;
		}else {
			act->keep_mot = EM_kamae_gun_high ;
		}
		AT_SetMode( act, AT_ActReadyGun ) ;
	}
	act->ctrl->step = DG_ZeroVector ;
}


static void AT_ActDiscovery( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_SetHeadMark( act, BODY21_HEAD, HEADMARK_BR ) ;
	}
	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		act->keep_mot = EM_kamae_gun_high ;
		AT_SetMode( act, ENE_ActStandStill ) ;
//		AT_SetMode( act, AT_ActLoop ) ;
	}

	act->ctrl->step = DG_ZeroVector ;
}
/*Shl*/
static void AT_ActDiscoveryShl( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_SetHeadMark( act, BODY21_HEAD, HEADMARK_BR ) ;
		MotLayer_Shield(act);
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		act->keep_mot = EM_shl_nom_idle ;
		AT_SetMode( act, ENE_ActStandStill ) ;
//		AT_SetMode( act, AT_ActLoop ) ;
	}

	act->ctrl->step = DG_ZeroVector ;
}


static void AT_ActSurpHide( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_SetHeadMark( act, BODY21_HEAD, HEADMARK_BR ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_LIE ) ;
	}
	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
			return ;
		}
	}
	if(
	 ( GM_GetObjectMotionEnd( act->body, 0 ) )
	||(time > DIRECT_TICK(15))
	 ) {
		act->act_end = 1 ;
//		act->keep_mot = EM_squat_hide_idle ;
//		AT_SetMode( act, ENE_ActStandStill ) ;
	}
	act->ctrl->step = DG_ZeroVector ;
}


void AT_ActOneTimeTurn( act, time )
ACTION	*act ;
int		time ;
{


#if 1
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
#else
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
#endif
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
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		Layer_Off(act,1);
	}


	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		act->keep_mot = EM_kamae_gun_high ;
		AT_SetMode( act, ENE_ActStandStill ) ;
//		AT_SetMode( act, AT_ActLoop ) ;
	}
}
#if 0
static void AT_ActLoopNoAdj( act, time )
ACTION	*act ;
int		time ;
{

	AT_SetActStatus( act, ACT_STATUS_STAND  ) ;
	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, 
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;
		Layer_Off(act,1);
	}


	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
//		AT_SetMode( act, ENE_ActStandStill ) ;
//		AT_SetMode( act, AT_ActReadyGun ) ;
//		act->keep_mot = EM_kamae_gun_high ;
		AT_SetMode( act, ENE_ActStandStill ) ;
//		AT_SetMode( act, AT_ActLoop) ;
	}
	
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
	}

}
#endif
static void AT_ActSquatNoAdj( act, time )
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
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), 
//			ENE_BODYTRGSIZE_SQUAT ) ;
			ENE_BODYTRGSIZE_LIE ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
//			ACT_INTERP_DEF ) ;
			ACT_INTERP_M ) ;

		Layer_Off(act,1);
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

static void AT_ActOneTimeTurnShl( act, time )
ACTION	*act ;
int		time ;
{

	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
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
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND_SHIELD ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, 
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;

#if 1
		MotLayer_Shield(act);
#endif

	}


	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
//		AT_SetMode( act, AT_ActReadyGun ) ;
	}
	

}




#define LOW_SPEED -20.0F
#define MID_SPEED -15.0F
#define HI_SPEED  0.0F

static void AT_ActMukadeMoveLow( act, time )
ACTION	*act ;
int		time ;
{
#if 0
	AT_SetActStatus( act, ACT_STATUS_STAND| ACT_STATUS_ADJ_X ) ;
#else
	AT_SetActStatus( act,(ACT_STATUS_STAND|ACT_STATUS_MOVE
	|ACT_STATUS_IK_MUKADE));
//	ACT_STATUS_GUN_FREE
#endif

	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
#if 0
	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0,   act->keep_mot, 0, MOTION_MASK_FULL,  
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;

		if(act->move_s != MoveWalkGun ) {
			MotLayer_Mukade(act);
		}else {
			Layer_Off(act,1);
		}
	}
#endif
	if( act->pad != act->keep_pad ){
		 act->CheckPad( act );
	}
	/**/

	switch(act->move_s){
		/* 低速 */
		case MoveWalk:
//			speed = LOW_SPEED ;
			act->mot_speed_correct = 0.8F;
			break;
		/* 中速 */
		case MoveRun :
			break;
		/* 中速 先頭は銃構え */
		case MoveWalkGun :
			break;
		/* 高速 */
		case MoveAttackRun :
			act->mot_speed_correct = 1.2F;
//			speed = HI_SPEED ;
			break;
		default :
			break;
	}

	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0,   act->keep_mot, 0, MOTION_MASK_FULL,  
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;

	}



}
static void AT_ActMukadeMoveLowShl( act, time )
ACTION	*act ;
int		time ;
{
	float speed;

	AT_SetActStatus( act,(ACT_STATUS_STAND|ACT_STATUS_MOVE|ACT_STATUS_IK_MUKADE));
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
		AT_SetAction( act, 0,   act->keep_mot, 0, MOTION_MASK_FULL,  
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;

		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND_SHIELD ) ;
#if 1
		MotLayer_Shield(act);
#endif
//		Layer_Off(act,1);
	}

	if( act->pad != act->keep_pad ){
		 act->CheckPad( act );
	}
	/**/

#if 1
	switch(act->move_s){
		/* 低速 */
		case MoveWalk:
			speed = LOW_SPEED ;
			break;
		/* 中速 */
		case MoveRun :
			speed = MID_SPEED ;
			break;
		/* 高速 */
		case MoveAttackRun :
			speed = HI_SPEED ;
			break;
		default :
			speed = 0.0F ;
			break;
	}

	act->ctrl->step.vx = speed * _RsinF( (int)act->dir ) ;
	act->ctrl->step.vz = speed * _RcosF( (int)act->dir ) ;
#endif


}



static void ActStandReloadShield( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
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
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,ACT_INTERP_M ) ;
		Layer_Off(act,1);
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
}

static void ActSquatReload( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_SHAGAMI | ACT_STATUS_ADJ_X ) ;
	if ( act->CheckDamage( act ) ) {
		act->sw->magg = 1 ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M );
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_SQUAT ) ;
		Layer_Off(act,1);
	}


	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->magg = 1 ;
			return ;
		}
	}

	if ( (BP_IsPAL()!=TRUE) && time == 19 ) act->sw->magg = 4 ;
	if ( (BP_IsPAL()!=TRUE) && time == 66 ) act->sw->magg = 2 ;
	if ( (BP_IsPAL()!=TRUE) && time == 101 ) act->sw->magg = 1 ;
	if ( (BP_IsPAL()==TRUE) && time == 16 ) act->sw->magg = 4 ;
	if ( (BP_IsPAL()==TRUE) && time == 55 ) act->sw->magg = 2 ;
	if ( (BP_IsPAL()==TRUE) && time == 84 ) act->sw->magg = 1 ;

	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->sw->magg = 1 ;
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
}

/*張り付き*/
static void AT_ActBehind( act, time )
ACTION	*act ;
int		time ;
{
	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	act->ctrl->s_sphere = ENE_BEHIND_SPHERE;
	act->ctrl->r_sphere = ENE_BEHIND_SPHERE+10;
	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		Layer_Off(act,1);
	}


#if 0
	if ( act->CheckPad( act ) ) {
		return ;
	}
#else
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
#endif

}



#if 1
static void AT_ActShlBreak(act, time)
ACTION	*act ;
int		time ;
{
	if ( act->CheckDamage( act ) ) {
		act->sw->sub_weapon = 0;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
//	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, 
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;

		Layer_Off(act,1);
	}


	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->sub_weapon = 0;
			return ;
		}
	}

	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		act->sw->sub_weapon = 0;
		AT_SetMode( act, ENE_ActStandStill ) ;
//		AT_SetMode( act, AT_ActReadyGun ) ;
	}
	

}

#endif


void AT_ActStandNoAdj( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	ctrl = act->ctrl ;
	AT_SetActStatus( act, ACT_STATUS_STAND) ;
	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
		ACT_INTERP_M ) ;
		TrgSizeSetNormal(act);
		Layer_Off(act,1);
	}

	if( act->pad != act->keep_pad ){
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
	}
}




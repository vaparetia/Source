//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ataction.c
	attacker チェックパッド

	1999/07/06 K.Sigeno
	$Id: ataction.c,v 1.1.1.3 2002/11/19 11:48:59 Yoshizawa1 Exp $
*/

#include	"at_act.c"
#include	"../conv/conv_act.x"

int	AT_AlertCheckPad( act )
ACTION	*act ;
{


	switch ( act->pad ) {
		case SP_READYGUN :
			if(act->bodyp.type & ENE_TYPE_SHOTGUN){
				act->keep_mot = EM_gbs_sps_nom_fire_p ;
			}else if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_m4_nom_fire_p_high ;
			}else {
				act->keep_mot = EM_kamae_gun_high ;
//				act->keep_mot = EM_idle_holdup ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE) AT_ActReadyGun ) ;
			return 1 ;
		break ;
		case SP_READYGUN_PB :
			if(act->bodyp.type & ENE_TYPE_SHOTGUN){
				act->keep_mot = EM_gbs_sps_nom_fire_p ;
			}else if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_m4_nom_fire_p_high ;
			}else {
				act->keep_mot = EM_kamae_gun_high ;
//				act->keep_mot = EM_idle_holdup ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActReadyGunPbreak ) ;
			return 1 ;
		break ;
		case SP_MEDICATION :
printf("SP_MEDICATION SET!!!\n") ;
			act->keep_mot = EM_legl_morph_legl ;
			act->keep_pad = act->pad ;
			ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
			AT_SetMode( act,(ACTIONMODE) ENE_ActMedication ) ;
			return 1 ;
		break ;
		case SP_WAIT :
			if(act->bodyp.type & ENE_TYPE_SHOTGUN){
				act->keep_mot = EM_gbs_sps_nom_fire_p ;
			}else if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_m4_nom_fire_p_high ;
			}else {
				act->keep_mot = EM_kamae_gun_high ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActStandWait ) ;
			return 1 ;
		break ;

		case SP_SHOOTGUN :
			if(act->bodyp.type & ENE_TYPE_SHOTGUN){
#if 1
//				act->keep_mot = EM_gbs_sps_nom_fire ;
				act->keep_mot = EM_gbs_sps_nom_fire2 ;
				act->keep_pad = act->pad ;
				AT_SetMode( act, (ACTIONMODE)AT_ActShotSPS);
#else 
				act->keep_mot = EM_gbs_sps_nom_fire2 ;
				act->keep_pad = act->pad ;
				AT_SetMode( act,(ACTIONMODE) AT_ActShotSPS2);
#endif

			}else {
				if(act->bodyp.type & ENE_TYPE_HITECH){
					act->keep_mot = EM_htc_m4_nom_fire_p_high ;
				}else {
					act->keep_mot = EM_kamae_gun_high ;
//					act->keep_mot = EM_idle_holdup ;
				}
				act->keep_pad = act->pad ;
				AT_SetMode( act, (ACTIONMODE)AT_ActShootGun ) ;
			}
			return 1 ;
		break ;
		case SP_SPS_SETUP :
//if(act->bodyp.type & ENE_TYPE_SHOTGUN)
			act->keep_mot = EM_gbs_sps_nom_fire_p ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActReadyGun ) ;
			return 1 ;
		break ;
		case SP_SPS_FIRE :
			act->keep_mot = EM_gbs_sps_nom_fire ;
			act->keep_pad = act->pad ;
//			AT_SetMode( act, AT_ActShootGun ) ;
			AT_SetMode( act, (ACTIONMODE)AT_ActOneTimeTurn);
			return 1 ;
		break ;
		case SP_SQUATGUN :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_m4_nom_squat_fire ;
//				act->keep_mot = EM_squat_fire ;
			}else {
				act->keep_mot = EM_squat_fire ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActSquatReadyGun ) ;
			return 1 ;
		break ;
		case SP_SQUATSHOOTGUN :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_m4_nom_squat_fire ;
			}else {
				act->keep_mot = EM_squat_fire ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActSquatShootGun ) ;
			return 1 ;
		break ;
		case SP_SQUATHIDE :
			act->keep_mot = EM_squat_hide_idle ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActReadyGun ) ;
			return 1 ;
		break ;
		case SP_INTRUDE_FIRE :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_ak_nom_fire_intrude ;
			}else {
				act->keep_mot = EM_squat_fire_intrude ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActSquatNoAdj ) ;
			return 1 ;
		break ;
		case SP_ATTACK_KICK :
			act->keep_mot = EM_attack_near ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActPunchFAMAS ) ;
			return 1 ;
		break ;
		case SP_ATTACK_PUNCH_L :
			act->keep_mot = EM_ak_attack_near_punch_l ;
			act->keep_pad = act->pad ;
			ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
			AT_SetMode( act,(ACTIONMODE) ENE_ActNearAtk ) ;
			return 1 ;
		break ;
		case SP_ATTACK_KICK_L :
			act->keep_mot = EM_ak_attack_near_kick_l ;
			act->keep_pad = act->pad ;
			ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
			AT_SetMode( act, (ACTIONMODE)ENE_ActNearAtk ) ;
			return 1 ;
		break ;
		case SP_ATTACK_PUNCH_R :
			act->keep_mot = EM_ak_attack_near_punch_r ;
			act->keep_pad = act->pad ;
			ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
			AT_SetMode( act, (ACTIONMODE)ENE_ActNearAtk ) ;
			return 1 ;
		break ;
		case SP_ATTACK_KICK_R :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_m4_nom_attack_near ;
			}else {
				act->keep_mot = EM_ak_attack_near_kick_r ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)ENE_ActNearAtk ) ;
			return 1 ;
		break ;
		case SP_CLE_GOSIGN :
			/*クリアリング用*/
//			act->keep_mot = EM_cle_signe_go ;
			/*戦闘用*/
			act->keep_mot = EM_nom_signe_go_atk ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActOneTimeTurn);
			return 1 ;
		break ;

		case SP_RELOAD :
			if(act->bodyp.type & ENE_TYPE_SHOTGUN){
				act->keep_mot = EM_gbs_sps_nom_fire_reload ;
			}else if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_m4_nom_fire_reload ;
			}else {
				act->keep_mot = EM_reload ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)ActStandReload ) ;
			return 1 ;
		break ;
		case SP_RELOAD_SQUAT :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_m4_nom_squat_fire_reload ;
			}else {
				act->keep_mot = EM_squat_fire_reload ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)ActSquatReload ) ;
			return 1 ;
		break ;

		case SP_TURN_GO_L :
			act->keep_mot = EM_tobidasi_l_t ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActOneTimeTurn ) ;
			return 1 ;
		break ;
		case SP_TURN_GO_R :
			act->keep_mot = EM_tobidasi_r_t ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActOneTimeTurn ) ;
			return 1 ;
		break ;

		case SP_MOVE_RUN :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_m4_nom_run_atk;
			}else {
				act->keep_mot = EM_run_atk;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActMoveRun);
			return 1 ;
		break ;
		case SP_BIKKURI :
			act->keep_mot = ENE_DiscoveryMotion(act) ;
			act->keep_pad = act->pad ;
//			AT_SetMode( act, (ACTIONMODE)AT_ActOneTimeTurn);
			ENE_SetBodyTargetSize( &(act->bodyp.deftrg),ENE_BODYTRGSIZE_STAND ) ;
			AT_SetMode( act, (ACTIONMODE)AT_ActDiscovery );
			return 1 ;
		break ;
		case SP_BIKKURI_HIDE :
			act->keep_mot = EM_avoid_hang ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActSurpHide ) ;
			return 1 ;
		break ;
		case SP_ROLL_L :
			act->keep_mot = EM_squat_roll_l ;
			act->keep_pad = act->pad ;
//			AT_SetMode( act, (ACTIONMODE)AT_ActOneTimeTurn_Squat);
			AT_SetMode( act, (ACTIONMODE)AT_ActNoCancel);
			return 1 ;
		break ;
		case SP_ROLL_R :
			act->keep_mot = EM_squat_roll_r ;
			act->keep_pad = act->pad ;
//			AT_SetMode( act, (ACTIONMODE)AT_ActOneTimeTurn_Squat);
			AT_SetMode( act, (ACTIONMODE)AT_ActNoCancel);
			return 1 ;
		break ;
/*11_01 １タイムモーションに変更*/
		case SP_STEP_L :
			act->keep_mot = EM_step_l ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActOneTimeTurn);
			return 1 ;
		break ;
		case SP_STEP_R :
			act->keep_mot = EM_step_r ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActOneTimeTurn);

			return 1 ;
		break ;
		case SP_MUKADE_WALK_LOW :
			act->keep_mot = EM_run;
 			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActMukadeMoveLow ) ;
			return 1 ;
		break;
		case SP_BEHIND :
			act->keep_mot = EM_behind_idle ;
 			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActBehind ) ;
			return 1 ;
		break;
		case SP_SHL_BREAK :
			act->keep_mot = EM_shl_nom_dam_gun ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActShlBreak);
			return 1 ;
		break ;
		case SP_STOMP :		/*下段攻撃*/
			act->keep_mot = EM_nom_stomp ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)ENE_ActNearAtk);
			return 1 ;
		break ;
		case SP_BEYOND :	/*エルード攻撃*/
			act->keep_mot = EM_nom_stomp ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActBeyondAtk);
			return 1 ;
		case SP_STOMP_WAKE :	/*起こし*/
			act->keep_mot = EM_nom_turn_out ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActWakeStomp);
			return 1 ;
		break ;
		case SP_GRD_LOW :
//printf("SP_GRD_LOW!!!!!!!\n");
//			act->keep_mot = EM_gm_fire_low ;
			act->keep_mot = EM_gm_fire_korokoro ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) ActGrdThrLow ) ;
			return 1 ;
		break ;
		case SP_GRD_HIGH :
			act->keep_mot = EM_gm_fire_high ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) ActGrdThrHigh ) ;
			return 1 ;
		break ;
		case SP_NSIGHT_ON : /*暗視ゴーグル付ける*/
			act->keep_mot = EM_gogle_on ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) ENE_ActNSight_ON ) ;
			return 1 ;
		break ;
		case SP_PEEK_HIGH_R :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_ak_nom_peek_r_start ;
			}else {
				act->keep_mot = EM_gbs_ak_nom_peek_r_start ;
			}
			act->keep_pad = act->pad ;
//			AT_SetMode( act,(ACTIONMODE) AT_ActPeek ) ;
			AT_SetMode( act, (ACTIONMODE)AT_ActOneTimeTurn ) ;
			return 1 ;
		break ;
		case SP_PEEK_HIGH_IDLE_R :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_ak_nom_peek_r_fire ;
			}else {
				act->keep_mot = EM_gbs_ak_nom_peek_r_fire ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActOneTimeTurn ) ;
			return 1 ;
		break ;
		case SP_PEEK_HIGH_END_R :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_ak_nom_peek_r_end ;
			}else {
				act->keep_mot = EM_gbs_ak_nom_peek_r_end ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActOneTimeTurn ) ;
//			AT_SetMode( act, AT_ActReturn ) ;
			return 1 ;
		break ;
		case SP_PEEK_HIGH_L :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_ak_nom_peek_l_start ;
			}else {
				act->keep_mot = EM_gbs_ak_nom_peek_l_start ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActOneTimeTurn ) ;
//			AT_SetMode( act,(ACTIONMODE) AT_ActPeek ) ;
			return 1 ;
		break ;
		case SP_PEEK_HIGH_IDLE_L :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_ak_nom_peek_l_fire ;
			}else {
				act->keep_mot = EM_gbs_ak_nom_peek_l_fire ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActOneTimeTurn ) ;
//			AT_SetMode( act, (ACTIONMODE)AT_ActPeek ) ;
			return 1 ;
		break ;
		case SP_PEEK_HIGH_END_L :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_ak_nom_peek_l_end ;
			}else {
				act->keep_mot = EM_gbs_ak_nom_peek_l_end ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActOneTimeTurn ) ;
			return 1 ;
		break ;

		case SP_STOPSIGN :
			act->keep_mot = EM_cle_signe_stop ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActOneTimeTurn ) ;
			return 1 ;
		break ;
		case SP_OPEN_LOCKER :
			act->keep_mot = EM_cle_locker_open ;
			act->keep_pad = act->pad ;
//			AT_SetMode( act,(ACTIONMODE) AT_ActOneTimeTurn ) ;
			AT_SetMode( act, (ACTIONMODE)ENE_ActOneTimeMotion) ;
			return 1 ;
		break ;
		case SP_BACKWALK :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_m4_nom_walk_b_atk ;
			}else {
				act->keep_mot = EM_walk_b_atk ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)AT_ActOneTimeTurn) ;
			return 1 ;
		break ;
#if 0
		case SP_GRD_RELOAD :
			/*グレネードランチャー装填　モーション未発注*/
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_m4_nom_fire_reload ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, (ACTIONMODE)Act_GrdM4Shoot) ;
			return 1 ;
		break ;
#endif
		/*ぐれね装填＆発射*/
		case SP_GRD_M4_SHOOT :
//			act->keep_mot = EM_htc_m4_nom_fire_p_high ;
			act->keep_mot = EM_htc_m4_nom_fire_grenade ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) Act_GrdM4Shoot) ;
			return 1 ;
		break ;
		case SP_AIM_POS :
			if(act->bodyp.type & ENE_TYPE_SHOTGUN){
				act->keep_mot = EM_gbs_sps_nom_fire_p ;
			}else if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_m4_nom_fire_p_high ;
			}else {
				act->keep_mot = EM_kamae_gun_high ;
			}
			act->keep_pad = act->pad ;
//			AT_SetMode( act,(ACTIONMODE) AT_ActStandNoAdj ) ;
			AT_SetMode( act,(ACTIONMODE) ENE_ActKeepMotion ) ;
			return 1 ;
		break ;
		case SP_DISCOVERY :
			act->keep_mot = ENE_DiscoveryMotion(act) ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) SIG_ActDiscovery ) ;
			return 1 ;
		break ;
	}
	return 0 ;
}
//grd
int	AT_AlertPadShl( act )
ACTION	*act ;
{
	switch ( act->pad ) {
		case SP_STAND_DEFENCE :
			act->keep_mot = EM_shl_nom_defence_idle;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActReadyGunShl ) ;
			return 1 ;
		break ;
		case SP_WAIT :
			act->keep_mot = EM_shl_nom_defence_idle;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActReadyGunShl ) ;
			return 1 ;
		break ;

		case SP_READYGUN :
			act->keep_mot = EM_shl_nom_idle ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActReadyGunShl ) ;
			return 1 ;
		break ;
		case SP_SHL_SIDE_READYGUN :
			act->keep_mot = EM_shl_nom_fire_side_p ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActReadyGunShl ) ;
			return 1 ;
		break ;

		case SP_READYGUN_PB :
			act->keep_mot = EM_shl_nom_idle ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActReadyGunPbreak ) ;
			return 1 ;
		break ;
		case SP_MEDICATION :
printf("SP_MEDICATION SET!!!\n") ;
			act->keep_mot = EM_legl_morph_legl ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) ENE_ActMedication ) ;
			return 1 ;
		break ;


		case SP_SHOOTGUN :
			act->keep_mot = EM_shl_nom_idle ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActShootGunShl ) ;
			return 1 ;
		break ;
		case SP_SHL_SIDE_SHOOTGUN :
			act->keep_mot = EM_shl_nom_fire_side_p ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActShootGunShl ) ;
			return 1 ;
		break ;
		case SP_SQUAT_DEFENCE :
			act->keep_mot = EM_shl_nom_defence_squat;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActSquatReadyGunShl ) ;
			return 1 ;
		break ;
		case SP_SQUATGUN :
			act->keep_mot = EM_shl_nom_squat_fire;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActSquatReadyGunShl ) ;
			return 1 ;
		break ;
		case SP_SQUATSHOOTGUN :
			act->keep_mot = EM_shl_nom_squat_fire;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActSquatShootGunShl ) ;
			return 1 ;
		break ;
		case SP_SQUATHIDE :
			act->keep_mot = EM_shl_nom_squat_fire;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActReadyGunShl ) ;
			return 1 ;
		break ;
		case SP_INTRUDE_FIRE :
			act->keep_mot = EM_gbs_shl_nom_fire_intrude ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActSquatNoAdj ) ;
			return 1 ;
		break ;
		case SP_ATTACK_KICK :
			act->keep_mot = EM_shl_nom_attack_near ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActPunchShl ) ;
			return 1 ;
		break ;
		case SP_SHL_PUSH :
			/*盾殴り版*/
			act->keep_mot = EM_shl_nom_attack_near_shl ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActPunchShl ) ;
			return 1 ;
		break ;
		case SP_KICK_LOCKER : /*ロッカーを蹴る*/
			act->keep_mot = EM_shl_nom_attack_near ;
			act->keep_pad = act->pad ;
//			AT_SetMode( act,(ACTIONMODE) AT_ActPunchShl ) ;
			AT_SetMode( act,(ACTIONMODE) AT_ActKickLocker ) ;
			return 1 ;
		break ;

		case SP_CLE_GOSIGN :
			act->keep_mot = EM_shl_nom_signe_go ;
//			act->keep_mot = EM_nom_signe_go_atk ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActOneTimeTurn);
			return 1 ;
		break ;
			return 1 ;
		break;
		case SP_RELOAD :
//			act->keep_mot = EM_reload ;
			act->keep_mot = EM_shl_nom_mkr_reload ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) ActStandReloadShield ) ;
			return 1 ;
		break ;
		case SP_RELOAD_SQUAT :
			act->keep_mot = EM_shl_nom_mkr_reload ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) ActStandReloadShield ) ;
			return 1 ;
		break ;

		case SP_TURN_GO_L :
			act->keep_mot = EM_tobidasi_l_t ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActOneTimeTurnShl ) ;
			return 1 ;
		break ;
		case SP_TURN_GO_R :
			act->keep_mot = EM_tobidasi_r_t ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActOneTimeTurnShl ) ;
			return 1 ;
		break ;

		case SP_MOVE_RUN :
			act->keep_mot = EM_shl_nom_run;
			act->keep_pad = act->pad ;
//			AT_SetMode( act,(ACTIONMODE) AT_ActMoveShield ) ;
			AT_SetMode( act,(ACTIONMODE) AT_ActMoveRun ) ;
			return 1 ;
		break ;

		case SP_BACKWALK :
			act->keep_mot = EM_shl_nom_walk_b_atk ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActOneTimeTurn) ;
			return 1 ;

		case SP_BIKKURI :
			act->keep_mot = ENE_DiscoveryMotion(act) ;
			act->keep_pad = act->pad ;
//			AT_SetMode( act,(ACTIONMODE) AT_ActOneTimeTurnShl);
			AT_SetMode( act,(ACTIONMODE) AT_ActDiscoveryShl );
			return 1 ;
		break ;
		case SP_BIKKURI_HIDE :
			act->keep_mot = EM_avoid_hang ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActSurpHide ) ;
			return 1 ;
		break ;
		case SP_STEP_L :
			act->keep_mot = EM_step_l ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActOneTimeTurnShl);
			return 1 ;
		break ;
		case SP_STEP_R :
			act->keep_mot = EM_step_r ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActOneTimeTurnShl);
			return 1 ;
		break ;
		case SP_MUKADE_WALK_LOW :
//			act->keep_mot = EM_cau_mukade_idle;
			act->keep_mot = EM_run;
			//	EM_walk,
			//	EM_run,
 			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActMukadeMoveLowShl ) ;
			return 1 ;
		break;
		case SP_SHL_BREAK :
			act->keep_mot = EM_shl_nom_dam_gun ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActShlBreak);
			return 1 ;
		break ;
		case SP_STOMP :	/*下段攻撃*/
			act->keep_mot = EM_shl_nom_stomp ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) ENE_ActNearAtk);
			return 1 ;
		break ;
		case SP_BEYOND :	/*エルード攻撃*/
			act->keep_mot = EM_shl_nom_stomp ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActBeyondAtk);
			return 1 ;
		case SP_STOMP_WAKE :	/*起こし*/
			act->keep_mot = EM_shl_nom_stomp ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) AT_ActWakeStomp);
			return 1 ;
		break ;
		case SP_GRD_HIGH :
//			act->keep_mot = EM_gm_fire_high ;
			act->keep_mot = EM_gbs_shl_nom_gm_fire_high ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) ActGrdThrHigh ) ;
			return 1 ;
		break ;
		case SP_GRD_LOW :
			act->keep_mot = EM_gm_fire_low ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) ActGrdThrLow ) ;
			return 1 ;
		break ;
		case SP_NSIGHT_ON : /*暗視ゴーグル付ける*/
			act->keep_mot = EM_gogle_on ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) ENE_ActNSight_ON ) ;
			return 1 ;
		break ;
		case SP_AIM_POS :
			act->keep_mot = EM_shl_nom_idle ;
			act->keep_pad = act->pad ;
//			AT_SetMode( act,(ACTIONMODE) AT_ActStandNoAdj ) ;
			AT_SetMode( act,(ACTIONMODE) ENE_ActKeepMotion ) ;

			return 1 ;
		case SP_DISCOVERY :
			act->keep_mot = ENE_DiscoveryMotion(act) ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,(ACTIONMODE) SIG_ActDiscovery ) ;
			return 1 ;
		break ;

	}
	return 0 ;
}


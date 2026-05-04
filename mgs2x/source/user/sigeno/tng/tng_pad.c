//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	tng_pad.c
	天狗アクションパッド
	2001/02/06 K.Sigeno
	$Id: tng_pad.c,v 1.1.1.3 2002/11/19 11:49:54 Yoshizawa1 Exp $
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

#include	"../attacker/at_enum.h"
#include	"../attacker/at_act.x"
#include	"../conv/conv_act.x"
#include	"tng.h"

extern void AT_ActMoveRun( ACTION * ,int) ;
extern void ActGrdThrHigh( ACTION * ,int) ;
extern void TNG_ActNearAtk( ACTION * ,int) ;
extern void TNG_ActBladeGuard( ACTION * ,int) ;
extern void ActHangStart( ACTION * ,int) ;
extern void TNG_ActSquatNoAdj( ACTION * ,int) ;


#define MT_REV_ALL (ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D )


int	TNG_AlertCheckPad( act )
ACTION	*act ;
{
extern void ActTngJumpStart( ACTION *, int ) ;
extern void ActTngJumpStartLevel( ACTION *, int ) ;
	int mot_rnd,tmp_mot ;

	switch ( act->pad ) {
		case SP_READYGUN :
			if( act->bodyp.type & ENE_TYPE_KATANA){
				act->keep_mot = EM_tng_tbl_nom_idle ;
			}else {
				act->keep_mot = EM_tng_p90_nom_fire ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActReadyGun ) ;
			return 1 ;
		break ;
		case SP_SHOOTGUN :
//			act->keep_mot = EM_kamae_gun_high ;
			act->keep_mot = EM_tng_p90_nom_fire ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActShootGun ) ;
			return 1 ;
		break ;
		case SP_SQUATGUN :
			act->keep_mot = EM_tng_p90_nom_squat_fire ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActSquatReadyGun ) ;
			return 1 ;
		break ;
		case SP_INTRUDE_FIRE :
			act->keep_mot = EM_tng_p90_nom_squat_fire ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, TNG_ActSquatNoAdj ) ;
			return 1 ;
		break ;
		case SP_SQUATSHOOTGUN :
			act->keep_mot = EM_tng_p90_nom_squat_fire ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActSquatShootGun ) ;
			return 1 ;
		break ;
		case SP_SQUATHIDE :
			act->keep_mot = EM_tng_p90_nom_squat_fire ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActReadyGun ) ;
			return 1 ;
		break ;
		case SP_ATTACK_PUNCH_L :
			if(!(act->bodyp.type & ENE_TYPE_KATANA )){
				long64 pl_status ;
				pl_status = GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ;

				mot_rnd = ((irnd()>>8)&7) ;
				switch(mot_rnd) {
					case 0 :
						act->keep_mot = 
							EM_tng_p90_nom_attack_near_spinkick_high ;
						break ;
					case 1 :
						act->keep_mot = 
						EM_tng_p90_nom_attack_near_shoutei ;
					break ;
					case 2 :
						act->keep_mot = 
							EM_tng_p90_nom_attack_near_somersault ;
						break ;
					case 3 :
						act->keep_mot = 
							EM_tng_p90_nom_attack_near_elbow ;
						break ;
					case 4 :
						act->keep_mot = 
							EM_tng_p90_nom_attack_near_kneekick ;
						break ;
					case 5 :
						act->keep_mot = 
							EM_tng_p90_nom_attack_near_spinkick_low ;
						break ;
					case 6 :
						if(pl_status){
							/*はだかライデン*/
							act->keep_mot = 
								EM_tng_p90_nom_attack_near_spinkick_low ;
						}else {
							act->keep_mot = 
								EM_tng_p90_nom_attack_near_spiraluper ;
						}
						break ;
					case 7 :
						if(pl_status){
							/*はだかライデン*/
							act->keep_mot = 
								EM_tng_p90_nom_attack_near_spinkick_low ;
						}else {
							act->keep_mot = 
								EM_tng_p90_nom_sliding ;
						}
						break ;
				}
			}else {
				mot_rnd = ((irnd()>>8)&7) ;
				if(mot_rnd <= 3 ){
/*上段*/
					act->keep_mot = EM_tng_tbl_nom_attack1 ;
				}else if(mot_rnd <= 6 ){
/*下段*/
					act->keep_mot = EM_tng_tbl_nom_attack2 ;
				}else {
/*突き ガード不能なので頻度下げる*/
					act->keep_mot = EM_tng_tbl_nom_attack3 ;
				}
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, TNG_ActNearAtk ) ;
			return 1 ;
		break ;
		case SP_HANG_START :
#if 0
/*モーションができたらここで組み込み*/
			if(!(act->bodyp.type & ENE_TYPE_KATANA )){
				act->keep_mot = 
					EM_tng_p90_nom_attack_near_spinkick_high ;
			}else {
			}
#endif
			act->keep_pad = act->pad ;
			AT_SetMode( act, ActHangStart ) ;
			return 1 ;
		break ;
		case SP_ATTACK_KICK_L :
//			act->keep_mot = EM_tng_p90_nom_attack_near_shoutei ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ActTngJumpStart ) ;
			return 1 ;
		break ;
		case SP_CLE_GOSIGN :
			act->keep_mot = EM_nom_signe_go_atk ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn);
			return 1 ;
		break ;

		case SP_RELOAD :
//			act->keep_mot = EM_reload ;
			act->keep_mot = EM_tng_p90_nom_fire_reload ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ActStandReload ) ;
//			AT_SetMode( act, AT_ActOneTimeTurn);
			return 1 ;
		break ;
		case SP_RELOAD_SQUAT :
			act->keep_mot = EM_squat_fire_reload ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ActStandReload ) ;
//			AT_SetMode( act, ActSquatReload ) ;
			return 1 ;
		break ;

		case SP_TURN_GO_L :
			act->keep_mot = EM_tobidasi_l_t ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn ) ;
			return 1 ;
		break ;
		case SP_TURN_GO_R :
			act->keep_mot = EM_tobidasi_r_t ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn ) ;
			return 1 ;
		break ;

		case SP_MOVE_RUN :
			act->keep_mot = EM_run_atk;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActMoveRun);
			return 1 ;
		break ;
		case SP_ROLL_L :
//			act->keep_mot = EM_squat_roll_l ;
			act->keep_mot = EM_tng_p90_nom_somersault2 ;
			act->keep_pad = act->pad ;
//			AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			AT_SetMode( act, AT_ActNoCancel);
			return 1 ;
		break ;
		case SP_ROLL_R :
//			act->keep_mot = EM_squat_roll_r ;
			act->keep_mot = EM_tng_p90_nom_somersault2 | MT_REV_ALL;
			act->keep_pad = act->pad ;
//			AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			AT_SetMode( act, AT_ActNoCancel);
			return 1 ;
		break ;
/*11_01 １タイムモーションに変更*/
		case SP_STEP_L :
			act->keep_mot = EM_tng_p90_nom_roll_l ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn);
			return 1 ;
		break ;
		case SP_STEP_R :
//			act->keep_mot = EM_step_r ;
			act->keep_mot = EM_tng_p90_nom_roll_l | MT_REV_ALL;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn);

			return 1 ;
		break ;
		case SP_STOMP :
			act->keep_mot = EM_tng_p90_nom_stamp ;
			act->keep_pad = act->pad ;
//			AT_SetMode( act, AT_ActNomStomp);
			AT_SetMode( act, TNG_ActNearAtk ) ;
			return 1 ;
		break ;
#if 0
		case SP_STOMP_WAKE :
//			act->keep_mot = EM_nom_stomp_wake ;
			act->keep_mot = EM_tng_p90_nom_stamp ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActNomStomp);
			return 1 ;
		break ;
#endif
		case SP_GRD_LOW :
//printf("SP_GRD_LOW!!!!!!!\n");
//			act->keep_mot = EM_gm_fire_low ;
			act->keep_mot = EM_gm_fire_korokoro ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ActGrdThrLow ) ;
			return 1 ;
		break ;
		case SP_GRD_HIGH :
			act->keep_mot = EM_gm_fire_high ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ActGrdThrHigh ) ;
			return 1 ;
		break ;
		case SP_OPEN_LOCKER :
			act->keep_mot = EM_cle_locker_open ;
			act->keep_pad = act->pad ;
//			AT_SetMode( act, AT_ActOneTimeTurn ) ;
			AT_SetMode( act, ENE_ActOneTimeMotion) ;
			return 1 ;
		break ;
		case SP_BACKWALK :
			act->keep_mot = EM_tng_p90_nom_walk_b_atk ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn) ;
			return 1 ;
		break ;
		case SP_TNG_JUMP :
//			act->keep_mot = EM_tng_p90_nom_attack_near_shoutei ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ActTngJumpStart ) ;
			return 1 ;
		case SP_TNG_JUMP_LEVEL :
//			act->keep_mot = EM_tng_p90_nom_attack_near_shoutei ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ActTngJumpStartLevel ) ;
			return 1 ;
		case SP_BACK_TURN :
			act->keep_mot = EM_tng_p90_nom_somersault;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActNoCancel);
			return 1 ;
		case SP_SLIDING :
			act->keep_mot = EM_tng_p90_nom_sliding ;
			act->keep_pad = act->pad ;
//			AT_SetMode( act, AT_ActNoCancel);
			AT_SetMode( act, TNG_ActNearAtk ) ;
			return 1 ;
		case SP_JUMP_ATTACK :
			if(act->bodyp.type & ENE_TYPE_KATANA ){
				act->keep_mot = EM_tng_tbl_nom_leap_attack ;
			}else {
				act->keep_mot = EM_tng_p90_nom_leap ;
			}
			act->keep_pad = act->pad ;
//			AT_SetMode( act, AT_ActNoCancel ) ;
			AT_SetMode( act, TNG_ActNearAtk ) ;
			return 1 ;
		case SP_KATANA_GUARD :
//			mot_rnd = ((irnd()>>8)%3) ;
			switch(act->keep_mot) {
				case EM_tng_tbl_guard_2 :
					tmp_mot = EM_tng_tbl_guard_3 ;
					act->tmp_time = KATANA_ROLL_GUARD_INTR ;
					break ;
				case EM_tng_tbl_guard_3 :
					tmp_mot = EM_tng_tbl_guard_1 ;
					act->tmp_time = KATANA_GUARD_INTR ;
					break ;
				case EM_tng_tbl_guard_1 :
					tmp_mot = EM_tng_tbl_guard_2 ;
					act->tmp_time = KATANA_GUARD_INTR ;
					break ;
				default :
					tmp_mot = EM_tng_tbl_guard_2 ;
					act->tmp_time = KATANA_GUARD_INTR ;
			}
			act->keep_mot = tmp_mot ;
			act->keep_pad = act->pad ;
			AT_SetMode( act,TNG_ActBladeGuard) ;
			return 1 ;
	}

	return 0 ;
}

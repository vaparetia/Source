//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	conv_act.c
	敵兵 汎用関数
	2001/02/07 K.Sigeno
	$Id: conv_act.c,v 1.1.1.3 2002/11/19 11:49:06 Yoshizawa1 Exp $
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
#include	"korekado/enemy/motion.h"
//#include	"../tng/tng_motion.h"
#include	"libutl.h"

#include "BP_Misc.h"


//#include	"attacker.h"
//#include	"ataction.c"
//#include	"../defender/eve_a.h"

#define	AT_THK_RATE	(BP_BASE_TICK())

/*モーションスピード制御用*/
#define HITECH_MOVE_RATE	(0.60f)
#define ATTACKER_MOVE_RATE	(0.60f)
#define SHIELD_MOVE_RATE	(0.90f)

#define MT_REV_ALL (ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D )

void AT_ActNoCancel( ACTION *, int ) ;
void AT_ActAcrobat( ACTION *, int ) ;

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
//extern void SigZoneView( int , SVECTOR * ,float ) ; 
#endif

//extern void *NewLandingSmoke( FVECTOR * , float ) ;
extern void SIG_SetSpeak(int ,ACTION *);



/*足の着地タイミング*/
#define FOOTSMOKE (800.0f)
#if 0
static void FootCheck( act )
ACTION	*act ;
{
	MT3_CONTROL *mt3_ctrl ;
	int len, r_foot, l_foot, t ;
	mt3_ctrl = &act->body->m_ctrl->mt3_ctrl[0] ;
	len = COUNT_VMODE(mt3_ctrl->file_header->motion_length) ;
	t = mt3_ctrl->time ;
	
	l_foot = 1 + 6 ;
	r_foot = (len/2) + 1 + 8 ;
	
	if ( t == l_foot || t == l_foot+4 ) {
		pos = act->ctrl->mov ;
		pos.vy = act->ctrl->levels[0] ;
		NewLandingSmoke( &pos , FOOTSMOKE ) ;
	printf("FOOT SMOKE !!!!\n");
		return ;
	}
	if ( t == r_foot ) {
		pos = act->ctrl->mov ;
		pos.vy = act->ctrl->levels[0] ;
		NewLandingSmoke( &pos , FOOTSMOKE ) ;
	}
}
#endif
#if 1
static void Layer_Off(ACTION *act,int layer){
	act->body->m_ctrl->mt3_ctrl[ layer ].flag &= ~MT3_ACTIVE ;
}

#define LEFT_ARM_MASK (0x0600) /*手首と下の腕*/
static void MotLayer_Shield(ACTION *act){
	AT_SetAction( act, 1, EM_shl_nom_override, 0, (MOTION_MASK_UPPER), ACT_INTERP_DEF ) ;
	act->body->m_ctrl->mt3_ctrl[ 1 ].flag |= MT3_ACTIVE ;
}


#endif


#if 0
action.hに定義された補完値
ACT_INTERP_DEF		8*5	/* 1/300単位 */
ACT_INTERP_M		60*1 /* 1/300単位 */
ACT_INTERP_SLOWLY	60*5 /* 1/300単位 */
ACT_INTERP_VERY_SLOWLY	60*30 /* 1/300単位 */

グレネード銃口：	19.5, -653, 29.5

#endif

#define AT_GRD_MAX_DIS (4000.0F)
/*グレネード発射*/
/*
mode　0　必ずプレイヤの床位置
1　プレイヤ近辺の　開いてるとこ
2　必ずプレイヤFINDPOS
3　指定座標の近辺
4　指定座標の近辺で即爆破グレネードランチャー
5　回避不能グレネード
*/
int SIG_GrdCall(ACTION *act , FVECTOR *subtrg ,int mode){
	FVECTOR		pos,sub,trg0[2],trg1[2],testpos,trg;
	FVECTOR		flore_pos ;
	float		dis ,high;
	SVECTOR		rot;
	short		dir;
	int			i,ok = 0;


 extern void *NewMagicGrd( FVECTOR * ,FVECTOR * ,float ,int,int ,short);
	GV_MatToVec(&BODYWORLD( act->body, HUMAN21_MIGI_TE ),&pos);
	if(mode ==4){
		/*GRD 銃口シフト19.5, -653, 29.5*/
		FVECTOR grd_shift = {19.5f ,-653.0f,29.5f, 0.0f};

		DG_SetPos( &BODYWORLD( act->body, HUMAN21_MIGI_TE ) ) ;
		/* 銃口までシフト */
		DG_PutVector( &grd_shift, &pos, 1 ) ;
	}

//	GV_MatToVec(&BODYWORLD( act->body, HUMAN21_ATAMA ),&pos);

	trg = GM_PlayerFindPos ;
	flore_pos = GM_PlayerFindPos ;
	flore_pos.vy = GM_PlayerControl->levels[0] + 10.0F ;
	_sceVu0SubVector( &sub, &GM_PlayerFindPos, &pos ) ;

	if((mode == 3)||(mode == 4)||(mode == 5)){
		trg = *subtrg ;
		flore_pos = *subtrg ;
		flore_pos.vy = GM_PlayerControl->levels[0] + 10.0F ;
		_sceVu0SubVector( &sub, subtrg, &pos ) ;
	}

	dir = GV_VecDir2( &sub );
	dis = GV_VecLen3F( &sub );

	/*20001.01.19*/
	high = dis / 6.0F;

	rot.vx = 0;
	rot.vy = dir;
	rot.vz = 0;

	switch(mode){
		case 0 :
			/*プレイヤの床*/
			high = 100.0F;
			trg = flore_pos ;
		break;
			case 2 :
			/*プレイヤの頭部*/
			high = 100.0F;
		break;
		/*壁の開いてるとこ探す*/
		case 1 :
		case 3 :
		case 4 :
		case 5 :
/*目標が見えない場合*/
/*左右どちらかの点が見えてればそこを狙う*/
			if(!HZX_OnlineHazardCheck(GM_PlayerControl->hzx_id,
				&pos,&flore_pos,
				HZX_CHK_ALL,HZX_SEG_NO_ENEMY_EYES,
				HZX_FLOOR_NO_ENEMY ) ){
				/*直接狙う*/
				break ;
			}
			/*頭上が空いていれば*/
			testpos = flore_pos;
			testpos.vy += 1250.0F ;
			if(!HZX_OnlineHazardCheck(GM_PlayerControl->hzx_id,
				&pos,&testpos,HZX_CHK_ALL,HZX_SEG_NO_ENEMY_EYES,
				HZX_FLOOR_NO_ENEMY ) ){
				/*頭上を狙う*/
				trg = testpos ;
				high = 1000.0F;
				break ;
			}
			trg0[0] = trg0[1] = DG_ZeroVector ;
			trg0[0].vx =  1000.0F ;
			trg0[1].vx = -1000.0F ;
			DG_SetPos2( &flore_pos, &rot ) ;
			DG_PutVector( trg0, trg1, 2 );
			for(i=0;i<2;i++){
				if(!HZX_OnlineHazardCheck(GM_PlayerControl->hzx_id,
				&pos,&trg1[i],HZX_CHK_ALL,HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY ) ){
					/*見えた*/
					/*方向を再設定*/
					trg = trg1[i] ;
					ok = 1 ;
					break ;
				}
			}
		break;
	}
	switch (mode){
		case 0 :
			/*イントルード用 早めに爆発*/
			NewMagicGrd( &pos ,&trg ,high ,PLAYER_SIDE,DIRECT_TICK(120) ,0);
			break ;
		case 4 :
			/* M4GRD */
			high = 100.0F;
			NewMagicGrd( &pos ,&trg ,high ,PLAYER_SIDE,-1 ,3);
			break ;
		case 5 :
			/*no avoid*/
			high = 2000.0F;
			NewMagicGrd( &pos ,&trg ,high ,PLAYER_SIDE,-1 ,4);
			break ;
		default :
			NewMagicGrd( &pos ,&trg ,high ,PLAYER_SIDE,DIRECT_TICK(180) ,0);
			break ;
	}
	return 0;
}
#define TNG_ACROBAT_TIME (12)

/*攻撃兵 独自移動制御*/
void AT_ActMoveRun( act, time )
ACTION	*act ;
int		time ;
{
//	FVECTOR	tmp;
	int layer = 0;
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


#if 1
	AT_SetActStatus( act,(ACT_STATUS_STAND|ACT_STATUS_MOVE));

//	AT_SetActStatus( act, ACT_STATUS_SPHERE_100 ) ;


	if( act->bodyp.type & ENE_TYPE_KATANA)
	{
		AT_SetActStatus( act, ACT_STATUS_GURAD);
	}
#else
	AT_SetActStatus( act,(ACT_STATUS_STAND|ACT_STATUS_MOVE
	|ACT_STATUS_ADJ_X));
#endif

	if(act->dir >= 0){
		switch (act->move_s){
			case MoveAttackRun :
				if(act->tmp_time > 0){
					/*方向転換終わるまで静止*/
					act->tmp_time--;
					if(act->bodyp.type & ENE_TYPE_HITECH){
						act->keep_mot = EM_htc_m4_nom_fire_p_high ;
					}else {
						act->keep_mot = EM_kamae_gun_high ;
					}
				}else {
					if(act->bodyp.type & ENE_TYPE_HITECH){
						act->keep_mot = EM_htc_m4_nom_run_atk;
					}else {
						act->keep_mot = EM_run_atk;
					}
				}
				break;
			case MoveRun :
				if(act->bodyp.type & ENE_TYPE_HITECH){
					act->keep_mot = EM_htc_ak_nom_run;
				}else {
					act->keep_mot = EM_run ;
				}
				break;
			case MoveCautionWalk :
				if(act->bodyp.type & ENE_TYPE_HITECH){
					act->keep_mot = EM_htc_m4_nom_walk_atk;
				}else {
					act->keep_mot = EM_cle_walk;
				}
				break;
			case MoveSideL :
				if(act->bodyp.type & ENE_TYPE_HITECH){
					act->keep_mot = EM_htc_m4_nom_slide_fire_l ;
				}else {
					act->keep_mot = EM_slide_l_fire;
				}
				break;
			case MoveSideR :
				if(act->bodyp.type & ENE_TYPE_HITECH){
					act->keep_mot = EM_htc_m4_nom_slide_fire_r ;
				}else {
					act->keep_mot = EM_slide_r_fire;
				}
				break;
			case MoveBack :
				if(act->bodyp.type & ENE_TYPE_HITECH){
					act->keep_mot = EM_htc_m4_nom_walk_b_atk;
				}else {
					act->keep_mot = EM_walk_b_atk;
				}
				break;
			case MoveNoriNoriWalk :
				act->keep_mot = EM_norinori_walk ;
				break;
			case MoveWalkGun :
				if(act->bodyp.type & ENE_TYPE_HITECH){
					act->keep_mot = EM_htc_ak_nom_walk_light;
				}else {
					act->keep_mot = EM_walk_light ;
				}
				break;
			default :
printf("AT_ACT_MOVE_RUN REIGAI!!\n");
				act->keep_mot = EM_cle_walk;
				break;
		}
		act->dir &= 4095;
	}

	/*盾兵 わりこみ処理*/
	if( act->bodyp.type & ENE_TYPE_SHIELD){
		switch (act->keep_mot){
			case EM_run_atk :
			case EM_run :
				act->keep_mot = EM_shl_nom_run;
				Layer_Off(act,1);
				break ;
			case EM_cle_walk :
				act->keep_mot = EM_shl_nom_walk;
				Layer_Off(act,1);
				break ;
			default :
				layer = 1 ;
		/*レイヤーセットはmotセット後にすること*/
//				MotLayer_Shield(act);
				break ;
		}
	}
#if 1
	else if( act->bodyp.type & ENE_TYPE_KATANA){
		switch (act->keep_mot){
			case EM_run_atk :
				act->keep_mot = EM_tng_tbl_nom_run;
				Layer_Off(act,1);
				break ;
			case EM_cle_walk :
				act->keep_mot = EM_tng_tbl_nom_walk_f;
				Layer_Off(act,1);
				break ;
			case EM_slide_l_fire :
				act->keep_mot = EM_tng_tbl_nom_walk_l ;
				break;
			case EM_slide_r_fire :
				act->keep_mot = EM_tng_tbl_nom_walk_r ;
				break;
			case EM_walk_b_atk :
				act->keep_mot = EM_tng_tbl_nom_walk_b ;
				break;
		}
	}else if( act->bodyp.type & ENE_TYPE_TNG_A){
		long64 pl_status ;
		pl_status = GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ;

		switch (act->keep_mot){
			case EM_run_atk :
				if(pl_status){
					/* ぷれいや裸W42では警戒走り*/
					act->keep_mot = EM_tng_p90_cle_run;

				}else {
					/*天狗兵戦闘走り*/
					act->keep_mot = EM_tng_p90_nom_run;
				}
				Layer_Off(act,1);
				break ;
			case EM_cle_walk :
				act->keep_mot = EM_tng_p90_cle_walk;
				Layer_Off(act,1);
				break ;
			case EM_slide_l_fire :
					act->keep_mot = EM_tng_p90_nom_slide_fire_l ;
				break;
			case EM_slide_r_fire :
					act->keep_mot = EM_tng_p90_nom_slide_fire_r ;
				break;
			case EM_walk_b_atk :
				act->keep_mot = EM_tng_p90_nom_walk_b_atk ;
				break;
		}
	}
#endif	
//debug
//layer = 1 ;
//EM_shl_nom_run 231
	if (( time == 0 )||(act->c_motion_num[0] != act->keep_mot)) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),
		ENE_BODYTRGSIZE_STAND ) ;
		/*レイヤーセットはmotセット後にすること*/
		if(layer) {
			MotLayer_Shield(act);
		}else {
			Layer_Off(act,1);
		}
		if((act->bodyp.type & ENE_TYPE_HITECH)
		&&(act->keep_mot == EM_htc_m4_nom_run_atk)){
#if 1
			/*ハイテク兵は遅い*/
			MT_SetMotionSpeed( act->body->m_ctrl, 
				( float )TIME_BASE*HITECH_MOVE_RATE ) ;
#endif
		}else if(act->bodyp.type & ENE_TYPE_TNG_A){
			/*天狗は等倍*/
			MT_SetMotionSpeed( act->body->m_ctrl, 
				( float )TIME_BASE ) ;
		}else if((act->move_s == MoveAttackRun)
		||(act->move_s == MoveRun )){
			/*追っかけを遅く*/
			if( act->bodyp.type & ENE_TYPE_SHIELD){
				MT_SetMotionSpeed( act->body->m_ctrl, 
					( float )TIME_BASE*SHIELD_MOVE_RATE ) ;
			}else {
				MT_SetMotionSpeed( act->body->m_ctrl, 
					( float )TIME_BASE*ATTACKER_MOVE_RATE ) ;
			}
		}else {
			/*デフォルト*/
			MT_SetMotionSpeed( act->body->m_ctrl, 
				( float )TIME_BASE ) ;
		}
	}
	if(
	(act->move_s != MoveAttackRun)
	&&(act->move_s != MoveRun)
	&&(act->move_s != MoveWalkGun)
	){
		/*移動方向補正*/
		/*移動目標の方向*/
		act->mot_dir_correct = (int) act->tmp_dir ;
	}else {
		if(act->move_s == MoveAttackRun){
//			act->mot_speed_correct = 0.7F;
		}
	}

#if 0
	if((act->keep_mot == EM_tng_p90_nom_run)
	||(act->keep_mot ==EM_tng_tbl_nom_run)){
		FootCheck(act);
	}
#endif
}

#define	GRD_HIGH_THR_TIME	DIRECT_TICK(96)
void ActGrdThrHigh( act, time )
ACTION	*act ;
int		time ;
{
extern void *NewConnectObject( OBJECT *, int, FVECTOR *, SVECTOR *, int ) ;

	/*モーションの移動量が大きすぎるのでリセット*/
//	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;

	act->mot_speed_correct = 0.2F;

	act->ctrl->step = DG_ZeroVector ;

	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if( act->bodyp.type & ENE_TYPE_SHIELD){
		AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_GUN_FREE ) ;

		if(time == DIRECT_TICK(22)){
			if ( act->tmp_item == NULL){
			    GV_SetActorChild( act->w, 
//		    	( act->tmp_item = NewConnectObject( act->body, HUMAN21_MIGI_TE, &glass_sft, &glass_rot, MDL_GRENADE ))) ;
		    	( act->tmp_item = NewConnectObject( act->body, HUMAN21_MIGI_TE, &DG_ZeroVector, &DG_ZeroSVector, MDL_GRENADE ))) ;
			}
		}
		if(( time > DIRECT_TICK(22) ) &&( time < DIRECT_TICK(153) )){
			act->sw->sub_weapon = 0;
		}else {
			act->sw->sub_weapon = 1;
		}
	}else {
		if(( time > DIRECT_TICK(5) ) &&( time < DIRECT_TICK(149) )){
			AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_GUN_FREE ) ;
		}
		if(time == DIRECT_TICK(33)){
			if ( act->tmp_item == NULL){
			    GV_SetActorChild( act->w, 
//		    	( act->tmp_item = NewConnectObject( act->body, HUMAN21_MIGI_TE, &glass_sft, &glass_rot, MDL_GRENADE ))) ;
		    	( act->tmp_item = NewConnectObject( act->body, HUMAN21_MIGI_TE, &DG_ZeroVector, &DG_ZeroSVector, MDL_GRENADE ))) ;
			}
		}
	}
	if ( act->CheckDamage( act ) ) {
//		act->act_end = 1 ;
		if( act->bodyp.type & ENE_TYPE_SHIELD){
			act->sw->sub_weapon = 1;
		}
		if ( act->tmp_item != NULL){
			GV_DestroyOtherActor( act->tmp_item ) ;
			act->tmp_item = NULL ;
		}
		return ;
	}
	if ( act->dir >= 0 ) {
//		act->ctrl->turn.vy = act->dir ;
	}else	{
		//printf("DIR -1 TO STILL\n");
		//printf("AT_ACT LINE %d\n",__LINE__);
		if( act->bodyp.type & ENE_TYPE_SHIELD){
			act->sw->sub_weapon = 1;
		}
		if ( act->tmp_item != NULL){
			GV_DestroyOtherActor( act->tmp_item ) ;
			act->tmp_item = NULL ;
		}
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),
			ENE_BODYTRGSIZE_STAND );
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;
//		MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE*1.5f ) ;
		Layer_Off(act,1);
	}

	if( act->bodyp.type & ENE_TYPE_SHIELD){
		if ( time == 60 ) {
//			GM_SeSetMode( SD_V_C04MAKI+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			SIG_SetSpeak( SD_V_C04MAKI, act ) ;

		printf("NOW TEST SE [%d]\n",SD_V_C04MAKI+(act->name_id->voice%4));

		}
	}else {
		if ( time == 12 ) {
//			GM_SeSetMode( SD_V_C04MAKI+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			SIG_SetSpeak( SD_V_C04MAKI, act ) ;

		printf("NOW TEST SE [%d]\n",SD_V_C04MAKI+(act->name_id->voice%4));
		}
	}
#if 1
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			if( act->bodyp.type & ENE_TYPE_SHIELD){
				act->sw->sub_weapon = 1;
			}
			if ( act->tmp_item != NULL){
				GV_DestroyOtherActor( act->tmp_item ) ;
				act->tmp_item = NULL ;
			}
			act->act_end = 1 ;
			return ;
		}
	}
#endif

//	if ( time == 198 ) {
//	if ( time == 80 ) {
	if(act->keep_mot == EM_gbs_shl_nom_gm_fire_high ){
		if ( time == DIRECT_TICK(99)) {
			if( act->bodyp.type & ENE_TYPE_EVENT_A){
				if(SIG_GrdCall( act ,&act->target_pos,3) == 1){
				}
			}else {
				HZX_ZON		*pl_zone; 
				int mode ;
				pl_zone = ENE_HZX_GetZone(GM_PlayerAddress);
				if(pl_zone->flag&(HZX_ZONE_NO_AVOID|HZX_ZONE_INTRUDE)){
					mode = 5 ;
				}else {
					mode = 3 ;
				}
				if(SIG_GrdCall( act ,&act->target_pos,mode) == 1){

					act->act_end = 2 ;
				}
			}
			if ( act->tmp_item != NULL){
				GV_DestroyOtherActor( act->tmp_item ) ;
				act->tmp_item = NULL ;
			}
		}
	}else {
		if ( time == GRD_HIGH_THR_TIME ) {
			if( act->bodyp.type & ENE_TYPE_EVENT_A){
				if(SIG_GrdCall( act ,&act->target_pos,3) == 1){
				}
			}else {
				HZX_ZON		*pl_zone; 
				int mode ;
				pl_zone = ENE_HZX_GetZone(GM_PlayerAddress);
				if(pl_zone->flag&(HZX_ZONE_NO_AVOID|HZX_ZONE_INTRUDE)){
					mode = 5 ;
				}else {
					mode = 3 ;
				}
				if(SIG_GrdCall( act ,&act->target_pos,mode) == 1){
					act->act_end = 2 ;
				}
			}
			if ( act->tmp_item != NULL){
				GV_DestroyOtherActor( act->tmp_item ) ;
				act->tmp_item = NULL ;
			}
		}
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		if( act->bodyp.type & ENE_TYPE_SHIELD){
			act->sw->sub_weapon = 1;
		}
		if ( act->tmp_item != NULL){
			GV_DestroyOtherActor( act->tmp_item ) ;
			act->tmp_item = NULL ;
		}
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
}
extern void SK_BlowBack_M4A1( void * , char ) ;

void Act_GrdM4Shoot( act, time )
ACTION	*act ;
int		time ;
{
	/*モーションの移動量が大きすぎるのでリセット*/
//	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
//	act->mot_speed_correct = 0.2F;
//	act->ctrl->step = DG_ZeroVector ;

	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	if ( act->CheckDamage( act ) ) {
		act->sw->chamber2 = CBR2_NORAML ;
//		act->act_end = 1 ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		act->sw->chamber2 = CBR2_NORAML ;
		return ;
	}
	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),
			ENE_BODYTRGSIZE_STAND );
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
		ACT_INTERP_M ) ;
//		MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE*1.5f ) ;
//		Layer_Off(act,1);
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->chamber2 = CBR2_NORAML ;
			act->act_end = 1 ;
			return ;
		}
	}
/**
グレネード俳莢時エフェクト
extern void AN_CartridgeM4_Hi_Grenade_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
グレネード発射時エフェクト
void AN_MazzleM4demo_hi(FMATRIX *world,int silence,int mode)
******/
	/*40-49 open */
	/* 135- 144 close***/
	if (( time == DIRECT_TICK(42) )){
		act->sw->chamber2 = CBR2_OPEN ;
	}
	if (( time == DIRECT_TICK(52) )){
		extern void AN_CartridgeM4_Hi_Grenade_E( FMATRIX * , OBJECT * , CONTROL * ) ;
		FMATRIX *hand ;
		hand = &BODYWORLD( act->body, HUMAN21_MIGI_TE ) ;
		AN_CartridgeM4_Hi_Grenade_E( hand , act->body, act->ctrl) ;
	}

	if ( time == DIRECT_TICK(140) ) {
		act->sw->chamber2 = CBR2_CLOSE ;
//		GM_SeSetMode( SD_V_C04MAKI+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			SIG_SetSpeak( SD_V_C04MAKI, act ) ;

	}

	if ( time == DIRECT_TICK(182) ) {
		/*発射*/
		extern void AN_MazzleM4demo_hi(FMATRIX *,int ,int ) ;
		FMATRIX *hand ;

		hand = &BODYWORLD( act->body, HUMAN21_MIGI_TE ) ;
		SIG_GrdCall( act ,&act->target_pos,4) ;
		GM_SeSetMode( SD_E_M4EGRE01 , &act->ctrl->mov, GM_SEMODE_BOMB ) ;

		AN_MazzleM4demo_hi(hand,0,0) ;
	}
#if 1
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
//		SK_BlowBack_M4A1( &(act->sw->wctrl2),0 );
		AT_SetMode( act, ENE_ActStandStill ) ;
		act->sw->chamber2 = CBR2_NORAML ;
		return ;
	}
#endif

}
void Act_GrdM4Reload( act, time )
ACTION	*act ;
int		time ;
{
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT&&(!(GM_PlayerStatus & PLAYER_STEALTH))){
		AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	}else {
		AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	}
	if ( act->CheckDamage( act ) ) {
//		act->act_end = 1 ;
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
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
//		ACT_INTERP_DEF ) ;1
		ACT_INTERP_M ) ;
		Layer_Off(act,1);
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->act_end = 1 ;
			return ;
		}
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
}


void AT_ActShotSPS( act, time )
ACTION	*act ;
int		time ;
{
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT&&(!(GM_PlayerStatus & PLAYER_STEALTH))){
		AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	}else {
		AT_SetActStatus( act, ACT_STATUS_STAND ) ;
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
	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),
		ENE_BODYTRGSIZE_STAND );
 		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,ACT_INTERP_M ) ;
		Layer_Off(act,1);
	}
/*ショットガン排莢エフェクト*/
	if ( time == DIRECT_TICK(21) )	act->sw->wctrl2.flag = WPEF_FLG_START ;
/*発射*/
	if ( time == DIRECT_TICK(68) ) {
		int flag;
		flag = (ENE_BULLET_NORMAL|ENE_BULLET_NOLINE|ENE_BULLET_NORANDAM);
		ENE_ShootBullet( act,flag ,NULL ) ;
		*act->sw->bullet = *act->sw->bullet+1 ;
	}

	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		act->keep_mot = EM_gbs_sps_nom_fire_p ;
		AT_SetMode( act, ENE_ActStandStill ) ;
//		act->keep_pad = SP_READYGUN ;
//		AT_SetMode( act, AT_ActReadyGun ) ;
	}
}

#if 1
#define	STAND_DAM_MAX	32
/*ダメージ以外で中断しない*/
/*無敵！！*/
void AT_ActNoCancel( act, time )
ACTION	*act ;
int		time ;
{

	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_HOMING_SKIP ) ;
	act->bodyp.stand = STAND_DAM_MAX+1 ;

	if ( act->CheckDamage( act ) ) {
printf("act->bodyp.stand = STAND_DAM_MAX \n");
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		//printf("DIR -1 TO STILL\n");
		//printf("AT_ACT LINE %d\n",__LINE__);
printf("ACT_CANSEL BY DIR -1 !!!!\n");

		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( time == 0 ) {
		if ( act->dir >= 0 ) {
			act->ctrl->turn.vy = act->dir ;
		}
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, 
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;

		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_SQUAT ) ;
		Layer_Off(act,1);
	}
#if 0
	if((act->keep_mot &= ~MT_REV_ALL) == EM_tng_p90_nom_somersault2){
		if ( time == 40 ) {
		pos = act->ctrl->mov ;
		pos.vy = act->ctrl->levels[0] ;
			NewLandingSmoke( &pos, 800.0f ) ;
		}
	}
#endif
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		if ( act->CheckPad( act ) ) {
//printf("ACT_CANSEL BY MOT END !!!!\n");
				return ;
		}
		AT_SetMode( act, ENE_ActStandStill ) ;
	}
}
#endif
void AT_ActAcrobat( act, time )
ACTION	*act ;
int		time ;
{

   // BP_WARNING - Note the use of || here, which is likely a mistake
   //              Not fixing this because it may change behavior
	AT_SetActStatus( act, ACT_STATUS_SHAGAMI |ACT_STATUS_TARGET_SKIP||ACT_STATUS_HOMING_SKIP) ;
//	AT_SetActStatus( act, ACT_STATUS_SHAGAMI ) ;
//	act->bodyp.type |= ENE_TYPE_STAND_DMG_ABS ;

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if( time > 1 && time < COUNT_VMODE(60) ) {
		FVECTOR force = { 0.0F,-200.0F,100.0F } ;
		FVECTOR size = { 500.0F, 500.0F,500.0F } ;
		ENE_SetOffenseTarget3( act,WP_PUNCH ,
			HUMAN21_MIGI_KAKATO,&size ,&force) ;
	}

	if ( act->dir >= 0 ) {
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, 
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;

		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_SQUAT ) ;
		Layer_Off(act,1);
	}

	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		if ( act->CheckPad( act ) ) {
				return ;
		}
		AT_SetMode( act, ENE_ActStandStill ) ;
	}
}

/*******
スタンド状態でリロード
*******/
void ActStandReload( act, time )
ACTION	*act ;
int		time ;
{
//	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT&&(!(GM_PlayerStatus & PLAYER_STEALTH))){
	if(0){
		AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	}else {
		AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	}
	if ( act->CheckDamage( act ) ) {
		if( act->bodyp.type & ENE_TYPE_SHOTGUN){
			act->sw->magg = 0 ;
		}else if(act->keep_mot != EM_shl_nom_mkr_reload){
			act->sw->magg = 1 ;
		}
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		if( act->bodyp.type & ENE_TYPE_SHOTGUN){
			act->sw->magg = 0 ;
		}else if(act->keep_mot != EM_shl_nom_mkr_reload){
			act->sw->magg = 1 ;
		}
		return ;
	}
	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		Layer_Off(act,1);
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			if( act->bodyp.type & ENE_TYPE_SHOTGUN){
				act->sw->magg = 0 ;
			}else if(act->keep_mot != EM_shl_nom_mkr_reload){
				act->sw->magg = 1 ;
			}
			return ;
		}
	}

//  case 0:	/* 非表示 */
//  case 1:	/* 銃に接続されている（銃は手に付いていると仮定） */
//  case 2:	/* 左手にもっている */
//  case 3:	/* 右手にもっている */
//  case 4:	/* その場落下開始 */
//  case 5:	/* デストロイ */
	switch(act->keep_mot){
		case EM_gbs_sps_nom_fire_reload :
			if ( time == DIRECT_TICK(40) ) act->sw->magg = 2 ;/*temoti*/
			if ( time == DIRECT_TICK(69) ) act->sw->magg = 1 ;/*juutuke*/
			break ;
		case EM_tng_p90_nom_fire_reload :
			if ( time == DIRECT_TICK(19) ) act->sw->magg = 2 ;
			if ( time == DIRECT_TICK(75) ) act->sw->magg = 1 ;
			break ;
		case EM_htc_m4_nom_fire_reload :
			if ( time == DIRECT_TICK(24) ) act->sw->magg = 4 ; /*sute*/
			if ( time == DIRECT_TICK(130) ) act->sw->magg = 2 ;	/*temoti*/
			if ( time == DIRECT_TICK(181) ) act->sw->magg = 1 ;	/*juutuke*/
			break ;
		default :
			/* AK */
			if ( time == DIRECT_TICK(19) ) act->sw->magg = 4 ; /*sute*/
			if ( time == DIRECT_TICK(66) ) act->sw->magg = 2 ;	/*temoti*/
			if ( time == DIRECT_TICK(112) ) act->sw->magg = 1 ;	/*juutuke*/
			break ;
	}

	if( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		if( act->bodyp.type & ENE_TYPE_SHOTGUN){
			act->sw->magg = 0 ;
		}else if(act->keep_mot != EM_shl_nom_mkr_reload){
			act->sw->magg = 1 ;
		}
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
}

void SIG_ActDiscovery( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
//		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, 
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;

		act->sw->eye_anim = 2 ;/* 見開き */
		ENE_SetHeadMark( act, BODY21_HEAD, HEADMARK_BR ) ;
		if ( !(GM_GameStatus & GM_STATUS_DETECT) ) {
			GM_SeSetMode( SD_E_BIKKRI01 , &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			COM_DetectVibration( ) ;
		}
	}
	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = 1 ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->eye_anim = 1 ;
			return ;
		}
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		act->sw->eye_anim = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

/*グレネード下段投げ*/
void ActGrdThrLow( act, time )
ACTION	*act ;
int		time ;
{
extern void *NewConnectObject( OBJECT *,int,FVECTOR *,SVECTOR *,int) ;
extern int SIG_GrdCall(ACTION *,FVECTOR *,int) ;

	if ( time == DIRECT_TICK(30) ) {
		if ( act->tmp_item == NULL){
		    GV_SetActorChild( act->w, 
		    	( act->tmp_item = NewConnectObject( act->body, HUMAN21_MIGI_TE, &DG_ZeroVector, &DG_ZeroSVector, MDL_GRENADE ))) ;
		}else {
		}
	}

	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_GUN_FREE ) ;
	if ( act->CheckDamage( act ) ) {
//		act->act_end = 1 ;
		if ( act->tmp_item != NULL){
			GV_DestroyOtherActor( act->tmp_item ) ;
			act->tmp_item = NULL ;
		}
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		if ( act->tmp_item != NULL){
			GV_DestroyOtherActor( act->tmp_item ) ;
			act->tmp_item = NULL ;
		}
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
//		ACT_INTERP_DEF ) ;
		ACT_INTERP_M ) ;

		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),
			ENE_BODYTRGSIZE_STAND );
//		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, 
//			MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		Layer_Off(act,1);
		/*「くらえ！」*/
//		GM_SeSetMode( SD_V_C04MAKI+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
			SIG_SetSpeak( SD_V_C04MAKI, act ) ;
	}

#if 1
/*ダメージ以外でのモーションキャンセル不可*/
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->act_end = 1 ;
//			act->sw->magg = 1 ;
			if ( act->tmp_item != NULL){
				GV_DestroyOtherActor( act->tmp_item ) ;
				act->tmp_item = NULL ;
			}
			return ;
		}
	}
#endif

   // Armature note: BP_IsPAL was being "called" without the ()
   // i.e., the function pointer was being compared, so it would never be == TRUE
   // That's why I changed this to just check the time
   // Steve
#if 0
	if ( ((BP_IsPAL!=TRUE) && time == 227) ||
        ((BP_IsPAL==TRUE) && time == 189) )
#else
   if (time == 227)
#endif
   {

		FVECTOR		force = { 0.0F ,0.0F,200.0F };
		SIG_GrdCall( act ,&force,0) ;
		if ( act->tmp_item != NULL){
			GV_DestroyOtherActor( act->tmp_item ) ;
			act->tmp_item = NULL ;
		}
	}
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
//		act->sw->magg = 1 ;
		act->act_end = 1 ;
		if ( act->tmp_item != NULL){
			GV_DestroyOtherActor( act->tmp_item ) ;
			act->tmp_item = NULL ;
		}
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
}


/*
	defthink.c
	銃撃戦モードの思考

	2000/02/14 K.Sigeno
	$Id: defthink.c,v 1.3 2002/11/23 12:42:26 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado.x"
#include	"enemy.h"
#include	"libutl.h"

#include	"attacker.h"

/*extern*/
extern void AT_ActReadyGun( ACTION	*, int  );
extern void AT_ActShootGun( ACTION	*, int  );
extern void AT_ActSquatReadyGun( ACTION	*, int  );
extern void AT_ActOneTimeTurn( ACTION	*, int  );
extern void AT_ActMoveRun( ACTION	*, int  );
extern void AT_ActOneTimeTurn_Squat( ACTION	*, int  );

extern void SetRevMotion(ACTION *);
extern void ResetRevMotion(ACTION *);

extern void SetMoveMode(ENETHINK *);
extern int	CheckDirSub(int ,int);

extern int GetHomingDirY(ENETHINK *  , float );

/*prot*/
#include "defthink.h"

/*アクションリスト*/

static u_char def_list[20] = {
	SP_STAND_STILL,
	SP_SQUATHIDE,

	SP_ROLL_R,
	SP_ROLL_L,

	SP_PEEK_HIGH_START_R,
	SP_PEEK_HIGH_END_R,
	SP_PEEK_SQUAT_START_R,
	SP_PEEK_SQUAT_END_R,

	SP_PEEK_HIGH_START_L,
	SP_PEEK_HIGH_END_L,
	SP_PEEK_SQUAT_START_L,
	SP_PEEK_SQUAT_END_L,

	/*攻撃アクションはここ以降に登録*/
	SP_PEEK_HIGH_IDLE_R,
	SP_PEEK_SQUAT_IDLE_R,
	SP_PEEK_HIGH_IDLE_L,
	SP_PEEK_SQUAT_IDLE_L,

	/*反転しない攻撃モーションはここ*/
	SP_READYGUN,
	SP_SQUATGUN,
	SP_BLIND_IDLE_R ,
	SP_BLIND_IDLE_SQUAT_R 
};
#define START_TIME	(30)
#define IDLE_TIME	(120)
#define END_TIME	(14)
#define ROLL_TIME	(85)
enum {
	STAND_STILL = 1,
	SQUAT_STILL,

	ROLL_R,
	ROLL_L,

	PEEK_HIGH_START_R,
	PEEK_HIGH_END_R,
	PEEK_SQUAT_START_R,
	PEEK_SQUAT_END_R,

	PEEK_HIGH_START_L,
	PEEK_HIGH_END_L,
	PEEK_SQUAT_START_L,
	PEEK_SQUAT_END_L,

	PEEK_HIGH_IDLE_R,
	PEEK_SQUAT_IDLE_R,
	PEEK_HIGH_IDLE_L,
	PEEK_SQUAT_IDLE_L,

	READYGUN,
	SQUATGUN,
	BLIND_HIGH_IDLE_R ,
	BLIND_SQUAT_IDLE_R 

} ;

static u_char def_act_R[36] = {
	STAND_STILL,60,
	PEEK_HIGH_START_R,START_TIME,
	PEEK_HIGH_IDLE_R,IDLE_TIME,
	PEEK_HIGH_END_R,END_TIME,
	STAND_STILL,30,
	SQUAT_STILL,60,
	PEEK_SQUAT_START_R,START_TIME,
	PEEK_SQUAT_IDLE_R,IDLE_TIME,
	PEEK_SQUAT_END_R,END_TIME,
	SQUAT_STILL,30,
	ROLL_R,ROLL_TIME,
	SQUATGUN,IDLE_TIME,
	SQUAT_STILL,END_TIME,
	BLIND_SQUAT_IDLE_R ,IDLE_TIME,
	STAND_STILL,60,
	BLIND_HIGH_IDLE_R ,IDLE_TIME,
	0,0
};

static u_char def_act_L[28] = {
	STAND_STILL,60,
	PEEK_HIGH_START_L,START_TIME,
	PEEK_HIGH_IDLE_L,IDLE_TIME,
	PEEK_HIGH_END_L,END_TIME,
	STAND_STILL,30,
	SQUAT_STILL,60,
	PEEK_SQUAT_START_L,START_TIME,
	PEEK_SQUAT_IDLE_L,IDLE_TIME,
	PEEK_SQUAT_END_L,END_TIME,
	SQUAT_STILL,30,
	ROLL_L,ROLL_TIME,
	SQUATGUN,IDLE_TIME,
	SQUAT_STILL,END_TIME,
	0,0
};
static u_char def_act_LOW_R[12] = {
	SQUAT_STILL,60,
	PEEK_SQUAT_START_R,START_TIME,
	PEEK_SQUAT_IDLE_R,IDLE_TIME,
	PEEK_SQUAT_END_R,END_TIME,
	SQUAT_STILL,30,
	0,0
};

static u_char *act_list[3] =
{
	def_act_R,
	def_act_L,
	def_act_LOW_R
};


#if 0
void ReadDefSeq(entk)
ENETHINK *entk;
{
	int i;
printf("READ DEF SEQ!!\n");
	/*守備時アクションスクリプト*/
	for(i=0;i<(AT_SEQ_MAX*2); i++){
		entk->def_seq[i] = 0 ;
	}

	if ( GCL_GetOption( 'a' ) != NULL ) {
		for(i=0; i<(AT_SEQ_MAX*2); i++){
			if( GCL_NextStr() != NULL ){
				entk->def_seq[i] = (u_char)GCL_GetNextInt() ;
			}else {
				break;
			}
		}
	}

//	for(i=0;i<(AT_SEQ_MAX*2);i++){
//		printf("ACT NUM %d\n",entk->def_seq[i]);
//	}

}
#endif

/*ふっとばし*/
static void Def_Set_ForceTarget( act )
ACTION	*act ;
{
	static FVECTOR force = { 0.0F,-00.0F,-1000.0F } ;
	static FVECTOR size = { 500.0F,500.0F,500.0F } ;
	TARGET	*off ;
	POWER_TARGET	*pow ;
//	FVECTOR			v ;

	off = &( act->offense ) ;
	pow = &( act->off_pow ) ;

	GM_SetTarget( off, TARGET_OFFENSE | TARGET_CHECK_ONE, 0, PLAYER_SIDE,
	&size, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( off, WP_PUNCH ) ; 

//	GM_MoveTarget3( off, &( BODYWORLD(act->body, HUMAN21_MIGI_TE) ) ) ;
	GM_MoveTarget3( off, &( BODYWORLD(GM_PlayerBody,HUMAN21_MUNE) ) ) ;
//	GM_MoveTarget3( off, GM_PlayerBody ) ;
//	DG_SetPos2( &act->ctrl->mov, &act->ctrl->rot ) ;
//	DG_SetPos2( &GM_PlayerPosition, &act->ctrl->rot ) ;
//	DG_RotVector( &force, &v, 1 ) ;

//	GM_SetPowerTarget( off, pow, POWER_ONCE, 255, 0, 0, &v ) ;
	GM_SetPowerTarget( off, pow, POWER_ONCE, 255, 0, 0, &force ) ;
	GM_PutTarget( off ) ;

}

static	int	DefCheckPad( act )
ACTION	*act ;
{
	switch ( act->pad ) {
		case SP_READYGUN :
			ResetRevMotion(act);
			act->keep_mot = EM_kamae_gun_high ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn);
			return 1 ;
		break ;
		case SP_SQUATGUN :
			ResetRevMotion(act);
			act->keep_mot = EM_squat_fire ;
			act->keep_pad = act->pad ;
//			AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			AT_SetMode( act, AT_ActOneTimeTurn);
			return 1 ;
		break ;
		case SP_MOVE_RUN :
			ResetRevMotion(act);
			act->keep_mot = EM_run_atk;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActMoveRun);
			return 1 ;
		break ;
		case SP_ROLL_R :
			ResetRevMotion(act);
			act->keep_mot = EM_squat_roll_r ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			return 1 ;
		break ;
		case SP_ROLL_L :
			ResetRevMotion(act);
			act->keep_mot = EM_squat_roll_l ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			return 1 ;
		break ;
/********************
	オルガ流用実験
********************/
		case SP_PEEK_HIGH_START_R :
			ResetRevMotion(act);
			act->keep_mot = EM_test_peek_r_start_slow ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn);
			return 1 ;
		break ;
		case SP_PEEK_HIGH_END_R :
			ResetRevMotion(act);
			act->keep_mot = EM_test_peek_r_end ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn);
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_START_R :
			ResetRevMotion(act);
			act->keep_mot = EM_test_peek_r_squat_start_slow ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_END_R :
			ResetRevMotion(act);
			act->keep_mot = EM_test_peek_r_squat_end ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			return 1 ;
		break ;
		case SP_PEEK_HIGH_IDLE_R : 
			ResetRevMotion(act);
			act->keep_mot = EM_test_peek_r_fire_p ;
//			act->keep_mot = EM_nom_fire_blind ;
			act->keep_pad = act->pad ;
/*手が寝てるのでアジャスト無し*/
			AT_SetMode( act, AT_ActOneTimeTurn);
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_IDLE_R : 
			ResetRevMotion(act);
			act->keep_mot = EM_test_peek_r_squat_p ;
//			act->keep_mot = EM_nom_fire_blind_squat ;
			act->keep_pad = act->pad ;
/*手が寝てるのでアジャスト無し*/
			AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			return 1 ;
		break ;

		case SP_BLIND_IDLE_R : 
			ResetRevMotion(act);
			act->keep_mot = EM_nom_fire_blind ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn);
			return 1 ;
		break ;

		case SP_BLIND_IDLE_SQUAT_R : 
			ResetRevMotion(act);
			act->keep_mot = EM_nom_fire_blind_squat ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			return 1 ;
		break ;

		case SP_PEEK_HIGH_START_L :
			SetRevMotion(act);
			act->keep_mot = EM_test_peek_r_start_slow ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn);
			return 1 ;
		break ;
		case SP_PEEK_HIGH_END_L :
			SetRevMotion(act);
			act->keep_mot = EM_test_peek_r_end ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn);
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_START_L :
			SetRevMotion(act);
			act->keep_mot = EM_test_peek_r_squat_start_slow ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_END_L :
			SetRevMotion(act);
			act->keep_mot = EM_test_peek_r_squat_end ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			return 1 ;
		break ;
		case SP_PEEK_HIGH_IDLE_L : 
			SetRevMotion(act);
			act->keep_mot = EM_test_peek_r_fire_p ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn);
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_IDLE_L : 
			SetRevMotion(act);
			act->keep_mot = EM_test_peek_r_squat_p ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			return 1 ;
		break ;
		case SP_STAND_STILL :
			ResetRevMotion(act);
			act->keep_mot = EM_stand ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn);
			return 1 ;
		break ;
		case SP_SQUATHIDE :
			ResetRevMotion(act);
			act->keep_mot = EM_squat_hide_idle ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			return 1 ;
		break ;
printf("PAD ERROR %d\n",act->pad);
	}
	return 0 ;
}


/**  火器管制システム **/
static void Def_Shot( entk ,mode )
ENETHINK	*entk ;
int mode ;
{
	if(
		(entk->count3%4 ==0)
	){
		if(
		(entk->def_seq==1)
		||(mode ==0)
		){
			ENE_Shoot( &(BODYWORLD( entk->act->body,
			 HUMAN21_MIGI_TE )), entk->act ) ;
		}else {
			ENE_Shoot( &(BODYWORLD( entk->act->body,
			 HUMAN21_HIDARI_TE )), entk->act ) ;
		}
	}
}

static void Def_AimPosSet(ENETHINK *entk ,float sub)
{
	FVECTOR		shotpos;

	entk->status2 = ENE_STATUS2_AIM_GUN ;
//	shotpos = GM_PlayerPosition;
//	shotpos = GM_PlayerFindPos ;
	GV_MatToVec( &( BODYWORLD(GM_PlayerBody,HUMAN21_ATAMA) ),&shotpos);

//inline	void	GV_MatToVec( m, v )
//FMATRIX	*m ;
//FVECTOR	*v ;

//	shotpos.vx += sub;

//	if(entk->count3 <  )
//	shotpos.vx -= 2000.0F - (float) entk->count3 *(5.0F);
//	shotpos.vy -= 2000.0F - (float) entk->count3 *(5.0F);
//	shotpos.vx -= (float) entk->seq_time * (8.0F);

	entk->act->aim_pos = shotpos;
}

static void Think3_ZoneChaseTrgp( entk )
ENETHINK	*entk ;
{
	int reach;
	//,rank_dis = 1000;
if(entk->count3 %30==0){
//	NewZoneViewer( entk->trgpoint.addr,30,0);
}

#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			/*めくら時の処理*/
		}
	}
#endif

	entk->act->pad = SP_MOVE_RUN ;

	entk->at_status &= (~AT_ST_SQUAT);

	/*現在地アドレス更新*/
	if(!(entk->count3%30)) {
		entk->znavi->this_addr = 
		HZX_GetAddress( entk->ctrl->hzx_id, 
		&( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;
	}

	reach = HZX_ReachTo( entk->znavi->this_addr, entk->trgpoint.addr );
	/*ゾーン追跡*/
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint),
		entk->ctrl, entk->count3 ) ) {
	/*ダイレクト追跡に移行*/
		entk->think2 = TH2_MOVE ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

	if(entk->count3%15==0){
		if(
			( entk->pl_eyei.dis > 2000)&&
			(
			(entk->at_status & AT_ST_DEFENSE_MOVE)
			||(reach <= 2)
			)
		){
if(0){
	printf("TRG DIR %d\n",entk->trgpoint.dir);
	printf("PLY DIR %d\n",entk->pl_eyei.dir);
}
			SetMoveMode(entk);
		}else {
			if((entk->act->move_s == MoveBack) 
			){
				SetMoveMode(entk);
			}else {
				entk->act->move_s = MoveAttackRun ;
			}
		}
	}
	if(entk->act->move_s == MoveAttackRun) {
		entk->act->dir = entk->trgpoint.dir ;
	} else {
		/*プレイヤ方向見ながら移動*/
		Def_AimPosSet(entk ,-500) ;
		entk->act->dir = entk->pl_eyei.dir ;
		if(
		(entk->count3 >30) 
		&&( entk->pl_eyei.dis < entk->sense.eye_s*2)
		){
			Def_Shot( entk , 0 );
		}
	}
	entk->count3 ++ ;
}

static	void	Think3_DirectChaseTrgp( entk )
ENETHINK	*entk ;
{
	int		reach;
//	,rank_dis = 1000 ;
if(entk->count3 %30==0){
//	NewZoneViewer( entk->trgpoint.addr,30,0);
}


#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			/*めくら時の処理*/
		}
	}
#endif

	entk->act->pad = SP_MOVE_RUN ;

	entk->at_status &= (~AT_ST_SQUAT);

	/*目標点の高さを修正*/
//	entk->trgpoint.pos.vy = entk->znavi->flore_pos.vy ;
	/*追跡のための方向設定と距離判定*/
	if(ENE_DirectTrace( &(entk->trgpoint),
//	 &(entk->znavi->flore_pos), 250 ) <0 ){
//	 &(entk->znavi->flore_pos), 50 ) <0 ){
	 &(entk->znavi->flore_pos), 150 ) <0 ){
		/*実験のため位置の誤差をカット*/
		entk->ctrl->mov.vx = entk->def_pos.vx;
		entk->ctrl->mov.vz = entk->def_pos.vz;
		/*到達*/
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		entk->act->dir = entk->ctrl->turn.vy ;
		return ;
	}

	/*距離外*/
	if( entk->count3 ==0){
		/*現在値 計算し直し*/
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), -1 ) ;
	}
	/* いつまでも直線じゃいられない */
	entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;

	reach = HZX_ReachTo( entk->znavi->this_addr, entk->trgpoint.addr );

	if( reach > HZX_INDIRECT_REACH ) {
	/*ゾーン到達してない*/
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

	if(entk->count3%15==0){
//printf("H_DIS %d\n",entk->trgpoint.h_dis );
		if(
//			(entk->count3 < 60)
//			||( entk->trgpoint.h_dis < 4000 )
			(1)
		){
			SetMoveMode(entk);
		}else {
			if((entk->act->move_s == MoveBack) 
			){
				SetMoveMode(entk);
			}else {
				entk->act->move_s = MoveAttackRun ;
			}
		}
	}
	if(entk->act->move_s == MoveAttackRun) {
		entk->act->dir = entk->trgpoint.dir ;
	} else {
		Def_AimPosSet(entk ,-500) ;
		entk->act->dir = entk->pl_eyei.dir ;
		if(
		(1)
		&&( entk->pl_eyei.dis < entk->sense.eye_s*2)
		){
//			Def_Shot( entk,0 );
		}
	}
	entk->count3 ++ ;
}

/*ATTACK*/
static void Think3_AttackStand( entk )
ENETHINK	*entk ;
{
//	int reach;

	Def_AimPosSet(entk ,-500) ;

	/*ふっとばし攻撃*/
	/*発生条件は要検討*/
	if( entk->pl_eyei.dis < entk->sense.eye_s)
		Def_Set_ForceTarget( entk->act );

//    NewSpark( &mat ) ;
//    NewSpark( &(BODYWORLD( entk->act->body, HUMAN21_MIGI_TE ) ));
	if(entk->count3 ==0){


#if 1
		entk->seq_time = act_list[entk->def_seq][entk->seq_index*2+1];
//		act_list[3]
#else
		if(entk->def_seq[0]){
			entk->seq_time = def_act_R[entk->seq_index*2+1];
		}else {
			entk->seq_time = def_act_L[entk->seq_index*2+1];
		}
#endif
		if(entk->seq_index == 0) {
			entk->ctrl->mov.vx = entk->def_pos.vx;
			entk->ctrl->mov.vz = entk->def_pos.vz;
		}
	}
#if 1
//	entk->act->pad = def_list[(entk->def_seq[entk->seq_index*2])-1];
//	entk->act->pad = def_list[(def_act_R[entk->seq_index*2])-1];
	entk->act->pad = 
		def_list[(act_list[entk->def_seq][entk->seq_index*2])-1];
//	entk->act->pad = act_list[entk->def_seq][entk->seq_index*2+1];
#else
	if(entk->def_seq[0]){
		entk->act->pad = def_list[(def_act_R[entk->seq_index*2])-1];
	}else {
		entk->act->pad = def_list[(def_act_L[entk->seq_index*2])-1];
	}
//act_list[entk->def_seq][entk->seq_index*2]
#endif

#if 0
	if(
	((entk->def_seq[0])&&(def_act_R[entk->seq_index*2] >= PEEK_HIGH_IDLE_R))
	||
	((!entk->def_seq[0])&&(def_act_L[entk->seq_index*2] >= PEEK_HIGH_IDLE_R))	){
#else
	if(
	act_list[entk->def_seq][entk->seq_index*2] >= PEEK_HIGH_IDLE_R
	){
#endif
		if(
		(entk->seq_time < (IDLE_TIME-30) )&&(entk->seq_time%6 ==0)
		){
#if 1
{
	int flag ;
	
	flag = ( entk->def_seq == 0 ) ? 0 : ENE_BULLET_LEFT ;
	flag |= ENE_BULLET_NOATTACK ;
	ENE_ShootBullet( entk->act, flag ) ;
}
#else
			if(
			(entk->def_seq == 0 )
			)
			{
				ENE_Shoot( &(BODYWORLD( entk->act->body,
				 HUMAN21_MIGI_TE )), entk->act ) ;
			}else {
/*リバース撃ち*/
				ENE_Shoot( &(BODYWORLD( entk->act->body,
				 HUMAN21_HIDARI_TE )), entk->act ) ;
			}
#endif
			/*手ゆれ実験*/
//			entk->act->aim_pos.vy += 1000.0F;
		}else {
			/*撃ってない*/
//			entk->act->aim_pos.vy += -1000.0F;
		}
	}
	/*基本攻撃姿勢*/
	entk->at_status &= ~AT_ST_SQUAT;

	entk->bullet = 0 ;
/*射撃方向補正実験*/
#if 0
	if(
		(def_act_R[entk->seq_index*2] >= BLIND_HIGH_IDLE_R)
	){
		entk->act->dir = GetHomingDirY(entk , -300.0F) ;
//		entk->act->dir = GetHomingDirY(entk , 2000.0F) ;
//printf("PL DIR %d\n",entk->pl_eyei.dir);
//printf("COR DIR %d\n",entk->act->dir);

	}else {
		entk->act->dir = entk->pl_eyei.dir ;
	}
#else 
	entk->act->dir = entk->pl_eyei.dir  ;
#endif
	entk->count3 ++ ;
	entk->seq_time -- ;
	if(entk->seq_time==0){
		entk->count3 = 0 ;
	/* 目標ゾーンから離れていれば戻る*/
		if(
		(entk->act->pad == SP_STAND_STILL)
		||(entk->act->pad == SP_SQUATHIDE )
		){
			if(
			( entk->ctrl->addr != entk->trgpoint.addr )
			||(abs(entk->ctrl->mov.vx - entk->def_pos.vx) > 1000)
			||(abs(entk->ctrl->mov.vz - entk->def_pos.vz) > 1000)
			){
				/**/
				entk->at_status |= AT_ST_DEFENSE_MOVE ;
				entk->think1 = ENE_TH1_DEFENSE ; 
				entk->think2 = TH2_MOVE ; 
				entk->think3 = TH3_ZONE_CHASE ; 
				return ; 
			}
		}
		entk->seq_index++;
#if 1
	if(act_list[entk->def_seq][entk->seq_index*2] == 0 )
	{
		entk->seq_index = 0;
	}
#else
		if(entk->def_seq == 1){
			if(def_act_R[entk->seq_index*2]==0) {
				entk->seq_index = 0;
			}
		}else {
			if(def_act_L[entk->seq_index*2]==0) {
				entk->seq_index = 0;
			}
		}
#endif
	}
}

static void Think2_Move( entk ) 
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
		case TH3_ZONE_CHASE :
			Think3_ZoneChaseTrgp( entk ) ;
			break;
		case TH3_DIRECT_CHASE :
			Think3_DirectChaseTrgp( entk ) ;
			break;
	}
}

static void Think2_Attack( entk ) 
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
		case TH3_ATTACK_SETUP :
			Think3_AttackStand( entk ) ;
			break;
	}
}
static void Think2_Wait( entk ) 
ENETHINK	*entk ;
{
	entk->act->pad  = SP_STAND_STILL ;
	entk->count3 = 0;
}

void	Think1_Defense( entk )
ENETHINK	*entk ;
{
	/*守備中は警戒レベルに影響しない*/
	entk->alert = 0;
	if(entk->at_com->def_mode == AT_COM_DEF_WAIT)
	{
		Think2_Wait( entk ) ;
	}
	if(entk->at_com->def_mode == AT_COM_DEF_ACTIVE){
		switch ( entk->think2 ) {
		    case TH2_MOVE :		/* 追跡 */
				Think2_Move( entk ) ;
			break ;
		    case TH2_ATTACK :		/* 攻撃 */
				Think2_Attack( entk ) ;
			break ;
		}
	}

#if 0
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->alert = MAX_ALERT_LEVEL ;
	}
#endif

	/*前フレームでの目標位置待避*/
	entk->old_trgpos = entk->trgpoint.pos;
}

/*守備開始判定*/
void	DefenseModeStartCheck( entk )
ENETHINK	*entk ;
{
	if(
	(entk->think1 == ENE_TH1_DAMAGE)
	||(entk->think1 == ENE_TH1_RESURRECT)
	){
		/******
			通常処理に回す
		******/
		ResetRevMotion(entk->act);
	}else{
		/*守備開始処理*/
		if(entk->think1 != ENE_TH1_DEFENSE) {
			Sig_DefenseModeStart(entk);
		}
	}
}

/*ダメージモードへのみ移行する*/
void	DefenseModeCheck( entk )
ENETHINK	*entk ;
{
	if ( ENE_DamageCehack( entk ) ) {
//printf("RESET 408\n");
		ResetRevMotion(entk->act);
		ENE_EnemyStartModeDamage( entk ) ;
	}
}
/*守備モード終了時*/
void	DefenseModeEndCheck( entk )
ENETHINK	*entk ;
{
	ResetRevMotion(entk->act);
//	entk->act->sw->sub_weapon = 0;
	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
			ENE_AttackerStartModeSneak( entk ) ;
		break ;
		case ALERT_MODE_ALERT :
			AlertModeStart(entk);
		break ;
		case ALERT_MODE_SEARCH :
			ENE_AttackerStartModeSneak( entk ) ;
		break ;
	}

	if ( ENE_DamageCehack( entk ) ) {
		ENE_EnemyStartModeDamage( entk ) ;
	}
}

/*DEFmode中のダメージ後の復帰処理*/
void	DefenseDamageModeCheck( entk )
ENETHINK	*entk ;
{
	/*死んでないので思考セット後 再始動*/
	if ( entk->notice & ENE_NOTICE_DAMAGE ) {
//	ENE_AttackerStartModeSneak( entk ) ;
		Sig_DefenseModeStart( entk ) ;
	}
	/*再発生処理*/
	if ( entk->notice & ENE_NOTICE_RES ) {
		/*死んでた*/
		/*CheckPad設定へ*/
//		ENE_AttackerResurrectionMode( entk ) ;
	}
}

void Sig_DefenseModeStart( ENETHINK *entk)
{

	printf("DEF START\n");

	/*装備をマカロフに変更*/
//	entk->act->sw->sub_weapon = 1;

	/*守備位置を目標にセット*/
	ENE_SetTrgpDefense( &(entk->trgpoint), 
		&entk->def_pos, entk->def_mapbit ) ;

	AT_SetMode( entk->act, ENE_ActStandStill ) ;
	entk->act->CheckPad = DefCheckPad ;
	entk->think1 = ENE_TH1_DEFENSE ; 
	entk->think2 = TH2_MOVE ; 
	entk->think3 = TH3_ZONE_CHASE ; 
	entk->count3 = 0 ;

	entk->act->move_s = MoveAttackRun ;

	entk->act->pad = 0 ;
	entk->safeaddr = entk->at_com->usezones[entk->u_id][entk->id] 
	= HZX_NO_ZONE;
	entk->safetype = 0;
	entk->sight_time =0; 
	entk->at_status |= AT_ST_DEFENSE; /*守備モード*/
	entk->seq_index = 0;	/** 守備シーケンス用 **/
	entk->seq_time = 0;	/** 守備シーケンス用 **/
	entk->at_tmptime = 0;
}



//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	defthink.c
	銃撃戦モードの思考

	2000/02/14 K.Sigeno
	$Id: defthink.c,v 1.1.1.3 2002/11/19 11:49:09 Yoshizawa1 Exp $
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
#include	"libutl.h"

#include	"../attacker/at_thk.h"
#include	"eve_a.h"
#include	"defender.h"

#include	"../attacker/sig_conv.x"
#include	"../conv/conv_act.x"

#include	"camera.h"

/*extern*/
//extern void AT_ActReadyGun( ACTION	*, int  );
//extern void AT_ActShootGun( ACTION	*, int  );
//extern void AT_ActSquatReadyGun( ACTION	*, int  );

extern void AT_ActOneTimeTurn( ACTION	*, int  );
extern void AT_ActOneTimeTurn_Squat( ACTION	*, int  );
extern void AT_ActMoveRun( ACTION	*, int  );
extern void ActGrdThrHigh( ACTION	*, int  );
extern void ENE_ActNSight_ON( ACTION	*, int  );
extern void AlertModeStart(ENETHINK *);


extern void AT_ActOneTimeNoDmg( ACTION	*, int  );
extern float CheckGunPlOnline(ENETHINK *,FVECTOR * );

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern void SigZoneView( int , SVECTOR * ,float ) ; 
#endif


void CheckDefPos( ENETHINK *) ;
void CheckDefPos2( ENETHINK *) ;
void GetDefPos2( ENETHINK *) ;
void GetDefPos3( ENETHINK * ) ;


#define PL_NO_SHOOT (PLAYER_DAMAGED|PLAYER_DOWNED|PLAYER_FORCE)
#define DEF_LEVEL_TIME	(AT_THK_RATE*300) 

#define EVE_A_SHOT_DELAY	DIRECT_TICK(12)		/*飛び出してから撃つまで*/
//#define EVE_A_SHOT_TIME		60
#define EVE_A_SHOT_TIME		DIRECT_TICK(48)		/*射撃時間*/
#define EVE_A_WAIT_TIME		DIRECT_TICK(90)		/*射撃終了後の隙*/
#define DEF_MOVE_SHOT_DELAY	DIRECT_TICK(60)		/*移動時の射撃開始*/
#define DEF_WAIT_TIME		DIRECT_TICK(60)		/*安地待避時間*/
#define DEF_ABS_HIT_TIME	DIRECT_TICK(6)		/*かならずあたる時間*/
#define DEF_GRD_THR_LIMIT	(10000.0F)

//#define NO_SHOT_LIMIT		(200.0F)
#define NO_SHOT_LIMIT		(100.0F)

#define  DEF_POS_CHECK

#define SIG_AT_SetMode(x,y) AT_SetMode( x,(ACTIONMODE) y ) 

void DefPosFree(ENETHINK *) ;


/*現在守備位置からプレイヤまでのゾーン距離*/
static int CheckDefShotDis(ENETHINK *entk){
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;
	AT_THK *at_thk;
	int zone_dis ;

	at_thk = (AT_THK *) entk->character;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;

	/*射撃判定 ザコサバでは遠くても発砲*/
	if(eve_a->another_flag & ANOTHER_DARK_CAMP ){
		/*ザコサバ*/
		return 1 ;
	}
	/*右舷は無関係*/
	if(eve_a->type > 1){
		return 1 ;
	}
	zone_dis = def_com->def_zonedis[(int)eve_a->def_pos_num] ;
	if((zone_dis+entk->sense.eye_s+4000) > (def_com->pl_fi_dis ) ){
		/*撃てる*/
		return 1 ;
	}
	return 0 ;
}


void DEF_ActOneTime( act, time )
ACTION	*act ;
int		time ;
{


#if 0
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
#else
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
#endif
	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
	else	SIG_AT_SetMode( act, ENE_ActStandStill ) ;

	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}


	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
//		act->act_end = 1 ;
//		SIG_AT_SetMode( act,(ACTIONMODE) ENE_ActStandStill ) ;
		SIG_AT_SetMode( act,(ACTIONMODE) DEF_ActOneTime ) ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
}
#if 0
static void DEF_ActEndCheck( act, time )
ACTION	*act ;
int		time ;
{


#if 0
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
#else
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
#endif
	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
	else	SIG_AT_SetMode( act,(ACTIONMODE) ENE_ActStandStill ) ;

	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}


	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		SIG_AT_SetMode( act,(ACTIONMODE) ENE_ActStandStill ) ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
}
#endif
static void DEF_ActEndCheck_Muteki( act, time )
ACTION	*act ;
int		time ;
{



	if((GM_GameStatus & STATE_VR_ANOTHER)||(GM_GameStatus & STATE_BOSS_SURVIVAL)){
		AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	}else {
		AT_SetActStatus( act, ACT_STATUS_STAND| ACT_STATUS_TARGET_SKIP ) ;
	}


	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
	else	SIG_AT_SetMode( act,(ACTIONMODE) ENE_ActStandStill ) ;

	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}


	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		SIG_AT_SetMode( act,(ACTIONMODE) ENE_ActStandStill ) ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
}


static void ReverseCheck(entk)
ENETHINK *entk;
{
	if(
	(entk->act->body->m_ctrl->flag & MT_FLAG_REVERSAL1)
	&&
	(entk->act->body->m_ctrl->flag & MT_FLAG_REVERSAL2)
	){
		entk->sw_gun |= SW_FLAG_SWITCH1 ;
	}else {
		entk->sw_gun &= ~SW_FLAG_SWITCH1 ;
	}
}
extern void SetMoveMode(ENETHINK *);

extern int GetHomingDirY(ENETHINK *  , float );

/*prot*/
//#include "defthink.h"

/*アクションリスト*/

/**
SPリストの先頭にはno-actionを置いておく事
**/
enum {
	SP_NONE,
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
	SP_APPROCH,
	SP_BACKWALK,
	SP_MOVE_RUN_R,
	SP_MOVE_RUN_L,
	SP_MOVE_RUN,
	SP_GRD_HIGH, 
	SP_NSIGHT_ON,
	/*射撃アクションはここ以降に登録*/
	SHOT_ACT,/*if(pad > SHOT_ACT) で射撃*/
	SP_PEEK_HIGH_IDLE_R, 
	SP_PEEK_SQUAT_IDLE_R,
	SP_PEEK_HIGH_IDLE_L,
	SP_PEEK_SQUAT_IDLE_L,
	/*反転しない攻撃モーションはここ*/
	SP_READYGUN,
	SP_SQUATGUN,
	SP_SHOT_POS,
	SP_BLIND_IDLE_R ,
	SP_BLIND_IDLE_SQUAT_R ,
	SP_DATA_END
};

/*マジックナンバー 要注意*/
//#define WAIT_END	(0)
//#define IDLE_TIME	(1)
enum {
	WAIT_END = 0 ,
	IDLE_TIME ,
	MAGIC_END
};

#define BP_DUMMY_TIME (5*3)

#define START_TIME	(WAIT_END)
#define END_TIME	(WAIT_END)
#define ROLL_TIME	(WAIT_END)
//#define STILL_TIME	(AT_THK_RATE*5)
#define STILL_TIME	(AT_THK_RATE*1)
//#define SLIDE_TIME	(AT_THK_RATE*5)
#define SLIDE_TIME	(AT_THK_RATE*6)
#define PEEK_SLIDE_TIME	(AT_THK_RATE*3)


/*立ち右覗き*/
static u_char StandPeekR[16] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
   SP_MOVE_RUN_R,BP_DUMMY_TIME,
	SP_PEEK_HIGH_START_R,START_TIME,
	SP_PEEK_HIGH_IDLE_R,IDLE_TIME,
	SP_PEEK_HIGH_END_R,END_TIME,
	SP_MOVE_RUN_L,BP_DUMMY_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0
};


/*立ち左覗き*/
#if 0
static u_char StandPeekL[16] = {
	SP_STAND_STILL,STILL_TIME,
	SP_MOVE_RUN_L,PEEK_SLIDE_TIME,
	SP_PEEK_HIGH_START_L,START_TIME,
	SP_PEEK_HIGH_IDLE_L,IDLE_TIME,
	SP_PEEK_HIGH_END_L,END_TIME,
	SP_MOVE_RUN_R,PEEK_SLIDE_TIME,
	SP_STAND_STILL,STILL_TIME,
	SP_DATA_END,0
};
#endif
/*立ち右めくら*/
static u_char StandBlindR[8] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_BLIND_IDLE_R,IDLE_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0
};




/*しゃがみ右めくら*/
static u_char SquatBlindR[8] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_BLIND_IDLE_SQUAT_R,IDLE_TIME,
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_DATA_END,0
};


/*しゃがみ右覗き*/
static u_char SquatPeekR[12] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_PEEK_SQUAT_START_R,BP_DUMMY_TIME,
	SP_PEEK_SQUAT_IDLE_R,IDLE_TIME,
	SP_PEEK_SQUAT_END_R,END_TIME,
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_DATA_END,0
};




/*しゃがみ左覗き*/
#if 0
static u_char SquatPeekL[16] = {
	SP_SQUATHIDE,STILL_TIME,
	SP_MOVE_RUN_L,PEEK_SLIDE_TIME,
	SP_PEEK_SQUAT_START_L,START_TIME,
	SP_PEEK_SQUAT_IDLE_L,IDLE_TIME,
	SP_PEEK_SQUAT_END_L,END_TIME,
	SP_MOVE_RUN_R,PEEK_SLIDE_TIME,
	SP_SQUATHIDE,STILL_TIME,
	SP_DATA_END,0
};
/*めくら立ち右*/
static u_char StandBlindR[8] = {
	SP_STAND_STILL,STILL_TIME,
	SP_BLIND_IDLE_R ,IDLE_TIME,
	SP_STAND_STILL,STILL_TIME,
	SP_DATA_END,0
};
/*めくらしゃがみ右*/
static u_char SquatBlindR[8] = {
	SP_STAND_STILL,STILL_TIME,
	SP_BLIND_IDLE_R ,IDLE_TIME,
	SP_STAND_STILL,STILL_TIME,
	SP_DATA_END,0
};
#endif
/*転がり右*/
static u_char RollOutR[8] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_ROLL_R,ROLL_TIME,
	SP_SQUATGUN,IDLE_TIME,
	SP_DATA_END,0
};





/*転がり左*/
static u_char RollOutL[8] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_ROLL_L,ROLL_TIME,
	SP_SQUATGUN,IDLE_TIME,
	SP_DATA_END,0
};



/*グレネード投げ右*/
static u_char GrdThrowR[16] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_MOVE_RUN_R,BP_DUMMY_TIME,
	SP_GRD_HIGH,WAIT_END,
	SP_BACKWALK,BP_DUMMY_TIME,
	SP_MOVE_RUN_L,BP_DUMMY_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0,
	0,0
};


/*グレネード投げ左*/
static u_char GrdThrowL[16] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_MOVE_RUN_L,BP_DUMMY_TIME,
	SP_GRD_HIGH,WAIT_END,
	SP_BACKWALK,BP_DUMMY_TIME,
	SP_MOVE_RUN_R,BP_DUMMY_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0,
	0,0
};




/*グレネード投げ立ち*/
static u_char GrdThrowStand[8] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_GRD_HIGH,WAIT_END,
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_DATA_END,0
};



/*正面立ち撃ち*/
static u_char StandShot[8] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_READYGUN,IDLE_TIME,
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_DATA_END,0
};



static u_char StandShot2[8] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_READYGUN,IDLE_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0
};


/*スライド射撃右立ち*/
static u_char SlideShotR[12] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_MOVE_RUN_R,BP_DUMMY_TIME,
	SP_READYGUN,IDLE_TIME,
	SP_MOVE_RUN_L,BP_DUMMY_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0
};



/*スライド射撃右座り*/
static u_char SlideShotSquatR[12] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_MOVE_RUN_R,BP_DUMMY_TIME,
	SP_SQUATGUN,IDLE_TIME,
	SP_MOVE_RUN_L,BP_DUMMY_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0
};



/*スライド射撃左立ち*/
static u_char SlideShotL[12] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_MOVE_RUN_L,BP_DUMMY_TIME,
	SP_READYGUN,IDLE_TIME,
	SP_MOVE_RUN_R,BP_DUMMY_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0
};


/*スライド射撃左座り*/
static u_char SlideShotSquatL[12] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_MOVE_RUN_L,BP_DUMMY_TIME,
	SP_SQUATGUN,IDLE_TIME,
	SP_MOVE_RUN_R,BP_DUMMY_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0
};


/*待機立ち*/
#if 0
static u_char StandWait[4] = {
	SP_STAND_STILL,STILL_TIME,
	SP_DATA_END,0
};
#endif
/*待機しゃがみ*/
static u_char SquatWait[4] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_DATA_END,0
};




/*オブジェ破壊*/
static u_char ShotSubTrg[8] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_READYGUN,IDLE_TIME,
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_DATA_END,0
};




/*act_listの内容に準拠*/
enum {
	STAND_R,//0
	SQUAT_R,//1
	STAND_L,//2
	SQUAT_L,//3
	STAND_BACK1,//4
	SQUAT_WAIT,//5
	STAND_R_NARROW,//6
	STAND_L_NARROW,//7
	STAND_CENTER,//8
	ACT_LIST_END //9
};

enum {
	DEF_LEVEL_1 = 0,
	DEF_LEVEL_2,
	DEF_LEVEL_3,
	DEF_LEVEL_4,
	DEF_LEVEL_GRD,
	DEF_LEVEL_SUBTRG,
	DEF_LEVEL_END
} ;
//	DEF_LEVEL_MAX
//SlideShotL
static u_char *act_list[ACT_LIST_END][DEF_LEVEL_END] =
{
	{StandPeekR,SlideShotSquatR,RollOutR,StandBlindR,GrdThrowR,ShotSubTrg},		/*立ち*/
	{StandShot,StandShot,SquatPeekR,SquatBlindR,GrdThrowStand,ShotSubTrg},	/*しゃがみオンリー*/
	{SlideShotL,SlideShotSquatL,RollOutL,SlideShotL,GrdThrowL,ShotSubTrg},
	{SlideShotL,StandShot,RollOutL,SlideShotSquatL,GrdThrowStand,ShotSubTrg},	/*しゃがみオンリー*/
	{SquatWait,SquatWait,SquatWait,SquatWait,SquatWait,ShotSubTrg},		/*背後 */
	{SquatWait,SquatWait,SquatWait,SquatWait,SquatWait,ShotSubTrg},		/*絶対待機 */
	{StandPeekR,SlideShotSquatR,SlideShotSquatR,SlideShotR,GrdThrowR,ShotSubTrg}, /*転がらない*/
	{SlideShotL,SlideShotSquatL,SlideShotL,SlideShotL,GrdThrowL,ShotSubTrg},
	{StandShot2,StandShot,StandShot2,StandShot,GrdThrowStand,ShotSubTrg}	/*しゃがみオンリー*/
};


void BP_Update_defthink_Tables()
{
   StandPeekR[1]= STILL_TIME;
   StandPeekR[3]= PEEK_SLIDE_TIME;
   StandPeekR[11]= PEEK_SLIDE_TIME;
   StandPeekR[13]= STILL_TIME;

   StandBlindR[1]=STILL_TIME;
   StandBlindR[5]=STILL_TIME;

   SquatBlindR[1]=STILL_TIME;
   SquatBlindR[5]=STILL_TIME;

   SquatPeekR[1]=STILL_TIME;
   SquatPeekR[3]=START_TIME;
   SquatPeekR[9]=STILL_TIME;

   RollOutR[1]=STILL_TIME;

   RollOutL[1]=STILL_TIME;

   GrdThrowR[1]=STILL_TIME;
   GrdThrowR[3]=SLIDE_TIME;
   GrdThrowR[7]=SLIDE_TIME;
   GrdThrowR[9]=SLIDE_TIME;
   GrdThrowR[11]=STILL_TIME;

   GrdThrowL[1]=STILL_TIME;
   GrdThrowL[3]=SLIDE_TIME;
   GrdThrowL[7]=SLIDE_TIME;
   GrdThrowL[9]=SLIDE_TIME;
   GrdThrowL[11]=STILL_TIME;

   GrdThrowStand[1]=STILL_TIME;
   GrdThrowStand[5]=STILL_TIME;

   StandShot[1]=STILL_TIME;
   StandShot[5]=STILL_TIME;

   StandShot2[1]=STILL_TIME;
   StandShot2[5]=STILL_TIME;

   SlideShotR[1]=STILL_TIME;
   SlideShotR[3]=SLIDE_TIME;
   SlideShotR[7]=SLIDE_TIME;
   SlideShotR[9]=STILL_TIME;

   SlideShotSquatR[1]=STILL_TIME;
   SlideShotSquatR[3]=SLIDE_TIME;
   SlideShotSquatR[7]=SLIDE_TIME;
   SlideShotSquatR[9]=STILL_TIME;

   SlideShotL[1]=STILL_TIME;
   SlideShotL[3]=SLIDE_TIME;
   SlideShotL[7]=SLIDE_TIME;
   SlideShotL[9]=STILL_TIME;

   SlideShotSquatL[1]=STILL_TIME;
   SlideShotSquatL[3]=SLIDE_TIME;
   SlideShotSquatL[7]=SLIDE_TIME;
   SlideShotSquatL[9]=STILL_TIME;

   SquatWait[1]=STILL_TIME;

   ShotSubTrg[1]=STILL_TIME;
   ShotSubTrg[5]=STILL_TIME;
}


static int Def2AtCheck(ENETHINK *) ;

#if 0
static void DefLevelUp(entk)
ENETHINK *entk ;
{
	ACTION	*act;
	ENTK_TYPE_A *eve_a ;
//	DEF_COM		*def_com ;
	AT_THK *at_thk;

	act = entk->act ;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
//	def_com = (DEF_COM * ) eve_a->def_com ;
	at_thk = (AT_THK *) entk->character ;

	at_thk->at_tmptime = 0; /*着弾予測カウンタ*/
	entk->count3 = 0;
	eve_a->seq_index = 0;

	eve_a->seq_time = 
	act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2+1];
	eve_a->seq_count = 0 ;
}
#endif

#define CORRECT_SPEED	(1.0F)
void DEF_ActCorrectPos( act, time )
ACTION	*act ;
int		time ;
{

	if((GM_GameStatus & STATE_VR_ANOTHER)||(GM_GameStatus & STATE_BOSS_SURVIVAL)){
		AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	}else {
		AT_SetActStatus( act, ACT_STATUS_STAND| ACT_STATUS_TARGET_SKIP ) ;
	}
	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), 
		ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
		ACT_INTERP_DEF ) ;
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;

	if ( act->tmp_dir >= 0 )
	{
		act->ctrl->step.vx +=
			CORRECT_SPEED * _RsinF( (int)act->tmp_dir ) ;
		act->ctrl->step.vz += 
			CORRECT_SPEED * _RcosF( (int)act->tmp_dir ) ;
	}
}



/*ふっとばし*/
#if 0
static void Def_Set_ForceTarget( act ,force )
ACTION	*act ;
FVECTOR *force ;
{
//	static FVECTOR force = { 0.0F,0.0F,1000.0F } ;
	static FVECTOR size = { 500.0F,500.0F,500.0F } ;
	TARGET	*off ;
	POWER_TARGET	*pow ;
//	FVECTOR			v ;

	off = &( act->offense ) ;
	pow = &( act->off_pow ) ;

	/*プレイヤマップに合わせる*/

	GM_SetTarget( off, TARGET_OFFENSE | TARGET_CHECK_ONE, 0, PLAYER_SIDE,
	&size, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( off, WP_PUNCH|WP_NOPLAYER ) ; 

//	GM_MoveTarget3( off, &( BODYWORLD(GM_PlayerBody,HUMAN21_MUNE) ) ) ;
	GM_MoveTargetMap( off, &GM_PlayerFindPos, GM_PlayerMap );

	GM_SetPowerTarget( off, pow, POWER_ONCE, 255, 0, 0, force ) ;
	GM_PutTarget( off ) ;

}
#endif
/*プレイヤ方向へのforceベクトルで吹っ飛ばし攻撃*/
#if 0
static void Def_Set_ForceDir( ENETHINK *entk , float power)
{
	FVECTOR		vec1, vec2 ; /*入力 出力*/
//	float		dummy ;

// = { 0.0F,0.0F,1000.0F } ;

	vec1.vx = _RsinF( (int) entk->pl_eyei.dir ) * 1.0F ;
	vec1.vy = 0.0F;
	vec1.vz = _RcosF( (int) entk->pl_eyei.dir ) * 1.0F ;

	GV_LenVec3F( &vec1, &vec2, 0.0F, power ) ;
	Def_Set_ForceTarget( entk->act ,&vec2);
}
#endif
//static void Def_Set_ForceTarget( act ,force )
ACTION	*act ;
FVECTOR *force ;

static	int	DefCheckPad( act )
ACTION	*act ;
{
	switch ( act->pad ) {
		case SP_READYGUN :
//			ENE_ResetRevMotion(act);
			act->keep_mot = EM_kamae_gun_high ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, DEF_ActOneTime  );
			return 1 ;
		break ;
		case SP_SQUATGUN :
//			ENE_ResetRevMotion(act);
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_m4_nom_squat_fire ;
			}else {
				act->keep_mot = EM_squat_fire ;
			}
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, DEF_ActOneTime  );
			return 1 ;
		break ;
		case SP_MOVE_RUN :
//			ENE_ResetRevMotion(act);
			act->keep_mot = EM_run_atk;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, AT_ActMoveRun);
			return 1 ;
		break ;
		case SP_ROLL_R :
			act->keep_mot = EM_squat_roll_r ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, DEF_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_ROLL_L :
			act->keep_mot = EM_squat_roll_l ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, DEF_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_HIGH_START_R :
//			ENE_ResetRevMotion(act);
			act->keep_mot = EM_gbs_ak_nom_peek_r_start ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, DEF_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_HIGH_END_R :
//			ENE_ResetRevMotion(act);
			act->keep_mot = EM_gbs_ak_nom_peek_r_end ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, DEF_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_START_R :
//			ENE_ResetRevMotion(act);
			act->keep_mot = EM_gbs_ak_nom_peek_r_squat_start ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, DEF_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_END_R :
//			ENE_ResetRevMotion(act);
			act->keep_mot = EM_gbs_ak_nom_peek_r_squat_end ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, DEF_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_HIGH_IDLE_R : 
//			ENE_ResetRevMotion(act);
			act->keep_mot = EM_gbs_ak_nom_peek_r_fire ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, DEF_ActOneTime  );
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_IDLE_R : 
			act->keep_mot = EM_gbs_ak_nom_peek_r_squat_fire ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, DEF_ActOneTime  );
			return 1 ;
		break ;

		case SP_BLIND_IDLE_R : 
//			ENE_ResetRevMotion(act);
			act->keep_mot = EM_nom_fire_blind ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, DEF_ActOneTime  );
			return 1 ;
		break ;

		case SP_BLIND_IDLE_SQUAT_R : 
//			ENE_ResetRevMotion(act);
			act->keep_mot = EM_nom_fire_blind_squat ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, DEF_ActOneTime  );
			return 1 ;
		break ;

		case SP_PEEK_HIGH_START_L :
			act->keep_mot = EM_gbs_ak_nom_peek_l_start ;
//act->keep_mot |=(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, DEF_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_HIGH_END_L :
//			ENE_SetRevMotion(act);
			act->keep_mot = EM_gbs_ak_nom_peek_l_end ;
//act->keep_mot |=(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;

			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, DEF_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_START_L :
//			ENE_SetRevMotion(act);
			act->keep_mot = EM_gbs_ak_nom_peek_r_squat_start ;
act->keep_mot |=(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;

			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, DEF_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_END_L :
//			ENE_SetRevMotion(act);
			act->keep_mot = EM_gbs_ak_nom_peek_r_squat_end ;
act->keep_mot |=(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;

			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, DEF_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_HIGH_IDLE_L : 
//			ENE_SetRevMotion(act);
			act->keep_mot = EM_gbs_ak_nom_peek_l_fire ;
//act->keep_mot |=(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;

			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, DEF_ActOneTime  );
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_IDLE_L : 
//			ENE_SetRevMotion(act);
			act->keep_mot = EM_gbs_ak_nom_peek_r_squat_fire ;
act->keep_mot |=(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;

			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, DEF_ActOneTime  );
			return 1 ;
		break ;
		case SP_STAND_STILL :
			act->keep_mot = EM_stand ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, DEF_ActCorrectPos);
			return 1 ;
		break ;
		case SP_SQUATHIDE :
			act->keep_mot = EM_squat_hide_idle ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, DEF_ActCorrectPos);
			return 1 ;
		break ;
 		case SP_BACKWALK :
			act->keep_mot = EM_walk_b_atk;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, DEF_ActOneTime  );
			return 1 ;
		break ;
 		case SP_APPROCH :
			act->keep_mot = EM_cle_walk ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, DEF_ActOneTime  );
			return 1 ;
		break ;
 		case SP_MOVE_RUN_R :
			act->keep_mot = EM_slide_r_fire;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, DEF_ActOneTime  );
			return 1 ;
		break ;
 		case SP_MOVE_RUN_L :
			act->keep_mot = EM_slide_l_fire;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, DEF_ActOneTime  );
			return 1 ;
		break ;
		case SP_SHOT_POS : /*指定ポイント向け*/
//			ENE_ResetRevMotion(act);
			act->keep_mot = EM_kamae_gun_high ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, DEF_ActOneTime  );
			return 1 ;
		break ;
		/*GRD投げ*/
		case SP_GRD_HIGH :
			act->keep_mot = EM_gm_fire_high ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, ActGrdThrHigh ) ;
			return 1 ;
		break ;
		case SP_NSIGHT_ON : /*暗視ゴーグル付ける*/
			act->keep_mot = EM_gogle_on ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, ENE_ActNSight_ON ) ;
			return 1 ;
		break ;
	}
	return 0 ;
}


#if 0
ENE_ShootBullet( act, flag )
/* バレットフラグ */
#define	ENE_BULLET_NORMAL	0x00000000	/* ランダムずれ有り、狙いは頭、跳弾有り、軌跡有り、右手に銃 */
#define	ENE_BULLET_NORANDAM	0x00000001	/* ランダムずれなし */
#define	ENE_BULLET_NOWALL	0x00000002	/* 壁を突きぬける */
#define	ENE_BULLET_NOATTACK	0x00000004	/* 攻撃判定無し */
#define	ENE_BULLET_NOSPARK	0x00000010	/* 跳弾無し */
#define	ENE_BULLET_NOLINE	0x00000020	/* 軌跡無し */
#define	ENE_BULLET_LEFT		0x00000040	/* 左手に銃 */
#endif

#define	RIGHT_BULLET	(ENE_BULLET_NORMAL) 
#define	LEFT_BULLET		(ENE_BULLET_NORMAL|ENE_BULLET_LEFT)


/**  火器管制システム **/
#define SHOT_MODE_AIM	(0x01)
#define SHOT_MODE_NO_TRG	(0x02)
static void Def_Shot( entk ,mode )
ENETHINK	*entk ;
int mode ; /*0 銃の向き 1aim_pos方向 */
{
	FVECTOR *ShotPos ;
	int bullet_flag = 0 ;
	ENTK_TYPE_A *eve_a ;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;

	if(SIG_CheckStealthStatus(entk)){
		return ;
	}

	
	if((entk->status2 & ENE_STATUS2_AIM_GUNSHOOT )
	&&(mode&SHOT_MODE_AIM)&&(!( GM_PlayerStatus & PL_NO_SHOOT))
	)
	{
		ShotPos = &entk->act->aim_pos ;
	}else {
		ShotPos = NULL ;
	}
	if( entk->sw_gun & SW_FLAG_SWITCH1 )
	{
	/*左利き*/
		bullet_flag = LEFT_BULLET ;
	}else {
		bullet_flag = RIGHT_BULLET ;
	}
	if(eve_a->type == 3){
		bullet_flag |= (ENE_BULLET_NOATTACK) ;
	}
/*攻撃判定無し弾丸*/
	if(mode & SHOT_MODE_NO_TRG){
		bullet_flag |= (ENE_BULLET_NOATTACK) ;
	}
	if(mode & SHOT_MODE_AIM){
		bullet_flag |= ENE_BULLET_NORANDAM ;
	}
	ENE_ShootBullet( entk->act, bullet_flag,ShotPos ) ;
//	ENE_ShootBullet( entk->act, bullet_flag,&GM_PlayerPosition ) ;
}

static void Def_AimPosSet(ENETHINK *entk ,float sub)
{
	FVECTOR		shotpos ;

	/****
	新
	entk->status2 に ENE_STATUS2_AIM_GUNSHOOTを立てると
	entk->act->aim_posに向けてアジャストします。
	自動的にプレイヤーの狙う個所を設定するには
	ENE_SetAimPlayer( entk )
	を使用してください。
	****/
	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
	if( GM_PlayerStatus & PLAYER_WATCH){
		shotpos = GM_PlayerSubjectCamera[0]->position ;
	}else {
		GV_MatToVec( &( BODYWORLD(GM_PlayerBody,HUMAN21_ATAMA) ),&shotpos);
	}
	entk->act->aim_pos = shotpos;
}


static void Think3_NSight_ON( entk )
ENETHINK	*entk ;
{
//	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_NSIGHT_ON ;
//	}
	if ( entk->act->act_end ) {
		entk->think1 = ENE_TH1_DEFENSE ; 
		entk->think2 = TH2_MOVE ; 
		entk->think3 = TH3_ZONE_CHASE ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->count3 ++ ;
}
static void Think3_ZoneChaseTrgp( entk )
ENETHINK	*entk ;
{
	int reach;
	AT_THK *at_thk;
	ENTK_TYPE_A *eve_a ;

	at_thk = (AT_THK *) entk->character ;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	if(eve_a->type <= 1) {
		/*左舷兵の追っかけ移行処理*/
		if(Def2AtCheck(entk)){
			return;
		}
	}


#ifdef DEBUG_MODE
//	PosBox( &entk->trgpoint.pos ,250.0F ,NULL );
	if(entk->count3 %(AT_THK_RATE*5)==0){
//		SigZoneView( entk->trgpoint.addr , NULL , 1000.0F ) ; 
	}
#endif
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


	/*現在地アドレス更新*/
	if(!(entk->count3%(AT_THK_RATE*5))) {
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

	if(entk->count3%(AT_THK_RATE*3)==0){
		if(
//			( entk->pl_eyei.dis > 2000)&&
			( entk->pl_eyei.dis > 1000)&&
			(
//			(at_thk->at_status & AT_ST_DEFENSE_MOVE)
			(0)
			||(reach <= 2)
			||(entk->count3 < (AT_THK_RATE*5))
			)
		){
			SetMoveMode(entk);
		} else {
			if(eve_a->type >= 2){
				if(entk->count3 > DIRECT_TICK(60))
					SetMoveMode(entk);
				else 
					entk->act->move_s = MoveAttackRun ;
			}else {
#if 0
				if(entk->act->move_s != MoveAttackRun){
					entk->act->tmp_time = (AT_THK_RATE*2) ;
				}
				entk->act->move_s = MoveAttackRun ;
#else
				if(reach <= 2){
					SetMoveMode(entk);
				}else {
					entk->act->move_s = MoveAttackRun ;
				}
#endif
			}
		}
	}
//	ReverseCheck( entk );

	
	/*移動中の自衛*/
	if(
	(entk->count3 > DEF_MOVE_SHOT_DELAY)
	&&(entk->count3%(AT_THK_RATE) ==0)
	&&( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )
	){
		if(eve_a->type <= 1){
			if(
			(abs(SIG_CheckDirSub(entk->trgpoint.dir,entk->pl_eyei.dir)) < 128)
			){
				Def_Shot( entk ,0);
			}
		}else {
				Def_Shot( entk ,SHOT_MODE_NO_TRG);
		}
	}
	if(entk->act->move_s == MoveAttackRun) {
		entk->act->dir = entk->trgpoint.dir ;
	} else {
		entk->act->dir = entk->pl_eyei.dir ;
		/*プレイヤ方向見ながら移動*/
//吹っ飛ばし
#if 0
		if(eve_a->type <= 1){
			if( entk->pl_eyei.dis < 4000) Def_Set_ForceDir( entk ,500.0F);
		}
#endif
	}
	entk->count3 ++ ;
	if(at_thk->at_tmptime > 0)	at_thk->at_tmptime-- ;
}

static	void	Think3_DirectChaseTrgp( entk )
ENETHINK	*entk ;
{
	int		reach;
	ENTK_TYPE_A *eve_a ;
	AT_THK *at_thk;
//	SVECTOR		rgb;
	
#if 0
	rgb.vx = 255 ;	rgb.vy = 0 ;	rgb.vz = 0 ;
	PosBox( &entk->trgpoint.pos ,100.0F ,&rgb );
	rgb.vx = 0 ;	rgb.vy = 0 ;	rgb.vz = 255 ;
	PosBox( &(entk->znavi->flore_pos) ,50.0F ,&rgb );
//	SigZoneView( entk->trgpoint.addr , NULL , 1000.0F ) ; 
#endif
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;

	if(eve_a->type <= 1) {
		/*左舷兵の追っかけ移行処理*/
		if(Def2AtCheck(entk)){
			return;
		}
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


	/*目標点の高さを修正*/
//	entk->trgpoint.pos.vy = entk->znavi->flore_pos.vy ;
	/*追跡のための方向設定と距離判定*/
	if(ENE_DirectTrace( &(entk->trgpoint),
//	 &(entk->znavi->flore_pos), 250 ) <0 ){
//	 &(entk->znavi->flore_pos), 50 ) <0 ){
	 &(entk->znavi->flore_pos), 100 ) <0 ){
		entk->act->mot_speed_correct = 0.0F;
		entk->ctrl->mov.vx = entk->trgpoint.pos.vx ;
		entk->ctrl->mov.vz = entk->trgpoint.pos.vz ;
		/*到達*/
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
		entk->act->dir = entk->ctrl->turn.vy ;
		entk->act->pad = SP_STAND_STILL ;

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

	if(entk->count3%(AT_THK_RATE*3)==0){
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
		/*プレイヤ方向見ながら移動*/
			ENE_SetAimPlayer( entk ) ;
			entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
//			Def_AimPosSet(entk ,-500) ;
		entk->act->dir = entk->pl_eyei.dir ;
		if(
		(1)
		&&( entk->pl_eyei.dis < entk->sense.eye_s)
		){
		}
	}



	entk->count3 ++ ;
}

/*
守備位置待機から特定の攻撃アクションモードを開始する処理
*/
static void DEF_StartDefAttack(ENETHINK *entk){
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;
	AT_THK *at_thk;

   
   BP_Update_defthink_Tables();

	at_thk = (AT_THK *) entk->character;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;

	/*ライト破壊モード*/
	if((def_com->trgmode)&&(def_com->count == 0)&&(eve_a->type == 2)
	){
		eve_a->level = DEF_LEVEL_SUBTRG ;
//		COM_CallRouteVoice( -1 , 0, EV_TALK_ONESELF, entk ) ;
//		printf("VOICE STREAM CALLED!!!!!!\n");
	}else if(
		/*左舷条件*/
		((eve_a->type <=1 )
		&&(entk->at_com->Pl_StayTime > DEF_STAY_LIMIT)
		&&(at_thk->dis_rank==0)
		&&(entk->pl_eyei.dis < DEF_GRD_THR_LIMIT))
		/*右舷条件*/
		||
		((eve_a->type >1 )
		&&(def_com->mode_cnt < (DEF_MODE_CNT/2) )
		&&(def_com->grd_cnt == DEF_GRD_CNT)
		&&(at_thk->dis_rank == 2)
		)||
		((eve_a->another_flag & ANOTHER_DARK_CAMP )
		&&(entk->at_com->Pl_StayTime > DEF_STAY_LIMIT)
		&&(( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )
		||(at_thk->at_status & (AT_ST_FEEL|AT_ST_FEEL_LOW )))
		&&(entk->pl_eyei.dis < DEF_GRD_THR_LIMIT))
		){
/*throw bomb*/
/*
守備位置待機から特定の攻撃アクションモードを開始する処理
ザコサバでは爆弾投げ条件が違う
*/

			entk->at_com->Pl_StayTime = DEF_STAY_LIMIT - DEF_STAY_DECAY ;
			def_com->grd_cnt = 0 ;
//			entk->at_com->Pl_StayTime = 0 ;
			eve_a->level = DEF_LEVEL_GRD ;
			entk->act->target_pos = GM_PlayerPosition ;
			entk->act->target_pos.vy = GM_PlayerControl->levels[0] ;
/*2001.01.25 グレネード投げたら他の兵士を待機*/
		def_com->seq_cont = DEF_ATTACK_GRD_CYCLE ;
	}else {
		eve_a->level = (irnd()>>8)%(DEF_LEVEL_GRD) ;
	}

	at_thk->at_tmptime = 0;
	eve_a->seq_index = 0;
	entk->count3 = 0 ; 
	entk->think3 = TH3_ATTACK_SETUP ; 

	eve_a->def_seq = (u_char)(def_com->def_act_num[(int)eve_a->def_pos_num] ) ;
	eve_a->seq_time = 
		act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2+1];
	eve_a->seq_count = 0 ;


#if 1
	/*2001.01.22追加 右舷 突入モード*/
	if(
	((eve_a->type > 1)
	&&(def_com->mode_cnt >= (DEF_MODE_CNT/2) )
	&&(at_thk->dis_rank==0)
	&&(def_com->def_mode != AT_COM_DEF_WAIT ))
	||(def_com->def_mode == AT_COM_DEF_DYNAMIC )
	){
//		def_com->mode_cnt = 0 ;
		eve_a->status |= EVE_A_NO_DEF ;
		DefPosFree(entk);
		entk->act->pad = SP_READYGUN ;
		entk->count3 = 0;
	}

#endif

}

/*左舷兵の追っかけ移行処理*/
/*****
注意 左舷の２フェイズ以降の突撃は
static void DEF_StartDefAttack(ENETHINK *entk)
内で分岐
******/

static int Def2AtCheck(ENETHINK *entk){
	int 		zone_dis ;
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;
	AT_THK *at_thk;

	at_thk = (AT_THK *) entk->character;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;
	/*
	守備モードから追跡モードへ移行する条件
	ザコサバでは追跡しない
	*/
	if(eve_a->another_flag & ANOTHER_DARK_CAMP ){
		return 0 ;
	}

	if(entk->count3%(AT_THK_RATE*2) ==0){
		zone_dis = ENE_GetRouteDis(&def_com->final_pos,
			&entk->ctrl->mov,def_com->final_addr,
			entk->ctrl->addr,5000000);
		if( zone_dis > (def_com->pl_fi_dis-2000)){
			eve_a->status |= EVE_A_NO_DEF ;
			DefPosFree(entk);
			entk->act->pad = SP_READYGUN ;
			entk->count3 = 0;
			return 1;
		}
	}

	/*2001.01.22追加 時間制限条件*/
	/*プレイヤが止まって 直線距離で遠く 自分が一番近ければ攻撃兵モードへ*/
	if(
	(entk->at_com->Pl_StayTime > DEF_STAY_LIMIT )
	&&(entk->pl_eyei.dis >= DEF_GRD_THR_LIMIT)
	&&(at_thk->dis_rank==0)
	&&(eve_a->type <= 1)
	){
		eve_a->status |= EVE_A_NO_DEF ;
		DefPosFree(entk);
		entk->act->pad = SP_READYGUN ;
		entk->count3 = 0;
		return 1;
	}



	return 0;
}
/*攻撃兵モードから守備兵に戻る*/
void DEF_AT2DEF_Check(ENETHINK *entk)
{

	int i,ene_zone_dis ;
	ENTK_TYPE_A *eve_a ;
	DEF_COM	*def_com;
	AT_THK *at_thk;

	at_thk = (AT_THK *) entk->character;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	def_com = (DEF_COM *) eve_a->def_com ;
	ene_zone_dis = ENE_GetRouteDis(&entk->ctrl->mov,
		&def_com->final_pos,entk->ctrl->addr,
		def_com->final_addr,9000000 );
/*2001.01.22 TEST 左舷突入から復帰しない*/	
	if(eve_a->type > 1){
		if((def_com->mode_cnt >= (DEF_MODE_CNT/2) )
		&&(at_thk->dis_rank==0)
		){
			return ;
		}
		if(def_com->def_mode == AT_COM_DEF_DYNAMIC ){
			return ;
		}
		/*必ず守備復帰*/
		GetDefPos2(entk);
		eve_a->status &= ~EVE_A_NO_DEF ;
	}
/*************************************/


	/*敵兵がプレイヤより3000以上奥にいて*/
	/*プレイヤより3000以上奥のposが空いてる*/
	/*プレイヤにダメージを与えた*/
/*at_thk->zone_dis*/

/*攻撃兵モードから守備兵に戻る*/
/*ザコサバ*/
	if((eve_a->another_flag & ANOTHER_DARK_CAMP )) {
		for(i=0;i<EVENT_DEF_POINT_MAX;i++){
			/*
			守備目標があいてれば無条件移動
			距離制限つけたほうがよいか？
			ザコサバでは後ろを通ってプレイエリアを連結してるので、
			ゾーン距離基準で進行するとみんな後ろに集まる。
			リスト順にしておけばシナリオ指定の順で、意図的に集まりやすいところを
			制御できるからこの方がよいかも。
			*/

			if(
			( def_com->def_use[i] < 0  )
			){
				entk->def_pos = def_com->def_pos[i] ;
				entk->def_mapbit = def_com->def_mapbit[i] ;
				def_com->def_use[i] = entk->id ;
				eve_a->def_pos_req = i;
				eve_a->def_pos_num = i; /*守備位置番号*/
				eve_a->status &= ~EVE_A_NO_DEF ;
				break;
			}
		}
	}else if(((def_com->pl_fi_dis) > (ene_zone_dis+3000))) {
/*正式*/
		for(i=0;i<EVENT_DEF_POINT_MAX;i++){
			/*守備目標が手ごろにあいてれば*/
			if(
//			(def_com->def_zonedis[i] > (ene_zone_dis - 2000) )
			(def_com->def_zonedis[i] < (ene_zone_dis ) )
			&&(def_com->def_zonedis[i] > (def_com->pl_fi_dis - DEF_GRD_THR_LIMIT + 2000) )
			&&( def_com->def_use[i] < 0  )
			){
				entk->def_pos = def_com->def_pos[i] ;
				entk->def_mapbit = def_com->def_mapbit[i] ;
				def_com->def_use[i] = entk->id ;
				eve_a->def_pos_req = i;
				eve_a->def_pos_num = i; /*守備位置番号*/
				eve_a->status &= ~EVE_A_NO_DEF ;
				break;
			}
		}
	}
}


static void Think3_Wait( entk )
ENETHINK	*entk ;
{
	int zone_dis ;
	FVECTOR		testtrg,subvec;
	float len;
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;
	AT_THK *at_thk;
	ACTION	*act;
//	SVECTOR rgb;
	CONTROL	ctrl ;

	at_thk = (AT_THK *) entk->character;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	act = entk->act ;
	def_com = (DEF_COM * ) eve_a->def_com ;

	/*左舷 進行処理*/
	/*守備状態から、別の守備位置へ移動する判定*/
	if(entk->count3 > DEF_WAIT_TIME){
		/*ザコサバでは開始条件と選択条件が違う*/
		if(eve_a->another_flag & ANOTHER_DARK_CAMP ){
			if(eve_a->shuffle_time > DIRECT_TICK(60*5)){
				if(entk->at_com->Pl_StayTime > DEF_STAY_LIMIT ){
					/*適当にランダムっぽくdefposを選ぶ DARK FLAG兵 専用関数*/
					/*posがあればdef_pos_req更新 無ければNOP*/
					CheckDefPos2(entk);
					if(eve_a->def_pos_req != eve_a->def_pos_num){
						/*def_pos_req更新されてたら反映させる*/
						GetDefPos3( entk) ;
						eve_a->status &= ~EVE_A_NO_DEF ;
						ctrl.mov = entk->def_pos ;
						GM_ConfigControlMapID( &ctrl ) ;
						entk->trgpoint.pos = entk->def_pos ;
						entk->trgpoint.map = ctrl.map ;
						entk->trgpoint.addr = ctrl.addr ;
						eve_a->shuffle_time = 0 ;
					}
				}
			}
		}else if((eve_a->type <= 1)&&(at_thk->dis_rank==0)){
		/*プレイヤから廊下終端までのゾーン距離*/
			zone_dis = def_com->def_zonedis[(int)eve_a->def_pos_num] ;
			/*先頭待機兵から4000以上離れていれば次の場所調べる*/
			if((zone_dis+4000) < (def_com->pl_fi_dis ) ){
				CheckDefPos(entk);
			}
			if(eve_a->def_pos_req != eve_a->def_pos_num){
				GetDefPos3( entk) ;
				eve_a->status &= ~EVE_A_NO_DEF ;
				ctrl.mov = entk->def_pos ;
				GM_ConfigControlMapID( &ctrl ) ;
				entk->trgpoint.pos = entk->def_pos ;
				entk->trgpoint.map = ctrl.map ;
				entk->trgpoint.addr = ctrl.addr ;
			}
		}else {
#if 1
/* For Japan */
			if((eve_a->type <= 1)&&(entk->count3 > DIRECT_TICK(60))) {
				/*左舷兵の追っかけ移行処理*/
				if(Def2AtCheck(entk)){
printf("TH_WAIT 2 CHASER !!!!!!!!!!!!!!!!!!!!\n");
					return;
				}
			}
#endif
		} 
	}
	GetDefPos3(entk);
	if(
//	( entk->ctrl->addr != entk->trgpoint.addr )
	(abs(entk->znavi->flore_pos.vx - entk->def_pos.vx) > 250)
	||(abs(entk->znavi->flore_pos.vz - entk->def_pos.vz) > 250)
	){
		/*新目標に向けて移動開始*/
		entk->think1 = ENE_TH1_DEFENSE ; 
		entk->think2 = TH2_MOVE ; 
		entk->think3 = TH3_ZONE_CHASE ; 
		entk->count3 = 0 ;
		return ; 
	}
	/*そのままの場所で守備続行*/
	act->pad = SP_SQUATHIDE ;
	act->dir = entk->pl_eyei.dir  ;
	/* 位置補正処理 */

	testtrg = entk->def_pos ;
	testtrg.vy = entk->ctrl->mov.vy;
    _sceVu0SubVector( &subvec, &testtrg, &entk->ctrl->mov ) ;
	len = GV_VecLen3F( &subvec );
	if( len < 25.0F) {
		act->tmp_dir = -1;
	}else {
		act->tmp_dir = GV_VecDir2( &subvec );
	}

	/*アクション開始*/
	/*同期させる場合はここで条件設定*/
#if 0
	if(entk->count3 > DEF_WAIT_TIME){
		DEF_StartDefAttack(entk);
		return ;
	}
#else
/*2002.07 ザコサバ兵は飛び出しを不規則*/
/************************************************
ANOTHER_DARK_CAMP兵 は守備位置更新条件が違うので
ここで守備値未設定の場合がある設定されるまで待機
*************************************************/
	if(eve_a->def_pos_num<0){
		return ;
	}
	if(eve_a->type <= 1 ){
		if(
		((at_thk->dis_rank<=1)
		||((eve_a->another_flag & ANOTHER_DARK_CAMP )&&((irnd()>>8)%60==0)  ))
		&&(entk->count3 > DEF_WAIT_TIME)
//		&&(def_com->seq_cont%300 == 0)
		){
			DEF_StartDefAttack(entk);
			return ;
		}
	}else {
		if(
		(entk->count3 > DEF_WAIT_TIME)
//		&&(def_com->seq_cont%DEF_ATTACK_CYCLE == 0)
		&&(def_com->seq_cont  == 0)
		){
			DEF_StartDefAttack(entk);
			return ;
		}
	}
#endif
	entk->count3 ++ ;
}
/*ATTACK*/
static void Think3_AttackStand( entk )
ENETHINK	*entk ;
{
//	int zone_dis,next_pad,i,bure = 1,shot_mode;
	int i,bure = 1,shot_mode;
	FVECTOR		testtrg,subvec;
	float len;
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;
	AT_THK *at_thk;
	ACTION	*act;
//	SVECTOR rgb;

   BP_Update_defthink_Tables();

	at_thk = (AT_THK *) entk->character ;
	shot_mode = SHOT_MODE_AIM ;
	act = entk->act ;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;

	if(eve_a->type <= 1) {
		/*左舷兵の追っかけ移行処理*/
		if(Def2AtCheck(entk)){
			return;
		}
	}

	entk->act->dir = entk->pl_eyei.dir  ;
	/*守備時間のカウント*/
	if(eve_a->det_time < 30000 ) eve_a->det_time++;
	/* 位置補正処理 */
	testtrg = entk->def_pos ;
	testtrg.vy = entk->ctrl->mov.vy;
    _sceVu0SubVector( &subvec, &testtrg, &entk->ctrl->mov ) ;
	len = GV_VecLen3F( &subvec );
	if( len < 25.0F) {
		act->tmp_dir = -1;
	}else {
		act->tmp_dir = GV_VecDir2( &subvec );
	}

	/*これより下でシーケンス処理*/
	if(entk->count3 ==0){
		/*一番最初*/
		at_thk->at_tmptime = 0; /*着弾予測カウンタ*/
		eve_a->seq_time = 
		act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2+1];
		eve_a->seq_count = 0 ;
	}
	/*待機状態なら再飛び出しチェックは毎フレーム*/
	if(
	(eve_a->status & EVE_A_WAIT)
	&&(eve_a->seq_index == 0)
	){
		if( entk->pl_eyei.dis < (entk->sense.eye_s-250)){
			eve_a->seq_index = 1;
			eve_a->seq_time = 
			act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2+1];
			eve_a->seq_count = 0 ;
			eve_a->status &= ~EVE_A_WAIT ;
		}
	}

	/* パッドは毎フレーム更新 */
	act->pad = act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2];
	/* 
	tmptimeは着弾予測処理の回数を減らすためのカウンタに使用
	*/
	/* inndexに従いアクション時間セット */
	/* 時間は切り替わった時のみセットして以降減衰 */

	/*左持ち兵の判定*/
	ReverseCheck( entk );
	/*プレイヤ以外を目標に*/

	if((def_com->trgmode)
//	&&(act->pad > SHOT_ACT)
	&&(def_com->count == 0)
	&&(eve_a->level == DEF_LEVEL_SUBTRG)
	&&(eve_a->type == 2)
	){
//dir
		for(i=0;i<2;i++){
			if(def_com->trgmode& (1<<i)) {
				entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
				act->aim_pos = def_com->trgpos2[i] ;
				/*手ぶれ禁止*/
				bure = 0 ;
//				entk->act->dir = entk->pl_eyei.dir  ;
				entk->act->dir = GV_VecDir2FromTo( &entk->ctrl->mov,&act->aim_pos ) ;
				break;
			}
		}
	}else {
	/*パッドによって狙い場所変えてるのでpad設定後にaim*/
		Def_AimPosSet(entk ,-500) ;
	}
	/* 攻撃アクションなら発砲*/
	if(	act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2] > SHOT_ACT){
		float	trg_dis ;
		trg_dis = GV_VecLen3F2( &act->aim_pos, &entk->ctrl->mov );

		if(
			(eve_a->seq_count > eve_a->shot_delay )
			&&(eve_a->seq_count <= (eve_a->shot_delay+eve_a->shot_time) )
			&&(entk->count3%(AT_THK_RATE) ==0)
			&&(at_thk->at_tmptime == 0)
			&&(CheckDefShotDis(entk))
		){
			if((bure)&&(eve_a->shot_rnd > 0)){
				/*手ぶれ*/
				int		range1,range2 ;
				float	shot_time ,shot_rest;
				float	shot_rnd;
				shot_time =(float) (eve_a->shot_delay+eve_a->shot_time) ;
				shot_rest = shot_time - (float) eve_a->seq_count ;
#if 1
				shot_rnd = (float) eve_a->shot_rnd  * shot_rest/shot_time ;
#else
				if(shot_rest< DEF_ABS_HIT_TIME) shot_rnd = 0 ;
				else shot_rnd = (float) eve_a->shot_rnd  * shot_rest/shot_time ;
#endif
				shot_rnd *= (trg_dis/6000.0F);

//				if(shot_rest > 10 ) shot_mode |= SHOT_MODE_NO_TRG ;
//				if(shot_rnd >= 100.0F ) shot_mode |= SHOT_MODE_NO_TRG ;

				if(shot_rnd >= NO_SHOT_LIMIT ) shot_mode |= SHOT_MODE_NO_TRG ;

				if(shot_rnd >= 1.0F){
#if 0
					range1 = eve_a->shot_rnd ;
					range2 = eve_a->shot_rnd/2 ;
#else
					range1 = (int) (shot_rnd) ;
					range2 = (int) (shot_rnd/2.0F) ;
#endif

					entk->act->aim_pos.vx += ((float)((irnd()>>8)%range1)-(float)range2) ;
					entk->act->aim_pos.vy += ((float)((irnd()>>8)%range1)-(float)range2) ;
					entk->act->aim_pos.vz += ((float)((irnd()>>8)%range1)-(float)range2) ;
				}
			}
		/*
			着弾予測して自分の直前壁で遮られるようなら撃たない
		*/
#if 0
			if(1000.0F < CheckGunPlOnline(entk,&entk->act->aim_pos))
			{
				Def_Shot( entk ,shot_mode );
			} else {
				(at_thk->at_tmptime = (AT_THK_RATE*2)) ;
			}
#else
//			if((trg_dis - 2000.0f ) > CheckGunPlOnline(entk,&entk->act->aim_pos))
//For Japan

//			PosBox( &entk->act->aim_pos ,250.0F ,NULL );

			if((trg_dis - 3000.0f ) > CheckGunPlOnline(entk,&entk->act->aim_pos))
			{
				(at_thk->at_tmptime = (AT_THK_RATE*2)) ;
			} else {
				Def_Shot( entk ,shot_mode );
			}

#endif
		}
	}
//PosBox(&entk->act->aim_pos,250.0F,NULL);
	/*カウンタ減衰*/
	if(at_thk->at_tmptime>0) at_thk->at_tmptime--;
	entk->bullet = 0 ;
	entk->count3 ++ ;
	/*ビデオ用 待機*/
	if((eve_a->video != 1)
	&&(eve_a->seq_time != WAIT_END)
	){
		eve_a->seq_count++ ;
	}else {
	}
if(0) {
	printf("DEFTHINK eve_a->seq_count %d\n",eve_a->seq_count);
	printf("DEFTHINK eve_a->seq_time %d\n",eve_a->seq_time);
}
	/*次のフレームで新アクションをセットする時*/
	if(
	((eve_a->seq_time == IDLE_TIME)
	&&(eve_a->seq_count>(eve_a->shot_delay+eve_a->shot_time+eve_a->wait_time)))
	||((eve_a->seq_time == WAIT_END)&&(entk->act->act_end))
	||((eve_a->seq_time >= MAGIC_END)&&(eve_a->seq_count>=eve_a->seq_time))
	){
		at_thk->at_tmptime = 0;
		eve_a->seq_index++;
		/*次にセットされる予定が終端*/
		if(act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2] == SP_DATA_END )
		{
			if((abs(entk->znavi->flore_pos.vx - entk->def_pos.vx) > 500)
			||(abs(entk->znavi->flore_pos.vz - entk->def_pos.vz) > 500)
			){
				entk->think1 = ENE_TH1_DEFENSE ; 
				entk->think2 = TH2_MOVE ; 
				entk->think3 = TH3_ZONE_CHASE ; 
			}else {
				entk->think1 = ENE_TH1_DEFENSE ; 
				entk->think2 = TH2_ATTACK ; 
				entk->think3 = TH3_WAIT ;
			}
			entk->count3 = 0 ;
			return ;
		}
		/*次のアクションセット*/
		eve_a->def_seq = (u_char)(def_com->def_act_num[(int)eve_a->def_pos_num] ) ;

		eve_a->seq_time = 
			act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2+1];
	}
}
#define PL_NO_SHOOT (PLAYER_DAMAGED|PLAYER_DOWNED|PLAYER_FORCE)

#define ALERT_DIS 7000
#define ATTACK_DIS 6000
#define ATTACK_FORCE  (1000.0F)
/*左舷 後方 無敵兵のスタンド思考*/
static void Think3_DefenseStand( entk )
ENETHINK	*entk ;
{
	FVECTOR 	testtrg,subvec;
	float len,z_dis;
	ENTK_TYPE_A *eve_a ;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	eve_a = (ENTK_TYPE_A *) entk->eve_a ;

	/*守備兵の守備経過時間用のdet_timeを*/
	/*後方兵は必要ないので覗き込みシーケンス管理に流用*/

	entk->sw_gun |= SW_FLAG_SWITCH1 ;
	Def_AimPosSet(entk ,-500) ;

	/*守備座標への補正*/
	testtrg = entk->def_pos ;
	testtrg.vy = entk->ctrl->mov.vy;
//	GV_SubVec3( &testtrg, &entk->ctrl->mov, &subvec );
    _sceVu0SubVector( &subvec, &testtrg, &entk->ctrl->mov ) ;
	len = GV_VecLen3F( &subvec );
	if( len < 25.0F) {
		entk->act->tmp_dir = -1;
	}else {
		entk->act->tmp_dir = GV_VecDir2( &subvec );
	}
	if(entk->count3 == 0){
		eve_a->det_time = 0 ;
	}
#if 0
	if(eve_a->def_pos_num != eve_a->def_pos_req){
		/*守備位置更新されている*/
		eve_a->seq_index = 0 ;
		at_thk->at_status |= AT_ST_DEFENSE_MOVE ;
		entk->think1 = ENE_TH1_DEFENSE ; 
		entk->think2 = TH2_MOVE ; 
		entk->think3 = TH3_ZONE_CHASE ; 
		entk->count3 = 0 ;
		return ; 
	}
#endif
	/*吹っ飛ばし攻撃のためＺ距離判定*/
	z_dis = abs( entk->ctrl->mov.vz - GM_PlayerPosition.vz) ;


	ReverseCheck( entk );

	if(z_dis > ALERT_DIS) {
		/*隠れてろ*/
		if(eve_a->det_time == 0){
			entk->act->pad = SP_STAND_STILL ;
		} else if (eve_a->det_time == 1) {
			entk->act->pad = SP_PEEK_HIGH_END_L ;
		} 
		if(entk->act->act_end) {
			eve_a->det_time = 0 ;
			entk->act->pad = SP_STAND_STILL ;
		}
	} else if (z_dis > ATTACK_DIS) {
		/*警戒しろ*/
		if(eve_a->det_time == 0){
			entk->act->pad = SP_PEEK_HIGH_START_L ;
#if 1
			if(entk->act->act_end) {
				entk->act->pad = SP_PEEK_HIGH_IDLE_L ;
				eve_a->det_time = 1 ;
				entk->act->pad = -1;
				entk->act->dir = -1 ;
			}
#endif
		}else {
			entk->act->pad = SP_PEEK_HIGH_IDLE_L ;
			if(entk->count3%AT_THK_RATE ==0) {
				Def_Shot( entk ,SHOT_MODE_AIM );
			}
		}
	}else {
#if 0
		/*ふっ飛ばせ*/
		if(!(GM_PlayerStatus & PL_NO_SHOOT)) {
			if(entk->count3%(AT_THK_RATE) ==0) Def_Shot( entk ,SHOT_MODE_AIM );
			Def_Set_ForceDir( entk ,ATTACK_FORCE);
		}
#endif
		eve_a->det_time = 2 ;
		entk->act->pad = SP_PEEK_HIGH_IDLE_L ;
	}


	/*パッドによって狙い場所変えてるのでpad設定後にaim*/
	Def_AimPosSet(entk ,-500) ;

	entk->bullet = 0 ;

/*射撃方向*/
	entk->act->dir = entk->pl_eyei.dir  ;
	entk->count3 ++ ;
#if 0
	if(eve_a->seq_time==0){
		entk->count3 = 0 ;
	/* 目標ゾーンから離れていれば戻る*/
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
			entk->count3 = 0 ;
			return ; 
		}
	}
#endif
}


static void Think3_Wait_Dynamic( entk ) 
ENETHINK	*entk ;
{
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;

	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;

	entk->act->pad  = SP_STAND_STILL ;
	if(entk->count3 >DIRECT_TICK(60*3)){
		def_com->wait_num++ ;
	}
	if(entk->count3 >DIRECT_TICK(60*60)&&( def_com->def_mode == AT_COM_DEF_WAIT )){
		 def_com->def_mode = AT_COM_DEF_DYNAMIC ;
	}

	if(def_com->def_mode != AT_COM_DEF_WAIT ){
//		entk->think1 = ENE_TH1_DEFENSE ; 
//		entk->think2 = TH2_MOVE ; 
//		entk->think3 = TH3_ZONE_CHASE ; 
		entk->count3 = 0 ;
		eve_a->status |= EVE_A_NO_DEF ;
		DefPosFree(entk);
		AlertModeStart(entk);

		entk->act->pad = SP_READYGUN ;
	}
	entk->act->dir = entk->pl_eyei.dir  ;
	entk->act->tmp_dir = -1  ;
	entk->count3++ ;
}


static void Think2_Move( entk ) 
ENETHINK	*entk ;
{
	int zone_dis ;
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;

	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;


	if(eve_a->type <= 1){
#ifdef DEF_POS_CHECK
		zone_dis = def_com->def_zonedis[(int)eve_a->def_pos_num] ;
		/**/
		if((zone_dis) > (def_com->pl_fi_dis - 3000 ) ){
			CheckDefPos(entk);
		}
#endif
		if(
		(eve_a->def_pos_req != -1)
		&&(eve_a->def_pos_req != eve_a->def_pos_num)
		){
			CONTROL	ctrl;
		GetDefPos3( entk) ;
			eve_a->status &= ~EVE_A_NO_DEF ;
			ctrl.mov = entk->def_pos ;
			GM_ConfigControlMapID( &ctrl ) ;
			entk->trgpoint.pos = entk->def_pos ;
			entk->trgpoint.map = ctrl.map ;
			entk->trgpoint.addr = ctrl.addr ;
		}
	}

	switch ( entk->think3 ) {
		case TH3_ZONE_CHASE :
			Think3_ZoneChaseTrgp( entk ) ;
			break;
		case TH3_DIRECT_CHASE :
			Think3_DirectChaseTrgp( entk ) ;
			break;
		case TH3_NSIGHT_ON :
			Think3_NSight_ON(entk) ;
			break;
		case TH3_WAIT :
			Think3_Wait_Dynamic(entk) ;
			break;

	}
}

static void Think2_Attack( entk ) 
ENETHINK	*entk ;
{
	ENTK_TYPE_A *eve_a ;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;

	switch ( entk->think3 ) {
		case TH3_ATTACK_SETUP :
			if(eve_a->type != 3)
			{
				Think3_AttackStand( entk ) ;
			} else {
				Think3_DefenseStand( entk ) ;
			}
			break ;
		case TH3_WAIT :
				Think3_Wait( entk ) ;
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
//	FVECTOR force = { 0.0F,0.0F,-1000.0F } ;
	int pl_zone_dis,ene_zone_dis;

	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;

	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
//	def_com = (DEF_COM * ) entk->def_com ;
	def_com = (DEF_COM * ) eve_a->def_com ;

#ifndef ALERT_SET
	/*守備中は警戒レベルに影響しない*/
	entk->alert = 0;
#endif

	if(eve_a->status & EVE_A_SUPORT_WAIT)
	{
		Think2_Wait( entk ) ;
	} else {
		switch ( entk->think2 ) {
		    case TH2_MOVE :		/* 移動 */
				if((eve_a->type <=1)&&(!(eve_a->another_flag & ANOTHER_DARK_CAMP ))){
					/*追い抜かれたら切り替え*/
					pl_zone_dis = ENE_GetRouteDis(&GM_PlayerPosition,
						&def_com->final_pos,GM_PlayerControl->addr,
						def_com->final_addr,9000000 );
					ene_zone_dis = ENE_GetRouteDis(&entk->ctrl->mov,
						&def_com->final_pos,entk->ctrl->addr,
						def_com->final_addr,9000000 );
					if(pl_zone_dis < ene_zone_dis) {
//					entk->think_mode = ENE_TH1_ALERT ;
						eve_a->status |= EVE_A_NO_DEF ;
						DefPosFree(entk);
						entk->act->pad = SP_READYGUN ;
						entk->count3 = 0;
						return ;
					}
					if(eve_a->def_pos_req != eve_a->def_pos_num){
						CONTROL	ctrl;
						GetDefPos3( entk) ;
						eve_a->status &= ~EVE_A_NO_DEF ;
						ctrl.mov = entk->def_pos ;
						GM_ConfigControlMapID( &ctrl ) ;
						entk->trgpoint.pos = entk->def_pos ;
						entk->trgpoint.map = ctrl.map ;
						entk->trgpoint.addr = ctrl.addr ;
					}
				}
				Think2_Move( entk ) ;
			break ;
		    case TH2_ATTACK :		/* 攻撃 */
#if 0
				if(eve_a->type == 2){
					if( entk->pl_eyei.dis < 3000){
				/*プレイヤ方向に対する吹き飛ばし*/
						Def_Set_ForceDir( entk ,500.0F);
					}
				}
#endif
				Think2_Attack( entk ) ;
				eve_a->shuffle_time++ ;
			break ;
		}
	}
#ifdef ALERT_SET
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->alert = ALERT_LEVEL_MAX ;
	}
#endif

	/*前フレームでの目標位置待避*/
//	( (AT_THK * ) entk->character )->old_trgpos = entk->trgpoint.pos;
}

/*守備位置解放*/
void DefPosFree(ENETHINK *entk){
	ENTK_TYPE_A *eve_a ;
	DEF_COM	*def_com;
	eve_a = (ENTK_TYPE_A *) entk->eve_a ;
//	def_com = (DEF_COM *) entk->def_com ;
	def_com = (DEF_COM *) eve_a->def_com ;
	if(
	(eve_a->def_pos_num >= 0)
	&&(eve_a->def_pos_num < EVENT_DEF_POINT_MAX)
	){
		def_com->def_use[ (int) eve_a->def_pos_num]= -1 ;
	}
	eve_a->seq_index = 0 ;
	eve_a->def_pos_num = -1; /*守備位置番号*/
	eve_a->def_pos_req = -1;
	entk->def_pos = DG_ZeroVector ;
/*ＮＯDEF立ててないがいいのか？*/
}

/*守備位置確保*/
void GetDefPos( ENETHINK *entk)
{
	int i,zone_dis;
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;
//	CONTROL		ctrl ;
	AT_THK *at_thk ;

	at_thk = (AT_THK *) entk->character ;
	eve_a = ( ENTK_TYPE_A * ) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;

	ASSERT(entk->at_com != NULL) ;

#if 0
	/*現在地から廊下終端までのゾーン距離*/
	zone_dis = ENE_GetRouteDis(&entk->ctrl->mov,
		&def_com->final_pos,entk->ctrl->addr,
		def_com->final_addr,9000000 );
#else
	/*プレイヤから廊下終端までのゾーン距離*/
	zone_dis = (def_com->pl_fi_dis - 4000) ;
#endif

	for(i=0;i<EVENT_DEF_POINT_MAX;i++){
		/*守備目標が*/
		if((def_com->def_zonedis[i]< zone_dis )
		&&( def_com->def_use[i] < 0  )
		){
			entk->def_pos = def_com->def_pos[i] ;
			entk->def_mapbit = def_com->def_mapbit[i] ;
			def_com->def_use[i] = entk->id ;
			eve_a->def_pos_num = i; /*守備位置番号*/
			eve_a->def_pos_req = i;
			break;
		}
	}
	if(i >= def_com->def_pos_num ){
	/*ポイント無し*/
	/*追跡モード*/
		eve_a->status |= EVE_A_NO_DEF ;
		return ;
	}
}
/*左舷守備位置確保*/
void GetDefPos2( ENETHINK *entk){
	int i ;
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;
//	CONTROL		ctrl ;

	eve_a = ( ENTK_TYPE_A * ) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;

	/*ID順で位置割り当て*/
	i = entk->id ;
	entk->def_pos = def_com->def_pos[i] ;
	entk->def_mapbit = def_com->def_mapbit[i] ;
	def_com->def_use[i] = entk->id ;
	eve_a->def_pos_num = i; /*守備位置番号*/
	eve_a->def_pos_req = i;
}
/*リクエスト守備位置に更新*/
void GetDefPos3( ENETHINK *entk){
	int i ;
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;
//	CONTROL		ctrl ;


	eve_a = ( ENTK_TYPE_A * ) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;


//	if(eve_a->def_pos_req == eve_a->def_pos_num) return ;
	i = eve_a->def_pos_req ;
	entk->def_pos = def_com->def_pos[i] ;
	entk->def_mapbit = def_com->def_mapbit[i] ;
	def_com->def_use[i] = entk->id ;
	eve_a->def_pos_num = i; /*守備位置番号*/
	eve_a->def_pos_req = i;
//	eve_a->def_type = i ;
}

/*左舷用守備位置*/
#if 1
void GetDefPos4( ENETHINK *entk){
	GetDefPos( entk ) ;
}
#else
void GetDefPos4( ENETHINK *entk){
	int i,zone_dis;
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;
	CONTROL		ctrl ;

	eve_a = ( ENTK_TYPE_A * ) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;

//	if(eve_a->def_pos_req == eve_a->def_pos_num) return ;
	i = def_com->use_num ;
	if(i> def_com->def_pos_num) {
//		eve_a->status |= EVE_A_NO_DEF ;
//		return ;
//		i = def_com->def_pos_num - 1 ;
		def_com->use_num -= 4 ;
		i = def_com->use_num ;
	}
	entk->def_pos = def_com->def_pos[i] ;
	entk->def_mapbit = def_com->def_mapbit[i] ;
	def_com->def_use[i] = entk->id ;
	eve_a->def_pos_num = i; /*守備位置番号*/
	eve_a->def_pos_req = i;
	def_com->use_num++ ;
}
#endif

/*自分の守備位置が適切かチェック*/
void CheckDefPos( ENETHINK *entk)
{
	int i,zone_dis;
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;
//	CONTROL		ctrl ;
	AT_THK *at_thk ;

	at_thk = (AT_THK *) entk->character ;
	eve_a = ( ENTK_TYPE_A * ) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;
	zone_dis = def_com->pl_fi_dis - 3000 ;


	if(eve_a->another_flag & ANOTHER_DARK_CAMP ){
		return  ;
	}

	DefPosFree(entk);
	for(i=0;i<EVENT_DEF_POINT_MAX;i++){
		/*守備目標が*/
		if((def_com->def_zonedis[i]< zone_dis )
		&&( def_com->def_use[i] < 0  )
		){
			entk->def_pos = def_com->def_pos[i] ;
			entk->def_mapbit = def_com->def_mapbit[i] ;
			def_com->def_use[i] = entk->id ;
//			eve_a->def_pos_num = i; /*守備位置番号*/
			eve_a->def_pos_req = i;
			break;
		}
	}
	if(i >= def_com->def_pos_num ){
	/*ポイント無し*/
	/*追跡モード*/
		eve_a->status |= EVE_A_NO_DEF ;
		return ;
	}
}
/*適当にランダムっぽく選ぶ DARK FLAG兵 専用*/
void CheckDefPos2( ENETHINK *entk)
{
	int i,zone_dis;
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;
//	CONTROL		ctrl ;
	AT_THK *at_thk ;

	at_thk = (AT_THK *) entk->character ;
	eve_a = ( ENTK_TYPE_A * ) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;

//	DefPosFree(entk);

	/*検索順を入れ替える*/
	if(eve_a->shuffle_time & 1 ){
		for(i=0;i<EVENT_DEF_POINT_MAX;i++){
			/*守備目標があいてれば*/
			if(( def_com->def_use[i] < 0  )
			&&(abs(def_com->def_zonedis[i] - def_com->def_zonedis[eve_a->def_pos_num] ) < 7500)
			){
				DefPosFree(entk);
				entk->def_pos = def_com->def_pos[i] ;
				entk->def_mapbit = def_com->def_mapbit[i] ;
				def_com->def_use[i] = entk->id ;
				eve_a->def_pos_req = i;
				break;
			}
		}
	}else {
		for(i=(EVENT_DEF_POINT_MAX-1);i>=0;i--){
			/*守備目標があいてれば*/
			if(( def_com->def_use[i] < 0  )
			&&(abs(def_com->def_zonedis[i] - def_com->def_zonedis[eve_a->def_pos_num] ) < 5000)
			){
				DefPosFree(entk);
				entk->def_pos = def_com->def_pos[i] ;
				entk->def_mapbit = def_com->def_mapbit[i] ;
				def_com->def_use[i] = entk->id ;
				eve_a->def_pos_req = i;
				break;
			}
		}
	}
#if 0
	if(i >= def_com->def_pos_num ){
		/*ポイント無し*/
	}
#endif
}


/*守備モード開始*/
void Sig_DefenseModeStart( ENETHINK *entk)
{
//	int zone_dis;
	ENTK_TYPE_A *eve_a ;
	DEF_COM		*def_com ;
	CONTROL		ctrl ;
	AT_THK *at_thk ;

	at_thk = (AT_THK *) entk->character ;
	eve_a = ( ENTK_TYPE_A * ) entk->eve_a ;
	def_com = (DEF_COM * ) eve_a->def_com ;

	entk->alert = ALERT_LEVEL_MAX ;
	entk->act->CheckPad = DefCheckPad ;
	entk->act->keep_pad = -1 ;
	entk->act->pad = 0 ;
	entk->act->dir = -1 ;


	entk->think1 = ENE_TH1_DEFENSE ; 
	entk->think2 = TH2_MOVE ; 
	entk->think3 = TH3_ZONE_CHASE ; 
	entk->count3 = 0 ;

	//暗視ゴーグル
	if(
	(entk->act->sw->n_sight == 1)
	){
		/* 暗視ゴーグルON */
		entk->think3 = TH3_NSIGHT_ON ;
	}

	entk->act->move_s = MoveAttackRun ;

	at_thk->safeaddr = HZX_NO_ZONE;

	ctrl.mov = entk->def_pos ;
	GM_ConfigControlMapID( &ctrl ) ;
	entk->trgpoint.pos = entk->def_pos ;
	entk->trgpoint.map = ctrl.map ;
	entk->trgpoint.addr = ctrl.addr ;
	at_thk->safeaddr = entk->at_com->usezones[entk->u_id][entk->id] 
		= HZX_NO_ZONE;
	at_thk->safetype = 0;
	at_thk->sight_time =0; 
#if 1
	/*ダメージ時に戻らない用にstart処理から除外*/
	eve_a->seq_index = 0;	/** 守備シーケンス用 **/
#endif
	eve_a->seq_time = 0;	/** 守備シーケンス用 **/
//	eve_a->det_time = 0 ; /** 援護呼びまでのカウンタ **/
	at_thk->at_tmptime = 0;
	ctrl.mov = entk->def_pos ;
	GM_ConfigControlMapID( &ctrl ) ;
	entk->trgpoint.pos = entk->def_pos ;
	entk->trgpoint.map = ctrl.map ;
	entk->trgpoint.addr = ctrl.addr ;

	eve_a->shot_delay = EVE_A_SHOT_DELAY;
	eve_a->shot_time = EVE_A_SHOT_TIME;
//	eve_a->wait_time = 60;
	eve_a->wait_time = EVE_A_WAIT_TIME ;
//	eve_a->level = 0 ;

}

void Sig_DefenseModeStartDam( ENETHINK *entk ) {
	ENTK_TYPE_A *eve_a ;
	eve_a = ( ENTK_TYPE_A * ) entk->eve_a ;
	eve_a->status |= EVE_A_ATTAK_END ;
	if(eve_a->type >= 2){
		eve_a->def_seq = SQUAT_WAIT ;
printf("eve_a->def_seq %d\n",eve_a->def_seq);
	}
}


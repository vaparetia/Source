//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vmp_action.c
	ヴァンプさんモーション関係
	2001/03/12

	$Id: vmp_action.c,v 1.1.1.3 2002/11/19 11:48:56 Yoshizawa1 Exp $
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"camera.h"

#include	"../../shibata/t_headmark/headmark.h"
#include	"../../kano/attachment/attachment_called.h"
#include	"../../takabe/other/puppetik.h"
#include	"../../kira/face/face.h"
#include	"../util/ts_util.h"

#include	"vamp.h"
#include	"vmp_inline.h"

#define		SLASH_WAIT	(120)

//3が最小
static int GameDiff_SlashWait[2][5] = {
	{
		/*DIRECT_TICK*/(160),         //BP JG - DIRECT_TICK is now a function, so it's the DIRECT_TICK part is moved where referenced.
		/*DIRECT_TICK*/(140),
		/*DIRECT_TICK*/(SLASH_WAIT),
		/*DIRECT_TICK*/(60),
		/*DIRECT_TICK*/(60),
	},
	{
		/*DIRECT_TICK*/(60),
		/*DIRECT_TICK*/(40),
		/*DIRECT_TICK*/(20),
		/*DIRECT_TICK*/(13),
		/*DIRECT_TICK*/(13),
	},
};

//
static FVECTOR	VMP_ZeroVector = { 0.0f, 0.0f, 0.0f, 0.0f };
//プロトタイプ君
static void VMP_ActStandStill( Work *work, int time );
static void VMP_ActMoveNonRun( Work *work, int time );
static void VMP_ActDamege( Work *work, int time );
static void VMP_ActKnifeThrowSingleLeft( Work *work, int time );
static void VMP_ActStandLockOn( Work *work, int time );
static void VMP_ActMoveKnifeThrowSingleLeft( Work *work, int time );
static void VMP_ActSlice( Work *work, int time );
static void VMP_ActDive( Work *work, int time );
static void VMP_ActJumpOut( Work *work, int time );
static void VMP_ActSwim( Work *work, int time );

//static void VMP_ActWaitMesg( Work *work, int time );

static void VMP_ActWalk( Work *work, int time );
static void VMP_ActAvoid( Work *work, int time );
static void VMP_ActKnifeThrow3Left( Work *work, int time );
static void VMP_ActJumpOutLoft( Work *work, int time );
static void VMP_ActLoftMove( Work *work, int time );
static void VMP_ActKnifeSlash( Work *work, int time );
static void VMP_ActDiveLoftWater( Work *work, int time );
static void VMP_ActJumpOutWater( Work *work, int time );
static void VMP_ActKnifeThrowMany( Work *work, int time );

//static void VMP_ActKnifeThrowTest( Work *work, int time );

static void VMP_ActFloorJump( Work *work, int time );
static void VMP_ActKnifeThrowSingleLeftShadow( Work *work, int time );
static void VMP_ActAvoidFull( Work *work, int time );
static void VMP_ActOneSpin( Work *work, int time );
static void VMP_ActKnifeSlashShdw( Work *work, int time );

static void VMP_ActFloorLoftJump( Work *work, int time );
static void VMP_ActLoftFloorJump( Work *work, int time );
static void VMP_ActLoftLoftJump( Work *work, int time );
static void VMP_ActOneTime( Work *work, int time );

static void VMP_ActLongRun( Work *work, int time );
static void VMP_ActNearAttJump( Work *work, int time );
static void VMP_ActKnifeSlashNearAtt( Work *work, int time );
static void VMP_ActSpecial( Work *work, int time );
static void VMP_ActLoftTurnEnd( Work *work, int time );

static void VMP_ActWaterOutWater( Work *work, int time );
static void VMP_ActSpider( Work *work, int time );
static void VMP_ActToSpiderFromWater( Work *work, int time );
static void VMP_ActToWaterFromSpiderAtt( Work *work, int time );
//static void VMP_ActSpiderDamege( Work *work, int time );

static void VMP_ActJumpOutHndRail( Work *work, int time );
static void VMP_ActStandTurn( Work *work, int time );

static void VMP_ActDeadEnd( Work *work, int time );

static void VMP_ActProvocation( Work *work, int time );

static void VMP_ActCommonJump( Work *work, int time );
static void VMP_ActCommonLanding( Work *work, int time );
static void VMP_ActGroundAtt( Work *work, int time );

//新型回転モーション
static void VMP_ActSpin0Loop( Work *work, int time );
static void VMP_ActSpin1Loop( Work *work, int time );
static void VMP_ActSpinEnd( Work *work, int time );
static void VMP_ActSpinStart( Work *work, int time );


static void VMP_ActRapidAvoid_1st( Work *work, int time );
static void VMP_ActRapidAvoid_2nd( Work *work, int time );
static void VMP_ActRapidAvoid_3rd( Work *work, int time );
static void VMP_ActRapidAvoid_4th( Work *work, int time );
static void VMP_ActRapidAvoid_loop( Work *work, int time );
static void VMP_ActLowBow( Work *work, int time );
static void VMP_ActLowBow_CanBreak( Work *work, int time );
static void VMP_ActMutekiStand( Work *work, int time );
static void VMP_ActMutekiStandNoResetH( Work *work, int time );

static void VMP_ActOnWaterInWater( Work *work, int time );

static void VMP_ActKnifeThrow_Muteki( Work *work, int time );

static void VMP_ActJumpFlr2Flr( Work *work, int time );
static void VMP_ActJumpFlr2Wll( Work *work, int time );


static void VMP_ActJumpFlr2Wtr_Blow( Work *work, int time );

static void VMP_ActSpiderDamege( Work *work, int time );
static void VMP_ActSpiderDown( Work *work, int time );
static void VMP_ActSpiderEscape( Work *work, int time );

static void VMP_ActJumpLoft2Loft( Work *work, int time );
static void VMP_ActLoftDizzy( Work *work, int time );

static void VMP_ActLastDamBlastLoft( Work *work, int time );
static void VMP_ActLastDamLoft( Work *work, int time );
static void VMP_ActLastDamFloor( Work *work, int time );

extern void *NewBlood( FMATRIX *, FVECTOR *, FVECTOR *, int, int );
extern void *NewBlood2( FMATRIX *, FVECTOR *, FVECTOR *, int );

extern void *NewVampHola( DG_OBJS *objs );


//オーラエフェクトにシグナル送る
static inline void VMP_KillHola( Work *work )
{
	GV_CallChildSignalFunc( work, 0, 120 );
	work->holawork = NULL;
}

//ジャンプモーションと着地モーションセット
static inline void VMP_SetActCallJumpMotion( Work *work, int jump_num, int land_num, void *end_act, float height, short inv_timer )
{
	work->next_mot = jump_num;
	work->land_mot = land_num;
	work->Next_ActFunc_call = end_act;
	work->reset_height = height;
	work->inv_timer = inv_timer;
	VMP_SetActCall( work, VMP_ActCommonJump );
	
}

//ジャンプモーションと着地モーションセット
static inline void VMP_SetActSpinMotion( Work *work, int loop_num )
{
	work->next_mot = loop_num;
	VMP_SetActCall( work, VMP_ActSpinStart );	
}


static inline void VMP_SeCall( int code, FVECTOR *pos, int mode )
{
	extern void	GM_SeSetEx( int, FVECTOR*, int, int );
	GM_SeSetEx( code, pos, 50000, 51000 );
}


static inline void VMP_JimakuSeCall( int code, FVECTOR *pos, int mode )
{
	extern void	GM_SeSetEx( int, FVECTOR*, int, int );
	GM_JimakuSeSetEx( code, pos, 50000, 51000 );
}


static inline void VMP_VoiceSeCall( int code, FVECTOR *pos, int mode, int str_timer )
{
	extern void	GM_SeSetEx( int, FVECTOR*, int, int );
	if( str_timer >= 0 ) return;
	GM_SeSetEx( code, pos, 50000, 51000 );
}

static inline void VMP_VoiceJimakuSeCall( int code, FVECTOR *pos, int mode, int str_timer )
{
	extern void	GM_SeSetEx( int, FVECTOR*, int, int );
	if( str_timer >= 0 ) return;
	GM_JimakuSeSetEx( code, pos, 50000, 51000 );
}

static void VMP_DamegeSeCall( FVECTOR *pos, int str_timer, int str_handler )
{
/*	
			SD_V_VMPDMH01, //ヴァンプ強ダメージ１「ウアッ」
			SD_V_VMPDMH02, //ヴァンプ強ダメージ２「アァア」
			SD_V_VMPDMH03, //ヴァンプ強ダメージ３「ウァア」
*/
	int	se_table[] = {
		SD_V_VMPDMG01, //ヴァンプダメージ１「ウッ！」
		SD_V_VMPDMG02, //ヴァンプダメージ２「イッ！」
		SD_V_VMPDMG03, //ヴァンプダメージ３「オォッ！」
		SD_V_VMPDMG04, //ヴァンプダメージ４「カァッ！」
	};
	if( str_timer >= 0 ) GM_StreamStop( str_handler );
	VMP_SeCall( se_table[irnd()%4], pos, GM_SEMODE_BOMB );
	VmpShdwClear();
}

static void VMP_DamegeMegaSeCall( FVECTOR *pos, int str_timer, int str_handler )
{
	int	se_table[] = {
		//SD_V_VMPDMH01, //ヴァンプ強ダメージ１「ウアッ」
		SD_V_VMPDMH02, //ヴァンプ強ダメージ２「アァア」
		SD_V_VMPDMH03, //ヴァンプ強ダメージ３「ウァア」
	};
	if( str_timer >= 0 ) GM_StreamStop( str_handler );
	VMP_SeCall( se_table[irnd()%2], pos, GM_SEMODE_BOMB );
	VmpShdwClear();
}

static void VMP_DamegeLastSeCall( FVECTOR *pos, int str_timer, int str_handler )
{
	//SD_V_VMPDMH01, //ヴァンプ強ダメージ１「ウアッ」
	if( str_timer >= 0 ) GM_StreamStop( str_handler );
	VMP_SeCall( SD_V_VMPDMH01, pos, GM_SEMODE_BOMB );
	VmpShdwClear();
}

static void VMP_DamegeStunSeCall( FVECTOR *pos, int str_timer, int str_handler )
{
	int	se_table[] = {
		SD_V_VMPSTN01, //ヴァンプスタン被爆１「目が！」
		SD_V_VMPSTN02, //ヴァンプスタン被爆２「見えん」
	};
	if( str_timer >= 0 ) GM_StreamStop( str_handler );
	//VMP_SeCall( se_table[irnd()%2], pos, GM_SEMODE_BOMB );
	VMP_JimakuSeCall( se_table[irnd()%2], pos, GM_SEMODE_BOMB );
	VmpShdwClear();
}


void ThrowKnife( Work *work, FVECTOR *pos, FVECTOR *hit, FVECTOR *shift, int index, int mode, int shdw )
{
	FVECTOR		to,from;
	
	//VMP_PRINTF("ナイフ投げ\n");
	if( mode ){
		//ニキータ、スティンガー優先
	}else if( !hit && GM_CheckPlayerStatus( PLAYER_WATCH )){
		//主観だったらカメラ狙い
		DG_COPY_VEC( &to, &GM_PlayerSubjectCamera[ 0 ]->position ) ;
		
		//DG_COPY_VEC( &to, (FVECTOR*)(DG_Chanls[0].eye.m[3]) );
	}else{
		if( !hit ){
			DG_COPY_VEC( &to, (FVECTOR*)(BODYWORLD(GM_PlayerBody,12).m[3]) );
		}else{
			DG_COPY_VEC( &to, hit);
		}
	}
	if( !pos ){
		DG_COPY_VEC( &from, (FVECTOR*)(BODYWORLD(&work->body,index).m[3]) );
	}else{
		DG_COPY_VEC( &from, pos );
	}
	if( shift ){
		//_sceVu0AddVector( &from, &from, shift );
		_sceVu0AddVector( &to, &to, shift );
	}
	//SD_E_V_NNAGE1		ヴァンプナイフ投げ音
	VMP_SeCall( SD_E_V_NNAGE1, &from, GM_SEMODE_BOMB );
	
	VmpThrowKnifeSet( &from, &to, &work->control.map, shdw );
}


static inline void VMP_JumpVoice( FVECTOR *pos, int str_timer )
{
	int se_table[] = {
		SD_V_VMPKIK01, //ヴァンプキック気合１「フエッ」
		SD_V_VMPKIK02, //ヴァンプキック気合２「テエッ」
		SD_V_VMPKIK03, //ヴァンプキック気合３「イヤッ」
		SD_V_VMPKIK04, //ヴァンプキック気合４「フンッ」
	};
	VMP_VoiceSeCall( se_table[(irnd()>>6)%4], pos, GM_SEMODE_BOMB, str_timer );
}

//思考側からのアクト変更メッセージ処理
static int VmpCheckThinkMesg( Work *work )
{
	if( !work->think_mesg ) return 0;

	VMP_PRINTF("Get Mesg [%4d]\n",work->think_mesg);
	switch( work->think_mesg ){
	  case THINK_MESG_MOVE:
		VMP_SetActCall( work, VMP_ActMoveNonRun );
		break;
	  case THINK_MESG_STAND:
		VMP_SetActCall( work, VMP_ActStandStill );
		break;
	  case THINK_MESG_LOCKON:
		VMP_SetActCall( work, VMP_ActStandLockOn );
		break;
	  case THINK_MESG_MUTEKI:
		VMP_SetActCall( work, VMP_ActMutekiStand );
		break;
	  case THINK_MESG_SLICE:
		VMP_SetActCall( work, VMP_ActSlice );
		break;
	  case THINK_MESG_THOROWL:
		VMP_SetActCall( work, VMP_ActKnifeThrowSingleLeft );
		break;
	  case THINK_MESG_THRWL_MUTEKI:
		VMP_SetActCall( work, VMP_ActKnifeThrow_Muteki );
		break;
	  case THINK_MESG_THOROWL3:
		VMP_SetActCall( work, VMP_ActKnifeThrow3Left );
		break;
	  case THINK_MESG_THRWMANY:
		VMP_SetActCall( work, VMP_ActKnifeThrowMany );
		break;
	  case THINK_MESG_MOVTHRWL:
		VMP_SetActCall( work, VMP_ActMoveKnifeThrowSingleLeft );
		break;
	  case THINK_MESG_THRWSHDW:
		VMP_SetActCall( work, VMP_ActKnifeThrowSingleLeftShadow );
		break;
	  case THINK_MESG_DIVE:
		VMP_SetActCall( work, VMP_ActDive );
		break;
	  case THINK_MESG_DIVELOFT:
		VMP_SetActCall( work, VMP_ActDiveLoftWater );
		break;
	  case THINK_MESG_JUMPOUT:
		VMP_SetActCall( work, VMP_ActJumpOut );
		break;
	  case THINK_MESG_JUMPOUTLOFT:
		VMP_SetActCall( work, VMP_ActJumpOutLoft );
		break;
	  case THINK_MESG_SWIM:
		VMP_SetActCall( work, VMP_ActSwim );
		break;
	  case THINK_MESG_WALK:
		VMP_SetActCall( work, VMP_ActWalk );
		break;
	  case THINK_MESG_AVOID:
		VMP_SetActCall( work, VMP_ActAvoid );
		break;
	  case THINK_MESG_AVOIDFULL:
		VMP_SetActCall( work, VMP_ActAvoidFull );
		break;
	  case THINK_MESG_ONESPIN:
		VMP_SetActCall( work, VMP_ActOneSpin );
		break;
	  case THINK_MESG_LOFTMOVE:
		VMP_SetActCall( work, VMP_ActLoftMove );
		break;
	  case THINK_MESG_LOFTJUMP:
		VMP_SetActCall( work, VMP_ActJumpLoft2Loft );
		//VMP_SetActCall( work, VMP_ActLoftSpinJump );
		break;
	  case THINK_MESG_SLASH:
		VMP_SetActCall( work, VMP_ActKnifeSlash );
		break;
	  case THINK_MESG_SLASHSHDW:
		VMP_SetActCall( work, VMP_ActKnifeSlashShdw );
		break;
	  case THINK_MESG_JUMPOUTWATER:
		VMP_SetActCall( work, VMP_ActJumpOutWater );
		break;
	  case THINK_MESG_FLOORJUMP:
		VMP_SetActCall( work, VMP_ActFloorJump );
		break;
	  case THINK_MESG_TOLOFTJUMP:
		VMP_SetActCall( work, VMP_ActFloorLoftJump );
		break;
	  case THINK_MESG_TOFLOORJUMP:
		VMP_SetActCall( work, VMP_ActLoftFloorJump );
		break;
	  case THINK_MESG_LOFTLOFTJUMP:
		VMP_SetActCall( work, VMP_ActLoftLoftJump );
		break;
	  case THINK_MESG_ONETIME:
		VMP_SetActCall( work, VMP_ActOneTime );
		break;
	  case THINK_MESG_LONGRUN:
		VMP_SetActCall( work, VMP_ActLongRun );
		break;
	  case THINK_MESG_NEARJUMP:
		VMP_SetActCall( work, VMP_ActNearAttJump );
		break;
	  case THINK_MESG_SPECIAL:
		VMP_SetActCall( work, VMP_ActSpecial );
		break;
	  case THINK_MESG_SLASHNEAR:
		VMP_SetActCall( work, VMP_ActKnifeSlashNearAtt );
		break;
	  case THINK_MESG_TURNEND:
		VMP_SetActCall( work, VMP_ActLoftTurnEnd );
		break;
	  case THINK_MESG_DIZZY:
		VMP_SetActCall( work, VMP_ActLoftDizzy );
		break;
	  case THINK_MESG_DEADEND:
		VMP_SetActCall( work, VMP_ActDeadEnd );
		break;
	  case THINK_MESG_DAMAGE:
		VMP_SetActCall( work, VMP_ActDamege );
		break;
	  case THINK_MESG_WATEROUTWATER:
		VMP_SetActCall( work, VMP_ActWaterOutWater );
		break;
	  case THINK_MESG_SPIDER:
		VMP_SetActCall( work, VMP_ActSpider );
		break;
	  case THINK_MESG_TOSPIDERWATER:
		VMP_SetActCall( work, VMP_ActToSpiderFromWater );
		break;
	  case THINK_MESG_TOWATERSPIDER:
		VMP_SetActCall( work, VMP_ActToWaterFromSpiderAtt );	
		break;
	  case THINK_MESG_SPIDERDAM:
		VMP_SetActCall( work, VMP_ActSpiderDamege );
		break;
	  case THINK_MESG_JUMPOUTHNDRAIL:
		VMP_SetActCall( work, VMP_ActJumpOutHndRail );
		break;
	  case THINK_MESG_STANDTURN:
		VMP_SetActCall( work, VMP_ActStandTurn );
		break;
	  case THINK_MESG_PROVO:
		VMP_SetActCall( work, VMP_ActProvocation );
		break;
	  case THINK_MESG_GROUND_ATT:
		VMP_SetActCall( work, VMP_ActGroundAtt );
		break;
	//ここからジャンプ関係のメッセージ
	  case THINK_MESG_LOFT2LOFT:
		VMP_SetActCallJumpMotion( work, VMP_MOT_NON_START_LOFT,
								  VMP_MOT_NON_END_LOFT, VMP_ActMutekiStand, 1152.0f+4500.0f, 48 );
		break;
	  case THINK_MESG_LOFT2FLOOR:
		VMP_SetActCallJumpMotion( work, VMP_MOT_NON_START_LOFT,
								  VMP_MOT_NON_END_FLOOR, VMP_ActMutekiStand, 1152.0f+4500.0f, 64 );
		break;
	  case THINK_MESG_LOFT2RAIL:
		VMP_SetActCallJumpMotion( work, VMP_MOT_NON_START_LOFT,
								  VMP_MOT_NON_END_RAIL, VMP_ActMutekiStand, 1152.0f+4500.0f, 64 );
		break;
	  case THINK_MESG_LOFT2WATER:
		VMP_SetActCallJumpMotion( work, VMP_MOT_NON_START_LOFT,
								  VMP_MOT_NON_END_WATER, VMP_ActSwim, 1152.0f+4500.0f, 1000 );
		break;
	  case THINK_MESG_RAIL2RAIL:
		VMP_SetActCallJumpMotion( work, VMP_MOT_NON_START_RAIL,
								  VMP_MOT_NON_END_RAIL, VMP_ActMutekiStand, 1152.0f+1000.0f, 64 );
		break;
	  case THINK_MESG_RAIL2FLOOR:
		VMP_SetActCallJumpMotion( work, VMP_MOT_NON_START_RAIL,
								  VMP_MOT_NON_END_FLOOR, VMP_ActMutekiStand, 1152.0f+1000.0f, 64 );
		break;
	  case THINK_MESG_RAIL2WATER:
		VMP_SetActCallJumpMotion( work, VMP_MOT_NON_START_RAIL,
								  VMP_MOT_NON_END_WATER, VMP_ActSwim, 1152.0f+1000.0f, 100 );
		break;
	  case THINK_MESG_FLOOR2LOFT:
		VMP_SetActCallJumpMotion( work, VMP_MOT_NON_START_FLOOR,
								  VMP_MOT_NON_END_LOFT, VMP_ActMutekiStand, 1152.0f, 48 );
		break;
	  case THINK_MESG_FLOOR2RAIL:
		VMP_SetActCallJumpMotion( work, VMP_MOT_NON_START_FLOOR,
								  VMP_MOT_NON_END_RAIL, VMP_ActMutekiStand, 1152.0f, 64 );
		break;
	  case THINK_MESG_FLOOR2FLOOR:
		VMP_SetActCallJumpMotion( work, VMP_MOT_NON_START_FLOOR,
								  VMP_MOT_NON_END_FLOOR, VMP_ActMutekiStand, 1152.0f, 64 );
		break;
	  case THINK_MESG_WATER2LOFT:
		VMP_SetActCallJumpMotion( work, VMP_MOT_NON_START_WATER,
								  VMP_MOT_NON_END_LOFT, VMP_ActMutekiStand, -1700.0f, 48 );
		break;
	  case THINK_MESG_WATER2RAIL:
		VMP_SetActCallJumpMotion( work, VMP_MOT_NON_START_WATER,
								  VMP_MOT_NON_END_RAIL, VMP_ActMutekiStand, -1700.0f, 64 );
		break;
	  case THINK_MESG_WATER2WALL:
		VMP_SetActCallJumpMotion( work, VMP_MOT_NON_START_WATER,
								  VMP_MOT_NON_END_WALL, VMP_ActSpider, -1700.0f, 1000 );
		break;


		//新スピンモーション関係
	  case THINK_MESG_ONESPINLOOP0:
		SET_FLAG( work->act_flags, VMP_ACTFLAG_ENDSPIN);
	  case THINK_MESG_SPINLOOP0:
		VMP_SetActSpinMotion( work, VMP_MOT_NON_SPIN_LOOP0 );
		break;
	  case THINK_MESG_ONESPINLOOP1:
		SET_FLAG( work->act_flags, VMP_ACTFLAG_ENDSPIN);
	  case THINK_MESG_SPINLOOP1:
		VMP_SetActSpinMotion( work, VMP_MOT_NON_SPIN_LOOP1 );
		break;
	  case THINK_MESG_SPIN:
		VMP_SetActCall( work, VMP_ActSpinStart );
		break;

	  case THINK_MESG_LOWBOW:
		VMP_SetActCall( work, VMP_ActLowBow );
		break;
	  case THINK_MESG_LOWBOW_CANBREAK:
		VMP_SetActCall( work, VMP_ActLowBow_CanBreak );
		break;
	  case THINK_MESG_RAPIDAVOID:
		VMP_SetActCall( work, VMP_ActRapidAvoid_1st );
		break;
	  case THINK_MESG_INTOWATER:
		VMP_SetActCall( work, VMP_ActOnWaterInWater );
		break;
	  case THINK_MESG_FLR2FLR_F:
		VMP_SetActCall( work, VMP_ActJumpFlr2Flr );
		break;
	  case THINK_MESG_FLR2WLL:
		VMP_SetActCall( work, VMP_ActJumpFlr2Wll );
		break;
	  case THINK_MESG_FLR2WRT_B:
		VMP_SetActCall( work, VMP_ActJumpFlr2Wtr_Blow );
		break;
	  default:
		VMP_PRINTF( "ERR!! VMP_CheckThinkMesg[%d]\n",work->think_mesg);
	}
	work->think_mesg = THINK_MESG_NONE;

	return 1;
}

extern void *NewBloodWater( FVECTOR *pos, FVECTOR *vec, float pow, int mode );

static void VMP_SetDamegeMot( Work *work, int index, FVECTOR *force )
{
	if( index < 2 ){
		int mot_list[] = {
			VMP_MOT_KNF_DAM_HEAD,		//ダメージ頭
			VMP_MOT_KNF_DAM_FLANK,		//ダメージわき腹
		};
		work->dam_count = 0;
		work->dam_timer = 0;
		VMP_DamegeMegaSeCall( &work->control.mov, work->vox_timer, work->stream_handler );

		VMP_SetActCallMotion( work, mot_list[index], VMP_ActDamege );
	}else if( ( work->ActFunc_call == (void*)VMP_ActSpecial && work->act_timer < DIRECT_TICK(112)) ){// ||
//			  ( work->ActFunc_call == (void*)VMP_ActKnifeSlashNearAtt &&
//				work->act_timer < GameDiff_SlashWait[((work->think_now_main_w == 4/*THK_SUB_MAIN3_W*/)?1:0)][work->game_diff]) ){
		//VMP_MOT_KNF_DAM_CROUCH
		VMP_DamegeSeCall( &work->control.mov, work->vox_timer, work->stream_handler );
		VMP_SetActCallMotion( work, VMP_MOT_KNF_DAM_CROUCH, VMP_ActDamege );
	}else{
		//VMP_MOT_KNF_DAM_FRONT,		//ダメージ前から
		//VMP_MOT_KNF_DAM_BACK,			//ダメージ後ろから
		float tmp;
		VMP_DamegeSeCall( &work->control.mov, work->vox_timer, work->stream_handler );
		tmp = _sceVu0InnerProduct( (FVECTOR*)BODYWORLD(&work->body, 0).m[2], force );
		if( tmp < 0.0f )
			VMP_SetActCallMotion( work, VMP_MOT_KNF_DAM_FRONT, VMP_ActDamege );
		else
			VMP_SetActCallMotion( work, VMP_MOT_KNF_DAM_BACK, VMP_ActDamege );
	}
	SET_FLAG( work->status, VMP_STATUS_DAMEGE );
}

static int VmpCheckDamage( Work *work )
{
	int ret = 1;

	//DEBUG_Locate( 20, 40, 0 );
	//DEBUG_Printf( "v" );

/*	if( work->ActFunc_call == (void*)VMP_ActLoftTurnEnd && work->act_timer == 219 ){
		work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_M9, HUMAN21_ATAMA,
										  0, 0,
										  2 );
	}
*/
	if( work->dmg_flags == 0xffffffff ) return 0;	
	switch(VMP_GET_DMFLAG_KINDS(work->dmg_flags)){
	  case VMP_WPDM_PUNCH:			//パンチ	（親にくる）
#if 0
		//VMP_SetActCallMotion( work, VMP_MOT_KNF_GUNDAM, VMP_ActDamege );
		VMP_SetDamegeMot( work, VMP_GET_DMFLAG_INDEX( work->dmg_flags ), NULL );
		work->think_damnum++;
#else
		if( VMP_GET_DMFLAG_INDEX( work->dmg_flags ) < 2 ){
			int mot_list[] = {
				VMP_MOT_KNF_DAM_HEAD,		//ダメージ頭
				VMP_MOT_KNF_DAM_FLANK,		//ダメージわき腹
			};
			VMP_SetActCallMotion( work, mot_list[VMP_GET_DMFLAG_INDEX( work->dmg_flags )], VMP_ActDamege );
			VMP_DamegeMegaSeCall( &work->control.mov, work->vox_timer, work->stream_handler );
			SET_FLAG( work->status, VMP_STATUS_DAMEGE );
		}else{
			VMP_DamegeSeCall( &work->control.mov, work->vox_timer, work->stream_handler );
			ret = 0;
		}
		work->piku_timer = 6;

#endif
		//SD_P_GUNPNC01 小火器パンチ・ヒット
		VMP_SeCall( SD_P_GUNPNC01, &work->control.mov, GM_SEMODE_BOMB );
		VMP_PRINTF("punch!!\n");
		break;
	  case VMP_WPDM_KICK:			//キック	（親にくる）
#if 0
		work->think_damnum++;
		//VMP_SetActCallMotion( work, VMP_MOT_KNF_GUNDAM, VMP_ActDamege );
		VMP_SetDamegeMot( work, VMP_GET_DMFLAG_INDEX( work->dmg_flags ), NULL );
#else
		SET_FLAG( work->status, VMP_STATUS_BLAST );
		work->think_damnum++;
		work->piku_timer = 6;
#endif
		VMP_DamegeSeCall( &work->control.mov, work->vox_timer, work->stream_handler );
		VMP_SeCall( SD_P_GUNPNC01, &work->control.mov, GM_SEMODE_BOMB );
		VMP_PRINTF("kick!!\n");
		break;
	  case VMP_WPDM_BLAST:			//爆発		（親にくる）
		VMP_PRINTF("blast!!\n");
		SET_FLAG( work->status, VMP_STATUS_BLAST );
		VMP_DamegeSeCall( &work->control.mov, work->vox_timer, work->stream_handler );
		VMP_SeCall( SD_P_GUNPNC01, &work->control.mov, GM_SEMODE_BOMB );
		work->think_damnum++;
		work->piku_timer = 6;
		
		if( CHECK_FLAG( work->status, VMP_STATUS_SWIM )){
			work->o2 -= COUNT_VMODE( VAMP_O2 )/2;
			work->wtr_blast_count++;
		}
		break;
	  case VMP_WPDM_STUN:			//スタン	（親にくる）
		VMP_SetActCallMotion( work, VMP_MOT_KNF_DAM_FRONT, VMP_ActDamege );
		VMP_DamegeStunSeCall( &work->control.mov, work->vox_timer, work->stream_handler );
		SET_FLAG( work->status, VMP_STATUS_DAMEGE );
		VMP_PRINTF("stun!!\n");
		break;
	  case VMP_WPDM_M9:				//m9
		work->think_damnum++;
		//VMP_SetActCallMotion( work, VMP_MOT_KNF_GUNDAM, VMP_ActDamege );
		VMP_PRINTF("m92!!\n");
		//SD_A_M_NINE01 Ｍ９注射器刺さる
		//VMP_SetActCallMotion( work, VMP_MOT_KNF_GUNDAM, VMP_ActDamege );
		VMP_SetDamegeMot( work, VMP_GET_DMFLAG_INDEX( work->dmg_flags ), &work->off_force );
		VMP_SeCall( SD_A_M_NINE01, &work->control.mov, GM_SEMODE_BOMB );
		//VMP_PRINTF("vmp m9 dam[%d]\n", work->act_timer );
		//ret = 0;
		break;
	  case VMP_WPDM_PSG1:			//ライフル
		work->think_damnum++;
		//VMP_SetActCallMotion( work, VMP_MOT_KNF_GUNDAM, VMP_ActDamege );
		VMP_SetDamegeMot( work, VMP_GET_DMFLAG_INDEX( work->dmg_flags ), &work->off_force );

		//NewBlood( &BODYWORLD(&work->body,VMP_GET_DMFLAG_JOINT(work->dmg_flags)),
		//		  &work->off_hit, &work->off_force, 1, 0 );		
		
		//NewBlood_M9( &BODYWORLD(&work->body,VMP_GET_DMFLAG_JOINT(work->dmg_flags)), &work->off_hit );
		
		VMP_PRINTF("psg1!!\n");
		break;
	  case VMP_WPDM_BULLET:			//弾丸
		work->think_damnum++;
		//VMP_SetActCallMotion( work, VMP_MOT_KNF_GUNDAM, VMP_ActDamege );
		VMP_SetDamegeMot( work, VMP_GET_DMFLAG_INDEX( work->dmg_flags ), &work->off_force );

		
		//NewBlood_M9( &BODYWORLD(&work->body,VMP_GET_DMFLAG_JOINT(work->dmg_flags)), &work->off_hit );
		
		//NewBlood( &BODYWORLD(&work->body,VMP_GET_DMFLAG_JOINT(work->dmg_flags)),
		//		  &work->off_hit, &work->off_force, 1, 0 );
		//NewBlood2( &BODYWORLD(&work->body,VMP_GET_DMFLAG_JOINT(work->dmg_flags)),
		//			&work->off_hit, &work->off_force, 0 );
		VMP_PRINTF("bullet!!\n");
		//SD_P_GUNPNC01 小火器パンチ・ヒット
		VMP_SeCall( SD_P_GUNPNC01, &work->control.mov, GM_SEMODE_BOMB );
		break;
	  case VMP_WPDM_BLADE:			//刀
		VMP_PRINTF("blade!!\n");
		break;
	  case VMP_WPDM_SPRAY:			//スプレー
		VMP_PRINTF("spray!!\n");
		ret = 0;
		break;
	  case VMP_WPDM_AVOID1:
		//VMP_SetActCall( work, VMP_ActOneSpin );
		VMP_SetActCall( work, VMP_ActRapidAvoid_1st );
		if( work->life > 64 && work->faint > 64 && !CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) &&
			work->last_call != work->think_sub ){
			int se_table[] = {
			//SD_V_VMPESB01, //ヴァンプバレエ嘲笑「ハハハッ」
				SD_V_VMPESB02, //ヴァンプバレエ避１「無駄だ！」
				SD_V_VMPESB03, //ヴァンプバレエ避２「読めるぞ」
				SD_V_VMPESR01, //ヴァンプ水面避け１「遅い」
				SD_V_VMPESR02, //ヴァンプ水面避け２「無駄だ」
			};
			static int se_debug = 0;
			work->last_call = work->think_sub;
			VMP_VoiceJimakuSeCall( se_table[se_debug%4], &work->control.mov, GM_SEMODE_BOMB, work->vox_timer );
			se_debug++;
		}
		VMP_PRINTF("start avoid !!\n");
		break;
	  case VMP_WPDM_AVOID2:
		VMP_SetActCall( work, VMP_ActOneSpin );
		VMP_PRINTF("start avoid2 !!\n");
		break;
	  case VMP_WPDM_RPD_AVD:
		SET_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID );
		VMP_PRINTF( "next avoid !!\n" );
		ret = 0;
		break;
	  case VMP_WPDM_SPDR_DAM:
		VMP_PRINTF("spider damege !!\n");
		work->think_damnum++;
		if( VMP_GET_DMFLAG_DAMGE(work->dmg_flags) || VMP_GET_DMFLAG_FAINT(work->dmg_flags) ){
			VMP_SetActCall( work, VMP_ActSpiderDamege );
			if( VMP_GET_DMFLAG_DAMGE(work->dmg_flags) ){
				
				//NewBlood_M9( &BODYWORLD(&work->body,VMP_GET_DMFLAG_JOINT(work->dmg_flags)), &work->off_hit );

				//NewBlood( &BODYWORLD(&work->body,VMP_GET_DMFLAG_JOINT(work->dmg_flags)),
				//			&work->off_hit, &work->off_force, 1, 0 );
			}
		}else{
			VMP_SetActCall( work, VMP_ActSpiderEscape );
		}
		break;
	  case VMP_WPDM_SWIM_DAM:
		work->piku_timer = 6;
		{
			//FVECTOR vec;
			VMP_DamegeSeCall( &work->control.mov, work->vox_timer, work->stream_handler );
			if( VMP_GET_DMFLAG_JOINT(work->dmg_flags) != 31 ){
#ifdef BP_PS2
				NewBloodWater( (FVECTOR*)(BODYWORLD(&work->body,VMP_GET_DMFLAG_JOINT(work->dmg_flags)).m[3]),
							   &(FVECTOR){ 0.0f, 1.0f, 0.0f, 0.0f }, 50.0f, 3 );
				NewBloodWater( (FVECTOR*)(BODYWORLD(&work->body,VMP_GET_DMFLAG_JOINT(work->dmg_flags)).m[3]),
							   &(FVECTOR){ 0.0f, 1.0f, 0.0f, 0.0f }, 50.0f, 3 );
#else
				{
				FVECTOR dmdm;
				NewBloodWater( (FVECTOR*)(BODYWORLD(&work->body,VMP_GET_DMFLAG_JOINT(work->dmg_flags)).m[3]),
							   &dmdm, 50.0f, 3 );
				NewBloodWater( (FVECTOR*)(BODYWORLD(&work->body,VMP_GET_DMFLAG_JOINT(work->dmg_flags)).m[3]),
							   &dmdm, 50.0f, 3 );				
				}
#endif
			}
			work->o2 -= 8;
		}
		VMP_PRINTF("swim damege !!\n");
		work->think_damnum++;
		break;
	  default:
	
		VMP_PRINTF("ERR!!VmpDmCheck!![%d]\n",VMP_GET_DMFLAG_KINDS(work->dmg_flags));
		break;
	}

	//if( work->think_damnum == 2 ) work->think_ntbl = 0;

	VMP_PRINTF("dam id [%d] joint[%d] damege[%d] faint[%d] st[%08x]\n",
			   VMP_GET_DMFLAG_KINDS(work->dmg_flags),
			   VMP_GET_DMFLAG_JOINT(work->dmg_flags),
			   VMP_GET_DMFLAG_DAMGE(work->dmg_flags),
			   VMP_GET_DMFLAG_FAINT(work->dmg_flags),
			   work->status );
	if ( !GM_IsGameOver() ) {
		// ゲームオーバー処理に入っているか
		if( CHECK_FLAG( work->status, VMP_STATUS_DAM_MUL ) ){
			VMP_PRINTF( "counter damega!!\n" );
			work->life  -= (int)((float)VMP_GET_DMFLAG_DAMGE(work->dmg_flags)*1.5f);
			work->faint -= (int)((float)VMP_GET_DMFLAG_FAINT(work->dmg_flags)*1.5f);
		}else{
			work->life  -= VMP_GET_DMFLAG_DAMGE(work->dmg_flags);
			work->faint -= VMP_GET_DMFLAG_FAINT(work->dmg_flags);
		}
	}
	work->think_wait_time = 60 - (work->think_damnum/2)*6;
	if( work->think_wait_time < 30 ) work->think_wait_time = 30;

	if( work->o2 < 0 ){
		work->o2 = 0;
	}
	if(work->life <= 0 &&
	   CHECK_FLAG( work->status, VMP_STATUS_SWIM|VMP_STATUS_SPIDER ) ){
		work->life = 1;
	}else if(work->life <= 0){

		VMP_LifeCheck |= 1 ;

		GM_SetPlayerStatus( PLAYER_INVINCIBLE_PRG ) ;
		work->life = 0;
		//if ( GM_KillCount < 30000 ) ++ GM_KillCount ;
		if( !CHECK_FLAG( work->status, VMP_STATUS_DEATH ) ){
			
			if( ++GM_KillCount > GM_MAX_RESULT_COUNT ) GM_KillCount = GM_MAX_RESULT_COUNT;
			
			VMP_DamegeLastSeCall( &work->control.mov, work->vox_timer, work->stream_handler );
			SET_FLAG( work->status, VMP_STATUS_DEATH );
			if( VMP_GET_DMFLAG_KINDS(work->dmg_flags) == VMP_WPDM_BLAST ){
				//爆発系
				//if( CHECK_FLAG( work->think_flags, VMP_FLAGS_LOFT ) ){
					//２階
					VMP_SetActCall( work, VMP_ActLastDamBlastLoft );
					VMP_SetThinkSub( work, 43/* THK_SUB_LAST_BLASTDAM_LFT */, 0 );
				//}else{
				//}
			}else{
				//その他
				if( CHECK_FLAG( work->think_flags, VMP_FLAGS_LOFT ) ){
					//２階
					VMP_SetActCall( work, VMP_ActLastDamLoft );
					VMP_SetThinkSub( work, 41/* THK_SUB_LAST_DAM_LFT */, 0 );
				}else{
					VMP_SetActCall( work, VMP_ActLastDamFloor );
					VMP_SetThinkSub( work, 40/* THK_SUB_LAST_DAM_FLR */, 0 );
				}
			}
		}
		return 1;
		//if( work->think_sub != 66 )VMP_SetThinkSub( work, 66, 0 );
	}
	
	if(work->faint <= 0 &&
	   CHECK_FLAG( work->status, VMP_STATUS_SWIM|VMP_STATUS_SPIDER ) ){
		work->faint = 1;
	}else if(work->faint <= 0){
		GM_SetPlayerStatus( PLAYER_INVINCIBLE_PRG ) ;
		work->faint = 0;
		if( !CHECK_FLAG( work->status, VMP_STATUS_DEATH ) ){
			VMP_DamegeLastSeCall( &work->control.mov, work->vox_timer, work->stream_handler );
			SET_FLAG( work->status, VMP_STATUS_DEATH );
			//その他
			
			if( VMP_GET_DMFLAG_KINDS(work->dmg_flags) == VMP_WPDM_KICK ){
				VMP_SetActCall( work, VMP_ActLastDamBlastLoft );
				VMP_SetThinkSub( work, 43/* THK_SUB_LAST_BLASTDAM_LFT */, 0 );
			}else{
				if( CHECK_FLAG( work->think_flags, VMP_FLAGS_LOFT ) ){
					//２階
					VMP_SetActCall( work, VMP_ActLastDamLoft );
					VMP_SetThinkSub( work, 41/* THK_SUB_LAST_DAM_LFT */, 0 );
				}else{
					VMP_SetActCall( work, VMP_ActLastDamFloor );
					VMP_SetThinkSub( work, 40/* THK_SUB_LAST_DAM_FLR */, 0 );
				}
			}
		}
		return 1;
		//if( work->think_sub != 66 )VMP_SetThinkSub( work, 66, 0 );
	}
	return ret;
}

//ラストダメージ：爆発（途中まで）
static void VMP_ActLastDamBlastLoft( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_MUL_STEP|VMP_STATUS_FLY|VMP_STATUS_MOTAVOID|VMP_STATUS_DEATH );

	if( time == 0 ) {
		GM_ControlResetHeightMotion( ctrl );
		VMP_SetActMotion( work, VMP_MOT_KNF_DAM_BLOW );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		ctrl->turn.vy = work->dir;
		work->target.class |= TARGET_SKIP;
		VMP_SetThinkMulStep( work, work->think_mul_x, work->think_mul_y, work->think_mul_z );
		ctrl->interp = 1;
		VMP_PRINTF("act call\n");
	}
	if( time == DIRECT_TICK(30) ){
	//if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		body->m_ctrl->mt3_ctrl->flag |= MT3_SLEEP;
		//VMP_SetActCall( work, VMP_ActMutekiStandNoResetH );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
	}
}
/*
//ラストダメージ：爆発（途中まで）
static void VMP_ActLastDamBlastFloor( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_MUL_STEP|VMP_STATUS_FLY|VMP_STATUS_MOTAVOID );

	if( time == 0 ) {
		GM_ControlResetHeightMotion( ctrl );
		VMP_SetActMotion( work, VMP_MOT_KNF_DAM_BLOW );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		ctrl->turn.vy = work->dir;
		//ここでけいさんする？
		VMP_SetThinkMulStep( work, work->think_mul_x, work->think_mul_y, work->think_mul_z );
		ctrl->interp = 1;
	}

	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		VMP_SetActCall( work, VMP_ActMutekiStandNoResetH );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
	}
}
*/


//ラストダメージ：２階
static void VMP_ActLastDamLoft( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_FLY|VMP_STATUS_MOTAVOID|VMP_STATUS_DEATH );

	if( time == 0 ) {
		work->reset_height = 1152.0f+4500.0f;
		SET_FLAG( work->status, VMP_STATUS_RESET_H );
		//GM_ControlResetHeightMotion( ctrl );
		VMP_SetActMotion( work, VMP_MOT_KNF_DAM_LAST_LFT );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		ctrl->turn.vy = work->dir;
		work->target.class |= TARGET_SKIP;
		//VMP_SetThinkMulStep( work, work->think_mul_x, work->think_mul_y, work->think_mul_z );
		//VMP_SetThinkMulStep( work, 1.0f, 1.0f, 1.0f );
		ctrl->interp = 1;
		VMP_PRINTF("act call\n");
	}
	//if( time == 60 ){
	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		body->m_ctrl->mt3_ctrl->flag |= MT3_SLEEP;
		//VMP_SetActCall( work, VMP_ActMutekiStandNoResetH );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
	}
}

//ラストダメージ：１階
static void VMP_ActLastDamFloor( Work *work, int time )
{	
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_FLY|VMP_STATUS_MOTAVOID|VMP_STATUS_DEATH|VMP_STATUS_MUL_STEP );

	if( time == 0 ) {
		work->reset_height = 1152.0f;
		SET_FLAG( work->status, VMP_STATUS_RESET_H );
		//GM_ControlResetHeightMotion( ctrl );
		VMP_SetActMotion( work, VMP_MOT_KNF_DAM_LAST_FLR );
		//ctrl->skip_flag |= CTRL_SKIP_HZX;
		ctrl->turn.vy = work->dir;
		work->target.class |= TARGET_SKIP;
		VMP_SetThinkMulStep( work, 1.0f, 1.0f, 1.0f );
		ctrl->interp = 1;
		VMP_PRINTF("act call\n");
	}else if( time == DIRECT_TICK(94) ){
		FVECTOR		fvtemp;
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		VMP_DivePos( &work->think_init_pos, 19 );
		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );

		work->think_mul_x = work->think_mul_z =
			bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz)/(2131.21166f - -1302.13122f);  //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;
		VMP_SetThinkMulStep( work, work->think_mul_x, work->think_mul_y, work->think_mul_z );
	}/*else if( time > 94 ){
	}*/
	//if( time == 60 ){
	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		body->m_ctrl->mt3_ctrl->flag |= MT3_SLEEP;
		//VMP_SetActCall( work, VMP_ActMutekiStandNoResetH );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
	}
}

//水上から水中へ
static void VMP_ActOnWaterInWater( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_MUL_STEP|VMP_STATUS_MOTAVOID );
	if ( time == 0 ) {
		//ctrl->turn.vy = ctrl->rot.vy = work->dir;
		//GM_ControlResetHeightMotion( ctrl ) ;
		//VMP_SetActMotionEx( work, VMP_MOT_KNF_WATER2WATER, 0, 0x1fffff, 19 );
		VMP_SetActMotionCheck( work, VMP_MOT_KNF_INWATER );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 0.5f );
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		//ctrl->turn.vy = ctrl->rot.vy = work->dir;
		VMP_SetThinkMulStep( work, 1.0f, 1.0f, 1.0f );
		if( !work->holawork ){
			work->holawork = (void*)NewVampHola( work->body.objs );
			GV_SetActorChild( work, work->holawork );
		}
	}
	if( time == DIRECT_TICK(7) ){
		VMP_KnfObjMesg( PARTS_KNF_NAME, 0, 1 );
		VMP_KnfObjMesg( ALL_KNF_NAME, 0, 0 );
	}else if( time == DIRECT_TICK(8) ){
		DG_VisibleObjs( work->knf_objs );
		GV_DestroyOtherActor( work->all_knf_work );
	}else if( time > DIRECT_TICK(8) ){
		work->knf_alpha += 4;
		if( work->knf_alpha > 96 ) work->knf_alpha = 96;
	}

#if 0
	if ( VmpCheckDamage( work ) ){
		return;
	}
#endif
	if( work->dir >= 0 ){
		ctrl->turn.vy = work->dir;
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActSwim );
		ctrl->mov.vy = -8000.0f;
		VMP_KillHola( work );
		return ;
	}
}


static void VMP_ActRapidAvoid_1st( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;
	
	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_RAPIDAVOID );

	if ( time == 0 ) {
		GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotionEx( work, VMP_MOT_KNF_RAPID_AVOID, 0, 0x1fffff, 0 );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f );
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
		SET_FLAG( work->act_flags, VMP_ACTFLAG_RPDDAM );
		//左右反転
		if( work->act_flags & VMP_ACTFLAG_TURN ){
			body->m_ctrl->flag |= (MT_FLAG_REVERSAL1|MT_FLAG_REVERSAL2);
		}		
		work->act_flags ^= VMP_ACTFLAG_TURN;
		if( !work->holawork ){
			work->holawork = (void*)NewVampHola( work->body.objs );
			GV_SetActorChild( work, work->holawork );
		}
		work->think_damtime = 8;
	}

	if(!CHECK_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID|VMP_ACTFLAG_RPDDAM )){
		if( VmpCheckDamage( work ) ) return;
	}
	if ( work->dir >= 0 ) {
		ctrl->turn.vy = work->dir;
	}

	if( time == DIRECT_TICK((148-110)/2) ){
		if( CHECK_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID ) ){
			VMP_SetActCall( work, VMP_ActRapidAvoid_2nd );
			UNSET_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID );
			SET_FLAG( work->act_flags, VMP_ACTFLAG_RPDDAM );
			work->think_damtime = 8;
		}else{
			SET_FLAG( work->status, VMP_STATUS_ACT_END );
			VMP_SetActCall( work, VMP_ActMutekiStand );
			VMP_KillHola( work );
		}
		return;
	}
}

static void VMP_ActRapidAvoid_2nd( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;
	
	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_RAPIDAVOID );

	if(!CHECK_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID|VMP_ACTFLAG_RPDDAM )){
		if( VmpCheckDamage( work ) ) return;
	}
	if( VmpCheckThinkMesg( work ) ){
		return;
	}

	if ( work->dir >= 0 ) {
		ctrl->turn.vy = work->dir;
	}

	if( time == DIRECT_TICK((182-148)/2) ){
		if( CHECK_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID ) ){
			VMP_SetActCall( work, VMP_ActRapidAvoid_3rd );
			UNSET_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID );
			SET_FLAG( work->act_flags, VMP_ACTFLAG_RPDDAM );
			work->think_damtime = 8;
		}else{
			SET_FLAG( work->status, VMP_STATUS_ACT_END );
			VMP_SetActCall( work, VMP_ActMutekiStand );
			VMP_KillHola( work );
		}
		return;
	}
}

static void VMP_ActRapidAvoid_3rd( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;
	
	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_RAPIDAVOID );
	
	if(!CHECK_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID|VMP_ACTFLAG_RPDDAM )){
		if( VmpCheckDamage( work ) ) return;
	}

	if( VmpCheckThinkMesg( work ) ){
		return;
	}
	
	if ( work->dir >= 0 ) {
		ctrl->turn.vy = work->dir;
	}

	if( time == DIRECT_TICK((224-182)/2) ){
		if( CHECK_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID ) ){
			VMP_SetActCall( work, VMP_ActRapidAvoid_4th );
			UNSET_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID );
			SET_FLAG( work->act_flags, VMP_ACTFLAG_RPDDAM );
			work->think_damtime = 8;
		}else{
			SET_FLAG( work->status, VMP_STATUS_ACT_END );
			VMP_SetActCall( work, VMP_ActMutekiStand );
			VMP_KillHola( work );
		}
		return;
	}
}

static void VMP_ActRapidAvoid_4th( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;
	
	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_RAPIDAVOID );

	if(!CHECK_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID|VMP_ACTFLAG_RPDDAM )){
		if( VmpCheckDamage( work ) ) return;
	}
	if( VmpCheckThinkMesg( work ) ){
		return;
	}
	
	if ( work->dir >= 0 ) {
		ctrl->turn.vy = work->dir;
	}

	if( time == DIRECT_TICK((304-224)/2) ){
		if( CHECK_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID ) ){
			VMP_SetActCall( work, VMP_ActRapidAvoid_loop );
			UNSET_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID );
			SET_FLAG( work->act_flags, VMP_ACTFLAG_RPDDAM );
			work->think_damtime = 4;
		}else{
			SET_FLAG( work->status, VMP_STATUS_ACT_END );
			VMP_SetActCall( work, VMP_ActMutekiStand );
			VMP_KillHola( work );
		}
		return;
	}
}

static void VMP_ActRapidAvoid_loop( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;
	
	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_RAPIDAVOID );

	if ( time == 0 ) {
		VMP_SetActMotionCheck( work, VMP_MOT_NON_SPIN_LOOP1 );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f );
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
	}

	if(!CHECK_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID|VMP_ACTFLAG_RPDDAM )){
		if( VmpCheckDamage( work ) ) return;
	}
	if( VmpCheckThinkMesg( work ) ){
		return;
	}
	
	if ( work->dir >= 0 ) {
		ctrl->turn.vy = work->dir;
	}

	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		if( !CHECK_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID ) ){
			SET_FLAG( work->status, VMP_STATUS_ACT_END );
			VMP_SetActCall( work, VMP_ActMutekiStand );
			VMP_KillHola( work );
			return;
		}
		UNSET_FLAG( work->act_flags, VMP_ACTFLAG_NXTAVOID );
		work->think_damtime = 4;
	}
}

static void VMP_ActLowBow( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;
	
	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND );

	if ( time == 0 ) {
		//VMP_SetActMotionEx( work, VMP_MOT_KNF_LOWBOW, 0, 0x1fffff, 85 );
		VMP_SetActMotionCheck( work, VMP_MOT_KNF_LOWBOW );
		ctrl->interp = 8;
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f );
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
	}
	if( VmpCheckThinkMesg( work ) ){
		return;
	}
	
#if 1
	if ( VmpCheckDamage( work ) ){
		return;
	}
#endif
	if ( work->dir >= 0 ) {
		/* 進行方向 */
		ctrl->turn.vy = work->dir;
	}

	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
	//if( time == 512 ){
		VMP_KillHola( work );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActMutekiStand );
		return;
	}
}


static void VMP_ActLowBow_CanBreak( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;
	
	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_CAN_BREAK );

	if ( time == 0 ) {
		//VMP_SetActMotionEx( work, VMP_MOT_KNF_LOWBOW, 0, 0x1fffff, 85 );
		VMP_SetActMotionCheck( work, VMP_MOT_KNF_LOWBOW );
		ctrl->interp = 8;
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f );
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
	}
	if( VmpCheckThinkMesg( work ) ){
		return;
	}
	
#if 1
	if ( VmpCheckDamage( work ) ){
		return;
	}
#endif
	if ( work->dir >= 0 ) {
		/* 進行方向 */
		ctrl->turn.vy = work->dir;
	}

	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
	//if( time == 512 ){
		VMP_KillHola( work );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActMutekiStand );
		return;
	}
}





static void VMP_ActSpinStart( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_MOTAVOID );

	if ( time == 0 ) {
		GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotionCheck( work, VMP_MOT_NON_SPIN_START );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f );
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
		if( !work->holawork ){
			work->holawork = (void*)NewVampHola( work->body.objs );
			GV_SetActorChild( work, work->holawork );
		}

	}
#if 0
	if ( VmpCheckDamage( work ) ){
		return;
	}
#endif
	if( VmpCheckThinkMesg( work ) ){
		return;
	}

	if ( work->dir >= 0 ) {
		/* 進行方向 */
		ctrl->turn.vy = work->dir;
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		if( work->n_spin0 ) VMP_SetActCall( work, VMP_ActSpin0Loop );
		else if( work->n_spin1 ) VMP_SetActCall( work, VMP_ActSpin1Loop );
		else VMP_SetActCall( work, VMP_ActSpinEnd );
		return ;
	}
}


static void VMP_ActSpin0Loop( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;
	
	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_MOTAVOID|VMP_STATUS_NOWAVOID );

	if ( time == 0 ) {
		VMP_SetActMotionCheck( work, VMP_MOT_NON_SPIN_LOOP0 );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f );
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
	}
#if 0
	if ( VmpCheckDamage( work ) ){
		return;
	}
#endif
	if( VmpCheckThinkMesg( work ) ){
		return;
	}
	
	if ( work->dir >= 0 ) {
		/* 進行方向 */
		ctrl->turn.vy = work->dir;
	}

	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_SEMIEND );
		if( --work->n_spin0 == 0 ){
			if( work->n_spin1 ) VMP_SetActCall( work, VMP_ActSpin1Loop );
			else VMP_SetActCall( work, VMP_ActSpinEnd );
		}
		//if( CHECK_FLAG( work->act_flags, VMP_ACTFLAG_ENDSPIN )){
		//	UNSET_FLAG( work->act_flags, VMP_ACTFLAG_ENDSPIN);
		//	VMP_SetActCall( work, VMP_ActSpinEnd );
		//}
		return;
	}
}

static void VMP_ActSpin1Loop( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;
	FVECTOR fvtemp;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_MOTAVOID|VMP_STATUS_NOWAVOID|VMP_STATUS_ADD_STEP );

	if ( time == 0 ) {
		VMP_SetActMotionCheck( work, VMP_MOT_NON_SPIN_LOOP1 );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f );
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
	}

#if 0
	if ( VmpCheckDamage( work ) ){
		return;
	}
#endif
	
	if( VmpCheckThinkMesg( work ) ){
		return;
	}

	if( CHECK_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT )){
		UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
		ThrowKnife(work, NULL, &work->think_init_pos, NULL, 10, 0, 0 );
	}

	if ( work->dir >= 0 ) {
		/* 進行方向 */
		ctrl->turn.vy = work->dir;
	}

	DG_COPY_VEC( &fvtemp, &VMP_ZeroVector );
	fvtemp.vx = work->spin_scale_vec * TS_SINs(ctrl->rot.vy);
	fvtemp.vz = work->spin_scale_vec * TS_COSs(ctrl->rot.vy);
	DG_COPY_VEC( &work->think_mot_step, &fvtemp );
	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_SEMIEND );
		//printf("set semi end\n");
		if( --work->n_spin1 == 0 ){
			VMP_SetActCall( work, VMP_ActSpinEnd );
			work->spin_scale_vec = 0.0f;
		}
		return;
	}
}

static void VMP_ActSpinEnd( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND );// );

	if ( time == 0 ) {
		VMP_SetActMotionCheck( work, VMP_MOT_NON_SPIN_END );
		VMP_KillHola( work );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f );
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
	}
	if( VmpCheckThinkMesg( work ) ){
		return;
	}

#if 1
	if ( VmpCheckDamage( work ) ){
		return;
	}
#endif
	if ( work->dir >= 0 ) {
		/* 進行方向 */
		ctrl->turn.vy = work->dir;
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActStandStill );
		return ;
	}
}

#define		GROUND_DAM	(24)

static int GameDiffGroundDamage[5] = {
	4,
	12,
	GROUND_DAM,
	26,
	30,
};

static void VMP_ActGroundAtt( Work *work, int time )
{
	CONTROL		*ctrl;
	OBJECT		*body;
	TARGET		*trg = &work->att_trgt;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ATTACK|VMP_STATUS_CAN_BREAK );//|VMP_STATUS_ADD_STEP|VMP_STATUS_MUL_STEP );

	if ( time == 0 ) {
		static FVECTOR	AttSize = { 400.0f, 400.0f, 400.0f, 0.0f };
		POWER_TARGET	*pow_trg = &work->att_pow;
		//FVECTOR			force;
		GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotionCheck( work, VMP_MOT_KNF_STOMP );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE*0.5f );
		GM_SetPowerTarget( trg, pow_trg, POWER_CONST, 255, 0, GameDiffGroundDamage[work->game_diff], &DG_ZeroVector );
		GM_SetTargetSize( trg, &AttSize );
	}
	//GM_MoveOnlineTarget( &work->knf_trgt, &work->knf_att_pos[0], &work->knf_att_pos[1] );
	//GM_PutTarget( &work->knf_trgt );

	if( DIRECT_TICK(47) < time && time < DIRECT_TICK(64) ){
		GM_MoveTarget( trg, (FVECTOR*)(BODYWORLD(body,6).m[3]) );
		GM_PutTarget( trg );
	}
	if( VmpCheckThinkMesg( work ) ){
		return;
	}

#if 1
	if ( VmpCheckDamage( work ) ){
		return;
	}
#endif
	if ( work->dir > 0 ) {
		work->control.turn.vy = work->dir;
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActStandStill );
		return;
	}
}

//挑発
static void VMP_ActProvocation( Work *work, int time )
{
	//1258 2076
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_CAN_BREAK );
	if ( time == 0 ) {
		GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotionEx( work, VMP_MOT_NON_PROVO, 0, 0x1fffff, 1260 );
		work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;
	}
	
	if( VmpCheckDamage( work ) ) return;

	if( work->dir >= 0 ){
		work->control.turn.vy = work->dir;
	}
	if( time == DIRECT_TICK(820) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActStandStill );
	}
}

//プレイヤーホーミング立ち
static void VMP_ActStandLockOn( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_CAN_BREAK );
	if( time == 0 ){
		GM_ControlResetHeightMotion( &work->control );
		VMP_SetActMotion( work, work->main_mot );
		//VMP_PRINTF("act lock on init\n");
	}

	if( VmpCheckThinkMesg( work ) ) return;
	if( VmpCheckDamage( work ) ) return;


	if( work->dir >= 0 ){
		work->control.turn.vy = work->dir;
	}
}

//立ち通常
static void VMP_ActStandStill( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_NO_DIR|VMP_STATUS_CAN_BREAK );// );

	if( time == 0 ){
		GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotion( work, work->main_mot );
		//VMP_PRINTF("とまって！[%d]\n",work->dir);
		return;
	}
	if( VmpCheckThinkMesg( work ) ) return;
	if( VmpCheckDamage( work ) ) return;
}

//立ち無敵
static void VMP_ActMutekiStand( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_AVOID );

	if( time == 0 ){
		GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotion( work, work->main_mot );
		work->target.class |= TARGET_SKIP;
		return;
	}
	if( VmpCheckThinkMesg( work ) ){
		//
		return;
	}
}

//立ち無敵リセットしない
static void VMP_ActMutekiStandNoResetH( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_AVOID );

	if( time == 0 ){
		VMP_SetActMotion( work, work->main_mot );
		work->target.class |= TARGET_SKIP;
		return;
	}
	if( VmpCheckThinkMesg( work ) ){
		work->target.class &= ~TARGET_SKIP;
		return;
	}
}

//移動
static void VMP_ActMoveNonRun( Work *work, int time )
{
	CONTROL	*ctrl;

	ctrl = &work->control;

	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_MOVE|VMP_STATUS_CAN_BREAK );//

	if ( time == 0 ) {
		VMP_SetActMotionCheck( work, VMP_MOT_NON_MOVE );//VMP_MOT_KNF_RAIL );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 0.8f);
	}
	if( !(time % 16) ){
		//SD_P_FOOTL01		通常足音左
		
		extern void	GM_SeSetEx( int, FVECTOR*, int, int );
		GM_SeSetEx( SD_P_FOOTL01, &work->control.mov, 50000, 51000 );
	}
	if( VmpCheckThinkMesg( work ) ){
		ctrl->turn.vz = 0 ;
		return;
	}

	if ( VmpCheckDamage( work ) ){
		ctrl->turn.vz = 0;
		return;
	}
	if ( work->dir < 0 ) {
		VMP_SetActCall( work, VMP_ActStandStill ) ;
		//VMP_PRINTF( "おしまい\n" );
		ctrl->turn.vz = 0 ;
		return ;
	}else{
		/* 進行方向 */
		ctrl->turn.vy = work->dir;
	}
#if 0
	/* 方向転換時の体の傾き */
	NPC_Incline( ctrl ) ;
#endif

#if 0
	if ( speed != 0.0F ) {
		act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
		ctrl->step.vx = speed * _RsinF( (int)act->dir ) ;
		ctrl->step.vz = speed * _RcosF( (int)act->dir ) ;
	}
#endif
}

//ダメージ
static void VMP_ActDamege( Work *work, int time )
{
	CONTROL	*ctrl;
	ctrl = &work->control;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_DAMEGE|VMP_STATUS_NO_SETACT );

	if( time == 0 ) {
		
		GM_ControlResetHeightMotion( ctrl ) ;
		//VMP_SetActMotion( work, work->next_mot );
		VMP_SetActMotionNoIntrp( work, work->next_mot );
		ctrl->interp = 1;
		
		//work->control.skip_flag |= CTRL_SKIP_SEG_CHECK;
		//work->control.skip_flag |= CTRL_SKIP_HZX;
		//work->target.class |= TARGET_SKIP;
		//work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;
	}
	//if( time == 8 ) work->target.class &= ~TARGET_SKIP;

	if( (work->next_mot == VMP_MOT_KNF_DAM_FRONT || work->next_mot == VMP_MOT_KNF_DAM_BACK) ){
		if( time >= DIRECT_TICK(20) && VmpCheckDamage( work ) ){
			return;
		}
	}else{
		if( time >= DIRECT_TICK(160) && VmpCheckDamage( work ) ){
			return;
		}
	}

	//if( work->dir >= 0 ){
		//if( work->next_mot == VMP_MOT_KNF_DAM_BACK ){
		//	ctrl->turn.vy = (work->dir+2048)%4096;
		//}else{
		//	ctrl->turn.vy = work->dir;
		//}
	//}
	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActMutekiStand );
		return;
	}


}

//壁ダメージ
static void VMP_ActSpiderDamege( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_DAMEGE|VMP_STATUS_NO_SETACT );

	if( time == 0 ) {
		VMP_DamegeSeCall( &work->control.mov, work->vox_timer, work->stream_handler );
		//GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotion( work, VMP_MOT_KNF_DAM_WALL );
		//work->control.skip_flag |= CTRL_SKIP_SEG_CHECK;
		work->control.skip_flag |= CTRL_SKIP_HZX;
		//work->target.class |= TARGET_SKIP;
		//work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;	
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
//		SET_FLAG( work->status, VMP_STATUS_ACT_END );
//		work->body.flag &= ~OBJECT_MOTIONSTEP_THROUGH;
		VMP_SetActCall( work, VMP_ActSpiderDown );
		return;
	}
	//if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

//壁ダメージ
static void VMP_ActSpiderDown( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_DAMEGE|VMP_STATUS_NO_SETACT );

	if( time == 0 ) {
		//GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotion( work, VMP_MOT_KNF_WLL2WTR_F );
		//work->control.skip_flag |= CTRL_SKIP_SEG_CHECK;
		work->control.skip_flag |= CTRL_SKIP_HZX;
		//work->target.class |= TARGET_SKIP;
		//work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;	
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
//		work->body.flag &= ~OBJECT_MOTIONSTEP_THROUGH;
		VMP_SetActCall( work, VMP_ActSwim );
		return;
	}
	//if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

//壁から逃げ
static void VMP_ActSpiderEscape( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_NO_SETACT|VMP_STATUS_DAMEGE );

	if( time == 0 ) {
		//GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotion( work, VMP_MOT_KNF_WLL2WTR_E );
		//work->control.skip_flag |= CTRL_SKIP_SEG_CHECK;
		work->control.skip_flag |= CTRL_SKIP_HZX;
		//work->target.class |= TARGET_SKIP;
		//work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;	
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
//		work->body.flag &= ~OBJECT_MOTIONSTEP_THROUGH;
		VMP_SetActCall( work, VMP_ActSwim );
		return;
	}
	//if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}



//ナイフ投げshadow（左１）
static void VMP_ActKnifeThrowSingleLeftShadow( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ATTACK|VMP_STATUS_ADD_STEP );
	if ( time == 0 ) {
		GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotion( work, VMP_MOT_KNF_SHDWATT_L );
		body->flag |= OBJECT_MOTIONSTEP_THROUGH;
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE );
	}
	
	if ( work->dir > 0 ) {
		ctrl->turn.vy = work->dir;
	}

//	if( VmpCheckDamage( work ) ) return;
//	if( VmpCheckThinkMesg( work ) ) return;

	//_sceVu0AddVector( &ctrl->step, &VMP_ZeroVector, &work->think_mot_step );
	
	if( time == DIRECT_TICK(28) && VMP_CheckShdw() ){
		int	se_table[] = {
			SD_V_VMPGRE01, //ヴァンプグレネード１「食らえ」
			SD_V_VMPGRE02, //ヴァンプグレネード２「フッ！」
			SD_V_VMPGRE03, //ヴァンプグレネード３「ヘァッ」
			SD_V_VMPGRE04, //ヴァンプグレネード４「テヤッ」
		};
		FVECTOR fvtemp;
		VMP_GetShdwPos( &fvtemp );
		ThrowKnife(work, NULL, &fvtemp, NULL, 10, 0, 1 );
		//ThrowKnife(work, NULL, &fvtemp, NULL, 10, 0, 1 );
		VMP_VoiceSeCall( se_table[irnd()%4], &work->control.mov, GM_SEMODE_BOMB, work->vox_timer );
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_NO_DIR|VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActSwim );
		ctrl->skip_flag &= ~CTRL_SKIP_HZX;
		return ;
	}
}


//ナイフ投げ（左１）
static void VMP_ActKnifeThrowSingleLeft( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ATTACK|VMP_STATUS_CAN_BREAK );
	if ( time == 0 ) {
		GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotion( work, VMP_MOT_KNF_THROW_L );
		work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;
		//work->control.skip_flag |= CTRL_SKIP_FLR_CHECK;
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 1.5f);
	}
	
	if ( work->dir > 0 ) {
		work->control.turn.vy = work->dir;
	}

	if( VmpCheckThinkMesg( work ) ){
		UNSET_FLAG( work->act_flags, VMP_ACTFLAG_COUNTER|VMP_ACTFLAG_SPINATT );
		return;
	}	
	if( VmpCheckDamage( work ) ){
		UNSET_FLAG( work->act_flags, VMP_ACTFLAG_COUNTER|VMP_ACTFLAG_SPINATT );
		return;
	}

	if( CHECK_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT )){
		FVECTOR shift,fvtemp;
		FMATRIX	world;
		_sceVu0SubVector( &shift, &GM_PlayerPosition, &work->control.mov );
		TS_MakeMatrix( &world, &shift, &GM_PlayerPosition );
		_sceVu0ScaleVector( &shift, (FVECTOR*)world.m[0], 100.0f * frnd() );
		_sceVu0ScaleVector( &fvtemp, (FVECTOR*)world.m[0], 100.0f * frnd() );
		_sceVu0AddVector( &shift, &fvtemp, &shift );
		ThrowKnife(work, NULL, NULL, &shift, 10, 0, 0 );
		if( !CHECK_FLAG( work->act_flags, VMP_ACTFLAG_COUNTER ) )SET_FLAG( work->status, VMP_STATUS_THRWKNF );
		UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
		SET_FLAG( work->act_flags, VMP_ACTFLAG_COUNTER );
		//ThrowKnife(work, NULL, NULL, NULL, 10, 0, 0 );
	}
	
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_NO_DIR|VMP_STATUS_ACT_END );
		UNSET_FLAG( work->act_flags, VMP_ACTFLAG_COUNTER );
		//VMP_SetActCall( work, VMP_ActStandStill );
		VMP_SetActCall( work, VMP_ActMutekiStand );
		return ;
	}
}

//ナイフ投げ（左１）
static void VMP_ActKnifeThrow_Muteki( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ATTACK );
	if ( time == 0 ) {
		GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotion( work, VMP_MOT_KNF_THROW_L );
		work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;
		//work->control.skip_flag |= CTRL_SKIP_FLR_CHECK;
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 1.5f);
	}
	
	if ( work->dir > 0 ) {
		work->control.turn.vy = work->dir;
	}
	
	if( VmpCheckThinkMesg( work ) ) return;

	
	if( CHECK_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT )){
		FVECTOR shift,fvtemp;
		FMATRIX	world;
		_sceVu0SubVector( &shift, &GM_PlayerPosition, &work->control.mov );
		TS_MakeMatrix( &world, &shift, &GM_PlayerPosition );
		_sceVu0ScaleVector( &shift, (FVECTOR*)world.m[0], 100.0f * frnd() );
		_sceVu0ScaleVector( &fvtemp, (FVECTOR*)world.m[0], 100.0f * frnd() );
		_sceVu0AddVector( &shift, &fvtemp, &shift );
		ThrowKnife(work, NULL, NULL, &shift, 10, 0, 0 );
		if( !CHECK_FLAG( work->act_flags, VMP_ACTFLAG_COUNTER ))SET_FLAG( work->status, VMP_STATUS_THRWKNF );
		UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
		SET_FLAG( work->act_flags, VMP_ACTFLAG_COUNTER );
		//ThrowKnife(work, NULL, NULL, NULL, 10, 0, 0 );
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		UNSET_FLAG( work->act_flags, VMP_ACTFLAG_COUNTER );
		SET_FLAG( work->status, VMP_STATUS_NO_DIR|VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActMutekiStand );
		return ;
	}
}



//マシンガン投げ
static void VMP_ActKnifeThrowMany( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ATTACK );
//	if ( time == 0 ) {
//		VMP_SetActMotion( work, VMP_MOT_KNF_THROW_L );
//		VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f);
//	}
	if ( time == 0 ) {
		GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotion( work, VMP_MOT_KNF_THROW_L );
		//work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f);
//		work->body2.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE;
//		work->body1.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE;
	}else if( time == 4 ){
		//VMP_SetActMotionObj( &work->body1, VMP_MOT_KNF_THROW_L, (float)TIME_BASE );
	}else if( time == 8 ){
		//VMP_SetActMotionObj( &work->body2, VMP_MOT_KNF_THROW_L, (float)TIME_BASE );
	}
	
	if ( work->dir > 0 ) {
		work->control.turn.vy = work->dir;
	}
	
	if( VmpCheckThinkMesg( work ) ){
		//work->body1.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
		//work->body2.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
//		work->body.flag &= ~OBJECT_MOTIONSTEP_THROUGH;
		return;
	}
	if( VmpCheckDamage( work ) ){
		//work->body1.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
		//work->body2.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
//		work->body.flag &= ~OBJECT_MOTIONSTEP_THROUGH;
		return;
	}
	if( time > 16 && !(time%10) ){
		FVECTOR shift,fvtemp;
		_sceVu0ScaleVector( &shift, (FVECTOR*)BODYWORLD(&work->body,0).m[0], 200.0f * frnd() );
		_sceVu0ScaleVector( &fvtemp, (FVECTOR*)BODYWORLD(&work->body,0).m[1], 200.0f * frnd() );
		_sceVu0AddVector( &shift, &fvtemp, &shift );
		ThrowKnife(work, NULL, NULL, &shift, 10, 0, 0 );
		//ThrowKnife(work, NULL, NULL, NULL, 10, 0, 0 );
	}
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_NO_DIR|VMP_STATUS_ACT_END );
		//work->body1.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
		//work->body2.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
		VMP_SetActCall( work, VMP_ActStandStill );
//		work->body.flag &= ~OBJECT_MOTIONSTEP_THROUGH;
		return ;
	}
}

//中距離攻撃
static void VMP_ActKnifeSlash( Work *work, int time )
{

	if ( time == 0 ) {
		VMP_SetActMotion( work, VMP_MOT_KNF_SLASH );
		work->knf_trgt.class &= ~TARGET_SKIP;
	}
	if( VmpCheckThinkMesg( work ) ) return;
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ATTACK|VMP_STATUS_NO_SETACT );
	if( VmpCheckDamage( work ) ) return;

	//if( time == 11 ){
	//	VMP_SeCall( SD_P_V_NFURI2, &work->control.mov, GM_SEMODE_BOMB );
	//}

	GM_MoveOnlineTarget( &work->knf_trgt, &work->knf_att_pos[0], &work->knf_att_pos[1] );
	GM_PutTarget( &work->knf_trgt );

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_NO_DIR|VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActStandStill );
		return ;
	}

	if ( work->dir > 0 ) {
		work->control.turn.vy = work->dir;
	}
}

//中距離攻撃(影縛り時)
static void VMP_ActKnifeSlashShdw( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_NO_SETACT );
	if ( time == 0 ) {
		work->knf_trgt.class &= ~TARGET_SKIP;
		VMP_SetActMotion( work, VMP_MOT_KNF_SLASH );
		VMP_SetActMotionTimeBase( work, (float)TIME_BASE );
	}else if( time == 1 ){
		//VMP_PRINTF( "止め\n" );
		work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
	}else if( time == DIRECT_TICK(GameDiff_SlashWait[((work->think_now_main_w == 4/*THK_SUB_MAIN3_W*/)?1:0)][work->game_diff])
			  || CHECK_FLAG( work->act_flags, VMP_ACTFLAG_ENDSPIN ) ){
		//VMP_PRINTF( "再動\n" );
		UNSET_FLAG( work->act_flags, VMP_ACTFLAG_ENDSPIN );
		work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
	}

	if( VmpCheckThinkMesg( work ) ){
		work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
		return;
	}
	
	if( time <= DIRECT_TICK(GameDiff_SlashWait[((work->think_now_main_w == 4/*THK_SUB_MAIN3_W*/)?1:0)][work->game_diff]) ){
		SET_FLAG( work->status, VMP_STATUS_CAN_BREAK );
		if( VmpCheckDamage( work ) ){
			work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
			return;
		}
	}

	if( time >= DIRECT_TICK(11)+DIRECT_TICK(GameDiff_SlashWait[((work->think_now_main_w == 4/*THK_SUB_MAIN3_W*/)?1:0)][work->game_diff]) ){
		GM_MoveOnlineTarget( &work->knf_trgt, &work->knf_att_pos[0], &work->knf_att_pos[1] );
		GM_PutTarget( &work->knf_trgt );
		SET_FLAG( work->status, VMP_STATUS_ATTACK );
	}
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_NO_DIR|VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActStandStill );
		return ;
	}

	if ( work->dir > 0 ) {
		work->control.turn.vy = work->dir;
	}
}


//ナイフ投げ（左）
static void VMP_ActKnifeThrow3Left( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ATTACK );
	if ( time == 0 ) {
		GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotion( work, VMP_MOT_KNF_THROW_L );
		work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;
//		VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 1.0f );
	}
	if ( work->dir > 0 ) {
		work->control.turn.vy = work->dir;
	}
	if( VmpCheckThinkMesg( work ) ) return;	
	if( VmpCheckDamage( work ) ) return;


	if( time == DIRECT_TICK(24)/*28*/ || time == DIRECT_TICK(26)/*28*/ || time == DIRECT_TICK(28)/*28*/ || time == DIRECT_TICK(30)/*30*/ || time == DIRECT_TICK(34)/*30*/ ){
		FVECTOR shift,fvtemp;
		_sceVu0ScaleVector( &shift, (FVECTOR*)BODYWORLD(&work->body,0).m[0], 750.0f * frnd() );
		_sceVu0ScaleVector( &fvtemp, (FVECTOR*)BODYWORLD(&work->body,0).m[1], 750.0f * frnd() );
		_sceVu0AddVector( &shift, &fvtemp, &shift );
		ThrowKnife(work, NULL, NULL, &shift, 10, 0 ,0 );
		
		_sceVu0ScaleVector( &shift, (FVECTOR*)BODYWORLD(&work->body,0).m[0], 750.0f * frnd() );
		_sceVu0ScaleVector( &fvtemp, (FVECTOR*)BODYWORLD(&work->body,0).m[1], 750.0f * frnd() );
		_sceVu0AddVector( &shift, &fvtemp, &shift );
		ThrowKnife(work, NULL, NULL, &shift, 10, 0, 0 );		
	}
	
	if( time == DIRECT_TICK(32)/*32*/ ) ThrowKnife(work, NULL, NULL, NULL, 10, 0, 0 );
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_NO_DIR|VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActStandStill );
		return ;
	}
}


//移動ナイフ投げ（左１）
static void VMP_ActMoveKnifeThrowSingleLeft( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ATTACK|VMP_STATUS_MOVE );
	if ( time == 0 ) {
		GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotionEx( work, VMP_MOT_KNF_THROW_L, 1, 0x00000ffe, 0 );
	}
	
	if( VmpCheckDamage( work ) ) return;
	if( time == DIRECT_TICK(32) ) ThrowKnife(work, NULL, NULL, NULL, 10, 0, 0 );

	if ( GM_CheckObject_IsEnd( &work->body, 1 ) ){
		VMP_SetActCall( work, VMP_ActMoveNonRun );
		VMP_SetActMotionEx( work, 0, 1, 0, 0 );
		return ;
	}
	if ( work->dir > 0 ) {
		work->control.turn.vy = work->dir;
	}
}

//斬り
static void VMP_ActSlice( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ATTACK );
	if( time == 0 ){
		work->knf_trgt.class &= ~TARGET_SKIP;
		VMP_SetActMotionEx( work, VMP_MOT_KNF_SLICE, 0, 0x0001ffff, irnd()%40 );
		//VMP_SetActMotion( work, VMP_MOT_KNF_SLICE );
	}

	if( VmpCheckThinkMesg( work ) ) return;
	if( VmpCheckDamage( work ) ) return;

	_sceVu0ScaleVector( &work->knf_pow.force, (FVECTOR*)BODYWORLD(&work->body,0).m[2], 1000.0f );

	GM_MoveOnlineTarget( &work->knf_trgt, &work->knf_att_pos[0], &work->knf_att_pos[1] );
	GM_PutTarget( &work->knf_trgt );

	if( work->dir >= 0 ){
		work->control.turn.vy = work->dir;
	}
	
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		return ;
	}
	
#if 1
	if( 0 && work->think_len > 1000.0f ){
		VMP_SetActCall( work, VMP_ActStandLockOn );
	}
#else
	if( time > DIRECT_TICK(50) ){
		VMP_SetActCall( work, VMP_ActStandLockOn );
	}
#endif
}

/* --------------------------------------------------------------------------------------

ジャンプ関係(旧式)

-------------------------------------------------------------------------------------- */

//水中へ飛び込み
static void VMP_ActDive( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_ADD_STEP );
	if ( time == 0 ) {
		VMP_SetActMotion( work, VMP_MOT_NON_DIVE );
		
		//GM_ConfigObjectFlags( OBJECT *object, int flag );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		//ctrl->skip_flag |= CTRL_SKIP_FLR_CHECK;

		body->flag |= OBJECT_MOTIONSTEP_THROUGH;
	}
	//if(time == 16 ){
	//	//SD_E_V_JUMP04		ヴァンプジャンプ音
	//	VMP_SeCall( SD_E_V_JUMP04, &work->control.mov, GM_SEMODE_BOMB );
	//}
	if( work->dir >= 0 ){
		work->control.turn.vy = work->dir;
	}

	//DG_COPY_VEC( &ctrl->step, &work->think_mot_step );
	//_sceVu0AddVector( &ctrl->step, &ctrl->step, &work->think_mot_step );
	//_sceVu0AddVector( &ctrl->step, &VMP_ZeroVector, &work->think_mot_step );
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		//VMP_SetActCall( work, VMP_ActStandLockOn );
		VMP_SetActCall( work, VMP_ActSwim );
		ctrl->skip_flag &= ~CTRL_SKIP_HZX;
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		//DG_COPY_VEC( &ctrl->step, &VMP_ZeroVector );
		//GM_ControlResetHeightMotion( ctrl ) ;
		//DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
		//body->flag &= ~OBJECT_MOTIONSTEP_THROUGH;
		return ;
	}
}
#if 0
//陸へ飛び出し
static void VMP_ActJumpOut( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_NO_DIR|VMP_STATUS_ADD_STEP );
	if ( time == 0 ) {
		GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotion( work, VMP_MOT_NON_JUMPOUT30 );
		//GM_ConfigObjectFlags( OBJECT *object, int flag );
		body->flag |= OBJECT_MOTIONSTEP_THROUGH;
		ctrl->skip_flag |= CTRL_SKIP_HZX;
	}

	if( work->dir >= 0 ){
		work->control.turn.vy = work->dir;
	}
	
	if( VmpCheckThinkMesg( work ) ) return;

	if( time == DIRECT_TICK(39) ){
		SET_FLAG( work->status, VMP_STATUS_FLY );
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		VMP_SetActCall( work, VMP_ActMutekiStand );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		return ;
	}
}
#endif
//二階へ飛び出し
static void VMP_ActJumpOutLoft( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_MUL_STEP );//|VMP_STATUS_NO_DIR|VMP_STATUS_MUL_STEP );
	if ( time == 0 ) {
		GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotion( work, VMP_MOT_NON_WATER2LOFT );
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		ctrl->rot.vy = work->dir;
		ctrl->turn.vy = work->dir;
		VMP_SetThinkMulStep( work, work->think_mul_x, work->think_mul_y, work->think_mul_z );
		//VMP_PRINTF("act dir %d\n",work->dir);
	}else if( time == DIRECT_TICK(35) ){
		VMP_SetThinkMulStep( work, 1.0f, 1.0f, 1.0f );
	}/*else if( time == 53 ){
		//SD_E_V_TYAKU1		ヴァンプ着地
		VMP_SeCall( SD_E_V_TYAKM1, &work->control.mov, GM_SEMODE_BOMB );
	}else if( time == 112 ){
		//SD_E_V_FOTML3 ヴァンプ足踏み左（鉄棒）
		VMP_SeCall( SD_E_V_FOTML3, &work->control.mov, GM_SEMODE_BOMB );
	}*/
 
	//if( work->dir >= 0 ){
	//	ctrl->rot.vy = work->dir;
	//}
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		VMP_SetActCall( work, VMP_ActStandLockOn );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		//DG_COPY_VEC( &ctrl->step, &VMP_ZeroVector );
		return ;
	}
}

//１階手摺へ飛び出し
static void VMP_ActJumpOutHndRail( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_MUL_STEP );//|VMP_STATUS_NO_DIR|VMP_STATUS_MUL_STEP );
	if ( time == 0 ) {
		GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotion( work, VMP_MOT_NON_WATER2RAIL );
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		ctrl->rot.vy = work->dir;
		ctrl->turn.vy = work->dir;
		VMP_SetThinkMulStep( work, work->think_mul_x, work->think_mul_y, work->think_mul_z );
		//VMP_PRINTF("act dir %d\n",work->dir);
	}else if( time == 63 ){
		VMP_SetThinkMulStep( work, 1.0f, 1.0f, 1.0f );
	}/*else if( time == 53 ){
		//SD_E_V_TYAKU1		ヴァンプ着地
		VMP_SeCall( SD_E_V_TYAKM1, &work->control.mov, GM_SEMODE_BOMB );
	}else if( time == 112 ){
		//SD_E_V_FOTML3 ヴァンプ足踏み左（鉄棒）
		VMP_SeCall( SD_E_V_FOTML3, &work->control.mov, GM_SEMODE_BOMB );
	}*/
 
	//if( work->dir >= 0 ){
	//	ctrl->rot.vy = work->dir;
	//}
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		VMP_SetActCall( work, VMP_ActStandLockOn );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		//DG_COPY_VEC( &ctrl->step, &VMP_ZeroVector );
		return ;
	}
}

//２階から水中へ飛び込み
static void VMP_ActDiveLoftWater( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_ADD_STEP );
	if ( time == 0 ) {
		VMP_SetActMotion( work, VMP_MOT_NON_DIVE );
		//GM_ConfigObjectFlags( OBJECT *object, int flag );
		//ctrl->skip_flag |= CTRL_SKIP_HZX;
		ctrl->skip_flag |= CTRL_SKIP_FLR_CHECK;
		body->flag |= OBJECT_MOTIONSTEP_THROUGH;
	}
/*
	if(time == 16 ){
		//SD_E_V_JUMP04		ヴァンプジャンプ音
		VMP_SeCall( SD_E_V_JUMP04	, &work->control.mov, GM_SEMODE_BOMB );
	}
*/
	if( work->dir >= 0 ){
		work->control.turn.vy = work->dir;
	}

	//DG_COPY_VEC( &ctrl->step, &work->think_mot_step );
	//_sceVu0AddVector( &ctrl->step, &ctrl->step, &work->think_mot_step );
	//_sceVu0AddVector( &ctrl->step, &VMP_ZeroVector, &work->think_mot_step );
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		VMP_SetActCall( work, VMP_ActSwim );
		ctrl->skip_flag &= ~CTRL_SKIP_FLR_CHECK;
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		//DG_COPY_VEC( &ctrl->step, &VMP_ZeroVector );
		//body->flag &= ~OBJECT_MOTIONSTEP_THROUGH;
		return ;
	}
}

//水上飛び出し
static void VMP_ActJumpOutWater( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;
	static int knf_pos_index;
	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_MUL_STEP );
	if ( time == 0 ) {
		GM_ControlResetHeightMotion( ctrl ) ;
//		VMP_SetActMotion( work, VMP_MOT_KNF_JUMP );
		VMP_SetActMotion( work, VMP_MOT_KNF_WATER2WATER );
		VMP_SetActMotionEx( work, VMP_MOT_KNF_WATER2WATER, 0, 0x1fffff, 8 );
		
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 0.5f );
		body->flag |= OBJECT_MOTIONSTEP_THROUGH;
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		VMP_SetThinkMulStep( work, 1.0f, 1.2f, 1.0f );
		knf_pos_index = 0;
	}

	if( work->dir >= 0 ){
		work->control.turn.vy = work->dir;
	}

	if( time == 20 ){
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 0.25f );
	}else if( time == 40 ){
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 0.5f );
	}

	if( time >= 30 && !(time%4) && knf_pos_index <= 8 ){
#if 1
		static FVECTOR knf_att_pod[] = {
			{ 4400.0f, 0.0f, 0.0f, 1.0f },
			{ 4000.0f, 0.0f, 0.0f, 1.0f },
			{ 3600.0f, 0.0f, 0.0f, 1.0f },
			{ 3200.0f, 0.0f, 0.0f, 1.0f },
//			{ 2800.0f, 0.0f, 0.0f, 1.0f },
			{ 2400.0f, 0.0f, 0.0f, 1.0f },
//			{ 2000.0f, 0.0f, 0.0f, 1.0f },
			{ 1600.0f, 0.0f, 0.0f, 1.0f },
//			{ 1200.0f, 0.0f, 0.0f, 1.0f },
			{ 800.0f, 0.0f, 0.0f, 1.0f },
//			{ 400.0f, 0.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			{ -400.0f, 0.0f, 0.0f, 1.0f },
//			{ -800.0f, 0.0f, 0.0f, 1.0f },
//			{ -1200.0f, 0.0f, 0.0f, 1.0f },
//			{ -1600.0f, 0.0f, 0.0f, 1.0f },
//			{ -2000.0f, 0.0f, 0.0f, 1.0f },
//			{ -2400.0f, 0.0f, 0.0f, 1.0f },
//			{ -2800.0f, 0.0f, 0.0f, 1.0f },
//			{ -3200.0f, 0.0f, 0.0f, 1.0f },
		};
		FVECTOR hit;
		FMATRIX	world;
		_sceVu0SubVector( &hit, (FVECTOR*)(BODYWORLD(&work->body,12).m[3]),(FVECTOR*)(BODYWORLD(GM_PlayerBody,12).m[3]) );
		
		TS_MakeMatrix( &world, &hit, (FVECTOR*)(BODYWORLD(GM_PlayerBody,12).m[3]) );
		//TS_MakeMatrix2( &world, &hit, NULL, (FVECTOR*)(BODYWORLD(GM_PlayerBody,12).m[3]) );
		_RotTrans( &hit, &world, &knf_att_pod[knf_pos_index] );
		//HZX_ViewMatrix( &world, 1000.0f );
		//VMP_PRINTF("knf_num [%d]\n",knf_pos_index);
		knf_pos_index++;
		if(0){
			FVECTOR		line[2];
			DG_COPY_VEC( &line[0], (FVECTOR*)(BODYWORLD(&work->body,12).m[3]) );
			DG_COPY_VEC( &line[1], (FVECTOR*)(BODYWORLD(GM_PlayerBody,12).m[3]) );
			NewLineView( line, 1, 160, 32, 140 );
		}
		ThrowKnife(work, NULL, &hit, NULL, 10, 0, 0 );
#else
		FVECTOR shift,fvtemp;
		
		_sceVu0ScaleVector( &shift, (FVECTOR*)BODYWORLD(&work->body,0).m[0], 200.0f * frnd() );
		_sceVu0ScaleVector( &fvtemp, (FVECTOR*)BODYWORLD(&work->body,0).m[1], 200.0f * frnd() );
		_sceVu0AddVector( &shift, &fvtemp, &shift );
		ThrowKnife(work, NULL, NULL, &shift, 10, 0, 0 );
#endif
		//ThrowKnife(work, NULL, NULL, NULL, 10, 0, 0 );
	}
/*	
	if( time > 16 && time & 4 && time < 64 ){
		FVECTOR shift,fvtemp;
		_sceVu0ScaleVector( &shift, (FVECTOR*)BODYWORLD(&work->body,0).m[0], 750.0f * frnd() );
		_sceVu0ScaleVector( &fvtemp, (FVECTOR*)BODYWORLD(&work->body,0).m[1], 750.0f * frnd() );
		_sceVu0AddVector( &shift, &fvtemp, &shift );
		ThrowKnife(work, NULL, NULL, &shift, 10, 0, 0 );
	}
*/
	//_sceVu0AddVector( &ctrl->step, &VMP_ZeroVector, &work->think_mot_step );

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){

		SET_FLAG( work->status, VMP_STATUS_NO_DIR|VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActSwim );
		ctrl->skip_flag &= ~CTRL_SKIP_HZX;
		ctrl->mov.vy = -8000.0f;
		//body->flag &= ~OBJECT_MOTIONSTEP_THROUGH;
		return ;
	}
}


//水上飛び出し
static void VMP_ActWaterOutWater( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;
	static int knf_pos_index;
	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_MUL_STEP|VMP_STATUS_ADD_STEP );
	if ( time == 0 ) {
		GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotion( work, VMP_MOT_KNF_JUMP );
//		VMP_SetActMotion( work, VMP_MOT_KNF_WATER2WATER );
		body->flag |= OBJECT_MOTIONSTEP_THROUGH;
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		VMP_SetThinkMulStep( work, 1.0f, 2.5f, 1.0f );
		VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 0.5f );
		knf_pos_index = 0;
	}

	if( work->dir >= 0 ){
		work->control.turn.vy = work->dir;
	}


	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){

		SET_FLAG( work->status, VMP_STATUS_NO_DIR|VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActSwim );
		return ;
	}
}
//ジャンプ
static void VMP_ActFloorJump( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ADD_STEP|VMP_STATUS_FLY );

	if ( time == 0 ) {
		GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotionCheck( work, VMP_MOT_KNF_JUMP );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		body->flag |= OBJECT_MOTIONSTEP_THROUGH;
		VMP_JumpVoice( &ctrl->mov, work->vox_timer );
	}
/*
	if(time == 0 ){
		//SD_E_V_JUMP04		ヴァンプジャンプ音
		VMP_SeCall( SD_E_V_JUMP04	, &work->control.mov, GM_SEMODE_BOMB );
	}
*/
	//_sceVu0AddVector( &ctrl->step, &VMP_ZeroVector, &work->think_mot_step );
	//ctrl->step.vy = 0.0f;

	if( VmpCheckThinkMesg( work ) ){
		return;
	}

	if ( work->dir >= 0 ) {
		// 進行方向 直値代入
		ctrl->turn.vy = work->dir;
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActStandStill );
		return;
	}
}

//ジャンプ
static void VMP_ActFloorLoftJump( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ADD_STEP );

	if ( time == 0 ) {
		//GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotionCheck( work, VMP_MOT_KNF_JUMP );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		body->flag |= OBJECT_MOTIONSTEP_THROUGH;
	}
/*
	if(time == 0 ){
		//SD_E_V_JUMP04		ヴァンプジャンプ音
		VMP_SeCall( SD_E_V_JUMP04	, &work->control.mov, GM_SEMODE_BOMB );
	}
*/
	//_sceVu0AddVector( &ctrl->step, &VMP_ZeroVector, &work->think_mot_step );
	//ctrl->step.vy = 0.0f;

	if( VmpCheckThinkMesg( work ) ){
		ctrl->skip_flag &= ~CTRL_SKIP_HZX;
		return;
	}

	if ( work->dir >= 0 ) {
		// 進行方向 直値代入
		ctrl->turn.vy = work->dir;
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActStandStill );
		ctrl->skip_flag &= ~CTRL_SKIP_HZX;

		return;
	}
}

//ジャンプ
static void VMP_ActLoftFloorJump( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ADD_STEP );

	if ( time == 0 ) {
		//GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotionCheck( work, VMP_MOT_KNF_JUMP );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		body->flag |= OBJECT_MOTIONSTEP_THROUGH;
	}
/*
	if(time == 0 ){
		//SD_E_V_JUMP04		ヴァンプジャンプ音
		VMP_SeCall( SD_E_V_JUMP04	, &work->control.mov, GM_SEMODE_BOMB );
	}
*/
	//_sceVu0AddVector( &ctrl->step, &VMP_ZeroVector, &work->think_mot_step );
	//ctrl->step.vy = 0.0f;

	if( VmpCheckThinkMesg( work ) ){
		ctrl->skip_flag &= ~CTRL_SKIP_HZX;
		return;
	}

	if ( work->dir >= 0 ) {
		// 進行方向 直値代入
		ctrl->turn.vy = work->dir;
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActStandStill );
		ctrl->skip_flag &= ~CTRL_SKIP_HZX;

		return;
	}
}

//ジャンプ
static void VMP_ActLoftLoftJump( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ADD_STEP );

	if ( time == 0 ) {
		//GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotionCheck( work, VMP_MOT_KNF_JUMP );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		body->flag |= OBJECT_MOTIONSTEP_THROUGH;
	}
/*
	if(time == 0 ){
		//SD_E_V_JUMP04		ヴァンプジャンプ音
		VMP_SeCall( SD_E_V_JUMP04	, &work->control.mov, GM_SEMODE_BOMB );
	}
*/
	//_sceVu0AddVector( &ctrl->step, &VMP_ZeroVector, &work->think_mot_step );
	//ctrl->step.vy = 0.0f;

	if( VmpCheckThinkMesg( work ) ){
		ctrl->skip_flag &= ~CTRL_SKIP_HZX;
		return;
	}

	if ( work->dir >= 0 ) {
		// 進行方向 直値代入
		ctrl->turn.vy = work->dir;
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActStandStill );
		ctrl->skip_flag &= ~CTRL_SKIP_HZX;

		return;
	}
}

//ジャンプ(jump near att)
static void VMP_ActNearAttJump( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ADD_STEP );

	if ( time == 0 ) {
		//GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotionCheck( work, VMP_MOT_KNF_FLR2FLR_F );
		//VMP_SetActMotionCheck( work, VMP_MOT_KNF_JUMP );
		body->flag |= OBJECT_MOTIONSTEP_THROUGH;
		//ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK;
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		//VMP_SeCall( SD_V_VMPRUN02, &ctrl->mov, GM_SEMODE_BOMB );
	}
/*
	if(time == 0 ){
		//SD_E_V_JUMP04		ヴァンプジャンプ音
		VMP_SeCall( SD_E_V_JUMP04, &work->control.mov, GM_SEMODE_BOMB );
	}
*/
	//_sceVu0AddVector( &ctrl->step, &VMP_ZeroVector, &work->think_mot_step );
	//ctrl->step.vy = 0.0f;

	if( time == DIRECT_TICK(23) ){
		SET_FLAG( work->status, VMP_STATUS_FLY );
	}

	if( VmpCheckThinkMesg( work ) ){
		return;
	}

	if ( work->dir >= 0 ) {
		// 進行方向 直値代入
		ctrl->turn.vy = work->dir;
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActKnifeSlashNearAtt );
		return;
	}
}

//張り付きへつながる飛び出し
static void VMP_ActToSpiderFromWater( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ADD_STEP );

	if ( time == 0 ) {
		//GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotionCheck( work, VMP_MOT_KNF_JUMP );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE*0.5f );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		body->flag |= OBJECT_MOTIONSTEP_THROUGH;
	}

	if( VmpCheckThinkMesg( work ) ){
		return;
	}

	if ( work->dir >= 0 ) {
		work->control.turn.vy = work->dir;
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActSpider );
		return;
	}
}

#define		SPIDER_DAM	(32)
static int GameDiffSpiderDamage[5] = {
	8,
	16,
	SPIDER_DAM,		// 32
	32,
	32,
};
//張り付きからふっとばし攻撃
static void VMP_ActToWaterFromSpiderAtt( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;
	TARGET			*trg = &work->att_trgt;
	

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_FLY|VMP_STATUS_ATTACK|VMP_STATUS_SWIM|VMP_STATUS_MUL_STEP );//|VMP_STATUS_ADD_STEP );

	if ( time == 0 ) {
		static FVECTOR	AttSize = { 300.0f, 300.0f, 300.0f, 0.0f };
		POWER_TARGET	*pow_trg = &work->att_pow;
		FVECTOR			force;
		GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotionCheck( work, VMP_MOT_NON_SPDR_ATT );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE*0.5f );
		VMP_SetThinkMulStep( work, 0.8f, 1.0f, 0.8f );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		//ctrl->skip_flag |= CTRL_SKIP_FLR_CHECK;
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;

		_sceVu0ScaleVector( &force, (FVECTOR*)(BODYWORLD(body,12).m[2]), 1000.0f );
		GM_SetPowerTarget( trg, pow_trg, POWER_CONST, 255, 0, GameDiffSpiderDamage[work->game_diff], &force );
		GM_SetTargetSize( trg, &AttSize );
	}
	GM_MoveTarget( trg, &ctrl->mov );
	GM_PutTarget( trg );
	//if( VmpCheckThinkMesg( work ) ){
	//	return;
	//}

	if ( work->dir > 0 ) {
		work->control.turn.vy = work->dir;
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActSwim );
		return;
	}
}

#if 0
//張り付きダメージ
static void VMP_ActSpiderDamege( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;
	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_DAMEGE|VMP_STATUS_NO_SETACT|VMP_STATUS_ADD_STEP|VMP_STATUS_MUL_STEP );

	if( time == 0 ) {
		VMP_DamegeSeCall( &work->control.mov, work->vox_timer, work->stream_handler );
		GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotion( work, VMP_MOT_KNF_JUMP );
		VMP_SetThinkMulStep( work, 1.0f, 0.45f, 1.0f );
		//work->target.class |= TARGET_SKIP;
		work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		
	}
	//if( time == 8 ) work->target.class &= ~TARGET_SKIP;

//	if( VmpCheckDamage( work ) ){
//		work->body.flag &= ~OBJECT_MOTIONSTEP_THROUGH;
//		return;
//	}
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActSwim );
		return;
	}
	//if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}
#endif
/* ------------------------------------------------------------------------------------------

ジャンプ関係（新式）

------------------------------------------------------------------------------------------ */
//ジャンプ汎用アクト
static void VMP_ActCommonJump( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_MUL_STEP|VMP_STATUS_FLY|VMP_STATUS_MOTAVOID );

	if( time == 0 ) {
		//GM_ControlResetHeightMotion( ctrl ) ;
		SET_FLAG( work->status, VMP_STATUS_RESET_H );
		VMP_SetActMotion( work, work->next_mot );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE*2.0f );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		//ctrl->rot.vy = work->dir;
		ctrl->turn.vy = work->dir;
		VMP_SetThinkMulStep( work, work->think_mul_x, work->think_mul_y, work->think_mul_z );
		ctrl->interp = 8;
		if( !work->holawork ){
			work->holawork = (void*)NewVampHola( work->body.objs );
			GV_SetActorChild( work, work->holawork );
		}
		//VMP_PRINTF("flag 立てましたので\n");
		work->motblur_flag = 2;
	}
	//if( work->dir >= 0 ) ctrl->turn.vy = work->dir;
	//VMP_PRINTF("rot = %d turn = %d\n",ctrl->rot.vy,ctrl->turn.vy);
	//if( ctrl->rot.vy == ctrl->turn.vy) VMP_PRINTF("補完終了\n");

#if 0
	if ( VmpCheckDamage( work ) ){
		//return;
	}
#endif
	if( CHECK_FLAG( work->act_flags, VMP_ACTFLAG_CHANGE_MUL ) ){
		VMP_SetThinkMulStep( work, work->think_mul_x, work->think_mul_y, work->think_mul_z );
		UNSET_FLAG( work->act_flags, VMP_ACTFLAG_CHANGE_MUL );
		VMP_PRINTF("[%d]mul xz = %f\n", time, work->think_mul_z );
		VMP_JumpVoice( &ctrl->mov, work->vox_timer );
		//VMP_PRINTF("time [%d]\n",time);
	}

	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		VMP_SetActCall( work, VMP_ActCommonLanding );
		SET_FLAG( work->status, VMP_STATUS_GO_LAND );

		//VMP_PRINTF("go to land mot\n");
		//ctrl->interp = 0;
		return ;
	}
}

//着地汎用アクト
static void VMP_ActCommonLanding( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_MOTAVOID );

	if( time == 0 ) {
		//GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotion( work, work->land_mot );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE*2.0f );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		VMP_SetThinkMulStep( work, work->think_mul_x, work->think_mul_y, work->think_mul_z );
	}

	if( CHECK_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT ) && VMP_CheckShdw() ){
		int	se_table[] = {
			SD_V_VMPGRE01, //ヴァンプグレネード１「食らえ」
			SD_V_VMPGRE02, //ヴァンプグレネード２「フッ！」
			SD_V_VMPGRE03, //ヴァンプグレネード３「ヘァッ」
			SD_V_VMPGRE04, //ヴァンプグレネード４「テヤッ」
		};
		FVECTOR fvtemp;
		VMP_GetShdwPos( &fvtemp );
		ThrowKnife(work, NULL, &fvtemp, NULL, 10, 0, 1 );
		//ThrowKnife(work, NULL, &fvtemp, NULL, 10, 0, 1 );
		VMP_VoiceSeCall( se_table[irnd()%4], &work->control.mov, GM_SEMODE_BOMB, work->vox_timer );
		UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
	}
#if 0
	if( (work->inv_timer*2) >= time ){
		SET_FLAG( work->status, VMP_STATUS_MOTAVOID );
	}else{
		SET_FLAG( work->status, VMP_STATUS_FLY );
	}
#endif
	
	if( DIRECT_TICK(work->inv_timer/*2*/) >= time ){
		SET_FLAG( work->status, VMP_STATUS_FLY );
	}

	if( DIRECT_TICK(work->inv_timer/*2*/) == time ){
		SET_FLAG( work->status, VMP_STATUS_ACT_SEMIEND );
		VMP_KillHola( work );
	}
	if( VmpCheckThinkMesg( work ) ){
		work->motblur_flag = 0;
		VMP_KillHola( work );
		return;
	}
	if( VmpCheckDamage( work ) ) return;
	
	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		VMP_SetActCall( work, work->Next_ActFunc_call );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_KillHola( work );
		work->motblur_flag = 0;
		//ctrl->hzx_base = ctrl->mov.vy - 1158.0F;
		return ;
	}
}


static void VMP_ActJumpFlr2Flr( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_MUL_STEP|VMP_STATUS_FLY|VMP_STATUS_MOTAVOID );

	if( time == 0 ) {
		work->motblur_flag = 2;
		VMP_SetActMotion( work, VMP_MOT_KNF_FLR2FLR_F );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		ctrl->turn.vy = work->dir;
		VMP_SetThinkMulStep( work, work->think_mul_x, work->think_mul_y, work->think_mul_z );
		ctrl->interp = 1;
		if( !work->holawork ){
			work->holawork = (void*)NewVampHola( work->body.objs );
			GV_SetActorChild( work, work->holawork );
		}
	}

	//if( VmpCheckDamage( work ) ) return;
	
	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		VMP_SetActCall( work, VMP_ActMutekiStandNoResetH );
		DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
		VMP_KillHola( work );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		work->motblur_flag = 0;
	}
}

static void VMP_ActJumpFlr2Wll( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_MUL_STEP|VMP_STATUS_FLY|VMP_STATUS_MOTAVOID );

	if( time == 0 ) {
		VMP_SetActMotion( work, VMP_MOT_KNF_FLR2WLL );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		ctrl->turn.vy = work->dir;
		VMP_SetThinkMulStep( work, work->think_mul_x, work->think_mul_y, work->think_mul_z );
		ctrl->interp = 1;
		//work->motblur_flag = 2;
	}

	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		VMP_SetActCall( work, VMP_ActSpider );
		DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		//work->motblur_flag = 0;
		return ;
	}
}


static void VMP_ActJumpFlr2Wtr_Blow( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_MUL_STEP|VMP_STATUS_FLY|VMP_STATUS_MOTAVOID );

	if( time == 0 ) {
		//VMP_DamegeSeCall( &work->control.mov, work->vox_timer, work->stream_handler );
		VMP_SetActMotion( work, VMP_MOT_KNF_DAM_BLOW );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		ctrl->turn.vy = work->dir;
		work->target.class |= TARGET_SKIP;
		VMP_SetThinkMulStep( work, work->think_mul_x, work->think_mul_y, work->think_mul_z );
		ctrl->interp = 1;
	}

	//if( VmpCheckDamage( work ) ) return;
	
	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		VMP_SetActCall( work, VMP_ActSwim );
		DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		return ;
	}
}

static void VMP_ActJumpLoft2Loft( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_FLY|VMP_STATUS_MOTAVOID|VMP_STATUS_ADD_STEP );

	if( time == 0 ) {
		VMP_SetActMotion( work, VMP_MOT_KNF_SPINJUMP );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		ctrl->turn.vy = work->dir;
		//VMP_SetThinkMulStep( work, work->think_mul_x, work->think_mul_y, work->think_mul_z );
		ctrl->interp = 1;
	}
	
	if( VmpCheckDamage( work ) ) return;
	
	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		VMP_SetActCall( work, VMP_ActMutekiStandNoResetH );
		DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		return ;
	}
}

//陸へ飛び出し
static void VMP_ActJumpOut( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_MOVE|VMP_STATUS_NO_DIR|VMP_STATUS_MUL_STEP );
	if ( time == 0 ) {
		//ctrl->rot.vx = 0;
		GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotion( work, VMP_MOT_NON_JUMPOUT30 );
		//GM_ConfigObjectFlags( OBJECT *object, int flag );
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		ctrl->rot.vx = 0;
		ctrl->turn.vx = 0;
		ctrl->turn.vy = work->dir;
		ctrl->interp = 1;
		VMP_PRINTF("set param mul xz[%f]\n", work->think_mul_x );
		VMP_SetThinkMulStep( work, work->think_mul_x, work->think_mul_y, work->think_mul_z );
	}

	if( VmpCheckThinkMesg( work ) ) return;

	if( time == DIRECT_TICK(39) ){
		SET_FLAG( work->status, VMP_STATUS_FLY);//|VMP_STATUS_CAN_BREAK );
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		//VMP_SetActCall( work, VMP_ActMutekiStand );
		VMP_SetActCall( work, VMP_ActMutekiStandNoResetH );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		return ;
	}
}

/* ------------------------------------------------------------------------------------------ */
//泳ぎ
static void VMP_ActSwim( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_SWIM|VMP_STATUS_MOVE|VMP_STATUS_ADD_STEP|VMP_STATUS_CAN_BREAK );//|VMP_STATUS_MUL_STEP );
	if ( time == 0 ) {
		//work->reset_height = -1700.0f;
		//SET_FLAG( work->status, VMP_STATUS_RESET_H );
		GM_ControlResetHeightMotion( ctrl );
		VMP_SetActMotion( work, VMP_MOT_NON_SWIM );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 1.5f );
		ctrl->skip_flag |= CTRL_SKIP_FLR_CHECK;
		ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK;
		//ctrl->hzx_height = -750;
		
	}
	//if( time % 12 == 0 ){
	//	//SD_E_V_SWIM01		ヴァンプ泳ぎ中（12fps連）
	//	VMP_SeCall( SD_E_V_SWIM01, &ctrl->mov, GM_SEMODE_BOMB );
	//}
	//ctrl->step.vy = 0.0f;
	if( work->dir_z >= 0 ){
		ctrl->turn.vz = work->dir_z;
	}
	if( work->dir >= 0 ){		
		ctrl->turn.vy = work->dir;
	}
	if( work->dir_x >= 0 ){
		ctrl->turn.vx = work->dir_x;
	}
	

	if( VmpCheckThinkMesg( work ) ){
		ctrl->turn.vz = 0;
		//ctrl->turn.vx = 0;
		//ctrl->hzx_height = 750;
		return;
	}
#if 1
	if ( VmpCheckDamage( work ) ){
		
		//ctrl->turn.vx = 0;
		return;
	}
#endif

}
#if 0
static void VMP_ActSwimDamage( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_SWIM|VMP_STATUS_MOVE|VMP_STATUS_ADD_STEP );//|VMP_STATUS_MUL_STEP|VMP_STATUS_CAN_BREAK );
	if ( time == 0 ) {
		//GM_ControlResetHeightMotion( ctrl );
		//VMP_SetActMotion( work, VMP_MOT_NON_SWIM );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 1.5f );
		//ctrl->skip_flag |= CTRL_SKIP_FLR_CHECK;
		//ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK;
		//ctrl->hzx_height = -750;
	}
	if( time % 12 == 0 ){
		//SD_E_V_SWIM01		ヴァンプ泳ぎ中（12fps連）
		VMP_SeCall( SD_E_V_SWIM01, &ctrl->mov, GM_SEMODE_BOMB );
	}

	if( work->dir >= 0 ){		
		ctrl->turn.vy = work->dir;
	}
	if( work->dir_x >= 0 ){
		ctrl->turn.vx = work->dir_x;
	}
#if 0
	if ( VmpCheckDamage( work ) ){
		ctrl->turn.vz = 0;
		ctrl->turn.vx = 0;
		return;
	}
#endif

	if( VmpCheckThinkMesg( work ) ){
		ctrl->turn.vz = 0;
		ctrl->turn.vx = 0;
		ctrl->hzx_height = 750;
		return;
	}
}
#endif

//移動(歩き)
static void VMP_ActWalk( Work *work, int time )
{
	CONTROL	*ctrl;

	ctrl = &work->control;

	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_MOVE|VMP_STATUS_CAN_BREAK );//|VMP_STATUS_AVOID2 );//
	if( work->game_diff ) SET_FLAG( work->status, VMP_STATUS_AVOID2 );
	if ( time == 0 ) {
		GM_ControlResetHeightMotion( ctrl );
		VMP_SetActMotionCheck( work, VMP_MOT_NON_WALK );

		if( work->game_diff && !work->holawork ){
			work->holawork = (void*)NewVampHola( work->body.objs );
			GV_SetActorChild( work, work->holawork );
		}
	}
	
	if( VmpCheckThinkMesg( work ) ){
		VMP_KillHola( work );
		ctrl->turn.vz = 0 ;
		return;
	}
	if ( VmpCheckDamage( work ) ){
		ctrl->turn.vz = 0;

		return;
	}
	if ( work->dir >= 0 ){
		ctrl->turn.vy = work->dir;
	}
/*
	if ( work->dir < 0 ) {
		VMP_SetActCall( work, VMP_ActStandLockOn );
		GV_CallChildSignalFunc( work, 0, 60 );
		ctrl->turn.vz = 0 ;
		return ;
	}else{
		// 進行方向
		ctrl->turn.vy = work->dir;
	}
*/
}

//回避
static void VMP_ActAvoid( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_CAN_BREAK );

	if ( time == 0 ) {
		VMP_SetActMotionCheck( work, VMP_MOT_KNF_AVOID );
		VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f );
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
	}
#if 0
	if ( VmpCheckDamage( work ) ){
		return;
	}
#endif
	if( VmpCheckThinkMesg( work ) ){
		return;
	}

	if ( work->dir >= 0 ) {
		/* 進行方向 */
		ctrl->turn.vy = work->dir;
	}

//	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
	if ( time == DIRECT_TICK(19) ){
		//VMP_SetActCall( work, VMP_ActStandStill );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		//body->flag &= ~OBJECT_MOTIONSTEP_THROUGH;
		return ;
	}
}


//回避
static void VMP_ActAvoidFull( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_CAN_BREAK );

	if ( time == 0 ) {
		int se_table[] = {
			SD_V_VMPESB01, //ヴァンプバレエ嘲笑「ハハハッ」
			SD_V_VMPESB02, //ヴァンプバレエ避１「無駄だ！」
			SD_V_VMPESB03, //ヴァンプバレエ避２「読めるぞ」
			SD_V_VMPESR01, //ヴァンプ水面避け１「遅い」
			SD_V_VMPESR02, //ヴァンプ水面避け２「無駄だ」
		};
		static int se_debug = 0;
		
		VMP_VoiceJimakuSeCall( se_table[se_debug%5], &ctrl->mov, GM_SEMODE_BOMB, work->vox_timer );
		se_debug++;
		VMP_SetActMotionCheck( work, VMP_MOT_KNF_AVOID );
		VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f );

		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
	}
#if 0
	if ( VmpCheckDamage( work ) ){
		return;
	}
#endif
	if( VmpCheckThinkMesg( work ) ){
		return;
	}

	if ( work->dir >= 0 ) {
		/* 進行方向 */
		ctrl->turn.vy = work->dir;
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		VMP_SetActCall( work, VMP_ActStandStill );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		//body->flag &= ~OBJECT_MOTIONSTEP_THROUGH;
		return ;
	}
}


//回避
static void VMP_ActOneSpin( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_MOTAVOID|VMP_STATUS_NOWAVOID|VMP_STATUS_ADD_STEP|VMP_STATUS_CAN_BREAK );

	if ( time == 0 ) {
		FVECTOR to;
		GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotionCheck( work, VMP_MOT_NON_SPIN_LOOP1 );//VMP_MOT_KNF_AVOID );
		//VMP_SetActMotionEx( work, VMP_MOT_KNF_AVOID, 0, 0x1fffff, 16 );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.5f );
		//VMP_PRINTF("one spin  [%d]\n", ctrl->rot.vy );
#if 1
		if( work->act_flags & VMP_ACTFLAG_TURN ){
			to.vx = 1000.0f * TS_SINs( work->dir + 1024 ) + 1500.0f * TS_SINs( work->dir );
			to.vz = 1000.0f * TS_COSs( work->dir + 1024 ) + 1500.0f * TS_COSs( work->dir );
		}else{
			to.vx = 1000.0f * TS_SINs( work->dir - 1024 ) + 1500.0f * TS_SINs( work->dir );
			to.vz = 1000.0f * TS_COSs( work->dir - 1024 ) + 1500.0f * TS_COSs( work->dir );	
		}
		to.vy = 0.0f;
		to.vw = 0.0f;
#else
		to.vx = 1000.0f * TS_SINs( ctrl->rot.vy );
		to.vy = 0.0f;
		to.vz = 1000.0f * TS_COSs( ctrl->rot.vy );
		to.vw = 0.0f;
#endif
		_sceVu0AddVector( &to, &to, &ctrl->mov );
		work->act_flags ^= VMP_ACTFLAG_TURN;
		GetInitialVelocity( &work->think_mot_step,
							&work->control.mov,
							&to,
							16.0f );
		
		//body->flag |= OBJECT_MOTIONSTEP_THROUGH;
		//AN_Test_Eye2( &to, 2 );
	}
	//_sceVu0AddVector( &ctrl->step, &ctrl->step, &work->think_mot_step );
#if 0
	if ( VmpCheckDamage( work ) ){
		return;
	}
#endif
	if( VmpCheckThinkMesg( work ) ){
		VMP_KillHola( work );
		return;
	}

	if( CHECK_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT )){
		FVECTOR shift,fvtemp;
		FMATRIX	world;
		_sceVu0SubVector( &shift, &GM_PlayerPosition, &work->control.mov );
		TS_MakeMatrix( &world, &shift, &GM_PlayerPosition );
		_sceVu0ScaleVector( &shift, (FVECTOR*)world.m[0], 100.0f * frnd() );
		_sceVu0ScaleVector( &fvtemp, (FVECTOR*)world.m[0], 100.0f * frnd() );
		_sceVu0AddVector( &shift, &fvtemp, &shift );
		ThrowKnife(work, NULL, NULL, &shift, 10, 0, 0 );
		
		UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
		//ThrowKnife(work, NULL, &work->think_init_pos, NULL, 10, 0, 0 );
	}

	if ( work->dir >= 0 ) {
		/* 進行方向 */
		ctrl->turn.vy = work->dir;
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
//	if ( time == 19 ){
//		printf("aaaaaaaaa\n");
		VMP_KillHola( work );
		//VMP_SetActCall( work, VMP_ActStandStill );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		return ;
	}
}


#if 0
//ナイフ抜き
static void VMP_ActGrasp( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_DAMEGE|VMP_STATUS_NO_SETACT );

	if( time == 0 ) {
		VMP_SetActMotionCheck( work, VMP_MOT_NON_GRASP );

	}
	if( time == 20 ){

	}
	if( VmpCheckDamage( work ) ) return;

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		VMP_SetActCall( work, VMP_ActStandLockOn );
		return;
	}
	//if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

//ナイフ差し（戻し）
static void VMP_ActReturn( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_DAMEGE|VMP_STATUS_NO_SETACT );

	if( time == 0 ) {
		VMP_SetActMotionCheck( work, VMP_MOT_NON_RETURN );
	}
	if( time == 20 ){

	}
	if( VmpCheckDamage( work ) ) return;

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		VMP_SetActCall( work, VMP_ActStandLockOn );
		return;
	}
	//if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}
#endif
//2階移動
static void VMP_ActLoftMove( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_CAN_BREAK );//|VMP_STATUS_AVOID );//
	//if( work->game_diff ) SET_FLAG( work->status, VMP_STATUS_AVOID );
	if ( time == 0 ) {
		GM_ControlResetHeightMotion( &work->control ) ;
		//VMP_SeCall( SD_E_V_ROOL01, &ctrl->mov, GM_SEMODE_BOMB );
		VMP_SetActMotionCheck( work, VMP_MOT_KNF_RAIL );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f );
		ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK;
		//if( work->game_diff && !work->holawork ){
		//	work->holawork = (void*)NewVampHola( work->body.objs );
		//	GV_SetActorChild( work, work->holawork );
		//}
		//ctrl->rot.vy = work->dir;
	}

	if( VmpCheckThinkMesg( work ) ){
		//VMP_KillHola( work );
		return;
	}

	if( VmpCheckDamage( work ) ){
		//VMP_KillHola( work );
		return;
	}
	if ( work->dir >= 0 ) {
		// 進行方向 
		ctrl->turn.vy = work->dir;
	}
}


//2階手拍子
static void VMP_ActLoftTurnEnd( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_CAN_BREAK );

	if ( time == 0 ) {
		//GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotionCheck( work, VMP_MOT_NON_TURNEND );
		//VMP_SetActMotionEx( work, VMP_MOT_NON_TURNEND, 0, 0x1fffff, 110 );
		//VMP_SetActMotionEx( work, VMP_MOT_NON_TURNEND, 0, 0x1fffff, 93 );
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f );
		ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK;
	}

	if( VmpCheckThinkMesg( work ) ){
		body->m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
		return;
	}

	if ( work->dir >= 0 ) {
		// 進行方向 直値代入
		ctrl->turn.vy = work->dir;
	}

//	if( time >= 60 ){
		if( VmpCheckDamage( work ) ){
			body->m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
			return;
		}
//	}
	/*
	if( time == 228-93 || time == 238-93 ){
		int se_table[] = {
			SD_E_V_HNCL01, //ヴァンプ手拍子１
			SD_E_V_HNCL02, //ヴァンプ手拍子２
		};
		VMP_SeCall( se_table[(irnd()>>4)%2], &ctrl->mov, GM_SEMODE_BOMB );
		
	}
	*/
	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		//VMP_PRINTF("turn end[%d]\n",time);
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActMutekiStand );
		return;
	}
/*
	if( time == 160 ){
		body->m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
	}
	if( time == 220 ){
		ctrl->skip_flag &= ~CTRL_SKIP_SEG_CHECK;
		SET_FLAG( work->status, VMP_STATUS_ACT_END );		
	}
*/
}

//2階手拍子
static void VMP_ActLoftDizzy( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_CAN_BREAK );

	if ( time == 0 ) {
		VMP_SetActMotionCheck( work, VMP_MOT_KNF_DIZZY );
		ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK;
	}

	if( VmpCheckThinkMesg( work ) ){
		return;
	}

	if( VmpCheckDamage( work ) ){
		return;
	}
	if ( work->dir >= 0 ) {
		// 進行方向 直値代入
		ctrl->turn.vy = work->dir;
	}

	if( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		//VMP_PRINTF("turn end[%d]\n",time);
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActMutekiStand );
		return;
	}
}


//2階ロングラン
static void VMP_ActLongRun( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;

	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_MOVE|VMP_STATUS_CAN_BREAK );

	if ( time == 0 ) {
		//GM_ControlResetHeightMotion( ctrl ) ;
		VMP_SetActMotionCheck( work, VMP_MOT_NON_LONGRUN );
		VMP_SetActMotionTimeBase( work, (float)TIME_BASE );
		ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK;		
	}else if( time == 8 ){
		//VMP_SetActMotionObj( &work->body1, VMP_MOT_NON_LONGRUN, (float)TIME_BASE /* 2.0f */);
		//work->body1.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE;
		//work->body1.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
	}else if( time == 16 ){
		//VMP_SetActMotionObj( &work->body2, VMP_MOT_NON_LONGRUN, (float)TIME_BASE /* 2.0f */);
		//work->body2.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE;
		//work->body2.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
	}

	if( time == DIRECT_TICK(60) ){
		work->target.class |= TARGET_SKIP;
	}
	if( time == DIRECT_TICK(75) ||
		time == DIRECT_TICK(130) ||
		time == DIRECT_TICK(228) ||
		time == DIRECT_TICK(258) ||
		time == DIRECT_TICK(308) ){
		
		FVECTOR shift,fvtemp;
		FMATRIX	world;
		_sceVu0SubVector( &shift, &GM_PlayerPosition, &ctrl->mov );
		TS_MakeMatrix( &world, &shift, &GM_PlayerPosition );
		_sceVu0ScaleVector( &shift, (FVECTOR*)world.m[0], 100.0f * frnd() );
		_sceVu0ScaleVector( &fvtemp, (FVECTOR*)world.m[0], 100.0f * frnd() );
		_sceVu0AddVector( &shift, &fvtemp, &shift );
		ThrowKnife(work, NULL, NULL, &shift, 10, 0, 0 );
		//ThrowKnife(work, NULL, NULL, NULL, 10, 0, 0 );
	}
	//ctrl->step.vy = 0.0f;

	if( time == DIRECT_TICK(330) ){
		work->target.class &= ~TARGET_SKIP;
		//VMP_SeCall( SD_V_VMPRUN02, &ctrl->mov, GM_SEMODE_BOMB );
	}
/*
	if( time == 154 || time == 225 ){
		//SD_E_V_FOOTL3 ヴァンプ足踏み左
		VMP_SeCall( SD_E_V_FOOTL3, &ctrl->mov, GM_SEMODE_BOMB );
	}

	if( time == 100 || time == 310 ){
		//SD_E_V_FOOTR3 ヴァンプ足踏み右
		VMP_SeCall( SD_E_V_FOOTR3, &ctrl->mov, GM_SEMODE_BOMB );
	}
*/
	if( time >= 330 && !((time-330)%60)){
		//SD_V_VMPRUN01 ヴァンプ壁走り１「ケッヘヘヘ」
		//SD_V_VMPRUN02 ヴァンプ壁走り２「ィアーーー」

		VMP_VoiceSeCall( SD_V_VMPRUN01, &ctrl->mov, GM_SEMODE_BOMB, work->vox_timer );
	}

	if( VmpCheckThinkMesg( work ) ){
		//work->body1.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
		//work->body2.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
		//work->body1.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
		//work->body2.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
		return;
	}

#if 1
	if ( VmpCheckDamage( work ) ){
		//work->body1.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
		//work->body2.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
		//work->body1.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
		//work->body2.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;

		return;
	}
#endif
	
	if ( work->dir >= 0 ) {
		// 進行方向 直値代入
		ctrl->turn.vy = work->dir;
	}
	
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
//	if ( time == 19 ){
		VMP_SetActCall( work, VMP_ActStandStill );
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		//work->body1.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
		//work->body2.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
		//work->body1.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
		//work->body2.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
		return ;
	}
}

#if 0
//ナイフ投げ（左１）
static void VMP_ActKnifeThrowTest( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_ATTACK );

	if ( time == 0 ) {
		VMP_SetActMotion( work, VMP_MOT_KNF_THROW_LU );
//		VMP_SetActMotionTimeBase( work, (float)TIME_BASE * 2.0f);
		//work->body2.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE;
		//work->body1.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE;
		//VMP_SetActMotionObj( &work->body1, VMP_MOT_KNF_THROW_LS, (float)TIME_BASE /* 2.0f */);
		//VMP_SetActMotionObj( &work->body2, VMP_MOT_KNF_THROW_LD, (float)TIME_BASE /* 2.0f */);
	}else if( time == 4 ){
//		VMP_SetActMotionObj( &work->body1, VMP_MOT_KNF_THROW_LU, (float)TIME_BASE * 2.0f );
	}else if( time == 8 ){
//		VMP_SetActMotionObj( &work->body2, VMP_MOT_KNF_THROW_LU, (float)TIME_BASE * 2.0f );
	}
	
//	if ( work->dir > 0 ) {
//		work->control.turn.vy = work->dir;
//	}
	/*
	if( time & 1){
		work->body1.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
		work->body2.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE;
	}else{
		work->body2.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
		work->body1.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE;
	}
	*/
//	if( VmpCheckDamage( work ) ) return;
//	if( VmpCheckThinkMesg( work ) ) return;

	if( time == 16/*32*/ ) ThrowKnife(work, NULL, NULL, NULL, 10, 0, 0 );

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
//		SET_FLAG( work->status, VMP_STATUS_NO_DIR|VMP_STATUS_ACT_END );
//		VMP_SetActCall( work, VMP_ActStandStill );
//		work->body1.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
//		work->body2.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
//		return ;
	}
}
#endif


static void VMP_ActSpecial( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_NO_SETACT|VMP_STATUS_CAN_BREAK );//

	if( time == 0 ){
		work->reset_height = 456.17150f;
		SET_FLAG( work->status, VMP_STATUS_RESET_H );
		//GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotion( work, VMP_MOT_NON_SPECIAL );
		//work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;
		//VMP_SetActMotionTimeBase( work, (float)TIME_BASE*2.0f );
		//VMP_SeCall( SD_V_VMPCUT01, &work->control.mov, GM_SEMODE_BOMB );
		work->knf_trgt.class &= ~TARGET_SKIP;
		
	}
	if( time < DIRECT_TICK(140) ){
		SET_FLAG( work->status, VMP_STATUS_STOMP );
	}
	if( time >= DIRECT_TICK(140) && work->dir >= 0 ){
		work->control.turn.vy = work->dir;
	}
	if( VmpCheckThinkMesg( work ) ){
		return;
	}
	if( VmpCheckDamage( work ) ) return;
/*
	if( time == 270/2 || time == 294/2 ){
		VMP_SeCall( SD_E_V_ROOL01, &work->control.mov, GM_SEMODE_BOMB );
	}
	if( time == 330/2 ){
		VMP_SeCall( SD_E_V_FOOTL3, &work->control.mov, GM_SEMODE_BOMB );
	}
	if( time == 360/2 ){
		VMP_SeCall( SD_P_V_NFURI2, &work->control.mov, GM_SEMODE_BOMB );
	}*/
	if( time > DIRECT_TICK(240/2) ){
		GM_MoveOnlineTarget( &work->knf_trgt, &work->knf_att_pos[0], &work->knf_att_pos[1] );
		GM_PutTarget( &work->knf_trgt );
	}
	if( time > DIRECT_TICK(190) ){
		if( work->think_now_main_w != 4 /* THK_SUB_MAIN3_W */ && !work->holawork ){
			work->holawork = (void*)NewVampHola( work->body.objs );
			GV_SetActorChild( work, work->holawork );
		}
	}

	
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
//	if( time == 450/2 ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActMutekiStand );
		return;
	}
}


//中距離攻撃(ニアアタック)
static void VMP_ActKnifeSlashNearAtt( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_NO_SETACT );

	//printf("time %d/%d\n",time, GameDiff_SlashWait[((work->think_now_main_w == 4/*THK_SUB_MAIN3_W*/)?1:0)][work->game_diff]);
	if ( time == 0 ) {
		VMP_SetActMotion( work, VMP_MOT_KNF_SLASH );
		VMP_SetActMotionTimeBase( work, (float)TIME_BASE );
	}else if( time == 1 ){
		//VMP_PRINTF( "止め\n" );
		//VMP_SeCall( SD_V_VMPCUT01, &work->control.mov, GM_SEMODE_BOMB );
		work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
	}else if( time == DIRECT_TICK(GameDiff_SlashWait[((work->think_now_main_w == 4/*THK_SUB_MAIN3_W*/)?1:0)][work->game_diff]) ||
			  CHECK_FLAG( work->act_flags, VMP_ACTFLAG_ENDSPIN )){
		//VMP_PRINTF( "再動\n" );
		UNSET_FLAG( work->act_flags, VMP_ACTFLAG_ENDSPIN );
		work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
	}

	if( VmpCheckThinkMesg( work ) ){
		work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
		return;
	}
	
	if( time <= DIRECT_TICK(GameDiff_SlashWait[((work->think_now_main_w == 4/*THK_SUB_MAIN3_W*/)?1:0)][work->game_diff]) ){
		SET_FLAG( work->status, VMP_STATUS_CAN_BREAK|VMP_STATUS_DAM_MUL );
		if( VmpCheckDamage( work ) ){
			work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
			return;
		}
	}
	if( time == DIRECT_TICK(11)+DIRECT_TICK(GameDiff_SlashWait[((work->think_now_main_w == 4/*THK_SUB_MAIN3_W*/)?1:0)][work->game_diff]) ){
		int se_table[] = {
			SD_V_VMPCUT02, //ヴァンプナイフ攻撃１「シェッ」
			SD_V_VMPCUT03, //ヴァンプナイフ攻撃２「フンッ」
			SD_V_VMPCUT04, //ヴァンプナイフ攻撃３「ホッ！」
			SD_V_VMPCUT05, //ヴァンプナイフ攻撃４「ハッ！」
		};
		VMP_VoiceSeCall( se_table[(irnd()>>6)%4], &work->control.mov, GM_SEMODE_BOMB, work->vox_timer );
		VMP_SeCall( SD_P_V_NFURI2, &work->control.mov, GM_SEMODE_BOMB );
	}
		

	
	if( time >= 11+DIRECT_TICK(GameDiff_SlashWait[((work->think_now_main_w == 4/*THK_SUB_MAIN3_W*/)?1:0)][work->game_diff]) ){
		//明日なおしてねーん

		//VMP_PRINTF("あわわわわ\n");


		//SET_FLAG( work->status, VMP_STATUS_ADD_STEP );
		SET_FLAG( work->status, VMP_STATUS_ATTACK|VMP_STATUS_ADD_STEP );
		GM_MoveOnlineTarget( &work->knf_trgt, &work->knf_pos[0], (FVECTOR*)BODYWORLD(&work->body,2).m[3] );
		GM_PutTarget( &work->knf_trgt );
		work->think_mot_step.vx = work->control.step.vx;
		work->think_mot_step.vy = 0.0f;
		work->think_mot_step.vz = work->control.step.vz;
		work->think_mot_step.vw = 0.0f;
		
		//_sceVu0AddVector( &work->control.step, &VMP_ZeroVector, &work->think_mot_step );
	}
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_NO_DIR|VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActMutekiStand );
		return ;
	}

	if ( work->dir > 0 ) {
		work->control.turn.vy = work->dir;
	}
}

static void VMP_ActOneTime( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_NO_SETACT );

	if( time == 0 ) {
		VMP_SetActMotion( work, work->next_mot );
	}

	if ( GM_CheckObject_IsEnd( &work->body, 0 ) ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActStandStill );
		return;
	}
}


static void VMP_ActDeadEnd( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND );

	if( time == 0 ) {
		VMP_SetActMotion( work, VMP_MOT_NON_END );
	}

	if( VmpCheckDamage( work ) ){
		return;
	}

	if( time == DIRECT_TICK(390) ){
		time = -1;
		return;
	}
}

//張り付き
static void VMP_ActSpider( Work *work, int time )
{
	CONTROL	*ctrl;
	OBJECT	*body;

	ctrl = &work->control;
	body = &work->body;
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_NO_DIR|VMP_STATUS_SPIDER );// );

	if( time == 0 ){
		GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotion( work, VMP_MOT_NON_SPIDER );
		ctrl->skip_flag |= CTRL_SKIP_HZX;
		return;
	}
	if( VmpCheckThinkMesg( work ) ) return;
	if( VmpCheckDamage( work ) ) return;


	if ( work->dir >= 0 ) {
		work->control.turn.vy = work->dir;
	}
	//VMP_PRINTF("rot %d: turn %d\n", work->control.rot.vy, work->control.turn.vy );

}


//指定の方向を見たら終わる待ちモーション
static void VMP_ActStandTurn( Work *work, int time )
{
	SET_FLAG( work->status, VMP_STATUS_STAND|VMP_STATUS_NO_DIR|VMP_STATUS_CAN_BREAK );// );

	if( time == 0 ){
		GM_ControlResetHeightMotion( &work->control ) ;
		VMP_SetActMotion( work, work->main_mot );
		work->control.interp = 15;
		return;
	}

	if ( work->dir >= 0 ) {
		work->control.turn.vy = work->dir;
	}
	
	if( VmpCheckThinkMesg( work ) ) return;
	if( VmpCheckDamage( work ) ) return;

	//VMP_PRINTF("%d -> %d\n", work->control.rot.vy, work->control.turn.vy );
	if( work->control.rot.vy == work->control.turn.vy ){
		SET_FLAG( work->status, VMP_STATUS_ACT_END );
		VMP_SetActCall( work, VMP_ActLoftMove );
	}

}

/* ------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
void VMP_PreAct( Work *work )
{
	work->status = 0;
	work->act_timer++;
}
void VMP_AfterAct( Work *work )
{
	
	if( CHECK_FLAG( work->status, VMP_STATUS_RAPIDAVOID ) &&
		CHECK_FLAG( work->act_flags, VMP_ACTFLAG_RPDDAM ) ){
		//VMP_PRINTF("check RAPID DAM FLAG\n");
		if( --work->think_damtime < 0 ){
			UNSET_FLAG( work->act_flags, VMP_ACTFLAG_RPDDAM );
			//VMP_PRINTF("unset RAPID DAM FLAG\n");
			work->think_damtime = 0;
		}
		
	}
	

}
void VMP_InitAction( Work *work )
{
	work->act_timer = -1;
	work->main_mot = VMP_MOT_RIL_WAIT;
	VMP_SetActCall( work, VMP_ActStandStill );
	//VMP_SetActCall( work, VMP_ActAvoidLoop0 );
}

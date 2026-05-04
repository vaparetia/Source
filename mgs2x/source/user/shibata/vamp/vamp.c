//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vamp.c
	ヴァンプさん
	2001/03/02

	$Id: vamp.c,v 1.1.1.3 2002/11/19 11:48:55 Yoshizawa1 Exp $
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

#include	"../../shibata/t_headmark/headmark.h"
#include	"../../kano/attachment/attachment_called.h"
#include	"../../takabe/other/puppetik.h"
#include	"../../kira/face/face.h"
#include	"../util/ts_util.h"

#include	"vamp.h"
#include	"vmp_inline.h"
#include "libfs.h"

#define ALL_KNFMDL_NAME		(GV_StrCode( "vmp_parts_vkpa_mh" ))
#define PARTS_KNFMDL_NAME	(GV_StrCode( "vmp_parts_vkpb_mh" ))
#define BOUND_MDL_NAME		(GV_StrCode( "vmp_naked_bounding" ))


#define MODEL_NAME		(15093926)	// GV_StrCode( "vmp_naked_sh" )
#define SHDWMDL_NAME	(10519393)	// GV_StrCode( "vmp_naked_shadow" )
#define MW_MODEL_NAME	(7123410)	// GV_StrCode( "vmp_naked_light_mh_mt" )
#define BASE_MOTION		(15526203)	// GV_StrCode( "w31c_vmp_base" )

#define KNIFE_MODL_NAME	(GV_StrCode( "vkf_big_inv" ))

#define OBJECT_FLAG		(DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)
#define CHILD_TRG_FLAG	(TARGET_ROTATE|TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER|TARGET_THROUGH|TARGET_CALL_CALLBACK_THROUGH_HIT)
// 標準|TARGET_CAPTURE
#define DEF_TARGET_CLASS (TARGET_LOCKON|TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER| \
							TARGET_CHILD|TARGET_CHILD_ALWAYS|TARGET_THROUGH|TARGET_CALL_CALLBACK_THROUGH_HIT)
#define DEF_TARGET_CLASS2 (TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER|TARGET_THROUGH)



static FVECTOR	VMP_ZeroVector = { 0.0f, 0.0f, 0.0f, 0.0f };

static int	Child_Level_Num[] = { 
	0,	// Level 0 のターゲット数
	2,	// Level 1 のターゲット数
	9,	// Level 2 のターゲット数
	1	// Level 3 のターゲット数
};

int	VMP_LifeCheck ;

#if 0
enum {
	SMP_TARGET_CHILD_ODEK,
	SMP_TARGET_CHILD_HART,
	
	SMP_TARGET_CHILD_HEAD,
	SMP_TARGET_CHILD_ARMR1,
	SMP_TARGET_CHILD_ARMR2,
	SMP_TARGET_CHILD_ARML1,
	SMP_TARGET_CHILD_ARML2,
	SMP_TARGET_CHILD_LEGR1,
	SMP_TARGET_CHILD_LEGR2,
	SMP_TARGET_CHILD_LEGL1,
	SMP_TARGET_CHILD_LEGL2,

	SMP_TARGET_CHILD_BPDY,
};
#endif
static int	Child_Target_Connect[CHILD_TARGET_NUM] = { 
	HUMAN21_ATAMA,
	HUMAN21_ONAKA,

	HUMAN21_ATAMA,
	HUMAN21_MIGI_UDE1,
	HUMAN21_MIGI_UDE2,
	HUMAN21_HIDARI_UDE1,
	HUMAN21_HIDARI_UDE2,
	HUMAN21_MIGI_ASHI1,
	HUMAN21_MIGI_ASHI2,
	HUMAN21_HIDARI_ASHI1,
	HUMAN21_HIDARI_ASHI2,

	HUMAN21_MUNE
};

static FVECTOR	Child_Target_Shift[CHILD_TARGET_NUM] = { 
	// level 0 は無し
	// level 1
	{ -0.0F, 68.0F, 116.0F },		// おでこ0,68,116	
	{ -55.0F, 60.0F, 110.0F },		// 心臓
	// level 2
	{ 0.0F, 30.0F, 0.0F },			// 頭
	{ 0.0F, -130.0F, 0.0F },		// 右腕１
	{ 0.0F, -130.0F, 0.0F },		// 右腕２
	{ 0.0F, -130.0F, 0.0F },		// 左腕１
	{ 0.0F, -130.0F, 0.0F },		// 左腕２
	{ -10.0F, -200.0F, -10.0F },	// 右足１
	{ -20.0F, -250.0F, -20.0F },	// 右足２
	{ -10.0F, -200.0F, -10.0F },	// 左足１
	{ -20.0F, -250.0F, -20.0F },	// 左足２
	// level 3
	{ 0.0F, -40.0F, 0.0F },			// 胴体
} ;

static FVECTOR	Child_Target_Size[CHILD_TARGET_NUM] = { 
	// level 0 は無し
	// level 1
	{ 10.0F, 10.0F, 10.0F },		// おでこ
	{ 20.0F, 20.0F, 20.0F },		// 心臓
	// level 2
	{ 96.0F, 130.0F, 96.0F },		// 頭
	{ 75.0F, 150.0F, 75.0F },		// 右腕１
	{ 75.0F, 150.0F, 75.0F },		// 右腕２
	{ 75.0F, 150.0F, 75.0F },		// 左腕１
	{ 75.0F, 150.0F, 75.0F },		// 左腕２
	{ 85.0F, 200.0F, 120.0F },		// 右足１
	{ 85.0F, 260.0F, 120.0F },		// 右足２
	{ 85.0F, 200.0F, 120.0F },		// 左足１
	{ 85.0F, 260.0F, 120.0F },		// 左足２
	// level 3
	{ 200.0F, 290.0F, 140.0F },		// 胴体
};


// ターゲットサイズ
static FVECTOR SampleTrgSize[] = {
	{ 800.0F, 1000.0F, 800.0F },	/* 立ち */
	{ 1000.0F, 200.0F, 1000.0F },	/* ダウン */
	{ 700.0F, 650.0F, 700.0F },		/* しゃがみ */
} ;

#if 0
// 引き摺りシフト
static FVECTOR SampleDragShift[] = {
	{ 26.483F, -393.755F, 940.002F },	/* 仰向け頭16 */
	{ 0.0F, -113.487F, 291.057F },		/* 仰向け頭 */
	{ 0.0F, -421.119F, 1192.692F },		/* 仰向け足16 */
	{ 0.0F, -382.115F, 422.438F },		/* 仰向け足 */
	{ 0.0F, -346.867F, 1192.629F },		/* うつぶせ足16 */
	{ 0.0F, -294.030F, 463.501F },		/* うつぶせ足 */
	{ 45.761F, -317.266F, 946.003F },	/* うつ伏せ頭16 */
};
#endif

//ナイフ軌跡用
static FVECTOR VmpKnifePos[2] = {
	{ 13.5f, -93.1f, -60.0f, 1.0f },
	{ 13.5f, -93.1f, -339.0f, 1.0f },
};


static Work *g_VMP_Work = NULL;
/* ------------------------------------------------------------------
    ダメージコールバック
------------------------------------------------------------------ */
static void CallOffenceWhenThrough( TARGET *off, TARGET *def )
{
    off->hit = def->hit ;
    off->damaged |= off->class & TARGET_CHECK_CLASS ;
    if ( off->callback != NULL )
	( *off->callback )( off, def, off->work ) ;
}

static void KnifeDmgCallBack( TARGET *off, TARGET *def, Work *work )
{
    if( def->damaged & TARGET_POWER ){
		VmpShdwClear();
		if( !GM_CheckPlayerStatus( PLAYER_DAMAGED ) ){
			GM_SeSetMode( SD_E_V_NCUT01, &work->control.mov, GM_SEMODE_BOMB );
			off->class |= TARGET_SKIP;
		}
	}
}

static void KnifeDmgCallBack2( TARGET *off, TARGET *def, Work *work )
{
    if( def->damaged & TARGET_POWER ){
		VmpShdwClear();
		if( !GM_CheckPlayerStatus( PLAYER_DAMAGED )){
			if( CHECK_FLAG( work->status, VMP_STATUS_STOMP ) )
				GM_SeSetMode( SD_E_V_NCUT01, &work->control.mov, GM_SEMODE_BOMB );
			off->class |= TARGET_SKIP;
		}
	}
}


#define	VMP_CAN_AVOID		(WP_PSG1|WP_PSG1T|WP_BULLET|WP_M92)
#define	MAX_DAMEGE			(32.0f)
#define	MIN_DAMEGE			(1)

#define MAX_BLAST_DAMEGE_W	(4)
#define MIN_BLAST_DAMEGE_W	(0)

#define BLAST_DAMEGE		(14)
#define PUNCH_DAMEGE		(4)
#define KICK_DAMEGE			(6)
#define STAN_DAMEGE			(8)


//難易度テーブル
static float GameDiffDam_MaxDamege[5] = {
	48.0f,
	40.0f,
	MAX_DAMEGE,			//32.0f
	24.0f,
	16.0f,
};

static int GameDiffDam_MinDamege[5] = {
	2,
	1,
	MIN_DAMEGE,			// 1
	MIN_DAMEGE,
	MIN_DAMEGE,
};

static int GameDiffDam_MaxBlastWaterDamege[5] = {
	16,
	8,
	MAX_BLAST_DAMEGE_W,			// 4
	2,
	2,
};

static int GameDiffDam_MinBlastWaterDamege[5] = {
	MIN_BLAST_DAMEGE_W,
	MIN_BLAST_DAMEGE_W,
	MIN_BLAST_DAMEGE_W,			// 0
	MIN_BLAST_DAMEGE_W,
	MIN_BLAST_DAMEGE_W,
};

static int GameDiffDam_BlastDamege[5] = {
	24,
	20,
	BLAST_DAMEGE,			// 14
	12,
	8,
};


static int GameDiffDam_PunchDamege[5] = {
	12,
	10,
	PUNCH_DAMEGE,			// 4
	4,
	2,
};

static int GameDiffDam_KichDamege[5] = {
	16,
	14,
	KICK_DAMEGE,			// 6
	6,
	4,
};

static int GameDiffDam_StanDamege[5] = {
	18,
	14,
	STAN_DAMEGE,			// 8
	6,
	4,
};

#define SLUSH_DAMEGE	(24)

static float GameDiffDam_SlushDamege[5] = {
	4,
	12,
	SLUSH_DAMEGE,			// 24
	26,
	30,
};


static float ChildDmgRatio[CHILD_TARGET_NUM] = {
	// level 0 は無し
	// level 1
	1.0f,		// おでこ
	1.0f,		// 心臓
	// level 2
	0.45f,		// 頭
	0.2f,		// 右腕１
	0.2f,		// 右腕２
	0.2f,		// 左腕１
	0.2f,		// 左腕２
	0.2f,		// 右足１
	0.2f,		// 右足２
	0.2f,		// 左足１
	0.2f,		// 左足２
	// level 3
	0.32f,		// 胴体
};
#if 0
static int ChildNonActDmg[CHILD_TARGET_NUM] = {
	// level 0 は無し
	// level 1
	MAX_DAMEGE,		// おでこ
	MAX_DAMEGE,		// 心臓
	// level 2
	MIN_DAMEGE,		// 頭
	MIN_DAMEGE,		// 右腕１
	MIN_DAMEGE,		// 右腕２
	MIN_DAMEGE,		// 左腕１
	MIN_DAMEGE,		// 左腕２
	MIN_DAMEGE,		// 右足１
	MIN_DAMEGE,		// 右足２
	MIN_DAMEGE,		// 左足１
	MIN_DAMEGE,		// 左足２
	// level 3
	MIN_DAMEGE,		// 胴体
};
#endif

/*
	・rai_katana_01		:	ブレード斬りヒット       WP_BLADE
	・rai_katana_02		:	ブレード突きヒット    	WP_BLADESTAB
	・rai_katana_03		:	ブレード峰討ちヒット	WP_BLADEFAINT

	・rai_kick_01		:	キックヒット（1回目）  	WP_KICK | WP_KICK1 が入ってます。
	・rai_kick_02		:	キックヒット（2回目）     WP_KICK が入ってます。
	
	・rai_punch_01		:	パンチヒット（弱）        WP_PUNCH
	・rai_punch_02		:	パンチヒット（中）		WP_PUNCH、Ｍ４、ＡＫ、クレイモア、ＲＧＢ装備時
	・rai_punch_03		:	ニキータアタックヒット	WP_KICKがはいっているので、装備で見てください。

	振動の呼び方は、
お疲れさまです。
ダメージ振動の組み込みをさきほどお願いしましたが、
パンチ振動の強弱の呼び分け方を知らせていませんでした。
以下のようにお願いします。

	NewPadVibration2( GV_StrCode( "名前" ), 0 ) ;
*/

static void PrntDmgCallBack( TARGET *off, TARGET *def, Work *work )
{
	long64	weapon;
	weapon = def->weapon_type;

	//VMP_PRINTF("親　type[%016lx][%016lx]!!\n",weapon,weapon&WP_WEAPONCORE );

	if( CHECK_FLAG( work->status, VMP_STATUS_FLY ) ){
		
	}else if( CHECK_FLAG( work->status, VMP_STATUS_SPIDER ) ){
		if( weapon & WP_PUNCHALL ){
			if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
				NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 強 */
			} else {
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 弱 */
			}

			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SPDR_DAM, -1, 0, GameDiffDam_PunchDamege[work->game_diff], -1 );
		}else if( weapon & WP_KICK ){		
			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SPDR_DAM, -1, 0, GameDiffDam_KichDamege[work->game_diff], -1 );
		}else if( weapon & WP_BLAST && !(work->status & VMP_STATUS_SWIM) ){
			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SPDR_DAM, -1, GameDiffDam_BlastDamege[work->game_diff], 0, -1 );
		}else if( weapon & WP_STUNGRENADE ){
			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SPDR_DAM, -1, 0, GameDiffDam_StanDamege[work->game_diff], -1 );
		}
		work->target.class |= TARGET_SKIP;
	}else{
		if( weapon & WP_PUNCHALL ){
			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_PUNCH, -1, 0, GameDiffDam_PunchDamege[work->game_diff], -1 );
			if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
				NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 強 */
			} else {
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 弱 */
			}

		}else if( weapon & WP_KICK1 ){
			NewPadVibration2( GV_StrCode( "rai_kick_01" ), 0 ) ;
			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_PUNCH, -1, 0, GameDiffDam_PunchDamege[work->game_diff], -1 );
			//work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_KICK, -1, 0, GameDiffDam_KichDamege[work->game_diff], -1 );
		}else if( weapon & WP_KICK ){	//ラスト蹴り
			if( !GM_CheckPlayerStatus( PLAYER_BEYOND ) ){
				NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ;
				DG_COPY_VEC( &work->off_force, &off->power->force );
				DG_COPY_VEC( &work->off_hit, &GM_PlayerPosition );//&off->hit );
				if( work->clmr_timer < 0 ){
					if( CHECK_FLAG( work->think_flags, VMP_FLAGS_NO_BLOW ) )
						work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_BULLET, -1, 0, GameDiffDam_KichDamege[work->game_diff], -1 );
					else
						work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_KICK, -1, 0, GameDiffDam_KichDamege[work->game_diff], -1 );
					work->clmr_timer = 8;
				}
			}
//		}else if( weapon & (WP_BLOW /* WP_BLAST */ ) && !(work->status & VMP_STATUS_SWIM) ){
		}else if( weapon & WP_BLAST && !(work->status & VMP_STATUS_SWIM) ){
			if( !(weapon & WP_CLAYMORE) || work->clmr_timer < 0 ){
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_BLAST, -1, GameDiffDam_BlastDamege[work->game_diff], 0, -1 );
				if( weapon & WP_CLAYMORE ) work->clmr_timer = 8;
			}
			
			DG_COPY_VEC( &work->off_force, &off->power->force );
			DG_COPY_VEC( &work->off_hit, &GM_PlayerPosition );//&off->hit );
			//
			_sceVu0AddVector( &work->off_hit, &off->center, &off->offset );
			if( work->think_sub == 30 /*THK_SUB_STARTONWATER*/){
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_BULLET, -1, GameDiffDam_BlastDamege[work->game_diff], 0, -1 );
			}
		}else if( weapon & (WP_STUNGRENADE|WP_STUNFAR) && !(work->status & VMP_STATUS_SWIM) ){
			
			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_STUN, -1, 0, GameDiffDam_StanDamege[work->game_diff], -1 );
		}
		work->target.class |= TARGET_SKIP;
	}
	if( weapon & WP_WEAPONCORE ){
		//if( off->name == WP_Claymore ){
		CallOffenceWhenThrough( off, def );
		//}
	}


	GM_ClearTargetDamage( def );
}


static void WaterDmgCallBack( TARGET *off, TARGET *def, Work *work )
{
	long64	weapon;
	weapon = def->weapon_type;
	//VMP_PRINTF("水　type[%016lx][%f][%f]!!\n",weapon,(off->offset.vy+off->center.vy),GM_WaterLevel );


	if( weapon & WP_BLAST && (off->offset.vy+off->center.vy) < (GM_WaterLevel+500.0f) &&
		CHECK_FLAG( work->status, VMP_STATUS_SWIM ) && work->o2 > 0){
		FVECTOR bomb_pos;
		float len;
		
		_sceVu0AddVector( &bomb_pos, &off->center, &off->offset );
		//PRINT_PFVEC(0,&bomb_pos);
		_sceVu0SubVector( &bomb_pos, &bomb_pos, &work->control.mov );
		len = _Vu0VecLenXYZ( &bomb_pos );
		if( len < 5000.0f ){
			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_BLAST, -1,
											  GameDiffDam_MaxBlastWaterDamege[work->game_diff],
											  GameDiffDam_MinBlastWaterDamege[work->game_diff],
											  -1 );
		}else{
			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_BLAST, -1,
											  GameDiffDam_MinBlastWaterDamege[work->game_diff],
											  GameDiffDam_MaxBlastWaterDamege[work->game_diff],
											  -1 );
		}
	}
#if 0
	if( weapon & WP_STUN ){
		VMP_PRINTF("wtr target stun\n");
	}
#endif
	GM_ClearTargetDamage( def );
	def->class |= TARGET_SKIP;
}


static void ChildDmgCallBack( TARGET *off, TARGET *def, Work *work )
{
	long64	weapon;

	weapon = def->weapon_type;

	//VMP_PRINTF("子　dmg [%2d]type[%016lx]!!\n", def->name, weapon );

	if(weapon & VMP_CAN_AVOID){
		//off->power->force
		DG_COPY_VEC( &work->off_force, &off->power->force );
		DG_COPY_VEC( &work->off_hit, &def->hit );
	}


	if( (weapon & VMP_CAN_AVOID) &&
		(CHECK_FLAG( work->status, (VMP_STATUS_MOTAVOID|VMP_STATUS_RAPIDAVOID|
									VMP_STATUS_DAMEGE|VMP_STATUS_AVOID|VMP_STATUS_AVOID2) ) ||
		 CHECK_FLAG( work->think_flags, VMP_FLAGS_LOCKON )) ){
		//VMP_PRINTF("state %08x\n", work->status );
		//避けるたり当たらなかったり。
		if( CHECK_FLAG( work->status, VMP_STATUS_RAPIDAVOID ) ){
			//連続回避中
			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_RPD_AVD, -1, 0, 0, -1 );

		}else if( CHECK_FLAG( work->status, VMP_STATUS_SWIM ) ){
			//水中避け（まわす？）
			
		}else if( CHECK_FLAG( work->status, VMP_STATUS_MOTAVOID ) ){
			//モーション事態に回避能力あり
			//取り合えず無敵

		}else if( CHECK_FLAG( work->status, VMP_STATUS_SPIDER ) ){
			//張り付き時
			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SPDR_DAM, def->name&0xff, 0, 0, -1 );

		}else if( CHECK_FLAG( work->status, VMP_STATUS_AVOID2 ) ){
			//前の移動回避へ
			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_AVOID2, -1, 0, 0, -1 );
		}else{
			//回避モーション発動（連続開始）
			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_AVOID1, -1, 0, 0, -1 );
		}
	}else{
		work->target.class |= TARGET_SKIP;
		CallOffenceWhenThrough( off, def );
		if( weapon & WP_PUNCHALL ){
			//熱い拳
			if( CHECK_FLAG( work->status, VMP_STATUS_SPIDER ) ){
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SPDR_DAM, def->name&0xff,
												  (int)(ChildDmgRatio[(def->name>>16)]*GameDiffDam_MaxDamege[work->game_diff]),
												  0, (def->name>>16) );
			}else{
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_PUNCH, def->name&0xff,
												  (int)(ChildDmgRatio[(def->name>>16)]*GameDiffDam_MaxDamege[work->game_diff]),
												  0, (def->name>>16) );
			}
			if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
				NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 強 */
			} else {
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 弱 */
			}

		}else if( weapon & WP_PSG1 ){
			if( CHECK_FLAG( work->status, VMP_STATUS_SWIM ) ){
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SWIM_DAM, def->name&0xff,
												  GameDiffDam_MinDamege[work->game_diff], 0, (def->name>>16) );
			}else if( CHECK_FLAG( work->status, VMP_STATUS_SPIDER ) ){
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SPDR_DAM, def->name&0xff,
												  (int)(ChildDmgRatio[(def->name>>16)]*GameDiffDam_MaxDamege[work->game_diff]),
												  0, (def->name>>16) );
			}else{
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_PSG1, def->name&0xff,
												  (int)(ChildDmgRatio[(def->name>>16)]*GameDiffDam_MaxDamege[work->game_diff]),
												  0, (def->name>>16) );
			}
		}else if( weapon & WP_PSG1T ){
			if( CHECK_FLAG( work->status, VMP_STATUS_SWIM ) ){
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SWIM_DAM, def->name&0xff,
												  0, GameDiffDam_MinDamege[work->game_diff], (def->name>>16) );
			}else if( CHECK_FLAG( work->status, VMP_STATUS_SPIDER ) ){
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SPDR_DAM, def->name&0xff,
												  0, (int)(ChildDmgRatio[(def->name>>16)]*GameDiffDam_MaxDamege[work->game_diff]),
												  (def->name>>16) );
			}else{
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_PSG1, def->name&0xff,
												  0, (int)(ChildDmgRatio[(def->name>>16)]*GameDiffDam_MaxDamege[work->game_diff]),
												  (def->name>>16) );
			}
		}else if( weapon & WP_BULLET ){
			//VMP_PRINTF("state %08x\n", work->status );
			if( CHECK_FLAG( work->status, VMP_STATUS_SWIM ) ){
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SWIM_DAM, def->name&0xff,
												  GameDiffDam_MinDamege[work->game_diff], 0, (def->name>>16) );
			}else if( CHECK_FLAG( work->status, VMP_STATUS_SPIDER ) ){
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SPDR_DAM, def->name&0xff,
												  (int)(ChildDmgRatio[(def->name>>16)]*GameDiffDam_MaxDamege[work->game_diff]),
												  0, (def->name>>16) );
			}else{
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_BULLET, def->name&0xff,
												  (int)(ChildDmgRatio[(def->name>>16)]*GameDiffDam_MaxDamege[work->game_diff]),
												  0, (def->name>>16) );
			}
		}else if( weapon & WP_M92 ){
			if( CHECK_FLAG( work->status, VMP_STATUS_SWIM ) ){
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SWIM_DAM, def->name&0xff,
												  0, GameDiffDam_MinDamege[work->game_diff], (def->name>>16) );
			}else if( CHECK_FLAG( work->status, VMP_STATUS_SPIDER ) ){
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SPDR_DAM, def->name&0xff,
												  0, (int)(ChildDmgRatio[(def->name>>16)]*GameDiffDam_MaxDamege[work->game_diff]),
												  (def->name>>16) );
			}else{
				work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_M9, def->name&0xff,
												  0, (int)(ChildDmgRatio[(def->name>>16)]*GameDiffDam_MaxDamege[work->game_diff]),
												  (def->name>>16) );
			}			
		} else if( weapon & WP_BLADE ){
			//取り合えず
			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_BLADE, def->name&0xff,
											  (int)(ChildDmgRatio[(def->name>>16)]*48.0f), 0, (def->name>>16) );
		}else if( weapon & WP_COLDSPRAY ){
			//取り合えず
			work->dmg_flags = VMP_SET_DMFLAG( VMP_WPDM_SPRAY, def->name&0xff, 0, 0, (def->name>>16) );
		}
	}


	//else if( weapon & WP_NIKITA ){
	//	CallOffenceWhenThrough( off, def );
	//}
	//if(work->life < 0)work->life = 128;
	if( weapon & WP_LASERSIGHT ){
		CallOffenceWhenThrough( off, def );
	}
	if( weapon & WP_WEAPONCORE ){
		//if( off->name == WP_Claymore ){
		CallOffenceWhenThrough( off, def );
		//}
	}
	GM_ClearTargetDamage( def );	
}
#if 0
static void debug_GetNearAttRndPos( FVECTOR *player, int map )
{
#define CHECK_OFFSET	(1800.0f)
#define GO_OFFSET		(1300.0f)

#define	ROOT_2			(1.41421f)
	FVECTOR		check_offset[8] = {
		{  CHECK_OFFSET, 0.0f, 0.0f, 0.0f },
		{ -CHECK_OFFSET, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f,  CHECK_OFFSET, 0.0f },
		{ 0.0f, 0.0f, -CHECK_OFFSET, 0.0f },
		
		{  CHECK_OFFSET/ROOT_2, 0.0f,  CHECK_OFFSET/ROOT_2, 0.0f },
		{ -CHECK_OFFSET/ROOT_2, 0.0f,  CHECK_OFFSET/ROOT_2, 0.0f },
		{  CHECK_OFFSET/ROOT_2, 0.0f, -CHECK_OFFSET/ROOT_2, 0.0f },
		{ -CHECK_OFFSET/ROOT_2, 0.0f, -CHECK_OFFSET/ROOT_2, 0.0f },
	};
	
	FVECTOR		go_offset[8] = {
		{  GO_OFFSET, 0.0f, 0.0f, 0.0f },
		{ -GO_OFFSET, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f,  GO_OFFSET, 0.0f },
		{ 0.0f, 0.0f, -GO_OFFSET, 0.0f },
		
		{  GO_OFFSET/ROOT_2, 0.0f,  GO_OFFSET/ROOT_2, 0.0f },
		{ -GO_OFFSET/ROOT_2, 0.0f,  GO_OFFSET/ROOT_2, 0.0f },
		{  GO_OFFSET/ROOT_2, 0.0f, -GO_OFFSET/ROOT_2, 0.0f },
		{ -GO_OFFSET/ROOT_2, 0.0f, -GO_OFFSET/ROOT_2, 0.0f },
	};
	FVECTOR		from;
	FVECTOR		to;
	int			hzx_result,i,ok[8],ok_cnt = 0;

#if 0
	from.vx = -6100.0f;
	from.vy = -5000.0f;
	from.vz = -243900.0f;
	from.vw = 1.0f;
	
	AN_Test_Eye2( &from, 2 );
#endif
	DG_COPY_VEC( &from, player );
	from.vy = -6000.0f;
	
	//AN_Test_Eye2( &from, 2 );
	for( i = 0; i < 8; i++ ){
		_sceVu0AddVector( &to, &from, &check_offset[i] );
		hzx_result = HZX_OnlineHazardCheck( GM_GetHzxGroupID( map ), &from, &to,
											HZX_CHK_SEGMENT, HZX_SEG_NO_ENEMY, HZX_FLOOR_NO_ENEMY );
#if 1
		{
			FVECTOR	line[2];
			DG_COPY_VEC( &line[0], &from );
			DG_COPY_VEC( &line[1], &to );
			
			NewLineView( line, 1, 160, 32, 140 );
		}
#endif
		if(!(hzx_result)){
			FVECTOR		temp;
			int			seg_num;
			_sceVu0AddVector( &temp, &from, &go_offset[i] );
			seg_num = HZX_NearHazardCheck( GM_GetHzxGroupID( map ), &temp, 400,
										   HZX_CHK_SEGMENT, HZX_TYPE_ENEMY,
										   400 );
			if( !seg_num ){
				AN_Test_Eye2( &temp, 2 );
				ok_cnt++;
			}
		}
		hzx_result = 0;
	}
	if( !ok_cnt ){
		VMP_PRINTF("ERR\n");
		PRINT_PFVEC(0,&from);
	}
	//AN_Test_Eye2( out, 2 );
	
}
#endif
/* --------------------------------------------------------------- */
static void Act( Work *work )
{
	OBJECT		*body;
	CONTROL		*ctrl;

	body = &work->body;
	ctrl = &work->control;

	//body->evmobj->flag ^= DG_EVMOBJ_INVISIBLE;
	
	//コントロール関係
	work->old_body_height = body->height;
	DG_COPY_VEC( &ctrl->step, &VMP_ZeroVector );
#if 0
	VMP_ControlAct( ctrl, body, work->lights, work->old_body_height, work->status,
					&work->think_mot_step, &work->think_mul_step, work->reset_height );
	
	if( CHECK_FLAG( work->status, VMP_STATUS_RESET_H ) ){
		work->old_body_height = work->reset_height;
	}
#else
#if 1
    GM_ActMotion( body );
	//PRINT_PFVEC( work->act_timer, &ctrl->step );
	if( CHECK_FLAG( work->status, VMP_STATUS_RESET_H ) ){
		work->old_body_height = work->reset_height;
//		work->old_body_height = body->height = body->m_ctrl->mt3_ctrl[ 0 ].move->step.vw ;
//		work->reset_height = body->height ;
//printf( "%f\n", body->m_ctrl->mt3_ctrl[ 0 ].move->step.vw ) ;
	}
	VMP_Gravitation( ctrl, body, work->old_body_height, work->status,
					 &work->think_mot_step, &work->think_mul_step, work->reset_height );
//printf( "%f %f %f\n", work->old_body_height, body->height, ctrl->step.vy ) ;	
	GM_ActControl( ctrl );

	MT_SetMotionSeTable( body->m_ctrl, work->map, MT_SETABLE_VAMP, 
						 HZX_GetSeCode( work->control.flr_atrs[ 0 ] ), -1 ) ;
	GM_ActObject2( body );

	DG_GetLightMatrix( &ctrl->mov, work->lights );
    /* ＳＥ変換用 */
    //MT_SetMotionSeTable( entk->act->body->m_ctrl, GM_CurrentMap, (entk->id%4)+1, 
	//					 ( entk->ctrl->flr_atrs[ 0 ] & 0xf0000000 ) >> 28, 0 );
	
#else
    GM_ActMotion( &work->body1 );
    GM_ActMotion( &work->body2 );
	GM_ActMotion( body );
	VMP_Gravitation( ctrl, body, work->old_body_height, work->status, &work->think_mot_step );
	ctrl->height = body->height;
	GM_ActControl( ctrl );
	GM_ActObject2( body );
	GM_ActObject2( &work->body1 );
	GM_ActObject2( &work->body2 );

	DG_GetLightMatrix( &ctrl->mov, work->lights );
#endif
#endif
	VMP_IkControl( work->ik_work, 0 );
	//振動センサー
	GM_VibrateSensor( &work->control, &work->vib_time ) ;	
	//モーション
	VMP_PreAct( work );
	work->ActFunc_call( work, work->act_timer );
	VMP_AfterAct( work );
	//思考前処理
	VMP_PreThink( work );
	//思考本体
	VMP_ThinkMain( work );
	//アジャスト
	VMP_SetAdjustBody( work );
	//防御ターゲット移動
	VMP_MoveTarget( work, Child_Target_Connect );
	//思考後処理
	VMP_AfterThink( work );
	//ピクピク
	VMP_AdjustPiku( work );
	//ナイフ軌跡
	{
		DG_SetPos( work->knf_root );
		DG_PutVector( VmpKnifePos, &work->knf_pos[0], 2 );
	}
	DG_COPY_VEC( &work->knf_att_pos[0], &work->knf_pos[1] );
	DG_COPY_VEC( &work->knf_att_pos[1], (FVECTOR*)BODYWORLD(body,2).m[3] );

	work->sys_timer++;
	if( --work->clmr_timer < 0 ) work->clmr_timer = -1;
	
	//NewLineView( work->knf_att_pos, 2, 160, 32, 140 );

	
#if 1
	{
		extern int PL_PAD_LOCKON ;
		//ロックオンを引き剥がす
		if( GV_PadData[0].status & PL_PAD_LOCKON ){
			//VMP_PRINTF("lock on\n");
			work->homing.status &= ~HOMING_SKIP;
		}else{
			work->homing.status |= HOMING_SKIP;
		}
		work->def_water.class &= ~TARGET_SKIP;
		work->target.class &= ~TARGET_SKIP;
	}
#endif
	//debug_GetNearAttRndPos( &GM_PlayerPosition, ctrl->map );

	//if( !(GV_Time % 2460) ) NewScrDrop( 2400 );
	//HZX_ViewMatrix( &work->body.objs->objs[12].world, 500.0f );
}

static void Die( Work *work )
{
	DG_DisconnectObjs( work->body.objs, work->knf_objs );
	DG_DequeueObjs( work->knf_objs );
	DG_FreeObjs( work->knf_objs );	

	if( work->shdw_objs ) DG_FreeObjs( work->shdw_objs );
	
	GM_RemoveGageSet( &work->gage );
	GM_RemoveGageSet( &work->oxygen );

	GM_FreeTarget( &work->target );
	GM_FreeTarget( &work->def_water );
	TAKABE_FreePuppetIK( work->ik_work );
	GM_FreeControl( &work->control );
	GM_FreeObject( &work->body );
	//GM_FreeObject( &work->body1 );
	//GM_FreeObject( &work->body2 );

	GM_FreeHomingTrg( &work->homing );
	GM_ResetPlayerStatus( PLAYER_INVINCIBLE_PRG ) ;
	g_VMP_Work = NULL;
}

static DG_OBJS *InitItemObjs( int code, FMATRIX *root, FMATRIX *light )
{
	DG_DEF		*def;
	DG_OBJS		*objs;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( code, 'k' ) ) ;
	if(!def){ VMP_PRINTF("ERR!! NO MODEL!!\n"); return NULL; }
	objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE, 0 );
	if(!objs) return NULL;
	if(DG_QueueObjs( objs )<0) return NULL;
	if(root) objs->root = root;
	
	if(light) DG_SetLightMatrix( objs, light );
	return objs;
}

static int ChangeGameLevel( int lvl )
{

   if ( BP_Area_JP() )
   {
//#ifdef JAPANESE_BP_IGNORE()
	   switch( lvl ){
	     case GM_LEVEL_VERYEASY:
		   VMP_PRINTF("-------------- very easy\n");
		   return 0;
	     case GM_LEVEL_EASY:
		   VMP_PRINTF("-------------- easy\n");
		   return 1;
	     case GM_LEVEL_NORMAL:
		   VMP_PRINTF("-------------- normal\n");
		   return 2;
	     case GM_LEVEL_HARD:
		   VMP_PRINTF("-------------- hard\n");
		   return 3;
	     case GM_LEVEL_EXTREME:
	     case GM_LEVEL_E_EXTREME:
		   VMP_PRINTF("-------------- extreme\n");
		   return 4;
	     default:
		   return 0;
	   }
   }
   else
   {
//#else
	   switch( lvl ){
	     case GM_LEVEL_VERYEASY:
		   VMP_PRINTF("-------------- very easy\n");
		   return 0;
	     case GM_LEVEL_EASY:
		   VMP_PRINTF("-------------- easy\n");
		   return 1;
	     case GM_LEVEL_NORMAL:
		   VMP_PRINTF("-------------- normal\n");
		   return 2;
	     case GM_LEVEL_HARD:
		   VMP_PRINTF("-------------- hard\n");
		   return 3;
	     case GM_LEVEL_EXTREME:
	     case GM_LEVEL_E_EXTREME:
		   VMP_PRINTF("-------------- extreme\n");
		   return 4;
	     default:
		   return 0;
	   }
   }
//#endif
}

static int GetResources( Work *work )
{
//	extern void GM_InitRadarControl( RADAR_CTRL *, FVECTOR *, int , int  );
//	extern void GM_RadarSetVRange( RADAR_CTRL *, float , float );
	CONTROL		*ctrl;
	OBJECT		*body;
	FVECTOR		fvtemp;
	TARGET		*deftrg, *child, *level_child ;
	int			i,j,index = 0;

	ctrl = &work->control;
	body = &work->body;
	deftrg = &work->target;

	//ゲームレベルセット
	work->game_diff = ChangeGameLevel( GM_GameLevel );

	// コントロール初期化
	if( GM_InitControl( ctrl, work->name, 0 ) < 0 ) return -1;
	ctrl->hzx_height = 750;
	ctrl->height = 1158.0F;
	GM_ConfigControlHazard( ctrl, 1200, 450, 500 );
	GM_ConfigControlTrapCheck( ctrl );
	GM_ConfigControlMessageCheck( ctrl );
	GM_ConfigControlAddressCheck( ctrl );
	GM_ConfigControlMapCheck( ctrl );
	GM_ConfigControlHzxCheckFlag( ctrl, HZX_TYPE_ENEMY, HZX_FLOOR_NO_ENEMY );
	ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK;

#if 0
	GM_InitObject( &work->body1, MODEL_NAME, OBJECT_FLAG&(~DG_FLAG_SHADOWMAKE) );
	//GM_ConfigControlObject( ctrl, &work->body1 );
	GM_ConfigObjectEvm( &work->body1, MW_MODEL_NAME, DG_EVMOBJ_SEMITRANS );
	GM_ConfigObjectLight( &work->body1, work->lights );
	work->body1.objs->flag |= DG_FLAG_INVISIBLE;
	GM_InitObject( &work->body2, MODEL_NAME, OBJECT_FLAG&(~DG_FLAG_SHADOWMAKE) );
	//GM_ConfigControlObject( ctrl, &work->body2 );
	GM_ConfigObjectEvm( &work->body2, MW_MODEL_NAME, DG_EVMOBJ_SEMITRANS );
	GM_ConfigObjectLight( &work->body2, work->lights );
	work->body2.objs->flag |= DG_FLAG_INVISIBLE;

	work->body1.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
	work->body2.evmobj->flag |= DG_EVMOBJ_INVISIBLE;
#endif

	// 関節型モデルの初期化
	GM_InitObject( body, MODEL_NAME, OBJECT_FLAG );
	//
	if(1){
		DG_DEF		*def ;
		def = ( DG_DEF * )GV_GetCache( GV_CacheID( SHDWMDL_NAME, 'k' ) ) ;
		if ( def != NULL ) {
			work->shdw_objs = DG_MakeObjs( def, DG_FLAG_FINISHCALC, 0 ) ;
			if( !work->shdw_objs ) return -1;
			DG_SetLowObjs( body->objs, work->shdw_objs ) ;
		}
	}

	// object control の関連付け
	GM_ConfigControlObject( ctrl, body );
	// EVMモデル初期化
	GM_ConfigObjectEvm( body, MW_MODEL_NAME, DG_EVMOBJ_IRREACTION );
	GM_ConfigObjectLight( body, work->lights );
	body->objs->flag |= DG_FLAG_INVISIBLE;
	//body->evmobj->flag |= DG_EVMOBJ_INVISIBLE;
	//ターゲット初期化
	GM_SetTarget( deftrg, DEF_TARGET_CLASS, work->map, ENEMY_SIDE, SampleTrgSize, &DG_ZeroVector );

	printf("vamp target crass[%x]\n",DEF_TARGET_CLASS);
	printf("vamp target crass[%x]\n",CHILD_TRG_FLAG);
	GM_SetTargetWeaponType( deftrg, 0 );
	GM_PutTarget( deftrg );
	//work->target.class = DEF_TARGET_CLASS;
	GM_SetTargetCallBack( deftrg, (TARGET_CALLBACK)PrntDmgCallBack, work );
	//NewTargetView( deftrg, 34, 184, 200 );
	child = work->def_child;

	for( j = 0; j < 4; j++ ){
		level_child = child ;
		for( i = 0; i < Child_Level_Num[j]; i++ ){
			GM_SetTarget( child, CHILD_TRG_FLAG, work->map, ENEMY_SIDE,
						  &Child_Target_Size[index], &Child_Target_Shift[index] );
			GM_SetTargetWeaponType( child, 0 );
			GM_SetTargetCallBack( child, (TARGET_CALLBACK)ChildDmgCallBack, work );
			//child->class |= TARGET_POWER;
			GM_SetTargetName( child, Child_Target_Connect[index] | (index<<16) );
			//NewTargetView( child, 200, 34, 184 );
			child++;
			index++;
		}
		if ( i!=0 ) GM_SetTargetParts( deftrg, level_child, i, j );
		//npctrg->trg_level_num[j] = *level_num ;
	}
#if 1
	{
		//水中ターゲット
		static FVECTOR DefWaterPos = { -10000.0f, -12000.0f, -239000.0f, 1.0f };
		static FVECTOR DefWaterSize = { 7000.0f, 5000.0f, 7000.0f, 0.0f };

		//ターゲット初期化
		TARGET		*deftrg_w = &work->def_water;

		GM_SetTarget( deftrg_w, DEF_TARGET_CLASS2, work->map, ENEMY_SIDE, &DefWaterSize, &DefWaterPos );
		GM_SetTargetWeaponType( deftrg_w, 0 );
		GM_PutTarget( deftrg_w );
		GM_SetTargetCallBack( deftrg_w, (TARGET_CALLBACK)WaterDmgCallBack, work );
		//NewTargetView( deftrg_w, 34, 184, 200 );
	}
#endif
//	ASSERT( sum <= 16 ) ;
	
	//モーション初期化
	GM_ConfigObjectMotion( body, 3, BASE_MOTION, MT_FLAG_HUMAN2 );
	GM_ConfigObjectStep( body, &ctrl->step );
	GM_ConfigObjectAction( body, 0, 0, 0, 0x1fffff, 0 );
	GM_ConfigObjectAction( body, 1, 0, 0, 0x0, 0 );
	GM_ConfigObjectAction( body, 2, 0, 0, 0x0, 0 );
#if 0
	GM_ConfigObjectMotion( &work->body1, 3, BASE_MOTION, MT_FLAG_HUMAN2 );
	//GM_ConfigObjectStep( &work->body1, &ctrl->step );
	GM_ConfigObjectAction( &work->body1, 0, 0, 0, 0x1fffff, 0 );
	GM_ConfigObjectAction( &work->body1, 1, 0, 0, 0x0, 0 );
	GM_ConfigObjectAction( &work->body1, 2, 0, 0, 0x0, 0 );
	
	GM_ConfigObjectMotion( &work->body2, 3, BASE_MOTION, MT_FLAG_HUMAN2 );
	//GM_ConfigObjectStep( &work->body2, &ctrl->step );
	GM_ConfigObjectAction( &work->body2, 0, 0, 0, 0x1fffff, 0 );
	GM_ConfigObjectAction( &work->body2, 1, 0, 0, 0x0, 0 );
	GM_ConfigObjectAction( &work->body2, 2, 0, 0, 0x0, 0 );
#endif
	
	work->ik_work = TAKABE_MakePuppetIK( ctrl, body );

	/* 初期位置、方向、モーションのセット */
	if( GCL_GetOption( 'p' ) != NULL ){
		int	vec[ 3 ];
		if ( GCL_NextStr() != NULL ){
			GCL_GetNextIV( vec );
			fvtemp.vx = (float)vec[0];
			fvtemp.vy = (float)vec[1];
			fvtemp.vz = (float)vec[2];
		}
	} else {
		DG_COPY_VEC( &fvtemp, &GM_PlayerPosition );
		fvtemp.vz -= 1500;
	}
	DG_COPY_VEC( &ctrl->mov, &fvtemp );
	ctrl->turn.vy = ctrl->rot.vy = 0;

	GM_ConfigControlHzxHeight( ctrl, 750.0F, (ctrl->mov.vy + 100.0F) ) ;
	GM_ConfigControlMapID( ctrl );
	GM_ConfigObjectAction( body,0, 0, 0, 0x1fffff, 0 );

	// レーダー
	GM_InitRadarControl( &work->radar, &ctrl->mov, RADAR_VISIBLE|RADAR_SIGHT, -1 );
	GM_RadarSetVRange( &work->radar, 2000.0f, -2000.0f );

	//ホーミング
	GM_SetHomingTrg( &work->homing, &BODYWORLD(body,0), body, &ctrl->map, ctrl, 0 );
	GM_PutHomingTrg( &work->homing );

	//ナビシステム
	GM_SetNavi( &work->navigate, ctrl );

	//ライフゲージ
	GM_InitGageSet( &work->gage, "VAMP", 16, 196, 3,
					VAMP_LIFE, VAMP_LIFE,
					0, 30, VAMP_LIFEGAGE_LEVEL );
	work->gage.text_len = 43;	// 見栄えで、えー感じに設定
	//GM_SetGageColor( &work->gage, 0, 0, 0, 31, 63, 192, 31,
	//				 127, 255, 255, 0, 0 );
	GM_SetGageColor( &work->gage, 0,0,0, 40,128,118, 110,190,118, 255,0,0 ) ;
	GM_InitGageM9( &work->gage, VAMP_LIFE, VAMP_LIFE, 0, 30 ) ;
	GM_AppendGageSet( &work->gage );
	GM_VisibleGage( &work->gage );

	//Ｏ２ゲージ
#if 1
	GM_InitGageSet( &work->oxygen, "VAMP O2", 16, 196, 3,
					COUNT_VMODE( VAMP_O2 ), COUNT_VMODE( VAMP_O2 ),
					0, 30, VAMP_O2GAGE_LEVEL );
	work->oxygen.text_len = 72;	// 見栄えで、えー感じに設定
	GM_SetGageColor( &work->oxygen, 0, 0, 0, 31, 63, 192, 31, 127, 255, 255, 0, 0 );
	GM_AppendGageSet( &work->oxygen );
	GM_VisibleGage( &work->oxygen );
	GM_InvisibleGage( &work->oxygen );
#endif

	
	if( GCL_GetOption( 'v' ) != NULL ){
		for( i = 0; i < N_MAX_VMP_VOX; i++ ){
			if ( GCL_NextStr() == NULL ) break;
			work->vox_id[i] = GCL_GetNextInt();
			work->vox_len[i] = (GCL_GetNextInt() / TIME_BASE) + 1;

			printf("id %x: len %d\n",work->vox_id[i],work->vox_len[i]);
			work->n_vox++;
		}
		for( ; i < N_MAX_VMP_VOX; i++ ){
			work->vox_id[i] = -1;
			work->vox_len[i] = 0;
		}
	}else{
		for( i = 0; i < N_MAX_VMP_VOX; i++ ){
			work->vox_id[i] = -1;
			work->vox_len[i] = 0;
		}
	}

	work->end_proc = GCL_GetOptionValue( 'e', 0 );

	//work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;
	{
		//投げナイフ初期化
		extern void NewVampKnife( int, int );
		NewVampKnife( 0, work->map );
	}

	{
		static FVECTOR	AttSize = { 300.0f, 300.0f, 300.0f, 0.0f };
		TARGET			*trg = &work->att_trgt;
		POWER_TARGET	*pow_trg = &work->att_pow;

#define ATT_TRG_CLASS	(TARGET_OFFENSE|TARGET_SEEK)

		GM_SetTarget( trg, ATT_TRG_CLASS, work->map, PLAYER_SIDE, &AttSize, &DG_ZeroVector  );
		GM_SetTargetWeaponType( trg, WP_BLAST );
		GM_SetPowerTarget( trg, pow_trg, POWER_CONST, 255, 0, 10, &DG_ZeroVector );
		//GM_PutTarget( trg );
		GM_SetTargetCallBack( trg, (TARGET_CALLBACK)KnifeDmgCallBack2, work );
		//NewTargetView( trg, 34, 184, 200 );
	}

	{
		//セクシーナイフ初期化
		//static FVECTOR	KnifeSize = { 10.0f, 30.0f, 175.0f, 0.0f };
		//static FVECTOR	KnifeShift = { 17.5f, -100.0f, -75.0f, 1.0f };
		TARGET			*trg = &work->knf_trgt;
		POWER_TARGET	*pow_trg = &work->knf_pow;
		void			*child;

#define KNF_TRG_CLASS	(TARGET_OFFENSE|TARGET_SEEK|TARGET_ONLINE)

		work->knf_objs = InitItemObjs( KNIFE_MODL_NAME, &BODYWORLD(&work->body,6), work->lights );
		if(!work->knf_objs) return -1;
		DG_ConnectObjs( body->objs, work->knf_objs ) ;
		work->knf_root = &BODYWORLD(&work->body,6);

		DG_InvisibleObjs( work->knf_objs );

		GM_SetTarget( trg, KNF_TRG_CLASS, work->map, PLAYER_SIDE, &DG_ZeroVector, &DG_ZeroVector  );
		//GM_SetTargetWeaponType( trg, WP_BLADE );
		GM_SetTargetWeaponType( trg, WP_BLAST );
		GM_SetPowerTarget( trg, pow_trg, POWER_CONST, 255, 0, GameDiffDam_SlushDamege[work->game_diff], &DG_ZeroVector );
		//GM_PutTarget( trg );
		GM_SetTargetCallBack( trg, (TARGET_CALLBACK)KnifeDmgCallBack, work );

		//NewTargetView( trg, 34, 184, 200 );
		DG_SetPos( work->knf_root );
		DG_PutVector( VmpKnifePos, work->knf_pos, 2 );

		if(1){
			extern void *NewBladeEft( FVECTOR *pos0, FVECTOR *pos1, int n_disp, int init_col, int *alpha );
			work->knf_alpha = 0;
			//child = NewBladeEft( &work->knf_pos[0], &work->knf_pos[1], 8, 0x040a0e00, &work->knf_alpha );
			child = NewBladeEft( &work->knf_pos[0], &work->knf_pos[1], 14, 0x10283000, &work->knf_alpha );
			if(child) GV_SetActorChild( work, child );
		}
	}
	{
		extern void *NewActionBlurEffect( OBJECT *body, u_long64 mask, int *enable_flag );
		work->motblur = NewActionBlurEffect( body, 0x1fffff, &work->motblur_flag );
	}
	if(0){
		extern void *NewAutoSplush( FVECTOR *pos, float radius );
		NewAutoSplush( &ctrl->mov, 1000.0f );
	}
	{
		extern void *NewEvmHairModel_called(int name,int model_name,int sample_num,
											 OBJECT *target,int objnum, FVECTOR *, SVECTOR *,
											 float oval_param,int collision_flag,unsigned char *collision_objs,
											 int visible_flag,int light_flag,int boundmodel_name);
		char	bound_obj_num[] = { 0, 13, 17 };

//	void			*all_knf_work;
#ifdef BP_PSX2_GCC //yano
		work->all_knf_work = NewEvmHairModel_called( ALL_KNF_NAME, ALL_KNFMDL_NAME, 10,
												 body, 0, &DG_ZeroVector, &(SVECTOR){ 0, 0, 0 , 0 },
												 1.2f, 3, bound_obj_num,
												 0, 0, BOUND_MDL_NAME );

		work->knf_work = NewEvmHairModel_called( PARTS_KNF_NAME, PARTS_KNFMDL_NAME, 10,
												 body, 0, &DG_ZeroVector, &(SVECTOR){ 0, 0, 0 , 0 },
												 1.2f, 3, bound_obj_num,
												 0, 0, BOUND_MDL_NAME );
#else
		{
		SVECTOR dmdm = { 0, 0, 0 , 0 };
		work->all_knf_work = NewEvmHairModel_called( ALL_KNF_NAME, ALL_KNFMDL_NAME, 10,
												 body, 0, &DG_ZeroVector, &dmdm,
												 1.2f, 3, bound_obj_num,
												 0, 0, BOUND_MDL_NAME );

		work->knf_work = NewEvmHairModel_called( PARTS_KNF_NAME, PARTS_KNFMDL_NAME, 10,
												 body, 0, &DG_ZeroVector, &dmdm,
												 1.2f, 3, bound_obj_num,
												 0, 0, BOUND_MDL_NAME );

		}
#endif


		GV_SetActorChild( work, work->all_knf_work );
		GV_SetActorChild( work, work->knf_work );

		VMP_KnfObjMesg( PARTS_KNF_NAME, 0, 0 );
		//VMP_KnfObjMesg( ALL_KNF_NAME, 0, 0 );

	}
	/*
	if(0){
		GV_SetActorChild( work, NewVampHola( work->body.objs ) );
	}
	*/
	//NewVampShdwTrgt( 0, work->map );
	VMP_InitAction( work );
	VMP_ThinkInit( work );

	//void *NewScrGoggles_demo( int mode, int camera_num );
	//NewScrGoggles_demo( 0, 0 );
	work->vib_time = 0;

	VMP_LifeCheck = 0 ;

	return 0;
}

void *NewSuperVamp( int name, int map )
{	
	Work		*work ;

	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;
		work->map = map;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
		g_VMP_Work = work;
	}
	return (void *)work ;
}

OBJECT *VMP_GetVampBody()
{
	if( !g_VMP_Work ) return NULL;

	return &g_VMP_Work->body;
}

int VMP_GetVmpStatus(){
	return VMP_LifeCheck ;
}

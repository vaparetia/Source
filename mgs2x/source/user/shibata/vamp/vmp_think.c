//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	test_vamp.c
	ヴァンプさん思考関係
	2001/03/02

	$Id: vmp_think.c,v 1.1.1.3 2002/11/19 11:48:57 Yoshizawa1 Exp $
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

#include "BP_Misc.h"

#ifdef TS_DEBUG_MODE
#include "debugmenu.h"
static int _VMP_DebugFlags = 0;
#endif

extern int BP_FRAMES_PER_SEC();

extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );
//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )


static FVECTOR	VMP_ZeroVector = { 0.0f, 0.0f, 0.0f, 0.0f };

enum {
	THK_MAIN_NON = 0,
	THK_MAIN_KNF,
};


enum {
	THK_SUB_MAIN1_F = 0,			//使用確認
	THK_SUB_MAIN3_F,				//使用確認
	THK_SUB_MAIN1_W,				//使用確認
	THK_SUB_MAIN2_W,				//使用確認
	THK_SUB_MAIN3_W,				//使用確認

	// 10
	THK_SUB_DIVE_FLOOR = 10,		//床から水へ						使用確認
	THK_SUB_DIVE_LOFT,				//2階から水へ						使用確認
	THK_SUB_DIVE_RAIL,				//手摺から水へ						未使用保存
	THK_SUB_DIVEOUT_FAR,			//プレイヤの反対に潜って出る		使用確認
	THK_SUB_DIVEOUT_NEAR,			//プレイヤの近くに潜って出る		未使用保存
	THK_SUB_OUTATT_NEAR,			//プレイヤの近くに出て攻撃			保存
	THK_SUB_AVOID_ATT_FAR2,			//ラストフェーズ					使用確認
	THK_SUB_AVOID_ATT_FARLOFT,		//２階ナイフ投げ					使用確認
	THK_SUB_AVOID_ATT_NEAR,			//プレイヤの近寄って攻撃			使用確認
	THK_SUB_WATERLOFTWATER_ATT,		//2階行って攻撃	（くるくる投げ）	使用確認
	
	// 20
	THK_SUB_JMPOUT_ON_WATER_ATT,	//しゅびびび						一時保存
	THK_SUB_SHADOW_ATT,				//影攻撃							使用確認
	THK_SUB_SWIMING,				//遊泳		水から					使用確認
	THK_SUB_SWIMINGEX,				//遊泳		水から					使用確認
	THK_SUB_OUTNEARRNDATT,			//近距離ランダムアタック			使用確認
	THK_SUB_WATEROUTWATER,			//いるかちゃん		水から			保存
	THK_SUB_WATEROUTSPIDE,			//ハリツッキー		水から			保存
	THK_SUB_GROUND_ATT,				//プレイヤーが寝てるとき			使用確認
	THK_SUB_JUMPOUTLOFT_WALK,		//２階手摺で歩いて投げる			使用確認
	THK_SUB_JUMPLOFTTOFLOOR,		//床からへ		２階から			保存
	// 30
	THK_SUB_STARTONWATER,			//スタート水の上から				使用確認
	THK_SUB_BLOW,					//爆発吹き飛ばされ					使用確認
	//ラストダメージ系
	THK_SUB_LAST_DAM_FLR = 40,		//ラストダメージ：床
	THK_SUB_LAST_DAM_LFT,			//ラストダメージ：２階
	THK_SUB_LAST_BLASTDAM_FLR,		//ラスト爆ダメージ：床
	THK_SUB_LAST_BLASTDAM_LFT,		//ラスト爆ダメージ：２階
	// 特殊
	THK_SUB_DEADEND = 66,			//									取り合えず
	THK_SUB_TEST_THINK = 666,		//テスト用
};


#define DIVE_MOT_TIME		DIRECT_TICK(58)
#define JUMPOUT_MOT_TIME	DIRECT_TICK(75)
#define JUMPOUT_MOT_TIME30	DIRECT_TICK(30)

#define GET_MUL_STEP(_len,_cb,_ad)	( ((_len)+((_cb)-(_ad)))/(_cb) )

#define LOFT2LOFT_DIFF0			(535.34460f - 262.44836f)
#define LOFT2LOFT_DIFF1			(2057.51831f - -19.58925f)
#define LOFT2RAIL_DIFF0			(535.34460f - 262.44836f)
#define LOFT2RAIL_DIFF1			(1055.12341f - -19.58925f)
#define LOFT2FLOOR_DIFF0		(535.34460f - 262.44836f)
#define LOFT2FLOOR_DIFF1		(1057.68896f - -19.58925f)
#define LOFT2WATER_DIFF0		(535.34460f - 262.44836f)
#define LOFT2WATER_DIFF1		(1075.86767f - -19.58925f)

#define RAIL2RAIL_DIFF0			(535.34460f - 282.08856f)
#define RAIL2RAIL_DIFF1			(1055.12341f - -19.58925f)
#define RAIL2FLOOR_DIFF0		(535.34460f - 282.08856f)
#define RAIL2FLOOR_DIFF1		(1057.68896f - -19.58925f)
#define RAIL2WATER_DIFF0		(535.34460f - 282.08856f)
#define RAIL2WATER_DIFF1		(1075.86767f - -19.58925f)

#define FLOOR2LOFT_DIFF0		(535.34460f - 273.28521f)
#define FLOOR2LOFT_DIFF1		(2057.51831f - -19.58925f)
#define FLOOR2RAIL_DIFF0		(535.34460f - 273.28521f)
#define FLOOR2RAIL_DIFF1		(1055.12341f - -19.58925f)
#define FLOOR2FLOOR_DIFF0		(535.34460f - 273.28521f)
#define FLOOR2FLOOR_DIFF1		(1057.68896f - -19.58925f)


#define WATER2LOFT_DIFF0		(535.34460f - -5.70972f)
#define WATER2LOFT_DIFF1		(2057.51831f - -5.70972f)
#define WATER2RAIL_DIFF0		(535.34460f - -5.70972f)
#define WATER2RAIL_DIFF1		(1055.12341f - -5.70972f)
#define WATER2WALL_DIFF0		(535.34460f - -5.70972f)
#define WATER2WALL_DIFF1		(2542.16845f - -5.70972f)

#define FLR2FLR_SHORT_DIFF		(2281.63037f - 5.59906f)
#define FLR2WLL_SHORT_DIFF		(1533.48901 - 18.09920f)			//-1533.48901 <-> -18.09920f
#define FLR2WTR_BLOW_DIFF		(4693.88427f + 15.50078f)			//-4693.88427 <-> 15.50078
#define WTR2FLR_SHORT_DIFF		(2427.38476f - 5.70972f )	//-2427.38476 <-> -5.70972

#define WATER2LOFT_DIFF		(535.34460f)
#define WATER2LOFT_LAST		(2057.51831f - WATER2LOFT_DIFF)

#define WATER2RAIL_DIFF		(2146.28515f)
#define WATER2RAIL_LAST		(2459.78540f - WATER2RAIL_DIFF)

#define LOFT2WATER_DIFF		(535.34460f - 265.39343f)
#define LOFT2WATER_LAST		(1075.86767f - LOFT2WATER_DIFF)

#define LOFT2FLOOR_DIFF		(535.34460f - 265.39343f)
#define LOFT2FLOOR_LAST		(1057.68896f - LOFT2FLOOR_DIFF)

#define FLOOR2LOFT_DIFF		(535.34460f - 264.81127f)
#define FLOOR2LOFT_LAST		(2057.51831f - FLOOR2LOFT_DIFF)

#define LOFT2LOFT_DIFF		(535.34460f - 265.39343f)
#define LOFT2LOFT_LAST		(2057.51831f - LOFT2LOFT_DIFF)

#define FLOOR2FLOOR_DIFF		(535.34460f - 264.81127f)
#define FLOOR2FLOOR_LAST		(2057.51831f - FLOOR2LOFT_DIFF)

static int GameDiff_SpiderWait[5] = {
	/*DIRECT_TICK*/(200),
	/*DIRECT_TICK*/(160),
	/*DIRECT_TICK*/(100),
	/*DIRECT_TICK*/(80),
	/*DIRECT_TICK*/(60),
};

/*
static int GameDiff_SpiderWait[5] = {
   DIRECT_TICK(200),
   DIRECT_TICK(160),
   DIRECT_TICK(100),
   DIRECT_TICK(80),
   DIRECT_TICK(60),
};
*/

/* ------------------------------------------------------------------------------------

	思考部分

------------------------------------------------------------------------------------ */

//第１フェーズ
static void VMP_ThinkLocal_Main1_F( Work *work )
{
	if( work->think_timer >= 0 ){
		//VMP_SetThinkSub( work, THK_SUB_STARTONWATER, 0 );
		VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
	}
}

#define		MODE_CHANGE_LIFE0	(70)	//第三へ
#define		MODE_CHANGE_LIFE1	(20)	//ラストへ

static void VMP_ThinkLocal_Main1_W( Work *work )
{
	int think_table[] = {
		THK_SUB_JUMPOUTLOFT_WALK,
		THK_SUB_SWIMING,

		THK_SUB_JUMPOUTLOFT_WALK,
		THK_SUB_SWIMING,
		THK_SUB_SWIMING,
		THK_SUB_SWIMING,
	};

	VMP_SetThinkSub( work, think_table[work->think_ntbl%2], 0 );
	if( ++work->think_ntbl >= 1 ){
		work->think_now_main_w = THK_SUB_MAIN2_W;
		work->think_ntbl = 0;
	}
}

//第２フェーズ
#if 0
static void VMP_ThinkLocal_Main2_F( Work *work )
{
	if( work->think_timer >= 0 ){
		VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
	}
}
#endif
static void VMP_ThinkLocal_Main2_W( Work *work )
{
	int think_table[] = {
		THK_SUB_SWIMING,
		THK_SUB_OUTNEARRNDATT,
		THK_SUB_AVOID_ATT_FARLOFT,
		
		THK_SUB_AVOID_ATT_FARLOFT,
		THK_SUB_AVOID_ATT_FARLOFT,
		
		THK_SUB_WATERLOFTWATER_ATT,
		THK_SUB_AVOID_ATT_FARLOFT,
		THK_SUB_SWIMINGEX,
		THK_SUB_WATEROUTSPIDE,//THK_SUB_WATEROUTWATER,
		THK_SUB_AVOID_ATT_FARLOFT,
	};
	VMP_SetThinkSub( work, think_table[work->think_ntbl%3], 0 );
	work->think_ntbl++;

	if( work->life < MODE_CHANGE_LIFE0 || work->faint < MODE_CHANGE_LIFE0 ){
		work->think_now_main_w = THK_SUB_MAIN3_W;
		VMP_SetThinkSub( work, work->think_now_main_w, 0 );
		work->think_ntbl = 0;
	}
}

//第３フェーズ
#if 1
static void VMP_ThinkLocal_Main3_F( Work *work )
{
	if( work->think_timer >= 0 ){
		VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_FAR2, 0 );
		//VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_NEAR, 0 );
	}
}
#endif

static void VMP_ThinkLocal_Main3_W( Work *work )
{
	if( work->life <= MODE_CHANGE_LIFE1 || work->faint <= MODE_CHANGE_LIFE1 ){
		VMP_SetThinkSub( work, THK_SUB_DIVEOUT_FAR, 0 );
	}else{
		int think_table[] = {
			THK_SUB_OUTNEARRNDATT,
			THK_SUB_AVOID_ATT_FARLOFT,
			//THK_SUB_AVOID_ATT_FARLOFT,
		};
		//GV_CallChildSignalFunc( work, 0, 0 );
		VMP_SetThinkSub( work, think_table[work->think_ntbl%2], 0 );
		work->think_ntbl++;
		
	}
	
	if( work->think_now_main_f != THK_SUB_MAIN3_F && (work->life <= MODE_CHANGE_LIFE1 || work->faint <= MODE_CHANGE_LIFE1) ){
		VMP_SetThinkSub( work, THK_SUB_DIVEOUT_FAR, 0 );
		work->think_now_main_f = THK_SUB_MAIN3_F;
	}
}


/* --------------------------------------------------------------------------------- */


static void VMP_ThinkThrwKnfLine( Work *work )
{
#if 0
	static FVECTOR knf_att_pod[] = {
		{ -4000.0f, 0.0f, 0.0f, 1.0f },
		{ 4000.0f, 0.0f, 0.0f, 1.0f },
	};

	static FVECTOR to_pos[] = {
		{ -13400.0f, -5000.0f, -239000.0f, 1.0f },
		
		{ -10000.0f, -5000.0f, -242400.0f, 1.0f },
		{ -10000.0f, -5000.0f, -242400.0f, 1.0f },
		
		{  -6400.0f, -5000.0f, -239000.0f, 1.0f },
		
		{ -10000.0f, -5000.0f, -235500.0f, 1.0f },
		{ -10000.0f, -5000.0f, -235500.0f, 1.0f },
	};
	
	static FVECTOR from_pos[] = {
		{ -4700.0f, -5000.0f, -239000.0f, 1.0f },
		
		{ -10000.0f, -5000.0f, -233950.0f, 1.0f },
		{ -10000.0f, -5000.0f, -233950.0f, 1.0f },
		
		{ -15300.0f, -5000.0f, -239000.0f, 1.0f },
		
		{ -10000.0f, -5000.0f, -244550.0f, 1.0f },
		{ -10000.0f, -5000.0f, -244550.0f, 1.0f },
	};

	FVECTOR hit;
	FMATRIX	world;

	//if( work->think_n_att >= 8 ) return;
	_sceVu0SubVector( &hit, &to_pos[work->think_v_loft], &from_pos[work->think_v_loft] );

	TS_MakeMatrix( &world, &hit, &to_pos[work->think_v_loft] );

	_sceVu0SubVector( &hit, &knf_att_pod[1], &knf_att_pod[0] );
	_sceVu0ScaleVector( &hit, &hit, (float)(work->think_n_att+1)/(float)work->think_tmp_max_att );
	_sceVu0AddVector( &hit, &hit, &knf_att_pod[0] );

	_RotTrans( &work->think_init_pos, &world, &hit );
	//_RotTrans( &hit, &world, &hit );

	//ThrowKnife(work, NULL, &hit, NULL, 10, 0, 0 );

	VMP_PRINTF("knf_num [%d]\n",work->think_n_att);
	if(0){
		FVECTOR		line[2];
		DG_COPY_VEC( &line[0], (FVECTOR*)(BODYWORLD(&work->body,12).m[3]) );
		DG_COPY_VEC( &line[1], (FVECTOR*)(BODYWORLD(GM_PlayerBody,12).m[3]) );
		NewLineView( line, 1, 160, 32, 140 );
	}
#else
	FVECTOR hit;

	hit.vy = -3200.0f;
	hit.vw = 1.0f;
	
	switch(work->think_v_loft){
	  case 5:
	  case 0:
	  case 3:
		hit.vx = -10000.0f;
		hit.vz = work->control.mov.vz;
		break;
	  case 1:
	  case 2:
	  case 4:
		hit.vx = work->control.mov.vx;
		hit.vz = -239000.0f;
		break;
	}
	DG_COPY_VEC( &work->think_init_pos, &hit );
	//_RotTrans( &hit, &world, &hit );

	//ThrowKnife(work, NULL, &hit, NULL, 10, 0, 0 );

	//VMP_PRINTF("knf_num [%d]\n",work->think_n_att);
#endif
}


/* ------------------------------------------------------------------------------------

補助関数軍

------------------------------------------------------------------------------------ */

//移動（ライデン）
static void VMP_ThinkLocal_Non_MoveRai( Work *work )
{
	if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
		ClearAjustToPlayer( work );
		return;
	}
	work->dir = VMP_GetFromZoneToZoneDirFloor( &work->control.mov,
											work->think_v_fzn, work->think_p_fzn, 200.0f, &GM_PlayerPosition );


	//へそと胸
	SetAjustToPlayer( work );
}

//水中飛び込み
static void VMP_ThinkLocal_Non_Dive( Work *work )
{
	//work->think_mot_step.vy += 2.0f*-2500.0f/(float)(DIVE_MOT_TIME*DIVE_MOT_TIME);

	if( work->think_timer == 0 ){
		FVECTOR	fvtemp;

		VMP_DivePos( &fvtemp, work->think_dive_pos );
		GetInitialVelocity( &work->think_mot_step,
							&work->control.mov,
							&fvtemp,
							(float)DIVE_MOT_TIME );
	}

}

//床への飛び出し
static void VMP_ThinkLocal_Non_JumpOutFloor( Work *work )
{
//	if( !CHECK_FLAG( work->status, VMP_STATUS_NO_DIR ) ){
//		work->dir = (work->think_dir+2048)%4096;
//	}

	// WTR2FLR_SHORT_DIFF
	if( work->think_timer == 0 ){
		FVECTOR	fvtemp;
		SVECTOR		rot;
		
		VMP_JumpOutPos( &work->think_init_pos, work->think_jumpout_pos );
		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		work->think_mul_x = work->think_mul_z =
			bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz)/WTR2FLR_SHORT_DIFF;   //BP_MATH - emulate PS2 sqrtf
			//GET_MUL_STEP(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz),WTR2FLR_SHORT_DIFF,0.0f);   //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;

		_sceVu0Normalize( &fvtemp, &fvtemp );
		TS_VecToRot( &rot, &fvtemp );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
		//VMP_PRINTF( "init %d to_rai %d\n", work->dir,work->think_dir);		
		work->dir = (work->dir+2048)%4096;
		VMP_PRINTF("get param mul xz[%f]\n", work->think_mul_x );
	}
	//work->think_mot_step.vy += 2.0f*1500.0f/(float)(JUMPOUT_MOT_TIME30*JUMPOUT_MOT_TIME30);
}

//ジャンプ
static int VMP_ThinkLocal_Non_FloorJump( Work *work )
{
	if( work->think_timer == 0 ){
		FVECTOR	fvtemp;
		SVECTOR rot;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		TS_VecToRot( &rot, &fvtemp );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
		//printf("dir = %d\n",work->dir);
	}else if( work->think_timer == DIRECT_TICK(36) ){
		FVECTOR	fvtemp;
		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		work->think_mul_x = work->think_mul_z =
			GET_MUL_STEP(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz),FLOOR2FLOOR_DIFF0,FLOOR2FLOOR_DIFF1);  //BP_MATH - emulate PS2 sqrtf
			//(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz) - FLOOR2FLOOR_LAST)/FLOOR2FLOOR_DIFF;   //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;
		SET_FLAG( work->act_flags, VMP_ACTFLAG_CHANGE_MUL );
	}
	if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
		work->think_local = -1;
		return -1;
	}
	return 0;
}

//ジャンプ
static void VMP_ThinkLocal_Non_NearFloorJump_1st( Work *work )
{
	if( work->think_timer == 0 ){
		FVECTOR	floor;
		_sceVu0SubVector( &floor, &GM_PlayerPosition, &work->control.mov );
		_sceVu0ScaleVector( &floor, &floor, 0.6f );
		_sceVu0AddVector( &floor, &floor, &work->control.mov );
		GetInitialVelocity( &work->think_mot_step,
							&work->control.mov,
							&floor,
							23.0f );
		_sceVu0SubVector( &floor, &floor, &work->control.mov );
		work->dir = (int)(2048.0f * atan2f( floor.vx , floor.vz ) / PI);
		if( work->dir < 0 ) work->dir += 4096;
	}
}

//ジャンプ
static void VMP_ThinkLocal_Non_NearFloorJump_2nd( Work *work )
{	
	work->dir = work->think_dir;
	if( work->think_timer == 0 ){
		FVECTOR	floor;
		if( GetNearAttRndPos( &floor, &GM_PlayerPosition, work->control.map ) ){
			floor.vx = -6100.0f;
			floor.vy = -5000.0f;
			floor.vz = -243900.0f;
			floor.vw = 1.0f;
		}
		if( CHECK_FLAG( work->think_flags, VMP_FLAGS_NO_ZONE) ){
			//多分真中にいると思う。
			VMP_GetZoneNumFromForce( &floor, &work->control.mov, &work->control.step );
			VMP_PRINTF("********************* CALL Shibata Please ****************\n");
		}
		GetInitialVelocity( &work->think_mot_step,
							&work->control.mov,
							&floor,
							15.0f );
	}
}

//２階への飛び出し
static int VMP_ThinkLocal_Non_JumpOutLoftRnd( Work *work )
{

	if( work->think_timer == 0 ){
		FVECTOR		loft;
		SVECTOR		rot;
		
		//DG_COPY_VEC( &work->think_init_pos, &loft );

		_sceVu0SubVector( &loft, &work->think_init_pos, &work->control.mov );

		work->think_mul_x = work->think_mul_z =
			GET_MUL_STEP(bp_sqrtf(loft.vx*loft.vx+loft.vz*loft.vz),WATER2LOFT_DIFF0,WATER2LOFT_DIFF1);   //BP_MATH - emulate PS2 sqrtf
			//(bp_sqrtf(loft.vx*loft.vx+loft.vz*loft.vz) - WATER2LOFT_LAST)/WATER2LOFT_DIFF; //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;

		_sceVu0Normalize( &loft, &loft );
		TS_VecToRot( &rot, &loft );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
		//VMP_PRINTF( "init %d to_rai %d\n", work->dir,work->think_dir);
	}
	if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
		work->think_local = -1;
		return -1;
	}
	return 0;

}

//ロフトからロフトへ（短い版）
static void VMP_ThinkLocal_Loft2LoftShort( Work *work )
{
	if( work->think_timer == 0 ){
		FVECTOR		loft;
		SVECTOR		rot;

		//DG_COPY_VEC( &work->think_init_pos, &loft );
		_sceVu0SubVector( &loft, &work->think_init_pos, &work->control.mov );

      if ( BP_IsPAL()==TRUE )
		   _sceVu0ScaleVector( &work->think_mot_step, &loft, 1.0f/24.0f );
      else
	      _sceVu0ScaleVector( &work->think_mot_step, &loft, 1.0f/30.0f );

      work->think_mot_step.vy = 0.0f;
		
		_sceVu0Normalize( &loft, &loft );
		TS_VecToRot( &rot, &loft );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
		//VMP_PRINTF( "init %d to_rai %d\n", work->dir,work->think_dir);
	}
}


//床から床（短い版）
static void VMP_ThinkLocal_Flr2FlrShort( Work *work )
{
	if( work->think_timer == 0 ){
		FVECTOR		loft;
		SVECTOR		rot;

		//DG_COPY_VEC( &work->think_init_pos, &loft );
		_sceVu0SubVector( &loft, &work->think_init_pos, &work->control.mov );
		work->think_mul_x = work->think_mul_z =
			bp_sqrtf(loft.vx*loft.vx+loft.vz*loft.vz)/FLR2FLR_SHORT_DIFF;  //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;

		_sceVu0Normalize( &loft, &loft );
		TS_VecToRot( &rot, &loft );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
		//VMP_PRINTF( "init %d to_rai %d\n", work->dir,work->think_dir);
	}
}

//床から壁（短い版）
static void VMP_ThinkLocal_Flr2WllShort( Work *work )
{
	if( work->think_timer == 0 ){
		FVECTOR		loft;
		SVECTOR		rot;

		_sceVu0SubVector( &loft, &work->think_init_pos, &work->control.mov );
		work->think_mul_x = work->think_mul_z =
			bp_sqrtf(loft.vx*loft.vx+loft.vz*loft.vz)/FLR2WLL_SHORT_DIFF;  //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;

		_sceVu0Normalize( &loft, &loft );
		TS_VecToRot( &rot, &loft );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = (rot.vy+2048)%4096;
		
		//work->dir = (work->think_jumpout_pos*2048);
	}
}

//１階手摺から床へ
static int VMP_ThinkLocal_Non_JumpHndRailToFloor( Work *work )
{	
	if( work->think_timer == 0 ){
		FVECTOR	fvtemp;
		SVECTOR rot;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		TS_VecToRot( &rot, &fvtemp );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
		//printf("dir = %d\n",work->dir);
	}else if( work->think_timer == DIRECT_TICK(36) ){
		FVECTOR	fvtemp;
		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		work->think_mul_x = work->think_mul_z =
			GET_MUL_STEP(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz),RAIL2FLOOR_DIFF0,RAIL2FLOOR_DIFF1); //BP_MATH - emulate PS2 sqrtf
			//(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz) - RAIL2FLOOR_LAST)/RAIL2FLOOR_DIFF;  //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;
		SET_FLAG( work->act_flags, VMP_ACTFLAG_CHANGE_MUL );
	}
	if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
		work->think_local = -1;
		return -1;
	}
	return 0;
}

//２階へのジャンプ
static int VMP_ThinkLocal_Non_JumpToLoft( Work *work )
{
//	if( !CHECK_FLAG( work->status, VMP_STATUS_NO_DIR ) ){
//		work->dir = (work->think_dir+2048)%4096;
//		work->dir = work->think_dir;
//	}
//	printf("a\n");
	if( work->think_timer == 0 ){
		FVECTOR	fvtemp;
		SVECTOR rot;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		TS_VecToRot( &rot, &fvtemp );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
		//printf("dir = %d\n",work->dir);
	}else if( work->think_timer == DIRECT_TICK(36) ){
		FVECTOR	fvtemp;
		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		work->think_mul_x = work->think_mul_z =
			GET_MUL_STEP(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz),FLOOR2LOFT_DIFF0,FLOOR2LOFT_DIFF1); //BP_MATH - emulate PS2 sqrtf
			//(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz) - FLOOR2LOFT_LAST)/FLOOR2LOFT_DIFF;  //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;
		SET_FLAG( work->act_flags, VMP_ACTFLAG_CHANGE_MUL );
	}
	if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
		work->think_local = -1;
		return -1;
	}
	return 0;
}
#if 0
//床から一階手摺へのジャンプ
static int VMP_ThinkLocal_Non_FloorToHndRail( Work *work )
{
	if( work->think_timer == 0 ){
		FVECTOR	fvtemp;
		SVECTOR rot;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		TS_VecToRot( &rot, &fvtemp );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
		//printf("dir = %d\n",work->dir);
	}else if( work->think_timer == 36 ){
		FVECTOR	fvtemp;
		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		work->think_mul_x = work->think_mul_z =
			GET_MUL_STEP(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz),FLOOR2RAIL_DIFF0,FLOOR2RAIL_DIFF1); //BP_MATH - emulate PS2 sqrtf
			//(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz) - FLOOR2LOFT_LAST)/FLOOR2LOFT_DIFF;  //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;
		SET_FLAG( work->act_flags, VMP_ACTFLAG_CHANGE_MUL );
	}
	if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
		work->think_local = -1;
		return -1;
	}
	return 0;
}
#endif
//２階から２階
static int VMP_ThinkLocal_Non_JumpLoft( Work *work )
{
	
	if( work->think_timer == 0 ){
		FVECTOR	fvtemp;
		SVECTOR rot;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		TS_VecToRot( &rot, &fvtemp );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
	}else if( work->think_timer == DIRECT_TICK(36) ){
		FVECTOR	fvtemp;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		work->think_mul_x = work->think_mul_z =
			GET_MUL_STEP(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz),LOFT2LOFT_DIFF0,LOFT2LOFT_DIFF1);   //BP_MATH - emulate PS2 sqrtf
			//(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz) - LOFT2LOFT_LAST)/LOFT2LOFT_DIFF; //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;

		SET_FLAG( work->act_flags, VMP_ACTFLAG_CHANGE_MUL );
	}
	if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
		work->think_local = -1;
		return -1;
	}
	return 0;
}


//２階から２階
static int VMP_ThinkLocal_Non_JumpLoft_Temp( Work *work )
{
	
	if( work->think_timer == 0 ){
		FVECTOR	fvtemp;
		SVECTOR rot;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		TS_VecToRot( &rot, &fvtemp );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
	}else if( work->think_timer == DIRECT_TICK(37) ){
		FVECTOR	fvtemp;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		work->think_mul_x = work->think_mul_z =
			GET_MUL_STEP(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz),LOFT2LOFT_DIFF0,LOFT2LOFT_DIFF1);   //BP_MATH - emulate PS2 sqrtf
			//(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz) - LOFT2LOFT_LAST)/LOFT2LOFT_DIFF; //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;

		SET_FLAG( work->act_flags, VMP_ACTFLAG_CHANGE_MUL );
	}
	if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
		work->think_local = -1;
		return -1;
	}
	return 0;
}

#if 0
//２階から一階手摺へ
static int VMP_ThinkLocal_Non_JumpLoftToRail( Work *work )
{
	
	if( work->think_timer == 0 ){
		FVECTOR	fvtemp;
		SVECTOR rot;
		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		TS_VecToRot( &rot, &fvtemp );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
	}else if( work->think_timer == DIRECT_TICK(36) ){
		FVECTOR	fvtemp;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		work->think_mul_x = work->think_mul_z =
			GET_MUL_STEP(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz),LOFT2RAIL_DIFF0,LOFT2RAIL_DIFF1);   //BP_MATH - emulate PS2 sqrtf
			//(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz) - LOFT2RAIL_LAST)/LOFT2RAIL_DIFF; //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;

		SET_FLAG( work->act_flags, VMP_ACTFLAG_CHANGE_MUL );
	}
	if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
		work->think_local = -1;
		return -1;
	}
	return 0;
}
#endif

//２階から飛び降り
static int VMP_ThinkLocal_Non_JumpToFloor( Work *work )
{
//	if( !CHECK_FLAG( work->status, VMP_STATUS_NO_DIR ) ){
//		work->dir = work->think_dir;
//	}
	
	if( work->think_timer == 0 ){
		FVECTOR	fvtemp;
		SVECTOR rot;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		TS_VecToRot( &rot, &fvtemp );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
	}else if( work->think_timer == DIRECT_TICK(36) ){
		FVECTOR	fvtemp;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		work->think_mul_x = work->think_mul_z =
			GET_MUL_STEP(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz),LOFT2FLOOR_DIFF0,LOFT2FLOOR_DIFF1); //BP_MATH - emulate PS2 sqrtf
			//(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz) - LOFT2FLOOR_LAST)/LOFT2FLOOR_DIFF;  //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;

		SET_FLAG( work->act_flags, VMP_ACTFLAG_CHANGE_MUL );

		//_sceVu0Normalize( &fvtemp, &fvtemp );
		//TS_VecToRot( &rot, &fvtemp );
		//if( rot.vy < 0 ) rot.vy += 4096;
		//work->dir = rot.vy;
		//SET_FLAG( work->act_flags, VMP_ACTFLAG_CHANGE_MUL );
	}
	if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
		work->think_local = -1;
		return -1;
	}
	return 0;
}



//２階から水へ
static void VMP_ThinkLocal_Non_DiveLoftWater( Work *work )
{
//	if( !CHECK_FLAG( work->status, VMP_STATUS_NO_DIR ) ){
//		work->dir = work->think_dir;
//	}
	if( work->think_timer == 0 ){
		FVECTOR	fvtemp;
		SVECTOR rot;
		VMP_DivePos( &fvtemp, work->think_dive_pos );
		DG_COPY_VEC( &work->think_init_pos, &fvtemp );
		_sceVu0SubVector( &fvtemp, &fvtemp, &work->control.mov );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		TS_VecToRot( &rot, &fvtemp );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
	}else if( work->think_timer == DIRECT_TICK(36) ){
		FVECTOR	fvtemp;

		VMP_DivePos( &fvtemp, work->think_dive_pos );
		DG_COPY_VEC( &work->think_init_pos, &fvtemp );
		
		_sceVu0SubVector( &fvtemp, &fvtemp, &work->control.mov );

		work->think_mul_x = work->think_mul_z =
			GET_MUL_STEP(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz),LOFT2WATER_DIFF0,LOFT2WATER_DIFF1); //BP_MATH - emulate PS2 sqrtf
			//(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz) - LOFT2WATER_LAST)/LOFT2WATER_DIFF;  //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;

		SET_FLAG( work->act_flags, VMP_ACTFLAG_CHANGE_MUL );

		//_sceVu0Normalize( &fvtemp, &fvtemp );
		//TS_VecToRot( &rot, &fvtemp );
		//if( rot.vy < 0 ) rot.vy += 4096;
		//work->dir = rot.vy;
		//SET_FLAG( work->act_flags, VMP_ACTFLAG_CHANGE_MUL );
	}
}


//１階手摺から水へ
static void VMP_ThinkLocal_Non_DiveRailWater( Work *work )
{
	if( work->think_timer == 0 ){
		FVECTOR	fvtemp;
		SVECTOR rot;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		TS_VecToRot( &rot, &fvtemp );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
		//printf("dir = %d\n",work->dir);
	}else if( work->think_timer == DIRECT_TICK(36) ){
		FVECTOR	fvtemp;
		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		work->think_mul_x = work->think_mul_z =
			GET_MUL_STEP(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz),RAIL2WATER_DIFF0,RAIL2WATER_DIFF1); //BP_MATH - emulate PS2 sqrtf
			//(bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz) - RAIL2FLOOR_LAST)/RAIL2FLOOR_DIFF;  //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;
		SET_FLAG( work->act_flags, VMP_ACTFLAG_CHANGE_MUL );
	}
}


//水から水
static void VMP_ThinkLocal_Non_WaterToWater( Work *work )
{
	if( !CHECK_FLAG( work->status, VMP_STATUS_NO_DIR ) ){
		work->dir = (work->think_dir+2048) % 4096;
	}
	
	if( work->think_timer == 0 ){
		//FVECTOR	water;
		//VMP_DivePos( &water, work->think_jumpout_pos );
		GetInitialVelocity( &work->think_mot_step,
							&work->control.mov,
							&work->think_init_pos,
							45.0f );

	}else if( work->think_timer == DIRECT_TICK(45) ){
		DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );		
	}
	//VMP_PRINTF(" step vy %f\n", work->control.step.vy);
	//if( work->think_timer < 10 ) work->think_mot_step.vy += 2.0f*500.0f/(float)(11.0f*11.0f);
	//work->think_mot_step.vy = 1.5f * (work->body.height - work->old_body_height);
}

//水から水
static void VMP_ThinkLocal_Non_WaterToWater_Splash( Work *work )
{
//	if( !CHECK_FLAG( work->status, VMP_STATUS_NO_DIR ) ){
//		work->dir = (work->think_dir+2048) % 4096;
//	}

	if( work->think_timer == 0 ){
		FVECTOR	water;
		//VMP_DivePos( &water, work->think_jumpout_pos );
		GetInitialVelocity( &work->think_mot_step,
							&work->control.mov,
							&work->think_init_pos,
							48.0f );
		_sceVu0SubVector( &water, &water, &work->control.mov );
		work->dir = (int)(2048.0f * atan2f( water.vx ,water.vz ) / PI);
		if( work->dir < 0 ) work->dir += 4096;

	}else if( work->think_timer == DIRECT_TICK(48) ){
		DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
	}

}

#if 1
//水から壁張り付き
static void VMP_ThinkLocal_Non_SpiderPos( Work *work )
{

	if( work->think_timer == 0 ){
		FVECTOR		loft;
		SVECTOR		rot;

		//デバック用
		DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
		
		_sceVu0SubVector( &loft, &work->think_init_pos, &work->control.mov );
		work->think_mul_x = work->think_mul_z =
			GET_MUL_STEP(bp_sqrtf(loft.vx*loft.vx+loft.vz*loft.vz),WATER2LOFT_DIFF0,WATER2LOFT_DIFF1);   //BP_MATH - emulate PS2 sqrtf
			//(bp_sqrtf(loft.vx*loft.vx+loft.vz*loft.vz) - WATER2LOFT_LAST)/WATER2LOFT_DIFF; //BP_MATH - emulate PS2 sqrtf
		work->think_mul_y = 1.0f;

		_sceVu0Normalize( &loft, &loft );
		TS_VecToRot( &rot, &loft );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = rot.vy;
		//VMP_PRINTF( "init %d to_rai %d\n", work->dir,work->think_dir);		
	}
#if 0
	if( work->think_timer == 0 ){
		FVECTOR	water;

		//GetTriAttPosition( &water, work->think_jumpout_pos );
		GetInitialVelocity( &work->think_mot_step,
							&work->control.mov,
							&work->think_init_pos,
							30.0f );//*29.0f );
		_sceVu0SubVector( &water, &water, &work->control.mov );
		//work->dir = (int)(2048.0f * atan2f( water.vx ,water.vz ) / PI);
		//if( work->dir < 0 ) work->dir += 4096;
		//work->dir = (work->dir+2048)%4096;
		
		//printf("あはーん %d\n",work->dir);
		work->dir = (work->think_dive_pos*2048);
	}
	work->think_mot_step.vy = 1400.0f/(29.0f);//*58.0f);
#endif
}
#endif
#if 0
//床からから壁張り付き
static void VMP_ThinkLocal_SpiderFromPos( Work *work )
{
#if 1
	if( work->think_timer == 0 ){
		FVECTOR	water;
		
		//GetTriAttPosition( &water, work->think_jumpout_pos );
#ifdef PAL        //BP JG - unused
		GetInitialVelocity( &work->think_mot_step,
							&work->control.mov,
							&work->think_init_pos,
							25.0f );//*29.0f );
#else
		GetInitialVelocity( &work->think_mot_step,
							&work->control.mov,
							&work->think_init_pos,
							30.0f );//*29.0f ;)
#endif
		_sceVu0SubVector( &water, &water, &work->control.mov );
		//work->dir = (int)(2048.0f * atan2f( water.vx ,water.vz ) / PI);
		//if( work->dir < 0 ) work->dir += 4096;
		//work->dir = (work->dir+2048)%4096;
		
		//printf("あはーん %d\n",work->dir);
		work->dir = (work->think_jumpout_pos*2048);
	}
	work->think_mot_step.vy = 1000.0f/(29.0f);//*58.0f);
#endif
}
#endif

//壁張り付きからふっとばし
static void VMP_ThinkLocal_Non_SpiderToWaterAtt( Work *work )
{
	if( work->think_timer == 0 ){
		//bugbug
//		FVECTOR	water;
//		float	len;

//		VMP_DivePos( &water, 19 );
//		_sceVu0SubVector( &water, &water, &work->control.mov );
//		len = bp_sqrtf( water.vx*water.vx + water.vz*water.vz ); //BP_MATH - emulate PS2 sqrtf
//		_sceVu0ScaleVector( &water, (FVECTOR*)(BODYWORLD(&work->body,12).m[2]), len );
//		_sceVu0AddVector( &water, &water, &work->control.mov );
//
//		GetInitialVelocity( &work->think_mot_step,
//							&work->control.mov,
//							&water,
//							29.0f );//*2.0f );//*29.0f );
//		_sceVu0SubVector( &water, &water, &work->control.mov );
//		_sceVu0Normalize( &water, &water );
#if 0
		work->dir = (int)(2048.0f * atan2f( water.vx ,water.vz ) / PI);
		if( work->dir < 0 ) work->dir += 4096;
#else
		work->dir = (int)(2048.0f * atan2f( BODYWORLD(&work->body,12).m[2][0], BODYWORLD(&work->body,12).m[2][2] ) / PI);
		if( work->dir < 0 ) work->dir += 4096;
#endif
		//work->dir = (work->dir+2048)%4096;
		//printf("あはーん %d\n",work->dir);
	}
//	work->think_mot_step.vy = -3000.0f/(29.0f);//*58.0f);
}


//張り付き中の処理
static void VMP_ThinkLocal_Non_SpiderWait( Work *work )
{
	work->dir = (work->think_dive_pos*2048);
	
	//VMP_SetAdjustWork( work, &(SVECTOR){ 300, 0, 0, 0 }, 0 );
	if( work->think_jumpout_pos == GetTriAttNum( &GM_PlayerPosition ) ){
		int		rot_y,rot_x,check;
		CONTROL *ctrl = &work->control;
		SVECTOR to11 = { 0, 0, 0, 0 };
		SVECTOR to12 = { 0, 0, 0, 0 };
		
		rot_y = (work->think_dir - ctrl->rot.vy)&0x0fff;
		if( rot_y > 2048 ) rot_y -= 4096;
		check = (rot_y<0)?-rot_y:rot_y;
		//VMP_PRINTF("rot_y = %d check = %d ",rot_y,check);
		
		if( 3 < check && check < 1024+512 ){
			to11.vy = rot_y;
			to12.vy = rot_y;
			//VMP_PRINTF("回れー\n");
		}
		
		//頭のみｘ回転
		rot_x = work->adjust_rot_x - ctrl->rot.vx;
		if( rot_x > 2048 ) rot_x -= 4096;
		check = (rot_x<0)?-rot_x:rot_x;
		
		//VMP_PRINTF("rot_x = %d rot_x = %d check = %d\n",work->adjust_rot_x,rot_x,check);
		if( 0 < check && check < 700 ){
			to12.vx = rot_x;
		}
		
		VMP_SetAdjustWork( work, &to11, 11 );
		VMP_SetAdjustWork( work, &to12, 12 );
	} else {
	//	ClearAjustToPlayer( work );
#ifdef BP_PS2 //yano
		VMP_SetAdjustWork( work, &(SVECTOR){ 0, 0, 0, 0 }, 11 );
		VMP_SetAdjustWork( work, &(SVECTOR){ 0, 0, 0, 0 }, 12 );
#else
	{
		SVECTOR dmdm = { 0, 0, 0, 0 };
		VMP_SetAdjustWork( work, &dmdm, 11 );
		VMP_SetAdjustWork( work, &dmdm, 12 );
	}
#endif
	}
	
}	

//水上への（攻撃）
static void VMP_ThinkLocal_Non_JumpOutWater( Work *work )
{
	if( !CHECK_FLAG( work->status, VMP_STATUS_NO_DIR ) ){
		work->dir = (work->think_dir+2048)%4096;
//		work->dir = work->think_dir;
	}
	
	//work->think_mot_step.vy = 1.5f * (work->body.height - work->old_body_height);
}

//泳ぎ移動（ライデン）
static void VMP_ThinkLocal_Non_SwimRai( Work *work )
{	
	work->dir = VMP_GetFromZoneToZoneDirWater( &work->control.mov, work->think_v_wzn,
											work->think_p_wzn, 600.0f, work->think_pre_v_wzn );
#if 0
	if( work->dir < 0 ){
		work->think_local = THK_LOCAL_NON_SWIM_POS;
		work->think_timer = -1;
		work->think_jumpout_pos = VMP_RaiMayBeNextZone( work->think_p_fzn, work->think_pre_p_fzn,
														CHECK_FLAG(work->think_flags,VMP_FLAGS_STOP) );
		work->think_dive_pos = VMP_GetDivePosNum( work->think_jumpout_pos );
		work->think_to_wzn = work->think_dive_pos;
	}
#endif

}

//泳ぎ移動（ポス）
static void VMP_ThinkLocal_Non_SwimPos( Work *work )
{
	work->dir = VMP_GetFromZoneToZoneDirWater( &work->control.mov, work->think_v_wzn,
											work->think_to_wzn, 600.0f, work->think_pre_v_wzn );

}

//泳ぎ移動潜る（ポス）
static void VMP_ThinkLocal_Non_SwimPosDive( Work *work )
{
	work->dir = VMP_GetFromZoneToZoneDirWater( &work->control.mov, work->think_v_wzn,
											work->think_to_wzn, 600.0f, work->think_pre_v_wzn );
	//
	if( work->think_timer == 0 ){
		work->think_mot_step.vx = 0.0f;
		work->think_mot_step.vy = -100.0f;
		work->think_mot_step.vz = 0.0f;
	}
	if( work->think_now_main_w == THK_SUB_MAIN3_W ){
		if( work->control.mov.vy > -14000.0f ){
			work->dir_x = 300;//-300 + 4096;
		}else{
			work->think_mot_step.vy = 0.0f;
			work->dir_x = 0;
		}
		work->dir_z = 0;
	}/*else if( work->think_now_main_w == THK_SUB_MAIN2_W && work->think_ntbl == 4 ){
		if( work->control.mov.vy < -8200.0f ){
			//work->dir_x = 100;//-300 + 4096;
			work->think_mot_step.vy = 50.0f;
		}else if( work->control.mov.vy > -7800.0f ){
			//work->dir_x = -100;//-300 + 4096;
			work->think_mot_step.vy = -50.0f;
		}else{
			work->think_mot_step.vy = 0.0f;
			work->dir_x = 0;
		}
		work->dir_z = 2048;
	}*/else{
		if( work->control.mov.vy > -12000.0f ){
			work->dir_x = 300;//-300 + 4096;
		}else{
			work->think_mot_step.vy = 0.0f;
			work->dir_x = 0;
		}
		work->dir_z = 0;
	}
}

//泳ぎ移動浮上（ポス）
static void VMP_ThinkLocal_Non_SwimPosUp( Work *work )
{
	if( work->think_timer == 0 ){
		FVECTOR		fvtemp;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		work->dir = (int)(2048.0f * atan2f( fvtemp.vx, fvtemp.vz ) / PI);
		if( work->dir < 0 ) work->dir += 4096;
		//work->dir;
		work->think_mot_step.vx = 0.0f;
		work->think_mot_step.vy = 200.0f;
		work->think_mot_step.vz = 0.0f;
		work->dir_x = -1024 + 4096;

		
	}
}

//泳ぎ移動浮上（ポス）
static void VMP_ThinkLocal_Non_SwimPosUp_Invr( Work *work )
{
	if( work->think_timer == 0 ){
		FVECTOR		fvtemp;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		work->dir = (int)(2048.0f * atan2f( fvtemp.vx, fvtemp.vz ) / PI);
		if( work->dir < 0 ) work->dir += 4096;
		
		work->dir = (work->dir+2048)%4096;
		work->think_mot_step.vx = 0.0f;
		work->think_mot_step.vy = 200.0f;
		work->think_mot_step.vz = 0.0f;
		work->dir_x = -1024 + 4096;
	}
}

//泳ぎ移動浮上（ポス）
static void VMP_ThinkLocal_Non_SwimPosUpHigh( Work *work )
{
	if( work->think_timer == 0 ){
		FVECTOR		fvtemp;

		_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
		work->dir = (int)(2048.0f * atan2f( fvtemp.vx, fvtemp.vz ) / PI);
		if( work->dir < 0 ) work->dir += 4096;
		//work->dir;
		work->think_mot_step.vx = 0.0f;
		work->think_mot_step.vy = 400.0f;
		work->think_mot_step.vz = 0.0f;
		work->dir_x = -1024 + 4096;

		
	}
}

//泳ぎ移動（ライデン）
static void VMP_ThinkLocal_Non_SwimRaiDive( Work *work )
{	
	work->dir = VMP_GetFromZoneToZoneDirWater( &work->control.mov, work->think_v_wzn,
											work->think_p_wzn, 600.0f, work->think_pre_v_wzn );

	if( work->think_timer == 0 ){
		work->think_mot_step.vx = 0.0f;
		work->think_mot_step.vy = -100.0f;
		work->think_mot_step.vz = 0.0f;
	}
	
	if( work->think_now_main_w == THK_SUB_MAIN3_W ){
		if( work->control.mov.vy > -14000.0f ){
			work->dir_x = 300;//-300 + 4096;
		}else{
			work->think_mot_step.vy = 0.0f;
			work->dir_x = 0;
		}
		work->dir_z = 0;
	}/*else if( work->think_now_main_w == THK_SUB_MAIN2_W && work->think_ntbl == 3 ){
		if( work->control.mov.vy > -8200.0f ){
			work->dir_x =-100;//-300 + 4096;
			work->think_mot_step.vy = -50.0f;
			
		}else if( work->control.mov.vy < -7800.0f ){
			work->dir_x = 100;//-300 + 4096;
			work->think_mot_step.vy = 50.0f;
		}else{
			work->think_mot_step.vy = 0.0f;
			work->dir_x = 0;
		}
		work->dir_z = 2048;
	}*/else{
		if( work->control.mov.vy > -12000.0f ){
			work->dir_x = 300;//-300 + 4096;
		}else{
			work->think_mot_step.vy = 0.0f;
			work->dir_x = 0;
		}
		work->dir_z = 0;
	}/*
	if( work->control.mov.vy > -12000.0f ){
		work->dir_x = 300;//-300 + 4096;
	}else{
		work->think_mot_step.vy = 0.0f;
		work->dir_x = 0;
	}*/
}


//床から水へ吹き飛ばされ
static void VMP_ThinkLocal_Flr2Wtr_Blow( Work *work )
{	
	if( work->think_timer == 0 ){
		FVECTOR		loft;
		SVECTOR		rot;

		_sceVu0SubVector( &loft, &work->think_init_pos, &work->control.mov );
		work->think_mul_x = work->think_mul_z =
			bp_sqrtf(loft.vx*loft.vx+loft.vz*loft.vz)/FLR2WTR_BLOW_DIFF;   //BP_MATH - emulate PS2 sqrtf
			//(bp_sqrtf(loft.vx*loft.vx+loft.vz*loft.vz) - WATER2RAIL_LAST)/WATER2RAIL_DIFF; //BP_MATH - emulate PS2 sqrtf
		
		work->think_mul_y = 1.0f;// - 4000.0f/(5653.41796f-(-1700.01354f));
		
		_sceVu0Normalize( &loft, &loft );
		TS_VecToRot( &rot, &loft );
		if( rot.vy < 0 ) rot.vy += 4096;
		work->dir = (rot.vy + 2048)%4096;		
	}
}

/* ------------------------------------------------------------------------------------

各フェーズステップ関数 

 ----------------------------------------------------------------------------------- */

static void VMP_ThinkLocalLastDamFloor( Work *work )
{
	switch( work->think_local ){
	  case 0:
		{
			SVECTOR		rot;
			FVECTOR		fvtemp;
			
			VMP_DivePos( &work->think_init_pos, 19 );
			
			_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
			_sceVu0Normalize( &fvtemp, &fvtemp );
			TS_VecToRot( &rot, &fvtemp );
			if( rot.vy < 0 ) rot.vy += 4096;
			
			ClearAjustToPlayer( work );
			work->think_timer = -1;
			work->think_local++;
			work->dir = rot.vy;//work->think_dir;
			VMP_PRINTF("think call\n");
#if 1
			if( work->end_proc ){
				GCL_ARGS	gcl_args;
				int			data[4];
				data[0] = 1;
				data[1] = work->control.mov.vx;
				data[2] = work->control.mov.vy;
				data[3] = work->control.mov.vz;
				gcl_args.argc = 4;
				gcl_args.argv = data;
				GCL_ExecProc( work->end_proc, &gcl_args );
			}
#endif
		}
		break;
	  case 1:
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
#if 0
			if( work->end_proc ){
				
				GCL_ARGS	gcl_args;
				int			data[4];
				data[0] = 1;
				data[1] = work->control.mov.vx;
				data[2] = work->control.mov.vy;
				data[3] = work->control.mov.vz;
				gcl_args.argc = 4;
				gcl_args.argv = data;
				GCL_ExecProc( work->end_proc, &gcl_args );
			}
#endif
			work->think_local++;
		}
		break;
	  case 2:
		break;
	}

}

static void VMP_ThinkLocalLastDamLoft( Work *work )
{

	switch( work->think_local ){
	  case 0:
		{
			SVECTOR		rot;
			FVECTOR		fvtemp;
			ClearAjustToPlayer( work );
			work->think_timer = -1;
			work->think_local++;
			VMP_DivePos( &work->think_init_pos, 19 );
			_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
			//work->think_mul_x = work->think_mul_z =
			//	bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz)/FLR2WTR_BLOW_DIFF; //BP_MATH - emulate PS2 sqrtf

			//work->think_mul_y = 1.0f;
			_sceVu0Normalize( &fvtemp, &fvtemp );
			TS_VecToRot( &rot, &fvtemp );
			if( rot.vy < 0 ) rot.vy += 4096;
			//work->dir = (rot.vy + 2048)%4096;
			VMP_PRINTF("think call\n");
#if 1
			if( work->end_proc ){
				GCL_ARGS	gcl_args;
				int			data[4];
				data[0] = 0;
				data[1] = work->control.mov.vx;
				data[2] = work->control.mov.vy;
				data[3] = work->control.mov.vz;
				gcl_args.argc = 4;
				gcl_args.argv = data;
				GCL_ExecProc( work->end_proc, &gcl_args );
			}
#endif
		}
		break;
	  case 1:
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
#if 0
			if( work->end_proc ){
				
				GCL_ARGS	gcl_args;
				int			data[4];
				data[0] = 0;
				data[1] = work->control.mov.vx;
				data[2] = work->control.mov.vy;
				data[3] = work->control.mov.vz;
				gcl_args.argc = 4;
				gcl_args.argv = data;
				GCL_ExecProc( work->end_proc, &gcl_args );
			}
#endif
			work->think_local++;
		}
		break;
	  case 2:
		break;
	}

}
#if 0
static void VMP_ThinkLocalLastBlastDamFloor( Work *work )
{
	switch( work->think_local ){
	  case 0:
		// 計算
		break;
	  case 1:
		break;
	  case 2:
		//プロック呼び出し
		break;
	}
}
#endif
static void VMP_ThinkLocalLastBlastDamLoft( Work *work )
{

	switch( work->think_local ){
	  case 0:
		{
			SVECTOR		rot;
			FVECTOR		fvtemp;
			ClearAjustToPlayer( work );
			work->think_timer = -1;
			work->think_local++;
			//_sceVu0SubVector( &fvtemp, &work->control.mov, &work->off_hit );	
			//VMP_GetBlowWaterPos( &work->think_init_pos, &work->control.mov, &fvtemp );//&work->off_force );
			VMP_DivePos( &work->think_init_pos, 19 );
			_sceVu0SubVector( &fvtemp, &work->think_init_pos, &work->control.mov );
			work->think_mul_x = work->think_mul_z =
				bp_sqrtf(fvtemp.vx*fvtemp.vx+fvtemp.vz*fvtemp.vz)/FLR2WTR_BLOW_DIFF; //BP_MATH - emulate PS2 sqrtf

			work->think_mul_y = 1.0f;
			_sceVu0Normalize( &fvtemp, &fvtemp );
			TS_VecToRot( &rot, &fvtemp );
			if( rot.vy < 0 ) rot.vy += 4096;
			work->dir = (rot.vy + 2048)%4096;
			VMP_PRINTF("think call\n");
#if 1
			if( work->end_proc ){
				GCL_ARGS	gcl_args;
				int			data[4];
				data[0] = 2;
				data[1] = work->control.mov.vx;
				data[2] = work->control.mov.vy;
				data[3] = work->control.mov.vz;
				gcl_args.argc = 4;
				gcl_args.argv = data;
				GCL_ExecProc( work->end_proc, &gcl_args );
			}
#endif
		}
		break;
	  case 1:
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
#if 0
			if( work->end_proc ){
				
				GCL_ARGS	gcl_args;
				int			data[4];
				data[0] = 2;
				data[1] = work->control.mov.vx;
				data[2] = work->control.mov.vy;
				data[3] = work->control.mov.vz;
				gcl_args.argc = 4;
				gcl_args.argv = data;
				GCL_ExecProc( work->end_proc, &gcl_args );
			}
#endif
			work->think_local++;
		}
		break;
	  case 2:
		break;
	}
}

//床から水へ吹き飛ばされ
static void VMP_ThinkLocalFlr2WtrBlow( Work *work )
{
	SET_FLAG( work->status, VMP_STATUS_BLOW );
	switch( work->think_local ){
	  case 0:
		work->think_mesg = THINK_MESG_FLR2WRT_B;
		work->think_timer = -1;
		work->think_local++;
		/*
		work->think_to_fzn = VMP_GetZoneRndFromLen( work->think_v_fzn, 4, work->think_v_fzn );
		work->think_dive_pos = VMP_GetDivePosNum( work->think_to_fzn );
		VMP_DivePos( &work->think_init_pos, work->think_dive_pos );
		*/
		{
			FVECTOR fvtemp;
			_sceVu0SubVector( &fvtemp, &work->control.mov, &work->off_hit );
			VMP_GetBlowWaterPos( &work->think_init_pos, &work->control.mov, &fvtemp );//&work->off_force );
		}
		ClearAjustToPlayer( work );
		//PRINT_PFVEC( 0, &work->think_init_pos );
		break;
	  case 1:
		VMP_ThinkLocal_Flr2Wtr_Blow( work );
		//VMP_ThinkLocal_Non_JumpToFloor( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			VMP_SetThinkSub( work, work->think_now_main_w, 0 );
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}


//水の上から始まる
static void VMP_ThinkLocal_StartOnWater( Work *work )
{
	static void *debug_spwork;
	SET_FLAG( work->think_flags, VMP_FLAGS_NO_BLOW );
	switch( work->think_local ){
	  case 0:
		work->think_mesg = THINK_MESG_LOCKON;
		work->think_timer = -1;
		work->think_temp0 = 0;
		work->think_local++;
		{
			extern void *NewWaterWindSplush( FMATRIX *world, float inf_height, float max_width, int *flag );
			debug_spwork = NewWaterWindSplush( &BODYWORLD(&work->body,0),
												2000.0f,
												3000.0f,
												NULL );
		}
		break;
	  case 1:
		//待ち
		SetAjustToPlayer( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			work->think_timer = -1;
			work->dir = work->think_dir;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_RAPIDAVOID ) ){
			ClearAjustToPlayer( work );
			work->think_local = -2;
		}else if( work->think_timer > DIRECT_TICK(60) ){
			//お辞儀へ
			ClearAjustToPlayer( work );
			work->dir = work->think_dir;
			work->think_mesg = THINK_MESG_LOWBOW;
			work->think_timer = -1;
			work->think_local++;
			work->think_temp0 = 1;
		}
		break;
	  case 2:
		//お辞儀
		work->dir = work->think_dir;
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			work->think_local = -1;
			work->think_timer = -1;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_RAPIDAVOID ) ){
			work->think_local = -2;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_mesg = THINK_MESG_INTOWATER;
			work->think_timer = -1;
			work->think_local++;
		}
		break;
	  case 3:
		//水中へ
		work->dir = work->think_dir;
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
//			if( debug_spwork ) GV_DestroyOtherActor( debug_spwork );
#if 1
			if( debug_spwork ){
				extern void OK_SpecialFade( Work *work );
				OK_SpecialFade( debug_spwork );
			}
#endif
			VMP_SetThinkSub( work, work->think_now_main_w, 0 );
		}
		break;
	  case -1:
		//ダメージ
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			//ジャンプ水中へ
			work->think_mesg = THINK_MESG_INTOWATER;
			work->think_timer = -1;
			work->think_local = 3;
		}
		break;
	  case -2:
		//回避中
		work->dir = work->think_dir;
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			if( work->think_temp0 ){
				//ジャンプ水中へ
				work->think_mesg = THINK_MESG_INTOWATER;
				work->think_timer = -1;
				work->think_local = 3;
			}else{
				//お辞儀へ
				work->dir = work->think_dir;
				work->think_mesg = THINK_MESG_LOWBOW;
				work->think_timer = -1;
				work->think_local = 2;
				work->think_temp0 = 1;
			}
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}


//2階から床へ
static void VMP_ThinkLocal_JumpLoftToFloor( Work *work )
{
	switch( work->think_local ){
	  case 0:
		work->think_to_fzn = VMP_GetRndZone( work->think_p_fzn, ZONE_INFO_NEAR );
		VMP_JumpOutPos( &work->think_init_pos, work->think_to_fzn );
		work->think_mesg = THINK_MESG_LOFT2FLOOR;
		work->think_timer = -1;
		work->think_local++;
		break;
	  case 1:
		if( VMP_ThinkLocal_Non_JumpToFloor( work ) ) return;
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_NEAR, 0 );
		}
		break;
	  case -1:
		//ダメージ
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_NEAR, 0 );
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}

static void VMP_ThinkLocal_JumpOutLoft_Walk( Work *work )
{
	switch( work->think_local ){
	  case 0:
		// 2か3を決める
		work->dir = -1;
		work->think_v_loft = (irnd()&0x0400)?2:3;
		work->think_jumpout_pos = GetHandRailDammyNum( work->think_v_loft );
		work->think_to_wzn = 19;
		work->think_local++;
		work->think_timer = -1;
		work->think_n_att = 0;
		break;
	  case 1:
		//移動中継1	取り合えず19へ
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			work->think_local++;
			work->think_timer = -1;
			work->think_to_wzn = GetGoSplashWaterNum_1st( work->think_jumpout_pos );
		}
		break;
	  case 2:
		//移動中継2 回り込む
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			work->think_local++;
			work->think_timer = -1;
			work->think_to_wzn = GetGoSplashWaterNum_2nd( work->think_jumpout_pos );
		}
		break;
	  case 3:
		//移動中継2 回り込む
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			work->think_local++;
			work->think_timer = -1;
			work->think_to_wzn = 19;
		}
		break;
	  case 4:
		//移動中継3 19へ
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			work->think_local++;
			work->think_timer = -1;
			
			GetLoftPosition_Temp( &work->think_init_pos, work->think_v_loft );
		}
		break;
	  case 5:
		VMP_ThinkLocal_Non_SwimPosUp( work );
		if( work->control.mov.vy > -8000.0f ){
			work->think_timer = -1;
			work->think_local++;
			work->dir_x = 0;
			work->think_mot_step.vy = 0.0f;
			work->control.mov.vy = -8000.0f;
			work->think_mesg = THINK_MESG_WATER2LOFT;
		}
		break;
	  case 6:
		// 水から2階へ
		if( VMP_ThinkLocal_Non_JumpOutLoftRnd( work ) ) return;
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_SEMIEND ) ){
			work->think_local++;
			work->think_timer = -1;
			work->think_mesg = THINK_MESG_LOFTMOVE;
			work->think_turn = (irnd()>>17)&1;
			work->dir = VMP_GetLoftDir_Temp( &work->control.mov, work->think_v_loft, work->think_turn );
			//VMP_PRINTF("およよ %d<%d>\n",work->think_turn,work->dir );
		}
		break;
	  case 7:
		// 移動
		{
			int tmp;
			SetAjustToPlayer( work );
			tmp = VMP_GetLoftDir_Temp( &work->control.mov, work->think_v_loft, work->think_turn );
			//VMP_PRINTF("tmp %d\n",tmp);
			if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
				ClearAjustToPlayer( work );
				work->think_local = -1;
				work->think_timer = -1;
				work->dir = work->think_dir;
			}else if( tmp != work->dir ){
				ClearAjustToPlayer( work );
				work->think_turn = (tmp)?1:0;
				work->think_local++;
				work->think_timer = -1;
				work->dir = work->think_dir;
				UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
				work->think_mesg = THINK_MESG_THOROWL;
			}else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
				work->think_mesg = THINK_MESG_LOFTMOVE;
			}

		}

		break;
	  case 8:
		//攻撃
		//SetAjustToPlayer( work );
		VMP_GetLoftDir_Temp( &work->control.mov, work->think_v_loft, work->think_turn );
		if( CHECK_FLAG( work->status, VMP_STATUS_THRWKNF ) ){
			SET_FLAG( work->think_flags, VMP_FLAGS_KNFATT );
			work->think_n_att++;
		}


		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			work->think_local = -1;
			work->think_timer = -1;
			work->dir = work->think_dir;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_RAPIDAVOID ) ){
			work->think_local = -2;
			work->think_timer = -1;
			work->dir = work->think_dir;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			int tmp = (work->think_v_loft==2)?3:2;
			if( work->think_n_att > 1 ){
				VMP_SetThinkSub( work, THK_SUB_WATERLOFTWATER_ATT, 0 );
				UNSET_FLAG( work->think_flags, VMP_FLAGS_KNFATT );
				if( work->think_v_loft == 2 ) work->think_v_loft = 0;
				return;
			}
			work->think_mesg = THINK_MESG_LOFT2LOFT;
			work->think_local++;
			work->think_timer = -1;
			work->think_v_loft = tmp;
			GetLoftPosition_Temp( &work->think_init_pos, work->think_v_loft );
		}else if( CHECK_FLAG( work->status, VMP_STATUS_ATTACK ) && work->think_timer == DIRECT_TICK(44) ){
			SET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
		}

		break;
	  case 9:
		//2階から2階へ
		if( VMP_ThinkLocal_Non_JumpLoft_Temp( work ) ) return;
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_SEMIEND ) ){
			work->think_timer = -1;
			work->think_local = 7;
			UNSET_FLAG( work->think_flags, VMP_FLAGS_KNFATT );
			work->dir = VMP_GetLoftDir_Temp( &work->control.mov, work->think_v_loft, work->think_turn );
			work->think_mesg = THINK_MESG_LOFTMOVE;
		}
		break;
	  case -1:
		//ダメージ
		VMP_GetLoftFixPos_Temp( work->think_v_loft, &work->control.mov );
		//VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			//VMP_SetThinkSub( work, THK_SUB_DIVE_LOFT, 0 );
			work->think_turn = (work->think_turn)?0:1;
			if( work->think_n_att > 1 ){
				VMP_SetThinkSub( work, THK_SUB_WATERLOFTWATER_ATT, 0 );
				if( work->think_v_loft == 2 ) work->think_v_loft = 0;
				UNSET_FLAG( work->think_flags, VMP_FLAGS_KNFATT );
			}else if( CHECK_FLAG( work->think_flags, VMP_FLAGS_KNFATT ) ){
				int tmp = (work->think_v_loft==2)?3:2;
				work->think_mesg = THINK_MESG_LOFT2LOFT;
				work->think_local = 9;
				work->think_timer = -1;
				work->think_v_loft = tmp;
				GetLoftPosition_Temp( &work->think_init_pos, work->think_v_loft );
				UNSET_FLAG( work->think_flags, VMP_FLAGS_KNFATT );
			}else{
				UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
				work->think_local = 8;
				work->dir = work->think_dir;
				work->think_timer = -1;
				work->think_mesg = THINK_MESG_THRWL_MUTEKI;
			}
		}
		break;
	  case -2:
		//ダメージ
		VMP_GetLoftFixPos_Temp( work->think_v_loft, &work->control.mov );
		//VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			if( work->think_n_att > 1 ){
				VMP_SetThinkSub( work, THK_SUB_WATERLOFTWATER_ATT, 0 );
				if( work->think_v_loft == 2 ) work->think_v_loft = 0;
				UNSET_FLAG( work->think_flags, VMP_FLAGS_KNFATT );
			}else if( CHECK_FLAG( work->think_flags, VMP_FLAGS_KNFATT ) ){
				int tmp = (work->think_v_loft==2)?3:2;
				work->think_mesg = THINK_MESG_LOFT2LOFT;
				work->think_local = 9;
				work->think_timer = -1;
				work->think_v_loft = tmp;
				GetLoftPosition_Temp( &work->think_init_pos, work->think_v_loft );
				UNSET_FLAG( work->think_flags, VMP_FLAGS_KNFATT );
			}else{
				work->think_local = 8;
				work->dir = work->think_dir;
				work->think_timer = -1;
				UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
				work->think_mesg = THINK_MESG_THRWL_MUTEKI;
			}
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}

//近距離回避攻撃ジャンプ
static void VMP_ThinkLocal_AvoidAttNear( Work *work )
{
	switch( work->think_local ){
	  case 0:
		work->think_n_att = 0;	// 攻撃回数
		work->think_temp0 = 0;	// ジャンプ回数
		work->think_temp1 = 0;	// ダメージ回数
		work->think_to_fzn = VMP_GetRndZone( work->think_p_fzn, ZONE_INFO_CONTRAST );
		work->think_temp2 = VMP_GetZoneLen( work->think_p_fzn, work->think_to_fzn );	//プレイヤまでのゾーン番号
	  case 1:
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			return;
		}else if( work->think_local == 1 ){
			//最初からじゃない
			work->think_temp2 -= 2;
			ClearAjustToPlayer( work );
			if( work->think_temp2 <= 1 ){
				//2マス以内のときは攻撃
				if( work->think_now_main_w == THK_SUB_MAIN3_W &&
					( work->think_jumpout_pos = GetTriAttNum( &GM_PlayerPosition ) ) >= 0 &&
					1 ){//(irnd() & 0x0100000) ){
					//壁攻撃へ
					GetTriAttPosition( &work->think_init_pos, work->think_jumpout_pos );
					work->think_timer = -1;
					work->think_local = 11;
					work->think_mesg = THINK_MESG_FLR2WLL;//THINK_MESG_FLOORJUMP;
					work->think_dive_pos = work->think_jumpout_pos;
				}else if( !GM_CheckPlayerStatus( PLAYER_BEYOND ) && !CHECK_FLAG( work->think_flags, VMP_FLAGS_NO_ZONE) ){
					//近距離攻撃へ（モーション入れ替えする）
					work->think_local = 4;
					work->think_mesg = THINK_MESG_NEARJUMP;
					work->think_timer = -1;
				}else{
					int tmp;
					work->think_temp2 += 2;
					if( work->think_temp2 < 2 ) work->think_temp2 = 2;
					tmp = VMP_GetZoneRndFromLen( work->think_p_fzn, work->think_temp2, work->think_v_fzn );
					if( work->think_v_fzn == tmp ){
						//移動できない（ジャンプしなくていい）
						work->think_local = 9;
						work->think_timer = -1;
						if( work->game_diff == 4 || work->think_now_main_w == THK_SUB_MAIN3_W )
							work->think_mesg = THINK_MESG_MOVE;
						else
							work->think_mesg = THINK_MESG_WALK;					
					}else{
						work->think_to_fzn = tmp;
						work->think_mesg = THINK_MESG_FLR2FLR_F;
						work->think_local = 2;
						work->think_timer = -1;
						VMP_JumpOutPos( &work->think_init_pos, work->think_to_fzn );
					}
				}
			}else{
				//ちょっと近いところへ
				int tmp = VMP_GetZoneRndFromLen( work->think_p_fzn, work->think_temp2, work->think_v_fzn );
				if( work->think_v_fzn == tmp ){
					//移動できない（ジャンプしなくていい）
					work->think_local = 9;
					work->think_timer = -1;
					if( work->game_diff == 4 || work->think_now_main_w == THK_SUB_MAIN3_W )
						work->think_mesg = THINK_MESG_MOVE;
					else
						work->think_mesg = THINK_MESG_WALK;					
				}else{
					work->think_to_fzn = tmp;
					work->think_mesg = THINK_MESG_FLR2FLR_F;
					work->think_local = 2;
					work->think_timer = -1;
					VMP_JumpOutPos( &work->think_init_pos, work->think_to_fzn );
				}
			}
		}else{
			//最初
			int tmp = VMP_GetZoneRndFromLen( work->think_p_fzn, work->think_temp2, work->think_v_fzn );
			if( work->think_v_fzn == tmp ){
				//移動できない（ジャンプしなくていい）
				work->think_local = 9;
				work->think_timer = -1;
				if( work->game_diff == 4 || work->think_now_main_w == THK_SUB_MAIN3_W )
					work->think_mesg = THINK_MESG_MOVE;
				else
					work->think_mesg = THINK_MESG_WALK;
			}else{
				work->think_to_fzn = tmp;
				work->think_mesg = THINK_MESG_FLR2FLR_F;
				work->think_local = 2;
				work->think_timer = -1;
				VMP_JumpOutPos( &work->think_init_pos, work->think_to_fzn );
			}
		}
		work->think_temp0++;

		break;
	  case 2:
		VMP_ThinkLocal_Flr2FlrShort( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			if( work->think_p_fzn == 1 && work->think_v_fzn == 1 ){
				//ゾーン１の時 特殊処理
				work->think_local = 6;
				work->think_mesg = THINK_MESG_SLASHNEAR;
				work->think_timer = -1;
			}else{
				if( work->think_now_main_f != THK_SUB_MAIN3_F ){
					//隙無し歩きへ
					work->think_local = 9;
					work->think_timer = -1;
					//ラストは走る
					if( work->game_diff == 4 || work->think_now_main_w == THK_SUB_MAIN3_W )
						work->think_mesg = THINK_MESG_MOVE;
					else
						work->think_mesg = THINK_MESG_WALK;
				}else{
					//ラストにこれを使うときナイフ投げを入れる
					work->think_local = 8;
					work->think_timer = -1;
					work->think_mesg = THINK_MESG_THOROWL;
					work->dir = work->think_dir;
				}
			}
		}
		break;
	  case 3:
		//待ちからニアアッタクヘ
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			return;
		}else if( work->think_len < 2000.0f ){
			work->think_local = 1;
			VMP_PRINTF("近すぎだっちゃなー\n");
		}else if( work->think_timer > DIRECT_TICK(60) ){
			if( work->think_temp0 < 4 && GM_CheckPlayerStatus( PLAYER_BEYOND ) ){
				//CHECK_FLAG( work->think_flags, VMP_FLAGS_NO_ZONE)
				work->think_local = 1;
			}else{
				ClearAjustToPlayer( work );
				//VMP_SetThinkSub( work, work->think_now_main_f, 0 );
				work->think_local = 4;
				work->think_mesg = THINK_MESG_NEARJUMP;
				work->think_timer = -1;
			}
		}
		break;
	  case 4:
		VMP_ThinkLocal_Non_NearFloorJump_1st( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_FLY ) ){
			work->think_timer = -1;
			work->think_local++;
		}
		break;
	  case 5:
		VMP_ThinkLocal_Non_NearFloorJump_2nd( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			//VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
			work->think_local = 6;
			work->think_timer = -1;
			work->think_n_att++;
		}
		break;
	  case 6:
		//どーーーん
		if( !CHECK_FLAG( work->status, VMP_STATUS_NO_DIR ) ){
			work->dir = work->think_dir;
		}
		//printf( "flag %d, %d\n", CHECK_FLAG( work->status, VMP_STATUS_ATTACK ) );
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			//work->think_n_att++;
			work->think_to_fzn = VMP_GetRndZone( work->think_p_fzn, ZONE_INFO_CONTRAST );
			work->think_temp2 = VMP_GetZoneLen( work->think_p_fzn, work->think_to_fzn );
			return;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			//work->think_local++;
			//work->think_timer = -1;
			//work->think_n_att++;

			//要チェック
			if( work->think_now_main_w == THK_SUB_MAIN2_W &&
				work->think_n_att >= 2 && work->think_now_main_f != THK_SUB_MAIN3_F){
				
				VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
			}else if( work->think_now_main_w != THK_SUB_MAIN2_W &&
					  work->think_n_att >= 3 && work->think_now_main_f != THK_SUB_MAIN3_F){
				
				VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
			}else{
				
				if( GM_CheckPlayerStatus( PLAYER_DAMAGED ) ){
					work->think_local++;
					work->think_timer = -1;
					work->think_mesg = THINK_MESG_LOWBOW_CANBREAK;//THINK_MESG_LOWBOW;
				}else{
					work->think_local = 1;
					work->think_timer = -1;
					work->think_temp0 = 0;
					work->think_to_fzn = VMP_GetRndZone( work->think_p_fzn, ZONE_INFO_CONTRAST );
					work->think_temp2 = VMP_GetZoneLen( work->think_p_fzn, work->think_to_fzn );
				}
			}
		}else if( !CHECK_FLAG( work->status, VMP_STATUS_ATTACK ) && work->think_len > 2500.0f ){
			SET_FLAG( work->act_flags, VMP_ACTFLAG_ENDSPIN );
			//VMP_PRINTF("遠すぎだっちゃなー\n");
		}
		break;
	  case 7:
		//プレイヤー当たり時の待ち
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			return;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ||
			( work->think_timer > DIRECT_TICK(120) &&
			  !(GM_CheckPlayerStatus( PLAYER_DAMAGED|PLAYER_GROUND))) ){  //GM_CheckPlayerStatus( PLAYER_GROUND )) ){
		//if( work->think_timer > DIRECT_TICK(120) ){
			//work->think_n_att++;
			work->think_local = 1;
			work->think_timer = -1;
			work->think_temp0 = 0;
			work->think_to_fzn = VMP_GetRndZone( work->think_p_fzn, ZONE_INFO_CONTRAST );
			work->think_temp2 = VMP_GetZoneLen( work->think_p_fzn, work->think_to_fzn );
		}

		break;
	  case 8:
		//新しいナイフ投げ機構未対応
		//ナイフ投げをいれてー
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
		}else if( work->think_len < 2000.0f ){
			work->think_local = 1;
			//VMP_PRINTF("近すぎだっちゃなー\n");
		}else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			if( work->think_temp0 < 2 ||
				GM_CheckPlayerStatus( PLAYER_BEYOND ) ||
				CHECK_FLAG( work->think_flags, VMP_FLAGS_NO_ZONE)){
				work->think_local = 1;
			}else{
				ClearAjustToPlayer( work );
				//VMP_SetThinkSub( work, work->think_now_main_f, 0 );
				work->think_local = 4;
				work->think_mesg = THINK_MESG_NEARJUMP;
				work->think_timer = -1;
			}
		}
		break;
	  case 9:
		//たたたたたたたた
		VMP_ThinkLocal_Non_MoveRai( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
		}else if( work->think_timer > DIRECT_TICK(60*20) && !CHECK_FLAG( work->status, VMP_STATUS_NOWAVOID )){
			//逃げすぎだからジャンプへ
			work->think_local = 1;
		}else if( work->think_len < 2000.0f ){
			work->think_local = 6;
			work->think_timer = -1;
			work->think_mesg = THINK_MESG_SLASHNEAR;//THINK_MESG_SLASHSHDW;
			work->think_n_att++;
			ClearAjustToPlayer( work );
		}else if( work->think_len < 4000.0f &&
				  !CHECK_FLAG( work->think_flags, VMP_FLAGS_NO_ZONE) &&
				  !GM_CheckPlayerStatus( PLAYER_BEYOND ) ){//&& work->think_now_main_w == THK_SUB_MAIN3_W ){
			work->think_local = 1;
			work->think_temp2 = 0;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_local = 1;
			//work->think_temp2 = 0;
			//if( work->think_now_main_w == THK_SUB_MAIN3_W ) work->think_mesg = THINK_MESG_MOVE;
			//else work->think_mesg = THINK_MESG_WALK;
			//ClearAjustToPlayer( work );
			
		}
		break;
	  case 10:
		//どーん
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			return;
		}else if( work->think_len < 2000.0f ){
			work->think_local = 6;
			work->think_timer = -1;
			work->think_mesg = THINK_MESG_SLASHNEAR;//THINK_MESG_SLASHSHDW;
			work->think_n_att++;
			ClearAjustToPlayer( work );
			VMP_PRINTF("近すぎだっちゃなー\n");
		}else if( work->think_timer > DIRECT_TICK(60) ){
			work->think_local = 9;
			work->think_timer = -1;
			work->think_mesg = THINK_MESG_WALK;
		}
		break;
		
#if 1
		//ここから壁張り付きモード追加
	  case 11:
		//張り付きジャンプ
		//VMP_ThinkLocal_Non_SpiderPos( work );
		VMP_ThinkLocal_Flr2WllShort( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_mesg = THINK_MESG_SPIDER;
			work->think_local++;
			work->think_timer = -1;
			DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
			printf("aaaa %d\n",work->think_dive_pos);
		}
		break;
	  case 12:
		// 張り付き待ち
		VMP_ThinkLocal_Non_SpiderWait( work );
		if( work->think_timer > DIRECT_TICK(GameDiff_SpiderWait[work->game_diff]) ){
			work->think_mesg = THINK_MESG_TOWATERSPIDER;
			work->think_local++;
			work->think_timer = -1;
			DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
#ifdef BP_PS2 //yano
			VMP_SetAdjustWork( work, &(SVECTOR){ 0, 0, 0, 0 }, 0 );
#else
			{
			SVECTOR dmdm = { 0, 0, 0, 0 };	
			VMP_SetAdjustWork( work, &dmdm, 0 );		
			}
#endif
			ClearAjustToPlayer( work );
		}else if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
#ifdef BP_PS2 //yano
			VMP_SetAdjustWork( work, &(SVECTOR){ 0, 0, 0, 0 }, 0 );
#else
			{
			SVECTOR dmdm = { 0, 0, 0, 0 };
			VMP_SetAdjustWork( work, &dmdm, 0 );			
			}
#endif
			ClearAjustToPlayer( work );
			work->think_local = -2;
			work->think_timer = -1;
			return;
		}
		break;
	  case 13:
		//張り付きアタック
		VMP_ThinkLocal_Non_SpiderToWaterAtt( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
			VMP_SetThinkSub( work, work->think_now_main_w, 0 );
			//VMP_SetThinkSub( work, THK_SUB_WATEROUTWATER, 0 );
		}
		break;
#endif
	  case -1:
		//ダメージ（通常）
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			//if( work->think_now_main_w != THK_SUB_MAIN2_W ){
			//}
			if( work->life <= MODE_CHANGE_LIFE1 || work->faint < MODE_CHANGE_LIFE1 ){
				//ラストフェーズへ
				VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_FAR2, 0 );
				return;
			}else if( work->think_now_main_w == THK_SUB_MAIN2_W &&
				(work->think_n_att >= 1 || (work->think_temp1 > 4 && work->think_now_main_f != THK_SUB_MAIN3_F)) ){
				
				VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
				return;
			}else if( work->think_now_main_w == THK_SUB_MAIN3_W &&
				(work->think_n_att >= 2 || (work->think_temp1 > 4 && work->think_now_main_f != THK_SUB_MAIN3_F)) ){
				VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
				return;
			}
			work->think_temp1++;
			work->think_local = 1;
		}
		break;
	  case -2:
		//張り付き時のダメージ
		//VMP_ThinkLocal_Non_SpiderDamage( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			//VMP_SetThinkSub( work, THK_SUB_WATEROUTWATER, 0 );
			VMP_SetThinkSub( work, work->think_now_main_w, 0 );
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}

	//AN_Test_Eye( &work->think_init_pos, 2 );
}


static void VMP_ThinkLocal_GroundAtt( Work *work )
{	
	SET_FLAG( work->status, VMP_STATUS_STOMP);
	switch( work->think_local ){
	  case 0:
		work->think_to_fzn = VMP_GetRndZone( work->think_p_fzn, ZONE_INFO_NEAR );
		VMP_JumpOutPos( &work->think_init_pos, work->think_to_fzn );
		work->think_timer = -1;
		if( CHECK_FLAG( work->think_flags, VMP_FLAGS_LOFT ) ){
			// ２階
			work->think_mesg = THINK_MESG_LOFT2FLOOR;
			work->think_local = 1;
		}else if( CHECK_FLAG( work->think_flags, VMP_FLAGS_RAIL ) ){
			// １階手摺
			work->think_mesg = THINK_MESG_RAIL2FLOOR;
			work->think_local = 2;
		}else{
			// 床
			work->think_mesg = THINK_MESG_FLOOR2FLOOR;
			work->think_local = 3;
		}
		break;
	  case 1:
		// ２階から
		if( VMP_ThinkLocal_Non_JumpToFloor( work ) ) return;
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_SEMIEND ) ){
			work->think_timer = -1;
			work->think_local = 4;
			work->think_v_loft = -1;
			work->think_mesg = THINK_MESG_WALK;
		}
		break;
	  case 2:
		// １階手摺から
		if( VMP_ThinkLocal_Non_JumpHndRailToFloor( work ) ) return;
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_SEMIEND ) ){
			work->think_timer = -1;
			work->think_local = 4;
			work->think_v_loft = -1;
			work->think_mesg = THINK_MESG_WALK;
		}
		break;
	  case 3:
		// 床から
		if( VMP_ThinkLocal_Non_FloorJump( work ) ) return;
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_SEMIEND ) ){
			work->think_timer = -1;
			work->think_local = 4;
			work->think_v_loft = -1;
			work->think_mesg = THINK_MESG_WALK;
		}
		break;
	  case 4:
		// 移動
		VMP_ThinkLocal_Non_MoveRai( work );
		//if( work->dir < 0 ){
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			return;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_mesg = THINK_MESG_WALK;
		}else if( work->think_len < 700.0f &&
			VMP_GetZoneInfo( work->think_v_fzn, work->think_p_fzn ) < ZONE_INFO_NEAR ){
			ClearAjustToPlayer( work );
			work->think_local++	;
			work->think_timer = -1;
			work->think_mesg = THINK_MESG_GROUND_ATT;
		}else if( !CHECK_FLAG( work->think_flags, VMP_FLAGS_GROUND|VMP_FLAGS_BEYOND)){
			if( work->think_now_main_f == THK_SUB_MAIN3_F ) VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_FAR2, 0 );
			else VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
		}
		break;
	  case 5:
		// 攻撃
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			//VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
			//if( CHECK_FLAG( work->think_flags, VMP_FLAGS_GROUND|VMP_FLAGS_BEYOND) ){
				work->think_timer = -1;
				work->think_local = 4;
				work->think_mesg = THINK_MESG_WALK;
				work->think_ground_timer = DIRECT_TICK(60);
				work->think_beyond_timer = DIRECT_TICK(60);
			//}else{
			//}
			//work->think_timer = -1;
			//work->think_local = 5;
			//work->think_mesg = THINK_MESG_GROUND_ATT;
		}
		break;
	  case 6:
		break;
		
	  case -1:
		//ダメージ
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
			//VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_NEAR, 0 );
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}

static void VMP_ThinkLocal_DeadEnd( Work *work )
{

	switch( work->think_local ){
	  case 0:
		work->think_mesg = THINK_MESG_DAMAGE;
		work->think_local++;
		if( work->end_proc ) GCL_ExecProc( work->end_proc, NULL );
		break;
	  case 1:
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			if( work->control.mov.vy > -3000.0f ){
				//FVECTOR	fvtemp;
				//SVECTOR rot;
				work->think_timer = -1;
				work->think_local = 3;
				work->think_dive_pos = 19;
				VMP_DivePos( &work->think_init_pos, work->think_dive_pos );
				work->think_mesg = THINK_MESG_LOFT2WATER;
			}else{
				FVECTOR	fvtemp;
				SVECTOR rot;
				work->think_mesg = THINK_MESG_DIVE;
				work->think_timer = -1;
				work->think_local = 2;
				work->think_dive_pos = 19;
				VMP_DivePos( &fvtemp, work->think_dive_pos );
				_sceVu0SubVector( &fvtemp, &fvtemp, &work->control.mov );
				_sceVu0Normalize( &fvtemp, &fvtemp );
				TS_VecToRot( &rot, &fvtemp );
				if( rot.vy < 0 ) rot.vy += 4096;
				work->dir = rot.vy;
			}
		}
		break;
	  case 2:
		VMP_ThinkLocal_Non_Dive( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_local = 4;
			work->control.mov.vy = -5000.0f;
			work->think_mesg = THINK_MESG_DEADEND;
		}
		break;
	  case 3:
		VMP_ThinkLocal_Non_DiveLoftWater( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_local = 4;
			work->control.mov.vy = -5000.0f;
			work->think_mesg = THINK_MESG_DEADEND;
		}
		break;
	  case 4:
		work->control.step.vy = 0.0f;
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			work->think_local = -1;
		}
		break;
	  case -1:
		//ダメージ
		work->control.step.vy = 0.0f;
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_local = 4;
			work->think_mesg = THINK_MESG_DEADEND;
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
	
	//DEBUG_Locate( 230, 224, 0 );
	//DEBUG_Printf( "CLEAR" );

}

//三角落とし　Z軸方向　水からスタート
static void VMP_ThinkLocal_TriAtt( Work *work )
{
	switch( work->think_local ){
	  case 0:
		work->think_dive_pos = GetTriAttNum( &GM_PlayerPosition );
		if( work->think_dive_pos < 0 ){
			VMP_SetThinkSub( work, THK_SUB_WATEROUTWATER, 1 );
			work->think_n_att = 5;
			return;
		}
		work->think_jumpout_pos = GetTriAttDammyNum( work->think_dive_pos );
		work->think_to_wzn = 19;
		work->think_local++;
		work->think_timer = -1;
		//work->control.mov.vy = -8000.0f;
	  case 1:
		//移動中継1	取り合えず19へ
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			work->think_local++;
			work->think_timer = -1;
			work->think_to_wzn = GetGoSplashWaterNum_1st( work->think_jumpout_pos );
		}
		break;
	  case 2:
		//移動中継2 回り込む
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			work->think_local++;
			work->think_timer = -1;
			work->think_to_wzn = GetGoSplashWaterNum_2nd( work->think_jumpout_pos );
		}
		break;
	  case 3:
		//移動中継2 回り込む
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			work->think_local++;
			work->think_timer = -1;
			work->think_to_wzn = 19;
		}
		break;
	  case 4:
		//移動中継3 19へ
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			work->think_local++;
			work->think_timer = -1;
			work->think_jumpout_pos = work->think_dive_pos;
			GetTriAttPosition( &work->think_init_pos, work->think_jumpout_pos );
		}
		break;
	  case 5:
		VMP_ThinkLocal_Non_SwimPosUp( work );
		if( work->control.mov.vy > -8000.0f ){
			work->think_timer = -1;
			work->think_local++;
			work->dir_x = 0;
			work->think_mot_step.vy = 0.0f;
			if( GetTriAttNum( &GM_PlayerPosition ) != work->think_dive_pos ){
				VMP_SetThinkSub( work, THK_SUB_WATEROUTWATER, 7 );
				work->think_n_att = 5;
				return;
			}
			work->control.mov.vy = -8000.0f;
			work->think_mesg = THINK_MESG_WATER2WALL;//THINK_MESG_TOSPIDERWATER;
		}
		break;

	  case 6:
		//ジャンプ
		VMP_ThinkLocal_Non_SpiderPos( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_mesg = THINK_MESG_SPIDER;
			work->think_local++;
			work->think_timer = -1;
			//VMP_SetAdjustWork( work, &(SVECTOR){ 300, 0, 0, 0 }, 0 );
			//VMP_SetAdjustWork( work, &(SVECTOR){ -150, 0, 0, 0 }, 1 );
			//printf("aaaa %d\n",work->think_dive_pos);
		}
		break;
	  case 7:
		// wait
		VMP_ThinkLocal_Non_SpiderWait( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
#ifdef BP_PS2 //yano
			VMP_SetAdjustWork( work, &(SVECTOR){ 0, 0, 0, 0 }, 0 );
#else
			{
			SVECTOR dmdm = { 0, 0, 0, 0 };
			VMP_SetAdjustWork( work, &dmdm, 0 );			
			}
#endif
			ClearAjustToPlayer( work );
			work->think_local = -1;
			work->think_timer = -1;
			return;
		}else if( work->think_timer > DIRECT_TICK(GameDiff_SpiderWait[work->game_diff]) ){
			work->think_mesg = THINK_MESG_TOWATERSPIDER;
			work->think_local++;
			work->think_timer = -1;
#ifdef BP_PS2
			VMP_SetAdjustWork( work, &(SVECTOR){ 0, 0, 0, 0 }, 0 );
#else
			{
			SVECTOR dmdm ={ 0, 0, 0, 0 };
			VMP_SetAdjustWork( work, &dmdm, 0 );			
			}
#endif
			ClearAjustToPlayer( work );
		}
		break;
	  case 8:
		VMP_ThinkLocal_Non_SpiderToWaterAtt( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
			VMP_SetThinkSub( work, work->think_now_main_w, 0 );
			//VMP_SetThinkSub( work, THK_SUB_WATEROUTWATER, 0 );
		}
		break;
	  case -1:
		//VMP_ThinkLocal_Non_SpiderDamage( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			//VMP_SetThinkSub( work, THK_SUB_WATEROUTWATER, 0 );
			VMP_SetThinkSub( work, work->think_now_main_w, 0 );
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}

//水ぶっ掛け 水からスタート
static void VMP_ThinkLocal_SplashWater( Work *work )
{
	switch( work->think_local ){
	  case 0:
		work->think_n_att = 0;
	  case 1:
		//ランダム水掛ポイント選択 go 3
		work->think_jumpout_pos = GetGoSplashWaterNum( GetOutRandNum( 2, 13, work->think_p_fzn ) );
		work->think_to_wzn = 19;
		work->think_local = 3;
		work->think_timer = -1;
		work->think_n_att++;
		work->control.mov.vy = -8000.0f;

		
			//work->think_to_wzn = GetGoSplashWaterNum_1st( work->think_jumpout_pos );
		break;
	  case 2:
		//水掛ポイント選択	go 3
		if( work->think_p_fzn < 2){
			work->think_jumpout_pos = GetGoSplashWaterNum( GetOutRandNum( 2, 13, work->think_p_fzn ) );
		}else{
			work->think_jumpout_pos = GetGoSplashWaterNum( work->think_p_fzn );
		}
		work->think_to_wzn = 19;
		work->think_local = 3;
		work->think_timer = -1;
		work->think_n_att++;
		work->control.mov.vy = -8000.0f;

		
			//work->think_to_wzn = GetGoSplashWaterNum_1st( work->think_jumpout_pos );
		break;

	  case 3:
		//移動中継1	取り合えず19へ
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			work->think_local++;
			work->think_timer = -1;
			work->think_to_wzn = GetGoSplashWaterNum_1st( work->think_jumpout_pos );
		}
		break;
	  case 4:
		//移動中継2 回り込む
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			work->think_local++;
			work->think_timer = -1;
			work->think_to_wzn = GetGoSplashWaterNum_2nd( work->think_jumpout_pos );
		}
		break;
	  case 5:
		//移動中継2 回り込む
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			work->think_local++;
			work->think_timer = -1;
			work->think_to_wzn = 19;
			
			//スパイダーチェック
			if( (work->think_dive_pos = GetTriAttNum( &GM_PlayerPosition )) >= 0 )
				if( (work->think_dive_pos == 0 &&
					 (GetGoSplashWaterNum_1st( work->think_jumpout_pos ) == 14 ||
					  GetGoSplashWaterNum_1st( work->think_jumpout_pos ) == 7 )) ||
					(work->think_dive_pos == 1 && GetGoSplashWaterNum_1st( work->think_jumpout_pos ) == 4 ) ){
					VMP_SetThinkSub( work, THK_SUB_WATEROUTSPIDE, 4 );
					VMP_PRINTF( "割込みング\n" );
			}
		}
		break;
	  case 6:
		//移動中継3 19へ
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			work->think_local++;
			work->think_timer = -1;
			//work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;
			
			//VMP_SetThinkMulStep( work, 0.0f, 1.0f, 0.0f );
			//work->think_mesg = THINK_MESG_WATEROUTWATER;
			
			VMP_DivePos( &work->think_init_pos, work->think_jumpout_pos );
		}
		break;
	  case 7:
		VMP_ThinkLocal_Non_SwimPosUp( work );
		if( work->control.mov.vy > -8000.0f ){
			work->think_timer = -1;
			work->think_local++;
			work->dir_x = 0;
			work->think_mot_step.vy = 0.0f;
			work->control.mov.vy = -8000.0f;
			work->think_mesg = THINK_MESG_WATEROUTWATER;
		}
		break;
	  case 8:
		//ジャンプ
		VMP_ThinkLocal_Non_WaterToWater_Splash( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			if(work->think_n_att < 3){
				work->think_local = 1;
				work->think_timer = -1;
			}else if(work->think_n_att < 2){
				work->think_local = 2;
				work->think_timer = -1;
			}else{
				VMP_SetThinkSub( work, work->think_now_main_w, 0 );
			}
		}
		
		break;
	  case 9:
		//終了処理（1,2へ）満足したら終わり
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}


//床からダイヴ
static void VMP_ThinkLocal_DiveFloor( Work *work )
{
	switch( work->think_local ){
	  case 0:
		{
			FVECTOR	fvtemp;
			SVECTOR rot;
			work->think_mesg = THINK_MESG_DIVE;
			work->think_timer = -1;
			work->think_local++;
			work->think_dive_pos = VMP_GetDivePosNum( work->think_v_fzn );
			VMP_DivePos( &fvtemp, work->think_dive_pos );
			ClearAjustToPlayer( work );
			_sceVu0SubVector( &fvtemp, &fvtemp, &work->control.mov );
			_sceVu0Normalize( &fvtemp, &fvtemp );
			TS_VecToRot( &rot, &fvtemp );
			if( rot.vy < 0 ) rot.vy += 4096;
			work->dir = rot.vy;
		}
		break;
	  case 1:
		VMP_ThinkLocal_Non_Dive( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			//VMP_SetThinkSub( work, work->think_now_main_w, 0 );
			if( work->think_now_main_w == THK_SUB_MAIN3_W ) VMP_SetThinkSub( work, work->think_now_main_w, 0 );
															//VMP_SetThinkSub( work, THK_SUB_SWIMINGEX, 0 );
			else VMP_SetThinkSub( work, THK_SUB_SWIMING, 0 );
			DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
//			work->think_local++;
//			work->dir = -1;
//			work->think_to_wzn = 19;
			work->control.mov.vy = -8000.0f;
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}

//2階からダイヴ
static void VMP_ThinkLocal_DiveLoft( Work *work )
{
	switch( work->think_local ){
	  case 0:
		//戻る
		work->think_timer = -1;
		work->think_local++;
		work->think_dive_pos = 19;
		work->think_mesg = THINK_MESG_LOFT2WATER;//THINK_MESG_DIVELOFT;
		break;
	  case 1:
		VMP_ThinkLocal_Non_DiveLoftWater( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			//VMP_SetThinkSub( work, work->think_now_main_w, 0 );
			if( work->think_now_main_w == THK_SUB_MAIN3_W ) VMP_SetThinkSub( work, THK_SUB_SWIMINGEX, 0 );
			else VMP_SetThinkSub( work, THK_SUB_SWIMING, 0 );
			DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
			//VMP_SetThinkSub( work, THK_SUB_SWIMING, 0 );
			work->think_mot_step.vx = 0.0f;
			work->think_mot_step.vy = 0.0f;
			work->think_mot_step.vz = 0.0f;
			work->think_mot_step.vw = 0.0f;
			
			//work->think_local++;
			//work->dir = -1;
			//work->think_to_fzn = VMP_GetRndZone( work->think_p_fzn, ZONE_INFO_TOIMEN );
			//work->think_to_wzn = VMP_GetNearWaterZoneFromFloor( work->think_to_fzn, -1 );
			work->control.mov.vy = -8000.0f;
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}

//手摺からダイヴ
static void VMP_ThinkLocal_DiveRail( Work *work )
{
	switch( work->think_local ){
	  case 0:
		//戻る
		work->think_timer = -1;
		work->think_local++;
		work->think_dive_pos = 19;
		VMP_DivePos( &work->think_init_pos, work->think_dive_pos );
		work->think_mesg = THINK_MESG_RAIL2WATER;//THINK_MESG_DIVELOFT;
		break;
	  case 1:
		VMP_ThinkLocal_Non_DiveRailWater( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			//VMP_SetThinkSub( work, work->think_now_main_w, 0 );
			VMP_SetThinkSub( work, THK_SUB_SWIMING, 0 );
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}

//影攻撃
static void VMP_ThinkLocal_ShadowAtt( Work *work )
{
	switch( work->think_local ){
	  case 0:
		if( !VMP_CheckShdwBind() &&
			VMP_CheckShdw() &&
			(work->think_jumpout_pos = VMP_GetToShdwAttNum( work->think_p_fzn )) >= 0){
			VMP_DivePos( &work->think_init_pos, work->think_jumpout_pos );
			work->think_timer = -1;
			work->think_local++;

		}else{
			VMP_SetThinkSub( work, work->think_now_main_w, 0 );
			VMP_PRINTF("だめだめ\n");
			return;
		}
		break;
	  case 1:
		VMP_ThinkLocal_Non_SwimPosUpHigh( work );
		//VMP_ThinkLocal_Non_SwimPosUp( work );
		if( work->control.mov.vy > -8000.0f ){
			work->think_timer = -1;
			work->think_local++;
			work->dir_x = 0;
			work->think_mot_step.vy = 0.0f;
			//work->control.mov.vy = -8000.0f;
			work->think_mesg = THINK_MESG_THRWSHDW;
			work->think_timer = -1;
		}
		break;
	  case 2:
		VMP_ThinkLocal_Non_WaterToWater( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_local++;
			work->think_to_fzn = VMP_GetRndZone( work->think_p_fzn, ZONE_INFO_TOIMEN );
			work->think_jumpout_pos = work->think_to_fzn;
			work->think_dive_pos = VMP_GetDivePosNum( work->think_jumpout_pos );
			work->think_to_wzn = work->think_dive_pos;
			work->think_timer = -1;
			if( !VMP_CheckShdwBind() ){
				VMP_SetThinkSub( work, work->think_now_main_w, 0 );
			}
		}
		break;
	  case 3:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		//work->control.mov.vy = -8000.0f;
		if( work->dir < 0 ){
			//work->think_mesg = THINK_MESG_JUMPOUT;
			work->think_timer = -1;
			work->think_local++;
			//DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
			//work->dir = work->think_dir;
		}
		break;

	  case 4:
		VMP_ThinkLocal_Non_SwimPosUp_Invr( work );
		//VMP_ThinkLocal_Non_SwimPosUp( work );
		//work->control.mov.vy = -8000.0f;
		if( work->control.mov.vy > -8000.0f ){
		//if( work->dir < 0 ){
			work->think_mesg = THINK_MESG_JUMPOUT;
			work->think_timer = -1;
			work->think_local++;
			DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
			//work->dir = work->think_dir;
		}
		break;

	  case 5:
		VMP_ThinkLocal_Non_JumpOutFloor( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_timer = -1;
			work->think_local++;
			work->think_mesg = THINK_MESG_WALK;
			ClearAjustToPlayer( work );
		}
		break;
		
	  case 6:
		if( !VMP_CheckShdwBind() ){
			int	se_table[] = {
				SD_V_VMPAKT01, //ヴァンプ影縛解除悪態１「ちっ」
				SD_V_VMPAKT02, //ヴァンプ影縛解除悪態２「えぃ」
			};
			GM_SeSetMode( se_table[irnd()%2], &work->control.mov, GM_SEMODE_BOMB );
			work->think_mesg = THINK_MESG_MOVE;
			work->think_local++;
		}else if( work->think_timer == 0 ){
			VMP_StartStream( work, irnd()&1 );
		}

	  case 7:
		//if( CHECK_FLAG( work->status, VMP_STATUS_NOWAVOID|VMP_STATUS_ACT_END ) ==
		//	(VMP_STATUS_NOWAVOID|VMP_STATUS_ACT_END) ){
		VMP_ThinkLocal_Non_MoveRai( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			work->think_local = -1;
			work->think_timer = -1;
		}else if( work->think_len < 2000.0f &&
			VMP_GetZoneInfo( work->think_v_fzn, work->think_p_fzn ) < ZONE_INFO_NEAR ){
			ClearAjustToPlayer( work );
			work->think_local++	;
			work->think_mesg = THINK_MESG_SLASHNEAR;//THINK_MESG_SLASHSHDW;
			work->think_timer = -1;
			ClearAjustToPlayer( work );	
		}else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			if( work->think_local == 6 ) work->think_mesg = THINK_MESG_WALK;
			else work->think_mesg = THINK_MESG_MOVE;
			ClearAjustToPlayer( work );
		}
		break;
	  case 8:
		if( !CHECK_FLAG( work->status, VMP_STATUS_NO_DIR ) ){
			work->dir = work->think_dir;
		}
		
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			
			VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_NEAR, 0 );
			//VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
		}else if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			work->think_local = -1;
			work->think_timer = -1;
		}else if( !CHECK_FLAG( work->status, VMP_STATUS_ATTACK ) && work->think_len > 2500.0f ){
			SET_FLAG( work->act_flags, VMP_ACTFLAG_ENDSPIN );
		}
		break;
	  case -1:
		//ダメージ
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_NEAR, 0 );
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}

//飛び出しナイフ攻撃(しゅびびんまん)
static void VMP_ThinkLocal_JumpOutAtt( Work *work )
{
	switch( work->think_local ){
	  case 0:
		work->think_local++;
		work->dir = -1;
		work->think_to_wzn = ((irnd()%5)+5+work->think_v_wzn)%18;
		work->think_timer = -1;
		break;
	  case 1:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			work->think_timer = -1;
			work->think_local++;
			work->think_to_wzn = 19;
			//work->dir = work->think_dir;
		}
		break;
	  case 2:
		//中心へ
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			//work->think_mesg = THINK_MESG_JUMPOUTWATER;
			work->think_timer = -1;
			work->think_local++;
			DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
			work->dir = (work->think_dir+2048)%4096;
		}
		break;
	  case 3:
		VMP_ThinkLocal_Non_SwimPosUp( work );
		if( work->control.mov.vy > -8000.0f ){
			work->think_timer = -1;
			work->think_local++;
			work->dir_x = 0;
			work->think_mot_step.vy = 0.0f;
			work->control.mov.vy = -8000.0f;
			work->think_mesg = THINK_MESG_JUMPOUTWATER;
		}
		
		break;
	  case 4:
		VMP_ThinkLocal_Non_JumpOutWater( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_local++;
			//work->think_mesg = THINK_MESG_THOROWL3;
		}
		break;
	  case 5:
		VMP_SetThinkSub( work, work->think_now_main_w, 0 );
		work->control.mov.vy = -8000.0f;
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}

}

//水中から２階へ攻撃して水中へそして出てくる くるくる
static void VMP_ThinkLocal_WaterLoftWaterAtt( Work *work )
{
	int temp;
	//float len;
	FVECTOR diff;
	switch( work->think_local ){
#if 0
	  case 0:
		work->think_local++;
		work->dir = -1;
		work->think_to_wzn = 19;
		//work->act_flags &= ~VMP_ACTFLAG_SHDW;
		work->think_tmp_max_att = 8 + work->think_damnum-2+1;
		work->think_n_att = 0;
		break;
	  case 1:
		//中心へ
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			//work->think_mesg = THINK_MESG_JUMPOUTLOFT;
			work->think_timer = -1;
			work->think_local++;
			DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
			//work->dir = work->think_dir;
			work->think_v_loft = VMP_GetLoftJumpOutPos2( &work->think_init_pos, NULL, work->think_pre_v_wzn );
		}
		break;
	  case 2:
		VMP_ThinkLocal_Non_SwimPosUp( work );
		if( work->control.mov.vy > -8000.0f ){
			work->think_timer = -1;
			work->think_local++;
			work->dir_x = 0;
			work->think_mot_step.vy = 0.0f;
			work->control.mov.vy = -8000.0f;
			work->think_mesg = THINK_MESG_WATER2LOFT;
			//work->think_mesg = THINK_MESG_WATEROUTWATER;
		}
		break;
	  case 3:
		if( VMP_ThinkLocal_Non_JumpOutLoft2( work ) ) return;
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_local++;
			work->think_timer = -1;
			work->think_mesg = THINK_MESG_SPIN;
			work->n_spin0 = 1;
			work->n_spin1 = work->think_tmp_max_att;
			VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
			{
				float ftemp = VMP_GetSpinScale( &work->control.mov, work->think_v_loft );
				work->spin_scale_vec = ftemp/(float)(work->think_tmp_max_att*16);
			}
			work->think_turn = 0;//irnd()&0x0040;
		}
		break;
#else
	  case 0:

		if( work->think_p_fzn == 1 || ( 2 <= work->think_p_fzn && work->think_p_fzn < 4 ) ||
			( 13 <= work->think_p_fzn && work->think_p_fzn < 14 ) ){
			work->think_pre_v_wzn = 18;
		}else if( 4 <= work->think_p_fzn && work->think_p_fzn < 7 ){
			work->think_pre_v_wzn = 22;
		}else if( work->think_p_fzn == 0 || ( 7 <= work->think_p_fzn && work->think_p_fzn < 10 ) ){
			work->think_pre_v_wzn = 20;
		}else if( 10 <= work->think_p_fzn && work->think_p_fzn < 13 ){
			work->think_pre_v_wzn = 21;
		}else{
			VMP_PRINTF( "ERR!! TRANS\n" );
		}
		work->think_tmp_max_att = 4 + work->think_damnum;
		
		if( work->think_tmp_max_att > 16 ) work->think_tmp_max_att = 16;
		VMP_PRINTF("spin num [%d]\n", work->think_tmp_max_att );
		work->think_n_att = 0;
		ClearAjustToPlayer( work );
		work->think_timer = -1;
		work->think_move_cnt = 0;
		work->think_local++;
		work->think_mesg = THINK_MESG_LOFT2LOFT;
		work->think_temp0 = VMP_GetLoftJumpOutPos2( &work->think_init_pos, NULL, work->think_pre_v_wzn );
		//work->think_v_loft = VMP_GetLoftJumpOutPos2( &work->think_init_pos, NULL, work->think_pre_v_wzn );

		_sceVu0SubVector( &diff, &work->think_init_pos, &work->control.mov );
		if( _Vu0VecLenXZ( &diff ) < 2000.0f ){
#if 0
			work->think_local = 4;
			work->think_timer = -1;
			work->think_mesg = THINK_MESG_SPIN;
			work->n_spin0 = 1;
			work->n_spin1 = work->think_tmp_max_att;
			VMP_GetLoftFixPosEx( work->think_temp0, &work->control.mov );
			{
				float ftemp = VMP_GetSpinScale( &work->control.mov, work->think_temp0 );
				work->spin_scale_vec = ftemp/(float)(work->think_tmp_max_att*16);
			}
			printf("v %d:temp %d\n",work->think_v_loft,work->think_temp0);
			work->think_turn = 0;//irnd()&0x0040;
#else
			// 歩きにする
			SVECTOR rot;
			_sceVu0Normalize( &diff, &diff );
			TS_VecToRot( &rot, &diff );			
			//if( rot.vy > 0 ) work->think_turn = 1;
			//else work->think_turn = 0;

			if( work->think_v_loft != work->think_temp0 ){
				switch( work->think_v_loft ){
				  case 0:
					if( work->think_temp0 == 1 ) work->think_turn = 0;
					else if( work->think_temp0 == 5 ) work->think_turn = 1;
					else VMP_PRINTF("ERR!!\n");
					break;
				  case 1:
					if( work->think_temp0 == 2 ) work->think_turn = 0;
					else if( work->think_temp0 == 0 ) work->think_turn = 1;
					else VMP_PRINTF("ERR!!\n");
					break;
				  case 2:
					if( work->think_temp0 == 3 ) work->think_turn = 0;
					else if( work->think_temp0 == 1 ) work->think_turn = 1;
					else VMP_PRINTF("ERR!!\n");
					break;
				  case 3:
					if( work->think_temp0 == 4 ) work->think_turn = 0;
					else if( work->think_temp0 == 2 ) work->think_turn = 1;
					else VMP_PRINTF("ERR!!\n");
					break;
				  case 4:
					if( work->think_temp0 == 5 ) work->think_turn = 0;
					else if( work->think_temp0 == 3 ) work->think_turn = 1;
					else VMP_PRINTF("ERR!!\n");
					break;
				  case 5:
					if( work->think_temp0 == 0 ) work->think_turn = 0;
					else if( work->think_temp0 == 4 ) work->think_turn = 1;
					else VMP_PRINTF("ERR!!\n");
					break;
				}
			}else{
				work->think_turn = VMP_GetTurnToPos( &work->control.mov, &work->think_init_pos, work->think_temp0 );
			}
			printf("v %d:temp %d\n",work->think_v_loft,work->think_temp0);
			work->think_local = 2;
			work->think_timer = -1;
			work->think_mesg = THINK_MESG_LOFTMOVE;
			//VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
#endif
		}else{
			work->think_v_loft = work->think_temp0;
		}

		break;
	  case 1:
		//２階から２階へ
		if( VMP_ThinkLocal_Non_JumpLoft( work ) ) return;
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_SEMIEND ) ){
			work->think_local = 4;
			work->think_timer = -1;
			work->think_mesg = THINK_MESG_SPIN;
			work->n_spin0 = 1;
			work->n_spin1 = work->think_tmp_max_att;
			VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
			{
				float ftemp = VMP_GetSpinScale( &work->control.mov, work->think_v_loft );
				work->spin_scale_vec = ftemp/(float)(work->think_tmp_max_att*DIRECT_TICK(16));
			}
			work->think_turn = 0;//irnd()&0x0040;
		}
		break;
#if 1
	  case 2:
		//移動
		temp = VMP_GetLoftDir( &work->dir, &work->think_v_loft, &work->control.mov, work->think_turn );
		_sceVu0SubVector( &diff, &work->think_init_pos, &work->control.mov );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END )){
			work->think_mesg = THINK_MESG_LOFTMOVE;
		}
		if( _Vu0VecLenXZ( &diff ) < 400.0f && work->think_v_loft == work->think_temp0 ){
			work->think_local = 4;
			work->think_timer = -1;
			work->think_mesg = THINK_MESG_SPIN;
			work->n_spin0 = 1;
			work->n_spin1 = work->think_tmp_max_att;
			VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
			//VMP_GetLoftFixPos( work->think_v_loft, &work->control.mov );
			{
				float ftemp = VMP_GetSpinScale( &work->control.mov, work->think_v_loft );
				work->spin_scale_vec = ftemp/(float)(work->think_tmp_max_att*16);
			}
			work->think_turn = 0;
		}else if( temp >= 0 ){
			//FVECTOR	fvtemp;
			work->think_local++;
			work->think_mesg = THINK_MESG_LOFTJUMP;
			work->think_timer = -1;			
			VMP_GetLinePos( &work->think_init_pos, temp );
		}
		break;
	  case 3:
		//ジャンプ
		VMP_ThinkLocal_Loft2LoftShort( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END )){
			work->think_local--;
			work->think_mesg = THINK_MESG_LOFTMOVE;
		}
		break;
#endif
#endif
	  case 4:
		//移動

		VMP_GetLoftDirNoChangeLoftNum( &work->dir, &work->think_v_loft, &work->control.mov, work->think_turn );
		//VMP_GetLoftDir( &work->dir, &work->think_v_loft, &work->control.mov, work->think_turn );
		//if(work->think_timer >= DIRECT_TICK(210)){
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			return;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END )){
			work->think_timer = -1;
			work->think_local++;
			if( work->think_tmp_max_att < 12 ) work->think_mesg = THINK_MESG_TURNEND;
			else work->think_mesg = THINK_MESG_DIZZY;
			//work->think_mesg = THINK_MESG_DIZZY;
			//ClearAjustToPlayer( work );
		}else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_SEMIEND ) ){
			VMP_ThinkThrwKnfLine( work );
			work->think_n_att++;
			work->think_timer = -1;
		}

		if( CHECK_FLAG( work->status, VMP_STATUS_NOWAVOID ) && work->think_timer == DIRECT_TICK(2) ){
			SET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
			//printf("att flag set\n");
		}
/*
		if( work->think_n_att > work->think_tmp_max_att ){
			//SET_FLAG( work->act_flags, VMP_ACTFLAG_ENDSPIN);
			//work->think_timer = -1;
			//work->think_local++;
			//work->think_mesg = THINK_MESG_TURNEND;
			//ClearAjustToPlayer( work );
		}
*/
		break;
	  case 5:
		//待ち
		VMP_GetLoftDirNoChangeLoftNum( &work->dir, &work->think_v_loft, &work->control.mov, work->think_turn );
		//temp = VMP_GetLoftDir( &work->dir, &work->think_v_loft, &work->control.mov, work->think_turn );
		//VMP_GetLoftDir( &work->dir, &work->think_v_loft, &work->control.mov, work->think_turn );
		//VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
		//SetAjustToPlayer( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			return;
		}/*else if( temp >= 0 ){
			//FVECTOR	fvtemp;
			//VMP_PRINTF( "jump to %d\n", temp );
			work->think_local++;
			work->think_mesg = THINK_MESG_LOFTJUMP;
			work->think_timer = -1;
			
			VMP_GetLinePos( &work->think_init_pos, temp );

			ClearAjustToPlayer( work );
		}*/
		else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END )){
			//if( work->think_now_main_w == THK_SUB_MAIN2_W ){
			//	VMP_SetThinkSub( work, THK_SUB_JUMPLOFTTOFLOOR, 0 );
			//}else{
				VMP_SetThinkSub( work, THK_SUB_DIVE_LOFT, 0 );
			//}
		}
		break;
	  case 6:
		//ジャンプ
		VMP_ThinkLocal_Loft2LoftShort( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END )){
			//if( work->think_now_main_w == THK_SUB_MAIN2_W ){
			//	VMP_SetThinkSub( work, THK_SUB_JUMPLOFTTOFLOOR, 0 );
			//}else{
				VMP_SetThinkSub( work, THK_SUB_DIVE_LOFT, 0 );
			//}
		}
		break;

	  case -1:
		//ダメージ
		VMP_GetLoftDirNoChangeLoftNum( &work->dir, &work->think_v_loft, &work->control.mov, work->think_turn );
		//VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			VMP_SetThinkSub( work, THK_SUB_DIVE_LOFT, 0 );
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}

//遊泳
static void VMP_ThinkLocal_Swim( Work *work )
{
	if( (1 <= work->think_local && work->think_local < 7 ) &&
//		work->wtr_blast_count > 3 ){
		work->o2 <= 0 ){
		work->think_local = 7;
		work->think_timer = -1;
		work->think_to_wzn = 19;
	}

	switch( work->think_local ){
	  case 0:
		{
			short to_table[4] = { 18, 20, 21, 22 };
			work->think_local = (irnd()>>6)%4;
			work->dir = -1;
			work->think_to_wzn = to_table[work->think_local];
			work->think_local++;
			work->think_n_att = 0;
			work->think_timer = -1;
		}
		break;
	  case 1:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->think_v_wzn == work->think_to_wzn ){
			work->think_local = 5;
			work->dir = -1;
			work->think_to_wzn = 3;
		}
		break;
	  case 2:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->think_v_wzn == work->think_to_wzn ){
			short to_table[2] = { 8, 13 };
			work->think_local = 5;
			work->dir = -1;
			work->think_to_wzn = to_table[(irnd()>>6)%2];
		}
		break;
	  case 3:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->think_v_wzn == work->think_to_wzn ){
			short to_table[2] = { 3, 8 };
			work->think_local = 5;
			work->dir = -1;
			work->think_to_wzn = to_table[(irnd()>>6)%2];
		}
		break;
	  case 4:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->think_v_wzn == work->think_to_wzn ){
			work->think_local = 5;
			work->dir = -1;
			work->think_to_wzn = 13;
		}
		break;
	  case 5:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->think_v_wzn == work->think_to_wzn ){
			work->think_local++;
			work->think_timer = -1;
			if( work->think_pre_v_wzn > work->think_to_wzn )
				work->think_to_wzn = work->think_to_wzn-1;
			else
				work->think_to_wzn = work->think_to_wzn+1;				
		}
		break;
		
	  case 6:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->think_v_wzn == work->think_to_wzn ){
			work->think_local++;
			work->think_timer = -1;
			work->think_to_wzn = 19;
		}
		break;
	  case 7:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->think_v_wzn == work->think_to_wzn ){

			VMP_SetThinkSub( work, work->think_now_main_w, 0 );

			if( work->think_p_fzn == 1 || ( 2 <= work->think_p_fzn && work->think_p_fzn < 4 ) ||
				( 13 <= work->think_p_fzn && work->think_p_fzn < 14 ) ){
				work->think_pre_v_wzn = 18;
			}else if( 4 <= work->think_p_fzn && work->think_p_fzn < 7 ){
				work->think_pre_v_wzn = 22;
			}else if( work->think_p_fzn == 0 || ( 7 <= work->think_p_fzn && work->think_p_fzn < 10 ) ){
				work->think_pre_v_wzn = 20;
			}else if( 10 <= work->think_p_fzn && work->think_p_fzn < 13 ){
				work->think_pre_v_wzn = 21;
			}else{
				VMP_PRINTF( "ERR!!\n" );
			}
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}


static void VMP_ThinkLocal_SwimEx( Work *work )
{

	if( (1 <= work->think_local && work->think_local < 7 ) &&
		//work->wtr_blast_count > 3 ){
		work->o2 <= 0 ){
		work->think_local = 7;
		work->think_timer = -1;
		work->think_to_wzn = 19;
	}
	switch( work->think_local ){
	  case 0:
		{
			short to_table[4] = { 18, 20, 21, 22 };
			work->think_local = (irnd()>>6)%4;
			work->dir = -1;
			work->think_to_wzn = to_table[work->think_local];
			work->think_local++;
			work->think_n_att = 0;
			work->think_timer = -1;
		}
		break;
	  case 1:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->think_v_wzn == work->think_to_wzn ){
			work->think_local = 5;
			work->dir = -1;
			work->think_to_wzn = 3;
		}
		break;
	  case 2:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->think_v_wzn == work->think_to_wzn ){
			short to_table[2] = { 8, 13 };
			work->think_local = 5;
			work->dir = -1;
			work->think_to_wzn = to_table[(irnd()>>6)%2];
		}
		break;
	  case 3:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->think_v_wzn == work->think_to_wzn ){
			short to_table[2] = { 3, 8 };
			work->think_local = 5;
			work->dir = -1;
			work->think_to_wzn = to_table[(irnd()>>6)%2];
		}
		break;
	  case 4:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->think_v_wzn == work->think_to_wzn ){
			work->think_local = 5;
			work->dir = -1;
			work->think_to_wzn = 13;
		}
		break;
	  case 5:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->think_v_wzn == work->think_to_wzn ){
			work->think_local++;
			work->think_timer = -1;
			if( work->think_pre_v_wzn > work->think_to_wzn )
				work->think_to_wzn = work->think_to_wzn-1;
			else
				work->think_to_wzn = work->think_to_wzn+1;				
		}
		break;
		
	  case 6:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->think_v_wzn == work->think_to_wzn ){
			work->think_local++;
			work->think_timer = -1;
			work->think_to_wzn = 19;
		}
		break;
	  case 7:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		//if( work->think_v_wzn == work->think_to_wzn ){
		if( work->dir < 0 ){
			VMP_SetThinkSub( work, work->think_now_main_w, 0 );

	if( work->think_v_wzn == 19 &&
		!(work->act_flags & VMP_ACTFLAG_SHDW ) &&
		!VMP_CheckShdwBind() &&
		VMP_CheckShdw() &&
		(work->think_jumpout_pos = VMP_GetToShdwAttNum( work->think_p_fzn )) >= 0){
		work->act_flags |= VMP_ACTFLAG_SHDW;
		VMP_SetThinkSub( work, THK_SUB_SHADOW_ATT, 0 );
		return;
	}

			if( work->think_p_fzn == 1 || ( 2 <= work->think_p_fzn && work->think_p_fzn < 4 ) ||
				( 13 <= work->think_p_fzn && work->think_p_fzn < 14 ) ){
				work->think_pre_v_wzn = 18;
			}else if( 4 <= work->think_p_fzn && work->think_p_fzn < 7 ){
				work->think_pre_v_wzn = 22;
			}else if( work->think_p_fzn == 0 || ( 7 <= work->think_p_fzn && work->think_p_fzn < 10 ) ){
				work->think_pre_v_wzn = 20;
			}else if( 10 <= work->think_p_fzn && work->think_p_fzn < 13 ){
				work->think_pre_v_wzn = 21;
			}else{
				VMP_PRINTF( "ERR!!\n" );
			}
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}


//遠距離回避攻撃ジャンプ版
static void VMP_ThinkLocal_AvoidAttFar2( Work *work )
{
	switch( work->think_local ){
	  case 0:
		work->think_n_att = 0;
		work->think_v_loft = -1;
		work->think_temp0 = 0;
		work->think_temp1 = 0;
	  case 1:
		//回避準備
		//if( work->think_v_loft < 0 && VMP_GetZoneInfo( work->think_p_fzn, work->think_v_fzn ) < ZONE_INFO_TOIMEN ){
		//	//近寄ってるよーん
		//	VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
		//	return;
		//}

		ClearAjustToPlayer( work );
		work->think_timer = -1;
		work->think_wait_time = DIRECT_TICK(((irnd()>>8)%3)?6:18);

		if( work->think_v_loft < 0 ){
			//２階にいません（２階か床か）
			if( irnd() & 0x00400000 ){
				//床
				work->think_to_fzn = VMP_GetRndZoneFar( work->think_p_fzn, work->think_v_fzn, ZONE_INFO_TOIMEN, 2 );
				VMP_JumpOutPos( &work->think_init_pos, work->think_to_fzn );
				work->think_local = 2;
				//work->think_mesg = THINK_MESG_FLOOR2FLOOR;//THINK_MESG_FLOORJUMP;
				work->think_mesg = THINK_MESG_FLR2FLR_F;
			}else{
				//２階
				work->think_local = 3;
				work->think_mesg = THINK_MESG_FLOOR2LOFT;//THINK_MESG_TOLOFTJUMP;
				work->think_v_loft = VMP_GetLinePosFromFloorLoft_long( &work->think_init_pos, work->think_p_fzn, -1 );
			}
		}else{
			if( irnd() & 0x00400000 ){
				work->think_local = 4;
				work->think_mesg = THINK_MESG_LOFT2LOFT;
				//work->think_mesg = THINK_MESG_LOFTJUMP;
				//work->think_v_loft = VMP_GetLinePosFromFloorLoft_long( &work->think_init_pos,
				//														work->think_p_fzn, work->think_v_loft );
				work->think_v_loft = VMP_GetLinePosFromFloorLoft( &work->think_init_pos,
																	work->think_p_fzn, work->think_v_loft,
																	&work->control.mov );
				
			}else{
				//床
				work->think_to_fzn = VMP_GetRndZoneFar( work->think_p_fzn, work->think_v_fzn, ZONE_INFO_TOIMEN, 0 );
				VMP_JumpOutPos( &work->think_init_pos, work->think_to_fzn );
				work->think_local = 5;
				work->think_mesg = THINK_MESG_LOFT2FLOOR;//THINK_MESG_TOFLOORJUMP;
			}
		}
		break;
	  case 2:
		//床から床へ
		VMP_ThinkLocal_Flr2FlrShort( work );
		//if( VMP_ThinkLocal_Non_FloorJump( work ) ) return;
		//if( CHECK_FLAG( work->status, VMP_STATUS_ACT_SEMIEND ) ){
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_timer = -1;
			work->think_local = 6;
			work->dir = work->think_dir;
		}
		break;
	  case 3:
		//床から２階へ
		if( VMP_ThinkLocal_Non_JumpToLoft( work ) ) return;

		if( work->think_temp1 && CHECK_FLAG( work->status, VMP_STATUS_GO_LAND ) ){
			SET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
		}
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_SEMIEND ) ){
			work->think_timer = -1;
			work->think_local = 6;
			VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
			work->dir = work->think_dir;
			UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
		}
		break;
	  case 4:
		//２階から２階へ
		//VMP_ThinkLocal_Loft2LoftShort( work );
		if( VMP_ThinkLocal_Non_JumpLoft( work ) ) return;
		if( work->think_temp1 && CHECK_FLAG( work->status, VMP_STATUS_GO_LAND ) ){
			SET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
		}
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_SEMIEND ) ){
		//if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_timer = -1;
			work->think_local = 6;
			VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
			work->dir = work->think_dir;
			UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
		}
		break;
	  case 5:
		//２階から床へ
		if( VMP_ThinkLocal_Non_JumpToFloor( work ) ) return;
		if( work->think_temp1 && CHECK_FLAG( work->status, VMP_STATUS_GO_LAND ) ){
			SET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
		}
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_SEMIEND ) ){
			work->think_timer = -1;
			work->think_local = 6;
			work->dir = work->think_dir;
			UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
			work->think_v_loft = -1;
		}
		break;
	  case 6:
		//待ち
		if( work->think_v_loft >= 0 ) VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
		SetAjustToPlayer( work );
		if( work->think_timer >= work->think_wait_time ){
			if( work->think_n_att <= 3 && work->think_wait_time == DIRECT_TICK(6) ){
				work->think_local = 1;
				work->think_n_att++;
			}else{
				work->think_n_att = 0;
				work->think_local++;
				work->think_mesg = THINK_MESG_THOROWL;
				UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
				work->think_timer = -1;
				//work->think_n_att++;
				work->dir = work->think_dir;
				ClearAjustToPlayer( work );
			}
		}
		if( !VMP_CheckShdwBind() ){
			if( ++work->think_temp0 > 5 )
				work->think_temp1 = 1;
		}else{
			work->think_temp0 = 0;
			work->think_temp1 = 0;
		}

		//if( work->think_v_loft < 0 && VMP_GetZoneInfo( work->think_p_fzn, work->think_v_fzn ) < ZONE_INFO_TOIMEN ){
			//近寄ってるよーん
		//	VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_NEAR, 0 );
		//	ClearAjustToPlayer( work );
		//	return;
		//}
		break;
//	  case 7:
//		//一回チェックを入れるのです！
//		work->dir = work->think_dir;
//		if( work->think_v_loft < 0 && VMP_GetZoneInfo( work->think_p_fzn, work->think_v_fzn ) < ZONE_INFO_TOIMEN ){
//			//近寄ってるよーん
//			VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
//			return;
//		}
//		work->think_local++;
	  case 7:
		if( CHECK_FLAG( work->status, VMP_STATUS_THRWKNF ) ){
			SET_FLAG( work->think_flags, VMP_FLAGS_KNFATT );
			//work->think_n_att++;
		}
		//攻撃
		if( work->think_v_loft >= 0 ) VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
		work->dir = work->think_dir;

		if( work->game_diff < 2 ){
			if( CHECK_FLAG( work->status, VMP_STATUS_ATTACK ) &&
				(work->think_timer == DIRECT_TICK(36) ||
				 work->think_timer == DIRECT_TICK(44) )){

				SET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
			}
		}else{
			if( CHECK_FLAG( work->status, VMP_STATUS_ATTACK ) &&
				(work->think_timer == DIRECT_TICK(36) ||
			 	work->think_timer == DIRECT_TICK(40) ||
			 	work->think_timer == DIRECT_TICK(44) ||
			 	work->think_timer == DIRECT_TICK(48) ) ){
				
				SET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
			}
		}
		
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
			work->think_local = -1;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_RAPIDAVOID ) ){
			UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
			work->think_local = -1;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_local = 1;
			
			UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
			//if( 0 && work->think_v_loft < 0 && work->think_n_att >= 500/*8*/ ){
			//	VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
			//}
			UNSET_FLAG( work->think_flags, VMP_FLAGS_KNFATT );
		}
		if( CHECK_FLAG( work->status, VMP_STATUS_THRWKNF ) ){
			SET_FLAG( work->think_flags, VMP_FLAGS_KNFATT );
			//work->think_n_att++;
		}

		break;
	  case -1:
		//ダメージ
		if( work->think_v_loft >= 0 ) VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			if( !CHECK_FLAG( work->think_flags, VMP_FLAGS_KNFATT )){
				work->think_local = 7;
				work->think_mesg = THINK_MESG_THRWL_MUTEKI;
				work->think_timer = -1;
				UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );

				ClearAjustToPlayer( work );
				
			}else{
				work->think_local = 1;
			}
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}

	//AN_Test_Eye( &work->think_init_pos, 2 );
}

//遠距離回避攻撃ジャンプ版
static void VMP_ThinkLocal_AvoidAttFarFromW( Work *work )
{
	int temp;

	switch( work->think_local ){
	  case 0:
		work->think_local++;
		work->dir = -1;
		work->think_to_wzn = 19;
		work->think_tmp_max_att = work->think_max_att;
		work->think_timer = -1;
		break;
	  case 1:
		//中心へ
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			//work->think_mesg = THINK_MESG_JUMPOUTLOFT;
			work->think_timer = -1;
			work->think_local++;
			DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
			//work->dir = work->think_dir;
			work->think_v_loft = VMP_GetLinePosFromFloorLoft( &work->think_init_pos, work->think_p_fzn, -1, &work->control.mov );
		}
		break;
	  case 2:
		VMP_ThinkLocal_Non_SwimPosUp( work );
		if( work->control.mov.vy > -8000.0f ){
			work->think_timer = -1;
			work->think_local++;
			work->dir_x = 0;
			work->think_mot_step.vy = 0.0f;
			work->control.mov.vy = -8000.0f;
			work->think_mesg = THINK_MESG_WATER2LOFT;
		}
		break;
	  case 3:
		if( VMP_ThinkLocal_Non_JumpOutLoftRnd( work ) ) return;
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_SEMIEND ) ){
			work->think_local = 6;
			work->think_timer = -1;
			//work->think_mesg = THINK_MESG_LOFTMOVE;
			work->think_mesg = THINK_MESG_LOCKON;
			work->think_n_att = 0;
			//work->think_turn = irnd()&0x0040;
			VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
			work->dir = work->think_dir;
		}
		break;
	  case 4:
		//回避準備
		//if( work->think_v_loft < 0 && VMP_GetZoneInfo( work->think_p_fzn, work->think_v_fzn ) < ZONE_INFO_TOIMEN ){
			//近寄ってるよーん
		//	VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
		//	return;
		//}

		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			return;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_RAPIDAVOID )){
			work->think_timer = -1;
			work->think_move_cnt = 0;
			work->think_local= -1;
		}else{
			ClearAjustToPlayer( work );
			work->think_timer = -1;
			work->think_move_cnt = 0;
			work->think_local++;
			work->think_mesg = THINK_MESG_LOFT2LOFT;
		}
		break;
	  case 5:
		//２階から２階へ
		if( VMP_ThinkLocal_Non_JumpLoft( work ) ) return;
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_SEMIEND ) ){
			work->think_timer = -1;
			work->think_local = 9;//6;
			work->dir = work->think_dir;
			//work->think_mesg = THINK_MESG_LOCKON;
			VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
		}
		break;
	  case 6:
		//待ち
		if( work->think_v_loft >= 0 ) VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
		SetAjustToPlayer( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			return;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_RAPIDAVOID )){
			work->think_timer = -1;
			work->think_local = -1;
		}else{//work->think_timer >= work->think_wait_time ){
			if(work->think_now_main_w == THK_SUB_MAIN3_W){
				
				//work->think_mesg = THINK_MESG_SPIN;
				//work->n_spin0 = 0;
				//work->n_spin1 = 6000;
				//work->spin_scale_vec = 80.0f;
					
				work->think_timer = -1;
				work->think_mesg = THINK_MESG_ONESPIN;//THINK_MESG_LOFTMOVE;
				work->think_turn = irnd()&0x0040;
				work->think_local++;
				ClearAjustToPlayer( work );
				
			}else{
				work->dir = work->think_dir;
				//work->think_move_cnt++;
				work->think_local = 10;
				work->think_mesg = THINK_MESG_THOROWL;
				UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
				work->think_timer = -1;			
				ClearAjustToPlayer( work );
			}
		}
		break;
	  case 7:
		//ちょい移動
		temp = VMP_GetLoftDir( &work->dir, &work->think_v_loft, &work->control.mov, work->think_turn );
		
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_SEMIEND ) ){
			//VMP_ThinkThrwKnfLine( work );
			//DG_COPY_VEC( &work->think_init_pos, &GM_PlayerPosition );
			//work->think_timer = -1;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_RAPIDAVOID )){
			work->think_timer = -1;
			work->think_local = -1;
		}else if( temp >= 0 ){
			//FVECTOR	fvtemp;
			//VMP_PRINTF( "jump to %d\n", temp );
			work->think_local++;
			work->think_mesg = THINK_MESG_LOFTJUMP;
			work->think_timer = -1;
			VMP_GetLinePos( &work->think_init_pos, temp );
		}else if(work->think_timer >= DIRECT_TICK(32)){
			work->think_timer = -1;
			work->think_local = 9;
			work->think_mesg = THINK_MESG_LOCKON;
			work->think_move_cnt++;

			//work->dir = work->think_dir;
			//work->think_local = 10;
			//work->think_mesg = THINK_MESG_THOROWL;
		}
#define		OFFSET_TIME	(0)
		if( work->game_diff > 1 && /*CHECK_FLAG( work->status, VMP_STATUS_NOWAVOID ) &&*/
			(work->think_timer == DIRECT_TICK(1)  + OFFSET_TIME ||
			 work->think_timer == DIRECT_TICK(17)  + OFFSET_TIME //||
			 //work->think_timer == DIRECT_TICK(16) + OFFSET_TIME ||
			 //work->think_timer == DIRECT_TICK(20) + OFFSET_TIME
			) ){
			SET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
			printf("att flag set[%d]\n",work->think_timer);
		}
		
		
		break;
	  case 8:
		//ジャンプ
		VMP_ThinkLocal_Loft2LoftShort( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_local--;
			work->think_timer = DIRECT_TICK(90);
		}
		break;
	  case 9:
		//待ち
		//work->dir = work->think_dir;
		if( work->think_v_loft >= 0 ) VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
		SetAjustToPlayer( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			return;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_RAPIDAVOID )){
			work->think_timer = -1;
			ClearAjustToPlayer( work );
			work->think_local =-1;
		}else if( work->think_timer >= 0 ){
			//if( work->life > MODE_CHANGE_LIFE0 || work->faint > MODE_CHANGE_LIFE0 ){
			if( work->think_now_main_w == THK_SUB_MAIN2_W ){
				work->think_local++;
				work->think_mesg = THINK_MESG_THOROWL;
			}else if( work->think_now_main_w == THK_SUB_MAIN3_W ){
				if( work->think_move_cnt < 1 ){	
					work->think_timer = -1;
					work->think_mesg = THINK_MESG_ONESPIN;
					//work->think_mesg = THINK_MESG_LOFTMOVE;
					work->think_turn = irnd()&0x0040;
					work->think_local = 7;
					DG_COPY_VEC( &work->think_init_pos, &GM_PlayerPosition );
				}else{
					work->think_local++;
					work->think_mesg = THINK_MESG_THOROWL;
					UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
					work->think_timer = -1;
				}
			}else{
				if( work->think_move_cnt < 2 ){
					work->think_timer = -1;
					work->think_timer = -1;
					work->think_mesg = THINK_MESG_ONESPIN;
					//work->think_mesg = THINK_MESG_LOFTMOVE;
					work->think_turn = irnd()&0x0040;
					work->think_local = 7;
					DG_COPY_VEC( &work->think_init_pos, &GM_PlayerPosition );
				}else{
					work->think_local++;
					work->think_mesg = THINK_MESG_THOROWL;
					UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
				}
			}
			ClearAjustToPlayer( work );
		}
		break;
	  case 10:
		//攻撃
		if( work->think_v_loft >= 0 ) VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
		if( CHECK_FLAG( work->status, VMP_STATUS_THRWKNF ) ){
			SET_FLAG( work->think_flags, VMP_FLAGS_KNFATT );
			work->think_n_att++;
		}

		if( work->think_now_main_w == THK_SUB_MAIN2_W && work->think_timer == DIRECT_TICK(44) ){
			SET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
		}else if( (/*work->think_timer == DIRECT_TICK(36) ||
				   work->think_timer == DIRECT_TICK(40) ||*/
				   work->think_timer == DIRECT_TICK(44) /*||
				   work->think_timer == DIRECT_TICK(48) */) ){
			SET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
		}
		//if( work->think_v_loft >= 0 ) VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
		work->dir = work->think_dir;
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			return;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_RAPIDAVOID )){
			work->think_timer = -1;
			work->think_local= -1;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			//work->think_timer = -1;
			//work->think_local++;
			//work->think_n_att++;
			if( work->think_n_att >= work->think_max_att ){

				if( work->think_now_main_w == THK_SUB_MAIN2_W ) VMP_SetThinkSub( work, THK_SUB_WATERLOFTWATER_ATT, 0 );
				else VMP_SetThinkSub( work, THK_SUB_DIVE_LOFT, 0 );
				work->think_max_att = work->think_tmp_max_att;
				if( work->think_max_att > 3 ) work->think_max_att = 3;
			}else{

				work->think_local = 4;
				work->think_v_loft = VMP_GetLinePosFromFloorLoft( &work->think_init_pos,
																	work->think_p_fzn, work->think_v_loft,
																	&work->control.mov );
			}

			UNSET_FLAG( work->think_flags, VMP_FLAGS_KNFATT );
		}
		break;
	  case 11:
		//待ち
		if( work->think_v_loft >= 0 ) VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
		SetAjustToPlayer( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			return;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_RAPIDAVOID )){
			work->think_timer = -1;
			ClearAjustToPlayer( work );
			work->think_local= -1;
		}else if( work->think_timer >= work->think_wait_time ){
			ClearAjustToPlayer( work );
			work->think_local = 3;
			if( work->think_n_att >= work->think_max_att ){
				
				if( work->think_now_main_w == THK_SUB_MAIN2_W ) VMP_SetThinkSub( work, THK_SUB_WATERLOFTWATER_ATT, 0 );
				else VMP_SetThinkSub( work, THK_SUB_DIVE_LOFT, 0 );
				work->think_max_att = work->think_tmp_max_att;
				if( work->think_max_att > 3 ) work->think_max_att = 3;
			}
		}
		break;
	  case -1:
		//ダメージ
		if( work->think_v_loft >= 0 ) VMP_GetLoftFixPosEx( work->think_v_loft, &work->control.mov );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			if( work->think_n_att >= work->think_max_att ||
				(work->life <= MODE_CHANGE_LIFE1 || work->faint <= MODE_CHANGE_LIFE1) ){
				//VMP_SetThinkSub( work, THK_SUB_DIVE_LOFT, 0 );
				
				if( work->think_now_main_w == THK_SUB_MAIN2_W ) VMP_SetThinkSub( work, THK_SUB_WATERLOFTWATER_ATT, 0 );
				else VMP_SetThinkSub( work, THK_SUB_DIVE_LOFT, 0 );
				
				//VMP_SetThinkSub( work, THK_SUB_WATERLOFTWATER_ATT, 0 );
				work->think_max_att = work->think_tmp_max_att;
				if( work->think_max_att > 3 ) work->think_max_att = 3;
			}else{
				if( CHECK_FLAG( work->think_flags, VMP_FLAGS_KNFATT ) ){
					work->think_local = 4;
					work->think_v_loft = VMP_GetLinePosFromFloorLoft( &work->think_init_pos, work->think_p_fzn,
					work->think_v_loft, &work->control.mov );
					//work->think_mesg = THINK_MESG_LOFT2LOFT;
					//work->think_local = 3;
					//work->think_tmp_max_att++;
					work->think_timer = -1;
					UNSET_FLAG( work->think_flags, VMP_FLAGS_KNFATT );
				}else{
					work->dir = work->think_dir;
					//work->think_move_cnt++;
					work->think_local = 10;
					work->think_timer = -1;
					UNSET_FLAG( work->act_flags, VMP_ACTFLAG_SPINATT );
					work->think_mesg = THINK_MESG_THRWL_MUTEKI;
				}
			
			}
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}

	//AN_Test_Eye( &work->think_init_pos, 2 );
}
	

//プレイヤの反対に潜って出る
static void VMP_ThinkLocal_DiveOutFar( Work *work )
{
	switch( work->think_local ){		
	  case 0:
		work->think_local++;
		work->think_timer = -1;
		work->think_to_fzn = VMP_GetRndZone( work->think_p_fzn, ZONE_INFO_CONTRAST );
		work->think_jumpout_pos = work->think_to_fzn;
		work->think_dive_pos = VMP_GetDivePosNum( work->think_jumpout_pos );
		work->think_to_wzn = work->think_dive_pos;
		break;
	  case 1:
		{
			int temp;
			temp = VMP_GetRndZone( work->think_p_fzn, ZONE_INFO_CONTRAST );
			if( work->o2 > COUNT_VMODE( VAMP_O2 )/3 && temp != work->think_to_fzn ){
				work->think_to_fzn = temp;
				work->think_jumpout_pos = work->think_to_fzn;
				work->think_dive_pos = VMP_GetDivePosNum( work->think_jumpout_pos );
				work->think_to_wzn = work->think_dive_pos;
			}
		}
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			//work->think_mesg = THINK_MESG_JUMPOUT;
			work->think_timer = -1;
			work->think_local++;
			//DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
			//work->dir = work->think_dir;
		}
		//なかなか出られない時の対処を考える
		break;
	  case 2:
		VMP_ThinkLocal_Non_SwimPosUp_Invr( work );
		//VMP_ThinkLocal_Non_SwimPosUp( work );
		if( work->control.mov.vy > -8000.0f ){
			work->think_timer = -1;
			work->think_local++;
			work->dir_x = 0;
			work->think_mot_step.vy = 0.0f;
			work->control.mov.vy = -8000.0f;
			DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
			work->think_mesg = THINK_MESG_JUMPOUT;
		}
		break;
	  case 3:
		VMP_ThinkLocal_Non_JumpOutFloor( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
//		if( CHECK_FLAG( work->status, VMP_STATUS_FLY ) ){
			VMP_SetThinkSub( work, work->think_now_main_f, 0 );
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}

//近距離攻撃ランダム
static void VMP_ThinkLocal_OutNearRndAtt( Work *work )
{
	switch( work->think_local ){
	  case 0:
		work->think_local++;
		work->think_timer = -1;
		work->think_to_fzn = VMP_GetRndZone( work->think_p_fzn, ZONE_INFO_CONTRAST );
		work->think_jumpout_pos = work->think_to_fzn;
		work->think_dive_pos = VMP_GetDivePosNum( work->think_jumpout_pos );
		work->think_to_wzn = work->think_dive_pos;
		break;
	  case 1:
		{
			int temp;
			temp = VMP_GetRndZone( work->think_p_fzn, ZONE_INFO_CONTRAST );
			if( work->o2 > COUNT_VMODE( VAMP_O2 )/3 && temp != work->think_to_fzn ){
				work->think_to_fzn = temp;
				work->think_jumpout_pos = work->think_to_fzn;
				work->think_dive_pos = VMP_GetDivePosNum( work->think_jumpout_pos );
				work->think_to_wzn = work->think_dive_pos;
			}
		}
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			//work->think_mesg = THINK_MESG_JUMPOUT;
			work->think_timer = -1;
			work->think_local++;
			VMP_JumpOutPos( &work->think_init_pos, work->think_jumpout_pos );
			DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
			//work->dir = work->think_dir;
		}
		//なかなか出られない時の対処を考える
		break;
	  case 2:
		//VMP_ThinkLocal_Non_SwimPosUp( work );
		VMP_ThinkLocal_Non_SwimPosUp_Invr( work );
		if( work->control.mov.vy > -8000.0f ){
			work->think_timer = -1;
			work->think_local++;
			work->dir_x = 0;
			work->think_mot_step.vy = 0.0f;
			work->control.mov.vy = -8000.0f;
			work->think_mesg = THINK_MESG_JUMPOUT;
			DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
		}
		break;
	  case 3:
		VMP_ThinkLocal_Non_JumpOutFloor( work );
		//if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
		if( CHECK_FLAG( work->status, VMP_STATUS_FLY ) ){
			if( work->game_diff < 4 ){
				work->think_local++;
				work->think_timer = -1;
				work->think_mesg = THINK_MESG_SPECIAL;
			}else{
				VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_NEAR, 0 );
			}
		}
		break;
	  case 4:
		//if( !CHECK_FLAG( work->status, VMP_STATUS_NO_DIR ) ){
		//	work->dir = work->think_dir;
		//}
		if( work->think_timer > DIRECT_TICK(140) ) work->dir = work->think_dir;
		if( CHECK_FLAG( work->status, VMP_STATUS_DAMEGE ) ){
			ClearAjustToPlayer( work );
			work->think_local = -1;
			//return;
		}else if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			if( work->act_flags & VMP_ACTFLAG_SHDW ){
				work->act_flags &= ~VMP_ACTFLAG_SHDW;

				work->think_n_att = 2;
				work->think_timer = -1;
				work->think_temp0 = 0;
				work->think_to_fzn = VMP_GetRndZone( work->think_p_fzn, ZONE_INFO_CONTRAST );
				work->think_temp2 = VMP_GetZoneLen( work->think_p_fzn, work->think_to_fzn );
				
				VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_NEAR, 1 );
				VMP_PRINTF("near att after shadow\n");
			}else{
				VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_NEAR, 0 );
			}
			return;
		}
		break;
	  case -1:
		//ダメージ
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			//VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_NEAR, 0 );
			if( work->act_flags & VMP_ACTFLAG_SHDW ){
				work->act_flags &= ~VMP_ACTFLAG_SHDW;

				work->think_n_att = 2;
				work->think_timer = -1;
				work->think_temp0 = 0;
				work->think_to_fzn = VMP_GetRndZone( work->think_p_fzn, ZONE_INFO_CONTRAST );
				work->think_temp2 = VMP_GetZoneLen( work->think_p_fzn, work->think_to_fzn );
				
				VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_NEAR, 1 );
				VMP_PRINTF("near att after shadow\n");
			}else{
				VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_NEAR, 0 );
			}
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}


//プレイヤの近くに潜って出る
static void VMP_ThinkLocal_DiveOutNear( Work *work )
{
	switch( work->think_local ){
	  case 0:
		{
			FVECTOR	fvtemp;
			SVECTOR rot;
			work->think_mesg = THINK_MESG_DIVE;
			work->think_timer = -1;
			work->think_local++;
			work->think_dive_pos = VMP_GetDivePosNum( work->think_v_fzn );
			VMP_DivePos( &fvtemp, work->think_dive_pos );
			ClearAjustToPlayer( work );

			_sceVu0SubVector( &fvtemp, &fvtemp, &work->control.mov );
			_sceVu0Normalize( &fvtemp, &fvtemp );
			TS_VecToRot( &rot, &fvtemp );
			if( rot.vy < 0 ) rot.vy += 4096;
			work->dir = rot.vy;
		}
		break;
	  case 1:
		VMP_ThinkLocal_Non_Dive( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_local++;
			work->dir = -1;
		}
		break;
	  case 2:
		VMP_ThinkLocal_Non_SwimRai( work );
		if( work->dir < 0 ){
			work->think_local++;
			work->think_timer = -1;
			work->think_jumpout_pos = VMP_RaiMayBeNextZone( work->think_p_fzn, work->think_pre_p_fzn,
															CHECK_FLAG(work->think_flags,VMP_FLAGS_STOP) );
			work->think_dive_pos = VMP_GetDivePosNum( work->think_jumpout_pos );
			work->think_to_wzn = work->think_dive_pos;
			
		}
		break;
	  case 3:
		VMP_ThinkLocal_Non_SwimPos( work );
		if( work->dir < 0 ){
			work->think_mesg = THINK_MESG_JUMPOUT;
			work->think_timer = -1;
			work->think_local++;
			DG_COPY_VEC( &work->think_mot_step, &VMP_ZeroVector );
			//work->dir = work->think_dir;
		}
		break;
	  case 4:
		VMP_ThinkLocal_Non_JumpOutFloor( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			//int zone_info = VMP_GetZoneInfo( work->think_jumpout_pos, work->think_p_fzn );
			VMP_SetThinkSub( work, work->think_now_main_f, 0 );
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}

//プレイヤの近くに潜って出るそしてナイフぎり
static void VMP_ThinkLocal_OutAttNear( Work *work )
{
	SET_FLAG( work->status, VMP_STATUS_SPPATT );
	switch( work->think_local ){
	  case 0:
		//if( work->think_raitimer < DIRECT_TICK(300) ){
		//	VMP_SetThinkSub( work, work->think_now_main_w, 0 );
		//	return;
		//}
		work->think_local++;
	  case 1:
		VMP_ThinkLocal_Non_SwimRaiDive( work );
		if( work->dir < 0 ){
			work->think_local++;
			work->think_timer = -1;
			work->think_jumpout_pos = VMP_RaiMayBeNextZone( work->think_p_fzn, work->think_pre_p_fzn,
															CHECK_FLAG(work->think_flags,VMP_FLAGS_STOP) );
			work->think_jumpout_pos = VMP_GetRndZone( work->think_jumpout_pos, ZONE_INFO_NEXT );
			work->think_dive_pos = VMP_GetDivePosNum( work->think_jumpout_pos );
			work->think_to_wzn = work->think_dive_pos;
		}
		if( !CHECK_FLAG( work->think_flags, VMP_FLAGS_CAN_SPPATT ) ){
			VMP_SetThinkSub( work, work->think_now_main_w, 0 );
		}
		break;
	  case 2:
		VMP_ThinkLocal_Non_SwimPosDive( work );
		if( work->dir < 0 ){
			VMP_JumpOutPos( &work->think_init_pos, work->think_jumpout_pos );
			work->think_timer = -1;
			work->think_local++;
		}
		break;
	  case 3:
		//VMP_ThinkLocal_Non_SwimPosUp( work );
		VMP_ThinkLocal_Non_SwimPosUp_Invr( work );
		if( work->control.mov.vy > -8000.0f ){
			work->think_timer = -1;
			work->think_local++;
			work->dir_x = 0;
			work->think_mot_step.vy = 0.0f;
			work->control.mov.vy = -8000.0f;
			work->think_mesg = THINK_MESG_JUMPOUT;
		}
		break;
	  case 4:
		VMP_ThinkLocal_Non_JumpOutFloor( work );
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			work->think_local++;
			work->think_timer = -1;
			work->think_mesg = THINK_MESG_SLASH;
		}
		break;
	  //case 6:
		//break;
	  case 5:
		if( !CHECK_FLAG( work->status, VMP_STATUS_NO_DIR ) ){
			work->dir = work->think_dir;
		}
		if( CHECK_FLAG( work->status, VMP_STATUS_ACT_END ) ){
			VMP_SetThinkSub( work, THK_SUB_AVOID_ATT_NEAR, 0 );
			//VMP_SetThinkSub( work, THK_SUB_DIVE_FLOOR, 0 );
		}
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_local[%d]\n",work->think_local);
	}
}

/* ---------------------------------------------------------------------------

    思考sub

--------------------------------------------------------------------------- */
static void VMP_ThinkSub_Non( Work *work )
{
	switch( work->think_sub ){
	  case THK_SUB_MAIN1_F:
		VMP_ThinkLocal_Main1_F( work );
		break;
	  case THK_SUB_MAIN3_F:
		VMP_ThinkLocal_Main3_F( work );
		break;
	  case THK_SUB_MAIN1_W:
		VMP_ThinkLocal_Main1_W( work );
		break;
	  case THK_SUB_MAIN2_W:
		VMP_ThinkLocal_Main2_W( work );
		break;
	  case THK_SUB_MAIN3_W:
		VMP_ThinkLocal_Main3_W( work );
		break;
	  case THK_SUB_DIVE_FLOOR:
		VMP_ThinkLocal_DiveFloor( work );
		break;
	  case THK_SUB_DIVE_LOFT:
		VMP_ThinkLocal_DiveLoft( work );
		break;
	  case THK_SUB_DIVE_RAIL:
		VMP_ThinkLocal_DiveRail( work );
		break;
	  case THK_SUB_DIVEOUT_FAR:				//プレイヤの反対に潜って出る
		VMP_ThinkLocal_DiveOutFar( work );
		break;
	  case THK_SUB_DIVEOUT_NEAR:			//プレイヤの近くに潜って出る
		VMP_ThinkLocal_DiveOutNear( work );
		break;
	  case THK_SUB_OUTATT_NEAR:				//プレイヤの近くに出て攻撃
		VMP_ThinkLocal_OutAttNear( work );
		break;
	  case THK_SUB_AVOID_ATT_FAR2:			//プレイヤの遠くに出て攻撃
		VMP_ThinkLocal_AvoidAttFar2( work );
		break;
	  case THK_SUB_AVOID_ATT_FARLOFT:
		VMP_ThinkLocal_AvoidAttFarFromW( work );
		break;
	  case THK_SUB_AVOID_ATT_NEAR:			//プレイヤの近くに出て攻撃
		VMP_ThinkLocal_AvoidAttNear( work );
		break;
	  case THK_SUB_WATERLOFTWATER_ATT:
		VMP_ThinkLocal_WaterLoftWaterAtt( work );
		break;
	  case THK_SUB_JMPOUT_ON_WATER_ATT:
		VMP_ThinkLocal_JumpOutAtt( work );
		break;
	  case THK_SUB_SHADOW_ATT:
		VMP_ThinkLocal_ShadowAtt( work );
		break;
	  case THK_SUB_SWIMING:
		VMP_ThinkLocal_Swim( work );
		break;
	  case THK_SUB_SWIMINGEX:
		VMP_ThinkLocal_SwimEx( work );
		break;
	  case THK_SUB_OUTNEARRNDATT:
		VMP_ThinkLocal_OutNearRndAtt( work );
		break;
	  case THK_SUB_WATEROUTWATER:
		VMP_ThinkLocal_SplashWater( work );
		break;
	  case THK_SUB_WATEROUTSPIDE:
		VMP_ThinkLocal_TriAtt( work );
		break;
	  case THK_SUB_GROUND_ATT:
		VMP_ThinkLocal_GroundAtt( work );
		break;
	  case THK_SUB_JUMPOUTLOFT_WALK:
		VMP_ThinkLocal_JumpOutLoft_Walk( work );
		break;
	  case THK_SUB_JUMPLOFTTOFLOOR:
		VMP_ThinkLocal_JumpLoftToFloor( work );
		break;
	  case THK_SUB_STARTONWATER:
		VMP_ThinkLocal_StartOnWater( work );
		break;
	  case THK_SUB_BLOW:
		VMP_ThinkLocalFlr2WtrBlow( work );
		break;
		
	  case THK_SUB_LAST_DAM_FLR:
		VMP_ThinkLocalLastDamFloor( work );
		break;
	  case THK_SUB_LAST_DAM_LFT:
		VMP_ThinkLocalLastDamLoft( work );
		break;
	  case THK_SUB_LAST_BLASTDAM_FLR:
		break;
	  case THK_SUB_LAST_BLASTDAM_LFT:
		VMP_ThinkLocalLastBlastDamLoft( work );
		break;
	  case THK_SUB_DEADEND:
		VMP_ThinkLocal_DeadEnd( work );
		break;
	  case THK_SUB_TEST_THINK:
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_ThinkSub_non[%d]\n",work->think_sub);
	}
}

void VMP_ThinkMain( Work *work )
{
	switch( work->think_main ){
	  case THK_MAIN_NON:
		VMP_ThinkSub_Non( work );
		break;
	  case THK_MAIN_KNF:
		break;
	  default:
		VMP_PRINTF("ERR!! vmp_think_main[%d]\n",work->think_main);
	}
}
/* -------------------------------------------------------------------------- */
//
//     定例処理
//
/* -------------------------------------------------------------------------- */
void VMP_PreThink( Work *work )
{
	FVECTOR		fvtemp;
	int			temp;
	SVECTOR		rot;
work->dir_z = -1;
	work->think_mul_x = work->think_mul_z = work->think_mul_y = 1.0f;
	//プレイヤーの方向
	_sceVu0SubVector( &fvtemp, &GM_PlayerPosition, &work->control.mov );
	work->think_len = _Vu0VecLenXZ( &fvtemp );//_Vu0VecLenXYZ( &fvtemp );
	_sceVu0Normalize( &fvtemp, &fvtemp );
	TS_VecToRot( &rot, &fvtemp );
	if( rot.vy < 0 ) rot.vy += 4096;
	work->think_dir = rot.vy;

	_sceVu0SubVector( &fvtemp, (FVECTOR*)BODYWORLD(GM_PlayerBody,12).m[3], (FVECTOR*)BODYWORLD(&work->body,12).m[3] );
	_sceVu0Normalize( &fvtemp, &fvtemp );
	TS_VecToRot( &rot, &fvtemp );
	if( rot.vx < 0 ) rot.vx += 4096;
	work->adjust_rot_x = rot.vx;
	if(0){
		FVECTOR eye_line_debug[2];
		eye_line_debug[0] = *(FVECTOR*)BODYWORLD(GM_PlayerBody,12).m[3];
		eye_line_debug[1] = *(FVECTOR*)BODYWORLD(&work->body,12).m[3];

		NewLineView( eye_line_debug, 2, 160, 32, 140 );
	}

	temp = VMP_GetZoneNumFromPosFloor( &GM_PlayerPosition, work->think_p_fzn );

	if( temp & 0x8000 ) work->think_flags |= VMP_FLAGS_NO_ZONE;
	else work->think_flags &= ~VMP_FLAGS_NO_ZONE;
	temp &= 0x7fff;
	if( temp != work->think_p_fzn ){
		work->think_pre_p_fzn = work->think_p_fzn;
		work->think_raitimer = 0;
	}else{
		work->think_raitimer++;
	}
	
	work->think_p_fzn = temp;
	work->think_v_fzn = VMP_GetZoneNumFromPosFloor( &work->control.mov, work->think_v_fzn ) & 0x7fff;
	
	temp = VMP_GetZoneNumFromPosWater( &work->control.mov, work->think_v_wzn );
	work->think_p_wzn = VMP_GetZoneNumFromPosWater( &GM_PlayerPosition, work->think_p_wzn );
	if( work->think_v_wzn != temp ){
		work->think_pre_v_wzn = work->think_v_wzn;
	}
	work->think_v_wzn = temp;
	//VMP_PRINTF( "len to player: %f\n", work->think_len );
	work->think_timer++;
	
	//各種フラグ立て
	temp = work->think_flags&(VMP_FLAGS_MISS_INSIGHT|VMP_FLAGS_KNFATT|VMP_FLAGS_NO_ZONE);

	//止ってるか！？
	if( work->think_raitimer > DIRECT_TICK(240) ){
		SET_FLAG( temp, VMP_FLAGS_STOP );
		if( work->think_raitimer > DIRECT_TICK(5000) ) work->think_raitimer = DIRECT_TICK(5000);
	}

	//匍匐ってるか！？
	if( GM_CheckPlayerStatus( PLAYER_GROUND ) ){
		if( ++work->think_ground_timer > DIRECT_TICK(120) ){
			SET_FLAG( temp, VMP_FLAGS_GROUND );
			work->think_ground_timer = DIRECT_TICK(120);
		}
	}else{
		work->think_ground_timer = 0;
	}

	//ぶら下がってるか！？
	if( GM_CheckPlayerStatus( PLAYER_BEYOND ) ){
		if( ++work->think_beyond_timer > DIRECT_TICK(240) ){
			SET_FLAG( temp, VMP_FLAGS_BEYOND );
			work->think_beyond_timer = DIRECT_TICK(240);
		}
	}else{
		work->think_beyond_timer = 0;
	}

	//しゃがんでるか！？
	if( GM_CheckPlayerStatus( PLAYER_SQUAT ) ){
		SET_FLAG( temp, VMP_FLAGS_SQUAT );
	}

	//主観か！？
	if( GM_CheckPlayerStatus( PLAYER_WATCH ) &&
		GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ){
		SET_FLAG( temp, VMP_FLAGS_WATCH );
	}

	//狙われてるか！？
	if( CHECK_FLAG( work->homing.status, HOMING_YOU ) ){
		UNSET_FLAG( work->homing.status, HOMING_YOU );
		SET_FLAG( temp, VMP_FLAGS_LOCKON );
	}

	//場所はどこか
	if( work->control.mov.vy > -1500.0f ){
		SET_FLAG( temp, VMP_FLAGS_LOFT );
	}else if( work->control.mov.vy > -5200.0f + 500.0f ){
		SET_FLAG( temp, VMP_FLAGS_RAIL );
	}

	//ニキータ
	if( CHECK_FLAG( GM_WeaponAlive, WP_ALIVE_NIKITA ) ){
		SET_FLAG( temp, VMP_FLAGS_NIKITA );
		
	}else if( CHECK_FLAG( work->think_flags, VMP_FLAGS_NIKITA ) ){
		UNSET_FLAG( temp, VMP_FLAGS_MISS_INSIGHT );
	}
	
	if( CHECK_FLAG( GM_WeaponAlive, WP_ALIVE_STINGER ) ){
		SET_FLAG( temp, VMP_FLAGS_STG );
	}
	if( work->think_raitimer > DIRECT_TICK(60*20) ){
		SET_FLAG( temp, VMP_FLAGS_CAN_SPPATT );
	}
	//発砲したか！？
	//どうなんだーこの野郎！！

	work->think_flags = temp;
	
	//work->act_flags &= ~VMP_ACTFLAG_SHDW;
#if 0
	if( work->control.n_touches ){
		int i;
		for( i = 0; i < work->control.n_touches; i++ ){
			HZX_ViewHazard( work->control.segs[i] );
		}
	}
	if( work->control.flr_flag & 1 ){
		HZX_ViewHazard( work->control.level[ 0 ] );
	}
	//printf("aa         %f\n",work->control.hzx_base);
	
#endif
	
#ifdef TS_DEBUG_MODE
	if(_VMP_DebugFlags&1)
		VMP_DebugZoneView( work->think_p_fzn, work->think_v_fzn, work->think_dive_pos, work->think_jumpout_pos );
#endif
}
//static int sound_tbl_num = 0;
void VMP_AfterThink( Work *work )
{
//	int		face_dir;
	//FVECTOR	*to;

	//to = (FVECTOR*)BODYWORLD(&work->body,12).m[2];
	//face_dir = (int)(2048.0f * atan2f( to->vx , to->vz ) / PI);
	//work->status = 0;

	//GM_RadarSetSight( &work->radar, face_dir, 512, 5000.0f, 1 );
	GM_RadarSetSight( &work->radar, work->think_dir, 512, 5000.0f, 1 );

	//AN_Test_Eye2( &work->control.mov, 2 );
	work->dmg_flags = 0xffffffff;
	
	if ( work->gage.value != work->life ) {
		work->gage.value = work->life ;
	}
	if( work->gage.m9_value != work->faint ){
		work->gage.m9_value = work->faint;
	}
	if( work->oxygen.value != work->o2 ){
		work->oxygen.value = work->o2;
	}
	if( work->control.mov.vy < -7000.0f ){
		if( --work->o2 < 0 ){
			work->o2 = 0;
			//VMP_PRINTF( "no o2 help me.!!!!\n" );
			//if(--work->life <= 0){
			//	work->life = 0;
			//	//if ( GM_KillCount < 30000 ) ++ GM_KillCount ;
			//	if( work->think_sub != 66 )VMP_SetThinkSub( work, 66, 0 );
			//}
		}
		GM_VisibleGage( &work->oxygen );
	}else{
		work->wtr_blast_count = 0;
		GM_InvisibleGage( &work->oxygen );
		work->o2 = COUNT_VMODE( VAMP_O2 );
	}
	if( !(work->sys_timer%DIRECT_TICK(180)) ){	
		extern void  *NewBreath( FMATRIX *world, FVECTOR *move,/* シフトらしい */ int *sw /* ＯＮ／ＯＦＦらしい */);
#ifdef BP_PS2 //yano
		NewBreath( &BODYWORLD(&work->body,12), &(FVECTOR){ 0.0f, -30.0f, 110.0f, 1.0f }, &work->breath );
#else
		{
		FVECTOR dmdm = { 0.0f, -30.0f, 110.0f, 1.0f };
		NewBreath( &BODYWORLD(&work->body,12), &dmdm, &work->breath );
	    }
#endif
		work->breath = 1;
	}
	if( CHECK_FLAG( work->status, VMP_STATUS_SWIM ) ) work->breath = 0;
	if( --work->dam_timer < 0 ){
		work->dam_count = 0;
	}
	//割り込み
	if( CHECK_FLAG( work->status, VMP_STATUS_CAN_BREAK ) &&
		!CHECK_FLAG( work->status, VMP_STATUS_DEATH ) ){
/*
		if( CHECK_FLAG( work->status, VMP_STATUS_SWIM ) && !VMP_CheckShdwBind() ){
			if( !CHECK_FLAG( work->status, VMP_STATUS_SPPATT ) &&
				CHECK_FLAG( work->think_flags, VMP_FLAGS_CAN_SPPATT )){
				//VMP_SetThinkSub( work, THK_SUB_OUTATT_NEAR, 0 );
				//printf("aaa\n");
				return;
			}
		}else{
					if( !CHECK_FLAG( work->status, VMP_STATUS_MISS ) &&
				CHECK_FLAG( work->think_flags, VMP_FLAGS_NIKITA|VMP_FLAGS_STG ) ){
				//VMP_SetThinkSub( work, THK_SUB_GROUND_ATT, 0 );
				return;
			}
			

*/
//		VMP_PRINTF("st = [%08x]\n",work->status);
			if( !CHECK_FLAG( work->status, VMP_STATUS_BLOW|VMP_STATUS_SWIM ) &&
				CHECK_FLAG( work->status, VMP_STATUS_BLAST ) ){
				VMP_PRINTF( "break blast\n" );
				if( CHECK_FLAG( work->think_flags, VMP_FLAGS_LOFT ) ){
					VMP_SetThinkSub( work, THK_SUB_DIVE_LOFT, 0 );
				//}else if( CHECK_FLAG( work->think_flags, VMP_FLAGS_RAIL ) ){
				}else{
					VMP_SetThinkSub( work, THK_SUB_BLOW, 0 );
				}
				
			}else if( !CHECK_FLAG( work->status, VMP_STATUS_BLOW|VMP_STATUS_STOMP|VMP_STATUS_SWIM) &&
				CHECK_FLAG( work->think_flags, VMP_FLAGS_GROUND|VMP_FLAGS_BEYOND ) ){
				VMP_SetThinkSub( work, THK_SUB_GROUND_ATT, 0 );
				
			}
		
	//if( CHECK_FLAG( work->status, VMP_STATUS_BLAST ) ){
	//	VMP_SetThinkSub( work, THK_SUB_OUTATT_NEAR, 0 );
	//	return;
	//}	
//		}
	}

#if 0
	if(1){
		static u_int debug_vox_num;
		if( !VMP_StartStream( work, debug_vox_num%work->n_vox ) ){
			debug_vox_num++;
		}

	}
#endif
	if( --work->vox_timer < 0 ){
		work->vox_timer = -1;
	}
#if 0
	if( work->think_now_main_w == THK_SUB_MAIN1_W && (work->sys_timer > 60*60*2 || work->life < 110 || work->faint < 110) ){
		work->think_now_main_w = THK_SUB_MAIN2_W;
		work->think_ntbl = 0;
	}else if( work->think_now_main_w == THK_SUB_MAIN2_W && (work->sys_timer > 60*60*5 || work->life < 40 || work->faint < 40) ){
		work->think_now_main_w = THK_SUB_MAIN3_W;
		work->think_ntbl = 0;
	}

	if( work->think_now_main_f != THK_SUB_MAIN3_F && (work->life <= 20 || work->faint <= 20) ){
		work->think_now_main_f = THK_SUB_MAIN3_F;
	}
#endif
	
#ifdef TS_DEBUG_MODE
#if 0
	{
	if( GV_PadData[0].press & PAD_X ){
		int sound_tbl[7] = {
			SD_V_VMPDMG01, //ヴァンプダメージ１「ウッ！」
			SD_V_VMPDMG02, //ヴァンプダメージ２「イッ！」
			SD_V_VMPDMG03, //ヴァンプダメージ３「オォッ！」
			SD_V_VMPDMG04, //ヴァンプダメージ４「カァッ！」
			SD_V_VMPDMH01, //ヴァンプ強ダメージ１「ウアッ」
			SD_V_VMPDMH02, //ヴァンプ強ダメージ２「アァア」
			SD_V_VMPDMH03, //ヴァンプ強ダメージ３「ウァア」
		};
		char *exp_str[7] = {
			"ヴァンプダメージ１「ウッ！」",
			"ヴァンプダメージ２「イッ！」",
			"ヴァンプダメージ３「オォッ！」",
			"ヴァンプダメージ４「カァッ！」",
			"ヴァンプ強ダメージ１「ウアッ」",
			"ヴァンプ強ダメージ２「アァア」",
			"ヴァンプ強ダメージ３「ウァア」",
		};


		//GM_SeSetMode( sound_tbl[sound_tbl_num%7], &work->control.mov, GM_SEMODE_BOMB );
		GM_SeSetEx( sound_tbl[sound_tbl_num%7], &work->control.mov, 50000, 51000 );
		VMP_PRINTF("%s\n",exp_str[sound_tbl_num%7]);
		sound_tbl_num++;
	}
	}
#endif
#if 0
	{
		SVECTOR rot = { 0, work->dir, 0, 0 };
		FMATRIX mat;
		DG_SetPos2( &work->control.mov, &rot );
		DG_GetPos( &mat );
		HZX_ViewMatrix( &mat, 1000.0f );
	}
	{
		FVECTOR line[2];
		DG_COPY_VEC( &line[0], &work->control.mov );
		DG_COPY_VEC( &line[1], &work->think_init_pos );
		NewLineView( line, 1, 160, 32, 140 );
	}

	{
		FVECTOR fvtemp;
		DG_COPY_VEC( &fvtemp, &work->control.mov );
		fvtemp.vy = work->control.hzx_base + work->control.hzx_height;
		AN_Test_Eye2( &fvtemp, 2 );
		
	}
#endif
#endif
}

void VMP_ThinkInit( Work *work )
{
	work->status = 0;
	work->think_main = 0;
	work->think_sub = THK_SUB_STARTONWATER;//THK_SUB_MAIN1_F;
	//work->think_sub = THK_SUB_MAIN1_F;
	work->think_local = 0;
	work->think_timer = -60;
	work->think_mesg = 0;
	work->think_ntbl = 0;
	work->think_max_att = 3;//2;
	work->think_wait_time = BP_FRAMES_PER_SEC();//60;

	work->dam_max_count = 8;

	
	work->life = VAMP_LIFE;//COUNT_VMODE( VAMP_LIFE );
	//work->life = 50;
	work->faint = VAMP_LIFE;//COUNT_VMODE( VAMP_LIFE );
	work->o2 = COUNT_VMODE( VAMP_O2 );
	work->dir = -1;
	work->dir_x = -1;
	work->dmg_flags = 0xffffffff;
	work->act_flags = 0;//VMP_ACTFLAG_SHDW;

	work->think_now_main_f = THK_SUB_MAIN1_F;
	work->think_now_main_w = THK_SUB_MAIN1_W;

	//NewDbugSprite( &work->think_init_pos, 100.0f );
#ifdef TS_DEBUG_MODE
	{
		static void *NewVampDebug( Work *vmp, int *flags );
#ifdef PSX2 //yano
		static GM_DEBUG_MENU debug_menu_zone = {
			class:	"!VMP",
			menu:	"VMP_ZONE",
			max:    2,
			items:  ( char *[] ){ "ON", "OFF" },
			values: ( int [] ){ 0xffff, 0 },
			target: &_VMP_DebugFlags,	// intの変数へのポインタ。代入される。
			mask:   0x00000001,
			type:   GM_DEBUG_MENU_FLAG,
		};
		static GM_DEBUG_MENU debug_menu_txt = {
			class:	"!VMP",
			menu:	"VMP_TXT",
			max:    2,
			items:  ( char *[] ){ "ON", "OFF" },
			values: ( int [] ){ 0xffff, 0 },
			target: &_VMP_DebugFlags,	// intの変数へのポインタ。代入される。
			mask:   0x00000002,
			type:   GM_DEBUG_MENU_FLAG,
		};
#else
static char *debug_menu_zone_items[] = { "ON", "OFF" };
static int debug_menu_zone_values[] = { 0xffff, 0 };
static GM_DEBUG_MENU debug_menu_zone = { 
	NULL,
	"!VMP",
	"VMP_ZONE",
	debug_menu_zone_items,
	debug_menu_zone_values,
	&_VMP_DebugFlags,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	GM_DEBUG_MENU_FLAG,
	2,
	0,
	0
};
static char *debug_menu_txt_items[] = { "ON", "OFF" };
static int debug_menu_txt_values[] = { 0xffff, 0 };
static GM_DEBUG_MENU debug_menu_txt = { 
	NULL,
	"!VMP",
	"VMP_TXT",
	debug_menu_txt_items,
	debug_menu_txt_values,
	&_VMP_DebugFlags,
	0x00000002,	 /* 必須 */
	NULL,
	0,
	GM_DEBUG_MENU_FLAG,
	2,
	0,
	0
};
#endif
		GM_AddDebugMenu( &debug_menu_zone );
		GM_AddDebugMenu( &debug_menu_txt );

		GV_SetActorChild(work,NewVampDebug( work, &_VMP_DebugFlags ));
	}
#endif
}

#ifdef TS_DEBUG_MODE
typedef struct {
	GV_ACT_EX		actor;
	Work			*vmp;
	int				*flags;
} Debug_Work;

static void Act_debug( Debug_Work *work )
{
	if(*work->flags&2){
		DEBUG_Locate( 20, 330, 0 );
		DEBUG_Printf( "TH M %2d S %2d L %2d\n", work->vmp->think_main, work->vmp->think_sub, work->vmp->think_local );
		//DEBUG_Printf( "TH PS %2d\n", work->vmp->think_pre_sub );
		DEBUG_Printf( "TIME %4d V %d TF %d\n", work->vmp->think_timer, work->vmp->think_v_loft, work->vmp->think_turn );
		DEBUG_Printf( "life %3d s %3d n %3d\n", work->vmp->life, work->vmp->think_now_main_w, work->vmp->think_ntbl );
		
		//DEBUG_Printf( "TH DIR %5d\n", work->vmp->dir );//work->vmp->think_dir );
		//DEBUG_Printf( "TH VL %2d\n", work->vmp->think_v_loft );
		//DEBUG_Printf( "TH TBL %2d WT%2d\n", work->vmp->think_ntbl,
		// work->vmp->think_wait_time );

		DEBUG_Locate( 200, 330, 0 );
		//DEBUG_Printf( "TH LEN %f\n", work->vmp->think_len );
		//DEBUG_Printf( "Dam %d > %d [%d]\n", work->vmp->dam_max_count, work->vmp->dam_count, work->vmp->dam_timer );
		DEBUG_Printf( "P %2d PP %2d V %2d D %2d T %2d\n", work->vmp->think_p_fzn, work->vmp->think_pre_p_fzn,
					  work->vmp->think_v_fzn, work->vmp->think_dive_pos, work->vmp->think_to_fzn );
		//DEBUG_Printf( "Pw %2d Vw %2d pVw %2d Tw %2d\n", work->vmp->think_p_wzn, work->vmp->think_v_wzn,
		//			  work->vmp->think_pre_v_wzn, work->vmp->think_to_wzn );
		//DEBUG_Printf( "STATUS %08x\n", work->vmp->status );
		//DEBUG_Printf( "dmg flag %08x\n", work->vmp->dmg_flags );
		//DEBUG_Printf( "vmp flag %08x\n", work->vmp->think_flags );
		DEBUG_Printf( "n_att %d tmp[%d][%d][%d][%d]\n", work->vmp->think_n_att,
					  work->vmp->think_temp0,
					  work->vmp->think_temp1,
					  work->vmp->think_temp2,
					  work->vmp->think_temp3 );

		//DEBUG_Locate( 360, 330, 0 );
		DEBUG_Printf( "%c%c%c%c%c%c%c%c%c%c%c\n",
					  "-L"[(work->vmp->think_flags&VMP_FLAGS_LOFT)?1:0],
					  "-R"[(work->vmp->think_flags&VMP_FLAGS_RAIL)?1:0],
					  "-G"[(work->vmp->think_flags&VMP_FLAGS_GROUND)?1:0],
					  "-B"[(work->vmp->think_flags&VMP_FLAGS_BEYOND)?1:0],
					  "-W"[(work->vmp->think_flags&VMP_FLAGS_WATCH)?1:0],
					  "-N"[(work->vmp->think_flags&VMP_FLAGS_NIKITA)?1:0],
					  "-S"[(work->vmp->think_flags&VMP_FLAGS_STG)?1:0],
					  "-I"[(work->vmp->think_flags&VMP_FLAGS_MISS_INSIGHT)?1:0],
					  "-A"[(work->vmp->think_flags&VMP_FLAGS_CAN_SPPATT)?1:0],
					  "-K"[(work->vmp->think_flags&VMP_FLAGS_KNFATT)?1:0],
					  "-Z"[(work->vmp->think_flags&VMP_FLAGS_NO_ZONE)?1:0] );
		//printf("aaa\n");
	}
}

static void Die_debug( Debug_Work *work ) { }

static void *NewVampDebug( Work *vmp, int *flags )
{
	Debug_Work		*work ;

	work = (Debug_Work*)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Debug_Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act_debug, Die_debug );
		GV_ActorEX( &work->actor );
		work->flags = flags;
		work->vmp = vmp;
	}

	return (void *)work ;
}
#endif

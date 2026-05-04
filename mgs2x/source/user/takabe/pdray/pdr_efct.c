//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pdr_efct.c
	量産型ＲＡＹエフェクト処理ルーチン

	2001/07/11 K.Takabe
	$Id: pdr_efct.c,v 1.1.1.3 2002/11/19 11:51:24 Yoshizawa1 Exp $

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
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"


#include	"../other/vec_util.h"

#include "pdray.h"
#include "r_common.h"

/* ---------------------------------------------------------------- */
extern void	*NewRayFootSplash( FMATRIX *world );
extern void *NewFogWave( FMATRIX *world, int size, int speed, int height );
extern void *NewRayMonoEye( int name, FMATRIX *root, FMATRIX *look, int *col, int *mode );
extern void *NewOozeBlood( DG_OBJS *objs, int model_id );
extern void TAKABE_OozeBloodSetColor( void *_work, float r, float g, float b );
extern void TAKABE_OozeBloodClear( void *_work, float scale );

extern void *NewRAYBreakSmoke( FMATRIX *parent, FVECTOR *offset );
#define SIGNAL_RAYBREAKSMOKE			(0x22cccd00)	/* GV_StrCode("RAYBreakSmoke")<<8 */
#define SIGNAL_RAYBREAKSMOKE_STOP		(SIGNAL_RAYBREAKSMOKE|0x01)

/* ---------------------------------------------------------------- */
typedef struct _break_smoke_pos_data{
	int		joint ;
	float	x, y, z ;
} BreakSmokePosData ;
#if 0
#define MAX_BREAK_SMOKE_POS		(30)
BreakSmokePosData smoke_data_list[MAX_BREAK_SMOKE_POS] = {
	/* body */
	{1,1000,0,0},
	{1,-1000,2000,1000},
	{2,1000,2000,2000},
	{2,-1000,-2000,2000},
	/* right arm */
	{3,0,0,0},
	{4,0,0,-1000},
	{5,0,1000,0},
	{6,0,0,0},
	{7,0,0,1000},
	/* left arm */
	{8,0,0,0},
	{9,0,0,1000},
	{10,0,1000,0},
	{11,0,0,0},
	{12,0,0,-1000},
	/* head */
	{14,-1000,0,-1000},
	{14,1000,-500,1000},
	/* right leg */
	{21,0,0,0},
	{22,0,0,0},
	{22,-2000,0,0},
	{22,-2000,0,5000},
	{23,1000,0,0},
	{24,-1000,0,0},
	{25,0,0,1000},
	/* left leg */
	{27,0,0,0},
	{28,0,0,0},
	{28,2000,0,0},
	{28,2000,0,5000},
	{29,-1000,0,0},
	{30,1000,0,0},
	{31,0,0,1000},
	/* end */
	//{-1,0,0,0}
};
#else
#define MAX_BREAK_SMOKE_POS		(14)
BreakSmokePosData smoke_data_list[MAX_BREAK_SMOKE_POS] = {
	/* body */
	{1,1000,0,0},
	{1,-1000,2000,1000},
	{2,1000,2000,2000},
	{2,-1000,-2000,2000},
	/* head */
	{14,-1000,0,-1000},
	{14,1000,-500,1000},
	/* right leg */
	{21,0,0,0},
	{22,0,0,0},
	{22,-2000,0,0},
	{22,-2000,0,5000},
	/* left leg */
	{27,0,0,0},
	{28,0,0,0},
	{28,2000,0,0},
	{28,2000,0,5000},
	/* end */
	//{-1,0,0,0}
};
#endif

/* ---------------------------------------------------------------- */
void PDRAY_InitEffect( Work *work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;


	work->in_water_leg_flag[0] = 1 ;
	work->in_water_leg_flag[1] = 1 ;

	/* 目エフェクト起動 */
	GTE_UnitMatrix( &work->eye_look_pos );
	work->eye_look_effect_ptr = NewRayMonoEye( work->name, &body->objs->objs[ PDRAY_JOINT_HEAD ].world,
											  &work->eye_look_pos, &work->eye_color, &work->eye_mode );
	GV_SetActorChild( work, work->eye_look_effect_ptr );

	/* 滲み血エフェクト起動 */
	work->oozeblood_effect_ptr = NewOozeBlood( body->objs, 2729025/* "pdray_def_mt" */ );
	//TAKABE_OozeBloodSetColor( work->oozeblood_effect_ptr, 0.5f*1.8f, 0.9f*1.8f, 0.9f*1.8f );
	TAKABE_OozeBloodSetColor( work->oozeblood_effect_ptr, 0.5f*1.4f, 0.9f*1.4f, 0.9f*1.4f );
	GV_SetActorChild( work, work->oozeblood_effect_ptr );

	work->initialized_effect_flag = 1 ;

#if 0
	{/* エフェクト実験 */
		FVECTOR		vec ;
		int			i ;
		BreakSmokePosData	*smk_dat ;
		for ( i = 0 ; ; i++ ){
			smk_dat = &smoke_data_list[ i ] ;
			if ( smk_dat->joint == -1 ) break ;
			GTE_SetVector( &vec, smk_dat->x, smk_dat->y, smk_dat->z, 1 );
			GV_SetActorChild( work, NewRAYBreakSmoke( &body->objs->objs[ smk_dat->joint ].world, &vec ) );
		}
#if 0
		GTE_SetVector( &vec, 0, 1000, 3000, 1 );
		//GV_SetActorChild( work, NewRAYBreakSmoke( &body->objs->objs[ PDRAY_JOINT_HEAD ].world, &vec ) );
		GTE_SetVector( &vec, 0, 0, 0, 1 );
		GV_SetActorChild( work, NewRAYBreakSmoke( &body->objs->objs[ 3 ].world, &vec ) );
		GV_SetActorChild( work, NewRAYBreakSmoke( &body->objs->objs[ 6 ].world, &vec ) );
		GV_SetActorChild( work, NewRAYBreakSmoke( &body->objs->objs[ 22 ].world, &vec ) );
		GTE_SetVector( &vec, 0, 1000, 0, 1 );
		GV_SetActorChild( work, NewRAYBreakSmoke( &body->objs->objs[ 10 ].world, &vec ) );
		GTE_SetVector( &vec, 1000, 0, 0, 1 );
		GV_SetActorChild( work, NewRAYBreakSmoke( &body->objs->objs[ 29 ].world, &vec ) );
#endif
	}
#endif

}

void PDRAY_EndEffect( Work *work )
{
	if ( work->initialized_effect_flag == 0 ) return ;

	/* 煙エフェクトのフェード処理 */
	GV_CallChildSignalFunc( work, SIGNAL_RAYBREAKSMOKE_STOP, 0 );

	/* 各種エフェクト開放処理 */
	GV_CallChildSignalFunc( work, GV_SIGNAL_KILL, 0 );

	work->initialized_effect_flag = 0 ;
}
/* ---------------------------------------------------------------- */
void PDRAY_ActEffect( Work *work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	/* エフェクト破棄チェック */
	if ( work->active_flag == 0 ){
		PDRAY_EndEffect( work );
		return ;
	}

	/* エフェクト初期化チェック */
	if ( work->initialized_effect_flag == 0 ){
		PDRAY_InitEffect( work );
	}

	{/* 足の座標をチェックして水しぶきエフェクトを出す */
		FMATRIX		mat ;
		//FVECTOR		speed_vec ;
		FVECTOR		*r_leg_pos, *l_leg_pos, *leg_pos ;
		int			i ;

		r_leg_pos = (FVECTOR*)body->objs->objs[ PDRAY_JOINT_RIGHT_TOE ].world.m[3] ;
		l_leg_pos = (FVECTOR*)body->objs->objs[ PDRAY_JOINT_LEFT_TOE ].world.m[3] ;

		///* 右足に関するチェック */
		//GTE_SubVector( &speed_vec, r_leg_pos, &work->old_leg_position[0] );

		/* １回目は右足、２回目は左足をチェックするように細工をしてある */
		leg_pos = r_leg_pos ;
		for ( i = 0 ; i < 2 ; i++, leg_pos = l_leg_pos ){
			static int se_count ;
			if ( work->in_water_leg_flag[i] == 0 ){
				/* 現在足が水の上にある場合 */
				if ( leg_pos->vy < WATER_LEVEL ){
					work->in_water_leg_flag[i] = 1 ;
					/* 水しぶきエフェクトを出す */
					GTE_UnitMatrix( &mat );
					mat.m[3][0] = leg_pos->vx ;
					mat.m[3][1] = WATER_LEVEL ;
					mat.m[3][2] = leg_pos->vz ;
					NewRayFootSplash( &mat );
					GM_SeSetMode( (se_count&1) ? SD_E_RWATER01 : SD_E_RWATER02,
								 (FVECTOR*)&mat.m[3], GM_SEMODE_BOMB ) ;
					se_count++ ;
				}
			} else {
				/* 現在足が水の下にある場合 */
				if ( leg_pos->vy > WATER_LEVEL ){
					work->in_water_leg_flag[i] = 0 ;
					/* 水しぶきエフェクトを出す */
					GTE_UnitMatrix( &mat );
					mat.m[3][0] = leg_pos->vx ;
					mat.m[3][1] = WATER_LEVEL ;
					mat.m[3][2] = leg_pos->vz ;
					NewRayFootSplash( &mat );
					GM_SeSetMode( (se_count&1) ? SD_E_RWATER01 : SD_E_RWATER02,
								 (FVECTOR*)&mat.m[3], GM_SEMODE_BOMB ) ;
					se_count++ ;
				}
			}
		}
		/* 現在の足座標を記録しておく */
		work->old_leg_position[0] = *r_leg_pos ;
		work->old_leg_position[1] = *l_leg_pos ;

	}


	{/* 目エフェクト処理 */
		static CVECTOR color_list[3] = { {0,154,255,128},{40,218,120,128},{218,210,40,128} };
		work->eye_color = *(int*)&color_list[ work->info.color_id ] ;
		*(FVECTOR*)work->eye_look_pos.m[3] = GM_PlayerPosition ;
		work->eye_look_pos.m[3][3] = 1.0f ;
		if ( ( GM_GameStatus & STATE_CHAFF ) == 0 ){
			work->eye_mode = 0 ;
		} else {
			work->eye_mode = 2 ;
		}
	}

	/* 滲み血回復処理 */
	if ( work->life > 0 ){
		work->oozeblood_count = work->oozeblood_count % 6 ;
		if ( work->oozeblood_count == ( work->no % 6 ) ){
			TAKABE_OozeBloodClear( work->oozeblood_effect_ptr, 0.998f );
		}
	}

	/* 出血処理 */
	if ( work->blood_flag && !( work->flags & FLAG_JUMP ) ){/* 念のためジャンプ中は出血しないようにする */
		/*
			ここで出血エフェクトを呼ぶ
			追従マトリクス:&body->objs->objs[ work->blood_joint ].world
			補正オフセット:&work->blood_offset
		*/
		extern void *NewRayFallBlood( FMATRIX *mat, FMATRIX *mat_rot );
		//FMATRIX		mat ;
#if 0
		printf("hit!!\n");
		GTE_MulMatrix( &mat, &body->objs->objs[ work->blood_joint ].world, &work->blood_offset );
		AN_Test_Eye2( mat.m[3], 3 );
#endif
		GV_SetActorChild( work,
						 NewRayFallBlood( &body->objs->objs[ work->blood_joint ].world, &work->blood_offset ) );
		work->blood_flag = 0 ;
	}
	if ( work->blood_wait_time ) work->blood_wait_time -= TIME_BASE ;

}
/* ---------------------------------------------------------------- */
/* 指定したレベルの破壊煙エフェクトをセットする */
void PDRAY_SetBreakSmoke( Work *work, int level )
{
	static int	use_smoke_list[ 4 ] = { 2, 3, 4, 6 };
	FVECTOR		vec ;
	int			n_smokes, index ;
	BreakSmokePosData	*smk_dat ;
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	if ( level >= 4 ) level = 3 ;
	n_smokes = use_smoke_list[ level ];
	while ( n_smokes > work->n_break_smokes ){
		index = RND( MAX_BREAK_SMOKE_POS );
		if ( work->break_smoke_pos_flag & ( 1 << index ) ) continue ;

		work->break_smoke_pos_flag |= 1 << index ;
		work->n_break_smokes++ ;
		smk_dat = &smoke_data_list[ index ] ;
		GTE_SetVector( &vec, smk_dat->x, smk_dat->y, smk_dat->z, 1 );
		GV_SetActorChild( work, NewRAYBreakSmoke( &body->objs->objs[ smk_dat->joint ].world, &vec ) );
	}
}
/* ---------------------------------------------------------------- */

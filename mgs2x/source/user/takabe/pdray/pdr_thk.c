//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pdr_thk.c
	量産型ＲＡＹ思考処理ルーチン

	2001/05/01 K.Takabe
	$Id: pdr_thk.c,v 1.1.1.3 2002/11/19 11:51:25 Yoshizawa1 Exp $

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
/* 攻撃範囲チェックデータ */
static int	range_data_list[8][4] = {
	{4000, 12000, -700, 700},	/* AREA_KICK */
	//{3500, 13000, -800, 800},	/* AREA_KICK_OVER */
	{12000, 20000, -450, 450},	/* AREA_CUTTER */
	//{8000, 22000, -500, 500},	/* AREA_CUTTER_OVER */
	{20000, 27000, -450, 450},	/* AREA_VALCAN */
	//{18000, 33000, -500, 500},	/* AREA_VALCAN_OVER */
	{18000, 33000, -500, 200},	/* AREA_VALCAN_OVER_RIGHT */
	{18000, 33000, -200, 500},	/* AREA_VALCAN_OVER_LEFT */
	{27000, 60000, -250, 250},	/* AREA_MISSILE */
	//{27000, 60000, -250, 250},	/* AREA_MISSILE_OVER */
};
static int	range_data_list2[8][4] = {
	{4000, 12000, -700, 700},	/* AREA_KICK */
	{5000, 30000, -550, 550},	/* AREA_CUTTER */
	{20000, 45000, -450, 450},	/* AREA_VALCAN */
	{18000, 48000, -500, 200},	/* AREA_VALCAN_OVER_RIGHT */
	{18000, 48000, -200, 500},	/* AREA_VALCAN_OVER_LEFT */
	{30000, 75000, -500, 500},	/* AREA_MISSILE */
};
/* ---------------------------------------------------------------- */
/* 円弧にて領域のチェックを行う */
static int CheckAreaArc( float len, int angle, float min_len, float max_len, int min_angle, int max_angle )
{
	if ( len < min_len ) return ( 0 );
	if ( len > max_len ) return ( 0 );
	if ( angle < min_angle ) return ( 0 );
	if ( angle > max_angle ) return ( 0 );
	return ( 1 );
}
/* ---------------------------------------------------------------- */
void PDRAY_InitThink( Work *work )
{
	//PDRAY_InitThink1( work );	// 必要なくなる
	PDRAY_InitThink2( work );	// 必要なくなる
	PDRAY_InitThink3( work );	// 必要なくなる

	PDRAY_InitThink1Work( work );
}
/* ---------------------------------------------------------------- */

void PDRAY_ThinkAct( Work *work )
{
	int		i ;

	/* 常に必要と思われる情報をここで求めておく */
	PDRAY_GetPositionInfo2D( NULL, &work->think_player_length, &work->think_player_angle,
							&work->control.mov, &GM_PlayerPosition, work->control.rot.vy );
	/* あまりにも近い場合には角度はないとみなす */
	if ( work->think_player_length < 2000.0f ){
		work->think_player_angle = 0 ;
	}

	/* 思考ルーチン内で設定する各種フラグの初期化 */
	work->enter_attack_area_flag = 0 ;

	/* 攻撃範囲チェック計算 */
	for ( i = 0 ; i < AREA_MAX ; i++ ){
		if ( work->on_stage_flag ){
			work->think_area_check[i] = CheckAreaArc( work->think_player_length, work->think_player_angle,
													 range_data_list[i][0], range_data_list[i][1],
													 range_data_list[i][2], range_data_list[i][3] );
		} else {
			work->think_area_check[i] = CheckAreaArc( work->think_player_length, work->think_player_angle,
													 range_data_list2[i][0], range_data_list2[i][1],
													 range_data_list2[i][2], range_data_list2[i][3] );
		}
	}

	/* 第１思考処理 */
	PDRAY_ActProc( work, &work->think1_work );
	/* 第２思考処理 */
	PDRAY_ActProc( work, &work->think2_work );
	/* 第３思考処理 */
	PDRAY_ActProc( work, &work->think3_work );

	/* 他のルーチンで設定された参照済み各種フラグの初期化 */
	work->damaged_flag = 0 ;


#if 0
	DG_SetFogParam( 300000, 1000000 );
	{/* 領域判定調整用デバッグコード */
//static int	range_data_list[4][4] = {
//	{3500,12000,-800,800},
//	{12000,18000,-450,450},
//	{18000,27000,-300,300},
//	{20000,40000,-120,120},
//};
		static int	data[8][4], flag ;
		int		i, j ;
		FMATRIX		mat = work->world ;
		mat.m[3][1] -= work->control.height ;
		if ( flag == 0 ){
			for ( i = 0 ; i < 8 ; i++ ){
				for ( j = 0 ; j < 4 ; j++ ){
					data[ i ][ j ] = range_data_list[ i ][ j ];
				}
			}
			flag = 1 ;
		}
		if ( ( GV_PadData[ 1 ].press & PAD_A ) && ( work->no == 0 ) ){
			printf("addr:%08x %08x\n", data, &data[4][0] );
			printf("%d %d %d %d\n", data[0][0], data[0][1], data[0][2], data[0][3] );
			printf("%d %d %d %d\n", data[1][0], data[1][1], data[1][2], data[1][3] );
			printf("%d %d %d %d\n", data[2][0], data[2][1], data[2][2], data[2][3] );
			printf("%d %d %d %d\n", data[3][0], data[3][1], data[3][2], data[3][3] );
			printf("\n");
			printf("%d %d %d %d\n", data[4][0], data[4][1], data[4][2], data[4][3] );
			printf("%d %d %d %d\n", data[5][0], data[5][1], data[5][2], data[5][3] );
			printf("%d %d %d %d\n", data[6][0], data[6][1], data[6][2], data[6][3] );
			printf("%d %d %d %d\n", data[7][0], data[7][1], data[7][2], data[7][3] );
		}
		NewDebugArcDisp( &mat, data[0][0], data[0][1], data[0][2], data[0][3], 0x400000ff );
		NewDebugArcDisp( &mat, data[1][0], data[1][1], data[1][2], data[1][3], 0x200000ff );

		NewDebugArcDisp( &mat, data[2][0], data[2][1], data[2][2], data[2][3], 0x4000ff00 );
		NewDebugArcDisp( &mat, data[3][0], data[3][1], data[3][2], data[3][3], 0x2000ff00 );

		NewDebugArcDisp( &mat, data[4][0], data[4][1], data[4][2], data[4][3], 0x40ff0000 );
		NewDebugArcDisp( &mat, data[5][0], data[5][1], data[5][2], data[5][3], 0x20ff0000 );

		NewDebugArcDisp( &mat, data[6][0], data[6][1], data[6][2], data[6][3], 0x4000ffff );
		NewDebugArcDisp( &mat, data[7][0], data[7][1], data[7][2], data[7][3], 0x2000ffff );

	}
#endif

}

/* ---------------------------------------------------------------- */
/* 思考管理ワークの初期化（予め０番の思考がセットされる） */
void PDRAY_InitProcWork( Work *work, PROC_WORK *proc_work, PROC_CALLBACK *proc_init_list )
{
	PROC_CALLBACK	new_proc ;

	proc_work->mode = 0 ;
	proc_work->next_mode = 0 ;
	proc_work->old_mode = 0 ;
	proc_work->phase = 0 ;
	proc_work->count = 0 ;
	//proc_work->act_func = NULL ;
	//proc_work->die_func = NULL ;
	proc_work->new_func_list = proc_init_list ;
	new_proc = proc_work->new_func_list[ proc_work->mode ] ;
	if ( new_proc != NULL ){
		(*new_proc)( work, proc_work );
	}
	proc_work->mark = "" ;
}

/* 新たな思考をセットする */
void PDRAY_SetNextProc( PROC_WORK *proc_work, int next_mode, int flag )
{
	if ( proc_work == NULL ) return ;
	/* 思考切り替えにプロテクトが掛かっている場合は設定しない */
	if ( proc_work->flag & PROC_FLAG_PROTECT ) return ;
	/* 同一思考の場合には設定しない */
	if ( proc_work->mode == next_mode ) return ;
	/* 次の思考を登録 */
	proc_work->next_mode = next_mode ;
}

/* 思考処理実行 */
int PDRAY_ActProc( Work *work, PROC_WORK *proc_work )
{
	int		res = -1 ;

	/* 思考切り替えチェック */
	if ( proc_work->mode != proc_work->next_mode ){
		/* 前の思考の終了処理が完了するまで切り替えない */
		if ( ( proc_work->die_func == NULL ) || ( ( (*proc_work->die_func)( work, proc_work ) ) != -1 ) ){
			/* 切り替え処理 */
			PROC_CALLBACK	new_proc ;
			proc_work->old_mode = proc_work->mode ;	/* 前思考の保存 */
			proc_work->mode = proc_work->next_mode ;	/* 思考モード設定 */
			proc_work->phase = -1 ;					/* 思考モードの初期化 */
			proc_work->count = 0 ;						/* 思考用カウンタの初期化 */
			//proc_work->act_func = proc_work->act_table[ proc_work->mode ] ;
			//proc_work->die_func = proc_work->act_table[ proc_work->mode ] ;
			/* 思考初期化関数の呼び出し */
			new_proc = proc_work->new_func_list[ proc_work->mode ] ;
			if ( new_proc != NULL ){
				proc_work->phase = 0 ;
				(*new_proc)( work, proc_work ) ;
			} else {
				proc_work->act_func = NULL ;
				proc_work->die_func = NULL ;
			}
		}
	}

	/* 思考処理の実行 */
	if ( proc_work->act_func != NULL ){
		res = (*proc_work->act_func)( work, proc_work );
	}

	return ( res );
}

void PDRAY_DumpProc( PROC_WORK *proc_work, char *header )
{
	printf("%-8s:mode      = %d\n", header, proc_work->mode );
	printf("%-8s:next_mode = %d\n", header, proc_work->next_mode );
	printf("%-8s:old_mode  = %d\n", header, proc_work->old_mode );
	printf("%-8s:flag      = %d\n", header, proc_work->flag );
	printf("%-8s:phase     = %d\n", header, proc_work->phase );
	printf("%-8s:count     = %d\n", header, proc_work->count );
	printf("%-8s:type      = %d\n", header, proc_work->type );
	printf("%-8s:mark      = %s\n", header, proc_work->mark );
}

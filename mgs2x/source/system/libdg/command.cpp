/*
	commnad.c
	ＧＣＬ用各種コマンド類

	2000/06/08 K.Takabe
	$Id: command.cpp,v 1.1.1.3 2002/11/19 11:42:04 Yoshizawa1 Exp $

*/
/*

command カメラ視界チェック[NewViewCheckCommand] \
	-camera $b:カメラフラグ \
	-pos $v:チェック座標 \
	-bound $v:バウンディング座標０ $v:バウンディング座標１ 
// チェック座標もしくはバウンディングがカメラの視界内に入っているかをチェックする
// 視界内に全て、もしくは一部が入っている場合に１が返り、そうでなければ０が返る
// カメラフラグは以下のビットフラグの論理和で指定する
// 0x01 - メインカメラ
// 0x02 - サブカメラ
// 0x04 - ＜予約＞
// 0x08 - ＜予約＞
// 座標でのチェックとバウンディングでのチェックはどちか一方のみ有効なので注意！

command メインスクリーンサイズ変更[NewScreenSizeChange] \
	-scale $s:スケール
// メイン画面の描画領域サイズを強制的に変更する（使用には十分注意すること）
// スケールは1~256で指定し、256で通常の大きさになる

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef KP_XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include "libgv.h"
#include "libgcl.h"
#include "libdg.h"
#include "dgconf.h"


/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
int NewViewCheckCommand( void )
{
	int		cam_flag, ret ;
	/* チェックカメラの取得 */
    if ( GCL_GetOption( 'c' ) != NULL ) {
		cam_flag = GCL_GetNextInt() ;
	}
	/* 座標取得 */
    if ( GCL_GetOption( 'p' ) != NULL ) {
		/* １点でのチェック */
		FVECTOR		pos ;
		pos.vx = (float)GCL_GetNextInt() ;
		pos.vy = (float)GCL_GetNextInt() ;
		pos.vz = (float)GCL_GetNextInt() ;
		pos.vw = 1.0f ;
		ret = DG_PointCheckOne( &pos, cam_flag );
		//printf("point check %d\n", ret);
		return ( ( ret & cam_flag ) != cam_flag );
	} else if ( GCL_GetOption( 'b' ) != NULL ) {
		/* バウンディングでのチェック */
		FVECTOR		max, min ;
		max.vx = (float)GCL_GetNextInt() ;
		max.vy = (float)GCL_GetNextInt() ;
		max.vz = (float)GCL_GetNextInt() ;
		max.vw = 1.0f ;
		min.vx = (float)GCL_GetNextInt() ;
		min.vy = (float)GCL_GetNextInt() ;
		min.vz = (float)GCL_GetNextInt() ;
		min.vw = 1.0f ;
		ret = DG_BoundCheckFlag( &DG_UnitMatrix, &max, &min, cam_flag );
		//printf("bound check %d\n", ret);
		return ( ( ret & cam_flag ) != cam_flag );
	}

	return ( 0 );
}

/* ---------------------------------------------------------------- */
int NewScreenSizeChange( void )
{
	int		scale = 256, w, h ;

	/* スケールの取得 */
    if ( GCL_GetOption( 's' ) != NULL ) {
		scale = GCL_GetNextInt() ;
	}

	w = (int)DRAW_WIDTH * scale / 256 ;
	h = (int)DRAW_HEIGHT * scale / 256 ;
	printf("main screen size %d %d\n", w, h);
	DG_SetDrawEnv( DG_Chanl(0),
				  DG_Chanl(0)->offset_x, DG_Chanl(0)->offset_y, w, h );

	return ( 0 );
}
/* ---------------------------------------------------------------- */


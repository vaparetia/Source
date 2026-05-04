//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	commnad.c
	ＧＣＬ用各種コマンド類

	2000/06/08 K.Takabe
	$Id: command.c,v 1.1.1.3 2002/11/19 11:42:04 Yoshizawa1 Exp $

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

command クリッププレーン変更[NewChangeClipPlaneCommand] \
	-near $i:ニア距離 \
	-far $i:ファー距離
// ニア距離の変更は不具合が多発する可能性があるので基本的には使用禁止とする
// オプションを指定しなければデフォルト値が適用される
// near default : 50
// far default : 65536 * 20

command ステージライトＯＮＯＦＦ[NewSwitchLightCommand] \
	-change $b:ＯＮＯＦＦ指定 \
	-sphere $v:中心座標 $w:影響半径 \
	-bound $v:最大座標 $v:最小座標 
// -change は０で消灯１で点灯させることが出来ます。省略時は０と見なします
// -sphere と -bound オプションは排他です。どちらかのみを使用してください
// 実際にプリシェードの再計算を行うので頻繁には使用しないでください
// 一部の光源タイプへの対応が不完全なので注意してください。


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

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libgcl.h"

#include	"gameheader.h"


/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
int NewViewCheckCommand( void )
{
	int		cam_flag = 0, ret ;
	/* チェックカメラの取得 */
    if ( GCL_GetOption( 'c' ) ) {
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
int	NewChangeClipPlaneCommand( void )
{
	float	far_length, near_length ;

	near_length = DRAW_NEAR_CLIP ;
	far_length = DRAW_FAR_CLIP ;
	if ( GCL_GetOption( 'f' ) != NULL ) {
		far_length = GCL_GetNextInt() ;
	}
	if ( GCL_GetOption( 'n' ) != NULL ) {
		near_length = GCL_GetNextInt() ;
	}

	DG_SetClipParam( near_length, far_length );

	return ( 0 );
}
/* ---------------------------------------------------------------- */
int NewSwitchLightCommand( void )
{
	int		change = 0 ;
	if ( GCL_GetOption( 'c' ) ){
		change = GCL_GetNextInt();
	}

	if ( GCL_GetOption( 's' ) ){
		FVECTOR		pos ;
		float		range ;
		pos.vx = GCL_GetNextInt();
		pos.vy = GCL_GetNextInt();
		pos.vz = GCL_GetNextInt();
		range = GCL_GetNextInt();
		DG_SwitchLightSphere( &pos, range, change );
	} else if ( GCL_GetOption( 'b' ) ){
		FVECTOR		max, min ;
		max.vx = GCL_GetNextInt();
		max.vy = GCL_GetNextInt();
		max.vz = GCL_GetNextInt();
		min.vx = GCL_GetNextInt();
		min.vy = GCL_GetNextInt();
		min.vz = GCL_GetNextInt();
		DG_SwitchLightBound( &max, &min, change );
	} else {
		return (-1);
	}

	return ( 0 );
}
/* ---------------------------------------------------------------- */
int		GCL_DG_SetTmpLight2( )
{
	FVECTOR point ;
	float r_range, e_range ;
	int color, flag ;

	/* 場所 */
	if ( GCL_GetOption( 'p' ) == NULL ) return -1 ;
	point.vx = ( float )GCL_GetNextInt() ;
	point.vy = ( float )GCL_GetNextInt() ;
	point.vz = ( float )GCL_GetNextInt() ;
	point.vw = 1.0F ;

	if ( GCL_GetOption( 'r' ) == NULL ) return -1 ;
	r_range = ( float )GCL_GetNextInt() ;

	if ( GCL_GetOption( 'e' ) == NULL ) return -1 ;
	e_range = ( float )GCL_GetNextInt() ;

	if ( GCL_GetOption( 'c' ) == NULL ) return -1 ;
	color = GCL_GetNextInt() ;//r
	color |= (GCL_GetNextInt()) << 8 ;//g
	color |= (GCL_GetNextInt()) <<16 ;//b

	if ( GCL_GetOption( 'f' ) == NULL ) return -1 ;
	flag = GCL_GetNextInt() ;

#ifdef KP_XBOX
	/*2002.09.02 sigeno ダークステージsp08aでステージ変形中にテンプライト当てない*/
	if(GM_VRStatus & (GM_VR_GAME_OVER|GM_VR_IDLE)){
		flag &= ~LIT_FLAG_BGONLY ;
	}
#endif

	DG_SetTmpLight2( &point, r_range, e_range, color, flag ) ;

	return 1 ;
}
/* ---------------------------------------------------------------- */

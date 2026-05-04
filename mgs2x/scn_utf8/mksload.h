/*
	mksload.h
	   メイキングＶＲ体験版 ステージロード用ヘッダファイル

	2002/06/02 Y.Korekado
	$Id: mksload.h,v 1.14 2002/07/03 08:02:03 usr01749 Exp $
*/

// sload.hより先に呼ぶことでsload.hを無視するため
// d:SLOAD_H を使用する

#ifndef d:SLOAD_H
#define SLOAD_H	1

#include "sound.h"
//#include "varinit.h"

#ifndef d:DEBUG_PRINT
#ifdef d:FIX_MODE		// ここはmakefileで切り分ける
	#define		DEBUG_PRINT		0	// こっちは0で固定
#else
	#define		DEBUG_PRINT		1
#endif
#endif

#include "a_vardef.h"

//=============================================
//ステージロード関数

// 常駐選択用
//---------------------------------------------
#define	RES_SELECT		0
#define	RES_SNAKE		1
#define	RES_RAIDEN		2
#define	RES_NAKEDRAIDEN	3
#define	SNAKE_STAGE		4
#define	SNAKE_STORY		5
#define	RAIDEN_STAGE	6
#define	RAIDEN_STORY	7
#define	DEMO_TEST		8		/* 期間限定:最終的には外します */
#define	BOMB_TEST1		9		/* 期間限定:最終的には外します */
#define	BOMB_TEST2		10		/* 期間限定:最終的には外します */
#define	BOMB_TEST3		11		/* 期間限定:最終的には外します */
#define	BOSS_CHECK		12		/* 期間限定:最終的には外します */
#define	MOVIE_TEST		13		/* 期間限定:最終的には外します */
#define BOSS_SURVIVAL	14
#define	ANOTHER_MISSION	15
#define	TANKER_STAGE_SNAKE_ANOTHER	16	/*	爆弾解体用とします */
#define	TANKER_STAGE_RAIDEN_ANOTHER	17	/*	爆弾解体用とします */
#define	PLANT_STAGE_SNAKE_ANOTHER	18	/*	爆弾解体用とします */
#define	PLANT_STAGE_RAIDEN_ANOTHER	19	/*	爆弾解体用とします */
// 以下ミッション選択用（上から続いてます）
enum MISSIONS {
	PLAYER_SELECT	=	20,
	SELECT,
	VR_SELECT,
	SNEAKING_SELECT,
	SNEAKING_STAGE_SELECT,
	WEAPON_SELECT,
	WEAPON_HANDGUN_SELECT,
	WEAPON_MACHINEGUN_SELECT,
	WEAPON_TRAPS_SELECT,
	WEAPON_PSG1_SELECT,
	WEAPON_GRANADE_SELECT,
	WEAPON_STINGER_SELECT,
	WEAPON_NIKITA_SELECT,
	WEAPON_BLADE_SELECT,
	VARIETY_SELECT,
	ALT_SELECT,
	TALE_STAGE_SELECT,
	BOMB_SELECT,
	ELIM_SELECT,
	HOLD_SELECT,
	ESCAPE_SELECT,
	PHOTO_SELECT,
	ALT_STAGE_SELECT,
	SNAKE_TALES_SELECT
}

proc config_fullitem_p {
	@プレイヤーフル装備

	chara delay リスタートディレイ \
		-time 30 \
		-exec {
			restart -save
		}
}

proc config_fullitem_p_normal {
	@プレイヤーフル装備（ノーマル）

	chara delay リスタートディレイ \
		-time 30 \
		-exec {
			restart -save
		}
}

//---------------
// キャラ選択
proc mk_load_raiden {
//	@プレイヤーコンフィグ用初期設定 d:LEVEL_NORMAL プラント編 d:RAIDEN_STORY
	eval( $s:選択プレイヤー = ライデン );
	@アナザーミッション用プレイヤー武器装備初期化
}
proc mk_load_snake {
//	@プレイヤーコンフィグ用初期設定 d:LEVEL_NORMAL プラント編 d:RAIDEN_STORY
	eval( $s:選択プレイヤー = スネーク );
	@アナザーミッション用プレイヤー武器装備初期化
}

//---------------
//---------------
//	時間帯コンフィグプロック
proc set_morning {
	eval( $b:ＶＲ起動時間帯 = d:VR_TIME:デバッグ );
	eval( $b:現在時刻 = 6 );

	chara delay リスタートディレイ \
		-time 30 \
		-exec {
			restart -save
		}

}

proc set_evening {
	eval( $b:ＶＲ起動時間帯 = d:VR_TIME:デバッグ );
	eval( $b:現在時刻 = 17 );

	chara delay リスタートディレイ \
		-time 30 \
		-exec {
			restart -save
		}

}

proc set_night {
	eval( $b:ＶＲ起動時間帯 = d:VR_TIME:デバッグ );
	eval( $b:現在時刻 = 22 );

	chara delay リスタートディレイ \
		-time 30 \
		-exec {
			restart -save
		}

}

// タイトル
//---------------------------------------------
proc mv_init_trtitle_0 {
	load "trtitle" -resident 'r_title' -change scenerio;
}

// メニュー
//---------------------------------------------
proc mv_trmenu_0 {
	load 'trmenu0' -resident 'r_title' ;
}
proc mv_trmenu_1 {
	load 'trmenu1' -resident 'r_title' ;
}

// セレクト
proc load_select {
	load 'trmenu1' -resident 'r_title' ;
}

/**************************************************************************************/
// サウンドデータ読み替えプロック
/**************************************************************************************/
#define	VR_SOUND		0
#define	TANKER_SOUND	1
#define	PLANT_SOUND		2

proc サウンドデータロード $:タイプ {
	#if d:DEBUG_PRINT
		print 'sound data load $:タイプ='$:タイプ
	#endif

	if ( $s:選択プレイヤー == スネーク || $s:選択プレイヤー == プリスキン || \
		 $s:選択プレイヤー == タキシードスネーク || $s:選択プレイヤー == 前作スネーク ) {
		if ( $:タイプ == d:VR_SOUND ) {
			command ロードサウンドパック -p 0x3
		} else if ( $:タイプ == d:TANKER_SOUND ) {
			command ロードサウンドパック -p 0x4
		} else if ( $:タイプ == d:PLANT_SOUND ) {
			command ロードサウンドパック -p 0x5
		}

	} else {

		if ( $:タイプ == d:VR_SOUND ) {
			command ロードサウンドパック -p 0x0
		} else if ( $:タイプ == d:TANKER_SOUND ) {
			command ロードサウンドパック -p 0x1
		} else if ( $:タイプ == d:PLANT_SOUND ) {
			command ロードサウンドパック -p 0x2
		}
	}

}



/**************************************************************************************/
// ＶＲステージ
/**************************************************************************************/
/**************************************************************************************/
// ＶＲ体験版１面 tr01a.gcl を呼び出すプロック
/**************************************************************************************/
// スニーキング３面
proc load_vs03a {
	@mk_load_snake
	eval( $b:ミッション番号 = d:MISSION:スニーキング );
	$b:モード番号 = d:SNEAKING:SNEAKING;
	$b:ＶＲ体験版ステージ番号 = 1 ;

	@サウンドデータロード d:VR_SOUND
	eval( $i:プレイヤー初期Ｘ位置 = 0 ) ;
	eval( $i:プレイヤー初期Ｙ位置 = 0 ) ;
	eval( $i:プレイヤー初期Ｚ位置 = 0 ) ;
	eval( $i:プレイヤー初期方向 = 1024 ) ;
	eval( $f:ロードチェックＯＮフラグ = 0 );
	$f:A_一括初期化拒否フラグ = 0;
	load 'tr01a' -resident 'r_vr_s';
}

//Next Stage
proc load_vs04a {
	@load_wp01a
}

/**************************************************************************************/
// ＶＲ体験版２面 tr03a.gcl を呼び出すプロック
/**************************************************************************************/
// 武器訓練 ハンドガン １面
proc load_wp01a {
	@mk_load_snake
	eval( $b:ミッション番号 = d:MISSION:武器訓練 );
	$b:モード番号 = d:WEAPON:HANDGUN;
	$b:ＶＲ体験版ステージ番号 = 2 ;

	@サウンドデータロード d:VR_SOUND
	eval( $i:プレイヤー初期Ｘ位置 = 0 ) ;
	eval( $i:プレイヤー初期Ｙ位置 = 0 ) ;
	eval( $i:プレイヤー初期Ｚ位置 = 4000 ) ;
	eval( $i:プレイヤー初期方向 = 2048 ) ;
	$f:A_一括初期化拒否フラグ = 0;
	load 'tr03a' -resident 'r_vr_s' ;
}

//Next Stage
proc load_wp02a {
	@load_vs08a
}

/**************************************************************************************/
// ＶＲ体験版３面 tr02a.gcl を呼び出すプロック
/**************************************************************************************/
// スニーキング８面
proc load_vs08a {
	@mk_load_raiden
	eval( $b:ミッション番号 = d:MISSION:スニーキング );
	$b:モード番号 = d:SNEAKING:ELIMINATE_ALL;
	$b:ＶＲ体験版ステージ番号 = 3 ;

	@サウンドデータロード d:VR_SOUND
	eval( $i:プレイヤー初期Ｘ位置 = 0 ) ;
	eval( $i:プレイヤー初期Ｙ位置 = 0 ) ;
	eval( $i:プレイヤー初期Ｚ位置 = 0 ) ;
	eval( $i:プレイヤー初期方向 = 2048 ) ;
	eval( $f:ロードチェックＯＮフラグ = 0 );
	$f:A_一括初期化拒否フラグ = 0;
	load 'tr02a' -resident 'r_vr_r' ;
}

//Next Stage
proc load_vs09a {
	@load_wp73a
}

/**************************************************************************************/
// ＶＲ体験版４面 tr04a.gcl を呼び出すプロック
/**************************************************************************************/
// 武器訓練 ブレード３面
proc load_wp73a {
	@mk_load_raiden
	eval( $b:ミッション番号 = d:MISSION:武器訓練 );
	$b:モード番号 = d:WEAPON:BLADE;
	$b:ＶＲ体験版ステージ番号 = 4 ;

	@サウンドデータロード d:VR_SOUND
	eval( $i:プレイヤー初期Ｘ位置 = 0 ) ;
	eval( $i:プレイヤー初期Ｙ位置 = 1000) ;
	eval( $i:プレイヤー初期Ｚ位置 = -1000) ;
	eval( $i:プレイヤー初期方向 = 2048 ) ;
	$f:A_一括初期化拒否フラグ = 0;
	load 'tr04a'  -resident 'r_vr_r';
}

//Next Stage
proc load_wp74a {
	@load_sp06a
}

/**************************************************************************************/
// ＶＲ体験版５面 tr05a.gcl を呼び出すプロック
/**************************************************************************************/
proc load_sp06a {
	@mk_load_snake
	eval( $b:ミッション番号 = d:MISSION:バラエティ );
	$b:ＶＲ体験版ステージ番号 = 5 ;

	@サウンドデータロード d:VR_SOUND
	eval( $i:プレイヤー初期Ｘ位置 = 0 ) ;
	eval( $i:プレイヤー初期Ｙ位置 = 6000 ) ;
	eval( $i:プレイヤー初期Ｚ位置 = 20000 ) ;
	eval( $i:プレイヤー初期方向 = 2048 ) ;
	eval( $f:ロードチェックＯＮフラグ = 0 );
	$f:A_一括初期化拒否フラグ = 0;
	load 'tr05a' -resident 'r_vr_s' ;
}

//Next Stage
proc load_sp07a {
	@load_select
}

#endif


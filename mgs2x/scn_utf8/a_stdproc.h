/*
	a_stdproc.h
	    各ファイル共通で使いそうなプロック(アナザーミッション用)

	2002/01/24 H.Yoshiike
	$Id: a_stdproc.h,v 1.245 2002/10/21 06:44:31 usr03682 Exp $


*/


// ファイルが二重呼びされたときの対処
#ifndef d:A_STDPROC_H
#define A_STDPROC_H	1

#ifndef d:ANOTHER
#define	ANOTHER		1	// アナザーミッションステージ切り分け用
#endif

#include "a_vardef.h"
#include "a_varinit.h"
#include "msn_stage_id.h"		// ステージＩＤのenumファイル
#include "a_stage_id_time.h"	// ステージＩＤやタイムを設定したファイル
#include "a_allmap_res.h"		// オルタナティブ系全体マップ用リソースファイル
#include "vr_window_j.h"		// ＶＲウィンドウのリソースファイル(日本語版)
#include "vr_window_e.h"		// ＶＲウィンドウのリソースファイル(英語版)
#include "vr_window_g.h"		// ＶＲウィンドウのリソースファイル(ドイツ語版)
#include "vr_window_f.h"		// ＶＲウィンドウのリソースファイル(フランス語版)
#include "vr_window_i.h"		// ＶＲウィンドウのリソースファイル(イタリア語版)
#include "vr_window_s.h"		// ＶＲウィンドウのリソースファイル(スペイン語版)
#include "vr_window_k.h"		// ＶＲウィンドウのリソースファイル(韓国語版)

#include "tales_window.h"		// スネークテイルズ用のＶＲウィンドウ

/*******************************************************************************************/
// ファイルが呼ばれた際、直に呼び出される処理
/*******************************************************************************************/
// chara ＶＲクリアのフラグ
$$i:ＶＲクリアフラグ = 0;

/////////////////////////////////
// プレイヤーの武器、装備アイテム関係を初期化する
/////////////////////////////////
// ザコサバイバル等、リスタートがかかる特殊なもののためにフラグを用意
if ( $f:A_一括初期化拒否フラグ == 0 ) {
	#ifdef	d:FIX_MODE
		@A_製品版初期化設定
	#else
		if ( $f:デバッグ用フル装備フラグ == 0 ) {
			@A_製品版初期化設定
		}
	#endif
}

#ifdef d:STAGE_VR
// ＶＲでは起動時間をチェックする
@時間帯チェック
#endif






// ここまで



/////////////////////////////////
// ＶＲステージ限定時間帯調査プロック
/////////////////////////////////
proc 時間帯チェック {
	#if d:DEBUG_PRINT
		print 'time_check_start'
	#endif

	#ifdef	d:FIX_MODE
		eval($b:現在時刻 =`%現在時刻取得`/60)		// 返り値は、[時]*60+[分]
	#else
		if ( $b:ＶＲ起動時間帯 != d:VR_TIME:デバッグ ) {
			eval($b:現在時刻 =`%現在時刻取得`/60)		// 返り値は、[時]*60+[分]
		}
	#endif

	if ( $b:現在時刻 < 4 ) {
		#if d:DEBUG_PRINT
			print '今は夜　$b:現在時刻='$b:現在時刻
		#endif
		eval( $b:ＶＲ起動時間帯 = d:VR_TIME:夜 );
		eval( $s:ＶＲライトファイル名 = vr_night );

	} else if ( $b:現在時刻 >= 4 && $b:現在時刻 < 6 ) {
		#if d:DEBUG_PRINT
			print '今は朝焼け　$b:現在時刻='$b:現在時刻
		#endif
		eval( $b:ＶＲ起動時間帯 = d:VR_TIME:夕方 );
		eval( $s:ＶＲライトファイル名 = vr_evening );

	} else if ( $b:現在時刻 >= 6 && $b:現在時刻 < 15 ) {
		#if d:DEBUG_PRINT
			print '今は朝か昼　$b:現在時刻='$b:現在時刻
		#endif
		eval( $b:ＶＲ起動時間帯 = d:VR_TIME:朝 );
		eval( $s:ＶＲライトファイル名 = vr_morning );

	} else if ( $b:現在時刻 >= 15 && $b:現在時刻 < 18 ) {
		#if d:DEBUG_PRINT
			print '今は夕方　$b:現在時刻='$b:現在時刻
		#endif
		eval( $b:ＶＲ起動時間帯 = d:VR_TIME:夕方 );
		eval( $s:ＶＲライトファイル名 = vr_evening );

	} else if ( $b:現在時刻 >= 18 ) {
		#if d:DEBUG_PRINT
			print '今は夜　$b:現在時刻='$b:現在時刻
		#endif
		eval( $b:ＶＲ起動時間帯 = d:VR_TIME:夜 );
		eval( $s:ＶＲライトファイル名 = vr_night );

	} else {
		#if d:DEBUG_PRINT
			print '今はいつ？？？　$b:現在時刻='$b:現在時刻
		#endif
		eval( $b:ＶＲ起動時間帯 = d:VR_TIME:夜 );
		eval( $s:ＶＲライトファイル名 = vr_night );

	}

}





/*******************************************************************************************/
// 各共通プロック
/*******************************************************************************************/
/////////////////////////////////
// 各ミッション共通システム関係(必ず最初に呼ぶこと！！)
/////////////////////////////////
//#define	DEBUG_KEYBOARD_CAMERA	1	// デバッグ用ＵＳＢキーボードカメラを使う場合に定義する

proc 全ミッション共通システム設定 $:p_クリア条件フラグ {
	#if d:DEBUG_PRINT
		print 'common_system_set'
	#endif

	#ifdef d:STAGE_VR
		// ＶＲステージの場合、タンカーフラグは必ず寝かすためここで一括処理。それ以外は各ロードプロックで対処
		$w:コンフィグ設定 = ($w:コンフィグ設定 & ~d:CONFIG_STORY_TANKER);
	#endif

	// 敵兵パラメータの設定
	@ミッション別敵兵パラメータ設定
	// ＶＲシステムの起動
	switch ( $b:ミッション番号 ) {
		case ( d:MISSION:爆弾解体 ) {
			@ミッション共通ＶＲシステム起動 ( d:VR_DEFUSE_BOMB | d:VR_COUNT_DOWN | d:VR_TIME_OVER )
		}
		case ( d:MISSION:スネークテイルズ ) {
			@ミッション共通ＶＲシステム起動 ( d:VR_NO_GOAL | $:p_クリア条件フラグ )
			// スネークテイルズの時には必ずこのフラグを立てる
			command テイルズステージセット

			// スネークテイルズではセレクトを押したときウィンドウを出す。
			@テイルズＶＲウィンドウセット
		}
		default {
			@ミッション共通ＶＲシステム起動 ( $:p_クリア条件フラグ | d:VR_COUNT_DOWN | d:VR_TIME_OVER )

			// ＶＲステージの敵兵かどうかを区別するのに必要
			#ifdef d:STAGE_VR
				command ＶＲステージセット
			#endif
		}
	}

	// ＶＲタイマーに渡すためステージローカル変数にフラグを代入
	@各ミッション用タイマー表示 $:p_クリア条件フラグ

#ifndef	d:FIX_MODE
#ifdef d:DEBUG_KEYBOARD_CAMERA
//	@デバッグ用ＵＳＢキーボード対応カメラ
//	@デバッグ用ＵＳＢキーボード対応パッドデモ e3_vs10a_paddemo_03
#endif
#endif

}


// ミッションごとに敵兵のパラメータを設定する(vardef.hの値は上書きされる)
proc ミッション別敵兵パラメータ設定 {
	#if d:DEBUG_PRINT
		print 'enemy_parameter_set'
	#endif

	// ミッションごとのライデン、スネーク時の敵兵パラメータ(アメリカ版ノーマル相当)
	switch ( $b:ミッション番号 ) {
		case ( d:MISSION:敵兵排除 ) {
			$w:敵潜入視力 = 7000;
			$w:敵危険視力 = ((8000*3)/2);
			$w:敵回避視力 = 8000;
			$w:敵標準聴力 = 8000;
			$w:敵標準体力 = 4000;
			$w:敵標準気絶 = 14;
			$i:敵標準麻酔持続 = 60*60*3;
			$i:敵標準気絶持続 = 60*30;
			$w:敵標準再発生数 = 0;
			$w:サイファー標準再発生数 = 0;
		}
		case ( d:MISSION:武装解除 ) {
			$w:敵潜入視力 = 6000;
			$w:敵危険視力 = ((6000*3)/2);
			$w:敵回避視力 = 6000;
			$w:敵標準聴力 = 6000;
			$w:敵標準体力 = 3000;
			$w:敵標準気絶 = 9;
			$i:敵標準麻酔持続 = 60*60*5;
			$i:敵標準気絶持続 = 60*30;
			$w:敵標準再発生数 = 0;		/*	発見即ゲームオーバーだが念のため	*/
			$w:サイファー標準再発生数 = 0;
		}
		default {
			$w:敵潜入視力 = 6000;
			$w:敵危険視力 = ((6000*3)/2);
			$w:敵回避視力 = 6000;
			$w:敵標準聴力 = 6000;
			$w:敵標準体力 = 3000;
			$w:敵標準気絶 = 9;
			$i:敵標準麻酔持続 = 60*60*5;
			$i:敵標準気絶持続 = 60*30;
			$w:敵標準再発生数 = 10;
			$w:サイファー標準再発生数 = 10;
		}
	}

	// スネークテイルズ以外は、キャラごとに敵兵視力を一律あげる
	if ( $b:ミッション番号 != d:MISSION:スネークテイルズ ) {
		if ( $s:選択プレイヤー == プリスキン || \
			 $s:選択プレイヤー == タキシードスネーク || \
			 $s:選択プレイヤー == 刀ライデン ) {
			$w:敵潜入視力 = 8000;
			$w:敵危険視力 = ((8000*3)/2);
			$w:敵回避視力 = 8000;
			$w:敵標準聴力 = 8000;
			$w:敵標準体力 = 4000;
			$w:敵標準気絶 = 14;
		} else if ( $s:選択プレイヤー == 前作スネーク ) {
			$w:敵潜入視力 = 10000;
			$w:敵危険視力 = ((10000*3)/2);
			$w:敵回避視力 = 10000;
			$w:敵標準聴力 = 10000;
			$w:敵標準体力 = 10000;
			$w:敵標準気絶 = 24;

			$i:敵標準麻酔持続 = 60*60*1;
			$i:敵標準気絶持続 = 60*15;
		}
	}

	$i:標準警戒時間 = 60*60 ;

	$i:監視カメラ縦視野角 = 256;
	$i:監視カメラ横視野角 = 256;
	$w:監視カメラ通常視力 = 5500;
	$w:監視カメラ危険視力 = 5500;
	$w:監視カメラ警戒視力 = 5500;
	$w:監視カメラ回避視力 = 5500;
	$i:監視カメラ初期方向 = 0;
	$i:監視カメラ首振り待ち時間 = 0;
	$i:監視カメラ緊急振り幅  = 0;
	$i:ガンカメラ緊急振り幅 = 512;
	$i:ガンカメラニキータ破壊時間 = 300;

	// 爆弾解体とスネークテイルズでは敵兵の情報を記録する
	if ( $b:ミッション番号 == d:MISSION:爆弾解体 || $b:ミッション番号 == d:MISSION:スネークテイルズ ) {
		$w:リセットロード回数 = 3;
//		$b:敵系再セットロード回数 = 3;	// サイファー用(現状、毎回再発生という仕様のためなにもしません)
	} else {
		$w:リセットロード回数 = 0;
//		$b:敵系再セットロード回数 = 0;	// サイファー用
	}
}

proc ミッション共通ＶＲシステム起動 $:p_クリア条件フラグ {
	#if d:DEBUG_PRINT
		print 'vr_system_set'
	#endif

	local $$:ステージＩＤ;
	local $$:タイマー初期値;
	$$:ステージＩＤ = 0;
	$$:タイマー初期値 = 0;

	switch ( $s:選択プレイヤー ) {
		case ( ライデン ) {
			$i:プレイヤー番号 = d:ANOTHER_PLAYER_RAIDEN;
			$$:ステージＩＤ = $$w:ライデンＩＤ;
			$$:タイマー初期値 = $$i:ライデンタイム;
		}
		case ( 刀ライデン ) {
			$i:プレイヤー番号 = d:ANOTHER_PLAYER_BLADE;
			$$:ステージＩＤ = $$w:刀ライデンＩＤ;
			$$:タイマー初期値 = $$i:刀ライデンタイム;
		}
		case ( スネーク ) {
			$i:プレイヤー番号 = d:ANOTHER_PLAYER_SNAKE;
			$$:ステージＩＤ = $$w:スネークＩＤ;
			$$:タイマー初期値 = $$i:スネークタイム;
		}
		case ( プリスキン ) {
			$i:プレイヤー番号 = d:ANOTHER_PLAYER_PLISKIN;
			$$:ステージＩＤ = $$w:プリスキンＩＤ;
			$$:タイマー初期値 = $$i:プリスキンタイム;
		}
		case ( タキシードスネーク ) {
			$i:プレイヤー番号 = d:ANOTHER_PLAYER_TUXEDO;
			$$:ステージＩＤ = $$w:タキシードスネークＩＤ;
			$$:タイマー初期値 = $$i:タキシードスネークタイム;
		}
		case ( 前作スネーク ) {
			$i:プレイヤー番号 = d:ANOTHER_PLAYER_OLD_SNAKE;
			$$:ステージＩＤ = $$w:前作スネークＩＤ;
			$$:タイマー初期値 = $$i:前作スネークタイム;
		}
		case ( 裸ライデン ) {
			$i:プレイヤー番号 = d:ANOTHER_PLAYER_RAIDEN;
			$$:ステージＩＤ = $$w:裸ライデンＩＤ;
			$$:タイマー初期値 = $$i:裸ライデンタイム;
		}

		default {
			#if d:DEBUG_PRINT
				print 'WARNING!!!!!!!不正な値が入っています。$s:選択プレイヤー='$s:選択プレイヤー
			#endif

			command assert ( d:FALSE ) 'player no error  (scenario err) ミッション共通ＶＲシステム起動'

			$i:プレイヤー番号 = d:ANOTHER_PLAYER_RAIDEN;
			$$:ステージＩＤ = $$w:ライデンＩＤ;
			$$:タイマー初期値 = $$i:ライデンタイム;
		}
	}

	switch ( $b:ミッション番号 ) {
		case ( d:MISSION:スニーキング ) {
			// ＶＲシステムの起動
			chara ＶＲシステム ＶＲシステム \
				-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
				-cproc 時間切れプロック 0	/*	ある時間になると呼ばれるプロック	*/

			// 発見即ゲームオーバーの設定
			$w:コンフィグ設定 = ( $w:コンフィグ設定 | d:CONFIG_END_IF_FOUND);

			// 敵兵のメモリーを全てリセットする(爆弾解体とスネークテイルズは一番最初のロード時に自前で呼ぶ)
			command 敵兵メモリーオールリセット
			// 敵兵が分解するところはこれを立てる
			command セットＶＲステータス d:VR_ENEMY_VANISH

			// レーダーの設定
			command メニュー設定 -node_access on
		}

		case ( d:MISSION:武器訓練 ) {
			// ＶＲシステムの起動
			chara ＶＲシステム ＶＲシステム \
				-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
				-score 10 20 30 50 70 100 150 200 300 500 \
			#ifdef d:STAGE_WP41A || d:STAGE_WP42A || d:STAGE_WP43A || d:STAGE_WP44A || d:STAGE_WP45A
				-goal ＰＳＧ１ゴールオープン時プロック \	// ＰＳＧ１ステージは的全滅後にレーダーを表示する
			#endif
				-cproc 時間切れプロック 0	/*	ある時間になると呼ばれるプロック	*/

			// 発見即ゲームオーバーの設定
			$w:コンフィグ設定 = ( $w:コンフィグ設定 & ~d:CONFIG_END_IF_FOUND );

			// レーダーの設定
			command メニュー設定 -node_access on
		}

		case ( d:MISSION:バラエティ ) {
			// 各ステージによってルールはまちまちなのでifdefで設定
			// ＶＲシステムの起動
			#ifdef d:STAGE_SP01A || d:STAGE_SP02A
				chara ＶＲシステム ＶＲシステム \
					-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
					-score 10 20 30 50 70 100 150 200 300 500 \
					-cproc 時間切れプロック 0	/*	ある時間になると呼ばれるプロック	*/

				// 発見即ゲームオーバーの設定
				$w:コンフィグ設定 = ( $w:コンフィグ設定 & ~d:CONFIG_END_IF_FOUND );

				// レーダーの設定
				command メニュー設定 -node_access off

			#elifdef d:STAGE_SP03A
				chara ＶＲシステム ＶＲシステム \
					-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
					-score 10 20 30 50 70 100 150 200 300 500 \
					-cproc 時間切れプロック 0	\/*	ある時間になると呼ばれるプロック	*/
					-goal 敵兵全滅プロック	// 実体はsp03a.gclにある

				// 発見即ゲームオーバーの設定
				$w:コンフィグ設定 = ( $w:コンフィグ設定 & ~d:CONFIG_END_IF_FOUND );

				// 敵兵のメモリーを全てリセットする(爆弾解体とスネークテイルズは一番最初のロード時に自前で呼ぶ)
				command 敵兵メモリーオールリセット
				// 敵兵が分解するところはこれを立てる
				command セットＶＲステータス d:VR_ENEMY_VANISH

				// レーダーの設定
				command メニュー設定 -node_access on

			#elifdef d:STAGE_SP04A
				chara ＶＲシステム ＶＲシステム \
					-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
					-score 10 20 30 50 70 100 150 200 300 500 \
					-cproc 時間切れプロック 0	/*	ある時間になると呼ばれるプロック	*/

				// 発見即ゲームオーバーの設定
				$w:コンフィグ設定 = ( $w:コンフィグ設定 & ~d:CONFIG_END_IF_FOUND );

				// 敵兵のメモリーを全てリセットする(爆弾解体とスネークテイルズは一番最初のロード時に自前で呼ぶ)
				command 敵兵メモリーオールリセット
				// 敵兵が分解するところはこれを立てる
				command セットＶＲステータス d:VR_ENEMY_VANISH

				// レーダーの設定
				command メニュー設定 -node_access on

			#elifdef d:STAGE_SP06A
				chara ＶＲシステム ＶＲシステム \
					-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
					-score 10 20 30 50 70 100 150 200 300 500 \
					-cproc 時間切れプロック 0	\/*	ある時間になると呼ばれるプロック	*/
					-goal 敵兵全滅プロック	// 実体はsp06a.gclにある

				// 発見即ゲームオーバーの設定
				$w:コンフィグ設定 = ( $w:コンフィグ設定 & ~d:CONFIG_END_IF_FOUND );
				// 敵兵が分解するところはこれを立てる
				command セットＶＲステータス d:VR_ENEMY_VANISH

				// 敵兵のメモリーを全てリセットする(爆弾解体とスネークテイルズは一番最初のロード時に自前で呼ぶ)
				command 敵兵メモリーオールリセット

				// レーダーの設定
				command メニュー設定 -node_access off

			#elifdef d:STAGE_SP07A
				chara ＶＲシステム ＶＲシステム \
					-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
					-score 10 20 30 50 70 100 150 200 300 500 \
					-cproc 時間切れプロック 0	/*	ある時間になると呼ばれるプロック	*/

				// 発見即ゲームオーバーの設定
				$w:コンフィグ設定 = ( $w:コンフィグ設定 | d:CONFIG_END_IF_FOUND );

				// 敵兵のメモリーを全てリセットする(爆弾解体とスネークテイルズは一番最初のロード時に自前で呼ぶ)
				command 敵兵メモリーオールリセット

				// レーダーの設定
				command メニュー設定 -node_access off

			#elifdef d:STAGE_SP08A
				chara ＶＲシステム ＶＲシステム \
					-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
					-score 10 20 30 50 70 100 150 200 300 500 \
					-cproc 時間切れプロック 0	/*	ある時間になると呼ばれるプロック	*/

				// 発見即ゲームオーバーの設定
				$w:コンフィグ設定 = ( $w:コンフィグ設定 & ~d:CONFIG_END_IF_FOUND );

				// 敵兵のメモリーを全てリセットする(爆弾解体とスネークテイルズは一番最初のロード時に自前で呼ぶ)
				command 敵兵メモリーオールリセット
				// 敵兵が分解するところはこれを立てる
				command セットＶＲステータス d:VR_ENEMY_VANISH

				// レーダーの設定
				command メニュー設定 -node_access off

			#else
				chara ＶＲシステム ＶＲシステム \
					-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
					-score 10 20 30 50 70 100 150 200 300 500 \
					-cproc 時間切れプロック 0	/*	ある時間になると呼ばれるプロック	*/

				// 発見即ゲームオーバーの設定
				$w:コンフィグ設定 = ( $w:コンフィグ設定 & ~d:CONFIG_END_IF_FOUND );

				// 敵兵のメモリーを全てリセットする(爆弾解体とスネークテイルズは一番最初のロード時に自前で呼ぶ)
				command 敵兵メモリーオールリセット
				// 敵兵が分解するところはこれを立てる
				command セットＶＲステータス d:VR_ENEMY_VANISH

				// レーダーの設定
				command メニュー設定 -node_access on
			#endif
		}

		case ( d:MISSION:主観モード ) {
			// ＶＲシステムの起動
			chara ＶＲシステム ＶＲシステム \
				-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
				-score 10 20 30 50 70 100 150 200 300 500 \
				-cproc 時間切れプロック 0	/*	ある時間になると呼ばれるプロック	*/

			// 発見即ゲームオーバーの設定
			$w:コンフィグ設定 = ( $w:コンフィグ設定 & ~d:CONFIG_END_IF_FOUND );

			// 敵兵のメモリーを全てリセットする(爆弾解体とスネークテイルズは一番最初のロード時に自前で呼ぶ)
			command 敵兵メモリーオールリセット
			// 敵兵が分解するところはこれを立てる
			command セットＶＲステータス d:VR_ENEMY_VANISH

			// レーダーの設定
			command メニュー設定 -node_access on
		}

		case ( d:MISSION:ストリーキング ) {
			// ＶＲシステムの起動
			chara ＶＲシステム ＶＲシステム \
				-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
				-cproc 時間切れプロック 0	/*	ある時間になると呼ばれるプロック	*/

			// 発見即ゲームオーバーの設定
			$w:コンフィグ設定 = ( $w:コンフィグ設定 | d:CONFIG_END_IF_FOUND);

			// 敵兵のメモリーを全てリセットする(爆弾解体とスネークテイルズは一番最初のロード時に自前で呼ぶ)
			command 敵兵メモリーオールリセット
			// 敵兵が分解するところはこれを立てる
			command セットＶＲステータス d:VR_ENEMY_VANISH

			// レーダーの設定
			command メニュー設定 -node_access on
		}

		case ( d:MISSION:爆弾解体 ) {
			if ( $f:爆弾解体ミッションＶＲウィンドウ表示フラグ == 0 ) {
				// オルタナティブ系で途中ステージの場合はグローバル変数のタイマーの値を代入
				$$:タイマー初期値 = $i:爆弾解体タイマー残り時間;
			}

			// ＶＲシステムの起動
			chara ＶＲシステム ＶＲシステム \
				-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
				-cproc 時間切れプロック 0	/*	ある時間になると呼ばれるプロック	*/

			if ( $f:爆弾解体ミッションＶＲウィンドウ表示フラグ == 1 ) {
				// 敵兵のメモリーを全てリセットする(ウィンドウが表示されたときのみ)
				command 敵兵メモリーオールリセット
			}
			// 敵兵が分解するところはこれを立てる
			command セットＶＲステータス d:VR_ENEMY_VANISH

			// 発見即ゲームオーバーの設定
			$w:コンフィグ設定 = ( $w:コンフィグ設定 & ~d:CONFIG_END_IF_FOUND );

			// レーダーの設定
			command メニュー設定 -node_access on
		}

		case ( d:MISSION:敵兵排除 ) {
			// ＶＲシステムの起動
			chara ＶＲシステム ＶＲシステム \
				-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
				-cproc 時間切れプロック 0	/*	ある時間になると呼ばれるプロック	*/

			// 敵兵のメモリーを全てリセットする(爆弾解体とスネークテイルズは一番最初のロード時に自前で呼ぶ)
			command 敵兵メモリーオールリセット
			// 敵兵が分解するところはこれを立てる
			command セットＶＲステータス d:VR_ENEMY_VANISH

			// 発見即ゲームオーバーの設定
			$w:コンフィグ設定 = ( $w:コンフィグ設定 & ~d:CONFIG_END_IF_FOUND );

			// レーダーの設定
			command メニュー設定 -node_access on
		}

		case ( d:MISSION:武装解除 ) {
			// ＶＲシステムの起動
			chara ＶＲシステム ＶＲシステム \
				-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
				-cproc 時間切れプロック 0 \	/*	ある時間になると呼ばれるプロック	*/
				-enep 敵殺したプロック	/*	殺し禁止敵を殺すと実行されるプロック	*/

			// 敵兵のメモリーを全てリセットする(爆弾解体とスネークテイルズは一番最初のロード時に自前で呼ぶ)
			command 敵兵メモリーオールリセット
			// 敵兵が分解するところはこれを立てる
			command セットＶＲステータス d:VR_ENEMY_VANISH

			// 発見即ゲームオーバーの設定
			$w:コンフィグ設定 = ( $w:コンフィグ設定 | d:CONFIG_END_IF_FOUND );

			// レーダーの設定
			command メニュー設定 -node_access on
		}

		case ( d:MISSION:写真撮影 ) {
			// ＶＲシステムの起動
			chara ＶＲシステム ＶＲシステム \
				-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
				-cproc 時間切れプロック 0	/*	ある時間になると呼ばれるプロック	*/

			// 敵兵のメモリーを全てリセットする(爆弾解体とスネークテイルズは一番最初のロード時に自前で呼ぶ)
			command 敵兵メモリーオールリセット
			// 敵兵が分解するところはこれを立てる
			command セットＶＲステータス d:VR_ENEMY_VANISH

			// 発見即ゲームオーバーの設定
			$w:コンフィグ設定 = ( $w:コンフィグ設定 | d:CONFIG_END_IF_FOUND );

			// レーダーの設定
			command メニュー設定 -node_access on
		}

		case ( d:MISSION:スネークテイルズ ) {
			// ＶＲシステムの起動
			chara ＶＲシステム ＶＲシステム \
				-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号

			// 発見即ゲームオーバーの設定
			$w:コンフィグ設定 = ( $w:コンフィグ設定 & ~d:CONFIG_END_IF_FOUND );

			// スネークテイルズだけは通常レーダーオフ
			command メニュー設定 -node_access off
		}

		default {
			#if d:DEBUG_PRINT
				print 'WARNING!!!!!!!不正な値が入っています。$b:ミッション番号='$b:ミッション番号
			#endif

			command assert ( d:FALSE ) 'mission no error  (scenario err) ミッション共通ＶＲシステム起動'

			// ＶＲシステムの起動
			chara ＶＲシステム ＶＲシステム \
				-rule $:p_クリア条件フラグ -time $$:タイマー初期値 -proc クリア時処理 -mode $i:プレイヤー番号 \
				-cproc 時間切れプロック 0 /*	ある時間になると呼ばれるプロック	*/

			// 敵兵のメモリーを全てリセットする(爆弾解体とスネークテイルズは一番最初のロード時に自前で呼ぶ)
			command 敵兵メモリーオールリセット
			// 敵兵が分解するところはこれを立てる
			command セットＶＲステータス d:VR_ENEMY_VANISH

			// 発見即ゲームオーバーの設定
			$w:コンフィグ設定 = ( $w:コンフィグ設定 & ~d:CONFIG_END_IF_FOUND );

			// レーダーの設定
			command メニュー設定 -node_access on
		}
	}

	// 前作スネークだけは強制的にレーダーオフにする
	if ( $s:選択プレイヤー == 前作スネーク ) {
		// レーダーの設定
		command メニュー設定 -node_access off
	}

	// 各ステージのステージＩＤをセットする（ステージの呼び出し順番やハイスコア等に影響）
	command セットステージＩＤ $$:ステージＩＤ
}



// タイマー表示
proc 各ミッション用タイマー表示 $:p_クリア条件フラグ {
	#if d:DEBUG_PRINT
		print 'timer_on'
	#endif

	switch ( $b:ミッション番号 ) {
		case ( d:MISSION:スニーキング ) {
			if ( $b:モード番号 == d:SNEAKING:SNEAKING ) {
				chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_SNEAKING | d:VRSCR_FIVE )
			} else {
				chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_ELIMINATE | d:VRSCR_FIVE )
			}
		}
		case ( d:MISSION:武器訓練 ) {
			chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_WEAPON | d:VRSCR_FIVE )
		}
		case ( d:MISSION:バラエティ ) {
			switch ( $:p_クリア条件フラグ ) {
				case (d:VR_GOAL_TARGET) {
					chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_WEAPON | d:VRSCR_FIVE )
				}
				case (d:VR_GOAL_FREE) {
					chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_SNEAKING | d:VRSCR_FIVE )
				}
				case (d:VR_GOAL_ENEMY) {
					chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_ELIMINATE | d:VRSCR_FIVE )
				}
				case ( d:VR_GOAL_ENEMY | d:VR_NO_OPEN_SE ) {
					chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_ELIMINATE | d:VRSCR_FIVE )
				}
				default {
					chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_SNEAKING | d:VRSCR_FIVE )
				}
			}
		}
		case ( d:MISSION:主観モード ) {
			switch ( $:p_クリア条件フラグ ) {
				case (d:VR_GOAL_TARGET) {
					#ifdef d:STAGE_SP21A
						chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_WEAPON | d:VRSCR_FIVE )
					#else
						// ダークステージ用
						chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_WEAPON2 | d:VRSCR_FIVE )
					#endif
				}
				case (d:VR_GOAL_FREE) {
					chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_SNEAKING | d:VRSCR_FIVE )
				}
				case (d:VR_GOAL_ENEMY) {
					chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_ELIMINATE | d:VRSCR_FIVE )
				}
				case (d:VR_GOAL_SCN) {
					chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_ELIMINATE | d:VRSCR_FIVE )
				}
//---------ここから 追加:大谷20020804
				case (d:VR_DEFUSE_BOMB) {
					chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_BOMB | d:VRSCR_FIVE )
					command セット爆弾数 $b:A_爆弾解体数 $b:A_爆弾総数
				}
//---------ここまで
				default {
					chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_SNEAKING | d:VRSCR_FIVE )
				}
			}
		}
		case ( d:MISSION:ストリーキング ) {
			chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_SNEAKING | d:VRSCR_FIVE )
		}
		case ( d:MISSION:爆弾解体 ) {
			chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_BOMB | d:VRSCR_HIDE_FRAME | d:VRSCR_FIVE )
			command セット爆弾数 $b:A_爆弾解体数 $b:A_爆弾総数
		}
		case ( d:MISSION:敵兵排除 ) {
			chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_ELIMINATE | d:VRSCR_HIDE_FRAME | d:VRSCR_FIVE )
		}
		case ( d:MISSION:武装解除 ) {
			chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_ELIMINATE | d:VRSCR_HIDE_FRAME | d:VRSCR_FIVE )
		}
		case ( d:MISSION:写真撮影 ) {
			chara ＶＲスクリーン ＶＲスクリーン君 -flag ( d:VRSCR_SNEAKING | d:VRSCR_HIDE_FRAME | d:VRSCR_FIVE )
		}
		case ( d:MISSION:スネークテイルズ ) {
			#if d:DEBUG_PRINT
				print 'スネークテイルズではＶＲスクリーンは使いません。'
			#endif
		}

		default {
			#if d:DEBUG_PRINT
				print 'WARNING!!!!!!!不正な値が入っています。$b:ミッション番号='$b:ミッション番号
			#endif

			command assert ( d:FALSE ) 'mission no error  (scenario err) 各ミッション用タイマー表示'
		}

	}
}


// オルタナティブなどでロードをはさむ場合のタイマー処理（マップ定義前設定のなかで呼んでください）
proc ロード時ＶＲタイマー処理 {
	#if d:DEBUG_PRINT
		print 'vr_timer check'
	#endif

	// 最初はタイマーストップ
	command ＶＲタイマーポーズ

	// プレイヤーが動けるようになったらタイマーを表示
	chara プロック連続実行 ＶＲスクリーンチェック \
		-time -1 \
		-exec { \
			if ( $f:ロードチェックＯＮフラグ == 1 ) {
				// ＶＲスクリーンの表示とスタートを合わせる
				chara delay ディレイ -time 1 -exec{ command ＶＲタイマースタート }
				mesg ＶＲスクリーン ＶＲスクリーン君 表示
				mesg プロック連続実行 ＶＲスクリーンチェック kill
			}
		}

}



//-----------------------------
// 爆弾ミッション用プロック
//-----------------------------
// 爆弾解体ミッション用ＶＲウィンドウ
proc 爆弾解体ミッションＶＲウィンドウ設定 $:p_終了プロック {
	#if d:DEBUG_PRINT
		print 'bomb disposal window set'
	#endif

	if ( $s:選択プレイヤー == ライデン || $s:選択プレイヤー == スネーク ) {
		@ＶＲウィンドウ設定	[VRMSG_J:ALT_BMB_1] [VRMSG_E:ALT_BMB_1] [VRMSG_G:ALT_BMB_1] \	// 日、英、独
							[VRMSG_F:ALT_BMB_1] [VRMSG_I:ALT_BMB_1] [VRMSG_S:ALT_BMB_1] \	// 仏、伊、西
							[VRMSG_K:ALT_BMB_1] \											// 韓
							$:p_終了プロック												// 終了プロック

	} else if ( $s:選択プレイヤー == プリスキン || $s:選択プレイヤー == タキシードスネーク ) {
		@ＶＲウィンドウ設定	[VRMSG_J:ALT_BMB_2] [VRMSG_E:ALT_BMB_2] [VRMSG_G:ALT_BMB_2] \	// 日、英、独
							[VRMSG_F:ALT_BMB_2] [VRMSG_I:ALT_BMB_2] [VRMSG_S:ALT_BMB_2] \	// 仏、伊、西
							[VRMSG_K:ALT_BMB_2] \											// 韓
							$:p_終了プロック												// 終了プロック

	} else {
		@ＶＲウィンドウ設定	[VRMSG_J:ALT_BMB_3] [VRMSG_E:ALT_BMB_3] [VRMSG_G:ALT_BMB_3] \	// 日、英、独
							[VRMSG_F:ALT_BMB_3] [VRMSG_I:ALT_BMB_3] [VRMSG_S:ALT_BMB_3] \	// 仏、伊、西
							[VRMSG_K:ALT_BMB_3] \											// 韓
							$:p_終了プロック												// 終了プロック
	}
}

proc 爆弾解体ミッションロード後用ＶＲウィンドウ設定 {
	#if d:DEBUG_PRINT
		print 'bomb disposal window set'
	#endif

	if ( $s:選択プレイヤー == ライデン || $s:選択プレイヤー == スネーク ) {
		@爆弾解体ロード後用ＶＲウィンドウ設定	[VRMSG_J:ALT_BMB_1] [VRMSG_E:ALT_BMB_1] [VRMSG_G:ALT_BMB_1] \	// 日、英、独
												[VRMSG_F:ALT_BMB_1] [VRMSG_I:ALT_BMB_1] [VRMSG_S:ALT_BMB_1] \	// 仏、伊、西
												[VRMSG_K:ALT_BMB_1]											// 韓

	} else if ( $s:選択プレイヤー == プリスキン || $s:選択プレイヤー == タキシードスネーク ) {
		@爆弾解体ロード後用ＶＲウィンドウ設定	[VRMSG_J:ALT_BMB_2] [VRMSG_E:ALT_BMB_2] [VRMSG_G:ALT_BMB_2] \	// 日、英、独
												[VRMSG_F:ALT_BMB_2] [VRMSG_I:ALT_BMB_2] [VRMSG_S:ALT_BMB_2] \	// 仏、伊、西
												[VRMSG_K:ALT_BMB_2]											// 韓

	} else {
		@爆弾解体ロード後用ＶＲウィンドウ設定	[VRMSG_J:ALT_BMB_3] [VRMSG_E:ALT_BMB_3] [VRMSG_G:ALT_BMB_3] \	// 日、英、独
												[VRMSG_F:ALT_BMB_3] [VRMSG_I:ALT_BMB_3] [VRMSG_S:ALT_BMB_3] \	// 仏、伊、西
												[VRMSG_K:ALT_BMB_3] 											// 韓
	}
}

// ロード時にタイマーを止める制御
proc A_爆弾タイマー停止 {
	#if d:DEBUG_PRINT
		print 'bomb timer stop'
	#endif

	if ( $b:ミッション番号 == d:MISSION:爆弾解体 ) {
		command ＶＲタイマーポーズ
	}
}

// 解体Ｃ４設置用共通プロック(通常設置用)
// 通常使用するプロック
// $:p_名前は「解体Ｃ４:01」という感じでつけること
proc A_解体Ｃ４設置 $:p_名前 $:p_位置Ｘ $:p_位置Ｙ $:p_位置Ｚ $:p_ＲＯＴＸ $:p_ＲＯＴＹ $:p_ＲＯＴＺ $:p_タイプ $:p_フラグ番号 $:p_フラグ $:p_トラップ名 $:p_もやもや名 {
	#if d:DEBUG_PRINT
		print 'bomb set'
	#endif

	chara 解体Ｃ４ $:p_名前 \
		-position $:p_位置Ｘ $:p_位置Ｙ $:p_位置Ｚ \
		-rotate $:p_ＲＯＴＸ $:p_ＲＯＴＹ $:p_ＲＯＴＺ \
		-type $:p_タイプ \
		-state $b:A_爆弾解体終了状態フラグ[$:p_フラグ番号] \
		-proc A_爆弾解体時処理 \
		-f $:p_フラグ

	if ( $b:A_爆弾解体終了状態フラグ[$:p_フラグ番号] == 0 ) {
		if ( $:p_タイプ == d:BOMB_A ) {
			// 匂いつき爆弾ならレーダーもやもやを設置
			chara 爆弾検知領域 $:p_もやもや名 -trap $:p_トラップ名
		} else {
			// 匂いなし爆弾なら２ｍ以内に近づいたら爆発するように設置
			trap $:p_トラップ名 d:PLAYER -mask 入る -exec {
				mesg 解体Ｃ４ 解体Ｃ４ blast		//指定解体Ｃ４を爆破
			}
		}
	} else {
		// 爆弾が解体されていたらトラップは消す
		command トラップ切り替え -trap $:p_トラップ名 -switch 0
	}
}

// 敵兵や動くものにつけたりするときに使用するプロック
// $:p_名前は「解体Ｃ４:01」という感じでつけること
// タイプがＢで$:p_トラップ名が「トラップなし」の場合は個別で爆弾処理を設定
proc A_特殊解体Ｃ４設置 $:p_名前 $:p_位置Ｘ $:p_位置Ｙ $:p_位置Ｚ $:p_ＲＯＴＸ $:p_ＲＯＴＹ $:p_ＲＯＴＺ $:p_タイプ $:p_フラグ番号  $:p_フラグ $:p_トラップ名 $:p_対象 $:p_関節番号 $:p_もやもや名 {
	#if d:DEBUG_PRINT
		print 'bomb set $b:A_爆弾解体終了状態フラグ[$:p_フラグ番号]='$b:A_爆弾解体終了状態フラグ[$:p_フラグ番号]
	#endif

	// 解体Ｃ４が下に落ちた（フラグ==2のとき）とき以外では起動する
	if ( $b:A_爆弾解体終了状態フラグ[$:p_フラグ番号] != 2 ) {
		chara 解体Ｃ４ $:p_名前 \
			-position $:p_位置Ｘ $:p_位置Ｙ $:p_位置Ｚ \
			-rotate $:p_ＲＯＴＸ $:p_ＲＯＴＹ $:p_ＲＯＴＺ \
			-type $:p_タイプ \
			-state $b:A_爆弾解体終了状態フラグ[$:p_フラグ番号] \
			-proc A_爆弾解体時処理 \
			-f $:p_フラグ \
			-enemy $:p_対象 $:p_関節番号

		if ( $b:A_爆弾解体終了状態フラグ[$:p_フラグ番号] == 0 ) {
			if ( $:p_タイプ == d:BOMB_A ) {
				// 匂いつき爆弾ならレーダーもやもやを設置
				chara 爆弾検知領域 $:p_もやもや名 -trap $:p_トラップ名
			} else {
				if ( $:p_トラップ名 != トラップなし ) {
					// 匂いなし爆弾なら２ｍ以内に近づいたら爆発するように設置
					trap $:p_トラップ名 d:PLAYER -mask 入る -exec {
						mesg 解体Ｃ４ 解体Ｃ４ blast		//指定解体Ｃ４を爆破
					}
				}
			}
		} else {
			// 爆弾が解体されていたらトラップは消す
			command トラップ切り替え -trap $:p_トラップ名 -switch 0
		}
	}
}


// 敵兵に爆弾をつけたときに毎フレーム行うチェック
proc A_敵兵解体Ｃ４チェック $:p_敵兵名 $:p_連続実行キャラ名 $:p_解体Ｃ４名 {

	$$i:敵兵状態 = 0;

	command ゲット敵兵状態拡張版 $:p_敵兵名 $$i:敵兵状態;

	if ( $$i:敵兵状態 & 0x00001000 ) {
		mesg 解体Ｃ４ $:p_解体Ｃ４名 blast	//指定解体Ｃ４を爆破
		mesg プロック連続実行 $:p_連続実行キャラ名 kill
	}
}

// 敵兵に匂いなし爆弾をつけたときに行う距離チェック(2m以内に近づくとTRUEを返す)
proc A_敵兵距離チェック $:p_敵兵名 {
	command ゲット敵兵座標 $:p_敵兵名 $i:敵兵Ｘ座標変数 $i:敵兵Ｙ座標変数 $i:敵兵Ｚ座標変数
	command VecLen	$i:敵兵Ｘ座標変数 $i:敵兵Ｙ座標変数 $i:敵兵Ｚ座標変数 \
					$i:プレイヤー位置Ｘ $i:プレイヤー位置Ｙ $i:プレイヤー位置Ｚ

	if ( $status <= 2000 ) {
		return d:TRUE
	} else {
		return d:FALSE
	}
}

// 爆弾を解体したときの制御
proc A_爆弾解体時処理 {
	#if d:DEBUG_PRINT
		print 'bomb_disposal $1 $2='$1 $2
	#endif


	if ( $2 == 1 ) {
		#if d:DEBUG_PRINT
			print 'bomb_disposal'
		#endif
		// ちゃんと凍結した
		$b:A_爆弾解体数 = $b:A_爆弾解体数 + 1;

		// フラグを立てる(このプロックは爆弾を置いたステージ内のa***_bomb.hで設定)
		@A_各ステージ別爆弾解体時処理 $1

		#ifndef d:FIX_MODE
			// デバッグ用に爆弾個数をチェックする
			if ( $b:A_爆弾解体数 > $b:A_爆弾総数 ) {
				command assert ( d:FALSE ) 'bomb disposal error (scenario err) A_爆弾解体時処理 解体数が総数を上回りました'
			}
			#if d:DEBUG_PRINT
				print '現在の状況' $b:A_爆弾解体数'/'$b:A_爆弾総数
			#endif

		#endif

		command セット解体済み爆弾数 $b:A_爆弾解体数
	} else if ( $2 == 3 ) {
		// 凍結後、床に落ちた
		#ifdef d:BOMB_DROP_PROC
			@凍結Ｃ４落下後処理 $1
		#endif
	} else if ( $2 == 0 ){
		// 爆発した(Ｂ爆弾対応のためカメラはそこで固定にしてしまう)
		// 主観とイントルード以外の時に修正 by 吉池 2002.10.21
		if ( (!(`command プレイヤー状態取得` & d:PFLAG_SUBJECT) && !(`command プレイヤー状態取得` & d:PFLAG_INTRUDE)) ) {
			chara カメラ 爆発ゲームオーバーカメラ		-c 0 -l 0 -p  0
			chara カメラ設定 爆発ゲームオーバーカメラ \
				-c 1 \
				-p $i:カメラＸ位置,$i:カメラＹ位置,$i:カメラＺ位置 \
				-t $i:注視点Ｘ位置,$i:注視点Ｙ位置,$i:注視点Ｚ位置 \
				-a 200 \
				-i 0 -1 0 0 \
				-s 1
		}

		@爆発ゲームオーバー処理 d:BOMB_EFFECT_ON
	}

}



//-----------------------------
// 各種ゲームオーバー用プロック
//-----------------------------
//	ゲームオーバー共通処理
proc ミッション共通ゲームオーバー処理 {
	#if d:DEBUG_PRINT
		print 'gameover_proc'
	#endif

	if (`%ゲームオーバーチェック` == 0 && $f:ロードチェックＯＮフラグ) {
		command ゲームオーバー処理開始 \
			-exec {
				#ifdef d:STAGE_SP03A
				// ゴルルゴンゲームオーバーの時はシネマスクリーンを消す
					mesg シネマスクリーン シネマ フェードイン 0
					command メニュー設定 -menu off -gage off  -radar off -subwin off
					command ゲームオーバー処理終了

				#elifdef d:STAGE_SP06A
					switch ( $$b:ゲームオーバータイプ ) {
						case ( 1 ) {
							// メリル死亡時ゲームオーバー
							chara delay ゲームオーバーディレイ -time (45+72) -exec {
								command メニュー設定 -menu off -gage off  -radar off -subwin off
								command ゲームオーバー処理終了
							}
						}
						case ( 2 ) {
							// エルード落下死ゲームオーバー
							chara delay ゲームオーバーディレイ -time 120 -exec {
								command メニュー設定 -menu off -gage off  -radar off -subwin off
								command ゲームオーバー処理終了
							}
						}
						default {
							// 通常ゲームオーバー
							command メニュー設定 -menu off -gage off  -radar off -subwin off
							command ゲームオーバー処理終了
						}
					}
					/*
					if( $$b:ゲームオーバータイプ == 1 ) {
					// メリル死亡時のゲームオーバー
						#if d:DEBUG_PRINT
							print 'mrl dead'
						#endif
						chara delay ゲームオーバーディレイ -time (45+72) -exec {
							command メニュー設定 -menu off -gage off  -radar off -subwin off
							command ゲームオーバー処理終了
						}
					} else if( $$b:ゲームオーバータイプ == 2 ) {
						chara delay ゲームオーバーディレイ -time 90 -exec {
							command メニュー設定 -menu off -gage off  -radar off -subwin off
							command ゲームオーバー処理終了
						}
					} else {
					// 通常時のゲームオーバー
						command メニュー設定 -menu off -gage off  -radar off -subwin off
						command ゲームオーバー処理終了
					}
					*/
				#else
					command メニュー設定 -menu off -gage off  -radar off -subwin off
					command ゲームオーバー処理終了
				#endif
			}
	}
}

//	時間切れ
proc 時間切れプロック {
	#if d:DEBUG_PRINT
		print 'time_out_gameover'
	#endif

	#ifdef d:STAGE_VR
		if (`%ゲームオーバーチェック` == 0 && $f:ロードチェックＯＮフラグ) {
			// 時間切れで EXIT のみのゲームオーバーにする場合
			// command ＶＲコンティニュー禁止 を
			// command ゲームオーバー処理開始 より先に呼ぶ必要がある。 02/06/27 T.Ohtani
			if ( $b:ミッション番号 == d:MISSION:ストリーキング ) {
				command ＶＲコンティニュー禁止

				#if d:DEBUG_PRINT
					print '$b:ミッション番号 ' $b:ミッション番号 '番'
					print '時間切れでコンテニューはいけないとおもいます。'
				#endif
			}

			command ゲームオーバー処理開始 \
				-exec {
					#if d:DEBUG_PRINT
						print 'GAMEOVER'
					#endif
					#ifdef d:STAGE_SP03A
					// ザコサバイバルのステージは独自で崩す
						mesg ＶＲオブジェ ゲノラを隠す床 crumble
						mesg ＶＲオブジェ ゴルルゴンを隠す壁 crumble
						mesg ＶＲオブジェ ゴルルゴンを隠す壁の影 crumble
						mesg ＶＲオブジェ 外壁 crumble
						mesg ＶＲオブジェ メインの床 crumble
						mesg ＶＲオブジェ 箱 crumble
						mesg ＶＲオブジェ 箱影 crumble
						command メニュー設定 -menu off -gage off  -radar off -subwin off

						chara delay ディレイ -time 90 \
							-exec {
								command ゲームオーバー処理終了
							}
					#else
						mesg ステージ変形 ステージ変形君 消失開始
						command メニュー設定 -menu off -gage off  -radar off -subwin off

						chara delay ディレイ -time d:STAGE_CHANGE_TIME \
							-exec {
								command ゲームオーバー処理終了
							}
					#endif
				}
		}


	#else
		if ( $b:ミッション番号 == d:MISSION:爆弾解体 ) {
			command ＶＲコンティニュー禁止
		}
		@ミッション共通ゲームオーバー処理
	#endif

}

// ゴールオープン時プロック
proc ＰＳＧ１ゴールオープン時プロック {
	print 'ゴールオープン レーダーＯＮ'
	command メニュー設定 -radar on
}

//	敵殺した
proc 敵殺したプロック {
	#if d:DEBUG_PRINT
		print 'enemy_kill_gameover'
	#endif

	@ミッション共通ゲームオーバー処理
}



/////////////////////////////////
// ゴールの表示（マップ設定の中で定義しないとだめ）
/////////////////////////////////
proc 各ミッション用ゴール表示 $:p_ゴールＸ $:p_ゴールＹ $:p_ゴールＺ {
	#if d:DEBUG_PRINT
		print 'goal_on'
	#endif

	chara ＶＲゴール ゴール君 -pos $:p_ゴールＸ $:p_ゴールＹ $:p_ゴールＺ -speed 30
}



/////////////////////////////////
// ＶＲウィンドウの設定(mesgで表示する)
/////////////////////////////////
proc ＶＲウィンドウ設定 $:p_リソース日 $:p_リソース英 $:p_リソース独 $:p_リソース仏 $:p_リソース伊 $:p_リソース西 $:p_リソース韓 $:p_終了時プロック {
	#if d:DEBUG_PRINT
		print 'vr_window_set'
	#endif

	local $$:ＶＲウィンドウフラグ;
	local $$:スタートジングル;

	if ( $w:言語 == d:LANG_JAPANESE ){
		$$:ＶＲウィンドウフラグ = d:VRWIN_FLAG_JAPANESE;
	} else {
		$$:ＶＲウィンドウフラグ = 0;
	}

	#ifdef d:STAGE_VR
		if ( $s:選択プレイヤー != 前作スネーク ) {
			$$:スタートジングル = t:vc030201;	// VRmission_startBGM
		} else {
			$$:スタートジングル = t:vc030203;	// VRmission_startBGM(前作スネーク使用時)
		}
	#else
			$$:スタートジングル = t:vc030202;	// Alternativemission_startBGM
	#endif

	#ifdef d:STAGE_SP03A
	// ザコサバイバルのみはセレクトのみの表示があるので特殊処理
		if ( $f:sp03a_ザコサバイバルウィンドウ表示終了フラグ == 0 ) {
			chara ＶＲウィンドウ ＶＲウィンドウ君 \
				-message $:p_リソース日 $:p_リソース英 $:p_リソース仏 $:p_リソース独 $:p_リソース伊 $:p_リソース西 $:p_リソース韓 \
				-text_a [VRMSG_J:時間] [VRMSG_E:時間] [VRMSG_F:時間] [VRMSG_G:時間] [VRMSG_I:時間] [VRMSG_S:時間] [VRMSG_K:時間]\
				-text_b [VRMSG_J:分] [VRMSG_E:分] [VRMSG_F:分] [VRMSG_G:分] [VRMSG_I:分] [VRMSG_S:分] [VRMSG_K:分] \
				-text_c [VRMSG_J:秒] [VRMSG_E:秒] [VRMSG_F:秒] [VRMSG_G:秒] [VRMSG_I:秒] [VRMSG_S:秒] [VRMSG_K:秒] \
				-text_d [VRMSG_J:敵兵数] [VRMSG_E:敵兵数] [VRMSG_F:敵兵数] [VRMSG_G:敵兵数] [VRMSG_I:敵兵数] [VRMSG_S:敵兵数] [VRMSG_K:敵兵数] \
				-text_e [VRMSG_J:人] [VRMSG_E:人] [VRMSG_F:人] [VRMSG_G:人] [VRMSG_I:人] [VRMSG_S:人] [VRMSG_K:人] \
				-text_f [VRMSG_J:標的数] [VRMSG_E:標的数] [VRMSG_F:標的数] [VRMSG_G:標的数] [VRMSG_I:標的数] [VRMSG_S:標的数] [VRMSG_K:標的数] \
				-text_g [VRMSG_J:個] [VRMSG_E:個] [VRMSG_F:個] [VRMSG_G:個] [VRMSG_I:個] [VRMSG_S:個] [VRMSG_K:個] \
				-text_h [VRMSG_J:爆弾数] [VRMSG_E:爆弾数] [VRMSG_F:爆弾数] [VRMSG_G:爆弾数] [VRMSG_I:爆弾数] [VRMSG_S:爆弾数] [VRMSG_K:爆弾数] \
				-text_i [VRMSG_J:個] [VRMSG_E:個] [VRMSG_F:個] [VRMSG_G:個] [VRMSG_I:個] [VRMSG_S:個] [VRMSG_K:個] \
				-proc	$:p_終了時プロック \
				-bgm 	$$:スタートジングル \
				#if d:MGS2_VRTRIAL
					-flag	( d:VRWIN_FLAG_JAPANESE | d:VRWIN_FLAG_TRIAL )
				#else
					-flag	$$:ＶＲウィンドウフラグ
				#endif
		} else {
			$$:ＶＲウィンドウフラグ = ( $$:ＶＲウィンドウフラグ | d:VRWIN_HIDE_START ) ;

			chara ＶＲウィンドウ ＶＲウィンドウ君 \
				-message $:p_リソース日 $:p_リソース英 $:p_リソース仏 $:p_リソース独 $:p_リソース伊 $:p_リソース西 $:p_リソース韓 \
				-text_a [VRMSG_J:時間] [VRMSG_E:時間] [VRMSG_F:時間] [VRMSG_G:時間] [VRMSG_I:時間] [VRMSG_S:時間] [VRMSG_K:時間]\
				-text_b [VRMSG_J:分] [VRMSG_E:分] [VRMSG_F:分] [VRMSG_G:分] [VRMSG_I:分] [VRMSG_S:分] [VRMSG_K:分] \
				-text_c [VRMSG_J:秒] [VRMSG_E:秒] [VRMSG_F:秒] [VRMSG_G:秒] [VRMSG_I:秒] [VRMSG_S:秒] [VRMSG_K:秒] \
				-text_d [VRMSG_J:敵兵数] [VRMSG_E:敵兵数] [VRMSG_F:敵兵数] [VRMSG_G:敵兵数] [VRMSG_I:敵兵数] [VRMSG_S:敵兵数] [VRMSG_K:敵兵数] \
				-text_e [VRMSG_J:人] [VRMSG_E:人] [VRMSG_F:人] [VRMSG_G:人] [VRMSG_I:人] [VRMSG_S:人] [VRMSG_K:人] \
				-text_f [VRMSG_J:標的数] [VRMSG_E:標的数] [VRMSG_F:標的数] [VRMSG_G:標的数] [VRMSG_I:標的数] [VRMSG_S:標的数] [VRMSG_K:標的数] \
				-text_g [VRMSG_J:個] [VRMSG_E:個] [VRMSG_F:個] [VRMSG_G:個] [VRMSG_I:個] [VRMSG_S:個] [VRMSG_K:個] \
				-text_h [VRMSG_J:爆弾数] [VRMSG_E:爆弾数] [VRMSG_F:爆弾数] [VRMSG_G:爆弾数] [VRMSG_I:爆弾数] [VRMSG_S:爆弾数] [VRMSG_K:爆弾数] \
				-text_i [VRMSG_J:個] [VRMSG_E:個] [VRMSG_F:個] [VRMSG_G:個] [VRMSG_I:個] [VRMSG_S:個] [VRMSG_K:個] \
				-proc	$:p_終了時プロック \
				#if d:MGS2_VRTRIAL
					-flag	( d:VRWIN_FLAG_JAPANESE | d:VRWIN_FLAG_TRIAL )
				#else
					-flag	$$:ＶＲウィンドウフラグ
				#endif
		}
	#else
		chara ＶＲウィンドウ ＶＲウィンドウ君 \
			-message $:p_リソース日 $:p_リソース英 $:p_リソース仏 $:p_リソース独 $:p_リソース伊 $:p_リソース西 $:p_リソース韓 \
			-text_a [VRMSG_J:時間] [VRMSG_E:時間] [VRMSG_F:時間] [VRMSG_G:時間] [VRMSG_I:時間] [VRMSG_S:時間] [VRMSG_K:時間]\
			-text_b [VRMSG_J:分] [VRMSG_E:分] [VRMSG_F:分] [VRMSG_G:分] [VRMSG_I:分] [VRMSG_S:分] [VRMSG_K:分] \
			-text_c [VRMSG_J:秒] [VRMSG_E:秒] [VRMSG_F:秒] [VRMSG_G:秒] [VRMSG_I:秒] [VRMSG_S:秒] [VRMSG_K:秒] \
			-text_d [VRMSG_J:敵兵数] [VRMSG_E:敵兵数] [VRMSG_F:敵兵数] [VRMSG_G:敵兵数] [VRMSG_I:敵兵数] [VRMSG_S:敵兵数] [VRMSG_K:敵兵数] \
			-text_e [VRMSG_J:人] [VRMSG_E:人] [VRMSG_F:人] [VRMSG_G:人] [VRMSG_I:人] [VRMSG_S:人] [VRMSG_K:人] \
			-text_f [VRMSG_J:標的数] [VRMSG_E:標的数] [VRMSG_F:標的数] [VRMSG_G:標的数] [VRMSG_I:標的数] [VRMSG_S:標的数] [VRMSG_K:標的数] \
			-text_g [VRMSG_J:個] [VRMSG_E:個] [VRMSG_F:個] [VRMSG_G:個] [VRMSG_I:個] [VRMSG_S:個] [VRMSG_K:個] \
			-text_h [VRMSG_J:爆弾数] [VRMSG_E:爆弾数] [VRMSG_F:爆弾数] [VRMSG_G:爆弾数] [VRMSG_I:爆弾数] [VRMSG_S:爆弾数] [VRMSG_K:爆弾数] \
			-text_i [VRMSG_J:個] [VRMSG_E:個] [VRMSG_F:個] [VRMSG_G:個] [VRMSG_I:個] [VRMSG_S:個] [VRMSG_K:個] \
			-proc	$:p_終了時プロック \
			-bgm 	$$:スタートジングル \
			#if d:MGS2_VRTRIAL
				-flag	( d:VRWIN_FLAG_JAPANESE | d:VRWIN_FLAG_TRIAL )
			#else
				-flag	$$:ＶＲウィンドウフラグ
			#endif
	#endif

	// ＶＲウィンドウが出る時はパッド操作を奪う
	command パッド操作 -release
	// メニューも非表示にする
	command メニュー設定 -menu off -gage off -radar off -subwin off -radio off -pause off
	// 敵や的の動きを止める
	command ＶＲステージポーズ
}
proc 爆弾解体ロード後用ＶＲウィンドウ設定 $:p_リソース日 $:p_リソース英 $:p_リソース独 $:p_リソース仏 $:p_リソース伊 $:p_リソース西 $:p_リソース韓 {
	#if d:DEBUG_PRINT
		print 'vr_window_set'
	#endif

	local $$:ＶＲウィンドウフラグ;

	if ( $w:言語 == d:LANG_JAPANESE ){
		$$:ＶＲウィンドウフラグ = d:VRWIN_FLAG_JAPANESE;
	} else {
		$$:ＶＲウィンドウフラグ = 0;
	}

	$$:ＶＲウィンドウフラグ = ( $$:ＶＲウィンドウフラグ | d:VRWIN_HIDE_START ) ;

	chara ＶＲウィンドウ ＶＲウィンドウ君 \
		-message $:p_リソース日 $:p_リソース英 $:p_リソース仏 $:p_リソース独 $:p_リソース伊 $:p_リソース西 $:p_リソース韓 \
		-text_a [VRMSG_J:時間] [VRMSG_E:時間] [VRMSG_F:時間] [VRMSG_G:時間] [VRMSG_I:時間] [VRMSG_S:時間] [VRMSG_K:時間]\
		-text_b [VRMSG_J:分] [VRMSG_E:分] [VRMSG_F:分] [VRMSG_G:分] [VRMSG_I:分] [VRMSG_S:分] [VRMSG_K:分] \
		-text_c [VRMSG_J:秒] [VRMSG_E:秒] [VRMSG_F:秒] [VRMSG_G:秒] [VRMSG_I:秒] [VRMSG_S:秒] [VRMSG_K:秒] \
		-text_d [VRMSG_J:敵兵数] [VRMSG_E:敵兵数] [VRMSG_F:敵兵数] [VRMSG_G:敵兵数] [VRMSG_I:敵兵数] [VRMSG_S:敵兵数] [VRMSG_K:敵兵数] \
		-text_e [VRMSG_J:人] [VRMSG_E:人] [VRMSG_F:人] [VRMSG_G:人] [VRMSG_I:人] [VRMSG_S:人] [VRMSG_K:人] \
		-text_f [VRMSG_J:標的数] [VRMSG_E:標的数] [VRMSG_F:標的数] [VRMSG_G:標的数] [VRMSG_I:標的数] [VRMSG_S:標的数] [VRMSG_K:標的数] \
		-text_g [VRMSG_J:個] [VRMSG_E:個] [VRMSG_F:個] [VRMSG_G:個] [VRMSG_I:個] [VRMSG_S:個] [VRMSG_K:個] \
		-text_h [VRMSG_J:爆弾数] [VRMSG_E:爆弾数] [VRMSG_F:爆弾数] [VRMSG_G:爆弾数] [VRMSG_I:爆弾数] [VRMSG_S:爆弾数] [VRMSG_K:爆弾数] \
		-text_i [VRMSG_J:個] [VRMSG_E:個] [VRMSG_F:個] [VRMSG_G:個] [VRMSG_I:個] [VRMSG_S:個] [VRMSG_K:個] \
		#if d:MGS2_VRTRIAL
			-flag	( d:VRWIN_FLAG_JAPANESE | d:VRWIN_FLAG_TRIAL )
		#else
			-flag	$$:ＶＲウィンドウフラグ
		#endif
}


proc 写真撮影用ＶＲウィンドウ設定 $:p_リソース日 $:p_リソース英 $:p_リソース独 $:p_リソース仏 $:p_リソース伊 $:p_リソース西 $:p_リソース韓 $:p_終了時プロック $:p_tri名 $:p_テクスチャ名 {
	#if d:DEBUG_PRINT
		print 'vr_window_set'
	#endif

	local $$:ＶＲウィンドウフラグ;

	if ( $w:言語 == d:LANG_JAPANESE ){
		$$:ＶＲウィンドウフラグ = d:VRWIN_FLAG_JAPANESE;
	} else {
		$$:ＶＲウィンドウフラグ = 0;
	}

	chara ＶＲウィンドウ ＶＲウィンドウ君 \
		-message $:p_リソース日 $:p_リソース英 $:p_リソース仏 $:p_リソース独 $:p_リソース伊 $:p_リソース西 $:p_リソース韓 \
		-text_a [VRMSG_J:時間] [VRMSG_E:時間] [VRMSG_F:時間] [VRMSG_G:時間] [VRMSG_I:時間] [VRMSG_S:時間] [VRMSG_K:時間]\
		-text_b [VRMSG_J:分] [VRMSG_E:分] [VRMSG_F:分] [VRMSG_G:分] [VRMSG_I:分] [VRMSG_S:分] [VRMSG_K:分] \
		-text_c [VRMSG_J:秒] [VRMSG_E:秒] [VRMSG_F:秒] [VRMSG_G:秒] [VRMSG_I:秒] [VRMSG_S:秒] [VRMSG_K:秒] \
		-text_d [VRMSG_J:敵兵数] [VRMSG_E:敵兵数] [VRMSG_F:敵兵数] [VRMSG_G:敵兵数] [VRMSG_I:敵兵数] [VRMSG_S:敵兵数] [VRMSG_K:敵兵数] \
		-text_e [VRMSG_J:人] [VRMSG_E:人] [VRMSG_F:人] [VRMSG_G:人] [VRMSG_I:人] [VRMSG_S:人] [VRMSG_K:人] \
		-text_f [VRMSG_J:標的数] [VRMSG_E:標的数] [VRMSG_F:標的数] [VRMSG_G:標的数] [VRMSG_I:標的数] [VRMSG_S:標的数] [VRMSG_K:標的数] \
		-text_g [VRMSG_J:個] [VRMSG_E:個] [VRMSG_F:個] [VRMSG_G:個] [VRMSG_I:個] [VRMSG_S:個] [VRMSG_K:個] \
		-text_h [VRMSG_J:爆弾数] [VRMSG_E:爆弾数] [VRMSG_F:爆弾数] [VRMSG_G:爆弾数] [VRMSG_I:爆弾数] [VRMSG_S:爆弾数] [VRMSG_K:爆弾数] \
		-text_i [VRMSG_J:個] [VRMSG_E:個] [VRMSG_F:個] [VRMSG_G:個] [VRMSG_I:個] [VRMSG_S:個] [VRMSG_K:個] \
		-pict	$:p_tri名 $:p_テクスチャ名 \
		-proc	$:p_終了時プロック \
		-flag	$$:ＶＲウィンドウフラグ \
		-bgm 	t:vc030202			// Alternativemission_startBGM

	// ＶＲウィンドウが出る時はパッド操作を奪う
	command パッド操作 -release

	// メニューも非表示にする
	command メニュー設定 -menu off -gage off -radar off -subwin off -radio off -pause off

	// 敵や的の動きを止める
	command ＶＲステージポーズ
}

// ストリーキングミッションはサブレベルを必要とします。
proc ＶＲストリーキングウィンドウ設定 $:p_リソース日 $:p_リソース英 $:p_リソース独 $:p_リソース仏 $:p_リソース伊 $:p_リソース西 $:p_リソース韓 $:p_終了時プロック $:p_サブレベル {
	#if d:DEBUG_PRINT
		print 'vr_window_set'
	#endif

	local $$:ＶＲウィンドウフラグ;
	local $$:スタートジングル;

	if ( $w:言語 == d:LANG_JAPANESE ){
		$$:ＶＲウィンドウフラグ = d:VRWIN_FLAG_JAPANESE;
	} else {
		$$:ＶＲウィンドウフラグ = 0;
	}

	#ifdef d:STAGE_VR
		if ( $s:選択プレイヤー != 前作スネーク ) {
			$$:スタートジングル = t:vc030201;	// VRmission_startBGM
		} else {
			$$:スタートジングル = t:vc030203;	// VRmission_startBGM(前作スネーク使用時)
		}
	#else
			$$:スタートジングル = t:vc030202;	// Alternativemission_startBGM
	#endif

	chara ＶＲウィンドウ ＶＲウィンドウ君 \
		-message $:p_リソース日 $:p_リソース英 $:p_リソース仏 $:p_リソース独 $:p_リソース伊 $:p_リソース西 $:p_リソース韓 \
		-text_a [VRMSG_J:時間] [VRMSG_E:時間] [VRMSG_F:時間] [VRMSG_G:時間] [VRMSG_I:時間] [VRMSG_S:時間] [VRMSG_K:時間]\
		-text_b [VRMSG_J:分] [VRMSG_E:分] [VRMSG_F:分] [VRMSG_G:分] [VRMSG_I:分] [VRMSG_S:分] [VRMSG_K:分] \
		-text_c [VRMSG_J:秒] [VRMSG_E:秒] [VRMSG_F:秒] [VRMSG_G:秒] [VRMSG_I:秒] [VRMSG_S:秒] [VRMSG_K:秒] \
		-text_d [VRMSG_J:敵兵数] [VRMSG_E:敵兵数] [VRMSG_F:敵兵数] [VRMSG_G:敵兵数] [VRMSG_I:敵兵数] [VRMSG_S:敵兵数] [VRMSG_K:敵兵数] \
		-text_e [VRMSG_J:人] [VRMSG_E:人] [VRMSG_F:人] [VRMSG_G:人] [VRMSG_I:人] [VRMSG_S:人] [VRMSG_K:人] \
		-text_f [VRMSG_J:標的数] [VRMSG_E:標的数] [VRMSG_F:標的数] [VRMSG_G:標的数] [VRMSG_I:標的数] [VRMSG_S:標的数] [VRMSG_K:標的数] \
		-text_g [VRMSG_J:個] [VRMSG_E:個] [VRMSG_F:個] [VRMSG_G:個] [VRMSG_I:個] [VRMSG_S:個] [VRMSG_K:個] \
		-text_h [VRMSG_J:爆弾数] [VRMSG_E:爆弾数] [VRMSG_F:爆弾数] [VRMSG_G:爆弾数] [VRMSG_I:爆弾数] [VRMSG_S:爆弾数] [VRMSG_K:爆弾数] \
		-text_i [VRMSG_J:個] [VRMSG_E:個] [VRMSG_F:個] [VRMSG_G:個] [VRMSG_I:個] [VRMSG_S:個] [VRMSG_K:個] \
		-proc	$:p_終了時プロック \
		-bgm 	$$:スタートジングル \
		-sublevel $:p_サブレベル \
		#if d:MGS2_VRTRIAL
			-flag	( d:VRWIN_FLAG_JAPANESE | d:VRWIN_FLAG_TRIAL )
		#else
			-flag	$$:ＶＲウィンドウフラグ
		#endif

	// ＶＲウィンドウが出る時はパッド操作を奪う
	command パッド操作 -release
	// メニューも非表示にする
	command メニュー設定 -menu off -gage off -radar off -subwin off -radio off -pause off
	// 敵や的の動きを止める
	command ＶＲステージポーズ
}



/////////////////////////////////
// 全ミッション共通ステージ開始時処理
/////////////////////////////////
proc 全ミッション共通ステージ開始時処理 {
	#if d:DEBUG_PRINT
		print 'stage_start!!!!!!'
	#endif

	// タイマーを表示
	mesg ＶＲスクリーン ＶＲスクリーン君 表示

	if ( $b:ミッション番号 != d:MISSION:主観モード ) {
		chara プレイヤ登場 プレイヤ登場君 -proc プレイヤー登場後処理
	} else {
		// カメラが終了しきるまでプレイヤーの制御を開始しない
		chara delay ディレイ \
			-time (d:START_CAMERA_CHANGE_TIME * 2) \
			-exec {
				@プレイヤー登場後処理
			}
	}

	#ifdef d:STAGE_VR
		mesg ステージ変形 ステージ変形君 変形開始
	#endif
}

proc プレイヤー登場後処理 {
	#if d:DEBUG_PRINT
		print 'player_start_end!!!!!!'
	#endif

	eval($f:ロードチェックＯＮフラグ = 1);
	// パッド操作開始
	command パッド操作 -cancel

	// メニューを表示する
	#ifdef d:STAGE_W01A || d:STAGE_W24A || d:STAGE_W24C
		// 写真撮影ではステージが狭いのでレーダーは必要なし
		if( $b:ミッション番号 == d:MISSION:写真撮影 ){
			command メニュー設定 -menu on -gage on -subwin on -radio on -pause on
		} else {
			command メニュー設定 -menu on -gage on -radar on -subwin on -radio on -pause on
		}
	#elifdef d:STAGE_WP41A || d:STAGE_WP42A || d:STAGE_WP43A || d:STAGE_WP44A || d:STAGE_WP45A
		// 狙撃ステージではレーダーは役に立たないので
		command メニュー設定 -menu on -gage on -subwin on -radio on -pause on
	#else
		command メニュー設定 -menu on -gage on -radar on -subwin on -radio on -pause on
	#endif

	// 敵兵等の動作開始
	command ＶＲステージスタート

	#ifdef d:STAGE_START
		@ゲーム開始直後処理
	#endif

}



/////////////////////////////////
// ＶＲクリア設定
/////////////////////////////////
// SCORE RATE
#define SCORE_RATE_A	2			// フレーム
#define SCORE_RATE_B	50			// Ｍ９、ソコム、ＵＳＰ
#define SCORE_RATE_C	5			// Ｍ４、ＡＫＳ
#define SCORE_RATE_D	50			// ＲＧＢ６、スティンガー、Ｃ４、クレイモア
#define SCORE_RATE_E	0			// コンボ
#define SCORE_RATE_F	500			// SNEAKING 1500→1000→500
#define SCORE_RATE_G	2000		// NO-KILL
// SCORE RATE(BOMB MISSION)
#define SCORE_RATE_A_BOMB	1		// フレーム
#define SCORE_RATE_B_BOMB	50		// Ｍ９、ソコム、ＵＳＰ
#define SCORE_RATE_C_BOMB	5		// Ｍ４、ＡＫＳ
#define SCORE_RATE_D_BOMB	50		// ＲＧＢ６、スティンガー、Ｃ４、クレイモア
#define SCORE_RATE_E_BOMB	0		// コンボ
#define SCORE_RATE_F_BOMB	500		// SNEAKING 1500→1000→500
#define SCORE_RATE_G_BOMB	2000	// NO-KILL

//	ＶＲクリア設定を変更した場合はテイルズＥで対応する必要があるので向手まで連絡してください

proc ＶＲクリア設定 $:p_ＮＥＸＴＳＴＡＧＥプロック $:p_ＲＥＴＲＹプロック {
	#if d:DEBUG_PRINT
		print 'vr_clear_set'
		print '$$i:ＶＲクリアフラグ='$$i:ＶＲクリアフラグ
	#endif

	local $$:スコアタイプ;
	local $$:秒変換レート;
	local $$:単発系武器変換レート;
	local $$:連射系武器変換レート;
	local $$:爆発系武器変換レート;
	local $$:隠密変換レート;
	local $$:不殺変換レート;
	local $$:一位ジングル;
	local $$:二位ジングル;
	local $$:三位ジングル;
	local $$:圏外ジングル;

	if ( $b:ミッション番号 == d:MISSION:爆弾解体 ) {
		$$:秒変換レート = d:SCORE_RATE_A_BOMB;
		$$:単発系武器変換レート = d:SCORE_RATE_B_BOMB;
		$$:連射系武器変換レート = d:SCORE_RATE_C_BOMB;
		$$:爆発系武器変換レート = d:SCORE_RATE_D_BOMB;
		$$:隠密変換レート = d:SCORE_RATE_F_BOMB;
		$$:不殺変換レート = d:SCORE_RATE_G_BOMB;
	} else {
		#ifdef d:STAGE_SP03A || d:STAGE_SP25A
			// 制限時間が12分を超えるステージはフレームレートが１
			$$:秒変換レート = d:SCORE_RATE_A_BOMB;
		#else
			$$:秒変換レート = d:SCORE_RATE_A;
		#endif
		$$:単発系武器変換レート = d:SCORE_RATE_B;
		$$:連射系武器変換レート = d:SCORE_RATE_C;
		$$:爆発系武器変換レート = d:SCORE_RATE_D;
		$$:隠密変換レート = d:SCORE_RATE_F;
		$$:不殺変換レート = d:SCORE_RATE_G;
	}


	#ifdef d:STAGE_VR
		if ( $s:選択プレイヤー == ライデン || $s:選択プレイヤー == 刀ライデン || $s:選択プレイヤー == 裸ライデン ) {
			$$:一位ジングル = t:vc030215;
			$$:二位ジングル = t:vc030216;
			$$:三位ジングル = t:vc030217;
			$$:圏外ジングル = t:vc030218;

		} else if ( $s:選択プレイヤー == スネーク || $s:選択プレイヤー == プリスキン || $s:選択プレイヤー == タキシードスネーク ) {
			$$:一位ジングル = t:vc030211;
			$$:二位ジングル = t:vc030212;
			$$:三位ジングル = t:vc030213;
			$$:圏外ジングル = t:vc030214;

		} else {
			// 前作スネーク
			$$:一位ジングル = t:vc030241;
			$$:二位ジングル = t:vc030242;
			$$:三位ジングル = t:vc030243;
			$$:圏外ジングル = t:vc030244;

		}
	#else
		if ( $s:選択プレイヤー == ライデン || $s:選択プレイヤー == 刀ライデン || $s:選択プレイヤー == 裸ライデン ) {
			$$:一位ジングル = t:vc030235;
			$$:二位ジングル = t:vc030236;
			$$:三位ジングル = t:vc030237;
			$$:圏外ジングル = t:vc030238;

		} else {
			// スネーク系
			$$:一位ジングル = t:vc030231;
			$$:二位ジングル = t:vc030232;
			$$:三位ジングル = t:vc030233;
			$$:圏外ジングル = t:vc030234;

		}
	#endif


	#if d:MGS2_VRTRIAL	//chara ＶＲクリア
		$$i:ＶＲクリアフラグ = ( $$i:ＶＲクリアフラグ | d:VRCLR_VRTRIAL );
	#endif

	if ( $b:ミッション番号 != d:MISSION:バラエティ && $b:ミッション番号 != d:MISSION:主観モード ) {
		// 通常バージョン
		chara ＶＲクリア ＶＲクリア君 \
			-rate_a $$:秒変換レート	\			// $i:秒変換レート(フレーム単位)
			-rate_b $$:単発系武器変換レート	\	// $i:単発系武器変換レート
			-rate_c $$:連射系武器変換レート	\	// $i:連射系武器変換レート
			-rate_d $$:爆発系武器変換レート	\	// $i:爆発系武器変換レート
			-rate_e 0	\						// $i:コンボ変換レート(コンボは仕様変更によりなくなりました）
			-rate_f $$:隠密変換レート	\		// $i:隠密変換レート
			-rate_g $$:不殺変換レート	\		// $i:不殺変換レート
			-next_stage $:p_ＮＥＸＴＳＴＡＧＥプロック \
			-try_again $:p_ＲＥＴＲＹプロック \
			-exit A_ＥＸＩＴプロック \
			-f $$i:ＶＲクリアフラグ \
			-bgm $$:一位ジングル $$:二位ジングル $$:三位ジングル $$:圏外ジングル

	} else {
		// バラエティと主観モードはステージごとにスコアタイプを設定
		#ifdef d:STAGE_SP01A
			if ( $b:ステージ番号 == 1 ) {
				// 敵が出ないor殺せない&最初からゴールが出ているタイプ
				$$:スコアタイプ = d:SCORE_TYPE_05;		//            TIME
			} else {
				// 的を全て壊すとゴールが出るタイプ
				$$:スコアタイプ = d:SCORE_TYPE_03;		// MAP SCORE  TIME  BULLET  COMBO
			}

		#elifdef d:STAGE_SP02A || d:STAGE_SP07A
			// 敵が出ないor殺せない&最初からゴールが出ているタイプ
			$$:スコアタイプ = d:SCORE_TYPE_05;			//            TIME

		#elifdef d:STAGE_SP03A || d:STAGE_SP06A
			// 敵を全滅させるとゴールが出るタイプ
			$$:スコアタイプ = d:SCORE_TYPE_02;			//            TIME  BULLET                   NO KILL

		#elifdef d:STAGE_SP08A
			// 敵を全滅させるとゴールが出るタイプ
			$$:スコアタイプ = d:SCORE_TYPE_04;			//            TIME  BULLET         SNEAKING  NO KILL

		#elifdef d:STAGE_SP21A
			// 的を全て壊すとゴールが出るタイプ
			$$:スコアタイプ = d:SCORE_TYPE_03;		// MAP SCORE  TIME  BULLET  COMBO

		#elifdef d:STAGE_SP22A || d:d:STAGE_SP23A || d:STAGE_SP24A || d:STAGE_SP25A
			$$:スコアタイプ = d:SCORE_TYPE_04;			//            TIME  BULLET         SNEAKING  NO KILL

		#else
			// デフォルトタイプ
			$$:スコアタイプ = d:SCORE_TYPE_05;			//			  TIME

		#endif

		// バラエティは特殊バージョン
		chara ＶＲクリア ＶＲクリア君 \
			-rate_a $$:秒変換レート	\			// $i:秒変換レート(フレーム単位)
			-rate_b $$:単発系武器変換レート	\	// $i:単発系武器変換レート
			-rate_c $$:連射系武器変換レート	\	// $i:連射系武器変換レート
			-rate_d $$:爆発系武器変換レート	\	// $i:爆発系武器変換レート
			-rate_e 0	\						// $i:コンボ変換レート(コンボは仕様変更によりなくなりました）
			-rate_f $$:隠密変換レート	\		// $i:隠密変換レート
			-rate_g $$:不殺変換レート	\		// $i:不殺変換レート
			-next_stage $:p_ＮＥＸＴＳＴＡＧＥプロック \
			-try_again $:p_ＲＥＴＲＹプロック \
			-exit A_ＥＸＩＴプロック \
			-score_type $$:スコアタイプ \
			-f $$i:ＶＲクリアフラグ \
			-bgm $$:一位ジングル $$:二位ジングル $$:三位ジングル $$:圏外ジングル
	}

	//ＶＲポーズも一緒に設定
	#ifdef d:STAGE_VR
		chara ＶＲポーズ ＶＲポーズ君 \
			-restart A_ＲＥＳＴＡＲＴプロック \
			-exit    A_ＥＸＩＴプロック
	#else
		chara ＶＲ全体マップ表示３Ｄ ＶＲ全体マップ表示３Ｄ君 \
			-player				$$s:プレイヤ位置ラベル \
			#ifdef d:STAGE_TANKER
				-model			[A_全体マップ:タンカー] \
			#elifdef d:STAGE_PLANT
				#ifdef d:STAGE_W41A || d:STAGE_W42A || d:STAGE_W43A || d:STAGE_W44A || d:STAGE_W45A
					-model		[A_全体マップ:アーセナル] \
				#else
					-model		[A_全体マップ:プラント] \
				#endif
			#endif
			-active $i:A_全体マップ移動可能ステージ \
			-bomb $i:A_全体マップ爆弾設置ステージ \
			-restart A_ＲＥＳＴＡＲＴプロック \
			-exit    A_ＥＸＩＴプロック \
			-vr_pause	// ＶＲポーズ起動フラグ
	#endif
}
// ストリーキング途中ステージではchara ＶＲクリアは使わないので個別に設定
proc ＶＲポーズ設定 {
	chara ＶＲポーズ ＶＲポーズ君 \
		-restart A_ＲＥＳＴＡＲＴプロック \
		-exit    A_ＥＸＩＴプロック
}




/////////////////////////////////
// 全ミッション共通クリアモーション発動
// ※このプロックを呼ぶ際には必ず以下のdefineをすること
/////////////////////////////////
proc 全ミッション共通クリアモーション発動 $:p_発動モーション番号 $:p_初期方向 {
	#if d:DEBUG_PRINT
		print 'stage_clear!!!!! motion_start'
	#endif
	// 装備を外す
	if ( $w:武器 == d:武器:ブレード ) {
		// 刀を持っているときは以下のものを呼ぶ
		command プラグインブレード解除
	}
	$w:武器 = d:武器:素手
	$w:アイテム = d:アイテム:素手

	mesg プレイヤー d:PLAYER stance d:FA_END_STAND $:p_初期方向 $:p_初期方向 ( d:FA_NO_WEAPON | d:FA_GRAVITY_ON )
	command	強制モーション終了プロック -proc 立ち終了時処理 -args $:p_発動モーション番号 $:p_初期方向

	// レーダー系を全てオフにする
	command メニュー設定 -menu off -gage off -radar off -subwin off -radio off -pause off

	mesg ＶＲクリア ＶＲクリア君 ＦＯＣＵＳ
}

proc 立ち終了時処理 {
	if ( $1 == d:FA_PROCMODE_STANCE ) {
		mesg プレイヤー d:PLAYER motion $2 $3 -1 0 ( d:FA_NO_WEAPON | d:FA_GRAVITY_ON )
		command	強制モーション終了プロック -proc モーション終了時処理
	}
//	テイルズＥの対ゴルルゴン戦のときのみ例外処理
#ifdef d:STAGE_SP03A && d:E_TALES
	@スネークテイルズＥ用対ゴルルゴン戦終了処理
#endif
}

proc モーション終了時処理 {
	#ifdef d:STAGE_SP08A
		command テンプライト カメラフラッシュ -p 56833,1822,-34148 -r 2000000 -e 2000000 -c 200,200,200 -f 0x0300
	#elifdef d:STAGE_SP24A
		command テンプライト カメラフラッシュ -p 5396,1499,2899 -r 2000000 -e 2000000 -c 200,200,200 -f 0x0300
	#elifdef d:STAGE_SP03A
		if ( $b:ステージ番号 == d:ZAKO_STAGE:MECHGENOLA ) {
			command テンプライト カメラフラッシュ -p -8,7820,14531 -r 2000000 -e 2000000 -c 200,200,200 -f 0x0300
		}
	#endif

	mesg ＶＲクリア ＶＲクリア君 ＯＮ
}

proc 各ミッション共通モーションなしクリア処理 {
	#if d:DEBUG_PRINT
		print 'stage_clear!!!!! motion_start'
	#endif
	// レーダー系を全てオフにする
	command メニュー設定 -menu off -gage off -radar off -subwin off -radio off -pause off

	mesg ＶＲクリア ＶＲクリア君 ＦＯＣＵＳ
	chara delay ディレイ -time 85 -exec{ mesg ＶＲクリア ＶＲクリア君 ＯＮ }
}



/////////////////////////////////
// 各ミッション共通tryagain用プロック
/////////////////////////////////
// ポーズ中のRESTART時に呼ばれるプロック
proc A_ＲＥＳＴＡＲＴプロック {
	#if d:DEBUG_PRINT
		print 'stage_restart!!!!!'
	#endif

	switch ( $b:ミッション番号 ) {
		case ( d:MISSION:爆弾解体 ) {
			// 一回でもロードをかけてリスタートしたら、タイムを加算する
			if ( $f:爆弾解体ミッションＶＲウィンドウ表示フラグ == 0 ) {
				$i:爆弾解体タイマー残り時間 = `%ＶＲゲットタイム`;
				command varsave $i:爆弾解体タイマー残り時間
			}
		}
		case ( d:MISSION:バラエティ ) {
			#ifdef d:STAGE_SP03A
			// ザコサバイバルはリスタート時最初のテロップから開始させるため、変数を初期値に戻す
				$i:プレイヤー初期Ｘ位置 = 0;
				$i:プレイヤー初期Ｙ位置 = 0;
				$i:プレイヤー初期Ｚ位置 = 2000;
				$i:プレイヤー初期方向 = 2048 ;
				$f:ロードチェックＯＮフラグ = 0;
				$f:sp03a_ザコサバイバルゲームフラグ = 0;
				$b:ステージ番号 = 0;
				$f:A_一括初期化拒否フラグ = 0;
				command varsave
			#endif
		}
		case ( d:MISSION:ストリーキング ) {
			#ifdef d:STAGE_ST01A
				$i:A_ストリーキングミッションタイム = `%ＶＲゲットタイム`;
				command varsave $i:A_ストリーキングミッションタイム
			#endif
		}
		default {
		}
	}

}

// クリアした後のRETRY時に呼ばれるプロック(ロードするミッションはここを通らないので注意)
proc A_ＲＥＴＲＹプロック {
	#if d:DEBUG_PRINT
		print 'stage_try_again!!!!!'
	#endif

	#ifdef d:STAGE_SP03A
	// ザコサバイバルはリスタート時最初のテロップから開始させるため、変数を初期値に戻す
		$i:プレイヤー初期Ｘ位置 = 0;
		$i:プレイヤー初期Ｙ位置 = 0;
		$i:プレイヤー初期Ｚ位置 = 2000;
		$i:プレイヤー初期方向 = 2048 ;
		$f:ロードチェックＯＮフラグ = 0;
		$f:sp03a_ザコサバイバルゲームフラグ = 0;
		$b:ステージ番号 = 0;
		$f:A_一括初期化拒否フラグ = 0;
		command varsave
	#endif
}

// ミッションの最終面のnext stageとして呼ぶプロック（実際には何も出ないが気持ち悪いので一応用意）
proc A_ミッションエンド {
	#if d:DEBUG_PRINT
		print 'mission_end!!!!!'
	#endif
}


/////////////////////////////////
// 各ミッション共通exit用プロック
/////////////////////////////////
proc A_ＥＸＩＴプロック {
	#if d:DEBUG_PRINT
		print 'stage_exit!!!!!'
	#endif

	@load_missions_select
}



/////////////////////////////////
// プレイヤー設定
/////////////////////////////////
proc A_プレイヤー設定 $:p_スネークファイル $:p_ライデンファイル {
	#if d:DEBUG_PRINT
		print 'player_set'
	#endif

	if ( $b:ミッション番号 == d:MISSION:主観モード ) {
		switch ( $b:主観モード番号 ) {
			case ( 1 ) {
				// キーコンフィグで$b:主観モード番号を1に設定したとき
				command 主観移動セット -mode 1 // ヘイロー
			}
			#ifdef d:XBOX
			case ( 3 ) {
				// キーコンフィグで$b:主観モード番号を3に設定したとき(これはＸＢＯＸ版のみ)
				command 主観移動セット -mode 3
			}
			#endif
			default {
				// キーコンフィグで$b:主観モード番号を2に設定したときか、初期値が入っているとき
				command 主観移動セット -mode 2
			}
		}
	} else {
		// それ以外のミッションの場合は主観移動モードを解除する。
		command 主観移動セット -mode 0
	}

	if ( $s:選択プレイヤー == スネーク || $s:選択プレイヤー == タキシードスネーク || $s:選択プレイヤー == 前作スネーク ) {

		command プレイヤー描画モードセット -latter

		#ifdef d:STAGE_W00B
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s 0 \
				-homing $i:w00b_ホーミング距離 512 \
				-f (d:PLY_SPLASH | d:PLY_NO_FOOT_SHADOW | d:PLY_IS_SNAKE) \
				-m sna_def -a snake -o $:p_スネークファイル

		#elseifdef d:STAGE_W04C || STAGE_W04B
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s $b:プレイヤー初期状態 \
				-f d:PLY_IS_SNAKE \
				-m sna_def -a snake -o $:p_スネークファイル
		#else
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s 0 \
				-f d:PLY_IS_SNAKE \
				-m sna_def -a snake -o $:p_スネークファイル
		#endif

		if ( $s:選択プレイヤー == スネーク || $s:選択プレイヤー == 前作スネーク ) {
			chara ロープモデル３ バンダナ１ \
				-n sna_bdn1 \
				-d 1 \
				-y d:PLAYER \
				-z 12 \
				-x 0,60,-90 \
				-m 12 \
				-p 250 \
				-k 350 \
				-o 110 \
				-f 1

			chara ロープモデル３ バンダナ２ \
				-n sna_bdn2 \
				-d 1 \
				-y d:PLAYER \
				-z 12 \
				-x 0,50,-90 \
				-m 22 \
				-p 250 \
				-k 350 \
				-o 110 \
				-f 1
		}

		if ( $s:選択プレイヤー == タキシードスネーク ) {
			chara マルチウェイト髪の毛モデル マルチウェイトタキシードの裾 \
				-n sna_txd_suso_mh_mt \
				-d 0 \
				-b sna_txd_bounding \
				-m 100 \
				-a 300 \
				-k 1000 \
				-y d:PLAYER \
				-z 0 1 2 13 17 \
				-x 0,0,0 \
				-o 100 \
				-q 0 \
				-f 1 \
				-c 5 { 0 1 2 13 17 } \
				-w 100
		}

		chara ＬＯＤ制御 プレイヤーＬＯＤ \
			-name d:PLAYER \
			-type 2 \
			-model sna_def_sh \
			-value d:TRP_STATE_BEHIND \
			-value2 7000

	} else if ( $s:選択プレイヤー == プリスキン  ) {
		command プレイヤー描画モードセット -latter

		#ifdef d:STAGE_W00B
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s 0 \
				-homing $i:w00b_ホーミング距離 512 \
				-f (d:PLY_SPLASH | d:PLY_NO_FOOT_SHADOW | d:PLY_IS_SNAKE | d:PLY_IS_PLISKIN ) \
				-m sna_def -a snake -o $:p_スネークファイル

		#elseifdef d:STAGE_W04C || STAGE_W04B
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s $b:プレイヤー初期状態 \
				-f ( d:PLY_IS_SNAKE | d:PLY_IS_PLISKIN ) \
				-m sna_def -a snake -o $:p_スネークファイル
		#else
			//	プレイヤー初期状態見るフラグが立っているときはプレイヤー初期状態を見る
			if ($$f:プレイヤー初期状態見るフラグ == 1) {
				chara プレイヤー d:PLAYER \
					-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
					-d 0,$i:プレイヤー初期方向,0 \
					-s $b:プレイヤー初期状態 \
					-f ( d:PLY_IS_SNAKE | d:PLY_IS_PLISKIN ) \
					-m sna_def -a snake -o $:p_スネークファイル
			} else {
				chara プレイヤー d:PLAYER \
					-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
					-d 0,$i:プレイヤー初期方向,0 \
					-s 0 \
					-f ( d:PLY_IS_SNAKE | d:PLY_IS_PLISKIN ) \
					-m sna_def -a snake -o $:p_スネークファイル
			}
		#endif

		chara ＬＯＤ制御 プレイヤーＬＯＤ \
			-name d:PLAYER \
			-type 2 \
			-model sna_def_sh \
			-value d:TRP_STATE_BEHIND \
			-value2 7000

	} else if ( $s:選択プレイヤー == ライデン ) {
		#ifdef d:STAGE_W00B
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s 0 \
				-homing $i:w00b_ホーミング距離 512 \
				-f (d:PLY_SPLASH | d:PLY_NO_FOOT_SHADOW | d:PLY_IS_RAIDEN) \
				-m rai_def -a raiden -o $:p_ライデンファイル

		#elseifdef d:STAGE_W04C || STAGE_W04B
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s $b:プレイヤー初期状態 \
				-f d:PLY_IS_RAIDEN \
				-m rai_def -a raiden -o $:p_ライデンファイル

		#else
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s 0 \
				-f d:PLY_IS_RAIDEN \
				-m rai_def -a raiden -o $:p_ライデンファイル
		#endif

		@ライデン髪の毛設定 d:NORMAL_HAIR

		chara ＬＯＤ制御 プレイヤーＬＯＤ \
			-name d:PLAYER \
			-type 2 \
			-model rai_def_sh_mt \
			-value d:TRP_STATE_BEHIND \
			-value2 7000

	} else if ( $s:選択プレイヤー == 刀ライデン ) {
		command プラグインブレード -m rai_blade

		#ifdef d:STAGE_W00B
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s 0 \
				-homing $i:w00b_ホーミング距離 512 \
				-f (d:PLY_SPLASH | d:PLY_NO_FOOT_SHADOW | d:PLY_IS_RAIDEN | d:PLY_IS_NINJA ) \
				-m rai_def -a raiden -o $:p_ライデンファイル

		#elseifdef d:STAGE_W04C || STAGE_W04B
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s $b:プレイヤー初期状態 \
				-f ( d:PLY_IS_RAIDEN | d:PLY_IS_NINJA ) \
				-m rai_def -a raiden -o $:p_ライデンファイル

		#else
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s 0 \
				-f ( d:PLY_IS_RAIDEN | d:PLY_IS_NINJA ) \
				-m rai_def -a raiden -o $:p_ライデンファイル

		#endif

		chara ＬＯＤ制御 プレイヤーＬＯＤ \
			-name d:PLAYER \
			-type 2 \
			-model rai_def_sh_mt \
			-value d:TRP_STATE_BEHIND \
			-value2 7000

	} else if ( $s:選択プレイヤー == 裸ライデン ) {

		command はだかモードセット

		chara プレイヤー d:PLAYER \
			-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
			-d 0,$i:プレイヤー初期方向,0 \
			-s 0 \
			-f d:PLY_IS_RAIDEN \
			-m rai_naked -a raiden -o $:p_ライデンファイル

		@ライデン髪の毛設定 d:NORMAL_HAIR

		chara ＬＯＤ制御 プレイヤーＬＯＤ \
			-name d:PLAYER \
			-type 2 \
			-model rai_naked_sh \
			-value d:TRP_STATE_BEHIND \
			-value2 7000

	} else {
		#if d:DEBUG_PRINT
			print 'warning no_player_set!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'
		#endif

		command assert ( d:FALSE ) 'player no error  (scenario err) A_プレイヤー設定'
	}


	if ( $b:ミッション番号 != d:MISSION:爆弾解体 && $b:ミッション番号 != d:MISSION:スネークテイルズ && $b:ミッション番号 != d:MISSION:主観モード ) {
		#ifndef d:STAGE_SP03A
			command プレイヤー消去キャラ
		#endif
	}

	#ifdef d:STAGE_W31C
		chara Ｏ２ゲージ Ｏ２ -max 3500 -length d:O2_GAGE

	#elifdef d:STAGE_W61A
		chara Ｏ２ゲージ Ｏ２ -max 3500 -length d:O2_GAGE

	#endif
}

// プレイヤーチェック
proc スネーク達 {
	if( $s:選択プレイヤー == スネーク || \
		$s:選択プレイヤー == プリスキン || \
		$s:選択プレイヤー == タキシードスネーク || \
		$s:選択プレイヤー == 前作スネーク ) {

		return 1;
	} else {
		return 0;
	}
}

// 簡易武器設定
proc A_装備武器 $:p_武器名 {
	#if d:DEBUG_PRINT
		print '装備武器 武器番号は' ($:p_武器名)
	#endif
	eval ($w:武器 = $:p_武器名);
}

proc A_弾数設定 $:p_武器名 $:p_弾数 {
	#if d:DEBUG_PRINT
		local $$:chk;
		$$:chk = d:FALSE;
		print '武器名 武器番号は' ($:p_武器名)
		print '弾数は' ($:p_弾数) '発'
	#endif

	switch($:p_武器名){
		case(d:アイテム:ＵＳＰサプレッサ){
			// 強制サプレッサＵＳＰ化
			eval ( $w:プレイヤーフラグ = ( $w:プレイヤーフラグ|d:PL_USP_SPPRSR_ATTACHED ) );
			eval ( $w:武器弾数Ｒ[ d:武器:ＵＳＰ ] =$:p_弾数 );
			#if d:DEBUG_PRINT
				$$:chk = d:TRUE;
			#endif

		}
		case(d:アイテム:ソコムサプレッサ){
			// 強制サプレッサソコム化
			eval ( $w:プレイヤーフラグ = ( $w:プレイヤーフラグ|d:PL_SOCOM_SPPRSR_ATTACHED ) );
			eval ( $w:武器弾数Ｒ[ d:武器:ソコム ] = $:p_弾数 );
			#if d:DEBUG_PRINT
				$$:chk = d:TRUE;
			#endif
		}
		case(d:アイテム:ＡＫサプレッサ){
			// 強制サプレッサＡＫＳ化
			eval ( $w:プレイヤーフラグ = ( $w:プレイヤーフラグ|d:PL_AK_SPPRSR_ATTACHED ) );
			eval ( $w:武器弾数Ｒ[ d:武器:ＡＫＳ ] = $:p_弾数 );
			#if d:DEBUG_PRINT
				$$:chk = d:TRUE;
			#endif
		}
		default{ // 通常武器のばあい。
			eval ( $w:武器弾数Ｒ[ $:p_武器名 ] = $:p_弾数 );
			#if d:DEBUG_PRINT
				if($:p_武器名 <= d:武器:雑誌){
					$$:chk = d:TRUE;
				}
			#endif
		}
	}
	#if d:DEBUG_PRINT
		command assert ( $$:chk ) 'amo & wep setting error!!!! (scn)指定された武器がおかしいっぽ'
	#endif
}

proc A_装備アイテム $:p_アイテム名 {
	eval ( $w:アイテム = $:p_アイテム名 );
}

proc A_アイテム数設定 $:p_アイテム名 $:p_アイテム数
{
	eval ( $w:アイテム数Ｒ[ $:p_アイテム名 ] = $:p_アイテム数 );
}


/////////////////////////////////
// 敵兵設定
/////////////////////////////////
// 通常の警備兵
proc A_警備兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ {
	#if d:DEBUG_PRINT
		print 'watcher_set'
	#endif

	chara 警備兵 $:p_名前 \
		-r $:p_ルート -n $:p_ノード -d $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s $:p_ステータス
}

proc A_死亡判定付き警備兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ $:p_死亡プロック {
	#if d:DEBUG_PRINT
		print 'watcher_set'
	#endif

	chara 警備兵 $:p_名前 \
		-r $:p_ルート -n $:p_ノード -d $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s $:p_ステータス \
		-e $:p_死亡プロック
}

// ＶＲの警備兵
proc VR_警備兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ {
	#if d:DEBUG_PRINT
		print 'watcher_set'
	#endif

	chara 警備兵 $:p_名前 \
		-r $:p_ルート -n $:p_ノード -d $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s $:p_ステータス \
		#ifdef d:STAGE_SP06A
			-e 敵兵死亡プロック $:p_名前 \
		#endif
		-t d:ENE_STST_VRBODY
}

// 拡張ステータスか必要な警備兵
proc VR_拡張警備兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス $:p_拡張ステータス $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ {
	#if d:DEBUG_PRINT
		print 'watcher_set'
	#endif

	chara 警備兵 $:p_名前 \
		-r $:p_ルート -n $:p_ノード -d $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s $:p_ステータス \
		#ifdef d:STAGE_SP06A
			-e 敵兵死亡プロック $:p_名前 \
		#endif
		-t ( d:ENE_STST_VRBODY | $:p_拡張ステータス )
}

// スニーキングミッション用警備兵(SNEAKING/ELIMINATE_ALL 一応両対応)2002/05/30 T.Ohtani
// 危険モードが無いので警備ポイントは常に0 0 0
proc VR_スニーキング警備兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス $:p_守備マップ {
	// 拡張ステータス設定
	if( $b:モード番号 == d:SNEAKING:SNEAKING && $b:ミッション番号 == d:MISSION:スニーキング){
		@VR_拡張警備兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス d:ENE_STATUS_NORMAL 0 0 0 $:p_守備マップ
	}else if( $b:モード番号 == d:SNEAKING:ELIMINATE_ALL && $b:ミッション番号 == d:MISSION:スニーキング ){
		@VR_拡張警備兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス d:ENE_STST_FAINT_EXIT 0 0 0 $:p_守備マップ
	}else{
		#ifdef d:DEBUG_PRINT
			print '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!$b:ミッション番号' $b:ミッション番号
			print '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!$b:モード番号' $b:モード番号
		#endif
		command assert(d:FALSE) 'mission_Number or mode_Number ERROR!!!!! VR_スニーキング警備兵はここでは使われないはず'
	}
}

/* 来る日のために一応。2002/05/30 T.Ohtani
// スニーキングミッション用警備兵すーぱー(SNEAKING/ELIMINATE_ALL 完全両対応)
// ルート、ノード、ステータスを２つ渡せるようになってます。
proc VR_スニーキング両対応警備兵 $:p_名前	$:p_ルート１ $:p_ノード１ $:p_ステータス１ \
											$:p_ルート２ $:p_ノード２ $:p_ステータス２ $:p_守備マップ {
	// 拡張ステータス設定
	if($b:モード番号 == d:SNEAKING:SNEAKING && $b:ミッション番号 == d:MISSION:スニーキング ){
		@VR_拡張警備兵 $:p_名前 $:p_ルート１ $:p_ノード１ $:p_ステータス１ d:ENE_STATUS_NORMAL 0 0 0 $:p_守備マップ
	}else if($b:モード番号 == d:SNEAKING:ELIMINATE_ALL && $b:ミッション番号 == d:MISSION:スニーキング){
		@VR_拡張警備兵 $:p_名前 $:p_ルート２ $:p_ノード２ $:p_ステータス２ d:ENE_STST_FAINT_EXIT 0 0 0 $:p_守備マップ
	}else{
		#ifdef d:DEBUG_PRINT
			print '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!$b:ミッション番号' $b:ミッション番号
			print '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!$b:モード番号' $b:モード番号
		#endif
		command assert(d:FALSE) 'mission_Number or mode_Number ERROR!!!!! VR_スニーキング警備兵はここでは使われないはず'
	}
}
*/

// アイテムを出す警備兵
proc A_アイテム警備兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ {
	#if d:DEBUG_PRINT
		print 'item_watcher_set'
	#endif

	chara 警備兵 $:p_名前 \
		-r $:p_ルート -n $:p_ノード -d $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s $:p_ステータス \
		-q 引きずり１ $b:アイテム率１ 引きずり２ $b:アイテム率２ 引きずり３ $b:アイテム率３ \
		-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３
}

// 普通の攻撃兵
proc A_攻撃兵 $:p_名前 $:p_ルート $:p_ステータス $:p_装備 {
	#if d:DEBUG_PRINT
		print 'attack_set'
	#endif

	chara アタッカー $:p_名前 -r $:p_ルート -s $:p_ステータス -e $:p_装備 \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 -l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶
}

// 探索ルート付きの攻撃兵
proc A_探索ルート付き攻撃兵 $:p_名前 $:p_ルート $:p_探索ルート $:p_ステータス $:p_装備 {
	#if d:DEBUG_PRINT
		print 'attack_set'
	#endif

	chara アタッカー $:p_名前 -r $:p_ルート -s $:p_ステータス -e $:p_装備 \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 -l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-c $:p_探索ルート
}

// 普通の攻撃兵
proc A_拡張攻撃兵 $:p_名前 $:p_ルート $:p_ステータス $:p_拡張ステータス $:p_装備 {
	#if d:DEBUG_PRINT
		print 'attack_set'
	#endif

	chara アタッカー $:p_名前 -r $:p_ルート -s $:p_ステータス -t $:p_拡張ステータス -e $:p_装備 \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 -l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶
}


// ＶＲ用普通の攻撃兵
proc VR_攻撃兵 $:p_名前 $:p_ルート $:p_ステータス $:p_装備 {
	#if d:DEBUG_PRINT
		print 'attack_set'
	#endif

	chara アタッカー $:p_名前 -r $:p_ルート -s $:p_ステータス -e $:p_装備 \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 -l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 -t d:ENE_STST_VRBODY
}

// 探索モード時に独自のルートで探索を行う攻撃兵
proc A_探索攻撃兵 $:p_名前 $:p_ルート $:p_ステータス $:p_装備 $:p_探索ルート {
	#if d:DEBUG_PRINT
		print 'attack_set'
	#endif

	chara アタッカー $:p_名前 -r $:p_ルート -s $:p_ステータス -e $:p_装備 -c $:p_探索ルート \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 -l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶
}

// ＶＲ用探索モード時に独自のルートで探索を行う攻撃兵
proc VR_探索攻撃兵 $:p_名前 $:p_ルート $:p_ステータス $:p_装備 $:p_探索ルート {
	#if d:DEBUG_PRINT
		print 'attack_set'
	#endif

	chara アタッカー $:p_名前 -r $:p_ルート -s $:p_ステータス -e $:p_装備 -c $:p_探索ルート \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 -l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 -t d:ENE_STST_VRBODY
}
//	VRミッション用警備攻撃兵
//	危険になると攻撃兵に転向。$:p_ステータスにd:d:ENE_STATUS_CONVERTは立っている
proc VR_警備攻撃兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス $:p_拡張ステータス $:p_装備 {
	#if d:DEBUG_PRINT
		print 'attacker_set'
	#endif

	chara アタッカー $:p_名前 \
		-r $:p_ルート -n $:p_ノード \
		-s (d:ENE_STATUS_CONVERT | $:p_ステータス) \
		-t (d:ENE_STST_VRBODY    | $:p_拡張ステータス) \
		-e $:p_装備 \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶
}

// 探索モード時に他の敵兵にくっついて探索を行う攻撃兵
proc A_サポート攻撃兵 $:p_名前 $:p_ルート $:p_ステータス $:p_装備 $:p_サポートする敵兵名 {
	#if d:DEBUG_PRINT
		print 'attack_set'
	#endif

	chara アタッカー $:p_名前 -r $:p_ルート -s ($:p_ステータス | d:ENE_STATUS_CONVERT2) -e $:p_装備 -b $:p_サポートする敵兵名 \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 -l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶
}

// ＶＲ用探索モード時に他の敵兵にくっついて探索を行う攻撃兵
proc VR_サポート攻撃兵 $:p_名前 $:p_ルート $:p_ステータス $:p_装備 $:p_サポートする敵兵名 {
	#if d:DEBUG_PRINT
		print 'attack_set'
	#endif

	chara アタッカー $:p_名前 -r $:p_ルート -s ($:p_ステータス | d:ENE_STATUS_CONVERT2) -e $:p_装備 -b $:p_サポートする敵兵名 \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 -l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 -t d:ENE_STST_VRBODY
}

//	監視カメラ
proc A_監視カメラ $:p_名前 $:p_場所Ｘ $:p_場所Ｙ $:p_場所Ｚ $:p_取付縦方向 $:p_取付横方向 $:p_カメラ振り中心角 $:p_振り幅 $:p_フラグ {
	#if d:DEBUG_PRINT
		print 'svc_camera_set'
	#endif

	#ifdef d:STAGE_TANKER
		chara 監視カメラ $:p_名前 \
			-pos $:p_場所Ｘ $:p_場所Ｙ $:p_場所Ｚ \
			-rot $:p_取付縦方向 $:p_取付横方向 0 \ /*ＸＹ値のみ有効*/
			-center $:p_カメラ振り中心角 $i:監視カメラ初期方向 \
			-dir $:p_振り幅 \
			-xran $i:監視カメラ縦視野角 \
			-yran $i:監視カメラ横視野角 \
			-i $w:監視カメラ通常視力 $w:監視カメラ危険視力 $w:監視カメラ警戒視力 $w:監視カメラ回避視力 \
			-status $:p_フラグ \
			-wait $i:監視カメラ首振り待ち時間 \
			-alertdir ($:p_振り幅 + $i:監視カメラ緊急振り幅) \
			-exec {
				@監視カメラ破壊時処理 $1 $2
			}
	#else
		chara プラントカメラ $:p_名前 \
			-pos $:p_場所Ｘ $:p_場所Ｙ $:p_場所Ｚ \
			-rot $:p_取付縦方向 $:p_取付横方向 0 \ /*ＸＹ値のみ有効*/
			-center $:p_カメラ振り中心角 $i:監視カメラ初期方向 \
			-dir $:p_振り幅 \
			-xran $i:監視カメラ縦視野角 \
			-yran $i:監視カメラ横視野角 \
			-i $w:監視カメラ通常視力 $w:監視カメラ危険視力 $w:監視カメラ警戒視力 $w:監視カメラ回避視力 \
			-status $:p_フラグ \
			-wait $i:監視カメラ首振り待ち時間 \
			-alertdir ($:p_振り幅 + $i:監視カメラ緊急振り幅) \
			-exec {
				@監視カメラ破壊時処理 $1 $2
			}
	#endif
}

//	ガンカメラ
proc A_ガンカメラ $:p_名前 $:p_場所Ｘ $:p_場所Ｙ $:p_場所Ｚ $:p_取付縦方向 $:p_取付横方向 $:p_カメラ振り中心角 $:p_振り幅 $:p_フラグ {
	#if d:DEBUG_PRINT
		print 'svc_camera_set'
	#endif

	chara ガンカメラ $:p_名前 \
		-pos $:p_場所Ｘ $:p_場所Ｙ $:p_場所Ｚ \
		-rot $:p_取付縦方向 $:p_取付横方向 0 \ /*ＸＹ値のみ有効*/
		-center $:p_カメラ振り中心角 $i:監視カメラ初期方向 \
		-dir $:p_振り幅 \
		-xran $i:監視カメラ縦視野角 \
		-yran $i:監視カメラ横視野角 \
		-i $w:監視カメラ通常視力 $w:監視カメラ危険視力 $w:監視カメラ警戒視力 $w:監視カメラ回避視力 \
		-status $:p_フラグ \
		-alertdir $i:ガンカメラ緊急振り幅 \ /*片側角度*/
		-niktime $i:ガンカメラニキータ破壊時間 \
		-exec {
			@監視カメラ破壊時処理 $1 $2
		}
}

//	サイファ
proc A_サイファ $:p_名前 $:p_ルート $:p_ノード $:p_視力 $:p_帰宅Ｘ１ $:p_帰宅１Ｙ１ $:p_帰宅Ｚ１ $:p_帰宅Ｘ２ $:p_帰宅Ｙ２ $:p_帰宅Ｚ２ $:p_速度 {
	#if d:DEBUG_PRINT
		print 'cypher_set'
	#endif

	chara サイファ $:p_名前 -r $:p_ルート $:p_ノード -i $:p_視力 -s $:p_速度 \
		-h $:p_帰宅Ｘ１ $:p_帰宅１Ｙ１ $:p_帰宅Ｚ１ $:p_帰宅Ｘ２ $:p_帰宅Ｙ２ $:p_帰宅Ｚ２ \
		-t 0 -p サイファー破壊
}

//	ガンサイファ
proc A_ガンサイファ $:p_名前 $:p_ルート $:p_ノード $:p_視力 $:p_速度 $:p_危険速度 {
	#if d:DEBUG_PRINT
		print 'guncypher_set'
	#endif

	chara サイファ $:p_名前 -r $:p_ルート $:p_ノード -i $:p_視力 -s $:p_速度 -k $:p_危険速度 \
		-t 1 -skipflag 1 -p サイファー破壊
}

//	危険モード時に再発生するガンサイファ（再発生数、破壊プロックはコマンダーに登録する必要があるので注意！）
proc A_危険再発生ガンサイファ $:p_名前 $:p_ルート $:p_ノード $:p_潜入視力 $:p_危険視力 $:p_警戒視力 $:p_回避視力 \
							$:p_速度 $:p_危険速度 $:p_通常加速力 $:p_危険加速力 $:p_待機ルート $:p_中継ルート $:p_追跡ルート {
	#if d:DEBUG_PRINT
		print 'guncypher_set'
	#endif

	chara サイファ $:p_名前 -r $:p_ルート $:p_ノード -i $:p_潜入視力 $:p_危険視力 $:p_警戒視力 $:p_回避視力 \
		-s $:p_速度 -k $:p_危険速度 -fkasoku $:p_通常加速力 $:p_危険加速力 -zidouset $:p_待機ルート $:p_中継ルート $:p_追跡ルート \
		-t 1 -skipflag 1
}

//	天狗兵Ａ
proc A_天狗兵Ａ $:p_名前 $:p_ルート $:p_敵潜入視力 $:p_敵危険視力 {
	#if d:DEBUG_PRINT
		print 'teng_set'
	#endif

	chara 天狗兵Ａ $:p_名前 -r $:p_ルート -i $:p_敵潜入視力,$:p_敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s 0
}

//	守備する天狗兵Ａ
proc A_守備天狗兵Ａ $:p_名前 $:p_ルート $:p_敵潜入視力 $:p_敵危険視力 $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ {
	#if d:DEBUG_PRINT
		print 'teng_set'
	#endif

	chara 天狗兵Ａ $:p_名前 -r $:p_ルート -i $:p_敵潜入視力,$:p_敵危険視力,$w:敵回避視力 \
		-d $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s 0
}



/////////////////////////////////
// アナザーミッション共通用
/////////////////////////////////

//	進入禁止壁

//	これは古い
#if 0
proc アナザー用進入禁止壁 $:p_ドア名 $:p_ドア位置Ｘ $:p_ドア位置Ｙ $:p_ドア位置Ｚ $:p_ドア回転 $:p_ドアスライド幅 {
	eval ($$s:進入禁止壁 = $:p_ドア名 + 1) 	//	今更引数を増やしたくないのでこうします。
	chara 透明壁 $$s:進入禁止壁 \
		-position	$:p_ドア位置Ｘ $:p_ドア位置Ｙ $:p_ドア位置Ｚ 2500 \
					($:p_ドア位置Ｘ+ ($:p_ドアスライド幅+100)) $:p_ドア位置Ｙ $:p_ドア位置Ｚ 2500 \
		-attribute	( d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_PLAYER ))
	mesg 透明壁 $$s:進入禁止壁 rotate 1 $:p_ドア回転
}
#endif

//	こっちを使用する
proc アナザー用進入禁止壁 $:p_ドア名 $:p_ドア位置Ｘ $:p_ドア位置Ｙ $:p_ドア位置Ｚ $:p_ドア回転 $:p_ドアスライド幅 {
 	//	今更引数を増やしたくないのでこうします。
	$$b:ステージドア総数 = $$b:ステージドア総数 + 1;

	if ($$b:ステージドア総数 == 1) {
		$$s:進入禁止壁 = 進入禁止壁Ａ;
	} else if ($$b:ステージドア総数 == 2) {
		$$s:進入禁止壁 = 進入禁止壁Ｂ;
	} else if ($$b:ステージドア総数 == 3) {
		$$s:進入禁止壁 = 進入禁止壁Ｃ;
	} else if ($$b:ステージドア総数 == 4) {
		$$s:進入禁止壁 = 進入禁止壁Ｄ;
	} else if ($$b:ステージドア総数 == 5) {
		$$s:進入禁止壁 = 進入禁止壁Ｅ;
	} else if ($$b:ステージドア総数 == 6) {
		$$s:進入禁止壁 = 進入禁止壁Ｆ;
	} else if ($$b:ステージドア総数 == 7) {
		$$s:進入禁止壁 = 進入禁止壁Ｇ;
	} else if ($$b:ステージドア総数 == 8) {
		$$s:進入禁止壁 = 進入禁止壁Ｈ;
	} else if ($$b:ステージドア総数 == 9) {
		$$s:進入禁止壁 = 進入禁止壁Ｉ;
	} else if ($$b:ステージドア総数 == 10) {
		$$s:進入禁止壁 = 進入禁止壁Ｊ;
	} else if ($$b:ステージドア総数 == 11) {
		$$s:進入禁止壁 = 進入禁止壁Ｋ;
	} else if ($$b:ステージドア総数 == 12) {
		$$s:進入禁止壁 = 進入禁止壁Ｌ;
	} else if ($$b:ステージドア総数 == 13) {
		$$s:進入禁止壁 = 進入禁止壁Ｍ;
	} else if ($$b:ステージドア総数 == 14) {
		$$s:進入禁止壁 = 進入禁止壁Ｎ;
	} else if ($$b:ステージドア総数 == 15) {
		$$s:進入禁止壁 = 進入禁止壁Ｏ;
	} else {
		$$s:進入禁止壁 = 進入禁止壁Ｐ;
	}

/*
	if ($:p_ドア回転 == 0) {
		eval ($$s:進入禁止壁 = 進入禁止壁Ａ)
	} else if ($:p_ドア回転 == 1024) {
		eval ($$s:進入禁止壁 = 進入禁止壁Ｂ)
	} else if ($:p_ドア回転 == 2048) {
		eval ($$s:進入禁止壁 = 進入禁止壁Ｃ)
	} else if ($:p_ドア回転 == 3072) {
		eval ($$s:進入禁止壁 = 進入禁止壁Ｄ)
	} else if ( ($:p_ドア回転 == 721) || ($:p_ドア回転 == 722) ) {
		eval ($$s:進入禁止壁 = 進入禁止壁Ｅ)
	} else if ( ($:p_ドア回転 == 1326) || ($:p_ドア回転 == 1327) ) {
		eval ($$s:進入禁止壁 = 進入禁止壁Ｆ)
	} else if ($:p_ドア回転 == 1745) {
		eval ($$s:進入禁止壁 = 進入禁止壁Ｇ)
	} else if ($:p_ドア回転 == 2769) {
		eval ($$s:進入禁止壁 = 進入禁止壁Ｈ)
	}
*/

	chara 透明壁 $$s:進入禁止壁 \
		-position	$:p_ドア位置Ｘ $:p_ドア位置Ｙ $:p_ドア位置Ｚ 2500 \
					($:p_ドア位置Ｘ+ ($:p_ドアスライド幅+100)) $:p_ドア位置Ｙ $:p_ドア位置Ｚ 2500 \
		-attribute	( d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_PLAYER ))
	mesg 透明壁 $$s:進入禁止壁 rotate 1 $:p_ドア回転
}


/////////////////////////////////
// 敵兵排除ミッション用
/////////////////////////////////

//	敵兵排除ミッション用警備兵
//	$:p_拡張ステータスにd:ENE_STST_FAINT_EXITは立っている
proc A_敵兵排除警備兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス $:p_拡張ステータス $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ {
	#if d:DEBUG_PRINT
		print 'watcher_set'
	#endif

	chara 警備兵 $:p_名前 \
		-r $:p_ルート -n $:p_ノード -d $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s $:p_ステータス \
		-t (d:ENE_STST_FAINT_EXIT | $:p_拡張ステータス)	/* 気絶、居眠りで退場 */
}

//	敵兵排除ミッション用警備攻撃兵
//	$:p_拡張ステータスにd:ENE_STST_FAINT_EXITは立っている
//	危険になると攻撃兵に転向。$:p_ステータスにd:d:ENE_STATUS_CONVERTは立っている
proc A_敵兵排除警備攻撃兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス $:p_拡張ステータス $:p_装備 {
	#if d:DEBUG_PRINT
		print 'attacker_set'
	#endif

	chara アタッカー $:p_名前 \
		-r $:p_ルート -n $:p_ノード \
		-s (d:ENE_STATUS_CONVERT | $:p_ステータス) \
		-t (d:ENE_STST_FAINT_EXIT| $:p_拡張ステータス) \
		-e $:p_装備 \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶
}

//	敵兵排除ミッション用退避指定警備兵
//	$:p_拡張ステータスにd:ENE_STST_FAINT_EXITは立っている
//	退避位置を指定することで必ずその退避ポイントに逃げ込む。
//	退避位置番号は コマンダーに設定した順に０～７まで。
proc A_敵兵排除退避指定警備兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス $:p_拡張ステータス $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ $:p_退避位置番号 {
	#if d:DEBUG_PRINT
		print 'watcher_set'
	#endif
	chara 警備兵 $:p_名前 \
		-r $:p_ルート -n $:p_ノード -d $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s $:p_ステータス \
		-t (d:ENE_STST_FAINT_EXIT | $:p_拡張ステータス)	\ /* 気絶、居眠りで退場 */
		-m $:p_退避位置番号
}
//	敵兵排除ミッション用退避指定警備攻撃兵
//	$:p_拡張ステータスにd:ENE_STST_FAINT_EXITは立っている
//	危険になると攻撃兵に転向。$:p_ステータスにd:d:ENE_STATUS_CONVERTは立っている
//	退避位置を指定することで必ずその退避ポイントに逃げ込む。
//	退避位置番号は コマンダーに設定した順に０～７まで。
proc A_敵兵排除退避指定警備攻撃兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス $:p_拡張ステータス $:p_装備 $:p_退避位置番号 {
	#if d:DEBUG_PRINT
		print 'attacker_set'
	#endif

	chara アタッカー $:p_名前 \
		-r $:p_ルート -n $:p_ノード \
		-s (d:ENE_STATUS_CONVERT | $:p_ステータス) \
		-t (d:ENE_STST_FAINT_EXIT| $:p_拡張ステータス) \
		-e $:p_装備 \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-m $:p_退避位置番号
}

//	敵兵排除ミッション用天狗兵Ａ
//	$:p_拡張ステータスにd:ENE_STST_HOLD_EXITは立っている
proc A_敵兵排除天狗兵Ａ $:p_名前 $:p_ルート $:p_ノード $:p_拡張ステータス {
	#if d:DEBUG_PRINT
		print 'teng_set'
	#endif

	chara 天狗兵Ａ $:p_名前 -r $:p_ルート -n $:p_ノード -i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s 0 \
		-t (d:ENE_STST_FAINT_EXIT | $:p_拡張ステータス)
}

//	敵兵排除ミッション用守備天狗兵Ａ
//	$:p_拡張ステータスにd:ENE_STST_HOLD_EXITは立っている
proc A_敵兵排除守備天狗兵Ａ $:p_名前 $:p_ルート $:p_ノード $:p_拡張ステータス $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ {
	#if d:DEBUG_PRINT
		print 'teng_set'
	#endif

	chara 天狗兵Ａ $:p_名前 -r $:p_ルート -n $:p_ノード -i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-d $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s 0 \
		-t (d:ENE_STST_FAINT_EXIT | $:p_拡張ステータス)
}



/////////////////////////////////
// 武装解除ミッション用
/////////////////////////////////

//	武装解除ミッション用警備兵
//	$:p_拡張ステータスにd:ENE_STST_HOLD_EXIT、d:ENE_STST_DONT_KILLは立っている
proc A_武装解除警備兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス $:p_拡張ステータス $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ {
	#if d:DEBUG_PRINT
		print 'watcher_set'
	#endif

	chara 警備兵 $:p_名前 \
		-r $:p_ルート -n $:p_ノード -d $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ $:p_守備マップ \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s $:p_ステータス \
		-t (d:ENE_STST_HOLD_EXIT | d:ENE_STST_DONT_KILL | $:p_拡張ステータス)	/* ホールドアップで退場 */	/* 殺すとゲームオーバー */
}

//	武装解除ミッション用探索攻撃兵
//	探索モード時に独自のルートで探索を行う攻撃兵
//	$:p_装備にd:ENE_EQUIP_TYPE_HITECH_1は立っている
//	$:p_拡張ステータスにd:ENE_STST_DONT_KILLは立っている
proc A_武装解除探索攻撃兵 $:p_名前 $:p_ルート $:p_ノード $:p_ステータス $:p_拡張ステータス $:p_装備 $:p_探索ルート {
	#if d:DEBUG_PRINT
		print 'attacker_set'
	#endif

	chara アタッカー $:p_名前 \
		-r $:p_ルート -n $:p_ノード \
		-s $:p_ステータス \
		-t (d:ENE_STST_DONT_KILL | $:p_拡張ステータス) \	/* 殺すとゲームオーバー */
		-e (d:ENE_EQUIP_TYPE_HITECH_1 |$:p_装備) \
		-c $:p_探索ルート \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶
}

//	武装解除ミッション用サポート攻撃兵
// 探索モード時に他の敵兵にくっついて探索を行う攻撃兵
//	$:p_ステータスにd:ENE_STATUS_CONVERT2は立っている
//	$:p_装備にd:ENE_EQUIP_TYPE_HITECH_1は立っている
//	$:p_拡張ステータスにd:ENE_STST_DONT_KILLは立っている
proc A_武装解除サポート攻撃兵 $:p_名前 $:p_ルート $:p_ステータス $:p_拡張ステータス $:p_装備 $:p_サポートする敵兵名 {
	#if d:DEBUG_PRINT
		print 'attack_set'
	#endif

	chara アタッカー $:p_名前 \
		-r $:p_ルート \
		-s (d:ENE_STATUS_CONVERT2 | $:p_ステータス)\
		-t (d:ENE_STST_DONT_KILL | $:p_拡張ステータス) \	/* 殺すとゲームオーバー */
		-e (d:ENE_EQUIP_TYPE_HITECH_1 |$:p_装備) \
		-b $:p_サポートする敵兵名 \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶
}

//	武装解除ミッション用天狗兵Ａ
//	2002.06.05 大谷 変更 → 武装解除天狗兵は初期ノードを設定できます。
//	$:p_拡張ステータスにd:ENE_STST_HOLD_EXIT、d:ENE_STST_DONT_KILLは立っている
proc A_武装解除天狗兵Ａ $:p_名前 $:p_ルート $:p_ノード $:p_拡張ステータス $:p_敵潜入視力 $:p_敵危険視力 {
	#if d:DEBUG_PRINT
		print 'teng_set'
	#endif

	chara 天狗兵Ａ $:p_名前 -r $:p_ルート -n $:p_ノード -i $:p_敵潜入視力,$:p_敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s 0 \
		-t (d:ENE_STST_HOLD_EXIT | d:ENE_STST_DONT_KILL | $:p_拡張ステータス)	/* ホールドアップで退場 */	/* 殺すとゲームオーバー */

}

//	武装解除ミッション用守備天狗兵Ａ
//	2002.06.05 大谷 変更 → 武装解除天狗兵は初期ノードを設定できます。
//	$:p_拡張ステータスにd:ENE_STST_HOLD_EXIT、d:ENE_STST_DONT_KILLは立っている
proc A_武装解除守備天狗兵Ａ $:p_名前 $:p_ルート $:p_ノード $:p_拡張ステータス $:p_敵潜入視力 $:p_敵危険視力 $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ {
	#if d:DEBUG_PRINT
		print 'teng_set'
	#endif

	chara 天狗兵Ａ $:p_名前 -r $:p_ルート -n $:p_ノード -i $:p_敵潜入視力,$:p_敵危険視力,$w:敵回避視力 \
		-d $:p_守備Ｘ $:p_守備Ｙ $:p_守備Ｚ \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s 0 \
		-t (d:ENE_STST_HOLD_EXIT | d:ENE_STST_DONT_KILL | $:p_拡張ステータス)	/* ホールドアップで退場 */	/* 殺すとゲームオーバー */

}



/////////////////////////////////
// オルタナティブ、スネークテイルズ用ドア開閉設定
/////////////////////////////////
proc A_ドア開閉追加設定 $:p_ドア名 $:p_フラグ $:p_ドア位置Ｘ $:p_ドア位置Ｙ $:p_ドア位置Ｚ $:p_ドア回転 $:p_ドアスライド幅 {
	#if d:DEBUG_PRINT
		print 'door_open_close_set'
	#endif

	// ドアフラグを元にopenとcloseを決める
	switch ( $:p_フラグ ) {
		// 左側のドア
		case ( d:DOOR_FLAG_CLOSE ) {
			mesg ドア $:p_ドア名 locked
		}
		case ( d:DOOR_FLAG_ENEMY_ONLY ) {
			mesg ドア $:p_ドア名 openenemy
			mesg ドア $:p_ドア名 locked
			// プレイヤーだけがいけない壁を設定
			@アナザー用進入禁止壁 $:p_ドア名 $:p_ドア位置Ｘ $:p_ドア位置Ｙ $:p_ドア位置Ｚ $:p_ドア回転 $:p_ドアスライド幅
		}
		case ( d:DOOR_FLAG_CLOSE_LEVEL ) {
			mesg ドア $:p_ドア名 locked
		}
	}


}

/////////////////////////////////
// オルタナティブ、スネークテイルズ用ドアモデル名セット
/////////////////////////////////

#ifdef	d:STAGE_W00A || d:STAGE_W01A || d:STAGE_W01B || d:STAGE_W01F || \
		d:STAGE_W01E || d:STAGE_W00C || d:STAGE_W02A || d:STAGE_W03A || \
		d:STAGE_W12C || d:STAGE_W12B || d:STAGE_W13A || d:STAGE_W13B || \
		d:STAGE_W14A || d:STAGE_W15A || d:STAGE_W15B || d:STAGE_W16A || \
		d:STAGE_W17A || d:STAGE_W18A || d:STAGE_W19A || d:STAGE_W20A || \
		d:STAGE_W20B || d:STAGE_W20C || d:STAGE_W21A || d:STAGE_W21B || \
		d:STAGE_W22A || d:STAGE_W23A || d:STAGE_W23B || d:STAGE_W24A || \
		d:STAGE_W24B || d:STAGE_W24C || d:STAGE_W24D || d:STAGE_W25D || \
		d:STAGE_W28A || d:STAGE_W31D || d:STAGE_W41A || d:STAGE_W42A || \
		d:STAGE_W43A || d:STAGE_W44A || d:STAGE_W45A

	proc ドアモデル名セット $:p_配列番号 $:p_ドア名１ $:p_ドア名２ $:p_ドア名３ $:p_ドア名４ $:p_ドア名５ {
		switch ( $$b:ドアフラグ[$:p_配列番号] ) {
			case ( d:DOOR_FLAG_CLOSE ) {
				$$s:ドアモデル名[$:p_配列番号] = $:p_ドア名１;
			}
			case ( d:DOOR_FLAG_OPEN ) {
				$$s:ドアモデル名[$:p_配列番号] = $:p_ドア名２;
			}
			case ( d:DOOR_FLAG_ENEMY_ONLY ) {
				$$s:ドアモデル名[$:p_配列番号] = $:p_ドア名３;
			}
			case ( d:DOOR_FLAG_CLOSE_LEVEL ) {
				$$s:ドアモデル名[$:p_配列番号] = $:p_ドア名４;
			}
			case ( d:DOOR_FLAG_OPEN_LEVEL ) {
				$$s:ドアモデル名[$:p_配列番号] = $:p_ドア名５;
			}

			default {
				#if d:DEBUG_PRINT
					print 'WARNING!!!!!ドアフラグに不正な値が入ってます。$$b:ドアフラグ[$:p_配列番号]='$$b:ドアフラグ[$:p_配列番号]
				#endif

				command assert ( d:FALSE ) 'door flag error  (scenario err) ドアモデル名セット'
				$$s:ドアモデル名[$:p_配列番号] = $:p_ドア名１;
			}
		}
	}
#endif


/////////////////////////////////
// スネークテイルズ用紙芝居ロード
/////////////////////////////////
proc 紙芝居ステージロード $:p_ストーリーフラグ {
	#if d:DEBUG_PRINT
		print 'book_load'
		print '$:p_ストーリーフラグ'$:p_ストーリーフラグ
	#endif

	if ( `command ゲームオーバーチェック` == 0 ) {
		command 無線設定 -reset
		command パッド操作 -release
		command プレイヤー無敵セット
		mesg コマンダー 敵兵セット 視界オフ
//	ここだと、まずいので、tales.gclに移行
#if 0
		// 紙芝居に入るときは敵兵情報を全てリセットする
		command 敵兵メモリーオールリセット
#endif
		switch ( $b:tales_story_no ) {
			case ( d:TALES_NO:A ) {
				$b:tales_story_a = $:p_ストーリーフラグ;
			}
			case ( d:TALES_NO:B ) {
				$b:tales_story_b = $:p_ストーリーフラグ;
			}
			case ( d:TALES_NO:C ) {
				$b:tales_story_c = $:p_ストーリーフラグ;
			}
			case ( d:TALES_NO:D ) {
				$b:tales_story_d = $:p_ストーリーフラグ;
			}
			case ( d:TALES_NO:E ) {
				$b:tales_story_e = $:p_ストーリーフラグ;
			}

			default {
				#if d:DEBUG_PRINT
					print 'WARNING!!!!!不正な値が入ってます。$b:tales_story_no='$b:tales_story_no
				#endif

				command assert ( d:FALSE ) 'tales story flag error  (scenario err) 紙芝居ステージロード'
				$b:tales_story_a = $:p_ストーリーフラグ;
			}
		}

		// フェードインスタート
		chara フェードインアウト フェードアウト \
			-c 0,0,0 128 \
			-t d:FADEOUT_TIME \
			-p load_tales_book
	}
}



/////////////////////////////////////
// スネークテイルズＢ用
/////////////////////////////////////

//	テイルズＢ用ハプニング監視
proc スネークテイルズＢ用ハプニング監視 {
//	d:GM_STAGE_HAPPEN_DETECT			/*	発見された	*/
//	d:GM_STAGE_HAPPEN_ALERT				/*	危険モードになった	*/
//	d:GM_STAGE_HAPPEN_ENEMY_KILL		/*	敵兵殺した	*/
//	d:GM_STAGE_HAPPEN_ENEMY_FAIN		/*	敵兵気絶させた	*/
//	d:GM_STAGE_HAPPEN_ENEMY_SLEEP		/*	敵兵眠らせた	*/
//	d:GM_STAGE_HAPPEN_SNEAK2CAUTUION	/*	潜入から警戒になった	*/
	command ゲットステージハプニング \
		$$i:ステージハプニング

#if 0
	if ($$i:ステージハプニング & (d:GM_STAGE_HAPPEN_ALERT | d:GM_STAGE_HAPPEN_ENEMY_KILL | \
		d:GM_STAGE_HAPPEN_ENEMY_FAINT | d:GM_STAGE_HAPPEN_ENEMY_SLEEP) ) {
#else
	if ($$i:ステージハプニング & (d:GM_STAGE_HAPPEN_ALERT | d:GM_STAGE_HAPPEN_SNEAK2CAUTUION) ) {
#endif
print '#######################################################################################STAGE_HAPPENING'
		eval ($f:テイルズＢ見つかったりしたフラグ = 1)
	}
print '###################################################################$f:テイルズＢ見つかったりしたフラグ' $f:テイルズＢ見つかったりしたフラグ

}

// テイルズＢ用寄り道攻撃兵
proc A_テイルズＢ用寄り道攻撃兵 $:p_名前 $:p_ルート $:p_ステータス $:p_装備 $:p_寄り道Ｘ $:p_寄り道Ｙ $:p_寄り道Ｚ $:p_寄り道マップ {
	#if d:DEBUG_PRINT
		print 'attack_set'
	#endif

	chara アタッカー $:p_名前 -r $:p_ルート -s $:p_ステータス -e $:p_装備 \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-d $:p_寄り道Ｘ $:p_寄り道Ｙ $:p_寄り道Ｚ $:p_寄り道マップ
}

proc A_テイルズＢ用寄り道拡張攻撃兵 $:p_名前 $:p_ルート $:p_ステータス $:p_拡張ステータス $:p_装備 $:p_寄り道Ｘ $:p_寄り道Ｙ $:p_寄り道Ｚ $:p_寄り道マップ {
	#if d:DEBUG_PRINT
		print 'attack_set'
	#endif

	chara アタッカー $:p_名前 -r $:p_ルート -s $:p_ステータス -t $:p_拡張ステータス -e $:p_装備 \
		-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-d $:p_寄り道Ｘ $:p_寄り道Ｙ $:p_寄り道Ｚ $:p_寄り道マップ
}

//	双眼鏡兵状態保存
proc A_双眼鏡兵状態保存 $:p_双眼鏡兵名 {
	//	スネークテイルズＢのみ機能する
	if (($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
		($b:tales_story_no == d:TALES_NO:B) ) {

		//	双眼鏡兵の状態
		//	気絶か、眠っているかチェック
		command ゲット敵兵状態拡張版 \
			$:p_双眼鏡兵名 $i:双眼鏡兵状態

			//	気絶中
		if ($i:双眼鏡兵状態 & 0x00000001) {
			eval ($b:w20b_２Ｆ敵兵状態 = d:STUN)

		//	眠り中
		} else if ($i:双眼鏡兵状態 & 0x00000002) {
			eval ($b:w20b_２Ｆ敵兵状態 = d:SLEEP)

		//	それ以外ならば
		} else {
			//	死んでいるかをチェック
			command ゲット敵兵状態 \
				$:p_双眼鏡兵名 $i:双眼鏡兵状態
				//	死んでいる
			if ($i:双眼鏡兵状態 & 0x80000000) {
				eval ($b:w20b_２Ｆ敵兵状態 = d:DEAD)
				//	それ以外は普通に戻す
			} else {
				//	普通に戻す
				eval ($b:w20b_２Ｆ敵兵状態 = d:NORMAL)
			}
		}

	}

}

//	双眼鏡兵開始状態チェック
proc A_双眼鏡兵開始状態チェック	$:p_ステージ名 $:p_マップ名 $:p_双眼鏡兵名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_向き {

	//	スネークテイルズＢのみ機能する
	if (($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
		($b:tales_story_no == d:TALES_NO:B) ) {

		//	以下の条件のときは状態をセットする
		if (((($s:エリア == a19a) || ($s:エリア == a21a)) && (($s:エリア履歴[ 1 ] == a20b ) || ($s:エリア履歴[ 1 ] == a20d ))) || \
			((($s:エリア == a20b) || ($s:エリア == a20d)) && (($s:エリア履歴[ 1 ] == a19a ) || ($s:エリア履歴[ 1 ] == a21a ))) || \
			/*	以下の条件はテイルズＢで紙芝居が入るので例外的に	*/
			 (($s:エリア == a19a) && ($s:エリア履歴[ 0 ] == tales)) ) {

			//	双眼鏡兵
			//	気絶からスタート
			if ($b:w20b_２Ｆ敵兵状態 == d:STUN) {
				command セット敵兵メモリー \
					$:p_ステージ名 \
					$:p_マップ名 \
					$:p_双眼鏡兵名 \
					$:p_Ｘ座標 \
					$:p_Ｙ座標 \
					$:p_Ｚ座標\
					$:p_向き \
					d:ENEMEM_ST_FAINT \
					d:ENEMEM_PS_BACK \
					d:FAINT_TIME_3 \
					$w:敵標準気絶  \
					$w:敵標準体力  \
					0 \
					0

			//	眠りからスタート
			} else if ($b:w20b_２Ｆ敵兵状態 == d:SLEEP) {
				command セット敵兵メモリー \
					$:p_ステージ名 \
					$:p_マップ名 \
					$:p_双眼鏡兵名 \
					$:p_Ｘ座標 \
					$:p_Ｙ座標 \
					$:p_Ｚ座標\
					$:p_向き \
					d:ENEMEM_ST_SLEEP \
					d:ENEMEM_PS_BACK \
					d:SLEEP_TIME_3 \
					$w:敵標準気絶  \
					$w:敵標準体力  \
					0 \
					0

//	危険かもしれないので死亡スタートは止める
#if 0
			//	あの世からスタート
			} else if ($b:w20b_２Ｆ敵兵状態 == d:DEAD) {
				command セット敵兵メモリー \
					$:p_ステージ名 \
					$:p_マップ名 \
					$:p_双眼鏡兵名 \
					$:p_Ｘ座標 \
					$:p_Ｙ座標 \
					$:p_Ｚ座標\
					$:p_向き \
					d:ENEMEM_ST_HELL \
					d:ENEMEM_PS_BACK \
					0 \
					$w:敵標準気絶  \
					$w:敵標準体力  \
					0 \
					0
#endif
			}

		//	それ以外はそのままスタート
		} else {
			eval ($b:w20b_２Ｆ敵兵状態 = d:NORMAL)

		}

	}

}




/////////////////////////////////
// デバッグ用のプロック
/////////////////////////////////
#ifndef	d:FIX_MODE
#ifdef d:DEBUG_KEYBOARD_CAMERA
#include "usbkbd.h"
proc デバッグ用ＵＳＢキーボード対応カメラ {
	chara カメラ 撮影用カメラ		-c 0 -l 1 -p 0

	chara USBキーボード実行 test \
		-e d:KBD_ESC {
			chara カメラ設定 撮影用カメラ -s 0
		} \
		-e d:KBD_1 {
			chara カメラ設定 撮影用カメラ \
				-c 1 \
				-p 269,9596,-6082 -t 1669,7494,-7339 \
				-r 548,1501,0 -f 2821 \
				-a 200 \
				-i 0 2 0 0 \
				-s 1
		} \
		-e d:KBD_2 {
			chara カメラ設定 撮影用カメラ \
				-c 1 \
				-p -3008,5076,-9606 -t 1892,6888,-7750 \
				-r -217,788,0 -f 5544 \
				-a 200 \
				-i 0 2 0 0 \
				-s 1
		} \
		-e d:KBD_3 {
			chara カメラ設定 撮影用カメラ \
				-c 1 \
				-p -4099,5223,-8161 -t 373,6375,-9245 \
				-r -160,1179,0 -f 4743 \
				-a 200 \
				-i 0 2 0 0 \
				-s 1
		}

}

proc デバッグ用ＵＳＢキーボード対応パッドデモ $:p_パッドデモファイル名 {
	chara パッドデモ パッドデモ \
		-file $:p_パッドデモファイル名

	chara USBキーボード実行 test \
		-e d:KBD_P {
			mesg パッドデモ パッドデモ start
		}
}

#endif

proc デバッグ用オールタナティブミッション表示 {
	#if d:DEBUG_PRINT
		if( $b:ミッション番号 == d:MISSION:爆弾解体 ) {
			print 'bomb mission start - 爆弾解体ミッションスタート'
		} else if( $b:ミッション番号 == d:MISSION:敵兵排除 ) {
			print 'enemy mission start - 敵兵排除ミッションスタート'
		} else if( $b:ミッション番号 == d:MISSION:武装解除 ) {
			print 'hold mission start - 武装解除ミッションスタート'
		} else if( $b:ミッション番号 == d:MISSION:写真撮影 ) {
			print 'photo mission start - 写真撮影ミッションスタート'
		} else if( $b:ミッション番号 == d:MISSION:スネークテイルズ) {
			print 'photo mission start - スネークテイルズミッションスタート'
		}
	#endif
}

#endif

//	武器訓練刀ステージ用
proc 刀装備セット {
	// 初期装備の設定
	//	ライデン／忍者は刀を抜いた状態からスタート
	if (( $s:選択プレイヤー == ライデン ) || \
		( $s:選択プレイヤー == 刀ライデン ) ) {
		eval ( $w:武器最大数Ｒ[ d:武器:ブレード ] = 1 )
		eval ( $w:武器弾数Ｒ[ d:武器:ブレード ] = 1 )
		eval ($w:プレイヤーフラグ = $w:プレイヤーフラグ | 0x0002 )
		eval ($w:プレイヤーフラグ = $w:プレイヤーフラグ & ~(0x0004) )
		eval ($w:武器 = d:武器:ブレード)
	}
	//	プラグイン
	//	忍者はA_プレイヤー設定でプラグインを呼んでいる
	if ( $s:選択プレイヤー == ライデン ) {
		command プラグインブレード -m rai_blade
	}

}



//	マップ名つきアイテムボックス
#define シナリオの発生パターン	  0
#define シナリオの高さ			300

// ＵＳＰ弾薬（マップ名つき）
proc 弾薬_ＵＳＰ_初期設置（マップ名つき） $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_マップ名 {
	#if d:DEBUG_PRINT
		print 'usp_amo_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \	// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
		-k handgun_amo_ibox \			// モデル：箱本体
		-s handgun_amo_ibox_sh \		// モデル：箱影
		-l usp_amo_label \				// モデル：ラベル
		-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:ＵＳＰ \				// ＩＤ：from ..../scn/equip.h
		-n 15 \							// 個数
		-r $:p_水飛沫 \					// 0で飛沫無し。
		-h d:シナリオの高さ \			// 床面からの高さ
		-m $:p_マップ名 \					// マップ名
		-x {
			@アイテム取得時番号設定 $4
		}
}

// Ｍ９弾薬（マップ名つき）
proc 弾薬_Ｍ９_初期設置（マップ名つき） $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_マップ名 {
	#if d:DEBUG_PRINT
		print 'usp_amo_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \	// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
		-k handgun_amo_ibox \			// モデル：箱本体
		-s handgun_amo_ibox_sh \		// モデル：箱影
		-l m92_amo_label \				// モデル：ラベル
		-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:Ｍ９ \				// ＩＤ：from ..../scn/equip.h
		-n 15 \							// 個数
		-r $:p_水飛沫 \					// 0で飛沫無し。
		-h d:シナリオの高さ \			// 床面からの高さ
		-m $:p_マップ名 \					// マップ名
		-x {
			@アイテム取得時番号設定 $4
		}
}

// スティンガー弾薬（マップ名つき）
proc 弾薬_スティンガー_初期設置（マップ名つき） $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_マップ名 {
	#if d:DEBUG_PRINT
		print 'stinger_amo_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \	// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
		-k launcher_amo_ibox \			// モデル：箱本体
		-s launcher_amo_ibox_sh \		// モデル：箱影
		-l stg_amo_label \				// モデル：ラベル
		-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:スティンガー \		// ＩＤ：from ..../scn/equip.h
		-n 10 \							// 個数
		-r $:p_水飛沫 \					// 0で飛沫無し。
		-h d:シナリオの高さ \			// 床面からの高さ
		-m $:p_マップ名 \					// マップ名
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc レーション_初期設置（マップ名つき） $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_マップ名 {
	#if d:DEBUG_PRINT
		print 'ration_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \		// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
		-k ration_ibox \					// モデル：箱本体
		-s ration_ibox_sh \					// モデル：箱影
		-l rtn_label \						// モデル：ラベル
		-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
		-i d:アイテム:レーション \			// ＩＤ：from ..../scn/equip.h
		-n 1 \								// 個数
		-r $:p_水飛沫 \						// 0で飛沫無し。
		-h d:シナリオの高さ \				// 床面からの高さ
		-m $:p_マップ名 \					// マップ名
		-x {
			@アイテム取得時番号設定 $4
		}
}





/////////////////////////////////
//	スネークテイルズＡ用プロック集
/////////////////////////////////
// テイルズＡで使用する全体マップの爆弾表示チェック
// 全体マップの後に呼ばないとまずい
proc スネークテイルズＡ用マップ爆弾表示チェック {
	#if d:DEBUG_PRINT
		print 'bomb_map_check'
	#endif

	if ( $b:ミッション番号 == d:MISSION:スネークテイルズ && \
		 $b:tales_story_no == d:TALES_NO:A && \
		 $b:tales_story_a == d:TALES_A:爆弾解体前 && \
		 $f:テイルズＡファットマン戦中フラグ == 0 ) {

		command 全体マップ爆弾配置 -bomb d:シェル１中央棟マップ
	}
}




/////////////////////////////////
//	スネークテイルズＥ用プロック集
/////////////////////////////////
// テイルズＥ用変数退避
proc スネークテイルズＥ用変数退避 {
	#if d:DEBUG_PRINT
	print'#####################HENSU_TAIHI_START'

	print'####################$b:ミッション番号' $b:ミッション番号
	print'####################$b:tales_story_no' $b:tales_story_no
	print'####################$s:選択プレイヤー' $s:選択プレイヤー
	print'####################$b:tales_story_e' $b:tales_story_e
	print '$b:テイルズＥ爆弾解体１面クリア成績フラグ' $b:テイルズＥ爆弾解体１面クリア成績フラグ
	print '$f:テイルズＥハンドガン３面クリア成績フラグ' $f:テイルズＥハンドガン３面クリア成績フラグ;
	print '$f:テイルズＥコンティニューフラグ' $f:テイルズＥコンティニューフラグ;
	print '$b:テイルズＥ雷電テイルズループ回数' $b:テイルズＥ雷電テイルズループ回数
	print '$f:テイルズタイトル表示終了フラグ' $f:テイルズタイトル表示終了フラグ

	print '$f:テイルズＥレーション取ったフラグ' $f:テイルズＥレーション取ったフラグ

	print'###############################CONTINUE_COUNT' $i:ＶＲ設定

	print'#####################$i:プレイタイム' $i:プレイタイム
	print'#######################$w:セーブ回数' $w:セーブ回数
	print'###############$w:コンティニュー回数' $w:コンティニュー回数
	print'###################$w:危険モード回数' $w:危険モード回数
	print'#########################$w:殺傷人数' $w:殺傷人数
	print'###############$w:レーション使用回数' $w:レーション使用回数
	#endif

	//	スネークテイルズＥで使用している変数をローカル変数へ退避
	//	変数が増えたときにはここに書き込むこと！
	$$b:ミッション番号 = $b:ミッション番号;
	$$b:tales_story_no = $b:tales_story_no;
	$$s:選択プレイヤー = $s:選択プレイヤー;

	$$b:tales_story_e = $b:tales_story_e;

	$$b:テイルズＥ爆弾解体１面クリア成績フラグ = $b:テイルズＥ爆弾解体１面クリア成績フラグ;
	$$f:テイルズＥハンドガン３面クリア成績フラグ = $f:テイルズＥハンドガン３面クリア成績フラグ;
	$$f:テイルズＥコンティニューフラグ = $f:テイルズＥコンティニューフラグ;

	$$b:テイルズＥ雷電テイルズループ回数 = $b:テイルズＥ雷電テイルズループ回数;

	$$f:テイルズタイトル表示終了フラグ = $f:テイルズタイトル表示終了フラグ;

	//	関係ないはずだが念のため
	$$f:テイルズＥレーション取ったフラグ = $f:テイルズＥレーション取ったフラグ;


//	以下リザルト画面用
//	必要なくなったはず
#if 0
	$$i:プレイタイム = $i:プレイタイム;
	$$w:セーブ回数 = $w:セーブ回数;
	$$w:コンティニュー回数 = $w:コンティニュー回数;
	$$w:危険モード回数 = $w:危険モード回数;
	$$w:殺傷人数 = $w:殺傷人数;
	$$w:レーション使用回数 = $w:レーション使用回数;
#endif

	//	テイルズであることを明示
	command テイルズステージセット

	#if d:DEBUG_PRINT
	print'####################$$b:ミッション番号' $$b:ミッション番号
	print'####################$$b:tales_story_no' $$b:tales_story_no
	print'####################$$s:選択プレイヤー' $$s:選択プレイヤー
	print'####################$$b:tales_story_e' $$b:tales_story_e
	print '$$b:テイルズＥ爆弾解体１面クリア成績フラグ' $$b:テイルズＥ爆弾解体１面クリア成績フラグ
	print '$$f:テイルズＥハンドガン３面クリア成績フラグ' $$f:テイルズＥハンドガン３面クリア成績フラグ
	print '$$f:テイルズＥコンティニューフラグ' $$f:テイルズＥコンティニューフラグ;
	print '$$b:テイルズＥ雷電テイルズループ回数' $$b:テイルズＥ雷電テイルズループ回数
	print '$$f:テイルズタイトル表示終了フラグ' $$f:テイルズタイトル表示終了フラグ

	print '$$f:テイルズＥレーション取ったフラグ' $$f:テイルズＥレーション取ったフラグ

	print'###############################CONTINUE_COUNT' $i:ＶＲ設定

#if 0
	print'####################$$i:プレイタイム' $$i:プレイタイム
	print'######################$$w:セーブ回数' $$w:セーブ回数
	print'##############$$w:コンティニュー回数' $$w:コンティニュー回数
	print'##################$$w:危険モード回数' $$w:危険モード回数
	print'########################$$w:殺傷人数' $$w:殺傷人数
	print'##############$$w:レーション使用回数' $$w:レーション使用回数
#endif

	print'#####################HENSU_TAIHI_END'
	#endif
}

// テイルズＥ用変数戻しプロック
proc スネークテイルズＥ用変数戻し {
	#if d:DEBUG_PRINT
	print'#####################HENSU_MODOSHI_START'

	print'####################$$b:ミッション番号' $$b:ミッション番号
	print'####################$$b:tales_story_no' $$b:tales_story_no
	print'####################$$s:選択プレイヤー' $$s:選択プレイヤー
	print'####################$$b:tales_story_e' $$b:tales_story_e
	print '$$b:テイルズＥ爆弾解体１面クリア成績フラグ' $$b:テイルズＥ爆弾解体１面クリア成績フラグ
	print '$$f:テイルズＥハンドガン３面クリア成績フラグ' $$f:テイルズＥハンドガン３面クリア成績フラグ
	print '$$f:テイルズＥコンティニューフラグ' $$f:テイルズＥコンティニューフラグ;
	print '$$b:テイルズＥ雷電テイルズループ回数' $$b:テイルズＥ雷電テイルズループ回数
	print '$$f:テイルズタイトル表示終了フラグ' $$f:テイルズタイトル表示終了フラグ

	print '$$f:テイルズＥレーション取ったフラグ' $$f:テイルズＥレーション取ったフラグ

	print'###############################CONTINUE_COUNT' $i:ＶＲ設定

#if 0
	print'####################$$i:プレイタイム' $$i:プレイタイム
	print'######################$$w:セーブ回数' $$w:セーブ回数
	print'##############$$w:コンティニュー回数' $$w:コンティニュー回数
	print'##################$$w:危険モード回数' $$w:危険モード回数
	print'########################$$w:殺傷人数' $$w:殺傷人数
	print'##############$$w:レーション使用回数' $$w:レーション使用回数
#endif

	#endif

	//	ローカル変数へ退避した変数をもとに戻す
	//	変数が増えたときにはここに書き込むこと！
	$b:ミッション番号 = $$b:ミッション番号;
	$b:tales_story_no = $$b:tales_story_no;
	$s:選択プレイヤー = $$s:選択プレイヤー;

	$b:tales_story_e = $$b:tales_story_e;

	$b:テイルズＥ爆弾解体１面クリア成績フラグ = $$b:テイルズＥ爆弾解体１面クリア成績フラグ;
	$f:テイルズＥハンドガン３面クリア成績フラグ= $$f:テイルズＥハンドガン３面クリア成績フラグ;
	$f:テイルズＥコンティニューフラグ = $$f:テイルズＥコンティニューフラグ;

	$b:テイルズＥ雷電テイルズループ回数 = $$b:テイルズＥ雷電テイルズループ回数;

	$f:テイルズタイトル表示終了フラグ = $$f:テイルズタイトル表示終了フラグ;

	//	関係ないはずだが念のため
	$f:テイルズＥレーション取ったフラグ = $$f:テイルズＥレーション取ったフラグ;

//	以下リザルト画面用
//	必要なくなったはず
#if 0
	$i:プレイタイム = $$i:プレイタイム;
	$w:セーブ回数 = $$w:セーブ回数;
	$w:コンティニュー回数 = $$w:コンティニュー回数;
	$w:危険モード回数 = $$w:危険モード回数;
	$w:殺傷人数 = $$w:殺傷人数;
	$w:レーション使用回数 = $$w:レーション使用回数;
#endif

	//	戻した変数をバーセーブする
	command varsave \
		$b:ミッション番号							\
		$b:tales_story_no							\
		$s:選択プレイヤー							\
		$b:tales_story_e							\
		$b:テイルズＥ爆弾解体１面クリア成績フラグ	\
		$f:テイルズＥハンドガン３面クリア成績フラグ	\
		$f:テイルズＥコンティニューフラグ			\
		$b:テイルズＥ雷電テイルズループ回数			\
		$f:テイルズタイトル表示終了フラグ			\
		$f:テイルズＥレーション取ったフラグ			\
		$i:ＶＲ設定

//	必要なくなったはず
#if 0
		$i:プレイタイム								\
		$w:セーブ回数								\
		$w:コンティニュー回数						\
		$w:危険モード回数							\
		$w:殺傷人数									\
		$w:レーション使用回数
#endif

	#if d:DEBUG_PRINT
	print'####################$b:ミッション番号' $b:ミッション番号
	print'####################$b:tales_story_no' $b:tales_story_no
	print'####################$s:選択プレイヤー' $s:選択プレイヤー
	print'####################$b:tales_story_e' $b:tales_story_e
	print '$b:テイルズＥ爆弾解体１面クリア成績フラグ' $b:テイルズＥ爆弾解体１面クリア成績フラグ
	print '$f:テイルズＥハンドガン３面クリア成績フラグ' $f:テイルズＥハンドガン３面クリア成績フラグ;
	print '$f:テイルズＥコンティニューフラグ' $f:テイルズＥコンティニューフラグ;
	print '$b:テイルズＥ雷電テイルズループ回数' $b:テイルズＥ雷電テイルズループ回数
	print '$f:テイルズタイトル表示終了フラグ' $f:テイルズタイトル表示終了フラグ

	print '$f:テイルズＥレーション取ったフラグ' $f:テイルズＥレーション取ったフラグ

	print'###############################CONTINUE_COUNT' $i:ＶＲ設定

	print'#####################$i:プレイタイム' $i:プレイタイム
	print'#######################$w:セーブ回数' $w:セーブ回数
	print'###############$w:コンティニュー回数' $w:コンティニュー回数
	print'###################$w:危険モード回数' $w:危険モード回数
	print'#########################$w:殺傷人数' $w:殺傷人数
	print'###############$w:レーション使用回数' $w:レーション使用回数

	print'#####################HENSU_MODOSHI_END'
	#endif
}



//	スネークテイルズＥ用ＶＲクリア設定その１
//	以下の仕様である
/*
	クリア			「ＲＥＴＲＹ」	→	リトライ
	クリア			「ＥＸＩＴ」	→	次の紙芝居へ

	ポーズ			「ＲＥＴＲＹ」	→	リトライ
	ポーズ			「ＥＸＩＴ」	→	次の紙芝居へ

	ゲームオーバー	「ＲＥＴＲＹ」	→	リトライ
	ゲームオーバー	「ＥＸＩＴ」	→	次の紙芝居へ

	※「ＮＥＸＴ ＳＴＡＧＥ」は選択項目がない
*/

proc テイルズＥ用ＶＲクリア設定１ {
	#if d:DEBUG_PRINT
		print 'tales_e_1_vr_clear_set'
	#endif

	local $$:スコアタイプ;
	local $$:秒変換レート;
	local $$:単発系武器変換レート;
	local $$:連射系武器変換レート;
	local $$:爆発系武器変換レート;
	local $$:隠密変換レート;
	local $$:不殺変換レート;
	local $$:一位ジングル;
	local $$:二位ジングル;
	local $$:三位ジングル;
	local $$:圏外ジングル;

	if ( $b:ミッション番号 == d:MISSION:爆弾解体 ) {
		$$:秒変換レート = d:SCORE_RATE_A_BOMB;
		$$:単発系武器変換レート = d:SCORE_RATE_B_BOMB;
		$$:連射系武器変換レート = d:SCORE_RATE_C_BOMB;
		$$:爆発系武器変換レート = d:SCORE_RATE_D_BOMB;
		$$:隠密変換レート = d:SCORE_RATE_F_BOMB;
		$$:不殺変換レート = d:SCORE_RATE_G_BOMB;
	} else {
		$$:秒変換レート = d:SCORE_RATE_A;
		$$:単発系武器変換レート = d:SCORE_RATE_B;
		$$:連射系武器変換レート = d:SCORE_RATE_C;
		$$:爆発系武器変換レート = d:SCORE_RATE_D;
		$$:隠密変換レート = d:SCORE_RATE_F;
		$$:不殺変換レート = d:SCORE_RATE_G;
	}


	#ifdef d:STAGE_VR
		if ( $s:選択プレイヤー == ライデン || $s:選択プレイヤー == 刀ライデン || $s:選択プレイヤー == 裸ライデン ) {
			$$:一位ジングル = t:vc030215;
			$$:二位ジングル = t:vc030216;
			$$:三位ジングル = t:vc030217;
			$$:圏外ジングル = t:vc030218;

		} else if ( $s:選択プレイヤー == スネーク || $s:選択プレイヤー == プリスキン || $s:選択プレイヤー == タキシードスネーク ) {
			$$:一位ジングル = t:vc030211;
			$$:二位ジングル = t:vc030212;
			$$:三位ジングル = t:vc030213;
			$$:圏外ジングル = t:vc030214;

		} else {
			// 前作スネーク
			$$:一位ジングル = t:vc030241;
			$$:二位ジングル = t:vc030242;
			$$:三位ジングル = t:vc030243;
			$$:圏外ジングル = t:vc030244;

		}
	#else
		if ( $s:選択プレイヤー == ライデン || $s:選択プレイヤー == 刀ライデン || $s:選択プレイヤー == 裸ライデン ) {
			$$:一位ジングル = t:vc030235;
			$$:二位ジングル = t:vc030236;
			$$:三位ジングル = t:vc030237;
			$$:圏外ジングル = t:vc030238;

		} else {
			// スネーク系
			$$:一位ジングル = t:vc030231;
			$$:二位ジングル = t:vc030232;
			$$:三位ジングル = t:vc030233;
			$$:圏外ジングル = t:vc030234;

		}
	#endif

	#if d:MGS2_VRTRIAL	//chara ＶＲクリア
		$$i:ＶＲクリアフラグ = ( $$i:ＶＲクリアフラグ | d:VRCLR_VRTRIAL );
	#endif

	if ( $b:ミッション番号 != d:MISSION:バラエティ && $b:ミッション番号 != d:MISSION:主観モード ) {
		// 通常バージョン
		chara ＶＲクリア ＶＲクリア君 \
			-rate_a $$:秒変換レート	\			// $i:秒変換レート(フレーム単位)
			-rate_b $$:単発系武器変換レート	\	// $i:単発系武器変換レート
			-rate_c $$:連射系武器変換レート	\	// $i:連射系武器変換レート
			-rate_d $$:爆発系武器変換レート	\	// $i:爆発系武器変換レート
			-rate_e 0	\						// $i:コンボ変換レート(コンボは仕様変更によりなくなりました）
			-rate_f $$:隠密変換レート	\		// $i:隠密変換レート
			-rate_g $$:不殺変換レート	\		// $i:不殺変換レート

			-try_again テイルズＥ用ＲＥＴＲＹ \
			-exit load_tales_e_next \					//	次の紙芝居へ

			-f ($$i:ＶＲクリアフラグ | d:VRCLR_NO_NEXTSTAGE | d:VRCLR_NO_SETSUMEI) \		//	スネークテイルズでは「ＮＥＸＴ　ＳＴＡＧＥ」出ない、クリアコード出ない
			-bgm $$:一位ジングル $$:二位ジングル $$:三位ジングル $$:圏外ジングル

	} else {
		// バラエティと主観モードはステージごとにスコアタイプを設定
		#ifdef d:STAGE_SP01A
			if ( $b:ステージ番号 == 1 ) {
				// 敵が出ないor殺せない&最初からゴールが出ているタイプ
				$$:スコアタイプ = d:SCORE_TYPE_05;		//            TIME
			} else {
				// 的を全て壊すとゴールが出るタイプ
				$$:スコアタイプ = d:SCORE_TYPE_03;		// MAP SCORE  TIME  BULLET  COMBO
			}

		#elifdef d:STAGE_SP02A || d:STAGE_SP07A
			// 敵が出ないor殺せない&最初からゴールが出ているタイプ
			$$:スコアタイプ = d:SCORE_TYPE_05;			//            TIME

		#elifdef d:STAGE_SP03A || d:STAGE_SP06A
			// 敵を全滅させるとゴールが出るタイプ
			$$:スコアタイプ = d:SCORE_TYPE_02;			//            TIME  BULLET                   NO KILL

		#elifdef d:STAGE_SP08A
			// 敵を全滅させるとゴールが出るタイプ
			$$:スコアタイプ = d:SCORE_TYPE_04;			//            TIME  BULLET         SNEAKING  NO KILL

		#elifdef d:STAGE_SP21A
			// 的を全て壊すとゴールが出るタイプ
			$$:スコアタイプ = d:SCORE_TYPE_03;		// MAP SCORE  TIME  BULLET  COMBO

		#elifdef d:STAGE_SP22A || d:d:STAGE_SP23A || d:STAGE_SP24A || d:STAGE_SP25A
			$$:スコアタイプ = d:SCORE_TYPE_04;			//            TIME  BULLET         SNEAKING  NO KILL

		#else
			// デフォルトタイプ
			$$:スコアタイプ = d:SCORE_TYPE_05;			//			  TIME

		#endif

		// バラエティは特殊バージョン
		chara ＶＲクリア ＶＲクリア君 \
			-rate_a $$:秒変換レート	\			// $i:秒変換レート(フレーム単位)
			-rate_b $$:単発系武器変換レート	\	// $i:単発系武器変換レート
			-rate_c $$:連射系武器変換レート	\	// $i:連射系武器変換レート
			-rate_d $$:爆発系武器変換レート	\	// $i:爆発系武器変換レート
			-rate_e 0	\						// $i:コンボ変換レート(コンボは仕様変更によりなくなりました）
			-rate_f $$:隠密変換レート	\		// $i:隠密変換レート
			-rate_g $$:不殺変換レート	\		// $i:不殺変換レート

			-try_again テイルズＥ用ＲＥＴＲＹ \
			-exit load_tales_e_next \					//	次の紙芝居へ

			-score_type $$:スコアタイプ \
			-f ($$i:ＶＲクリアフラグ | d:VRCLR_NO_NEXTSTAGE | d:VRCLR_NO_SETSUMEI) \		//	スネークテイルズでは「ＮＥＸＴ　ＳＴＡＧＥ」出ない、クリアコード出ない
			-bgm $$:一位ジングル $$:二位ジングル $$:三位ジングル $$:圏外ジングル

	}

	//ＶＲポーズも一緒に設定
	#ifdef d:STAGE_VR
		chara ＶＲポーズ ＶＲポーズ君 \
			-restart テイルズＥ用ＲＥＳＴＡＲＴ \
			-exit    load_tales_e_next			//	次の紙芝居へ
	#else
		chara ＶＲ全体マップ表示３Ｄ ＶＲ全体マップ表示３Ｄ君 \
			-player				$$s:プレイヤ位置ラベル \
			#ifdef d:STAGE_TANKER
				-model			[A_全体マップ:タンカー] \
			#elifdef d:STAGE_PLANT
				#ifdef d:STAGE_W41A || d:STAGE_W42A || d:STAGE_W43A || d:STAGE_W44A || d:STAGE_W45A
					-model		[A_全体マップ:アーセナル] \
				#else
					-model		[A_全体マップ:プラント] \
				#endif
			#endif
			-active $i:A_全体マップ移動可能ステージ \
			-bomb $i:A_全体マップ爆弾設置ステージ \
			-restart テイルズＥ用ＲＥＳＴＡＲＴ \

			-exit load_tales_e_next \					//	次の紙芝居へ

			-vr_pause	// ＶＲポーズ起動フラグ
	#endif

	#if d:DEBUG_PRINT
	print '###############################VR_GAMEOVER'
	#endif

	//	テイルズＥ用ゲームオーバー設定
	command セットＶＲステータス d:VR_EXIT_TO_SPECIAL
	command 特殊ＶＲゲームオーバー -p load_tales_e_next					//	次の紙芝居へ
}

//	スネークテイルズＥ用ＶＲクリア設定その２
//	以下の仕様である
/*
	クリア			「ＲＥＴＲＹ」	→	リトライ
	クリア			「ＥＸＩＴ」	→	次の紙芝居へ

	ポーズ			「ＲＥＴＲＹ」	→	リトライ
	ポーズ			「ＥＸＩＴ」	→	やり直し紙芝居→リトライ

	ゲームオーバー	「ＲＥＴＲＹ」	→	リトライ
	ゲームオーバー	「ＥＸＩＴ」	→	やり直し紙芝居→リトライ

	※「ＮＥＸＴ ＳＴＡＧＥ」は選択項目がない
*/

proc テイルズＥ用ＶＲクリア設定２ {
	#if d:DEBUG_PRINT
		print 'tales_e_2_vr_clear_set'
	#endif

	local $$:スコアタイプ;
	local $$:秒変換レート;
	local $$:単発系武器変換レート;
	local $$:連射系武器変換レート;
	local $$:爆発系武器変換レート;
	local $$:隠密変換レート;
	local $$:不殺変換レート;
	local $$:一位ジングル;
	local $$:二位ジングル;
	local $$:三位ジングル;
	local $$:圏外ジングル;

	if ( $b:ミッション番号 == d:MISSION:爆弾解体 ) {
		$$:秒変換レート = d:SCORE_RATE_A_BOMB;
		$$:単発系武器変換レート = d:SCORE_RATE_B_BOMB;
		$$:連射系武器変換レート = d:SCORE_RATE_C_BOMB;
		$$:爆発系武器変換レート = d:SCORE_RATE_D_BOMB;
		$$:隠密変換レート = d:SCORE_RATE_F_BOMB;
		$$:不殺変換レート = d:SCORE_RATE_G_BOMB;
	} else {
		$$:秒変換レート = d:SCORE_RATE_A;
		$$:単発系武器変換レート = d:SCORE_RATE_B;
		$$:連射系武器変換レート = d:SCORE_RATE_C;
		$$:爆発系武器変換レート = d:SCORE_RATE_D;
		$$:隠密変換レート = d:SCORE_RATE_F;
		$$:不殺変換レート = d:SCORE_RATE_G;
	}


	#ifdef d:STAGE_VR
		if ( $s:選択プレイヤー == ライデン || $s:選択プレイヤー == 刀ライデン || $s:選択プレイヤー == 裸ライデン ) {
			$$:一位ジングル = t:vc030215;
			$$:二位ジングル = t:vc030216;
			$$:三位ジングル = t:vc030217;
			$$:圏外ジングル = t:vc030218;

		} else if ( $s:選択プレイヤー == スネーク || $s:選択プレイヤー == プリスキン || $s:選択プレイヤー == タキシードスネーク ) {
			$$:一位ジングル = t:vc030211;
			$$:二位ジングル = t:vc030212;
			$$:三位ジングル = t:vc030213;
			$$:圏外ジングル = t:vc030214;

		} else {
			// 前作スネーク
			$$:一位ジングル = t:vc030241;
			$$:二位ジングル = t:vc030242;
			$$:三位ジングル = t:vc030243;
			$$:圏外ジングル = t:vc030244;

		}
	#else
		if ( $s:選択プレイヤー == ライデン || $s:選択プレイヤー == 刀ライデン || $s:選択プレイヤー == 裸ライデン ) {
			$$:一位ジングル = t:vc030235;
			$$:二位ジングル = t:vc030236;
			$$:三位ジングル = t:vc030237;
			$$:圏外ジングル = t:vc030238;

		} else {
			// スネーク系
			$$:一位ジングル = t:vc030231;
			$$:二位ジングル = t:vc030232;
			$$:三位ジングル = t:vc030233;
			$$:圏外ジングル = t:vc030234;

		}
	#endif


	#if d:MGS2_VRTRIAL	//chara ＶＲクリア
		$$i:ＶＲクリアフラグ = ( $$i:ＶＲクリアフラグ | d:VRCLR_VRTRIAL );
	#endif

	if ( $b:ミッション番号 != d:MISSION:バラエティ && $b:ミッション番号 != d:MISSION:主観モード ) {
		// 通常バージョン
		chara ＶＲクリア ＶＲクリア君 \
			-rate_a $$:秒変換レート	\			// $i:秒変換レート(フレーム単位)
			-rate_b $$:単発系武器変換レート	\	// $i:単発系武器変換レート
			-rate_c $$:連射系武器変換レート	\	// $i:連射系武器変換レート
			-rate_d $$:爆発系武器変換レート	\	// $i:爆発系武器変換レート
			-rate_e 0	\						// $i:コンボ変換レート(コンボは仕様変更によりなくなりました）
			-rate_f $$:隠密変換レート	\		// $i:隠密変換レート
			-rate_g $$:不殺変換レート	\		// $i:不殺変換レート

			-try_again テイルズＥ用ＲＥＴＲＹ \
			-exit load_tales_e_next \					//	次の紙芝居へ

			-f ($$i:ＶＲクリアフラグ | d:VRCLR_NO_NEXTSTAGE | d:VRCLR_NO_SETSUMEI) \		//	スネークテイルズでは「ＮＥＸＴ　ＳＴＡＧＥ」出ない、クリアコード出ない
			-bgm $$:一位ジングル $$:二位ジングル $$:三位ジングル $$:圏外ジングル

	} else {
		// バラエティと主観モードはステージごとにスコアタイプを設定
		#ifdef d:STAGE_SP01A
			if ( $b:ステージ番号 == 1 ) {
				// 敵が出ないor殺せない&最初からゴールが出ているタイプ
				$$:スコアタイプ = d:SCORE_TYPE_05;		//            TIME
			} else {
				// 的を全て壊すとゴールが出るタイプ
				$$:スコアタイプ = d:SCORE_TYPE_03;		// MAP SCORE  TIME  BULLET  COMBO
			}

		#elifdef d:STAGE_SP02A || d:STAGE_SP07A
			// 敵が出ないor殺せない&最初からゴールが出ているタイプ
			$$:スコアタイプ = d:SCORE_TYPE_05;			//            TIME

		#elifdef d:STAGE_SP03A || d:STAGE_SP06A
			// 敵を全滅させるとゴールが出るタイプ
			$$:スコアタイプ = d:SCORE_TYPE_02;			//            TIME  BULLET                   NO KILL

		#elifdef d:STAGE_SP08A
			// 敵を全滅させるとゴールが出るタイプ
			$$:スコアタイプ = d:SCORE_TYPE_04;			//            TIME  BULLET         SNEAKING  NO KILL

		#elifdef d:STAGE_SP21A
			// 的を全て壊すとゴールが出るタイプ
			$$:スコアタイプ = d:SCORE_TYPE_03;		// MAP SCORE  TIME  BULLET  COMBO

		#elifdef d:STAGE_SP22A || d:d:STAGE_SP23A || d:STAGE_SP24A || d:STAGE_SP25A
			$$:スコアタイプ = d:SCORE_TYPE_04;			//            TIME  BULLET         SNEAKING  NO KILL

		#else
			// デフォルトタイプ
			$$:スコアタイプ = d:SCORE_TYPE_05;			//			  TIME

		#endif

	//	テイルズＥの対ゴルルゴン戦のときのみ例外処理
	#ifdef d:STAGE_SP03A && d:E_TALES
		//	ＶＲクリアは起動しない
	#else
		// バラエティは特殊バージョン
		chara ＶＲクリア ＶＲクリア君 \
			-rate_a $$:秒変換レート	\			// $i:秒変換レート(フレーム単位)
			-rate_b $$:単発系武器変換レート	\	// $i:単発系武器変換レート
			-rate_c $$:連射系武器変換レート	\	// $i:連射系武器変換レート
			-rate_d $$:爆発系武器変換レート	\	// $i:爆発系武器変換レート
			-rate_e 0	\						// $i:コンボ変換レート(コンボは仕様変更によりなくなりました）
			-rate_f $$:隠密変換レート	\		// $i:隠密変換レート
			-rate_g $$:不殺変換レート	\		// $i:不殺変換レート

			-try_again テイルズＥ用ＲＥＴＲＹ \
			-exit load_tales_e_next \					//	次の紙芝居へ

			-score_type $$:スコアタイプ \
			-f ($$i:ＶＲクリアフラグ | d:VRCLR_NO_NEXTSTAGE | d:VRCLR_NO_SETSUMEI) \		//	スネークテイルズでは「ＮＥＸＴ　ＳＴＡＧＥ」出ない、クリアコード出ない
			-bgm $$:一位ジングル $$:二位ジングル $$:三位ジングル $$:圏外ジングル
		#endif
	}

	//ＶＲポーズも一緒に設定
	#ifdef d:STAGE_VR
		chara ＶＲポーズ ＶＲポーズ君 \
			-restart テイルズＥ用ＲＥＳＴＡＲＴ \
			-exit   load_tales_e_retry			//	やり直し紙芝居へ
	#else
		chara ＶＲ全体マップ表示３Ｄ ＶＲ全体マップ表示３Ｄ君 \
			-player				$$s:プレイヤ位置ラベル \
			#ifdef d:STAGE_TANKER
				-model			[A_全体マップ:タンカー] \
			#elifdef d:STAGE_PLANT
				#ifdef d:STAGE_W41A || d:STAGE_W42A || d:STAGE_W43A || d:STAGE_W44A || d:STAGE_W45A
					-model		[A_全体マップ:アーセナル] \
				#else
					-model		[A_全体マップ:プラント] \
				#endif
			#endif
			-active $i:A_全体マップ移動可能ステージ \
			-bomb $i:A_全体マップ爆弾設置ステージ \
			-restart テイルズＥ用ＲＥＳＴＡＲＴ \

			-exit load_tales_e_retry \					//	やり直し紙芝居へ

			-vr_pause	// ＶＲポーズ起動フラグ
	#endif


	#if d:DEBUG_PRINT
	print '###############################VR_GAMEOVER'
	#endif

	//	テイルズＥ用ゲームオーバー設定
	command セットＶＲステータス d:VR_EXIT_TO_SPECIAL
	command 特殊ＶＲゲームオーバー -p load_tales_e_retry					//	やり直し紙芝居へ
}

//	スネークテイルズＥ用ＶＲクリア設定雷電テイルズ
//	以下の仕様である
/*
	クリア			「ＲＥＴＲＹ」			→		リトライ
	クリア			「ＥＸＩＴ」			→		正解紙芝居へ
	クリア			「ＮＥＸＴ ＳＴＡＧＥ」→		次の紙芝居へ

	ポーズ			「ＲＥＴＲＹ」	→	リトライ
	ポーズ			「ＥＸＩＴ」	→	正解紙芝居へ

	ゲームオーバー	「ＲＥＴＲＹ」	→	リトライ
	ゲームオーバー	「ＥＸＩＴ」	→	正解紙芝居へ
*/

proc テイルズＥ用ＶＲクリア設定雷電 {
	#if d:DEBUG_PRINT
		print 'tales_e_rai_vr_clear_set'
	#endif

	local $$:スコアタイプ;
	local $$:秒変換レート;
	local $$:単発系武器変換レート;
	local $$:連射系武器変換レート;
	local $$:爆発系武器変換レート;
	local $$:隠密変換レート;
	local $$:不殺変換レート;
	local $$:一位ジングル;
	local $$:二位ジングル;
	local $$:三位ジングル;
	local $$:圏外ジングル;

	if ( $b:ミッション番号 == d:MISSION:爆弾解体 ) {
		$$:秒変換レート = d:SCORE_RATE_A_BOMB;
		$$:単発系武器変換レート = d:SCORE_RATE_B_BOMB;
		$$:連射系武器変換レート = d:SCORE_RATE_C_BOMB;
		$$:爆発系武器変換レート = d:SCORE_RATE_D_BOMB;
		$$:隠密変換レート = d:SCORE_RATE_F_BOMB;
		$$:不殺変換レート = d:SCORE_RATE_G_BOMB;
	} else {
		$$:秒変換レート = d:SCORE_RATE_A;
		$$:単発系武器変換レート = d:SCORE_RATE_B;
		$$:連射系武器変換レート = d:SCORE_RATE_C;
		$$:爆発系武器変換レート = d:SCORE_RATE_D;
		$$:隠密変換レート = d:SCORE_RATE_F;
		$$:不殺変換レート = d:SCORE_RATE_G;
	}


	#ifdef d:STAGE_VR
		if ( $s:選択プレイヤー == ライデン || $s:選択プレイヤー == 刀ライデン || $s:選択プレイヤー == 裸ライデン ) {
			$$:一位ジングル = t:vc030215;
			$$:二位ジングル = t:vc030216;
			$$:三位ジングル = t:vc030217;
			$$:圏外ジングル = t:vc030218;

		} else if ( $s:選択プレイヤー == スネーク || $s:選択プレイヤー == プリスキン || $s:選択プレイヤー == タキシードスネーク ) {
			$$:一位ジングル = t:vc030211;
			$$:二位ジングル = t:vc030212;
			$$:三位ジングル = t:vc030213;
			$$:圏外ジングル = t:vc030214;

		} else {
			// 前作スネーク
			$$:一位ジングル = t:vc030241;
			$$:二位ジングル = t:vc030242;
			$$:三位ジングル = t:vc030243;
			$$:圏外ジングル = t:vc030244;

		}
	#else
		if ( $s:選択プレイヤー == ライデン || $s:選択プレイヤー == 刀ライデン || $s:選択プレイヤー == 裸ライデン ) {
			$$:一位ジングル = t:vc030235;
			$$:二位ジングル = t:vc030236;
			$$:三位ジングル = t:vc030237;
			$$:圏外ジングル = t:vc030238;

		} else {
			// スネーク系
			$$:一位ジングル = t:vc030231;
			$$:二位ジングル = t:vc030232;
			$$:三位ジングル = t:vc030233;
			$$:圏外ジングル = t:vc030234;

		}
	#endif

	#if d:MGS2_VRTRIAL	//chara ＶＲクリア
		$$i:ＶＲクリアフラグ = ( $$i:ＶＲクリアフラグ | d:VRCLR_VRTRIAL );
	#endif

	if ( $b:ミッション番号 != d:MISSION:バラエティ && $b:ミッション番号 != d:MISSION:主観モード ) {
		// 通常バージョン
		chara ＶＲクリア ＶＲクリア君 \
			-rate_a $$:秒変換レート	\			// $i:秒変換レート(フレーム単位)
			-rate_b $$:単発系武器変換レート	\	// $i:単発系武器変換レート
			-rate_c $$:連射系武器変換レート	\	// $i:連射系武器変換レート
			-rate_d $$:爆発系武器変換レート	\	// $i:爆発系武器変換レート
			-rate_e 0	\						// $i:コンボ変換レート(コンボは仕様変更によりなくなりました）
			-rate_f $$:隠密変換レート	\		// $i:隠密変換レート
			-rate_g $$:不殺変換レート	\		// $i:不殺変換レート

			-next_stage load_tales_e_next \				//	次の紙芝居へ
			-try_again テイルズＥ用ＲＥＴＲＹ \
			-exit load_tales_e_right \					//	正解紙芝居へ

			-f ($$i:ＶＲクリアフラグ | d:VRCLR_YES_NEXTSTAGE | d:VRCLR_NO_SETSUMEI) \		//	雷電テイルズでは「ＮＥＸＴ　ＳＴＡＧＥ」出る、クリアコード出ない
			-bgm $$:一位ジングル $$:二位ジングル $$:三位ジングル $$:圏外ジングル

	} else {
		// バラエティと主観モードはステージごとにスコアタイプを設定
		#ifdef d:STAGE_SP01A
			if ( $b:ステージ番号 == 1 ) {
				// 敵が出ないor殺せない&最初からゴールが出ているタイプ
				$$:スコアタイプ = d:SCORE_TYPE_05;		//            TIME
			} else {
				// 的を全て壊すとゴールが出るタイプ
				$$:スコアタイプ = d:SCORE_TYPE_03;		// MAP SCORE  TIME  BULLET  COMBO
			}

		#elifdef d:STAGE_SP02A || d:STAGE_SP07A
			// 敵が出ないor殺せない&最初からゴールが出ているタイプ
			$$:スコアタイプ = d:SCORE_TYPE_05;			//            TIME

		#elifdef d:STAGE_SP03A || d:STAGE_SP06A
			// 敵を全滅させるとゴールが出るタイプ
			$$:スコアタイプ = d:SCORE_TYPE_02;			//            TIME  BULLET                   NO KILL

		#elifdef d:STAGE_SP08A
			// 敵を全滅させるとゴールが出るタイプ
			$$:スコアタイプ = d:SCORE_TYPE_04;			//            TIME  BULLET         SNEAKING  NO KILL

		#elifdef d:STAGE_SP21A
			// 的を全て壊すとゴールが出るタイプ
			$$:スコアタイプ = d:SCORE_TYPE_03;		// MAP SCORE  TIME  BULLET  COMBO

		#elifdef d:STAGE_SP22A || d:d:STAGE_SP23A || d:STAGE_SP24A || d:STAGE_SP25A
			$$:スコアタイプ = d:SCORE_TYPE_04;			//            TIME  BULLET         SNEAKING  NO KILL

		#else
			// デフォルトタイプ
			$$:スコアタイプ = d:SCORE_TYPE_05;			//			  TIME

		#endif

		// バラエティは特殊バージョン
		chara ＶＲクリア ＶＲクリア君 \
			-rate_a $$:秒変換レート	\			// $i:秒変換レート(フレーム単位)
			-rate_b $$:単発系武器変換レート	\	// $i:単発系武器変換レート
			-rate_c $$:連射系武器変換レート	\	// $i:連射系武器変換レート
			-rate_d $$:爆発系武器変換レート	\	// $i:爆発系武器変換レート
			-rate_e 0	\						// $i:コンボ変換レート(コンボは仕様変更によりなくなりました）
			-rate_f $$:隠密変換レート	\		// $i:隠密変換レート
			-rate_g $$:不殺変換レート	\		// $i:不殺変換レート

			-next_stage load_tales_e_next \				//	次の紙芝居へ
			-try_again テイルズＥ用ＲＥＴＲＹ \
			-exit load_tales_e_right \					//	正解紙芝居へ

			-score_type $$:スコアタイプ \
			-f ($$i:ＶＲクリアフラグ | d:VRCLR_YES_NEXTSTAGE | d:VRCLR_NO_SETSUMEI) \		//	雷電テイルズでは「ＮＥＸＴ　ＳＴＡＧＥ」出る、クリアコード出ない
			-bgm $$:一位ジングル $$:二位ジングル $$:三位ジングル $$:圏外ジングル

	}

	//ＶＲポーズも一緒に設定
	#ifdef d:STAGE_VR
		chara ＶＲポーズ ＶＲポーズ君 \
			-restart テイルズＥ用ＲＥＳＴＡＲＴ \
			-exit load_tales_e_right \			//	正解紙芝居へ
	#else
		chara ＶＲ全体マップ表示３Ｄ ＶＲ全体マップ表示３Ｄ君 \
			-player				$$s:プレイヤ位置ラベル \
			#ifdef d:STAGE_TANKER
				-model			[A_全体マップ:タンカー] \
			#elifdef d:STAGE_PLANT
				#ifdef d:STAGE_W41A || d:STAGE_W42A || d:STAGE_W43A || d:STAGE_W44A || d:STAGE_W45A
					-model		[A_全体マップ:アーセナル] \
				#else
					-model		[A_全体マップ:プラント] \
				#endif
			#endif
			-active $i:A_全体マップ移動可能ステージ \
			-bomb $i:A_全体マップ爆弾設置ステージ \
			-restart テイルズＥ用ＲＥＳＴＡＲＴ \

			-exit load_tales_e_right \			//	正解紙芝居へ

			-vr_pause	// ＶＲポーズ起動フラグ
	#endif

	#if d:DEBUG_PRINT
	print '###############################VR_GAMEOVER'
	#endif

	//	テイルズＥ用ゲームオーバー設定
	command セットＶＲステータス d:VR_EXIT_TO_SPECIAL
	command 特殊ＶＲゲームオーバー -p load_tales_e_right			//	正解紙芝居へ

}

//	テイルズＥ用リトライ／リスタートプロック
proc テイルズＥ用ＲＥＴＲＹ {
	#if d:DEBUG_PRINT
	print'###############################TALES_RETRY'
	#endif

	//	コンティニューをカウントする
	@コンティニューカウント
	@スネークテイルズＥ用変数戻し

//	爆弾解体のときは最初までロードする
#ifdef d:E_TALES
	print'###############################TALES_RETRY_LOADTYPE'
	#ifdef d:STAGE_W02A
		@mv_tales_ta02a_e0
	#elifdef d:STAGE_W31D
		@mv_tales_ta31a_e0
	#elifdef d:STAGE_W01F || d:STAGE_W01B || d:STAGE_W01C || d:STAGE_W01D || d:STAGE_W01E
		@mv_tales_ta01f_e0
	#endif

//	それ以外は
#else
	@A_ＲＥＴＲＹプロック
#endif

}

proc テイルズＥ用ＲＥＳＴＡＲＴ {
	#if d:DEBUG_PRINT
	print'###############################TALES_RESTART'
	#endif

	//	コンティニューをカウントする
	@コンティニューカウント
	@スネークテイルズＥ用変数戻し
	@A_ＲＥＳＴＡＲＴプロック
}



//	テイルズＥ用ゲームオーバー／コンティニュー設定

/*	ゲームオーバー時に使用。ＶＲ／オルタナのみではなく、テイルズＥ全てに適用すること！*/

proc テイルズＥ用ゲームオーバー処理 {
	/*	コンティニュー処理はリスタート後呼ばれるので
		その前のゲームオーバー処理の中で変数を戻す	*/
	@スネークテイルズＥ用変数戻し
}

proc テイルズＥ用コンティニュー処理 {
	#if d:DEBUG_PRINT
	print'###############################TALES_G_OVER_CONTINUE'
	#endif

//	爆弾解体のときはコンティニューするとタイムを加算する
#ifdef d:E_TALES

	#ifdef d:STAGE_W02A || d:STAGE_W31D || d:STAGE_W01F || d:STAGE_W01B || d:STAGE_W01C || d:STAGE_W01D || d:STAGE_W01E || d:STAGE_W00A || d:STAGE_W01A
	@A_爆弾タイマー加算処理
	#endif

#endif

	//	コンティニューをカウントする
	@コンティニューカウント
}

proc コンティニューカウント {
	//	コンティニューしたフラグを立てる
	$f:テイルズＥコンティニューフラグ = 1;
	//	変数戻しのときのためにローカル変数に退避する
	$$f:テイルズＥコンティニューフラグ = $f:テイルズＥコンティニューフラグ;

	#if d:DEBUG_PRINT
	print '$f:テイルズＥコンティニューフラグ' $f:テイルズＥコンティニューフラグ;
	print '$$f:テイルズＥコンティニューフラグ' $$f:テイルズＥコンティニューフラグ;
	#endif

	//	何回コンティニューしたか
	$i:ＶＲ設定 = $i:ＶＲ設定 + 1;

	#if d:DEBUG_PRINT
	print'###############################CONTINUE_COUNT' $i:ＶＲ設定
	#endif
}



//	テイルズＥ用デジカメ設定
proc スネークテイルズＥ用デジカメ設定 {
	command タンカーカメラステータス \
		-d $i:格納場所変数
}


//	テイルズＥ用対ゴルルゴン戦終了処理
proc スネークテイルズＥ用対ゴルルゴン戦終了処理 {
	if ( `command ゲームオーバーチェック` == 0 ) {
		command 無線設定 -reset
		command パッド操作 -release
		command プレイヤー無敵セット
		mesg コマンダー 敵兵セット 視界オフ
		// 紙芝居に入るときは敵兵情報を全てリセットする
		command 敵兵メモリーオールリセット

		chara delay フェードアウトディレイ \
			-t 90 \
			-exec {
				// フェードインスタート
				chara フェードインアウト フェードアウト \
					-c 0,0,0 128 \
					-t d:FADEOUT_TIME \
					-p load_tales_e_next
			}
	}
}

// MGS2Xで追加された共通プロック、ここまで
///////////////////////////////////////////////////////////////////////



#else


print 'a_stdproc.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif


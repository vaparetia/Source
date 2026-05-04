/*
	boss_stdproc.h
	    ボスラッシュモード用共通プロック（元となるファイルでincludeする）

	2001/09/26 H.Yoshiike
	$Id: boss_stdproc.h,v 1.31 2002/10/05 13:27:46 usr03682 Exp $


*/


// ファイルが二重呼びされたときの対処
#ifndef d:BOSS_STDPROC_H
#define BOSS_STDPROC_H	1

// ボスラッシュ用アイテム設定ファイル
#include "boss_item.h"

// 各proc設定
//-------------------------------------------------------------------------------------
	// ボスラッシュ専用共通キャラ（各gclで呼ぶ必要があるもの）
	// ------------------------------------
	proc ボスラッシュ開始時キャラ設定 {
		#if d:DEBUG_PRINT
			print 'boss_start_chara_set'
		#endif

		#ifdef d:BOSS_MODE
		// ボスラッシュモード本編は以下に記述すること
		// バージョン情報
		// ----------------------------------------------
		#ifndef d:PAL
			#ifdef d:KOREA
				eval( $b:バージョン情報 = 3 ) ;	// 韓国版
			#elseifdef d:ENGLISH
				eval( $b:バージョン情報 = 0 ) ;	// 英語版
			#else
				eval( $b:バージョン情報 = 1 ) ;	// 日本語版
			#endif
		#else
			eval( $b:バージョン情報 = 2 ) ;		// 欧州版
		#endif
		// ボスラッシュ用のフラグを立てる
		command ボスラッシュモードセット

		@ボスラッシュサウンド開始

		// ボス戦専用ステージキャラ
		chara ボスラッシュ２結果 リザルト画面 -n リザルト画面終了時プロック \
			-v	$b:バージョン情報 \
			-t	$i:経過時間[d:boss_no:B_OLGA] \
				$i:経過時間[d:boss_no:B_SOLDIERS] \
				$i:経過時間[d:boss_no:B_FATMAN] \
				$i:経過時間[d:boss_no:B_HARRIER] \
				$i:経過時間[d:boss_no:B_VAMP] \
				$i:経過時間[d:boss_no:B_TENGUS] \
				$i:経過時間[d:boss_no:B_RAY] \
				$i:経過時間[d:boss_no:B_SOLIDUS] \
			-d	$i:気絶フラグ[d:boss_no:B_OLGA] \
				$i:気絶フラグ[d:boss_no:B_SOLDIERS] \
				$i:気絶フラグ[d:boss_no:B_FATMAN] \
				$i:気絶フラグ[d:boss_no:B_HARRIER] \
				$i:気絶フラグ[d:boss_no:B_VAMP] \
				$i:気絶フラグ[d:boss_no:B_TENGUS] \
				$i:気絶フラグ[d:boss_no:B_RAY] \
				$i:気絶フラグ[d:boss_no:B_SOLIDUS] \
			-c	t:vc130000

		command ゲームオーバー呼びわけ -c 4			//ボスラッシュ用

		chara 六角フェード 六角フェード君 \
			-time	60 \
			-mode	0 \		//0:フェードin 1:フェードout
			-col	0, 0, 0 \
			-proc	ポーズ許可 \
			-ex								//これがあるとロードのフェードより強くなります

		@テロップ表示
		chara ボスタイマー timer -time $i:経過時間[d:boss_no:B_ALL] -mode 0 -status d:TMR_WAIT

		chara ボスラッシュポーズ ボスラッシュポーズ君 \
			-r ボスラッシュモードリスタート \
			-e タイトル画面へ戻る

		// 無線機は禁止
		command メニュー設定 -menu on -gage on -radar on -radio off -pause off

		#elifdef d:STAGE_W00B || d:STAGE_W03B || d:STAGE_W20C || d:STAGE_W25A || d:STAGE_W31C || d:STAGE_W44A || d:STAGE_W46A || d:STAGE_W61A
		// 元のファイル用のextern宣言
		extern command 変数初期化
		extern command	ポーズセット
		extern command	ポーズリセット
		extern command 字幕制御
		extern command ボスラッシュモードセット
		extern chara ボスラッシュ２結果
		extern command ゲームオーバー呼びわけ
		extern chara ボスラッシュ２テロップ表示
		extern chara ボスタイマー
		extern chara ボスラッシュポーズ
		extern command ゲームオーバーチェック
		extern command ボスセットタイマーステータス
		extern command ボスゲット残りタイム
		extern command ボスゲットラップタイム
		extern command ボスアンセットタイマーステータス
		extern command 配列セット
		extern chara 六角フェード
		extern command ロードサウンドパック
		extern command ボスラッシュ２結果後書き
		extern command ボスラッシュ２音声交換
		extern command ボスタイマーエンド
		extern command ボスラッシュポーズ表示非表示
		extern chara ２Ｄスプライト表示
			#ifdef d:STAGE_TANKER
				extern chara マルチウェイト髪の毛モデル
			#else
				extern chara ロープモデル３
			#endif
			#ifdef d:STAGE_W44A
				extern command カメラ視界チェック
			#endif
			#ifdef d:STAGE_W46A
				extern command プレイヤー無敵セット
			#endif
			#ifdef d:STAGE_W61A
				extern command ゲームオーバー処理開始
				extern command ゲームオーバー処理終了
			#endif
			#ifdef d:STAGE_W20C
				extern command タイマーエンド
			#endif
		#endif
	}

	proc ボスラッシュ終了時処理 {
		#if d:DEBUG_PRINT
			print 'boss_survival_end'
		#endif

		#ifdef d:BOSS_MODE
		if (`command ゲームオーバーチェック` == 0 ) {
			// すぐ操作を奪う
			command ボスラッシュポーズ表示非表示

			if ( $b:ボスラッシュステージ != d:boss_no:B_RAY ) {
				command パッド操作 -release
				command	ポーズセット
			} else {
				command パッド操作	-which 0 \
					-mask ( d:PAD_U | d:PAD_D | d:PAD_L | d:PAD_R | d:PAD_A | d:PAD_B | d:PAD_X | d:PAD_Y | \
							 d:PAD_L1 | d:PAD_L2 | d:PAD_R1 | d:PAD_R2 | d:PAD_STA | d:PAD_SEL | d:PAD_AL | d:PAD_AR )
				#ifdef d:STAGE_W46A
					command プレイヤー無敵セット
				#endif
				chara delay ポーズディレイ -time 30 -exec {
					command パッド操作 -release
					command	ポーズセット
				}
			}

			command セットサウンドコード -c d:SE_JOUCHUU_OFF
			command 字幕制御 -disable
			// ボス戦専用ステージキャラ
			command メニュー設定 -menu off -gage off -radar off -pause off
			// タイムの取得とkillカウントの計算
			command ボスセットタイマーステータス -s d:TMR_WAIT
			eval( $i:経過時間[d:boss_no:B_ALL] = `%ボスゲット残りタイム` );
			if ( $b:ボスラッシュステージ == d:boss_no:B_OLGA ) {
				eval( $i:経過時間[d:boss_no:B_OLGA] = `%ボスゲットラップタイム` );
				// WIN表示
				chara ボスラッシュ２テロップ表示 テロップ君 -m 2 -n ボスラッシュ終了プロック
				// 気絶で倒したかどうかチェック
				if ( $w:殺傷人数 == $w:ボスラッシュキルカウント ) {
					eval( $i:気絶フラグ[d:boss_no:B_OLGA] = 1 );
				} else {
					eval( $i:気絶フラグ[d:boss_no:B_OLGA] = 0 );
				}
				eval ( $w:ボスラッシュキルカウント = $w:殺傷人数 );

			} else if ( $b:ボスラッシュステージ == d:boss_no:B_SOLDIERS ) {
				eval( $i:経過時間[d:boss_no:B_SOLDIERS] = `%ボスゲットラップタイム` );
				// WIN表示
				chara ボスラッシュ２テロップ表示 テロップ君 -m 2 -n ボスラッシュ終了プロック
				// 気絶で倒したかどうかチェック
				if ( $w:殺傷人数 == $w:ボスラッシュキルカウント ) {
					eval( $i:気絶フラグ[d:boss_no:B_SOLDIERS] = 1 );
				} else {
					eval( $i:気絶フラグ[d:boss_no:B_SOLDIERS] = 0 );
				}
				eval ( $w:ボスラッシュキルカウント = $w:殺傷人数 );

			} else if ( $b:ボスラッシュステージ == d:boss_no:B_FATMAN ) {
				eval( $i:経過時間[d:boss_no:B_FATMAN] = `%ボスゲットラップタイム` );
				// WIN表示
				chara ボスラッシュ２テロップ表示 テロップ君 -m 2 -n ボスラッシュ終了プロック
				// 気絶で倒したかどうかチェック
				if ( $w:殺傷人数 == $w:ボスラッシュキルカウント ) {
					eval( $i:気絶フラグ[d:boss_no:B_FATMAN] = 1 );
				} else {
					eval( $i:気絶フラグ[d:boss_no:B_FATMAN] = 0 );
				}
				eval ( $w:ボスラッシュキルカウント = $w:殺傷人数 );

			} else if ( $b:ボスラッシュステージ == d:boss_no:B_HARRIER ) {
				eval( $i:経過時間[d:boss_no:B_HARRIER] = `%ボスゲットラップタイム` );
				// WIN表示
				chara ボスラッシュ２テロップ表示 テロップ君 -m 2 -n ボスラッシュ終了プロック
				// 気絶で倒したかどうかチェック(ハリアーは固定)
				eval( $i:気絶フラグ[d:boss_no:B_HARRIER] = 0 );
				eval ( $w:ボスラッシュキルカウント = $w:殺傷人数 );

			} else if ( $b:ボスラッシュステージ == d:boss_no:B_VAMP ) {
				eval( $i:経過時間[d:boss_no:B_VAMP] = `%ボスゲットラップタイム` );
				// WIN表示
				chara ボスラッシュ２テロップ表示 テロップ君 -m 2 -n ボスラッシュ終了プロック
				// 気絶で倒したかどうかチェック
				if ( $w:殺傷人数 == $w:ボスラッシュキルカウント ) {
					eval( $i:気絶フラグ[d:boss_no:B_VAMP] = 1 );
				} else {
					eval( $i:気絶フラグ[d:boss_no:B_VAMP] = 0 );
				}
				eval ( $w:ボスラッシュキルカウント = $w:殺傷人数 );

			} else if ( $b:ボスラッシュステージ == d:boss_no:B_TENGUS ) {
				eval( $i:経過時間[d:boss_no:B_TENGUS] = `%ボスゲットラップタイム` );
				// WIN表示
				chara ボスラッシュ２テロップ表示 テロップ君 -m 2 -n ボスラッシュ終了プロック
				// 気絶で倒したかどうかチェック
				if ( $w:殺傷人数 == $w:ボスラッシュキルカウント ) {
					eval( $i:気絶フラグ[d:boss_no:B_TENGUS] = 1 );
				} else {
					eval( $i:気絶フラグ[d:boss_no:B_TENGUS] = 0 );
				}
				eval ( $w:ボスラッシュキルカウント = $w:殺傷人数 );


			} else if ( $b:ボスラッシュステージ == d:boss_no:B_RAY ) {
				eval( $i:経過時間[d:boss_no:B_RAY] = `%ボスゲットラップタイム` );
				// WIN表示
				chara ボスラッシュ２テロップ表示 テロップ君 -m 2 -n ボスラッシュ終了プロック
				// 気絶で倒したかどうかチェック(ＲＡＹは固定)
				eval( $i:気絶フラグ[d:boss_no:B_RAY] = 0 );
				eval ( $w:ボスラッシュキルカウント = $w:殺傷人数 );

			} else if ( $b:ボスラッシュステージ == d:boss_no:B_SOLIDUS ) {
				eval( $i:経過時間[d:boss_no:B_SOLIDUS] = `%ボスゲットラップタイム` );
				// WIN表示
				chara ボスラッシュ２テロップ表示 テロップ君 -m 3 -n ボスラッシュ終了プロック
				// 気絶で倒したかどうかチェック
				if ( $w:殺傷人数 == $w:ボスラッシュキルカウント ) {
					eval( $i:気絶フラグ[d:boss_no:B_SOLIDUS] = 1 );
				} else {
					eval( $i:気絶フラグ[d:boss_no:B_SOLIDUS] = 0 );
				}
				eval ( $w:ボスラッシュキルカウント = $w:殺傷人数 );

				command ボスラッシュ２結果後書き \
					-a	$i:経過時間[d:boss_no:B_OLGA] \
					-m	$i:経過時間[d:boss_no:B_SOLDIERS] \
					-b	$i:経過時間[d:boss_no:B_FATMAN] \
					-c	$i:経過時間[d:boss_no:B_HARRIER] \
					-d	$i:経過時間[d:boss_no:B_VAMP] \
					-n	$i:経過時間[d:boss_no:B_TENGUS] \
					-e	$i:経過時間[d:boss_no:B_RAY] \
					-f	$i:経過時間[d:boss_no:B_SOLIDUS] \
					-g	$i:気絶フラグ[d:boss_no:B_OLGA] \
					-o	$i:気絶フラグ[d:boss_no:B_SOLDIERS] \
					-h	$i:気絶フラグ[d:boss_no:B_FATMAN] \
					-i	$i:気絶フラグ[d:boss_no:B_HARRIER] \
					-j	$i:気絶フラグ[d:boss_no:B_VAMP] \
					-p	$i:気絶フラグ[d:boss_no:B_TENGUS] \
					-k	$i:気絶フラグ[d:boss_no:B_RAY] \
					-l	$i:気絶フラグ[d:boss_no:B_SOLIDUS]

				command ボスラッシュ２音声交換 -c t:vc030120 
			}

			if ( $b:ボスラッシュステージ <= d:boss_no:B_RAY ) {
				chara 六角フェード 六角フェード君 \
					-time	90 \
					-mode	1 \		//0:フェードin 1:フェードout
					-col	0, 0, 0 \
					-ex								//これがあるとロードのフェードより強くなります
				command ボスタイマーエンド
			} else {
				chara 六角フェード 六角フェード君 \
					-time	90 \
					-mode	1 \		//0:フェードin 1:フェードout
					-col	255, 255, 255 \
					-ex								//これがあるとロードのフェードより強くなります
				command ボスタイマーエンド
			}
		}
		#endif

		#if d:DEBUG_PRINT
			print '$i:経過時間[d:boss_no:B_OLGA]='$i:経過時間[d:boss_no:B_OLGA]
			print '$i:経過時間[d:boss_no:B_SOLDIERS]='$i:経過時間[d:boss_no:B_SOLDIERS]
			print '$i:経過時間[d:boss_no:B_FATMAN]='$i:経過時間[d:boss_no:B_FATMAN]
			print '$i:経過時間[d:boss_no:B_HARRIER]='$i:経過時間[d:boss_no:B_HARRIER]
			print '$i:経過時間[d:boss_no:B_VAMP]='$i:経過時間[d:boss_no:B_VAMP]
			print '$i:経過時間[d:boss_no:B_TENGUS]='$i:経過時間[d:boss_no:B_TENGUS]
			print '$i:経過時間[d:boss_no:B_RAY]='$i:経過時間[d:boss_no:B_RAY]
			print '$i:経過時間[d:boss_no:B_SOLIDUS]='$i:経過時間[d:boss_no:B_SOLIDUS]
			print '$i:経過時間[d:boss_no:B_ALL]='$i:経過時間[d:boss_no:B_ALL]
			print '$i:気絶フラグ[d:boss_no:B_OLGA]='$i:気絶フラグ[d:boss_no:B_OLGA]
			print '$i:気絶フラグ[d:boss_no:B_SOLDIERS]='$i:気絶フラグ[d:boss_no:B_SOLDIERS]
			print '$i:気絶フラグ[d:boss_no:B_FATMAN]='$i:気絶フラグ[d:boss_no:B_FATMAN]
			print '$i:気絶フラグ[d:boss_no:B_HARRIER]='$i:気絶フラグ[d:boss_no:B_HARRIER]
			print '$i:気絶フラグ[d:boss_no:B_VAMP]='$i:気絶フラグ[d:boss_no:B_VAMP]
			print '$i:気絶フラグ[d:boss_no:B_TENGUS]='$i:気絶フラグ[d:boss_no:B_TENGUS]
			print '$i:気絶フラグ[d:boss_no:B_RAY]='$i:気絶フラグ[d:boss_no:B_RAY]
			print '$i:気絶フラグ[d:boss_no:B_SOLIDUS]='$i:気絶フラグ[d:boss_no:B_SOLIDUS]
		#endif

	}

	proc ボスラッシュプレイヤー設定 $:初期Ｘ $:初期Ｙ $:初期Ｚ $:初期方向 $:姿勢 $:スネークファイル $:ライデンファイル {
		#if d:DEBUG_PRINT
			print 'boss_survival_player_set'
			print '$w:体力最大='$w:体力最大'$w:体力='$w:体力
		#endif

		@ボスラッシュ用武器アイテム設定
		@ボスラッシュ初期装備設定

		if ( $s:ボスラッシュプレイヤー == スネーク ) {
			#ifdef d:STAGE_W00B
				chara プレイヤー d:PLAYER \
					-p $:初期Ｘ, $:初期Ｙ, $:初期Ｚ \
					-d 0,$:初期方向,0 \
					-s $:姿勢 \
					-homing $i:w00b_ホーミング距離 512 \
					-f (d:PLY_SPLASH | d:PLY_NO_FOOT_SHADOW | d:PLY_IS_SNAKE) \
					-m sna_def -a snake -o $:スネークファイル
			#else
				chara プレイヤー d:PLAYER \
					-p $:初期Ｘ, $:初期Ｙ, $:初期Ｚ \
					-d 0,$:初期方向,0 \
					-s $:姿勢 \
					-f d:PLY_IS_SNAKE \
					-m sna_def -a snake -o $:スネークファイル
			#endif

			chara ロープモデル３ バンダナ１ \
				-n sna_bdn1 \
				-d 1 \
				-y スネーク \
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
				-y スネーク \
				-z 12 \
				-x 0,50,-90 \
				-m 22 \
				-p 250 \
				-k 350 \
				-o 110 \
				-f 1

			chara ＬＯＤ制御 プレイヤーＬＯＤ \
				-name d:PLAYER \
				-type 2 \
				-model sna_def_sh \
				-value d:TRP_STATE_BEHIND \
				-value2 7000
		} else {
			#ifdef d:STAGE_W00B
				chara プレイヤー d:PLAYER \
					-p $:初期Ｘ, $:初期Ｙ, $:初期Ｚ \
					-d 0,$:初期方向,0 \
					-s $:姿勢 \
					-homing $i:w00b_ホーミング距離 512 \
					-f (d:PLY_SPLASH | d:PLY_NO_FOOT_SHADOW | d:PLY_IS_RAIDEN) \
					-m rai_def -a raiden -o $:ライデンファイル
			#else
				chara プレイヤー d:PLAYER \
					-p $:初期Ｘ, $:初期Ｙ, $:初期Ｚ \
					-d 0,$:初期方向,0 \
					-s $:姿勢 \
					-f d:PLY_IS_RAIDEN \
					-m rai_def -a raiden -o $:ライデンファイル
			#endif
			// 髪の毛
			@ライデン髪の毛設定 d:NORMAL_HAIR

			chara ＬＯＤ制御 プレイヤーＬＯＤ \
				-name d:PLAYER \
				-type 2 \
				-model rai_def_sh_mt \
				-value d:TRP_STATE_BEHIND \
				-value2 7000
		}

		#ifdef d:STAGE_W44A
			//---------------------------------------------------------
			// 白息
			chara 白息 プレイヤー息 \
				-name     d:PLAYER \
				-object   12 \
				-shift    0,-30,130 \
				-interval 136
			mesg 白息 プレイヤー息 on

			//---------------------------------------------------------
			// 亀甲床
			chara 亀甲床 亀甲床 \
				-prims  128 \
				-alpha  7 \
				-vertex 5 \
				-user   64 \
				-time   150
		#endif

		#ifdef d:STAGE_W61A
			chara Ｏ２ゲージ Ｏ２ -max 3500 -length d:O2_GAGE
		#endif

	}



	// ボスラッシュ専用共通キャラから呼ばれるプロック
	// ------------------------------------
	proc 次のステージをロード {
		#if d:DEBUG_PRINT
			print 'stage_load stage_no='$b:ボスラッシュステージ
		#endif

		if ( $b:ボスラッシュステージ == d:boss_no:B_OLGA ) {
			load 'w00b' -change boss -no_save 1
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_SOLDIERS ) {
			command	ポーズリセット
			load 'w03b' -change boss -no_save 1
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_FATMAN ) {
			command	ポーズリセット
			load 'w20c' -change boss -no_save 1
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_HARRIER ) {
			eval($w:p_story = d:ST:P045_01_P01ハリアー登場１ポリゴンデモ１終了) ;
			eval($b:w25a_ステージ状態 = 1 )
			eval( $f:ハリアー戦中 = 1 ) ;
			command	ポーズリセット
			load 'w25a' -change boss -no_save 1
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_VAMP ) {
			command	ポーズリセット
			load 'w31c' -change boss -no_save 1
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_TENGUS ) {
			command	ポーズリセット
			load 'w44a' -change boss -no_save 1
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_RAY ) {
			command	ポーズリセット
			load 'w46a' -change boss -no_save 1
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_SOLIDUS ) {
			command	ポーズリセット
			load 'w61a' -change boss -no_save 1
		}
	}

	proc テロップ表示 {
		#if d:DEBUG_PRINT
			print 'telop_start!!!!!'
		#endif
		chara ボスラッシュ２テロップ表示 テロップ君 -m 1 -n ゲームスタート
	}

	proc ポーズ許可 {
		#if d:DEBUG_PRINT
			print 'pause_on!!!!!'
		#endif
		command メニュー設定 -pause on
	}

	proc ゲームスタート {
		#if d:DEBUG_PRINT
			print 'game_start!!!!!'
		#endif

		command パッド操作 -cancel
		command ボスアンセットタイマーステータス -s d:TMR_WAIT
	}

	proc ボスラッシュモードリスタート {
		#if d:DEBUG_PRINT
			print 'boss_mode_restart!!!!!!'
		#endif

		preseek 'boss'
		chara 六角フェード 中断六角フェード君 \
			-time	90 \
			-mode	1 \		//0:フェードin 1:フェードout
			-proc	ボスラッシュモードリスタート後処理 \
			-col	0, 0, 0 \
			-ex								//これがあるとロードのフェードより強くなります
		command ボスタイマーエンド
		command メニュー設定 -menu off -gage off -radar off
		command セットサウンドコード -c d:SNG_FOUTS_S
		command セットサウンドコード -c d:SE_JOUCHUU_OFF
		command ボスラッシュポーズ表示非表示
		command	ポーズセット
		command 字幕制御 -disable

		#ifdef d:STAGE_W20C
			command タイマーエンド
		#endif
	}


#define	リスタートしたらオルガ戦	1
	proc ボスラッシュモードリスタート後処理 {
		command	ポーズリセット

		#if d:リスタートしたらオルガ戦
			load 'boss' -change scenerio
		#else
			// リスタートで各ステージの先頭に飛ばす場合はこっち
			eval( $i:経過時間[d:boss_no:B_ALL] = `%ボスゲット残りタイム` );

			if ( $b:ボスラッシュステージ == d:boss_no:B_OLGA ) {
				load 'w00b' -change boss -no_save 1
			} else if ( $b:ボスラッシュステージ == d:boss_no:B_SOLDIERS ) {
				command	ポーズリセット
				load 'w03b' -change boss -no_save 1
			} else if ( $b:ボスラッシュステージ == d:boss_no:B_FATMAN ) {
				command	ポーズリセット
				load 'w20c' -change boss -no_save 1
			} else if ( $b:ボスラッシュステージ == d:boss_no:B_HARRIER ) {
				eval($w:p_story = d:ST:P045_01_P01ハリアー登場１ポリゴンデモ１終了) ;
				eval($b:w25a_ステージ状態 = 1 )
				command	ポーズリセット
				load 'w25a' -change boss -no_save 1
			} else if ( $b:ボスラッシュステージ == d:boss_no:B_VAMP ) {
				command	ポーズリセット
				load 'w31c' -change boss -no_save 1
			} else if ( $b:ボスラッシュステージ == d:boss_no:B_TENGUS ) {
				command	ポーズリセット
				load 'w44a' -change boss -no_save 1
			} else if ( $b:ボスラッシュステージ == d:boss_no:B_RAY ) {
				command	ポーズリセット
				load 'w46a' -change boss -no_save 1
			} else if ( $b:ボスラッシュステージ == d:boss_no:B_SOLIDUS ) {
				command	ポーズリセット
				load 'w61a' -change boss -no_save 1
			}
		#endif
	}

	proc タイトル画面へ戻る {

		preseek 'r_title'
		chara 六角フェード 中断六角フェード君 \
			-time	90 \
			-mode	1 \		//0:フェードin 1:フェードout
			-proc	タイトル画面ロード \
			-col	0, 0, 0 \
			-ex								//これがあるとロードのフェードより強くなります
		command ボスタイマーエンド
		command メニュー設定 -menu off -gage off -radar off
		command セットサウンドコード -c d:SNG_FOUTS_S
		command セットサウンドコード -c d:SE_JOUCHUU_OFF
		command ボスラッシュポーズ表示非表示
		command	ポーズセット
		command 字幕制御 -disable

		#ifdef d:STAGE_W20C
			command タイマーエンド
		#endif
	}

	proc タイトル画面ロード {
		command	ポーズリセット
		@mv_init_n_title_0
	}

	proc リザルト画面終了時プロック {
	}

	proc ボスラッシュサウンド開始 {
		if ( $b:ボスラッシュステージ == d:boss_no:B_OLGA ) {
			if ( $s:ボスラッシュプレイヤー == スネーク ) {
				command ロードサウンドパック -p 0xE
			} else {
				#if d:DEBUG_PRINT
					print 'ロードサウンドパック -p 0xF'
				#endif
				command ロードサウンドパック -p 0xF
			}
			chara サウンドマネージャー ＳＤマネ -pak 0
			mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_01
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_SOLDIERS ) {
			chara サウンドマネージャー ＳＤマネ -pak 1
			mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_01
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_FATMAN ) {
			if ( $s:ボスラッシュプレイヤー == スネーク ) {
				command ロードサウンドパック -p 0xE
			} else {
				#if d:DEBUG_PRINT
					print 'ロードサウンドパック -p 0xF'
				#endif
				command ロードサウンドパック -p 0xF
			}
			chara サウンドマネージャー ＳＤマネ -pak 0
			mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_01
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_HARRIER ) {
			chara サウンドマネージャー ＳＤマネ -pak 0
			mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_01
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_VAMP ) {
			if ( $s:ボスラッシュプレイヤー == スネーク ) {
				chara サウンドマネージャー ＳＤマネ -pak 1
			} else {
				chara サウンドマネージャー ＳＤマネ -pak 0
			}
			mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_01
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_TENGUS ) {
			chara サウンドマネージャー ＳＤマネ -pak 1
			mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_01
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_RAY ) {
			chara サウンドマネージャー ＳＤマネ -pak 0
			mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_01
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_SOLIDUS ) {
			chara サウンドマネージャー ＳＤマネ -pak 0
			mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_01
		}

		eval( $f:ＢＧＭマネージャー起動フラグ = 0 );
		@サウンド効果音設定
	}

	proc ボスラッシュ終了プロック {
		#if d:DEBUG_PRINT
			print 'boss_battle_end'
		#endif

		command セットサウンドコード -c d:SNG_FOUTS_S

		if ( $b:ボスラッシュステージ <= d:boss_no:B_RAY ) {
			eval ( $b:ボスラッシュステージ = $b:ボスラッシュステージ + 1 );

			if ( $b:ボスラッシュステージ == d:boss_no:B_SOLDIERS ) {
				preseek 'w03b'
			} else if ( $b:ボスラッシュステージ == d:boss_no:B_FATMAN ) {
				preseek 'w20c'
			} else if ( $b:ボスラッシュステージ == d:boss_no:B_HARRIER ) {
				preseek 'w25a'
			} else if ( $b:ボスラッシュステージ == d:boss_no:B_VAMP ) {
				preseek 'w31c'
			} else if ( $b:ボスラッシュステージ == d:boss_no:B_TENGUS ) {
				preseek 'w44a'
			} else if ( $b:ボスラッシュステージ == d:boss_no:B_RAY ) {
				preseek 'w46a'
			} else if ( $b:ボスラッシュステージ == d:boss_no:B_SOLIDUS ) {
				preseek 'w61a'
			}

			if ( $s:ボスラッシュプレイヤー == スネーク ) {
				chara ボスラッシュ２テロップ表示 テロップ君 -m 0 -b $b:ボスラッシュステージ -n 次のステージをロード -p 0
			} else {
				chara ボスラッシュ２テロップ表示 テロップ君 -m 0 -b $b:ボスラッシュステージ -n 次のステージをロード -p 1
			}

		} else {
		// ソリダス戦後はこっち
			preseek 'r_title'
			command ゲームオーバー処理開始
			command ゲームオーバー処理終了
		}
	}

	proc ボスラッシュ用武器アイテム設定 {
		#if d:DEBUG_PRINT
			print 'boss_item_set'
		#endif

		if ( $b:ボスラッシュステージ == d:boss_no:B_OLGA ) {
			if ( $w:ゲーム設定 == d:LEVEL_VERYEASY ) {
				eval($w:武器弾数[d:武器:Ｍ９] = 46);
				eval($w:武器弾数Ｒ[d:武器:Ｍ９] = 46);
			} else if ( $w:ゲーム設定 == d:LEVEL_EASY || $w:ゲーム設定 == d:LEVEL_NORMAL ) {
				eval($w:武器弾数[d:武器:Ｍ９] = 31);
				eval($w:武器弾数Ｒ[d:武器:Ｍ９] = 31);
			} else {
				eval($w:武器弾数[d:武器:Ｍ９] = 16);
				eval($w:武器弾数Ｒ[d:武器:Ｍ９] = 16);
			}

		} else if ( $b:ボスラッシュステージ == d:boss_no:B_SOLDIERS ) {
			if ( $w:ゲーム設定 == d:LEVEL_VERYEASY ) {
				eval($w:武器弾数[d:武器:ＵＳＰ] = 46);
				eval($w:武器弾数Ｒ[d:武器:ソコム] = 61);
			} else if ( $w:ゲーム設定 == d:LEVEL_EASY || $w:ゲーム設定 == d:LEVEL_NORMAL ) {
				eval($w:武器弾数[d:武器:ＵＳＰ] = 16);
				eval($w:武器弾数Ｒ[d:武器:ソコム] = 13);
			} else {
				eval($w:武器弾数[d:武器:ＵＳＰ] = 0);
				eval($w:武器弾数Ｒ[d:武器:ソコム] = 0);
			}

			eval($w:アイテム数[d:アイテム:ＵＳＰサプレッサ] = 1);
			eval($w:アイテム数Ｒ[d:アイテム:ソコムサプレッサ] = 1);
			eval($w:アイテム数[d:アイテム:サーマルゴーグル] = 1);
			eval($w:アイテム数Ｒ[d:アイテム:サーマルゴーグル] = 1);

		} else if ( $b:ボスラッシュステージ == d:boss_no:B_FATMAN ) {
			eval($w:武器弾数[d:武器:凍結スプレー] = 1);
			eval($w:武器弾数Ｒ[d:武器:凍結スプレー] = 1);
			eval($w:アイテム数[d:アイテム:センサーＡ] = 1);
			eval($w:アイテム数Ｒ[d:アイテム:センサーＡ] = 1);

		} else if ( $b:ボスラッシュステージ == d:boss_no:B_HARRIER ) {
			if ( $w:ゲーム設定 == d:LEVEL_VERYEASY ) {
				eval($w:武器弾数Ｒ[d:武器:Ｍ４] = 181);
				eval($w:武器弾数Ｒ[d:武器:ＡＫＳ] = 181);
				eval($w:武器弾数Ｒ[d:武器:ＰＳＧ１] = 41);
				eval($w:武器弾数Ｒ[d:武器:ＰＳＧ１−Ｔ] = 51);
				eval($w:武器弾数Ｒ[d:武器:ＲＧＢ６] = 24);
			} else if ( $w:ゲーム設定 == d:LEVEL_EASY || $w:ゲーム設定 == d:LEVEL_NORMAL ) {
				eval($w:武器弾数Ｒ[d:武器:Ｍ４] = 31);
				eval($w:武器弾数Ｒ[d:武器:ＡＫＳ] = 31);
				eval($w:武器弾数Ｒ[d:武器:ＰＳＧ１] = 21);
				eval($w:武器弾数Ｒ[d:武器:ＰＳＧ１−Ｔ] = 11);
				eval($w:武器弾数Ｒ[d:武器:ＲＧＢ６] = 6);
			} else {
				eval($w:武器弾数Ｒ[d:武器:Ｍ４] = 0);
				eval($w:武器弾数Ｒ[d:武器:ＡＫＳ] = 0);
				eval($w:武器弾数Ｒ[d:武器:ＰＳＧ１] = 0);
				eval($w:武器弾数Ｒ[d:武器:ＰＳＧ１−Ｔ] = 0);
				eval($w:武器弾数Ｒ[d:武器:ＲＧＢ６] = 0);
			}

			eval($w:アイテム数Ｒ[d:アイテム:ＡＫサプレッサ] = 1);

		} else if ( $b:ボスラッシュステージ == d:boss_no:B_VAMP ) {
			if ( $s:ボスラッシュプレイヤー == スネーク ) {
				if ( $w:武器弾数[d:武器:スティンガー] == -1 ) {
					if ( $w:ゲーム設定 == d:LEVEL_VERYEASY ) {
						eval( $w:武器弾数[d:武器:スティンガー] = 40 );
					} else if ( $w:ゲーム設定 == d:LEVEL_EASY || $w:ゲーム設定 == d:LEVEL_NORMAL ) {
						eval( $w:武器弾数[d:武器:スティンガー] = 20 );
					} else {
						eval( $w:武器弾数[d:武器:スティンガー] = 0 );
					}
				}
			} else {
				if ( $w:武器弾数Ｒ[d:武器:スティンガー] == -1 ) {
					if ( $w:ゲーム設定 == d:LEVEL_VERYEASY ) {
						eval( $w:武器弾数Ｒ[d:武器:スティンガー] = 40 );
					} else if ( $w:ゲーム設定 == d:LEVEL_EASY || $w:ゲーム設定 == d:LEVEL_NORMAL ) {
						eval( $w:武器弾数Ｒ[d:武器:スティンガー] = 20 );
					} else {
						eval( $w:武器弾数Ｒ[d:武器:スティンガー] = 0 );
					}
				}
			}

			if ( $w:ゲーム設定 == d:LEVEL_VERYEASY ) {
				eval($w:武器弾数Ｒ[d:武器:ニキータ] = 40);
			} else if ( $w:ゲーム設定 == d:LEVEL_EASY || $w:ゲーム設定 == d:LEVEL_NORMAL ) {
				eval($w:武器弾数Ｒ[d:武器:ニキータ] = 10);
			} else {
				eval($w:武器弾数Ｒ[d:武器:ニキータ] = 0);
			}

			eval($w:アイテム数Ｒ[d:アイテム:暗視ゴーグル] = 1);
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_TENGUS ) {
			eval($w:武器弾数Ｒ[d:武器:ブレード] = 1);

		} else if ( $b:ボスラッシュステージ == d:boss_no:B_RAY ) {
		} else if ( $b:ボスラッシュステージ == d:boss_no:B_SOLIDUS ) {
		}
	}

	proc ボスラッシュ初期装備設定 {
		#if d:DEBUG_PRINT
			print 'player_weapon_item_set'
		#endif

		#ifdef d:BOSS_MODE
		if ( $b:ボスラッシュステージ <= d:boss_no:B_RAY ) {
			$w:プレイヤーフラグ = ( $w:プレイヤーフラグ & ~(d:PL_START_STATE_BLADE_OUT) );	// 刀はしまっている
			$w:プレイヤーフラグ = ( $w:プレイヤーフラグ & ~(d:PL_START_STATE_BLADE_INV) );	// 峰うちじゃない
			eval($w:武器 = d:武器:素手);
		} else {
			if ( $s:ボスラッシュプレイヤー == スネーク ) {
				eval($w:武器 = d:武器:素手);
			} else {
				eval($w:武器 = d:武器:ブレード);
			}
		}

		if ( $b:ボスラッシュステージ == d:boss_no:B_FATMAN ) {
			eval($w:アイテム = d:アイテム:センサーＡ);
		} else {
			eval($w:アイテム = d:アイテム:素手);
		}
		#endif
	}

#else


print 'boss_stdproc.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif



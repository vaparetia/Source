/*
	stdproc.h
	    各ファイル共通で使いそうなプロック

	2000/11/07 H.Yoshiike
	$Id: stdproc.h,v 1.270 2002/08/12 17:36:15 usr03682 Exp $


*/

#include "sound.h"
#ifdef d:STAGE_ALT
#include "a_vardef.h"		// オルタナティブとかでミッション番号等のdefineを使用しているため必要
#endif

// ファイルが二重呼びされたときの対処
#ifndef d:STDPROC_H
#define STDPROC_H	1



///////////////////////////////////////////////////////////////////////
// ノード端末設定（要 ノードモーション発動proc、トラップ名はnodeで固定）
///////////////////////////////////////////////////////////////////////
#ifdef d:NODE
#if d:NODE
proc ノード端末設定 $:方向 {

	@ノードエフェクト設置

	trap node d:PLAYER \
		-mask いる \
		-and \				//Andモード
		-dir $:方向,512 ($:方向+2048),512 \
		-key d:PAD_ACTION d:PAD_ACTION \
		-state d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-exec {
			#if d:DEBUG_PRINT
				print 'node_on'
			#endif

			if ( $w:アラートモード == d:ALERT_MODE_SENNYU || $w:アラートモード == d:ALERT_MODE_TANSAKU ) {
				command パッド操作 -release
				command メニュー設定 -menu off -gage off -radar off -subwin off -radio off -pause off
				mesg コマンダー 敵兵セット 視界オフ
				@ノード接続時プロック
				@ノード端末起動
/* 以下はモーションを組み込む場合の処理
				// モーションキャンセルの登録
				chara デモキャンセルチェック ノードモーションキャンセル \
					-p ノード端末起動

				command	強制モーション終了プロック \
					-proc ノード端末起動
*/
			} else {
				command ＳＥセットモード -s d:SD_S_BUZZER01 -p 0,0,0  -m 1
			}
		}
}

proc ノード端末起動 {
	#if d:DEBUG_PRINT
		print 'node_switch_on'
	#endif

	if ($f:ノードフラグ) {
		// 2回目以降はメニューから
		command ノード画面起動
	} else {
		// 初回はダウンロード画面へ
		command ノード画面起動 -map
	}

	chara delay ディレイ -time 1 -exec {
		mesg コマンダー 敵兵セット 視界オン
		command パッド操作 -cancel
		command メニュー設定 -menu on -gage on -radar on -subwin on -radio on -pause on
	}

/* 以下はモーションを組み込む場合の処理
	chara カメラ設定 モーションカメラ \
		-s 0
	command 強制モーションキャンセル
	mesg デモキャンセルチェック ノードモーションキャンセル kill
*/
}

proc ノードエフェクト設置 {
	#if d:DEBUG_PRINT
		print 'node_effect_set'
	#endif

	#ifdef d:STAGE_W11A
		chara ノード端末ディスプレイ NewNodeLamp -r -512	-p 3500 -45000 -9500
	#elifdef d:STAGE_W11B
		chara ノード端末ディスプレイ NewNodeLamp -r -512	-p 3500 -45000 -9500
	#elifdef d:STAGE_W11C
		chara ノード端末ディスプレイ NewNodeLamp -r -512	-p 3500 -45000 -9500

	#elifdef d:STAGE_W12B
		chara ノード端末ディスプレイ NewNodeLamp -r 0		-p 2250 0 -9250

	#elifdef d:STAGE_W14A
		chara ノード端末ディスプレイ NewNodeLamp -r 1024	-p -49200 -2000 -25400

	#elifdef d:STAGE_W16A
		chara ノード端末ディスプレイ NewNodeLamp -r 0		-p -61000 0 -105000
	#elifdef d:STAGE_W16B
		chara ノード端末ディスプレイ NewNodeLamp -r 0		-p -61000 0 -105000

	#elifdef d:STAGE_W18A
		chara ノード端末ディスプレイ NewNodeLamp -r -1024	-p 13250 -4000 -118500

	#elifdef d:STAGE_W20A
		chara ノード端末ディスプレイ NewNodeLamp -r 0		-p 58900 0 -101000

	#elifdef d:STAGE_W22A
		chara ノード端末ディスプレイ NewNodeLamp -r 0		-p 46000 -5000 -24250

	#elifdef d:STAGE_W24A
		chara ノード端末ディスプレイ NewNodeLamp -r -1024	-p 4625 0 -53400

	#elifdef d:STAGE_W24B
		chara ノード端末ディスプレイ NewNodeLamp -r -1024	-p 8250 -4500 -72900

	#elifdef d:STAGE_W24D
		chara ノード端末ディスプレイ NewNodeLamp -r 0		-p 4300 -16000 -59750

	#elifdef d:STAGE_W31A
		chara ノード端末ディスプレイ NewNodeLamp -r 0		-p -6600 0 -247750
	#elifdef d:STAGE_W31D
		chara ノード端末ディスプレイ NewNodeLamp -r 0		-p -6600 0 -247750

	#elifdef d:STAGE_W31B
		chara ノード端末ディスプレイ NewNodeLamp -r 0		-p 5000 -4000 -253000

	#elifdef d:STAGE_W31C
		chara ノード端末ディスプレイ NewNodeLamp -r -1024	-p -10750 -6250 -254000

	#elifdef d:STAGE_W41A
		chara ノード端末ディスプレイ NewNodeLamp -r 0		-p 5650 0 -8500

	#endif
}



#endif
#endif


///////////////////////////////////////////////////////////////////////
// レーダーオンオフチェック
///////////////////////////////////////////////////////////////////////
proc レーダーオンオフチェック $:オンオフフラグ {
	#if d:DEBUG_PRINT
		print 'radar_on_off_check'
	#endif

	if ($:オンオフフラグ == 1) {
		command メニュー設定 -node_access on
	} else {
		command メニュー設定 -node_access off
	}
}

///////////////////////////////////////////////////////////////////////
// 全体マップ設定
///////////////////////////////////////////////////////////////////////
// Tqnker
#define	船尾甲板マップ						tmap_w00a_kanpan
#define	船橋一階居住区マップ				tmap_w00b_okujyou
#define	船橋一階リフレッシュルームマップ	tmap_w01a_1f
#define	船橋二階居住区マップ				tmap_w01b_2f
#define	船橋三階居住区マップ				tmap_w01c_3f
#define	船橋四階居住区マップ				tmap_w01d_4f
#define	船橋五階操舵室マップ				tmap_w01e_5f
#define	機関室マップ						tmap_w02a_engineroom
#define	第二甲板マップ						tmap_w03a_nagarouka
#define	第一船倉マップ						tmap_w04a_sensou1
#define	第二船倉マップ						tmap_w04b_sensou2
#define	第三船倉マップ						tmap_w04c_sensou3
// Plant
#define	Ａ脚マップ				a_w12
#define	ＡＢ連絡橋マップ		ab_w13
#define	ＦＡ連絡橋マップ		af_w23
#define	Ｂ脚マップ				b_w14
#define	ＢＣ連絡橋マップ		bc_w15
#define	Ｃ脚マップ				c_w16
#define	ＣＤ連絡橋マップ		cd_w17
#define	Ｄ脚マップ				d_w18
#define	ＤＥ連絡橋マップ		de_w19
#define	ＤＧ連絡橋マップ		dg_w25
#define	Ｅ脚マップ				e_w20
#define	ＥＦ連絡橋マップ		ef_w21
#define	Ｆ脚マップ				f_w22
#define	Ｇ脚マップ				g_w25
#define	ＧＬ連絡橋マップ		gl_w25
#define	Ｌ脚マップ				l_w25
#define	オイルフェンスマップ	l_w32
#define	ＫＬ連絡橋マップ		lk_w25
#define	その他マップ			other
#define	シェル１中央棟マップ	s1_center_w24
#define	シェル２中央棟マップ	s2_center_w31
#define	アーセナル				bug
// koba4
#define Ｈ脚倉庫                h
#define ＧＨ連絡橋マップ        gh
#define アーセナルギア胃        w41a
#define アーセナルギア空腸      w42a
#define アーセナルギア上行結腸  w43a
#define アーセナルギア回腸      w44a
#define アーセナルギアＳ状結腸  w45a
#define アーセナルギア直腸      w46a
#define 麻布十番                AZABU
#define 船橋市                  HUNABASHI
#define 青山二丁目              AOYAMA
#define ニューヨーク52番街      NEWYORK


#include	"allmap_res.h"		// プラント編全体マップ定義リソース

proc プラント編全体マップ設定 $:現在位置ラベル {
	#if d:DEBUG_PRINT
		print 'node_switch_on'
	#endif

	chara 全体マップ表示3D プラント全体マップ \
		-player $:現在位置ラベル \
		-model [全体マップ:プラント]


	// 難易度別で最初からない爆弾のフラグはあらかじめ立てておく
	if ( $w:ゲーム設定 <= d:LEVEL_NORMAL ) {
		eval( $f:w12a_爆弾処理完了 = 1 );
		eval( $f:w18a_爆弾処理完了2 = 1 );
		eval( $f:w18a_爆弾処理完了3 = 1 );
		eval( $f:w20a_爆弾処理完了 = 1 );
	} else if ( $w:ゲーム設定 == d:LEVEL_HARD ) {
		eval( $f:w18a_爆弾処理完了3 = 1 );

	}

	command 全体マップ爆弾配置初期化
	//かならず最初に呼ぶこと、これをしないとXBOXで完全にバグる

	if ( $w:p_story >= d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 && \
		 $w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了 ) {
		if ( $f:w12a_爆弾処理完了 == 0 || $f:w12b_爆弾処理完了 == 0 ) {
			command 全体マップ爆弾配置 -bomb d:Ａ脚マップ
		}
		if ( $f:w14a_爆弾処理完了 == 0 ) {
			command 全体マップ爆弾配置 -bomb d:Ｂ脚マップ
		}
		if ( $f:w16b_爆弾処理完了 == 0 ) {
			command 全体マップ爆弾配置 -bomb d:Ｃ脚マップ
		}
		if ( $f:w18a_爆弾処理完了1 == 0 || $f:w18a_爆弾処理完了2 == 0 || $f:w18a_爆弾処理完了3 == 0 ) {
			command 全体マップ爆弾配置 -bomb d:Ｄ脚マップ
		}
		if ( $f:w20a_爆弾処理完了 == 0 || $f:w20b_爆弾処理完了 == 0 ) {
			command 全体マップ爆弾配置 -bomb d:Ｅ脚マップ
		}
		if ( $f:w22a_爆弾処理完了 == 0 ) {
			command 全体マップ爆弾配置 -bomb d:Ｆ脚マップ
		}
	} else if ( $w:p_story >= d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了 && \
				$w:p_story < d:ST:P026_01_R01爆弾解体終了１無線デモ１終了 ) {
		command 全体マップ爆弾配置 -bomb d:Ａ脚マップ
	} else if ( $w:p_story >= d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了 && \
				$w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１終了 ) {
		command 全体マップ爆弾配置 -bomb d:Ｅ脚マップ
	}

	if ( $w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１終了 ) {
		command 全体マップ選択可能範囲設定 -limit 0
	} else if ( $w:p_story < d:ST:P046_01_P01ハリアー戦勝利１ポリゴンデモ１終了 ) {
		// ピーター爆死後はＨが破壊
		command 全体マップ選択可能範囲設定 -limit 0
		command 全体マップ壊れ設定 -position d:Ｈ脚倉庫
	} else {
		// ハリアー戦後はＤＧ、Ｇが破壊
		command 全体マップ選択可能範囲設定 -limit 1
		command 全体マップ壊れ設定 -position d:ＤＧ連絡橋マップ
		command 全体マップ壊れ設定 -position d:Ｇ脚マップ
		command 全体マップ壊れ設定 -position d:ＧＨ連絡橋マップ
		command 全体マップ壊れ設定 -position d:Ｈ脚倉庫
	}
}

proc タンカー編全体マップ設定 $:現在位置ラベル {
	#if d:DEBUG_PRINT
		print 'node_switch_on'
	#endif

	chara 全体マップ表示3D タンカー全体マップ \
		-player $:現在位置ラベル \
		-model [全体マップ:タンカー]
}


///////////////////////////////////////////////////////////////////////
// ポーズ時ステージ名表示
///////////////////////////////////////////////////////////////////////
proc ポーズ時ステージ名表示設定 $:表示文字 {
	#if d:DEBUG_PRINT
		print 'stage_name_set'
	#endif

	chara ２Ｄスプライト表示＿ポーズ ポーズ時マップネーム表示 \
		-tri 2D_tex \
		-x $:表示文字 \
		-pos 256 181 \
		-alp 200 \
		-flag (d:SPRT2D_FLAG_CENTER | d:SPRT2D_FLAG_ALPHA)
}


///////////////////////////////////////////////////////////////////////
// スライドドア判定
///////////////////////////////////////////////////////////////////////
proc スライドドアオープンチェック $:ドア名 $:対称キャラ $:入出フラグ $:カードレベル {
	#if d:DEBUG_PRINT
		print 'slide_door_check'
	#endif

	if ( $:入出フラグ == 入る ) {
		if ($:対称キャラ != d:PLAYER) {
			// プレイヤー以外は無条件にopenメッセージ
			mesg ドア $:ドア名 open
		} else if (($w:アイテム == d:アイテム:カード) && ($w:アイテム最大数Ｒ[d:アイテム:カード] >= $:カードレベル) && \
			($f:ドアフラグ == 0)) {
			// プレイヤーは該当カードを持っているときに一回だけopenメッセージ
			eval( $f:ドアフラグ = 1);
			mesg ドア $:ドア名 open
		}
	} else if ( $:入出フラグ == 出る ) {
		if ($:対称キャラ != d:PLAYER) {
			// プレイヤー以外は無条件にcloseメッセージ
			mesg ドア $:ドア名 close
		} else if ($f:ドアフラグ == 1) {
			// プレイヤーopenメッセージが送られていたときだけ、closeメッセージ
			eval( $f:ドアフラグ = 0);
			mesg ドア $:ドア名 close
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// エレベーター
// 以下のdefineがそれぞれ必要です。
// d:ELEVATOR_SET	エレベーター設置を有効にするのに必要
// d:ELEVATOR_Y		エレベーターをＹ軸にいくつずらすか(１Ｆは0 シェル１Ｂ１は-4000 シェル２Ｂ１は-4500 Ｂ２は-12000 )
// d:ELEVATOR_Z		エレベーターをＺ軸にいくつずらすか(シェル１は0 シェル２は-180000)
// 1Fはd:ELV_1Fを、B1はd:ELV_B1を、B2はd:ELV_B2を定義してください
// モーションカメラを用意する必要があります。
// 他の階へ行くときのprocを用意する必要があります。
// 1Fは「強制ドアクローズ終了_B1へ」「強制ドアクローズ終了_B2へ」
// B1は「強制ドアクローズ終了_1Fへ」「強制ドアクローズ終了_B2へ」
// B2は「強制ドアクローズ終了_1Fへ」「強制ドアクローズ終了_B1へ」
//
// エレベータパネル用にd:SHELLを定義してください(シェル１は0、シェル２は1で定義)
// さらにw24aはd:ELE_CHECKを定義してください
/////////////////////////////////////////////////////////////////////////////
#ifdef d:ELEVATOR_SET
// モデルのdefine
#define	ELV2_KMS		elv_2
#define	ELV2_CEIL_KMS	elv_2r
#define	ELV2_DOOR_KMS1	elv_dr1
#define	ELV2_DOOR_KMS2	elv_dr2
// パネル用
#ifdef d:ELV_1F
#define	ELV_FLOOR		0
#else
#ifdef d:ELV_B1
#define	ELV_FLOOR		1
#else
#define	ELV_FLOOR		2
#endif
#endif

proc エレベーター設置 $:マップ名 {
	#if d:DEBUG_PRINT
		print 'elevator_set'
	#endif

	// エレベーター本体と天井はプットオブジェ置き
	chara プットオブジェ エレベーター -m d:ELV2_KMS -r 0,0,0 -p 0,(0+d:ELEVATOR_Y),(0+d:ELEVATOR_Z) -s 100,100,100 -l d:LT2_NAME
	chara 天井君 エレベーター天井 -m d:ELV2_CEIL_KMS -h (3750+d:ELEVATOR_Y) -p 0,(0+d:ELEVATOR_Y),(0+d:ELEVATOR_Z) -l d:LT2_NAME

	// エレベーターパネル本体
	chara エレベータパネル制御 エレベータパネル \
		-k d:SHELL \
		-i d:ELV_FLOOR \
		-r 2048 \
		-p -1500 (1700+d:ELEVATOR_Y) (-73525+d:ELEVATOR_Z) \
		-c 32 32 32

	// エレベーターランプ
	chara ドアランプ エレベーターランプ \
		-r 0 \
		-p 2000,(2250+d:ELEVATOR_Y),(-72750+d:ELEVATOR_Z) // 階数表示部分の中央の座標

	// エレベータープログラム本体
	chara エレベータ 中央棟エレベーター \
		-map $:マップ名 \
		-type d:SHELL \ // 0（１Ｆ・Ｂ１・Ｂ２） 1（１Ｆ・Ｂ１）
		-floor d:ELV_FLOOR \ // 上から順に、0,1,2
		-door1 d:ELV2_DOOR_KMS1 0,(0+d:ELEVATOR_Y),(-73250+d:ELEVATOR_Z) 2048 1200 32 \
		-door2 d:ELV2_DOOR_KMS2 1250,(0+d:ELEVATOR_Y),(-73250+d:ELEVATOR_Z) 2048 2400 32 \
		-wall -1250,(0+d:ELEVATOR_Y),(-73250+d:ELEVATOR_Z) 1250,(0+d:ELEVATOR_Y),(-73250+d:ELEVATOR_Z) 2500 \
		-close_time 20 \
		-trap elv pane \
		-camera -1540,(1456+d:ELEVATOR_Y),(-74648+d:ELEVATOR_Z) -1540,(1456+d:ELEVATOR_Y),(-74548+d:ELEVATOR_Z) \
		-panel_proc パネル決定 \
		-npc_name 手繋ぎエマ \
		-panel_chara_name エレベータパネル \
		-lamp_chara_name エレベーターランプ

	// プレイヤーとトラップ関係
	command プラグインエレベータ

	//操作パネル部分
	trap pane d:PLAYER \
		-state d:TRP_STATE_STAND \
		-dir 0,256 \
		-mask 入る \
		-exec {
			#ifdef d:WITH_EMMA
				// エマがいたら強制的に手を離す
				if ( $f:エマ存在フラグ == 1 ) {
					command ゲットエマ手繋ぎ状況 $i:ローカル変数Ｉ１
					if ( $i:ローカル変数Ｉ１ == 0 ) {
						// エマと手繋ぎ状態でない
						// 主観に操作画面に自動的に変更
						mesg プレイヤー d:PLAYER evpanel 入る 0
					}
				} else {
					mesg プレイヤー d:PLAYER evpanel 入る 0
				}
			#else
				// 主観に操作画面に自動的に変更
				mesg プレイヤー d:PLAYER evpanel 入る 0
			#endif
		}

	//エレベータドア押したときの処理
	trap elv_switch d:PLAYER \
		-mask 入る \
		-button d:ACTION_BUTTON \
		-dir 2048 512 \
		-state d:TRP_STATE_STAND, d:TRP_STATE_SQUAT \
		-exec {
//	オルタナティブミッション
#ifdef d:STAGE_ALT
			//	スネーク系はモーションがちょっとずれているのでカメラを変える
			if (($s:選択プレイヤー == スネーク) || ($s:選択プレイヤー == プリスキン) || \
				($s:選択プレイヤー == タキシードスネーク) || ($s:選択プレイヤー == 前作スネーク)) {
				chara カメラ設定 モーションカメラ \
					-c 1 \
	#ifdef d:STAGE_W24B	//野尻2001.9.9追加		//	ここはわからないのでとりあえずそのまま
					-p 1868,-498,-69719 -t 2008,-2873,-72248 \
	#else
					-p 1173,(2959+d:ELEVATOR_Y),(-69289+d:ELEVATOR_Z) -t 1839,(1404+d:ELEVATOR_Y),(-72368+d:ELEVATOR_Z) \
	#endif
					-r 299,1909,0 -f 3514 \
					-a 200 \
					-i 1 2 0 0 \
					-s 1
			
			//	ライデン系はそのまま
			} else {
				chara カメラ設定 モーションカメラ \
					-c 1 \
	#ifdef d:STAGE_W24B	//野尻2001.9.9追加
					-p 1868,-498,-69719 -t 2008,-2873,-72248 \
	#else
					-p -114,(3483+d:ELEVATOR_Y),(-69758+d:ELEVATOR_Z) -t 2012,(1077+d:ELEVATOR_Y),(-72475+d:ELEVATOR_Z) \
	#endif
					-r 397,1615,0 -f 4206 \
					-a 200 \
					-i 1 2 0 0 \
					-s 1
			}

//	本編
#else
			chara カメラ設定 モーションカメラ \
				-c 1 \
	#ifdef d:STAGE_W24B	//野尻2001.9.9追加
				-p 1868,-498,-69719 -t 2008,-2873,-72248 \
	#else
				-p -114,(3483+d:ELEVATOR_Y),(-69758+d:ELEVATOR_Z) -t 2012,(1077+d:ELEVATOR_Y),(-72475+d:ELEVATOR_Z) \
	#endif
				-r 397,1615,0 -f 4206 \
				-a 200 \
				-i 1 2 0 0 \
				-s 1
#endif

			command パッド操作 -release

			mesg プレイヤー d:PLAYER motion \
				d:モーションリスト:rai_non_call_evt \
				2048 2048 0 \
				( d:FA_USE_NOW_HEIGHT | d:FA_NO_WEAPON ) \			//フラグ
				1950 (-72175+d:ELEVATOR_Z)		//$i:プレイヤー位置Ｘ $i:プレイヤー位置Ｚ

			command	強制モーション終了プロック \
				-motion d:モーションリスト:rai_non_call_evt \
				-proc エレベータボタン押した終了
		}
}

// エレベーター内のボタンを押したときの処理
proc パネル決定 {
	#if d:DEBUG_PRINT
		print 'elevator_panel_push'
	#endif

	#ifdef d:STAGE_ALT
	// スネークテイルズ用にオルタナティブ系はちょっと修正
		if( $1 == 0 ) {
			#ifdef d:ELV_1F
				mesg プレイヤー d:PLAYER evpanel 出る 0
			#else
			// 1Fにロード
				if ( $b:ミッション番号 == d:MISSION:スネークテイルズ && \
					 $b:tales_story_no == d:TALES_NO:A && \
					 $b:tales_story_a == d:TALES_A:爆弾解体中 ) {

					mesg エレベータ 中央棟エレベーター tales_loadclose 強制ドアクローズ開始 強制ドアクローズ終了_1Fへ
				} else {
					mesg エレベータ 中央棟エレベーター loadclose 強制ドアクローズ開始 強制ドアクローズ終了_1Fへ
				}
			#endif
		} else if( $1 == 1 ) {
			#ifdef d:ELV_B1
				mesg プレイヤー d:PLAYER evpanel 出る 0
			#else
			// B1にロード
				if ( $b:ミッション番号 == d:MISSION:スネークテイルズ && \
					 $b:tales_story_no == d:TALES_NO:A && \
					 $b:tales_story_a == d:TALES_A:爆弾解体中 ) {
		
					mesg エレベータ 中央棟エレベーター tales_loadclose 強制ドアクローズ開始 強制ドアクローズ終了_B1へ
				} else {
					mesg エレベータ 中央棟エレベーター loadclose 強制ドアクローズ開始 強制ドアクローズ終了_B1へ
				}
			#endif
		} else if( $1 == 2 ) {
			#ifdef d:ELV_B2
				mesg プレイヤー d:PLAYER evpanel 出る 0
			#else
			// B2にロード
				mesg エレベータ 中央棟エレベーター loadclose 強制ドアクローズ開始 強制ドアクローズ終了_B2へ
			#endif
		}
	#else
	// 本編はこっち
		if( $1 == 0 ) {
			#ifdef d:ELV_1F
				mesg プレイヤー d:PLAYER evpanel 出る 0
			#else
			// 1Fにロード
				mesg エレベータ 中央棟エレベーター loadclose 強制ドアクローズ開始 強制ドアクローズ終了_1Fへ
			#endif
		} else if( $1 == 1 ) {
			#ifdef d:ELV_B1
				mesg プレイヤー d:PLAYER evpanel 出る 0
			#else
			// B1にロード
				mesg エレベータ 中央棟エレベーター loadclose 強制ドアクローズ開始 強制ドアクローズ終了_B1へ
			#endif
		} else if( $1 == 2 ) {
			#ifdef d:ELV_B2
				mesg プレイヤー d:PLAYER evpanel 出る 0
			#else
			// B2にロード
				mesg エレベータ 中央棟エレベーター loadclose 強制ドアクローズ開始 強制ドアクローズ終了_B2へ
			#endif
		}
	#endif
}

proc 強制ドアクローズ開始 {
	#if d:DEBUG_PRINT
		print 'elevator_door_close_start'
	#endif
	// ドアが閉まり始めたら強制的にパッドを奪う
	command パッド操作 -release
}


// エレベータ外のスイッチを押したときの処理
#ifndef d:ELE_CHECK
// w24a以外は変装チェックがないので共通化
proc エレベータボタン押した終了 {
	#if d:DEBUG_PRINT
		print 'elevator_door_push'
	#endif

	command パッド操作 -cancel
	chara カメラ設定 モーションカメラ -s -1
	mesg エレベータ 中央棟エレベーター open -1 エレベータードアオープン開始 エレベータードアオープン終了
}
#else
// w24aは以下に記述してもＯＫ
#endif

//エレベータ開き時プロック(今は特にいらない)
proc エレベータードアオープン開始 {
	#if d:DEBUG_PRINT
		print 'elevator_door_open_start'
	#endif
}

proc エレベータードアオープン終了 {
	#if d:DEBUG_PRINT
		print 'elevator_door_open_start'
	#endif
}

#endif



///////////////////////////////////////////////////////////////////////
// ライデン髪の毛セット
///////////////////////////////////////////////////////////////////////
#ifndef d:RAIDEN
	#define RAIDEN 0
#endif

#define	HAIR_MODEL	rai_hair_mh_mt
#define	BOUND_MODEL	rai_hair_bounding
#define	NORMAL_HAIR		0
#define	WATER_HAIR		1
#define	OUT_WATER_HAIR	2
#define	DRY_HAIR		3

//#if d:RAIDEN

proc ライデン髪の毛設定 $:タイプ {
	#if d:DEBUG_PRINT
		print 'raiden_hair_set'
	#endif

	if ($:タイプ == d:NORMAL_HAIR) {
		// 地上用
		chara マルチウェイト髪の毛モデル ライデン髪の毛 \
			-n d:HAIR_MODEL \
			-b d:BOUND_MODEL \
			-d 0 \
			-y d:PLAYER \
			-z 12 \
			-x 0,0,0 \
			-o 100 \
			-f 1 \
			-c 3 { 2 11 12 } \
			-q 0

		eval( $b:髪の毛パラメータ = d:NORMAL_HAIR );

	} else if ($:タイプ == d:WATER_HAIR) {
		// 水中用
		chara マルチウェイト髪の毛モデル ライデン髪の毛 \
			-n d:HAIR_MODEL \
			-b d:BOUND_MODEL \
			-d 1 \
			-y d:PLAYER \
			-z 12 \
			-x 0,0,0 \
			-o 100 \
			-f 1 \
			-c 3 { 2 11 12 } \
			-q 0 \
			-w 80 \
			-k 1000 \
			-a 400

		eval( $b:髪の毛パラメータ = d:WATER_HAIR );
	}
}

// 水中モードとの連動
// 水に入るとウエイトが変化し、出ると時間で変化していく
#define	HAIR_CHANGE_TIME	300

proc ライデン髪の毛水中モード連動設定 $:水中トラップ名 {
	#if d:DEBUG_PRINT
		print 'hair_change_set'
	#endif

	// 水と髪の毛の連動
	trap $:水中トラップ名 d:PLAYER \
		-mask ＊ \
		-exec {
			command プレイヤー状態取得
			if (( $status & d:PFLAG_WATER ) && (!( $status & d:PFLAG_WATER_SURFACE ))) {
				// 水中モードであるが水面ではない
				// 水に入ったら即水中髪の毛
				if ( $b:髪の毛パラメータ != d:WATER_HAIR ) {
					#if d:DEBUG_PRINT
						print 'hair_change_water_hair'
					#endif
					if ( $b:髪の毛パラメータ != d:NORMAL_HAIR ) {
						// 髪の毛ディレイが呼ばれているためディレイをカットする
						mesg delay 髪の毛ディレイ kill
					}
					mesg 髪の毛モデル ライデン髪の毛 パラメータチェンジ d:WATER_HAIR
					eval( $b:髪の毛パラメータ = d:WATER_HAIR );
				}
			} else {
				if ( $b:髪の毛パラメータ == d:WATER_HAIR ) {
					mesg 髪の毛モデル ライデン髪の毛 パラメータチェンジ d:OUT_WATER_HAIR
					eval( $b:髪の毛パラメータ = d:OUT_WATER_HAIR );

					chara delay 髪の毛ディレイ \
						-time d:HAIR_CHANGE_TIME \
						-exec {
							#if d:DEBUG_PRINT
								print 'hair_change_1'
								print $b:髪の毛パラメータ
							#endif

							if ( $b:髪の毛パラメータ == d:OUT_WATER_HAIR ) {
								// 水から出た直後からちょっと乾いてきた髪の毛に
								mesg 髪の毛モデル ライデン髪の毛 パラメータチェンジ d:DRY_HAIR
								eval( $b:髪の毛パラメータ = d:DRY_HAIR );
								return d:HAIR_CHANGE_TIME ;
							} else if ( $b:髪の毛パラメータ == d:DRY_HAIR ) {
								// ちょっと乾いてきた髪の毛から通常髪の毛に
								mesg 髪の毛モデル ライデン髪の毛 パラメータチェンジ d:NORMAL_HAIR
								eval( $b:髪の毛パラメータ = d:NORMAL_HAIR );
								return 0 ;
							}
						}
				}
			}
		}
}

//#endif



///////////////////////////////////////////////////////////////////////
// プレイヤーＬＯＤセット
///////////////////////////////////////////////////////////////////////
#ifndef d:LOD_KMS_NAME
	#ifdef d:STAGE_PLANT
			#define	LOD_KMS_NAME	rai_def_sh_mt
	#else
			#define	LOD_KMS_NAME	sna_def_sh
	#endif
#endif

#define	P_STATE_TYPE	0
#define	C_DISTANCE_TYPE	1

proc プレイヤーＬＯＤ設定 $:タイプ $:制御値 {
	#if d:DEBUG_PRINT
		print 'player_lod_set'
	#endif

	if ( $:タイプ == d:P_STATE_TYPE ) {
		chara ＬＯＤ制御 プレイヤーＬＯＤ \
			-name d:PLAYER \
			-type 2 \
			-model d:LOD_KMS_NAME \
			-value $:制御値 \
			-value2 7000

	} else {
		chara ＬＯＤ制御 プレイヤーＬＯＤ \
			-name d:PLAYER \
			-type 2 \
			-model d:LOD_KMS_NAME \
			-value d:TRP_STATE_BEHIND \
			-value2 $:制御値

	}

}



///////////////////////////////////////////////////////////////////////
// 特殊装備解除(主観系装備など付けていると問題が起きそうな装備を一括して解除するプロック)
///////////////////////////////////////////////////////////////////////
proc 特殊装備解除 {
	#if d:DEBUG_PRINT
		print 'wepon&item_release'
	#endif

	if ( $w:武器 == d:武器:ＰＳＧ１ || $w:武器 == d:武器:ＰＳＧ１－Ｔ || $w:武器 == d:武器:スティンガー || \
		$w:武器 == d:武器:マイク || $w:武器 == d:武器:特殊マイク || $w:武器 == d:武器:凍結スプレー ) {
		eval($w:武器 = d:武器:素手);
	}

	if ( $w:アイテム == d:アイテム:双眼鏡 || $w:アイテム == d:アイテム:デジカメ || \
		$w:アイテム == d:アイテム:タンカー編カメラ ) {
		eval($w:アイテム = d:アイテム:素手);
	}
}


// ダンボール系を解除
proc ダンボール解除 {
	#if d:DEBUG_PRINT
		print 'c_box_release'
	#endif

	if ($w:アイテム == d:アイテム:ダンボール || $w:アイテム == d:アイテム:ダンボールＢ || \
		$w:アイテム == d:アイテム:ダンボールＣ || $w:アイテム == d:アイテム:ダンボールＤ || \
		$w:アイテム == d:アイテム:ダンボールＥ || $w:アイテム == d:アイテム:濡れダンボール) {
			eval($w:アイテム = d:アイテム:素手);
	}
}

// デモ後の武器アイテムを制御する
// 武器アイテム共通define
#define	デモ後必ず素手		0
#define	デモ後前を継続		1
// 武器用define
#define	デモ後銃装備		2	// Ｍ９を持ってたらＭ９、そうでないときはソコムかＵＳＰ(所持してなかったらＭ９)
#define	デモ後銃か素手		3
// アイテム用define
#define	デモ後特殊装備なし						2	// 各種ゴーグル、ダンボール、主観系アイテムのときは素手にする
#define	デモ後特殊装備なし（ステルスもなし）	3	// 各種ゴーグル、ダンボール、主観系アイテムのときは素手にする

proc デモ後武器アイテム制御 $:武器タイプ $:アイテムタイプ {
	#if d:DEBUG_PRINT
		print 'after_demo weapon&item set'
	#endif

	// 武器の制御
	if ( $:武器タイプ == d:デモ後必ず素手 ) {
		eval($w:武器 = d:武器:素手);
	} else if ( $:武器タイプ == d:デモ後銃装備 ) {
		if ( $w:武器 != d:武器:Ｍ９ ) {
			#ifdef d:STAGE_TANKER
				if ( $w:武器弾数[ d:武器:ＵＳＰ ] >= 0 ) {
					eval( $w:武器 = d:武器:ＵＳＰ );
				} else {
					eval( $w:武器 = d:武器:Ｍ９ );
				}
			#else
				if ( $w:武器弾数Ｒ[ d:武器:ソコム ] >= 0 ) {
					eval( $w:武器 = d:武器:ソコム );
				} else {
					eval( $w:武器 = d:武器:Ｍ９ );
				}
			#endif
		}
	} else if ( $:武器タイプ == d:デモ後銃か素手 ) {
		if ( $w:武器 != d:武器:Ｍ９ && $w:武器 != d:武器:素手 ) {
			#ifdef d:STAGE_TANKER
				if ( $w:武器弾数[ d:武器:ＵＳＰ ] >= 0 ) {
					eval( $w:武器 = d:武器:ＵＳＰ );
				} else {
					eval( $w:武器 = d:武器:Ｍ９ );
				}
			#else
				if ( $w:武器弾数Ｒ[ d:武器:ソコム ] >= 0 ) {
					eval( $w:武器 = d:武器:ソコム );
				} else {
					eval( $w:武器 = d:武器:Ｍ９ );
				}
			#endif
		}
	}

	// アイテムの制御
	if ( $:アイテムタイプ == d:デモ後必ず素手 ) {
		eval($w:アイテム = d:アイテム:素手);
	} else if ( $:アイテムタイプ == d:デモ後特殊装備なし ) {
		if ( $w:アイテム == d:アイテム:暗視ゴーグル || $w:アイテム == d:アイテム:サーマルゴーグル || \
			 $w:アイテム == d:アイテム:ダンボール || $w:アイテム == d:アイテム:ダンボールＢ || \
			 $w:アイテム == d:アイテム:ダンボールＣ || $w:アイテム == d:アイテム:ダンボールＤ || \
			 $w:アイテム == d:アイテム:ダンボールＥ || $w:アイテム == d:アイテム:濡れダンボール || \
			 $w:アイテム == d:アイテム:双眼鏡 || $w:アイテム == d:アイテム:デジカメ || \
			 $w:アイテム == d:アイテム:タンカー編カメラ || $w:アイテム == d:アイテム:煙草 ) {
			eval($w:アイテム = d:アイテム:素手);
		}
	} else if ( $:アイテムタイプ == d:デモ後特殊装備なし（ステルスもなし） ) {
		if ( $w:アイテム == d:アイテム:暗視ゴーグル || $w:アイテム == d:アイテム:サーマルゴーグル || \
			 $w:アイテム == d:アイテム:ダンボール || $w:アイテム == d:アイテム:ダンボールＢ || \
			 $w:アイテム == d:アイテム:ダンボールＣ || $w:アイテム == d:アイテム:ダンボールＤ || \
			 $w:アイテム == d:アイテム:ダンボールＥ || $w:アイテム == d:アイテム:濡れダンボール || \
			 $w:アイテム == d:アイテム:双眼鏡 || $w:アイテム == d:アイテム:デジカメ || \
			 $w:アイテム == d:アイテム:タンカー編カメラ || $w:アイテム == d:アイテム:煙草 || \
			 $w:アイテム == d:アイテム:ステルス ) {
			eval($w:アイテム = d:アイテム:素手);
		}
	}
}



///////////////////////////////////////////////////////////////////////
// 50cm隙間用透明壁
///////////////////////////////////////////////////////////////////////
proc 東西５０ｃｍ幅透明壁設置 $:名前 $:左Ｘ座標 $:左Ｙ座標 $:左Ｚ座標 $:高さ {
	#if d:DEBUG_PRINT
		print 'tr_wall_set'
	#endif

	chara 透明壁 $:名前 \
			-position		$:左Ｘ座標,$:左Ｙ座標,$:左Ｚ座標,$:高さ \
							($:左Ｘ座標+500),$:左Ｙ座標,$:左Ｚ座標,$:高さ \
			-attribute		( d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_PLAYER ))

}

proc 南北５０ｃｍ幅透明壁設置 $:名前 $:上Ｘ座標 $:上Ｙ座標 $:上Ｚ座標 $:高さ {
	#if d:DEBUG_PRINT
		print 'tr_wall_set'
	#endif

	chara 透明壁 $:名前 \
			-position		$:上Ｘ座標,$:上Ｙ座標,$:上Ｚ座標,$:高さ \
							$:上Ｘ座標,$:上Ｙ座標,($:上Ｚ座標+500),$:高さ \
			-attribute		( d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_PLAYER ))

}


/////////////////////////////////////////////////////////////////////////////
// 天井設置(モデル名と高さとライトファイル名を指定)
/////////////////////////////////////////////////////////////////////////////
proc 天井設置 $:モデル名 $:高さ $:ライトファイル名 {
	#if d:DEBUG_PRINT
		print 'cail_set'
		print $:モデル名
		print $:ライトファイル名
	#endif

	chara 天井君 天井 -m $:モデル名 -h $:高さ -p 0,0,0 -l $:ライトファイル名
}


/////////////////////////////////////////////////////////////////////////////
// プレイヤーの入っているロッカー名を取得するプロック
/////////////////////////////////////////////////////////////////////////////
proc プレイヤーロッカーに入ったかチェック $:実行キャラ名 $:ロッカー状態 $:ロッカー名 {
	#if d:DEBUG_PRINT
		print 'locker_check'
		print $:実行キャラ名
		print $:ロッカー状態
		print $:ロッカー名
	#endif

	if ( $:実行キャラ名 == d:PLAYER ) {
		if ( $:ロッカー状態 == 3 ) {
			// 中から閉めた場合のみフラグを更新する
			eval( $s:プレイヤーがいるロッカー名 = $:ロッカー名 );
		} else {
			eval( $s:プレイヤーがいるロッカー名 = なし );
		}
	} else if ( $:ロッカー名 == $s:プレイヤーがいるロッカー名 ) {
		eval( $s:プレイヤーがいるロッカー名 = なし );
	}
}



/////////////////////////////////////////////////////////////////////////////
// シナリオデモ
/////////////////////////////////////////////////////////////////////////////

// シナリオデモに入ってもいいかチェック
proc シナリオデモ用安全チェック {

	command ゲットゲームステータス $i:ゲームステータス

	if( !( $i:ゲームステータス & d:STATE_DETECT ) && \
		( $w:アラートモード == d:ALERT_MODE_SENNYU ) && \
		(`command ゲームオーバーチェック` == 0 )) {
		return 1
	} else {
		return 0
	}
}

proc シナリオデモ用安全チェック（警戒モードでもＯＫ） {

	command ゲットゲームステータス $i:ゲームステータス

	if( !( $i:ゲームステータス & d:STATE_DETECT ) && \
		( $w:アラートモード == d:ALERT_MODE_SENNYU || $w:アラートモード == d:ALERT_MODE_TANSAKU ) && \
		(`command ゲームオーバーチェック` == 0 )) {
		return 1
	} else {
		return 0
	}
}

// シナリオデモ関係のプロックを使用するステージはこのプロックを呼んでおく事
proc シナリオデモキャラセット {
	#if d:DEBUG_PRINT
		print 'scenario_demo_chara_set'
	#endif

	chara シネマスクリーン シネマ -top d:CINEMA_BELT_UPPER -bottom d:CINEMA_BELT_LOWER -ftime d:C_FADE_TIME
}

proc 基本シナリオデモ開始処理 {
	command シナリオデモ開始
	command 無線設定 -reset
	command メニュー設定 -menu off -gage off -radar off -subwin off
	command パッド操作 -release
	command プレイヤー無敵セット
	mesg コマンダー 敵兵セット 視界オフ
	mesg コマンダー 敵兵セット 聴力オフ
	@特殊装備解除
}

// 通常のシナリオデモで使用する
proc シナリオデモ開始処理 {
	#if d:DEBUG_PRINT
		print 'scenario_demo_start'
	#endif

	@基本シナリオデモ開始処理
	mesg シネマスクリーン シネマ フェードアウト 0
}

proc シナリオデモ開始処理（ダンボールも解除） {
	#if d:DEBUG_PRINT
		print 'scenario_demo_start'
	#endif

	@シナリオデモ開始処理
	@ダンボール解除
}

// ストリームにつけたカメラデモで使用する(ストリームが流れる前に呼んでいい)
proc シナリオストリームデモ開始処理 {
	#if d:DEBUG_PRINT
		print 'scenario_stream_demo_start'
	#endif

	command シナリオデモ開始
	command 無線設定 -reset
	command パッド操作 -release
	command プレイヤー無敵セット
	mesg コマンダー 敵兵セット 視界オフ
	mesg コマンダー 敵兵セット 聴力オフ
	@特殊装備解除
}

// MAXのデータから呼び出してもらう
proc ストリームカメラ開始処理 {
	#if d:DEBUG_PRINT
		print 'cam_demo_start'
	#endif

	command メニュー設定 -menu off -gage off -radar off -subwin off
	mesg シネマスクリーン シネマ フェードアウト 0
}

// シナリオデモ・シナリオストリームデモの終了時に使用
proc 基本シナリオデモ終了処理 {
	command シナリオデモ終了
	command メニュー設定 -menu on -gage on -radar on -subwin on
	command パッド操作 -cancel
	command プレイヤー無敵解除
	mesg コマンダー 敵兵セット 視界オン
	mesg コマンダー 敵兵セット 聴力オン
}

proc シナリオデモ終了処理 {
	#if d:DEBUG_PRINT
		print 'scenario_demo_end'
	#endif
	@基本シナリオデモ終了処理
	mesg シネマスクリーン シネマ フェードイン 0
}

proc シナリオデモ安全チェック {
	command ゲットゲームステータス $i:ゲームステータス

	if( ( $i:ゲームステータス & d:STATE_DETECT ) \
					|| ( $w:アラートモード == d:ALERT_MODE_KIKEN ) \	// 発見から危険
					|| ( $w:アラートモード == d:ALERT_MODE_KAIHI ) \	// 危険
					|| ( $i:ゲームステータス & d:STATE_CHAFF ) ) {		// チャフ(シナリオデモ中はならないはず。念のため)

		#if d:DEBUG_PRINT
			print ( $i:ゲームステータス & d:STATE_DETECT ) ':発見から危険'
			print ( $w:アラートモード == d:ALERT_MODE_KIKEN ) ':危険'
			print ( $i:ゲームステータス & d:STATE_CHAFF ) ':チャフ'

			print '安全確認監視  危険です'
		#endif
		return 1;
	} else {
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
// ポリゴンデモ
/////////////////////////////////////////////////////////////////////////////
proc リスタート {
	#if d:DEBUG_PRINT
		print 'restart'
	#endif

	restart -s
}

proc 仮ポリデモ表示 {
	#if d:DEBUG_PRINT
		print 'debug_pause_print'
	#endif
	eval ( $w:p_story = $w:p_story + 1 );
	chara デバッグポーズ デバッグポーズ君 'poly_demo' -p リスタート
}

proc ポリゴンデモスタート $:ストーリーフラグ {
	#if d:DEBUG_PRINT
		print $:ストーリーフラグ
		print 'polydemo_start'
	#endif

	if ( `command ゲームオーバーチェック` == 0 ) {
		command 無線設定 -reset
		command パッド操作 -release
		command プレイヤー無敵セット
		mesg コマンダー 敵兵セット 視界オフ
		eval ( $w:p_story = $:ストーリーフラグ );
		eval( $f:デモ直後フラグ = 1 );

		// ポリデモのときはレーダーオフするためのフラグ
		eval($f:global_polygon_demo = 1);

		// フェードインスタート
		#if d:NO_POLYDEMO
			chara フェードインアウト フェードアウト \
				-c 0,0,0 128 \
				-t d:FADEOUT_TIME \
				-p 仮ポリデモ表示 \
				-s 10
		#else
			chara フェードインアウト フェードアウト \
				-c 0,0,0 128 \
				-t d:FADEOUT_TIME \
				-p リスタート
		#endif
	}
}

#ifdef d:POLYDEMO_LOAD
proc ポリゴンデモロード $:ストーリーフラグ {
	#if d:DEBUG_PRINT
		print $:ストーリーフラグ
		print 'polydemo_load'
	#endif

	if ( `command ゲームオーバーチェック` == 0 ) {
		command 無線設定 -reset
		command パッド操作 -release
		command プレイヤー無敵セット
		mesg コマンダー 敵兵セット 視界オフ
		eval ( $w:p_story = $:ストーリーフラグ );
		eval( $f:デモ直後フラグ = 1 );

		// ポリデモのときはレーダーオフするためのフラグ
		eval($f:global_polygon_demo = 1);

		// フェードインスタート
		chara フェードインアウト フェードアウト \
			-c 0,0,0 128 \
			-t d:FADEOUT_TIME \
			-p ポリデモロードスタート
	}
}

proc ポリゴンデモロード（ホワイトフェード） $:ストーリーフラグ {
	#if d:DEBUG_PRINT
		print $:ストーリーフラグ
		print 'polydemo_load'
	#endif

	if ( `command ゲームオーバーチェック` == 0 ) {
		command 無線設定 -reset
		command パッド操作 -release
		command プレイヤー無敵セット
		mesg コマンダー 敵兵セット 視界オフ
		eval ( $w:p_story = $:ストーリーフラグ );
		eval( $f:デモ直後フラグ = 1 );

		// ポリデモのときはレーダーオフするためのフラグ
		eval($f:global_polygon_demo = 1);

		// フェードインスタート
		chara フェードインアウト フェードアウト \
			-c 255,255,255 128 \
			-t d:FADEOUT_TIME \
			-p ポリデモロードスタート
	}
}

proc ポリゴンデモロード（フェードなし） $:ストーリーフラグ {
	#if d:DEBUG_PRINT
		print $:ストーリーフラグ
		print 'polydemo_load'
	#endif

	if ( `command ゲームオーバーチェック` == 0 ) {
		command 無線設定 -reset
		command パッド操作 -release
		command プレイヤー無敵セット
		mesg コマンダー 敵兵セット 視界オフ
		eval ( $w:p_story = $:ストーリーフラグ );
		eval( $f:デモ直後フラグ = 1 );

		// ポリデモのときはレーダーオフするためのフラグ
		eval($f:global_polygon_demo = 1);

		@ポリデモロードスタート
	}
}

// ゴル兵制服があるかないかでロードかリスタートかを自動判別する
proc ポリデモアイテムチェック後スタート $:フラグ {
	if ( $w:アイテム == d:アイテム:ゴル兵制服 ) {
		@ポリゴンデモロード $:フラグ
	} else {
		@ポリゴンデモスタート $:フラグ
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////
// 爆弾関連ゲームオーバー処理
/////////////////////////////////////////////////////////////////////////////
#define	BOMB_EFFECT_OFF		0
#define	BOMB_EFFECT_ON		1
#define	BOMB_GAME_OVERTIME	100

proc 爆発ゲームオーバー処理 $:爆発エフェクトフラグ {
	#if d:DEBUG_PRINT
		print 'bomb_gameover_start'
	#endif

	// エフェクトあるかないかで振動を変える
	if ( $:爆発エフェクトフラグ == d:BOMB_EFFECT_ON ) {
		eval( $s:ローカル変数Ｓ１ = explosion );
	} else {
		eval( $s:ローカル変数Ｓ１ = explosion_far );
	}

	if (`command ゲームオーバーチェック` == 0 ) {
		command 無線設定 -reset
		command StreamStopAll
		command セットサウンドコード -c d:SNG_FOUTS_S		//ＢＧＭフェードアウト
		mesg コマンダー 敵兵セット 視界オフ
		chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１

		command ゲームオーバー処理開始 \
			-exec {
				command プレイヤーモーション振動ＯＦＦ
				command メニュー設定 \
					-gage off \
					-radar off \
					-subwin off \
					-radio off \
					-pause off
				}

		chara delay BombSE01 \
			-time 15 \
			-exec {
				command ＳＥセットモード -s d:SD_W_EXPLOS02 -p $i:カメラＸ位置,($i:カメラＹ位置-6000),$i:カメラＺ位置 -m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 23 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置-2000),($i:カメラＹ位置-6000),($i:カメラＺ位置-2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 31 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置-2000),($i:カメラＹ位置-6000),($i:カメラＺ位置+2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 46 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置+2000),($i:カメラＹ位置-6000),($i:カメラＺ位置-2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 52 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置+2000),($i:カメラＹ位置-6000),($i:カメラＺ位置+2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 62 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置-2000),($i:カメラＹ位置-3000),($i:カメラＺ位置-2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 73 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置+2000),($i:カメラＹ位置-3000),($i:カメラＺ位置-2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 90 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置-2000),($i:カメラＹ位置-2000),($i:カメラＺ位置+2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 100 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置+2000),($i:カメラＹ位置-6000),($i:カメラＺ位置+2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 120 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置),($i:カメラＹ位置-10000),($i:カメラＺ位置) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		if ( $:爆発エフェクトフラグ == d:BOMB_EFFECT_ON ) {
			chara delay Bomb01 \
				-time 10 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p $i:カメラＸ位置,($i:カメラＹ位置-3000),$i:カメラＺ位置 \
						-m 1
					chara 爆発 爆発エフェクト -p $i:カメラＸ位置,($i:カメラＹ位置-3000),$i:カメラＺ位置
					chara 爆発 爆発エフェクト -p $i:カメラＸ位置,($i:カメラＹ位置+3000),$i:カメラＺ位置
				}

			chara delay Bomb01 \
				-time 20 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p $i:注視点Ｘ位置,$i:注視点Ｙ位置,$i:注視点Ｚ位置 \
						-m 1
					chara 爆発 爆発エフェクト -p $i:注視点Ｘ位置,$i:注視点Ｙ位置,$i:注視点Ｚ位置
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p $i:注視点Ｘ位置,$i:注視点Ｙ位置,$i:注視点Ｚ位置
#endif
				}

			chara delay Bomb02 \
				-time 30 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:カメラＸ位置+1000),($i:カメラＹ位置-3500),($i:カメラＺ位置+1000) \
						-m 1
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+1000),($i:カメラＹ位置-3500),($i:カメラＺ位置+1000)
#endif
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+1000),($i:カメラＹ位置+3500),($i:カメラＺ位置+1000)
				}

			chara delay Bomb02 \
				-time 35 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:注視点Ｘ位置+1000),($i:カメラＹ位置-3000),($i:注視点Ｚ位置+1000) \
						-m 1
					chara 爆発 爆発エフェクト -p ($i:注視点Ｘ位置+1000),($i:カメラＹ位置-3000),($i:注視点Ｚ位置+1000)
#ifndef d:XBOX

					chara 爆発 爆発エフェクト -p ($i:注視点Ｘ位置+1000),($i:カメラＹ位置+3000),($i:注視点Ｚ位置+1000)
#endif
				}

			chara delay Bomb03 \
				-time 50 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:カメラＸ位置+1000),($i:カメラＹ位置-3500),($i:カメラＺ位置+1000) \
						-m 1
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置-2000),($i:カメラＹ位置-3500),($i:カメラＺ位置-2000)
#endif
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置-2000),($i:カメラＹ位置+3500),($i:カメラＺ位置-2000)
				}

			chara delay Bomb01 \
				-time 55 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:カメラＸ位置+500),($i:カメラＹ位置-500),($i:カメラＺ位置+500) \
						-m 1
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+500),($i:カメラＹ位置-2000),($i:カメラＺ位置+500)
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+500),($i:カメラＹ位置+2000),($i:カメラＺ位置+500)
#endif
				}

			chara delay Bomb01 \
				-time 65 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:カメラＸ位置-500),($i:カメラＹ位置-500),($i:カメラＺ位置+500) \
						-m 1
#ifndef d:XBOX

					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+500),($i:カメラＹ位置-3000),($i:カメラＺ位置+500)
#endif
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+500),($i:カメラＹ位置+3000),($i:カメラＺ位置+500)
				}

			chara delay Bomb04 \
				-time 70 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:カメラＸ位置-1000),($i:カメラＹ位置-3500),($i:カメラＺ位置+1000) \
						-m 1
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置-1000),($i:カメラＹ位置-3500),($i:カメラＺ位置+1000)
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置-1000),($i:カメラＹ位置+3500),($i:カメラＺ位置+1000)
#endif
				}

			chara delay Bomb01 \
				-time 75 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:カメラＸ位置-500),($i:カメラＹ位置-500),($i:カメラＺ位置-500) \
						-m 1
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置-500),($i:カメラＹ位置-1000),($i:カメラＺ位置-500)
#endif
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置-500),($i:カメラＹ位置+1000),($i:カメラＺ位置-500)
				}

			chara delay Bomb01 \
				-time 80 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:カメラＸ位置+500),($i:カメラＹ位置-500),($i:カメラＺ位置-500) \
						-m 1
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+500),($i:カメラＹ位置-500),($i:カメラＺ位置-500)
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+500),($i:カメラＹ位置+500),($i:カメラＺ位置-500)
#endif
				}


			chara delay Bomb05 \
				-time 90 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:カメラＸ位置+1000),($i:カメラＹ位置-3000),($i:カメラＺ位置-1000) \
						-m 1
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+1000),($i:カメラＹ位置-500),($i:カメラＺ位置-1000)
#endif
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+1000),($i:カメラＹ位置+500),($i:カメラＺ位置-1000)
				}
		}


		chara delay w25a_IR_GameOverFO -time (60*1) -exec {
			chara フェードインアウト ホワイトフェード \
			-c 255 255 255 128 \		//目標のＲＧＢα
			-t d:BOMB_GAME_OVERTIME \
//			-p 爆発ゲームオーバー処理終了 \
			-s 10
		}

		chara delay ゲームオーバーディレイ -time ((60*1)+d:BOMB_GAME_OVERTIME) -exec {
			@爆発ゲームオーバー処理終了
		}

	} else {
		// ゲームオーバー処理中はタイマーを止めてしまう
		@爆弾タイマー停止

	}
}

proc 爆発ゲームオーバー処理（簡易版） $:爆発エフェクトフラグ {
	#if d:DEBUG_PRINT
		print 'bomb_gameover_start'
	#endif

	// エフェクトあるかないかで振動を変える
	if ( $:爆発エフェクトフラグ == d:BOMB_EFFECT_ON ) {
		eval( $s:ローカル変数Ｓ１ = explosion );
	} else {
		eval( $s:ローカル変数Ｓ１ = explosion_far );
	}

	if (`command ゲームオーバーチェック` == 0 ) {
		command 無線設定 -reset
		command StreamStopAll
		command セットサウンドコード -c d:SNG_FOUTS_S		//ＢＧＭフェードアウト
		mesg コマンダー 敵兵セット 視界オフ
		chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１

		command ゲームオーバー処理開始 \
			-exec {
				command プレイヤーモーション振動ＯＦＦ
				command メニュー設定 \
					-gage off \
					-radar off \
					-subwin off \
					-radio off \
					-pause off
				}

		chara delay BombSE01 \
			-time 15 \
			-exec {
				command ＳＥセットモード -s d:SD_W_EXPLOS02 -p $i:カメラＸ位置,($i:カメラＹ位置-6000),$i:カメラＺ位置 -m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 23 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置-2000),($i:カメラＹ位置-6000),($i:カメラＺ位置-2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 31 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置-2000),($i:カメラＹ位置-6000),($i:カメラＺ位置+2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 46 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置+2000),($i:カメラＹ位置-6000),($i:カメラＺ位置-2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 52 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置+2000),($i:カメラＹ位置-6000),($i:カメラＺ位置+2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 62 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置-2000),($i:カメラＹ位置-3000),($i:カメラＺ位置-2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 73 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置+2000),($i:カメラＹ位置-3000),($i:カメラＺ位置-2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 90 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置-2000),($i:カメラＹ位置-2000),($i:カメラＺ位置+2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 100 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置+2000),($i:カメラＹ位置-6000),($i:カメラＺ位置+2000) \
					-m 1
				chara パッド振動 爆発振動 -vibfile $s:ローカル変数Ｓ１
			}

		chara delay BombSE01 \
			-time 120 \
			-exec {
				command ＳＥセットモード \
					-s d:SD_W_EXPLOS02 \
					-p ($i:カメラＸ位置),($i:カメラＹ位置-10000),($i:カメラＺ位置) \
					-m 1
			}

		if ( $:爆発エフェクトフラグ == d:BOMB_EFFECT_ON ) {
			chara delay Bomb01 \
				-time 10 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p $i:カメラＸ位置,($i:カメラＹ位置-3000),$i:カメラＺ位置 \
						-m 1
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p $i:カメラＸ位置,($i:カメラＹ位置-3000),$i:カメラＺ位置
#endif
					chara 爆発 爆発エフェクト -p $i:カメラＸ位置,($i:カメラＹ位置+3000),$i:カメラＺ位置
				}

			chara delay Bomb01 \
				-time 20 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p $i:注視点Ｘ位置,$i:注視点Ｙ位置,$i:注視点Ｚ位置 \
						-m 1
					chara 爆発 爆発エフェクト -p $i:注視点Ｘ位置,$i:注視点Ｙ位置,$i:注視点Ｚ位置
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p $i:注視点Ｘ位置,$i:注視点Ｙ位置,$i:注視点Ｚ位置
#endif
				}

			chara delay Bomb02 \
				-time 30 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:カメラＸ位置+1000),($i:カメラＹ位置-3500),($i:カメラＺ位置+1000) \
						-m 1
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+1000),($i:カメラＹ位置-3500),($i:カメラＺ位置+1000)
#endif
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+1000),($i:カメラＹ位置+3500),($i:カメラＺ位置+1000)
				}

			chara delay Bomb02 \
				-time 40 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:注視点Ｘ位置+1000),($i:カメラＹ位置-3000),($i:注視点Ｚ位置+1000) \
						-m 1
					chara 爆発 爆発エフェクト -p ($i:注視点Ｘ位置+1000),($i:カメラＹ位置-3000),($i:注視点Ｚ位置+1000)
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p ($i:注視点Ｘ位置+1000),($i:カメラＹ位置+3000),($i:注視点Ｚ位置+1000)
#endif
				}

			chara delay Bomb03 \
				-time 50 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:カメラＸ位置+1000),($i:カメラＹ位置-3500),($i:カメラＺ位置+1000) \
						-m 1
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置-2000),($i:カメラＹ位置-3500),($i:カメラＺ位置-2000)
#endif
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置-2000),($i:カメラＹ位置+3500),($i:カメラＺ位置-2000)
				}

			chara delay Bomb01 \
				-time 65 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:カメラＸ位置-500),($i:カメラＹ位置-500),($i:カメラＺ位置+500) \
						-m 1
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+500),($i:カメラＹ位置-3000),($i:カメラＺ位置+500)
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+500),($i:カメラＹ位置+3000),($i:カメラＺ位置+500)
#endif
				}

			chara delay Bomb01 \
				-time 75 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:カメラＸ位置-500),($i:カメラＹ位置-500),($i:カメラＺ位置-500) \
						-m 1
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置-500),($i:カメラＹ位置-1000),($i:カメラＺ位置-500)
#endif
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置-500),($i:カメラＹ位置+1000),($i:カメラＺ位置-500)
				}

			chara delay Bomb01 \
				-time 80 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:カメラＸ位置+500),($i:カメラＹ位置-500),($i:カメラＺ位置-500) \
						-m 1
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+500),($i:カメラＹ位置-500),($i:カメラＺ位置-500)
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+500),($i:カメラＹ位置+500),($i:カメラＺ位置-500)
#endif
				}


			chara delay Bomb05 \
				-time 90 \
				-exec {
					command ＳＥセットモード \
						-s d:SD_W_EXPLOS02 \
						-p ($i:カメラＸ位置+1000),($i:カメラＹ位置-3000),($i:カメラＺ位置-1000) \
						-m 1
#ifndef d:XBOX
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+1000),($i:カメラＹ位置-500),($i:カメラＺ位置-1000)
#endif
					chara 爆発 爆発エフェクト -p ($i:カメラＸ位置+1000),($i:カメラＹ位置+500),($i:カメラＺ位置-1000)
				}
		}


		chara delay w25a_IR_GameOverFO -time (60*1) -exec {
			// 簡易版のホワイトフェードは早めに消してしまう
			chara フェードインアウト ホワイトフェード \
			-c 255 255 255 128 \		//目標のＲＧＢα
			-t (d:BOMB_GAME_OVERTIME-40) \
//			-p 爆発ゲームオーバー処理終了 \
			-s 10
		}

		chara delay ゲームオーバーディレイ -time ((60*1)+(d:BOMB_GAME_OVERTIME-40)) -exec {
			@爆発ゲームオーバー処理終了
		}

	} else {
		// ゲームオーバー処理中はタイマーを止めてしまう
		@爆弾タイマー停止

	}
}


proc 爆発ゲームオーバー処理終了 {
	#if d:DEBUG_PRINT
		print 'bomb_gameover_end'
	#endif

	#ifdef d:STAGE_ALT
	#else
		#ifdef d:STAGE_W20B
			command ゲームオーバー背景設定 \
				-t gmov \
				-s plant_bomb_alp_ovl \
				-m 1
		#elifdef d:STAGE_W20C
			command ゲームオーバー背景設定 \
				-t gmov \
				-s plant_bomb_alp_ovl \
				-m 1
		#else
			command ゲームオーバー背景設定 \
				-t gmov \
				-s plant_bomb2_alp_ovl \
				-m 1
		#endif
	#endif

	command ゲームオーバー処理終了;
}

/////////////////////////////////////////////////////////////////////////////
// 爆弾解体後処理
/////////////////////////////////////////////////////////////////////////////

#ifndef d:BOMB_STAGE
	#define BOMB_STAGE 0
#endif

proc 爆弾解体後処理 {
	#if d:DEBUG_PRINT
		print 'bomb frozen'
	#endif

	if ( $2 == 1 ) {
		//時間を計算するのかもしれない。実体がcdc_common_p.hにあるという噂だがそうとは限らないかもしれないねぇ。
		@rp_爆弾解体時間計算

		// 爆弾処理完了フラグの更新
		if(d:BOMB_STAGE == w12a) {			// Ａ脚・屋上
			eval($f:w12a_爆弾処理完了 = 1)
			mesg 爆弾検知領域 Bomb_Type_A 消去 900
		} else if(d:BOMB_STAGE == w12b) {	// Ａ脚・ポンプ室
			eval($f:w12b_爆弾処理完了 = 1)
			mesg 爆弾検知領域 Bomb_Type_A 消去 900
		} else if(d:BOMB_STAGE == w14a) {	// Ｂ脚・変電室
			eval($f:w14a_爆弾処理完了 = 1)
			mesg 爆弾検知領域 Bomb_Type_A 消去 900
		} else if(d:BOMB_STAGE == w16b) {	// Ｃ脚・食堂
			eval($f:w16b_爆弾処理完了 = 1)
			mesg 爆弾検知領域 Bomb_Type_A 消去 900
		} else if(d:BOMB_STAGE == w18a) {	// Ｄ脚・第一沈殿池
			if ($1 == w18a匂い爆弾１) {
				eval($f:w18a_爆弾処理完了1 = 1)
				mesg 爆弾検知領域 Bomb_Type_A1 消去 900
			} else if ($1 == w18a匂い爆弾２) {		// ハードレベルのみ
				eval($f:w18a_爆弾処理完了2 = 1)
				mesg 爆弾検知領域 Bomb_Type_A2 消去 900
			} else if ($1 == w18a匂い爆弾３) {		// エクストリームのみ
				eval($f:w18a_爆弾処理完了3 = 1)
				mesg 爆弾検知領域 Bomb_Type_A3 消去 900
			}
		} else if(d:BOMB_STAGE == w20a) {	// Ｅ脚・集配所
			eval($f:w20a_爆弾処理完了 = 1)
			mesg 爆弾検知領域 Bomb_Type_A 消去 900
		} else if(d:BOMB_STAGE == w20b) {	// Ｅ脚・ヘリポート
			eval($f:w20b_爆弾処理完了 = 1)
			mesg 爆弾検知領域 Bomb_Type_A 消去 900
		} else if(d:BOMB_STAGE == w22a) {	// Ｆ脚・倉庫
			eval($f:w22a_爆弾処理完了 = 1)
			mesg 爆弾検知領域 Bomb_Type_A 消去 900
		} else {
			#if d:DEBUG_PRINT
				print '##### invalid bomb stage !!! #####'
			#endif
		}

		// 爆弾マーク消去
		if ( $f:w12a_爆弾処理完了 == 1 && $f:w12b_爆弾処理完了 == 1 ) {
			command 全体マップ爆弾配置解除 -bomb d:Ａ脚マップ
		}
		if ( $f:w14a_爆弾処理完了 == 1 ) {
			command 全体マップ爆弾配置解除 -bomb d:Ｂ脚マップ
		}
		if ( $f:w16b_爆弾処理完了 == 1 ) {
print 'W16 KAITAI------------------------------------------------------'
			command 全体マップ爆弾配置解除 -bomb d:Ｃ脚マップ
		}
		if ( $f:w18a_爆弾処理完了1 == 1 && $f:w18a_爆弾処理完了2 == 1 && $f:w18a_爆弾処理完了3 == 1 ) {
			command 全体マップ爆弾配置解除 -bomb d:Ｄ脚マップ
		}
		if ( $f:w20a_爆弾処理完了 == 1 && $f:w20b_爆弾処理完了 == 1 ) {
			command 全体マップ爆弾配置解除 -bomb d:Ｅ脚マップ
		}
		if ( $f:w22a_爆弾処理完了 == 1 ) {
			command 全体マップ爆弾配置解除 -bomb d:Ｆ脚マップ
		}

		// 凍結したときの処理
		// ストーリーフラグ分岐
		if($w:p_story < d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {
			#if d:DEBUG_PRINT
				print '##### invalid story flag !!! #####'
			#endif
		} else if($w:p_story == d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {
			eval($b:Ｃ４爆弾処理数 = $b:Ｃ４爆弾処理数+1)

			if(`@rp_爆弾解体脚数` >= 1){
				// 一つ目を解体無線
				command パッド操作 -release
				command メニュー設定 -radio off	//この間に無線されると悪。
				chara delay ディレイ \
					-time 60 \
					-exec {
						command メニュー設定 -radio on	//祝復帰！
						@rt_P015_01_R1爆弾解体最初の一つ１無線デモ１
					}
			}

		} else if($w:p_story == d:ST:P015_01_R1爆弾解体最初の一つ１無線デモ１終了) {
			eval($b:Ｃ４爆弾処理数 = $b:Ｃ４爆弾処理数+1)

//			if($b:Ｃ４爆弾設置数 <= $b:Ｃ４爆弾処理数*2)
			if(`@rp_爆弾解体脚数` >= 3){
				// 半分以上解体無線
				command パッド操作 -release
				command メニュー設定 -radio off	//この間に無線されると悪。
				chara delay ディレイ \
					-time 60 \
					-exec {
						command メニュー設定 -radio on	//祝復帰！
						@rt_P016_01_R1爆弾解体半分解体１無線デモ１
					}
			}

		} else if($w:p_story == d:ST:P016_01_R1爆弾解体半分解体１無線デモ１終了) {
			eval($b:Ｃ４爆弾処理数 = $b:Ｃ４爆弾処理数+1)

//			if($b:Ｃ４爆弾設置数 < $b:Ｃ４爆弾処理数+2)
			if(`@rp_爆弾解体脚数` >= 5){
				// 最後から二つ目を解体無線
				command パッド操作 -release
				command メニュー設定 -radio off	//この間に無線されると悪。
				chara delay ディレイ \
					-time 60 \
					-exec {
						command メニュー設定 -radio on	//祝復帰！
						@rt_P022_01_R01爆弾解体最後から二つ目１無線デモ１
					}
			}

		} else if($w:p_story == d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了) {
			eval($b:Ｃ４爆弾処理数 = $b:Ｃ４爆弾処理数+1)

			if(`@rp_爆弾解体脚数` >= 6){
				// 最後の一つを解体無線
				command パッド操作 -release
				command メニュー設定 -radio off	//この間に無線されると悪。
				chara delay ディレイ \
					-time 60 \
					-exec {
						command メニュー設定 -radio on	//祝復帰！
						@rt_P023_01_R01爆弾解体最後の一つ１無線デモ１
					}
			}
		} else if($w:p_story >= d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了) {
			#if d:DEBUG_PRINT
				print '##### invalid story flag !!! #####'
			#endif
		}


	} else if ( $2 == 0 ) {
#ifdef d:STAGE_W20B || d:STAGE_W20C
		@爆発ゲームオーバー処理（簡易版） d:BOMB_EFFECT_ON
#else
		@爆発ゲームオーバー処理 d:BOMB_EFFECT_ON
#endif
	}
}

/////////////////////////////////////////////////////////////////////////////
// 風基本設定
/////////////////////////////////////////////////////////////////////////////

	proc 風基本設定 {
		chara 風制御 風	\
			-i 500 \
			-j 1000 \
			-r -1024 0 0 \
			-a 4095 \
			-b 300 \
			-c 150
	}
	proc 風設定 $:トラップ名 {
		@風基本設定
		mesg 風制御 風 オフ

		trap $:トラップ名 d:PLAYER \
			-mask ？ \
			-exec {
				if ($3 == 入る) {
					mesg 風制御 風 オン
				} else {
					mesg 風制御 風 オフ
				}
			}
	}

/////////////////////////////////////////////////////////////////////////////
// ドアランプ設定
/////////////////////////////////////////////////////////////////////////////

proc ドアランプ設定 $:ドアランプ名 $:ドア方向 $:ドアＸ座標 $:ドアＹ座標 $:ドアＺ座標 $:表０裏１両面２ {
	#if d:DEBUG_PRINT
		print 'door lamp set'
	#endif

	if($:ドア方向 == 0){
		if($:表０裏１両面２ != 1) {
			chara ドアランプ $:ドアランプ名 \
				-r 0 \
				-p ($:ドアＸ座標+d:SD_LAMP_SHIFT_1) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標+d:SD_LAMP_SHIFT_2)
		}
		if($:表０裏１両面２ != 0) {
			chara ドアランプ $:ドアランプ名 \
				-r 2048 \
				-p ($:ドアＸ座標+d:SD_LAMP_SHIFT_1) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標-d:SD_LAMP_SHIFT_2)
		}

	} else if($:ドア方向 == 1024){
		if($:表０裏１両面２ != 1) {
			chara ドアランプ $:ドアランプ名 \
				-r 1024 \
				-p ($:ドアＸ座標+d:SD_LAMP_SHIFT_2) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標-d:SD_LAMP_SHIFT_1)
		}
		if($:表０裏１両面２ != 0) {
			chara ドアランプ $:ドアランプ名 \
				-r 3072 \
				-p ($:ドアＸ座標-d:SD_LAMP_SHIFT_2) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標-d:SD_LAMP_SHIFT_1)
		}

	} else if($:ドア方向 == 2048){
		if($:表０裏１両面２ != 1) {
			chara ドアランプ $:ドアランプ名 \
				-r 2048 \
				-p ($:ドアＸ座標-d:SD_LAMP_SHIFT_1) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標-d:SD_LAMP_SHIFT_2)
		}
		if($:表０裏１両面２ != 0) {
			chara ドアランプ $:ドアランプ名 \
				-r 0 \
				-p ($:ドアＸ座標-d:SD_LAMP_SHIFT_1) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標+d:SD_LAMP_SHIFT_2)
		}

	} else if($:ドア方向 == 3072){
		if($:表０裏１両面２ != 1) {
			chara ドアランプ $:ドアランプ名 \
				-r 3072 \
				-p ($:ドアＸ座標-d:SD_LAMP_SHIFT_2) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標+d:SD_LAMP_SHIFT_1)
		}
		if($:表０裏１両面２ != 0) {
			chara ドアランプ $:ドアランプ名 \
				-r 1024 \
				-p ($:ドアＸ座標+d:SD_LAMP_SHIFT_2) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標+d:SD_LAMP_SHIFT_1)
		}

	} else if($:ドア方向 == 722){
		if($:表０裏１両面２ != 1) {
			chara ドアランプ $:ドアランプ名 \
				-r 722 \
				-p ($:ドアＸ座標+(d:SD_LAMP_SHIFT_3C+d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標+(-d:SD_LAMP_SHIFT_3S+d:SD_LAMP_SHIFT_4C))
		}
		if($:表０裏１両面２ != 0) {
			chara ドアランプ $:ドアランプ名 \
				-r 2770 \
				-p ($:ドアＸ座標+(d:SD_LAMP_SHIFT_3C-d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標+(-d:SD_LAMP_SHIFT_3S-d:SD_LAMP_SHIFT_4C))
		}

	} else if($:ドア方向 == 1326){
		if($:表０裏１両面２ != 1) {
			chara ドアランプ $:ドアランプ名 \
				-r 1326 \
				-p ($:ドアＸ座標+(-d:SD_LAMP_SHIFT_3C+d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標+(-d:SD_LAMP_SHIFT_3S-d:SD_LAMP_SHIFT_4C))
		}
		if($:表０裏１両面２ != 0) {
			chara ドアランプ $:ドアランプ名 \
				-r 3374 \
				-p ($:ドアＸ座標+(-d:SD_LAMP_SHIFT_3C-d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標+(-d:SD_LAMP_SHIFT_3S+d:SD_LAMP_SHIFT_4C))
		}

	} else if($:ドア方向 == 2770){
		if($:表０裏１両面２ != 1) {
			chara ドアランプ $:ドアランプ名 \
				-r 2770 \
				-p ($:ドアＸ座標+(-d:SD_LAMP_SHIFT_3C-d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標+(d:SD_LAMP_SHIFT_3S-d:SD_LAMP_SHIFT_4C))
		}
		if($:表０裏１両面２ != 0) {
			chara ドアランプ $:ドアランプ名 \
				-r 722 \
				-p ($:ドアＸ座標+(-d:SD_LAMP_SHIFT_3C+d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標+(d:SD_LAMP_SHIFT_3S+d:SD_LAMP_SHIFT_4C))
		}

	} else if($:ドア方向 == 3374){
		if($:表０裏１両面２ != 1) {
			chara ドアランプ $:ドアランプ名 \
				-r 3374 \
				-p ($:ドアＸ座標+(-d:SD_LAMP_SHIFT_3C-d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標+(d:SD_LAMP_SHIFT_3S+d:SD_LAMP_SHIFT_4C))
		}
		if($:表０裏１両面２ != 0) {
			chara ドアランプ $:ドアランプ名 \
				-r 1326 \
				-p ($:ドアＸ座標+(-d:SD_LAMP_SHIFT_3C+d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標+(d:SD_LAMP_SHIFT_3S-d:SD_LAMP_SHIFT_4C))
		}

	} else if($:ドア方向 == 1745){
		// 向手ステージスペシャル
		if($:表０裏１両面２ != 1) {
			chara ドアランプ $:ドアランプ名 \
				-r 1745 \
				-p ($:ドアＸ座標+(-d:SD_LAMP_SHIFT_5C+d:SD_LAMP_SHIFT_6S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標+(-d:SD_LAMP_SHIFT_5S-d:SD_LAMP_SHIFT_6C))
		}
		if($:表０裏１両面２ != 0) {
			chara ドアランプ $:ドアランプ名 \
				-r 3793 \
				-p ($:ドアＸ座標+(-d:SD_LAMP_SHIFT_5C-d:SD_LAMP_SHIFT_6S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y) ($:ドアＺ座標+(-d:SD_LAMP_SHIFT_5S+d:SD_LAMP_SHIFT_6C))
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// 壊れドアランプ設定
/////////////////////////////////////////////////////////////////////////////

proc 壊れドアエフェクト設定 $:壊れエフェクト名 $:ドア方向 $:ドアＸ座標 $:ドアＹ座標 $:ドアＺ座標 $:表０裏１両面２ {
	#if d:DEBUG_PRINT
		print 'broken door effect set'
	#endif

	if($:ドア方向 == 0){
		if($:表０裏１両面２ != 1) {
			chara ドアパネル電撃 $:壊れエフェクト名 \
				-p ($:ドアＸ座標+d:SD_LAMP_SHIFT_1) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標+d:SD_LAMP_SHIFT_2) \
				-r 0, 0, 0 \
				-w 250 \
				-h 125
		}
		if($:表０裏１両面２ != 0) {
			chara ドアパネル電撃 $:壊れエフェクト名 \
				-p ($:ドアＸ座標+d:SD_LAMP_SHIFT_1) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標-d:SD_LAMP_SHIFT_2) \
				-r 0, 2048, 0 \
				-w 250 \
				-h 125
		}

	} else if($:ドア方向 == 1024){
		if($:表０裏１両面２ != 1) {
			#ifdef d:STAGE_W31C
				chara ドアパネル電撃 $:壊れエフェクト名 \
					-p ($:ドアＸ座標+d:SD_LAMP_SHIFT_2) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標-d:SD_LAMP_SHIFT_1) \
					-r 0, 1024, 0 \
					-w 250 \
					-h 125 \
					-mode 1 \
					-length 3500
			#else
				chara ドアパネル電撃 $:壊れエフェクト名 \
					-p ($:ドアＸ座標+d:SD_LAMP_SHIFT_2) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標-d:SD_LAMP_SHIFT_1) \
					-r 0, 1024, 0 \
					-w 250 \
					-h 125
			#endif
		}
		if($:表０裏１両面２ != 0) {
			#ifdef d:STAGE_W31C
				chara ドアパネル電撃 $:壊れエフェクト名 \
					-p ($:ドアＸ座標-d:SD_LAMP_SHIFT_2) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標-d:SD_LAMP_SHIFT_1) \
					-r 0, 3072, 0 \
					-w 250 \
					-h 125 \
					-mode 1 \
					-length 3500
			#else
				chara ドアパネル電撃 $:壊れエフェクト名 \
					-p ($:ドアＸ座標-d:SD_LAMP_SHIFT_2) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標-d:SD_LAMP_SHIFT_1) \
					-r 0, 3072, 0 \
					-w 250 \
					-h 125
			#endif
		}

	} else if($:ドア方向 == 2048){
		if($:表０裏１両面２ != 1) {
			chara ドアパネル電撃 $:壊れエフェクト名 \
				-p ($:ドアＸ座標-d:SD_LAMP_SHIFT_1) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標-d:SD_LAMP_SHIFT_2) \
				-r 0, 2048, 0 \
				-w 250 \
				-h 125
		}
		if($:表０裏１両面２ != 0) {
			chara ドアパネル電撃 $:壊れエフェクト名 \
				-p ($:ドアＸ座標-d:SD_LAMP_SHIFT_1) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標+d:SD_LAMP_SHIFT_2) \
				-r 0, 0, 0 \
				-w 250 \
				-h 125
		}

	} else if($:ドア方向 == 3072){
		if($:表０裏１両面２ != 1) {
			chara ドアパネル電撃 $:壊れエフェクト名 \
				-p ($:ドアＸ座標-d:SD_LAMP_SHIFT_2) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標+d:SD_LAMP_SHIFT_1) \
				-r 0, 3072, 0 \
				-w 250 \
				-h 125
		}
		if($:表０裏１両面２ != 0) {
			chara ドアパネル電撃 $:壊れエフェクト名 \
				-p ($:ドアＸ座標+d:SD_LAMP_SHIFT_2) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標+d:SD_LAMP_SHIFT_1) \
				-r 0, 1024, 0 \
				-w 250 \
				-h 125
		}

	} else if($:ドア方向 == 722){
		if($:表０裏１両面２ != 1) {
			chara ドアパネル電撃 $:壊れエフェクト名 \
				-p ($:ドアＸ座標+(d:SD_LAMP_SHIFT_3C+d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標+(-d:SD_LAMP_SHIFT_3S+d:SD_LAMP_SHIFT_4C)) \
				-r 0, 722, 0 \
				-w 250 \
				-h 125
		}
		if($:表０裏１両面２ != 0) {
			chara ドアパネル電撃 $:壊れエフェクト名 \
				-p ($:ドアＸ座標+(d:SD_LAMP_SHIFT_3C-d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標+(-d:SD_LAMP_SHIFT_3S-d:SD_LAMP_SHIFT_4C)) \
				-r 0, 2770, 0 \
				-w 250 \
				-h 125
		}

	} else if($:ドア方向 == 1326){
		if($:表０裏１両面２ != 1) {
			chara ドアパネル電撃 $:壊れエフェクト名 \
				-p ($:ドアＸ座標+(-d:SD_LAMP_SHIFT_3C+d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標+(-d:SD_LAMP_SHIFT_3S-d:SD_LAMP_SHIFT_4C)) \
				-r 0, 1326, 0 \
				-w 250 \
				-h 125
		}
		if($:表０裏１両面２ != 0) {
			chara ドアパネル電撃 $:壊れエフェクト名 \
				-p ($:ドアＸ座標+(-d:SD_LAMP_SHIFT_3C-d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標+(-d:SD_LAMP_SHIFT_3S+d:SD_LAMP_SHIFT_4C)) \
				-r 0, 3374, 0 \
				-w 250 \
				-h 125
		}

	} else if($:ドア方向 == 2770){
		if($:表０裏１両面２ != 1) {
			chara ドアパネル電撃 $:壊れエフェクト名 \
				-p ($:ドアＸ座標+(-d:SD_LAMP_SHIFT_3C-d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標+(d:SD_LAMP_SHIFT_3S-d:SD_LAMP_SHIFT_4C)) \
				-r 0, 2770, 0 \
				-w 250 \
				-h 125
		}
		if($:表０裏１両面２ != 0) {
			chara ドアパネル電撃 $:壊れエフェクト名 \
				-p ($:ドアＸ座標+(-d:SD_LAMP_SHIFT_3C+d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標+(d:SD_LAMP_SHIFT_3S+d:SD_LAMP_SHIFT_4C)) \
				-r 0, 722, 0 \
				-w 250 \
				-h 125
		}

	} else if($:ドア方向 == 3374){
		if($:表０裏１両面２ != 1) {
			chara ドアパネル電撃 $:壊れエフェクト名 \
				-p ($:ドアＸ座標+(-d:SD_LAMP_SHIFT_3C-d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標+(d:SD_LAMP_SHIFT_3S+d:SD_LAMP_SHIFT_4C)) \
				-r 0, 3374, 0 \
				-w 250 \
				-h 125
		}
		if($:表０裏１両面２ != 0) {
			chara ドアパネル電撃 $:壊れエフェクト名 \
				-p ($:ドアＸ座標+(-d:SD_LAMP_SHIFT_3C+d:SD_LAMP_SHIFT_4S)) ($:ドアＹ座標+d:SD_LAMP_SHIFT_Y-1200) ($:ドアＺ座標+(d:SD_LAMP_SHIFT_3S-d:SD_LAMP_SHIFT_4C)) \
				-r 0, 1326, 0 \
				-w 250 \
				-h 125
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// 爆弾タイマー
/////////////////////////////////////////////////////////////////////////////

proc 爆弾タイマー設置 {
#if d:DEBUG_PRINT
	print '*** Bomb Timer Set ***'
#endif

	// タイマー
	//   -t タイム(NTSCフレーム数で指定(1分の場合3600)) -p 終了プロック -s ステータス

	if(    ($w:p_story >= d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了) \
		&& ($w:p_story <  d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了)) {
		// 非表示・停止状態でＡ脚底部爆弾タイマー起動
		chara タイマー 爆弾タイマー -t $i:Ａ脚底部爆弾タイマー残り時間 -p 爆弾タイマー時間切れ -s 0x03 -m d:TMR_SEC

	} else if(($w:p_story >= d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了) \
		&&    ($w:p_story <  d:ST:P026_01_R01爆弾解体終了１無線デモ１開始)) {
		// 表示・停止状態でＡ脚底部爆弾タイマー起動
		chara タイマー 爆弾タイマー -t $i:Ａ脚底部爆弾タイマー残り時間 -p 爆弾タイマー時間切れ -s 0x01 -m d:TMR_SEC

	} else if(($w:p_story >= d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) \
		&&    ($w:p_story <  d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始)) {
		// 表示・停止状態でヘリポート爆弾タイマー起動
		chara タイマー 爆弾タイマー -t $i:ヘリポート爆弾タイマー残り時間 -p 爆弾タイマー時間切れ -s 0x01 -m d:TMR_SEC


	} else if(($w:p_story >= d:ST:P069_04_R02ヴァンプ狙撃終了４無線機デモ２終了 ) \
		&&    ($w:p_story < d:ST:P070_01_P01ＡＧ起動１ポリゴンデモ１開始 )) {
	// エマタイマー
		// 表示・停止状態でエマタイマー起動
		chara タイマー 爆弾タイマー -t $i:エマタイマー残り時間 -p エマタイマー時間切れ -s 0x01 -m d:TMR_SEC
	}
}
proc 爆弾タイマー開始 {
#if d:DEBUG_PRINT
	print '*** Bomb Timer Start ***'
#endif

	if(    (   ($w:p_story >= d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了) \
			&& ($w:p_story <  d:ST:P026_01_R01爆弾解体終了１無線デモ１開始)) \
		|| (   ($w:p_story >= d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) \
			&& ($w:p_story <  d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始)) \
		|| (   ($w:p_story >= d:ST:P069_04_R02ヴァンプ狙撃終了４無線機デモ２終了 ) \
			&& ($w:p_story <  d:ST:P070_01_P01ＡＧ起動１ポリゴンデモ１開始 ))) {

		mesg タイマー 爆弾タイマー 表示切り替え 1
		mesg タイマー 爆弾タイマー 開始
	}
}
proc 爆弾タイマー停止 {
#if d:DEBUG_PRINT
	print '*** Bomb Timer Stop ***'
#endif

	if(    (   ($w:p_story >= d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了) \
			&& ($w:p_story <  d:ST:P026_01_R01爆弾解体終了１無線デモ１開始)) \
		|| (   ($w:p_story >= d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) \
			&& ($w:p_story <  d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始)) \
		|| (   ($w:p_story >= d:ST:P069_04_R02ヴァンプ狙撃終了４無線機デモ２終了 ) \
			&& ($w:p_story <  d:ST:P070_01_P01ＡＧ起動１ポリゴンデモ１開始 ))) {

		command セットタイマーステータス -s d:TMR_WAIT		// 停止
	}
}
proc 爆弾タイマー加算 $:加算値 {
#if d:DEBUG_PRINT
	print '*** Bomb Timer Add ***'
#endif

	if(    (   ($w:p_story >= d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了) \
			&& ($w:p_story <  d:ST:P026_01_R01爆弾解体終了１無線デモ１開始)) \
		|| (   ($w:p_story >= d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) \
			&& ($w:p_story <  d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始)) \
		|| (   ($w:p_story >= d:ST:P069_04_R02ヴァンプ狙撃終了４無線機デモ２終了 ) \
			&& ($w:p_story <  d:ST:P070_01_P01ＡＧ起動１ポリゴンデモ１開始 ))) {

		command タイマー加算 -time $:加算値
	}
}
proc 爆弾タイマー残り時間保存 {
#if d:DEBUG_PRINT
	print '*** Bomb Timer Save ***'
#endif

	if(    ($w:p_story >= d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了) \
		&& ($w:p_story <  d:ST:P026_01_R01爆弾解体終了１無線デモ１開始)) {
		// Ａ脚底部爆弾タイマー残り時間保存
		eval($i:Ａ脚底部爆弾タイマー残り時間 = `%ゲット残りタイム`)
		command セットタイマーステータス -s d:TMR_WAIT		// 停止

	} else if(($w:p_story >= d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) \
		&&    ($w:p_story <  d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始)) {
		// ヘリポート爆弾タイマー残り時間保存
		eval($i:ヘリポート爆弾タイマー残り時間 = `%ゲット残りタイム`)
		command セットタイマーステータス -s d:TMR_WAIT		// 停止

	} else if(($w:p_story >= d:ST:P069_04_R02ヴァンプ狙撃終了４無線機デモ２終了) \
		&&    ($w:p_story <  d:ST:P070_01_P01ＡＧ起動１ポリゴンデモ１開始)) {
		// エマタイマー残り時間保存
		eval($i:エマタイマー残り時間 = `%ゲット残りタイム`)
		command セットタイマーステータス -s d:TMR_WAIT		// 停止
	}
}
proc 爆弾タイマー時間切れ {
#if d:DEBUG_PRINT
	print '*** Bomb Timer Time Over ***'
#endif

	if(    ($w:p_story >= d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了) \
		&& ($w:p_story <  d:ST:P026_01_R01爆弾解体終了１無線デモ１開始)) {

		//--------------------------------------------------------------
		// Ａ脚底部爆弾タイマー時間切れ

		mesg 解体Ｃ４ 解体Ｃ４ blast

		if($b:Ａ脚底部爆弾タイマーゲームオーバー回数 < 10) {
		#if d:DEBUG_PRINT
			print '*** Continue Count ***'
			print $b:Ａ脚底部爆弾タイマーゲームオーバー回数
		#endif

			eval($b:Ａ脚底部爆弾タイマーゲームオーバー回数 = $b:Ａ脚底部爆弾タイマーゲームオーバー回数 + 1)
			command varsave $b:Ａ脚底部爆弾タイマーゲームオーバー回数

			if($b:Ａ脚底部爆弾タイマーゲームオーバー回数 == 10) {
				eval($i:Ａ脚底部爆弾タイマー残り時間 = `command save_var_read $i:Ａ脚底部爆弾タイマー残り時間` + 12000)		// 残り時間を増やす
				command varsave $i:Ａ脚底部爆弾タイマー残り時間
			}
		}

#ifdef d:STAGE_W11B
		@爆発ゲームオーバー処理 d:BOMB_EFFECT_ON
#else
		@爆発ゲームオーバー処理 d:BOMB_EFFECT_OFF
#endif

	} else if(($w:p_story >= d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) \
		&&    ($w:p_story <  d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始)) {

		//--------------------------------------------------------------
		// ヘリポート爆弾タイマー時間切れ

		mesg 解体Ｃ４ ヘリポート爆弾 blast

		if($b:ヘリポート爆弾タイマーゲームオーバー回数 < 10) {
		#if d:DEBUG_PRINT
			print '*** Continue Count ***'
			print $b:ヘリポート爆弾タイマーゲームオーバー回数
		#endif

			eval($b:ヘリポート爆弾タイマーゲームオーバー回数 = $b:ヘリポート爆弾タイマーゲームオーバー回数 + 1)
			command varsave $b:ヘリポート爆弾タイマーゲームオーバー回数

			if($b:ヘリポート爆弾タイマーゲームオーバー回数 == 10) {
				eval($i:ヘリポート爆弾タイマー残り時間 = `command save_var_read $i:ヘリポート爆弾タイマー残り時間` + 12000)		// 残り時間を増やす
				command varsave $i:ヘリポート爆弾タイマー残り時間
			}
		}

#ifdef d:STAGE_W20C
		@爆発ゲームオーバー処理（簡易版） d:BOMB_EFFECT_ON
#else
		@爆発ゲームオーバー処理 d:BOMB_EFFECT_OFF
#endif
	}
}

proc エマタイマー時間切れ {
#if d:DEBUG_PRINT
	print '*** Emma Timeover ***'
#endif

	if (`command ゲームオーバーチェック` == 0 ) {
		command 無線設定 -reset
		command StreamStopAll
		command セットサウンドコード -c d:SNG_FOUTS_S		//ＢＧＭフェードアウト
		mesg コマンダー 敵兵セット 視界オフ

		eval($f:pgo_エマ死亡 = 1)

		command ゲームオーバー処理開始 \
			-exec {
					command プレイヤーモーション振動ＯＦＦ
					command メニュー設定 \
						-gage off \
						-radar off \
						-subwin off \
						-radio off \
						-pause off
					command ＳＥセットモード -s d:se_code:SD_V_EMAOUT01 \
						-p $i:プレイヤー位置Ｘ,$i:プレイヤー位置Ｙ,$i:プレイヤー位置Ｚ -m 1
				}

		chara delay w25a_IR_GameOverFO -time (60*1) -exec {
			chara フェードインアウト ホワイトフェード \
			-c 255 255 255 128 \		//目標のＲＧＢα
			-t d:BOMB_GAME_OVERTIME \
			-p エマゲームオーバー処理終了 \
			-s 10
		}

	} else {
		// ゲームオーバー処理中はタイマーを止めてしまう
		@爆弾タイマー停止

	}
}

proc エマゲームオーバー処理終了 {
	#if d:DEBUG_PRINT
		print 'emma_gameover_end'
	#endif
	command ゲームオーバー背景設定 \
		-t gmov \
		-s ema_died_alp_ovl \
		-m 1

	command ゲームオーバー処理終了;
}



/////////////////////////////////////////////////////////////////////////////
// 水中処理
/////////////////////////////////////////////////////////////////////////////
#ifdef d:WATER_MODE
/*
水中モード基本キャラ設定を使うには以下のdefineが必要です。
#define	WATER_MODE			1		// 水中モード基本キャラ設定を使用するのに必要
#define WATER_SURFACE		-6500	// 水面の高さ
#define	WATER_CAMERA_ZONE	1		// 追従カメラが使用するゾーンのあるグループ番号(必要があれば定義する)
#define	WATER_ELUDE_IN		1		// エルード状態から入水できる場合は定義する
#define	CAMERA_BUBBLE_NUM	1		// カメラ泡の個数(いらない場合は定義しない)
#define	CAMERA_DUST_NUM		1		// カメラダストの個数(いらない場合は定義しない)

また、この他に以下のものを個別に作成し呼ぶ必要があります。
入水処理、出水処理
水面設定
息継ぎポイント用設定(投影影、レーダー対応)
浮遊物、機雷←必要に応じて
*/

proc 水中モード基本キャラ設定 {
#if d:DEBUG_PRINT
	print 'water_mode_std_chara_set'
	print 'water_level'
	print d:WATER_SURFACE
#endif

#ifdef d:STAGE_W11A
	// 水位設定
	command 水位設定 -lv d:WATER_SURFACE -s d:se_code:SD_A_RICOCH08
#elifdef d:STAGE_W11B
	// 水位設定
	command 水位設定 -lv d:WATER_SURFACE -s d:se_code:SD_A_RICOCH08
#else
	// 水位設定
	command 水位設定 -lv d:WATER_SURFACE -s d:se_code:SD_A_RICOCH06
#endif


	// 水中プラグイン設定
#ifdef d:WATER_CAMERA_ZONE
	command 新プラグイン水中 -z d:WATER_CAMERA_ZONE
#else
	command 新プラグイン水中
#endif

	// 水位、モーション等を設定
#ifdef d:WATER_ELUDE_IN
	command 新水中設定 \
		-horizon d:WATER_SURFACE \
		-motion rai_swim \
		-elude2water d:モーションリスト:non_drop_fall_treadwater \
		-dead2water d:モーションリスト:non_drop_fall_treadwater_down \
		-flag d:WATER_FLAG_NO_FLOOR
#else
	command 新水中設定 \
		-horizon d:WATER_SURFACE \
		-motion rai_swim \
		-flag d:WATER_FLAG_AUTO_ROTATE
#endif

	// 以下エフェクト各種
	// 水中モード時画面が揺らぐ
	//chara 水中カメラ ゆらぎマン -col 128 128 160 -warp 2
	chara フレーム退避 水中フレーム退避
	chara ゆがみ泡 歪み気泡マン
	chara 水中カメラ ゆらぎマン -col 128 128 136 -warp 2

	// カメラ前に出てくる泡
#ifdef d:CAMERA_BUBBLE_NUM
	chara カメラ泡 泡ぶくマン -n d:CAMERA_BUBBLE_NUM	// 512*n
#endif

#ifdef d:STAGE_W11A
	command 泡色変更 0x60806620
#elifdef d:STAGE_W11B
	command 泡色変更 0x60806620
#endif


	// カメラ前に出てくるダスト
#ifdef d:CAMERA_DUST_NUM
	chara カメラダスト ダストマン -n d:CAMERA_DUST_NUM		// 512*n
#endif

	@水中カメラ環境音変更 d:WATER_SURFACE


#ifndef d:STAGE_W11A
#ifndef d:STAGE_W11B
	@息継ぎポイントスポットライト背景設定
	command トラップ切り替え -trap puddle -switch 0
#endif
#endif


	// エマフラグが立つのを待つため１フレーム遅らせる
	// ステージによってメッセージのオン・オフを細かく切り分けなければならない関係上
	// かなり入り組んだ書き方になってしまいました。
	// 後でソース見る人ごめんなさい
#ifndef d:STAGE_W11A
#ifndef d:STAGE_W11B
#ifndef d:DEMO_STAGE
	chara delay 水中エフェクトオフディレイ -time 1 -exec {
		if( $f:エマと一緒に水の中 == 0 ) {
			// w31bのビーターデモ後は再びオンにしている
			mesg カメラ泡     泡ぶくマン オフ
			mesg カメラダスト ダストマン オフ
		}
	}
#endif
#endif
#endif

}

// プレイヤーと連動するキャラはデモでは使用しないため切り分ける
proc 水中モードプレイヤー連動キャラ設定 $:水中トラップ名 {
#if d:DEBUG_PRINT
	print 'water_mode_playertype_chara_set'
#endif

#ifndef d:STAGE_W11A
	// 潜水服ライデン以外はライデンの髪の毛との連動
	@ライデン髪の毛水中モード連動設定 $:水中トラップ名
#endif

	// 以下エフェクト各種
	// プレイヤーと連動する水しぶき
	chara 詳細水飛沫 NewDropBodySplushScn -n d:PLAYER -c sna_skl3
	// プレイヤーと連動する波紋
	chara 泡で波紋管理 NewRipBubbleMan -n d:PLAYER

	// 水中のにごり
#ifdef d:STAGE_W11A
	chara 水中濁り 水中濁り \
		-c 108 176 100 36
#elifdef d:STAGE_W11B
	chara 水中濁り 水中濁り \
		-c 108 176 100 36
#else
	chara 水中濁り 水中濁り \
		-c 108 176 200 36
#endif

	// 水から出たときのカメラ前の曇り
	@カメラ前曇り制御 $:水中トラップ名
}

// 息継ぎポイントとレーダーの連動
proc 息継ぎポイントレーダー設定 $:レーダー用トラップ名 {
#if d:DEBUG_PRINT
	print 'breath_point_radar_set'
#endif

	chara 爆弾検知領域 息継ぎレーダー１ -trap $:レーダー用トラップ名 -rgb 50 50 200 -mode 1

}

proc 息継ぎポイントスポットライト背景設定 {
// 水中モード基本キャラ設定に登録しました。
#if d:DEBUG_PRINT
	print 'breath_point_spotlight_set'
#endif

	chara スポットライト背景イメージ指定 新スポットライト -file reflact02
#ifdef d:STAGE_W31C
	// 起動時はオフ、入水処理が始まるところでオンにする
	mesg スポットライト背景イメージ指定 新スポットライト テクスチャ許可 0
	mesg 静止スポットライト設置 息継ぎライト:01 off
	mesg 静止スポットライト設置 息継ぎライト:02 on

	mesg 天窓の光 ひかり１ スリープモード
	mesg 天窓の光 ひかり２ アクティブモード

	mesg エリアダスト 左ダスト オフ
	mesg エリアダスト 右ダスト オフ
#endif
}


proc 水中カメラ環境音変更 $:水面位置 {
#if d:DEBUG_PRINT
	print 'Environment Sound'
#endif

	eval( $i:global_水面高さ = $:水面位置 )

	chara プロック連続実行 水中環境音チェック -time -1 -exec {
		if( $i:カメラＹ位置 <= $i:global_水面高さ ) {
			if( $f:global_水中環境音フラグ == 0 ) {
				#if d:DEBUG_PRINT
					print '水中環境音ＯＮ'
					print '$i:カメラＹ位置='$i:カメラＹ位置
					print '$i:global_水面高さ='$i:global_水面高さ
				#endif
				eval( $f:global_水中環境音フラグ = 1 )
				command ＳＥセットモード -s d:se_code:SD_A_BGWATR01 -p 0 0 0 -m 1	// 水中サウンドセット
				command セットサウンドコード -c d:SNG_SYUKAN_ON
				if ( $f:ＢＧＭマネージャー起動フラグ == 1 ) {
					#if d:DEBUG_PRINT
						print 'BGMマネージャー FADER2'
					#endif
					mesg ＢＧＭマネージャー ＢＧＭマネ SD_SNEAK_AUTO d:SD_AUTO_FADER2
				} else {
					#if d:DEBUG_PRINT
						print 'SDマネージャー FADER2'
					#endif
					mesg サウンドマネージャー ＳＤマネ SD_CODE d:SD_AUTO_FADER2
				}
			}
		} else {
			 if( $f:global_水中環境音フラグ == 1 ) {
				#if d:DEBUG_PRINT
					print '水中環境音ＯＦＦ'
					print '$i:カメラＹ位置='$i:カメラＹ位置
					print '$i:global_水面高さ='$i:global_水面高さ
				#endif
				eval( $f:global_水中環境音フラグ = 0 )
				command セットサウンドコード -c d:SE_JOUCHUU_OFF					// 水中サウンドストップ
				command セットサウンドコード -c d:SNG_SYUKAN_OFF
				if ( $f:ＢＧＭマネージャー起動フラグ == 1 ) {
					#if d:DEBUG_PRINT
						print 'BGMマネージャー FADER1'
					#endif
					mesg ＢＧＭマネージャー ＢＧＭマネ SD_SNEAK_AUTO d:SD_AUTO_FADER1
				} else {
					#if d:DEBUG_PRINT
						print 'SDマネージャー FADER1'
					#endif
					mesg サウンドマネージャー ＳＤマネ SD_CODE d:SD_AUTO_FADER1
				}
			}
		}
	}
}

proc カメラ前曇り制御 $:水中トラップ名 {
	#if d:DEBUG_PRINT
		print 'camera_cloud_set'
	#endif

	trap $:水中トラップ名 d:PLAYER \
		-mask ＊ \
		-exec {
			// 水中モードの時に判定
			if ( $f:水中判定フラグ == 1 ) {
				if( $i:カメラＹ位置 > d:WATER_SURFACE ) {
					if( $f:カメラ前曇りフラグ == 0 ) {
						eval( $f:カメラ前曇りフラグ = 1 );
						chara カメラ前曇り カメラ曇り -t 250
						chara カメラ水膜 カメラ水膜
					}
				} else {
					if( $f:カメラ前曇りフラグ == 1 ) {
						eval( $f:カメラ前曇りフラグ = 0 );
						mesg カメラ前曇り カメラ曇り 殺す
					}
				}
			}

			command プレイヤー状態取得
			if ( $status & d:PFLAG_WATER && $i:カメラＹ位置 <= d:WATER_SURFACE ) {
			// 水中に潜ったら曇り判定を行う
				eval ( $f:水中判定フラグ = 1 );
			} else if ( $3 == 出る ){
				eval ( $f:水中判定フラグ = 0 );
			}
		}
}

/***************************************************************************************************/
// 入水プロック w11a w11bは独自に水中処理を行う
/***************************************************************************************************/
#ifdef d:STAGE_W11A
#elifdef d:STAGE_W11B
#elifdef d:DEMO_STAGE
#else

#define WATER_IN_ALONE		500
#define WATER_IN_EE 		1000
#define WATER_OUT_ALONE		0
#define WATER_OUT_EE		500

	proc 入水エマ幅チェック $:ライデン座標 $:エマ座標 {
		if( (($:ライデン座標 - $:エマ座標) <= 1000 ) && (($:ライデン座標 - $:エマ座標) >= -1000 ) ) {
			return 1
		} else {
			return 0
		}
	}
	proc 入水エマ奥行きチェック $:ライデン座標 $:エマ座標 {
		if( (($:ライデン座標 - $:エマ座標) <= 2000 ) && (($:ライデン座標 - $:エマ座標) >= -2000 ) ) {
			return 1
		} else {
			return 0
		}
	}

	proc 入水振動 {
		chara パッド振動 入出水振動 -vibfile rai_dive_02
	}

	proc 出水振動 {
		chara パッド振動 入出水振動 -vibfile rai_surfacing_02
	}

	proc 入出水チェック {
		if( ( $f:入出水処理中フラグ == 0 ) && (`command ゲームオーバーチェック` == 0 ) ) {
			return 1
		} else {
			return 0
		}
	}

	proc 出水最終処理 {
		print '出水最終処理'
		eval( $f:入出水処理中フラグ = 0 )
		@シナリオデモ終了処理

		#ifdef d:WATER_CAMERA
			chara カメラ設定 モーションカメラ -s -1
		#endif

		command トラップ切り替え -trap puddle -switch 1
		chara delay 足跡トラップ消しディレイ -time 2 -exec {
			command トラップ切り替え -trap puddle -switch 0
		}
	}

	proc 入水最終処理 {
		chara delay 入水最終処理遅らせディレイ -time 60 -exec {
			print '入水最終処理'
			eval( $f:入出水処理中フラグ = 0 )

			@シナリオデモ終了処理
			#ifdef d:WATER_CAMERA
				chara カメラ設定 モーションカメラ -s -1
			#endif
		}
	}

/***************************************************************************************************/
// 入水処理実行
/***************************************************************************************************/
	proc 入水処理実行 $:Ｘ $:Ｙ $:Ｚ $:方向 $:距離 $:独り用ディレイ $:二人用ディレイ {
		print '入水処理実行  入出水処理中フラグ  立った'
		eval( $f:入出水処理中フラグ = 1 )

		@特殊装備解除
		@ダンボール解除
		@シナリオデモ開始処理

		mesg カメラ泡     泡ぶくマン オン
		mesg カメラダスト ダストマン オン

		if ( $w:アイテム == d:アイテム:煙草 ) {
			eval($w:アイテム = d:アイテム:素手);
		}

		// エマがいるかチェック
		if ( $f:エマ存在フラグ == 0 ) {
			eval( $i:ローカル変数Ｉ１ = 0 )
		} else {
			command ゲットエマ手繋ぎ状況 $i:ローカル変数Ｉ１
		}

		// 入水処理実行
		if ( $i:ローカル変数Ｉ１ ) {
			@エマと一緒に入水処理 $1 $2 $3 $4 $5
			chara delay 入出水振動ディレイ -time $:二人用ディレイ -proc 入水振動

			#ifdef d:WATER_CAMERA2
				@背負いカメラＩＮ
			#endif
		} else {
			@独りで入水処理 $1 $2 $3 $4 $5
			chara delay 入出水振動ディレイ -time $:独り用ディレイ -proc 入水振動
			#ifdef d:WATER_CAMERA2
				@移動中カメラＩＮ
			#endif
		}

	#ifdef d:STAGE_W31C
		// 息継ぎポイントのスポットライトをオンにする
		mesg スポットライト背景イメージ指定 新スポットライト テクスチャ許可 1
	#endif
	}

///////////////////////
// 独りで入水処理
	proc 独りで入水処理 $:Ｘ $:Ｙ $:Ｚ $:方向 $:距離 {
		print '独りで入水処理'
		mesg 通路用水面 キラキラ水面 波発生源追加 d:PLAYER

		command 水中出入り時プロック -i 入り開始 入り完了 -o 出る開始 出る完了

		eval( $f:エマと一緒に水の中 = 0 )

		//mesg プレイヤー d:PLAYER position  $:Ｘ ($:Ｙ+1000) $:Ｚ
		mesg プレイヤー d:PLAYER water 階段入る ($:距離+d:WATER_IN_ALONE) $:方向 $:Ｘ $:Ｚ ($:Ｙ+1000)
	}

///////////////////////
// エマと一緒に入水処理
	proc エマと一緒に入水処理 $:Ｘ $:Ｙ $:Ｚ $:方向 $:距離 {
		#if d:DEBUG_PRINT
			print 'エマと一緒に入水処理'
		#endif

		mesg エマ・エメリッヒ 手繋ぎエマ 背中Ｃ４落す

		mesg 通路用水面 キラキラ水面 波発生源追加 d:PLAYER
		mesg 通路用水面 キラキラ水面 波発生源追加 引っ付き水中エマ

		eval( $f:エマと一緒に水の中 = 1 )
		eval( $w:入出水方向 = $:方向 )
		eval( $w:移動距離 = $:距離 )

		command 水中出入り時プロック -i 入り開始 入り完了 -o 出る開始 出る完了

		///////////////////
		// ライデン側の設定
		mesg プレイヤー d:PLAYER position  $:Ｘ ($:Ｙ+1000) $:Ｚ
		mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:rai_ema_onbu_start \
					$w:入出水方向 \
					$w:入出水方向 \
					0 \
					d:FA_NO_CHECK_TRP

		command 強制モーション終了プロック \
			-motion d:モーションリスト:rai_ema_onbu_start \
			-proc 背負い完了

		///////////////
		// エマ側の設定
		if( $w:入出水方向 == d:NORTH ) {
			eval( $i:ローカル変数Ｉ１ = 496 )
			eval( $i:ローカル変数Ｉ２ = 856 )
			eval( $i:ローカル変数Ｉ３ = 593 )
		} else if( $w:入出水方向 == d:SOUTH ) {
			eval( $i:ローカル変数Ｉ１ = -496 )
			eval( $i:ローカル変数Ｉ２ = 856 )
			eval( $i:ローカル変数Ｉ３ = -593 )
		} else if( $w:入出水方向 == d:EAST ) {
			eval( $i:ローカル変数Ｉ１ = -593 )
			eval( $i:ローカル変数Ｉ２ = 856 )
			eval( $i:ローカル変数Ｉ３ = 496 )
		} else if( $w:入出水方向 == d:WEST ) {
			eval( $i:ローカル変数Ｉ１ = 593 )
			eval( $i:ローカル変数Ｉ２ = 856 )
			eval( $i:ローカル変数Ｉ３ = -496)
		}

		mesg エマ・エメリッヒ 手繋ぎエマ 強制手繋ぎ解除
		mesg 水中エマ 引っ付き水中エマ position $w:入出水方向 \
				0 ( $:Ｘ + $i:ローカル変数Ｉ１ ) \
				1 ( $:Ｙ + $i:ローカル変数Ｉ２ ) \
				2 ( $:Ｚ + $i:ローカル変数Ｉ３ )

		mesg 水中エマ 引っ付き水中エマ motion 0 0 背負われ開始 NULL 0
		mesg エマ・エメリッヒ 手繋ぎエマ 待機
	}

	proc 背負い完了 {
		print '背負い完了'
		#ifdef d:WATER_CAMERA2
			@移動中カメラＩＮ
		#endif

		mesg プレイヤー d:PLAYER water 階段入る ($w:移動距離+d:WATER_IN_EE) $w:入出水方向
	}

	proc 背負われ開始 {
		print '背負われ開始'
		command ゲットエマライフ $i:エマライフ現在値
		command 水中エマライフセット -v $i:エマライフ現在値
	}

/***************************************************************************************************/
// 出水プロック
/***************************************************************************************************/
	proc 出水処理実行 $:Ｘ $:Ｙ $:Ｚ $:方向 $:距離 $:独り用ディレイ $:二人用ディレイ {
		if( ( $f:入出水処理中フラグ == 0 ) && (`command ゲームオーバーチェック` == 0 ) ) {
			print '出水処理実行  入出水処理中フラグ  立った'
			eval( $f:入出水処理中フラグ = 1 )
			@シナリオデモ開始処理
			mesg カメラ泡     泡ぶくマン オフ
			mesg カメラダスト ダストマン オフ


			chara キャラ付随水飛抹 出水飛沫ライデン \
				-nconcname d:PLAYER \
				-addone 32 \
				-time d:CHARA_SPLASH_TIME \
				-rgbadd 0x40404030 \
				-gbsub 0x40404030

			#ifdef d:WATER_CAMERA2
				@移動中カメラＯＵＴ
			#endif
			if( $f:エマと一緒に水の中 == 1 ) {
				chara キャラ付随水飛抹 出水飛沫水中エマ \
					-nconcname 引っ付き水中エマ \
					-addone 32 \
					-time d:CHARA_SPLASH_TIME \
					-rgbadd 0x40404030 \
					-gbsub 0x40404030

				chara キャラ付随水飛抹 出水飛沫手繋ぎエマ \
					-nconcname 手繋ぎエマ \
					-addone 32 \
					-time d:CHARA_SPLASH_TIME \
					-rgbadd 0x40404030 \
					-gbsub 0x40404030

				@エマと一緒に出水処理 $1 $2 $3 $4 $5
				chara delay 入出水振動ディレイ -time $:二人用ディレイ -proc 出水振動
			} else {
				@独りで出水処理 $1 $2 $3 $4 $5
				chara delay 入出水振動ディレイ -time $:独り用ディレイ -proc 出水振動
			}
		}

	#ifdef d:STAGE_W31C
		// 息継ぎポイントのスポットライトをオフにする
		mesg スポットライト背景イメージ指定 新スポットライト テクスチャ許可 0
	#endif

	}

///////////////////////
// 独りで出水処理
	proc 独りで出水処理 $:Ｘ $:Ｙ $:Ｚ $:方向 $:距離 {
		#if d:DEBUG_PRINT
			print '独りで出水処理'
		#endif
		mesg 通路用水面 キラキラ水面 波発生源削除 d:PLAYER

		command 水中出入り時プロック -i 入り開始 入り完了 -o 出る開始 出る完了

		mesg プレイヤー d:PLAYER position  $:Ｘ ($:Ｙ+1000) $:Ｚ
		mesg プレイヤー d:PLAYER water 階段出る ($:距離+d:WATER_OUT_ALONE) $:方向
	}

///////////////////////
// エマと一緒に出水処理
	proc エマと一緒に出水処理 $:p_Ｘ $:p_Ｙ $:p_Ｚ $:p_方向 $:p_距離 {
		#if d:DEBUG_PRINT
			print 'エマと一緒に出水処理'
		#endif
		mesg 通路用水面 キラキラ水面 波発生源削除 d:PLAYER
		mesg 通路用水面 キラキラ水面 波発生源削除 引っ付き水中エマ

		eval( $w:入出水方向 = $:p_方向 )
		eval( $w:移動距離 = $:p_距離 )
		eval( $i:Ｘ = $:p_Ｘ )
		eval( $i:Ｙ = $:p_Ｙ )
		eval( $i:Ｚ = $:p_Ｚ )

		command 水中出入り時プロック -i 入り開始 入り完了 -o 出る開始 エマと一緒に出る完了

		mesg プレイヤー d:PLAYER position  $:p_Ｘ ($:p_Ｙ+1000) $:p_Ｚ

		mesg プレイヤー d:PLAYER water 階段出る ($w:移動距離+d:WATER_OUT_EE) $w:入出水方向

	}

	proc エマ出水時開始プロック {
		print 'エマ出水時開始プロック'
	}

	proc エマ出水時終了プロック {
		eval( $f:エマと一緒に水の中 = 0 )
		print '$f:エマと一緒に水の中'$f:エマと一緒に水の中

		mesg エマ・エメリッヒ 手繋ぎエマ 復帰

/*
		chara キャラ付随水飛抹 出水飛沫手繋ぎエマ \
			-nconcname 手繋ぎエマ \
			-addone 32 \
			-time d:CHARA_SPLASH_TIME \
			-rgbadd 0x40404030 \
			-gbsub 0x40404030
*/
		// 位置調整
		if( $w:入出水方向 == d:SOUTH ) {
			//mesg エマ・エメリッヒ 手繋ぎエマ 強制瞬間移動 \
			//			($i:プレイヤー位置Ｘ-550) ($i:プレイヤー位置Ｙ+115) ($i:プレイヤー位置Ｚ/*-223*/-700)
			command  エマ強制瞬間移動 \
						($i:プレイヤー位置Ｘ-550) ($i:プレイヤー位置Ｙ+115) ($i:プレイヤー位置Ｚ/*-223*/-700)
		} else if( $w:入出水方向 == d:NORTH ) {
			//mesg エマ・エメリッヒ 手繋ぎエマ 強制瞬間移動 \
			//			($i:プレイヤー位置Ｘ+600) ($i:プレイヤー位置Ｙ+115) ($i:プレイヤー位置Ｚ/*+223*/+600)
			command  エマ強制瞬間移動 \
						($i:プレイヤー位置Ｘ+600) ($i:プレイヤー位置Ｙ+115) ($i:プレイヤー位置Ｚ/*+223*/+600)
		} else if( $w:入出水方向 == d:EAST ) {
			//mesg エマ・エメリッヒ 手繋ぎエマ 強制瞬間移動 \
			//			($i:プレイヤー位置Ｘ-700) ($i:プレイヤー位置Ｙ+115) ($i:プレイヤー位置Ｚ/*-25*/+600)
			command  エマ強制瞬間移動 \
						($i:プレイヤー位置Ｘ-700) ($i:プレイヤー位置Ｙ+115) ($i:プレイヤー位置Ｚ/*-25*/+600)
		} else if( $w:入出水方向 == d:WEST ) {
			//mesg エマ・エメリッヒ 手繋ぎエマ 強制瞬間移動 \
			//			($i:プレイヤー位置Ｘ+600) ($i:プレイヤー位置Ｙ+115) ($i:プレイヤー位置Ｚ/*+25*/-250)
			command  エマ強制瞬間移動 \
						($i:プレイヤー位置Ｘ+600) ($i:プレイヤー位置Ｙ+115) ($i:プレイヤー位置Ｚ/*+25*/-250)
		}
		// 方向指定
		mesg エマ・エメリッヒ 手繋ぎエマ 強制方向指定 $w:入出水方向

		// ライフの受け渡し
		command 水中エマライフゲット
		mesg エマ・エメリッヒ 手繋ぎエマ セットライフ $status

		// 水中エマ終了
		mesg 水中エマ 引っ付き水中エマ off

		@出水最終処理

/*
		#ifdef d:WATER_CAMERA
			chara カメラ設定 モーションカメラ -s -1
		#endif

		command トラップ切り替え -trap puddle -switch 1
		chara delay 足跡トラップ消しディレイ -time 1 -exec {
			command トラップ切り替え -trap puddle -switch 0
		}
*/
	}


	proc 入り開始 {
		print '入り開始'
	}
	proc 入り完了 {

		mesg キャラ付随水飛抹 出水飛沫ライデン 破棄

		if( $f:エマと一緒に水の中 == 1 ) {
			print 'エマと一緒なので 出水飛沫水 停止'
			mesg キャラ付随水飛抹 出水飛沫水中エマ   破棄
			mesg キャラ付随水飛抹 出水飛沫手繋ぎエマ 破棄
		}

		@入水最終処理
	}

	proc 出る開始 {
		print '出る開始'
	}

	proc 出る完了 {
		print '出る完了'
		@出水最終処理
	}

	proc エマと一緒に出る完了 {
		print 'エマと一緒に出る完了'
		#ifdef d:WATER_CAMERA2
			@背負いカメラＯＵＴ
		#endif

		if( $w:入出水方向 == d:SOUTH ) {
			eval( $i:ローカル変数Ｉ１ = -25 )
			eval( $i:ローカル変数Ｉ２ = 1163 )
			eval( $i:ローカル変数Ｉ３ = -223 )
		} else if ( $w:入出水方向 == d:NORTH ) {
			eval( $i:ローカル変数Ｉ１ = 25 )
			eval( $i:ローカル変数Ｉ２ = 1163 )
			eval( $i:ローカル変数Ｉ３ = 223 )
		} else if ( $w:入出水方向 == d:EAST ) {
			eval( $i:ローカル変数Ｉ１ = -223 )
			eval( $i:ローカル変数Ｉ２ = 1163 )
			eval( $i:ローカル変数Ｉ３ = 25 )
		} else if ( $w:入出水方向 == d:WEST ) {
			eval( $i:ローカル変数Ｉ１ = 223 )
			eval( $i:ローカル変数Ｉ２ = 1163 )
			eval( $i:ローカル変数Ｉ３ = -25 )
		}

		mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:rai_ema_onbu_end \
					$w:入出水方向 \
					$w:入出水方向 \
					0 \
					d:FA_NO_CHECK_TRP

		mesg 水中エマ 引っ付き水中エマ position $w:入出水方向 \
					0 ($i:プレイヤー位置Ｘ + $i:ローカル変数Ｉ１ ) \
					1 ( ( ( $i:Ｙ + $w:移動距離 ) + $i:ローカル変数Ｉ２ ) ) \
					2 (($i:プレイヤー位置Ｚ + $i:ローカル変数Ｉ３))

		print '床位置' ( $i:Ｙ + $w:移動距離 )
		print '$i:ローカル変数Ｉ２:' $i:ローカル変数Ｉ２
		print 'エマ座標'( ( ( $i:Ｙ + $w:移動距離 ) + $i:ローカル変数Ｉ２ ) )

		mesg 水中エマ 引っ付き水中エマ motion 1 \
					1 \
					エマ出水時開始プロック \
					エマ出水時終了プロック \
					0

	}

#endif
#endif

///////////////////////////////////////////////////////////////////////
// エマ存在チェック
///////////////////////////////////////////////////////////////////////
proc エマエレベーター内チェック $:ロード先ステージ名 {
	#if d:DEBUG_PRINT
		print 'emma_elv_check'
	#endif

	// エマがそのステージにいるときのみチェックをかける
	if ( $f:エマ存在フラグ == 1 ) {
		command ゲットエマ座標 $i:エマＸ座標変数 $i:エマＹ座標変数 $i:エマＺ座標変数
		if ( $i:エマＸ座標変数 > -1750 && $i:エマＸ座標変数 < 1750 && \
			$i:エマＺ座標変数 <= -253500 ) {
			#if d:DEBUG_PRINT
				print 'emma_elv_in!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'
				print $i:エマＸ座標変数
				print $i:エマＺ座標変数
			#endif
			eval( $s:エマ存在ステージ = $:ロード先ステージ名 );
			eval( $f:エマエレベータ内フラグ = 1 );
		} else {
			#if d:DEBUG_PRINT
				print 'emma_elv_out!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'
				print $i:エマＸ座標変数
				print $i:エマＺ座標変数
			#endif
			eval( $f:エマエレベータ内フラグ = 0 );
		}
	}
}


///////////////////////////////////////////////////////////////////////
// エマゲームオーバー時プロック
///////////////////////////////////////////////////////////////////////
proc 手繋ぎエマゲームオーバー {
	#if d:DEBUG_PRINT
		print 'tetunagi_emma_gameover'
	#endif

	eval( $f:pgo_エマ死亡 = 1 );
	//　エマが死んだときにニキータ主観だったときの対処
	command サイト表示制御 -switch off

#ifdef d:EMMA_GEOVER_CAMERA
	@エマゲームオーバー時カメラ
#endif
}

proc 水中エマゲームオーバー {
	#if d:DEBUG_PRINT
		print 'suityuu_emma_gameover'
	#endif

	eval( $f:pgo_エマ死亡 = 1 );
}


///////////////////////////////////////////////////////////////////////
// メニューフナ虫用プロック
///////////////////////////////////////////////////////////////////////
proc メニューフナムシ取り付き設定 {
	#if d:DEBUG_PRINT
		print 'menu_funa_check_set'
	#endif

	// 身体を這うフナムシ これがついたらメニューにフナムシがつく
	chara 体舟虫 身体虫 -j 4
	chara 体舟虫 身体虫 -j 5
	chara 体舟虫 身体虫 -j 6
	chara 体舟虫 身体虫 -j 8
	chara 体舟虫 身体虫 -j 9
	chara 体舟虫 身体虫 -j 10
	chara 体舟虫 身体虫 -j 13
	chara 体舟虫 身体虫 -j 14
	chara 体舟虫 身体虫 -j 15
	chara 体舟虫 身体虫 -j 16
	chara 体舟虫 身体虫 -j 17
	chara 体舟虫 身体虫 -j 18
	chara 体舟虫 身体虫 -j 19
	chara 体舟虫 身体虫 -j 20

	trap menu_funa d:PLAYER \
		-mask ？ \
		-state d:TRP_STATE_GROUND \
		-exec {
			if( $3 == 入る ) {
				@身体フナムシチェック
			} else {
				@身体フナムシ取れる
			}
		}
}

proc 身体フナムシチェック {
	#if d:DEBUG_PRINT
		print 'menu_funa_check_start'
	#endif

	chara delay 身体這いフナムシチェックディレイ -time 120 -exec {
		#if d:DEBUG_PRINT
			print 'フナムシついた'
		#endif
		mesg メニューフナムシ メニューフナ虫 transfer
		mesg 体舟虫 身体虫 這う
	}
}

proc 身体フナムシ取れる {
	#if d:DEBUG_PRINT
		print 'menu_funa_check_cancel'
		print 'フナムシとれた'
	#endif
	mesg delay 身体這いフナムシチェックディレイ kill
	mesg 体舟虫 身体虫 落とす
}



///////////////////////////////////////////////////////////////////////
// 操作説明テクスチャ表示
///////////////////////////////////////////////////////////////////////
#ifdef d:EX_TEX
proc 説明テクスチャ表示 $:キャラ名 $:テクスチャ $:表示時間 {
	#if d:DEBUG_PRINT
		print 'ex_tex_on'
	#endif

	command foreach -argc 5 -repeat 2 \
		-data { \
			{$1,$2,$3,32,d:SPRT2D_FLAG_ALPHA}\
			{$1,$2,$3,32,d:SPRT2D_FLAG_ADD}\
		} \
		-exec { \
			chara ２Ｄスプライト表示 $1 \
				-tri padtexture \
				-x $2 \
				-pos 16 38 \
				//-size $i:サイズＷ $i:サイズＨ \
				//-b $i:スケール ... \
				-count d:TEX_FADE_TIME,$3,d:TEX_FADE_TIME \
				-alp $4 \
				-flag $5
		}
}
#endif



///////////////////////////////////////////////////////////////////////
// 空関係
///////////////////////////////////////////////////////////////////////

proc 朝空 $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ {

	chara 汎用空 空 \
		-c 90 110 95 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l 32 \
		-t 4 \
			sky_m1_alp 200 211 200 100 \
			sky_m2_alp 155 165 145 50 \
			0 0 0 0 0 \
			sky_m4_alp 148 171 148 50 \
		-m 1 \
			m_sky_wall1 53 64 55 -100 0

	chara 太陽 ソル \
		-p $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ 		//中心の位置
		-f $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ \	//平行光源の-pと同じ値
		-a 11 \   //alpha
		-l 32 \
		-w (240*32) \
		-h (240*32) \
		-t sky_sun_m_alp
}
proc 朝空（レンズフレア指定） $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ $:レンズフレア用グループ番号 {

	chara 汎用空 空 \
		-c 90 110 95 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l 32 \
		-t 4 \
			sky_m1_alp 200 211 200 100 \
			sky_m2_alp 155 165 145 50 \
			0 0 0 0 0 \
			sky_m4_alp 148 171 148 50 \
		-m 1 \
			m_sky_wall1 53 64 55 -100 0

	chara 太陽 ソル \
		-p $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ 		//中心の位置
		-f $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ \	//平行光源の-pと同じ値
		-a 11 \   //alpha
		-l 32 \
		-w (240*32) \
		-h (240*32) \
		-t sky_sun_m_alp \
		-g $:レンズフレア用グループ番号
}


proc 昼空 $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ {

	chara 汎用空 空 \
		-c 150 175 175 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l 32 \
		-t 4 \
			sky_d1_alp 180 196 175 100 \
			sky_d2_alp 152 166 154 50 \
			0 0 0 0 0 \
			sky_d4_alp 145 163 155 50 \
		-m 1 \
			d_sky_wall1 88 98 94 -100 0

#ifndef	d:NO_SUN
	chara 太陽 ソル \
		-p $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ 		//中心の位置
		-f $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ \	//平行光源の-pと同じ値
		-a 9 \   //alpha
		-l 32 \
		-w (240*32) \
		-h (240*32) \
		-t sky_sun_d_alp
#endif
}
proc 昼空（レンズフレア指定） $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ $:レンズフレア用グループ番号 {

	chara 汎用空 空 \
		-c 150 175 175 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l 32 \
		-t 4 \
			sky_d1_alp 180 196 175 100 \
			sky_d2_alp 152 166 154 50 \
			0 0 0 0 0 \
			sky_d4_alp 145 163 155 50 \
		-m 1 \
			d_sky_wall1 88 98 94 -100 0

	chara 太陽 ソル \
		-p $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ 		//中心の位置
		-f $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ \	//平行光源の-pと同じ値
		-a 9 \   //alpha
		-l 32 \
		-w (240*32) \
		-h (240*32) \
		-t sky_sun_d_alp \
		-g $:レンズフレア用グループ番号
}


proc ハリアー昼空 $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ {

	chara 汎用空 空 \
		-c 150 175 175 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l 104 \
		-t 4 \
			sky_d1_alp 180 196 175 100 \
			sky_d2_alp 152 166 154 50 \
			0 0 0 0 0 \
			sky_d4_alp 145 163 155 50 \
		-m 1 \
			d_sky_wall1 88 98 94 -100 0

	chara 太陽 ソル \
		-p $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ 		//中心の位置
		-f $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ \	//平行光源の-pと同じ値
		-a 9 \   //alpha
		-l 32 \
		-w (240*32) \
		-h (240*32) \
		-t sky_sun_d_alp
}
proc ハリアー昼空（レンズフレア指定） $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ $:レンズフレア用グループ番号 {

	chara 汎用空 空 \
		-c 150 175 175 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l 104 \
		-t 4 \
			sky_d1_alp 180 196 175 100 \
			sky_d2_alp 152 166 154 50 \
			0 0 0 0 0 \
			sky_d4_alp 145 163 155 50 \
		-m 1 \
			d_sky_wall1 88 98 94 -100 0

	chara 太陽 ソル \
		-p $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ 		//中心の位置
		-f $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ \	//平行光源の-pと同じ値
		-a 9 \   //alpha
		-l 32 \
		-w (240*32) \
		-h (240*32) \
		-t sky_sun_d_alp \
		-g $:レンズフレア用グループ番号
}


proc 夕空 $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ {

	chara 汎用空 空 \
		-c 50 47 38 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l 32 \
		-t 4 \
			sky_e1_alp 116 106 96 50 \
			sky_e2_alp 94 89 79 50 \
			0 0 0 0 0 \
			sky_e4_alp 78 73 66 50 \
		-m 1 \
			e_sky_wall1 47 35 21 -100 0

	chara 太陽 ソル \
		-p $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ 		//中心の位置
		-f $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ \	//平行光源の-pと同じ値
		-a 11 \   //alpha
		-l 32 \
		-w (415*32) \
//		-h (687*16) \
		-h (344*32) \
		-t sky_sun_e_alp
}
proc 夕空（レンズフレア指定） $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ $:レンズフレア用グループ番号 {

	chara 汎用空 空 \
		-c 50 47 38 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l 32 \
		-t 4 \
			sky_e1_alp 116 106 96 50 \
			sky_e2_alp 94 89 79 50 \
			0 0 0 0 0 \
			sky_e4_alp 78 73 66 50 \
		-m 1 \
			e_sky_wall1 47 35 21 -100 0

	chara 太陽 ソル \
		-p $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ 		//中心の位置
		-f $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ \	//平行光源の-pと同じ値
		-a 11 \   //alpha
		-l 32 \
		-w (415*32) \
//		-h (687*16) \
		-h (344*32) \
		-t sky_sun_e_alp \
		-g $:レンズフレア用グループ番号
}


proc Ｗ２１Ｂ夕空 $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ {

	chara 汎用空 空 \
		-c 44 43 38 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l 32 \
		-t 4 \
			sky_e1_alp 116 106 96 50 \
			sky_e2_alp 94 89 79 50 \
			w21b_sky_e3_alp 165 107 58 0 \
			sky_e4_alp 74 69 62 50 \
/*
		-m 4 \
			w21b_sky_wall1 39 31 21 -100 0 \
			w21b_sky_wall2 114 84 37 0 3400 \
			w21b_sky_wall3 146 91 43 0 3000 \
			w21b_sky_wall4 144 123 60 0 2900
*/
		-m 3 \
			w21b_sky_wall1 39 31 21 -100 0 \
			w21b_sky_wall2 114 84 37 0 3400 \
			w21b_sky_wall3 146 91 43 0 3000



	chara 太陽 ソル \
		-p $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ 		//中心の位置
		-f $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ \	//平行光源の-pと同じ値
		-a 13 \   //alpha
		-l 32 \
		-w (415*32) \
//		-h (687*16) \
		-h (344*32) \
		-t sky_sun_e_alp
}
proc Ｗ２１Ｂ夕空（レンズフレア指定） $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ $:レンズフレア用グループ番号 {

	chara 汎用空 空 \
		-c 44 43 38 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l 32 \
		-t 4 \
			sky_e1_alp 116 106 96 50 \
			sky_e2_alp 94 89 79 50 \
			w21b_sky_e3_alp 165 107 58 0 \
			sky_e4_alp 74 69 62 50 \
/*
		-m 4 \
			w21b_sky_wall1 39 31 21 -100 0 \
			w21b_sky_wall2 114 84 37 0 3400 \
			w21b_sky_wall3 146 91 43 0 3000 \
			w21b_sky_wall4 144 123 60 0 2900
*/
		-m 3 \
			w21b_sky_wall1 39 31 21 -100 0 \
			w21b_sky_wall2 114 84 37 0 3400 \
			w21b_sky_wall3 146 91 43 0 3000

	chara 太陽 ソル \
		-p $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ 		//中心の位置
		-f $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ \	//平行光源の-pと同じ値
		-a 13 \   //alpha
		-l 32 \
		-w (415*32) \
//		-h (687*16) \
		-h (344*32) \
		-t sky_sun_e_alp \
		-g $:レンズフレア用グループ番号
}


proc 狙撃夕空 $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ {

	chara 汎用空 空 \
		-c 50 47 38 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l 32 \
		-t 4 \
			sky_e1_alp 116 106 96 50 \
			sky_e2_alp 94 89 79 50 \
			0 0 0 0 0 \
			sky_e4_alp 78 73 66 50 \
		-m 3 \
			w32a_sky_wall1 47 35 21 -100 0 \
			w32a_sky_wall2 177 106 55 0 -400 \
			w32a_sky_wall3 157 114 58 0 -900

	chara 太陽 ソル \
		-p $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ 		//中心の位置
		-f $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ \	//平行光源の-pと同じ値
		-a 11 \   //alpha
		-l 32 \
		-w (415*32) \
//		-h (687*16) \
		-h (344*32) \
		-c 128,128,128 8 \		//キラキラの色指定しなければキラキラ起動しません。
		-t sky_sun_e_alp
}
proc 狙撃夕空（レンズフレア指定） $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ $:レンズフレア用グループ番号 {

	chara 汎用空 空 \
		-c 50 47 38 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l 32 \
		-t 4 \
			sky_e1_alp 116 106 96 50 \
			sky_e2_alp 94 89 79 50 \
			0 0 0 0 0 \
			sky_e4_alp 78 73 66 50 \
		-m 3 \
			w32a_sky_wall1 47 35 21 -100 0 \
			w32a_sky_wall2 177 106 55 0 -400 \
			w32a_sky_wall3 157 114 58 0 -900

	chara 太陽 ソル \
		-p $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ 		//中心の位置
		-f $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ \	//平行光源の-pと同じ値
		-a 11 \   //alpha
		-l 32 \
		-w (415*32) \
//		-h (687*16) \
		-h (344*32) \
		-c 128,128,128 8 \		//キラキラの色指定しなければキラキラ起動しません。
		-t sky_sun_e_alp \
		-g $:レンズフレア用グループ番号
}


proc Ｗ２５Ｄ夕空 $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ {

	chara 汎用空 空 \
		-c 58 47 38 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l 32 \
		-t 4 \
			sky_e1_alp 116 106 96 50 \
			sky_e2_alp 94 89 79 50 \
			w25d_sky_e3_alp 160 112 58 0 \
			sky_e4_alp 78 73 66 50 \
		-m 3 \
			w25d_sky_wall1 47 35 21 -100 0 \
			w25d_sky_wall2 114 84 37 0 3000 \
			w25d_sky_wall3 135 100 47 0 3000

	chara 太陽 ソル \
		-p $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ 		//中心の位置
		-f $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ \	//平行光源の-pと同じ値
		-a 11 \   //alpha
		-l 32 \
		-w (415*32) \
//		-h (687*16) \
		-h (344*32) \
		-t sky_sun_e_alp
}
proc Ｗ２５Ｄ夕空（レンズフレア指定） $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ $:レンズフレア用グループ番号 {

	chara 汎用空 空 \
		-c 58 47 38 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l 32 \
		-t 4 \
			sky_e1_alp 116 106 96 50 \
			sky_e2_alp 94 89 79 50 \
			w25d_sky_e3_alp 160 112 58 0 \
			sky_e4_alp 78 73 66 50 \
		-m 3 \
			w25d_sky_wall1 47 35 21 -100 0 \
			w25d_sky_wall2 114 84 37 0 3000 \
			w25d_sky_wall3 135 100 47 0 3000

	chara 太陽 ソル \
		-p $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ 		//中心の位置
		-f $:太陽向きＸ $:太陽向きＹ $:太陽向きＺ \	//平行光源の-pと同じ値
		-a 11 \   //alpha
		-l 32 \
		-w (415*32) \
//		-h (687*16) \
		-h (344*32) \
		-t sky_sun_e_alp \
		-g $:レンズフレア用グループ番号
}

proc 夜空 $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ {
	chara 汎用空 空 \
		-c 9 19 21 \
		-pos $:空位置Ｘ $:空位置Ｙ $:空位置Ｚ \ //　各ステージの空の位置
		-f 0 \
		-l (32*10) \
		-t 0 \
			0 0 0 0 0 \
		-m 2 \
			w60a_sky_wall1 91 65 52 -100 0 \
			w60a_sky_wall2 157 133 126 0 3000
}



///////////////////////////////////////////////////////////////////////
// 主観禁止壁
///////////////////////////////////////////////////////////////////////

proc 主観禁止壁 $:壁名前 $:ドア位置Ｘ $:ドア位置Ｙ $:ドア位置Ｚ $:ドア回転 $:ドアスライド幅 {
	chara 透明壁 $:壁名前 \
		-position	$:ドア位置Ｘ $:ドア位置Ｙ $:ドア位置Ｚ 2500 \
					($:ドア位置Ｘ+ ($:ドアスライド幅+100)) $:ドア位置Ｙ $:ドア位置Ｚ 2500 \
		-attribute	( d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_PLAYER ))

		mesg 透明壁 $:壁名前 rotate 1 $:ドア回転
		mesg 透明壁 $:壁名前 off

		//	オンオフについては各自ドアトラップ内で主観のときにオン、そうでないときにオフにして下さい。
}

///////////////////////////////////////////////////////////////////////
// 主観禁止床
///////////////////////////////////////////////////////////////////////
proc 主観禁止床 $:床名前１ $:床名前２ $:ドア位置Ｘ $:ドア位置Ｙ $:ドア位置Ｚ $:ドア回転 $:ドアスライド幅 {
	print '#############################################################################################DAYNAMICFLOOR'
	chara 透明床 主観禁止床 \
		-position	$:ドア位置Ｘ ($:ドア位置Ｙ+2500) ($:ドア位置Ｚ-300) \
		            ($:ドア位置Ｘ+($:ドアスライド幅+100)) ($:ドア位置Ｙ+2500) ($:ドア位置Ｚ-300) \
					($:ドア位置Ｘ+($:ドアスライド幅+100)) ($:ドア位置Ｙ+2500) $:ドア位置Ｚ \
					$:ドア位置Ｘ ($:ドア位置Ｙ+2500) $:ドア位置Ｚ \
		-n_points 4 \
		-attribute	( d:HZX_FLOOR_ATR_ALL & ~(d:HZX_FLOOR_IK | d:HZX_FLOOR_STEP))

	chara 透明床 主観禁止床２ \
		-position	$:ドア位置Ｘ ($:ドア位置Ｙ+2500) $:ドア位置Ｚ \
					($:ドア位置Ｘ+($:ドアスライド幅+100)) ($:ドア位置Ｙ+2500) $:ドア位置Ｚ \
					($:ドア位置Ｘ+($:ドアスライド幅+100)) ($:ドア位置Ｙ+2500) ($:ドア位置Ｚ+400) \
					$:ドア位置Ｘ ($:ドア位置Ｙ+2500) ($:ドア位置Ｚ+400) \
		-n_points 4 \
		-attribute	( d:HZX_FLOOR_ATR_ALL & ~(d:HZX_FLOOR_IK | d:HZX_FLOOR_STEP))

		mesg 透明床 $:床名前１ rotate 4 0 $:ドア回転 0
		mesg 透明床 $:床名前２ rotate 1 0 $:ドア回転 0

}


///////////////////////////////////////////////////////////////////////
// 明るさ可変機能
///////////////////////////////////////////////////////////////////////

proc 明るさ可変機能  $:p_フォグ値Ｒ $:p_フォグ値Ｇ $:p_フォグ値Ｂ $:p_フォグ値ニア $:p_フォグ値ファー $:p_トラップ名 $:p_明るさ可変機能有効高度 {

	//	トラップ内でみるためにいったん変数に渡す
	eval ($i:明るさ可変機能有効高度 = $:p_明るさ可変機能有効高度)

	chara フォグコントローラ 霧管理 \
		-n 3 \
		-d \
		//	デフォルト
		$:p_フォグ値Ｒ $:p_フォグ値Ｇ $:p_フォグ値Ｂ \
		$:p_フォグ値ニア $:p_フォグ値ファー $:p_フォグ値ファー $:p_フォグ値ファー \

		//	黒
		0, 0, 0 \
		$:p_フォグ値ニア $:p_フォグ値ファー $:p_フォグ値ファー $:p_フォグ値ファー \

		//	もとの値と同じもの（０番が可変中にはメッセージを受け取らないため）
		$:p_フォグ値Ｒ $:p_フォグ値Ｇ $:p_フォグ値Ｂ \
		$:p_フォグ値ニア $:p_フォグ値ファー $:p_フォグ値ファー $:p_フォグ値ファー

		//	デフォルトの明るさにする
		mesg フォグコントローラ 霧管理 セット \
			0	1

	//	ハレーション機能
		trap $:p_トラップ名 d:PLAYER \
			-mask  ？ \
			-exec {
				if ($5 > $i:明るさ可変機能有効高度) {
					if ($3 == 入る) {
						//	もとの明るさに戻す
						mesg delay 霧管理ディレイ kill
						mesg フォグコントローラ 霧管理 セット \
						2	1
						mesg プロック連続実行 明るさ管理人 kill

					} else if ($3 == 出る) {

						//	主観のとき以外は（主観攻撃で移動してしまったときは何もしない）
						if ( `command プレイヤー状態取得` & ~d:PFLAG_SUBJECT ) {
							//	一旦真っ暗
							mesg フォグコントローラ 霧管理 セット \
							1	1

							//	徐々に戻っていく
							chara delay 霧管理ディレイ \
								-t 15 \
								-e {
									mesg フォグコントローラ 霧管理 セット \
										0	60
								}
						}
						//	明るさ管理人
						chara プロック連続実行 明るさ管理人\
							-t 76 \
							-exec {
								//	主観にしたら(主観で外を見られた場合に空が暗くなっているのを避けるための対処)
								if ( `command プレイヤー状態取得` & d:PFLAG_SUBJECT ) {
									//	もとの明るさに戻す
									mesg delay 霧管理ディレイ kill
									mesg フォグコントローラ 霧管理 セット \
									2	1
									mesg プロック連続実行 明るさ管理人 kill
								}
							}

					}
				}
		}

}






///////////////////////////////////////////////////////////////////////
// タンカーくしゃみ処理
///////////////////////////////////////////////////////////////////////


proc タンカーくしゃみ処理 {

	#if d:DEBUG_PRINT
		print 'タンカーくしゃみ処理'
		print 'tanker_sneeze'
	#endif

	command プラグインくしゃみ

	if($w:風邪状態 == 1){
		@くしゃみ開始
	}

	chara delay タンカーくしゃみディレイ -time 1 -exec{

		if($w:風邪状態 == 0){
			if($i:風邪引きカウンタ > 30 * 5){
				@くしゃみ開始
			}

		}else{
			//print 'くしゃみ中'$w:風邪くしゃみ間隔
			if(`%abs ($i:風邪引き始め時間 - $i:プレイタイム)` > 60 * 60 * 3){
				print 'cold recover!!!!'
				print 'ずずず……'
				mesg delay タンカーくしゃみ繰り返しディレイ kill
				command プレイヤー風邪治し
			}

		}

		return -1

	}




}


proc くしゃみ開始{
	#ifdef d:STAGE_ALT
		if( `@スネーク達` ) {
			print 'get_cold!!!!'
			print '風邪をひきまっしょい！！！！'
			command プレイヤー風邪引かせ -time 100	//$i:くしゃみ間隔（秒）
			//タンカーくしゃみ間隔配列
			command	配列セット $b:タンカーくしゃみ間隔配列[0] {
				10,	5,	20,	12,	16,
				10,	8,	20,	15,	16,
				14,	9,	8,	11,	20,
				15,	6,	20,	22,	15
			}

			command	配列セット $b:タンカーくしゃみ回数配列[0] {
				1,	1,	1,	2,	1,
				1,	3,	1,	1,	1,
				1,	1,	2,	1,	1,
				1,	1,	1,	2,	1
			}
			
			@くしゃみ実行
		}
	#else
		print 'get_cold!!!!'
		print '風邪をひきまっしょい！！！！'
		command プレイヤー風邪引かせ -time 100	//$i:くしゃみ間隔（秒）
		
		@くしゃみ実行
	#endif

	


}



proc くしゃみ実行 {

	//！！！！！注意！！！！！$w:風邪くしゃみ間隔はフレーム単位らしい

	eval($b:くしゃみ実行番号 =0)
	eval($w:風邪くしゃみ間隔 = $b:タンカーくしゃみ間隔配列[0] * 60)
	eval($b:風邪くしゃみ回数 = 0)

	chara delay タンカーくしゃみ繰り返しディレイ -time $w:風邪くしゃみ間隔 -exec{

		if($b:風邪くしゃみ回数 < $b:タンカーくしゃみ回数配列[$b:くしゃみ実行番号] ){

			eval($b:風邪くしゃみ回数 = $b:風邪くしゃみ回数 + 1)

			eval($w:風邪くしゃみ間隔 = 45)
			print $b:風邪くしゃみ回数

			return 45

		}else{

			if($b:くしゃみ実行番号 < 19){
				eval($b:くしゃみ実行番号 = $b:くしゃみ実行番号 + 1)
			}else{
				eval($b:くしゃみ実行番号 = 0)
			}

			eval($b:風邪くしゃみ回数 = 0)


			eval($w:風邪くしゃみ間隔 = $b:タンカーくしゃみ間隔配列[$b:くしゃみ実行番号] * 60)
			print '次は'$b:タンカーくしゃみ間隔配列[$b:くしゃみ実行番号]'秒後！！'

			return $w:風邪くしゃみ間隔

		}


	}


}





proc タンカー風邪引き判定 {
	if($w:風邪状態 == 0){
		if($w:ゲーム設定 >= d:LEVEL_EXTREME || ($w:ゲーム設定 == d:LEVEL_HARD && $i:ステージプレイ時間 >= (60 * 60 * 1) ) ){
			print 'さあーこれからくしゃみするでーー！！！！！'
			command プレイヤー風邪引かせ -time 100	//$i:くしゃみ間隔（秒）
		}
	}
}




















//園山作成
//髭剃りスネークテクスチャ入れ替えプロック
proc 髭剃りスネークテクスチャ入れ替え処理 {
	if ( $w:プレイヤーフラグ & d:PL_GAVE_SHAVER_TO_SNAKE ) {
		chara 髭剃りスネークテクスチャ入れ替え 剃残しなし
	}
}



///////////////////////////////////////////////////////////////////////
// 敵兵状態やりとり（向手・山下用）
///////////////////////////////////////////////////////////////////////

#define		NORMAL		0
#define		STUN		1			//	気絶中
#define		SLEEP		2			//	眠り中
#define		DEAD		3			//	あの世中


//	状態保存
proc 敵兵状態保存 $:双眼鏡兵名 $:屋上兵名 {

	//	双眼鏡兵の状態
	//	気絶か、眠っているかチェック
	command ゲット敵兵状態拡張版 \
		$:双眼鏡兵名 $i:双眼鏡兵状態

#if 0
		//	気絶中
	if ($i:双眼鏡兵状態 & 0x00000001) {
		eval ($b:w20b_２Ｆ敵兵状態 = d:STUN)
		//	眠り中
	} else if ($i:双眼鏡兵状態 & 0x00000002) {
		eval ($b:w20b_２Ｆ敵兵状態 = d:SLEEP)
#else
		//	眠り中
	if ($i:双眼鏡兵状態 & 0x00000002) {
		eval ($b:w20b_２Ｆ敵兵状態 = d:SLEEP)
#endif
		//	それ以外ならば
	} else {
#if 0
		//	死んでいるかをチェック
		command ゲット敵兵状態 \
			$:双眼鏡兵名 $i:双眼鏡兵状態
			//	死んでいる
		if ($i:双眼鏡兵状態 & 0x80000000) {
			eval ($b:w20b_２Ｆ敵兵状態 = d:DEAD)
		//	それ以外は普通に戻す
		} else {
			eval ($b:w20b_２Ｆ敵兵状態 = d:NORMAL)
		}
#else
		//	普通に戻す
		eval ($b:w20b_２Ｆ敵兵状態 = d:NORMAL)
	}
#endif

	//	屋上兵の状態
	//	気絶か、眠っているかチェック
	command ゲット敵兵状態拡張版 \
		$:屋上兵名 $i:屋上兵状態
#if 0
		//	気絶中
	if ($i:屋上兵状態 & 0x00000001) {
		eval ($b:w20b_屋上敵兵状態 = d:STUN)
		//	眠り中
	} else if ($i:屋上兵状態 & 0x00000002) {
		eval ($b:w20b_屋上敵兵状態 = d:SLEEP)
#else
		//	眠り中
	if ($i:屋上兵状態 & 0x00000002) {
		eval ($b:w20b_屋上敵兵状態 = d:SLEEP)
#endif
		//	それ以外ならば
	} else {
#if 0
		//	死んでいるかをチェック
		command ゲット敵兵状態 \
			$:屋上兵名 $i:屋上兵状態
			//	死んでいる
		if ($i:屋上兵状態 & 0x80000000) {
			eval ($b:w20b_屋上敵兵状態 = d:DEAD)
			//	それ以外は普通に戻す
		} else {
			eval ($b:w20b_屋上敵兵状態 = d:NORMAL)
		}
	}
#else
		//	普通に戻す
		eval ($b:w20b_屋上敵兵状態 = d:NORMAL)
	}
#endif
}


//	開始状態チェック
proc 敵兵開始状態チェック	$:ステージ名 $:マップ名 $:双眼鏡兵名 $:Ｘ座標 $:Ｙ座標 $:Ｚ座標 $:向き \
							$:ステージ名２ $:マップ名２ $:屋上兵名 $:Ｘ座標２ $:Ｙ座標２ $:Ｚ座標２ $:向き２ {

	//	以下の条件のときは状態をセットする
/*
	if ((($s:エリア == w19a) && (($s:エリア履歴[ 1 ] == w20b ) || ($s:エリア履歴[ 1 ] == w20c ) || ($s:エリア履歴[ 1 ] == w20d ))) || \
		(($s:エリア == w21a) && (($s:エリア履歴[ 1 ] == w20b ) || ($s:エリア履歴[ 1 ] == w20c ) || ($s:エリア履歴[ 1 ] == w20d ))) || \
		(($s:エリア == w21b) && (($s:エリア履歴[ 1 ] == w20b ) || ($s:エリア履歴[ 1 ] == w20c ) || ($s:エリア履歴[ 1 ] == w20d ))) || \
		(($s:エリア == w20b) && (($s:エリア履歴[ 1 ] == w19a ) || ($s:エリア履歴[ 1 ] == w21a ) || ($s:エリア履歴[ 1 ] == w21b ))) || \
		(($s:エリア == w20c) && (($s:エリア履歴[ 1 ] == w19a ) || ($s:エリア履歴[ 1 ] == w21a ) || ($s:エリア履歴[ 1 ] == w21b ))) || \
		(($s:エリア == w20d) && (($s:エリア履歴[ 1 ] == w19a ) || ($s:エリア履歴[ 1 ] == w21a ) || ($s:エリア履歴[ 1 ] == w21b ))) ) {
*/
	if (((($s:エリア == w19a) || ($s:エリア == w21a)) && (($s:エリア履歴[ 1 ] == w20b ) || ($s:エリア履歴[ 1 ] == w20d ))) || \
		((($s:エリア == w20b) || ($s:エリア == w20d)) && (($s:エリア履歴[ 1 ] == w19a ) || ($s:エリア履歴[ 1 ] == w21a ))) ) {

			//	双眼鏡兵
			//	気絶からスタート
			if ($b:w20b_２Ｆ敵兵状態 == d:STUN) {
				command セット敵兵メモリー \
					$:ステージ名 \
					$:マップ名 \
					$:双眼鏡兵名 \
					$:Ｘ座標 \
					$:Ｙ座標 \
					$:Ｚ座標\
					$:向き \
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
					$:ステージ名 \
					$:マップ名 \
					$:双眼鏡兵名 \
					$:Ｘ座標 \
					$:Ｙ座標 \
					$:Ｚ座標\
					$:向き \
					d:ENEMEM_ST_SLEEP \
					d:ENEMEM_PS_BACK \
					d:SLEEP_TIME_3 \
					$w:敵標準気絶  \
					$w:敵標準体力  \
					0 \
					0

			//	あの世からスタート
			} else if ($b:w20b_２Ｆ敵兵状態 == d:DEAD) {
				command セット敵兵メモリー \
					$:ステージ名 \
					$:マップ名 \
					$:双眼鏡兵名 \
					$:Ｘ座標 \
					$:Ｙ座標 \
					$:Ｚ座標\
					$:向き \
					d:ENEMEM_ST_HELL \
					d:ENEMEM_PS_BACK \
					0 \
					$w:敵標準気絶  \
					$w:敵標準体力  \
					0 \
					0
			}


			//	屋上兵
			//	気絶からスタート
			if ($b:w20b_屋上敵兵状態 == d:STUN) {
				command セット敵兵メモリー \
					$:ステージ名２ \
					$:マップ名２ \
					$:屋上兵名 \
					$:Ｘ座標２ \
					$:Ｙ座標２ \
					$:Ｚ座標２\
					$:向き２ \
					d:ENEMEM_ST_FAINT \
					d:ENEMEM_PS_BACK \
					d:FAINT_TIME_3 \
					$w:敵標準気絶  \
					$w:敵標準体力  \
					0 \
					0

			//	眠りからスタート
			} else if ($b:w20b_屋上敵兵状態 == d:SLEEP) {
				command セット敵兵メモリー \
					$:ステージ名２ \
					$:マップ名２ \
					$:屋上兵名 \
					$:Ｘ座標２ \
					$:Ｙ座標２ \
					$:Ｚ座標２\
					$:向き２ \
					d:ENEMEM_ST_SLEEP \
					d:ENEMEM_PS_BACK \
					d:SLEEP_TIME_3 \
					$w:敵標準気絶  \
					$w:敵標準体力  \
					0 \
					0

			//	あの世からスタート
			} else if ($b:w20b_屋上敵兵状態 == d:DEAD) {
				command セット敵兵メモリー \
					$:ステージ名２ \
					$:マップ名２ \
					$:屋上兵名 \
					$:Ｘ座標２ \
					$:Ｙ座標２ \
					$:Ｚ座標２\
					$:向き２ \
					d:ENEMEM_ST_HELL \
					d:ENEMEM_PS_BACK \
					0 \
					$w:敵標準気絶  \
					$w:敵標準体力  \
					0 \
					0
			}

		//	それ以外はそのままスタート
	} else {
		eval ($b:w20b_２Ｆ敵兵状態 = d:NORMAL)
		eval ($b:w20b_屋上敵兵状態 = d:NORMAL)
	}

}



//	状態保存（w21aステージ専用）
proc Ｗ２１Ａ敵兵状態保存 $:双眼鏡兵名 {

	//	双眼鏡兵の状態
	//	気絶か、眠っているかチェック
	command ゲット敵兵状態拡張版 \
		$:双眼鏡兵名 $i:双眼鏡兵状態

#if 0
		//	気絶中
	if ($i:双眼鏡兵状態 & 0x00000001) {
		eval ($b:w20b_２Ｆ敵兵状態 = d:STUN)
		//	眠り中
	} else if ($i:双眼鏡兵状態 & 0x00000002) {
		eval ($b:w20b_２Ｆ敵兵状態 = d:SLEEP)
#else
		//	眠り中
	if ($i:双眼鏡兵状態 & 0x00000002) {
		eval ($b:w20b_２Ｆ敵兵状態 = d:SLEEP)
#endif
		//	それ以外ならば
	} else {
#if 0
		//	死んでいるかをチェック
		command ゲット敵兵状態 \
			$:双眼鏡兵名 $i:双眼鏡兵状態
			//	死んでいる
		if ($i:双眼鏡兵状態 & 0x80000000) {
			eval ($b:w20b_２Ｆ敵兵状態 = d:DEAD)
			//	それ以外は普通に戻す
		} else {
			eval ($b:w20b_２Ｆ敵兵状態 = d:NORMAL)
		}
	}
#else
		//	普通に戻す
		eval ($b:w20b_２Ｆ敵兵状態 = d:NORMAL)
	}
#endif
}

//	開始状態チェック（w21aステージ専用）
proc Ｗ２１Ａ敵兵開始状態チェック	$:ステージ名 $:マップ名 $:双眼鏡兵名 $:Ｘ座標 $:Ｙ座標 $:Ｚ座標 $:向き {

	//	以下の条件のときは状態をセットする
/*
	if ((($s:エリア == w19a) && (($s:エリア履歴[ 1 ] == w20b ) || ($s:エリア履歴[ 1 ] == w20c ) || ($s:エリア履歴[ 1 ] == w20d ))) || \
		(($s:エリア == w21a) && (($s:エリア履歴[ 1 ] == w20b ) || ($s:エリア履歴[ 1 ] == w20c ) || ($s:エリア履歴[ 1 ] == w20d ))) || \
		(($s:エリア == w21b) && (($s:エリア履歴[ 1 ] == w20b ) || ($s:エリア履歴[ 1 ] == w20c ) || ($s:エリア履歴[ 1 ] == w20d ))) || \
		(($s:エリア == w20b) && (($s:エリア履歴[ 1 ] == w19a ) || ($s:エリア履歴[ 1 ] == w21a ) || ($s:エリア履歴[ 1 ] == w21b ))) || \
		(($s:エリア == w20c) && (($s:エリア履歴[ 1 ] == w19a ) || ($s:エリア履歴[ 1 ] == w21a ) || ($s:エリア履歴[ 1 ] == w21b ))) || \
		(($s:エリア == w20d) && (($s:エリア履歴[ 1 ] == w19a ) || ($s:エリア履歴[ 1 ] == w21a ) || ($s:エリア履歴[ 1 ] == w21b ))) ) {
*/
	if (((($s:エリア == w19a) || ($s:エリア == w21a)) && (($s:エリア履歴[ 1 ] == w20b ) || ($s:エリア履歴[ 1 ] == w20d ))) || \
		((($s:エリア == w20b) || ($s:エリア == w20d)) && (($s:エリア履歴[ 1 ] == w19a ) || ($s:エリア履歴[ 1 ] == w21a ))) ) {

			//	双眼鏡兵
			//	気絶からスタート
			if ($b:w20b_２Ｆ敵兵状態 == d:STUN) {
				command セット敵兵メモリー \
					$:ステージ名 \
					$:マップ名 \
					$:双眼鏡兵名 \
					$:Ｘ座標 \
					$:Ｙ座標 \
					$:Ｚ座標\
					$:向き \
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
					$:ステージ名 \
					$:マップ名 \
					$:双眼鏡兵名 \
					$:Ｘ座標 \
					$:Ｙ座標 \
					$:Ｚ座標\
					$:向き \
					d:ENEMEM_ST_SLEEP \
					d:ENEMEM_PS_BACK \
					d:SLEEP_TIME_3 \
					$w:敵標準気絶  \
					$w:敵標準体力  \
					0 \
					0

			//	あの世からスタート
			} else if ($b:w20b_２Ｆ敵兵状態 == d:DEAD) {
				command セット敵兵メモリー \
					$:ステージ名 \
					$:マップ名 \
					$:双眼鏡兵名 \
					$:Ｘ座標 \
					$:Ｙ座標 \
					$:Ｚ座標\
					$:向き \
					d:ENEMEM_ST_HELL \
					d:ENEMEM_PS_BACK \
					0 \
					$w:敵標準気絶  \
					$w:敵標準体力  \
					0 \
					0
			}
		//	それ以外はそのままスタート
	} else {
		eval ($b:w20b_２Ｆ敵兵状態 = d:NORMAL)
		eval ($b:w20b_屋上敵兵状態 = d:NORMAL)
	}

}





///////////////////////////////////////////////////////////////////////
// アーセナル内くしゃみ設定（向手・山下用）
///////////////////////////////////////////////////////////////////////

	proc アーセナルくしゃみ間隔 {

		//	ステージ開始後は２分
		eval ($w:風邪くしゃみ間隔 = (120*60) )

		@アーセナルくしゃみ実行
	}

	proc アーセナルくしゃみ実行 {
		chara delay アーセナルくしゃみ間隔ディレイ \
			-t $w:風邪くしゃみ間隔 \
			-e {

				//	風邪を引いているときは
				if($w:風邪状態 == 1) {
					command rand 2

					//	前回３秒だったら
/*
					if ($w:風邪くしゃみ間隔 == (3*60)) {
						eval ($status = ($status+1))
					}
*/
					if ($status == 0) {
					print '#############################################################3seconds'
						//	３秒
						eval($w:風邪くしゃみ間隔 = (3*60))
					} else {
					print '#############################################################2minutes'
						//	２分
						eval($w:風邪くしゃみ間隔 = (120*60))
					}

					//	次をセット
					@アーセナルくしゃみ実行

				//	風邪が治ったときは
				} else {
					mesg delay アーセナルくしゃみ間隔ディレイ kill
				}

			}

	}



///////////////////////////////////////////////////////////////////////
// 死体捨てチェック
///////////////////////////////////////////////////////////////////////

proc 死体捨てチェック {
	//	危険時、敵発見時は死体は捨てられない
	command ゲットゲームステータス $i:ゲームステータス
	if ( !($i:ゲームステータス & d:STATE_DETECT) && ($w:アラートモード != d:ALERT_MODE_KIKEN) \
			&& ( `command ゲームオーバーチェック` == 0 ) ) {
		print '#############################################################SAFE'
		return 1;
	} else {
		print '#############################################################OUT'
		return 0;
	}
}





#else


print 'stdproc.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif


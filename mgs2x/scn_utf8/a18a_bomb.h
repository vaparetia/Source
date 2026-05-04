/*
	a18a_bomb.h
		Ｄ脚(アナザーミッション用モード別設定ファイル)

	2002/02/22 H.Yoshiike
	$Id: a18a_bomb.h,v 1.22 2002/10/02 08:09:02 usr03682 Exp $
*/

//------------------------------------------------------------
// 各ミッション別で使用するdefine
//------------------------------------------------------------
#define		MISSION_BOMB		1		// 爆弾解体用プロック起動用


// 爆弾の場所
#define		BOMB_POS_RAI	1500 4000 -133250	1024,0,0		// 北の天井
#define		BOMB_POS_SNA	-7000 -4000 -106500	3072,0,0		// 南西下の階
#define		BOMB_POS_SNA_2	8875 -1180 -110000	0,1745,0		// ＤＥ連絡橋上の出口の下付近
#define		BOMB_POS_PRI	0 450 -133250		1024,0,0		// シェル１シェル２連絡橋出口の床下
#define		BOMB_POS_TUX	600 983 -120950		0,1980,0		// 中央タンクの北側
#define		BOMB_POS_TUX_2	0 -750 -118050		0,2048,0		// 中央南側のフェンスの内側
#define		BOMB_POS_NIN	-5625 -1300 -133500	1024,0,0		// 階段の下
#define		BOMB_POS_PRE	7650 -1000 -109284	0,3793,0 		// 南東上の手すり
#define		BOMB_POS_PRE_2	7500 -4000 -108000  3072,0,0 		// 南東下の階
#define		BOMB_POS_PRE_3	0 0 -120			0,2048,0		// 敵兵の背中

// アイテムの場所
#define		ITEM_POS_01		1250   -1000 -119250	// 中央
#define		ITEM_POS_02		-7000  -4000 -133500	// 階段の下

#include	"a_item_info.h"



//------------------------------------------------------------
// mode.hで呼び出されるプロック
//------------------------------------------------------------
proc 爆弾解体ミッション用マップ定義前設定 {
	#if d:DEBUG_PRINT
		print 'proc_set1-bomb_misson'
	#endif

	if ( $f:爆弾解体ミッションＶＲウィンドウ表示フラグ == 0 ) {
		// 他のステージからロードしてきたときは前のステージのタイムを引き継ぐ
		$$i:ライデンタイム 				= $i:爆弾解体タイマー残り時間;
		$$i:スネークタイム 				= $i:爆弾解体タイマー残り時間;
		$$i:プリスキンタイム 			= $i:爆弾解体タイマー残り時間;
		$$i:タキシードスネークタイム 	= $i:爆弾解体タイマー残り時間;
		$$i:刀ライデンタイム 			= $i:爆弾解体タイマー残り時間;
		$$i:前作スネークタイム			= $i:爆弾解体タイマー残り時間;
	}

	// 共通システムの起動 (爆弾解体ミッションは共通で処理するようになったので引数は見なくなりました)
	@全ミッション共通システム設定 0

	// セレクトを押したときに出る説明
	@爆弾解体ミッションロード後用ＶＲウィンドウ設定

	// プレイヤーが動けるようになったらタイマーをスタートさせる処理
	@ロード時ＶＲタイマー処理

	$$i:ＶＲクリアフラグ = d:VRCLR_NO_RETRY;	// プログラムでＲＥＴＲＹをかけない
	@ＶＲクリア設定 p:A_ミッションエンド p:load_alt_bomb_a14a

	if ( $s:選択プレイヤー == 前作スネーク ) {
		// 前作スネークのときのみレーダーをオフにする
		// レーダーの設定
		command メニュー設定 -node_access off
	}
}

proc 爆弾解体ミッション用マップ定義後設定 {
	#if d:DEBUG_PRINT
		print 'proc_set2-bomb_misson'
	#endif

	// 全マップ共通で使用する基本キャラの設定
	command マップ設定 Ｄ脚 -set {
		@爆弾解体ミッション用サウンド設定	// データは最初に呼ぶ
		@爆弾解体ミッション用変数設定		// ドアのスイッチなどの変数を設定
		@爆弾解体ミッション用コマンダー設定
		@爆弾解体ミッション用ＢＧＭ設定		// プレイヤー→敵兵→BGMの順で呼ぶ
		@爆弾解体ミッション用アイテム設置
		@爆弾解体ミッション用解体Ｃ４設置
	}
}



//------------------------------------------------------------
// クリアしたときの処理
//------------------------------------------------------------
proc 爆弾解体ミッション用クリア時処理 {
	#if d:DEBUG_PRINT
		print 'clear'
	#endif

	@各ミッション共通モーションなしクリア処理
}



//------------------------------------------------------------
// サウンドに関する設定
//------------------------------------------------------------
proc 爆弾解体ミッション用サウンド設定 {
	#if d:DEBUG_PRINT
		print 'sound_set-bomb_misson'
	#endif

	chara サウンドマネージャー ＳＤマネ -pak 0
	@サウンド効果音設定
}

proc 爆弾解体ミッション用ＢＧＭ設定 {
	#if d:DEBUG_PRINT
		print 'bgm_set-bomb_misson'
	#endif

	chara ＢＧＭマネージャー ＢＧＭマネ
}



//------------------------------------------------------------
// 変数に関する設定
//------------------------------------------------------------
proc 爆弾解体ミッション用変数設定 {
	#if d:DEBUG_PRINT
		print 'stage_var_set-bomb_misson'
	#endif

	//------------------------------------------------------------
	// ドアに関する設定(0-開かない 1-開く)
	//------------------------------------------------------------
	$$b:ドアフラグ[0] = d:DOOR_FLAG_OPEN;	// Ｃ脚方面ドア
	$$b:ドアフラグ[1] = d:DOOR_FLAG_CLOSE;	// Ｅ脚方面ドア上
	$$b:ドアフラグ[2] = d:DOOR_FLAG_CLOSE;	// Ｅ脚方面ドア下
	$$b:ドアフラグ[3] = d:DOOR_FLAG_CLOSE;	// Ｇ脚方面ドア
}



//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
proc 爆弾解体ミッション用警備兵セット {
	#if d:DEBUG_PRINT
		print 'watcher_set-bomb_misson'
	#endif

	@A_警備兵 敵兵:01 0 3 d:ENE_STATUS_GUNLIGHT -9500 0 -127000 Ｄ脚
	@A_警備兵 敵兵:02 1 0 d:ENE_STATUS_GUNLIGHT 0 0 -123250 Ｄ脚
	@A_警備兵 敵兵:03 4 5 ( d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT ) 9500 0 -127000 Ｄ脚
//	@A_警備兵 敵兵:04 2 0 d:ENE_STATUS_GUNLIGHT 12250 -4000 -118625 Ｄ脚

}

proc 爆弾解体ミッション用攻撃兵セット {
	#if d:DEBUG_PRINT
		print 'attacker_set-bomb_misson'
	#endif

	@A_攻撃兵 敵兵:21 5 d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_攻撃兵 敵兵:22 5 d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:23 5 d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:24 5 d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 爆弾解体ミッション用警備兵コマンダー {
	#if d:DEBUG_PRINT
		print 'wccomm_set-bomb_misson'
	#endif

	chara 警備コマンダー wccomm -e {爆弾解体ミッション用警備兵セット}
}

proc 爆弾解体ミッション用攻撃兵コマンダー {
	#if d:DEBUG_PRINT
		print 'atcomm_set-bomb_misson'
	#endif

	chara 攻撃コマンダー atcomm -e 爆弾解体ミッション用攻撃兵セット
}



proc 爆弾解体ミッション用コマンダー設定 {
	#if d:DEBUG_PRINT
		print 'commander_set-bomb_misson'
	#endif

	chara コマンダー 敵兵セット \
		-w 爆弾解体ミッション用警備兵コマンダー \
		-a 爆弾解体ミッション用攻撃兵コマンダー \
		-m $w:スタートアラートモード \
		-r -16000, 0, -110875 Ｄ脚 \
		-c -16000, 0, -110875 Ｄ脚 \
		-v [ene_plant_voice:06] \
		-x [ghq_area_alert_voice:w18a] \
		-y [ghq_caution_voice:w18a_2] \
		-f 4 0 t:vc044023 t:vc044023_len 4 4 t:vc044023 t:vc044023_len 4 9 t:vc044023 t:vc044023_len \
		-s (d:ENE_STAGE_GPS | d:ENE_STAGE_VR_ANOTHER) \
		-z $i:敵標準麻酔持続 \
		-b $w:敵標準再発生数 \
		-h $i:標準警戒時間 // 警戒時間←ここの行を追加する。

}



//------------------------------------------------------------
// アイテムに関する設定
//------------------------------------------------------------
proc 爆弾解体ミッション用アイテム取得時番号設定 $:ボックス名 {
	#if d:DEBUG_PRINT
		print 'item_no_set-bomb_misson'
	#endif

	// 全キャラ１つだけなのでこれでＯＫ
	$b:w18a_取得ロード回数[0] = $w:グローバルロード回数;

	#if d:DEBUG_PRINT
		print $:ボックス名
		print $w:グローバルロード回数
	#endif

}


proc 爆弾解体ミッション用アイテム設置 {
	#if d:DEBUG_PRINT
		print 'a12a_item_set-bomb_misson'
	#endif

	switch ( $s:選択プレイヤー) {
		case ( ライデン ) {
			// レーション
			@アイテム再セットチェック $b:w18a_取得ロード回数[0]
			if($f:再セットフラグ) {
				@レーション_初期設置 レーション:01 d:ITEM_POS_01 0
			}
		}
		case ( スネーク ) {
			// レーション
			@アイテム再セットチェック $b:w18a_取得ロード回数[0]
			if($f:再セットフラグ) {
				@レーション_初期設置 レーション:11 d:ITEM_POS_01 0
			}
		}
		case ( プリスキン ) {
			// ＰＳＧ１-Ｔ
			@アイテム再セットチェック $b:w18a_取得ロード回数[0]
			if($f:再セットフラグ) {
				@A_ＰＳＧ１－Ｔ_初期設置 ＰＳＧ１－Ｔ本体:21 d:ITEM_POS_02 0 なし 11
			}
		}
		case ( タキシードスネーク ) {
			// レーション
			@アイテム再セットチェック $b:w18a_取得ロード回数[0]
			if($f:再セットフラグ) {
				@レーション_初期設置 レーション:31 d:ITEM_POS_01 0
			}
		}
		case ( 刀ライデン ) {
			// レーション
			@アイテム再セットチェック $b:w18a_取得ロード回数[0]
			if($f:再セットフラグ) {
				@レーション_初期設置 レーション:41 d:ITEM_POS_01 0
			}
		}
		default {
			// 前作スネーク
			@アイテム再セットチェック $b:w18a_取得ロード回数[0]
			if($f:再セットフラグ) {
				@A_Ｍ９_初期設置 Ｍ９本体:51 d:ITEM_POS_01 0 なし 16
			}
		}
	}
}



//------------------------------------------------------------
// 解体Ｃ４に関する設定
//------------------------------------------------------------
proc 爆弾解体ミッション用解体Ｃ４設置 {
	#if d:DEBUG_PRINT
		print 'a18a_bomb_set-bomb_misson'
	#endif

	switch ( $s:選択プレイヤー) {
		case ( ライデン ) {
			@A_解体Ｃ４設置 解体Ｃ４:01 d:BOMB_POS_RAI d:BOMB_A d:BOMB_LV5_NO:A18A d:C4FLAG_LIKE_CEILING alt_a_bm001 もやもや:01
		}
		case ( スネーク ) {
			@A_解体Ｃ４設置 解体Ｃ４:11 d:BOMB_POS_SNA d:BOMB_A d:BOMB_LV5_NO:A18A 0 alt_a_bm002 もやもや:11
			@A_解体Ｃ４設置 解体Ｃ４:12 d:BOMB_POS_SNA_2 d:BOMB_A d:BOMB_LV5_NO:A18A_2 0 alt_a_bm003 もやもや:12
		}
		case ( プリスキン ) {
			@A_解体Ｃ４設置 解体Ｃ４:21 d:BOMB_POS_PRI d:BOMB_B d:BOMB_LV5_NO:A18A 0 alt_b_bm001 もやもや無し
		}
		case ( タキシードスネーク ) {
			@A_解体Ｃ４設置 解体Ｃ４:31 d:BOMB_POS_TUX d:BOMB_B d:BOMB_LV5_NO:A18A 0 alt_b_bm002 もやもや無し
			@A_解体Ｃ４設置 解体Ｃ４:32 d:BOMB_POS_TUX_2 d:BOMB_B d:BOMB_LV5_NO:A18A_2 0 alt_b_bm003 もやもや無し
		}
		case ( 刀ライデン ) {
			@A_解体Ｃ４設置 解体Ｃ４:41 d:BOMB_POS_NIN d:BOMB_B d:BOMB_LV5_NO:A18A 0 alt_b_bm004 もやもや無し
		}
		default {
			// 前作スネーク
			@A_解体Ｃ４設置 解体Ｃ４:51 d:BOMB_POS_PRE d:BOMB_A d:BOMB_LV5_NO:A18A 0 alt_a_bm004 もやもや:51
			@A_解体Ｃ４設置 解体Ｃ４:52 d:BOMB_POS_PRE_2 d:BOMB_B d:BOMB_LV5_NO:A18A_2 0 alt_b_bm005 もやもや無し
			@A_特殊解体Ｃ４設置 解体Ｃ４:53 d:BOMB_POS_PRE_3 d:BOMB_B d:BOMB_LV5_NO:A18A_3 0 トラップなし 敵兵:03 2 もやもや無し

			if ( $b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A_3] == 0 ) {
				chara プロック連続実行 敵兵Ｃ４チェック -time -1 \
					-exec {
						@A_敵兵解体Ｃ４チェック 敵兵:03 敵兵Ｃ４チェック 解体Ｃ４:53
					}
				// 匂いなし爆弾は２ｍ以内に近づいたら爆発するように設置(敵兵との距離で判定)
				chara プロック連続実行 敵兵距離チェック -time -1 \
					-exec {
						if (`@A_敵兵距離チェック 敵兵:03` == d:TRUE ) {
							mesg 解体Ｃ４ 解体Ｃ４ blast		//指定解体Ｃ４を爆破
						}
					}
			}
		}
	}
}

// 爆弾を解体したときに呼ばれるプロック
proc A_各ステージ別爆弾解体時処理 $:p_解体爆弾名 {
	#if d:DEBUG_PRINT
		print 'a18a_bomb_disposal $:p_解体爆弾名='$:p_解体爆弾名
	#endif

	switch ( $:p_解体爆弾名 ) {
		case ( 解体Ｃ４:01 ) {
			// ライデン
			$b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A] = 1;
			mesg 爆弾検知領域 もやもや:01 消去 d:BOMB_RADAR_OFF_TIME
		}
		case ( 解体Ｃ４:11 ) {
			// スネーク
			$b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A] = 1;
			mesg 爆弾検知領域 もやもや:11 消去 d:BOMB_RADAR_OFF_TIME
		}
		case ( 解体Ｃ４:12 ) {
			// スネーク
			$b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A_2] = 1;
			mesg 爆弾検知領域 もやもや:12 消去 d:BOMB_RADAR_OFF_TIME
		}
		case ( 解体Ｃ４:21 ) {
			// プリスキン
			$b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A] = 1;
			command トラップ切り替え -trap alt_b_bm001 -switch 0
		}
		case ( 解体Ｃ４:31 ) {
			// タキシードスネーク
			$b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A] = 1;
			command トラップ切り替え -trap alt_b_bm002 -switch 0
		}
		case ( 解体Ｃ４:32 ) {
			// タキシードスネーク
			$b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A_2] = 1;
			command トラップ切り替え -trap alt_b_bm003 -switch 0
		}
		case ( 解体Ｃ４:41 ) {
			// 刀ライデン
			$b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A] = 1;
			command トラップ切り替え -trap alt_b_bm004 -switch 0
		}
		case ( 解体Ｃ４:51 ) {
			// 前作スネーク
			$b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A] = 1;
			mesg 爆弾検知領域 もやもや:51 消去 d:BOMB_RADAR_OFF_TIME
		}
		case ( 解体Ｃ４:52 ) {
			// 前作スネーク
			$b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A_2] = 1;
			command トラップ切り替え -trap alt_b_bm005 -switch 0
		}
		case ( 解体Ｃ４:53 ) {
			// 前作スネーク
			$b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A_3] = 1;
			mesg プロック連続実行 敵兵Ｃ４チェック kill
			mesg プロック連続実行 敵兵距離チェック kill
		}
		default {
			command assert ( d:FALSE ) 'bomb disposal error (scenario err) A_爆弾解体時処理 想定外の名前の爆弾を解体しました。'
		}
	}

	// このエリアは爆弾が複数ある場合があるのでキャラ別に判定する
	switch ( $s:選択プレイヤー ) {
		case ( スネーク ) {
			if ( $b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A] == 1 && \
				 $b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A_2] == 1 ) {

				command ＶＲ全体マップ表示３Ｄ＿爆弾解除 -b d:F_PLANT_D
				$i:A_全体マップ爆弾設置ステージ = ( $i:A_全体マップ爆弾設置ステージ & ~(d:F_PLANT_D) );
			}
		}
		case ( タキシードスネーク ) {
			if ( $b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A] == 1 && \
				 $b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A_2] == 1 ) {

				command ＶＲ全体マップ表示３Ｄ＿爆弾解除 -b d:F_PLANT_D
				$i:A_全体マップ爆弾設置ステージ = ( $i:A_全体マップ爆弾設置ステージ & ~(d:F_PLANT_D) );
			}
		}
		case ( 前作スネーク ) {
			if ( $b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A] == 1 && \
				 $b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A_2] == 1 && \
				 $b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A_3] >= 1 ) {

				command ＶＲ全体マップ表示３Ｄ＿爆弾解除 -b d:F_PLANT_D
				$i:A_全体マップ爆弾設置ステージ = ( $i:A_全体マップ爆弾設置ステージ & ~(d:F_PLANT_D) );
			}
		}
		default {
			command ＶＲ全体マップ表示３Ｄ＿爆弾解除 -b d:F_PLANT_D
			$i:A_全体マップ爆弾設置ステージ = ( $i:A_全体マップ爆弾設置ステージ & ~(d:F_PLANT_D) );
		}
	}
}

// 凍結された爆弾が下に落ちたときの処理
proc 凍結Ｃ４落下後処理 $:p_爆弾名 {
	#if d:DEBUG_PRINT
		print 'a16a_bomb drop $:p_爆弾名='$:p_爆弾名
	#endif

	switch ( $:p_爆弾名 ) {
		case ( 解体Ｃ４:53 ) {
			// 前作スネーク
			$b:A_爆弾解体終了状態フラグ[d:BOMB_LV5_NO:A18A_3] = 2;
		}
	}
}

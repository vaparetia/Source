/*
	a01a_photo.h
	    居住区一階（写真撮影ミッション用）
	2002/05/14 S.Kaneyoshi
	$Id: a01a_photo.h,v 1.47 2002/08/23 08:00:46 usr04249 Exp $
*/



#define MISSION_PHOTO	1	// 写真撮影用プロック起動用

#define NUMBER_OF_STUFF	5

#include "dcam_alt.h"

/*************************************************************************************/
// chara マップの前に呼ばれるproc
/*************************************************************************************/
	proc 写真撮影ミッション用マップ定義前設定 {

		command セットＶＲステータス d:VR_PHOTOGRAPH

		// 共通システムの起動 $:クリア条件フラグ $:途中ステージフラグ
		@全ミッション共通システム設定 d:VR_GOAL_FREE
		@種初期化
		@写真撮影ローカル変数初期化
		@写真撮影ミッションＶＲウィンドウ設定
		command セットルートオフセット d:SCN:写真撮影

		switch ( $s:選択プレイヤー ) {
			case ( ライデン ) {
				@ＶＲクリア設定 p:load_alt_photo_a01b p:A_ＲＥＴＲＹプロック
			}
			case ( スネーク ) {
				if( $s:登場ポイント == 写真撮影スネーク１ ) {
					@ＶＲクリア設定 p:load_alt_photo_a24g p:A_ＲＥＴＲＹプロック
				} else {
					@ＶＲクリア設定 p:A_ミッションエンド p:A_ＲＥＴＲＹプロック
				}
			}
			default {
				command assert ( d:FALSE ) 'クリア時の処理が設定されていません'
			}
		}

	}

/*************************************************************************************/
// chara マップの後に呼ばれるproc
/*************************************************************************************/
/*
// マップ
１Ｆ廻廊部
ロッカー部屋
*/
	proc 写真撮影ミッション用マップ定義後設定 {

		// 全マップ共通基本キャラ設定
		command マップ設定 １Ｆ廻廊部 ロッカー部屋 -set {
			@各ミッション用ゴール表示 3750 0 /*5000*/20000
			@写真撮影ミッション用サウンド設定
			@写真撮影ミッション用変数設定
//			@写真撮影ミッション用コマンダー設定
			@写真撮影ミッション用水溜まりセット		// 敵兵を追加した時はここも追加する事
			@写真撮影ミッション用ＢＧＭ設定			// プレイヤー→敵兵→BGMの順で呼ぶ

		}

		// 各マップで使用するキャラの設定(アイテム等)
		command マップ設定 １Ｆ廻廊部 -set {
			//@写真撮影ミッション用廻廊部アイテム設置
		}

		command マップ設定 ロッカー部屋 -set {
			//@写真撮影ミッション用ロッカー部屋アイテム設置
			if( $s:選択プレイヤー == ライデン ) {
				@モアイ設置
			}
		}
	}

	proc 写真撮影ミッション用サウンド設定 {
		#if d:DEBUG_PRINT
			print 'sound_set-bomb_misson'
		#endif

		if( ( $s:選択プレイヤー == スネーク ) && \
			( $s:登場ポイント == 写真撮影スネーク２ ) ) {

			@サウンドデータ常駐読み替え設定
		}

		chara サウンドマネージャー ＳＤマネ -pak 1
		@サウンド効果音設定
	}

	proc 写真撮影ミッション用ＢＧＭ設定 {
		#if d:DEBUG_PRINT
			print 'bgm_set-bomb_misson'
		#endif
		chara ＢＧＭマネージャー ＢＧＭマネ
	}

/*************************************************************************************/
// 初期化関連
/*************************************************************************************/

	proc 種初期化 {
		// 一括初期化後に変数を戻す
		$i:グローバル変数プレイタイム = $$i:ローカル変数プレイタイム;
		command varsave $i:グローバル変数プレイタイム

		$$i:種 = ( $i:グローバル変数プレイタイム % 5 );
		$$i:種 = $$i:種 + `command ミッションズリトライ回数取得`
		$$i:種 = $$i:種 + `command ミッションズコンティニュー回数取得`

		$$i:ラウンド回数 = ( `command ミッションズリトライ回数取得` + \
							 `command ミッションズコンティニュー回数取得` )

		#if 0
			$$i:ラウンド回数 = 5;
			$$i:種 = 2;
		#endif
		
	}
	proc 写真撮影ローカル変数初期化 {

		@写真撮影用ローカル変数初期化プロック
	}

	proc 写真撮影ミッション用変数設定 {
		#if d:DEBUG_PRINT
			print 'stage_var_set-bomb_misson'
		#endif

		//------------------------------------------------------------
		// ドアに関する設定
		//		DOOR_FLAG_CLOSE			0		// 絶対開かない
		//		DOOR_FLAG_OPEN			1		// 絶対開く
		//		DOOR_FLAG_ENEMY_ONLY	2		// 敵兵のみ開く
		//		DOOR_FLAG_CLOSE_LEVEL	3		// カードレベルが足りなくて開かない
		//		DOOR_FLAG_OPEN_LEVEL	4		// カード持っているので開く
		//------------------------------------------------------------

		$$b:ドアフラグ[0] = d:DOOR_FLAG_CLOSE;	// リフレッシュとの左水密ドア
		$$b:ドアフラグ[1] = d:DOOR_FLAG_CLOSE;	// リフレッシュとの右スライドドア
		$$b:ドアフラグ[2] = d:DOOR_FLAG_CLOSE;	// 甲板との左水密ドア
		$$b:ドアフラグ[3] = d:DOOR_FLAG_CLOSE;	// 甲板との右水密ドア
		$$b:ドアフラグ[4] = d:DOOR_FLAG_CLOSE;	// ロッカールームとの左ドア
		$$b:ドアフラグ[5] = d:DOOR_FLAG_CLOSE;	// ロッカールームとの右ドア

		// 初期装備セット
		$w:アイテム数Ｒ[ d:アイテム:デジカメ ] = 1;
		command シナリオ前装備変更 -number d:アイテム:デジカメ

		$w:アイテム = d:アイテム:素手
		$w:武器 = d:武器:素手
	}

/*************************************************************************************/
// ＶＲウィンドウの設定と表示
/*************************************************************************************/
proc 写真撮影ミッションＶＲウィンドウ設定 {
	#if d:DEBUG_PRINT
		print 'vr_window_set'
	#endif

	if( $s:選択プレイヤー == ライデン ) {
	// モアイ撮影
		@写真撮影用ＶＲウィンドウ設定	\
			[VRMSG_J:ALT_PHT_a01a_RAI] [VRMSG_E:ALT_PHT_a01a_RAI] \	// 日、英
			[VRMSG_G:ALT_PHT_a01a_RAI] [VRMSG_F:ALT_PHT_a01a_RAI] \	// 独、仏
			[VRMSG_I:ALT_PHT_a01a_RAI] [VRMSG_S:ALT_PHT_a01a_RAI] \	// 伊、西
			[VRMSG_K:ALT_PHT_a01a_RAI] \							// 韓
			p:写真撮影ミッションステージ開始時処理 \				// 終了プロック
			poster	odai_moai
	} else {
		if( $s:登場ポイント == 写真撮影スネーク１ ) {
			switch ( $$i:種 % 5 ) {
				case ( 0 ) {
					$$s:ファイル名 = odai_a01a_ldr00
				}
				case ( 1 ) {
					$$s:ファイル名 = odai_a01a_ldr01
				}
				case ( 2 ) {
					$$s:ファイル名 = odai_a01a_ldr02
				}
				case ( 3 ) {
					$$s:ファイル名 = odai_a01a_ldr03
				}
				case ( 4 ) {
					$$s:ファイル名 = odai_a01a_ldr04
				}
			}

			@写真撮影用ＶＲウィンドウ設定	\
				[VRMSG_J:ALT_PHT_a01a_SNA_A] [VRMSG_E:ALT_PHT_a01a_SNA_A] \	// 日、英
				[VRMSG_G:ALT_PHT_a01a_SNA_A] [VRMSG_F:ALT_PHT_a01a_SNA_A] \	// 独、仏
				[VRMSG_I:ALT_PHT_a01a_SNA_A] [VRMSG_S:ALT_PHT_a01a_SNA_A] \	// 伊、西
				[VRMSG_K:ALT_PHT_a01a_SNA_A] \								// 韓
				p:写真撮影ミッションステージ開始時処理 \					// 終了プロック
				poster	$$s:ファイル名
		} else {
			if( ( $$i:ラウンド回数 % 11 ) < 5 ) {
				// ピンポイントクイズ
				switch ( $$i:種 % 5 ) {
					case ( 0 ) {
						$$s:ファイル名 = odai_a01a_ldr00a
					}
					case ( 1 ) {
						$$s:ファイル名 = odai_a01a_ldr01a
					}
					case ( 2 ) {
						$$s:ファイル名 = odai_a01a_ldr02a
					}
					case ( 3 ) {
						$$s:ファイル名 = odai_a01a_ldr03a
					}
					case ( 4 ) {
						$$s:ファイル名 = odai_a01a_ldr04a
					}
				}

				@写真撮影用ＶＲウィンドウ設定	\
					[VRMSG_J:ALT_PHT_a01a_SNA_A] [VRMSG_E:ALT_PHT_a01a_SNA_A] \	// 日、英
					[VRMSG_G:ALT_PHT_a01a_SNA_A] [VRMSG_F:ALT_PHT_a01a_SNA_A] \	// 独、仏
					[VRMSG_I:ALT_PHT_a01a_SNA_A] [VRMSG_S:ALT_PHT_a01a_SNA_A] \	// 伊、西
					[VRMSG_K:ALT_PHT_a01a_SNA_A] \								// 韓
					p:写真撮影ミッションステージ開始時処理 \					// 終了プロック
					poster	$$s:ファイル名
			} else if( ( $$i:ラウンド回数 % 11 ) == 10 ) {
				// 監督クイズ
				@ＶＲウィンドウ設定	\
							[VRMSG_J:ALT_PHT_a01a_SNA_B6] [VRMSG_E:ALT_PHT_a01a_SNA_B6] \	// 日、英
							[VRMSG_G:ALT_PHT_a01a_SNA_B6] [VRMSG_F:ALT_PHT_a01a_SNA_B6] \	// 独、仏
							[VRMSG_I:ALT_PHT_a01a_SNA_B6] [VRMSG_S:ALT_PHT_a01a_SNA_B6] \	// 伊、西
							[VRMSG_K:ALT_PHT_a01a_SNA_B6] \									// 韓
							p:写真撮影ミッションステージ開始時処理							// 終了プロック
			} else {
				// 製作者クイズ
				switch ( $$i:種 % 5 ) {
					case ( 0 ) {
						@ＶＲウィンドウ設定	\
							[VRMSG_J:ALT_PHT_a01a_SNA_B1] [VRMSG_E:ALT_PHT_a01a_SNA_B1] \	// 日、英
							[VRMSG_G:ALT_PHT_a01a_SNA_B1] [VRMSG_F:ALT_PHT_a01a_SNA_B1] \	// 独、仏
							[VRMSG_I:ALT_PHT_a01a_SNA_B1] [VRMSG_S:ALT_PHT_a01a_SNA_B1] \	// 伊、西
							[VRMSG_K:ALT_PHT_a01a_SNA_B1] \									// 韓
							p:写真撮影ミッションステージ開始時処理							// 終了プロック
					}
					case ( 1 ) {
						@ＶＲウィンドウ設定	\
							[VRMSG_J:ALT_PHT_a01a_SNA_B2] [VRMSG_E:ALT_PHT_a01a_SNA_B2] \	// 日、英
							[VRMSG_G:ALT_PHT_a01a_SNA_B2] [VRMSG_F:ALT_PHT_a01a_SNA_B2] \	// 独、仏
							[VRMSG_I:ALT_PHT_a01a_SNA_B2] [VRMSG_S:ALT_PHT_a01a_SNA_B2] \	// 伊、西
							[VRMSG_K:ALT_PHT_a01a_SNA_B2] \									// 韓
							p:写真撮影ミッションステージ開始時処理							// 終了プロック
					}
					case ( 2 ) {
						@ＶＲウィンドウ設定	\
							[VRMSG_J:ALT_PHT_a01a_SNA_B3] [VRMSG_E:ALT_PHT_a01a_SNA_B3] \	// 日、英
							[VRMSG_G:ALT_PHT_a01a_SNA_B3] [VRMSG_F:ALT_PHT_a01a_SNA_B3] \	// 独、仏
							[VRMSG_I:ALT_PHT_a01a_SNA_B3] [VRMSG_S:ALT_PHT_a01a_SNA_B3] \	// 伊、西
							[VRMSG_K:ALT_PHT_a01a_SNA_B3] \									// 韓
							p:写真撮影ミッションステージ開始時処理							// 終了プロック
					}
					case ( 3 ) {
						@ＶＲウィンドウ設定	\
							[VRMSG_J:ALT_PHT_a01a_SNA_B4] [VRMSG_E:ALT_PHT_a01a_SNA_B4] \	// 日、英
							[VRMSG_G:ALT_PHT_a01a_SNA_B4] [VRMSG_F:ALT_PHT_a01a_SNA_B4] \	// 独、仏
							[VRMSG_I:ALT_PHT_a01a_SNA_B4] [VRMSG_S:ALT_PHT_a01a_SNA_B4] \	// 伊、西
							[VRMSG_K:ALT_PHT_a01a_SNA_B4] \									// 韓
							p:写真撮影ミッションステージ開始時処理							// 終了プロック
					}
					case ( 4 ) {
						@ＶＲウィンドウ設定	\
							[VRMSG_J:ALT_PHT_a01a_SNA_B5] [VRMSG_E:ALT_PHT_a01a_SNA_B5] \	// 日、英
							[VRMSG_G:ALT_PHT_a01a_SNA_B5] [VRMSG_F:ALT_PHT_a01a_SNA_B5] \	// 独、仏
							[VRMSG_I:ALT_PHT_a01a_SNA_B5] [VRMSG_S:ALT_PHT_a01a_SNA_B5] \	// 伊、西
							[VRMSG_K:ALT_PHT_a01a_SNA_B5] \									// 韓
							p:写真撮影ミッションステージ開始時処理							// 終了プロック
					}
				}
			}
		}
	}


	// ＶＲウィンドウを表示する
	mesg ＶＲウィンドウ ＶＲウィンドウ君 開く

/*
	// スタート時とクリア時に使用するカットインカメラ
	chara カメラ カットインカメラ		-c 0 -l 0 -p  0

	// スタート時のカメラ(必要な場合)
	chara カメラ設定 カットインカメラ \
		-c 1 \
		-p 43,8528,-1398 -t -43,5745,-5427 \
		-r 394,2062,0 -f 4897 \
		-a 200 \
		-i 0 -1 0 0 \
		-s 1
*/
}



/*************************************************************************************/
// ステージ開始時処理
/*************************************************************************************/
proc 写真撮影ミッションステージ開始時処理 {
	#if d:DEBUG_PRINT
		print 'game_start!!!!!'
	#endif

	// プレイヤーやパッドの制御等を行う a_stdproc.hで定義
	@全ミッション共通ステージ開始時処理

/*
	// カメラワークスタート(必要な場合)
	// 全体図
	chara カメラ設定 カットインカメラ \
		-c 1 \
		-p -90,17878,2163 -t -90,12036,-1689 \
		-r 644,2048,0 -f 6997 \
		-a 200 \
		-i 4 4 d:START_CAMERA_CHANGE_TIME d:START_CAMERA_CHANGE_TIME \
		-s 1

	chara delay ディレイ \
		-time d:START_CAMERA_CHANGE_TIME \
		-exec {
			chara カメラ設定 カットインカメラ -s -1
		}
*/
}

/*************************************************************************************/
// クリア時に呼ばれるプロック
/*************************************************************************************/
proc 写真撮影ミッション用クリア時処理 {
	#if d:DEBUG_PRINT
		print 'game clear'
	#endif
	@全ミッション共通クリアモーション発動 d:モーションリスト:non_mclr_1 0
/*
	chara カメラ設定 カットインカメラ \
		-c 1 \
		-p -2209,-13877,-60193 -t -2480,-14860,-57681 \
		-r 242,4026,0 -f 2711 \
		-a 200 \
		-i 2 2 0 0 \
		-s 1
*/
}

/*************************************************************************************/
// 敵兵の設定
/*************************************************************************************/
#define ENE01_DEF_X		6000
#define ENE01_DEF_Y		0
#define ENE01_DEF_Z		-6000

	proc 写真撮影ミッション用警備兵セット {
		#if d:DEBUG_PRINT
			print 'watcher_set-bomb_misson'
		#endif
		/*************************************************/
		// 敵兵を増やした際は「水溜まりセット」を変更する事

		@A_警備兵 敵兵:01 \
			1 \		// ルート
			0 \		// ノード
			d:ENE_STATUS_NORMAL \
			d:ENE01_DEF_X d:ENE01_DEF_Y d:ENE01_DEF_Z \
			１Ｆ廻廊部
	}

	proc 写真撮影ミッション用攻撃兵セット {
		#if d:DEBUG_PRINT
			print 'attacker_set-bomb_misson'
		#endif
		/*************************************************/
		// 敵兵を増やした際は「水溜まりセット」を変更する事

		@A_攻撃兵 敵兵:61 \
			1 \
			d:ENE_STATUS_NORMAL \
			d:ENE_EQUIP_TYPE_NORMAL

		@A_攻撃兵 敵兵:62 \
			1 \
			d:ENE_STATUS_NORMAL \
			d:ENE_EQUIP_TYPE_NORMAL

	}

	proc 写真撮影ミッション用警備兵コマンダー {
		#if d:DEBUG_PRINT
			print 'wccomm_set-bomb_misson'
		#endif

		chara 警備コマンダー wccomm \
			-e { 写真撮影ミッション用警備兵セット }
	}

	proc 写真撮影ミッション用攻撃兵コマンダー {
		#if d:DEBUG_PRINT
			print 'atcomm_set-bomb_misson'
		#endif

		chara 攻撃コマンダー atcomm \
			-status 	d:AT_COM_WARP \
			-wait		180 \
			-avoidwait	300 \
			-e { 写真撮影ミッション用攻撃兵セット }
	}

	proc 写真撮影ミッション用コマンダー設定 {
		#if d:DEBUG_PRINT
			print 'commander_set-bomb_misson'
		#endif
		/*************************************************/
		// 敵兵を増やした際は「水溜まりセット」を変更する事

		chara コマンダー 敵兵セット \
			-w 写真撮影ミッション用警備兵コマンダー \
//			-a 写真撮影ミッション用攻撃兵コマンダー \
			-s d:ENE_STAGE_VR_ANOTHER \
			-m $w:スタートアラートモード \
			-r 6000,0,-13000 １Ｆ廻廊部 \
			-c 6000,0,-13000 １Ｆ廻廊部 \
			-x [ghq_area_alert_voice:w01a] \
			-y [ghq_caution_voice:w01a] \
			-v [ene_tank_voice:01] \
			-z $i:敵標準麻酔持続 \
			-o $i:敵標準気絶持続 \
			-b $w:敵標準再発生数 \
			-h $i:標準警戒時間 \
			-p   0,-1,-1 クリアリング初期設定１ \
				-2,-2,-2 クリアリングカメラＯＦＦ
	}

/*************************************************************************************/
// 水溜まりセット
// 敵兵を増やした際はここも変更する事
/*************************************************************************************/
	proc 写真撮影ミッション用水溜まりセット {
		print 'effect 水溜まりセット'

		chara 水溜り管理 水溜り管理人 \
			-n_obj	4 \
			-obj	d:PLAYER \
					敵兵:01 \
					敵兵:61 \
					敵兵:62

		if( $$b:ドアフラグ[2] ) {
			// 左舷水溜まりトラップ
			command トラップ切り替え -trap puddle:01 -switch 1
			command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 3
		}
		if( $$b:ドアフラグ[3] ) {
			// 右舷水溜まりトラップ
			command トラップ切り替え -trap puddle:02 -switch 1
			command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 4
		}

		trap puddle ？ \
			-mask ？ \
			-exec {
				if($3 == 入る){
					mesg 水溜り管理 水溜り管理人 入る $2
				}else{
					mesg 水溜り管理 水溜り管理人 出る $2
				}
			}
	}

/*************************************************************************************/
// 個別プログラム
/*************************************************************************************/
	proc モアイ設置 {
		chara プットオブジェ モアイ -m moai \
			-r 0,3072,0 \
			-p 4500 2200 3500 \
			-s 100,100,100 \
			-l d:LT2_NAME
	}

/*************************************************************************************/
// アイテム関連
/*************************************************************************************/

	proc 写真撮影ミッション用アイテム取得時番号設定 $:ボックス名 {
		#if d:DEBUG_PRINT
			print 'item_no_set'
		#endif

		if($:ボックス名 == ＵＳＰ本体) {
			eval($b:w01a_取得ロード回数[0] = $w:グローバルロード回数);
		} else if ($:ボックス名 == ＵＳＰ弾) {
			eval($b:w01a_取得ロード回数[1] = $w:グローバルロード回数);
		} else if ($:ボックス名 == Ｍ９弾) {
			eval($b:w01a_取得ロード回数[2] = $w:グローバルロード回数);
		}

		#if d:DEBUG_PRINT
			print $:ボックス名
			print $w:グローバルロード回数
		#endif
	}

	proc 写真撮影ミッション用廻廊部アイテム設置 {
	}

	proc 写真撮影ミッション用ロッカー部屋アイテム設置 {
		#if d:DEBUG_PRINT
			print 'locker room item set'
		#endif

		@アイテム再セットチェック $b:w01a_取得ロード回数[1]
		if ($f:再セットフラグ) {
			@レーション_初期設置 レーション -4520 0 3550 0 			// ロッカールーム左下
		}

		@アイテム再セットチェック $b:w01a_取得ロード回数[2]
		if ($f:再セットフラグ) {
			@弾薬_Ｍ９_初期設置 Ｍ９弾 4500 0 3550 0 				// Ｍ９弾 ロッカールーム右下
		}
	}


/***************************************************************************************/
// ロッカー関連プロック
/***************************************************************************************/

#define 和希沙也	a01a_ldr00
#define 安田美沙子	a01a_ldr01
#define 石田裕子	a01a_ldr02
#define 中川翔子	a01a_ldr03
#define 阿部英理奈	a01a_ldr04
#define ポスター無	w01alc_tobira

	proc 写真撮影ミッション用ロッカー設置 {

		if( $s:選択プレイヤー == スネーク ) {
			@グラビアカメラ
		}
		
		chara ロッカー管理 管理人さん -n 8
		command ロッカーモーション -motion locker
		command プレイヤーロッカーモーション -motion sna_locker

		local $$:ロッカーポスター１
		local $$:ロッカーポスター２
		local $$:ロッカーポスター３
		local $$:ロッカーポスター４
		local $$:ロッカーポスター５

		if( $s:選択プレイヤー == スネーク ) {
			$$:ロッカーポスター１ = d:和希沙也;
			$$:ロッカーポスター２ = d:安田美沙子;
			$$:ロッカーポスター３ = d:石田裕子;
			$$:ロッカーポスター４ = d:中川翔子;
			$$:ロッカーポスター５ = d:阿部英理奈;
		} else {
			$$:ロッカーポスター１ = d:ポスター無;
			$$:ロッカーポスター２ = d:ポスター無;
			$$:ロッカーポスター３ = d:ポスター無;
			$$:ロッカーポスター４ = d:ポスター無;
			$$:ロッカーポスター５ = d:ポスター無;
		}

	// ロッカー左上
		@ロッカーチェック 0
		chara ロッカー ロッカー１ \
			-kms $$:ロッカーポスター１ \
			-seno 2 \					// SE
			-sefloor 2 \
			-p -4250,0,(2050+50) \
			-r 0,1024,0 \
			-down_rot 0 870 0 \
			-down_shift 0 0 100 \
			//-flag 0x0001 \			// 影が投影される
			-s ( /*d:LOCKER_STATE_POSTER |*/ d:LOCKER_STATE_NO_OPEN_SEG ) \	//ポスターが張ってあるフラグを指定
			//-g 	-250,700,0 -750,1700,0 \
				//-300,1250,0 -600,1550,0 \
				//-350,850,0 -550,1100,0 \
			-load_state $i:w01a_ロッカーロード回数[0] \
			-exec {
				//@ロッカー状態保存 0 $3 $4
			}

		// 開け閉め用トラップ
		trap lk000 d:PLAYER \
			-and \
			-mask いる \
			-state 0,4 \
			-dir 3072,512 1024,512 \
			-button d:ACTION_BUTTON,-1 \
			-exec {
				command ロッカー状態 -n ロッカー１
				mesg プレイヤー d:PLAYER locker ロッカー１ $status 3072,512 -4250,(2000+50)
			}
		// 扉を開け閉めする時のカメラ
		trap lk000 d:PLAYER \
			-mask ？ \
			-state 6 \
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 1 \
					-p -2547,2304,3610 -t -3541,1770,2701 \
					-r 246,2589,0 -f 1449 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}

	// ロッカー左下
		@ロッカーチェック 1
		chara ロッカー ロッカー２ \
			//-kms w01alc_tobira \
			-kms $$:ロッカーポスター２ \
			-seno 2 \
			-sefloor 2 \
			-p -4250,0,(3050+50) \
			-r 0,1024,0 \
			-down_rot 0 512 0 \
			-down_shift 500 0 500 \
			-load_state $i:w01a_ロッカーロード回数[1] \
			-exec {
				//@ロッカー状態保存 1 $3 $4
			}
		trap lk001 d:PLAYER \
			-and \
			-mask いる \
			-state 0,4 \
			-dir 3072,512 1024,512 \
			-button d:ACTION_BUTTON,-1 \
			-exec {
				command ロッカー状態 -n ロッカー２
				mesg プレイヤー d:PLAYER locker ロッカー２ $status 3072,512 -4250,(3000+50)
				// 扉の開閉によるアイテムのON_OFF
				if( $f:ロッカーアイテム == 0 ) {
					chara delay ディレイ -time 10 -exec {
						mesg アイテム 左ロッカー オン
						eval( $f:ロッカーアイテム = 1 )
					}
				}else{
					chara delay ディレイ -time 30 -exec {
						mesg アイテム 左ロッカー オフ
						eval( $f:ロッカーアイテム = 0 )
					}
				}
			}
		trap lk001 d:PLAYER \
			-mask ？ \
			-state 6 \
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 1 \
					-p -2618,2724,4155 -t -3483,1631,3779 \
					-r 560,2805,0 -f 1443 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}
	// 真中左下
		@ロッカーチェック 2
		chara ロッカー ロッカー３ \
			-kms $$:ロッカーポスター３ \
			-seno 2 \
			-sefloor 2 \
			-p -550,0,(4500-50) \
			-r 0,3072,0 \
			-down_rot 0 2750 0 \
			-down_shift -150 0 350 \
			-load_state $i:w01a_ロッカーロード回数[2] \
			-exec {
				//@ロッカー状態保存 2 $3 $4
			}
		trap lk002 d:PLAYER \
			-and \
			-mask いる \
			-state 0,4 \
			-dir 1024,512 3072,512 \
			-button d:ACTION_BUTTON,-1 \
			-exec {
				command ロッカー状態 -n ロッカー３
				mesg プレイヤー d:PLAYER locker ロッカー３ $status 1024,512 -500,(4500-50)
			}
		trap lk002 d:PLAYER \
			-mask ？ \
			-state 6 \
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 1 \
					-p -3366,2175,4754 -t -2156,1870,4417 \
					-r 155,1201,0 -f 1292 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}
	// 真中右下
		@ロッカーチェック 3
		chara ロッカー ロッカー４ \
			-kms $$:ロッカーポスター４ \
			-seno 2 \
			-sefloor 2 \
			-p 550,0,(3500+50) \
			-r 0,1024,0 \
			-down_rot 0 1000 0 \
			-down_shift 0 0 -150 \
			-load_state $i:w01a_ロッカーロード回数[3] \
			-exec {
				//@ロッカー状態保存 3 $3 $4
			}
		trap lk003 d:PLAYER \
			-and \
			-mask いる \
			-state 0,4 \
			-dir 3072,512 1024,512 \
			-button d:ACTION_BUTTON,-1 \
			-exec {
				command ロッカー状態 -n ロッカー４
				mesg プレイヤー d:PLAYER locker ロッカー４ $status 3072,512 500,(3500+50)
			}
		trap lk003 d:PLAYER \
			-mask ？ \
			-state 6 \
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 1 \
					-p 3031,2080,5055 -t 672,1469,3738 \
					-r 145,2740,0 -f 2770 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}
	// 右側上
		@ロッカーチェック 4
		chara ロッカー ロッカー５ \
			-kms w01alc_tobira \
			-seno 2 \
			-s d:LOCKER_STATE_DESTROY \
			-sefloor 2 \
			-p 4250,0,(3050-50) \
			-r 0,3072,0 \
			//-down_rot 0 2870 0 \
			//-down_shift 0 0 200 \
			-load_state $i:w01a_ロッカーロード回数[4] \
			-exec {
				//@ロッカー状態保存 4 $3 $4
			}
		trap lk004 d:PLAYER \
			-and \
			-mask いる \
			-state 0,4 \
			-dir 1024,512 3072,512 \
			-button d:ACTION_BUTTON,-1 \
			-exec {
				command ロッカー状態 -n ロッカー５
				mesg プレイヤー d:PLAYER locker ロッカー５ $status 1024,512 4250,(3000-50)
			}
		trap lk004 d:PLAYER \
			-mask ？ \
			-state 6 \
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 1 \
					-p 1517,2097,3224 -t 3375,1622,2831 \
					-r 160,1160,0 -f 1957 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}
	// 右下
		@ロッカーチェック 5
		chara ロッカー ロッカー６ \
			//-kms w01alc_tobira_nu2 \	// ビキニポスター２
			-kms $$:ロッカーポスター５ \
			-seno 2 \
			-sefloor 2 \
			-p 4250,0,(4050-50) \
			-r 0,3072,0 \
			-down_rot 0 3400 0 \
			-down_shift 0 0 150 \
			//-s d:LOCKER_STATE_POSTER \	//ポスターが張ってあるフラグを指定
			//-g 	-250,700,0 -750,1700,0 \
				//-300,1250,0 -600,1550,0 \
				//-350,850,0 -550,1100,0 \
			-load_state $i:w01a_ロッカーロード回数[5] \
			-exec {
				//@ロッカー状態保存 5 $3 $4
			}
		trap lk005 d:PLAYER \
			-and \
			-mask いる \
			-state 0,4 \
			-dir 1024,512 3072,512 \
			-button d:ACTION_BUTTON,-1 \
			-exec {
				command ロッカー状態 -n ロッカー６
				mesg プレイヤー d:PLAYER locker ロッカー６ $status 1024,512 4250,(4000-50)
			}
		trap lk005 d:PLAYER \
			-mask ？ \
			-state 6 \
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 1 \
					-p 1927,2255,4212 -t 3102,1701,3621 \
					-r 260,1328,0 -f 1427 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}
	// 真中左上 壊れている
		@ロッカーチェック 6
		chara ロッカー ロッカー７ \
			-kms w01alc_tobira \
			-seno 2 \
			-sefloor 2 \
			-s d:LOCKER_STATE_DESTROY \
			-p -550,0,(3500-50) \
			-r 0,3072,0 \
			-load_state $i:w01a_ロッカーロード回数[6] \
			-exec {
				//@ロッカー状態保存 6 $3 $4
			}

		trap lk006 d:PLAYER \
			-and \
			-mask いる \
			-state 0,4 \
			-dir 1024,512 3072,512 \
			-button d:ACTION_BUTTON,-1 \
			-exec {
				eval($f:rft_開かないロッカー開けた_lk006 = 1)	//fukushima
				command ロッカー状態 -n ロッカー７
				mesg プレイヤー d:PLAYER locker ロッカー７ $status 1024,512 -500,(3500-50)
			}
		trap lk006 d:PLAYER \
			-mask ？ \
			-state 6 \
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 1 \
					-p -1945,2841,4839 -t -1031,960,2996 \
					-r 483,1748,0 -f 2788 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}
	// 真中右上 壊れている
		@ロッカーチェック 7
		chara ロッカー ロッカー８ \
			-kms w01alc_tobira \
			-seno 2 \
			-sefloor 2 \
			-s d:LOCKER_STATE_DESTROY \
			-p 550,0,(2500+50) \
			-r 0,1024,0 \
			-load_state $i:w01a_ロッカーロード回数[7] \
			-exec {
				//@ロッカー状態保存 7 $3 $4
			}
		trap lk007 d:PLAYER \
			-and \
			-mask いる \
			-state 0,4 \
			-dir 3072,512 1024,512 \
			-button d:ACTION_BUTTON,-1 \
			-exec {
				eval($f:rft_開かないロッカー開けた_lk007 = 1)	//fukushima
				command ロッカー状態 -n ロッカー８
				mesg プレイヤー d:PLAYER locker ロッカー８ $status 3072,512 500,(2500+50)
			}
		trap lk007 d:PLAYER \
			-mask ？ \
			-state 6 \
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 1 \
					-p 3371,2152,5028 -t 1579,1191,3394 \
					-r 246,2590,0 -f 2608 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}
	}

	proc グラビアカメラ {
// グラビア・ビハインド・カメラ
		// 1
		trap GBC001 d:PLAYER -mask ＊ -dir 0 512 -state d:TRP_STATE_CAUTION \
			-exec {
				command ロッカー状態 -n ロッカー１
				if( ($status & 0x0002) && !($status & 0x2000) ){
					command ロッカー状態 -n ロッカー２
					if( ($status & 0x0002) && !($status & 0x2000) ){
						chara カメラ設定 通路カメラ０ \
							-c 1 \
							-p -3794,2183,3153 -t -3786,1677,2417 \
							-r 393,2041,0 -f 893 \
							-a 200 \
							-i 0 1 0 0 \
							-s 1
					} else {
						chara カメラ設定 通路カメラ０ \
							-c 1 \
							-p -3795,1690,3648 -t -3610,1371,2318 \
							-r 152,1958,0 -f 1380 \
							-a 200 \
							-i 1 1 0 0 \
							-s 1
					}
				}else{
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
				if( $3 == 出る ){
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
			}
		// 2
		trap GBC002 d:PLAYER -mask ＊ -dir 0 512 -state d:TRP_STATE_CAUTION \
			-exec {
				command ロッカー状態 -n ロッカー２
				if( ($status & 0x0002) && !($status & 0x2000) ){
					chara カメラ設定 通路カメラ０ \
						-c 1 \
						-p -3676,2098,4752 -t -3807,1491,3421 \
						-r 278,2112,0 -f 1468 \
						-a 200 \
						-i 1 1 0 0 \
						-s 1
				}else{
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
				if( $3 == 出る ){
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
			}
		// 3
		trap GBC003 d:PLAYER -mask ＊ -dir 2048 512 -state d:TRP_STATE_CAUTION \
			-exec {
				command ロッカー状態 -n ロッカー３
				if( ($status & 0x0002) && !($status & 0x2000) ){
					chara カメラ設定 通路カメラ０ \
						-c 1 \
						-p -1039,1799,2990 -t -991,843,5810 \
						-r 213,11,0 -f 2978 \
						-a 200 \
						-i 0 1 0 0 \
						-s 1
				}else{
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
				if( $3 == 出る ){
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
			}
		// 4
		trap GBC004 d:PLAYER -mask ＊ -dir 0 512 -state d:TRP_STATE_CAUTION \
			-exec {
				command ロッカー状態 -n ロッカー４
				if( ($status & 0x0002) && !($status & 0x2000) ){
					chara カメラ設定 通路カメラ０ \
						-c 1 \
						-p 1036,1815,4835 -t 972,1418,3809 \
						-r 240,2089,0 -f 1102 \
						-a 200 \
						-i 1 1 0 0 \
						-s 1
				}else{
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
				if( $3 == 出る ){
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
			}

		// 5
		trap GBC005 d:PLAYER \
			-mask ＊ \
			-dir 2048 512 \
			-state d:TRP_STATE_CAUTION \
			-exec {
				command ロッカー状態 -n ロッカー６
				if( ($status & 0x0002) && !($status & 0x2000) ){
					chara カメラ設定 通路カメラ０ \
						-c 1 \
						-p 3714,1574,2373 -t 3770,1295,3789 \
						-r 127,26,0 -f 1445 \
						-a 200 \
						-i 1 1 0 0 \
						-s $3
				}else{
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
				if( $3 == 出る ){
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
			}

// グランド・グラビア・カメラ
		// 1
		trap GGC001 d:PLAYER -mask ＊ -state d:TRP_STATE_GROUND \
			-exec {
				command ロッカー状態 -n ロッカー１
				if( $status & 0x2000 ){
					chara カメラ設定 通路カメラ０ \
						-c 0 \
						-b -10000,0,-10000  10000,3000,10000 \
						-l -10000,0,-10000  10000,3000,10000 \
						-r 936,2048,0 -f 1732 \
						-a 200 \
						-i 1 1 0 0 \
						-s $3
				}
				// 念のためトラップから出た時、確実にカメラをＯＦＦしておく
				if( $3 == 出る ){
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
			}
		// 2
		trap GGC002 d:PLAYER -mask ＊ -state d:TRP_STATE_GROUND \
			-exec {
				command ロッカー状態 -n ロッカー２
				if( $status & 0x2000 ){
					chara カメラ設定 通路カメラ０ \
						-c 0 \
						-b -10000,0,-10000  10000,3000,10000 \
						-l -10000,0,-10000  10000,3000,10000 \
						-r 936,2048,0 -f 1732 \
						-a 200 \
						-i 1 1 0 0 \
						-s $3
				}
				// 念のためトラップから出た時、確実にカメラをＯＦＦしておく
				if( $3 == 出る ){
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
			}
		// 3
		trap GGC003 d:PLAYER -mask ＊ -state d:TRP_STATE_GROUND \
			-exec {
				command ロッカー状態 -n ロッカー３
				if( $status & 0x2000 ){
					chara カメラ設定 通路カメラ０ \
						-c 0 \
						-b -10000,0,-10000  10000,3000,10000 \
						-l -10000,0,-10000  10000,3000,10000 \
						-r 936,2048,0 -f 1732 \
						-a 200 \
						-i 1 1 0 0 \
						-s $3
				}
				// 念のためトラップから出た時、確実にカメラをＯＦＦしておく
				if( $3 == 出る ){
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
			}
		// 4
		trap GGC004 d:PLAYER -mask ＊ -state d:TRP_STATE_GROUND \
			-exec {
				command ロッカー状態 -n ロッカー４
				if( $status & 0x2000 ){
					chara カメラ設定 通路カメラ０ \
						-c 0 \
						-b -10000,0,-10000  10000,3000,10000 \
						-l -10000,0,-10000  10000,3000,10000 \
						-r 936,2048,0 -f 1732 \
						-a 200 \
						-i 1 1 0 0 \
						-s $3
				}
				// 念のためトラップから出た時、確実にカメラをＯＦＦしておく
				if( $3 == 出る ){
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
			}
		// 5
		trap GGC005 d:PLAYER \
			-mask ＊ \
			-state d:TRP_STATE_GROUND \
			-exec {
				command ロッカー状態 -n ロッカー６
				if( $status & 0x2000 ){
					chara カメラ設定 通路カメラ０ \
						-c 0 \
						-b -10000,0,-10000  10000,3000,10000 \
						-l -10000,0,-10000  10000,3000,10000 \
						-r 951,2048,0 -f 1442 \
						-a 200 \
						-i 1 1 0 0 \
						-s $3
				}

				// 念のためトラップから出た時、確実にカメラをＯＦＦしておく
				if( $3 == 出る ){
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
			}
// グラビア・特別カメラ
		// 3
		trap cg003 d:PLAYER -mask ＊ -dir 0 512 -state d:TRP_STATE_CAUTION \
			-exec {
				command ロッカー状態 -n ロッカー３
				if( ($status & 0x0002) && !($status & 0x2000) ){
					if ( `command プレイヤー状態取得` & d:PFLAG_SQUAT ) {
						chara カメラ設定 通路カメラ０ \
							-c 1 \
							-p -1331,1279,1866 -t -1323,1221,2205 \
							-r 110,14,0 -f 344 \
							-a 200 \
							-i 2 1 0 0 \
							-s $3
					} else {
						chara カメラ設定 通路カメラ０ \
							-c 1 \
							-p -1330,2027,1882 -t -1323,1881,2198 \
							-r 281,14,0 -f 348 \
							-a 200 \
							-i 1 1 0 0 \
							-s $3
					}
				}else{
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
				if( $3 == 出る ){
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
			}
		// 4
		trap cg004 d:PLAYER -mask ＊ -dir 2048 512 -state d:TRP_STATE_CAUTION \
			-exec {
				command ロッカー状態 -n ロッカー４
				if( ($status & 0x0002) && !($status & 0x2000) ){
					if ( `command プレイヤー状態取得` & d:PFLAG_SQUAT ) {
						chara カメラ設定 通路カメラ０ \
							-c 1 \
							-p 1080,1263,5806 -t 1080,1194,5326 \
							-r 93,2048,0 -f 485 \
							-a 200 \
							-i 2 1 0 0 \
							-s $3
					} else {
						chara カメラ設定 通路カメラ０ \
							-c 1 \
							-p 1058,2035,5807 -t 1058,1798,5375 \
							-r 327,2048,0 -f 493 \
							-a 200 \
							-i 1 1 0 0 \
							-s $3
					}
				}else{
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
				if( $3 == 出る ){
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
			}
			

		// 5
		trap cg005 d:PLAYER -mask ＊ -dir 0 512 -state d:TRP_STATE_CAUTION \
			-exec {
				command ロッカー状態 -n ロッカー６
				if( ($status & 0x0002) && !($status & 0x2000) ){
					if ( `command プレイヤー状態取得` & d:PFLAG_SQUAT ) {
						chara カメラ設定 通路カメラ０ \
							-c 1 \
							-p 3738,1220,1887 -t 3738,970,4209 \
							-r 70,0,0 -f 2335 \
							-a 200 \
							-i 2 1 0 0 \
							-s $3
					} else {
						chara カメラ設定 通路カメラ０ \
							-c 1 \
							-p 3720,2108,1885 -t 3720,919,3905 \
							-r 347,0,0 -f 2344 \
							-a 200 \
							-i 1 1 0 0 \
							-s $3
					}
				}else{
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
				if( $3 == 出る ){
					chara delay カメラＯＦＦ -time 2 -exec {
						chara カメラ設定 通路カメラ０ -s -1
					}
				}
			}
	}

/***************************************************************************************/
// 最後に行う処理
/***************************************************************************************/
	proc 写真撮影ミッション用最終設定 {
		#if d:DEBUG_PRINT
			print 'photo check'
		#endif

		if( $s:選択プレイヤー == ライデン ) {
			@モアイ撮影
		} else {
			if( $s:登場ポイント == 写真撮影スネーク１ ) {
				@ミスマガジン撮影
			} else {
				@クイズ系撮影
			}
		}

	}


/***************************************************************************************/
// モアイ撮影
/***************************************************************************************/
	proc モアイ撮影 {
		//@写真撮影ミッションデジカメ写真判定
		command カメラ撮影時プロック -proc モアイ撮影時プロック

		command カメラチェック面ワーク確保

		command カメラチェック面登録 \
			-points 4300 2800 (3500-250) \
					4300 2800 (3500+250) \
					4300 2300 (3500+250) \
					4300 2300 (3500-250) \
			-far 1 \
			-y_range 1024 \
			-proc モアイ写真判定大

		command カメラチェック面登録 \
			-points 4300 2750 (3500-100) \
					4300 2750 (3500+100) \
					4300 2480 (3500+100) \
					4300 2480 (3500-100) \
			-far 1 \
			-y_range 1024 \
			-proc モアイ写真判定小
	}

	proc モアイ写真判定大 {
		command ゲットデジカメ被写体情報 $$i:Ｘ画面位置 $$i:Ｙ画面位置 $$i:画面占有率

		#if d:DEBUG_PRINT
			print 'モアイ大判定'
			@写真判定プリント文 $1
			@２Ｄ用写真判定プリント文 $1
		#endif

		if( $1 == 4 ) {
			if( `@グラビアチェック範囲チェック 10 10 20` ) {
				$$b:撮影成功フラグ = 10;
			}
		}
	}

	proc モアイ写真判定小 {
		command ゲットデジカメ被写体情報 $$i:Ｘ画面位置 $$i:Ｙ画面位置 $$i:画面占有率

		#if d:DEBUG_PRINT
			print 'モアイ小判定'
			@写真判定プリント文 $1
			@２Ｄ用写真判定プリント文 $1
		#endif

		if( $1 == 4 ) {
			if( `@グラビアチェック範囲チェック 20 20 40` ) {
				$$b:撮影成功フラグ = 100;
			} else if( `@グラビアチェック範囲チェック 30 30 50` ) {
				$$b:撮影成功フラグ = 50;
			} else if( `@グラビアチェック範囲チェック 35 40 60` ) {
				$$b:撮影成功フラグ = 10;
			}
		}
	}

	proc モアイ撮影時プロック {
		print '撮影成功フラグ'$$b:撮影成功フラグ

		switch ( $$b:撮影成功フラグ ) {
			case ( 100 ) {
				command ＶＲクリア＿星の数の設定 3
				command ＶＲクリア＿スコアの設定 ( d:PHOTO_STAR_SCORE * 3 )
			}
			case ( 50 ) {
				command ＶＲクリア＿星の数の設定 2
				command ＶＲクリア＿スコアの設定 ( d:PHOTO_STAR_SCORE * 2 )
			}
			case ( 10 ) {
				command ＶＲクリア＿星の数の設定 1
				command ＶＲクリア＿スコアの設定 ( d:PHOTO_STAR_SCORE * 1 )
			}
		}

		if( $$b:撮影成功フラグ ) {
			command ＶＲポーズ中クリア
		} else {
			@ミッション共通ゲームオーバー処理
		}
	}
/***************************************************************************************/
// ミスマガジン撮影
/***************************************************************************************/
#define 和希沙也_XYZ	-3725 1400 2225
#define 阿部英理奈_XYZ	-3870 1450 3200
#define 石田裕子_XYZ	-1120 1500 4350
#define 中川翔子_XYZ	950 1550 3650
#define 安田美沙子_XYZ	3830 1440 3900



	proc ミスマガジン撮影 {
		command カメラ撮影時プロック -proc ミスマガジン撮影時プロック

		command カメラチェック面ワーク確保

		switch ( $$i:種 % 5 ) {
			case ( 0 ) {
				@チェック面登録＿表 d:和希沙也_XYZ p:ミスマガジン写真判定
			}
			case ( 1 ) {
				@チェック面登録＿表 d:阿部英理奈_XYZ p:ミスマガジン写真判定
			}
			case ( 2 ) {
				@チェック面登録＿裏 d:石田裕子_XYZ p:ミスマガジン写真判定
			}
			case ( 3 ) {
				@チェック面登録＿表 d:中川翔子_XYZ p:ミスマガジン写真判定
			}
			case ( 4 ) {
				@チェック面登録＿裏 d:安田美沙子_XYZ p:ミスマガジン写真判定
			}
		}
	}

	proc クイズ系撮影 {
		command カメラ撮影時プロック -proc ミスマガジン撮影時プロック

		command カメラチェック面ワーク確保

		if( ( $$i:ラウンド回数 % 11 ) < 5 ) {
		// ピンポイント
			switch ( $$i:種 % 5 ) {
				case ( 0 ) {
					@チェック面登録＿表 d:和希沙也_XYZ p:ミスマガジン写真判定
					@チェック面登録ピンポイント＿表 -3725 1330 2225 p:ピンポイント写真判定
				}
				case ( 1 ) {
					@チェック面登録＿表 d:阿部英理奈_XYZ p:ミスマガジン写真判定
					@チェック面登録ピンポイント＿表 -3950 1500 3200 p:ピンポイント写真判定
				}
				case ( 2 ) {
					@チェック面登録＿裏 d:石田裕子_XYZ p:ミスマガジン写真判定
					@チェック面登録ピンポイント＿裏 -1155 1530 4350 p:ピンポイント写真判定
				}
				case ( 3 ) {
					@チェック面登録＿表 d:中川翔子_XYZ p:ミスマガジン写真判定
					@チェック面登録ピンポイント＿表 960 1485 3650 p:ピンポイント写真判定
				}
				case ( 4 ) {
					@チェック面登録＿裏 d:安田美沙子_XYZ p:ミスマガジン写真判定
					@チェック面登録ピンポイント＿裏 3765 1340 3900 p:ピンポイント写真判定
				}
			}
		} else if( ( $$i:ラウンド回数 % 11 ) == 10 ) {
		// 監督クイズ
			@チェック面登録＿表 d:和希沙也_XYZ p:和希沙也写真判定
			@エロ登録＿表 		-3800  950 2225 p:和希沙也エロ判定
			
			@チェック面登録＿表 d:阿部英理奈_XYZ p:阿部英理奈写真判定
			@エロ登録＿表 		-3720 1000 3200 p:阿部英理奈エロ判定
			
			@チェック面登録＿裏 d:石田裕子_XYZ p:石田裕子写真判定
			@エロ登録＿裏 		-1040 1250 4350 p:石田裕子エロ判定
			
			@チェック面登録＿表 d:中川翔子_XYZ p:中川翔子写真判定
			@エロ登録＿表 		1080 1280 3650 p:中川翔子エロ判定
			
			@チェック面登録＿裏 d:安田美沙子_XYZ p:安田美沙子写真判定
			@エロ登録＿裏 		3830  900 3900 p:安田美沙子エロ判定
		} else {
		// 製作者クイズ
			switch ( $$i:種 % 5 ) {
				case ( 0 ) {
					@チェック面登録＿表 d:和希沙也_XYZ p:ミスマガジン写真判定
				}
				case ( 1 ) {
					@チェック面登録＿表 d:阿部英理奈_XYZ p:ミスマガジン写真判定
				}
				case ( 2 ) {
					@チェック面登録＿裏 d:石田裕子_XYZ p:ミスマガジン写真判定
				}
				case ( 3 ) {
					@チェック面登録＿表 d:中川翔子_XYZ p:ミスマガジン写真判定
				}
				case ( 4 ) {
					@チェック面登録＿裏 d:安田美沙子_XYZ p:ミスマガジン写真判定
				}
			}
		}
	}


	proc ロッカー状態チェック $:ロッカー名 {
		command ロッカー状態 -n $:ロッカー名
		if( ( $status & d:LOCKER_STATE_OPEN ) && !( $status & d:LOCKER_STATE_BROKEN ) ) {
			print 'ロッカー状態良好'
			return 1;
		} else {
			print 'ロッカー状態最悪！'
			return 0;
		}
	}
	
	proc ノーマル判定 $:判定値 {
		print 'ノーマル判定'
		command ゲットデジカメ被写体情報 $$i:Ｘ画面位置 $$i:Ｙ画面位置 $$i:画面占有率
		#if d:DEBUG_PRINT
			@写真判定プリント文 $1
			@２Ｄ用写真判定プリント文 $1
		#endif
		
		if( $:判定値 == 4 && $$i:画面占有率 > 200 ) {
			if( `@グラビアチェック範囲チェック 10 30 40` ) {
				if( $$b:撮影成功フラグ < 100 ) { $$b:撮影成功フラグ = 100; }
			} else if( `@グラビアチェック範囲チェック 20 30 50` ) {
				if( $$b:撮影成功フラグ < 50 ) { $$b:撮影成功フラグ = 50; }
			} else if( `@グラビアチェック範囲チェック 25 30 70` ) {
				if( $$b:撮影成功フラグ < 10 ) { $$b:撮影成功フラグ = 10; }
			}
		}
		print '暫定撮影成功フラグ'$$b:撮影成功フラグ
	}

	proc エロ判定 $:判定値 {
		print 'エロ判定'
		command ゲットデジカメ被写体情報 $$i:Ｘ画面位置 $$i:Ｙ画面位置 $$i:画面占有率
		#if d:DEBUG_PRINT
			@写真判定プリント文 $1
			@２Ｄ用写真判定プリント文 $1
		#endif
		
		if( $:判定値 == 4 && $$i:画面占有率 > 400 ) {
			if( `@グラビアチェック範囲チェック 25 30 30` ) {
				if( $$b:撮影成功フラグ < 200 ) { $$b:撮影成功フラグ = 200; }
			}
		}
		print '暫定撮影成功フラグ'$$b:撮影成功フラグ
	}

// 和希沙也
	proc 和希沙也写真判定 {
		print '和希沙也写真判定'
		if( `@ロッカー状態チェック ロッカー１` ) {
			@ノーマル判定 $1
		}
	}
	proc 和希沙也エロ判定 {
		print '和希沙也エロ判定'
		if( `@ロッカー状態チェック ロッカー１` ) {
			@エロ判定 $1
		}
	}
// 阿部英理奈
	proc 阿部英理奈写真判定 {
		print '阿部英理奈写真判定'
		if( `@ロッカー状態チェック ロッカー２` ) {
			@ノーマル判定 $1
		}
	}
	proc 阿部英理奈エロ判定 {
		print '阿部英理奈エロ判定'
		if( `@ロッカー状態チェック ロッカー２` ) {
			@エロ判定 $1
		}
	}
// 石田裕子
	proc 石田裕子写真判定 {
		print '石田裕子写真判定'
		if( `@ロッカー状態チェック ロッカー３` ) {
			@ノーマル判定 $1
		}
	}
	proc 石田裕子エロ判定 {
		print '石田裕子エロ判定'
		if( `@ロッカー状態チェック ロッカー３` ) {
			@エロ判定 $1
		}
	}
// 中川翔子
	proc 中川翔子写真判定 {
		print '中川翔子写真判定'
		if( `@ロッカー状態チェック ロッカー４` ) {
			@ノーマル判定 $1
		}
	}
	proc 中川翔子エロ判定 {
		print '中川翔子エロ判定'
		if( `@ロッカー状態チェック ロッカー４` ) {
			@エロ判定 $1
		}
	}
// 安田美沙子
	proc 安田美沙子写真判定 {
		print '安田美沙子写真判定'
		if( `@ロッカー状態チェック ロッカー６` ) {
			@ノーマル判定 $1
		}
	}
	proc 安田美沙子エロ判定 {
		print '安田美沙子エロ判定'
		if( `@ロッカー状態チェック ロッカー６` ) {
			@エロ判定 $1
		}
	}

	proc ミスマガジン写真判定 {
		print '写真１'
		/*
		command ゲットデジカメ被写体情報 $$i:Ｘ画面位置 $$i:Ｙ画面位置 $$i:画面占有率
		#if d:DEBUG_PRINT
			@写真判定プリント文 $1
			@２Ｄ用写真判定プリント文 $1
		#endif
		*/


		switch ( $$i:種 % 5 ) {
			case ( 0 ) {
				command ロッカー状態 -n ロッカー１
			}
			case ( 1 ) {
				command ロッカー状態 -n ロッカー２
			}
			case ( 2 ) {
				command ロッカー状態 -n ロッカー３
			}
			case ( 3 ) {
				command ロッカー状態 -n ロッカー４
			}
			case ( 4 ) {
				command ロッカー状態 -n ロッカー６
			}
		}

		if( ( $status & d:LOCKER_STATE_OPEN ) && !( $status & d:LOCKER_STATE_BROKEN ) ) {
			print 'ロッカー状態良好'
			print 'グラビアチェック'
			/*
			if( $1 == 4 && $$i:画面占有率 > 250 ) {
				if( `@グラビアチェック範囲チェック 10 30 40` ) {
					print 'A'
					$$b:撮影成功フラグ = 100;
				} else if( `@グラビアチェック範囲チェック 20 30 50` ) {
					print 'B'
					$$b:撮影成功フラグ = 50;
				} else if( `@グラビアチェック範囲チェック 20 30 70` ) {
					print 'C'
					$$b:撮影成功フラグ = 10;
				}
			}
			*/
			@ノーマル判定 $1
		} else {
			print 'ロッカー状態最悪！'
		}
	}

	proc ピンポイント写真判定 {
		print '写真１'
		command ゲットデジカメ被写体情報 $$i:Ｘ画面位置 $$i:Ｙ画面位置 $$i:画面占有率
		#if d:DEBUG_PRINT
			@写真判定プリント文 $1
			@２Ｄ用写真判定プリント文 $1
		#endif


		switch ( $$i:種 % 5 ) {
			case ( 0 ) {
				command ロッカー状態 -n ロッカー１
			}
			case ( 1 ) {
				command ロッカー状態 -n ロッカー２
			}
			case ( 2 ) {
				command ロッカー状態 -n ロッカー３
			}
			case ( 3 ) {
				command ロッカー状態 -n ロッカー４
			}
			case ( 4 ) {
				command ロッカー状態 -n ロッカー６
			}
		}

		if( ( $status & d:LOCKER_STATE_OPEN ) && !( $status & d:LOCKER_STATE_BROKEN ) ) {
			print 'ロッカー状態良好'
			print 'グラビアチェック'
			if( $1 == 4 && $$i:画面占有率 > 200 ) {
				print 'ピンポイント 成功！'
				$$b:撮影成功フラグ = 100;
			} else {
				print 'ピンポイントは失敗！！！！！'
			}
		} else {
			print 'ロッカー状態最悪！'
		}
	}

	proc ミスマガジン撮影時プロック {
		print '撮影成功フラグ'$$b:撮影成功フラグ
		switch ( $$b:撮影成功フラグ ) {
			case ( 200 ) {
				command ＶＲクリア＿星の数の設定 3
				command ＶＲクリア＿スコアの設定 ( 99999 )
			}
			case ( 100 ) {
				command ＶＲクリア＿星の数の設定 3
				command ＶＲクリア＿スコアの設定 ( d:PHOTO_STAR_SCORE * 3 )
			}
			case ( 50 ) {
				command ＶＲクリア＿星の数の設定 2
				command ＶＲクリア＿スコアの設定 ( d:PHOTO_STAR_SCORE * 2 )
			}
			case ( 10 ) {
				command ＶＲクリア＿星の数の設定 1
				command ＶＲクリア＿スコアの設定 ( d:PHOTO_STAR_SCORE * 1 )
			}
		}

		#if 0
			command ＶＲポーズ中クリア
		#else
			if( $$b:撮影成功フラグ ) {
				command ＶＲポーズ中クリア
			} else {
				@ミッション共通ゲームオーバー処理
			}
		#endif

	}



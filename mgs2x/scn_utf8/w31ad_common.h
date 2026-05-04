/*
	w31ad_common.h                     
		シェル２中央棟１Ｆ-共通プロック

	2001/05/04 H.Yoshiike         
	$Id: w31ad_common.h,v 1.60 2002/05/09 13:07:50 usr01475 Exp $                      

	
*/

// 壊れ物番号（多いのでenumで）
enum D_NO {
	FENCE_01 = 0,
	FENCE_02,
	FENCE_03,
	FENCE_04,
	FENCE_05,
	FENCE_06,
	FENCE_07,
	FENCE_08,
	FENCE_09,
	FENCE_10,
	FENCE_11,
	FENCE_12,
	FENCE_13,
	FENCE_14,
	FENCE_15,
	FENCE_16,
	FENCE_17,
	FENCE_18,
	FENCE_19,
	FENCE_20,
	FENCE_21,
	FENCE_22,
	GUNCAM_01,
	GUNCAM_02
}

// アイテム番号（アイテムが多いのでenumで）
enum I_NO {
	M9_R01 = 0,
	M9_R02,
	SOCOM_R01,
	SOCOM_R02,
	SOCOM_R03,
	SOCOM_R04,
	M4_R01,
	M4_R02,
	AKS_R01,
	AKS_R02,
	NIKITA_R01,
	NIKITA_R02,
	PSG1_R01,
	PSG1T_R01,
	RGB6_R01,
	STINGER_R01,
	C4_01,
	CLAY_01,
	CHAFF_01,
	MAG_01,
	RATION_01,
	RATION_02,
	DIAZE_01
}


// 各proc設定
//-------------------------------------------------------------------------------------
// 全マップ共通
	// サウンド
	// ------------------------------------
	proc サウンド設定 {
		#if d:DEBUG_PRINT 
			print 'sound_set'
		#endif

		chara サウンドマネージャー ＳＤマネ -pak 0
		@サウンド効果音設定

		eval( $f:ＢＧＭマネージャー起動フラグ = 0 );
	}

	// ローカル変数初期化
	// ------------------------------------
	proc ローカル変数初期化 {
		#if d:DEBUG_PRINT 
			print 'localvar_init'
		#endif

		// エマがドアを開け閉めしたりしないようにするのに必要
		eval( $f:エマゲームオーバーフラグ = 0 );

		#ifdef d:STAGE_W31A
			// 大統領の座標取得用
			eval( $i:ローカル変数Ｉ１ = 0 );
			eval( $i:ローカル変数Ｉ２ = 0 );
			eval( $i:ローカル変数Ｉ３ = 0 );
			// ニキータイベント用アイテム再配置
			eval($f:w31a_アイテム再セット１ = 0);
			eval($f:w31a_アイテム再セット２ = 0);
		#endif

		#ifdef d:STAGE_W31D
			eval( $w:敵標準再発生数 = 16 );
			// エマの手繋ぎチェック用
			eval( $i:ローカル変数Ｉ１ = 0 );
			// クリアリング中の子画面チェック用
			eval( $f:クリアリング子画面フラグ = 0 );
			// クリアリングゲームステータス取得用
			eval( $i:ローカル変数Ｉ２ = 0 );
		#endif
	}

	// 設定値を変更
	// ------------------------------------
	proc 設定値変更 {
		#if d:DEBUG_PRINT 
			print 'status_set'
		#endif
	}

	// プラグインプログラム
	// ------------------------------------
	proc プラグイン設定 {
		#if d:DEBUG_PRINT
			print 'plugin_set'
		#endif

		command プラグイン自動ジャンプ
	}

	// プレイヤー 
	// ------------------------------------
	proc プレイヤー設定 {
		#if d:DEBUG_PRINT
			print 'player_set'
		#endif

		chara プレイヤー d:PLAYER \
			-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
			-d 0,$i:プレイヤー初期方向,0 \
			-s $b:プレイヤー初期姿勢 \
			-m rai_def -a raiden -o rai_w31a \
			-i 0

		@プレイヤーＬＯＤ設定 d:C_DISTANCE_TYPE 6000

		// 髪の毛
		@ライデン髪の毛設定 d:NORMAL_HAIR
	}


	// 鏡面モデル設定
	// -----------------------------------
	proc 鏡面モデル設定 {
		#if d:DEBUG_PRINT 
			print 'mirror_model_set'
		#endif

		chara 鏡面モデル管理 鏡面モデル \
			-pos  0, -10, 0\
			-axis 1
		trap ev200 ？ -mask ？ \
			-exec {
				if($3 == 入る) {
					mesg 鏡面モデル管理 鏡面モデル add $2
				} else {
					mesg 鏡面モデル管理 鏡面モデル del $2
				}
			}
	}

	// 監視カメラ関係
	// -----------------------------------
	proc ガンカメラ設置 {
		#if d:DEBUG_PRINT 
			print 'gun_camera_set'
		#endif

		// 小部屋入り口付近
		@カメラ再セットチェック $b:w31a_破壊ロード回数[d:D_NO:GUNCAM_01]
		if ( $f:再セットフラグ == d:TRUE ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:GUNCAM_01] = 0 );
		}
		chara ガンカメラ ガンカメラ \
			-pos -8750,3000,-233750 \
			-rot 256,1024,0 \ /*ＸＹ値のみ有効*/
			-alertdir 512 \ /*片側角度*/
			-center 512 0 \ /*初期方向は符号付き相対指定*/
			-xran 256 \ 
			-yran 200 \ 
			-i 5000 5000 5000 \
			-status $b:w31a_破壊フラグ[d:D_NO:GUNCAM_01] \
			-dir 0 \
			-niktime 300 \
			-exec {
				eval( $b:w31a_破壊ロード回数[d:D_NO:GUNCAM_01] = $w:グローバルロード回数);	
				eval( $b:w31a_破壊フラグ[d:D_NO:GUNCAM_01] = $2 );
			}

		// 小部屋ダクト付近
		@カメラ再セットチェック $b:w31a_破壊ロード回数[d:D_NO:GUNCAM_02]
		if ( $f:再セットフラグ == d:TRUE ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:GUNCAM_02] = 0 );
		}

		chara ガンカメラ ガンカメラ \
			-pos -7500,3000,-231200 \
			-rot 380,2048,0 \ /*ＸＹ値のみ有効*/
			-alertdir 1024 \ /*片側角度*/
			-center 2000 0 \ /*初期方向は符号付き相対指定*/
			-xran 512 \ 
			-yran 200 \ 
			-i 5000 5000 5000 \
			-status $b:w31a_破壊フラグ[d:D_NO:GUNCAM_02] \
			-dir 0 \
			-niktime 300 \
			-exec {
				eval( $b:w31a_破壊ロード回数[d:D_NO:GUNCAM_02] = $w:グローバルロード回数);	
				eval( $b:w31a_破壊フラグ[d:D_NO:GUNCAM_02] = $2 );
			}
	}


	// ＢＧＭ
	// ------------------------------------
	proc ＢＧＭ設定 {
		#if d:DEBUG_PRINT 
			print 'bgm_set'
		#endif

		chara ＢＧＭマネージャー ＢＧＭマネ
		eval( $f:ＢＧＭマネージャー起動フラグ = 1 );
	}

	// 弾痕跳弾関係の設定
	// ------------------------------------
	proc 弾痕跳弾設定 {
		#if d:DEBUG_PRINT 
			print 'recoil_set'
		#endif

		command 弾痕ノーマル -n 0	// 通常
		command 弾痕ノーマル -n 1	// 鉄階段、鉄板
		command 弾痕ノーマル -n 2	// 階段、通常
		command 弾痕ノーマル -n 3	// 鉄床、鉄筋
		command 弾痕ノーマル -n 6	// ダンボール

		command 跳弾ノーマル -n 0
		command 跳弾ノーマル -n 1
		command 跳弾ノーマル -n 2
		command 跳弾ノーマル -n 3
		command 跳弾ＳＥのみ -n 6

		// ガラス弾痕
		chara ガラス弾痕 ガラス弾痕 \
			-flag   0 \
			-thick  1 \
			-size   60 \
			-number 4 \
			-point	-3970,1900,-247650 -3970,1000,-246900 \
			-point	-3970,1900,-246850 -3970,1000,-246100 \
			-point	-7100,1900,-243220 -7850,1000,-243220 \
			-point	-7900,1900,-243220 -8650,1000,-243220
		

	}

	// ノード端末
	// ------------------------------------
	proc ノード接続時プロック {
		#if d:DEBUG_PRINT
			print 'node_set'
		#endif

		eval($f:ノードフラグ = $f:w31a_ノードフラグ);
	}

	proc ノード接続後プロック {
		#if d:DEBUG_PRINT
			print 'node_event_go'
		#endif

		eval($f:w31a_ノードフラグ = 1);
		command メニュー設定 -node_access on 
	}

	// ドア＆マップ切り替え関係
	// ------------------------------------
	proc ドア設置 {
		#if d:DEBUG_PRINT
			print 'slide_door_set'
		#endif

		@ドアランプ設定 １Ｆ右ドアランプ 1024 19125,0,-234250 2
		if($s:d_num == not_demo) {
			chara ドア １Ｆ右ドア \
				-m d:DOOR_NAME_LOAD \
				-d 0,1024,0 \
				-p 19125,0,-234250 \
				-slide d:SD_SLIDE_OUTSIDE \
				-time d:SD_OPEN_TIME \
				-between シェル２中央棟１Ｆ シェル２中央棟１Ｆ \
				-A １Ｆ右ドアランプ \
				-exec {
				}
			trap dr001 ？ \
				-mask ？ \	
				-exec {
						if($3==入る){
							mesg ドア １Ｆ右ドア open
						}else{
							if ( $f:エマゲームオーバーフラグ == 0 ) {
								mesg ドア １Ｆ右ドア close
							}
						}
				}

			trap dr001 d:PLAYER -mask ？ \
				-state d:TRP_STATE_SUBJECT \
				-exec {
					if($3 == 入る) {
						mesg 透明壁 １Ｆ右ドアへの透明壁 on
					} else if($3 == 出る) {
						mesg 透明壁 １Ｆ右ドアへの透明壁 off
					}
				}
		} else {
			chara プットオブジェ １Ｆ右ドア \
				-m d:DOOR_NAME_LOAD \
				-r 0,1024,0 \
				-p 19125,0,-234250 \
				-s 100,100,100 -l d:LT2_NAME
		}

		// 開かずの扉(デモでは写らないのでいれない)
		if($s:d_num == not_demo) {
			@壊れドアエフェクト設定 壊れドアエフェクト 1024 -19125,0,-234250 0
			chara ドア １Ｆ左ドア \
				-m d:DOOR_NAME_LOAD \
				-d 0,1024,0 \
				-p -19125,0,-234250 \
				-slide d:SD_SLIDE_OUTSIDE \
				-time d:SD_OPEN_TIME \
				-between シェル２中央棟１Ｆ シェル２中央棟１Ｆ \
				-card_level 9999 \
				-exec {
				}
		}


		@ドアランプ設定 倉庫ドアランプ 1024 -8875,0,-231500 1
		if($s:d_num == not_demo) {
			chara ドア 倉庫ドア \
				-m d:DOOR_NAME \
				-d 0,1024,0 \
				-p -8875,0,-231500 \
				-slide d:SD_SLIDE_INSIDE \
				-time d:SD_OPEN_TIME \
				-between シェル２中央棟１Ｆ シェル２中央棟１Ｆ \
				-A 倉庫ドアランプ \
				-exec {
				}
			trap dr002 ？ \
				-mask ？ \	
				-exec {
						if($3==入る){
							mesg ドア 倉庫ドア open
						}else{
							if ( $f:エマゲームオーバーフラグ == 0 ) {
								mesg ドア 倉庫ドア close
							}
						}
				}
		} else {
			chara プットオブジェ 倉庫ドア \
				-m d:DOOR_NAME \
				-r 0,1024,0 \
				-p -8875,0,-231500 \
				-s 100,100,100 -l d:LT2_NAME
		}

		@ドアランプ設定 監禁室ドアランプ 1024 3625,0,-240000 0
		@ドアランプ設定 監禁室ドアランプ 1024 3375,0,-240000 1
		if($s:d_num == not_demo) {
			chara ドア 監禁室ドア \
				-m d:DOOR_NAME \
				-d 0,1024,0 \
				-p 3500,0,-240000 \
				-slide 1500 \
				-time d:SD_OPEN_TIME \
				-between シェル２中央棟１Ｆ シェル２中央棟１Ｆ \
				-A 監禁室ドアランプ \
				-exec {
				}
			trap dr003 ？ \
				-mask ？ \	
				-exec {
					if ( $w:p_story >= d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１終了 ) {
					// 電源パネルを破壊しないと中に入れない
						if( $3==入る && $2 != 健康な大統領 ){
							if ( $w:p_story < d:ST:P049_01_P01大統領１ポリゴンデモ１開始 ) {
								eval( $s:登場ポイント = ポリゴンデモへ );
								#ifdef d:STAGE_W31A
									command セットサウンドコード -c d:SNG_FOUTS_S
									@ポリデモアイテムチェック後スタート d:ST:P049_01_P01大統領１ポリゴンデモ１開始
								#endif
							} else if ( $w:p_story >= d:ST:P049_11_P02大統領１１ポリゴンデモ２終了 && \
										$w:p_story < d:ST:P050_01_R01タンカー編整理１無線機デモ１開始 && \
					 					(`command ゲームオーバーチェック` == 0 )) {
								#ifdef d:STAGE_W31A
									@シナリオデモ開始処理（ダンボールも解除）
									mesg プレイヤー d:PLAYER run 3250 0 -240750 0 1024 \
										(d:FA_DIRECT_MOVE | d:FA_NO_INTERP) 無線開始位置へ移動

									chara delay カメラディレイ \
										-time 2 \
										-exec {
											chara カメラ設定 シナリオデモカメラ \
												-c 1 \	
												-p 811,4087,-239710 -t 2333,2117,-240796 \
												-r 529,1428,0 -f 2716 \
												-a 200 \
												-i 0 0 0 0 \
												-s 1
										}
								#endif
							}

							mesg ドア 監禁室ドア open
						} else {
							#if d:DEBUG_PRINT 
								if ( $2 == 健康な大統領 ) {
									print '大統領は扉を開けられません---------------------------------------------------------'
								}
							#endif
							if ( $f:エマゲームオーバーフラグ == 0 ) {
								mesg ドア 監禁室ドア close
							}
						}
					}
				}
		} else {
			chara プットオブジェ 監禁室ドア \
				-m d:DOOR_NAME \
				-r 0,1024,0 \
				-p 3500,0,-240000 \
				-s 100,100,100 -l d:LT2_NAME
		}
	}

	proc 主観禁止壁設定 {
		@主観禁止壁 １Ｆ右ドアへの透明壁 19125,0,-234250 1024 d:SD_SLIDE_OUTSIDE
	}

	// 入出トラップ
	// ------------------------------------
	proc ロードプロック {
		#if d:DEBUG_PRINT 
			print 'load_proc'
		#endif

		#ifndef d:ANOTHER
		// 本編の処理
			#ifdef d:STAGE_W31A
				// ＢＧＭマネージャーがいないので強制的にフェードアウトをかける
				command セットサウンドコード -c d:SNG_FOUTS_S
				// 大統領の位置をロードのたびに覚える
				if ( $w:p_story >= d:ST:P049_11_P02大統領１１ポリゴンデモ２終了 ) {
					command コントロール座標取得 大統領死体 $i:w31a_大統領Ｘ位置,$i:w31a_大統領Ｙ位置,$i:w31a_大統領Ｚ位置
					command コントロール方向取得 大統領死体 $i:w31a_大統領方向
					command 倒れ状態取得 大統領死体 $w:w31a_大統領状態
				}
			#endif

			if ($s:呼び出しプロック == mv_w31a_w25c_0) {
				// ＫＬ連絡橋へ
				@mv_w31a_w25c_0
			} else if ($s:呼び出しプロック == mv_w31a_w31b_0) {
				// Ｂ１へ
				@mv_w31a_w31b_0
			}
		#else
		// アナザーミッションの処理
			if ($s:呼び出しプロック == mv_a31a_a25d_0) {
				// ＫＬ連絡橋へ
				@mv_a31a_a25d_0
			}
		#endif
	}

	proc ロード設定 {
		#if d:DEBUG_PRINT 
			print 'load_set'
		#endif

		#ifndef d:ANOTHER
			trap ld001 d:PLAYER \
				-mask 入る \
				-exec {
					if( $f:ロードチェックＯＮフラグ == 1 ) {
						// ＫＬ連絡橋へ
						@エンドカメラセット ＫＬ連絡橋
						if ( $f:夕方フラグ == 0 ) {
							preseek 'w25c'
						} else {
							preseek 'w25d'
						}
						@ステージ終了時処理（ステージ名表示） 21000 0 -235000 1024 mv_w31a_w25c_0 d:DEFAULT_M_TYPE d:wn_ＫＬ連絡橋
					}
				}
		#else
		// アナザーミッションの処理
			trap ld001 d:PLAYER \
				-mask 入る \
				-exec {
					if( $f:ロードチェックＯＮフラグ == 1 ) {
						// ＫＬ連絡橋へ
						@エンドカメラセット ＫＬ連絡橋
						preseek 'a25d'
						@ステージ終了時処理（ステージ名表示） 21000 0 -235000 1024 mv_a31a_a25d_0 d:DEFAULT_M_TYPE d:wn_ＫＬ連絡橋
					}
				}
		#endif
	}

	proc 強制ドアクローズ終了_B1へ {
		#if d:DEBUG_PRINT 
			print 'w31b_load_go'
		#endif

		if( $f:ロードチェックＯＮフラグ == 1 ) {
			// シェル２中央棟Ｂ１へ
			@エマエレベーター内チェック w31b
			preseek 'w31b'
			@ステージ終了時処理（ステージ名表示） 0 0 0 0 mv_w31a_w31b_0 d:NO_MOVE_M_TYPE d:wn_シェル２中央棟Ｂ１第一濾過室
		}
	}

	proc 強制ドアクローズ終了_B2へ {
		#if d:DEBUG_PRINT 
			print 'no_load_go'
		#endif

		mesg プレイヤー d:PLAYER evpanel 出る 0
	}


	// ダクト金網設定
	// ------------------------------------
	proc ダクト金網設定 {
		#if d:DEBUG_PRINT
			print 'duct_fence_set'
		#endif

		if ( $w:ゲーム設定 == d:LEVEL_VERYEASY ) {
			command foreach -argc 6 -repeat 13 \
				-data { \
					{ d:D_NO:FENCE_01,	金網:01, 250, -1500, -232875, 1024 }
					{ d:D_NO:FENCE_02,	金網:02, 500, -1500, -234125, 0 }
//					{ d:D_NO:FENCE_03,	金網:03, 1750, -1500, -232875, 1024 }
					{ d:D_NO:FENCE_04,	金網:04, 7250, -1000, -234000, 1024 }
					{ d:D_NO:FENCE_05,	金網:05, -4500, 250, -235500, 0 }
//					{ d:D_NO:FENCE_06,	金網:06, -4250, 250, -237250, 0 }
					{ d:D_NO:FENCE_07,	金網:07, -1000, 250, -237500, 0 }
					{ d:D_NO:FENCE_08,	金網:08, -5000, 2000, -234250, 3072 }
//					{ d:D_NO:FENCE_09,	金網:09, -2250, 1000, -233250, 1024 }
//					{ d:D_NO:FENCE_10,	金網:10, -2000, 1000, -234500, 0 }
//					{ d:D_NO:FENCE_11,	金網:11, -750, 1000, -233250, 1024 }
					{ d:D_NO:FENCE_12,	金網:12, -2000, 1000, -233000, 0 }
					{ d:D_NO:FENCE_13,	金網:13, 2250, 250, -233250, 1024 }
//					{ d:D_NO:FENCE_14,	金網:14, 2500, 250, -234500, 0 }
					{ d:D_NO:FENCE_15,	金網:15, 3750, 250, -233250, 1024 }
					{ d:D_NO:FENCE_16,	金網:16, -2250, 1250, -236250, 1024 }
					{ d:D_NO:FENCE_17,	金網:17, -2000, 1250, -237500, 0 }
//					{ d:D_NO:FENCE_18,	金網:18, 2250, 1250, -237750, 1024 }
					{ d:D_NO:FENCE_19,	金網:19, 3750, 1250, -237750, 1024 }
//					{ d:D_NO:FENCE_20,	金網:20, 2500, 1250, -237500, 0 }
//					{ d:D_NO:FENCE_21,	金網:21, -7500, 2000, -238000, 0 }
					{ d:D_NO:FENCE_22,	金網:22, -5750, 2000, -239375, 1024 }
				} \
				-exec { \
					// 壊れていたらロード回数を数えて再発生
					@壊れ物再セットチェック $b:w31a_破壊ロード回数[$1]
					if ( $f:再セットフラグ == d:TRUE ) {
						eval( $b:w31a_破壊フラグ[$1] = 0 );
						chara プット壊れ通気口 $2 \
							-model w31a_duct_fence0 w31a_duct_fence1 w31a_duct_fence2  \
							-pos   $3, $4, $5 \
							-rot   0, $6, 0 \
							-life  1 \
							-proc  ダクト金網状態保存設定

						// 健全な状態だったら主観設定を入れる
						if ( $2 == 金網:05 ) {
							command トラップ切り替え -trap fence_s:02 -switch 1
						} else if ( $2 == 金網:06 ) {
							command トラップ切り替え -trap fence_n -switch 1
						} else if ( $2 == 金網:07 ) {
							command トラップ切り替え -trap fence_s:01 -switch 1
						}

					} else {
						chara プット壊れ通気口 $2 \
							-model w31a_duct_fence0 w31a_duct_fence1 w31a_duct_fence2  \
							-pos   $3, $4, $5 \
							-rot   0, $6, 0 \
							-life  1 \
							-proc  ダクト金網状態保存設定 \
							-state $b:w31a_破壊フラグ[$1]

					}
				}
		} else if ( $w:ゲーム設定 == d:LEVEL_EASY ) {
			command foreach -argc 6 -repeat 11 \
				-data { \
					{ d:D_NO:FENCE_01,	金網:01, 250, -1500, -232875, 1024 }
//					{ d:D_NO:FENCE_02,	金網:02, 500, -1500, -234125, 0 }
//					{ d:D_NO:FENCE_03,	金網:03, 1750, -1500, -232875, 1024 }
//					{ d:D_NO:FENCE_04,	金網:04, 7250, -1000, -234000, 1024 }
					{ d:D_NO:FENCE_05,	金網:05, -4500, 250, -235500, 0 }
//					{ d:D_NO:FENCE_06,	金網:06, -4250, 250, -237250, 0 }
					{ d:D_NO:FENCE_07,	金網:07, -1000, 250, -237500, 0 }
					{ d:D_NO:FENCE_08,	金網:08, -5000, 2000, -234250, 3072 }
//					{ d:D_NO:FENCE_09,	金網:09, -2250, 1000, -233250, 1024 }
//					{ d:D_NO:FENCE_10,	金網:10, -2000, 1000, -234500, 0 }
//					{ d:D_NO:FENCE_11,	金網:11, -750, 1000, -233250, 1024 }
					{ d:D_NO:FENCE_12,	金網:12, -2000, 1000, -233000, 0 }
					{ d:D_NO:FENCE_13,	金網:13, 2250, 250, -233250, 1024 }
					{ d:D_NO:FENCE_14,	金網:14, 2500, 250, -234500, 0 }
//					{ d:D_NO:FENCE_15,	金網:15, 3750, 250, -233250, 1024 }
					{ d:D_NO:FENCE_16,	金網:16, -2250, 1250, -236250, 1024 }
					{ d:D_NO:FENCE_17,	金網:17, -2000, 1250, -237500, 0 }
//					{ d:D_NO:FENCE_18,	金網:18, 2250, 1250, -237750, 1024 }
//					{ d:D_NO:FENCE_19,	金網:19, 3750, 1250, -237750, 1024 }
					{ d:D_NO:FENCE_20,	金網:20, 2500, 1250, -237500, 0 }
//					{ d:D_NO:FENCE_21,	金網:21, -7500, 2000, -238000, 0 }
					{ d:D_NO:FENCE_22,	金網:22, -5750, 2000, -239375, 1024 }
				} \
				-exec { \
					// 壊れていたらロード回数を数えて再発生
					@壊れ物再セットチェック $b:w31a_破壊ロード回数[$1]
					if ( $f:再セットフラグ == d:TRUE ) {
						eval( $b:w31a_破壊フラグ[$1] = 0 );
						chara プット壊れ通気口 $2 \
							-model w31a_duct_fence0 w31a_duct_fence1 w31a_duct_fence2  \
							-pos   $3, $4, $5 \
							-rot   0, $6, 0 \
							-life  1 \
							-proc  ダクト金網状態保存設定

						// 健全な状態だったら主観設定を入れる
						if ( $2 == 金網:05 ) {
							command トラップ切り替え -trap fence_s:02 -switch 1
						} else if ( $2 == 金網:06 ) {
							command トラップ切り替え -trap fence_n -switch 1
						} else if ( $2 == 金網:07 ) {
							command トラップ切り替え -trap fence_s:01 -switch 1
						}

					} else {
						chara プット壊れ通気口 $2 \
							-model w31a_duct_fence0 w31a_duct_fence1 w31a_duct_fence2  \
							-pos   $3, $4, $5 \
							-rot   0, $6, 0 \
							-life  1 \
							-proc  ダクト金網状態保存設定 \
							-state $b:w31a_破壊フラグ[$1]

					}
				}
		} else if ( $w:ゲーム設定 == d:LEVEL_NORMAL ) {
			command foreach -argc 6 -repeat 11 \
				-data { \
					{ d:D_NO:FENCE_01,	金網:01, 250, -1500, -232875, 1024 }
//					{ d:D_NO:FENCE_02,	金網:02, 500, -1500, -234125, 0 }
//					{ d:D_NO:FENCE_03,	金網:03, 1750, -1500, -232875, 1024 }
//					{ d:D_NO:FENCE_04,	金網:04, 7250, -1000, -234000, 1024 }
					{ d:D_NO:FENCE_05,	金網:05, -4500, 250, -235500, 0 }
					{ d:D_NO:FENCE_06,	金網:06, -4250, 250, -237250, 0 }
					{ d:D_NO:FENCE_07,	金網:07, -1000, 250, -237500, 0 }
					{ d:D_NO:FENCE_08,	金網:08, -5000, 2000, -234250, 3072 }
//					{ d:D_NO:FENCE_09,	金網:09, -2250, 1000, -233250, 1024 }
//					{ d:D_NO:FENCE_10,	金網:10, -2000, 1000, -234500, 0 }
//					{ d:D_NO:FENCE_11,	金網:11, -750, 1000, -233250, 1024 }
					{ d:D_NO:FENCE_12,	金網:12, -2000, 1000, -233000, 0 }
//					{ d:D_NO:FENCE_13,	金網:13, 2250, 250, -233250, 1024 }
					{ d:D_NO:FENCE_14,	金網:14, 2500, 250, -234500, 0 }
					{ d:D_NO:FENCE_15,	金網:15, 3750, 250, -233250, 1024 }
					{ d:D_NO:FENCE_16,	金網:16, -2250, 1250, -236250, 1024 }
//					{ d:D_NO:FENCE_17,	金網:17, -2000, 1250, -237500, 0 }
//					{ d:D_NO:FENCE_18,	金網:18, 2250, 1250, -237750, 1024 }
//					{ d:D_NO:FENCE_19,	金網:19, 3750, 1250, -237750, 1024 }
					{ d:D_NO:FENCE_20,	金網:20, 2500, 1250, -237500, 0 }
//					{ d:D_NO:FENCE_21,	金網:21, -7500, 2000, -238000, 0 }
					{ d:D_NO:FENCE_22,	金網:22, -5750, 2000, -239375, 1024 }
				} \
				-exec { \
					// 壊れていたらロード回数を数えて再発生
					@壊れ物再セットチェック $b:w31a_破壊ロード回数[$1]
					if ( $f:再セットフラグ == d:TRUE ) {
						eval( $b:w31a_破壊フラグ[$1] = 0 );
						chara プット壊れ通気口 $2 \
							-model w31a_duct_fence0 w31a_duct_fence1 w31a_duct_fence2  \
							-pos   $3, $4, $5 \
							-rot   0, $6, 0 \
							-life  2 \
							-proc  ダクト金網状態保存設定

						// 健全な状態だったら主観設定を入れる
						if ( $2 == 金網:05 ) {
							command トラップ切り替え -trap fence_s:02 -switch 1
						} else if ( $2 == 金網:06 ) {
							command トラップ切り替え -trap fence_n -switch 1
						} else if ( $2 == 金網:07 ) {
							command トラップ切り替え -trap fence_s:01 -switch 1
						}

					} else {
						chara プット壊れ通気口 $2 \
							-model w31a_duct_fence0 w31a_duct_fence1 w31a_duct_fence2  \
							-pos   $3, $4, $5 \
							-rot   0, $6, 0 \
							-life  2 \
							-proc  ダクト金網状態保存設定 \
							-state $b:w31a_破壊フラグ[$1]

					}
				}
		} else if ( $w:ゲーム設定 == d:LEVEL_HARD ) {
			command foreach -argc 6 -repeat 6 \
				-data { \
//					{ d:D_NO:FENCE_01,	金網:01, 250, -1500, -232875, 1024 }
//					{ d:D_NO:FENCE_02,	金網:02, 500, -1500, -234125, 0 }
					{ d:D_NO:FENCE_03,	金網:03, 1750, -1500, -232875, 1024 }
//					{ d:D_NO:FENCE_04,	金網:04, 7250, -1000, -234000, 1024 }
//					{ d:D_NO:FENCE_05,	金網:05, -4500, 250, -235500, 0 }
					{ d:D_NO:FENCE_06,	金網:06, -4250, 250, -237250, 0 }
//					{ d:D_NO:FENCE_07,	金網:07, -1000, 250, -237500, 0 }
//					{ d:D_NO:FENCE_08,	金網:08, -5000, 2000, -234250, 3072 }
//					{ d:D_NO:FENCE_09,	金網:09, -2250, 1000, -233250, 1024 }
//					{ d:D_NO:FENCE_10,	金網:10, -2000, 1000, -234500, 0 }
//					{ d:D_NO:FENCE_11,	金網:11, -750, 1000, -233250, 1024 }
//					{ d:D_NO:FENCE_12,	金網:12, -2000, 1000, -233000, 0 }
//					{ d:D_NO:FENCE_13,	金網:13, 2250, 250, -233250, 1024 }
					{ d:D_NO:FENCE_14,	金網:14, 2500, 250, -234500, 0 }
					{ d:D_NO:FENCE_15,	金網:15, 3750, 250, -233250, 1024 }
					{ d:D_NO:FENCE_16,	金網:16, -2250, 1250, -236250, 1024 }
//					{ d:D_NO:FENCE_17,	金網:17, -2000, 1250, -237500, 0 }
//					{ d:D_NO:FENCE_18,	金網:18, 2250, 1250, -237750, 1024 }
//					{ d:D_NO:FENCE_19,	金網:19, 3750, 1250, -237750, 1024 }
//					{ d:D_NO:FENCE_20,	金網:20, 2500, 1250, -237500, 0 }
					{ d:D_NO:FENCE_21,	金網:21, -7500, 2000, -238000, 0 }
//					{ d:D_NO:FENCE_22,	金網:22, -5750, 2000, -239375, 1024 }
				} \
				-exec { \
					// 壊れていたらロード回数を数えて再発生
					@壊れ物再セットチェック $b:w31a_破壊ロード回数[$1]
					if ( $f:再セットフラグ == d:TRUE ) {
						eval( $b:w31a_破壊フラグ[$1] = 0 );
						chara プット壊れ通気口 $2 \
							-model w31a_duct_fence0 w31a_duct_fence1 w31a_duct_fence2  \
							-pos   $3, $4, $5 \
							-rot   0, $6, 0 \
							-life  3 \
							-proc  ダクト金網状態保存設定

						// 健全な状態だったら主観設定を入れる
						if ( $2 == 金網:05 ) {
							command トラップ切り替え -trap fence_s:02 -switch 1
						} else if ( $2 == 金網:06 ) {
							command トラップ切り替え -trap fence_n -switch 1
						} else if ( $2 == 金網:07 ) {
							command トラップ切り替え -trap fence_s:01 -switch 1
						}

					} else {
						chara プット壊れ通気口 $2 \
							-model w31a_duct_fence0 w31a_duct_fence1 w31a_duct_fence2  \
							-pos   $3, $4, $5 \
							-rot   0, $6, 0 \
							-life  3 \
							-proc  ダクト金網状態保存設定 \
							-state $b:w31a_破壊フラグ[$1]

					}
				}
		} else {
			command foreach -argc 6 -repeat 13 \
				-data { \
					{ d:D_NO:FENCE_01,	金網:01, 250, -1500, -232875, 1024 }
					{ d:D_NO:FENCE_02,	金網:02, 500, -1500, -234125, 0 }
					{ d:D_NO:FENCE_03,	金網:03, 1750, -1500, -232875, 1024 }
					{ d:D_NO:FENCE_04,	金網:04, 7250, -1000, -234000, 1024 }
					{ d:D_NO:FENCE_05,	金網:05, -4500, 250, -235500, 0 }
					{ d:D_NO:FENCE_06,	金網:06, -4250, 250, -237250, 0 }
					{ d:D_NO:FENCE_07,	金網:07, -1000, 250, -237500, 0 }
//					{ d:D_NO:FENCE_08,	金網:08, -5000, 2000, -234250, 3072 }
//					{ d:D_NO:FENCE_09,	金網:09, -2250, 1000, -233250, 1024 }
//					{ d:D_NO:FENCE_10,	金網:10, -2000, 1000, -234500, 0 }
//					{ d:D_NO:FENCE_11,	金網:11, -750, 1000, -233250, 1024 }
//					{ d:D_NO:FENCE_12,	金網:12, -2000, 1000, -233000, 0 }
//					{ d:D_NO:FENCE_13,	金網:13, 2250, 250, -233250, 1024 }
					{ d:D_NO:FENCE_14,	金網:14, 2500, 250, -234500, 0 }
					{ d:D_NO:FENCE_15,	金網:15, 3750, 250, -233250, 1024 }
//					{ d:D_NO:FENCE_16,	金網:16, -2250, 1250, -236250, 1024 }
					{ d:D_NO:FENCE_17,	金網:17, -2000, 1250, -237500, 0 }
					{ d:D_NO:FENCE_18,	金網:18, 2250, 1250, -237750, 1024 }
//					{ d:D_NO:FENCE_19,	金網:19, 3750, 1250, -237750, 1024 }
					{ d:D_NO:FENCE_20,	金網:20, 2500, 1250, -237500, 0 }
					{ d:D_NO:FENCE_21,	金網:21, -7500, 2000, -238000, 0 }
//					{ d:D_NO:FENCE_22,	金網:22, -5750, 2000, -239375, 1024 }
				} \
				-exec { \
					// 壊れていたらロード回数を数えて再発生
					@壊れ物再セットチェック $b:w31a_破壊ロード回数[$1]
					if ( $f:再セットフラグ == d:TRUE ) {
						eval( $b:w31a_破壊フラグ[$1] = 0 );
						chara プット壊れ通気口 $2 \
							-model w31a_duct_fence0 w31a_duct_fence1 w31a_duct_fence2  \
							-pos   $3, $4, $5 \
							-rot   0, $6, 0 \
							-life  5 \
							-proc  ダクト金網状態保存設定

						// 健全な状態だったら主観設定を入れる
						if ( $2 == 金網:05 ) {
							command トラップ切り替え -trap fence_s:02 -switch 1
						} else if ( $2 == 金網:06 ) {
							command トラップ切り替え -trap fence_n -switch 1
						} else if ( $2 == 金網:07 ) {
							command トラップ切り替え -trap fence_s:01 -switch 1
						}

					} else {
						chara プット壊れ通気口 $2 \
							-model w31a_duct_fence0 w31a_duct_fence1 w31a_duct_fence2  \
							-pos   $3, $4, $5 \
							-rot   0, $6, 0 \
							-life  5 \
							-proc  ダクト金網状態保存設定 \
							-state $b:w31a_破壊フラグ[$1]

					}
				}
		}
	}

	proc ダクト金網状態保存設定 {
		#if d:DEBUG_PRINT
			print 'duct_broken_check'
		#endif

		if ( $1 == 金網:01 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_01] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_01] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:02 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_02] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_02] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:03 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_03] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_03] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:04 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_04] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_04] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:05 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_05] = $2 );
			// 状態が変化したら主観はなし
			command トラップ切り替え -trap fence_s:02 -switch 0
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_05] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:06 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_06] = $2 );	
			// 状態が変化したら主観はなし
			command トラップ切り替え -trap fence_n -switch 0
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_06] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:07 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_07] = $2 );	
			// 状態が変化したら主観はなし
			command トラップ切り替え -trap fence_s:01 -switch 0
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_07] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:08 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_08] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_08] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:09 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_09] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_09] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:10 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_10] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_10] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:11 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_11] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_11] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:12 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_12] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_12] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:13 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_13] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_13] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:14 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_14] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_14] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:15 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_15] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_15] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:16 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_16] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_16] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:17 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_17] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_17] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:18 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_18] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_18] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:19 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_19] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_19] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:20 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_20] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_20] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:21 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_21] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_21] = $w:グローバルロード回数);	
			}
		} else if ( $1 == 金網:22 ) {
			eval( $b:w31a_破壊フラグ[d:D_NO:FENCE_22] = $2 );	
			if ( $2 >= 1 ) {
				// 完全に破壊されたらロード回数を保存
				eval( $b:w31a_破壊ロード回数[d:D_NO:FENCE_22] = $w:グローバルロード回数);	
			}
		}
	}

	// イントルード金網設定
	// ------------------------------------
	proc イントルード金網設定 {
		#if d:DEBUG_PRINT
			print 'w31a_intrude_fence_set'
		#endif

		trap fence_s d:PLAYER \
			-mask ＊ \
			-dir 0 256 \	//方向指定
			-exec {
				command イントルード主観カメラ補正 \
					-se d:se_code:SD_P_HEADNET1 \
					-len 800 \
					-inout $3
			}

		trap fence_n d:PLAYER \
			-mask ＊ \
			-dir 2048 256 \	//方向指定
			-exec {
				command イントルード主観カメラ補正 \
					-se d:se_code:SD_P_HEADNET1 \
					-len 800 \
					-inout $3
			}
	}

	// アイテム
	// ------------------------------------
	#ifndef d:ANOTHER
	// アナザーミッションでのアイテムは別ファイルで設定するためdefineで切り分ける
		proc アイテム取得時番号設定 $:ボックス名 {
			#if d:DEBUG_PRINT
				print 'item_no_set'
			#endif

			if ( $:ボックス名 == Ｍ９弾:01 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:M9_R01] = $w:グローバルロード回数);	
			} else if ( $:ボックス名 == Ｍ９弾:02 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:M9_R02] = $w:グローバルロード回数);	

			} else if ( $:ボックス名 == ソコム弾:01 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:SOCOM_R01] = $w:グローバルロード回数);	
			} else if ( $:ボックス名 == ソコム弾:02 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:SOCOM_R02] = $w:グローバルロード回数);	
			} else if ( $:ボックス名 == ソコム弾:03 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:SOCOM_R03] = $w:グローバルロード回数);	
			} else if ( $:ボックス名 == ソコム弾:04 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:SOCOM_R04] = $w:グローバルロード回数);	

			} else if ( $:ボックス名 == Ｍ４弾:01 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:M4_R01] = $w:グローバルロード回数);	
			} else if ( $:ボックス名 == Ｍ４弾:02 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:M4_R02] = $w:グローバルロード回数);	

			} else if ( $:ボックス名 == ＡＫＳ弾:01 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:AKS_R01] = $w:グローバルロード回数);	
			} else if ( $:ボックス名 == ＡＫＳ弾:02 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:AKS_R02] = $w:グローバルロード回数);	

			} else if ( $:ボックス名 == ニキータ弾:01 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:NIKITA_R01] = $w:グローバルロード回数);	
				eval( $f:w31a_アイテム再セット１ = 0 );
			} else if ( $:ボックス名 == ニキータ弾:02 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:NIKITA_R02] = $w:グローバルロード回数);	
				eval( $f:w31a_アイテム再セット２ = 0 );

			} else if ( $:ボックス名 == ＰＳＧ１弾:01 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:PSG1_R01] = $w:グローバルロード回数);	

			} else if ( $:ボックス名 == ＰＳＧ１－Ｔ弾:01 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:PSG1T_R01] = $w:グローバルロード回数);	

			} else if ( $:ボックス名 == ＲＧＢ６弾:01 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:RGB6_R01] = $w:グローバルロード回数);	

			} else if ( $:ボックス名 == スティンガー弾:01 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:STINGER_R01] = $w:グローバルロード回数);	

			} else if ( $:ボックス名 == チャフ:01 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:CHAFF_01] = $w:グローバルロード回数);	

			} else if ( $:ボックス名 == Ｃ４:01 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:C4_01] = $w:グローバルロード回数);	

			} else if ( $:ボックス名 == クレイモア:01) {
				eval($b:w31a_取得ロード回数[d:I_NO:CLAY_01] = $w:グローバルロード回数);	

			} else if ( $:ボックス名 == 雑誌:01 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:MAG_01] = $w:グローバルロード回数);	

			} else if ( $:ボックス名 == レーション:01 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:RATION_01] = $w:グローバルロード回数);	
			} else if ( $:ボックス名 == レーション:02 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:RATION_02] = $w:グローバルロード回数);	

			} else if ( $:ボックス名 == ジアゼパム:01 ) {
				eval($b:w31a_取得ロード回数[d:I_NO:DIAZE_01] = $w:グローバルロード回数);	

			}

			#ifdef d:DEBUG_PRINT
				print $:ボックス名
				print $w:グローバルロード回数
			#endif
		}
	#endif

	proc アイテム設置 {
		#if d:DEBUG_PRINT
			print 'item_set'
		#endif

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:M9_R01]
		if ($f:再セットフラグ) {
			@弾薬_Ｍ９_初期設置 Ｍ９弾:01 -6500 -2000 -228500 0			// 南西階段脇
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:SOCOM_R01]
		if ($f:再セットフラグ) {
			@弾薬_ソコム_初期設置 ソコム弾:01 625 0 -246250 0			// 監禁部屋北
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:SOCOM_R02]
		if ($f:再セットフラグ) {
			@弾薬_ソコム_初期設置 ソコム弾:02 3000 -1250 -228500 0		// 南荷物上
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:M4_R01]
		if ($f:再セットフラグ) {
			@弾薬_Ｍ４_初期設置 Ｍ４弾:01 8500 0 -252000 0				// 北西通路奥
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:M4_R02]
		if ($f:再セットフラグ) {
			@弾薬_Ｍ４_初期設置 Ｍ４弾:02 -4500 0 -243750 0				// 西東西通路上
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:NIKITA_R01]
		if ($f:再セットフラグ) {
			@弾薬_ニキータ_初期設置 ニキータ弾:01 -5500 750 -231750 0	// ガンカメ部屋
			eval( $f:w31a_アイテム再セット１ = 1 );
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:NIKITA_R02]
		if ($f:再セットフラグ) {
			@弾薬_ニキータ_初期設置 ニキータ弾:02 6500 -2000 -228500 0	// 南東階段脇
			eval( $f:w31a_アイテム再セット２ = 1 );
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:STINGER_R01]
		if ($f:再セットフラグ) {
			@弾薬_スティンガー_初期設置 スティンガー弾:01 -2000 0 -238250 0	// ダクトの中
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:CHAFF_01]
		if ($f:再セットフラグ) {
			@チャフ_初期設置 チャフ:01 -18000 0 -236000 0				// 西東西通路上
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:MAG_01]
		if ($f:再セットフラグ) {
			@雑誌_初期設置 雑誌:01 -3750 0 -237750 0				// ダクトの中
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:RATION_01]
		if ($f:再セットフラグ) {
			@レーション_初期設置 レーション:01 -12500 0 -231500 0		// 南西通路奥
		}

		// ＲＧＢ６は持ってないときは無条件に設置(ロード回数は関係なし)
		@ＲＧＢ６_初期設置 ＲＧＢ６本体 6500, 0, -252000 0 なし

		// Ｍ４は持ってないときは無条件に設置(ロード回数は関係なし)
		@Ｍ４_初期設置 Ｍ４本体 -18000, 0, -234000 0 なし

		// ニキータイベント用再セットチェック
		trap ev300 d:PLAYER \
			-mask いる \
			-exec {
				if ( ($w:武器弾数Ｒ[d:武器:ニキータ] <= 0) \
					&& (`command カメラ視界チェック -camera 0x01 \
							-bound (-5500-1000) (750) (-231750-1000) \
							       (-5500+1000) (1750) (-231750+1000)` == 0)){
					if ( $f:w31a_アイテム再セット１ == 0 ) {
						@弾薬_ニキータ_初期設置 ニキータ弾:01 -5500 750 -231750 0	// ガンカメ部屋
						eval( $f:w31a_アイテム再セット１ = 1 );
					}
				}

				if ( ($w:武器弾数Ｒ[d:武器:ニキータ] <= 0) \
					&& (`command カメラ視界チェック -camera 0x01 \
							-bound (6500-1000) (-2000) (-228500-1000) \
							       (6500+1000) (-1000) (-228500+1000)` == 0)){
					if ( $f:w31a_アイテム再セット２ == 0 ) {
						@弾薬_ニキータ_初期設置 ニキータ弾:02 6500 -2000 -228500 0	// 南東階段脇
						eval( $f:w31a_アイテム再セット２ = 1 );
					}
				}
			}

	}

	proc エマイベントアイテム設置 {
		#if d:DEBUG_PRINT
			print 'item_set'
		#endif
		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:M9_R02]
		if ($f:再セットフラグ) {
			@弾薬_Ｍ９_初期設置 Ｍ９弾:02 -2000 0 -245500 0				// 監禁部屋
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:SOCOM_R03]
		if ($f:再セットフラグ) {
			@弾薬_ソコム_初期設置 ソコム弾:03 -11500 0 -252000 0		// 北西通路奥
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:SOCOM_R04]
		if ($f:再セットフラグ) {
			@弾薬_ソコム_初期設置 ソコム弾:04 -18000 0 -234000 0		// ＨＩ連絡橋扉前
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:AKS_R01]
		if ($f:再セットフラグ) {
			@弾薬_ＡＫＳ_初期設置 ＡＫＳ弾:01 8500 0 -252000 0			// 北東通路奥
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:AKS_R02]
		if ($f:再セットフラグ) {
			@弾薬_ＡＫＳ_初期設置 ＡＫＳ弾:02 7500 0 -243500 0			// 監禁部屋
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:PSG1_R01]
		if ($f:再セットフラグ) {
			@弾薬_ＰＳＧ１_初期設置 ＰＳＧ１弾:01 4750 0 -244500 0		// 監禁部屋
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:PSG1T_R01]
		if ($f:再セットフラグ) {
			@弾薬_ＰＳＧ１－Ｔ_初期設置 ＰＳＧ１－Ｔ弾:01 -2000 0 -238250 0		// ダクトの中
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:RGB6_R01]
		if ($f:再セットフラグ) {
			@弾薬_ＲＧＢ６_初期設置 ＲＧＢ６弾:01 -3750 0 -237750 0		// ダクトの中
		}
/* 監督指示によりはずす 2001.08.22
		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:C4_01]
		if ($f:再セットフラグ) {
			@Ｃ４_初期設置 Ｃ４:01 -7500 0 -247250 0			// 休憩室
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:CLAY_01]
		if ($f:再セットフラグ) {
			@クレイモア_初期設置 クレイモア:01 -3500 0 -244500 0		// 休憩室
		}
*/
		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:DIAZE_01]
		if ($f:再セットフラグ) {
			@ジアゼパム_初期設置 ジアゼパム:01 -12500 0 -226500 0		// 南西通路奥
		}

		@アイテム再セットチェック $b:w31a_取得ロード回数[d:I_NO:RATION_02]
		if ($f:再セットフラグ) {
			@レーション_初期設置 レーション:02 -500 0 -243500 0		// 監禁部屋
		}
	}


	// フォグ
	// ------------------------------------
	proc フォグ設定 {
		#if d:DEBUG_PRINT
			print 'fog_set'
		#endif

		chara フォグ 霧\
			-c 34, 53, 50 \
			-n 0 \
			-f 50000 
	}

	// キャラ用光源(マップの設置が終わってから設定することで、キャラだけに光を当てる)
	// ------------------------------------
	proc キャラ用ライト設置 {
		#if d:DEBUG_PRINT
			print 'chara_light_set'
		#endif

		chara 環境光 環境光源 \
			 -c 40 67 62

		chara 平行光 平行光源 \
			 -c 51 56 45 \
			 -p 1409 -1289 -4621
	}

	// 子画面設定 
	// ------------------------------------
	proc 子画面設定 {
		#if d:DEBUG_PRINT
			print 'sub_camera_set'
		#endif

		chara サブ画面 子画面 \
			-w (512-d:SUB_MARGIN-d:SUB_WIDTH),d:SUB_MARGIN (512-d:SUB_MARGIN),(d:SUB_HEIGHT+d:SUB_MARGIN)
	}

	// ステージカメラ設定 
	// ------------------------------------
	proc カメラセット {
		#if d:DEBUG_PRINT
			print 'stage_camera_set'
		#endif

		chara カメラ ステージカメラ		-c 0 -l 3 -p 0 
		chara カメラ メインカメラ		-c 0 -l 2 -p 50
		chara カメラ サブカメラ１		-c 0 -l 2 -p 40
		chara カメラ サブカメラ２		-c 0 -l 2 -p 30
		chara カメラ モーションカメラ	-c 0 -l 2 -p 20 
		chara カメラ シナリオデモカメラ	-c 0 -l 0 -p 1 
		#ifdef d:STAGE_W31D
			chara カメラ クリアリングカメラ		-c 1 -l 2 -p 0
		#endif

		// デフォルトカメラ
		chara カメラ設定 ステージカメラ \
			-c 0 \	
			-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
			-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
			-r 750,2048,0 \
			-f 6500 \
			-a 200 \
			-i 2 2 0 0 \
			-s 1
	}


	// ステージ開始時カメラ 
	// ------------------------------------
	proc スタートカメラセット {
		#if d:DEBUG_PRINT
			print 'start_camera_set'
		#endif

		if( $s:登場ポイント == sp_w25c_w31a_0 ) {
			// ＫＬ連絡橋から
			chara カメラ設定 モーションカメラ \
				-c 1 \
				-p 17128,4504,-233651 -t 18150,1699,-235327 \
				-r 626,1691,0 -f 3424 \
				-a 200 \
				-i 0 2 0 0 \
				-s 1
		}
	}

	proc スタートカメラオフ {
		#if d:DEBUG_PRINT
			print 'start_camera_off'
		#endif

		chara カメラ設定 モーションカメラ -s -1
	}

	// ステージ終了時カメラ 
	// ------------------------------------
	proc エンドカメラセット $:エンドポイント {
		#if d:DEBUG_PRINT
			print 'end_camera_set'
		#endif

		if( $:エンドポイント == ＫＬ連絡橋 ) {
			chara カメラ設定 モーションカメラ \
				-c 1 \
				-p 17128,4504,-233651 -t 18150,1699,-235327 \
				-r 626,1691,0 -f 3424 \
				-a 200 \
				-i 0 2 0 0 \
				-s 1
		}
	}


// 各マップ別設定
// ------------------------------------
// シェル２中央棟１Ｆ
	// 天井設置
	// ------------------------------------
	proc シェル２中央棟１Ｆ天井設置 {
		#if d:DEBUG_PRINT
			print 'w31a_ceil_set'
		#endif

		if($s:d_num == not_demo) {
			chara 天井君 シェル２中央棟１Ｆ天井 -m d:CEIL_NAME0 -h 3750 -p 0,0,0 -l d:LT2_NAME
		} else {
			chara プットオブジェ シェル２中央棟１Ｆ天井 -m d:CEIL_NAME0 -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME
		}
	}

	proc シェル２中央棟１Ｆ南壁設置 {
		#if d:DEBUG_PRINT
			print 'w31a_wall_set'
		#endif

		if($s:d_num == not_demo) {
			chara 天井君 シェル２中央棟１Ｆ南壁 -m d:WALL_NAME0 -p 0,0,0 -c 0,0,-226000 -a 5 -l d:LT2_NAME
		} else {
			chara プットオブジェ シェル２中央棟１Ｆ南壁 -m d:WALL_NAME0 -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME
		}
	}

	// パイプ部屋のパイプオンオフ
	// ------------------------------------
	proc パイプ部屋パイプ設置 {
		#if d:DEBUG_PRINT
			print 'w31a_pipe on_off_set'
		#endif

		chara プットオブジェ 通常パイプ -m d:KMS_NAME2 -r 0,0,0 -p -6923,701,-236032 -s 100,100,100 -l d:LT2_NAME \
			-f ( d:PUT_OBJ_CALC_LIGHT | d:PUT_OBJ_THERMAL | d:PUT_OBJ_STATUS_NV )
	}

	// 強制モーション用トラップ
	// ------------------------------------
	// ジャンプ設定
	// ------------------------------------
	proc シェル２中央棟１Ｆジャンプ設定 {
		#if d:DEBUG_PRINT
			print 'w31a_jump_set'
		#endif

		trap ky001 d:PLAYER \
			-mask 入る \
			-exec {
				@ジャンプ発動 ky001 1000 10
			}
	}

	// 各種カメラの設定
	// ------------------------------------
	proc シェル２中央棟１Ｆカメラ設定 {
		#if d:DEBUG_PRINT
			print 'w31a_camera_set'
		#endif

		// エレベーター
		trap ca001 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 1 \	
					-p 0,6672,-254430 -t 0,2949,-254976 \
					-r 929,2048,0 -f 3763 \
					-a 200 \
					-i 0 0 0 0 \
					-s $3
			}

		// エレベーター前通路
		trap ca002 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {
				if ( $4 < -2500 ) {
					@CS_スムーズカメラ昇順 $4 -8000 -2500 -12000 -525
					eval( $i:補完ＢＸ = $i:CS_結果１ );
					eval( $i:補完ＬＸ = $i:CS_結果１ );
					eval( $i:補完ＢＸ２ = 525 );
					eval( $i:補完ＬＸ２ = 525 );

					eval( $i:補完ＬＺ = -250253 );
				} else if ( $4 > -2500 && $4 < 2500 ) {
					eval( $i:補完ＢＸ = -525 );
					eval( $i:補完ＬＸ = -525 );
					eval( $i:補完ＢＸ２ = 525 );
					eval( $i:補完ＬＸ２ = 525 );

					eval( $i:補完ＬＺ = -250858 );
				} else {
					@CS_スムーズカメラ昇順 $4 2500 6000 525 7000
					eval( $i:補完ＢＸ = -525 );
					eval( $i:補完ＬＸ = -525 );
					eval( $i:補完ＢＸ２ = $i:CS_結果１ );
					eval( $i:補完ＬＸ２ = $i:CS_結果１ );

					eval( $i:補完ＬＺ = -250253 );
				}

				chara カメラ設定 メインカメラ \
					-c 0 \	
					-b $i:補完ＢＸ,6572,-248366 $i:補完ＢＸ２,6572,-248450 \
					-l $i:補完ＬＸ,2078,$i:補完ＬＺ $i:補完ＬＸ２,2078,-249370 \
					-r 875,2048,0 \
					-f 5493 \
					-a 200 \
					-i 0 0 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// ノード部屋前南北通路
		trap ca003 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {
				@CS_スムーズカメラ降順 $4 -8000 -9125 -9000 -10250
				eval( $i:補完ＢＸ = $i:CS_結果１ );
				eval( $i:補完ＬＸ = $i:CS_結果１ );

				if ( $6 > -232076 ) {
					@CS_スムーズカメラ昇順 $6 -232076 -227000 5000 6906
					eval( $i:補完ＢＹ = $i:CS_結果１ );
					@CS_スムーズカメラ昇順 $6 -232076 -225500 530 771
					eval( $i:補完ＲＸ = $i:CS_結果１ );
					@CS_スムーズカメラ昇順 $6 -232076 -235500 5100 5100
					eval( $w:補完距離 = $i:CS_結果１ );
				} else {
					@CS_スムーズカメラ昇順 $6 -236000 -232076 3849 5000
					eval( $i:補完ＢＹ = $i:CS_結果１ );
					@CS_スムーズカメラ昇順 $6 -239000 -232076 520 530
					eval( $i:補完ＲＸ = $i:CS_結果１ );
					@CS_スムーズカメラ昇順 $6 -237000 -232076 8500 5100
					eval( $w:補完距離 = $i:CS_結果１ );
				}

				chara カメラ設定 メインカメラ \
					-c 0 \	
					-b -10250,$i:補完ＢＹ,d:CAM_MIN $i:補完ＢＸ,$i:補完ＢＹ,-226405 \
					-l -10250,1500,d:CAM_MIN $i:補完ＬＸ,1500,-227699 \
					-r $i:補完ＲＸ,2048,0 \
					-f $w:補完距離 \
					-a 200 \
					-i 0 -1 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// 監禁室前南北通路
		trap ca004 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {
				@CS_スムーズカメラ昇順 $4 8000 9125 9000 10250
				eval( $i:補完ＢＸ = $i:CS_結果１ );
				eval( $i:補完ＬＸ = $i:CS_結果１ );

				if ( $6 > -232076 ) {
					@CS_スムーズカメラ昇順 $6 -232076 -227000 5000 6906
					eval( $i:補完ＢＹ = $i:CS_結果１ );
					@CS_スムーズカメラ昇順 $6 -232076 -225500 530 771
					eval( $i:補完ＲＸ = $i:CS_結果１ );
					@CS_スムーズカメラ昇順 $6 -232076 -235500 5100 5100
					eval( $w:補完距離 = $i:CS_結果１ );
				} else {
					@CS_スムーズカメラ昇順 $6 -236000 -232076 3849 5000
					eval( $i:補完ＢＹ = $i:CS_結果１ );
					@CS_スムーズカメラ昇順 $6 -239000 -232076 520 530
					eval( $i:補完ＲＸ = $i:CS_結果１ );
					@CS_スムーズカメラ昇順 $6 -237000 -232076 8500 5100
					eval( $w:補完距離 = $i:CS_結果１ );
				}

				chara カメラ設定 メインカメラ \
					-c 0 \	
					-b $i:補完ＢＸ,$i:補完ＢＹ,d:CAM_MIN 10250,$i:補完ＢＹ,-226405 \
					-l $i:補完ＬＸ,1500,d:CAM_MIN 10250,1500,-227699 \
					-r $i:補完ＲＸ,2048,0 \
					-f $w:補完距離 \
					-a 200 \
					-i 0 -1 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// ＨＩ連絡橋出口前東西通路
		trap ca005 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ２ \
					-c 0 \	
					-b -17435,6215,-233893 -13760,6215,-233893 \
					-l -17435,1838,-235000 -13760,d:CAM_MAX,-235000 \
					-r 875,2048,0 \
					-f 4500 \
					-a 200 \
					-i 0 0 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// ＫＬ連絡橋出口前東西通路
		trap ca006 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ２ \
					-c 0 \	
					-b 13760,6215,-233893 17435,6215,-233893 \
					-l 13760,1838,-235000 17435,d:CAM_MAX,-235000 \
					-r 875,2048,0 \
					-f 4500 \
					-a 200 \
					-i 0 0 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// 監禁室前東西通路
		trap ca007 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 0 \	
					-b 5500,6587,-239902 10000,7000,-239902 \
					-l 5500,1721,-241034 10000,d:CAM_MAX,-241034 \
					-r 875,2048,0 \
					-f 5000 \
					-a 200 \
					-i 0 0 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// 南側東西通路
		trap ca008 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 0 \	
					-b -4323,2000,-224701 4323,5150,-223971 \
					-l -4323,1250,-227729 4323,d:CAM_MAX,-226930 \
					-r 600,2048,0 \
					-f 5000 \
					-a 200 \
					-i 2 -1 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// 南側東西通路階段左
		trap ca009 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				@CS_スムーズカメラ降順 $4 -6000 -8000 -6000 -8500
				eval( $i:補完ＢＸ = $i:CS_結果１ );
				eval( $i:補完ＬＸ = $i:CS_結果１ );

				chara カメラ設定 サブカメラ１ \
					-c 0 \	
					-b d:CAM_MIN,5445,-225815 $i:補完ＢＸ,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,1250,-228544 $i:補完ＬＸ,d:CAM_MAX,d:CAM_MAX \
					-r 600,2068,0 \
					-f 4500 \
					-a 200 \
					-i 3 3 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// 南側東西通路階段右
		trap ca010 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {
				@CS_スムーズカメラ昇順 $4 6000 8000 6000 8500
				eval( $i:補完ＢＸ = $i:CS_結果１ );
				eval( $i:補完ＬＸ = $i:CS_結果１ );

				chara カメラ設定 サブカメラ１ \
					-c 0 \	
					-b $i:補完ＢＸ,5445,-225815 d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l $i:補完ＬＸ,1250,-228544 d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 600,2028,0 \
					-f 4500 \
					-a 200 \
					-i 3 3 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// 休憩室
		trap ca011 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ１ \
					-c 0 \	
					-b -6422,7158,-242750 -5337,7158,-242560 \
					-l -6422,1903,-245432 -5337,d:CAM_MAX,-244357 \
					-r 692,2048,0 -f 5500 \
					-a 200 \
					-i 0 0 0 0 \
					-s $3
			}

		// 監禁室右
		trap ca012 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 0 \	
					-b -500,7500,-243517 6234,7500,-243517 \
					-l -500,2050,-245583 6234,2050,-244785 \
					-r 800,2048,0 \
					-f 5597 \
					-a 200 \
					-i 0 0 0 0 \
					-s $3
			}

		// 監禁室左
		trap ca013 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				#ifndef d:ANOTHER
				// 本編
					if ( $w:p_story < d:ST:P049_01_P01大統領１ポリゴンデモ１開始 ) {
						// 大統領イベント前はカメラを変えない
						chara カメラ設定 メインカメラ \
							-c 0 \	
							-b 5500,6587,-239902 10000,7000,-239902 \
							-l 5500,1721,-241034 10000,d:CAM_MAX,-241034 \
							-r 875,2048,0 \
							-f 5000 \
							-a 200 \
							-i 0 0 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s $3
					} else {
						// 大統領イベント後はこっち
						chara カメラ設定 メインカメラ \
							-c 0 \	
							-b 328,7510,-240905 986,7510,-240214 \
							-l 328,1798,-244285 986,1798,-241452 \
							-r 700,2048,0 \
							-f 6500 \
							-a 200 \
							-i 0 0 0 0 \
							-s $3
					}
				#else
				// アナザーミッション
					// 大統領イベント後はこっち
					chara カメラ設定 メインカメラ \
						-c 0 \	
						-b 328,7510,-240905 986,7510,-240214 \
						-l 328,1798,-244285 986,1798,-241452 \
						-r 700,2048,0 \
						-f 6500 \
						-a 200 \
						-i 0 0 0 0 \
						-s $3
				#endif
			}

		// 倉庫
		trap ca014 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ１ \
					-c 1 \	
					-p -7000,6785,-231521 -t -7000,564,-233356 \
					-r 837,2048,0 -f 6486 \
					-a 200 \
//					-p -6849,7151,-231508 -t -6849,927,-233354 \
//					-r 836,2048,0 -f 6492 \
//					-a 200 \
					-i 0 0 0 0 \
					-s $3
			}

		// 北側廊下部分の張り付きカメラ(電源パネルが破壊されるとトラップがオフされます)
		trap ca015 d:PLAYER \
			-mask ？ \
			-dir 2048 512 \
			-state d:TRP_STATE_CAUTION \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ２ \
					-c 0 \	
					-b 0,8725,-247509 5600,8725,-247509 \
					-l 0,1588,-247772 5600,1588,-247772 \
					-r 1000,2048,0 \
					-f 7142 \
					-a 200 \
					-i 3 3 0 0 \
					-s $3
			}

		// 休憩室部分の張り付きカメラ(電源パネルが破壊されるとトラップがオフされます)
		trap ca016 d:PLAYER \
			-mask ？ \
			-dir 3072 512 \
			-state d:TRP_STATE_CAUTION \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ２ \
					-c 0 \	
					-b -2891,8002,-242183 -2891,8002,-242183 \
					-l -2891,1623,-245046 -2891,1623,-245046 \
					-r 749,2048,0 \
					-f 6992 \
					-a 200 \
					-i 3 3 0 0 \
					-s $3
			}

		// 南側通路部分の張り付きカメラ
		trap ca017 d:PLAYER \
			-mask ？ \
			-dir 2048 512 \
			-state d:TRP_STATE_CAUTION \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ２ \
					-c 0 \	
					-b -4323,5320,-226599 4323,5320,-226599 \
					-l -4323,-271,-227082 4323,-271,-227082 \
					-r 900,2048,0 \
					-f 5731 \
					-a 200 \
					-i 0 0 0 0 \
					-s $3
			}

		// 倉庫側南北通路部分の匍匐カメラ
		trap ca018 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ１ \
					-c 1 \	
					-p -10500,6621,-232921 -t -10500,1700,-235102 \
					-r 752,2048,0 -f 5382 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}

		// 監禁部屋側南北通路部分の匍匐カメラ
		trap ca019 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ１ \
					-c 1 \	
					-p 10500,6621,-232921 -t 10500,1700,-235102 \
					-r 752,2048,0 -f 5382 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}

		// 倉庫側階段部分の匍匐カメラ
		trap ca020 d:PLAYER \
			-mask ＊ \
			-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
			-camera \
			-exec {
				@CS_スムーズカメラ降順 $4 -6000 -8000 -6000 -8500
				eval( $i:補完ＢＸ = $i:CS_結果１ );
				eval( $i:補完ＬＸ = $i:CS_結果１ );

				chara カメラ設定 サブカメラ２ \
					-c 0 \	
					-b d:CAM_MIN,5676,-227405 $i:補完ＢＸ,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,-813,-229953 $i:補完ＬＸ,-813,d:CAM_MAX \
					-r 780,2068,0 \
					-f 6972 \
					-a 200 \
					-i 3 3 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// 監禁部屋側階段部分の匍匐カメラ
		trap ca021 d:PLAYER \
			-mask ＊ \
			-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
			-camera \
			-exec {
				@CS_スムーズカメラ昇順 $4 6000 8000 6000 8500
				eval( $i:補完ＢＸ = $i:CS_結果１ );
				eval( $i:補完ＬＸ = $i:CS_結果１ );

				chara カメラ設定 サブカメラ２ \
					-c 0 \	
					-b $i:補完ＢＸ,5676,-227405 d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l $i:補完ＬＸ,-813,-229953 d:CAM_MAX,-813,d:CAM_MAX \
					-r 780,2028,0 \
					-f 6972 \
					-a 200 \
					-i 3 3 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// 南側東西通路荷物裏の匍匐カメラ
		trap ca022 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ２ \
					-c 0 \	
					-b 2011,2000,-228485 4323,5150,-223971 \
					-l 2011,-1072,-229975 4323,-1072,-226930 \
					-r 870,2048,0 \
					-f 6365 \
					-a 200 \
					-i 3 3 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// ダクト室上
		trap ca023 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 0 \	
					-b -6500,7786,-239302 -5000,7786,-239302 \
					-l -6500,d:CAM_MIN,d:CAM_MIN -5000,d:CAM_MAX,-240618 \
					-r 849,2048,0 \
					-f 4990 \
					-a 200 \
					-i 0 1 0 0 \
					-s $3
			}
	}








// FIXデータはここまで
// -----------------------------------------------------------------------------------












// 以下仮キャラ用proc
// -----------------------------------------------------------------------------------
// 仮キャラ
// ------------------------------------




















// 以下デバッグ専用proc
// -----------------------------------------------------------------------------------
// デバッグキャラ
// ------------------------------------
proc デバッグキャラセット {
	#if d:DEBUG_PRINT
		print 'debug_chara_set'
	#endif

	#if d:CHARA_CAMERA
		chara キャラカメラ 敵兵カメラ \
			-c d:TARGET_CHARA \
			-s -920,610,200 \
			-r -200,1024,0 
	#endif

	#if d:SCREENSHOT
		chara スクリーンショット shot
	#endif

	#if d:USB_KEY
		chara USBキーボード実行 test \
			-e d:KBD_ESC {	
				mesg プレイヤー d:PLAYER position -5500, -13000, -500
			} \
	#endif
}

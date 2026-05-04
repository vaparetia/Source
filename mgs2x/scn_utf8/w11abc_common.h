/*
	w11abc_common.h                     
		海底ドック共通プロック(w11a,w11b, w11cで使用)

	2001/04/17 H.Yoshiike         
	$Id: w11abc_common.h,v 1.80 2001/11/09 10:57:08 usr03682 Exp $                      

	
*/


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
		eval( $f:ＢＧＭマネージャー起動フラグ = 0 );

		#ifdef d:STAGE_W11A
			if ( $w:p_story < d:ST:P003_02_R02スネーク昇降機上昇２無線デモ１開始 ) {
				mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06
			}
		#elifdef d:STAGE_W11B
			if ( $w:p_story < d:ST:P026_01_R01爆弾解体終了１無線デモ１開始 ) {
				// 爆弾を解体するまでは爆弾曲を鳴らす
				mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_05
			} else {
				mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06
			}

		#elifdef d:STAGE_W11C
			// w11c-フォーチュン戦だけは特殊BGMを鳴らす
			mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_01
		#endif

		@サウンド効果音設定
		
	}

	// ローカル変数初期化
	// ------------------------------------
	proc ローカル変数初期化 {
		#if d:DEBUG_PRINT 
			print 'localvar_init'
		#endif

		eval( $f:rfp_w11_昇降機に乗れますよ = 0 );

		// 昇降機ホール揺れライトフラグ取得用
		eval( $i:ローカル変数Ｉ１ = 0 );

		#ifdef d:STAGE_W11A
			// フナ虫、オプション用
			eval( $f:ローカルフラグ１ = 0 );

			// パッドデモ説明用フラグ
			eval( $w:ローカル変数Ｗ１ = 0 );

			// クリアリング子画面チェック用
			eval( $f:クリアリング中フラグ = 0 );
			// クリアリングゲームステータス取得用
			eval( $i:ローカル変数Ｉ２ = 0 );
		#endif

		#ifdef d:STAGE_W11B
			// デバッグ用にストーリーフラグを勝手に進める
			#if d:DEBUG_MODE
				if ( $w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１終了 ) {
					eval ( $w:p_story = d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１終了 );
				}
			#endif
		#endif

		#ifdef d:STAGE_W11C
		// アイテム再セット用
			eval( $f:アイテム再セット１ = 0 );
			eval( $f:アイテム再セット２ = 0 );
		// フォーチュン戦時壊れ物カウント用変数を初期化
			eval ( $b:フォークリフト前面荷物破壊数 = 0 );
//			eval ( $b:フォークリフト周辺荷物破壊数 = 0 );
			eval ( $b:フォークリフト破壊カウント = 0 );
			eval ( $b:２ｍ破壊カウント = 0 );
		#endif
	}

	// 設定値を変更
	// ------------------------------------
	proc 設定値変更 {
		#if d:DEBUG_PRINT 
			print 'status_set'
		#endif

		#ifdef d:STAGE_W11A
		// w11aの時だけ敵兵の気絶値を低くする
			if ( $w:ゲーム設定 <= d:LEVEL_NORMAL ) {
				eval($w:敵標準気絶 = 3);
			}
		#endif
	}

	// プラグインプログラム
	// ------------------------------------
	proc プラグイン設定 {
		#if d:DEBUG_PRINT
			print 'plugin_set'
		#endif

		#if d:JP_MOTION
			command プラグイン自動ジャンプ
		#endif

		#ifdef d:STAGE_W11C
			command プラグイン強制吹っ飛ばし
		#endif
	}

	// プレイヤー 
	// ------------------------------------
	proc プレイヤー設定 {
		#if d:DEBUG_PRINT
			print 'player_set'
		#endif
		chara プレイヤー d:PLAYER \
			-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
//			-p -3460 , -45000, 250 \
			-d 0,$i:プレイヤー初期方向,0 \
			-s $b:プレイヤー初期姿勢 \
			-m rai_def -a raiden -o d:R_MT_FILE
//			-f d:PLY_NO_FOOT_SHADOW

		@プレイヤーＬＯＤ設定 d:C_DISTANCE_TYPE 4000

		#ifdef d:STAGE_W11A
			chara	Ｏ２ゲージ ライデンＯ２ -max $w:潜水服Ｏ２最大 -length (d:O2_GAGE*2)
		#else
			chara	Ｏ２ゲージ ライデンＯ２ -max $w:Ｏ２最大 -length d:O2_GAGE
		#endif
	
		// 髪の毛処理
		#ifdef d:STAGE_W11A
		// wllaは潜水服用
			chara マルチウェイト髪の毛モデル ライデン髪の毛 \
				-n rai_hair_diver_mh_mt \
				-b rai_diver_bounding \
				-d 15 \
				-y d:PLAYER \
				-z 11 12 \
				-x 0,0,0 \
				-o 100 \
				-q 0 \
				-f 1 \
				-c 2 { 2 11 }

			chara 潜水ゴーグル テスト -model raiden_gogle_sev
			
			// 潜水服状態ではカツラを使用不可にする
			command	不可装備設定 -item 	d:アイテム:無限カツラ, \
										d:アイテム:カツラＡ, \
										d:アイテム:カツラＢ, \
										d:アイテム:カツラＣ, \
										d:アイテム:カツラＤ

		#else
		// w11b,w11cは通常髪の毛プログラムを起動
			@ライデン髪の毛設定 d:NORMAL_HAIR
		#endif

		// フォーチュン戦中はソーコムを持たせる(M9は別)
		#ifdef d:STAGE_W11C
			@デモ後武器アイテム制御 d:デモ後銃装備 d:デモ後特殊装備なし
			mesg プレイヤー d:PLAYER stance d:FA_END_STAND 2048 2048 0
		#endif
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
		command 弾痕ノーマル -n 2	// 水床、通常
		command 弾痕ノーマル -n 3	// 鉄床、鉄筋
		command 弾痕ノーマル -n 4	// ロッカー、金網
		command 弾痕ノーマル -n 5	// 木床、木箱
		command 弾痕ノーマル -n 6	// ダンボール

		command 跳弾ノーマル -n 0
		command 跳弾ノーマル -n 1
		command 跳弾ノーマル -n 2
		command 跳弾ノーマル -n 3
		command 跳弾ノーマル -n 4
		command 跳弾ＳＥのみ -n 5
		command 跳弾ＳＥのみ -n 6

		#ifndef d:STAGE_W11C
			command 弾水飛沫処理
		#endif
	}


	// スライドドア
	// ------------------------------------
	proc ドア設置 {
		#if d:DEBUG_PRINT
			print 'slide_door_set'
		#endif

		#ifdef d:STAGE_W11C
		// w11c-フォーチュン戦は１マップ
			chara ドア ホールドア \
				-m d:DOOR_NAME1 \
				-d 0,3072,0 \
				-p -6625,-45000,2500 \
				-slide d:SD_SLIDE_INSIDE \
				-time d:SD_OPEN_TIME \
				-between 昇降機ホール 昇降機ホール \
				-exec {
				}
		#else
		// w11aとw11bは別マップつながり
			chara ドア ホールドア \
				-m d:DOOR_NAME1 \
				-d 0,3072,0 \
				-p -6625,-45000,2500 \
				-slide d:SD_SLIDE_INSIDE \
				-time d:SD_OPEN_TIME \
				-between 昇降機ホール 連結ハッチ \
				-exec {
					if ( $w:p_story < d:ST:P003_01_P01スネーク昇降機上昇１ポリゴンデモ１終了 || $w:p_story == d:ST:P028_01_P01爆弾解体後フォーチュン再登場１ポリゴンデモ１開始 ) {
						// スネークポリゴンデモ時＆フォーチュン再登場デモ時はずっとマップをＯＮにしておく
						mesg マップ接合 連結ハッチ／昇降機ホール on
					} else {
						if( $:開閉フラグ == 0 ){
							mesg マップ接合 連結ハッチ／昇降機ホール off

						} else {
							mesg マップ接合 連結ハッチ／昇降機ホール on

						}
					}
				}
			trap dr003 ？ \
				-mask ？ \	
				-exec {
						if($3==入る){
							mesg ドア ホールドア open
							#ifdef d:STAGE_W11A
							#ifndef d:DEMO_STAGE
							// ドアが開いたらスネーク上昇デモへ
								if ( $w:p_story < d:ST:P003_01_P01スネーク昇降機上昇１ポリゴンデモ１開始 ) {
									mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_FOUTS_S
									preseek 'd001p02'
									@ポリゴンデモロード d:ST:P003_01_P01スネーク昇降機上昇１ポリゴンデモ１開始
								}
							#endif
							#endif
						}else{
							mesg ドア ホールドア close
							// クリアリング中にマップをオフしたら子画面を消す
							if ( $2 == d:PLAYER ) {
								if ( $i:プレイヤー位置Ｘ > -6625 && $f:クリアリング中フラグ == 1 ) {
									#if d:SUB_CAMERA
										mesg サブ画面 子画面 off
									#endif
								} else if ( $f:クリアリング中フラグ == 1 ) {
									#if d:SUB_CAMERA
										mesg サブ画面 子画面 on
									#endif
								}
							}
						}
				}
		#endif

	}


	// アイテム
	// ------------------------------------
	// アイテム番号（アイテムが多いのでenumで）
#ifndef d:DEMO_STAGE
	enum I_NO {
		RATION_01 = 0,
		RATION_02,
		RATION_03,
		RATION_04,
		M9,
		M9_R01,
		M9_R02,
		SOCOM_R01,
		SOCOM_R02
	}
#endif

	proc アイテム取得時番号設定 $:ボックス名 {
		#if d:DEBUG_PRINT
			print 'item_no_set'
		#endif

		#ifndef d:STAGE_W11C
		// w11a w11b用
		if ( $:ボックス名 == レーション:01 ) {
			eval( $b:w11a_取得ロード回数[d:I_NO:RATION_01] = $w:グローバルロード回数 );
		} else if ( $:ボックス名 == レーション:02 ) {
			eval( $b:w11a_取得ロード回数[d:I_NO:RATION_02] = $w:グローバルロード回数 );
		} else if ( $:ボックス名 == レーション:03 ) {
			eval( $b:w11a_取得ロード回数[d:I_NO:RATION_03] = $w:グローバルロード回数 );
		} else if ( $:ボックス名 == Ｍ９本体 ) {
			eval( $b:w11a_取得ロード回数[d:I_NO:M9] = $w:グローバルロード回数 );
		} else if ( $:ボックス名 == Ｍ９弾:01 ) {
			eval( $b:w11a_取得ロード回数[d:I_NO:M9_R01] = $w:グローバルロード回数 );
		} else if ( $:ボックス名 == Ｍ９弾:02 ) {
			eval( $b:w11a_取得ロード回数[d:I_NO:M9_R02] = $w:グローバルロード回数 );
		}
		#endif 

		#ifdef d:STAGE_W11B
		// w11b用
		if ( $:ボックス名 == レーション:04 ) {
			eval( $b:w11a_取得ロード回数[d:I_NO:RATION_04] = $w:グローバルロード回数 );
			eval( $f:アイテム再セット１ = 0 );
		} else if ( $:ボックス名 == ソコム弾:01 ) {
			eval( $b:w11a_取得ロード回数[d:I_NO:SOCOM_R01] = $w:グローバルロード回数 );
		} else if ( $:ボックス名 == ソコム弾:02 ) {
			eval( $b:w11a_取得ロード回数[d:I_NO:SOCOM_R02] = $w:グローバルロード回数 );
		}
		#endif

		#ifdef d:STAGE_W11C
		// w11c用
		if ( $:ボックス名 == レーション:04 ) {
			eval( $b:w11a_取得ロード回数[d:I_NO:RATION_04] = $w:グローバルロード回数 );
			eval( $f:アイテム再セット１ = 0 );
		} else if ( $:ボックス名 == ソコム弾:01 ) {
			eval( $b:w11a_取得ロード回数[d:I_NO:SOCOM_R01] = $w:グローバルロード回数 );
		} else if ( $:ボックス名 == ソコム弾:02 ) {
			eval( $b:w11a_取得ロード回数[d:I_NO:SOCOM_R02] = $w:グローバルロード回数 );
			eval( $f:アイテム再セット２ = 0 );
		}
		#endif

		#ifdef d:DEBUG_PRINT
			print $:ボックス名
			print $w:グローバルロード回数
		#endif
	}

	proc 発着口アイテム設置 {
		#if d:DEBUG_PRINT
			print 'item_set'
		#endif

		// 髭剃りは最初のときだけ(日本語版以降は新hard以上で設置)
		#ifdef d:STAGE_W11A
			#ifdef d:JAPANESE
			// 日本版ではhard以上
				if ( ( $w:アイテム数Ｒ[ d:アイテム:髭剃り] == 0 ) && ( $w:ゲーム設定 >= d:LEVEL_NORMAL ) ) {
					@髭剃り_初期設置 髭剃り -10000 -46000 19000 0				// 発着口金網中(一度きりのセット)
				}
			#else
			// それ以外でもhard以上
				if ( ( $w:アイテム数Ｒ[ d:アイテム:髭剃り] == 0 ) && ( $w:ゲーム設定 >= d:LEVEL_HARD ) ) {
					@髭剃り_初期設置 髭剃り -10000 -46000 19000 0				// 発着口金網中(一度きりのセット)
				}
			#endif
		#endif

		// w11a w11bのアイテムは共有
		#ifndef d:STAGE_W11C
		if ( $w:アイテム数Ｒ[ d:アイテム:サーマルゴーグル ] == 0  ) {
			@サーマルゴーグル_初期設置 サーマルゴーグル -5500 -54000 26500 0	// 水中
			mesg アイテム サーマルゴーグル 影だけオフ
		}

		@アイテム再セットチェック $b:w11a_取得ロード回数[d:I_NO:RATION_01] 
		if ($f:再セットフラグ) {
			@レーション_初期設置 レーション:01 -15750 -44800 14250 0	// 発着口ロッカー真ん中
		}

		@アイテム再セットチェック $b:w11a_取得ロード回数[d:I_NO:RATION_02] 
		if ($f:再セットフラグ) {
			@レーション_初期設置 レーション:02 -3750 -46000 16875 0		// 発着口右奥
		}
		
		// 以下難易度別に設置を変えるもの
		if ($w:ゲーム設定 == d:LEVEL_VERYEASY) {
			if ($b:w11a_取得ロード回数[d:I_NO:M9] == 0) {
				@Ｍ９_初期設置 Ｍ９本体 -18000 -46250 20250 0 なし			// 棚の下
			}
		}else if ($w:ゲーム設定 == d:LEVEL_EASY) {
			if ($b:w11a_取得ロード回数[d:I_NO:M9] == 0) {
				@Ｍ９_初期設置 Ｍ９本体 -5250 -46000 22125 0 なし			// ジムスーツの横
			}
		}

		if ($w:ゲーム設定 <= d:LEVEL_EASY) {
			@アイテム再セットチェック $b:w11a_取得ロード回数[d:I_NO:M9_R01] 
			if ($f:再セットフラグ) {
				@弾薬_Ｍ９_初期設置 Ｍ９弾:01 -7750 -44250 15125 0			// 荷物の上
			}

			@アイテム再セットチェック $b:w11a_取得ロード回数[d:I_NO:M9_R02] 
			if ($f:再セットフラグ) {
				@弾薬_Ｍ９_初期設置 Ｍ９弾:02 -8625 -44250 14375 0			// 荷物の上
			}
		}
		#endif
	}

	proc 昇降機ホールアイテム設置 {
		#if d:DEBUG_PRINT
			print 'item_set'
		#endif

		#ifdef d:STAGE_W11A
		@アイテム再セットチェック $b:w11a_取得ロード回数[d:I_NO:RATION_03] 
		if ($f:再セットフラグ) {
			@レーション_初期設置 レーション:03 -5500 -44000 -3500 0		// 昇降機ホール左奥
		}
		#endif

		#ifdef d:STAGE_W11B
		@アイテム再セットチェック $b:w11a_取得ロード回数[d:I_NO:RATION_04] 
		if ($f:再セットフラグ) {
			@レーション_初期設置 レーション:04 -5500 -45000 -1250 0		// 昇降機ホール左
		}

		@アイテム再セットチェック $b:w11a_取得ロード回数[d:I_NO:SOCOM_R01] 
		if ($f:再セットフラグ) {
			@弾薬_ソコム_初期設置 ソコム弾:01 500 -45000 -500 0			// フォークリフト脇
		}

		@アイテム再セットチェック $b:w11a_取得ロード回数[d:I_NO:SOCOM_R02] 
		if ($f:再セットフラグ) {
			@弾薬_ソコム_初期設置 ソコム弾:02 5500 -45000 -1000 0		// 昇降機ホール右
		}
		#endif

		#ifdef d:STAGE_W11C
		@アイテム再セットチェック $b:w11a_取得ロード回数[d:I_NO:RATION_04] 
		if ($f:再セットフラグ) {
			@レーション_初期設置 レーション:04 -5500 -45000 -1250 0		// 昇降機ホール左
			eval($f:アイテム再セット１ = 1);
		}

		@アイテム再セットチェック $b:w11a_取得ロード回数[d:I_NO:SOCOM_R01] 
		if ($f:再セットフラグ) {
			@弾薬_ソコム_初期設置 ソコム弾:01 500 -45000 -500 0			// フォークリフト脇
		}

		@アイテム再セットチェック $b:w11a_取得ロード回数[d:I_NO:SOCOM_R02] 
		if ($f:再セットフラグ) {
			@弾薬_ソコム_初期設置 ソコム弾:02 5500 -45000 -1000 0		// 昇降機ホール右
			eval($f:アイテム再セット２ = 1);
		}

		trap ev021 d:PLAYER \
			-mask いる \
			-exec {
				// フォーチュン戦中はレーションとソコム弾:02はなければ再発生
				if(    ($w:アイテム数Ｒ[d:アイテム:レーション] < 1) \
					&& (`command カメラ視界チェック -camera 0x01 \
							-bound (-5500-1000) (-45000) (-1250-1000) \
							       (-5500+1000) (-44000) (-1250+1000)` == 0)){

					if($f:アイテム再セット１ == 0) {
						@レーション_初期設置 レーション:03 -5500 -45000 -1250 0		// 昇降機ホール左
						eval( $f:アイテム再セット１ = 1 );
					}
				}

				if(    ($w:武器弾数Ｒ[d:武器:ソコム] < 12) \
					&& (`command カメラ視界チェック -camera 0x01 \
							-bound (5500-1000) (-45000) (-1000-1000) \
							       (5500+1000) (-44000) (-1000+1000)` == 0)){

					if($f:アイテム再セット２ == 0) {
						@弾薬_ソコム_初期設置 ソコム弾:02 5500 -45000 -1000 0		// 昇降機ホール右
						eval( $f:アイテム再セット２ = 1 );
					}
				}
			}

		#endif
	}


	// フォグ
	// ------------------------------------
	proc フォグ設定 {
		#if d:DEBUG_PRINT
			print 'fog_set'
		#endif

		#ifdef d:STAGE_W11C
			chara フォグ 霧\
				-c 59, 45, 20 \
				-n -2000 \
				-f 34000 
		#else
			if ( $w:p_story < d:ST:P004_01_P01ノード初接続１ポリゴンデモ１開始 || \
			 $w:p_story > d:ST:P004_01_P01ノード初接続１ポリゴンデモ１終了 ) {

				chara フォグコントローラ フォグ \
						-n 2 \
						-z \
						-d   59 45 20	-2000  34000  34000 34000 \	// 地上
							 16 60 32	-10000 24000  16000  4000	// 水中

				trap wa100 d:PLAYER \
					-mask ＊ \
					-exec {
						command プレイヤー状態取得
						if (( $status & d:PFLAG_WATER ) && (!( $status & d:PFLAG_WATER_SURFACE ))){
							// 水中モードであるが水面ではない
							mesg フォグコントローラ フォグ セット 1 0
						} else {
							mesg フォグコントローラ フォグ セット 0 0
						}
					}
			}
		#endif
	}

	// キャラ用光源(マップの設置が終わってから設定することで、キャラだけに光を当てる)
	// ------------------------------------
	proc キャラ用ライト設置 {
		#if d:DEBUG_PRINT
			print 'chara_light_set'
		#endif

		chara 環境光 環境光源 \
			 -c 50 42 18

		chara 平行光 平行光源 \
			 -c 84 64 32 \
			 -p 4000 -3000 -1000
	}


	// ステージカメラ設定 
	// ------------------------------------
	proc カメラセット {
		#if d:DEBUG_PRINT
			print 'stage_camera_set'
		#endif

		chara カメラ ステージカメラ		-c 0 -l 3 -p 0 
		chara カメラ メインカメラ		-c 0 -l 2 -p 50
		chara カメラ サブカメラ			-c 0 -l 2 -p 40
		chara カメラ サブカメラ２		-c 0 -l 2 -p 30
		#ifdef d:STAGE_W11C
		#else
			chara カメラ モーションカメラ	-c 0 -l 1 -p 20
		#endif
		chara カメラ シナリオデモカメラ		-c 0 -l 0 -p 10
		#ifdef d:STAGE_W11A
			chara カメラ クリアリングカメラ		-c 1 -l 2 -p 0
		#endif

		// デフォルトカメラ
		chara カメラ設定 ステージカメラ \
			-c 0 \	
			-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
			-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
			-r 250,2048,0 \
			-f 6500 \
			-a 200 \
			-i 2 2 0 0 \
			-s 1
	}



// 各マップ別設定
// ------------------------------------
// 昇降機ホール
	// 昇降機設置
	// ------------------------------------
#define	ELV_NAME	elv_1

	proc 昇降機設置	{
		#if d:DEBUG_PRINT
		        print 'w11_elevator_set'
		#endif

		#ifdef d:STAGE_W11A
			chara フォーチュン戦昇降機 昇降機 \
				-e_model	d:ELV_NAME \
				-e_top		0, 90000, 0 \
				-e_bottom	0, 0, 0 \
				-e_rot		0, 0, 0 \
				-trap		ev100 ev000 \	// 起動トラップ 敵兵チェックトラップ
				-proc		シャッターデモ 到着フラグ設定 \
				-flag (d:FRT_ELV_W11 | d:FRT_ELV_CLOSE)
		#endif

		#ifdef d:STAGE_W11B
			chara フォーチュン戦昇降機 昇降機 \
				-e_model	d:ELV_NAME \
				-e_top		0, 10000, 0 \
				-e_bottom	0, 0, 0 \
				-e_rot		0, 0, 0 \
				-trap		ev100 ev000 \
				-proc		シャッターデモ 到着デモ \
				-flag (d:FRT_ELV_W11 | d:FRT_ELV_CLOSE)

			mesg フォーチュン戦昇降機 昇降機 降下
		#endif

		#ifdef d:STAGE_W11C
			if($s:d_num == not_demo) {
			// ゲームではフォーチュン昇降機が必要
				chara フォーチュン戦昇降機 昇降機 \
					-e_model  elv_1 \
					-e_top    0, 45000, 0 \
					-e_bottom 0,     0, 0 \
					-e_rot    0,     0, 0 \
					-trap	ev100 ev000 \
					-proc	空プロック 到着フラグ設定 \
					-flag (d:FRT_ELV_W11 | d:FRT_ELV_CLOSE) \
			                -name      human_vmp_naked \
					-kms_model vmp_coat_bounding \
					-evm_model vmp_coat_mh_mt \
					-motion    raiden         \
					-rotation  0,0,0 \
					-position  0,-45000,-12000
			#if d:DEBUG_PRINT
			        print 'w11_elevator_vamphair_set'
			#endif
				chara マルチウェイト髪の毛モデル マルチウェイトヴァンプの髪 \
					-n vmp_hair_coat_mh_mt \
					-d 9  \ /* ヴァンプ髪の毛 */
					-y human_vmp_naked \
					-z 2 11 12 \
					-f 1

			#if d:DEBUG_PRINT
				print 'w11_elevator_vampknif_set'
			#endif
				chara マルチウェイト髪の毛モデル マルチウェイトヴァンプのナイフ \
					-n vmp_parts_vkpa_mh\
					-d 10 \ /* ヴァンプナイフ */
					-y human_vmp_naked \
					-z 0 \
					-f 1

			#if d:DEBUG_PRINT
				print 'w11_elevator_vampcoat_set'
			#endif
				chara マルチウェイト髪の毛モデル マルチウェイトヴァンプのコート\
					-n vmp_parts_coat_mh\
					-d 11 \ /* ヴァンプコート */
					-y human_vmp_naked \
					-z 0 13 17 \
					-f 1

			} else {
			// デモではスイッチだけ必要
				chara フォーチュン戦昇降機スイッチ 昇降機スイッチ \
					-up   1747, -43441, -9930 \
					-down 1747, -43541, -9930 \
					-mode 1
			}
		#endif

	}

	proc シャッターデモ {
		#if d:DEBUG_PRINT
		        print 'w11_elevator_go'
		#endif

		if ( `command ゲームオーバーチェック` == 0 ) {
			@シナリオデモ開始処理
			chara カメラ設定 シナリオデモカメラ  \
				-c 1 \	
				-p 0,-42999,-3037 -t 0,-43401,-6872 \
				-r 68,2048,0 -f 3856 \
				-a 200 \
				-i 0 0 0 0 \
				-s 1
		}
	}

	proc 到着フラグ設定 {
		#if d:DEBUG_PRINT
		        print 'w11_elevator_flagon'
		#endif

		eval( $f:rfp_w11_昇降機に乗れますよ = 1 );
	}

	proc 到着デモ {
		#if d:DEBUG_PRINT
		        print 'w11b_elevator_end'
		#endif

		command シナリオデモ開始
		command 無線設定 -reset
		command パッド操作 -release
		command プレイヤー無敵セット
		@到着フラグ設定
		chara カメラ設定 シナリオデモカメラ  \
			-c 1 \	
			-p 0,-42999,-3037 -t 0,-43401,-6872 \
			-r 68,2048,0 -f 3856 \
			-a 200 \
			-i 0 1 0 0 \
			-s 1

		chara delay ディレイ \
			-time 60 \
			-exec {
				mesg プレイヤー d:PLAYER run 750 -45000 -9000 -1 0 d:FA_DIRECT_MOVE 到着デモ終了
			}
	}

	proc 到着デモ終了 {
		#if d:DEBUG_PRINT
		        print 'w11b_elevator_demo_end'
		#endif

		command シナリオデモ終了
		command パッド操作 -cancel
		command プレイヤー無敵解除
		@ステージ開始時処理終了（昇降機）
		chara カメラ設定 シナリオデモカメラ -s -1
		// エレベータ上昇用トラップ＆ロード用トラップを有効にする
		command トラップ切り替え -trap ev000 -switch 1
		command トラップ切り替え -trap ev100 -switch 1
		command トラップ切り替え -trap ld001 -switch 1

		#ifdef d:CTDOWN_TIMER_P
			// プラント編爆弾タイマー
			@爆弾タイマー開始
		#endif
	}

	proc 空プロック {
	}

	// 天井設置
	// ------------------------------------
	proc 昇降機ホール天井設置 {
		#if d:DEBUG_PRINT
			print 'w11c_ceil_set'
		#endif

		#ifdef d:STAGE_W11A
			chara 天井君 ホール天井 -m d:CEIL_NAME2 -h -41700 -p 0,0,0 -l d:LT2_NAME
		#else
			chara 天井君 ホール天井 -m d:CEIL_NAME2 -h -41700 -p 0,0,0 -l d:LT2_NAME -v 天井表示制御
		#endif

		chara 天井君 ホール南壁 -m d:WALL_NAME1 -a 5 -c 0,0,6000 -p 0,0,0 -l d:LT2_NAME -v 壁表示制御
	}

	proc 壁表示制御 {
		#if d:DEBUG_PRINT
			print 'w11c_wall_vanish'
		#endif
		command プットスポットライトオブジェライト管理 \
			-n ホール壁スポットライト:03 \
			-f $1

		command プットスポットライトオブジェライト管理 \
			-n ホール壁スポットライト:04 \
			-f $1
	}

	proc 天井表示制御 {
		#if d:DEBUG_PRINT
			print 'w11c_ceil_broken_vanish'
		#endif
		command フォーチュン戦天井崩れ表示管理 $1
	    //mesg フォーチュン戦天井崩れ 天井崩れ 表示管理 $1
	}

	// フォーチュン壊したオブジェクト設定
	// ------------------------------------
	proc デモ中壊れオブジェクト設定 {
		#if d:DEBUG_PRINT
			print 'w11c_cable_set'
		#endif

		#ifdef d:STAGE_W11B
			chara 天井君 天井カーブル -m d:BK_CABLE_NAME -h -41700 -p 0,0,0 -l d:LT2_NAME
		#endif

		#ifdef d:STAGE_W11C
			if ( $w:p_story < d:ST:P028_01_P01爆弾解体後フォーチュン再登場１ポリゴンデモ１終了 ) {
				chara 天井君 天井カーブル -m d:BK_CABLE_NAME -h -41700 -p 0,0,0 -l d:LT2_NAME
			} else {
				chara 天井君 天井カーブル -m d:BK_CABLE_NAME -h -41700 -p 0,0,0 -l d:LT2_NAME
				chara プットオブジェ 箱 -m d:BK_OBJ_NAME -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME
			}
		#endif
	}


	// ライトオブジェクト設定
	// ------------------------------------
	// 壊れ物ライトの番号
	enum L_NO {
		W_LIT_0_1 = 0,
		W_LIT_0_2,
		W_LIT_1_1,
		W_LIT_1_2,
		W_LIT_1_3,
		W_LIT_1_4,
		W_LIT_1_5,
		W_LIT_1_6
	}

	// ゲートにぶら下がっているライト
	// ------------------------------------
	proc 発着口壁スポットライト {
		#if d:DEBUG_PRINT
		        print 'w11ab0_wallspot_light'
		#endif

		command foreach -argc 3 -repeat 2 \
			-data { \
				{ d:L_NO:W_LIT_0_1, 発着口壁スポットライト:01, -8300 }
				{ d:L_NO:W_LIT_0_2, 発着口壁スポットライト:02, -5700 }
			} \
			-exec { \
				chara プットスポットライトオブジェ $2 \
					-m w11c2_gatelit w11c2_gatelit_brk \
					-p $3,-42100, 14200  -r 0 0 0  -l d:LT2_NAME \
//					-f d:BRK_LGT_SHOWTRGT \
					-t $3,-42250, 14350 100 1000 500 \
					-b ライト破壊後処理

				@壊れ物再セットチェック $b:w11a_破壊ロード回数[$1]
				if ( $f:再セットフラグ == 0 ) {
					command プットスポットライトオブジェライト消す -n $2
				}
			}
	}

	proc 昇降機ホール壁スポットライト {
		#if d:DEBUG_PRINT
		        print 'w11abc2_wallspot_light'
		#endif

		command foreach -argc 7 -repeat 6 \
			-data { \
				{ d:L_NO:W_LIT_1_1, ホール壁スポットライト:01, 6300 1250 -1024 6150 1250 }
				{ d:L_NO:W_LIT_1_2, ホール壁スポットライト:02, 6300 -1250 -1024 6150 -1250 }
				{ d:L_NO:W_LIT_1_3, ホール壁スポットライト:03, -1250 6000 2048 -1250 5850 }
				{ d:L_NO:W_LIT_1_4, ホール壁スポットライト:04, -3750 6000 2048 -3750 5850 }
				{ d:L_NO:W_LIT_1_5, ホール壁スポットライト:05, 1750 -9750 0 1750 -9600 }
				{ d:L_NO:W_LIT_1_6, ホール壁スポットライト:06, -1750 -9750 0 -1750 -9600 }
			} \
			-exec { \
				chara プットスポットライトオブジェ $2 \
					-m w11c2_gatelit w11c2_gatelit_brk \
					-p  $3,-42100,$4  -r 0 $5 0  -l d:LT2_NAME \
//					-f  d:BRK_LGT_SHOWTRGT \
					-t  $6,-42250,$7 100 1000 500 \
					-b ライト破壊後処理

				@壊れ物再セットチェック $b:w11a_破壊ロード回数[$1]
				if ( $f:再セットフラグ == 0 ) {
					command プットスポットライトオブジェライト消す -n $2
				}
			}
	}

	// 破壊されたときの処理
	proc ライト破壊後処理 {
		#if d:DEBUG_PRINT
			print 'w11_light_broken'
		#endif

		if ( $1 == 発着口壁スポットライト:01 ) {
			eval( $b:w11a_破壊ロード回数[d:L_NO:W_LIT_0_1] = $w:グローバルロード回数 );
		} else if ( $1 == 発着口壁スポットライト:02 ) {
			eval( $b:w11a_破壊ロード回数[d:L_NO:W_LIT_0_2] = $w:グローバルロード回数 );
		} else if ( $1 == ホール壁スポットライト:01 ) {
			eval( $b:w11a_破壊ロード回数[d:L_NO:W_LIT_1_1] = $w:グローバルロード回数 );
		} else if ( $1 == ホール壁スポットライト:02 ) {
			eval( $b:w11a_破壊ロード回数[d:L_NO:W_LIT_1_2] = $w:グローバルロード回数 );
		} else if ( $1 == ホール壁スポットライト:03 ) {
			eval( $b:w11a_破壊ロード回数[d:L_NO:W_LIT_1_3] = $w:グローバルロード回数 );
		} else if ( $1 == ホール壁スポットライト:04 ) {
			eval( $b:w11a_破壊ロード回数[d:L_NO:W_LIT_1_4] = $w:グローバルロード回数 );
		} else if ( $1 == ホール壁スポットライト:05 ) {
			eval( $b:w11a_破壊ロード回数[d:L_NO:W_LIT_1_5] = $w:グローバルロード回数 );
		} else if ( $1 == ホール壁スポットライト:06 ) {
			eval( $b:w11a_破壊ロード回数[d:L_NO:W_LIT_1_6] = $w:グローバルロード回数 );
		}
	}

	// 天井からぶら下がっているライト
	// ------------------------------------
	// ロード時に実行各ライトの破壊状況を調べる
	proc 昇降機ホール揺れフラグチェック {
		#if d:DEBUG_PRINT
		        print 'w11abc2_light_check'
		#endif

		command foreach -argc 5 -repeat 3 \
			-data { \
				{ ライト群右側, 0, 1, 2, 3 }
				{ ライト群中央, 4, 5, 6, 7 }
				{ ライト群左側, 8, 9, 10, 11 }
			} \
			-exec { \
				command フォーチュン戦揺れライト壊れフラグ取得 \
					-name $1 -flag $i:ローカル変数Ｉ１

				if ( ($i:ローカル変数Ｉ１ & 0x0100) && $f:w11a_揺れライト壊れフラグ[$2] == 0 ) {
					eval( $f:w11a_揺れライト壊れフラグ[$2] = 1 );
					eval( $b:w11a_揺れライト破壊ロード回数[$2] = $w:グローバルロード回数 );
				}

				if ( ($i:ローカル変数Ｉ１ & 0x0200) && $f:w11a_揺れライト壊れフラグ[$3] == 0 ) {
					eval( $f:w11a_揺れライト壊れフラグ[$3] = 1 );
					eval( $b:w11a_揺れライト破壊ロード回数[$3] = $w:グローバルロード回数 );
				}

				if ( ($i:ローカル変数Ｉ１ & 0x0400) && $f:w11a_揺れライト壊れフラグ[$4] == 0 ) {
					eval( $f:w11a_揺れライト壊れフラグ[$4] = 1 );
					eval( $b:w11a_揺れライト破壊ロード回数[$4] = $w:グローバルロード回数 );
				}

				if ( ($i:ローカル変数Ｉ１ & 0x0800) && $f:w11a_揺れライト壊れフラグ[$5] == 0 ) {
					eval( $f:w11a_揺れライト壊れフラグ[$5] = 1 );
					eval( $b:w11a_揺れライト破壊ロード回数[$5] = $w:グローバルロード回数 );
				}
			}
	}

	proc 昇降機ホール揺れライト設定 {
		#if d:DEBUG_PRINT
		        print 'w11abc2_light_set'
		#endif

		command foreach -argc 6 -repeat 3 \
			-data { \
				{ ライト群右側, 3000,	0, 1, 2, 3 }
				{ ライト群中央, 0,		4, 5, 6, 7 }
				{ ライト群左側, -3000,	8, 9, 10, 11 }
			} \
			-exec { \
				eval( $i:ローカル変数Ｉ１ = 0 );

				@壊れ物再セットチェック $b:w11a_揺れライト破壊ロード回数[$3]
				if ( $f:再セットフラグ ) {
					// 復活する
					eval( $f:w11a_揺れライト壊れフラグ[$3] = 0 );
				} else {
					// 壊れたまま
					eval( $i:ローカル変数Ｉ１ = $i:ローカル変数Ｉ１ + 0x0100 );
				}

				@壊れ物再セットチェック $b:w11a_揺れライト破壊ロード回数[$4]
				if ( $f:再セットフラグ ) {
					// 復活する
					eval( $f:w11a_揺れライト壊れフラグ[$4] = 0 );
				} else {
					// 壊れたまま
					eval( $i:ローカル変数Ｉ１ = $i:ローカル変数Ｉ１ + 0x0200 );
				}

				@壊れ物再セットチェック $b:w11a_揺れライト破壊ロード回数[$5]
				if ( $f:再セットフラグ ) {
					// 復活する
					eval( $f:w11a_揺れライト壊れフラグ[$5] = 0 );
				} else {
					// 壊れたまま
					eval( $i:ローカル変数Ｉ１ = $i:ローカル変数Ｉ１ + 0x0400 );
				}

				@壊れ物再セットチェック $b:w11a_揺れライト破壊ロード回数[$6]
				if ( $f:再セットフラグ ) {
					// 復活する
					eval( $f:w11a_揺れライト壊れフラグ[$6] = 0 );
				} else {
					// 壊れたまま
					eval( $i:ローカル変数Ｉ１ = $i:ローカル変数Ｉ１ + 0x0800 );
				}

				if ( $1 == ライト群中央 ) {
					chara フォーチュン戦揺れライト $1 \
						-model  w11c2_ceillit_hanger w11c2_ceillit w11c2_ceillit_brk \
						-volume w11c2_ceillit_hl_cm w11c2_ceillit_volume \
						-pos $2,-40000,2755 \
						-rot 0,1024,0 \
						-hazard d:BRK_HZD_W11C_FORTUNE \
						-brk_flag $i:ローカル変数Ｉ１ \
						#ifdef d:STAGE_W11A
							-flag (1|0x20) \ //サウンドを鳴らす
						#else
							-flag 1 \ //サウンドを鳴らす
						#endif
						-light  d:LT2_NAME
				} else {
					chara フォーチュン戦揺れライト $1 \
						-model  w11c2_ceillit_hanger w11c2_ceillit w11c2_ceillit_brk \
						-volume w11c2_ceillit_hl_cm w11c2_ceillit_volume \
						-pos $2,-40000,2755 \
						-rot 0,1024,0 \
						-hazard d:BRK_HZD_W11C_FORTUNE \
						-brk_flag $i:ローカル変数Ｉ１ \
						-light  d:LT2_NAME
				}
			}
	}

	// 壁にある長細いライト（プレイヤーは壊せない）
	// ------------------------------------
	proc 昇降機ホール壁ライト設定 {
		#if d:DEBUG_PRINT
		        print 'w11abc2_walllight'
		#endif

		chara フォーチュン戦壁ライト 前列右壁ライト \
			-model  w11c2_walllit w11c2_walllit_brk1 w11c2_walllit_brk2 w11c2_walllit_hl \
			-pos    3500,-42375,-6505 \
			-rot       0,     0,    0 \
			-hazard d:BRK_HZD_W11C_FORTUNE \
			-light  d:LT2_NAME

		chara フォーチュン戦壁ライト 中列右壁ライト \
			-model  w11c2_walllit w11c2_walllit_brk1 w11c2_walllit_brk2 w11c2_walllit_hl \
			-pos    6000,-42375,-1995 \
			-rot       0,     0,    0 \
			-hazard d:BRK_HZD_W11C_FORTUNE \
			-light  d:LT2_NAME

		chara フォーチュン戦壁ライト 後列右壁ライト \
			-model  w11c2_walllit w11c2_walllit_brk1 w11c2_walllit_brk2 w11c2_walllit_hl \
			-pos    6000,-42375, 2005 \
			-rot       0,     0,    0 \
			-hazard d:BRK_HZD_W11C_FORTUNE \
			-light  d:LT2_NAME

		chara フォーチュン戦壁ライト 前列左壁ライト \
			-model  w11c2_walllit w11c2_walllit_brk1 w11c2_walllit_brk2 w11c2_walllit_hl \
			-pos   -3500,-42375,-6505 \
			-rot       0, 180, 0 \
			-hazard d:BRK_HZD_W11C_FORTUNE \
			-light  d:LT2_NAME 

		chara フォーチュン戦壁ライト 中列左壁ライト \
			-model  w11c2_walllit w11c2_walllit_brk1 w11c2_walllit_brk2 w11c2_walllit_hl \
			-pos   -6000,-42375,-1995 \
			-rot       0, 180, 0 \
			-hazard d:BRK_HZD_W11C_FORTUNE \
			-light  d:LT2_NAME 

		chara フォーチュン戦壁ライト 後ろ列左壁ライト \
			-model  w11c2_walllit w11c2_walllit_brk1 w11c2_walllit_brk2 w11c2_walllit_hl \
			-pos   -6000,-42375, 2005 \
			-rot       0, 180, 0 \
			-hazard d:BRK_HZD_W11C_FORTUNE \
			-flag   0x0020 \
			-light  d:LT2_NAME 
	}



	// スポットライト設置
	// ------------------------------------
	proc 昇降機ホールスポットライト設定 {
		#if d:DEBUG_PRINT
			print 'w11_spot_light_set'
		#endif

		#ifdef d:STAGE_W11A
			command foreach -argc 13 -repeat 2 \
				-data { \
					{ ライト:01, w11a2k1, -6500, -42000,  3250, 640, 1024, 4500, 35, 128, 128, 64, 0x0001 }
					{ ライト:02, w11a2k2,     0, -42000, -6000, 840, 2048, 5000, 45,  80,  80, 36, 0x0001 }
				} \
				-exec { \
					chara スポットライト投影モデル $1 \
						-m $2 \
						-p 0,0,0

					chara 静止スポットライト設置 $1 \
						-pos $3, $4, $5 \
						-dir $6, $7, 0 \
						-range $8 \
						-angle $9 \
						-color $10,$11,$12 \
						-option $13

				}
			// 敵兵発見用に設定
			command 影投影ハザード -name 昇降機ホール -group 6

		#else
			command foreach -argc 13 -repeat 3 \
				-data { \
					{ ライト:01, w11c2k1, -6500, -42000,  3250, 640, 1024, 4500, 35, 128, 128, 64, 0x0001 }
					{ ライト:02, w11c2k2,     0, -42000, -6000, 840, 2048, 5000, 45,  80,  80, 36, 0x0001 }
					{ ライト:03, w11c2k3, -5671, -42044, -2025, 560, 1024, 4500, 35, 128, 128, 64, 0x0001 }/*デモ専用*/
				} \
				-exec { \
					if($s:d_num == not_demo) {
						if ( $1 == ライト:01 || $1 == ライト:02 ) {
							chara スポットライト投影モデル $1 \
								-m $2 \
								-p 0,0,0

							chara 静止スポットライト設置 $1 \
								-pos $3, $4, $5 \
								-dir $6, $7, 0 \
								-range $8 \
								-angle $9 \
								-color $10,$11,$12 \
								-option $13
						}
					} else {
						chara スポットライト投影モデル $1 \
							-m $2 \
							-p 0,0,0

						chara 静止スポットライト設置 $1 \
							-pos $3, $4, $5 \
							-dir $6, $7, 0 \
							-range $8 \
							-angle $9 \
							-color $10,$11,$12 \
							-option $13
					}
				}
		#endif
	}



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
}

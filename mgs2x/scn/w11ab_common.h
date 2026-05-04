/*
	w11ab_common.h                     
		海底ドック共通プロック(w11a,w11bで使用)

	2001/04/11 H.Yoshiike         
	$Id: w11ab_common.h,v 1.87 2001/11/16 07:02:18 usr03682 Exp $                      

	
*/

// 各proc設定
//-------------------------------------------------------------------------------------
// 全マップ共通
	// ジャンプ設定
	// ------------------------------------
	proc ジャンプ設定 {
		#if d:DEBUG_PRINT
			print 'w11a0_jump_set'
		#endif

		trap ky001 d:PLAYER \
			-mask 入る \
			-exec {
				@ジャンプ発動 ky001 1000 10
			}
	}

	// ドア＆マップ切り替え関係
	// ------------------------------------
	proc 水密ドア設置 {
		#if d:DEBUG_PRINT 
			print 'wt_door_set' 
		#endif

		chara 水密ドア管理 ドア管理人 -n 1 \
			-motions rai_wt_hatch hatch_door_sna hatch_handle_sna \
			-seno 3

		if ( $w:p_story < d:ST:P002_01_S01Ａ脚底部連結ハッチ１シナリオデモ１開始 ) {
			chara 水密ドア 水密君 \
				-kms w11a_sdr_body_l w11a_sdr_handle \
				-position -12300,-45000,12950 \
				-rotate 0,0,0 \
				-seno 3 \
				-hazard -12250,-45000,12900 -11250,-45000,12900 \
				-exec {
					#ifdef d:STAGE_W11A 
						if ( $:トリガ == 1 ) {
							// 扉を握ったらスネークが倒す音を呼ぶ
							if ( $f:w11a_スネーク敵兵をやっつける音フラグ == 0 ) {
								command ＳＥセットモード -s d:se_code:SD_A_SNAPPK01 -p -11750,-45000,3250 -m 1
								eval( $f:w11a_スネーク敵兵をやっつける音フラグ = 1 );
							}
						} else if ( $:トリガ == 0 ) {
							mesg 透明壁 水密ドア用透明壁 on
							@Ａ脚底部連結ハッチ１シナリオデモ１開始
						}
					#endif
				}
		} else {
			chara 水密ドア 水密君 \
				-kms w11a_sdr_body_l w11a_sdr_handle \
				-position -12300,-45000,12950 \
				-rotate 0,0,0 \
				-seno 3 \
				-flag d:WT_DOOR_FLAG_OPEN_START
		}

		trap dr001 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \
			-state 0 \
			-exec {
				command 水密ドア状態 -n 水密君
				mesg プレイヤー d:PLAYER wt_door 水密君 $status 2048,512 -12300,12950
			}
		trap dr001 d:PLAYER \
			-mask ？ \
			-state 6 \
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 1 \
					-p -10657,-41962,16353 -t -11440,-43497,13935 \
					-r 354,2252,0 -f 2969 \
					-a 200 \
					-i 0 2 0 0 \
					-s $3
			}
	}



	// 入出トラップ
	// ------------------------------------
	proc ロードプロック {
		#if d:DEBUG_PRINT 
			print 'load_proc'
		#endif

		@昇降機ホール揺れフラグチェック			// w11abc_common.h

		if ( $s:呼び出しプロック == mv_w11a2_demo ) {
			// オープニングポリデモへ
			eval( $w:p_story = d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１開始 )
			eval( $w:スタートアラートモード = d:ALERT_MODE_SENNYU );
			command セットサウンドコード -c d:SNG_FOUTS_S
			load  'd005p01' -r 'r_plt0'
		} else if ( $s:呼び出しプロック == mv_w11b_w12a_0 ) {
			// Ａ脚屋上へ
			command セットサウンドコード -c d:SNG_FOUTS_S
			@mv_w11b_w12a_0
		} else if ( $s:呼び出しプロック == mv_w11b_w11c_0 ) {
			// フォーチュン戦ポリデモへ
			command セットサウンドコード -c d:SNG_FOUTS_S
			@mv_w11b_w11c_0
		}
	}

	proc 昇降機ホールロード設定 {
		#if d:DEBUG_PRINT 
			print 'load_set'
		#endif

		trap ld001 d:PLAYER \
			-mask 入る \
			-exec {
				if( $f:ロードチェックＯＮフラグ == 1 ) {
					if ( $w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１開始 ) {
						// オープニングポリデモへ
						preseek 'd005p01'
						@ステージ終了時処理 0 0 0 0 mv_w11a2_demo d:NO_MOVE_M_TYPE
					} else {
						// そのままＡ脚屋上へ(w11b)
						preseek 'w12c'
						@爆弾タイマー残り時間保存			// ストーリーフラグは自動で
						@ステージ終了時処理（ステージ名表示） \
							0 0 0 0 mv_w11b_w12a_0 d:NO_MOVE_M_TYPE \
							d:wn_Ａ脚屋上
					}
				}
			}

		trap ld002 d:PLAYER \
			-mask 入る \
			-exec {
				// このトラップは爆弾解体終了無線が終了すると起動します。
				// フォーチュン戦ポリデモへ
				eval ( $w:p_story = d:ST:P028_01_P01爆弾解体後フォーチュン再登場１ポリゴンデモ１開始 );
				preseek 'w11c'
				@ステージ終了時処理 -5439 -45000 2974 0 mv_w11b_w11c_0 d:M_TYPE:WALK_CHANGE
			}
	}


// 各マップ別設定
// ------------------------------------
// 発着口
	// 天井設置
	// ------------------------------------
	proc 発着口天井設置 {
		#if d:DEBUG_PRINT
			print 'w11a0_ceil_set'
		#endif

		chara 天井君 発着口天井 -m d:CEIL_NAME0 -h -41500 -p 0,0,0 -l d:LT2_NAME
		chara 天井君 南壁 -m d:WALL_NAME0 -a 5 -c 0 0 29250 -p 0,0,0 -l d:LT2_NAME
	}

	// 透明壁設置
	// -----------------------------------
	proc 透明壁設置 {
		#if d:DEBUG_PRINT
			print 'w11a0_tr_wall_set'
		#endif

		// 水密ドア用
		chara 透明壁 水密ドア用透明壁 \
			-position	-12250 -45000 12875 2000 \
						-12250 -45000 13875 2000 \
			-attribute	( d:HZX_SEG_NO_C4 | d:HZX_SEG_NO_BULLETHOLE | d:HZX_SEG_NO_HARITSUKI ) \
			-seno 3

		if ( $w:p_story < d:ST:P002_01_S01Ａ脚底部連結ハッチ１シナリオデモ１開始 ) {
			// 水密扉が閉まっているときはoff
			mesg 透明壁 水密ドア用透明壁 off
		}

		// 階段横の隙間
		@東西５０ｃｍ幅透明壁設置 海底ドック下壁 -17250 -46000 19750 2000

		trap wl001 d:PLAYER \
			-mask ？ \
			-state 4 \	
			-exec {
				if ($3 == 入る) {
					command パッド操作 -mask d:PAD_B
					mesg 透明壁 海底ドック下壁 off
				} else {
					command パッド操作 -cancel 0
					mesg 透明壁 海底ドック下壁 on
				}
			}
	}

	// ロッカー関係
	// -----------------------------------
	proc ロッカーチェック $:番号 {
		@壊れ物再セットチェック $b:w11a_ロッカー破壊ロード回数[$:番号]
		if($f:再セットフラグ) {
			eval( $i:w11a_ロッカー状態[$:番号] = 0 );
		}
	}

	proc ロッカー状態保存 $:番号 $:行動 $:ロッカー名 {
		if(($:行動 != 6) && ($:行動 != 8)) {
			// 状態変化時
			eval($b:w11a_ロッカー破壊ロード回数[$:番号] = $w:グローバルロード回数);
		} else if($:行動 == 8) {
			// ステージ終了時
			command ロッカー状態 -n $:ロッカー名
			eval($i:w11a_ロッカー状態[$:番号] = $status)
		}
	}

	proc ロッカー設置 {
		#if d:DEBUG_PRINT
			print 'w11a0_locker_set'
		#endif

		chara ロッカー管理 管理人さん -n 3
		command ロッカーモーション -motion locker
		command プレイヤーロッカーモーション -motion sna_locker

		@ロッカーチェック 0
		chara ロッカー ロッカー01 \
			-kms neo_locker_door \
			-p -16300,-45000,14625 \
			-r 0,0,0 \
			-down_rot 0,3900,0 \
			-down_shift -100,0,100 \
			-s 0 \
			-flag 0x0001 \
			-seno d:SD_LOCKER_HZD \
			-sefloor d:SD_LOCKER_FLR \
			-load_state $i:w11a_ロッカー状態[0] \
			-exec {
				@プレイヤーロッカーに入ったかチェック $2 $3 $4
				@ロッカー状態保存 0 $3 $4
			}
		trap lk001 d:PLAYER \
			-and \
			-mask いる \
			-state 0,4 \
			-dir 2048,512 0,512 \
			-button d:ACTION_BUTTON,-1 \
			-exec {
				command ロッカー状態 -n ロッカー01
				mesg プレイヤー d:PLAYER locker ロッカー01 $status 2048,512 -16300,14625
			}
		trap lk001 d:PLAYER \
			-mask ？ \
			-state 6 \
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 1 \
					-p -17913,-42634,17368 -t -17145,-43211,15905 \
					-r 219,1733,0 -f 1750 \
					-a 200 \
					-i 0 2 0 0 \
					-s $3
			}

		@ロッカーチェック 1
		chara ロッカー ロッカー02 \
			-kms neo_locker_door \
			-p -15300,-45000,14625 \
			-r 0,0,0 \
			-s 0 \
			-flag 0x0001 \
			-seno d:SD_LOCKER_HZD \
			-sefloor d:SD_LOCKER_FLR \
			-load_state $i:w11a_ロッカー状態[1] \
			-exec {
				@プレイヤーロッカーに入ったかチェック $2 $3 $4
				@ロッカー状態保存 1 $3 $4
			}
		trap lk002 d:PLAYER \
			-and \
			-mask いる \
			-state 0,4 \
			-dir 2048,512 0,512 \
			-button d:ACTION_BUTTON,-1 \
			-exec {
				command ロッカー状態 -n ロッカー02
				mesg プレイヤー d:PLAYER locker ロッカー02 $status 2048,512 -15300,14625
			}
		trap lk002 d:PLAYER \
			-mask ？ \
			-state 6 \
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 1 \
					-p -17217,-43006,17250 -t -16405,-43497,15786 \
					-r 186,1718,0 -f 1744 \
					-a 200 \
					-i 0 2 0 0 \
					-s $3
			}

		@ロッカーチェック 2
		chara ロッカー ロッカー03 \
			-kms neo_locker_door \
			-p -14300,-45000,14625 \
			-r 0,0,0 \
			-down_rot 0,128,0 \
			-down_shift 100,0,-100 \
			-s 0 \
			-flag 0x0001 \
			-seno d:SD_LOCKER_HZD \
			-sefloor d:SD_LOCKER_FLR \
			-load_state $i:w11a_ロッカー状態[2] \
			-exec {
				@プレイヤーロッカーに入ったかチェック $2 $3 $4
				@ロッカー状態保存 2 $3 $4
			}
		trap lk003 d:PLAYER \
			-and \
			-mask いる \
			-state 0,4 \
			-dir 2048,512 0,512 \
			-button d:ACTION_BUTTON,-1 \
			-exec {
				command ロッカー状態 -n ロッカー03
				mesg プレイヤー d:PLAYER locker ロッカー03 $status 2048,512 -14300,14625
			}
		trap lk003 d:PLAYER \
			-mask ？ \
			-state 6 \
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 1 \
					-p -16302,-42527,17194 -t -15398,-43197,15868 \
					-r 258,1658,0 -f 1739 \
					-a 200 \
					-i 0 2 0 0 \
					-s $3
			}
	}

	// フェンス開かないモーション
	// -----------------------------------
	proc フェンスドア設定 {
		#if d:DEBUG_PRINT
			print 'w11_fence_door_set'
		#endif

		trap fe001 d:PLAYER \
			-and \
			-mask いる \
			-state 0,4 \
			-dir 1024,512 3072,512 \
			-button d:ACTION_BUTTON,d:ACTION_BUTTON \
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 1 \
					-p -14104,-43673,20315 -t -12064,-44660,19466 \
					-r 274,1281,0 -f 2420 \
					-a 200 \
					-i 0 2 0 0 \
					-s 1

				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:locker_s_not_open 1024 1024 0 \
					(d:FA_NO_CHECK_TRP | d:FA_NO_CHECK_SEG | d:FA_NO_WEAPON) \
					(-10750-850) (18500+575)
				command	強制モーション終了プロック \
					-motion d:モーションリスト:locker_s_not_open \
					-proc フェンスドアカメラオフ \
			}

		trap fe002 d:PLAYER \
			-and \
			-mask いる \
			-state 0,4 \
			-dir 3072,512 1024,512 \
			-button d:ACTION_BUTTON,d:ACTION_BUTTON \
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 1 \
					-p -3859,-42039,21813 -t -4081,-43449,21316 \
					-r 784,2322,0 -f 1511 \
					-a 200 \
					-i 0 2 0 0 \
					-s 1

				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:locker_s_not_open 3072 3072 0 \
					(d:FA_NO_CHECK_TRP | d:FA_NO_CHECK_SEG | d:FA_NO_WEAPON) \
					(-4750+800) (21250-475)
				command	強制モーション終了プロック \
					-motion d:モーションリスト:locker_s_not_open \
					-proc フェンスドアカメラオフ \
			}
	}

	proc フェンスドアカメラオフ {
		chara カメラ設定 モーションカメラ -s -1
	}


	// イントルード金網設定
	// ------------------------------------
	proc イントルード金網設定 {
		#if d:DEBUG_PRINT
			print 'w11a0_intrude_fence_set'
		#endif

		trap fence_s d:PLAYER \
			-mask ＊ \
			-dir 0 256 \	//方向指定
			-exec {
				command イントルード主観カメラ補正 \
					-se d:se_code:SD_P_HEADNET1 \
					-len 1000 \
					-inout $3
			}

		trap fence_n d:PLAYER \
			-mask ＊ \
			-dir 2048 256 \	//方向指定
			-exec {
				command イントルード主観カメラ補正 \
					-se d:se_code:SD_P_HEADNET1 \
					-len 1000 \
					-inout $3
			}
	}


	// フナ虫設定
	// ------------------------------------
	proc フナ虫設定 {
		#if d:DEBUG_PRINT
			print 'w11a0_intrude_fence_set'
		#endif

		// メニューフナ虫用のトラップをＯＮ
		command トラップ切り替え -trap menu_funa -switch 1

		if($s:d_num == not_demo) {
			chara 潰れふなむし ぺちゃんこフナ虫
			eval( $f:ローカルフラグ１ = 1 );

		} else {
			eval( $f:ローカルフラグ１ = 0 );

		}

		// 中央付近のフナ虫
		chara フナ虫リーダー ゴールドリーダー:01 \
			-n_obj	$f:ローカルフラグ１ \
			-obj	d:PLAYER \
			-pos	-12750 -46000 19250 \	// 中心
			-wide	1500 1500 \				// 横幅、縦幅
			-rot	0 0 0 \					// 回転
			-R		1 \						// 逃げ込みエリア数
				-E  -1000 -500 500 500 \	// 中継地点
				-e  -2750 -1000 500 250		// 最終地点
//			-debug

		@フナ虫子分設定全体 ゴールド:01 200 -200 400 -400
		@フナ虫子分設定全体 ゴールド:02 600 -600 700 -700
		@フナ虫子分設定全体 ゴールド:03 800 -800 900 -900

		// 東側上のフナ虫
		chara フナ虫リーダー ゴールドリーダー:03 \
			-n_obj	$f:ローカルフラグ１ \
			-obj	d:PLAYER \
			-pos	-19750 -46000 21250 \	// 中心
			-wide	1000 1500 \				// 横幅、縦幅
			-rot	0 0 0 \					// 回転
			-R		2 \						// 逃げ込みエリア数
				-E  -500 -1000 500 500 \		// 中継地点
				-e  -1000 -1500 500 500 \		// 最終地点
				-E -500 1000 500 500 \		//
				-e -1000 1500 500 500		//-R個
//			-debug

		@フナ虫子分設定片側 ゴールド:21 -100 -200 -300 -400 400 -150 200 -350

		// 東側壁上のフナ虫
		chara フナ虫リーダー ゴールドリーダー:04 \
			-n_obj	$f:ローカルフラグ１ \
			-obj	d:PLAYER \
			-flag	d:ALLSLT_FLAG_NO_STOMP \
			-pos	-20750 -45000 21250 \	// 中心
			-wide	1500 1000 \				// 横幅、縦幅
			-rot	3072 3072 0 \					// 回転
			-R		2 \						// 逃げ込みエリア数
				-E  -500 1000 500 500 \		// 中継地点
				-e  -1000 1500 500 500 \		// 最終地点
				-E -500 1000 500 500 \		//
				-e -1000 1500 500 500		//-R個
//			-debug

		@フナ虫子分設定全体 ゴールド:31 600 -600 400 -400

		// 東側下のフナ虫
		chara フナ虫リーダー ゴールドリーダー:05 \
			-n_obj	$f:ローカルフラグ１ \
			-obj	d:PLAYER \
			-pos	-19750 -46000 27250 \	// 中心
			-wide	1000 1500 \				// 横幅、縦幅
			-rot	0 0 0 \					// 回転
			-R		2 \						// 逃げ込みエリア数
				-E  -500 -1000 500 500 \		// 中継地点
				-e  -1000 -1500 500 500 \		// 最終地点
				-E -500 1000 500 500 \		//
				-e -1000 1500 500 500		//-R個
//			-debug

		@フナ虫子分設定片側 ゴールド:41 -100 -200 -300 -400 -50 -150 -250 -350

		// 東側壁下のフナ虫
		chara フナ虫リーダー ゴールドリーダー:06 \
			-n_obj	$f:ローカルフラグ１ \
			-obj	d:PLAYER \
			-flag	d:ALLSLT_FLAG_NO_STOMP \
			-pos	-20750 -45000 27250 \	// 中心
			-wide	1500 1000 \				// 横幅、縦幅
			-rot	3072 3072 0 \					// 回転
			-R		2 \						// 逃げ込みエリア数
				-E  -500 1000 500 500 \		// 中継地点
				-e  -1000 1500 500 500 \		// 最終地点
				-E -500 1000 500 500 \		//
				-e -1000 1500 500 500		//-R個
//			-debug

		@フナ虫子分設定全体 ゴールド:51 600 -600 400 -400

		// 西側のフナ虫
		chara フナ虫リーダー ゴールドリーダー:07 \
			-n_obj	$f:ローカルフラグ１ \
			-obj	d:PLAYER \
			-pos	-3750 -46000 27250 \	// 中心
			-wide	1000 1500 \				// 横幅、縦幅
			-rot	0 0 0 \					// 回転
			-R		2 \						// 逃げ込みエリア数
				-E  500 -1000 500 500 \		// 中継地点
				-e  1000 -1500 500 500 \		// 最終地点
				-E 500 1000 500 500 \		//
				-e 1000 1500 500 500		//-R個
//			-debug

		@フナ虫子分設定片側 ゴールド:61 100 200 300 400 50 150 250 350

		// 西側壁のフナ虫
		chara フナ虫リーダー ゴールドリーダー:08 \
			-n_obj	$f:ローカルフラグ１ \
			-obj	d:PLAYER \
			-flag	d:ALLSLT_FLAG_NO_STOMP \
			-pos	-2750 -45000 27250 \	// 中心
			-wide	1500 1000 \				// 横幅、縦幅
			-rot	3072 1024 0 \					// 回転
			-R		2 \						// 逃げ込みエリア数
				-E  -500 1000 500 500 \		// 中継地点
				-e  -1000 1500 500 500 \		// 最終地点
				-E -500 1000 500 500 \		//
				-e -1000 1500 500 500		//-R個
//			-debug

		@フナ虫子分設定全体 ゴールド:71 300 -300 400 -400

	}

	proc フナ虫子分設定全体 $:名前 $:値１ $:値２ $:値３ $:値４ {
		chara 舟虫 $:名前 \
			-group 16 \
			-in_group 16 \
				-data  $:値１ $:値２ 600 \
				-data  $:値２ $:値１ 600 \
				-data  $:値１ $:値３ 600 \
				-data  $:値３ $:値１ 600 \
				-data  $:値１ $:値４ 600 \
				-data  $:値４ $:値１ 600 \
				-data  $:値２ $:値３ 600 \
				-data  $:値３ $:値２ 600 \
				-data  $:値２ $:値４ 600 \
				-data  $:値４ $:値２ 600 \
				-data  $:値３ $:値４ 600 \
				-data  $:値４ $:値３ 600 \
				-data  $:値１ $:値１ 600 \
				-data  $:値２ $:値２ 600 \
				-data  $:値３ $:値３ 600 \
				-data  $:値４ $:値４ 600
	}

	proc フナ虫子分設定片側 $:名前 $:値Ｘ１ $:値Ｘ２ $:値Ｘ３ $:値Ｘ４ $:値Ｚ１ $:値Ｚ２ $:値Ｚ３ $:値Ｚ４ {
		chara 舟虫 $:名前 \
			-group 16 \
			-in_group 16 \
				-data  $:値Ｘ１ $:値Ｚ１ 600 \
				-data  $:値Ｘ２ $:値Ｚ１ 600 \
				-data  $:値Ｘ３ $:値Ｚ１ 600 \
				-data  $:値Ｘ４ $:値Ｚ１ 600 \
				-data  $:値Ｘ１ $:値Ｚ２ 600 \
				-data  $:値Ｘ２ $:値Ｚ２ 600 \
				-data  $:値Ｘ３ $:値Ｚ２ 600 \
				-data  $:値Ｘ４ $:値Ｚ２ 600 \
				-data  $:値Ｘ１ $:値Ｚ３ 600 \
				-data  $:値Ｘ２ $:値Ｚ３ 600 \
				-data  $:値Ｘ３ $:値Ｚ３ 600 \
				-data  $:値Ｘ４ $:値Ｚ３ 600 \
				-data  $:値Ｘ１ $:値Ｚ４ 600 \
				-data  $:値Ｘ２ $:値Ｚ４ 600 \
				-data  $:値Ｘ３ $:値Ｚ４ 600 \
				-data  $:値Ｘ４ $:値Ｚ４ 600
	}


	// 潜水艇
	// ------------------------------------
	proc 潜水艇設置 {
		#if d:DEBUG_PRINT
			print 'w11a0_submarine_set'
		#endif

		chara プットオブジェ 潜水艇 -m w11a0_sbmr -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME
		chara プットオブジェ 潜水艇窓 -m w11a0_speobj2_mt -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME -f 0x0020
	}


	// 解体Ｃ４設置
	// ------------------------------------
	proc 解体Ｃ４設置 {
		#if d:DEBUG_PRINT
			print 'w11a0_bomb_set'
		#endif

		if ( $w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１終了 ) {
			// 最初はプットオブジェにして反応させない
			if ( $w:ゲーム設定 <= d:LEVEL_NORMAL ) {
				chara プットオブジェ 解体Ｃ４ -m c4_kaitai_b1 -r 1024,1024,0 -p -10000,-43800,25200 -s 100,100,100 -l d:LT2_NAME
			} else if ( $w:ゲーム設定 == d:LEVEL_HARD ){
				chara プットオブジェ 解体Ｃ４ -m c4_kaitai_b1 -r 150,2048,0 -p -9070,-43420,23980 -s 100,100,100 -l d:LT2_NAME
			} else {
				chara プットオブジェ 解体Ｃ４ -m c4_kaitai_b1 -r 0,1024,0 -p -20250,-42750,24875 -s 100,100,100 -l d:LT2_NAME
			}
		} else if ( $w:p_story >= d:ST:P026_01_R01爆弾解体終了１無線デモ１終了 ){
			if ( $w:ゲーム設定 <= d:LEVEL_NORMAL ) {
				chara 解体Ｃ４ 解体Ｃ４ \
					-position -10000,-43800,25200 \
					-rotate 1024,1024,0 \
					-type 1 \
					-state 1 \
					-proc 爆弾解体終了
			} else if ( $w:ゲーム設定 == d:LEVEL_HARD ){
				chara 解体Ｃ４ 解体Ｃ４ \
					-position -9070,-43420,23980 \
//					-position -9810,-43800,23423 \
					-rotate 150,2048,0 \
					-type 1 \
					-state 1 \
					-proc 爆弾解体終了
			} else {
				chara 解体Ｃ４ 解体Ｃ４ \
					-position -20250,-42750,24875 \
					-rotate 0,1024,0 \
					-type 1 \
					-state 1 \
					-proc 爆弾解体終了
			}
		} else {
			if ( $w:ゲーム設定 <= d:LEVEL_NORMAL ) {
				chara 解体Ｃ４ 解体Ｃ４ \
					-position -10000,-43800,25200 \
					-rotate 1024,1024,0 \
					-type 1 \
					-state 0 \
					-proc 爆弾解体終了
			} else if ( $w:ゲーム設定 == d:LEVEL_HARD ){
				chara 解体Ｃ４ 解体Ｃ４ \
					-position -9070,-43420,23980 \
//					-position -9810,-43800,23423 \
					-rotate 150,2048,0 \
					-type 1 \
					-state 0 \
					-proc 爆弾解体終了
			} else {
				chara 解体Ｃ４ 解体Ｃ４ \
					-position -20250,-42750,24875 \
					-rotate 0,1024,0 \
					-type 1 \
					-state 0 \
					-proc 爆弾解体終了
			}
		}
	}

	proc 爆弾解体終了 {
		#if d:DEBUG_PRINT
			print 'bomb_event_start'
		#endif

		// 凍結時に実行
		if ( $2 == 1 ) {
			mesg タイマー 爆弾タイマー 表示切り替え 0
			eval( $i:プレイヤー初期Ｘ位置 = $i:プレイヤー位置Ｘ ) ;
			eval( $i:プレイヤー初期Ｙ位置 = $i:プレイヤー位置Ｙ ) ;
			eval( $i:プレイヤー初期Ｚ位置 = $i:プレイヤー位置Ｚ ) ;
			eval( $i:プレイヤー初期方向 = $w:プレイヤー方向 )
			eval( $b:プレイヤー初期姿勢 = $w:プレイヤー姿勢 ) ;
			eval( $s:登場ポイント = 爆弾解体終了１無線デモ１終了後 );
			command 全体マップ爆弾配置解除 -bomb d:Ａ脚マップ
			@昇降機ホール揺れフラグチェック			// w11abc_common.h
			@rt_P026_01_R01爆弾解体終了１無線デモ１
			eval ( $w:p_story = d:ST:P026_01_R01爆弾解体終了１無線デモ１終了 );
			// 無効にしていたロードプロックと無線用プロックを起動
			command トラップ切り替え -trap ev010 -switch 1
			command トラップ切り替え -trap ld002 -switch 1

		} else if ( $2 == 0 ) {
			@爆発ゲームオーバー処理 d:BOMB_EFFECT_ON

		}
	}


	// 強制モーション用トラップ
	// ------------------------------------
	// ビヨンド設定
	// ------------------------------------
	proc 発着口ビヨンド設定 {
		#if d:DEBUG_PRINT
			print 'w11a0_beyond_set'
		#endif

		// 一番北側
		trap by001 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \		//アクションボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@南向きビヨンド発動 by001 21000 $4 -14750 -9250 0x00000002
			}
		trap by001 d:PLAYER \
			-mask いる \
			-state 6 \
			-dir 0 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,-44000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 475,1358,0 -f 3539 \
					-a 200 \
					-i 0 -1 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s 1
			}
		trap by001 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 2048 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b -15066,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,-44500,d:CAM_MAX \
					-l -13252,-46000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 220,1838,0 -f 6073 \
					-a 250 \
					-i 3 0 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// 南側右
		trap by002 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \		//アクションボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@北向きビヨンド発動 by002 27000 $4 -9750 -5250 0x00000002
			}
		trap by002 d:PLAYER \
			-mask いる \
			-state 6 \
			-dir 2048 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,-42500,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,28851 \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,27338 \
					-r 774,1662,0 -f 3100 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1
			}
		trap by002 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 0 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,-44000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,27685 \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,26793 \
					-r 956,1969,0 -f 7295 \
					-a 250 \
					-i 3 2 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// 南側左
		trap by003 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \		//アクションボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@北向きビヨンド発動 by003 27000 $4 -18250 -13750 0x00000002
			}
		trap by003 d:PLAYER \
			-mask いる \
			-state 6 \
			-dir 2048 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,-42500,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,28851 \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,27338 \
					-r 656,2323,0 -f 3100 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1
			}
		trap by003 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 0 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,-44000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,27685 \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,26793 \
					-r 959,2121,0 -f 7295 \
					-a 250 \
					-i 3 2 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// 梯子左
		trap by004 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \		//アクションボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@南向きビヨンド発動 by004 23000 $4 -18250 -17250 0x00000002
			}
		trap by004 d:PLAYER \
			-mask いる \
			-state 6 \
			-dir 0 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,-44000,26262 d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,23024 d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 520,2244,0 -f 4855 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1
			}
		trap by004 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 2048 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,-44500,d:CAM_MIN d:CAM_MAX,-44000,d:CAM_MAX \
					-l d:CAM_MIN,-460000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 231,2146,0 -f 4144 \
					-a 225 \
					-i 3 2 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// 梯子右
		trap by005 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \		//アクションボタン
			-state 0,4 \	//立ち状態or張り付き
			-dir 0,256 2048,256 \	//方向指定
			-exec {
				@南向きビヨンド発動 by005 23000 $4 -16250 -14750 0x00000002
			}
		trap by005 d:PLAYER \
			-mask いる \
			-state 6 \
			-dir 0 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,-44000,25881 d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,23220 d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 565,1838,0 -f 4335 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1
			}
		trap by005 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 2048 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,-44000,d:CAM_MIN d:CAM_MAX,-44000,d:CAM_MAX \
					-l d:CAM_MIN,-46000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 245,1749,0 -f 4599 \
					-a 225 \
					-i 3 2 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// 梯子右(南北方向)
		trap by006 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \		//アクションボタン
			-state 0,4 \				//立ち状態or張り付き
			-dir 1024,256 3072,256 \	//方向指定
			-exec {
				@東向きビヨンド発動 by006 -14750 $6 21000 23000 0x00000002
			}
		trap by006 d:PLAYER \
			-mask いる \
			-state 6 \
			-dir 1024 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,-45000,d:CAM_MIN d:CAM_MAX,-44500,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 50,2494,0 -f 4902 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1
			}
		trap by006 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 3072 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,-44500,d:CAM_MAX \
					-l d:CAM_MIN,-46000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 166,2649,0 -f 4898 \
					-a 250 \
					-i 3 2 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}
	}


	// 水中の設定
	// ------------------------------------
//エルード時は体半分なので短め
#define		CHARA_SPLASH_TIME2		180

	proc エルード時の体水飛沫設定 {
		#if d:DEBUG_PRINT
			print 'elude_body_water_set'
		#endif

		
		trap by001 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 2048 256 \	//方向指定
			-exec {
				if ( $3 == 出る ) {
					if ( $5 >= -46400 ) {
						chara キャラ付随水飛抹 出水飛沫ライデン \
							-nconcname d:PLAYER \
							-addone 32 \
							-time d:CHARA_SPLASH_TIME2 \
							-rgbadd 0x40404030 \
							-gbsub 0x40404030
					}
				} else {
					mesg キャラ付随水飛抹 出水飛沫ライデン 破棄
				}
			}

		trap by002 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 0 256 \	//方向指定
			-exec {
				if ( $3 == 出る ) {
					if ( $5 >= -46400 ) {
						chara キャラ付随水飛抹 出水飛沫ライデン \
							-nconcname d:PLAYER \
							-addone 32 \
							-time d:CHARA_SPLASH_TIME2 \
							-rgbadd 0x40404030 \
							-gbsub 0x40404030
					}
				} else {
					mesg キャラ付随水飛抹 出水飛沫ライデン 破棄
				}
			}

		trap by003 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 0 256 \	//方向指定
			-exec {
				if ( $3 == 出る ) {
					if ( $5 >= -46400 ) {
						chara キャラ付随水飛抹 出水飛沫ライデン \
							-nconcname d:PLAYER \
							-addone 32 \
							-time d:CHARA_SPLASH_TIME2 \
							-rgbadd 0x40404030 \
							-gbsub 0x40404030
					}
				} else {
					mesg キャラ付随水飛抹 出水飛沫ライデン 破棄
				}
			}

		trap by004 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 2048 256 \	//方向指定
			-exec {
				if ( $3 == 出る ) {
					if ( $5 >= -46400 ) {
						chara キャラ付随水飛抹 出水飛沫ライデン \
							-nconcname d:PLAYER \
							-addone 32 \
							-time d:CHARA_SPLASH_TIME2 \
							-rgbadd 0x40404030 \
							-gbsub 0x40404030
					}
				} else {
					mesg キャラ付随水飛抹 出水飛沫ライデン 破棄
				}
			}

		trap by005 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 2048 256 \	//方向指定
			-exec {
				if ( $3 == 出る ) {
					if ( $5 >= -46400 ) {
						chara キャラ付随水飛抹 出水飛沫ライデン \
							-nconcname d:PLAYER \
							-addone 32 \
							-time d:CHARA_SPLASH_TIME2 \
							-rgbadd 0x40404030 \
							-gbsub 0x40404030
					}
				} else {
					mesg キャラ付随水飛抹 出水飛沫ライデン 破棄
				}
			}


		trap by006 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 3072 256 \	//方向指定
			-exec {
				if ( $3 == 出る ) {
					if ( $5 >= -46400 ) {
						chara キャラ付随水飛抹 出水飛沫ライデン \
							-nconcname d:PLAYER \
							-addone 32 \
							-time d:CHARA_SPLASH_TIME2 \
							-rgbadd 0x40404030 \
							-gbsub 0x40404030
					}
				} else {
					mesg キャラ付随水飛抹 出水飛沫ライデン 破棄
				}
			}

	}


	// 敵兵が水に落ちたときに飛沫と音が出る指定
	// ------------------------------------
	proc 敵兵水落ち設定 {
		#if d:DEBUG_PRINT
			print 'enemy_water_sound_set'
		#endif
		// 水飛沫はあらかじめ登録してオフしておく
		chara 水面監視水飛沫 落下水飛沫 -n d:PLAYER
	}


	proc 発着口水面設定 {
		#if d:DEBUG_PRINT
			print 'water_set'
		#endif

		// 水エフェクト関連
		chara 通路用水面 キラキラ水面 \
			-tex water w11a0_ref_water_alp_ovl \
			-scale 170 \
			-num 2,1 \
			-pos -11750, d:WATER_SURFACE, 24000 \
			-wave 30 170 12 \
			-alpha_blend \ // 強制５０％半透明描画
			-force 10 

		chara プットオブジェ 写りこみモデル壁 -m w11a0_wall_ref_water -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME
		chara プットオブジェ 写りこみモデル潜水艇 -m w11a0_sbmr_ref_water -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME

		chara アニメテクスチャセット 水面写りこみ \
			-tex w11a w11_sbmr_ref_add_alp_ovl \
			-anmtex w11 \
			-interval 2 \
			-flag 0x0001

	}

	proc 発着口水中モードプレイヤー連動キャラ設定 {
		// 以下プレイヤーと水との連動関係(デモではとる)
		if($s:d_num == not_demo) {
			@水中モードプレイヤー連動キャラ設定 wa100

			// アイテム煙草の制御
			trap wa100 d:PLAYER \
				-mask 入る \
				-exec {
					if ( $w:アイテム == d:アイテム:煙草 ) {
						eval($w:アイテム = d:アイテム:素手);
					}
				}

			// 波と動きの連動
			trap wa000 ？ \
				-mask ？ \
				-exec {
					if ( $3 == 入る ) {
						mesg 通路用水面 キラキラ水面 波発生源追加 $2
					} else {
						mesg 通路用水面 キラキラ水面 波発生源削除 $2
					}
				}

			// 強制モーション経由の水中エントリー処理
			// 梯子
			trap wa001 d:PLAYER \
				-mask 入る -dir 0,512 -state d:TRP_STATE_STAND -key d:PAD_X\
				-exec {
					if (`command ゲームオーバーチェック` == 0 ) {
						chara カメラ設定 モーションカメラ \
							-c 0 \
							-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
							-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
							-r 573,2590,0 -f 4429 \
							-a 200 \
							-i 0 3 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s 1

						chara delay ディレイ \
							-time 150 \
							-exec {
								chara カメラ設定 モーションカメラ -s -1
							}

						@入水処理１
					}
				}
			trap wa001 d:PLAYER \
				-mask 入る -dir 2048,512 -state d:TRP_STATE_WATER -key d:PAD_X \
				-exec {
					if (`command ゲームオーバーチェック` == 0 ) {
						chara カメラ設定 モーションカメラ \
							-c 0 \
							-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
							-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
							-r 177,2721,0 -f 3741 \
							-a 200 \
							-i 0 3 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s 1

						chara delay ディレイ \
							-time 150 \
							-exec {
								chara カメラ設定 モーションカメラ -s -1
							}

						@出水処理１
					}
				}

			// 飛び込み
			trap wa002 d:PLAYER \
				-mask 入る -dir 1024,512 \
				-state d:TRP_STATE_STAND \
				-exec {
					if (`command ゲームオーバーチェック` == 0 ) {
						chara カメラ設定 モーションカメラ \
							-c 0 \
							-b -15845,-43704,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,27625 \
							-l -17333,-47487,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,25194 \
							-r 603,2406,0 -f 4737 \
							-a 200 \
							-i 0 3 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s 1

						chara delay ディレイ \
							-time 120 \
							-exec {
								chara カメラ設定 モーションカメラ -s -1
							}

						// 音声と振動はdelayで
						chara delay ディレイ \
							-time 51 \
							-exec {
								command ＳＥセットモード -s d:se_code:SD_P_INWTERL1 \
									-p $i:プレイヤー位置Ｘ,$i:プレイヤー位置Ｙ,$i:プレイヤー位置Ｚ -m 0

								chara パッド振動 飛び込み振動 \
									-vibfile rai_dive_03
							}
						
						@飛び込み処理 1024 -18500 -44951 $i:プレイヤー位置Ｚ
					}
				}

			trap wa003 d:PLAYER \
				-mask 入る -dir 2048,512 \
				-state d:TRP_STATE_STAND \
				-exec {
					if (`command ゲームオーバーチェック` == 0 ) {
						chara カメラ設定 モーションカメラ \
							-c 0 \
							-b -9283,-43728,27755 d:CAM_MAX,d:CAM_MAX,29684 \
							-l -12702,-46320,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,26717 \
							-r 258,2606,0 -f 4906 \
							-a 200 \
							-i 0 3 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s 1

						chara delay ディレイ \
							-time 120 \
							-exec {
								chara カメラ設定 モーションカメラ -s -1
							}

						// 音声と振動はdelayで
						chara delay ディレイ \
							-time 51 \
							-exec {
								command ＳＥセットモード -s d:se_code:SD_P_INWTERL1 \
									-p $i:プレイヤー位置Ｘ,$i:プレイヤー位置Ｙ,$i:プレイヤー位置Ｚ -m 0

								chara パッド振動 飛び込み振動 \
									-vibfile rai_dive_03
							}

						@飛び込み処理 2048 $i:プレイヤー位置Ｘ -45201 27250
					}
				}

			trap wa004 d:PLAYER \
				-mask 入る -dir 3072,512 \
				-state d:TRP_STATE_STAND \
				-exec {
					if (`command ゲームオーバーチェック` == 0 ) {
						chara カメラ設定 モーションカメラ \
							-c 0 \
							-b d:CAM_MIN,-43000,d:CAM_MIN -7026,d:CAM_MAX,29254 \
							-l d:CAM_MIN,-46125,d:CAM_MIN -5652,d:CAM_MAX,26046 \
							-r 368,1791,0 -f 4236 \
							-a 200 \
							-i 0 3 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s 1

						chara delay ディレイ \
							-time 120 \
							-exec {
								chara カメラ設定 モーションカメラ -s -1
							}

						// 音声と振動はdelayで
						chara delay ディレイ \
							-time 51 \
							-exec {
								command ＳＥセットモード -s d:se_code:SD_P_INWTERL1 \
									-p $i:プレイヤー位置Ｘ,$i:プレイヤー位置Ｙ,$i:プレイヤー位置Ｚ -m 0

								chara パッド振動 飛び込み振動 \
									-vibfile rai_dive_03
							}

						@飛び込み処理 3072 -5000 -44951 $i:プレイヤー位置Ｚ
					}
				}

			trap wa005 d:PLAYER \
				-mask 入る -dir 0,512 \
				-state d:TRP_STATE_STAND \
				-exec {
					if (`command ゲームオーバーチェック` == 0 ) {
						chara カメラ設定 モーションカメラ \
							-c 0 \
							-b d:CAM_MIN,-43286,26500 -3850,-42908,27128 \
							-l d:CAM_MIN,-47000,23039 -7125,-44409,24911 \
							-r 199,2542,0 -f 4996 \
							-a 200 \
							-i 0 3 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s 1

						chara delay ディレイ \
							-time 120 \
							-exec {
								chara カメラ設定 モーションカメラ -s -1
							}

						// 音声と振動はdelayで
						chara delay ディレイ \
							-time 57 \
							-exec {
								command ＳＥセットモード -s d:se_code:SD_P_INWTERL1 \
									-p $i:プレイヤー位置Ｘ,$i:プレイヤー位置Ｙ,$i:プレイヤー位置Ｚ -m 0

								chara パッド振動 飛び込み振動 \
									-vibfile rai_dive_03
							}

						@飛び込み処理 0 $i:プレイヤー位置Ｘ -43951 22750
					}
				}


			#ifdef d:STAGE_W11A
			// マスクから出る泡(潜水服専用)
				chara ライデン水中マスクの泡 水中マスク泡 -n d:PLAYER
				mesg ライデン水中マスクの泡 水中マスク泡 オフ
				trap wa000 d:PLAYER \
					-mask ？ \
					-exec {
						if ( $3 == 入る ) {
							mesg ライデン水中マスクの泡 水中マスク泡 オン
						} else {
							mesg ライデン水中マスクの泡 水中マスク泡 オフ
						}
					}
			#endif
		} else {
			// デモで使用する波紋
//			chara 泡で波紋管理 NewRipBubbleMan
		}
	} 

	proc 入水処理１ {
		mesg プレイヤー d:PLAYER water 入る \
			d:モーションリスト:non_ladder_down_poolside \
			0 -16750 (23000-500) (-46000+926)

		chara delay ディレイ -time 64 -exec {
			mesg キャラ付随水飛抹 出水飛沫ライデン 破棄
		}
	}
	proc 出水処理１ {
		chara キャラ付随水飛抹 出水飛沫ライデン \
			-nconcname d:PLAYER \
			-addone 32 \
			-time d:CHARA_SPLASH_TIME \
			-rgbadd 0x40404030 \
			-gbsub 0x40404030
		mesg プレイヤー d:PLAYER water 出る \
			d:モーションリスト:non_ladder_up_poolside \
			2048 (-16750+1) (23000+660) (-46000-1699)
		mesg プレイヤー d:PLAYER water 出る 0 2048
	}

	proc 飛び込み処理 $:方向 $:プレイヤーＸ $:プレイヤーＹ $:プレイヤーＺ {
		chara delay ディレイ -time 49 -exec {
			mesg キャラ付随水飛抹 出水飛沫ライデン 破棄
		}
		mesg プレイヤー d:PLAYER water 入る d:モーションリスト:non_swim_dive $:方向 \
			$:プレイヤーＸ $:プレイヤーＺ $:プレイヤーＹ 
	}

	// 各種カメラの設定
	// ------------------------------------
	proc 発着口カメラ設定 {
		#if d:DEBUG_PRINT
			print 'w11a0_camera_set'
		#endif

		// 階段下のエリア全体
		trap ca001 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {
				@CS_スムーズカメラ昇順 $6 18000 21000 -18750 -17315
				eval( $i:補完ＢＸ = $i:CS_結果１ );
				eval( $i:補完ＬＸ = $i:CS_結果１ );

				chara カメラ設定 メインカメラ \
					-c 0 \	
					-b $i:補完ＢＸ,-42662,d:CAM_MIN -6179,-38750,33587 \
					-l $i:補完ＬＸ,-44625,d:CAM_MIN -6179,d:CAM_MAX,27391 \
					-r 200,2048,0 \
					-f 6500 \
					-a 200 \
					-i 2 -1 0 0 \
					-s $3
			}

		// 階段下右
		trap ca002 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 0 \	
					-b -19000,d:CAM_MIN,d:CAM_MIN -4500,-44250,29023 \
					-l -19000,d:CAM_MIN,d:CAM_MIN -4500,d:CAM_MAX,28500 \
					-r 40,2048,0 \
					-f 6500 \
					-a 200 \
					-i 2 -1 0 0 \
					-s $3
			}

		// 金網内
		trap ca003 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 0 \	
					-b -10000,-38996,23146 -7630,-38996,24981 \
					-l -10000,-44560,19789 -7630,-44560,21623 \
					-r 670,2048,0 \
					-f 6500 \
					-a 200 \
					-i 2 -1 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
			}

		// 右側奥
		trap ca004 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {
				@CS_スムーズカメラ昇順 $6 19000 22000 300 791
				eval( $i:補完ＲＸ = $i:CS_結果１ );

				chara カメラ設定 メインカメラ \
					-c 0 \	
					-b d:CAM_MIN,-40450,23802 -4250,-40450,23802 \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN -4250,-40450,23302 \
					-r $i:補完ＲＸ,2048,0 \
					-f 3500 \
					-a 200 \
					-i 0 0 0 0 \
					-s $3
			}

		// 階段
		trap ca005 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {
				@CS_スムーズカメラ降順 $6 19500 18500 28000 23500
				eval( $i:補完ＢＺ = $i:CS_結果１ );

				@CS_スムーズカメラ降順 $6 19500 18500 200 160
				eval( $i:補完ＲＸ = $i:CS_結果１ );

				chara カメラ設定 サブカメラ \
					-c 0 \	
					-b d:CAM_MIN,d:CAM_MIN,22500 d:CAM_MAX,-40250,$i:補完ＢＺ \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,23000 \
					-r $i:補完ＲＸ,2048,0 \
					-f 6500 \
					-a 200 \
					-i 3 3 0 0 \
					-s $3
			}

		// 階段上
		trap ca006 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {
				@CS_スムーズカメラ昇順 $4 -10000 -7000 160 385
				eval( $i:補完ＲＸ = $i:CS_結果１ );

				chara カメラ設定 メインカメラ \
					-c 0 \	
					-b d:CAM_MIN,d:CAM_MIN,22500 d:CAM_MAX,-40250,23500 \
					-l d:CAM_MIN,-44500,d:CAM_MIN d:CAM_MAX,-44500,23500 \
					-r $i:補完ＲＸ,2048,0 \
					-f 6500 \
					-a 200 \
					-i 2 -1 0 0 \
					-s $3
			}

		// 水面のエリア
		trap ca007 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 0 \	
					-b -17500,-44000,28000 -5750,-43750,28000 \
					-l -17500,d:CAM_MIN,d:CAM_MIN -5750,d:CAM_MAX,27000 \
					-r 600,2048,0 \
					-f 6500 \
					-a 200 \
					-i 2 -1 0 0 \
					-s $3
			}

		// プール北手すり張り付き時
		trap ca101 d:PLAYER \
			-mask ？ \
			-dir 2048,256 \
			-state d:TRP_STATE_CAUTION \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ \
					-c 0 \	
					-b d:CAM_MIN,d:CAM_MIN,22250 d:CAM_MAX,d:CAM_MAX,22250 \
					-l d:CAM_MIN,d:CAM_MIN,21250 d:CAM_MAX,d:CAM_MAX,21250 \
					-r 924,2048,0 \
					-f 4000 \
					-a 200 \
					-i 0 0 0 0 \
					-s $3
			}

		// プール北西手すり張り付き時
		trap ca102 d:PLAYER \
			-mask ？ \
			-dir 3072,128 \
			-state d:TRP_STATE_CAUTION \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ \
					-c 1 \	
					-p -14332,-41081,23661 -t -14332,-44751,22054 \
					-r 755,2048,0 -f 4006 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}

		// プール梯子付近手すり張り付き時
		trap ca103 d:PLAYER \
			-mask ？ \
			-dir 2048,128 \
			-state d:TRP_STATE_CAUTION \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ \
					-c 0 \	
					-b d:CAM_MIN,d:CAM_MIN,23750 d:CAM_MAX,d:CAM_MAX,23750 \
					-l d:CAM_MIN,d:CAM_MIN,22750 d:CAM_MAX,d:CAM_MAX,22750 \
					-r 924,2048,0 \
					-f 4000 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}

		// プール南手すり張り付き時
		trap ca104 d:PLAYER \
			-mask ？ \
			-dir 0,256 \
			-state d:TRP_STATE_CAUTION \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ \
					-c 0 \	
					-b d:CAM_MIN,d:CAM_MIN,27000 d:CAM_MAX,d:CAM_MAX,27000 \
					-l d:CAM_MIN,d:CAM_MIN,26750 d:CAM_MAX,d:CAM_MAX,26750 \
					-r 950,2048,0 \
					-f 4000 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}

		// 南壁張り付き時
		trap ca105 d:PLAYER \
			-mask ？ \
			-dir 2048,256 \
			-state d:TRP_STATE_CAUTION \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ \
					-c 0 \	
					-b -18000,d:CAM_MIN,28329 -5250,d:CAM_MAX,28329 \
					-l -18000,d:CAM_MIN,27847 -5250,d:CAM_MAX,27847 \
					-r 969,2048,0 \
					-f 4500 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}

		// 発着口上段手すり南側左
		trap ca106 d:PLAYER \
			-mask ？ \
			-dir 2048,256 \
			-state d:TRP_STATE_CAUTION \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ \
					-c 0 \	
					-b d:CAM_MIN,d:CAM_MIN,19522 -17681,d:CAM_MAX,19522 \
					-l d:CAM_MIN,d:CAM_MIN,18770 -17681,d:CAM_MAX,18770 \
					-r 796,2048,0 \
					-f 2195 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}

		// 発着口上段手すり南側右
		trap ca107 d:PLAYER \
			-mask ？ \
			-dir 2048,256 \
			-state d:TRP_STATE_CAUTION \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ \
					-c 0 \	
					-b d:CAM_MIN,d:CAM_MIN,19589 d:CAM_MAX,d:CAM_MAX,19589 \
					-l d:CAM_MIN,d:CAM_MIN,18583 d:CAM_MAX,d:CAM_MAX,18583 \
					-r 796,2048,0 \
					-f 2938 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}

		// 発着口上段手すり西側
		trap ca108 d:PLAYER \
			-mask ？ \
			-dir 1024,256 \
			-state d:TRP_STATE_CAUTION \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ \
					-c 1 \	
					-p -19548,-42763,20843 -t -18208,-45764,15240 \
					-r 313,1895,0 -f 6496 \
					-a 188 \
					-i 2 2 0 0 \
					-s $3
			}

	}


// 連結ハッチ
	// 天井設置
	// ------------------------------------
	proc 連結ハッチ天井設置 {
		#if d:DEBUG_PRINT
			print 'w11a1_ceil_set'
		#endif

		chara 天井君 連結ハッチ天井 -m d:CEIL_NAME1 -h -42000 -p 0,0,0 -l d:LT2_NAME
	}

	// 各種カメラの設定
	// ------------------------------------
	proc 連結ハッチカメラ設定 {
		#if d:DEBUG_PRINT
			print 'w11a1_camera_set'
		#endif

		trap ca011 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 0 \	
					-b d:CAM_MIN,-43000,d:CAM_MIN d:CAM_MAX,-43000,d:CAM_MAX \
					-l -11750,-43650,d:CAM_MIN -11750,d:CAM_MAX,d:CAM_MAX \
					-r 55,2048,0 \
					-f 6500 \
					-a 180 \
					-i 3 -1 0 0 \
					-s $3
			}

		trap ca012 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 サブカメラ２ \
					-c 0 \	
					-b -11316,-39467,4286 -8474,-39467,4286 \
					-l -11316,-43497,2993 -8474,-43497,2993 \
					-r 821,2048,0 \
					-f 4239 \
					-a 200 \
					-i 0 0 0 0 \
					-z d:CAMERA_NO_CUSHION \
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

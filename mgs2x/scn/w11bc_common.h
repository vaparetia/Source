/*
	w11bc_common.h                     
	    海底ドック（w11b,w11c共通プロック/壊れ物オブジェ）

	2001/04/19 H.Yoshiike         
	$Id: w11bc_common.h,v 1.23 2002/08/16 07:04:52 usr03682 Exp $                      

	
*/


// 各proc設定
//-------------------------------------------------------------------------------------
// 各マップ別設定
// ------------------------------------
// 昇降機ホール
	// 壊れ天井
	// ------------------------------------
	proc 天井崩れ {
		#if d:DEBUG_PRINT
		        print 'w11c_boss_ceiling'
		#endif
		chara フォーチュン戦天井崩れ 天井崩れ \
			-model  w11c2_ceiling1 w11c2_ceiling1_brk1 \
			-pos    0,0,0 \
			-proc   ゲーム終了のカット \
			-break  w11c2_hari1, w11c_ceil,0  -1750,-40499,1996 \
			-break  w11c2_hari2, w11c_ceil,1   -100,-40500,2000 \
			-break  w11c2_hari3, w11c_ceil,2   -350,-40000,1500 \
			-aim    -300, -41000, 0 \
			-aim    -500, -41000, 0 \
			-aim   -1000, -41000, 0 
	}

	// フォーチュン戦終了後のもの（ないとmakeが通らないためここにおく）
	proc ゲーム終了のカット {
		#if d:DEBUG_PRINT
		        print 'w11c_boss_gameend_cut'
		#endif
	}

	// 荷物オブジェクト設置
	// ------------------------------------
	proc 昇降機ホールオブジェ設置 $:フラグ {
		#if d:DEBUG_PRINT
		        print 'w11c_object_set'
		#endif

		chara フォーチュン戦ライデン隠れオブジェ 鉄と木箱群 \
			-light d:LT2_NAME \
			-flag  $:フラグ \
			-type (d:FRT_CONTAINER1A) \
				 -move 1 \
				 -proc 下段前列一番右の箱破壊後処理 \
				 -phase 0x0410 -rot  170 -pos  2080,-45000,-2500 \ /*前列下*/
			-type (d:FRT_WOODEN_1M   | d:FRT_UPPER_DECK | d:FRT_SHADOW1) \
				 -proc 上段前列一番右の箱破壊後処理 \
				 -phase 0x0400 -rot  -70 -pos  2000,-44000,-2550 \ /*前列上*/
			-type (d:FRT_CONTAINER1A | d:FRT_SHADOW2) \
				 -move 1 \
				 -proc 下段後列一番右の箱破壊後処理 \
				 -phase 0x0410 -rot  -60 -pos  2000,-45000,-1500 \ /*前列下*/
			-type (d:FRT_CONTAINER1A | d:FRT_UPPER_DECK | d:FRT_SHADOW3) \ /*前列上*/
				 -proc 上段後列一番右の箱破壊後処理 \
				 -phase 0x0400 -rot   60 -pos  2040,-44000,-1470 \
			-type (d:FRT_CONTAINER1A | d:FRT_SHADOW2) \
				 -proc 下段前列右から２番目の箱破壊後処理 \
				 -phase 0x0410 -rot   70 -pos  1000,-45000,-2500 \ /*前列下*/
			-type (d:FRT_CONTAINER1A) \
				 -move 1 \
				 -proc 下段後列一番左の箱破壊後処理 \
				 -phase 0x0410 -rot   30 -pos  1000,-45000,-1500 \ /*前列下*/
			-type (d:FRT_WOODEN_1M   | d:FRT_UPPER_DECK | d:FRT_SHADOW1) \ /*前列上*/
				 -proc 上段後列一番左の箱破壊後処理 \
				 -phase 0x0400 -rot    0 -pos  1000,-44000,-1500 \
			-type (d:FRT_CONTAINER1A) \
				 -move 1 \
				 -proc 下段前列中央の箱破壊後処理 \
				 -phase 0x0410 -rot  100 -pos    30,-45000,-2500 \ /*前列下*/
			-type (d:FRT_WOODEN_1M | d:FRT_UPPER_DECK | d:FRT_SHADOW1) \ /*前列上*/
				 -proc 上段前列中央の箱破壊後処理 \
				 -phase 0x0000 -rot   70 -pos     0,-44000,-2500 \
			-type (d:FRT_WOODEN_1M | d:FRT_SHADOW2) \
				 -move 2 \
				 -proc 下段前列左から２番目の箱破壊後処理 \
				 -phase 0x0310 -rot   90 -pos -1050,-45000,-2500 \ /*前列下*/
			-type (d:FRT_CONTAINER1A) \
				 -move 1 \
				 -proc 下段前列一番左の箱破壊後処理 \
				 -phase 0x0400 -rot    0 -pos -2110,-45000,-2500 \ /*前列下*/
			-type (d:FRT_WOODEN_1M   | d:FRT_UPPER_DECK | d:FRT_SHADOW3) \ /*前列上*/
				 -proc 上段前列一番左の箱破壊後処理 \
				 -phase 0x0000 -rot  -80 -pos -1500,-44000,-2500 \

			-type (d:FRT_FORKLIFT_A | d:FRT_SHADOW4 ) \
				 -proc フォークリフト破壊時処理 フォークリフト命中時処理 \
				 -phase 0x0600 -rot   30 -pos -1000,-44900,-1350 \ /*フォークリフト*/

			-type (d:FRT_CONTAINER1H) \
				 -phase 0x1010 -rot   30 -pos -4950,-45000,-3150 \ /*前左側下*/
			-type (d:FRT_CONTAINER1H | d:FRT_SHADOW2) \
				 -phase 0x1010 -rot   40 -pos -5950,-45000,-3150 \
			-type (d:FRT_EXPL_DRUM_LA| d:FRT_SHADOW2) \                /*爆発ドラム缶*/
				 -phase 0x0810 -rot    0 -pos -5300,-44375,-2100 \
			-type (d:FRT_CONTAINER1E | d:FRT_SHADOW1) \
				 -phase 0x0800 -rot   30 -pos -4800,-44000,-3500 \ /*前左側上*/

			-type (d:FRT_CONTAINER1A | d:FRT_ON_ME) \
#ifdef d:STAGE_W11B
				 -move 2 \
#endif
				 -phase 0x0410 -rot    0 -pos -6000,-45000, 1000 \ /*後左側下*/
			-type (d:FRT_CONTAINER1A | d:FRT_SHADOW2) \
#ifdef d:STAGE_W11B
				 -move 2 \
#endif
				 -phase 0x0810 -rot    0 -pos -5500,-45000, 2000 \
#ifdef d:STAGE_W11B
			-type (d:FRT_CONTAINER1A | d:FRT_UPPER_DECK | d:FRT_SHADOW3) \
				 -slide -4500,-45000, 1000 \
				 -phase 0x0800 -rot    0 -pos -6000,-44000, 1000 \ /*後左側上*/
			-type (d:FRT_CONTAINER1A | d:FRT_UPPER_DECK | d:FRT_SHADOW1) \
				 -phase 0x0800 -rot    0 -pos -5500,-44000, 2000 \
#endif
			-type (d:FRT_CONTAINER1A | d:FRT_ON_ME) \
				 -move 4 \
				 -phase 0x0910 -rot    0 -pos  (-500-3000),-45000, 1500 \ /*後列下*/
			-type (d:FRT_CONTAINER1A | d:FRT_SHADOW2) \
				 -move 4 \
				 -phase 0x0910 -rot    0 -pos  (-500-3000),-45000, 2500 \
			-type (d:FRT_CONTAINER1A | d:FRT_SHADOW2 | d:FRT_ON_ME) \
				 -move 4 \
				 -phase 0x0910 -rot    0 -pos  ( 500-3000),-45000, 1500 \
			-type (d:FRT_CONTAINER1A) \
				 -move 4 \
				 -phase 0x0910 -rot    0 -pos  ( 500-3000),-45000, 2500 \
			-type (d:FRT_CONTAINER1A | d:FRT_UPPER_DECK | d:FRT_SHADOW1) \
				 -slide (-1500-2000),-45000, (1500-1000) \
				 -phase 0x0900 -rot    0 -pos  (-500-3000),-44000, 1500 \ /*後列上 前左*/
			-type (d:FRT_CONTAINER1A | d:FRT_UPPER_DECK | d:FRT_SHADOW3) \
				 -phase 0x0900 -rot    0 -pos  (-500-3000),-44000, 2500 \
			-type (d:FRT_CONTAINER1A | d:FRT_UPPER_DECK | d:FRT_SHADOW3) \
				 -slide (1500-3000),-45000, 1500 \
				 -phase 0x0900 -rot    0 -pos ( 500-3000),-44000, 1500 \
			-type (d:FRT_WOODEN_1M   | d:FRT_UPPER_DECK | d:FRT_SHADOW1) \
				 -phase 0x0900 -rot 1024 -pos ( 500-3000),-44000, 2500 \ /* 後列上 後ろ右*/

			-type (d:FRT_CONTAINER1H ) \
				 -phase 0x1010 -rot    0 -pos  4700,-45000,-3500 \ /*前右側下*/
			-type (d:FRT_CONTAINER1H |d:FRT_SHADOW2) \
				 -phase 0x1010 -rot  784 -pos  5820,-45000,-3100 \
//			-type (d:FRT_WOODEN_1M) \
//				 -move 4 \
//				 -phase 0x0810 -rot 1050 -pos  4500,-45000,-2500 \
			-type (d:FRT_EXPL_DRUM_RA| d:FRT_SHADOW2) \
				 -phase 0x1010 -rot    0 -pos  5200,-44375,-2400 \ /*爆発ドラム缶*/
			-type (d:FRT_CONTAINER1E | d:FRT_UPPER_DECK | d:FRT_SHADOW3) \
				 -phase 0x1000 -rot 2048 -pos  4500,-44000,-3500 \ /*前右側上*/
			-type (d:FRT_CONTAINER1H | d:FRT_UPPER_DECK | d:FRT_SHADOW1) \
				 -phase 0x1000 -rot 2088 -pos  5550,-44000,-3450 \
//			-type (d:FRT_WOODEN_1M   | d:FRT_UPPER_DECK | d:FRT_SHADOW1) \
//				 -phase 0x0800 -rot  -30 -pos  4500,-44000,-2480 \
			-type (d:FRT_EXPL_CAN_A | d:FRT_SHADOW2) \
				 -proc 中央右ドラム缶破壊時処理 \
				 -phase 0x0200 -rot 1024 -pos  6000,-44375, 1000 \ /*ドラム缶*/

			-type (d:FRT_EXPL_CAN_A ) \
				 -proc 右下ドラム缶破壊時処理 \
				 -phase 0x0210 -rot  800 -pos  5500,-44375, 4000 \ /*後列ドラム缶*/
			-type (d:FRT_EXPL_CAN_A ) \
				 -phase 0x0200 -rot    0 -pos  2000,-44375, 4500 \
			-type (d:FRT_EXPL_CAN_A ) \
				 -phase 0x0210 -rot    0 -pos -2000,-44375, 4500 \
			-type (d:FRT_CONTAINER2A) \
				 -proc ２Ｍコンテナ破壊時処理 ２Ｍコンテナ命中時処理 \
				 -phase 0x1000 -rot    0 -pos  3000,-45000, 2000   /*後列2M右コンテナ*/
	}

	// 壊れオブジェ破壊時処理
	// ------------------------------------
	proc 下段前列一番左の箱破壊後処理 {
		#if d:DEBUG_PRINT
			print 'w11c_d_f_l1_box_broken'
		#endif

		// フォーチュン戦のときだけアタリチェンジ
		#ifdef d:STAGE_W11C
			@フォークリフト前面荷物チェック
			command 追加ＨＺＸグループ削除 -h 0 -a 7
		#endif
	}

	proc 下段前列左から２番目の箱破壊後処理 {
		#if d:DEBUG_PRINT
			print 'w11c_d_f_l2_box_broken'
		#endif

		// フォーチュン戦のときだけアタリチェンジ
		#ifdef d:STAGE_W11C
			@フォークリフト前面荷物チェック
			command 追加ＨＺＸグループ削除 -h 0 -a 8
		#endif
	}

	proc 下段前列中央の箱破壊後処理 {
		#if d:DEBUG_PRINT
			print 'w11c_d_f_m_box_broken'
		#endif

		// フォーチュン戦のときだけアタリチェンジ
		#ifdef d:STAGE_W11C
			@フォークリフト前面荷物チェック
			command 追加ＨＺＸグループ削除 -h 0 -a 9
		#endif
	}

	proc 下段前列右から２番目の箱破壊後処理 {
		#if d:DEBUG_PRINT
			print 'w11c_d_f_r2_box_broken'
		#endif

		// フォーチュン戦のときだけアタリチェンジ
		#ifdef d:STAGE_W11C
			command 追加ＨＺＸグループ削除 -h 0 -a 10
		#endif
	}

	proc 下段前列一番右の箱破壊後処理 {
		#if d:DEBUG_PRINT
			print 'w11c_d_f_r1_box_broken'
		#endif

		// フォーチュン戦のときだけアタリチェンジ
		#ifdef d:STAGE_W11C
			command 追加ＨＺＸグループ削除 -h 0 -a 11
		#endif
	}

	proc 下段後列一番右の箱破壊後処理 {
		#if d:DEBUG_PRINT
			print 'w11c_d_b_r1_box_broken'
		#endif

		// フォーチュン戦のときだけアタリチェンジ
		#ifdef d:STAGE_W11C
			command 追加ＨＺＸグループ削除 -h 0 -a 12
		#endif
	}

	proc 下段後列一番左の箱破壊後処理 {
		#if d:DEBUG_PRINT
			print 'w11c_d_b_l1_box_broken'
		#endif

		// フォーチュン戦のときだけアタリチェンジ
		#ifdef d:STAGE_W11C
//			command 追加ＨＺＸグループ削除 -h 0 -a 2
			command 追加ＨＺＸグループ削除 -h 0 -a 13
		#endif
	}

	proc 上段前列一番左の箱破壊後処理 {
		#if d:DEBUG_PRINT
			print 'w11c_u_f_l1_box_broken'
		#endif

		// フォーチュン戦のときだけアタリチェンジ
		#ifdef d:STAGE_W11C
			command 追加ＨＺＸグループ削除 -h 0 -a 14
		#endif
	}

	proc 上段前列中央の箱破壊後処理 {
		#if d:DEBUG_PRINT
		        print 'w11c_u_f_m_box_broken'
		#endif

		// フォーチュン戦のときだけアタリチェンジ
		#ifdef d:STAGE_W11C
			command 追加ＨＺＸグループ削除 -h 0 -a 15
		#endif
	}

	proc 上段前列一番右の箱破壊後処理 {
		#if d:DEBUG_PRINT
		        print 'w11c_u_f_r1_box_broken'
		#endif

		// フォーチュン戦のときだけアタリチェンジ
		#ifdef d:STAGE_W11C
			command 追加ＨＺＸグループ削除 -h 0 -a 16
		#endif
	}

	proc 上段後列一番右の箱破壊後処理 {
		#if d:DEBUG_PRINT
		        print 'w11c_u_b_r1_box_broken'
		#endif

		// フォーチュン戦のときだけアタリチェンジ
		#ifdef d:STAGE_W11C
			command 追加ＨＺＸグループ削除 -h 0 -a 17
		#endif
	}

	proc 上段後列一番左の箱破壊後処理 {
		#if d:DEBUG_PRINT
		        print 'w11c_u_b_l1_box_broken'
		#endif

		// フォーチュン戦のときだけアタリチェンジ
		#ifdef d:STAGE_W11C
			command 追加ＨＺＸグループ削除 -h 0 -a 18
		#endif
	}


	proc フォークリフト前面荷物チェック {
		#if d:DEBUG_PRINT
		        print 'w11c_fklt_frontbox_check'
		#endif

		#ifdef d:STAGE_W11C
			eval( $b:フォークリフト前面荷物破壊数 = $b:フォークリフト前面荷物破壊数 + 1 );

			if ( $b:フォークリフト前面荷物破壊数 == 3 && $b:フォークリフト破壊カウント > 1 )  {
				// 前の荷物がすべてなくなってフォークリフトが倒れた
				command 追加ＨＺＸグループ削除 -h 0 -a 1
				command 追加ＨＺＸグループ削除 -h 0 -a 2
			}
		#endif
	}

	proc フォークリフト命中時処理 {
		#if d:DEBUG_PRINT
		        print 'w11c_fklt_hit'
		#endif

		#ifdef d:STAGE_W11C
			eval( $b:フォークリフト破壊カウント = $b:フォークリフト破壊カウント + 1 );

			if ( $b:フォークリフト破壊カウント == 1 ) {
				// 屋根吹っ飛び
				command 追加ＨＺＸグループ削除 -h 0 -a 29 // 跳弾は倒れたら自動的に消去
				command 追加ＨＺＸグループ登録 -h 0 -a 30
			} else if ( $b:フォークリフト破壊カウント == 2 ) {
				// 倒れはじめ
			} else if ( $b:フォークリフト破壊カウント == 3 ) {
				// 倒れ
				if ( `command ゲームオーバーチェック` == 0 ) {
					chara delay ディレイ \
						-time 8 \ 
						-exec{
							#if d:DEBUG_PRINT
								print 'フォークリフト 当たりを貼ったぜ！！'
								print '$f:フォークリフトゾーン中央にいるフラグ'
								print $f:フォークリフトゾーン中央にいるフラグ
							#endif
							command 追加ＨＺＸグループ登録 -h 0 -a 3
							command 追加ＨＺＸグループ削除 -h 0 -a 30 // 跳弾は倒れたら自動的に消去
							// アタリを張ったときに中に閉じ込められていたら、シナリオデモにして強制的に吹っ飛ばす
							if ( $f:フォークリフトゾーン中央にいるフラグ == 1 ) {
								@デモ用基本設定開始
								command パッド操作 -release
								mesg ボスフォーチュン フォーチュン  フォーチュン静止 0 2048

								chara delay シナリオカメラディレイ \
									-time 1 \ 
									-exec{
										chara カメラ設定 シナリオデモカメラ \
											-c 1 \	
											-p 2124,-42988,86 -t -433,-43933,-834 \
											-r 218,2847,0 -f 2877 \
											-a 230 \
											-i 0 0 0 0 \
											-s 1
								}

								mesg プレイヤー d:PLAYER position -424 -44000 1658

								chara delay ディレイ \
									-time 120 \ 
									-exec{
										@デモ用基本設定終了
										command パッド操作 -cancel
										chara カメラ設定  シナリオデモカメラ -s -1
										mesg ボスフォーチュン フォーチュン フォーチュン静止解除
										eval( $f:フォークリフトゾーン中央にいるフラグ = 0 );
										command トラップ切り替え -trap ev022 -switch 0
									}
							}
						}


				}

			} else if ( $b:フォークリフト破壊カウント == 4 ) {
				// 吹っ飛び
				// アタリを張ったときに中に閉じ込められていたら、シナリオデモにして強制的に吹っ飛ばす
				if ( `command ゲームオーバーチェック` == 0 ) {
					chara delay ディレイ \
						-time 8 \ 
						-exec{
							#if d:DEBUG_PRINT
								print 'フォークリフト 当たりを貼ったぜ！！'
								print '$f:フォークリフトゾーン左にいるフラグ'
								print $f:フォークリフトゾーン左にいるフラグ
								print '$f:フォークリフトゾーン右にいるフラグ'
								print $f:フォークリフトゾーン右にいるフラグ
							#endif
							@フォークリフト前面荷物チェック
							command 追加ＨＺＸグループ削除 -h 0 -a 3
							command 追加ＨＺＸグループ登録 -h 0 -a 4

							chara delay ディレイ \
								-time 1 \
								-exec{
									eval( $f:フォークリフトアタリ張ったフラグ = 1 );
								}

							// アタリを張ったときに中に閉じ込められていたら、シナリオデモにして強制的に吹っ飛ばす
							if ( $f:フォークリフトゾーン左にいるフラグ == 1 ) {
								mesg プレイヤー d:PLAYER position -4000 -44000 -1000
								eval( $f:フォークリフトゾーン左にいるフラグ = 0 );
								eval( $f:フォークリフトゾーン右にいるフラグ = 0 );
								command トラップ切り替え -trap ev023 -switch 0
								command トラップ切り替え -trap ev024 -switch 0
							} else if ( $f:フォークリフトゾーン右にいるフラグ == 1 ) {
								mesg プレイヤー d:PLAYER position 5241 -44000 -955
								eval( $f:フォークリフトゾーン左にいるフラグ = 0 );
								eval( $f:フォークリフトゾーン右にいるフラグ = 0 );
								command トラップ切り替え -trap ev023 -switch 0
								command トラップ切り替え -trap ev024 -switch 0
							}
						}
				}	
			}
		#endif
	}

	proc フォークリフト破壊時処理 {
		#if d:DEBUG_PRINT
		        print 'w11c_fklt_broken'
		#endif

		#ifdef d:STAGE_W11C
			@巻き上げ煙
			command 追加ＨＺＸグループ削除 -h 0 -a 1
			command 追加ＨＺＸグループ削除 -h 0 -a 2
			command 追加ＨＺＸグループ削除 -h 0 -a 3

			chara delay ディレイ \
				-time 30 \
				-exec{
					if ( `command ゲームオーバーチェック` == 0 ) {
						// アタリを張ったときに中に閉じ込められていたら、シナリオデモにして強制的に吹っ飛ばす
						if ( $f:フォークリフトゾーン左にいるフラグ == 1 ) {
							@デモ用基本設定開始
							command パッド操作 -release
							mesg ボスフォーチュン フォーチュン  フォーチュン静止 0 2048
							chara カメラ設定 シナリオデモカメラ \
								-c 1 \	
								-p -6480,-43969,517 -t -6396,-43947,488 \
								-r 3933,1241,0 -f 91 \
								-a 175 \
								-i 0 0 0 0 \
								-s 1

							chara delay ディレイ \
								-time 120 \ 
								-exec{
									if ( $f:フォークリフトアタリ張ったフラグ == 1 ) {
										@デモ用基本設定終了
										command パッド操作 -cancel
										chara カメラ設定  シナリオデモカメラ -s -1
										mesg ボスフォーチュン フォーチュン フォーチュン静止解除
										return 0;
									} else {
										return 1;
									}
								}
						} else if ( $f:フォークリフトゾーン右にいるフラグ == 1 ) {
						// アタリを張ったときに中に閉じ込められていたら、シナリオデモにして強制的に吹っ飛ばす
							@デモ用基本設定開始
							command パッド操作 -release
							mesg ボスフォーチュン フォーチュン  フォーチュン静止 0 2048
							chara カメラ設定 シナリオデモカメラ \
								-c 1 \	
								-p 6724,-44955,557 -t 6168,-44758,470 \
								-r 3877,2971,0 -f 596 \
								-a 179 \
								-i 0 0 0 0 \
								-s 1

							chara delay ディレイ \
								-time 120 \ 
								-exec{
									if ( $f:フォークリフトアタリ張ったフラグ == 1 ) {
										@デモ用基本設定終了
										command パッド操作 -cancel
										chara カメラ設定  シナリオデモカメラ -s -1
										mesg ボスフォーチュン フォーチュン フォーチュン静止解除
										return 0;
									} else {
										return 1;
									}
								}
						}
					}
				}
		#endif
	}

	proc 巻き上げ煙 {
		#if d:DEBUG_PRINT
		        print 'w11c_boss_raising_smoke'
		#endif

		#ifdef d:STAGE_W11C
			chara フォーチュン戦巻き上げ煙 煙 \
		        	-b -6000,-43000,-3500  6000,-41000,6000 \            // XYZ のセット２つ
			        -s 2000
		#endif
	}


	proc 中央右ドラム缶破壊時処理 {
		#if d:DEBUG_PRINT
		        print 'w11c_middle_right_dram_broken'
		#endif

		#ifdef d:STAGE_W11C
			command 追加ＨＺＸグループ削除 -h 0 -a 5
		#endif
	}

	proc 右下ドラム缶破壊時処理 {
		#if d:DEBUG_PRINT
		        print 'w11c_south_right_dram_broken'
		#endif

		#ifdef d:STAGE_W11C
			command 追加ＨＺＸグループ削除 -h 0 -a 6
		#endif
	}


	proc ２Ｍコンテナ命中時処理 {
		#if d:DEBUG_PRINT
		        print 'w11c_2mbox_hit'
		#endif

		#ifdef d:STAGE_W11C
			eval( $b:２ｍ破壊カウント = $b:２ｍ破壊カウント + 1 );

			if ( $b:２ｍ破壊カウント == 1 ) {
				command 追加ＨＺＸグループ削除 -h 0 -a 19
				command 追加ＨＺＸグループ登録 -h 0 -a 20
			} else if ( $b:２ｍ破壊カウント == 2 ) {
				command 追加ＨＺＸグループ削除 -h 0 -a 20
				command 追加ＨＺＸグループ登録 -h 0 -a 21
			} else if ( $b:２ｍ破壊カウント == 3 ) {
				command 追加ＨＺＸグループ削除 -h 0 -a 21
				command 追加ＨＺＸグループ登録 -h 0 -a 22
			} else if ( $b:２ｍ破壊カウント == 4 ) {
				command 追加ＨＺＸグループ削除 -h 0 -a 22
				command 追加ＨＺＸグループ登録 -h 0 -a 23
			} else if ( $b:２ｍ破壊カウント == 5 ) {
				command 追加ＨＺＸグループ削除 -h 0 -a 23
				command 追加ＨＺＸグループ登録 -h 0 -a 24
			} else if ( $b:２ｍ破壊カウント == 6 ) {
				command 追加ＨＺＸグループ削除 -h 0 -a 24
				command 追加ＨＺＸグループ登録 -h 0 -a 25
			} else if ( $b:２ｍ破壊カウント == 7 ) {
				command 追加ＨＺＸグループ削除 -h 0 -a 25
				command 追加ＨＺＸグループ登録 -h 0 -a 26
			} else if ( $b:２ｍ破壊カウント == 8 ) {
				command 追加ＨＺＸグループ削除 -h 0 -a 26
				command 追加ＨＺＸグループ登録 -h 0 -a 27
			}
		#endif
	}

	proc ２Ｍコンテナ破壊時処理 {
		#if d:DEBUG_PRINT
		        print 'w11c_2mbox_broken'
		#endif

		#ifdef d:STAGE_W11C
			command 追加ＨＺＸグループ削除 -h 0 -a 26
			command 追加ＨＺＸグループ登録 -h 0 -a 27
		#endif
	}

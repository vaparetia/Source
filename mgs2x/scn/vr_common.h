/*
	vr_common.h
		ＶＲで使用する共通プロック集(includeファイルの中で一番最後にincludeすること)

	2002/02/13 H.Yoshiike
	$Id: vr_common.h,v 1.52 2002/09/27 11:28:19 usr03682 Exp $


*/

// ------------------------------------------------------
// 全てのＶＲステージで共通に使用するものを集めたプロック
// ------------------------------------------------------
proc ＶＲ共通プロックセット $:スネークファイル $:雷電ファイル $:クリア条件フラグ $:ゴールＸ $:ゴールＹ $:ゴールＺ {
	#if d:DEBUG_PRINT
		print 'vr_common_proc_set'
	#endif

	#ifndef d:NO_VR_EFFECT
		@ＶＲステータスセット
	#endif
	@全ミッション共通システム設定 $:クリア条件フラグ
	@常駐キャラ設定											// stdch.hで定義
	@各ミッション用ゴール表示 $:ゴールＸ $:ゴールＹ $:ゴールＺ
	#ifdef d:VR_WEAPON
		@ＶＲ的設定
	#endif
	@サウンド設定
	#ifdef	d:BLADE_STAGE	//	刀ステージで刀を装備したままスタートさせるため
		@刀装備セット		//	a_stdproc.hで定義
	#endif
	@A_プレイヤー設定 $:スネークファイル $:雷電ファイル		// a_stdproc.hで定義
	@ＶＲエフェクト設置
	@ＶＲ空設置
	@ＶＲステージ変形設定
}

//-----------------------------------------------------
// スニーキング専用 (SNEAKING/ELIMINATE_ALL 両対応)
//-----------------------------------------------------
proc ＶＲスニーキング共通プロックセット $:スネークファイル $:雷電ファイル $:ゴールＸ $:ゴールＹ $:ゴールＺ {
	if($b:モード番号 == d:SNEAKING:SNEAKING){// スニーキング
		@ＶＲ共通プロックセット $:スネークファイル $:雷電ファイル d:VR_GOAL_FREE $:ゴールＸ $:ゴールＹ $:ゴールＺ
	}else if($b:モード番号 == d:SNEAKING:ELIMINATE_ALL){// 全殺し
		@ＶＲ共通プロックセット $:スネークファイル $:雷電ファイル d:VR_GOAL_SCN $:ゴールＸ $:ゴールＹ $:ゴールＺ
	}else{
		#if d:DEBUG_PRINT
			print 'WARNING!!!!!!!不正なモード番号です。$b:モード番号=' $b:モード番号
		#endif

		command assert ( d:FALSE ) 'Mode number error  (scenario err) ＶＲスニーキング共通プロックセット'
	}
}

//------------------------------------------------------------
// クリア時に呼ばれるプロック
//------------------------------------------------------------
proc クリア時処理 {
	#if d:DEBUG_PRINT
		print 'clear'
	#endif

	@ＶＲクリア時処理
}



//------------------------------------------------------------
// 全てのエフェクト等をＶＲ風にかえるプロック
//------------------------------------------------------------
proc ＶＲステータスセット {
	#if d:DEBUG_PRINT
		print 'vr_status_set'
	#endif

	command セットＶＲステータス d:VR_STATUS_VR_EFFECT
}



// ------------------------------------
// ＶＲで使用される光源関係
// ------------------------------------
// フォグ
// ------------------------------------
/* a_vardef.hで定義
enum VR_FOG {
	朝Ｎ = 0,
	朝Ｆ = 50000,
	夕Ｎ = -3000,
	夕Ｆ = 40000,
	夜Ｎ = -500,
	夜Ｆ = 18000
}
*/

proc ＶＲフォグ設定 $:p_ＮＥＡＲ_朝 $:p_ＦＡＲ_朝 $:p_ＮＥＡＲ_夕 $:p_ＦＡＲ_夕 $:p_ＮＥＡＲ_夜 $:p_ＦＡＲ_夜 {
	#if d:DEBUG_PRINT
		print 'fog_set'$:p_ＮＥＡＲ_朝 $:p_ＦＡＲ_朝 $:p_ＮＥＡＲ_夕 $:p_ＦＡＲ_夕 $:p_ＮＥＡＲ_夜 $:p_ＦＡＲ_夜
	#endif

	switch ( $b:ＶＲ起動時間帯 ) {
		case ( d:VR_TIME:朝 ) {
			chara フォグ 霧 -c 45, 48, 52 -n $:p_ＮＥＡＲ_朝 -f $:p_ＦＡＲ_朝
		}
		case ( d:VR_TIME:夕方 ) {
			chara フォグ 霧 -c 139, 100, 52 -n $:p_ＮＥＡＲ_夕 -f $:p_ＦＡＲ_夕
		}
		case ( d:VR_TIME:夜 ) {
			chara フォグ 霧 -c 20, 30, 35 -n $:p_ＮＥＡＲ_夜 -f $:p_ＦＡＲ_夜
		}
		default {
			#if d:DEBUG_PRINT
				print 'WARNING!!!!!!!不正な起動時間帯です。$b:ＶＲ起動時間帯='$b:ＶＲ起動時間帯
			#endif

			command assert ( d:FALSE ) 'VR TIME error  (scenario err) ＶＲフォグ設定'
			chara フォグ 霧 -c 45, 48, 52 -n $:p_ＮＥＡＲ_朝 -f $:p_ＦＡＲ_朝
		}
	}
}


// キャラ用光源(マップの設置が終わってから設定することで、キャラだけに光を当てる)
// ------------------------------------
proc ＶＲキャラ用ライト設置 {
	#if d:DEBUG_PRINT
		print 'chara_light_set'
	#endif

	switch ( $b:ＶＲ起動時間帯 ) {
		case ( d:VR_TIME:朝 ) {
			chara 環境光 環境光源 -c 64 96 80
			chara 平行光 平行光源 -c 160 96 64 -p -4000 -2500 -1000
		}
		case ( d:VR_TIME:夕方 ) {
			chara 環境光 環境光源 -c 64 96 80
			chara 平行光 平行光源 -c 176 160 79 -p -2500 -3500 -2500
		}
		case ( d:VR_TIME:夜 ) {
			chara 環境光 環境光源 -c 68 103 97
			chara 平行光 平行光源 -c 172 182 117 -p -3139 -3879 299
		}
		default {
			#if d:DEBUG_PRINT
				print 'WARNING!!!!!!!不正な起動時間帯です。$b:ＶＲ起動時間帯='$b:ＶＲ起動時間帯
			#endif

			command assert ( d:FALSE ) 'VR TIME error  (scenario err) ＶＲキャラ用ライト設置'
			chara 環境光 環境光源 -c 64 96 80
			chara 平行光 平行光源 -c 160 96 64 -p -4000 -2500 -1000
		}
	}
}



// ------------------------------------------------------
// ＶＲで使用するアウトライン
// ------------------------------------------------------
proc ＶＲアウトライン $:モデル名 $:トンボモデル名 $:ラインモデル名 {
	#if d:DEBUG_PRINT
		print 'vr_line_set'
	#endif

	switch ( $b:ミッション番号 ) {
		case ( d:MISSION:スニーキング ) {
			// スニーキング
			chara ステージアウトライン ステージアウトライン -file $:モデル名 \
				-a_color 30 100 60 128

			chara フェードオブジェ トンボ君 \
				-pos 0, 0, 0 \
				-rgb 10 80 40 \	/*最も明るくなったときの値*/
				-time 90 \			/*最も明るくなるまでの時間*/
				-kms $:トンボモデル名
		}
		case ( d:MISSION:武器訓練 ) {
			// ウエポン
			chara ステージアウトライン ステージアウトライン -file    $:モデル名 \
				-a_color 128 64 0 128

			chara フェードオブジェ トンボ君 \
				-pos 0, 0, 0 \
				-rgb 108, 44, 0 \	/*最も明るくなったときの値*/
				-time 90 \			/*最も明るくなるまでの時間*/
				-kms $:トンボモデル名
		}
		case ( d:MISSION:バラエティ ) {
			// バラエティ
			#ifdef d:STAGE_SP08A
				// ダークステージは特殊
				chara ステージアウトライン ステージアウトライン -file    $:モデル名 \
					-a_color 50 30 10 128

				chara フェードオブジェ トンボ君 \
					-pos 0, 0, 0 \
					-rgb 50 30 10 \	/*最も明るくなったときの値*/
					-time 90 \			/*最も明るくなるまでの時間*/
					-kms $:トンボモデル名
			#else
				chara ステージアウトライン ステージアウトライン -file    $:モデル名 \
					-a_color 100 100 100 128

				chara フェードオブジェ トンボ君 \
					-pos 0, 0, 0 \
					-rgb 80 80 80 \	/*最も明るくなったときの値*/
					-time 90 \			/*最も明るくなるまでの時間*/
					-kms $:トンボモデル名
			#endif

			// バラエティでさらに追加されるもの
			#ifdef d:STAGE_SP02A
				chara ステージアウトライン ステージアウトライン -file    sp02a1 \
					-a_color 100 100 100 128

				chara フェードオブジェ トンボ君 \
					-pos 0, 0, 0 \
					-rgb 80 80 80 \	/*最も明るくなったときの値*/
					-time 90 \			/*最も明るくなるまでの時間*/
					-kms sp02a1_tonbo
			#elifdef d:STAGE_SP06A
				// 狙撃ステージは特別に以下のものを追加
				// １つ目の足場
				chara ステージアウトライン ステージアウトライン \
					-file    sp06a_building \
					-a_color 100 100 100 128 \
					-center 0 6000 20000

				chara フェードオブジェ トンボ君 \
					-pos 0 6000 20000 \
					-rgb 80 80 80 \	/*最も明るくなったときの値*/
					-time 90 \			/*最も明るくなるまでの時間*/
					-kms sp06a_building_tonbo

				chara フェードオブジェ トンボ君 \
					-pos 0 6000 20000 \
					-rgb 160, 160, 160 \	/*最も明るくなったときの値*/
					-time 45 \			/*最も明るくなるまでの時間*/
					-kms sp06a_building_line

				// ２つ目の足場
				chara ステージアウトライン ステージアウトライン \
					-file    sp06a_building \
					-a_color 100 100 100 128 \
					-center 0 0 20000

				chara フェードオブジェ トンボ君 \
					-pos 0 0 20000 \
					-rgb 80 80 80 \	/*最も明るくなったときの値*/
					-time 90 \			/*最も明るくなるまでの時間*/
					-kms sp06a_building_tonbo

				chara フェードオブジェ トンボ君 \
					-pos 0 0 20000 \
					-rgb 160, 160, 160 \	/*最も明るくなったときの値*/
					-time 45 \			/*最も明るくなるまでの時間*/
					-kms sp06a_building_line

				#if d:DOCUMENT
					// ポスター
					chara ステージアウトライン ステージアウトライン \
						-file    $$s:ポスター名 \
						-a_color 100 100 100 128 \
						-center 0 0 0
				#endif

			#elifdef d:STAGE_SP07A
				// ゴルルゴンステージは特別に以下のものを追加
				chara フェードオブジェ トンボ君 \
					-pos 0, 0, 0 \
					-rgb 80 80 80 \	/*最も明るくなったときの値*/
					-time 90 \			/*最も明るくなるまでの時間*/
					-kms sp07a1_tonbo

				chara フェードオブジェ トンボ君 \
					-pos 0, 0, 0 \
					-rgb 80 80 80 \	/*最も明るくなったときの値*/
					-time 90 \			/*最も明るくなるまでの時間*/
					-kms sp07a2_tonbo

				chara フェードオブジェ トンボ君 \
					-pos 0, 0, 0 \
					-rgb 160, 160, 160 \	/*最も明るくなったときの値*/
					-time 45 \			/*最も明るくなるまでの時間*/
					-kms sp07a1_line

				chara フェードオブジェ トンボ君 \
					-pos 0, 0, 0 \
					-rgb 160, 160, 160 \	/*最も明るくなったときの値*/
					-time 45 \			/*最も明るくなるまでの時間*/
					-kms sp07a2_line

			#endif

		}
		case ( d:MISSION:主観モード ) {
			#ifdef d:STAGE_SP24A
				// ダークステージは特殊
				chara ステージアウトライン ステージアウトライン -file    $:モデル名 \
					-a_color 50 30 10 128

				chara ステージアウトライン ステージアウトライン -file sp08a_wall \
					-a_color 50 30 10 128

				chara フェードオブジェ トンボ君 \
					-pos 0, 0, 0 \
					-rgb 50 30 10 \	/*最も明るくなったときの値*/
					-time 90 \			/*最も明るくなるまでの時間*/
					-kms $:トンボモデル名
			#else
				chara ステージアウトライン ステージアウトライン -file    $:モデル名 \
					-a_color 100 100 100 128

				chara フェードオブジェ トンボ君 \
					-pos 0, 0, 0 \
					-rgb 80 80 80 \	/*最も明るくなったときの値*/
					-time 90 \			/*最も明るくなるまでの時間*/
					-kms $:トンボモデル名
			#endif
		}
		case ( d:MISSION:ストリーキング ) {
			chara ステージアウトライン ステージアウトライン -file    $:モデル名 \
				-a_color 100 100 100 128

			chara フェードオブジェ トンボ君 \
				-pos 0, 0, 0 \
				-rgb 80 80 80 \	/*最も明るくなったときの値*/
				-time 90 \			/*最も明るくなるまでの時間*/
				-kms $:トンボモデル名
		}
		default {
			#if d:DEBUG_PRINT
				print 'WARNING!!!!!!!ＶＲ以外のミッションでトンボを出そうとしました。$b:ミッション番号='$b:ミッション番号
			#endif

			command assert ( d:FALSE ) 'mission no error  (scenario err) ＶＲアウトライン'
			chara ステージアウトライン ステージアウトライン -file $:モデル名 \
				-a_color 30 100 60 128

			chara フェードオブジェ トンボ君 \
				-pos 0, 0, 0 \
				-rgb 10 80 40 \	/*最も明るくなったときの値*/
				-time 90 \			/*最も明るくなるまでの時間*/
				-kms $:トンボモデル名
		}
	}

	// そこから先行けないことを示すライン
	#ifdef d:STAGE_SP08A
		// ダークステージは特殊
		chara フェードオブジェ トンボ君 \
			-pos 0, 0, 0 \
			-rgb 40, 40, 40 \	/*最も明るくなったときの値*/
			-time 45 \			/*最も明るくなるまでの時間*/
			-kms $:ラインモデル名

		chara フェードオブジェ トンボ君 \
			-pos 0, 0, 0 \
			-rgb 40, 40, 40 \	/*最も明るくなったときの値*/
			-time 45 \			/*最も明るくなるまでの時間*/
			-kms sp08a_line2

	#elifdef d:STAGE_SP24A
		// ダークステージは特殊
		chara フェードオブジェ トンボ君 \
			-pos 0, 0, 0 \
			-rgb 40, 40, 40 \	/*最も明るくなったときの値*/
			-time 45 \			/*最も明るくなるまでの時間*/
			-kms $:ラインモデル名

	#else
		chara フェードオブジェ トンボ君 \
			-pos 0, 0, 0 \
			-rgb 160, 160, 160 \	/*最も明るくなったときの値*/
			-time 45 \			/*最も明るくなるまでの時間*/
			-kms $:ラインモデル名
	#endif
}


// ------------------------------------------------------
// ＶＲで使用する追加エフェクト
// ------------------------------------------------------
proc ＶＲエフェクト設置 {
	#if d:DEBUG_PRINT
		print 'vr_effect_set'
	#endif

	if ( $b:ミッション番号 != d:MISSION:主観モード ) {
		chara ＶＲ壁マーカー ＶＲ壁マーカー
		chara ＶＲ床マーカー ＶＲ床マーカー
	}
	chara ＶＲ跳弾 ＶＲ跳弾君
}


// ------------------------------------------------------
// ＶＲで使用使用される空
// ------------------------------------------------------
proc ＶＲ空設置 {
	#if d:DEBUG_PRINT
		print 'vr_sky_set'
	#endif


	#ifdef d:STAGE_SP08A || d:STAGE_SP24A
		// ダークステージの空
		chara	ＶＲ空	ＶＲ空 \
			-col 30 40 35 \
			-pos 0 0 0 \
			-l   32 \
			-f   0 \
			-t   4 \
				sky_m1_alp 28 32 30 100 \
				sky_m2_alp 33 53 67  50 \
				0            0   0   0   0 \
				sky_m4_alp 37 67 85  50 \
			-m   2 \
				vs_sky_wall 17 26 25 -100     0 \
				vr2_tenkyu  10 45 45    0 15708 \
			-h   2 \
				vr2_tenkyu 52 46 0 -2513 -3523 250 \
				vr2_tenkyu 42 36 0 2513 3523 -250 \
			-r   4 \
				vr2_tenkyu 1 d:VRSKY_Y_ROT -200 \
				vr2_tenkyu 2 d:VRSKY_X_ROT  300 \
				vr2_tenkyu 3 d:VRSKY_X_ROT  400 \
				vr2_tenkyu 4 d:VRSKY_X_ROT  500

		chara 太陽 ソル \
			-p 0 -40000 0 \ 		//中心の位置
	//				-f 3650 -1200 3200 \	//平行光源の-pと同じ値
			-f -30 -70 20 \
			-a 5 \
			-l 32 \
			-w (240*32) \
			-h (240*32) \
			-t sky_sun_m_alp

	#elifdef d:STAGE_SP03A
		if ( $b:ステージ番号 == d:ZAKO_STAGE:GURLUGON || $b:ステージ番号 == d:ZAKO_STAGE:MECHGENOLA ) {
			// ゴルルゴン、メカゲノラ戦の空
			chara	ＶＲ空	ＶＲ空 \
				-col 30 40 35 \
				-pos 0 0 0 \
				-l   32 \
				-f   0 \
				-t   4 \
					sky_m1_alp 28 32 30 100 \
					sky_m2_alp 33 53 67  50 \
					0            0   0   0   0 \
					sky_m4_alp 37 67 85  50 \
				-m   2 \
					vs_sky_wall 17 26 25 -100     0 \
					vr2_tenkyu  10 45 45    0 15708 \
				-h   2 \
					vr2_tenkyu 52 46 0 -2513 -3523 250 \
					vr2_tenkyu 42 36 0 2513 3523 -250 \
				-r   4 \
					vr2_tenkyu 1 d:VRSKY_Y_ROT -200 \
					vr2_tenkyu 2 d:VRSKY_X_ROT  300 \
					vr2_tenkyu 3 d:VRSKY_X_ROT  400 \
					vr2_tenkyu 4 d:VRSKY_X_ROT  500

/* ゴルルゴン戦とメカゲノラ戦では太陽を取ります。 2002.09.27 吉池
			chara 太陽 ソル \
				-p 0 -40000 0 \ 		//中心の位置
		//				-f 3650 -1200 3200 \	//平行光源の-pと同じ値
				-f -30 -70 20 \
				-a 5 \
				-l 32 \
				-w (240*32) \
				-h (240*32) \
				-t sky_sun_m_alp
*/
		} else {
			// 通常の夜空
			switch ( $b:ＶＲ起動時間帯 ) {
				case ( d:VR_TIME:朝 ) {
					chara	ＶＲ空	ＶＲ空 \
						-col 104 80 74 \
						-pos 0 0 0 \
						-l   32 \
						-f   0 \
						-t   4 \
							sky_m1_alp 255 144 100 100 \
							sky_m2_alp 220 156 110  50 \
							0            0   0   0   0 \
							sky_m4_alp 220 156 110  50 \
						-m   2 \
							vs_sky_wall 95 68 55 -100     0 \
							vr2_tenkyu  45 48 52    0 15708 \
						-h   2 \
							vr2_tenkyu 28 20 16 0 -5000 0 \
							vr2_tenkyu 89 53 37 0 5000 0 \
						-r   4 \
							vr2_tenkyu 1 d:VRSKY_Y_ROT -200 \
							vr2_tenkyu 2 d:VRSKY_X_ROT  300 \
							vr2_tenkyu 3 d:VRSKY_X_ROT  400 \
							vr2_tenkyu 4 d:VRSKY_X_ROT  500

					chara 太陽 ソル \
						-p 0 -40000 0 \ 		//中心の位置
		//				-f 3650 -1200 3200 \	//平行光源の-pと同じ値
						-f -30 -70 20 \
		//				-a 11 \   //alpha
						-a 10 \
						-l 32 \
						-w (240*32) \
						-h (240*32) \
						-t sky_sun_m_alp
				}
				case ( d:VR_TIME:夕方 ) {
					chara	ＶＲ空	ＶＲ空 \
						-c 50 47 38 \
						-pos 0 0 0 \
						-l   32 \
						-f   0 \
						-t   4 \
							sky_m1_alp 116 106 96 50 \
							sky_m2_alp 94 89 79 50 \
							0 0 0 0 0 \
							sky_m4_alp 78 73 66 50 \
						-m   2 \
							vs_sky_wall 47 35 21 -100     0 \
							vr2_tenkyu  96 64 38    0 15708 \
						-h   2 \
							vr2_tenkyu 36 54 34 0 -5000 0 \
							vr2_tenkyu 16 44 40 0 5000 0 \
						-r   4 \
							vr2_tenkyu 1 d:VRSKY_Y_ROT -200 \
							vr2_tenkyu 2 d:VRSKY_X_ROT  300 \
							vr2_tenkyu 3 d:VRSKY_X_ROT  400 \
							vr2_tenkyu 4 d:VRSKY_X_ROT  500

					chara 太陽 ソル \
						-p 0 -40000 0 \ 		//中心の位置
		//				-f 3650 -1200 3200 \	//平行光源の-pと同じ値
						-f -30 -70 20 \
		//				-a 11 \   //alpha
						-a 10 \
						-l 32 \
						-w (240*32) \
						-h (240*32) \
						-t sky_sun_m_alp
				}
				case ( d:VR_TIME:夜 ) {
					chara	ＶＲ空	ＶＲ空 \
						-col 41 54 54 \
						-pos 0 0 0 \
						-l   32 \
						-f   0 \
						-t   4 \
							sky_m1_alp 38 42 50 100 \
							sky_m2_alp 43 63 87  50 \
							0            0   0   0   0 \
							sky_m4_alp 47 77 105  50 \
						-m   2 \
							vs_sky_wall 27 36 40 -100     0 \
							vr2_tenkyu  20 55 65    0 15708 \
						-h   2 \
							vr2_tenkyu 62 56 7 -2513 -3523 250 \
							vr2_tenkyu 52 46 0 2513 3523 -250 \
						-r   4 \
							vr2_tenkyu 1 d:VRSKY_Y_ROT -200 \
							vr2_tenkyu 2 d:VRSKY_X_ROT  300 \
							vr2_tenkyu 3 d:VRSKY_X_ROT  400 \
							vr2_tenkyu 4 d:VRSKY_X_ROT  500

					chara 太陽 ソル \
						-p 0 -40000 0 \ 		//中心の位置
		//				-f 3650 -1200 3200 \	//平行光源の-pと同じ値
						-f -30 -70 20 \
						-a 5 \
						-l 32 \
						-w (240*32) \
						-h (240*32) \
						-t sky_sun_m_alp
				}
				default {
					#if d:DEBUG_PRINT
						print 'WARNING!!!!!!!不正な起動時間帯です。$b:ＶＲ起動時間帯='$b:ＶＲ起動時間帯
					#endif

					command assert ( d:FALSE ) 'VR TIME error  (scenario err) ＶＲ空設置'
					chara	ＶＲ空	ＶＲ空 \
						-col 104 80 74 \
						-pos 0 0 0 \
						-l   32 \
						-f   0 \
						-t   4 \
							sky_m1_alp 255 144 100 100 \
							sky_m2_alp 220 156 110  50 \
							0            0   0   0   0 \
							sky_m4_alp 220 156 110  50 \
						-m   2 \
							vs_sky_wall 95 68 55 -100     0 \
							vr2_tenkyu  45 48 52    0 15708 \
						-h   2 \
							vr2_tenkyu 28 20 16 0 -5000 0 \
							vr2_tenkyu 89 53 37 0 5000 0 \
						-r   4 \
							vr2_tenkyu 1 d:VRSKY_Y_ROT -200 \
							vr2_tenkyu 2 d:VRSKY_X_ROT  300 \
							vr2_tenkyu 3 d:VRSKY_X_ROT  400 \
							vr2_tenkyu 4 d:VRSKY_X_ROT  500

					chara 太陽 ソル \
						-p 0 -40000 0 \ 		//中心の位置
		//				-f 3650 -1200 3200 \	//平行光源の-pと同じ値
						-f -30 -70 20 \
		//				-a 11 \   //alpha
						-a 10 \
						-l 32 \
						-w (240*32) \
						-h (240*32) \
						-t sky_sun_m_alp
				}
			}
		}

	#else
		// 通常の夜空
		switch ( $b:ＶＲ起動時間帯 ) {
			case ( d:VR_TIME:朝 ) {
				chara	ＶＲ空	ＶＲ空 \
					-col 104 80 74 \
					-pos 0 0 0 \
					-l   32 \
					-f   0 \
					-t   4 \
						sky_m1_alp 255 144 100 100 \
						sky_m2_alp 220 156 110  50 \
						0            0   0   0   0 \
						sky_m4_alp 220 156 110  50 \
					-m   2 \
						vs_sky_wall 95 68 55 -100     0 \
						vr2_tenkyu  45 48 52    0 15708 \
					-h   2 \
						vr2_tenkyu 28 20 16 0 -5000 0 \
						vr2_tenkyu 89 53 37 0 5000 0 \
					-r   4 \
						vr2_tenkyu 1 d:VRSKY_Y_ROT -200 \
						vr2_tenkyu 2 d:VRSKY_X_ROT  300 \
						vr2_tenkyu 3 d:VRSKY_X_ROT  400 \
						vr2_tenkyu 4 d:VRSKY_X_ROT  500

				chara 太陽 ソル \
					-p 0 -40000 0 \ 		//中心の位置
	//				-f 3650 -1200 3200 \	//平行光源の-pと同じ値
					-f -30 -70 20 \
	//				-a 11 \   //alpha
					-a 10 \
					-l 32 \
					-w (240*32) \
					-h (240*32) \
					-t sky_sun_m_alp
			}
			case ( d:VR_TIME:夕方 ) {
				chara	ＶＲ空	ＶＲ空 \
					-c 50 47 38 \
					-pos 0 0 0 \
					-l   32 \
					-f   0 \
					-t   4 \
						sky_m1_alp 116 106 96 50 \
						sky_m2_alp 94 89 79 50 \
						0 0 0 0 0 \
						sky_m4_alp 78 73 66 50 \
					-m   2 \
						vs_sky_wall 47 35 21 -100     0 \
						vr2_tenkyu  96 64 38    0 15708 \
					-h   2 \
						vr2_tenkyu 36 54 34 0 -5000 0 \
						vr2_tenkyu 16 44 40 0 5000 0 \
					-r   4 \
						vr2_tenkyu 1 d:VRSKY_Y_ROT -200 \
						vr2_tenkyu 2 d:VRSKY_X_ROT  300 \
						vr2_tenkyu 3 d:VRSKY_X_ROT  400 \
						vr2_tenkyu 4 d:VRSKY_X_ROT  500

				chara 太陽 ソル \
					-p 0 -40000 0 \ 		//中心の位置
	//				-f 3650 -1200 3200 \	//平行光源の-pと同じ値
					-f -30 -70 20 \
	//				-a 11 \   //alpha
					-a 10 \
					-l 32 \
					-w (240*32) \
					-h (240*32) \
					-t sky_sun_m_alp
			}
			case ( d:VR_TIME:夜 ) {
				chara	ＶＲ空	ＶＲ空 \
					-col 41 54 54 \
					-pos 0 0 0 \
					-l   32 \
					-f   0 \
					-t   4 \
						sky_m1_alp 38 42 50 100 \
						sky_m2_alp 43 63 87  50 \
						0            0   0   0   0 \
						sky_m4_alp 47 77 105  50 \
					-m   2 \
						vs_sky_wall 27 36 40 -100     0 \
						vr2_tenkyu  20 55 65    0 15708 \
					-h   2 \
						vr2_tenkyu 62 56 7 -2513 -3523 250 \
						vr2_tenkyu 52 46 0 2513 3523 -250 \
					-r   4 \
						vr2_tenkyu 1 d:VRSKY_Y_ROT -200 \
						vr2_tenkyu 2 d:VRSKY_X_ROT  300 \
						vr2_tenkyu 3 d:VRSKY_X_ROT  400 \
						vr2_tenkyu 4 d:VRSKY_X_ROT  500

				chara 太陽 ソル \
					-p 0 -40000 0 \ 		//中心の位置
	//				-f 3650 -1200 3200 \	//平行光源の-pと同じ値
					-f -30 -70 20 \
					-a 5 \
					-l 32 \
					-w (240*32) \
					-h (240*32) \
					-t sky_sun_m_alp
			}
			default {
				#if d:DEBUG_PRINT
					print 'WARNING!!!!!!!不正な起動時間帯です。$b:ＶＲ起動時間帯='$b:ＶＲ起動時間帯
				#endif

				command assert ( d:FALSE ) 'VR TIME error  (scenario err) ＶＲ空設置'
				chara	ＶＲ空	ＶＲ空 \
					-col 104 80 74 \
					-pos 0 0 0 \
					-l   32 \
					-f   0 \
					-t   4 \
						sky_m1_alp 255 144 100 100 \
						sky_m2_alp 220 156 110  50 \
						0            0   0   0   0 \
						sky_m4_alp 220 156 110  50 \
					-m   2 \
						vs_sky_wall 95 68 55 -100     0 \
						vr2_tenkyu  45 48 52    0 15708 \
					-h   2 \
						vr2_tenkyu 28 20 16 0 -5000 0 \
						vr2_tenkyu 89 53 37 0 5000 0 \
					-r   4 \
						vr2_tenkyu 1 d:VRSKY_Y_ROT -200 \
						vr2_tenkyu 2 d:VRSKY_X_ROT  300 \
						vr2_tenkyu 3 d:VRSKY_X_ROT  400 \
						vr2_tenkyu 4 d:VRSKY_X_ROT  500

				chara 太陽 ソル \
					-p 0 -40000 0 \ 		//中心の位置
	//				-f 3650 -1200 3200 \	//平行光源の-pと同じ値
					-f -30 -70 20 \
	//				-a 11 \   //alpha
					-a 10 \
					-l 32 \
					-w (240*32) \
					-h (240*32) \
					-t sky_sun_m_alp
			}
		}

	#endif
}



// ------------------------------------------------------
// ＶＲで使用する共通変数の初期化
// ------------------------------------------------------
proc ＶＲステージ変形設定 {
	#if d:DEBUG_PRINT
		print 'vr_stage_change_set'
	#endif

	#ifdef d:STAGE_SP02A
		// ビルのアスレチックステージではゴール側が別モデルなので追加
		chara ステージ変形 ステージ変形君 \
			-direct 1 \
			-pos 0 0 0 \	//座標
			-time 30 d:STAGE_CHANGE_TIME \	//1ポリの合体完了までの時間 端から端までの変形の遅れ時間
			-len -5000 		//ポリゴン飛行距離

	#elifdef d:STAGE_SP06A
		// 敵兵狙撃ミッションは足場が別モデル
		chara ステージ変形 ステージ変形君 \
			-name ビル \
			-pos 0 0 0 \	//座標
			-time 30 d:STAGE_CHANGE_TIME \	//1ポリの合体完了までの時間 端から端までの変形の遅れ時間
			-len -5000 		//ポリゴン飛行距離

		chara ステージ変形 ステージ変形君 \
			-name １階 \
			-pos 0 0 0 \	//座標
			-time 30 d:STAGE_CHANGE_TIME \	//1ポリの合体完了までの時間 端から端までの変形の遅れ時間
			-len -5000 		//ポリゴン飛行距離

	#elifdef d:STAGE_SP07A
		chara ステージ変形 ステージ変形君 \
			-direct 1 \
			-pos 0 0 0 \	//座標
			-time 30 d:STAGE_CHANGE_TIME \	//1ポリの合体完了までの時間 端から端までの変形の遅れ時間
			-len -5000 		//ポリゴン飛行距離

		chara ステージ変形 ステージ変形君 \
			-direct 2 \
			-pos 0 0 0 \	//座標
			-time 30 d:STAGE_CHANGE_TIME \	//1ポリの合体完了までの時間 端から端までの変形の遅れ時間
			-len -5000 		//ポリゴン飛行距離

	#endif
		chara ステージ変形 ステージ変形君 \
			-pos 0 0 0 \	//座標
			-time 30 d:STAGE_CHANGE_TIME \	//1ポリの合体完了までの時間 端から端までの変形の遅れ時間
			-len -5000 		//ポリゴン飛行距離


}


// ------------------------------------------------------
// 呼び出されているプロック
// ------------------------------------------------------
// ＶＲで使用する共通変数の初期化
// ------------------------------------------------------
proc ＶＲ的設定 {
	#if d:DEBUG_PRINT
		print 'commonvar_init'
	#endif

	if ( $s:選択プレイヤー == プリスキン || $s:選択プレイヤー == タキシードスネーク ) {

		#ifndef d:CUSTOM_TRG_BLINK
			command セットＶＲ的出現消え周期 180 60
		#endif

	} else if ( $s:選択プレイヤー == 前作スネーク ) {

		#ifndef d:CUSTOM_TRG_SPEED
			command	ＶＲ的速度係数 6144
		#endif

		#ifndef d:CUSTOM_TRG_BLINK
			command セットＶＲ的出現消え周期 180 60
		#endif

	}


	// 全キャラ共通設定
	command ＶＲ移動的ライフ	20
	command ＶＲ壁ライフ 		20
	command ＶＲ箱ライフ		20
	command ＶＲ移動的スケール	4096
	#ifdef d:STAGE_WP45A
		command ＶＲ爆発範囲		4000
	#else
		if ( $b:モード番号 == d:WEAPON:C4 ) {
			// ＶＲ爆発範囲の半径がＣ４の爆発直径(2000)を超えると、
			// Ｃ４の有効性がなくなってしまうのでわからない程度に縮小。
			command ＶＲ爆発範囲		2750
		} else {
			command ＶＲ爆発範囲		4000
		}
	#endif
	command	ＶＲコンボタイム	120

	/*
	#enum {
		VR_MDL_TRI = 0 ,		// 三角
		VR_MDL_OCT,				// 8面体的
		VR_MDL_KATANA,			// カタナ
		VR_MDL_CROSS,			// 十字
		VR_MDL_HEX,				// スティンガー用大型
		VR_MDL_FIX_CUBE,		// 固定箱
		VR_MDL_MOVE_CUBE,		// 移動箱(部位無し6面体)
		VR_MDL_ONE_TRI, 		// 三角部位なし
		VR_MDL_WALL,			// 壁
		VR_MDL_PUNCH,			// 素手攻撃
	};
	*/

#define TR_SCR_A	500
#define TR_SCR_B	50
#define TR_SCR_C	10

	// 三角
	command セットＶＲ的部位ダメージ	d:VR_MDL_TRI		20	20 20
	command セットＶＲ的部位スコア		d:VR_MDL_TRI		d:TR_SCR_A d:TR_SCR_B d:TR_SCR_C
	// 三角部位無し
	command セットＶＲ的部位ダメージ	d:VR_MDL_ONE_TRI	20	20 20
	command セットＶＲ的部位スコア		d:VR_MDL_ONE_TRI	d:TR_SCR_A d:TR_SCR_A d:TR_SCR_A
	// 十字
	command セットＶＲ的部位ダメージ	d:VR_MDL_CROSS		7 7 7
	command セットＶＲ的部位スコア		d:VR_MDL_CROSS		d:TR_SCR_A d:TR_SCR_B d:TR_SCR_C
	// 8面体的
	command セットＶＲ的部位ダメージ	d:VR_MDL_OCT		20 7 4
	command セットＶＲ的部位スコア		d:VR_MDL_OCT		d:TR_SCR_A d:TR_SCR_B d:TR_SCR_C
	// 部位無し6面体
	command セットＶＲ的部位ダメージ	d:VR_MDL_MOVE_CUBE	7 7 7
	command セットＶＲ的部位スコア		d:VR_MDL_MOVE_CUBE	d:TR_SCR_A d:TR_SCR_A d:TR_SCR_A
	// スティンガー用大型
	command セットＶＲ的部位ダメージ	d:VR_MDL_HEX		2 2 2
	command セットＶＲ的部位スコア		d:VR_MDL_HEX		d:TR_SCR_A d:TR_SCR_A d:TR_SCR_A

	// 壁
	command セットＶＲ的部位ダメージ	d:VR_MDL_WALL		4 4 4
	command セットＶＲ的部位スコア		d:VR_MDL_WALL		d:TR_SCR_A d:TR_SCR_A d:TR_SCR_A
	// 固定箱
	command セットＶＲ的部位ダメージ	d:VR_MDL_FIX_CUBE	2 2 2
	command セットＶＲ的部位スコア		d:VR_MDL_FIX_CUBE	d:TR_SCR_A d:TR_SCR_A d:TR_SCR_A


	// 刀
	command セットＶＲ的部位ダメージ	d:VR_MDL_KATANA		5 5 5
	command セットＶＲ的部位スコア		d:VR_MDL_KATANA		d:TR_SCR_A d:TR_SCR_A d:TR_SCR_A
/*
	// 素手攻撃
	command セットＶＲ的部位ダメージ	d:VR_MDL_KATANA 1 1 1
	command セットＶＲ的部位スコア		d:VR_MDL_PUNCH 100 100 100
*/
	command セットＶＲＮＧ的スコア -2000
}


// サウンドデータの読み込み
// ------------------------------------------------------
proc サウンド設定 {
	#if d:DEBUG_PRINT
		print 'sound_set'
	#endif

	if ( $b:ミッション番号 == d:MISSION:スニーキング || $b:ミッション番号 == d:MISSION:武器訓練 ) {
		// スニーキングと武器訓練モードのみキャラごとにサウンドを変える
		switch ( $s:選択プレイヤー ) {
			case ( ライデン ) {
				chara サウンドマネージャー ＳＤマネ -pak 0
			}
			case ( 刀ライデン ) {
				chara サウンドマネージャー ＳＤマネ -pak 5
			}
			case ( スネーク ) {
				chara サウンドマネージャー ＳＤマネ -pak 3
			}
			case ( プリスキン ) {
				chara サウンドマネージャー ＳＤマネ -pak 4
			}
			case ( タキシードスネーク ) {
				chara サウンドマネージャー ＳＤマネ -pak 1
			}
			case ( 前作スネーク ) {
				chara サウンドマネージャー ＳＤマネ -pak 2
			}
			default {
				// 裸ライデン
				chara サウンドマネージャー ＳＤマネ -pak 0
			}
		}
	} else {
		// それ以外は０で固定
		#ifndef d:STAGE_SP03A
			chara サウンドマネージャー ＳＤマネ -pak 0
		#else
			// ザコサバイバルは怪獣達の前後でパックを返る
			if ( $b:ステージ番号 <= d:ZAKO_STAGE:T_G_SOLS_TEAM ) {
				chara サウンドマネージャー ＳＤマネ -pak 0
			} else if ( $b:ステージ番号 == d:ZAKO_STAGE:GURLUGON ) {
				chara サウンドマネージャー ＳＤマネ -pak 1
			} else if ( $b:ステージ番号 == d:ZAKO_STAGE:MECHGENOLA ) {
				chara サウンドマネージャー ＳＤマネ -pak 2
			} else if ( $b:ステージ番号 == d:ZAKO_STAGE:GENOLA ) {
				chara サウンドマネージャー ＳＤマネ -pak 3
			}

		#endif
	}

	@サウンド効果音設定		// a_sd_set.hで定義
	@弾痕跳弾設定
}


// 弾痕跳弾関係の設定
// ------------------------------------
proc 弾痕跳弾設定 {
	#if d:DEBUG_PRINT
		print 'recoil_set'
	#endif

	command ＶＲ弾痕ノーマル -n 0	// 通常
	command ＶＲ弾痕ノーマル -n 1	// 鉄板、ロッカー
	command ＶＲ弾痕ノーマル -n 2	// スケ床
	command ＶＲ弾痕ノーマル -n 3	// 鉄筋、鉄床
	command ＶＲ弾痕ノーマル -n 4	// 鳴り床
	command ＶＲ弾痕ノーマル -n 5	// 金網
	command ＶＲ弾痕ノーマル -n 6	// 水床

	command 跳弾ノーマル -n 0
	command 跳弾ノーマル -n 1
	command 跳弾ノーマル -n 2
	command 跳弾ノーマル -n 3
	command 跳弾ノーマル -n 4
	command 跳弾ノーマル -n 5
	command 跳弾ノーマル -n 6
}

proc スニーキング判定 {
	if( $b:ミッション番号 == d:MISSION:スニーキング ){
		if( $b:モード番号 == d:SNEAKING:SNEAKING ){
			return 1;
		}else{
			return 0;
		}
	}else{
		#ifndef d:FIX_MODE
			command assert(d:FALSE) 'スニーキングミッションではない。'
		#endif
		return 1;// 一応。
	}
}


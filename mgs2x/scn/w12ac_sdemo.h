//=============================================================================
// w12ac_sdemo.h
// Ａ脚・屋上(シナリオデモ)
// 2001/08/06 S.Yamashita
// $Id: w12ac_sdemo.h,v 1.10 2001/10/04 07:20:00 usr03692 Exp $
//

//-----------------------------------------------------------------------------
// 昇降機設置

#define	ELV_NAME elv_1

proc 昇降機設置	{
#if d:DEBUG_PRINT
	print '*** w12 Elevator Set ***'
#endif

	if(    (   ($w:p_story <  d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了) \
			|| (d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了 <= $w:p_story)) \
		&& ($f:w12a_Ａ脚ポンプ室爆弾解体前にセンサーＢゲット == 0)) {

		if($s:登場ポイント == sp_w11c_w12a_0) {
			#if d:DEBUG_PRINT
				print '##### On Elevator Go Up: Go Down #####'
			#endif

			//---------------------------------------------------------
			// フォーチュン戦直後。昇降機で登場して、昇降機下がる

			chara フォーチュン戦昇降機 昇降機 \
				-e_model	d:ELV_NAME \
				-e_top		0, 50000, 0 \
				-e_pos		0, 25000, 0 \
				-e_bottom	0,     0, 0 \
				-e_dir 1 \
				-e_rot		0, 0, 0 \
				-trap		ev006 ev007 \	// 起動トラップ 敵兵チェックトラップ
				-proc		昇降機デモ１ 昇降機デモ２ \
				-flag (d:FRT_ELV_W12 | d:FRT_ELV_CLOSE)

			mesg フォーチュン戦昇降機 昇降機 上昇	// 上昇

			command 追加ＨＺＸグループ登録 -hzx_id 0  -add_id 22
			command 追加ＨＺＸグループ登録 -hzx_id 17 -add_id 20

			// chara 透明壁 -p 頂点１Ｘ 頂点１Ｙ 頂点１Ｚ 頂点１高さ 頂点２Ｘ 頂点２Ｙ 頂点２Ｚ 頂点２高さ -a アトリビュート -m マップ名
			chara 透明壁 柵越え禁止壁 \
				-p -2000 6000 -10500 5000 \
				    2000 6000 -10500 5000 \
				-a (d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_PLAYER))

		} else {
			#if d:DEBUG_PRINT
				print '##### No Elevator #####'
			#endif

			//---------------------------------------------------------
			// 昇降機はない

			// 柵
			@SS_プットオブジェ エレベーターの柵 d:OBJ_NAME6

			command 追加ＨＺＸグループ登録 -hzx_id 0  -add_id 3
			command 追加ＨＺＸグループ登録 -hzx_id 17 -add_id 20

			// chara 透明壁 -p 頂点１Ｘ 頂点１Ｙ 頂点１Ｚ 頂点１高さ 頂点２Ｘ 頂点２Ｙ 頂点２Ｚ 頂点２高さ -a アトリビュート -m マップ名
			chara 透明壁 柵越え禁止壁 \
				-p -2000 6000 -10500 5000 \
				    2000 6000 -10500 5000 \
				-a (d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_PLAYER))

			// エフェクトバウンド
			command エフェクトバウンド追加 -2000 -50000 -14000 2000 -40000 -10500 1	/* １だと新規として０だと追加登録できる */
			command エフェクトバウンド実行
		}
	} else {
		if($s:登場ポイント == sp_w11b_w12a_0) {
			#if d:DEBUG_PRINT
				print '##### On Elevator Go Up: Stay #####'
			#endif

			//---------------------------------------------------------
			// 昇降機で登場

			chara フォーチュン戦昇降機 昇降機 \
				-e_model	d:ELV_NAME \
				-e_top		0, 50000, 0 \
				-e_pos		0, 25000, 0 \
				-e_bottom	0,     0, 0 \
				-e_dir 1 \
				-e_rot		0, 0, 0 \
				-trap		ev006 ev007 \	// 起動トラップ 敵兵チェックトラップ
				-proc		昇降機デモ１ 昇降機デモ２ \
				-flag (d:FRT_ELV_W12 | d:FRT_ELV_CLOSE)

			mesg フォーチュン戦昇降機 昇降機 上昇	// 上昇

		} else {
			#if d:DEBUG_PRINT
				print '##### Stay #####'
			#endif

			//---------------------------------------------------------
			// 昇降機は上で待機中

			chara フォーチュン戦昇降機 昇降機 \
				-e_model	d:ELV_NAME \
				-e_top		0, 50000, 0 \
				-e_bottom	0, 0, 0 \
				-e_rot		0, 0, 0 \
				-trap		ev006 ev007 \	// 起動トラップ 敵兵チェックトラップ
				-proc		昇降機デモ１ 昇降機デモ２ \
				-flag (d:FRT_ELV_W12 | d:FRT_ELV_OPEN)

			@シナリオデモキャラセット
		}
	}
}

//-----------------------------------------------------------------------------
// 昇降機デモ

proc 昇降機デモ１ {
#if d:DEBUG_PRINT
	print '*** Elevator Demo 1 ***'
#endif

	//---------------------------------------------------------
	// 昇降機が発車するため、フェンスが上がった時

	if($w:p_story < d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) {
		//##### フォーチュン戦後以外の降下時 #####

		// 進入禁止ゾーンを設定
		command セットゾーンフラグ d:HZX_ZONE_ZINTRPT 0 5000 -12250

		@シナリオデモ開始処理
		@ＳＥＴエレベータ状態 (d:_E_ON_DOWN_DEMO)

		command 死亡カモメ非表示

		// エフェクトバウンド
		command エフェクトバウンド追加 -2000 5000 -14000 2000 10000 -10500 1	/* １だと新規として０だと追加登録できる */
		command エフェクトバウンド実行
		command フォーチュン戦昇降機ボム消去
#if d:DEBUG_PRINT
	print '*** Bomb Erase ***'
#endif

		command プレイヤー状態取得
		if($status & d:PFLAG_CAUTION) {
			mesg プレイヤー d:PLAYER stance 0 $w:プレイヤー方向 $w:プレイヤー方向 0
		}

		command パッド操作 -release
		mesg 髪の毛モデル ライデン髪の毛 パラメータチェンジ 24
		@爆弾タイマー残り時間保存

	} else {
		//##### フォーチュン戦後、誰も乗せずに降下時 #####

		@ＳＥＴエレベータ状態 (d:_E_GO_DOWN)

		// エフェクトバウンド
		command エフェクトバウンド追加 -2000 -40000 -14000 2000 0 -10500 1	/* １だと新規として０だと追加登録できる */
		command エフェクトバウンド実行

		// エレベータ降下の監視
		chara プロック連続実行 連続実行３ \
			-time -1 \
			-exec {
				if(`@ＧＥＴエレベータ状態 (d:_E_GO_DOWN)`) {
					if(`%フォーチュン戦昇降機位置取得` > 50000) {
						#if d:DEBUG_PRINT
							print '*** > 50000 ***'
						#endif

					} else if(`%フォーチュン戦昇降機位置取得` < 15000) {
						#if d:DEBUG_PRINT
							print '*** < 15000 ***'
						#endif

						@ＳＥＴエレベータ状態 (d:_E_NORMAL)

						// エフェクトバウンド
						command エフェクトバウンド追加 -2000 -50000 -14000 2000 -40000 -10500 1	/* １だと新規として０だと追加登録できる */

					} else if(`%フォーチュン戦昇降機位置取得` < 20000) {
						#if d:DEBUG_PRINT
							print '*** < 20000 ***'
						#endif
					} else if(`%フォーチュン戦昇降機位置取得` < 30000) {
						#if d:DEBUG_PRINT
							print '*** < 30000 ***'
						#endif
					} else if(`%フォーチュン戦昇降機位置取得` < 40000) {
						#if d:DEBUG_PRINT
							print '*** < 40000 ***'
						#endif
					} else {
						#if d:DEBUG_PRINT
							print '*** < 50000 ***'
						#endif
					}
				}
			}
	}
}
proc 昇降機デモ２ {
#if d:DEBUG_PRINT
	print '*** Elevator Demo 2 ***'
#endif

	//---------------------------------------------------------
	// 昇降機が到着して、フェンスが降りる時

	if(`@ＧＥＴエレベータ状態 (d:_E_ON_UP_DEMO)`) {
		chara delay ディレイ \
			-time 60 \
			-exec {
				mesg プレイヤー d:PLAYER run 0 5000 -9000 1 0 d:FA_DIRECT_MOVE 昇降機デモ３
			}
	}
}
proc 昇降機デモ３ {
#if d:DEBUG_PRINT
	print '*** Elevator Demo 3 ***'
#endif

	//---------------------------------------------------------
	// 昇降機を降りる強制移動後

	if($s:登場ポイント == sp_w11c_w12a_0) {
		//##### フォーチュン戦後 #####

		chara delay ディレイ \
			-time 30 \
			-exec {
				// 昇降機降下
				mesg フォーチュン戦昇降機 昇降機 降下
			}

		chara delay ディレイ \
			-time 180 \
			-exec {
				@ステージ開始時処理終了（昇降機）
				@シナリオデモ終了処理
				@爆弾タイマー開始

				chara カメラ設定 でもデモカメラ -s 0
			}

	} else {
		//##### フォーチュン戦後以外 #####

		chara delay ディレイ \
			-time 30 \
			-exec {
				@ステージ開始時処理終了（昇降機）
				@シナリオデモ終了処理
				@爆弾タイマー開始

				chara カメラ設定 でもデモカメラ -s 0

				// 昇降機デモ終了
				@ＳＥＴエレベータ状態 (d:_E_NORMAL)
			}
	}
}

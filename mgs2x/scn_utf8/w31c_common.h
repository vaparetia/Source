/*
	w31c_common.h
		シェル２中央棟Ｂ１第２濾過室-デモとの共通プロック

	2001/07/28 H.Yoshiike
	$Id: w31c_common.h,v 1.36 2002/08/19 06:31:21 usr03682 Exp $


*/


// 全マップ共通
// ------------------------------------
	// ドア＆マップ切り替え関係
	// ------------------------------------
	// ゲーム部分
	proc ドア設置 {
		#if d:DEBUG_PRINT
			print 'slide_door_set'
		#endif

		#ifndef d:MISSION_TALE
			if ( $w:p_story < d:ST:P057_02_R01ヴァンプ戦終了２無線機デモ１終了 ) {
				// ヴァンプ戦が終わるまでは扉開かない
				eval ( $i:ローカル変数Ｉ２ = 9999 );
			} else {
				// ヴァンプ戦が終わったら開く
				eval ( $i:ローカル変数Ｉ２ = 4 );
			}
		#else
			// スネークテイルズでは必ず開かない
			eval ( $i:ローカル変数Ｉ２ = 9999 );
		#endif

		@ドアランプ設定 南側ドアランプ 0 -9250,-6250,-232375 2
		#ifndef d:DEMO_STAGE
			chara ドア 南側ドア \
				#ifndef d:MISSION_TALE
				-m d:DOOR_NAME_LV4 \	// 本編
				#else
				-m d:DOOR_NAME_LV0 \	// スネークテイルズ
				#endif
				-d 0,0,0 \
				-p -9250,-6250,-232375 \
				-slide d:SD_SLIDE_INSIDE \
				-time d:SD_OPEN_TIME \
				-between 第２濾過層 第２濾過層 \
				-A 南側ドアランプ \
				-card_level $i:ローカル変数Ｉ２ \
				-exec {
				}
			trap dr001 ？ \
				-mask ？ \
				-exec {
						if($3==入る){
							mesg ドア 南側ドア open $2
						}else{
							if ( $f:エマゲームオーバーフラグ == 0 ) {
								mesg ドア 南側ドア close $2
							}
						}
				}
			trap dr001 d:PLAYER -mask ？ \
				-state d:TRP_STATE_SUBJECT \
				-exec {
					if($3 == 入る) {
						mesg 透明壁 南側ドアへの透明壁 on
					} else if($3 == 出る) {
						mesg 透明壁 南側ドアへの透明壁 off
					}
				}
		#else
			chara プットオブジェ 南側ドア \
				-m d:DOOR_NAME_LV4 \
				-r 0,0,0 \
				-p -9250,-6250,-232375 \
				-s 100,100,100 \
				-l d:LT2_NAME
		#endif

		@ドアランプ設定 北側ドアランプ 0 -6250,-6250,-246125 2
		#ifndef d:DEMO_STAGE
			chara ドア 北側ドア \
				#ifndef d:MISSION_TALE
				-m d:DOOR_NAME_LV4 \	// 本編
				#else
				-m d:DOOR_NAME_LV0 \	// スネークテイルズ
				#endif
				-d 0,0,0 \
				-p -6250,-6250,-246125 \
				-slide d:SD_SLIDE_INSIDE \
				-time d:SD_OPEN_TIME \
				-between 第２濾過層 北西エリア \
				-A 北側ドアランプ \
				-card_level $i:ローカル変数Ｉ２ \
				-exec {
					if( $:開閉フラグ == 0 ){
						mesg マップ接合 第２濾過層／北西エリア off
					} else {
						mesg マップ接合 第２濾過層／北西エリア on
					}
				}
			trap dr002 ？ \
				-mask ？ \
				-exec {
						if($3==入る){
							mesg ドア 北側ドア open $2
						}else{
							if ( $f:エマゲームオーバーフラグ == 0 ) {
								mesg ドア 北側ドア close $2
							}
						}
				}
		#else
			chara プットオブジェ 北側ドア \
				-m d:DOOR_NAME_LV4 \
				-r 0,0,0 \
				-p -6250,-6250,-246125 \
				-s 100,100,100 \
				-l d:LT2_NAME
		#endif

		if ( $w:p_story < d:ST:P058_04_R02エマ救出４無線機デモ２開始 ) {
			@ドアランプ設定 ロッカー部屋ドアランプ 1024 -10625,-6250,-251000 2
		} else {
			@壊れドアエフェクト設定 壊れドアエフェクト 1024 -10625,-6250,-251000 2
		}
		#ifndef d:DEMO_STAGE
			if ( $w:p_story < d:ST:P058_04_R02エマ救出４無線機デモ２開始 ) {
				chara ドア ロッカー部屋ドア \
					-m d:DOOR_NAME_LV0 \
					-d 0,1024,0 \
					-p -10625,-6250,-251000 \
					-slide d:SD_SLIDE_INSIDE \
					-time d:SD_OPEN_TIME \
					-between 北西エリア 北西エリア \
					-A ロッカー部屋ドアランプ \
					-exec {
					}
			} else {
				chara ドア ロッカー部屋ドア \
					-m d:DOOR_NAME_LV0 \
					-d 0,1024,0 \
					-p -10625,-6250,-251000 \
					-slide d:SD_SLIDE_INSIDE \
					-time d:SD_OPEN_TIME \
					-between 北西エリア 北西エリア \
					-card_level 9999 \
					-exec {
					}
			}
			trap dr101 ？ \
				-mask ？ \
				-exec {
						if($3==入る){
							#if d:DEBUG_PRINT
								print 'SD_CODE d:SD_AUTO_FADER1'
							#endif
							mesg ドア ロッカー部屋ドア open
							mesg サウンドマネージャー ＳＤマネ SD_CODE d:SD_AUTO_FADER1
						}else{
							mesg ドア ロッカー部屋ドア close
							if ( $i:プレイヤー位置Ｘ < -10625 ) {
								#if d:DEBUG_PRINT
									print 'SD_CODE d:SD_AUTO_FADER3'
								#endif
								mesg サウンドマネージャー ＳＤマネ SD_CODE d:SD_AUTO_FADER3
							}
						}
				}
		#else
			chara プットオブジェ ロッカー部屋ドア \
				-m d:DOOR_NAME_LV0 \
				-r 0,1024,0 \
				-p -10625,-6250,-251000 \
				-s 100,100,100 \
				-l d:LT2_NAME
		#endif
	}

	proc 主観禁止壁設定 {
		@主観禁止壁 南側ドアへの透明壁 -9250 -6250 -232375 0 d:SD_SLIDE_INSIDE
	}


	// フォグ
	// ------------------------------------
	proc フォグ設定 {
		#if d:DEBUG_PRINT
			print 'fog_set'
		#endif

		chara フォグコントローラ フォグ \
				-n 7 \
				-z \
				-d   10 25 20	-3000  28000  32000 32000 \	// 第２濾過室
					 48 112 96	-5800  41000  16000  4000 \	// 水中
					 40 39 28	0  36000 36000 36000 \	// 北西エリア
					 48 112 96  -5800  35000 16000 4000 \
					 48 112 96  -5800  28000 16000 4000 \
					 48 112 96  -5800  22000 16000 4000 \
					 48 112 96  -5800  15000 16000 4000

		#ifndef d:DEMO_STAGE
			trap wa100 d:PLAYER \
				-mask ＊ \
				-exec {
					command プレイヤー状態取得
					if (( $status & d:PFLAG_WATER ) && (!( $status & d:PFLAG_WATER_SURFACE ))){
						// 水中モードであるが水面ではない
						if( $4 > -5500 ) {
							if( $6 > -252000 ) {
								if( $b:w31c_フォグコントロールフラグ != 1 ) {
									eval( $b:w31c_フォグコントロールフラグ = 1 )
									mesg フォグコントローラ フォグ セット 1 30
									print 'フォグ 1'
								}
							} else if( $6 > -254000 ) {
								if( $b:w31c_フォグコントロールフラグ != 2 ) {
									eval( $b:w31c_フォグコントロールフラグ = 2 )
									mesg フォグコントローラ フォグ セット 3 30
									print 'フォグ 3'
								}
							} else if( $6 > -256000 ) {
								if( $b:w31c_フォグコントロールフラグ != 3 ) {
									eval( $b:w31c_フォグコントロールフラグ = 3 )
									mesg フォグコントローラ フォグ セット 4 30
									print 'フォグ 4'
								}
							} else if( $6 > -258000 ) {
								if( $b:w31c_フォグコントロールフラグ != 4 ) {
									eval( $b:w31c_フォグコントロールフラグ = 4 )
									mesg フォグコントローラ フォグ セット 5 30
									print 'フォグ 5'
								}
							} else {
								if( $b:w31c_フォグコントロールフラグ != 5 ) {
									eval( $b:w31c_フォグコントロールフラグ = 5 )
									mesg フォグコントローラ フォグ セット 6 30
									print 'フォグ 6'
								}
							}
						} else {
							if( $b:w31c_フォグコントロールフラグ != 6 ) {
								print'ノーマル水中フォグ'
								eval( $b:w31c_フォグコントロールフラグ = 6 )
								mesg フォグコントローラ フォグ セット 1 0
							}
						}
					} else {
						if( $b:w31c_フォグコントロールフラグ != 0 ) {
							print '水面に出た'
							eval( $b:w31c_フォグコントロールフラグ = 0 )
							if ( $6 > -248875 ) {
								mesg フォグコントローラ フォグ セット 0 0
							} else {
								mesg フォグコントローラ フォグ セット 2 0
							}
						}
					}
				}
		#endif

	}

	// 水位変更プロック(各キャラ起動後はトラップによって、水位を変えていく)
	// ------------------------------------
	proc 水位管理設定 {
		// ヴァンププールにいるときは無条件に-7000にする
		trap wa001 d:PLAYER \
			-mask ？ \
			-exec {
				if ($3 == 入る) {
					#if d:DEBUG_PRINT
						print 'change_water_level -7000'
					#endif
					command 水位設定 -lv d:VAMP_WATER_SURFACE -s d:se_code:SD_A_RICOCH06
					eval( $i:global_水面高さ = d:VAMP_WATER_SURFACE );
				}
			}

		// ヴァンププール前の廊下にいるときは、主観時の角度とプレイヤー位置の関係によって水位を変化させる
		trap wa201 d:PLAYER \
			-mask いる \
			-exec {
				if ( $i:プレイヤー位置Ｘ < -9000 ) {
				// プレイヤーが水間際だったら無条件に水位を上げる
					if (  $i:global_水面高さ != d:WATER_SURFACE ) {
						#if d:DEBUG_PRINT
							print 'change_water_level -6550'
						#endif
						command 水位設定 -lv d:WATER_SURFACE -s d:se_code:SD_A_RICOCH06
						eval( $i:global_水面高さ = d:WATER_SURFACE );
					}

				} else if ( `command プレイヤー状態取得` & d:PFLAG_SUBJECT ) {
					if ( ($w:プレイヤー方向 & 4095) > 2048 && \
						 ($w:プレイヤー方向 & 4095) < (-((6*$i:カメラＺ位置)/25)-56028) ) {
						// 主観で階段側の水面を狙っている
						if ( $i:global_水面高さ != d:WATER_SURFACE ) {
							#if d:DEBUG_PRINT
								print 'change_water_level -6550'
							#endif
							command 水位設定 -lv d:WATER_SURFACE -s d:se_code:SD_A_RICOCH06
							eval( $i:global_水面高さ = d:WATER_SURFACE );
						}
					} else {
						// 主観で階段側の水面を見ていない
						if ( $i:global_水面高さ != d:VAMP_WATER_SURFACE ) {
							#if d:DEBUG_PRINT
								print 'change_water_level -7000'
							#endif
							command 水位設定 -lv d:VAMP_WATER_SURFACE -s d:se_code:SD_A_RICOCH06
							eval( $i:global_水面高さ = d:VAMP_WATER_SURFACE );
						}
					}

				} else if ( `command プレイヤー状態取得` & d:PFLAG_BEHIND ) {
					// ビハインド中だったら無条件に-7000
					if ( $i:global_水面高さ != d:VAMP_WATER_SURFACE ){
						#if d:DEBUG_PRINT
							print 'change_water_level -7000'
						#endif
						command 水位設定 -lv d:VAMP_WATER_SURFACE -s d:se_code:SD_A_RICOCH06
						eval( $i:global_水面高さ = d:VAMP_WATER_SURFACE );
					}

				} else if ( $i:プレイヤー位置Ｚ < -246125 ) {
					// それ以外の俯瞰画面で廊下側なら-6550
					if ( $i:global_水面高さ != d:WATER_SURFACE ) {
						#if d:DEBUG_PRINT
							print 'change_water_level -6550'
						#endif
						command 水位設定 -lv d:WATER_SURFACE -s d:se_code:SD_A_RICOCH06
						eval( $i:global_水面高さ = d:WATER_SURFACE );
					}
				} else {
					if ( $i:global_水面高さ != d:VAMP_WATER_SURFACE ) {
						#if d:DEBUG_PRINT
							print 'change_water_level -7000'
						#endif
						command 水位設定 -lv d:VAMP_WATER_SURFACE -s d:se_code:SD_A_RICOCH06
						eval( $i:global_水面高さ = d:VAMP_WATER_SURFACE );
					}

				}
			}

		// エマロッカー前階段付近にいるときは、主観時の角度とプレイヤー位置の関係によって水位を変化させる
		trap wa202 d:PLAYER \
			-mask いる \
			-exec {
				if ( $i:プレイヤー位置Ｚ < -252500 && $i:カメラＺ位置 < -252500 ) {
				// プレイヤーが水間際だったら無条件に水位を上げる
					if ( $i:global_水面高さ != d:WATER_SURFACE ) {
						#if d:DEBUG_PRINT
							print 'change_water_level -6550'
						#endif
						command 水位設定 -lv d:WATER_SURFACE -s d:se_code:SD_A_RICOCH06
						eval( $i:global_水面高さ = d:WATER_SURFACE );
					}

				} else if ( `command プレイヤー状態取得` & d:PFLAG_SUBJECT ) {
					if ( ($w:プレイヤー方向 & 4095) > 1024 && \
						 ($w:プレイヤー方向 & 4095) < ((3*$i:カメラＸ位置)/10)+5350 ) {
						// 主観で階段側の水面を狙っている
						if ( $i:global_水面高さ != d:WATER_SURFACE ) {
							#if d:DEBUG_PRINT
								print 'change_water_level -6550'
							#endif
							command 水位設定 -lv d:WATER_SURFACE -s d:se_code:SD_A_RICOCH06
							eval( $i:global_水面高さ = d:WATER_SURFACE );
						}
					} else {
						// 主観で階段側の水面を見ていない
						if ( $i:global_水面高さ != d:EMMA_WATER_SURFACE ) {
							#if d:DEBUG_PRINT
								print 'change_water_level -7100'
							#endif
							command 水位設定 -lv d:EMMA_WATER_SURFACE -s d:se_code:SD_A_RICOCH06
							eval( $i:global_水面高さ = d:EMMA_WATER_SURFACE );
						}
					}

				} else if ( $i:プレイヤー位置Ｘ >= -10625 ) {
					// それ以外の俯瞰画面では階段側にいるときだけ水位を上げる
					if ( $i:global_水面高さ != d:WATER_SURFACE ) {
						#if d:DEBUG_PRINT
							print 'change_water_level -6550'
						#endif
						command 水位設定 -lv d:WATER_SURFACE -s d:se_code:SD_A_RICOCH06
						eval( $i:global_水面高さ = d:WATER_SURFACE );
					}

				} else {
					// ロッカー部屋で俯瞰の時には水位を下げる
					if ( $i:global_水面高さ != d:EMMA_WATER_SURFACE ){
						#if d:DEBUG_PRINT
							print 'change_water_level -7100'
						#endif
						command 水位設定 -lv d:EMMA_WATER_SURFACE -s d:se_code:SD_A_RICOCH06
						eval( $i:global_水面高さ = d:EMMA_WATER_SURFACE );
					}

				}
			}

	}


	// キャラ用光源(マップの設置が終わってから設定することで、キャラだけに光を当てる)
	// ------------------------------------
	proc キャラ用ライト設置 {
		#if d:DEBUG_PRINT
			print 'chara_light_set'
		#endif

//		chara 環境光 環境光源 \
//			 -c 86 96 81

		chara 平行光 平行光源 \
			 -c 86 96 81 \
			 -p -1566 -3626 -3065
	}

	//


// 各マップ別設定
// ------------------------------------
// 第２濾過層
	// 天井設置
	// ------------------------------------
	proc 第２濾過層天井設置 {
		#if d:DEBUG_PRINT
			print 'w31c0_ceil_set'
		#endif

		#ifndef d:DEMO_STAGE
			chara 天井君 第２濾過層天井 -m d:CEIL_NAME0 -h 1000 -p 0,0,0 -l d:LT2_NAME
		#else
			chara プットオブジェ 第２濾過層天井 -m d:CEIL_NAME0 -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME
		#endif
	}


	// 水面設置
	// ------------------------------------
	proc 第２濾過層水面設定 {
		#if d:DEBUG_PRINT
			print 'w31c0_water_set'
			print 'water_level'
			print d:VAMP_WATER_SURFACE
		#endif

		command 水位設定 -lv d:VAMP_WATER_SURFACE -s d:se_code:SD_A_RICOCH06

		#ifndef d:DEMO_STAGE
			if ( $w:p_story < d:ST:P057_02_R01ヴァンプ戦終了２無線機デモ１終了 ) {
				// ヴァンプがいるときに設置
				chara バンププール 濾過層水面 \
					-b -16750 -245500 -3250 -232500 \
					-n ヴァンプ君

				chara 泡で波紋管理 NewRipBubbleMan \
					-n ヴァンプ君
			} else {
				chara バンププール 濾過層水面 \
					-b -16750 -245500 -3250 -232500 \
			}

		#else
			chara バンププール 濾過層水面 \
				-b -16750 -245500 -3250 -232500 \

		#endif

		// 濾過層ステージ写りこみ&テクスチャアニメ
		chara プットオブジェ 写りこみモデル -m w31c0_water_ref -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME
		chara アニメテクスチャセット 第二濾過層水面写りこみ \
			-tex w31c caust00_alp_add_ovl \
			-anmtex w31c \
			-interval 1 \
			-flag 0x0001

	}


	// スポットライトの設定
	// ------------------------------------
	proc 第２濾過層スポットライト設定 {
		#if d:DEBUG_PRINT
			print 'w31c0_spot_light_set'
		#endif

		// スポットライト
		command foreach -argc 8 -repeat 4 \
			-data { \
				{ 0, 濾過層ライト:01, w31c0_k0, -5000, -3000, -242250, 2972, 0x0001 }
				{ 1, 濾過層ライト:02, w31c0_k0, -15000,-3000, -242250, 1124, 0x0001 }
				{ 2, 濾過層ライト:03, w31c0_k1, -5000, -3000, -236250, 3322, 0x0001 }
				{ 3, 濾過層ライト:04, w31c0_k1, -15000,-3000, -236250, 874, 0x0001 }
			} \
			-exec { \
				@壊れ物再セットチェック $b:w31c_破壊ロード回数[$1]
				if ( $f:再セットフラグ ) {
					chara スポットライト投影モデル $2 \
						-m $3 \
						-p 0,0,0

					chara 静止スポットライト設置 $2 \
						-pos $4, $5, $6 \
						-dir 497,$7,0 \
						-range 6000 \
						-angle 25 \
						-color 204,205,188 \
						-option $8
				}
			}

		// スポットライトモデル
		command foreach -argc 6 -repeat 4 \
			-data { \
				{ 0 , スポットライト:01, -5000, -3000, -242250, 3072 }
				{ 1 , スポットライト:02, -15000, -3000, -242250, 1024 }
				{ 2 , スポットライト:03, -5000, -3000, -236250, 3072 }
				{ 3 , スポットライト:04, -15000, -3000, -236250, 1024 }
			} \
			-exec { \
				@壊れ物再セットチェック $b:w31c_破壊ロード回数[$1]
				chara プットスポットライトオブジェ $2 \
					-m w31c0_light null \
					-t $3,($4+100),$5 120 2000 200 \
					-r 0 $6 0 \
					-l d:LT2_NAME \
//					-f ( d:BRK_LGT_SHOWTRGT ) \
//					-f ( d:BRK_LGT_OBJ_FITALL ) \
					-b ライト破壊後処理 \
					-p $3 $4 $5

				if ( $f:再セットフラグ == 0 ) {
				// 再セット回数に行っていなければ破壊する
					command プットスポットライトオブジェライト消す -n $2
				}
			}
	}

	proc ライト破壊後処理 {
		#if d:DEBUG_PRINT
			print 'w31c0_light_broken'
		#endif

		if ( $1 == スポットライト:01 ) {
			eval( $b:w31c_破壊ロード回数[0] = $w:グローバルロード回数 );
			mesg 静止スポットライト設置 濾過層ライト:01 off
		} else if ( $1 == スポットライト:02 ) {
			eval( $b:w31c_破壊ロード回数[1] = $w:グローバルロード回数 );
			mesg 静止スポットライト設置 濾過層ライト:02 off
		} else if ( $1 == スポットライト:03 ) {
			eval( $b:w31c_破壊ロード回数[2] = $w:グローバルロード回数 );
			mesg 静止スポットライト設置 濾過層ライト:03 off
		} else if ( $1 == スポットライト:04 ) {
			eval( $b:w31c_破壊ロード回数[3] = $w:グローバルロード回数 );
			mesg 静止スポットライト設置 濾過層ライト:04 off
		}
	}


// 北西エリア
	// 天井設置
	// ------------------------------------
	proc 北西エリア天井設置 {
		#if d:DEBUG_PRINT
			print 'w31c1_ceil_set'
		#endif

		#ifndef d:DEMO_STAGE
			chara 天井君 北西エリア天井 -m d:CEIL_NAME1 -h -2250 -p 0,0,0 -l d:LT2_NAME
		#else
			chara プットオブジェ 北西エリア天井 -m d:CEIL_NAME1 -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME
		#endif
	}

	// ブラインド設置
	// ------------------------------------
	proc ブラインド設置 {
		#if d:DEBUG_PRINT
			print 'w31c1_blind_set'
		#endif

		if ( $w:p_story < d:ST:P058_04_R02エマ救出４無線機デモ２開始 ) {
			chara プットオブジェ エマロッカーブラインド -m w31c1_blind -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME
		} else {
			chara プットオブジェ エマロッカーブラインド壊れ -m w31c1_blind_brk -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME
		}

	}

	// モアイ設置
	// ------------------------------------
	proc 北西エリアモアイ設置 {
		#if d:DEBUG_PRINT
			print 'w31c1_moai_set'
		#endif

		chara プットオブジェ モアイ君 -m moai -r 0,0,0 -p -12100,-6950,-259850 -s 100,100,100 -l d:LT2_NAME
	}

	// 水面設置
	// ------------------------------------
	proc 北西エリア水面設置 {
		#if d:DEBUG_PRINT
			print 'w31c1_water_set'
		#endif

		// スケールは10500mm=256だが、なぜかうまくいかないので、目で見て少し値を補正しました。
		command foreach -argc 5 -repeat 3 \
			-data { \
					{キラキラ水面:01,((-17500-11000)/2-100),d:WATER_SURFACE,((-251500-245500)/2),145} \
					{キラキラ水面:02,-4000,d:WATER_SURFACE,-250250,45} \
					{キラキラ水面:03,((-10500-3500)/2),d:WATER_SURFACE,((-260000-253000)/2),171} \
				} \
			-exec { \
				chara 通路用水面 $1 \
					-tex water w31c_ref_water_alp_ovl_01 \
					-scale $5 \
					-pos $2,$3,$4 \
					-wave 80 170 24 \
					-alpha_blend \
					-force 50
			}
	}

	// スポットライトの設定
	// ------------------------------------
	proc 北西エリアスポットライト設定 {
		#if d:DEBUG_PRINT
			print 'w31c1_spot_light_set'
		#endif

		// スポットライト
		command foreach -argc 6 -repeat 2 \
			-data { \
				{ 息継ぎライト:01, w31c1_k1, -16500, -5000, -250250, 0x0001 }
				{ 息継ぎライト:02, w31c1_k1, -4000,  -5000, -250250, 0x0001 }
			} \
			-exec { \
				chara スポットライト投影モデル $1 \
					-m $2 \
					-p 0,0,0

				chara 静止スポットライト設置 $1 \
					-pos $3, $4, $5 \
					-dir 1000,2048,0 \
					-range 7878 \
					-angle 13 \
					-color 38,87,100 \
					-option $6
			}

		chara スポットライト投影モデル エマ部屋ライト \
			-m w31c1_k0 \
			-p 0,0,0

		chara 静止スポットライト設置 エマ部屋ライト \
			-pos -16500, -4981, -250622 \
			-dir 778,2048,0 \
			-range 5000 \
			-angle 40 \
			-color 39,39,34 \
			-option 0x0001
	}

	// ロッカーの設定
	// ------------------------------------
	// モーションリスト
	enum 横開きロッカーモーションリスト {
		locker_sl_rai_open = 0,
		locker_sl_rai_close
	}

	proc 北西エリアロッカー設定 {
		#if d:DEBUG_PRINT
			print 'w31c1_locker_set'
		#endif

		command foreach -argc 10 -repeat 8 \
			-data { \
				{ 0, ロッカー:01, -17800, -254000, 3072, ロッカー壁:01, -17800, -254000, -17800, (-254000-1000) }
				{ 1, ロッカー:02, -17800, -256000, 3072, ロッカー壁:02, -17800, -256000, -17800, (-256000-1000) }
				{ 2, ロッカー:03, -17800, -258000, 3072, ロッカー壁:03, -17800, -258000, -17800, (-258000-1000) }
				{ 3, ロッカー:04, -16750, -259050, 2048, ロッカー壁:04, -16750, -259000, (-16750+1000), -259000 }
				{ 4, ロッカー:05, -14750, -259050, 2048, ロッカー壁:05, -14750, -259000, (-14750+1000), -259000 }
				{ 5, ロッカー:06, -12750, -259050, 2048, ロッカー壁:06, -12750, -259000, (-12750+1000), -259000 }
				{ 6, ロッカー:07, -11700, -258000, 1024, ロッカー壁:07, -11700, -258000, -11700, (-258000+1000) }
				{ 7, ロッカー:08, -11700, -256000, 1024, ロッカー壁:08, -11700, -256000, -11700, (-256000+1000) }
			} \
			-exec { \
				// エマ救出デモステージではエマのいるロッカーだけ扉を置かない
				if ( $s:d_num == p058_01_p01 || $s:d_num == p058_03_p02 ) {
					if ( $2 != ロッカー:05 ) {
						if ( $f:w31c_ロッカー開閉フラグ[$1] == d:LOCKER_CLOSE ) {
							chara プットモーションモデル $2 \
								-pos $3, -7000, $4 \
								-dir $5 \
								-kms w31c1_locker_door \ /*evmを表示する場合はダミーKMSが必要*/
								-mar locker_sl \
								-number d:横開きロッカーモーションリスト:locker_sl_rai_open \ /*省略時０*/
								-s (d:PUT_MOT_WAIT | d:PUT_MOT_PRESHADE )
						} else {
							// あいている状態のときはシフト値の関係で方向ごとに設定
							if ( $5 == 3072 ) {
								chara プットモーションモデル $2 \
									-pos $3, -7000, $4 \
									-a 0, 0, 950 \
									-dir $5 \
									-kms w31c1_locker_door \ /*evmを表示する場合はダミーKMSが必要*/
									-mar locker_sl \
									-number d:横開きロッカーモーションリスト:locker_sl_rai_close \ /*省略時０*/
									-s (d:PUT_MOT_WAIT | d:PUT_MOT_PRESHADE)
							} else if ( $5 == 2048 ) {
								chara プットモーションモデル $2 \
									-pos $3, -7000, $4 \
									-a -950, 0, 0 \
									-dir $5 \
									-kms w31c1_locker_door \ /*evmを表示する場合はダミーKMSが必要*/
									-mar locker_sl \
									-number d:横開きロッカーモーションリスト:locker_sl_rai_close \ /*省略時０*/
									-s (d:PUT_MOT_WAIT | d:PUT_MOT_PRESHADE)
							} else if ( $5 == 1024 ) {
								chara プットモーションモデル $2 \
									-pos $3, -7000, $4 \
									-a 0, 0, -950 \
									-dir $5 \
									-kms w31c1_locker_door \ /*evmを表示する場合はダミーKMSが必要*/
									-mar locker_sl \
									-number d:横開きロッカーモーションリスト:locker_sl_rai_close \ /*省略時０*/
									-s (d:PUT_MOT_WAIT | d:PUT_MOT_PRESHADE)
							}
						}
					}
				} else {
				// ゲームおよびその他のデモではエマのロッカーも普通に置く
					if ( $f:w31c_ロッカー開閉フラグ[$1] == d:LOCKER_CLOSE ) {
						chara プットモーションモデル $2 \
							-pos $3, -7000, $4 \
							-dir $5 \
							-kms w31c1_locker_door \ /*evmを表示する場合はダミーKMSが必要*/
							-mar locker_sl \
							-number d:横開きロッカーモーションリスト:locker_sl_rai_open \ /*省略時０*/
							-s (d:PUT_MOT_WAIT | d:PUT_MOT_PRESHADE )
					} else {
						// あいている状態のときはシフト値の関係で方向ごとに設定
						if ( $5 == 3072 ) {
							chara プットモーションモデル $2 \
								-pos $3, -7000, $4 \
								-a 0, 0, 950 \
								-dir $5 \
								-kms w31c1_locker_door \ /*evmを表示する場合はダミーKMSが必要*/
								-mar locker_sl \
								-number d:横開きロッカーモーションリスト:locker_sl_rai_close \ /*省略時０*/
								-s (d:PUT_MOT_WAIT | d:PUT_MOT_PRESHADE)
						} else if ( $5 == 2048 ) {
							chara プットモーションモデル $2 \
								-pos $3, -7000, $4 \
								-a -950, 0, 0 \
								-dir $5 \
								-kms w31c1_locker_door \ /*evmを表示する場合はダミーKMSが必要*/
								-mar locker_sl \
								-number d:横開きロッカーモーションリスト:locker_sl_rai_close \ /*省略時０*/
								-s (d:PUT_MOT_WAIT | d:PUT_MOT_PRESHADE)
						} else if ( $5 == 1024 ) {
							chara プットモーションモデル $2 \
								-pos $3, -7000, $4 \
								-a 0, 0, -950 \
								-dir $5 \
								-kms w31c1_locker_door \ /*evmを表示する場合はダミーKMSが必要*/
								-mar locker_sl \
								-number d:横開きロッカーモーションリスト:locker_sl_rai_close \ /*省略時０*/
								-s (d:PUT_MOT_WAIT | d:PUT_MOT_PRESHADE)
						}
					}
				}

				#ifndef d:DEMO_STAGE
					chara 透明壁 $6 \
						-position $7 -7000 $8 2250 \
								  $9 -7000 $10 2250 \
						-attribute ( d:HZX_SEG_NO_C4 | d:HZX_SEG_NO_BULLETHOLE ) \
						-direct \
						-seno 1 \
						-map 北西エリア

					if ( $f:w31c_ロッカー開閉フラグ[$1] == d:LOCKER_OPEN ) {
						mesg 透明壁 $6 off
					}
				#endif
			}

		#ifndef d:DEMO_STAGE
			trap lk101 d:PLAYER \
				-and \
				-mask いる \
				-state 0,4 \
				-dir 3072,512 1024,512 \
				-button d:ACTION_BUTTON, d:ACTION_BUTTON \
				-exec {
					if (`@エマ距離チェック` == d:TRUE ) {
						chara カメラ設定 モーションカメラ \
							-c 1 \
							-p -14526,-3570,-252471 -t -16931,-5507,-254514 \
							-r 359,2613,0 -f 3703 \
							-a 200 \
							-i 0 2 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s 1

						if ( $f:w31c_ロッカー開閉フラグ[0] == d:LOCKER_CLOSE ) {
							if ( `command プレイヤー状態取得` & d:PFLAG_CAUTION ) {
								chara delay ロッカーディレイ \
									-time 17 \
									-exec {
										mesg プットモーションモデル ロッカー:01 終了判定 \
											d:横開きロッカーモーションリスト:locker_sl_rai_open
									}
								mesg プレイヤー d:PLAYER motion \
									d:モーションリスト:non_sllocker_open_beheind 1024 1024 0 \
									(d:FA_NO_CHECK_TRP | d:FA_NO_CHECK_SEG) \
									(-17800+300) (-254000-445)
								command	強制モーション終了プロック \
									-motion d:モーションリスト:non_sllocker_open_beheind \
									-proc ロッカーチェック \
									-args 0
							} else {
								mesg プットモーションモデル ロッカー:01 終了判定 \
									d:横開きロッカーモーションリスト:locker_sl_rai_open
								mesg プレイヤー d:PLAYER motion \
									d:モーションリスト:non_locker_open 3072 3072 0 d:FA_NO_CHECK_TRP \
									(-17800+850) (-254000-463)
								command	強制モーション終了プロック \
									-motion d:モーションリスト:non_locker_open \
									-proc ロッカーチェック \
									-args 0
							}
						} else {
							mesg プットモーションモデル ロッカー:01 ドア閉め再生 \
								d:横開きロッカーモーションリスト:locker_sl_rai_close
							mesg プレイヤー d:PLAYER motion \
								d:モーションリスト:non_locker_close 3072 3072 0 d:FA_NO_CHECK_TRP \
								(-17800+746) (-254000-463)
							command	強制モーション終了プロック \
								-motion d:モーションリスト:non_locker_close \
								-proc ロッカーチェック \
								-args 0
						}
					}
				}

			trap lk102 d:PLAYER \
				-and \
				-mask いる \
				-state 0,4 \
				-dir 3072,512 1024,512 \
				-button d:ACTION_BUTTON, d:ACTION_BUTTON \
				-exec {
					if (`@エマ距離チェック` == d:TRUE ) {
						chara カメラ設定 モーションカメラ \
							-c 1 \
							-p -14526,-3570,-254471 -t -16931,-5507,-256514 \
							-r 359,2613,0 -f 3703 \
							-a 200 \
							-i 0 2 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s 1

						if ( `command プレイヤー状態取得` & d:PFLAG_CAUTION ) {
							mesg プレイヤー d:PLAYER motion \
								d:モーションリスト:non_sllocker_open_beheind_fail_r 1024 1024 0 \
								(d:FA_NO_CHECK_TRP | d:FA_NO_CHECK_SEG) \
								(-17800+300) (-256000-445)
							command	強制モーション終了プロック \
								-motion d:モーションリスト:non_sllocker_open_beheind_fail_r \
								-proc ロッカーチェック \
								-args 1
						} else {
							mesg プレイヤー d:PLAYER motion \
								d:モーションリスト:non_locker_open_fail_r 3072 3072 0 d:FA_NO_CHECK_TRP \
								(-17800+800) (-256000-463)

							command	強制モーション終了プロック \
								-motion d:モーションリスト:non_locker_open_fail_r \
								-proc ロッカーチェック \
								-args 1
						}
					}
				}

			trap lk103 d:PLAYER \
				-and \
				-mask いる \
				-state 0,4 \
				-dir 3072,512 1024,512 \
				-button d:ACTION_BUTTON, d:ACTION_BUTTON \
				-exec {
					if (`@エマ距離チェック` == d:TRUE ) {
						chara カメラ設定 モーションカメラ \
							-c 1 \
							-p -14526,-3570,-256471 -t -16931,-5507,-258514 \
							-r 359,2613,0 -f 3703 \
							-a 200 \
							-i 0 2 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s 1

						if ( $f:w31c_ロッカー開閉フラグ[2] == d:LOCKER_CLOSE ) {
							if ( `command プレイヤー状態取得` & d:PFLAG_CAUTION ) {
								chara delay ロッカーディレイ \
									-time 17 \
									-exec {
										mesg プットモーションモデル ロッカー:03 終了判定 \
											d:横開きロッカーモーションリスト:locker_sl_rai_open
									}
								mesg プレイヤー d:PLAYER motion \
									d:モーションリスト:non_sllocker_open_beheind 1024 1024 0 \
									(d:FA_NO_CHECK_TRP | d:FA_NO_CHECK_SEG) \
									(-17800+300) (-258000-445)
								command	強制モーション終了プロック \
									-motion d:モーションリスト:non_sllocker_open_beheind \
									-proc ロッカーチェック \
									-args 2
							} else {
								mesg プットモーションモデル ロッカー:03 終了判定 \
									d:横開きロッカーモーションリスト:locker_sl_rai_open
								mesg プレイヤー d:PLAYER motion \
									d:モーションリスト:non_locker_open 3072 3072 0 d:FA_NO_CHECK_TRP \
									(-17800+850) (-258000-463)
								command	強制モーション終了プロック \
									-motion d:モーションリスト:non_locker_open \
									-proc ロッカーチェック \
									-args 2
							}
						} else {
							mesg プットモーションモデル ロッカー:03 ドア閉め再生 \
								d:横開きロッカーモーションリスト:locker_sl_rai_close
							mesg プレイヤー d:PLAYER motion \
								d:モーションリスト:non_locker_close 3072 3072 0 d:FA_NO_CHECK_TRP \
								(-17800+746) (-258000-463)
							command	強制モーション終了プロック \
								-motion d:モーションリスト:non_locker_close \
								-proc ロッカーチェック \
								-args 2
						}
					}
				}

			trap lk104 d:PLAYER \
				-and \
				-mask いる \
				-state 0,4 \
				-dir 2048,512 0,512 \
				-button d:ACTION_BUTTON, d:ACTION_BUTTON \
				-exec {
					if (`@エマ距離チェック` == d:TRUE ) {
						chara カメラ設定 モーションカメラ \
							-c 1 \
							-p -17196,-3938,-255636 -t -16136,-5580,-258151 \
							-r 353,1788,0 -f 3185 \
							-a 200 \
							-i 0 2 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s 1

						if ( `command プレイヤー状態取得` & d:PFLAG_CAUTION ) {
							mesg プレイヤー d:PLAYER motion \
								d:モーションリスト:non_sllocker_open_beheind_fail_r 0 0 0 \
								(d:FA_NO_CHECK_TRP | d:FA_NO_CHECK_SEG) \
								(-16750+445) (-259050+300)
							command	強制モーション終了プロック \
								-motion d:モーションリスト:non_sllocker_open_beheind_fail_r \
								-proc ロッカーチェック \
								-args 3
						} else {
							mesg プレイヤー d:PLAYER motion \
								d:モーションリスト:non_locker_open_fail_r 2048 2048 0 d:FA_NO_CHECK_TRP \
								(-16750+463) (-259050+800)

							command	強制モーション終了プロック \
								-motion d:モーションリスト:non_locker_open_fail_r \
								-proc ロッカーチェック \
								-args 3
						}
					}
				}

			trap lk105 d:PLAYER \
				-and \
				-mask いる \
				-state 0,4 \
				-dir 2048,512 0,512 \
				-button d:ACTION_BUTTON, d:ACTION_BUTTON \
				-exec {
					if($w:アイテム == d:アイテム:ゴル兵制服){
						preseek 'r_plt5'
					} else {
						preseek 'd055p01'
					}
					@ポリゴンデモロード d:ST:P058_01_P01エマ救出１ポリゴンデモ１開始

/* アクションボタンを押したら直接ポリデモへ飛ばすように修正 2001.08.25

					if (`@エマ距離チェック` == d:TRUE ) {
						chara カメラ設定 モーションカメラ \
							-c 1 \
							-p -15196,-3938,-255636 -t -14136,-5580,-258151 \
							-r 353,1788,0 -f 3185 \
							-a 200 \
							-i 0 2 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s 1

						if ( $f:w31c_ロッカー開閉フラグ[4] == d:LOCKER_CLOSE ) {
							if ( `command プレイヤー状態取得` & d:PFLAG_CAUTION ) {
								chara delay ロッカーディレイ \
									-time 17 \
									-exec {
										mesg プットモーションモデル ロッカー:05 終了判定 \
											d:横開きロッカーモーションリスト:locker_sl_rai_open
									}
								mesg プレイヤー d:PLAYER motion \
									d:モーションリスト:non_sllocker_open_beheind 0 0 0 (d:FA_NO_CHECK_TRP | d:FA_NO_CHECK_SEG) \
									(-14750+445) (-259050+300)
								command	強制モーション終了プロック \
									-motion d:モーションリスト:non_sllocker_open_beheind \
									-proc ロッカーチェック \
									-args 4
							} else {
								mesg プットモーションモデル ロッカー:05 終了判定 \
									d:横開きロッカーモーションリスト:locker_sl_rai_open
								mesg プレイヤー d:PLAYER motion \
									d:モーションリスト:non_locker_open 2048 2048 0 d:FA_NO_CHECK_TRP \
									(-14750+463) (-259050+850)
								command	強制モーション終了プロック \
									-motion d:モーションリスト:non_locker_open \
									-proc ロッカーチェック \
									-args 4
							}

							if ( $w:p_story < d:ST:P058_01_P01エマ救出１ポリゴンデモ１開始 ) {
								eval( $f:w31c_ロッカー開閉フラグ[4] = d:LOCKER_OPEN );
								chara delay ロッカーディレイ \
									-time 10 \
									-exec {
										mesg プットモーションモデル 隠れエマ モーション再生 1
									}
								@ポリゴンデモロード d:ST:P058_01_P01エマ救出１ポリゴンデモ１開始
							}
						} else {
							mesg プットモーションモデル ロッカー:05 ドア閉め再生 \
								d:横開きロッカーモーションリスト:locker_sl_rai_close
							mesg プレイヤー d:PLAYER motion \
								d:モーションリスト:non_locker_close 2048 2048 0 d:FA_NO_CHECK_TRP \
								(-14750+463) (-259050+746)
							command	強制モーション終了プロック \
								-motion d:モーションリスト:non_locker_close \
								-proc ロッカーチェック \
								-args 4
						}
					}
*/
				}

			trap lk106 d:PLAYER \
				-and \
				-mask いる \
				-state 0,4 \
				-dir 2048,512 0,512 \
				-button d:ACTION_BUTTON, d:ACTION_BUTTON \
				-exec {
					if (`@エマ距離チェック` == d:TRUE ) {
						chara カメラ設定 モーションカメラ \
							-c 1 \
							-p -13196,-3938,-255636 -t -12136,-5580,-258151 \
							-r 353,1788,0 -f 3185 \
							-a 200 \
							-i 0 2 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s 1

						if ( $w:ゲーム設定 <= d:LEVEL_HARD ) {
							if ( `command プレイヤー状態取得` & d:PFLAG_CAUTION ) {
								mesg プレイヤー d:PLAYER motion \
									d:モーションリスト:non_sllocker_open_beheind_fail_r 0 0 0 \
									(d:FA_NO_CHECK_TRP | d:FA_NO_CHECK_SEG) \
									(-12750+445) (-259050+300)
								command	強制モーション終了プロック \
									-motion d:モーションリスト:non_sllocker_open_beheind_fail_r \
									-proc ロッカーチェック \
									-args 5
							} else {
								mesg プレイヤー d:PLAYER motion \
									d:モーションリスト:non_locker_open_fail_r 2048 2048 0 d:FA_NO_CHECK_TRP \
									(-12750+463) (-259050+800)

								command	強制モーション終了プロック \
									-motion d:モーションリスト:non_locker_open_fail_r \
									-proc ロッカーチェック \
									-args 5
							}
						} else {
							if ( $f:w31c_ロッカー開閉フラグ[5] == d:LOCKER_CLOSE ) {
								if ( `command プレイヤー状態取得` & d:PFLAG_CAUTION ) {
									chara delay ロッカーディレイ \
										-time 17 \
										-exec {
											mesg プットモーションモデル ロッカー:06 終了判定 \
												d:横開きロッカーモーションリスト:locker_sl_rai_open
										}
									mesg プレイヤー d:PLAYER motion \
										d:モーションリスト:non_sllocker_open_beheind 0 0 0 (d:FA_NO_CHECK_TRP | d:FA_NO_CHECK_SEG) \
										(-12750+445) (-259050+300)
									command	強制モーション終了プロック \
										-motion d:モーションリスト:non_sllocker_open_beheind \
										-proc ロッカーチェック \
										-args 5
								} else {
									mesg プットモーションモデル ロッカー:06 終了判定 \
										d:横開きロッカーモーションリスト:locker_sl_rai_open
									mesg プレイヤー d:PLAYER motion \
										d:モーションリスト:non_locker_open 2048 2048 0 d:FA_NO_CHECK_TRP \
										(-12750+463) (-259050+850)
									command	強制モーション終了プロック \
										-motion d:モーションリスト:non_locker_open \
										-proc ロッカーチェック \
										-args 5
								}
							} else {
								mesg プットモーションモデル ロッカー:06 ドア閉め再生 \
									d:横開きロッカーモーションリスト:locker_sl_rai_close
								mesg プレイヤー d:PLAYER motion \
									d:モーションリスト:non_locker_close 2048 2048 0 d:FA_NO_CHECK_TRP \
									(-12750+463) (-259050+746)
								command	強制モーション終了プロック \
									-motion d:モーションリスト:non_locker_close \
									-proc ロッカーチェック \
									-args 5
							}
						}
					}
				}

			trap lk107 d:PLAYER \
				-and \
				-mask いる \
				-state 0,4 \
				-dir 1024,512 3072,512 \
				-button d:ACTION_BUTTON, d:ACTION_BUTTON \
				-exec {
					if (`@エマ距離チェック` == d:TRUE ) {
						chara カメラ設定 モーションカメラ \
							-c 1 \
							-p -15456,-4418,-255912 -t -12723,-5420,-257073 \
							-r 212,1286,0 -f 3134 \
							-a 200 \
							-i 0 2 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s 1

						if ( `command プレイヤー状態取得` & d:PFLAG_CAUTION ) {
							mesg プレイヤー d:PLAYER motion \
								d:モーションリスト:non_sllocker_open_beheind_fail_r 3072 3072 0 \
								(d:FA_NO_CHECK_TRP | d:FA_NO_CHECK_SEG) \
								(-11700-300) (-258000+445)
							command	強制モーション終了プロック \
								-motion d:モーションリスト:non_sllocker_open_beheind_fail_r \
								-proc ロッカーチェック \
								-args 6
						} else {
							mesg プレイヤー d:PLAYER motion \
								d:モーションリスト:non_locker_open_fail_r 1024 1024 0 d:FA_NO_CHECK_TRP \
								(-11700-800) (-258000+463)

							command	強制モーション終了プロック \
								-motion d:モーションリスト:non_locker_open_fail_r \
								-proc ロッカーチェック \
								-args 6
						}
					}
				}

			trap lk108 d:PLAYER \
				-and \
				-mask いる \
				-state 0,4 \
				-dir 1024,512 3072,512 \
				-button d:ACTION_BUTTON, d:ACTION_BUTTON \
				-exec {
					if (`@エマ距離チェック` == d:TRUE ) {
						chara カメラ設定 モーションカメラ \
							-c 1 \
							-p -15456,-4418,-253912 -t -12723,-5420,-255073 \
							-r 212,1286,0 -f 3134 \
							-a 200 \
							-i 0 2 0 0 \
							-z d:CAMERA_NO_CUSHION \
							-s 1

						if ( $f:w31c_ロッカー開閉フラグ[7] == d:LOCKER_CLOSE ) {
							if ( `command プレイヤー状態取得` & d:PFLAG_CAUTION ) {
								chara delay ロッカーディレイ \
									-time 17 \
									-exec {
										mesg プットモーションモデル ロッカー:08 終了判定 \
											d:横開きロッカーモーションリスト:locker_sl_rai_open
									}
								mesg プレイヤー d:PLAYER motion \
									d:モーションリスト:non_sllocker_open_beheind 3072 3072 0 \
									(d:FA_NO_CHECK_TRP | d:FA_NO_CHECK_SEG) \
									(-11700-300) (-256000+445)
								command	強制モーション終了プロック \
									-motion d:モーションリスト:non_sllocker_open_beheind \
									-proc ロッカーチェック \
									-args 7
							} else {
								mesg プットモーションモデル ロッカー:08 終了判定 \
									d:横開きロッカーモーションリスト:locker_sl_rai_open
								mesg プレイヤー d:PLAYER motion \
									d:モーションリスト:non_locker_open 1024 1024 0 d:FA_NO_CHECK_TRP \
									(-11700-850) (-256000+463)
								command	強制モーション終了プロック \
									-motion d:モーションリスト:non_locker_open \
									-proc ロッカーチェック \
									-args 7
							}
						} else {
							mesg プットモーションモデル ロッカー:08 ドア閉め再生 \
								d:横開きロッカーモーションリスト:locker_sl_rai_close
							mesg プレイヤー d:PLAYER motion \
								d:モーションリスト:non_locker_close 1024 1024 0 d:FA_NO_CHECK_TRP \
								(-11700-746) (-256000+463)
							command	強制モーション終了プロック \
								-motion d:モーションリスト:non_locker_close \
								-proc ロッカーチェック \
								-args 7
						}
					}
				}
		#endif
	}


	proc ロッカーチェック {
		#if d:DEBUG_PRINT
			print 'w31c1_locker_check'
		#endif

		if ( $2 == 0 ) {
			if ( $f:w31c_ロッカー開閉フラグ[$2] == d:LOCKER_CLOSE ) {
				eval ( $f:w31c_ロッカー開閉フラグ[$2] = d:LOCKER_OPEN );
				mesg 透明壁 ロッカー壁:01 off
			} else {
				eval ( $f:w31c_ロッカー開閉フラグ[$2] = d:LOCKER_CLOSE );
				mesg 透明壁 ロッカー壁:01 on
			}
		} else if ( $2 == 1 ) {
		} else if ( $2 == 2 ) {
			if ( $f:w31c_ロッカー開閉フラグ[$2] == d:LOCKER_CLOSE ) {
				eval ( $f:w31c_ロッカー開閉フラグ[$2] = d:LOCKER_OPEN );
				mesg 透明壁 ロッカー壁:03 off
			} else {
				eval ( $f:w31c_ロッカー開閉フラグ[$2] = d:LOCKER_CLOSE );
				mesg 透明壁 ロッカー壁:03 on
			}
		} else if ( $2 == 3 ) {
		} else if ( $2 == 4 ) {
			if ( $f:w31c_ロッカー開閉フラグ[$2] == d:LOCKER_CLOSE ) {
				eval ( $f:w31c_ロッカー開閉フラグ[$2] = d:LOCKER_OPEN );
				mesg 透明壁 ロッカー壁:05 off
			} else {
				eval ( $f:w31c_ロッカー開閉フラグ[$2] = d:LOCKER_CLOSE );
				mesg 透明壁 ロッカー壁:05 on
			}
		} else if ( $2 == 5 ) {
			if ( $w:ゲーム設定 > d:LEVEL_HARD ) {
				if ( $f:w31c_ロッカー開閉フラグ[$2] == d:LOCKER_CLOSE ) {
					eval ( $f:w31c_ロッカー開閉フラグ[$2] = d:LOCKER_OPEN );
					mesg 透明壁 ロッカー壁:06 off
				} else {
					eval ( $f:w31c_ロッカー開閉フラグ[$2] = d:LOCKER_CLOSE );
					mesg 透明壁 ロッカー壁:06 on
				}
			}
		} else if ( $2 == 6 ) {
		} else if ( $2 == 7 ) {
			if ( $f:w31c_ロッカー開閉フラグ[$2] == d:LOCKER_CLOSE ) {
				eval ( $f:w31c_ロッカー開閉フラグ[$2] = d:LOCKER_OPEN );
				mesg 透明壁 ロッカー壁:08 off
			} else {
				eval ( $f:w31c_ロッカー開閉フラグ[$2] = d:LOCKER_CLOSE );
				mesg 透明壁 ロッカー壁:08 on
			}
		}

		chara カメラ設定 モーションカメラ -s -1
	}


	// 機雷の設定
	// ------------------------------------
	proc 北西エリア機雷設置 {
		#if d:DEBUG_PRINT
			print 'wa_mine_set'
		#endif

		if ( $w:ゲーム設定 == d:LEVEL_VERYEASY ) {
			command foreach -argc 5 -repeat 2 \
				-data { \
					{ 0 , 機雷:01, -12000, -8000, -250250 }
//					{ 1 , 機雷:02, -7000, -8000, -255500 }
					{ 2 , 機雷:03, -4250, -8750, -255500 }
//					{ 3 , 機雷:04, -16000, -8750, -253500 }
//					{ 4 , 機雷:05, -6000, -9750, -248250 }
//					{ 5 , 機雷:06, -4000, -7750, -247000 }
				} \
				-exec { \
					// 一回爆発したら二度と出ない
					if ( $f:w31c_爆発物フラグ[$1] == 0 ) {
						chara 水中機雷 $2 \
							-position $3 $4 $5 \
							-rotate 0 0 0 \
							-proc 機雷爆破
					}
				}
		} else if ( $w:ゲーム設定 >= d:LEVEL_EASY ) {
			command foreach -argc 5 -repeat 6 \
				-data { \
					{ 0 , 機雷:01, -12000, -8000, -250250 }
					{ 1 , 機雷:02, -7000, -8000, -255500 }
					{ 2 , 機雷:03, -4250, -8750, -255500 }
					{ 3 , 機雷:04, -16000, -8750, -253500 }
					{ 4 , 機雷:05, -6000, -9750, -248250 }
					{ 5 , 機雷:06, -4000, -7750, -247000 }
				} \
				-exec { \
					// 一回爆発したら二度と出ない
					if ( $f:w31c_爆発物フラグ[$1] == 0 ) {
						chara 水中機雷 $2 \
							-position $3 $4 $5 \
							-rotate 0 0 0 \
							-proc 機雷爆破
					}
				}
		}
	}

	proc 機雷爆破 {
		// 爆破時なら
		if( $2 == 1 ) {
			if( $1 == 機雷:01 ) {
				eval( $f:w31c_爆発物フラグ[0] = $w:グローバルロード回数 )
				eval( $f:w31c_機雷01爆発した =1 );
			} else if ( $1 == 機雷:02 ) {
				eval( $f:w31c_爆発物フラグ[1] = $w:グローバルロード回数 )
				eval( $f:w31c_機雷02爆発した =1 );
			} else if ( $1 == 機雷:03 ) {
				eval( $f:w31c_爆発物フラグ[2] = $w:グローバルロード回数 )
				eval( $f:w31c_機雷03爆発した =1 );
			} else if ( $1 == 機雷:04 ) {
				eval( $f:w31c_爆発物フラグ[3] = $w:グローバルロード回数 )
				eval( $f:w31c_機雷04爆発した =1 );
			} else if ( $1 == 機雷:05 ) {
				eval( $f:w31c_爆発物フラグ[4] = $w:グローバルロード回数 )
				eval( $f:w31c_機雷05爆発した =1 );
			} else if ( $1 == 機雷:06 ) {
				eval( $f:w31c_爆発物フラグ[5] = $w:グローバルロード回数 )
				eval( $f:w31c_機雷06爆発した =1 );
			}
		}
	}


	proc 水中ゴミ設定 {
		// 水中砂
		chara 水中ゴミマネージャ テストマネージャ \
			-unit 4 \
			-prim 32 \
			-center \
			-9500, -10750, -259000, \
			-7000, -10750, -259000, \
			-7000, -10750, -257000, \
			-7000, -10750, -255000, \
			-7000, -10750, -253000, \
			-7000, -10750, -251000, \
			-size \
			1500, 1, 1500, \
			1500, 1, 1500, \
			1500, 1, 1500, \
			1000, 1, 1500, \
			1000, 1, 1500, \
			1000, 1, 1500,

		// エリアダスト
		chara エリアダスト 右ダスト \
			-p -4750 d:WATER_SURFACE -251000 -3250 -10750 -249500 \
			-n 4

		chara エリアダスト 左ダスト \
			-p -17250 d:WATER_SURFACE -251000 -15750 -10750 -249500 \
			-n 4

		@浮遊ゴミ
		@紙
		@窓からの光
		//@泡
		//@空き缶
	}

#define TRASH_GROUP			4

#define ペットボトル大		wtd_petbottle_mt
#define ペットボトル小		wtd_smallpet_mt
#define 紙コップ			wtd_papercup
#define バルカン人形		wtd_vul_figure

	proc 浮遊ゴミ {
	print '浮遊ゴミ'

		chara 浮遊物 浮遊物:01 -group d:TRASH_GROUP -trap pt001 -m d:ペットボトル小 -y d:PET_MODE_SMALL
		chara 浮遊物 浮遊物:02 -group d:TRASH_GROUP -trap pt002 -m d:ペットボトル大 -y d:PET_MODE_BIG
		chara 浮遊物 浮遊物:03 -group d:TRASH_GROUP -trap pt003 -m d:ペットボトル大 -y d:PET_MODE_BIG

	}

	proc 紙 {
		print '紙'
		chara 浮遊紙 ペーパーＡ \
			-c 1 \
			-o d:PLAYER \
			-n 8 \
			-m paper_a -g d:TRASH_GROUP -t kami1 \
			-m paper_b -g d:TRASH_GROUP -t kami1 \
			-m paper_a -g d:TRASH_GROUP -t kami2 \
			-m paper_b -g d:TRASH_GROUP -t kami2 \
			-m paper_a -g d:TRASH_GROUP -t kami3 \
			-m paper_b -g d:TRASH_GROUP -t kami3 \
			-m paper_a -g d:TRASH_GROUP -t kami3 \
			-m paper_b -g d:TRASH_GROUP -t kami3
	}

	proc 窓からの光 {
		chara 天窓の光 ひかり１ \
			-pos 	-17250, -6500, -251000, -15750, -6500, -249500 \
					(-16750-500), -10750, (-251000-500), (-15750+500), -10500, (-249500+500) \
			-color	0x34323030 \
			-dcol	0x34323010

		chara 天窓の光 ひかり２ \
			-pos 	-4750, -6500, -251000, -3250, -6500, -249500 \
					(-4750-500), -10750, (-251000-500), (-3250+500), -10500, (-249500+500) \
			-color	0x34323030 \
			-dcol	0x34323010

		trap ce001 d:PLAYER \
			-mask ？ \
			-exec {
				if( $3 == 入る ) {
					mesg 静止スポットライト設置 息継ぎライト:01 on
					mesg 天窓の光 ひかり１ アクティブモード
					mesg エリアダスト 左ダスト オン
				} else {
					mesg 静止スポットライト設置 息継ぎライト:01 off
					mesg 天窓の光 ひかり１ スリープモード
					mesg エリアダスト 左ダスト オフ
				}
		}

		trap ce002 d:PLAYER \
			-mask ？ \
			-exec {
				if( $3 == 入る ) {
					mesg 静止スポットライト設置 息継ぎライト:02 on
					mesg 天窓の光 ひかり２ アクティブモード
					mesg エリアダスト 右ダスト オン
				} else {
					mesg 静止スポットライト設置 息継ぎライト:02 off
					mesg 天窓の光 ひかり２ スリープモード
					mesg エリアダスト 右ダスト オフ
				}
		}
	}

	proc 泡 {
		// 南
		chara 連続発生線状泡 プクプク泡 -p -8375 -10750 -258750

		chara 連続発生線状泡 プクプク泡 -p -4250 -10750 -248875

		chara 連続発生線状泡 プクプク泡 -p -16875 -10750 -248875

		// 北
		chara 連続発生線状泡 プクプク泡 -p -4500 -10750 -251625

		chara 連続発生線状泡 プクプク泡 -p -7375 -10750 -251625

		chara 連続発生線状泡 プクプク泡 -p -16875 -10750 -251625

	}

	proc 空き缶 {
		chara あきかん 空き缶:01 \
			-p -16250 0 -250500 \
			-c 0 \	// 浮かんでる
			-m wtd_kan
	}


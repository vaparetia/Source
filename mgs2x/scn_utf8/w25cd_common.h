/*
	w25cd_common.h                     
	    w25c w25d の共通プロック

	2000/11/30 Y.Matsuhana         
	$Id: w25cd_common.h,v 1.37 2001/11/09 13:33:24 usr00725 Exp $                      

	
*/


#define	DOOR_NAME			w25c_dr00
#define	DOOR_NAME_LOAD		w25c_dr00
	// 各種カメラの設定
	// ------------------------------------
	proc w25cd共通サウンド設定 {
		@サウンド効果音設定 ;
		@弾痕跳弾設定
	}

	// 弾痕跳弾関係の設定
	// ------------------------------------
	proc 弾痕跳弾設定 {
		#if d:DEBUG_PRINT 
			print 'recoil_set'
		#endif

		command 弾痕ノーマル -n 0	// 通常
		command 弾痕ノーマル -n 1	// 階段、通常
		command 弾痕ノーマル -n 2	// 鉄板
		command 弾痕ノーマル -n 3	// 鉄筋
		command 弾痕ノーマル -n 4	// ダンボール
		command 弾痕ノーマル -n 5	// 木箱
		command 弾痕ノーマル -n 6	// パイプ中
		command 弾痕ノーマル -n 7	// パイプ大

		command 跳弾ノーマル -n 0
		command 跳弾ノーマル -n 1
		command 跳弾ノーマル -n 2
		command 跳弾ノーマル -n 3
		command 跳弾ノーマル -n 5
		command 跳弾水蒸気中 -n 6
		command 跳弾水蒸気大 -n 7
	}


	// 各種カメラの設定
	// ------------------------------------
	proc w25cd共通カメラ設定 {
		#if d:DEBUG_PRINT
			print 'w25cd_common_camera_set'
		#endif

		// 
		trap ca005 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 小部屋カメラ \
					-c 0 \	
					-b -320000,-320000,-320000 320000   3363 -234180 \
					-l   20118,-320000,-320000  20957 320000 -234225 \
					-r 790,2048,0 -f 3915 \
					-a 200 \
					-i 0 0 0 0 \
					-s $3
			}
		// 
		#ifdef STAGE_W25C
			trap ca006 d:PLAYER \
				-mask ？ \
				-camera \
				-exec {
					chara カメラ設定 小部屋カメラ \
						-c 0 \	
						-b -320000,-320000,-320000 320000 320000 -232733 \
						-l -320000,-320000,-320000 320000 320000 -240526 \
						-r 630,2262,0 -f 8500 \
						-a 200 \
						-i 2 -1 15 0 \
						-s $3
				}
			trap ca007 d:PLAYER \
				-mask ？ \
				-camera \
				-exec {
					chara カメラ設定 小部屋カメラ \
						-c 0 \	
						-b 30297,-320000,-247229 320000 320000 -225992 \
						-l 27643,-320000,-251802 320000 320000 -230564 \
						-r 630,2262,0 -f 8500 \
						-a 200 \
						-i 2 -1 15 0 \
						-s $3
				}
		#else
			trap ca006 d:PLAYER \
				-mask ？ \
				-camera \
				-exec {
					chara カメラ設定 小部屋カメラ \
						-c 0 \	
						-b -320000,-320000,-320000 320000 320000 -232733 \
						-l -320000,-320000,-320000 320000 320000 -240526 \
						-r 630,2262,0 -f 8500 \
						-a 200 \
						-i 2 -1 15 0 \
						-s $3
				}
			trap ca007 d:PLAYER \
				-mask ？ \
				-camera \
				-exec {
					chara カメラ設定 小部屋カメラ \
						-c 0 \	
						-b -320000,-320000,-247229 320000 320000 -225992 \
						-l -320000,-320000,-251802 320000 320000 -230564 \
						-r 630,2262,0 -f 8500 \
						-a 200 \
						-i 2 -1 15 0 \
						-s $3
				}
		#endif
		// 
		// 
		trap ca008 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 小部屋カメラ \
					-c 1 \	
					-p 52431,7227,-226362 -t 52406,1582,-227448 \
					-r  90,2048,0 -f 8500 \
					-a 200 \
					-i 1 -1 0 0 \
					-s $3
			}
		// Ｋ脚玄関口
		trap ca009 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 1 \	
					-p 52043,4981,-254418 -t 52048,1210,-255338 \
					-r 868,2044,0 -f 3882 \
					-a 194 \
					-i 0 0 0 0 \
					-s $3
			}
		// Ｌ脚玄関口
		trap ca0010 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 小部屋カメラ \
					-c 0 \	
					-b 48764 -320000 -224330 52373 320000 -224330 \
					-l 48764 -320000 -224636 52373 320000 -224636 \
					-r 960,2048,0 -f 3116 \
					-a 200 \
					-i 0 0 0 0 \
					-s $3
			}
		// 補助連絡橋階段
		#ifdef STAGE_W25C
			trap ca0011 d:PLAYER \
				-mask ？ \
				-camera \
				-exec {
					chara カメラ設定 小部屋カメラ \
						-c 0 \	
						-b 30297,-320000,-240745 320000 320000 -225115 \
						-l 27643,-320000,-248538 320000 320000 -232908 \
						-r 630,2262,0 -f 8500 \
						-a 200 \
						-i 2 -1 15 0 \
						-s $3
				}
			trap ca0012 d:PLAYER \
				-mask ？ \
				-camera \
				-exec {
					chara カメラ設定 小部屋カメラ \
						-c 0 \	
						-b 30297, 5278,-246173 320000 320000 -226146 \
						-l 27643,-1717,-250746 320000 320000 -230718 \
						-r 630,2262,0 -f 8500 \
						-a 200 \
						-i 2 -1 15 0 \
						-s $3
				}
		#else
			trap ca0011 d:PLAYER \
				-mask ？ \
				-camera \
				-exec {
					chara カメラ設定 小部屋カメラ \
						-c 0 \	
						-b 30297,-320000,-240745 320000 320000 -225115 \
						-l 27643,-320000,-248538 320000 320000 -232908 \
						-r 630,2262,0 -f 8500 \
						-a 200 \
						-i 2 -1 15 0 \
						-s $3
				}
			trap ca0012 d:PLAYER \
				-mask ？ \
				-camera \
				-exec {
					chara カメラ設定 小部屋カメラ \
						-c 0 \	
						-b 30297, 5278,-246173 320000 320000 -226146 \
						-l 27643,-1717,-250746 320000 320000 -230718 \
						-r 630,2262,0 -f 8500 \
						-a 200 \
						-i 2 -1 15 0 \
						-s $3
				}
		#endif
		// ＫＬ連絡橋／補助廊下（手前）
		trap ca0013 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 通路カメラ \
					-c 1 \	
					-p 59030,1363,-227367 -t 59030,-7102,-227770 \
					-r 993,2048,0 -f 8474 \
					-a 200 \
					-i 0 0 0 0 \
					-s $3
			}
		#ifdef STAGE_W25C
			trap ev005 d:PLAYER -mask ＊ \
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 1 \	
						-p ($i:プレイヤー位置Ｘ+692),($i:プレイヤー位置Ｙ+1884),($i:プレイヤー位置Ｚ+703) \
						-t ($i:プレイヤー位置Ｘ+559),($i:プレイヤー位置Ｙ+ 228), ($i:プレイヤー位置Ｚ+149) \
						-a 200 \
						-i 4 4 15 15 \
						-s $3
				}
		#endif
		// 張付きアングル換え
		// ------------------------------------
		trap ca100 d:PLAYER -mask ＊ -camera -dir 0,512 -state d:TRP_STATE_BEHINDCAMERA_ENABLE \
			-exec {
				if( $3 == いる) {
					if ( $f:ca100張付きアングル換え初期化 == 0 ) {
						eval( $f:ca100張付きアングル換え初期化 = 1 ) ;
						eval( $i:dct = 1000 ) ;
						eval( $w:wd_カメラ角度Ｙ = $i:カメラ角度Ｙ & 4095 ) ;
						eval( $i:dcx = `command sin $w:wd_カメラ角度Ｙ` * $i:dct / 4096 )
						eval( $i:dcz = `command cos $w:wd_カメラ角度Ｙ` * $i:dct / 4096 )
						print $i:dct $i:dcx $i:dcz $i:カメラ角度Ｙ
					}
				}
				chara カメラ設定 カットインカメラ \
					-c 1 \
					-p ($i:プレイヤー位置Ｘ-$i:dcx) ($i:プレイヤー位置Ｙ+4000) ($i:プレイヤー位置Ｚ-$i:dcz) \
					-t $i:プレイヤー位置Ｘ $i:プレイヤー位置Ｙ ($i:プレイヤー位置Ｚ-500) \
					-f 3000 \
					-d 1 \
					-a 200 \
					-i 2 -1 0 0 \
					-s $3
			}
		trap ca101 d:PLAYER -mask ＊ -camera -dir 2048,512 -state d:TRP_STATE_BEHINDCAMERA_ENABLE \
			-exec {
				if( $3 == いる) {
					if ( $f:ca101張付きアングル換え初期化 == 0 ) {
						eval( $f:ca101張付きアングル換え初期化 = 1 ) ;
						eval( $i:dct = 1000 ) ;
						eval( $w:wd_カメラ角度Ｙ = $i:カメラ角度Ｙ & 4095 ) ;
						eval( $i:dcx = `command sin $w:wd_カメラ角度Ｙ` * $i:dct / 4096 )
						eval( $i:dcz = `command cos $w:wd_カメラ角度Ｙ` * $i:dct / 4096 )
						print $i:dct $i:dcx $i:dcz $i:カメラ角度Ｙ
					}
				}
				chara カメラ設定 カットインカメラ \
					-c 1 \
					-p ($i:プレイヤー位置Ｘ-$i:dcx) ($i:プレイヤー位置Ｙ+4000) ($i:プレイヤー位置Ｚ-$i:dcz) \
					-t $i:プレイヤー位置Ｘ $i:プレイヤー位置Ｙ ($i:プレイヤー位置Ｚ-500) \
					-f 3000 \
					-d 1 \
					-a 200 \
					-i 2 -1 0 0 \
					-s $3
			}
		trap ca102 d:PLAYER -mask ＊ -camera -dir 1024,512 -state d:TRP_STATE_BEHINDCAMERA_ENABLE \
			-exec {
				if( $3 == いる) {
					if ( $f:ca102張付きアングル換え初期化 == 0 ) {
						eval( $f:ca102張付きアングル換え初期化 = 1 ) ;
						eval( $i:dct = 1000 ) ;
						eval( $w:wd_カメラ角度Ｙ = $i:カメラ角度Ｙ & 4095 ) ;
						eval( $i:dcx = `command sin $w:wd_カメラ角度Ｙ` * $i:dct / 4096 )
						eval( $i:dcz = `command cos $w:wd_カメラ角度Ｙ` * $i:dct / 4096 )
						print $i:dct $i:dcx $i:dcz $i:カメラ角度Ｙ
					}
				}
				chara カメラ設定 カットインカメラ \
					-c 1 \
					-p ($i:プレイヤー位置Ｘ-$i:dcx) ($i:プレイヤー位置Ｙ+4000) ($i:プレイヤー位置Ｚ-$i:dcz) \
					-t $i:プレイヤー位置Ｘ $i:プレイヤー位置Ｙ ($i:プレイヤー位置Ｚ-500) \
					-f 3000 \
					-d 1 \
					-a 200 \
					-i 2 -1 0 0 \
					-s $3
			}
		trap ca103 d:PLAYER -mask ＊ -camera -dir 3072,512 -state d:TRP_STATE_BEHINDCAMERA_ENABLE \
			-exec {
				if( $3 == いる) {
					if ( $f:ca103張付きアングル換え初期化 == 0 ) {
						eval( $f:ca103張付きアングル換え初期化 = 1 ) ;
						eval( $i:dct = 1000 ) ;
						eval( $w:wd_カメラ角度Ｙ = $i:カメラ角度Ｙ & 4095 ) ;
						eval( $i:dcx = `command sin $w:wd_カメラ角度Ｙ` * $i:dct / 4096 )
						eval( $i:dcz = `command cos $w:wd_カメラ角度Ｙ` * $i:dct / 4096 )
						print $i:dct $i:dcx $i:dcz $i:カメラ角度Ｙ
					}
				}
				chara カメラ設定 カットインカメラ \
					-c 1 \
					-p ($i:プレイヤー位置Ｘ-$i:dcx) ($i:プレイヤー位置Ｙ+4000) ($i:プレイヤー位置Ｚ-$i:dcz) \
					-t $i:プレイヤー位置Ｘ $i:プレイヤー位置Ｙ ($i:プレイヤー位置Ｚ-500) \
					-f 3000 \
					-d 1 \
					-a 200 \
					-i 2 -1 0 0 \
					-s $3
			}
		trap by002 d:PLAYER -mask ＊ -camera -dir 3413,512 -state d:TRP_STATE_BEHINDCAMERA_ENABLE \
			-exec {
				if( $3 == いる) {
					if ( $f:by002張付きアングル換え初期化 == 0 ) {
						eval( $f:by002張付きアングル換え初期化 = 1 ) ;
						eval( $i:dct = 1000 ) ;
						eval( $w:wd_カメラ角度Ｙ = $i:カメラ角度Ｙ & 4095 ) ;
						eval( $i:dcx = `command sin $w:wd_カメラ角度Ｙ` * $i:dct / 4096 )
						eval( $i:dcz = `command cos $w:wd_カメラ角度Ｙ` * $i:dct / 4096 )
						print $i:dct $i:dcx $i:dcz $i:カメラ角度Ｙ
					}
				}
				chara カメラ設定 カットインカメラ \
					-c 1 \
					-p ($i:プレイヤー位置Ｘ-$i:dcx) ($i:プレイヤー位置Ｙ+4000) ($i:プレイヤー位置Ｚ-$i:dcz) \
					-t $i:プレイヤー位置Ｘ $i:プレイヤー位置Ｙ ($i:プレイヤー位置Ｚ-500) \
					-f 3000 \
					-d 1 \
					-a 200 \
					-i 2 -1 0 0 \
					-s $3
			}
		trap by003 d:PLAYER -mask ＊ -camera -dir 2730,512 -state d:TRP_STATE_BEHINDCAMERA_ENABLE \
			-exec {
				if( $3 == いる) {
					if ( $f:by003張付きアングル換え初期化 == 0 ) {
						eval( $f:by003張付きアングル換え初期化 = 1 ) ;
						eval( $i:dct = 1000 ) ;
						eval( $w:wd_カメラ角度Ｙ = $i:カメラ角度Ｙ & 4095 ) ;
						eval( $i:dcx = `command sin $w:wd_カメラ角度Ｙ` * $i:dct / 4096 )
						eval( $i:dcz = `command cos $w:wd_カメラ角度Ｙ` * $i:dct / 4096 )
						print $i:dct $i:dcx $i:dcz $i:カメラ角度Ｙ
					}
				}
				chara カメラ設定 カットインカメラ \
					-c 1 \
					-p ($i:プレイヤー位置Ｘ-$i:dcx) ($i:プレイヤー位置Ｙ+4000) ($i:プレイヤー位置Ｚ-$i:dcz) \
					-t $i:プレイヤー位置Ｘ $i:プレイヤー位置Ｙ ($i:プレイヤー位置Ｚ-500) \
					-f 3000 \
					-d 1 \
					-a 200 \
					-i 2 -1 0 0 \
					-s $3
			}
	}

// -----------------------------------------------------------------------------------
// 落下
// ------------------------------------
proc w25cd落下死亡処理 {
	#if d:DEBUG_PRINT
		print 'falldown_set'
	#endif
	trap ev002 d:PLAYER \
		-mask 入る \
		-exec {
			print 'falldown set'
			command ゲームオーバー背景設定 \
				-t gmov \
				-s dekisi_alp_ovl \
				-m 1
			@ジャンプ落下死発動 0x0004 -41000
		}
}
// ジャンプ落下死開始処理 (fall.h内部よりコールされる)
proc ジャンプ落下死開始処理 {
	chara カメラ設定 モーションカメラ \
			-c 0 \
			-b d:CAM_MIN,-5000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
			-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
			-r 3800,2400,0 -f 4340 \
			-a 200 \
			-i 0 -1 0 0 \
			-s 1
	@ゲームオーバー開始処理
}
//  ゲームオーバー終了処理 (fall.h内部よりコールされる)
proc ゲームオーバー終了処理 {
	chara パッド振動 落下振動 -vibfile drop_sea
}

	// ドア＆マップ切り替え関係
	// ------------------------------------
	proc ドア設置 {
		#if d:DEBUG_PRINT
			print 'slide_door_set'
		#endif

		chara ドア 中央棟へのドア \
			-m d:DOOR_NAME_LOAD \
			-d 0,1024,0 \
			-p 19125,-1500,-234250 \
			-slide d:SD_SLIDE_OUTSIDE \
			-A 中央棟方面ドアランプ \
			-time d:SD_OPEN_TIME \
			-exec {
				if ( $:開閉フラグ == 1 ) {
					mesg プットオブジェ 中央棟接続マップ on
				} else {
					mesg プットオブジェ 中央棟接続マップ off
				}
			}
		trap dr001 ？ \
			-mask ？ \
			-exec {
				if($3==入る){
					mesg ドア 中央棟へのドア open $2
				}else{
					if ( $f:エマゲームオーバーフラグ == 0 ) {
						mesg ドア 中央棟へのドア close
					}
				}
			}
		@ドアランプ設定 中央棟方面ドアランプ 1024 19125,-1500,-234250 0

		chara ドア Ｌ脚ドア \
			-m d:DOOR_NAME \
			-d 0,0,0 \
			-p 48000,0,-223625 \
			-slide d:SD_SLIDE_OUTSIDE \
			-A Ｌ脚方面ドアランプ \
			-card_level 5 \
			-time d:SD_OPEN_TIME \
			-exec {
				if ( $:開閉フラグ == 1 ) {
						mesg プットオブジェ Ｌ脚接続マップ on
				} else {
						mesg プットオブジェ Ｌ脚接続マップ off
				}
			}
		trap dr002 ？ \
			-mask ？ \	
			-exec {
					if($3==入る){
						mesg ドア Ｌ脚ドア open $2
					}else{
						if ( $f:エマゲームオーバーフラグ == 0 ) {
							mesg ドア Ｌ脚ドア close
						}
					}
			}
		@ドアランプ設定 Ｌ脚方面ドアランプ 0 48000,0,-223625 1

		#ifdef STAGE_W25D
		chara プットオブジェ Ｌ脚接続マップ -m d:KMS_ENT_L_0 -r 0 0 0 -p 0 0 0 -l d:LT2_NAME -s 100 100 100 -f 0
		#endif
		chara プットオブジェ 中央棟接続マップ -m d:KMS_ENT_C_0 -r 0 0 0 -p 0 0 0 -l d:LT2_NAME -s 100 100 100 -f 0

		// K脚ドアはいけないためドア壊れエフェクトを呼びます
		@壊れドアエフェクト設定 壊れドアエフェクト 2048 52750 0 -256125 1
	}
	proc 主観禁止壁設定 {
		@主観禁止壁 中央棟へのドア壁 19125 -1500 -234250 1024 d:SD_SLIDE_OUTSIDE
		trap dr001 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				if( $3 == 入る ) {
					mesg 透明壁 中央棟へのドア壁 on
				} else {
					mesg 透明壁 中央棟へのドア壁 off
				}
			}
		@主観禁止壁 Ｌ脚ドア壁 48000 0 -223625 0 d:SD_SLIDE_OUTSIDE
		trap dr002 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				if( $3 == 入る ) {
					mesg 透明壁 Ｌ脚ドア壁 on
				} else {
					mesg 透明壁 Ｌ脚ドア壁 off
				}
			}
		// ニキータ防止
		command ミサイル移動範囲設定 18000 -80000 -259250 68000 80000 -219250
	}

	// エルードの設定
	// ------------------------------------
	proc w25cd共通エルード設定 {
		#if d:DEBUG_PRINT
			print 'w25cd_common_beyond_set'
		#endif

		// 
			trap by101 d:PLAYER \
				-mask いる \
				-and \
				-button d:ACTION_BUTTON d:ACTION_BUTTON \		//ビヨンドボタン
				-state 0,4 \	//立ち状態or張り付き
				-exec {
					if ( `@南向きビヨンドハイ発動 by101 -239000 $4 32000 34000 0x000002` != 0 ) {
						eval( $b:ビヨンド判定1 = 1 );
					}
				}

			trap by101 d:PLAYER \
				-mask ＊ -camera -state 6 \	// 強制モーション中
				-exec {
						if ( $b:ビヨンド判定1 == 1 ) {
							chara カメラ設定 モーションカメラ \
								-c 1 \
								-p 40056,-3058,-236808 \
								-t 33130,  313,-239298 \
								-a 200 \
								-i 0 0 0 0 \
								-s $3
						}
				}

			trap by101 d:PLAYER \
				-camera \
				-mask ？ \
				-state 5 \	// エルード中
				-exec {
						if ( $b:ビヨンド判定1 == 1 ) {
							chara カメラ設定 ビヨンドカメラ \
								-c 0 \
								-b -320000,-320000,-320000 320000 320000 320000 \
								-l -320000,-320000,-320000 320000 320000 320000 \
								-r 860,2187,0 \
								-f 8095 \
								-a 200 \
								-i 1 0 0 0 \
								-s $3
						}
				}


			trap by102 d:PLAYER \
				-mask いる \
				-and \
				-button d:ACTION_BUTTON d:ACTION_BUTTON \		//ビヨンドボタン
				-state 0,4 \	//立ち状態or張り付き
				-exec {
					if( `@南向きビヨンドハイ発動 by102 -239000 $4 37000 40000 0x000002` != 0 ) {
						eval( $b:ビヨンド判定1 = 1 );
					}
				}
			trap by102 d:PLAYER \
				-mask ＊ -camera -state 6 \	// 強制モーション中
				-exec {
						if ( $b:ビヨンド判定1 == 1 ) {
							chara カメラ設定 モーションカメラ \
								-c 1 \
								-p 45799,-4294,-234440 \
								-t 39310,-1328,-238235 \
								-a 200 \
								-i 0 0 0 0 \
								-s $3
						}
				}

			trap by102 d:PLAYER \
				-camera \
				-mask ？ \
				-state 5 \	// エルード中
				-exec {
						if ( $b:ビヨンド判定1 == 1 ) {
							chara カメラ設定 ビヨンドカメラ \
								-c 0 \
								-b -320000,-320000,-320000 320000 320000 320000 \
								-l -320000,-320000,-320000 320000 320000 320000 \
								-r 860,2187,0 \
								-f 8095 \
								-a 200 \
								-i 1 0 0 0 \
								-s $3
						}
				}

			trap by103 d:PLAYER \
				-mask いる \
				-and \
				-button d:ACTION_BUTTON d:ACTION_BUTTON \		//ビヨンドボタン
				-state 0,4 \	//立ち状態or張り付き
				-exec {
					if ( `@北向きビヨンドハイ発動 by103 -241000 $4 32000 40000 0x000002` != 0 ) {
						eval( $b:ビヨンド判定1 = 1 );
					}
				}
			trap by103 d:PLAYER \
				-mask ＊ -camera -state 6 \	// 強制モーション中
				-exec {
						if ( $b:ビヨンド判定1 == 1 ) {
							chara カメラ設定 モーションカメラ \
								-c 0 \
								-b -320000,-320000,-320000 320000 320000 320000 \
								-l -320000,-320000,-320000 320000 320000 320000 \
								-r 3831 3252 0 \
								-f 8500 \
								-a 200 \
								-i 0 0 0 0 \
								-s $3
						}
				}
			trap by103 d:PLAYER \
				-camera \
				-mask ？ \
				-state 5 \	// エルード中
				-exec {
						if ( $b:ビヨンド判定1 == 1 ) {
							chara カメラ設定 ビヨンドカメラ \
								-c 0 \
								-b -320000,-320000,-320000 320000 320000 320000 \
								-l -320000,-320000,-320000 320000 320000 320000 \
								-r 1010,2048,0 \
								-f 8500 \
								-a 200 \
								-i 1 0 0 0 \
								-s $3
						}
				}

			trap by104 d:PLAYER \
				-mask いる \
				-and \
				-button d:ACTION_BUTTON d:ACTION_BUTTON \		//ビヨンドボタン
				-state 0,4 \	//立ち状態or張り付き
				-exec {
					if ( `@東向きビヨンドハイ発動 by104 59500 $6 -252000 -228000 0x000002` != 0 ) {
						eval( $b:ビヨンド判定1 = 1 );
					}
				}
			trap by104 d:PLAYER \
				-mask ＊ -camera -state 6 \	// 強制モーション中
				-exec {
						if ( $b:ビヨンド判定1 == 1 ) {
							chara カメラ設定 モーションカメラ \
								-c 0 \
								-b -320000 -320000 -320000  320000 320000 320000 \
								-l -320000 -320000 -320000  320000 320000 320000 \
								-r 3880,2713,0 -f 8597 \
								-a 250 \
								-i 0 0 0 0 \
								-s $3
						}
				}

			trap by104 d:PLAYER \
				-camera \
				-mask ？ \
				-state 5 \	// エルード中
				-exec {
						if ( $b:ビヨンド判定1 == 1 ) {
							chara カメラ設定 ビヨンドカメラ \
								-c 0 \
								-b -320000 -320000 -320000  60200   6700 -225987 \
								-l -320000 -320000 -320000  59559  -3235 -228391 \
								-r 864,2218,0 \
								-f 10242 \
								-a 250 \
								-i 1 0 0 0 \
								-s $3
						}
				}
	}

	// ロード時カメラの設定
	// ------------------------------------
	proc Ｌ脚ロード時カメラ設定 {
		chara カメラ設定 ロードカメラ \
						-c 1 \
						-p 49155,4804,-224513 -t 49104,-8759,-224715 \
						-r 1014,2208,0 -f 13565 \
						-a 200 \
						-i 0 1 0 0 \
						-s 1
		chara プロック連続実行 カメラ戻し管理人 \
			-proc カメラ戻し管理
	}
	proc 中央棟ロード時カメラ設定 {
		chara カメラ設定 ロードカメラ \
						-c 1 \
						-p 20152,3045,-234426 -t 19794,357,-235389 \
						-r 786,2280,0 -f 2877 \
						-a 200 \
						-i 0 1 0 0 \
						-s 1
		chara プロック連続実行 カメラ戻し管理人 \
			-proc カメラ戻し管理
	}
	proc カメラ戻し管理 {
		command パッドチェック \
			-no_use
		if ($status != 1) {
			chara カメラ設定 ロードカメラ \
				-s -1
			mesg プロック連続実行 カメラ戻し管理人 kill
		}
	}



/*
	vs08a_common.h.gcl

	2002/06/26 T.Ohtani
	$Id: vs08a_common.h,v 1.1 2002/06/27 01:09:08 usr13731 Exp $
*/


// ------------------------------------
// 監視カメラ
// ------------------------------------
// プロック定義。
proc vs08a_簡易監視カメラ $:名前 $:場所Ｘ $:場所Ｙ $:場所Ｚ $:取付縦方向 $:取付横方向 $:カメラ振り中心角 $:監視カメラ初期方向 $:振り幅 $:フラグ $:監視カメラ首振り待ち時間 {
	chara プラントカメラ $:名前 \
		-pos $:場所Ｘ $:場所Ｙ $:場所Ｚ \
		-rot $:取付縦方向 $:取付横方向 0 \ /*ＸＹ値のみ有効*/
		-center $:カメラ振り中心角 $:監視カメラ初期方向 \
		-dir $:振り幅 \
		-xran $i:監視カメラ縦視野角 \
		-yran $i:監視カメラ横視野角 \
		-i $w:監視カメラ通常視力 $w:監視カメラ危険視力 $w:監視カメラ警戒視力 $w:監視カメラ回避視力 \
		-status $:フラグ \
		-wait $:監視カメラ首振り待ち時間 \
		-alertdir $i:監視カメラ緊急振り幅 \
		-exec {
			@監視カメラ破壊時処理 $1 $2
		}
}

proc 監視カメラ破壊時処理 $:名前 $:フラグ {
}

// ------------------------------------
// カメラ関係の設定
// ------------------------------------
// ステージカメラ設定
// ------------------------------------
proc カメラセット {
	#if d:DEBUG_PRINT
		print 'chara_camera & stage_camera_set'
	#endif

	chara カメラ ステージカメラ		-c 0 -l 3 -p 0
	chara カメラ メインカメラ		-c 0 -l 2 -p 40
	chara カメラ モーションカメラ	-c 0 -l 2 -p 30
	chara カメラ クリア時カメラ		-c 0 -l 0 -p 10

	// デフォルトカメラ
	chara カメラ設定 ステージカメラ \
		-c 0 \
		-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
		-l 1100,1500,-30000 12000,d:CAM_MAX,-822 \
		-r 670,2048,0 \
		-f 10000 \
		-a 200 \
		-i 2 2 0 0 \
		-s 1
}

// ステージ固有のカメラ設定
// ------------------------------------
proc vs08aカメラ設定 {
	#if d:DEBUG_PRINT
		print 'stage_camera_set'
	#endif
	// -------------------------
	// 2m壁裏カメラ
	// -------------------------
	// 左側高台壁裏
	trap ca001 d:PLAYER \
		-mask ？ \
		-camera \
		-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
		-exec {
			chara カメラ設定 メインカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l 1100,1500,-30000 12000,d:CAM_MAX,-822 \
				-r 870,2048,0 \
				-f 9000 \
				-a 200 \
				-i 3 3 0 0 \
				-s $3
		}
	// 右側高台南壁裏
	trap ca002 d:PLAYER \
		-mask ？ \
		-camera \
		-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
		-exec {
			chara カメラ設定 メインカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l 1100,1500,-30000 12000,d:CAM_MAX,-822 \
				-r 900,2048,0 \
				-f 9000 \
				-a 200 \
				-i 3 3 0 0 \
				-s $3
		}
	// 右側高台中央壁裏
	trap ca003 d:PLAYER \
		-mask ？ \
		-camera \
		-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
		-exec {
			chara カメラ設定 メインカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l 1100,1500,-30000 12000,d:CAM_MAX,-822 \
				-r 900,2048,0 \
				-f 9000 \
				-a 200 \
				-i 3 3 0 0 \
				-s $3
		}
	// 右側高台北壁裏
	trap ca004 d:PLAYER \
		-mask ？ \
		-camera \
		-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
		-exec {
			chara カメラ設定 メインカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l 1100,1500,-30000 12000,d:CAM_MAX,-822 \
				-r 900,2048,0 \
				-f 9000 \
				-a 200 \
				-i 3 3 0 0 \
				-s $3
		}
	// ゴール階段前障害物裏
	trap ca005 d:PLAYER \
		-mask ？ \
		-camera \
		-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
		-exec {
			chara カメラ設定 メインカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l 1100,1500,-30000 12000,d:CAM_MAX,-822 \
				-r 900,2048,0 \
				-f 9000 \
				-a 200 \
				-i 3 3 0 0 \
				-s $3
		}
	// ステージ左高台北階段障害物裏
	trap ca006 d:PLAYER \
		-mask ？ \
		-camera \
		-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l 1100,1500,-30000 12000,d:CAM_MAX,-822 \
				-r 900,2048,0 \
				-f 9000 \
				-a 200 \
				-i 3 3 0 0 \
				-s $3
		}
	// ステージ右高台裏
	trap ca007 d:PLAYER \
		-mask ？ \
		-camera \
//			-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
		-exec {
			chara カメラ設定 メインカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l 1100,1500,-30000 12000,d:CAM_MAX,-822 \
				-r 930,2048,0 \
				-f 9000 \
				-a 200 \
				-i 3 3 0 0 \
				-s $3
		}
	// ステージ右高台北階段裏
	trap ca008 d:PLAYER \
		-mask ＊ \
		-camera \
		-exec {

			@CS_スムーズカメラ降順 $4 13500 11000 670 930
			eval( $i:補完ＲＸ = $i:CS_結果１ );

			chara カメラ設定 メインカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l 1100,1500,-30000 12000,d:CAM_MAX,-822 \
				-r $i:補完ＲＸ,2048,0 \
				-f 9000 \
				-a 200 \
				-i 3 3 0 0 \
				-s $3
		}
	// ステージ右高台北階段裏
	trap ca009 d:PLAYER \
		-mask ？ \
		-camera \
//			-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
		-exec {
			chara カメラ設定 メインカメラ \
				-c 0 \
				-b 1100,10000,d:CAM_MIN 1100,d:CAM_MAX,d:CAM_MAX \
				-l 1100,3500,d:CAM_MIN 1100,d:CAM_MAX,d:CAM_MAX \
				-r 450,2048,0 \
				-f 10000 \
				-a 200 \
				-i 3 3 0 0 \
				-s $3
		}
	// ゴール高台前階段
	trap ca010 d:PLAYER \
		-mask ＊ \
		-camera \
		-exec {
			if( $6 > -27000 ) {
				@CS_スムーズカメラ降順 $6 -25000 -27000 -20000 -26000
				eval( $i:補完ＰＺ = $i:CS_結果１ )
				@CS_スムーズカメラ降順 $6 -25000 -27000 -25000 -27776
				eval( $i:補完ＴＺ = $i:CS_結果１ )
			}else{
				@CS_スムーズカメラ降順 $6 -27000 -30250 -26000 -25500
				eval( $i:補完ＰＺ = $i:CS_結果１ )
				@CS_スムーズカメラ降順 $6 -27000 -30250 -27776 -30500
				eval( $i:補完ＴＺ = $i:CS_結果１ )
			}
			@CS_スムーズカメラ降順 $6 -25000 -27000 10250 12250
			eval( $i:補完ＰＹ = $i:CS_結果１ )
			@CS_スムーズカメラ降順 $6 -25000 -27000 1500 3500
			eval( $i:補完ＴＹ = $i:CS_結果１ )

			@CS_スムーズカメラ降順 $4 9573 5427 9500 5150
			eval( $i:補完ＰＸ = $i:CS_結果１ )
			@CS_スムーズカメラ降順 $4 9573 5427 9500 5150
			eval( $i:補完ＴＸ = $i:CS_結果１ )
			chara カメラ設定 モーションカメラ \
				-c 1 \
				-p $i:補完ＰＸ,$i:補完ＰＹ,$i:補完ＰＺ \
				-t $i:補完ＴＸ,$i:補完ＴＹ,$i:補完ＴＺ \
				-a 200 \
				-i 3 2 0 0 \
				-s $3
		}
	// ------------------------
	// 手すり張りつきカメラ
	// ------------------------
	trap ca101 d:PLAYER \
		-mask ？ \
		-dir 1024 512 \
		-state d:TRP_STATE_CAUTION \
		-camera \
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN	d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,1500,d:CAM_MIN			d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 825,2048,0 -f 7500 \
				-a 200 \
				-i 3 3 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3
		}
	trap ca102 d:PLAYER \
		-mask ？ \
		-dir 1024 512 \
		-state d:TRP_STATE_CAUTION \
		-camera \
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN	d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,1500,d:CAM_MIN			d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 825,2048,0 -f 7500 \
				-a 200 \
				-i 3 3 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3
		}
	trap ca103 d:PLAYER \
		-mask ？ \
		-dir 1024 512 \
		-state d:TRP_STATE_CAUTION \
		-camera \
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN	d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,1500,d:CAM_MIN			d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 825,2048,0 -f 7500 \
				-a 200 \
				-i 3 3 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3
		}
	trap ca111 d:PLAYER \
		-mask ？ \
		-dir 2048 512 \
		-state d:TRP_STATE_CAUTION \
		-camera \
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN		d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,1500,d:CAM_MIN				d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 825,2048,0 -f 7500 \
				-a 200 \
				-i 3 3 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3
		}
	trap ca112 d:PLAYER \
		-mask ？ \
		-dir 2048 512 \
		-state d:TRP_STATE_CAUTION \
		-camera \
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN		d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,1500,d:CAM_MIN				d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 825,2048,0 -f 7500 \
				-a 200 \
				-i 3 3 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3
		}
	trap ca113 d:PLAYER \
		-mask ？ \
		-dir 2048 512 \
		-state d:TRP_STATE_CAUTION \
		-camera \
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN		d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,1500,d:CAM_MIN				d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 825,2048,0 -f 7500 \
				-a 200 \
				-i 3 3 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3
		}
	// 1F 階段前通路北側
	trap ca121 d:PLAYER \
		-mask ？ \
		-dir 0 512 \
		-state d:TRP_STATE_CAUTION \
		-camera \
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN		d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,-3500,d:CAM_MIN			d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 930,2048,0 -f 7500 \
				-a 200 \
				-i 3 3 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3
		}
}




// -----------------------------------------------------------------------------------
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

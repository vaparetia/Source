/*
	vs03a_common.gcl
	2002/06/21 T.Ohtani
	$Id: vs03a_common.h,v 1.1 2002/06/27 01:09:08 usr13731 Exp $
*/
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
		chara カメラ クリア時カメラ		-c 0 -l 0 -p 10

		// デフォルトカメラ
		chara カメラ設定 ステージカメラ \
			-c 0 \
			-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN 	d:CAM_MAX,d:CAM_MAX d:CAM_MAX \
			-l 2478,1500,-13500 				10600,d:CAM_MAX,-1500 \
			-r 700,2048,0 \
			-f 9000 \
			-a 200 \
			-i 2 2 0 0 \
			-s 1
	}

	// ステージ固有のカメラ設定
	// ------------------------------------
	proc vs03aカメラ設定 {
		#if d:DEBUG_PRINT
			print 'stage_camera_set'
		#endif

		// ステージ東窪み
		trap ca001 d:PLAYER \
			-mask ？ \
			-camera \
			-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN	d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l 2478,1500,-13500  				10600,d:CAM_MAX,-1500 \
					-r 870,2048,0 \
					-f 9000 \
					-a 200 \
					-i 3 3 0 0 \
					-s $3
			}
		// スタート地点北
		trap ca002 d:PLAYER \
			-mask ？ \
			-camera \
			-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN	d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l 2478,1500,-13500  				10600,d:CAM_MAX,-1500 \
					-r 870,2048,0 \
					-f 9000 \
					-a 200 \
					-i 3 3 0 0 \
					-s $3
			}
		// スタート地点北東
		trap ca003 d:PLAYER \
			-mask ？ \
			-camera \
			-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN	d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l 2478,1500,-13500  				10600,d:CAM_MAX,-1500 \
					-r 870,2048,0 \
					-f 9000 \
					-a 200 \
					-i 3 3 0 0 \
					-s $3
			}
		// 中央障害物北
		trap ca004 d:PLAYER \
			-mask ？ \
			-camera \
			-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN	d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l 2478,1500,-13500  				10600,d:CAM_MAX,-1500 \
					-r 870,2048,0 \
					-f 9000 \
					-a 200 \
					-i 3 3 0 0 \
					-s $3
			}
		// 東障害物北
		trap ca005 d:PLAYER \
			-mask ？ \
			-camera \
			-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN	d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l 2478,1500,-13500  				10600,d:CAM_MAX,-1500 \
					-r 870,2048,0 \
					-f 9000 \
					-a 200 \
					-i 3 3 0 0 \
					-s $3
			}

		// 北端通路
		trap ca006 d:PLAYER \
			-mask ？ \
			-camera \
//			-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
			-exec {
				chara カメラ設定 メインカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN	d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l 2478,1500,d:CAM_MIN  			10600,d:CAM_MAX,d:CAM_MAX \
					-r 820,2048,0 \
					-f 9000 \
					-a 200 \
					-i 3 3 0 0 \
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

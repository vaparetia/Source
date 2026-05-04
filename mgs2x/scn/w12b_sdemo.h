//=============================================================================
// w12b_sdemo.h
// Ａ脚・ポンプ室(シナリオデモ)
// 2001/07/23 S.Yamashita
// $Id: w12b_sdemo.h,v 1.14 2002/05/09 13:07:49 usr01475 Exp $
//

//-----------------------------------------------------------------------------
// 開始

proc w12b_シナリオデモ開始 {
#if d:DEBUG_PRINT
	print '*** Scenario Demo Start ***'
#endif

	// パッドデモ
	chara パッドデモ パッドデモ \
#ifdef d:PAL
		-file     w12b_paddemo_pal \
#else
		-file     w12b_paddemo \
#endif
		-end_proc w12b_シナリオデモ終了

	chara デモキャンセルチェック デモキャン -p w12b_シナリオデモキャンセル
	@シナリオデモキャラセット
	@シナリオデモ開始処理

	// 開始
	@w12b_シナリオデモ実行
}

//-----------------------------------------------------------------------------
// 終了

proc w12b_シナリオデモ終了 {
#if d:DEBUG_PRINT
	print '*** Scenario Demo End ***'
#endif

	mesg デモキャンセルチェック デモキャン kill
	@SS_CB_フェードアウト後
}

proc w12b_シナリオデモキャンセル {
#if d:DEBUG_PRINT
	print '*** Scenario Demo Cancel ***'
#endif

	@SS_フェードアウト
}





//-----------------------------------------------------------------------------
// シナリオデモ

proc w12b_シナリオデモ実行 {
#if d:DEBUG_PRINT
	print '*** Scenario Demo Execute ***'
#endif

	mesg パッドデモ パッドデモ start

	chara delay ディレイ \
#ifdef d:PAL
		-time 140 \ 
#else
		-time 130 \ 
#endif
		-exec{
			eval($i:STREAM_ID = `%Stream t:vc045031`)
		}

	chara delay ディレイ \
#ifdef d:PAL
		-time 420 \ 
#else
		-time 400 \ 
#endif
		-exec{
			mesg 警備兵 敵兵:02 ルート変更 6    0 5000 -15000 制御室
			mesg 警備兵 敵兵:01 ルート変更 9 5000    0  -1250 制御室
		}

	chara delay ディレイ \
#ifdef d:PAL
		-time 780 \ 
#else
		-time 710 \ 
#endif
		-exec{
			chara カメラ設定 でもデモカメラ \
				-c 1 \
				-p -8245,4396,2213 \
				-t -7888,2732,12 \
				-a 200 \
				-i 0 0 0 0 \
				-s 1

			chara カメラ設定 でもデモカメラ \
				-c 1 \
				-p -2358,4395,2212 \
				-t -2001,2732,12 \
				-a 200 \
				-z d:CAMERA_INTERP_CAMERA \ //補完カメラフラグを指定。
#ifdef d:PAL
				-interp_time 200 \ 			//補完時間を指定
#else
				-interp_time 250 \ 			//補完時間を指定
#endif
				-s 1
		}

	chara delay ディレイ \
		-time 980 \ 
		-exec{
			chara カメラ設定 でもデモカメラ \
				-c 1 \
				-p 1528,3419,-5540 \
				-t 1646,2875,-6264 \
				-a 200 \
				-z d:CAMERA_INTERP_CAMERA \ //補完カメラフラグを指定。
				-interp_time 300 \ 			//補完時間を指定
				-s 1
		}
}

//=============================================================================
// w44a_sdemo.h
// ＡＧ・第二メタルハンガー(シナリオデモ)
// 2001/08/18 S.Yamashita
// $Id: w44a_sdemo.h,v 1.5 2001/08/28 07:36:43 usr03692 Exp $
//

proc サイファーデモキャラセット {
#if d:DEBUG_PRINT
	print '*** Cypher Demo Characters Set ***'
#endif

	//---------------------------------------------------------
	// デモスネーク
	if($w:ゲーム設定 < d:LEVEL_NORMAL) {
		chara プットモーションモデル デモスネーク \
			-pos 35000 4000 -96000 \
			-dir 2400 \
			-kms sna_def \
			-evm sna_def_mh \
			-mar sna_w44a \
			-number d:モーションリスト:fms_stand
		chara 装備品Ｃ Ｍ４ \
			-n m4a_nm \
			-y デモスネーク \
			-z 6 \
			-x 0,0,0 \
			-r 0,0,0 \
			-f 1

	} else {
		chara プットモーションモデル デモスネーク \
			-pos 35000 4000 -96000 \
			-dir 2400 \
			-kms sna_def \
			-evm sna_def_mh \
			-mar sna_w44a \
			-number d:モーションリスト:m92_stand
		chara 装備品Ｃ ＵＳＰ \
			-n usp \
			-y デモスネーク \
			-z 6 \
			-x 0,0,0 \
			-r 0,0,0 \
			-f 1
	}

	//---------------------------------------------------------
	// サイファ
	chara サイファ サイファ \
		-route       97 0 \
		-type        0 \						/*0-NORMAL,1-GUN*/
		-i           25000 25000 25000 25000 \	/*4つまで 通常 危険 警戒 回避 の順 */
		-speed       120 \						/*初期値80*/
		-kinkyuspeed 500 \						/* 省略可能 初期値 speedと同じ*/
		-fkasoku     400 600 \					/*初期値 200*/ 
		-touchaku    ノードに到着 98 1

	//---------------------------------------------------------
	// カメラ
	chara カメラ設定 でもデモカメラ \
		-c 1 \
		-p 29998,4779,-92273 \
		-t 34398,4779,-94748 \
		-a 180 \
		-i 0 0 0 0 \
		-s 1
}

proc ノードに到着 {
#if d:DEBUG_PRINT
	print '*** Reached To The Point ***'
#endif

//	mesg サイファ サイファ びっくり
	mesg コマンダー 敵兵セット 視界オン
	chara delay ディレイ \
		-time 80 \
		-exec {
			mesg サイファ サイファ 強制通常状態
			mesg コマンダー 敵兵セット 視界オフ
			mesg サイファ サイファ ルート変更 99 0
		}
}

proc サイファーデモ開始 {
#if d:DEBUG_PRINT
	print '*** Cypher Demo Start ***'
#endif

	mesg サイファ サイファ ルート変更 98 0

	chara カメラ設定 でもデモカメラ \
		-c 1 \
		-p 30423,6045,-92905 \
		-t 34240,8353,-95413 \
		-a 180 \
		-z d:CAMERA_INTERP_CAMERA \ //補完カメラフラグを指定。
		-interp_time 60 \ 			//補完時間を指定
		-s 1

	chara delay ディレイ \
		-time 350 \
		-exec {
			@サイファーデモ終了
		}
}

proc サイファーデモキャンセル {
#if d:DEBUG_PRINT
	print '*** Cypher Demo Cancel ***'
#endif

	@サイファーデモ終了
}

proc サイファーデモ終了 {
#if d:DEBUG_PRINT
	print '*** Cypher Demo End ***'
#endif

	eval($f:デモ再生フラグ = 0)
	@シナリオデモ終了処理

	// リスタート
	eval($i:プレイヤー初期Ｘ位置 =  33000)
	eval($i:プレイヤー初期Ｙ位置 =   4000)
	eval($i:プレイヤー初期Ｚ位置 = -94000)
	eval($i:プレイヤー初期方向   =   2048)
	eval($s:登場ポイント = デモ後リスタート)
	restart -save
//	restart
}

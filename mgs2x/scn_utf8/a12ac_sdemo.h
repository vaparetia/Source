//=============================================================================
// a12ac_sdemo.h
// Ａ脚・屋上(シナリオデモ スネークテイルズ用)
// 2002/06/06 H.Yoshiike
// $Id: a12ac_sdemo.h,v 1.1 2002/06/06 07:06:10 usr03682 Exp $
//

//-----------------------------------------------------------------------------
// 昇降機設置

#define	ELV_NAME elv_1
//-----------------------------------------------------------------------------
// 昇降機デモ

proc 昇降機デモ１ {
#if d:DEBUG_PRINT
	print '*** Elevator Demo 1 ***'
#endif

	//##### フォーチュン戦後以外の降下時 #####

	// 進入禁止ゾーンを設定
	command セットゾーンフラグ d:HZX_ZONE_ZINTRPT 0 5000 -12250

	@シナリオデモ開始処理
	@ＳＥＴエレベータ状態 (d:_E_ON_DOWN_DEMO)

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

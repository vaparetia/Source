//=============================================================================
// w14a_sdemo.h
// Ｂ脚・変電室(シナリオデモ)
// 2001/08/12 S.Yamashita
// $Id: w14a_sdemo.h,v 1.7 2002/10/03 15:15:49 usr04761 Exp $
//

//-----------------------------------------------------------------------------
// ソーコムパッドデモ

proc ソーコムパッドデモ開始 {
#if d:DEBUG_PRINT
	print '*** Pad Demo Start ***'
#endif

	//---------------------------------------------------------
	// パッドデモ
	chara パッドデモ パッドデモ \
		-file     w14a_pdemo_00 \
		-end_proc ソーコムパッドデモ終了

	//---------------------------------------------------------
	// デモキャンセル
	chara デモキャンセルチェック デモキャン -p ソーコムパッドデモキャンセル

	//---------------------------------------------------------
	// @シナリオデモ開始処理
	command シナリオデモ開始
	command 無線設定 -reset
	command メニュー設定 -menu on -gage off -radar off -subwin off
	mesg シネマスクリーン シネマ フェードアウト 0		//ゼロでデフォルト値
	command パッド操作 -release
	command プレイヤー無敵セット
	@特殊装備解除

	mesg レーダー レーダー off

	//---------------------------------------------------------
	// 前武器変更
	command シナリオ前武器変更 -number d:武器:ソコム
	eval($w:武器 = d:武器:素手)

	//---------------------------------------------------------
	// 開始
	mesg パッドデモ パッドデモ start

	eval($w:ソーコムパッドデモステップ = 0)
	chara delay パッドデモディレイ \
		-time 296 \
		-exec{
			if($w:ソーコムパッドデモステップ == 0) {
				print '覗き込み説明'
				if ( $w:言語 == d:LANG_FRENCH ) {
					@説明テクスチャ表示 説明テクスチャ act_telop03_fr_alp_ovl 165
				} else if ( $w:言語 == d:LANG_GERMANY ) {
					@説明テクスチャ表示 説明テクスチャ act_telop03_de_alp_ovl 165
				} else if ( $w:言語 == d:LANG_ITALY ) {
					@説明テクスチャ表示 説明テクスチャ act_telop03_it_alp_ovl 165
				} else if ( $w:言語 == d:LANG_SPANISH ) {
					@説明テクスチャ表示 説明テクスチャ act_telop03_es_alp_ovl 165
				} else if ( $w:言語 == d:LANG_JAPANESE ) {
					@説明テクスチャ表示 説明テクスチャ act_telop03_jp_alp_ovl 165
				} else {
					@説明テクスチャ表示 説明テクスチャ act_telop3_alp_ovl 165
				}
				eval($w:ソーコムパッドデモステップ = $w:ソーコムパッドデモステップ +1)
				return 240

			} else if($w:ソーコムパッドデモステップ == 1) {
				print '飛び出し撃ち説明'
				if ( $w:言語 == d:LANG_FRENCH ) {
					@説明テクスチャ表示 説明テクスチャ act_telop04_fr_alp_ovl 165
				} else if ( $w:言語 == d:LANG_GERMANY ) {
					@説明テクスチャ表示 説明テクスチャ act_telop04_de_alp_ovl 165
				} else if ( $w:言語 == d:LANG_ITALY ) {
					@説明テクスチャ表示 説明テクスチャ act_telop04_it_alp_ovl 165
				} else if ( $w:言語 == d:LANG_SPANISH ) {
					@説明テクスチャ表示 説明テクスチャ act_telop04_es_alp_ovl 165
				} else if ( $w:言語 == d:LANG_JAPANESE ) {
					@説明テクスチャ表示 説明テクスチャ act_telop04_jp_alp_ovl 165
				} else {
					@説明テクスチャ表示 説明テクスチャ act_telop4_alp_ovl 165
				}
				eval($w:ソーコムパッドデモステップ = $w:ソーコムパッドデモステップ +1)
				return 240

			} else if($w:ソーコムパッドデモステップ == 2) {
				print '主観撃ち説明'
				if ( $w:言語 == d:LANG_FRENCH ) {
					@説明テクスチャ表示 説明テクスチャ act_telop06_fr_alp_ovl 165
				} else if ( $w:言語 == d:LANG_GERMANY ) {
					@説明テクスチャ表示 説明テクスチャ act_telop06_de_alp_ovl 165
				} else if ( $w:言語 == d:LANG_ITALY ) {
					@説明テクスチャ表示 説明テクスチャ act_telop06_it_alp_ovl 165
				} else if ( $w:言語 == d:LANG_SPANISH ) {
					@説明テクスチャ表示 説明テクスチャ act_telop06_es_alp_ovl 165
				} else if ( $w:言語 == d:LANG_JAPANESE ) {
					@説明テクスチャ表示 説明テクスチャ act_telop06_jp_alp_ovl 165
				} else {
					@説明テクスチャ表示 説明テクスチャ act_telop6_alp_ovl 165
				}
				eval($w:ソーコムパッドデモステップ = $w:ソーコムパッドデモステップ +1)
			}
		}
}

//-----------------------------------------------------------------------------
// ソーコムパッドデモ終了

proc ソーコムパッドデモ終了 {
#if d:DEBUG_PRINT
	print '*** Pad Demo End ***'
#endif

	eval($f:デモ再生フラグ = 0)

	//---------------------------------------------------------
	// kill
	mesg デモキャンセルチェック デモキャン kill
	mesg パッドデモ パッドデモ kill
	mesg delay パッドデモディレイ kill
	mesg ２Ｄスプライト表示 説明テクスチャ kill

	//---------------------------------------------------------
	// @シナリオデモ終了処理
	command シナリオデモ終了
	command メニュー設定 -menu on -gage on -radar on -subwin on
	mesg シネマスクリーン シネマ フェードイン 0		//ゼロでデフォルト値
	command パッド操作 -cancel
	command プレイヤー無敵解除
	command StreamStopAll

	mesg レーダー レーダー on

	//---------------------------------------------------------
	// プレイヤー初期位置修正
	eval($i:プレイヤー初期Ｘ位置 = -51103)
	eval($i:プレイヤー初期Ｙ位置 =      0)
	eval($i:プレイヤー初期Ｚ位置 = -26971)
	eval($i:プレイヤー初期方向   =   2048)
	eval($w:武器                 = d:武器:ソコム);
	command varsave $i:プレイヤー初期Ｘ位置
	command varsave $i:プレイヤー初期Ｙ位置
	command varsave $i:プレイヤー初期Ｚ位置
	command varsave $i:プレイヤー初期方向
	command varsave $w:武器
	command varsave $w:武器弾数Ｒ[d:武器:ソコム]
	command varsave $w:アイテム最大数Ｒ[d:アイテム:髭剃り]
	command varsave $w:アイテム数Ｒ[d:アイテム:煙草]
}

//-----------------------------------------------------------------------------
// ソーコムパッドデモキャンセル

proc ソーコムパッドデモキャンセル {
#if d:DEBUG_PRINT
	print '*** Pad Demo Cancel ***'
#endif

	@SS_フェードアウト
}
proc SS_CB_フェードアウト後 {
#if d:DEBUG_PRINT
	print '*** After Fade Out ***'
#endif

	eval($f:デモ再生フラグ = 0)

	//---------------------------------------------------------
	// kill
	mesg デモキャンセルチェック デモキャン kill
	mesg パッドデモ パッドデモ kill
	mesg delay パッドデモディレイ kill
	mesg ２Ｄスプライト表示 説明テクスチャ kill

	//---------------------------------------------------------
	// @シナリオデモ終了処理
	command シナリオデモ終了
	command メニュー設定 -menu on -gage on -radar on -subwin on
	mesg シネマスクリーン シネマ フェードイン 0		//ゼロでデフォルト値
//	command パッド操作 -cancel
	command プレイヤー無敵解除
	command StreamStopAll

	mesg レーダー レーダー on

	//---------------------------------------------------------
	// 強制移動
	chara delay ディレイ \
		-time 20 \
		-exec {
			mesg プレイヤー d:PLAYER position -51103 1000 -26971
		}
	chara delay ディレイ \
		-time 30 \
		-exec {
			mesg プレイヤー d:PLAYER rotate 2048
		}

	//---------------------------------------------------------
	// プレイヤー初期位置修正
	eval($i:プレイヤー初期Ｘ位置 = -51103)
	eval($i:プレイヤー初期Ｙ位置 =      0)
	eval($i:プレイヤー初期Ｚ位置 = -26971)
	eval($i:プレイヤー初期方向   =   2048)
	eval($w:武器                 = d:武器:ソコム);
	command varsave $i:プレイヤー初期Ｘ位置
	command varsave $i:プレイヤー初期Ｙ位置
	command varsave $i:プレイヤー初期Ｚ位置
	command varsave $i:プレイヤー初期方向
	command varsave $w:武器
	command varsave $w:武器弾数Ｒ[d:武器:ソコム]
	command varsave $w:アイテム最大数Ｒ[d:アイテム:髭剃り]
	command varsave $w:アイテム数Ｒ[d:アイテム:煙草]

	//---------------------------------------------------------
	// フェードイン
	chara delay ディレイ \
		-time 40 \
		-exec {
			@SS_フェードイン
		}
}
proc SS_CB_フェードイン後 {
#if d:DEBUG_PRINT
	print '*** After Fade In ***'
#endif

	command パッド操作 -cancel
}

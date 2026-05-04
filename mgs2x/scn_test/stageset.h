//=============================================================================
// stageset.h
// 2001/08/04 S.Yamashita
// $Id: stageset.h,v 1.1 2002/02/01 06:08:32 usr01475 Exp $
//

//=============================================================================
// フラグ
//=============================================================================

eval($i:SS_敵兵ステータス         = 0)	// 通常は 0
eval($i:SS_ロッカー状態           = 0)
eval($i:SS_ロッカー番号           = 0)

//=============================================================================
// 敵兵ＩＤ
//=============================================================================

enum SS_警備兵ＩＤ {
	Ａ脚屋上兵,
	Ａ脚屋上兵_ＤＥＭＯ,	// シナリオデモ用
	Ａ脚制御室兵,
	Ａ脚制御室兵_ＤＥＭＯ,	// シナリオデモ用
	Ａ脚ポンプ室兵,
	Ｂ脚変電室兵,
	Ｂ脚廊下兵,
	Ｃ脚食堂兵,
	Ｃ脚トイレ兵,
	Ｅ脚２Ｆ兵,
	Ｅ脚ヘリポート兵,
	Ｅ脚ヘリポート応援兵,
};

//=============================================================================
// コマンダー音声リソース
//=============================================================================

#ifdef d:STAGE_W12A || d:STAGE_W12C	//----- w12a, w12c

	resource ghq_area_alert_voice {
	w12a:
		$t{
			// PLAREA0
			t:vc040110,	// 指揮官　　「了解。応援部隊、Ａ脚屋上へ向かえ！侵入者を逃がすな！」
			t:vc040111,	// 指揮官　　「了解。増援部隊をＡ脚屋上に送る！敵を包囲するんだ！」
			-1
		}
	}

	resource ghq_caution_voice {
	w12a:
		$t{
			t:vc040274,	// 指揮官　　「了解。Ａ脚屋上を増員する。警戒を強化せよ」
			t:vc040275,	// 指揮官　　「了解。Ａ脚屋上に増員を送る。パトロールを強化せよ！」
			-2,
			t:vc040418,	// 指揮官　　「Ａ脚屋上との連絡が途絶えた」
			-1
		}
	}

#elifdef d:STAGE_W12B	//----- w12b

	resource ghq_area_alert_voice {
	w12b:
		$t{
			// PLAREA0
			t:vc040112,	// 指揮官　　「了解。増援部隊、ＡＢ連絡橋への扉付近を守れ！侵入者を食い止めるんだ！」
			t:vc040113,	// 指揮官　　「了解。応援部隊をＡ脚いっかい１Ｆに派遣する！ＡＢ連絡橋への扉を固めろ」
			-2,
			// PLAREA1
			t:vc040114,	// 指揮官　　「了解。増援部隊、Ａ脚いっかい１Ｆ・ＦＡ連絡橋への扉付近を守れ！侵入者を食い止めるんだ！」
			t:vc040115,	// 指揮官　　「了解。バックアップをＡ脚いっかい１Ｆに送る。ＦＡ連絡橋への扉を守れ」
			-2,
			// PLAREA2
			t:vc040116,	// 指揮官　　「了解。応援部隊、Ａ脚いっかい１Ｆ制御室へ向かえ！敵を包囲せよ！」
			t:vc040117,	// 指揮官　　「了解。増援部隊をＡ脚いっかい１Ｆに送る。制御室を守れ！」
			-2,
			// PLAREA3
			t:vc040118,	// 指揮官　　「了解。増援部隊、Ａ脚いっかい１Ｆポンプ室へ急行せよ！敵を逃がすな！」
			t:vc040119,	// 指揮官　　「了解。増援部隊をＡ脚いっかい１Ｆに派遣する。ポンプ室を包囲しろ！」
			-2,
			// PLAREA4
			t:vc040120,	// 指揮官　　「了解。増援部隊、Ａ脚いっかい１Ｆへ向かえ！敵を殲滅せよ！」
			t:vc040121,	// 指揮官　　「了解。応援部隊、Ａ脚いっかい１Ｆへ急行せよ！侵入者を逃がすな！」
			-1
		}
	}

	resource ghq_caution_voice {
	w12b:
		$t{
			t:vc040285,	// 指揮官　　「了解。Ａ脚いっかい１Ｆに増援を送る。警備を強化しろ」
			t:vc040284,	// 指揮官　　「了解。Ａ脚いっかい１Ｆを増員。警戒を強化しろ」
			-2,
			t:vc040419,	// 指揮官　　「Ａ脚１Ｆとの連絡が途絶えた」
			-3,
			t:vc040605,	// 指揮官　　「(監視カメラ破壊時の音声)」
			-1
		}
	}

#elifdef d:STAGE_W14A	//----- w14a

	resource ghq_area_alert_voice {
	w14a:
		$t{
			// PLAREA0
			t:vc040128,	// 指揮官　　「了解。増援部隊、Ｂ脚変電室へ向かえ！奴を仕留めろ！」
			t:vc040129,	// 指揮官　　「了解。増援部隊をＢ脚に送る！変電室を守れ！」
			-2,
			// PLAREA1
			t:vc040130,	// 指揮官　　「了解。増援部隊、Ｂ脚に向かえ。ＡＢ連絡橋への扉を固めろ」
			t:vc040131,	// 指揮官　　「了解。応援部隊、Ｂ脚・ＡＢ連絡橋への扉付近に向かえ！奴を食い止めろ！」
			-2,
			// PLAREA2
			t:vc040132,	// 指揮官　　「了解。増援部隊、Ｂ脚に向かえ！ＢＣ連絡橋への扉を固めろ！」
			t:vc040133,	// 指揮官　　「了解。応援部隊、Ｂ脚・ＢＣ連絡橋への扉付近に向かえ！奴を食い止めろ！」
			-2,
			// PLAREA3
			t:vc040134,	// 指揮官　　「了解。応援部隊、Ｂ脚へ向かえ！敵を包囲するんだ！」
			t:vc040135,	// 指揮官　　「了解。増援部隊、Ｂ脚へ向かえ！敵を逃がすな！」
			-1
		}
	}

	resource ghq_caution_voice {
	w14a:
		$t{
			t:vc040292,	// 指揮官　　「了解。Ｂ脚・変電室を増員する。警備を強化しろ」
			t:vc040293,	// 指揮官　　「了解。Ｂ脚に増員を送る。変電室の警戒を強化しろ」
			-2,
			t:vc040421,	// 指揮官　　「Ｂ脚との連絡が途絶えた」
			-3,
			t:vc040606,	// 指揮官　　「(監視カメラ破壊時の音声)」
			-1
		}
	}

#elifdef d:STAGE_W16A || d:STAGE_W16B	//----- w16a, w16b

	resource ghq_area_alert_voice {
	w16a:
		$t{
			// PLAREA0
			t:vc040142,	// 指揮官　　「了解。応援部隊をＣ脚・男子トイレへ派遣する！奴を追い込むんだ」
			t:vc040143,	// 指揮官　　「了解。増援部隊、Ｃ脚に向かえ！男子トイレを固めろ」
			-2,
			// PLAREA1
			t:vc040144,	// 指揮官　　「了解。増援部隊、Ｃ脚・女子トイレへ向かえ！敵を追い込め」
			t:vc040145,	// 指揮官　　「了解。応援部隊をＣ脚に送る！女子トイレを守れ！」
			-2,
			// PLAREA2
			t:vc040146,	// 指揮官　　「了解。増援部隊、Ｃ脚食堂へ向かえ。奴を逃がすな！」
			t:vc040147,	// 指揮官　　「了解。応援部隊をＣ脚に送る！食堂を包囲しろ！」
			-2,
			// PLAREA3
			t:vc040148,	// 指揮官　　「了解。増援部隊、Ｃ脚へ急行せよ！敵を追い込むんだ！」
			t:vc040149,	// 指揮官　　「了解。バックアップ、Ｃ脚へ向かえ！敵を仕留めろ！」
			-1
		}
	}

	resource ghq_caution_voice {
	w16a:
		$t{
			t:vc040312,	// 指揮官　　「了解。Ｃ脚を増員する。警備を強化しろ」
			t:vc040313,	// 指揮官　　「了解。Ｃ脚に増員を送る。警戒を強化しろ」
			-2,
			t:vc040423,	// 指揮官　　「Ｃ脚との連絡が途絶えた」
			-1
		}
	}

#elifdef d:STAGE_W20B || d:STAGE_W20C || d:STAGE_W20D	//----- w20b, w20c, w20d

	resource ghq_area_alert_voice {
	w20b:
		$t{
			// PLAREA0
			t:vc040188,	// 指揮官　　「了解。バックアップ、Ｅ脚屋上ヘリポートへ急行せよ！敵を包囲するんだ！」
			t:vc040189,	// 指揮官　　「了解。増援部隊、Ｅ脚屋上に急行せよ！ヘリポートを守れ！」
			-2,
			// PLAREA1
			t:vc040190,	// 指揮官　　「了解。増援部隊、Ｅ脚屋上へ向かえ。Ｅ脚への扉を固めろ！」
			t:vc040191,	// 指揮官　　「了解。応援部隊をＥ脚屋上・Ｅ脚への扉付近に送る。侵入者を逃がすな！」
			-2,
			// PLAREA2
			t:vc040192,	// 指揮官　　「了解。応援部隊、Ｅ脚屋上に急行せよ！敵を包囲するんだ！」
			t:vc040193,	// 指揮官　　「了解。増援部隊をＥ脚屋上に送る。敵を逃がすな！」
			-1
		}
	}

	resource ghq_caution_voice {
	w20b:
		$t{
			t:vc040351,	// 指揮官　　「了解。Ｅ脚屋上に増員を送る。ヘリポートの警戒を強化しろ」
			t:vc040350,	// 指揮官　　「了解。Ｅ脚屋上ヘリポートを増員。警備を強化しろ」
			-2,
			t:vc040428,	// 指揮官　　「Ｅ脚屋上ヘリポートとの連絡が途絶えた」
			-1
		}
	}

#elifdef d:STAGE_W42A	//----- w42a

	resource ghq_caution_voice {
	w42a:
		$t{
			t:vc040601,	// 指揮官　　「」
			-2,
			t:vc049066,	// 指揮官　　「どうしたんだ」
			-1
		}
	}

#endif

//=============================================================================
// 共通設定 proc
//=============================================================================

//-----------------------------------------------------------------------------
// サウンド設定
proc SS_サウンド設定 {
#if d:DEBUG_PRINT
	print '*** SS Sound Set ***'
#endif

	// ※※※ w20b では使用していない ※※※

#ifdef d:JAPANESE	//----- 日本語版

	if(    ($s:エリア != w12a) \
		&& ($s:エリア != w12c) \
		&& (   (   ($w:p_story >= d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了) \
				&& ($w:p_story <  d:ST:P026_01_R01爆弾解体終了１無線デモ１開始)) \
			|| (   ($w:p_story >= d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) \
				&& ($w:p_story <  d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始)))) {
		chara サウンドマネージャー ＳＤマネ \
			-pak 1	// パック番号
	} else {
		chara サウンドマネージャー ＳＤマネ \
			-pak 0	// パック番号
	}

#else				//----- 英語版

	if(    ($s:エリア != w12a) \
		&& ($s:エリア != w12c) \
		&& (   (   ($w:p_story >= d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了) \
				&& ($w:p_story <  d:ST:P026_01_R01爆弾解体終了１無線デモ１開始)) \
			|| (   ($w:p_story >= d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) \
				&& ($w:p_story <  d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始)))) {
		chara サウンドマネージャー ＳＤマネ \
			-pak 1	// パック番号
	} else {

		if($w:p_story < d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {
			chara サウンドマネージャー ＳＤマネ \
				-pak 0	// パック番号

		} else {

		#ifdef d:STAGE_W12C		//----- w12c
			chara サウンドマネージャー ＳＤマネ \
				-pak 1	// パック番号

		#elifdef d:STAGE_W12B	//----- w12b
			chara サウンドマネージャー ＳＤマネ \
				-pak 2	// パック番号

		#elifdef d:STAGE_W14A	//----- w14b
			chara サウンドマネージャー ＳＤマネ \
				-pak 2	// パック番号

		#elifdef d:STAGE_W16B	//----- w16b
			chara サウンドマネージャー ＳＤマネ \
				-pak 2	// パック番号

		#else
			chara サウンドマネージャー ＳＤマネ \
				-pak 0	// パック番号

		#endif
		}

	}
#endif

	@サウンド効果音設定
}

//-----------------------------------------------------------------------------
// 跳弾弾痕設定
proc SS_跳弾弾痕設定 {
#if d:DEBUG_PRINT
	print '*** SS Recoil Scar Set ***'
#endif

#ifdef d:STAGE_W12A || d:STAGE_W12C	//----- w12a, w12c

	command 跳弾ノーマル -n 0	// 通常
	command 跳弾ノーマル -n 1	// 鉄板
	command 跳弾ノーマル -n 2
	command 跳弾ノーマル -n 3	// 鉄筋
	command 跳弾ＳＥのみ -n 4	// ×金網
	command 跳弾ＳＥのみ -n 5
	command 跳弾ＳＥのみ -n 6	// ×ダンボール
	command 跳弾ＳＥのみ -n 7

	command 弾痕ノーマル -n 0
	command 弾痕ノーマル -n 1
	command 弾痕ノーマル -n 2
	command 弾痕ノーマル -n 3
	command 弾痕ノーマル -n 4
	command 弾痕ノーマル -n 5
	command 弾痕ノーマル -n 6
	command 弾痕ノーマル -n 7

#elifdef d:STAGE_W12B	//----- w12b

	command 跳弾ノーマル -n 0	// 通常
	command 跳弾ノーマル -n 1	// 鉄板
	command 跳弾ノーマル -n 2
	command 跳弾ノーマル -n 3	// 鉄筋
	command 跳弾ノーマル -n 4	// 鉄板
	command 跳弾ノーマル -n 5	// サッシ
	command 跳弾ＳＥのみ -n 6	// ×ダンボール
	command 跳弾ＳＥのみ -n 7

	command 弾痕ノーマル -n 0
	command 弾痕ノーマル -n 1
	command 弾痕ノーマル -n 2
	command 弾痕ノーマル -n 3
	command 弾痕ノーマル -n 4
	command 弾痕ノーマル -n 5
	command 弾痕ノーマル -n 6
	command 弾痕ノーマル -n 7

#elifdef d:STAGE_W14A	//----- w14a

	command 跳弾ノーマル -n 0	// 通常
	command 跳弾ノーマル -n 1	// 通常
	command 跳弾ノーマル -n 2	// 鉄板
	command 跳弾ノーマル -n 3	// 鉄筋
	command 跳弾ノーマル -n 4	// 鉄板
	command 跳弾ＳＥのみ -n 5	// ×金網
	command 跳弾ＳＥのみ -n 6	// ×ダンボール
//	command 跳弾ＳＥのみ -n 7	// ×ガラス
	command	跳弾水蒸気中 -n 7

	command 弾痕ノーマル -n 0
	command 弾痕ノーマル -n 1
	command 弾痕ノーマル -n 2
	command 弾痕ノーマル -n 3
	command 弾痕ノーマル -n 4
	command 弾痕ノーマル -n 5
	command 弾痕ノーマル -n 6
	command 弾痕ノーマル -n 7

#elifdef d:STAGE_W16A || d:STAGE_W16B	//----- w16a, w16b

	command 跳弾ノーマル -n 0	// 通常
	command 跳弾ノーマル -n 1	// 鉄板
	command 跳弾ノーマル -n 2
	command 跳弾ノーマル -n 3	// 鉄筋
	command 跳弾ノーマル -n 4	// 鉄板
	command 跳弾ＳＥのみ -n 5
	command 跳弾ＳＥのみ -n 6	// ×ダンボール
	command 跳弾ＳＥのみ -n 7

	command 弾痕ノーマル -n 0
	command 弾痕ノーマル -n 1
	command 弾痕ノーマル -n 2
	command 弾痕ノーマル -n 3
	command 弾痕ノーマル -n 4
	command 弾痕ノーマル -n 5
	command 弾痕ノーマル -n 6
	command 弾痕ノーマル -n 7

#elifdef d:STAGE_W20B || d:STAGE_W20C || d:STAGE_W20D	//----- w20b, w20c, w20d

	command 跳弾ノーマル -n 0	// 通常
	command 跳弾ノーマル -n 1	// 鉄板
	command 跳弾ノーマル -n 2	// 通常
	command 跳弾ノーマル -n 3	// 鉄筋
	command 跳弾ＳＥのみ -n 4
	command 跳弾ＳＥのみ -n 5	// ×木箱
	command 跳弾ＳＥのみ -n 6	// ×ダンボール
	command 跳弾ＳＥのみ -n 7	// ×ホロ

	// 7 はホロで弾痕なし
	command 弾痕ノーマル -n 0
	command 弾痕ノーマル -n 1
	command 弾痕ノーマル -n 2
	command 弾痕ノーマル -n 3
	command 弾痕ノーマル -n 4
	command 弾痕ノーマル -n 5
	command 弾痕ノーマル -n 6

#elifdef d:STAGE_W42A	//----- w42a

	command 跳弾ノーマル -n 0	// 通常
	command 跳弾ノーマル -n 1	// 鉄板
	command 跳弾ノーマル -n 2	// 鉄板
	command 跳弾ノーマル -n 3	// 鉄筋
	command 跳弾ＳＥのみ -n 4
	command 跳弾ＳＥのみ -n 5
	command 跳弾ＳＥのみ -n 6
	command 跳弾ＳＥのみ -n 7

	command 弾痕ノーマル -n 0
	command 弾痕ノーマル -n 1
	command 弾痕ノーマル -n 2
	command 弾痕ノーマル -n 3
	command 弾痕ノーマル -n 4
	command 弾痕ノーマル -n 5
	command 弾痕ノーマル -n 6
	command 弾痕ノーマル -n 7

#elifdef d:STAGE_W44A	//----- w44a

	command 跳弾ノーマル -n 0	// 通常
	command 跳弾ノーマル -n 1	// 鉄板
	command 跳弾ノーマル -n 2	// 鉄板
	command 跳弾ノーマル -n 3	// 鉄筋
	command 跳弾ＳＥのみ -n 4
	command 跳弾ＳＥのみ -n 5
	command 跳弾ＳＥのみ -n 6
	command 跳弾ＳＥのみ -n 7

	command 弾痕ノーマル -n 0
	command 弾痕ノーマル -n 1
	command 弾痕ノーマル -n 2
	command 弾痕ノーマル -n 3
	command 弾痕ノーマル -n 4
	command 弾痕ノーマル -n 5
	command 弾痕ノーマル -n 6
	command 弾痕ノーマル -n 7

#endif
}

//-----------------------------------------------------------------------------
// プレイヤー設定
proc SS_プレイヤー設定 $:強制モーションファイル $:ＬＯＤ距離 {
#if d:DEBUG_PRINT
	print '*** SS Player Set ***'
#endif

	// プレイヤー
	//   -m モデル名  -a モーション名  -p 出現位置  -d 出現方向  -s 出現時姿勢  -o 強制モーションファイル名  -i SE 変換テーブル ID
	chara プレイヤー d:PLAYER \
		-m rai_def \
		-a raiden \
		-p $i:プレイヤー初期Ｘ位置 $i:プレイヤー初期Ｙ位置 $i:プレイヤー初期Ｚ位置 \
		-d 0, $i:プレイヤー初期方向, 0 \
		-s 0 \
		-o $:強制モーションファイル \
		-i 0

	// 髪の毛
	@ライデン髪の毛設定 d:NORMAL_HAIR

	// ＬＯＤ
	if($:ＬＯＤ距離 == SUBJECT) {			// 主観とビハインド時
	chara ＬＯＤ制御 プレイヤーＬＯＤ \
		-name   d:PLAYER \
		-type   2 \
		-model  d:LOD_KMS_NAME      d:LOD_KMS_NAME \
		-value  d:TRP_STATE_SUBJECT d:TRP_STATE_BEHIND \
		-value2 6000

	} else if($:ＬＯＤ距離 == BEHIND) {		// ビハインド時
		@プレイヤーＬＯＤ設定 d:P_STATE_TYPE d:TRP_STATE_BEHIND

	} else {								// 距離
		@プレイヤーＬＯＤ設定 d:C_DISTANCE_TYPE $:ＬＯＤ距離
	}
}

#include "ene_item.h"

//-----------------------------------------------------------------------------
// 警備兵設定
proc SS_警備兵設定 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ $:ＩＤ {
#if d:DEBUG_PRINT
	print '*** SS Watcher Set ***'
#endif

	// 警備兵
	//   -r 初期ルート  -n 初期ノード  -d 初期守備位置 マップ  -s ステータス  -h 聴力  -i 視力  -l ライフ値  -f 気絶値  -q アイテムプロック 確率  -e 終了proc  -b 個別ＢＧＭ  -v 声優  -a ホールドアップアイテムプロック 確率  -w 気づいた時プロック

	if($:ＩＤ == d:SS_警備兵ＩＤ:Ａ脚屋上兵) {
		#if d:DEBUG_PRINT
			print '*** SS Watcher W12A Set ***'
		#endif
		//----- Ａ脚屋上兵
		chara 警備兵 $:名前 -r $:ルート -n $:ノード -d $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ -s $:ステータス \
			-h $w:敵標準聴力 -i $w:敵潜入視力, $w:敵危険視力, $w:敵回避視力 -l $w:敵標準体力 -f $w:敵標準気絶 \
			-j ドッグタグ_敵兵 \
				[dogtag:plt_w12a_guard_ve_01] \
				[dogtag:plt_w12a_guard_ea_01] \
				[dogtag:plt_w12a_guard_no_01] \
				[dogtag:plt_w12a_guard_ha_01] \
				[dogtag:plt_w12a_guard_vh_01] \
			-q 引きずり１       $b:アイテム率１ 引きずり２       $b:アイテム率２ 引きずり３       $b:アイテム率３ \
			-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３

	} else if($:ＩＤ == d:SS_警備兵ＩＤ:Ａ脚屋上兵_ＤＥＭＯ) {
		#if d:DEBUG_PRINT
			print '*** SS Watcher W12A DEMO Set ***'
		#endif
		//----- Ａ脚屋上兵_ＤＥＭＯ
		chara 警備兵 $:名前 -r $:ルート -n $:ノード -d $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ -s $:ステータス \
			-h 0 -i 0, 0, 0 -l $w:敵標準体力 -f $w:敵標準気絶 \
			-j ドッグタグ_敵兵 \
				[dogtag:plt_w12a_guard_ve_01] \
				[dogtag:plt_w12a_guard_ea_01] \
				[dogtag:plt_w12a_guard_no_01] \
				[dogtag:plt_w12a_guard_ha_01] \
				[dogtag:plt_w12a_guard_vh_01] \
			-q 引きずり１       $b:アイテム率１ 引きずり２       $b:アイテム率２ 引きずり３       $b:アイテム率３ \
			-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３


	} else if($:ＩＤ == d:SS_警備兵ＩＤ:Ａ脚制御室兵) {
		#if d:DEBUG_PRINT
			print '*** SS Watcher W12B1 Set ***'
		#endif
		//----- Ａ脚制御室兵
		chara 警備兵 $:名前 -r $:ルート -n $:ノード -d $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ -s $:ステータス \
			-h $w:敵標準聴力 -i $w:敵潜入視力, $w:敵危険視力, $w:敵回避視力 -l $w:敵標準体力 -f $w:敵標準気絶 \
			-j ドッグタグ_敵兵 \
				[dogtag:plt_w12b_guard_ve_01] \
				[dogtag:plt_w12b_guard_ea_01] \
				[dogtag:plt_w12b_guard_no_01] \
				[dogtag:plt_w12b_guard_ha_01] \
				[dogtag:plt_w12b_guard_vh_01] \
			-q 引きずり１       $b:アイテム率１ 引きずり２       $b:アイテム率２ 引きずり３       $b:アイテム率３ \
			-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３


	} else if($:ＩＤ == d:SS_警備兵ＩＤ:Ａ脚制御室兵_ＤＥＭＯ) {
		#if d:DEBUG_PRINT
			print '*** SS Watcher W12B1 DEMO Set ***'
		#endif
		//----- Ａ脚制御室兵_ＤＥＭＯ
		chara 警備兵 $:名前 -r $:ルート -n $:ノード -d $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ -s $:ステータス \
			-h 0 -i 0, 0, 0 -l $w:敵標準体力 -f $w:敵標準気絶 \
			-j ドッグタグ_敵兵 \
				[dogtag:plt_w12b_guard_ve_01] \
				[dogtag:plt_w12b_guard_ea_01] \
				[dogtag:plt_w12b_guard_no_01] \
				[dogtag:plt_w12b_guard_ha_01] \
				[dogtag:plt_w12b_guard_vh_01] \
			-q 引きずり１       $b:アイテム率１ 引きずり２       $b:アイテム率２ 引きずり３       $b:アイテム率３ \
			-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３


	} else if($:ＩＤ == d:SS_警備兵ＩＤ:Ａ脚ポンプ室兵) {
		#if d:DEBUG_PRINT
			print '*** SS Watcher W12B2 Set ***'
		#endif
		//----- Ａ脚ポンプ室兵
		chara 警備兵 $:名前 -r $:ルート -n $:ノード -d $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ -s $:ステータス \
			-h $w:敵標準聴力 -i $w:敵潜入視力, $w:敵危険視力, $w:敵回避視力 -l $w:敵標準体力 -f $w:敵標準気絶 \
			-j ドッグタグ_敵兵 \
				[dogtag:plt_w12b_guard_ve_02] \
				[dogtag:plt_w12b_guard_ea_02] \
				[dogtag:plt_w12b_guard_no_02] \
				[dogtag:plt_w12b_guard_ha_02] \
				[dogtag:plt_w12b_guard_vh_02] \
			-q 引きずり１       $b:アイテム率１ 引きずり２       $b:アイテム率２ 引きずり３       $b:アイテム率３ \
			-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３


	} else if($:ＩＤ == d:SS_警備兵ＩＤ:Ｂ脚変電室兵) {
		#if d:DEBUG_PRINT
			print '*** SS Watcher W14A1 Set ***'
		#endif
		//----- Ｂ脚変電室兵
		chara 警備兵 $:名前 -r $:ルート -n $:ノード -d $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ -s $:ステータス \
			-h $w:敵標準聴力 -i $w:敵潜入視力, $w:敵危険視力, $w:敵回避視力 -l $w:敵標準体力 -f $w:敵標準気絶 \
			-j ドッグタグ_敵兵 \
				[dogtag:plt_w14a_guard_ve_01] \
				[dogtag:plt_w14a_guard_ea_01] \
				[dogtag:plt_w14a_guard_no_01] \
				[dogtag:plt_w14a_guard_ha_01] \
				[dogtag:plt_w14a_guard_vh_01] \
			-q 引きずり１       $b:アイテム率１ 引きずり２       $b:アイテム率２ 引きずり３       $b:アイテム率３ \
			-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３


	} else if($:ＩＤ == d:SS_警備兵ＩＤ:Ｂ脚廊下兵) {
		#if d:DEBUG_PRINT
			print '*** SS Watcher W14A2 Set ***'
		#endif
		//----- Ｂ脚廊下兵
		chara 警備兵 $:名前 -r $:ルート -n $:ノード -d $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ -s $:ステータス \
			-h $w:敵標準聴力 -i $w:敵潜入視力, $w:敵危険視力, $w:敵回避視力 -l $w:敵標準体力 -f $w:敵標準気絶 \
			-j ドッグタグ_敵兵 \
				[dogtag:plt_w14a_guard_ve_02] \
				[dogtag:plt_w14a_guard_ea_02] \
				[dogtag:plt_w14a_guard_no_02] \
				[dogtag:plt_w14a_guard_ha_02] \
				[dogtag:plt_w14a_guard_vh_02] \
			-q 引きずり１       $b:アイテム率１ 引きずり２       $b:アイテム率２ 引きずり３       $b:アイテム率３ \
			-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３


	} else if($:ＩＤ == d:SS_警備兵ＩＤ:Ｃ脚食堂兵) {
		#if d:DEBUG_PRINT
			print '*** SS Watcher W16A1 Set ***'
		#endif
		//----- Ｃ脚食堂兵
		chara 警備兵 $:名前 -r $:ルート -n $:ノード -d $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ -s $:ステータス \
			-h $w:敵標準聴力 -i $w:敵潜入視力, $w:敵危険視力, $w:敵回避視力 -l $w:敵標準体力 -f $w:敵標準気絶 \
			-j ドッグタグ_敵兵 \
				[dogtag:plt_w16a_guard_ve_01] \
				[dogtag:plt_w16a_guard_ea_01] \
				[dogtag:plt_w16a_guard_no_01] \
				[dogtag:plt_w16a_guard_ha_01] \
				[dogtag:plt_w16a_guard_vh_01] \
			-q 引きずり１       $b:アイテム率１ 引きずり２       $b:アイテム率２ 引きずり３       $b:アイテム率３ \
			-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３


	} else if($:ＩＤ == d:SS_警備兵ＩＤ:Ｃ脚トイレ兵) {
		#if d:DEBUG_PRINT
			print '*** SS Watcher W16A2 Set ***'
		#endif
		//----- Ｃ脚トイレ兵
		chara 警備兵 $:名前 -r $:ルート -n $:ノード -d $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ -s $:ステータス \
			-h $w:敵標準聴力 -i $w:敵潜入視力, $w:敵危険視力, $w:敵回避視力 -l $w:敵標準体力 -f $w:敵標準気絶 \
			-j ドッグタグ_敵兵 \
				[dogtag:plt_w16a_guard_ve_02] \
				[dogtag:plt_w16a_guard_ea_02] \
				[dogtag:plt_w16a_guard_no_02] \
				[dogtag:plt_w16a_guard_ha_02] \
				[dogtag:plt_w16a_guard_vh_02] \
			-q 引きずり１       $b:アイテム率１ 引きずり２       $b:アイテム率２ 引きずり３       $b:アイテム率３ \
			-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３


	} else if($:ＩＤ == d:SS_警備兵ＩＤ:Ｅ脚２Ｆ兵) {
		#if d:DEBUG_PRINT
			print '*** SS Watcher W20B1 Set ***'
		#endif
		//----- Ｅ脚２Ｆ兵
		chara 警備兵 $:名前 -r $:ルート -n $:ノード -d $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ -s $:ステータス \
			-h $w:敵標準聴力 -i $w:敵潜入視力, $w:敵危険視力, $w:敵回避視力 -l $w:敵標準体力 -f $w:敵標準気絶 \
			-j ドッグタグ_敵兵 \
				[dogtag:plt_w20b_guard_ve_01] \
				[dogtag:plt_w20b_guard_ea_01] \
				[dogtag:plt_w20b_guard_no_01] \
				[dogtag:plt_w20b_guard_ha_01] \
				[dogtag:plt_w20b_guard_vh_01] \
			-q 引きずり１       $b:アイテム率１ 引きずり２       $b:アイテム率２ 引きずり３       $b:アイテム率３ \
			-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３


	} else if($:ＩＤ == d:SS_警備兵ＩＤ:Ｅ脚ヘリポート兵) {
		#if d:DEBUG_PRINT
			print '*** SS Watcher W20B2 Set ***'
		#endif
		//----- Ｅ脚ヘリポート兵
		chara 警備兵 $:名前 -r $:ルート -n $:ノード -d $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ -s $:ステータス \
			-h $w:敵標準聴力 -i $w:敵潜入視力, $w:敵危険視力, $w:敵回避視力 -l $w:敵標準体力 -f $w:敵標準気絶 \
			-j ドッグタグ_敵兵 \
				[dogtag:plt_w20b_guard_ve_02] \
				[dogtag:plt_w20b_guard_ea_02] \
				[dogtag:plt_w20b_guard_no_02] \
				[dogtag:plt_w20b_guard_ha_02] \
				[dogtag:plt_w20b_guard_vh_02] \
			-q 引きずり１       $b:アイテム率１ 引きずり２       $b:アイテム率２ 引きずり３       $b:アイテム率３ \
			-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３


	} else if($:ＩＤ == d:SS_警備兵ＩＤ:Ｅ脚ヘリポート応援兵) {
		#if d:DEBUG_PRINT
			print '*** SS Watcher W20B3 Set ***'
		#endif
		//----- Ｅ脚ヘリポート応援兵
		chara 警備兵 $:名前 -r $:ルート -n $:ノード -d $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ -s $:ステータス \
			-h $w:敵標準聴力 -i $w:敵潜入視力, $w:敵危険視力, $w:敵回避視力 -l $w:敵標準体力 -f $w:敵標準気絶 \
			-j ドッグタグ_敵兵 \
				[dogtag:dummy] \
				[dogtag:dummy] \
				[dogtag:plt_w20b_guard_no_03] \
				[dogtag:plt_w20b_guard_ha_03] \
				[dogtag:plt_w20b_guard_vh_03] \
			-q 引きずり１       $b:アイテム率１ 引きずり２       $b:アイテム率２ 引きずり３       $b:アイテム率３ \
			-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３
	}
}

//-----------------------------------------------------------------------------
// サポート兵設定
proc SS_サポート兵設定 $:名前 $:ルート $:ステータス $:相手 {
#if d:DEBUG_PRINT
	print '*** SS Support Set ***'
#endif

	chara アタッカー $:名前 \
		-r $:ルート -s (($:ステータス) | 0x40000000) -b $:相手 \
		-e 4 \
		-h $w:敵標準聴力 -i $w:敵潜入視力, $w:敵危険視力, $w:敵回避視力 \
		-l $w:敵標準体力 -f $w:敵標準気絶 \
		-q レーション_敵兵 20 弾薬_Ｍ９_敵兵 20 ジアゼパム_敵兵 10 弾薬_ソコム_敵兵 10
}

//-----------------------------------------------------------------------------
// アタッカー設定
proc SS_アタッカー設定 $:名前 $:ルート $:ステータス $:装備 {
#if d:DEBUG_PRINT
	print '*** SS Attacker Set ***'
#endif

	// アタッカー
	//   -r 初期ルート  -e 装備タイプ  -s ステータス  -i 視力  -l ライフ値  -f 気絶値  -d 守備位置 マップ  -o 待避位置  -q プロック 確率  -c 探索ルート  -v 声優
	chara アタッカー $:名前 \
		-r $:ルート -s $:ステータス -e $:装備 \
		-h $w:敵標準聴力 -i $w:敵潜入視力, $w:敵危険視力, $w:敵回避視力 \
		-l $w:敵標準体力 -f $w:敵標準気絶
}

//-----------------------------------------------------------------------------
// 天狗兵Ａ設定
proc SS_天狗兵Ａ設定 $:名前 $:ルート $:敵潜入視力 $:敵危険視力 {
#if d:DEBUG_PRINT
	print '*** SS Tengu A Set ***'
#endif

	chara 天狗兵Ａ $:名前 \
		-r $:ルート  \
		-i $:敵潜入視力,$:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s 0
}

//-----------------------------------------------------------------------------
// 天狗兵Ａ設定
proc SS_天狗兵Ａ設定_守備 $:名前 $:ルート $:敵潜入視力 $:敵危険視力 $:守備Ｘ $:守備Ｙ $:守備Ｚ {
#if d:DEBUG_PRINT
	print '*** SS Tengu A Set (Guard) ***'
#endif

	chara 天狗兵Ａ $:名前 \
		-r $:ルート  \
		-d $:守備Ｘ $:守備Ｙ $:守備Ｚ \
		-i $:敵潜入視力,$:敵危険視力,$w:敵回避視力 \
		-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s 0
}

//-----------------------------------------------------------------------------
// 天狗兵Ｂ設定
proc SS_天狗兵Ｂ設定 $:名前 $:ルート $:天狗兵視力 $:天狗兵体力 {
#if d:DEBUG_PRINT
	print '*** SS Tengu B Set ***'
#endif

	chara 天狗兵Ｂ $:名前 \
		-r $:ルート  \
		-i $:天狗兵視力,$:天狗兵視力,$:天狗兵視力 \
		-l $:天狗兵体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
		-s 0 -t 1
}

//-----------------------------------------------------------------------------
// ＢＧＭ設定
proc SS_ＢＧＭ設定 {
#if d:DEBUG_PRINT
	print '*** SS BGM Set ***'
#endif

	// ※※※ w20b では使用していない ※※※

	if(($s:エリア == w12a) || ($s:エリア == w12c)) {
		chara ＢＧＭマネージャー ＢＧＭマネ -s 0x00000001	// 潜入モードの曲を他のキャラが管理
		chara ＢＧＭ風音 ＢＧＭ風音 -s d:BGM_WINDNOISE_WEATHER_FAIN
	} else {
		chara ＢＧＭマネージャー ＢＧＭマネ
	}
}

//-----------------------------------------------------------------------------
// 北ビヨンド登録
proc SS_北ビヨンド登録 $:トラップ名 {
#if d:DEBUG_PRINT
	print '*** SS North Beyond Trap ***'
#endif

	trap $:トラップ名 d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON,   d:ACTION_BUTTON \
		-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-dir    2048,256           0,256 \
		-exec {
			@SS_CB_ビヨンド $1 0 $3 $4 $5 $6
		}
	trap $:トラップ名 d:PLAYER \
		-mask いる \
		-and \
		-state d:TRP_STATE_FORCE, d:TRP_STATE_FORCE \
		-dir   2048,256           0,256 \	//方向指定
		-camera \							//カメラトラップであることを明示
		-exec {
			@SS_CB_ビヨンド $1 1 $3 $4 $5 $6
		}
	trap $:トラップ名 d:PLAYER \
		-mask ？ \
		-state d:TRP_STATE_ELUDE \
		-dir   0 256 \	//方向指定
		-camera \		//カメラトラップであることを明示
		-exec {
			@SS_CB_ビヨンド $1 2 $3 $4 $5 $6
		}
}

//-----------------------------------------------------------------------------
// 南ビヨンド登録
proc SS_南ビヨンド登録 $:トラップ名 {
#if d:DEBUG_PRINT
	print '*** SS South Beyond Trap ***'
#endif

	trap $:トラップ名 d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON,   d:ACTION_BUTTON \
		-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-dir    0,256              2048,256 \
		-exec {
			@SS_CB_ビヨンド $1 0 $3 $4 $5 $6
		}
	trap $:トラップ名 d:PLAYER \
		-mask いる \
		-and \
		-state d:TRP_STATE_FORCE, d:TRP_STATE_FORCE \
		-dir   0,256              2048,256 \	//方向指定
		-camera \								//カメラトラップであることを明示
		-exec {
			@SS_CB_ビヨンド $1 1 $3 $4 $5 $6
		}
	trap $:トラップ名 d:PLAYER \
		-mask ？ \
		-state d:TRP_STATE_ELUDE \
		-dir   2048 256 \	//方向指定
		-camera \			//カメラトラップであることを明示
		-exec {
			@SS_CB_ビヨンド $1 2 $3 $4 $5 $6
		}
}

//-----------------------------------------------------------------------------
// 東ビヨンド登録
proc SS_東ビヨンド登録 $:トラップ名 {
#if d:DEBUG_PRINT
	print '*** SS East Beyond Trap ***'
#endif

	trap $:トラップ名 d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON,   d:ACTION_BUTTON \
		-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-dir    1024,256           3072,256 \
		-exec {
			@SS_CB_ビヨンド $1 0 $3 $4 $5 $6
		}
	trap $:トラップ名 d:PLAYER \
		-mask いる \
		-and \
		-state d:TRP_STATE_FORCE, d:TRP_STATE_FORCE \
		-dir   1024,256           3072,256 \	//方向指定
		-camera \								//カメラトラップであることを明示
		-exec {
			@SS_CB_ビヨンド $1 1 $3 $4 $5 $6
		}
	trap $:トラップ名 d:PLAYER \
		-mask ？ \
		-state d:TRP_STATE_ELUDE \
		-dir   3072 256 \	//方向指定
		-camera \			//カメラトラップであることを明示
		-exec {
			@SS_CB_ビヨンド $1 2 $3 $4 $5 $6
		}
}

//-----------------------------------------------------------------------------
// 西ビヨンド登録
proc SS_西ビヨンド登録 $:トラップ名 {
#if d:DEBUG_PRINT
	print '*** SS West Beyond Trap ***'
#endif

	trap $:トラップ名 d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON,   d:ACTION_BUTTON \
		-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-dir    3072,256           1024,256 \
		-exec {
			@SS_CB_ビヨンド $1 0 $3 $4 $5 $6
		}
	trap $:トラップ名 d:PLAYER \
		-mask いる \
		-and \
		-state d:TRP_STATE_FORCE, d:TRP_STATE_FORCE \
		-dir   3072,256           1024,256 \	//方向指定
		-camera \								//カメラトラップであることを明示
		-exec {
			@SS_CB_ビヨンド $1 1 $3 $4 $5 $6
		}
	trap $:トラップ名 d:PLAYER \
		-mask ？ \
		-state d:TRP_STATE_ELUDE \
		-dir   1024 256 \	//方向指定
		-camera \			//カメラトラップであることを明示
		-exec {
			@SS_CB_ビヨンド $1 2 $3 $4 $5 $6
		}
}

//-----------------------------------------------------------------------------
// 北ロッカー登録
proc SS_北ロッカー登録 $:ロッカー名 $:トラップ名 $:番号 $:モデル名 $:Ｘ座標 $:Ｙ座標 $:Ｚ座標 $:ステータス $:壁音番号 $:倒れ時床音番号 $:フラグ $:倒れシフトＸ $:倒れシフトＹ $:倒れシフトＺ $:倒れ回転Ｙ $:影ＩＤ {
#if d:DEBUG_PRINT
	print '*** SS North Locker ***'
#endif

	eval($i:SS_ロッカー状態 = `@SS_CB_ロッカーチェック $:番号`)

	// ロッカー
	//   -kms モデル -body 箱モデル(省略すると扉のみ) -position 位置 -rotate 回転 
	//   -down_rot 倒れたときの角度 -doWn_shift 倒れたときのシフト量 -state 初期状態 
	//   -flag フラグ -light スポットライト名 -exec { $:トリガ $:実行キャラ $:行動 } 
	//   -prOc 呼び出しプロック($:トリガ $:実行キャラ $:行動) -seNo 壁音番号 
	//   -gravure キス音発生範囲最小 キス音発生範囲最大 $v:胸範囲最小 $v:胸範囲最大 $v:秘所範囲最小 $v:秘所範囲最大
	if($:ステータス & d:LOCKER_STATE_POSTER) {
		chara ロッカー  $:ロッカー名 \
			-kms        $:モデル名 \
			-position   ($:Ｘ座標 -50),$:Ｙ座標,$:Ｚ座標 \
			-rotate     0,0,0 \
			-state      $:ステータス \
			-seno       $:壁音番号 \			// 壁音番号
			-sefloor    $:倒れ時床音番号 \		// 倒れ時床音番号
			-flag       $:フラグ \
			-down_shift $:倒れシフトＸ $:倒れシフトＹ $:倒れシフトＺ \
			-down_rot   0 $:倒れ回転Ｙ 0 \
			-load_state $i:SS_ロッカー状態 \
			-light      $:影ＩＤ \
#ifdef d:STAGE_W12B						//----- w12b
			-gravure -250, 1000,0 -670,1600,0 \
			            0,    0,0   -1,   1,0 \
			            0,    0,0   -1,   1,0 \
#elifdef d:STAGE_W14A					//----- w14a
			-gravure -250, 1120,0 -500,1620,0 \
			            0,    0,0   -1,   1,0 \
			            0,    0,0   -1,   1,0 \
#elifdef d:STAGE_W16A || d:STAGE_W16B	//----- w16a, w16b
			-gravure -200, 850,0 -700,1550,0 \
			            0,   0,0   -1,   1,0 \
			            0,   0,0   -1,   1,0 \
#else
			-gravure -250, 700,0 -750,1700,0 \
			            0,   0,0   -1,   1,0 \
			            0,   0,0   -1,   1,0 \
#endif
			-exec {
				@SS_CB_ロッカー状態保存 $1 $2 $3 $4
			}
	} else {
		chara ロッカー  $:ロッカー名 \
			-kms        $:モデル名 \
			-position   ($:Ｘ座標 -50),$:Ｙ座標,$:Ｚ座標 \
			-rotate     0,0,0 \
			-state      $:ステータス \
			-seno       $:壁音番号 \			// 壁音番号
			-sefloor    $:倒れ時床音番号 \		// 倒れ時床音番号
			-flag       $:フラグ \
			-down_shift $:倒れシフトＸ $:倒れシフトＹ $:倒れシフトＺ \
			-down_rot   0 $:倒れ回転Ｙ 0 \
			-load_state $i:SS_ロッカー状態 \
			-light      $:影ＩＤ \
			-exec {
				@SS_CB_ロッカー状態保存 $1 $2 $3 $4
			}
	}
	trap $:トラップ名 d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON,   -1 \
		-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-dir    2048,512           0,512 \
		-exec {
			@SS_CB_ロッカー実行 $1 0 $3 $4 $5 $6
		}
	trap $:トラップ名 d:PLAYER \
		-mask ？ \
		-state d:TRP_STATE_FORCE \
		-camera \			//カメラトラップであることを明示
		-exec {
			@SS_CB_ロッカー実行 $1 1 $3 $4 $5 $6
		}
}

//-----------------------------------------------------------------------------
// 南ロッカー登録
proc SS_南ロッカー登録 $:ロッカー名 $:トラップ名 $:番号 $:モデル名 $:Ｘ座標 $:Ｙ座標 $:Ｚ座標 $:ステータス $:壁音番号 $:倒れ時床音番号 $:フラグ $:倒れシフトＸ $:倒れシフトＹ $:倒れシフトＺ $:倒れ回転Ｙ $:影ＩＤ {
#if d:DEBUG_PRINT
	print '*** SS South Locker ***'
#endif

	eval($i:SS_ロッカー状態 = `@SS_CB_ロッカーチェック $:番号`)

	// ロッカー
	//   -kms モデル -body 箱モデル(省略すると扉のみ) -position 位置 -rotate 回転 
	//   -down_rot 倒れたときの角度 -doWn_shift 倒れたときのシフト量 -state 初期状態 
	//   -flag フラグ -light スポットライト名 -exec { $:トリガ $:実行キャラ $:行動 } 
	//   -prOc 呼び出しプロック($:トリガ $:実行キャラ $:行動) -seNo 壁音番号 
	//   -gravure キス音発生範囲最小 キス音発生範囲最大 $v:胸範囲最小 $v:胸範囲最大 $v:秘所範囲最小 $v:秘所範囲最大
	if($:ステータス & d:LOCKER_STATE_POSTER) {
		chara ロッカー  $:ロッカー名 \
			-kms        $:モデル名 \
			-position   ($:Ｘ座標 +50),$:Ｙ座標,$:Ｚ座標 \
			-rotate     0,2048,0 \
			-state      $:ステータス \
			-seno       $:壁音番号 \			// 壁音番号
			-sefloor    $:倒れ時床音番号 \		// 倒れ時床音番号
			-flag       $:フラグ \
			-down_shift $:倒れシフトＸ $:倒れシフトＹ $:倒れシフトＺ \
			-down_rot   0 $:倒れ回転Ｙ 0 \
			-load_state $i:SS_ロッカー状態 \
			-light      $:影ＩＤ \
			-gravure -250, 700,0 -750,1700,0 \
			            0,   0,0   -1,   1,0 \
			            0,   0,0   -1,   1,0 \
			-exec {
				@SS_CB_ロッカー状態保存 $1 $2 $3 $4
			}
	} else {
		chara ロッカー  $:ロッカー名 \
			-kms        $:モデル名 \
			-position   ($:Ｘ座標 +50),$:Ｙ座標,$:Ｚ座標 \
			-rotate     0,2048,0 \
			-state      $:ステータス \
			-seno       $:壁音番号 \			// 壁音番号
			-sefloor    $:倒れ時床音番号 \		// 倒れ時床音番号
			-flag       $:フラグ \
			-down_shift $:倒れシフトＸ $:倒れシフトＹ $:倒れシフトＺ \
			-down_rot   0 $:倒れ回転Ｙ 0 \
			-load_state $i:SS_ロッカー状態 \
			-light      $:影ＩＤ \
			-exec {
				@SS_CB_ロッカー状態保存 $1 $2 $3 $4
			}
	}
	trap $:トラップ名 d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON,   -1 \
		-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-dir    0,512              2048,512 \
		-exec {
			@SS_CB_ロッカー実行 $1 0 $3 $4 $5 $6
		}
	trap $:トラップ名 d:PLAYER \
		-mask ？ \
		-state d:TRP_STATE_FORCE \
		-camera \			//カメラトラップであることを明示
		-exec {
			@SS_CB_ロッカー実行 $1 1 $3 $4 $5 $6
		}
}

//-----------------------------------------------------------------------------
// 東ロッカー登録
proc SS_東ロッカー登録 $:ロッカー名 $:トラップ名 $:番号 $:モデル名 $:Ｘ座標 $:Ｙ座標 $:Ｚ座標 $:ステータス $:壁音番号 $:倒れ時床音番号 $:フラグ $:倒れシフトＸ $:倒れシフトＹ $:倒れシフトＺ $:倒れ回転Ｙ $:影ＩＤ {
#if d:DEBUG_PRINT
	print '*** SS East Locker ***'
#endif

	eval($i:SS_ロッカー状態 = `@SS_CB_ロッカーチェック $:番号`)

	// ロッカー
	//   -kms モデル -body 箱モデル(省略すると扉のみ) -position 位置 -rotate 回転 
	//   -down_rot 倒れたときの角度 -doWn_shift 倒れたときのシフト量 -state 初期状態 
	//   -flag フラグ -light スポットライト名 -exec { $:トリガ $:実行キャラ $:行動 } 
	//   -prOc 呼び出しプロック($:トリガ $:実行キャラ $:行動) -seNo 壁音番号 
	//   -gravure キス音発生範囲最小 キス音発生範囲最大 $v:胸範囲最小 $v:胸範囲最大 $v:秘所範囲最小 $v:秘所範囲最大
	if($:ステータス & d:LOCKER_STATE_POSTER) {
		chara ロッカー  $:ロッカー名 \
			-kms        $:モデル名 \
			-position   $:Ｘ座標,$:Ｙ座標,($:Ｚ座標 -50) \
			-rotate     0,3072,0 \
			-state      $:ステータス \
			-seno       $:壁音番号 \			// 壁音番号
			-sefloor    $:倒れ時床音番号 \		// 倒れ時床音番号
			-flag       $:フラグ \
			-down_shift $:倒れシフトＸ $:倒れシフトＹ $:倒れシフトＺ \
			-down_rot   0 $:倒れ回転Ｙ 0 \
			-load_state $i:SS_ロッカー状態 \
			-light      $:影ＩＤ \
			-gravure -250, 700,0 -750,1700,0 \
			            0,   0,0   -1,   1,0 \
			            0,   0,0   -1,   1,0 \
			-exec {
				@SS_CB_ロッカー状態保存 $1 $2 $3 $4
			}
	} else {
		chara ロッカー  $:ロッカー名 \
			-kms        $:モデル名 \
			-position   $:Ｘ座標,$:Ｙ座標,($:Ｚ座標 -50) \
			-rotate     0,3072,0 \
			-state      $:ステータス \
			-seno       $:壁音番号 \			// 壁音番号
			-sefloor    $:倒れ時床音番号 \		// 倒れ時床音番号
			-flag       $:フラグ \
			-down_shift $:倒れシフトＸ $:倒れシフトＹ $:倒れシフトＺ \
			-down_rot   0 $:倒れ回転Ｙ 0 \
			-load_state $i:SS_ロッカー状態 \
			-light      $:影ＩＤ \
			-exec {
				@SS_CB_ロッカー状態保存 $1 $2 $3 $4
			}
	}
	trap $:トラップ名 d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON,   -1 \
		-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-dir    1024,512           3072,512 \
		-exec {
			@SS_CB_ロッカー実行 $1 0 $3 $4 $5 $6
		}
	trap $:トラップ名 d:PLAYER \
		-mask ？ \
		-state d:TRP_STATE_FORCE \
		-camera \			//カメラトラップであることを明示
		-exec {
			@SS_CB_ロッカー実行 $1 1 $3 $4 $5 $6
		}
}

//-----------------------------------------------------------------------------
// 西ロッカー登録
proc SS_西ロッカー登録 $:ロッカー名 $:トラップ名 $:番号 $:モデル名 $:Ｘ座標 $:Ｙ座標 $:Ｚ座標 $:ステータス $:壁音番号 $:倒れ時床音番号 $:フラグ $:倒れシフトＸ $:倒れシフトＹ $:倒れシフトＺ $:倒れ回転Ｙ $:影ＩＤ {
#if d:DEBUG_PRINT
	print '*** SS West Locker ***'
#endif

	eval($i:SS_ロッカー状態 = `@SS_CB_ロッカーチェック $:番号`)

	// ロッカー
	//   -kms モデル -body 箱モデル(省略すると扉のみ) -position 位置 -rotate 回転 
	//   -down_rot 倒れたときの角度 -doWn_shift 倒れたときのシフト量 -state 初期状態 
	//   -flag フラグ -light スポットライト名 -exec { $:トリガ $:実行キャラ $:行動 } 
	//   -prOc 呼び出しプロック($:トリガ $:実行キャラ $:行動) -seNo 壁音番号 
	//   -gravure キス音発生範囲最小 キス音発生範囲最大 $v:胸範囲最小 $v:胸範囲最大 $v:秘所範囲最小 $v:秘所範囲最大
	if($:ステータス & d:LOCKER_STATE_POSTER) {
		chara ロッカー  $:ロッカー名 \
			-kms        $:モデル名 \
			-position   $:Ｘ座標,$:Ｙ座標,($:Ｚ座標 +50) \
			-rotate     0,1024,0 \
			-state      $:ステータス \
			-seno       $:壁音番号 \			// 壁音番号
			-sefloor    $:倒れ時床音番号 \		// 倒れ時床音番号
			-flag       $:フラグ \
			-down_shift $:倒れシフトＸ $:倒れシフトＹ $:倒れシフトＺ \
			-down_rot   0 $:倒れ回転Ｙ 0 \
			-load_state $i:SS_ロッカー状態 \
			-light      $:影ＩＤ \
			-gravure -250, 700,0 -750,1700,0 \
			            0,   0,0   -1,   1,0 \
			            0,   0,0   -1,   1,0 \
			-exec {
				@SS_CB_ロッカー状態保存 $1 $2 $3 $4
			}
	} else {
		chara ロッカー  $:ロッカー名 \
			-kms        $:モデル名 \
			-position   $:Ｘ座標,$:Ｙ座標,($:Ｚ座標 +50) \
			-rotate     0,1024,0 \
			-state      $:ステータス \
			-seno       $:壁音番号 \			// 壁音番号
			-sefloor    $:倒れ時床音番号 \		// 倒れ時床音番号
			-flag       $:フラグ \
			-down_shift $:倒れシフトＸ $:倒れシフトＹ $:倒れシフトＺ \
			-down_rot   0 $:倒れ回転Ｙ 0 \
			-load_state $i:SS_ロッカー状態 \
			-light      $:影ＩＤ \
			-exec {
				@SS_CB_ロッカー状態保存 $1 $2 $3 $4
			}
	}
	trap $:トラップ名 d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON,   -1 \
		-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-dir    3072,512           1024,512 \
		-exec {
			@SS_CB_ロッカー実行 $1 0 $3 $4 $5 $6
		}
	trap $:トラップ名 d:PLAYER \
		-mask ？ \
		-state d:TRP_STATE_FORCE \
		-camera \			//カメラトラップであることを明示
		-exec {
			@SS_CB_ロッカー実行 $1 1 $3 $4 $5 $6
		}
}

//-----------------------------------------------------------------------------
// 爆弾解体設定
proc SS_爆弾解体設定 $:爆弾名 $:処理状態フラグ $:Ｘ座標 $:Ｙ座標 $:Ｚ座標 $:Ｘ回転 $:Ｙ回転 $:Ｚ回転 {
#if d:DEBUG_PRINT
	print '*** SS Bomb Set ***'
#endif

	// 解体Ｃ４ -p 位置 -r 回転 -t タイプ (０／１：臭いあり／なし) -s 初期状態 (０／１：通常／凍結) -e つける敵の名前 関節番号 -prOc プロック
	// 爆弾検知領域 -t トラップ名 -rgb red green blue
	if(($w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了) && ($:処理状態フラグ == 0)) {
		chara 解体Ｃ４ $:爆弾名 \
			-position $:Ｘ座標 $:Ｙ座標 $:Ｚ座標 \
			-rotate $:Ｘ回転 $:Ｙ回転 $:Ｚ回転 \
			-type  0 \
			-state 0 \
			-proc 爆弾解体後処理

		chara 爆弾検知領域 Bomb_Type_A -t bm000

	} else {
		chara 解体Ｃ４ $:爆弾名 \
			-position $:Ｘ座標 $:Ｙ座標 $:Ｚ座標 \
			-rotate $:Ｘ回転 $:Ｙ回転 $:Ｚ回転 \
			-type  0 \
			-state 1
	}
}

proc SS_爆弾解体設定_CB使用 $:爆弾名 $:処理状態フラグ $:Ｘ座標 $:Ｙ座標 $:Ｚ座標 $:Ｘ回転 $:Ｙ回転 $:Ｚ回転 {
#if d:DEBUG_PRINT
	print '*** SS Bomb Set use CB ***'
#endif

	// 解体Ｃ４ -p 位置 -r 回転 -t タイプ (０／１：臭いあり／なし) -s 初期状態 (０／１：通常／凍結) -e つける敵の名前 関節番号 -prOc プロック
	// 爆弾検知領域 -t トラップ名 -rgb red green blue
	if(($w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了) && ($:処理状態フラグ == 0)) {
		chara 解体Ｃ４ $:爆弾名 \
			-position $:Ｘ座標 $:Ｙ座標 $:Ｚ座標 \
			-rotate $:Ｘ回転 $:Ｙ回転 $:Ｚ回転 \
			-type  0 \
			-state 0 \
			-proc SS_CB_爆弾解体後処理

		chara 爆弾検知領域 Bomb_Type_A -t bm000

	} else {
		chara 解体Ｃ４ $:爆弾名 \
			-position $:Ｘ座標 $:Ｙ座標 $:Ｚ座標 \
			-rotate $:Ｘ回転 $:Ｙ回転 $:Ｚ回転 \
			-type  0 \
			-state 1
	}
}

//-----------------------------------------------------------------------------
// フォグ_キャラ用ライト設定
proc SS_フォグ_キャラ用ライト設定 {
#if d:DEBUG_PRINT
	print '*** SS Fog & Chara Light Set ***'
#endif

#ifdef d:STAGE_W12A	//----- w12a

	chara フォグ 霧       -c 194 199 163 -n -20000 -f 175000
	chara 環境光 環境光源 -c  33  53  50
	chara 平行光 平行光源 -c 201 169 103 -p 1930 -1500 4350		// 太陽と連動

#elifdef d:STAGE_W12C	//----- w12c

	chara フォグ 霧       -c 220 237 198 -n -15000 -f 175000
	chara 環境光 環境光源 -c  55  63  38
	chara 平行光 平行光源 -c 163 164 150 -p -1600 -3800 2800	// 太陽と連動

#elifdef d:STAGE_W12B	//----- w12b

//	chara フォグ 霧 -c 18 25 21 -n -2000 -f 28600
//	@明るさ可変機能 18 25 21 -2000 28600 ev004 5000

	chara フォグコントローラ 霧管理 -n 2 -d \
		25 39 34 -2000 28600 28600 28600 \	// デフォルト
		 0  0  0 -2000 14000 14000 14000	// 黒
	// デフォルトの明るさにする
	mesg フォグコントローラ 霧管理 セット 0 1
	// ハレーション機能
	trap ev004 d:PLAYER \
		-mask  ？ \
		-exec {
			if($3 == 出る) {
				chara delay 霧管理ディレイ \
					-t 40 \
					-e {
						// 一旦真っ暗
						mesg フォグコントローラ 霧管理 セット 1 12
					}
				chara delay 霧管理ディレイ２ \
					-t 75 \
					-e {
						// 徐々に戻っていく
						mesg フォグコントローラ 霧管理 セット 0 85
					}
			}
		}

	chara 環境光 環境光源 -c  24  43  38
	chara 平行光 平行光源 -c 106 107  91 -p -2784 -2784 -3082

#elifdef d:STAGE_W14A	//----- w14a

	chara フォグ 霧       -c  16  38  41 -n -1000 -f 38000
	chara 環境光 環境光源 -c  25  39  34
	chara 平行光 平行光源 -c 113 113  97 -p -3800 -2800 -1400

#elifdef d:STAGE_W16A || d:STAGE_W16B	//----- w16a, w16b

	chara フォグ 霧       -c 50 61 42 -n -1000 -f 32000
	chara 環境光 環境光源 -c 69 71 52
	chara 平行光 平行光源 -c 65 80 60 -p -240 280 -5000

#elifdef d:STAGE_W20B || d:STAGE_W20C || d:STAGE_W20D	//----- w20b, w20c, w20d

	chara フォグ 霧       -c 220 237 198 -n -15000 -f 265000
	chara 環境光 環境光源 -c  40  58  46
	chara 平行光 平行光源 -c 163 164 150 -p 800 -4400 2100	// 太陽と連動

#elifdef d:STAGE_W42A	//----- w42a

	chara フォグ 霧       -c  3 18 11 -n -4000 -f 35000 -z
	chara 環境光 環境光源 -c 52 88 72
	chara 平行光 平行光源 -c 58 75 95 -p -863 -2329 -4338

#elifdef d:STAGE_W44A	//----- w44a

	chara フォグ 霧       -c  0 16  8 -n -4000 -f 35000 -z
	chara 環境光 環境光源 -c 52 88 72
	chara 平行光 平行光源 -c 58 75 95 -p -863 -2329 -4338

#endif
}

//-----------------------------------------------------------------------------
// フェードイン
proc SS_フェードイン {
#if d:DEBUG_PRINT
	print '*** SS Fade In ***'
#endif

	chara フェードインアウト フェードイン \
		-i 0,0,0 128 \
		-c 0,0,0 0 \
		-t d:FADEIN_TIME \
		-p SS_CB_フェードイン後
}

//-----------------------------------------------------------------------------
// フェードアウト
proc SS_フェードアウト {
#if d:DEBUG_PRINT
	print '*** SS Fade Out ***'
#endif

	command パッド操作 -release

	chara フェードインアウト フェードアウト \
		-c 0,0,0 128 \
		-t d:FADEOUT_TIME \
		-p SS_CB_フェードアウト後
}

//-----------------------------------------------------------------------------
// プットオブジェ
proc SS_プットオブジェ $:名前 $:モデル {
#if d:DEBUG_PRINT
	print '*** SS Put Obj ***'
#endif

	chara プットオブジェ $:名前 \
		-m $:モデル \
		-r 0,0,0 \
		-p 0,0,0 \
		-s 100,100,100 \
		-l d:LT2_NAME
}

//-----------------------------------------------------------------------------
// 空中文字
proc SS_空中文字 $:ＯＸ $:ＯＹ $:ＯＺ $:半径 $:Ｙシフト $:絵幅 $:絵高さ $:ＴＲＩ名 $:枠テクスチャ名 $:文字テクスチャ名 $:文字表示枠Ｙ１ $:文字表示枠Ｙ２ $:初期方向 $:１スクロール秒 $:スクロール１ $:スクロール２ {
#if d:DEBUG_PRINT
	print '*** SS BLINK TEXTURE ***'
#endif

	chara 空中文字 空中文字 \
		-O        $:ＯＸ $:ＯＹ $:ＯＺ \						// 起点
		-r        $:半径 \										// 半径
		-posy     $:Ｙシフト \									// Ｙシフト
		-width    $:絵幅 \										// イメージ幅
		-height   $:絵高さ \									// イメージ高さ
		-tex      $:ＴＲＩ名 \									// tri 名
			      $:枠テクスチャ名 \							// 枠テクスチャ
			      $:文字テクスチャ名 \							// 文字テクスチャ
		-chr      0 $:文字表示枠Ｙ１ 1000 $:文字表示枠Ｙ２ \	// スクロール枠(縦1000 横1000 として)
		-spin     $:初期方向 0 \								// 軸回転(初期  回転)
		-uvanim   (300 * $:１スクロール秒) \					// スクロール時間(300ベース)
			      0 1000 0              $:スクロール２ \		// スクロール開始(Ｘ  幅  Ｚ  高さ)
			      0 1000 $:スクロール１ $:スクロール２ \		// スクロール終了(Ｘ  幅  Ｚ  高さ)
		-plate													// 円筒ではなく、円筒に対する接面として作成
}

//-----------------------------------------------------------------------------
// 進入禁止エリア設定
proc SS_進入禁止エリア設定 $:壁の名前 $:床の高さ $:壁の高さ $:頂点１Ｘ $:頂点１Ｚ $:頂点２Ｘ $:頂点２Ｚ $:マップ名 {
#if d:DEBUG_PRINT
	print '*** SS No Enter Area ***'
#endif

	chara 透明壁 $:壁の名前 \
		-position  $:頂点１Ｘ $:床の高さ $:頂点１Ｚ $:壁の高さ \
			       $:頂点２Ｘ $:床の高さ $:頂点１Ｚ $:壁の高さ \
		-attribute (d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_PLAYER)) \
		-map       $:マップ名
	chara 透明壁 $:壁の名前 \
		-position  $:頂点２Ｘ $:床の高さ $:頂点１Ｚ $:壁の高さ \
			       $:頂点２Ｘ $:床の高さ $:頂点２Ｚ $:壁の高さ \
		-attribute (d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_PLAYER)) \
		-map       $:マップ名
	chara 透明壁 $:壁の名前 \
		-position  $:頂点２Ｘ $:床の高さ $:頂点２Ｚ $:壁の高さ \
			       $:頂点１Ｘ $:床の高さ $:頂点２Ｚ $:壁の高さ \
		-attribute (d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_PLAYER)) \
		-map       $:マップ名
	chara 透明壁 $:壁の名前 \
		-position  $:頂点１Ｘ $:床の高さ $:頂点２Ｚ $:壁の高さ \
			       $:頂点１Ｘ $:床の高さ $:頂点１Ｚ $:壁の高さ \
		-attribute (d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_PLAYER)) \
		-map       $:マップ名
}

//-----------------------------------------------------------------------------
// デバッグ用キャラ設定
proc SS_デバッグ用キャラ設定 {
#if d:DEBUG_PRINT
	print '*** SS Debug Chara Set ***'
#endif

#ifndef d:FIX_MODE
	#ifdef d:SCREENSHOT
	#if    d:SCREENSHOT
		// スクリーンショット
		chara スクリーンショット スクリーンショット
	#endif
	#endif
#endif
}

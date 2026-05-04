//
// アイテムＢＯＸ定義
// 2000.6/8 S.Okajima
// $Id: item_info.h,v 1.83 2002/06/25 09:12:45 usr03682 Exp $
//

// ファイルが二重呼びされたときの対処
#ifndef d:ITEM_INFO_H
#define ITEM_INFO_H		1


#include "equip.h"
#include "dogtag.h"

#define シナリオの発生パターン	  0
#define 敵兵の発生パターン		  2
#define シナリオの高さ			300
#define 敵兵の高さ				300

//--------------------------------------------
// 変わりにおくもの判定
/* 処理を少しでも軽減するため、指定できるアイテムは最小限にする
	※ $:p_耐久力はダンボールが追加されたときに使用する。
Ｍ９弾薬
ＵＳＰ弾薬
ソコム弾薬
Ｍ４弾薬
ＰＳＧ１弾薬
ＰＳＧ１−Ｔ弾薬	※現在仮。ラベルが出来次第モデル差し替え
ＲＧＢ６弾薬
ニキータ弾薬
スティンガー弾薬
ＡＫＳ弾薬
Ｃ４
チャフ
スタン
グレネード
雑誌				※現在仮。モデルが出来次第モデル差し替え

レーション
止血剤
ジアゼパム
*/
//--------------------------------------------
proc 設置アイテム変更 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_耐久力 $:p_変更アイテム名 {
	#if d:DEBUG_PRINT
		print 'item_set_change!'
		print $:p_変更アイテム名
	#endif

	if ( $:p_変更アイテム名 == なし ) {
	} else if ( $:p_変更アイテム名 == Ｍ９弾薬 ) {
		@弾薬_Ｍ９_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == ＵＳＰ弾薬 ) {
		@弾薬_ＵＳＰ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == ソコム弾薬 ) {
		@弾薬_ソコム_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == Ｍ４弾薬 ) {
		@弾薬_Ｍ４_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == ＰＳＧ１弾薬 ) {
		@弾薬_ＰＳＧ１_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == ＰＳＧ１−Ｔ弾薬 ) {
		@弾薬_ＰＳＧ１−Ｔ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == ＲＧＢ６弾薬 ) {
		@弾薬_ＲＧＢ６_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == ニキータ弾薬 ) {
		@弾薬_ニキータ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == スティンガー弾薬 ) {
		@弾薬_スティンガー_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == ＡＫＳ弾薬 ) {
		@弾薬_ＡＫＳ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == Ｃ４ ) {
		@Ｃ４_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == チャフ ) {
		@チャフ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == スタン ) {
		@スタン_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == グレネード ) {
		@グレネード_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == 雑誌 ) {
		@雑誌_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == レーション ) {
		@レーション_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == 止血剤 ) {
		@止血剤_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else if ( $:p_変更アイテム名 == ジアゼパム ) {
		@ジアゼパム_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫
	} else {
		#if d:DEBUG_PRINT
			print 'no_item!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'
			print $:p_変更アイテム名
		#endif
	}
}




//--------------------------------------------
// 武器
/*	出現武器リスト
※本体を持っていた場合弾薬に自動変換します。したくない場合はNO_BULLET_CHANGEを定義してください
Ｍ９
Ｍ９弾薬
ＵＳＰ
ＵＳＰ弾薬
ソコム				※アイテムボックスはいらない
ソコム弾薬
Ｍ４
Ｍ４弾薬
ＰＳＧ１
ＰＳＧ１弾薬
ＰＳＧ１−Ｔ		※現在仮。ラベルが出来次第モデル差し替え
ＰＳＧ１−Ｔ弾薬	※現在仮。ラベルが出来次第モデル差し替え
ＲＧＢ６
ＲＧＢ６弾薬
ニキータ
ニキータ弾薬
スティンガー
スティンガー弾薬
ＡＫＳ
ＡＫＳ弾薬
クレイモア
Ｃ４
チャフ
スタン
マイク
グレネード
雑誌				※現在仮。モデルが出来次第モデル差し替え

以下はアイテムボックス不要
凍結スプレー
高周波ブレード
マガジン
*/
//--------------------------------------------
// Ｍ９
proc Ｍ９_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_変更アイテム名 {
	#if d:DEBUG_PRINT
		print 'm9_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:Ｍ９ ] == -1 ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \	// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
			-k handgun_ibox \				// モデル：箱本体
			-s handgun_ibox_sh \			// モデル：箱影
			-l m92_label \					// モデル：ラベル
			-f d:アイテム種別:武器本体 \	// ＩＤ：from ..../scn/equip.h
			-i d:武器:Ｍ９ \				// ＩＤ：from ..../scn/equip.h
			-n 16 \							// 個数
			-r $:p_水飛沫 \					// 0で飛沫無し。
			-h d:シナリオの高さ \			// 床面からの高さ
			-x {
				@アイテム取得時番号設定 $4
			}
	} else {
		@設置アイテム変更 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 0 $:p_変更アイテム名
	}
}

proc Ｍ９_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'm9_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		if ( $w:武器弾数Ｒ[ d:武器:Ｍ９ ] == -1 ) {
			chara アイテム 敵兵_Ｍ９ \
				-b d:敵兵の発生パターン \			// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
				-k handgun_ibox \					// モデル：箱本体
				-s handgun_ibox_sh \				// モデル：箱影
				-l m92_label \						// モデル：ラベル
				-f d:アイテム種別:武器本体 \			// ＩＤ：from ..../scn/equip.h
				-i d:武器:Ｍ９ \					// ＩＤ：from ..../scn/equip.h
				-n 16 \								// 個数
				-r 0 \								// 0で飛沫無し。
				-h d:敵兵の高さ						// 床面からの高さ
		} else {
			#ifndef d:NO_BULLET_CHANGE
			// 本体があったら弾薬に変換
				chara アイテム 敵兵_Ｍ９弾 \
					-b d:敵兵の発生パターン \			// 発生パターン
					-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
					-k handgun_amo_ibox \				// モデル：箱本体
					-s handgun_amo_ibox_sh \			// モデル：箱影
					-l m92_amo_label \					// モデル：ラベル
					-f d:アイテム種別:武器弾 \			// ＩＤ：from ..../scn/equip.h
					-i d:武器:Ｍ９ \					// ＩＤ：from ..../scn/equip.h
					-n 15 \								// 個数
					-r 0 \								// 0で飛沫無し。
					-h d:敵兵の高さ						// 床面からの高さ
			#endif
		}
	}
}

// Ｍ９弾薬
proc 弾薬_Ｍ９_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'm9_amo_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \	// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
		-k handgun_amo_ibox \			// モデル：箱本体
		-s handgun_amo_ibox_sh \		// モデル：箱影
		-l m92_amo_label \				// モデル：ラベル
		-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:Ｍ９ \				// ＩＤ：from ..../scn/equip.h
		-n 15 \							// 個数
		-r $:p_水飛沫 \					// 0で飛沫無し。
		-h d:シナリオの高さ \			// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc 弾薬_Ｍ９_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'm9_amo_ene_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:Ｍ９ ] != -1 ) {
	// Ｍ９を持っているときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_Ｍ９弾 \
				-b d:敵兵の発生パターン \			// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
				-k handgun_amo_ibox \				// モデル：箱本体
				-s handgun_amo_ibox_sh \			// モデル：箱影
				-l m92_amo_label \					// モデル：ラベル
				-f d:アイテム種別:武器弾 \			// ＩＤ：from ..../scn/equip.h
				-i d:武器:Ｍ９ \					// ＩＤ：from ..../scn/equip.h
				-n 15 \								// 個数
				-r 0 \								// 0で飛沫無し。
				-h d:敵兵の高さ						// 床面からの高さ
		}
	}
}

//ＵＳＰ取得時にＣＡＬＬするプロック。
#ifndef d:ANOTHER
proc rt_ＵＳＰ取得時ＣＡＬＬ {
	if( (!$f:rft_ＵＳＰ取得時聞いた) && ($w:ゲーム設定 <= d:LEVEL_EASY) ){
		command 無線設定 \
			-call 14112 t:RTO_ＵＳＰ取得時ＣＡＬＬ d:CODEC_CALL
	}
}
#endif

// ＵＳＰ
proc ＵＳＰ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_変更アイテム名 {
	#if d:DEBUG_PRINT
		print 'usp_set'
	#endif

	if ( $w:武器弾数[ d:武器:ＵＳＰ ] == -1 ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \	// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
			-k handgun_ibox \				// モデル：箱本体
			-s handgun_ibox_sh \			// モデル：箱影
			-l usp_label \					// モデル：ラベル
			-f d:アイテム種別:武器本体 \	// ＩＤ：from ..../scn/equip.h
			-i d:武器:ＵＳＰ \				// ＩＤ：from ..../scn/equip.h
			-n 16 \							// 個数
			-r $:p_水飛沫 \					// 0で飛沫無し。
			-h d:シナリオの高さ \			// 床面からの高さ
			-x {
				@アイテム取得時番号設定 $4
				#ifndef d:ANOTHER
					@rt_ＵＳＰ取得時ＣＡＬＬ		//ＵＳＰ取得時のＣＡＬＬ（まんま
				#endif
			}
	} else {
		@設置アイテム変更 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 0 $:p_変更アイテム名
	}
}

// ＵＳＰ弾薬
proc 弾薬_ＵＳＰ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'usp_amo_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \	// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
		-k handgun_amo_ibox \			// モデル：箱本体
		-s handgun_amo_ibox_sh \		// モデル：箱影
		-l usp_amo_label \				// モデル：ラベル
		-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:ＵＳＰ \				// ＩＤ：from ..../scn/equip.h
		-n 15 \							// 個数
		-r $:p_水飛沫 \					// 0で飛沫無し。
		-h d:シナリオの高さ \			// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc 弾薬_ＵＳＰ_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'usp_amo_ene_set'
	#endif

	if ( $w:武器弾数[ d:武器:ＵＳＰ ] != -1 ) {
	// ＵＳＰを持っているときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_ＵＳＰ弾 \
				-b d:敵兵の発生パターン \			// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
				-k handgun_amo_ibox \				// モデル：箱本体
				-s handgun_amo_ibox_sh \			// モデル：箱影
				-l usp_amo_label \					// モデル：ラベル
				-f d:アイテム種別:武器弾 \			// ＩＤ：from ..../scn/equip.h
				-i d:武器:ＵＳＰ \					// ＩＤ：from ..../scn/equip.h
				-n 15 \								// 個数
				-r 0 \								// 0で飛沫無し。
				-h d:敵兵の高さ						// 床面からの高さ
		}
	}
}

// ソコム
proc ソコム_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_変更アイテム名 {
	#if d:DEBUG_PRINT
		print 'socom_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:ソコム ] == -1 ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \	// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
			-k handgun_ibox \				// モデル：箱本体
			-s handgun_ibox_sh \			// モデル：箱影
			-l scm_label \					// モデル：ラベル
			-f d:アイテム種別:武器本体 \	// ＩＤ：from ..../scn/equip.h
			-i d:武器:ソコム \				// ＩＤ：from ..../scn/equip.h
			-n 13 \							// 個数
			-r $:p_水飛沫 \					// 0で飛沫無し。
			-h d:シナリオの高さ \			// 床面からの高さ
			-x {
				@アイテム取得時番号設定 $4
			}
	} else {
		@設置アイテム変更 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 0 $:p_変更アイテム名
	}
}


// ソコム弾薬
proc 弾薬_ソコム_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'socom_amo_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \	// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
		-k handgun_amo_ibox \			// モデル：箱本体
		-s handgun_amo_ibox_sh \		// モデル：箱影
		-l scm_amo_label \				// モデル：ラベル
		-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:ソコム \				// ＩＤ：from ..../scn/equip.h
		-n 12 \							// 個数
		-r $:p_水飛沫 \					// 0で飛沫無し。
		-h d:シナリオの高さ \			// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc 弾薬_ソコム_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'socom_amo_ene_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:ソコム ] != -1 ) {
	// ソコムを持っているときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_ソコム弾 \
				-b d:敵兵の発生パターン \			// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
				-k handgun_amo_ibox \				// モデル：箱本体
				-s handgun_amo_ibox_sh \			// モデル：箱影
				-l scm_amo_label \					// モデル：ラベル
				-f d:アイテム種別:武器弾 \			// ＩＤ：from ..../scn/equip.h
				-i d:武器:ソコム \					// ＩＤ：from ..../scn/equip.h
				-n 12 \								// 個数
				-r 0 \								// 0で飛沫無し。
				-h d:敵兵の高さ						// 床面からの高さ
		}
	}
}

// Ｍ４
proc Ｍ４_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_変更アイテム名 {
	#if d:DEBUG_PRINT
		print 'm4_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:武器弾数[ d:武器:Ｍ４ ] == -1) || \
		(d:PLAYER == ライデン && $w:武器弾数Ｒ[ d:武器:Ｍ４ ] == -1) ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \	// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
			-k rifle_ibox \					// モデル：箱本体
			-s rifle_ibox_sh \				// モデル：箱影
			-l m4_label \					// モデル：ラベル
			-f d:アイテム種別:武器本体 \	// ＩＤ：from ..../scn/equip.h
			-i d:武器:Ｍ４ \				// ＩＤ：from ..../scn/equip.h
			-n 31 \							// 個数
			-r $:p_水飛沫 \					// 0で飛沫無し。
			-h d:シナリオの高さ \			// 床面からの高さ
			-x {
				@アイテム取得時番号設定 $4
			}
	} else {
		@設置アイテム変更 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 0 $:p_変更アイテム名
	}
}

proc Ｍ４_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'm4_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		if ( (d:PLAYER == スネーク && $w:武器弾数[ d:武器:Ｍ４ ] == -1) || \
			(d:PLAYER == ライデン && $w:武器弾数Ｒ[ d:武器:Ｍ４ ] == -1) ) {
			chara アイテム 敵兵_Ｍ４ \
				-b d:敵兵の発生パターン \			// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
				-k rifle_ibox \						// モデル：箱本体
				-s rifle_ibox_sh \					// モデル：箱影
				-l m4_label \						// モデル：ラベル
				-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
				-i d:武器:Ｍ４ \					// ＩＤ：from ..../scn/equip.h
				-n 31 \								// 個数
				-r 0 \								// 0で飛沫無し。
				-h d:敵兵の高さ						// 床面からの高さ
		} else {
			#ifndef d:NO_BULLET_CHANGE
			// 本体があったら弾薬に変換
				chara アイテム 敵兵_Ｍ４弾 \
					-b d:敵兵の発生パターン \			// 発生パターン
					-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
					-k rifle_amo_ibox \					// モデル：箱本体
					-s rifle_amo_ibox_sh \				// モデル：箱影
					-l m4_amo_label \					// モデル：ラベル
					-f d:アイテム種別:武器弾 \			// ＩＤ：from ..../scn/equip.h
					-i d:武器:Ｍ４ \					// ＩＤ：from ..../scn/equip.h
					-n 30 \								// 個数
					-r 0 \								// 0で飛沫無し。
					-h d:敵兵の高さ						// 床面からの高さ
			#endif
		}
	}
}

// Ｍ４弾薬
proc 弾薬_Ｍ４_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'm4_amo_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \	// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
		-k rifle_amo_ibox \				// モデル：箱本体
		-s rifle_amo_ibox_sh \			// モデル：箱影
		-l m4_amo_label \				// モデル：ラベル
		-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:Ｍ４ \				// ＩＤ：from ..../scn/equip.h
		-n 30 \							// 個数
		-r $:p_水飛沫 \					// 0で飛沫無し。
		-h d:シナリオの高さ \			// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc 弾薬_Ｍ４_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'm4_amo_ene_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:武器弾数[ d:武器:Ｍ４ ] != -1) || \
		(d:PLAYER == ライデン && $w:武器弾数Ｒ[ d:武器:Ｍ４ ] != -1) ) {
		// Ｍ４を持っているときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_Ｍ４弾 \
				-b d:敵兵の発生パターン \			// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
				-k rifle_amo_ibox \					// モデル：箱本体
				-s rifle_amo_ibox_sh \				// モデル：箱影
				-l m4_amo_label \					// モデル：ラベル
				-f d:アイテム種別:武器弾 \			// ＩＤ：from ..../scn/equip.h
				-i d:武器:Ｍ４ \					// ＩＤ：from ..../scn/equip.h
				-n 30 \								// 個数
				-r 0 \								// 0で飛沫無し。
				-h d:敵兵の高さ						// 床面からの高さ
		}
	}
}


// ＰＳＧ１
proc ＰＳＧ１_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_変更アイテム名 {
	#if d:DEBUG_PRINT
		print 'psg1_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:ＰＳＧ１ ] == -1 ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \	// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
			-k rifle_ibox \					// モデル：箱本体
			-s rifle_ibox_sh \				// モデル：箱影
			-l psg_label \					// モデル：ラベル
			-f d:アイテム種別:武器本体 \	// ＩＤ：from ..../scn/equip.h
			-i d:武器:ＰＳＧ１ \			// ＩＤ：from ..../scn/equip.h
			-n 21 \							// 個数
			-r $:p_水飛沫 \					// 0で飛沫無し。
			-h d:シナリオの高さ \			// 床面からの高さ
			-x {
				//極悪ちょうざＣＡＬＬ処理。実体はcdc_common_p.hにあるとかないとか。
				#ifndef d:ANOTHER
				#ifdef d:STAGE_PLANT
					@rp_ＰＳＧ１取得時無線系処理
				#endif
				#endif
				@アイテム取得時番号設定 $4
			}
	} else {
		@設置アイテム変更 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 0 $:p_変更アイテム名
	}
}


// ＰＳＧ１弾薬
proc 弾薬_ＰＳＧ１_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'psg1_amo_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \	// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
		-k rifle_amo_ibox \				// モデル：箱本体
		-s rifle_amo_ibox_sh \			// モデル：箱影
		-l psg_amo_label \				// モデル：ラベル
		-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:ＰＳＧ１ \			// ＩＤ：from ..../scn/equip.h
		-n 20 \							// 個数
		-r $:p_水飛沫 \					// 0で飛沫無し。
		-h d:シナリオの高さ \			// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc 弾薬_ＰＳＧ１_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'psg1_amo_ene_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:ＰＳＧ１ ] != -1 ) {
	// ＰＳＧ１を持っているときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_ＰＳＧ１弾 \
				-b d:敵兵の発生パターン \ // 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \              // 発生位置
				-k rifle_amo_ibox \           // モデル：箱本体
				-s rifle_amo_ibox_sh \           // モデル：箱影
				-l psg_amo_label \             // モデル：ラベル
				-f d:アイテム種別:武器弾 \ // ＩＤ：from ..../scn/equip.h
				-i d:武器:ＰＳＧ１ \ // ＩＤ：from ..../scn/equip.h
				-n 20 \                     // 個数
				-r 0 \ // 0で飛沫無し。
				-h d:敵兵の高さ           // 床面からの高さ
		}
	}
}


// ＰＳＧ１−Ｔ
proc ＰＳＧ１−Ｔ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_変更アイテム名 {
	#if d:DEBUG_PRINT
		print 'psg1-t_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:ＰＳＧ１−Ｔ ] == -1 ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \	// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
			-k rifle_ibox \					// モデル：箱本体
			-s rifle_ibox_sh \				// モデル：箱影
			-l psg_t_label \					// モデル：ラベル
			-f d:アイテム種別:武器本体 \	// ＩＤ：from ..../scn/equip.h
			-i d:武器:ＰＳＧ１−Ｔ \			// ＩＤ：from ..../scn/equip.h
			-n 6 \							// 個数
			-r $:p_水飛沫 \					// 0で飛沫無し。
			-h d:シナリオの高さ \			// 床面からの高さ
			-x {
				@アイテム取得時番号設定 $4
			}
	} else {
		@設置アイテム変更 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 0 $:p_変更アイテム名
	}
}


// ＰＳＧ１−Ｔ弾薬
proc 弾薬_ＰＳＧ１−Ｔ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'psg1-t_amo_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \	// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
		-k rifle_amo_ibox \				// モデル：箱本体
		-s rifle_amo_ibox_sh \			// モデル：箱影
		-l psgT_amo_label \				// モデル：ラベル
		-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:ＰＳＧ１−Ｔ \			// ＩＤ：from ..../scn/equip.h
		-n 10 \							// 個数
		-r $:p_水飛沫 \					// 0で飛沫無し。
		-h d:シナリオの高さ \			// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc 弾薬_ＰＳＧ１−Ｔ_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'psg1-t_amo_ene_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:ＰＳＧ１−Ｔ ] != -1 ) {
	// ＰＳＧ１−Ｔを持っているときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_ＰＳＧ１−Ｔ弾 \
				-b d:敵兵の発生パターン \ // 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \              // 発生位置
				-k rifle_amo_ibox \           // モデル：箱本体
				-s rifle_amo_ibox_sh \           // モデル：箱影
				-l psgT_amo_label \             // モデル：ラベル
				-f d:アイテム種別:武器弾 \ // ＩＤ：from ..../scn/equip.h
				-i d:武器:ＰＳＧ１−Ｔ \ // ＩＤ：from ..../scn/equip.h
				-n 10 \                     // 個数
				-r 0 \ // 0で飛沫無し。
				-h d:敵兵の高さ           // 床面からの高さ
		}
	}
}


// ＲＧＢ６
proc ＲＧＢ６_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_変更アイテム名 {
	#if d:DEBUG_PRINT
		print 'rgb_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:ＲＧＢ６ ] == -1 ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \	// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
			-k launcher_ibox \				// モデル：箱本体
			-s launcher_ibox_sh \			// モデル：箱影
			-l rgb_label \					// モデル：ラベル
			-f d:アイテム種別:武器本体 \	// ＩＤ：from ..../scn/equip.h
			-i d:武器:ＲＧＢ６ \			// ＩＤ：from ..../scn/equip.h
			-n 6 \							// 個数
			-r $:p_水飛沫 \					// 0で飛沫無し。
			-h d:シナリオの高さ \			// 床面からの高さ
			-x {
				@アイテム取得時番号設定 $4
			}
	} else {
		@設置アイテム変更 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 0 $:p_変更アイテム名
	}
}

proc ＲＧＢ６_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'rgb_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		if ( $w:武器弾数Ｒ[ d:武器:ＲＧＢ６ ] == -1 ) {
			chara アイテム 敵兵_ＲＧＢ６ \
				-b d:敵兵の発生パターン \			// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
				-k launcher_ibox \					// モデル：箱本体
				-s launcher_ibox_sh \				// モデル：箱影
				-l rgb_label \						// モデル：ラベル
				-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
				-i d:武器:ＲＧＢ６ \				// ＩＤ：from ..../scn/equip.h
				-n 6 \								// 個数
				-r 0 \								// 0で飛沫無し。
				-h d:敵兵の高さ						// 床面からの高さ
		} else {
			#ifndef d:NO_BULLET_CHANGE
			// 本体があったら弾薬に変換
				chara アイテム 敵兵_ＲＧＢ６弾 \
					-b d:敵兵の発生パターン \			// 発生パターン
					-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
					-k launcher_amo_ibox \				// モデル：箱本体
					-s launcher_amo_ibox_sh \			// モデル：箱影
					-l rgb_amo_label \					// モデル：ラベル
					-f d:アイテム種別:武器弾 \			// ＩＤ：from ..../scn/equip.h
					-i d:武器:ＲＧＢ６ \				// ＩＤ：from ..../scn/equip.h
					-n 6 \								// 個数
					-r 0 \								// 0で飛沫無し。
					-h d:敵兵の高さ						// 床面からの高さ
			#endif
		}
	}
}

// ＲＧＢ６弾薬
proc 弾薬_ＲＧＢ６_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'rgb_amo_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \	// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
		-k launcher_amo_ibox \			// モデル：箱本体
		-s launcher_amo_ibox_sh \		// モデル：箱影
		-l rgb_amo_label \				// モデル：ラベル
		-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:ＲＧＢ６ \			// ＩＤ：from ..../scn/equip.h
		-n 6 \							// 個数
		-r $:p_水飛沫 \					// 0で飛沫無し。
		-h d:シナリオの高さ \			// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc 弾薬_ＲＧＢ６_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'rgb_amo_ene_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:ＲＧＢ６ ] != -1 ) {
	// ＲＧＢ６を持っているときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_ＲＧＢ６弾 \
				-b d:敵兵の発生パターン \			// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
				-k launcher_amo_ibox \				// モデル：箱本体
				-s launcher_amo_ibox_sh \			// モデル：箱影
				-l rgb_amo_label \					// モデル：ラベル
				-f d:アイテム種別:武器弾 \			// ＩＤ：from ..../scn/equip.h
				-i d:武器:ＲＧＢ６ \				// ＩＤ：from ..../scn/equip.h
				-n 6 \								// 個数
				-r 0 \								// 0で飛沫無し。
				-h d:敵兵の高さ						// 床面からの高さ
		}
	}
}


// ニキータ
proc ニキータ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_変更アイテム名 {
	#if d:DEBUG_PRINT
		print 'nkt_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:ニキータ ] == -1 ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \	// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
			-k launcher_ibox \				// モデル：箱本体
			-s launcher_ibox_sh \			// モデル：箱影
			-l nkt_label \					// モデル：ラベル
			-f d:アイテム種別:武器本体 \	// ＩＤ：from ..../scn/equip.h
			-i d:武器:ニキータ \			// ＩＤ：from ..../scn/equip.h
			-n 20 \							// 個数
			-r $:p_水飛沫 \					// 0で飛沫無し。
			-h d:シナリオの高さ \			// 床面からの高さ
			-x {
				//極悪ちょうざＣＡＬＬ処理。実体はcdc_common_p.hにあるとかないとか。
				#ifndef d:ANOTHER
				#ifdef d:STAGE_PLANT
					@rp_ニキータ取得時無線系処理
				#endif
				#endif
				@アイテム取得時番号設定 $4
			}
	} else {
		@設置アイテム変更 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 0 $:p_変更アイテム名
	}
}


// ニキータ弾薬
proc 弾薬_ニキータ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'nkt_amo_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \	// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
		-k launcher_amo_ibox \			// モデル：箱本体
		-s launcher_amo_ibox_sh \		// モデル：箱影
		-l nkt_amo_label \				// モデル：ラベル
		-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:ニキータ \			// ＩＤ：from ..../scn/equip.h
		-n 10 \							// 個数
		-r $:p_水飛沫 \					// 0で飛沫無し。
		-h d:シナリオの高さ \			// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc 弾薬_ニキータ_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'nkt_amo_ene_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:ニキータ ] != -1 ) {
	// ニキータを持っているときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_ニキータ弾 \
				-b d:敵兵の発生パターン \			// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
				-k launcher_amo_ibox \				// モデル：箱本体
				-s launcher_amo_ibox_sh \			// モデル：箱影
				-l nkt_amo_label \					// モデル：ラベル
				-f d:アイテム種別:武器弾 \			// ＩＤ：from ..../scn/equip.h
				-i d:武器:ニキータ \				// ＩＤ：from ..../scn/equip.h
				-n 10 \								// 個数
				-r 0 \								// 0で飛沫無し。
				-h d:敵兵の高さ						// 床面からの高さ
		}
	}
}


// スティンガー
proc スティンガー_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_変更アイテム名 {
	#if d:DEBUG_PRINT
		print 'stg_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:スティンガー ] == -1 ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \	// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
			-k launcher_ibox \				// モデル：箱本体
			-s launcher_ibox_sh \			// モデル：箱影
			-l stg_label \					// モデル：ラベル
			-f d:アイテム種別:武器本体 \	// ＩＤ：from ..../scn/equip.h
			-i d:武器:スティンガー \		// ＩＤ：from ..../scn/equip.h
			-n 20 \							// 個数
			-r $:p_水飛沫 \					// 0で飛沫無し。
			-h d:シナリオの高さ \			// 床面からの高さ
			-x {
				@アイテム取得時番号設定 $4
			}
	} else {
		@設置アイテム変更 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 0 $:p_変更アイテム名
	}
}


// スティンガー弾薬
proc 弾薬_スティンガー_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'stg_amo_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \	// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \	// 発生位置
		-k launcher_amo_ibox \			// モデル：箱本体
		-s launcher_amo_ibox_sh \		// モデル：箱影
		-l stg_amo_label \				// モデル：ラベル
		-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:スティンガー \		// ＩＤ：from ..../scn/equip.h
		-n 10 \							// 個数
		-r $:p_水飛沫 \					// 0で飛沫無し。
		-h d:シナリオの高さ \			// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc 弾薬_スティンガー_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'stg_amo_ene_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:スティンガー ] != -1 ) {
	// スティンガーを持っているときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_スティンガー弾 \
				-b d:敵兵の発生パターン \			// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
				-k launcher_amo_ibox \				// モデル：箱本体
				-s launcher_amo_ibox_sh \			// モデル：箱影
				-l stg_amo_label \					// モデル：ラベル
				-f d:アイテム種別:武器弾 \			// ＩＤ：from ..../scn/equip.h
				-i d:武器:スティンガー \			// ＩＤ：from ..../scn/equip.h
				-n 10 \								// 個数
				-r 0 \								// 0で飛沫無し。
				-h d:敵兵の高さ						// 床面からの高さ
		}
	}
}


// ＡＫＳ
proc ＡＫＳ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_変更アイテム名 {
	#if d:DEBUG_PRINT
		print 'aks_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:ＡＫＳ ] == -1 ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k rifle_ibox \						// モデル：箱本体
			-s rifle_ibox_sh \					// モデル：箱影
			-l ak_label \						// モデル：ラベル
			-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
			-i d:武器:ＡＫＳ \					// ＩＤ：from ..../scn/equip.h
			-n 31 \								// 個数
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ \				// 床面からの高さ
			-x {
				//極悪ちょうざＣＡＬＬ処理。実体はcdc_common_p.hにあるとかないとか。
				#ifndef d:ANOTHER
				#ifdef d:STAGE_PLANT
					@rp_ＡＫ取得時無線系処理
				#endif
				#endif
				@アイテム取得時番号設定 $4
			}
	} else {
		@設置アイテム変更 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 0 $:p_変更アイテム名
	}
}

proc ＡＫＳ_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'aks_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		if ( $w:武器弾数Ｒ[ d:武器:ＡＫＳ ] == -1 ) {
			chara アイテム 敵兵_ＡＫＳ \
				-b d:敵兵の発生パターン \ // 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \              // 発生位置
				-k rifle_ibox \           // モデル：箱本体
				-s rifle_ibox_sh \           // モデル：箱影
				-l ak_label \             // モデル：ラベル
				-f d:アイテム種別:武器本体 \ // ＩＤ：from ..../scn/equip.h
				-i d:武器:ＡＫＳ \ // ＩＤ：from ..../scn/equip.h
				-n 31 \            // 個数
				-r 0 \ // 0で飛沫無し。
				-h d:敵兵の高さ           // 床面からの高さ
		} else {
			#ifndef d:NO_BULLET_CHANGE
			// 本体があったら弾薬に変換
				chara アイテム 敵兵_ＡＫＳ弾 \
					-b d:敵兵の発生パターン \			// 発生パターン
					-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
					-k rifle_amo_ibox \					// モデル：箱本体
					-s rifle_amo_ibox_sh \				// モデル：箱影
					-l aks_amo_label \					// モデル：ラベル
					-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
					-i d:武器:ＡＫＳ \					// ＩＤ：from ..../scn/equip.h
					-n 30 \								// 個数
					-r 0 \								// 0で飛沫無し。
					-h d:敵兵の高さ						// 床面からの高さ
			#endif
		}
	}
}

// ＡＫＳ弾薬
proc 弾薬_ＡＫＳ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'aks_amo_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \		// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
		-k rifle_amo_ibox \					// モデル：箱本体
		-s rifle_amo_ibox_sh \				// モデル：箱影
		-l aks_amo_label \					// モデル：ラベル
		-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:ＡＫＳ \					// ＩＤ：from ..../scn/equip.h
		-n 30 \								// 個数
		-r $:p_水飛沫 \						// 0で飛沫無し。
		-h d:シナリオの高さ \				// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc 弾薬_ＡＫＳ_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'aks_amo_ene_set'
	#endif

	if ( $w:武器弾数Ｒ[ d:武器:ＡＫＳ ] != -1 ) {
	// ＡＫＳを持っているときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_ＡＫＳ弾 \
				-b d:敵兵の発生パターン \			// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
				-k rifle_amo_ibox \					// モデル：箱本体
				-s rifle_amo_ibox_sh \				// モデル：箱影
				-l aks_amo_label \					// モデル：ラベル
				-f d:アイテム種別:武器弾 \		// ＩＤ：from ..../scn/equip.h
				-i d:武器:ＡＫＳ \					// ＩＤ：from ..../scn/equip.h
				-n 30 \								// 個数
				-r 0 \								// 0で飛沫無し。
				-h d:敵兵の高さ						// 床面からの高さ
		}
	}
}


// クレイモア
proc クレイモア_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'clm_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \		// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
		-k grenade_ibox \					// モデル：箱本体
		-s grenade_ibox_sh \				// モデル：箱影
		-l clm_label \						// モデル：ラベル
		-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:クレイモア \				// ＩＤ：from ..../scn/equip.h
		-n 4 \								// 個数
		-r $:p_水飛沫 \						// 0で飛沫無し。
		-h d:シナリオの高さ \				// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc クレイモア_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'clm_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		chara アイテム 敵兵_クレイモア \
			-b d:敵兵の発生パターン \			// 発生パターン
			-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
			-k grenade_ibox \					// モデル：箱本体
			-s grenade_ibox_sh \				// モデル：箱影
			-l clm_label \						// モデル：ラベル
			-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
			-i d:武器:クレイモア \				// ＩＤ：from ..../scn/equip.h
			-n 4 \								// 個数
			-r 0 \								// 0で飛沫無し。
			-h d:敵兵の高さ						// 床面からの高さ
	}
}


// Ｃ４
proc Ｃ４_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'c4_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \		// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
		-k grenade_ibox \					// モデル：箱本体
		-s grenade_ibox_sh \				// モデル：箱影
		-l cfr_label \						// モデル：ラベル
		-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:Ｃ４ \					// ＩＤ：from ..../scn/equip.h
		-n 4 \								// 個数
		-r $:p_水飛沫 \						// 0で飛沫無し。
		-h d:シナリオの高さ \				// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc Ｃ４_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'c4_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		chara アイテム 敵兵_Ｃ４ \
			-b d:敵兵の発生パターン \			// 発生パターン
			-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
			-k grenade_ibox \					// モデル：箱本体
			-s grenade_ibox_sh \				// モデル：箱影
			-l cfr_label \						// モデル：ラベル
			-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
			-i d:武器:Ｃ４ \					// ＩＤ：from ..../scn/equip.h
			-n 4 \								// 個数
			-r 0 \								// 0で飛沫無し。
			-h d:敵兵の高さ						// 床面からの高さ
	}
}


// チャフ
proc チャフ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'chaff_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \		// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
		-k grenade_ibox \					// モデル：箱本体
		-s grenade_ibox_sh \				// モデル：箱影
		-l cgr_label \						// モデル：ラベル
		-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:チャフ \					// ＩＤ：from ..../scn/equip.h
		-n 2 \								// 個数
		-r $:p_水飛沫 \						// 0で飛沫無し。
		-h d:シナリオの高さ \				// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc チャフ_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'chaff_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		chara アイテム 敵兵_チャフ \
			-b d:敵兵の発生パターン \			// 発生パターン
			-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
			-k grenade_ibox \					// モデル：箱本体
			-s grenade_ibox_sh \				// モデル：箱影
			-l cgr_label \						// モデル：ラベル
			-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
			-i d:武器:チャフ \					// ＩＤ：from ..../scn/equip.h
			-n 2 \								// 個数
			-r 0 \								// 0で飛沫無し。
			-h d:敵兵の高さ						// 床面からの高さ
	}
}


// スタン
proc スタン_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'stun_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \		// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
		-k grenade_ibox \					// モデル：箱本体
		-s grenade_ibox_sh \				// モデル：箱影
		-l sgr_label \						// モデル：ラベル
		-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:スタン \					// ＩＤ：from ..../scn/equip.h
		-n 2 \								// 個数
		-r $:p_水飛沫 \						// 0で飛沫無し。
		-h d:シナリオの高さ \				// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc スタン_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'stun_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		chara アイテム 敵兵_スタン \
			-b d:敵兵の発生パターン \			// 発生パターン
			-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
			-k grenade_ibox \					// モデル：箱本体
			-s grenade_ibox_sh \				// モデル：箱影
			-l sgr_label \						// モデル：ラベル
			-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
			-i d:武器:スタン \					// ＩＤ：from ..../scn/equip.h
			-n 2 \								// 個数
			-r 0 \								// 0で飛沫無し。
			-h d:敵兵の高さ						// 床面からの高さ
	}
}


// マイク
proc マイク_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'mic_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \		// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
		-k box_ibox \						// モデル：箱本体
		-s box_ibox_sh \					// モデル：箱影
		-l dmp_label \						// モデル：ラベル
		-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:マイク \					// ＩＤ：from ..../scn/equip.h
		-n 1 \								// 個数
		-r $:p_水飛沫 \						// 0で飛沫無し。
		-h d:シナリオの高さ	\				// 床面からの高さ
		-x {
			//極悪ちょうざＣＡＬＬ処理。実体はcdc_common_p.hにあるとかないとか。
			#ifndef d:ANOTHER
			#ifdef d:STAGE_PLANT
				@rp_まいく取得時無線系処理
			#endif
			#endif
		}

}

proc マイク_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'mic_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		chara アイテム 敵兵_マイク \
			-b d:敵兵の発生パターン \			// 発生パターン
			-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
			-k box_ibox \						// モデル：箱本体
			-s box_ibox_sh \					// モデル：箱影
			-l dmp_label \						// モデル：ラベル
			-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
			-i d:武器:マイク \					// ＩＤ：from ..../scn/equip.h
			-n 1 \								// 個数
			-r 0 \								// 0で飛沫無し。
			-h d:敵兵の高さ						// 床面からの高さ
	}
}


// グレネード
proc グレネード_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'gre_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \		// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
		-k grenade_ibox \					// モデル：箱本体
		-s grenade_ibox_sh \				// モデル：箱影
		-l gre_label \						// モデル：ラベル
		-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:グレネード \				// ＩＤ：from ..../scn/equip.h
		-n 2 \								// 個数
		-r $:p_水飛沫 \						// 0で飛沫無し。
		-h d:シナリオの高さ \				// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc グレネード_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'gre_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		chara アイテム 敵兵_グレネード \
			-b d:敵兵の発生パターン \			// 発生パターン
			-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
			-k grenade_ibox \					// モデル：箱本体
			-s grenade_ibox_sh \				// モデル：箱影
			-l gre_label \						// モデル：ラベル
			-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
			-i d:武器:グレネード \				// ＩＤ：from ..../scn/equip.h
			-n 2 \								// 個数
			-r 0 \								// 0で飛沫無し。
			-h d:敵兵の高さ						// 床面からの高さ
	}
}


// 雑誌
proc 雑誌_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'book_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \		// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
		-k magazine_ibox \					// モデル：箱本体
		-s magazine_sh \				// モデル：箱影
		-l magazine_label \						// モデル：ラベル
		-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
		-i d:武器:雑誌 \				// ＩＤ：from ..../scn/equip.h
		-n 1 \								// 個数
		-r $:p_水飛沫 \						// 0で飛沫無し。
		-h d:シナリオの高さ \				// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc 雑誌_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'book_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		chara アイテム 敵兵_雑誌 \
			-b d:敵兵の発生パターン \			// 発生パターン
			-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \	// 発生位置
			-k magazine_ibox \					// モデル：箱本体
			-s magazine_sh \				// モデル：箱影
			-l magazine_label \						// モデル：ラベル
			-f d:アイテム種別:武器本体 \		// ＩＤ：from ..../scn/equip.h
			-i d:武器:雑誌 \					// ＩＤ：from ..../scn/equip.h
			-n 1 \								// 個数
			-r 0 \								// 0で飛沫無し。
			-h d:敵兵の高さ						// 床面からの高さ
	}
}













//--------------------------------------------
// アイテム
/*  出現アイテムリスト
レーション
止血剤
ジアゼパム
タバコ					※現在仮。ラベルが出来次第差し替え。
風邪薬					※現在仮。ラベルが出来次第差し替え。
防弾チョッキ			※個数有限。敵兵が出す場合、持っていたら出ない
地雷探知機				※個数有限。敵兵が出す場合、持っていたら出ない
センサーＢ				※個数有限。敵兵が出す場合、持っていたら出ない
振動センサー			※個数有限。敵兵が出す場合、持っていたら出ない
暗視ゴーグル			※個数有限。敵兵が出す場合、持っていたら出ない
サーマルゴーグル		※個数有限。敵兵が出す場合、持っていたら出ない
髭剃り					※個数有限。敵兵が出す場合、持っていたら出ない
ソコムサプレッサ		※個数有限。敵兵が出す場合、持っていたら出ない 現在仮。モデル差し替え。
ＡＫサプレッサ			※個数有限。敵兵が出す場合、持っていたら出ない 現在仮。モデル差し替え。
デジカメ				※個数有限。敵兵が出す場合、持っていたら出ない 現在仮。モデル差し替え。
濡れダンボール			※持ってなかったら発生
ダンボール				※持ってなかったら発生
ダンボールＢ			※持ってなかったら発生
ダンボールＣ			※持ってなかったら発生
ダンボールＤ			※持ってなかったら発生
ダンボールＥ			※持ってなかったら発生
ドックタグ				※現在仮。モデルが出来次第差し替え。

以下はアイテムボックス不要
双眼鏡
タンカー編カメラ
センサーＡ
カード
野戦服
携帯電話
ＭＯディスク
無限バンダナ
無限カツラ
カツラＡ
カツラＢ
カツラＣ
ステルス迷彩
*/
//--------------------------------------------
// レーション
proc レーション_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'ration_set'
	#endif
	// エクストリーム以上ではレーションを出さない
	if ( $w:ゲーム設定 < d:LEVEL_EXTREME ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k ration_ibox \					// モデル：箱本体
			-s ration_ibox_sh \					// モデル：箱影
			-l rtn_label \						// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:レーション \			// ＩＤ：from ..../scn/equip.h
			-n 1 \								// 個数
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ \				// 床面からの高さ
			-x {
				//極悪ちょうざＣＡＬＬ処理。実体はcdc_common_p.hにあるとかないとか。
				#ifndef d:ANOTHER
				#ifdef d:STAGE_PLANT
				#ifndef d:BOSS_MODE
					@rp_レーション取得時無線系処理
				#endif
				#endif
				#endif
				@アイテム取得時番号設定 $4
			}
	}
}

proc レーション_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'ration_ene_set'
	#endif

	// ヨーロピアンエクストリームではレーションを出さない
	if ( $w:ゲーム設定 <= d:LEVEL_EXTREME ) {
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_レーション \
				-b d:敵兵の発生パターン \				// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \		// 発生位置
				-k ration_ibox \						// モデル：箱本体
				-s ration_ibox_sh \						// モデル：箱影
				-l rtn_label \							// モデル：ラベル
				-f d:アイテム種別:アイテム \			// ＩＤ：from ..../scn/equip.h
				-i d:アイテム:レーション \				// ＩＤ：from ..../scn/equip.h
				-n 1 \									// 個数
				-r 0 \									// 0で飛沫無し。
				-h d:敵兵の高さ \							// 床面からの高さ
				-x {
					//極悪ちょうざＣＡＬＬ処理。実体はcdc_common_p.hにあるとかないとか。
					#ifndef d:ANOTHER
					#ifdef d:STAGE_PLANT
						@rp_レーション取得時無線系処理
					#endif
					#endif
				}
		}
	}
}


// 止血剤
proc 止血剤_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'band_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \		// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
		-k medicine_ibox \					// モデル：箱本体
		-s medicine_ibox_sh \				// モデル：箱影
		-l sbs_label \						// モデル：ラベル
		-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
		-i d:アイテム:止血剤 \				// ＩＤ：from ..../scn/equip.h
		-n 1 \								// 個数
		-r $:p_水飛沫 \						// 0で飛沫無し。
		-h d:シナリオの高さ \				// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc 止血剤_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'band_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		chara アイテム 敵兵_止血剤 \
			-b d:敵兵の発生パターン \				// 発生パターン
			-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \		// 発生位置
			-k medicine_ibox \						// モデル：箱本体
			-s medicine_ibox_sh \					// モデル：箱影
			-l sbs_label \							// モデル：ラベル
			-f d:アイテム種別:アイテム \			// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:止血剤 \					// ＩＤ：from ..../scn/equip.h
			-n 1 \									// 個数
			-r 0 \									// 0で飛沫無し。
			-h d:敵兵の高さ							// 床面からの高さ
	}
}


// ジアゼパム
proc ジアゼパム_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'dzp_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \		// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
		-k medicine_ibox \					// モデル：箱本体
		-s medicine_ibox_sh \				// モデル：箱影
	#ifdef d:JAPANESE
		-l dzp_label \						// モデル：ラベル
	#else
		-l dzp2_label \						// モデル：ラベル
	#endif
		-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
		-i d:アイテム:ジアゼパム \			// ＩＤ：from ..../scn/equip.h
		-n 5 \								// 個数
		-r $:p_水飛沫 \						// 0で飛沫無し。
		-h d:シナリオの高さ \				// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc ジアゼパム_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'dzp_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		chara アイテム 敵兵_ジアゼパム \
			-b d:敵兵の発生パターン \				// 発生パターン
			-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \		// 発生位置
			-k medicine_ibox \						// モデル：箱本体
			-s medicine_ibox_sh \					// モデル：箱影
		#ifdef d:JAPANESE
			-l dzp_label \						// モデル：ラベル
		#else
			-l dzp2_label \						// モデル：ラベル
		#endif
			-f d:アイテム種別:アイテム \			// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:ジアゼパム \				// ＩＤ：from ..../scn/equip.h
			-n 5 \									// 個数
			-r 0 \									// 0で飛沫無し。
			-h d:敵兵の高さ							// 床面からの高さ
	}
}


// 煙草
proc 煙草_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'tabacco_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \		// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
		-k cigarette_ibox \					// モデル：箱本体
		-s cigarette_sh \				// モデル：箱影
		-l cigarette_label \						// モデル：ラベル
		-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
		-i d:アイテム:煙草 \				// ＩＤ：from ..../scn/equip.h
		-n 1 \								// 個数
		-r $:p_水飛沫 \						// 0で飛沫無し。
		-h d:シナリオの高さ \				// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc 煙草_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'tabacco_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		chara アイテム 敵兵_煙草 \
			-b d:敵兵の発生パターン \				// 発生パターン
			-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \		// 発生位置
			-k cigarette_ibox \					// モデル：箱本体
			-s cigarette_sh \				// モデル：箱影
			-l cigarette_label \						// モデル：ラベル
			-f d:アイテム種別:アイテム \			// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:煙草 \					// ＩＤ：from ..../scn/equip.h
			-n 1 \									// 個数
			-r 0 \									// 0で飛沫無し。
			-h d:敵兵の高さ							// 床面からの高さ
	}
}


// 風邪薬
proc 風邪薬_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'dzp_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \		// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
		-k medicine_ibox \					// モデル：箱本体
		-s medicine_ibox_sh \				// モデル：箱影
		-l cold_medicine_label \			// モデル：ラベル
		-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
		-i d:アイテム:風邪薬 \				// ＩＤ：from ..../scn/equip.h
		-n 1 \								// 個数
		-r $:p_水飛沫 \						// 0で飛沫無し。
		-h d:シナリオの高さ \				// 床面からの高さ
		-x {
			@アイテム取得時番号設定 $4
		}
}

proc 風邪薬_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'dzp_ene_set'
	#endif

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)

	command マップ設定 $:p_マップ -set {
		chara アイテム 敵兵_風邪薬 \
			-b d:敵兵の発生パターン \				// 発生パターン
			-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \		// 発生位置
			-k medicine_ibox \						// モデル：箱本体
			-s medicine_ibox_sh \					// モデル：箱影
			-l cold_medicine_label \				// モデル：ラベル
			-f d:アイテム種別:アイテム \			// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:風邪薬 \					// ＩＤ：from ..../scn/equip.h
			-n 1 \									// 個数
			-r 0 \									// 0で飛沫無し。
			-h d:敵兵の高さ							// 床面からの高さ
	}
}


// 防弾チョッキ
proc 防弾チョッキ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'bamer_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:アイテム数[ d:アイテム:防弾チョッキ ] == 0) || \
		(d:PLAYER == ライデン && $w:アイテム数Ｒ[ d:アイテム:防弾チョッキ ] == 0) ) {
			chara アイテム $:p_ボックス名 \
				-b d:シナリオの発生パターン \		// 発生パターン
				-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
				-k box2_ibox \						// モデル：箱本体
				-s box2_ibox_sh \					// モデル：箱影
				-l bam_label \						// モデル：ラベル
				-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
				-i d:アイテム:防弾チョッキ \		// ＩＤ：from ..../scn/equip.h
				-n 1 \								// 個数
				-r $:p_水飛沫 \						// 0で飛沫無し。
				-h d:シナリオの高さ					// 床面からの高さ
	}
}

proc 防弾チョッキ_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'bamer_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:アイテム数[ d:アイテム:防弾チョッキ ] == 0) || \
		(d:PLAYER == ライデン && $w:アイテム数Ｒ[ d:アイテム:防弾チョッキ ] == 0) ) {
		// 防弾チョッキを持っていないときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_防弾チョッキ \
				-b d:敵兵の発生パターン \				// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \		// 発生位置
				-k box2_ibox \							// モデル：箱本体
				-s box2_ibox_sh \						// モデル：箱影
				-l bam_label \							// モデル：ラベル
				-f d:アイテム種別:アイテム \			// ＩＤ：from ..../scn/equip.h
				-i d:アイテム:防弾チョッキ \			// ＩＤ：from ..../scn/equip.h
				-n 1 \									// 個数
				-r 0 \									// 0で飛沫無し。
				-h d:敵兵の高さ							// 床面からの高さ
		}
	}
}


// 地雷探知器
proc 地雷探知器_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'mine_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:アイテム数[ d:アイテム:地雷探知器 ] == 0) || \
		(d:PLAYER == ライデン && $w:アイテム数Ｒ[ d:アイテム:地雷探知器 ] == 0) ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k detector_ibox \					// モデル：箱本体
			-s detector_ibox_sh \				// モデル：箱影
			-l mnd_label \						// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:地雷探知器 \			// ＩＤ：from ..../scn/equip.h
			-n 1 \								// 個数
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ					// 床面からの高さ
	}
}


// センサーＢ
proc センサーＢ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'b_sens_b_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:アイテム数[ d:アイテム:センサーＢ ] == 0) || \
		(d:PLAYER == ライデン && $w:アイテム数Ｒ[ d:アイテム:センサーＢ ] == 0) ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k detector_ibox \					// モデル：箱本体
			-s detector_ibox_sh \				// モデル：箱影
			-l bsn_b_label \					// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:センサーＢ \			// ＩＤ：from ..../scn/equip.h
			-n 1 \								// 個数
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ \				// 床面からの高さ
			-x {
				@アイテム取得時番号設定 $4
			}
	}
}

proc センサーＢ_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'b_sens_b_ene_set'
	#endif

	if ( $w:アイテム数Ｒ[ d:アイテム:センサーＢ ] == 0 ) {
	// センサーＢを持っていないときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_センサーＢ \
				-b d:敵兵の発生パターン \				// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \		// 発生位置
				-k detector_ibox \						// モデル：箱本体
				-s detector_ibox_sh \					// モデル：箱影
				-l bsn_b_label \						// モデル：ラベル
				-f d:アイテム種別:アイテム \			// ＩＤ：from ..../scn/equip.h
				-i d:アイテム:センサーＢ \				// ＩＤ：from ..../scn/equip.h
				-n 1 \									// 個数
				-r 0 \									// 0で飛沫無し。
				-h d:敵兵の高さ							// 床面からの高さ
		}
	}
}


// 振動センサー
proc 振動センサー_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'v_sens_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:アイテム数[ d:アイテム:振動センサー ] == 0) || \
		(d:PLAYER == ライデン && $w:アイテム数Ｒ[ d:アイテム:振動センサー ] == 0) ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k a_p_sensor_ibox \				// モデル：箱本体
			-s medicine_ibox_sh \				// モデル：箱影
	//		-l bsn_b_label \					// モデル：ラベル（振動センサーはラベル無し）
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:振動センサー \		// ＩＤ：from ..../scn/equip.h
			-n 1 \								// 個数
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ					// 床面からの高さ
	}
}

proc 振動センサー_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'v_sens_ene_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:アイテム数[ d:アイテム:振動センサー ] == 0) || \
		(d:PLAYER == ライデン && $w:アイテム数Ｒ[ d:アイテム:振動センサー ] == 0) ) {
		// 振動センサーを持っていないときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_振動センサー \
				-b d:敵兵の発生パターン \				// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \		// 発生位置
				-k a_p_sensor_ibox \					// モデル：箱本体
				-s medicine_ibox_sh \					// モデル：箱影
//				-l bsn_b_label \						// モデル：ラベル（振動センサーはラベル無し）
				-f d:アイテム種別:アイテム \			// ＩＤ：from ..../scn/equip.h
				-i d:アイテム:振動センサー \			// ＩＤ：from ..../scn/equip.h
				-n 1 \									// 個数
				-r 0 \									// 0で飛沫無し。
				-h d:敵兵の高さ							// 床面からの高さ
		}
	}
}


// 暗視ゴーグル
proc 暗視ゴーグル_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'n_goggle_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:アイテム数[ d:アイテム:暗視ゴーグル ] == 0) || \
		(d:PLAYER == ライデン && $w:アイテム数Ｒ[ d:アイテム:暗視ゴーグル ] == 0) ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k goggle_ibox \					// モデル：箱本体
			-s goggle_ibox_sh \					// モデル：箱影
			-l ngl_label \						// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:暗視ゴーグル \		// ＩＤ：from ..../scn/equip.h
			-n 1 \								// 個数
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ					// 床面からの高さ
	}
}


proc 暗視ゴーグル_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'n_goggle_ene_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:アイテム数[ d:アイテム:暗視ゴーグル ] == 0) || \
		(d:PLAYER == ライデン && $w:アイテム数Ｒ[ d:アイテム:暗視ゴーグル ] == 0) ) {
		// 暗視ゴーグルを持っていないときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_暗視ゴーグル \
				-b d:敵兵の発生パターン \				// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \		// 発生位置
				-k goggle_ibox \						// モデル：箱本体
				-s goggle_ibox_sh \						// モデル：箱影
				-l ngl_label \							// モデル：ラベル
				-f d:アイテム種別:アイテム \			// ＩＤ：from ..../scn/equip.h
				-i d:アイテム:暗視ゴーグル \			// ＩＤ：from ..../scn/equip.h
				-n 1 \									// 個数
				-r 0 \									// 0で飛沫無し。
				-h d:敵兵の高さ							// 床面からの高さ
		}
	}
}


// サーマルゴーグル
proc サーマルゴーグル_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 't_goggle_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:アイテム数[ d:アイテム:サーマルゴーグル ] == 0) || \
		(d:PLAYER == ライデン && $w:アイテム数Ｒ[ d:アイテム:サーマルゴーグル ] == 0) ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k goggle_ibox \					// モデル：箱本体
			-s goggle_ibox_sh \					// モデル：箱影
			-l tgl_label \						// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:サーマルゴーグル \	// ＩＤ：from ..../scn/equip.h
			-n 1 \								// 個数
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ					// 床面からの高さ
	}
}


proc サーマルゴーグル_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 't_goggle_ene_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:アイテム数[ d:アイテム:サーマルゴーグル ] == 0) || \
		(d:PLAYER == ライデン && $w:アイテム数Ｒ[ d:アイテム:サーマルゴーグル ] == 0) ) {
		// サーマルゴーグルを持っていないときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_サーマルゴーグル \
				-b d:敵兵の発生パターン \				// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \		// 発生位置
				-k goggle_ibox \						// モデル：箱本体
				-s goggle_ibox_sh \						// モデル：箱影
				-l tgl_label \							// モデル：ラベル
				-f d:アイテム種別:アイテム \			// ＩＤ：from ..../scn/equip.h
				-i d:アイテム:サーマルゴーグル \		// ＩＤ：from ..../scn/equip.h
				-n 1 \									// 個数
				-r 0 \									// 0で飛沫無し。
				-h d:敵兵の高さ							// 床面からの高さ
		}
	}
}


// 髭剃り
proc 髭剃り_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'shver_set'
	#endif

	chara アイテム $:p_ボックス名 \
		-b d:シナリオの発生パターン \		// 発生パターン
		-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
		-k medicine_ibox \					// モデル：箱本体
		-s medicine_ibox_sh \				// モデル：箱影
		-l shv_label \						// モデル：ラベル
		-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
		-i d:アイテム:髭剃り \				// ＩＤ：from ..../scn/equip.h
		-n 1 \								// 個数
		-r $:p_水飛沫 \						// 0で飛沫無し。
		-h d:シナリオの高さ					// 床面からの高さ
}


// ソコムサプレッサ
proc ソコムサプレッサ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'scm_sp_set'
	#endif

	if ( $w:アイテム数Ｒ[ d:アイテム:ソコムサプレッサ ] == 0 && !($w:プレイヤーフラグ & d:PL_SOCOM_SPPRSR_ATTACHED) ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k handgun_ibox \					// モデル：箱本体
			-s handgun_ibox_sh \				// モデル：箱影
			-l scm_sp_label \					// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:ソコムサプレッサ \	// ＩＤ：from ..../scn/equip.h
			-n 1 \								// 個数
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ					// 床面からの高さ
	}
}


proc ソコムサプレッサ_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'scm_sp_ene_set'
	#endif

	if ( $w:アイテム数Ｒ[ d:アイテム:ソコムサプレッサ ] == 0 ) {
	// ソコムサプレッサを持っていないときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_ソコムサプレッサ \
				-b d:敵兵の発生パターン \				// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \		// 発生位置
				-k handgun_ibox \						// モデル：箱本体
				-s handgun_ibox_sh \					// モデル：箱影
				-l scm_sp_label \						// モデル：ラベル
				-f d:アイテム種別:アイテム \			// ＩＤ：from ..../scn/equip.h
				-i d:アイテム:ソコムサプレッサ \		// ＩＤ：from ..../scn/equip.h
				-n 1 \									// 個数
				-r 0 \									// 0で飛沫無し。
				-h d:敵兵の高さ							// 床面からの高さ
		}
	}
}


// ＡＫサプレッサ
proc ＡＫサプレッサ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'ak_sp_set'
	#endif

	if ( $w:アイテム数Ｒ[ d:アイテム:ＡＫサプレッサ ] == 0 && !($w:プレイヤーフラグ & d:PL_AK_SPPRSR_ATTACHED) ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k rifle_ibox \					// モデル：箱本体
			-s rifle_ibox_sh \				// モデル：箱影
			-l ak_sp_label \					// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:ＡＫサプレッサ \		// ＩＤ：from ..../scn/equip.h
			-n 1 \								// 個数
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ					// 床面からの高さ
	}
}


proc ＡＫサプレッサ_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'ak_sp_ene_set'
	#endif

	if ( $w:アイテム数Ｒ[ d:アイテム:ＡＫサプレッサ ] == 0 ) {
	// ＡＫサプレッサを持っていないときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_ＡＫサプレッサ \
				-b d:敵兵の発生パターン \				// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \		// 発生位置
				-k handgun_ibox \						// モデル：箱本体
				-s handgun_ibox_sh \					// モデル：箱影
				-l ak_sp_label \						// モデル：ラベル
				-f d:アイテム種別:アイテム \			// ＩＤ：from ..../scn/equip.h
				-i d:アイテム:ＡＫサプレッサ \			// ＩＤ：from ..../scn/equip.h
				-n 1 \									// 個数
				-r 0 \									// 0で飛沫無し。
				-h d:敵兵の高さ							// 床面からの高さ
		}
	}
}


// ＵＳＰサプレッサ
proc ＵＳＰサプレッサ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'usp_sp_set'
	#endif

	if ( $w:アイテム数[ d:アイテム:ＵＳＰサプレッサ ] == 0 && !($w:プレイヤーフラグ & d:PL_USP_SPPRSR_ATTACHED) ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k handgun_ibox \					// モデル：箱本体
			-s handgun_ibox_sh \				// モデル：箱影
			-l usp_sp_label \					// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:ＵＳＰサプレッサ \		// ＩＤ：from ..../scn/equip.h
			-n 1 \								// 個数
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ					// 床面からの高さ
	}
}


proc ＵＳＰサプレッサ_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 {
	#if d:DEBUG_PRINT
		print 'usp_sp_ene_set'
	#endif

	if ( $w:アイテム数[ d:アイテム:ＵＳＰサプレッサ ] == 0 ) {
	// ＵＳＰサプレッサを持っていないときのみ発動
		eval($i:Ｘ座標 = $:p_Ｘ座標)
		eval($i:Ｙ座標 = $:p_Ｙ座標)
		eval($i:Ｚ座標 = $:p_Ｚ座標)

		command マップ設定 $:p_マップ -set {
			chara アイテム 敵兵_ＵＳＰサプレッサ \
				-b d:敵兵の発生パターン \				// 発生パターン
				-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \		// 発生位置
				-k handgun_ibox \						// モデル：箱本体
				-s handgun_ibox_sh \					// モデル：箱影
				-l usp_sp_label \						// モデル：ラベル
				-f d:アイテム種別:アイテム \			// ＩＤ：from ..../scn/equip.h
				-i d:アイテム:ＵＳＰサプレッサ \			// ＩＤ：from ..../scn/equip.h
				-n 1 \									// 個数
				-r 0 \									// 0で飛沫無し。
				-h d:敵兵の高さ							// 床面からの高さ
		}
	}
}


// デジカメ
proc デジカメ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 {
	#if d:DEBUG_PRINT
		print 'dcamera_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:アイテム数[ d:アイテム:デジカメ ] == 0) || \
		(d:PLAYER == ライデン && $w:アイテム数Ｒ[ d:アイテム:デジカメ ] == 0) ) {
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k digital_camera_ibox \			// モデル：箱本体
			-s digital_camera_sh \				// モデル：箱影
			-l digital_camera_label \			// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:デジカメ \			// ＩＤ：from ..../scn/equip.h
			-n 1 \								// 個数
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ					// 床面からの高さ
	}
}


// 濡れダンボール 耐久力は25がMAX
proc 濡れダンボール_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_耐久力 {
	#if d:DEBUG_PRINT
		print 'cbox_wet_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:アイテム数[ d:アイテム:濡れダンボール ] == 0) || \
		(d:PLAYER == ライデン && $w:アイテム数Ｒ[ d:アイテム:濡れダンボール ] == 0) ) {
		// 濡れダンボールを持っていないときのみ発動
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k box2_ibox \						// モデル：箱本体
			-s box2_ibox_sh \					// モデル：箱影
			-l cbx_label \						// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:濡れダンボール \		// ＩＤ：from ..../scn/equip.h
			-n $:p_耐久力 \						// 個数（ダンボールでは耐久力）
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ					// 床面からの高さ
	}
}


// ダンボール 耐久力は25がMAX
proc ダンボール_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_耐久力 {
	#if d:DEBUG_PRINT
		print 'cbox_a_set'
	#endif

	if ( (d:PLAYER == スネーク && $w:アイテム数[ d:アイテム:ダンボール ] == 0) || \
		(d:PLAYER == ライデン && $w:アイテム数Ｒ[ d:アイテム:ダンボール ] == 0) ) {
		// ダンボールを持っていないときのみ発動
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k box2_ibox \						// モデル：箱本体
			-s box2_ibox_sh \					// モデル：箱影
			-l cbx_label \						// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:ダンボール \			// ＩＤ：from ..../scn/equip.h
			-n $:p_耐久力 \						// 個数（ダンボールでは耐久力）
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ					// 床面からの高さ
	}
}


// ダンボールＢ 耐久力は25がMAX
proc ダンボールＢ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_耐久力 {
	#if d:DEBUG_PRINT
		print 'cbox_b_set'
	#endif

	if ( $w:アイテム数Ｒ[ d:アイテム:ダンボールＢ ] == 0 ) {
	// ダンボールＢを持っていないときのみ発動
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k box2_ibox \						// モデル：箱本体
			-s box2_ibox_sh \					// モデル：箱影
			-l cbx_label \						// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:ダンボールＢ \		// ＩＤ：from ..../scn/equip.h
			-n $:p_耐久力 \						// 個数（ダンボールＢでは耐久力）
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ					// 床面からの高さ
	}
}


// ダンボールＣ 耐久力は25がMAX
proc ダンボールＣ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_耐久力 {
	#if d:DEBUG_PRINT
		print 'cbox_c_set'
	#endif

	if ( $w:アイテム数Ｒ[ d:アイテム:ダンボールＣ ] == 0 ) {
	// ダンボールＣを持っていないときのみ発動
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k box2_ibox \						// モデル：箱本体
			-s box2_ibox_sh \					// モデル：箱影
			-l cbx_label \						// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:ダンボールＣ \		// ＩＤ：from ..../scn/equip.h
			-n $:p_耐久力 \						// 個数（ダンボールＣでは耐久力）
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ					// 床面からの高さ
	}
}


// ダンボールＤ 耐久力は25がMAX
proc ダンボールＤ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_耐久力 {
	#if d:DEBUG_PRINT
		print 'cbox_d_set'
	#endif

	if ( $w:アイテム数Ｒ[ d:アイテム:ダンボールＤ ] == 0 ) {
	// ダンボールＤを持っていないときのみ発動
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k box2_ibox \						// モデル：箱本体
			-s box2_ibox_sh \					// モデル：箱影
			-l cbx_label \						// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:ダンボールＤ \		// ＩＤ：from ..../scn/equip.h
			-n $:p_耐久力 \						// 個数（ダンボールＤでは耐久力）
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ					// 床面からの高さ
	}
}


// ダンボールＥ 耐久力は25がMAX
proc ダンボールＥ_初期設置 $:p_ボックス名 $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_水飛沫 $:p_耐久力 {
	#if d:DEBUG_PRINT
		print 'cbox_e_set'
	#endif

	if ( $w:アイテム数Ｒ[ d:アイテム:ダンボールＥ ] == 0 ) {
	// ダンボールＥＤを持っていないときのみ発動
		chara アイテム $:p_ボックス名 \
			-b d:シナリオの発生パターン \		// 発生パターン
			-p $:p_Ｘ座標,$:p_Ｙ座標,$:p_Ｚ座標 \		// 発生位置
			-k box2_ibox \						// モデル：箱本体
			-s box2_ibox_sh \					// モデル：箱影
			-l cbx_label \						// モデル：ラベル
			-f d:アイテム種別:アイテム \		// ＩＤ：from ..../scn/equip.h
			-i d:アイテム:ダンボールＥ \		// ＩＤ：from ..../scn/equip.h
			-n $:p_耐久力 \						// 個数（ダンボールＥでは耐久力）
			-r $:p_水飛沫 \						// 0で飛沫無し。
			-h d:シナリオの高さ \				// 床面からの高さ
			-x {
				@アイテム取得時番号設定 $4
			}
	}
}


// ドッグタグ
proc ドッグタグ_敵兵 $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_ステータス $:p_ドッグタグＩＤ $:p_ドッグタグ名前 {

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)
	
	eval($w:ドッグタグＩＤ = $:p_ドッグタグＩＤ);
	eval($i:ドッグタグ名前 = $:p_ドッグタグ名前);
	
	command マップ設定 $:p_マップ -set {
		chara アイテム 敵兵_ドッグダグ \
			-b d:敵兵の発生パターン \ // 発生パターン
			-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \              // 発生位置
			-k dogtag_ibox \           // モデル：箱本体
			-s dogtag_ibox_sh \           // モデル：箱影
			-l dogtag_ibox \             // モデル：ラベル
			-f d:アイテム種別:アイテム \ // ＩＤ：from ..../scn/equip.h
			-i d:アイテム:ドッグタグ \ // ＩＤ：from ..../scn/equip.h
			-n 1 \                     // 個数
			-r 0 \ // 0で飛沫無し。
			-h d:敵兵の高さ \          // 床面からの高さ
			-u $w:ドッグタグＩＤ \
			-t $i:ドッグタグ名前 \
			-x {
				command ドッグタグフラグセット $5
			}
	}
}


//オルガ専用
proc ドッグタグ_オルガ $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_ステータス $:p_ドッグタグＩＤ $:p_ドッグタグ名前 {

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)
	eval($w:ドッグタグＩＤ = $:p_ドッグタグＩＤ)
	eval($i:ドッグタグ名前 = $:p_ドッグタグ名前)

	command マップ設定 $:p_マップ -set {
		chara アイテム 敵兵_ドッグダグ \
			-b d:敵兵の発生パターン \ // 発生パターン
			-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \              // 発生位置
			-k dogtag_ibox \           // モデル：箱本体
			-s dogtag_ibox_sh \           // モデル：箱影
			-l dogtag_ibox \             // モデル：ラベル
			-f d:アイテム種別:アイテム \ // ＩＤ：from ..../scn/equip.h
			-i d:アイテム:ドッグタグ \ // ＩＤ：from ..../scn/equip.h
			-n 1 \                     // 個数
			-r 0 \ // 0で飛沫無し。
			-h d:敵兵の高さ \          // 床面からの高さ
			-t $i:ドッグタグ名前 \
			-x {
				command ドッグタグオルガセット
				command ドッグタグ取得セット [dogtag:tnk_w00c_olga_ve]
				command ドッグタグ取得セット [dogtag:tnk_w00c_olga_ea]
				command ドッグタグ取得セット [dogtag:tnk_w00c_olga_no]
				command ドッグタグ取得セット [dogtag:tnk_w00c_olga_ha]
				command ドッグタグ取得セット [dogtag:tnk_w00c_olga_vh]
			}

	}
}



//スネーク専用
proc ドッグタグ_スネーク $:p_マップ $:p_Ｘ座標 $:p_Ｙ座標 $:p_Ｚ座標 $:p_ステータス $:p_ドッグタグＩＤ $:p_ドッグタグ名前 {

	eval($i:Ｘ座標 = $:p_Ｘ座標)
	eval($i:Ｙ座標 = $:p_Ｙ座標)
	eval($i:Ｚ座標 = $:p_Ｚ座標)
	eval($w:ドッグタグＩＤ = $:p_ドッグタグＩＤ)
	eval($i:ドッグタグ名前 = $:p_ドッグタグ名前)

	command マップ設定 $:p_マップ -set {
		chara アイテム 敵兵_ドッグダグ \
			-b d:敵兵の発生パターン \ // 発生パターン
			-p $i:Ｘ座標,$i:Ｙ座標,$i:Ｚ座標 \              // 発生位置
			-k dogtag_ibox \           // モデル：箱本体
			-s dogtag_ibox_sh \           // モデル：箱影
			-l dogtag_ibox \             // モデル：ラベル
			-f d:アイテム種別:アイテム \ // ＩＤ：from ..../scn/equip.h
			-i d:アイテム:ドッグタグ \ // ＩＤ：from ..../scn/equip.h
			-n 1 \                     // 個数
			-r 0 \ // 0で飛沫無し。
			-h d:敵兵の高さ \          // 床面からの高さ
			-t $i:ドッグタグ名前 \
			-x {
				command ドッグタグスネークセット
				command ドッグタグ取得セット [dogtag:plt_w43a_snake_ve]
				command ドッグタグ取得セット [dogtag:plt_w43a_snake_ea]
				command ドッグタグ取得セット [dogtag:plt_w43a_snake_no]
				command ドッグタグ取得セット [dogtag:plt_w43a_snake_ha]
				command ドッグタグ取得セット [dogtag:plt_w43a_snake_vh]
			}

	}
}


#else

print 'item_info.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif


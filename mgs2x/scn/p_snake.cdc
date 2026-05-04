//
//	p_snake.cdc
//	プラント編スネーク無線
//
//	2001/06/15	T.Fukushima
//	$Id: p_snake.cdc,v 1.87 2002/06/07 09:53:01 usr01475 Exp $

/* メッセージ定義 */
#define CODEC_FILE 1
#include "vardef.h"			// 武器・アイテム関係のdefine
#include "cdc_proc_p.h"

#include "p_snake_e.cm"
#include "p_snake_e.ct"

#include "p_snake_f.cm"
#include "p_snake_f.ct"

#include "p_snake_g.cm"
#include "p_snake_g.ct"

#include "p_snake_s.cm"
#include "p_snake_s.ct"

#include "p_snake_i.cm"
#include "p_snake_i.ct"

#include "p_snake_j.cm"
#include "p_snake_j.ct"

proc rp_無線デフォルト前処理＿スネーク {
	@rp_無線デフォルト前処理
	if(`prefreq` != 14180){
		eval($w:rfp_callcount = 1)
	}
}

proc rps_PS_ばぐっち {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ばぐっち
}

proc rps_PS_気にするな {
	if(!$f:rfp_PS_気にするな聞いた){
		@rp_PS_気にするな
		eval($f:rfp_PS_気にするな聞いた = 1)
	}
}

proc rps_PS_エマ救出後＿通信途絶中 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	noise 0 1
	@rpd_顔表示

	@rp_PS_無言
}

proc rps_PS_地雷探知器のありか {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	if(!$f:rfp_PS_地雷探知器のありか聞いた) {
		@rp_PS_地雷探知器のありか１
	}
	if($f:rfp_地雷地帯にＦ脚から来た) {
		@rp_PS_地雷探知器のありか２＿３
	}else {
		@rp_PS_地雷探知器のありか２＿１
	}
	if(!$f:rfp_PS_地雷探知器のありか聞いた) {
		@rp_PS_地雷探知器のありか３
	}
	eval($f:rfp_PS_地雷探知器のありか聞いた = 1)
}


proc rps_PS_標準＿消火器 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_標準＿消火器
}

proc rps_PS_標準＿蒸気パイプ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_標準＿蒸気パイプ
}

proc rps_PS_投擲距離 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_投擲距離
}

proc rps_PS_赤外線ドッグタグ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_赤外線ドッグタグ
}

proc rps_PS_敵兵気絶タイマー {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_敵兵気絶タイマー
}

proc rps_PS_落とし穴死体落し {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_落とし穴死体落し
}

proc rps_PS_ＤＧ連絡橋＿渡り方エルードヒント {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ＤＧ連絡橋＿渡り方エルードヒント
}

proc rps_PS_水中＿ＬＩＦＥまんたん {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_水中＿ＬＩＦＥまんたん
}

//野戦服ないときのみＸＸＸＸＸＸＸＸ
proc rps_PS_野戦服足跡隠し {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_野戦服足跡隠し
}

//ＲＧＢ６あるときのみＸＸＸＸＸＸＸＸＸＸ
proc rps_PS_エアバースト {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_エアバースト
}
proc rps_PS_かねきっく {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_かねきっく
}
proc rps_PS_かねきゃっち {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_かねきゃっち
}
proc rps_PS_エルード踏み {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_エルード踏み
}
proc rps_PS_敵の部位攻撃 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_敵の部位攻撃
}
proc rps_PS_爆弾解体中＿冷却スプレー {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_爆弾解体中＿冷却スプレー
}
proc rps_PS_ゆすりアイテム搾取 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ゆすりアイテム搾取
}
proc rps_PS_ホールドアップ＿搾取 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ホールドアップ＿搾取
}

proc rps_PS_主観パンチ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_主観パンチ
}

proc rps_PS_ローリング活用 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ローリング活用
}

proc rps_PS_敵兵人質 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_敵兵人質
}

//走り撃ち仕様はＸＸＸＸＸＸＸＸＸＸＸｘ
proc rps_PS_走り撃ち {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_走り撃ち２
	@rp_PS_走り撃ち３
}
proc rps_PS_Ｃ４連鎖爆発 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_Ｃ４連鎖爆発
}
proc rps_PS_防弾装備にグレネード {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_防弾装備にグレネード
}
proc rps_PS_ニキータスピードアップ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ニキータスピードアップ
}
proc rps_PS_しゃがみ遮蔽物 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_しゃがみ遮蔽物
}
proc rps_PS_タクティカルリロード {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_タクティカルリロード
}
/*
proc rps_ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_
}
*/

proc rps_PS_麻酔銃 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_麻酔銃１
	//タンカー編やったＸＸＸＸＸＸＸＸＸＸＸＸ
	if( ($w:タンカー編クリア回数 >= 1) && \
		($w:p_story <= d:ST:P046_02_R01ハリアー戦勝利２無線デモ１終了) ){
		@rp_PS_麻酔銃２
	}
	@rp_PS_麻酔銃３
}



proc rps_PS_ソーコム {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ソーコム１
	//サプレッサある
	if($w:プレイヤーフラグ & d:PL_SOCOM_SPPRSR_ATTACHED){
		@rp_PS_ソーコム＿サプレッサー２
		@rp_PS_ソーコム＿サプレッサー３
		eval($f:rfp_PS_ソーコムサプレッサー聞いた = 1 )
	//サプレッサない
	}else{
		@rp_PS_ソーコム２
	}
	eval($f:rfp_PS_ソーコム聞いた = 1)
}

proc rps_PS_ソーコム＿サプレッサー付き {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ソーコム＿サプレッサー２＿１
	@rp_PS_ソーコム＿サプレッサー３
}

proc rps_PS_ＰＳＧ１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ＰＳＧ１
}
proc rps_PS_ＲＧＢ６ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ＲＧＢ６
}
proc rps_PS_リモコンミサイル {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_リモコンミサイル
}
proc rps_PS_スティンガー {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_スティンガー
}
proc rps_PS_クレイモア {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_クレイモア１
	@rp_PS_クレイモア３
}

proc rps_PS_Ｃ４ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_Ｃ４
}

proc rps_PS_チャフグレネード {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_チャフグレネード
}

proc rps_PS_スタングレネード {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_スタングレネード
}

proc rps_PS_指向性マイク {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_指向性マイク
}

proc rps_PS_冷却スプレー {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_冷却スプレー１
}
proc rps_PS_ＡＫ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ＡＫ１
	//変装可能状態
	if(d:ST:P036_13_R04忍者登場１３無線デモ４終了 <= $w:p_story && \
			$w:p_story < d:ST:P040_09_P05エイムズ発見９ポリゴンデモ５終了) {
		@rp_PS_ＡＫ２
	}
	@rp_PS_ＡＫ３
	if($w:プレイヤーフラグ & d:PL_AK_SPPRSR_ATTACHED){
		@rp_PS_ＡＫ＿サプレッサーある
	}else {
		@rp_PS_ＡＫ＿サプレッサーない
	}
	//変装可能状態
	if(d:ST:P036_13_R04忍者登場１３無線デモ４終了 <= $w:p_story && \
			$w:p_story < d:ST:P040_09_P05エイムズ発見９ポリゴンデモ５終了) {
		@rp_PS_ＡＫ４
	}
}
proc rps_PS_マガジン {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_マガジン
}
proc rps_PS_手投げグレネード {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_手投げグレネード
}
proc rps_PS_Ｍ４ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_Ｍ４
}
proc rps_PS_ＰＳＧ１Ｔ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ＰＳＧ１Ｔ
}
proc rps_PS_エロ本 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_エロ本
}


proc rps_PS_レーション {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_レーション
}
proc rps_PS_止血剤 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_止血剤
}
proc rps_PS_ジアゼパム {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ジアゼパム
}
proc rps_PS_敵兵服 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_敵兵服
}
proc rps_PS_ボディアーマー {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ボディアーマー
}
proc rps_PS_地雷探知器 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_地雷探知器１
	//レーダー使用不可状態ＸＸＸＸＸＸ
	if($w:コンフィグ設定 & d:CONFIG_RADAR_OFF){
		@rp_PS_地雷探知器２
	}
}
proc rps_PS_センサーＡ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_センサーＡ１
	@rp_PS_センサーＡ２
	if($w:p_story >= d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了) {
		@rp_PS_センサーＡ３
	}
}
proc rps_PS_センサーＢ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_センサーＢ１
	@rp_PS_センサーＢ２
	if($w:p_story >= d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了) {
		@rp_PS_センサーＢ３
	}
}
proc rps_PS_暗視ゴーグル {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_暗視ゴーグル
}
proc rps_PS_赤外線ゴーグル {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_赤外線ゴーグル１
	//ドッグタグあるときのみ？ＸＸＸＸＸＸＸＸＸｘｘ
	@rp_PS_赤外線ゴーグル２
}
proc rps_PS_双眼鏡 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_双眼鏡１
	if($w:p_story <= d:ST:P070_11_M02ＡＧ起動１１ムービーデモ２開始) {
		@rp_PS_双眼鏡２
	}
	@rp_PS_双眼鏡３
}

proc rps_PS_カメラ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_カメラ
}
proc rps_PS_たばこ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_たばこ
}
proc rps_PS_ダンボール {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ダンボール１
	//箱男撃ったＸＸＸＸＸＸＸＸＸＸＸＸＸＸ
	if($f:rfp_箱男攻撃した){
		@rp_PS_ダンボール２
	}
	@rp_PS_ダンボール３
}

proc rps_PS_生体センサー {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_生体センサー
}
/*
proc rps_ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_
}
proc rps_ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_
}
proc rps_ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_
}
proc rps_ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_
}
proc rps_ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_
}
*/


proc rps_デフォルト操作説明普通操作系 {
	if(!$f:rfp_PS_標準＿消火器聞いた) {
		@rps_PS_標準＿消火器
		eval($f:rfp_PS_標準＿消火器聞いた = 1)

	}else if(!$f:rfp_PS_標準＿蒸気パイプ聞いた){
		@rps_PS_標準＿蒸気パイプ
		eval($f:rfp_PS_標準＿蒸気パイプ聞いた = 1 )
	
	}else if(!$f:rfp_PS_投擲距離聞いた) {
		@rps_PS_投擲距離
		eval($f:rfp_PS_投擲距離聞いた = 1)

	//赤外線ゴーグルある場合のみ？ＸＸＸＸＸＸＸＸＸ
	}else if(!$f:rfp_PS_赤外線ドッグタグ聞いた) {
		@rps_PS_赤外線ドッグタグ
		eval($f:rfp_PS_赤外線ドッグタグ聞いた = 1)

	}else if(!$f:rfp_PS_敵兵気絶タイマー聞いた) {
		@rps_PS_敵兵気絶タイマー
		eval($f:rfp_PS_敵兵気絶タイマー聞いた = 1)
	
	//落とし穴のみ？ＸＸＸＸＸＸＸＸＸＸ
	}else if(!$f:rfp_PS_落とし穴死体落し聞いた){
		@rps_PS_落とし穴死体落し
		eval($f:rfp_PS_落とし穴死体落し聞いた = 1 )
	
	}else if(!$f:rfp_PS_ＤＧ連絡橋＿渡り方エルードヒント聞いた){
		@rps_PS_ＤＧ連絡橋＿渡り方エルードヒント
		eval($f:rfp_PS_ＤＧ連絡橋＿渡り方エルードヒント聞いた = 1 )
	
	}else if(!$f:rfp_PS_水中＿ＬＩＦＥまんたん聞いた){
		@rps_PS_水中＿ＬＩＦＥまんたん
		eval($f:rfp_PS_水中＿ＬＩＦＥまんたん聞いた = 1 )
	
	//野戦服ないときのみＸＸＸＸＸＸＸＸＸＸＸＸＸＸＸＸ
	}else if( (!$f:rfp_PS_野戦服足跡隠し聞いた) && \
				($w:p_story >= d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) ){
		@rps_PS_野戦服足跡隠し
		eval($f:rfp_PS_野戦服足跡隠し聞いた = 1 )
/*
	//ＲＧＢ６あるときのみＸＸＸＸＸＸＸＸＸＸ
	}else if( (!$f:rfp_PS_エアバースト聞いた) && ($w:武器弾数Ｒ[5] >= 0) ){
		@rps_PS_エアバースト
		eval($f:rfp_PS_エアバースト聞いた = 1 )
*/	
	}else if(!$f:rfp_PS_かねきっく聞いた){
		@rps_PS_かねきっく
		eval($f:rfp_PS_かねきっく聞いた = 1 )
	
	}else if(!$f:rfp_PS_かねきゃっち聞いた){
		@rps_PS_かねきゃっち
		eval($f:rfp_PS_かねきゃっち聞いた = 1 )
	
	}else if(!$f:rfp_PS_エルード踏み聞いた){
		@rps_PS_エルード踏み
		eval($f:rfp_PS_エルード踏み聞いた = 1 )
	
	}else if(!$f:rfp_PS_敵の部位攻撃聞いた){
		@rps_PS_敵の部位攻撃
		eval($f:rfp_PS_敵の部位攻撃聞いた = 1 )
	
	//スプレー取得後のみ
	}else if( (!$f:rfp_PS_爆弾解体中＿冷却スプレー聞いた) && ($w:武器弾数Ｒ[14] >= 0) ){
		@rps_PS_爆弾解体中＿冷却スプレー
		eval($f:rfp_PS_爆弾解体中＿冷却スプレー聞いた = 1 )
	
	}else if(!$f:rfp_PS_ゆすりアイテム搾取聞いた){
		@rps_PS_ゆすりアイテム搾取
		eval($f:rfp_PS_ゆすりアイテム搾取聞いた = 1 )
	
	}else if(!$f:rfp_PS_ホールドアップ＿搾取聞いた){
		@rps_PS_ホールドアップ＿搾取
		eval($f:rfp_PS_ホールドアップ＿搾取聞いた = 1 )
	
	}else if(!$f:rfp_PS_主観パンチ聞いた){
		@rps_PS_主観パンチ
		eval($f:rfp_PS_主観パンチ聞いた = 1 )
	
	}else if(!$f:rfp_PS_ローリング活用聞いた){
		@rps_PS_ローリング活用
		eval($f:rfp_PS_ローリング活用聞いた = 1 )
	
	}else if(!$f:rfp_PS_敵兵人質聞いた){
		@rps_PS_敵兵人質
		eval($f:rfp_PS_敵兵人質聞いた = 1 )
	
	}else if(!$f:rfp_PS_走り撃ち聞いた){
		@rps_PS_走り撃ち
		eval($f:rfp_PS_走り撃ち聞いた = 1 )

	}else if( (!$f:rfp_PS_Ｃ４連鎖爆発聞いた) && ($w:武器弾数Ｒ[d:武器:Ｃ４] > 0) ){
		@rps_PS_Ｃ４連鎖爆発
		eval($f:rfp_PS_Ｃ４連鎖爆発聞いた = 1 )

	}else if(!$f:rfp_PS_防弾装備にグレネード聞いた){
		@rps_PS_防弾装備にグレネード
		eval($f:rfp_PS_防弾装備にグレネード聞いた = 1 )

	//リモコンあるときのみＸＸＸＸＸＸＸＸＸＸＸＸｘｘ
	}else if( (!$f:rfp_PS_ニキータスピードアップ聞いた) && ($w:武器弾数Ｒ[6] >= 0) ){
		@rps_PS_ニキータスピードアップ
		eval($f:rfp_PS_ニキータスピードアップ聞いた = 1 )

	}else if(!$f:rfp_PS_しゃがみ遮蔽物聞いた){
		@rps_PS_しゃがみ遮蔽物
		eval($f:rfp_PS_しゃがみ遮蔽物聞いた = 1 )

	}else if(!$f:rfp_PS_タクティカルリロード聞いた){
		@rps_PS_タクティカルリロード
		eval($f:rfp_PS_タクティカルリロード聞いた = 1 )

	}else {
		rand 26
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rps_PS_標準＿消火器

		}else if($w:rfp_rand < 2){
			@rps_PS_標準＿蒸気パイプ

		}else if($w:rfp_rand < 3){
			@rps_PS_投擲距離

		}else if($w:rfp_rand < 4){
			//赤外線ゴーグルある場合のみ？ＸＸＸＸＸＸＸＸＸ
			@rps_PS_赤外線ドッグタグ

		}else if($w:rfp_rand < 5){
			@rps_PS_敵兵気絶タイマー
		
		}else if($w:rfp_rand < 6){
			//落とし穴のみ？ＸＸＸＸＸＸＸＸＸＸ
			@rps_PS_落とし穴死体落し

		}else if($w:rfp_rand < 7){
			@rps_PS_ＤＧ連絡橋＿渡り方エルードヒント

		}else if($w:rfp_rand < 8){
			@rps_PS_水中＿ＬＩＦＥまんたん

		}else if($w:rfp_rand < 9){
			//野戦服ないときのみＸＸＸＸＸＸＸＸＸＸＸＸＸＸＸＸ
			if($w:p_story >= d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了){
				@rps_PS_野戦服足跡隠し
			}else {
				@rps_PS_敵兵気絶タイマー
			}

/*
		}else if($w:rfp_rand < 10){
			//ＲＧＢ６あるときのみＸＸＸＸＸＸＸＸＸＸ
			if($w:武器弾数Ｒ[5] >= 0){
				@rps_PS_エアバースト
			}else {
				@rps_PS_タクティカルリロード
			}
*/

		}else if($w:rfp_rand < 11){
			@rps_PS_かねきっく

		}else if($w:rfp_rand < 12){
			@rps_PS_かねきゃっち

		}else if($w:rfp_rand < 13){
			@rps_PS_エルード踏み

		}else if($w:rfp_rand < 14){
			@rps_PS_敵の部位攻撃

		}else if($w:rfp_rand < 15){
			//スプレー取得後のみ
			if($w:武器弾数Ｒ[14] >= 0){
				@rps_PS_爆弾解体中＿冷却スプレー
			}else {
				@rps_PS_防弾装備にグレネード
			}
		
		}else if($w:rfp_rand < 16){
			@rps_PS_ゆすりアイテム搾取

		}else if($w:rfp_rand < 17){
			@rps_PS_ホールドアップ＿搾取

		}else if($w:rfp_rand < 18){
			@rps_PS_主観パンチ

		}else if($w:rfp_rand < 19){
			@rps_PS_ローリング活用

		}else if($w:rfp_rand < 20){
			@rps_PS_敵兵人質

		}else if($w:rfp_rand < 21){
			@rps_PS_走り撃ち

		}else if( ($w:rfp_rand < 22) && ($w:武器弾数Ｒ[d:武器:Ｃ４] > 0) ){
			@rps_PS_Ｃ４連鎖爆発

		}else if($w:rfp_rand < 23){
			@rps_PS_防弾装備にグレネード

		}else if($w:rfp_rand < 24){
			//リモコンあるときのみＸＸＸＸＸＸＸＸＸＸＸＸｘｘ
			if($w:武器弾数Ｒ[6] >= 0){
				@rps_PS_ニキータスピードアップ
			}else {
				@rps_PS_敵の部位攻撃
			}

		}else if($w:rfp_rand < 25){
			@rps_PS_しゃがみ遮蔽物

		}else {
			@rps_PS_タクティカルリロード
		}
	}
}




proc rps_デフォルト操作説明普通 {
	//Ｍ９
	if( ($w:武器 == 1) && (!$f:rfp_PS_麻酔銃聞いた) ){
		@rps_PS_麻酔銃
		eval($f:rfp_PS_麻酔銃聞いた = 1)

	//２のＵＳＰはなし

	//ソーコムサプレッサなし
	}else if( ($w:武器 == 3) && (!$f:rfp_PS_ソーコム聞いた) ){
		@rps_PS_ソーコム
		eval($f:rfp_PS_ソーコム聞いた = 1)

	//ソーコム サプレッサ
	}else if( ($w:武器 == 3) && ($w:プレイヤーフラグ & d:PL_SOCOM_SPPRSR_ATTACHED) && \
				(!$f:rfp_PS_ソーコムサプレッサー聞いた) ){
		@rps_PS_ソーコム＿サプレッサー付き
		eval($f:rfp_PS_ソーコムサプレッサー聞いた = 1)
		eval($f:rfp_PS_ソーコム聞いた = 1)

	//ＰＳＧ１
	}else if( ($w:武器 == 4) && (!$f:rfp_PS_ＰＳＧ１聞いた) ){
		@rps_PS_ＰＳＧ１
		eval($f:rfp_PS_ＰＳＧ１聞いた = 1)

	//ＲＧＢ６
	}else if( ($w:武器 == 5) && (!$f:rfp_PS_ＲＧＢ６聞いた) ){
		@rps_PS_ＲＧＢ６
		eval($f:rfp_PS_ＲＧＢ６聞いた = 1)

	//リモコンミサイル
	}else if( ($w:武器 == 6) && (!$f:rfp_PS_リモコンミサイル聞いた) ){
		@rps_PS_リモコンミサイル
		eval($f:rfp_PS_リモコンミサイル聞いた = 1)

	//スティンガー
	}else if( ($w:武器 == 7) && (!$f:rfp_PS_スティンガー聞いた) ){
		@rps_PS_スティンガー
		eval($f:rfp_PS_スティンガー聞いた = 1)

	//くれいもや
	}else if( ($w:武器 == 8) && (!$f:rfp_PS_クレイモア聞いた) ){
		@rps_PS_クレイモア
		eval($f:rfp_PS_クレイモア聞いた = 1)

	//Ｃ４
	}else if( ($w:武器 == 9) && (!$f:rfp_PS_Ｃ４聞いた) ){
		@rps_PS_Ｃ４
		eval($f:rfp_PS_Ｃ４聞いた = 1)

	//チャフ
	}else if( ($w:武器 == 10) && (!$f:rfp_PS_チャフグレネード聞いた) ){
		@rps_PS_チャフグレネード
		eval($f:rfp_PS_チャフグレネード聞いた = 1)

	//スタン
	}else if( ($w:武器 == 11) && (!$f:rfp_PS_スタングレネード聞いた) ){
		@rps_PS_スタングレネード
		eval($f:rfp_PS_スタングレネード聞いた = 1)

	//マイク 20はトクシュマイク
	}else if( ( ($w:武器 == 12) || ($w:武器 == 20) )&& (!$f:rfp_PS_指向性マイク聞いた) ){
		@rps_PS_指向性マイク
		eval($f:rfp_PS_指向性マイク聞いた = 1)

	//13のブレードはなしゃしょ。

	//冷却スプレー
	}else if( ($w:武器 == 14) && (!$f:rfp_PS_冷却スプレー聞いた) && \
				($w:p_story < d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了) ){
		@rps_PS_冷却スプレー
		eval($f:rfp_PS_冷却スプレー聞いた = 1)

	//ＡＫ
	}else if( ($w:武器 == 15) && (!$f:rfp_PS_ＡＫ聞いた) ){
		@rps_PS_ＡＫ
		eval($f:rfp_PS_ＡＫ聞いた = 1)

	//空マガジン
	}else if( ($w:武器 == 16) && (!$f:rfp_PS_マガジン聞いた) ){
		@rps_PS_マガジン
		eval($f:rfp_PS_マガジン聞いた = 1)

	//手投げグレネード
	}else if( ($w:武器 == 17) && (!$f:rfp_PS_手投げグレネード聞いた) ){
		@rps_PS_手投げグレネード
		eval($f:rfp_PS_手投げグレネード聞いた = 1)

	//Ｍ４
	}else if( ($w:武器 == 18) && (!$f:rfp_PS_Ｍ４聞いた) ){
		@rps_PS_Ｍ４
		eval($f:rfp_PS_Ｍ４聞いた = 1)

	//ＰＳＧ１Ｔ
	}else if( ($w:武器 == 19) && (!$f:rfp_PS_ＰＳＧ１Ｔ聞いた) ){
		@rps_PS_ＰＳＧ１Ｔ
		eval($f:rfp_PS_ＰＳＧ１Ｔ聞いた = 1)

	//エロ本
	}else if( ($w:武器 == 21) && (!$f:rfp_PS_エロ本聞いた) ){
		@rps_PS_エロ本
		eval($f:rfp_PS_エロ本聞いた = 1)

	//あいてみゃーん
	//レーション
	}else if( ($w:アイテム == 1) && (!$f:rfp_PS_レーション聞いた) ){
		@rps_PS_レーション
		eval($f:rfp_PS_レーション聞いた = 1)

	//3の風邪クスリはなぢゅん
	//止血剤
	}else if( ($w:アイテム == 4) && (!$f:rfp_PS_止血剤聞いた) ){
		@rps_PS_止血剤
		eval($f:rfp_PS_止血剤聞いた = 1)

	//ジアゼパムという噂
	}else if( ($w:アイテム == 5) && (!$f:rfp_PS_ジアゼパム聞いた) ){
		@rps_PS_ジアゼパム
		eval($f:rfp_PS_ジアゼパム聞いた = 1)

	//ゴル兵野戦服
	}else if( ($w:アイテム == 6) && (!$f:rfp_PS_敵兵服聞いた) && \
				($w:p_story < d:ST:P040_01_P01エイムズ発見１ポリゴンデモ１開始) ){
		@rps_PS_敵兵服
		eval($f:rfp_PS_敵兵服聞いた = 1)

	//ぼであま
	}else if( ($w:アイテム == 7) && (!$f:rfp_PS_ボディアーマー聞いた) ){
		@rps_PS_ボディアーマー
		eval($f:rfp_PS_ボディアーマー聞いた = 1)

	//８のステルスはなちゅ

	//地雷探知器
	}else if( ($w:アイテム == 9) && (!$f:rfp_PS_地雷探知器聞いた) ){
		@rps_PS_地雷探知器
		eval($f:rfp_PS_地雷探知器聞いた = 1)

	//センサーＡ
	}else if( ($w:アイテム == 10) && (!$f:rfp_PS_センサーＡ聞いた) ){
		@rps_PS_センサーＡ
		eval($f:rfp_PS_センサーＡ聞いた = 1)

	//センサーＢ
	}else if( ($w:アイテム == 11) && (!$f:rfp_PS_センサーＢ聞いた) && \
				($w:p_story >= d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了) ){
		@rps_PS_センサーＢ
		eval($f:rfp_PS_センサーＢ聞いた = 1)

	//暗視ごぅぐぅ
	}else if( ($w:アイテム == 12) && (!$f:rfp_PS_暗視ゴーグル聞いた) ){
		@rps_PS_暗視ゴーグル
		eval($f:rfp_PS_暗視ゴーグル聞いた = 1)

	//サーマルごぅぐぅ
	}else if( ($w:アイテム == 13) && (!$f:rfp_PS_赤外線ゴーグル聞いた) ){
		@rps_PS_赤外線ゴーグル
		eval($f:rfp_PS_赤外線ゴーグル聞いた = 1)

	//双眼鏡
	}else if( ($w:アイテム == 14) && (!$f:rfp_PS_双眼鏡聞いた) ){
		@rps_PS_双眼鏡
		eval($f:rfp_PS_双眼鏡聞いた = 1)

	//デジカメ
	}else if( ($w:アイテム == 15) && (!$f:rfp_PS_カメラ聞いた) ){
		@rps_PS_カメラ
		eval($f:rfp_PS_カメラ聞いた = 1)

	//ダンボール
	}else if( ( 	($w:アイテム == 16) || ($w:アイテム == 22) || ($w:アイテム == 23) || \
					($w:アイテム == 24) || ($w:アイテム == 26) || ($w:アイテム == 27) ) && \
					(!$f:rfp_PS_ダンボール聞いた) ){
		@rps_PS_ダンボール
		eval($f:rfp_PS_ダンボール聞いた = 1)

	//煙草
	}else if( ($w:アイテム == 17) && (!$f:rfp_PS_たばこ聞いた) ){
		@rps_PS_たばこ
		eval($f:rfp_PS_たばこ聞いた = 1)

	//振動センサー
	}else if( ($w:アイテム == 25) && (!$f:rfp_PS_生体センサー聞いた) ){
		@rps_PS_生体センサー
		eval($f:rfp_PS_生体センサー聞いた = 1)
	
	}else {
		@rps_デフォルト操作説明普通操作系
	}
}


proc rps_デフォルト操作説明危険 {
	//今は仮すま
	@rps_デフォルト操作説明普通
}

proc rps_PS_シェル２中央棟１Ｆ＿ノード位置 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_シェル２中央棟１Ｆ＿ノード位置
}

proc rps_PS_ピーター {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ピーター
}

proc rps_PS_ファットマン {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ファットマン１
}

proc rps_PS_怪奇箱男 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_怪奇箱男
}

proc rps_PS_イロコイ語源 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_イロコイ語源
}

proc rps_PS_ヴァンプ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ヴァンプ１
	if(!$f:rfp_PS_ヴァンプとフォーチュン聞いた){
		@rp_PS_ヴァンプ２
		@rp_PS_ヴァンプとフォーチュン
		eval($f:rfp_PS_ヴァンプとフォーチュン聞いた = 1)
	}
	@rp_PS_ヴァンプ遭遇直後
}

proc rps_PS_デッドセル {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_デッドセル１
	@rp_PS_デッドセル２
	@rp_PS_デッドセル２＿２
}

proc rps_PS_フォーチュン {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_フォーチュン１
	@rp_PS_フォーチュン１＿２
	if(!$f:rfp_PS_ヴァンプとフォーチュン聞いた){
		@rp_PS_フォーチュン２
		@rp_PS_ヴァンプとフォーチュン
		eval($f:rfp_PS_ヴァンプとフォーチュン聞いた = 1)
	}
}

proc rps_PS_ＳＥＡＬＳ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ＳＥＡＬＳ
}

proc rps_PS_海軍大佐 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_海軍大佐１
	@rp_PS_海軍大佐２
	@rps_PS_気にするな
}

proc rps_PS_プリスキン自身 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_プリスキン自身
}

proc rps_PS_オルガ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_オルガ
}

proc rps_PS_ファットマン戦後＿忍者 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ファットマン戦後＿忍者
}

proc rps_PS_ファットマン戦後＿エイムズ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ファットマン戦後＿エイムズ
	@rps_PS_気にするな
}

proc rps_PS_エイムズ後＿エイムズ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_エイムズ後＿エイムズ
	if($w:p_story >= d:ST:P046_02_R01ハリアー戦勝利２無線デモ１終了){
		@rp_PS_エイムズ後＿エイムズ２
	}
	@rp_PS_エイムズ後＿エイムズ３
}

proc rps_PS_アウターヘブン {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_アウターヘブン
}

proc rps_PS_メタルギア拡散 {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PS_メタルギア拡散
}

proc rps_PS_ＧＷの防壁 {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PS_ＧＷの防壁
}

proc rps_PS_プラント偽装 {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PS_プラント偽装
}

proc rps_PS_大統領警護隊 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_大統領警護隊１
	@rp_PS_大統領警護隊２
}

proc rps_連続無線デフォルト {
	if(	(d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了 <= $w:p_story && \
			$w:p_story < d:ST:P069_01_P01ヴァンプ狙撃終了１ポリゴンデモ１開始)  && \
		( ($s:エリア == "w31a") || ($s:エリア == "w31b") || ($s:エリア == "w31c") || \
		($s:エリア == "w31d") || ($s:エリア == "w28a") || ($s:エリア == "w25d") || \
		($s:エリア == "w32a") || ($s:エリア == "w32b") || ($s:エリア == "w31f") ) ){
		command ゲットエマ状態 $b:rfp_ゲットされたエマ状態
		command ゲットエマライフ $i:エマライフ現在値
		command ゲットエマ座標 $i:エマＸ座標変数 $i:エマＹ座標変数 $i:エマＺ座標変数
	}
	if( (!$f:rfp_PS_大統領警護隊聞いた) && \
		(d:ST:P042_01_R01シェル１シェル２連絡橋１無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P044_01_R01赤外線センサークリア１無線デモ１開始) ){
		@rps_PS_大統領警護隊
		eval($f:rfp_PS_大統領警護隊聞いた = 1)
	
	}else if( (!$f:rfp_PS_エマ救出後＿兄妹喧嘩１聞いた) && \
		( ($b:rfp_ゲットされたエマ状態 == d:EMA_F_DO_NOTHING) || \
			($b:rfp_ゲットされたエマ状態 == d:EMA_F_I_SAT_DOWN) ) && \
		( (`@rp_エマが近くにいます`) && (!`@rp_エマたん瀕死` ) ) && \
		(d:ST:P059_03_R01エマ休憩３無線機デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P065_01_P01エマＬ脚１ポリゴンデモ１開始) ){
		@rps_PS_エマ救出後＿兄妹喧嘩１
		eval($f:rfp_PS_エマ救出後＿兄妹喧嘩１聞いた = 1)

	}else if( (!$f:rfp_PS_エマ救出後＿兄妹喧嘩２聞いた) && \
		( ($b:rfp_ゲットされたエマ状態 == d:EMA_F_DO_NOTHING) || \
			($b:rfp_ゲットされたエマ状態 == d:EMA_F_I_SAT_DOWN) ) && \
		( (`@rp_エマが近くにいます`) && (!`@rp_エマたん瀕死` ) ) && \
		(d:ST:P059_03_R01エマ休憩３無線機デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P065_01_P01エマＬ脚１ポリゴンデモ１開始) ){
		@rps_PS_エマ救出後＿兄妹喧嘩２
		eval($f:rfp_PS_エマ救出後＿兄妹喧嘩２聞いた = 1)
			
	}else if( (!$f:rfp_PS_エマ救出後＿兄妹喧嘩３聞いた) && \
		( ($b:rfp_ゲットされたエマ状態 == d:EMA_F_DO_NOTHING) || \
			($b:rfp_ゲットされたエマ状態 == d:EMA_F_I_SAT_DOWN) ) && \
		( (`@rp_エマが近くにいます`) && (!`@rp_エマたん瀕死` ) ) && \
		(d:ST:P062_08_P04エマとＡＩ８ポリゴンデモ４終了 <= $w:p_story && \
			$w:p_story < d:ST:P065_01_P01エマＬ脚１ポリゴンデモ１開始) ){
		@rps_PS_エマ救出後＿兄妹喧嘩３
		eval($f:rfp_PS_エマ救出後＿兄妹喧嘩３聞いた = 1)

	}else if( (!$f:rfp_PS_オルガ聞いた) && \
		($f:w20b_オルガデモフラグ) && \
		($w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１開始) ){
		@rps_PS_オルガ
		eval($f:rfp_PS_オルガ聞いた = 1)

	}else if( (!$f:rfp_PS_ファットマン戦後＿忍者聞いた) && \
				(d:ST:P036_13_R04忍者登場１３無線デモ４終了 <= $w:p_story && \
					$w:p_story < d:ST:P038_01_S01網膜チェック１シナリオデモ１開始) ){
		@rps_PS_ファットマン戦後＿忍者
		eval($f:rfp_PS_ファットマン戦後＿忍者聞いた = 1)

	}else if( (!$f:rfp_PS_ファットマン戦後＿エイムズ聞いた) && \
				(d:ST:P036_13_R04忍者登場１３無線デモ４終了 <= $w:p_story && \
					$w:p_story < d:ST:P040_01_P01エイムズ発見１ポリゴンデモ１開始) ){
		@rps_PS_ファットマン戦後＿エイムズ
		eval($f:rfp_PS_ファットマン戦後＿エイムズ聞いた = 1)

	}else if( (!$f:rfp_PS_アウターヘブン聞いた) && \
				(d:ST:P040_09_P05エイムズ発見９ポリゴンデモ５終了 <= $w:p_story && \
					$w:p_story < d:ST:P049_01_P01大統領１ポリゴンデモ１開始) ){
		@rps_PS_アウターヘブン
		eval($f:rfp_PS_アウターヘブン聞いた = 1)

	}else if( (!$f:rfp_PS_エイムズ後＿エイムズ聞いた) && \
				(d:ST:P040_09_P05エイムズ発見９ポリゴンデモ５終了 <= $w:p_story && \
					$w:p_story < d:ST:P049_01_P01大統領１ポリゴンデモ１開始) ){
		@rps_PS_エイムズ後＿エイムズ
		eval($f:rfp_PS_エイムズ後＿エイムズ聞いた = 1)

	}else if( (!$f:rfp_PS_デッドセル聞いた) && \
		(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
			$w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始) ){
		@rps_PS_デッドセル
		eval($f:rfp_PS_デッドセル聞いた = 1)

	}else if( (!$f:rfp_PS_ヴァンプ聞いた) && \
				(	(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
						$w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１開始) || \
					(d:ST:P046_02_R01ハリアー戦勝利２無線デモ１終了 <= $w:p_story && \
						$w:p_story < d:ST:P049_01_P01大統領１ポリゴンデモ１開始) ) ){
		@rps_PS_ヴァンプ
		eval($f:rfp_PS_ヴァンプ聞いた = 1)

	}else if( (!$f:rfp_PS_フォーチュン聞いた) && \
				(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
					$w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１開始) ){
			@rps_PS_フォーチュン
			eval($f:rfp_PS_フォーチュン聞いた = 1)

	}else if( (!$f:rfp_PS_ファットマン聞いた) && \
			(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
				$w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１開始) ){
		@rps_PS_ファットマン
		eval($f:rfp_PS_ファットマン聞いた = 1)

	}else if( (!$f:rfp_PS_怪奇箱男聞いた) && ($f:rfp_箱男見た) && \
				(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
					$w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１開始) ){
		@rps_PS_怪奇箱男
		eval($f:rfp_PS_怪奇箱男聞いた = 1)

	}else if( (!$f:rfp_PS_ピーター聞いた) && \
			(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
				$w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１開始) ){
		@rps_PS_ピーター
		eval($f:rfp_PS_ピーター聞いた = 1)

	}else if( (!$f:rfp_PS_ＳＥＡＬＳ聞いた) && \
				(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
					$w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１開始) ){
		@rps_PS_ＳＥＡＬＳ
		eval($f:rfp_PS_ＳＥＡＬＳ聞いた = 1)

	}else if( (!$f:rfp_PS_海軍大佐聞いた) && \
				(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
					$w:p_story < d:ST:P036_13_R04忍者登場１３無線デモ４終了) ){
		@rps_PS_海軍大佐
		eval($f:rfp_PS_海軍大佐聞いた = 1)

	}else if( (!$f:rfp_PS_プリスキン自身聞いた) && \
				(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
					$w:p_story < d:ST:P046_02_R01ハリアー戦勝利２無線デモ１終了) ){
		@rps_PS_プリスキン自身
		eval($f:rfp_PS_プリスキン自身聞いた = 1)


	}else if( (!$f:rfp_PS_イロコイ語源聞いた) && \
				(d:ST:P046_02_R01ハリアー戦勝利２無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P055_01_P01ヴァンプ戦前１ポリゴンデモ１開始) ){
		@rps_PS_イロコイ語源
		eval($f:rfp_PS_イロコイ語源聞いた = 1)

	}else if( (!$f:rfp_PS_プラント偽装聞いた) && \
				(d:ST:P049_12_R05大統領１２無線機デモ５終了 <= $w:p_story && \
					$w:p_story < d:ST:P065_01_P01エマＬ脚１ポリゴンデモ１開始) ){
		@rps_PS_プラント偽装
		eval($f:rfp_PS_プラント偽装聞いた = 1)

	}else if( (!$f:rfp_PS_メタルギア拡散聞いた) && \
				(d:ST:P049_12_R05大統領１２無線機デモ５終了 <= $w:p_story && \
					$w:p_story < d:ST:P058_01_P01エマ救出１ポリゴンデモ１開始) ){
		@rps_PS_メタルギア拡散
		eval($f:rfp_PS_メタルギア拡散聞いた = 1)

	}else if( (!$f:rfp_PS_ＧＷの防壁聞いた) && \
				(d:ST:P049_12_R05大統領１２無線機デモ５終了 <= $w:p_story && \
					$w:p_story < d:ST:P065_01_P01エマＬ脚１ポリゴンデモ１開始) ){
		@rps_PS_ＧＷの防壁
		eval($f:rfp_PS_ＧＷの防壁聞いた = 1)


	}else if( ($s:エリア == "w31a") && (!$f:w31a_ノードフラグ) && \
			(!$f:rfp_PS_シェル２中央棟１Ｆ＿ノード位置聞いた) ){
		@rps_PS_シェル２中央棟１Ｆ＿ノード位置
		eval($f:rfp_PS_シェル２中央棟１Ｆ＿ノード位置聞いた = 1)

	}else {
		rand 5
		eval($w:rfp_rand = $status)

		if( ($w:rfp_rand < 2) && ($s:エリア == "w31a") && (!$f:w31a_ノードフラグ) ){
			@rps_PS_シェル２中央棟１Ｆ＿ノード位置

		}else if($w:アラートモード == 1){
			@rps_デフォルト操作説明危険

		//今は仮りん
		}else {
			@rps_デフォルト操作説明普通
		}
	}
}


proc rps_デフォルト操作説明 {
	if($w:アラートモード == 1){
		@rps_デフォルト操作説明危険
	}else {
		@rps_デフォルト操作説明普通
	}
}

proc rps_ＮＰＣスネークダメージ {
	if(!$f:rfp_PS_ＮＰＣスネーク＿ダメージ１聞いた) {
		@rp_PS_ＮＰＣスネーク＿ダメージ１
		eval($f:rfp_PS_ＮＰＣスネーク＿ダメージ１聞いた = 1)

	}else if(!$f:rfp_PS_ＮＰＣスネーク＿ダメージ２聞いた) {
		@rp_PS_ＮＰＣスネーク＿ダメージ２
		eval($f:rfp_PS_ＮＰＣスネーク＿ダメージ２聞いた = 1)

	}else if(!$f:rfp_PS_ＮＰＣスネーク＿ダメージ３聞いた) {
		@rp_PS_ＮＰＣスネーク＿ダメージ３
		eval($f:rfp_PS_ＮＰＣスネーク＿ダメージ３聞いた = 1)

	}else {
		rand 3
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rp_PS_ＮＰＣスネーク＿ダメージ１

		}else if($w:rfp_rand < 2){
			@rp_PS_ＮＰＣスネーク＿ダメージ２

		}else {
			@rp_PS_ＮＰＣスネーク＿ダメージ３
		}
	}
}

proc rps_眠りプリスキン反撃中 {
	@rp_PS_エマ狙撃＿撃たれた４
}

proc rps_眠りプリスキンおやすみ中{
//	eval($f:rfp_PS_眠りプリスキンおやすみ中聞いた = 1)

	if(!$f:rfp_PS_寝ている２聞いた){
		@rp_PS_寝ている２
		eval($f:rfp_PS_寝ている２聞いた = 1)

	}else if(!$f:rfp_PS_寝ている３聞いた){
		@rp_PS_寝ている３
		eval($f:rfp_PS_寝ている３聞いた = 1)

	}else if(!$f:rfp_PS_寝ている４聞いた){
		@rp_PS_寝ている４
		eval($f:rfp_PS_寝ている４聞いた = 1)

	}else if(!$f:rfp_PS_寝ている５聞いた){
		@rp_PS_寝ている５
		eval($f:rfp_PS_寝ている５聞いた = 1)

	}else {
		rand 3
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rp_PS_寝ている２

		}else if($w:rfp_rand < 2){
			@rp_PS_寝ている３
			
		}else {
			@rp_PS_寝ている４
		}
	}
}

proc rps_眠りプリスキン＿その他 {
	@rp_PS_無言
}

proc rps_眠りプリスキン {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	if($s:エリア == "w14a") {
		command ＮＰＣスネークの状態
		//ダメージモーション中
		if( ($status == 8) || ($status == 3) ) {
			@rps_ＮＰＣスネークダメージ

		}else if( ($status == 5) || ($status == 7) ){
			@rps_眠りプリスキン反撃中

		}else if($status == 6){
			@rps_眠りプリスキン＿その他
		
		}else {
			@rps_眠りプリスキンおやすみ中
		}

	}else {
		@rps_眠りプリスキンおやすみ中
	}
}

proc rps_PS_爆弾解体中＿敵排除のススメ１ {
	@rp_PS_爆弾解体中＿敵排除のススメ１＿１
	@rp_PS_爆弾解体中＿敵排除のススメ１＿２
}




proc rps_爆弾解体助言 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	if(!$f:rfp_PS_爆弾解体中＿敵排除のススメ１聞いた) {
		@rps_PS_爆弾解体中＿敵排除のススメ１
		eval($f:rfp_PS_爆弾解体中＿敵排除のススメ１聞いた = 1)
	
	}else if(!$f:rfp_PS_爆弾解体中＿敵排除のススメ２聞いた) {
		@rp_PS_爆弾解体中＿敵排除のススメ２
		eval($f:rfp_PS_爆弾解体中＿敵排除のススメ２聞いた = 1)
	
	}else {
		rand 3
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PS_爆弾解体中＿敵排除のススメ１＿１

		}else if($w:rfp_rand < 2){
			@rp_PS_爆弾解体中＿敵排除のススメ１＿２

		}else {
			@rp_PS_爆弾解体中＿敵排除のススメ２
		}
	}
}

proc rps_PS_爆弾解体中＿催促１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_爆弾解体中＿催促１
}

proc rps_PS_爆弾解体報告１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_爆弾解体報告１
	@rp_PS_爆弾ヒント＿天井
}

proc rps_PS_爆弾解体報告２ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_爆弾解体報告２
	@rp_PS_爆弾ヒント＿ホフク
}

proc rps_PS_爆弾解体報告４ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_爆弾解体報告４
	@rp_PS_爆弾ヒント＿主観
}

proc rps_PS_爆弾解体報告３ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_爆弾解体報告３
	//背中爆弾ありの時のみＸＸＸＸＸＸＸＸｘ
	@rp_PS_爆弾ヒント＿敵の背中
}
/*
proc rps_爆弾解体中 {
	if( ($w:p_story < d:ST:P015_01_R1爆弾解体最初の一つ１無線デモ１終了) && \
		($f:w17a_ダンボール男フラグ) && \
		($s:エリア != w17a) && \
			(!$f:rfp_PS_爆弾解体中＿催促１聞いた) ){
		@rps_PS_爆弾解体中＿催促１
		eval($f:rfp_PS_爆弾解体中＿催促１聞いた = 1)
	
	}else if( ($b:Ｃ４爆弾処理数 == 1) && (!$f:rfp_PS_爆弾解体報告１聞いた) ){
		@rps_PS_爆弾解体報告１
		eval($f:rfp_PS_爆弾解体報告１聞いた = 1 )

	}else if( ($b:Ｃ４爆弾処理数 == 2) && (!$f:rfp_PS_爆弾解体報告２聞いた) ){
		@rps_PS_爆弾解体報告２
		eval($f:rfp_PS_爆弾解体報告２聞いた = 1 )

	}else if( ($b:Ｃ４爆弾処理数 == 3) && (!$f:rfp_PS_爆弾解体報告４聞いた) ){
		@rps_PS_爆弾解体報告４
		eval($f:rfp_PS_爆弾解体報告４聞いた = 1 )

	}else if( ($b:Ｃ４爆弾処理数 == 4) && (!$f:rfp_PS_爆弾解体報告３聞いた) ){
		@rps_PS_爆弾解体報告３
		eval($f:rfp_PS_爆弾解体報告３聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 3
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 2){
			@rps_爆弾解体助言

		}else {
			@rps_デフォルト操作説明
		}

	//連続無線
	}else {
		@rps_連続無線デフォルト
	}
}
*/

proc rps_PS_Ｈ脚底部無臭Ｃ４ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_Ｈ脚底部無臭Ｃ４
}

proc rps_PS_ピーターは歩ける {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	if(!$f:rfp_PS_ピーターは歩ける聞いた) {
		@rp_PS_ピーターは歩ける１
		eval($f:rfp_PS_ピーターは歩ける聞いた = 1)
	}
	@rp_PS_ピーターは歩ける２
}

proc rps_PS_センサーＢ取に行け聞いた {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_センサーＢ
}

proc rps_PS_Ａ脚底部に行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_Ａ脚底部に行け
}


proc rps_PS_ＤＧ連絡橋＿ＰＳＧ１ない {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	if(!$f:rfp_PS_ＤＧ連絡橋＿ＰＳＧ１ない聞いた) {
		@rp_PS_ＤＧ連絡橋＿ＰＳＧ１使え
		eval($f:rfp_PS_ＤＧ連絡橋＿ＰＳＧ１ない聞いた = 1)
	}
	@rp_PS_ＤＧ連絡橋＿ＰＳＧ１ない
	eval($f:rpf_PS_スネークＰＳＧ１場所聞いた = 1)
}

proc rps_PS_ＤＧ連絡橋以外 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ＤＧ連絡橋以外

	//ＰＳＧ１ない
	if($w:武器弾数Ｒ[4] < 0){
		@rp_PS_ＤＧ連絡橋＿ＰＳＧ１ない
		eval($f:rpf_PS_スネークＰＳＧ１場所聞いた = 1)
	}else {
		@rp_PS_ＤＧ連絡橋以外＿ＰＳＧ１ある
	}
}

proc rps_PS_ＤＧ連絡橋＿ＰＳＧ１使え {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ＤＧ連絡橋＿ＰＳＧ１使え
	@rp_PS_ＤＧ連絡橋＿ＰＳＧ１ある
}

proc rps_PS_ＤＧ連絡橋＿場所かえ撃ち {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ＤＧ連絡橋＿場所かえ撃ち
}

proc rps_PS_ＤＧ連絡橋＿海鳥 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	if(!$f:rfp_PS_ＤＧ連絡橋＿海鳥聞いた) {
		@rp_PS_ＤＧ連絡橋＿見落とし
	}

	//ズバリ条件むＸＸＸＸＸＸＸＸｘｘ
	if( ($f:rfp_PS_ＤＧ連絡橋＿海鳥聞いた) && \
		( ( ($i:プレイタイム - $i:rfp_w25a_狙撃イベント開始時刻) / d:FRAME_RATE) >= 300 ) ){
		@rp_PS_ＤＧ連絡橋＿海鳥ズバリ
	}else {
		@rp_PS_ＤＧ連絡橋＿海鳥ヒント
	}
	eval($f:rfp_PS_ＤＧ連絡橋＿海鳥聞いた = 1 )
}

proc rps_PS_ＤＧ連絡橋＿旗 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	if(!$f:rfp_PS_ＤＧ連絡橋＿旗聞いた) {
		@rp_PS_ＤＧ連絡橋＿見落とし
	}

	//ズバリ条件むＸＸＸＸＸＸＸＸｘｘ
	if( ($f:rfp_PS_ＤＧ連絡橋＿旗聞いた) && \
		( ( ($i:プレイタイム - $i:rfp_w25a_狙撃イベント開始時刻) / d:FRAME_RATE) >= 300 ) ){
		@rp_PS_ＤＧ連絡橋＿旗ズパリ１
//		@rp_PS_ＤＧ連絡橋＿旗ズパリ２
	}else {
		@rp_PS_ＤＧ連絡橋＿旗ヒント
	}
	eval($f:rfp_PS_ＤＧ連絡橋＿旗聞いた = 1 )
}

proc rps_PS_セムテックスうんちく {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_セムテックスうんちく
}

proc rps_PS_ＤＧ連絡橋＿サイファー {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	if(!$f:rfp_PS_ＤＧ連絡橋＿サイファー聞いた) {
		@rp_PS_ＤＧ連絡橋＿見落とし
	}
	//ズバリ条件むＸＸＸＸＸＸＸＸｘｘ
	if( ($f:rfp_PS_ＤＧ連絡橋＿サイファー聞いた) && \
		( ( ($i:プレイタイム - $i:rfp_w25a_狙撃イベント開始時刻) / d:FRAME_RATE) >= 300 ) ){
		@rp_PS_ＤＧ連絡橋＿サイファーズバリ１
		@rp_PS_ＤＧ連絡橋＿サイファーズバリ２
	}else {
		@rp_PS_ＤＧ連絡橋＿サイファーヒント
	}
	eval($f:rfp_PS_ＤＧ連絡橋＿サイファー聞いた = 1 )
}


proc rps_PS_ＤＧ連絡橋＿後ろ側 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	if(!$f:rfp_PS_ＤＧ連絡橋＿後ろ側聞いた) {
		@rp_PS_ＤＧ連絡橋＿見落とし
	}

	//ズバリ条件むＸＸＸＸＸＸＸＸｘｘ
	if( ($f:rfp_PS_ＤＧ連絡橋＿後ろ側聞いた) && \
		( (!$f:制御装置破壊フラグ[6]) && ($w:ゲーム設定 >= d:LEVEL_EASY) ) && \
		( ( ($i:プレイタイム - $i:rfp_w25a_狙撃イベント開始時刻) / d:FRAME_RATE) >= 300 ) ){
		@rp_PS_ＤＧ連絡橋＿後ろ側ズバリ１
	}else {
		@rp_PS_ＤＧ連絡橋＿後ろ側ヒント１
	}
	eval($f:rfp_PS_ＤＧ連絡橋＿後ろ側聞いた = 1 )
}


/*
$f:制御装置破壊フラグ[0] SW0 \	// デモ用ドラム壁面
$f:制御装置破壊フラグ[1] SW1 \	// 手前の通路
$f:制御装置破壊フラグ[2] SW2 \	// 手前の通路奥
$f:制御装置破壊フラグ[3] SW3 \	// 左の奥
$f:制御装置破壊フラグ[4] SW4 \	// サイファー
$f:制御装置破壊フラグ[5] SW5 \	// G脚壁面(右)
// VERY EASY
$f:制御装置破壊フラグ[6] SW6 \	// 背後の壁
$f:制御装置破壊フラグ[7] SW7 \	// G脚壁面(左)
// EASY
$f:制御装置破壊フラグ[8] // かもめ群がり
$f:制御装置破壊フラグ[9] // 旗の後ろ
// NORMAL
$f:制御装置破壊フラグ[10] SWA \	// 下段(伸び上がり撃ち）
$f:制御装置破壊フラグ[11] SWB \	// 右足元の壁
// HARD
$f:制御装置破壊フラグ[12] SWC \	// サイファー
$f:制御装置破壊フラグ[13] SWD \	// 見えにくい奴(難易度高い）
*/

//どのスイッチが生き残ってるかをなんかする
proc rps_狙撃イベント中 {
	if(!$f:rfp_PS_ＤＧ連絡橋＿ＰＳＧ１使え聞いた){
		@rps_PS_ＤＧ連絡橋＿ＰＳＧ１使え
		eval($f:rfp_PS_ＤＧ連絡橋＿ＰＳＧ１使え聞いた = 1)
	
	}else if(!$f:rfp_PS_ＤＧ連絡橋＿場所かえ撃ち聞いた) {
		@rps_PS_ＤＧ連絡橋＿場所かえ撃ち
		eval($f:rfp_PS_ＤＧ連絡橋＿場所かえ撃ち聞いた = 1)

	}else if( (!$f:rfp_PS_ＤＧ連絡橋＿サイファー聞いた) && \
				(!`@rp_w25a_赤外線センサー一つも壊してません`) && \
				( (!$f:制御装置破壊フラグ[4]) || \
					( (!$f:制御装置破壊フラグ[12]) && ($w:ゲーム設定 >= d:LEVEL_EXTREME) ) ) ){
		@rps_PS_ＤＧ連絡橋＿サイファー
		eval($f:rfp_PS_ＤＧ連絡橋＿サイファー聞いた = 1)

	}else if( (!$f:rfp_PS_ＤＧ連絡橋＿後ろ側聞いた) && \
				(!`@rp_w25a_赤外線センサー一つも壊してません`) && \
				( (!$f:制御装置破壊フラグ[6]) && ($w:ゲーム設定 >= d:LEVEL_EASY) ) || \
				( (!$f:制御装置破壊フラグ[11]) && ($w:ゲーム設定 >= d:LEVEL_HARD) ) ){
		@rps_PS_ＤＧ連絡橋＿後ろ側
		eval($f:rfp_PS_ＤＧ連絡橋＿後ろ側聞いた = 1)

	}else if( (!$f:rfp_PS_ＤＧ連絡橋＿海鳥聞いた) && \
				(!`@rp_w25a_赤外線センサー一つも壊してません`) && \
				(!$f:制御装置破壊フラグ[8]) && \
				($w:ゲーム設定 >= d:LEVEL_NORMAL) ){
		@rps_PS_ＤＧ連絡橋＿海鳥
		eval($f:rfp_PS_ＤＧ連絡橋＿海鳥聞いた = 1 )

	}else if( (!$f:rfp_PS_ＤＧ連絡橋＿旗聞いた) && \
				(!`@rp_w25a_赤外線センサー一つも壊してません`) && \
				(!$f:制御装置破壊フラグ[9]) && \
				($w:ゲーム設定 >= d:LEVEL_NORMAL) ){
		@rps_PS_ＤＧ連絡橋＿旗
		eval($f:rfp_PS_ＤＧ連絡橋＿旗聞いた = 1)
	
	}else if(!$f:rfp_PS_セムテックスうんちく聞いた) {
		@rps_PS_セムテックスうんちく
		eval($f:rfp_PS_セムテックスうんちく聞いた = 1)


	}else {
		//ホントはらんダムじゃないよＸＸＸＸＸＸＸＸｘｘｘｘ
		rand 6
		eval($w:rfp_rand = $status)

		if( ($w:rfp_rand < 1) && \
			(!`@rp_w25a_赤外線センサー一つも壊してません`) && \
				( (!$f:制御装置破壊フラグ[6]) && ($w:ゲーム設定 >= d:LEVEL_EASY) ) || \
				( (!$f:制御装置破壊フラグ[11]) && ($w:ゲーム設定 >= d:LEVEL_HARD) ) ){
				@rps_PS_ＤＧ連絡橋＿後ろ側

		}else if( ($w:rfp_rand < 2) && \
				(!`@rp_w25a_赤外線センサー一つも壊してません`) && \
				( (!$f:制御装置破壊フラグ[4]) || \
				( (!$f:制御装置破壊フラグ[12]) && ($w:ゲーム設定 >= d:LEVEL_EXTREME) ) ) ){
			@rps_PS_ＤＧ連絡橋＿サイファー

		}else if( ($w:rfp_rand < 3) && \
				(!`@rp_w25a_赤外線センサー一つも壊してません`) && \
				(!$f:制御装置破壊フラグ[8]) && \
				($w:ゲーム設定 >= d:LEVEL_NORMAL) ){
			@rps_PS_ＤＧ連絡橋＿海鳥

		}else if( ($w:rfp_rand < 4) && \
					(!`@rp_w25a_赤外線センサー一つも壊してません`) && \
					(!$f:制御装置破壊フラグ[9]) && \
					($w:ゲーム設定 >= d:LEVEL_NORMAL) ){
			@rps_PS_ＤＧ連絡橋＿旗

		}else if($w:rfp_rand < 5) {
			@rps_PS_ＤＧ連絡橋＿場所かえ撃ち

		}else {
			@rps_PS_ＤＧ連絡橋＿ＰＳＧ１使え
		}
	}
}

proc rps_PS_ハリアー戦＿援護 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ハリアー戦＿援護
}

proc rps_PS_ハリアー戦＿スティンガー {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	if(!$f:rfp_PS_ハリアー戦＿スティンガー聞いた) {
		@rp_PS_ハリアー戦＿スティンガー１
		eval($f:rfp_PS_ハリアー戦＿スティンガー聞いた = 1)
	}
	@rp_PS_ハリアー戦＿スティンガー２
}


proc rps_PS_ハリアー戦＿ハリアー {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PS_ハリアー戦＿ハリアー
}

proc rps_PS_ハリアー戦＿クラスター爆弾 {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	if(!$f:rfp_PS_ハリアー戦＿クラスター爆弾聞いた) {
		@rp_PS_ハリアー戦＿クラスター爆弾１
		eval($f:rfp_PS_ハリアー戦＿クラスター爆弾聞いた = 1 )
	}
	@rp_PS_ハリアー戦＿クラスター爆弾２
}

proc rps_PS_ハリアー戦＿アムラーム {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	if(!$f:rfp_PS_ハリアー戦＿アムラーム聞いた) {
		@rp_PS_ハリアー戦＿アムラーム１
		eval($f:rfp_PS_ハリアー戦＿アムラーム聞いた = 1)
	}
	@rp_PS_ハリアー戦＿アムラーム２
}

proc rps_PS_ハリアー戦＿ロケット {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	if(!$f:rfp_PS_ハリアー戦＿ロケット聞いた) {
		@rp_PS_ハリアー戦＿ロケット１
		eval($f:rfp_PS_ハリアー戦＿ロケット聞いた = 1)
	}
	@rp_PS_ハリアー戦＿ロケット２
}


proc rps_PS_ハリアー戦＿カサッカ {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PS_ハリアー戦＿カサッカ
}


proc rps_PS_ハリアー戦＿撃たれた１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ハリアー戦＿撃たれた１
}

proc rps_PS_ハリアー戦＿撃たれた２ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ハリアー戦＿撃たれた２
}

proc rps_PS_ハリアー戦＿撃たれた３ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ハリアー戦＿撃たれた３
}


proc rps_ハリアー戦中＿ライデンに撃たれた{
	if(!$f:rfp_PS_ハリアー戦＿撃たれた１聞いた) {
		@rps_PS_ハリアー戦＿撃たれた１
		eval($f:rfp_PS_ハリアー戦＿撃たれた１聞いた = 1)

	}else if(!$f:rfp_PS_ハリアー戦＿撃たれた２聞いた) {
		@rps_PS_ハリアー戦＿撃たれた２
		eval($f:rfp_PS_ハリアー戦＿撃たれた２聞いた = 1)

	}else if(!$f:rfp_PS_ハリアー戦＿撃たれた３聞いた) {
		@rps_PS_ハリアー戦＿撃たれた３
		eval($f:rfp_PS_ハリアー戦＿撃たれた３聞いた = 1)
	
	}else {
		rand 3
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rps_PS_ハリアー戦＿撃たれた１
		}else if($w:rfp_rand < 2){
			@rps_PS_ハリアー戦＿撃たれた２
		}else {
			@rps_PS_ハリアー戦＿撃たれた３
		}
	}
}

proc rps_ハリアー戦中 {

	//ライデンに撃たれたダメージ中
	//カサッカライフ
	command ゲットカサッカステータス $b:rfp_カサッカすてーたす
	if($b:rfp_カサッカすてーたす == 1) {
		@rps_ハリアー戦中＿ライデンに撃たれた

	}else if(!$f:rfp_PS_ハリアー戦＿援護聞いた) {
		@rps_PS_ハリアー戦＿援護
		eval($f:rfp_PS_ハリアー戦＿援護聞いた = 1)

	}else if(!$f:rfp_PS_ハリアー戦＿スティンガー聞いた) {
		@rps_PS_ハリアー戦＿スティンガー
		eval($f:rfp_PS_ハリアー戦＿スティンガー聞いた = 1)

	}else if(!$f:rfp_PS_ハリアー戦＿ハリアー聞いた) {
		@rps_PS_ハリアー戦＿ハリアー
		eval($f:rfp_PS_ハリアー戦＿ハリアー聞いた = 1 )
	
	}else if(!$f:rfp_PS_ハリアー戦＿クラスター爆弾聞いた) {
		@rps_PS_ハリアー戦＿クラスター爆弾
		eval($f:rfp_PS_ハリアー戦＿クラスター爆弾聞いた = 1 )

	}else if(!$f:rfp_PS_ハリアー戦＿アムラーム聞いた) {
		@rps_PS_ハリアー戦＿アムラーム
		eval($f:rfp_PS_ハリアー戦＿アムラーム聞いた = 1)

	}else if(!$f:rfp_PS_ハリアー戦＿ロケット聞いた) {
		@rps_PS_ハリアー戦＿ロケット
		eval($f:rfp_PS_ハリアー戦＿ロケット聞いた = 1)

	}else if(!$f:rfp_PS_ハリアー戦＿カサッカ聞いた) {
		@rps_PS_ハリアー戦＿カサッカ
		eval($f:rfp_PS_ハリアー戦＿カサッカ聞いた = 1)

	}else {
		rand 5
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rps_PS_ハリアー戦＿援護

		}else if($w:rfp_rand < 2){
			@rps_PS_ハリアー戦＿スティンガー

		}else if($w:rfp_rand < 3){
			@rps_PS_ハリアー戦＿クラスター爆弾

		}else if($w:rfp_rand < 4){
			@rps_PS_ハリアー戦＿アムラーム

		}else {
			@rps_PS_ハリアー戦＿ロケット
		}
	}
}


proc rps_PS_ハリアー戦後＿大統領救出せよ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ハリアー戦後＿大統領救出せよ
}


proc rps_PS_リモコンミサイルの場所 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	if(!$f:rfp_PS_リモコンミサイルの場所聞いた) {
		@rp_PS_リモコンミサイル１
	}
	@rp_PS_リモコンミサイル２
}

proc rps_PS_シェル２中央棟＿南西行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_シェル２中央棟Ｂ１＿エマ居場所１
	@rp_PS_シェル２中央棟＿南西行け１
	@rp_PS_シェル２中央棟＿南西行け２
}

proc rps_PS_シェル２中央棟＿西行け１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_シェル２中央棟Ｂ１＿エマ居場所１
	@rp_PS_シェル２中央棟＿西行け１
}

proc rps_PS_シェル２中央棟＿北行け１{
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_シェル２中央棟Ｂ１＿エマ居場所１
	@rp_PS_シェル２中央棟＿北行け１
}

proc rps_PS_シェル２中央棟Ｂ１＿エマ居場所１{
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_シェル２中央棟Ｂ１＿エマ居場所１
}

proc rps_PS_シェル２中央棟Ｂ１＿水中レーダー使え {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_シェル２中央棟Ｂ１＿水中レーダー使え１
	@rp_PS_シェル２中央棟Ｂ１＿水中レーダー使え２
}

proc rps_シェル２中央棟Ｂ１エマ助けに行け {
	if( ($s:rfp_w31b_いまいるところ == "north") && \
		(!$f:rfp_PS_シェル２中央棟＿南西行け１聞いた) ) {
		@rps_PS_シェル２中央棟＿南西行け
		eval($f:rfp_PS_シェル２中央棟＿南西行け１聞いた = 1)

	}else if( ($s:rfp_w31b_いまいるところ == "south") && \
				(!$f:rfp_PS_シェル２中央棟＿西行け１聞いた) ){
			@rps_PS_シェル２中央棟＿西行け１
			eval($f:rfp_PS_シェル２中央棟＿西行け１聞いた = 1)
		
	}else if( ($s:rfp_w31b_いまいるところ == "west") && \
				(!$f:rfp_PS_シェル２中央棟＿北行け１聞いた) ){
			@rps_PS_シェル２中央棟＿北行け１
			eval($f:rfp_PS_シェル２中央棟＿北行け１聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 5
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 4){
			if($s:rfp_w31b_いまいるところ == "north") {
				@rps_PS_シェル２中央棟＿南西行け

			}else if($s:rfp_w31b_いまいるところ == "south"){
					@rps_PS_シェル２中央棟＿西行け１
				
			}else if($s:rfp_w31b_いまいるところ == "west"){
					@rps_PS_シェル２中央棟＿北行け１
			}else {
				@rps_デフォルト操作説明
			}
		}else {
			@rps_デフォルト操作説明
		}

	//連続無線
	}else {
		@rps_連続無線デフォルト
	}
}


proc rps_PS_ヴァンプ戦＿ロックオン使うな {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示


	@rp_PS_ヴァンプ戦＿ロックオン使うな１
/*
	if(!$f:rfp_PS_ヴァンプ戦＿ロックオン使うな聞いた) {
		@rp_PS_ヴァンプ戦＿ロックオン使うな２
		eval($f:rfp_PS_ヴァンプ戦＿ロックオン使うな聞いた = 1 )
	}
*/
	@rp_PS_ヴァンプ戦＿ロックオン使うな３
}

proc rps_PS_シェル２中央棟＿濾過装置うんちく {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_シェル２中央棟＿濾過装置うんちく１
	if(!$f:rfp_PS_シェル２中央棟＿濾過装置うんちく聞いた){
		@rpd_オタコン顔表示
		@rp_PS_シェル２中央棟＿濾過装置うんちく２
		eval($f:rfp_PS_シェル２中央棟＿濾過装置うんちく聞いた = 1)
	}
}

proc rps_PS_ヴァンプ戦＿影縫い {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	if(!$f:rfp_PS_ヴァンプ戦＿影縫い聞いた){
		@rp_PS_ヴァンプ戦＿影縫い１
		@rpd_プリスキン顔表示
		@rp_PS_ヴァンプ戦＿影縫い２
		//@rp_PS_ヴァンプ戦＿影縫い３
		eval($f:rfp_PS_ヴァンプ戦＿影縫い聞いた = 1 )
	}else {
		rand 2
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PS_ヴァンプ戦＿影縫い２
		}else {
			@rp_PS_ヴァンプ戦＿影縫い３
		}
	}
}

proc rps_ヴァンプ戦中 {
	if(!$f:rfp_PS_ヴァンプ戦＿ロックオン使うな聞いた) {
		@rps_PS_ヴァンプ戦＿ロックオン使うな
		eval($f:rfp_PS_ヴァンプ戦＿ロックオン使うな聞いた = 1 )
	
	//影縫い引っかかったぼよーんフラグＸＸＸＸＸＸＸＸＸｘ
	}else if( ($f:w31c_影縛りシナリオデモフラグ) && (!$f:rfp_PS_ヴァンプ戦＿影縫い聞いた) && \
				( (!$b:w31c_破壊ロード回数[0]) && (!$b:w31c_破壊ロード回数[1]) && \
					(!$b:w31c_破壊ロード回数[2]) && (!$b:w31c_破壊ロード回数[3]) ) ){
		@rps_PS_ヴァンプ戦＿影縫い
		eval($f:rfp_PS_ヴァンプ戦＿影縫い聞いた = 1 )

	//どーなの？PS_ヴァンプ戦＿水中赤外線ゴーグルＸＸＸＸＸＸＸＸ
	//ひきずり落しはなし？？ＸＸＸＸＸＸＸＸ
	}else if(!$f:rfp_PS_シェル２中央棟＿濾過装置うんちく聞いた){
		@rps_PS_シェル２中央棟＿濾過装置うんちく
		eval($f:rfp_PS_シェル２中央棟＿濾過装置うんちく聞いた = 1)

	}else {
		rand 3
		eval($w:rfp_rand = $status)

		if( ($w:rfp_rand < 1) && ($f:w31c_影縛りシナリオデモフラグ) && \
			( (!$b:w31c_破壊ロード回数[0]) && (!$b:w31c_破壊ロード回数[1]) && \
				(!$b:w31c_破壊ロード回数[2]) && (!$b:w31c_破壊ロード回数[3]) ) ){
			@rps_PS_ヴァンプ戦＿影縫い

		}else if($w:rfp_rand < 2){
			@rps_PS_シェル２中央棟＿濾過装置うんちく
		}else {
			@rps_PS_ヴァンプ戦＿ロックオン使うな
		}
	}
}

proc rps_PS_ヴァンプ戦後＿エマ助けろ濾過槽北水中 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ヴァンプ戦後＿エマ助けろ濾過槽北水中
}

proc rps_PS_ヴァンプ戦後＿エマ助けろ濾過槽以外 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ヴァンプ戦後＿エマ助けろ濾過槽以外
}

proc rps_PS_ヴァンプ戦後＿エマ助けろ＿ロッカー部屋 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

/*
	if($f:rfp_w31c_エマ心音聞いた) {
		@rp_PS_ヴァンプ戦後＿ロッカー部屋＿心音
	}else 
*/

		if(!$f:rfp_PS_ヴァンプ戦後＿エマ助けろ＿ロッカー部屋聞いた){
			@rp_PS_ヴァンプ戦後＿エマ助けろ＿ロッカー部屋１
			eval($f:rfp_PS_ヴァンプ戦後＿エマ助けろ＿ロッカー部屋聞いた = 1)
			@rpd_プリスキン顔表示
		}
		@rp_PS_ヴァンプ戦後＿ロッカー部屋２
	
}


proc rps_PS_ヴァンプ戦後＿純粋水爆 {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PS_ヴァンプ戦後＿純粋水爆
}

proc rps_PS_エマ救出後＿Ｌ脚行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_エマ救出後＿Ｌ脚行け
}


proc rps_PS_エマ救出後＿通信途絶前 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_エマ救出後＿通信途絶前
}

proc rps_PS_エマ救出後＿兄妹喧嘩１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PS_エマ救出後＿兄妹喧嘩１＿１
	@rpd_エマ顔表示プレイヤー側
	@rp_PS_エマ救出後＿兄妹喧嘩１＿２
}

proc rps_PS_エマ救出後＿兄妹喧嘩２ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_エマ救出後＿兄妹喧嘩２＿１
	@rpd_オタコン顔表示
	@rp_PS_エマ救出後＿兄妹喧嘩２＿２
}


proc rps_PS_エマ救出後＿兄妹喧嘩３ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_エマ救出後＿兄妹喧嘩３＿１
	@rp_PS_エマ救出後＿兄妹喧嘩３＿２
}

proc rps_エマ狙撃＿到着してない {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	//残り大麻ーで変える？？ＸＸＸＸＸＸＸ
	//Ｌ脚じゃないから没ね
/*
	if(!$f:rfp_PS_エマ狙撃＿まだです１聞いた){
		@rp_PS_エマ狙撃＿まだです１
		eval($f:rfp_PS_エマ狙撃＿まだです１聞いた = 1)
*/

	if(!$f:rfp_PS_エマ狙撃＿まだです２聞いた) {
		@rp_PS_エマ狙撃＿まだです２
		eval($f:rfp_PS_エマ狙撃＿まだです２聞いた = 1)
		
	}else if(!$f:rfp_PS_エマ狙撃＿まだです３聞いた){
		@rp_PS_エマ狙撃＿まだです３
		eval($f:rfp_PS_エマ狙撃＿まだです３聞いた = 1)
	
	}else {
		rand 2
		eval($w:rfp_rand = $status)
/*
		if($w:rfp_rand < 1)
			@rp_PS_エマ狙撃＿まだです１
*/
		if($w:rfp_rand < 1){
			@rp_PS_エマ狙撃＿まだです２
		}else {
			@rp_PS_エマ狙撃＿まだです３
		}
	}
}


proc rps_PS_エマ狙撃＿依頼１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_エマ狙撃＿依頼１
}

proc rps_PS_エマ狙撃＿依頼２ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_エマ狙撃＿依頼２
}

proc rps_PS_エマ狙撃＿依頼説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_エマ狙撃＿依頼説明
}

proc rps_PS_エマ狙撃＿依頼後＿強い {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_エマ狙撃＿依頼後＿強い
}

proc rps_エマ狙撃＿反撃中 {
	if(!$f:rfp_PS_エマ狙撃＿撃たれた１聞いた) {
		@rp_PS_エマ狙撃＿撃たれた１
		eval($f:rfp_PS_エマ狙撃＿撃たれた１聞いた = 1)

	}else if(!$f:rfp_PS_エマ狙撃＿撃たれた２聞いた) {
		@rp_PS_エマ狙撃＿撃たれた２
		eval($f:rfp_PS_エマ狙撃＿撃たれた２聞いた = 1)
	
	}else if(!$f:rfp_PS_エマ狙撃＿撃たれた３聞いた) {
		@rp_PS_エマ狙撃＿撃たれた３
		eval($f:rfp_PS_エマ狙撃＿撃たれた３聞いた = 1)

	}else if(!$f:rfp_PS_エマ狙撃＿撃たれた４聞いた) {
		@rp_PS_エマ狙撃＿撃たれた４
		eval($f:rfp_PS_エマ狙撃＿撃たれた４聞いた = 1)

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PS_エマ狙撃＿撃たれた１
		}else if($w:rfp_rand < 2){
			@rp_PS_エマ狙撃＿撃たれた２
		}else if($w:rfp_rand < 3){
			@rp_PS_エマ狙撃＿撃たれた３
		}else {
			@rp_PS_エマ狙撃＿撃たれた４
		}
	}
}

proc rps_エマ狙撃＿援護 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	//ライデンに撃たれてダメージ中ＸＸＸＸＸＸＸＸ
	command ＮＰＣスネークの状態
	//ダメージモーション中
	if( ($status == 8) || ($status == 3) ){
		@rps_ＮＰＣスネークダメージ

	}else if($status == 5) {
		@rps_エマ狙撃＿反撃中

	}else if($status == 2) {
		@rps_眠りプリスキンおやすみ中

	}else if(!$f:rfp_w32a_スネーク援護開始した) {
		rand 2
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PS_エマ狙撃＿依頼１
		}else {
			@rp_PS_エマ狙撃＿依頼２
		}
		@rp_PS_エマ狙撃＿依頼後＿強い
		//弱い時ＸＸＸＸＸＸＸＸＸＸｘ
		//@rp_PS_エマ狙撃＿依頼後＿弱い
		@rp_PS_エマ狙撃＿依頼説明
		eval($f:rfp_w32a_スネーク援護開始した = 1)

		mesg ＮＰＣスネーク スネーク 狙撃依頼
		eval($f:w32a_お願いスネークフラグ = 1) //向手さん的ＸＸＸＸ
		

	}else {
		rand 2
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PS_エマ狙撃＿依頼後＿強い
			//弱い時ＸＸＸＸＸＸＸＸＸＸｘ
			//@rp_PS_エマ狙撃＿依頼後＿弱い

		}else {
			@rp_PS_エマ狙撃＿依頼説明
		}
		mesg ＮＰＣスネーク スネーク 狙撃依頼
	}
}

proc rps_PS_狙撃後＿電算室来い {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_狙撃後＿電算室来い
}

proc rps_PS_狙撃後＿敵いない {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_狙撃後＿敵いない
}

proc rps_PS_狙撃後＿開かない扉 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_狙撃後＿開かない扉
}


proc rps_PS_刀説明＿防御 {
	@rp_PS_刀説明＿防御２
	@rp_PS_刀説明＿防御３
}

proc rps_PS_刀説明＿みねうち {
	@rp_PS_刀説明＿みねうち２
	@rp_PS_刀説明＿みねうち４
}

proc rps_スネークと一緒ライデンに反撃中 {
	if(!$f:rfp_PS_ハリアー戦＿撃たれた１聞いた){
		@rp_PS_ハリアー戦＿撃たれた１
		eval($f:rfp_PS_ハリアー戦＿撃たれた１聞いた = 1)

	}else if(!$f:rfp_PS_ハリアー戦＿撃たれた２聞いた) {
		@rp_PS_ハリアー戦＿撃たれた２
		eval($f:rfp_PS_ハリアー戦＿撃たれた２聞いた = 1)

	}else if(!$f:rfp_PS_エマ狙撃＿撃たれた４聞いた){
		@rp_PS_エマ狙撃＿撃たれた４
		eval($f:rfp_PS_エマ狙撃＿撃たれた４聞いた = 1)
	}else {
		rand 3
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rp_PS_ハリアー戦＿撃たれた１

		}else if($w:rfp_rand < 3){
			@rp_PS_ハリアー戦＿撃たれた２
		}else {
			@rp_PS_エマ狙撃＿撃たれた４
		}
	}
}

proc rps_刀練習 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	//催促ＸＸＸＸＸＸＸＸＸPS_刀練習催促

	command ＮＰＣスネークの状態
	//ダメージモーション中
	if( ($status == 8) || ($status == 3) ){
		@rps_ＮＰＣスネークダメージ

	}else if( ($status == 2) || ($status == 4) ){
		@rps_ＮＰＣスネーク意識なし

	}else if( ( $status == 5) || ( $status == 6) || ( $status == 7) ) {
		@rps_スネークと一緒ライデンに反撃中

	}else if(!$f:rfp_PS_刀説明１聞いた){
		@rp_PS_刀練習しろ
		@rp_PS_刀説明１
		eval($f:rfp_PS_刀説明１聞いた = 1)
	
	}else if(!$f:rfp_PS_刀説明２聞いた) {
		@rp_PS_刀説明２
		eval($f:rfp_PS_刀説明２聞いた = 1)
	
	}else if(!$f:rfp_PS_刀説明３聞いた){
		@rp_PS_刀説明３
		eval($f:rfp_PS_刀説明３聞いた = 1)
	
	}else if(!$f:rfp_PS_刀説明＿防御聞いた) {
		@rps_PS_刀説明＿防御
		eval($f:rfp_PS_刀説明＿防御聞いた = 1)
	
	}else if(!$f:rfp_PS_刀説明＿みねうち聞いた) {
		@rps_PS_刀説明＿みねうち
		eval($f:rfp_PS_刀説明＿みねうち聞いた = 1 )

	}else if(!$f:rfp_PS_刀練習すんだら来い聞いた) {
		@rp_PS_刀練習すんだら来い
		eval($f:rfp_PS_刀練習すんだら来い聞いた = 1 )
	
	}else {
		rand 5
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PS_刀説明１

		}else if($w:rfp_rand < 2){
			@rp_PS_刀説明２

		}else if($w:rfp_rand < 3){
			@rp_PS_刀説明３

		}else if($w:rfp_rand < 4){
			@rps_PS_刀説明＿防御
		
		}else {
			@rps_PS_刀説明＿みねうち
		}
	}
}

proc rps_PS_ＮＰＣスネーク＿弱音 {
	if(!$f:rfp_PS_ＮＰＣスネーク＿弱音１聞いた) {
		@rp_PS_ＮＰＣスネーク＿弱音１
		eval($f:rfp_PS_ＮＰＣスネーク＿弱音１聞いた = 1)

	}else if(!$f:rfp_PS_ＮＰＣスネーク＿弱音２聞いた) {
		@rp_PS_ＮＰＣスネーク＿弱音２
		eval($f:rfp_PS_ＮＰＣスネーク＿弱音２聞いた = 1)

	}else if(!$f:rfp_PS_ＮＰＣスネーク＿弱音３聞いた) {
		@rp_PS_ＮＰＣスネーク＿弱音３
		eval($f:rfp_PS_ＮＰＣスネーク＿弱音３聞いた = 1)

	}else {
		rand 3
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PS_ＮＰＣスネーク＿弱音１
		}else if($w:rfp_rand < 2){
			@rp_PS_ＮＰＣスネーク＿弱音２
		}else {
			@rp_PS_ＮＰＣスネーク＿弱音３
		}
	}
}


proc rps_ＮＰＣスネーク＿せかし {
	if(!$f:rfp_PS_ＮＰＣスネーク＿せかし１聞いた){
		@rp_PS_ＮＰＣスネーク＿せかし１
		eval($f:rfp_PS_ＮＰＣスネーク＿せかし１聞いた = 1)
	
	}else if(!$f:rfp_PS_ＮＰＣスネーク＿せかし２聞いた){
		@rp_PS_ＮＰＣスネーク＿せかし２
		eval($f:rfp_PS_ＮＰＣスネーク＿せかし２聞いた = 1)

	}else if(!$f:rfp_PS_ＮＰＣスネーク＿せかし３聞いた){
		@rp_PS_ＮＰＣスネーク＿せかし３
		eval($f:rfp_PS_ＮＰＣスネーク＿せかし３聞いた = 1)

	}else if(!$f:rfp_PS_ＮＰＣスネーク＿せかし４聞いた){
		@rp_PS_ＮＰＣスネーク＿せかし４
		eval($f:rfp_PS_ＮＰＣスネーク＿せかし４聞いた = 1)

	}else {
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rp_PS_ＮＰＣスネーク＿せかし１

		}else if($w:rfp_rand < 2){
			@rp_PS_ＮＰＣスネーク＿せかし２

		}else if($w:rfp_rand < 3){
			@rp_PS_ＮＰＣスネーク＿せかし３

		}else {
			@rp_PS_ＮＰＣスネーク＿せかし４
		}
	}
}

proc rps_ＮＰＳスネークはげまし {
	if( ( (`%ＮＰＣスネークのライフ`)*100 / 400) <= 30 ){
		@rps_PS_ＮＰＣスネーク＿弱音
	
	}else if(!$f:rfp_PS_ＮＰＣスネーク＿はげまし１聞いた) {
		@rp_PS_ＮＰＣスネーク＿はげまし１
		eval($f:rfp_PS_ＮＰＣスネーク＿はげまし１聞いた = 1 )
	
	}else if(!$f:rfp_PS_ＮＰＣスネーク＿はげまし２聞いた) {
		@rp_PS_ＮＰＣスネーク＿はげまし２
		eval($f:rfp_PS_ＮＰＣスネーク＿はげまし２聞いた = 1 )

	}else if(!$f:rfp_PS_ＮＰＣスネーク＿はげまし３聞いた) {
		@rp_PS_ＮＰＣスネーク＿はげまし３
		eval($f:rfp_PS_ＮＰＣスネーク＿はげまし３聞いた = 1 )

	}else if(!$f:rfp_PS_ＮＰＣスネーク＿はげまし４聞いた) {
		@rp_PS_ＮＰＣスネーク＿はげまし４
		eval($f:rfp_PS_ＮＰＣスネーク＿はげまし４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PS_ＮＰＣスネーク＿はげまし１

		}else if($w:rfp_rand < 2){
			@rp_PS_ＮＰＣスネーク＿はげまし２

		}else if($w:rfp_rand < 3){
			@rp_PS_ＮＰＣスネーク＿はげまし３
		
		}else {
			@rp_PS_ＮＰＣスネーク＿はげまし４
		}
	}
}

proc rps_ＮＰＣスネーク意識なし {
	@rp_PS_ＮＰＣスネーク意識なし
}

proc rps_スネークといっしょ天狗通路Ｂ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	command コントロール座標取得 ＮＰＣスネーク $i:ＮＰＣスネークＸ座標 $i:ＮＰＣスネークＹ座標 $i:ＮＰＣスネークＺ座標
	//ダメージモーション中
	if( (`%ＮＰＣスネークの状態` == 8) || (`%ＮＰＣスネークの状態` == 3) ){
		@rps_ＮＰＣスネークダメージ

	}else if( (`%ＮＰＣスネークの状態` == 2) || (`%ＮＰＣスネークの状態` == 4) ){
			@rps_ＮＰＣスネーク意識なし

	}else if( ( ( (`%ＮＰＣスネークのライフ`)*100 / 400) <= 30) && \
				(!$f:rfp_PS_ＮＰＣスネーク＿弱音１聞いた) ){
		@rps_PS_ＮＰＣスネーク＿弱音
		eval($f:rfp_PS_ＮＰＣスネーク＿弱音１聞いた = 1)

	}else if( ( ($i:ＮＰＣスネークＺ座標 - $i:プレイヤー位置Ｚ) <= -2000) && \
				(!$f:rfp_PS_ＮＰＣスネーク＿せかし１聞いた) ){
		@rps_ＮＰＣスネーク＿せかし
		if($i:ＮＰＣスネークＸ座標){}	//わーにんぐけしよう
		if($i:ＮＰＣスネークＹ座標){}	//わーにんぐけしよう
		eval($f:rfp_PS_ＮＰＣスネーク＿せかし１聞いた = 1)

	}else if(!$f:rfp_PS_ＮＰＣスネーク＿天狗通路Ｂ＿１聞いた) {
		@rp_PS_ＮＰＣスネーク＿天狗通路Ｂ＿１
		eval($f:rfp_PS_ＮＰＣスネーク＿天狗通路Ｂ＿１聞いた = 1 )
	
	}else if(!$f:rfp_PS_ＮＰＣスネーク＿はげまし３聞いた) {
		@rp_PS_ＮＰＣスネーク＿はげまし３
		eval($f:rfp_PS_ＮＰＣスネーク＿はげまし３聞いた = 1 )

	}else if(!$f:rfp_PS_ＮＰＣスネーク＿天狗通路Ｂ＿２聞いた) {
		@rp_PS_ＮＰＣスネーク＿天狗通路Ｂ＿２
		eval($f:rfp_PS_ＮＰＣスネーク＿天狗通路Ｂ＿２聞いた = 1)
	
	}else {
		rand 6
		eval($w:rfp_rand = $status)

		if( ($w:rfp_rand < 2) && ($i:プレイヤー位置Ｚ <= -12000) ){
			@rps_ＮＰＳスネークはげまし

		}else if($w:rfp_rand < 3){
			@rp_PS_ＮＰＣスネーク＿天狗通路Ｂ＿１

		}else if( ($w:rfp_rand < 5) && \
					($i:ＮＰＣスネークＺ座標 - $i:プレイヤー位置Ｚ) <= -2000){
			@rps_ＮＰＣスネーク＿せかし

		}else {
			@rp_PS_ＮＰＣスネーク＿天狗通路Ｂ＿２
		}
	}
}

proc rps_PS_ＮＰＣスネーク＿天狗ラッシュ前 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	command ＮＰＣスネークの状態
	//ダメージモーション中
	if( (`%ＮＰＣスネークの状態` == 8) || (`%ＮＰＣスネークの状態` == 3) ){
		@rps_ＮＰＣスネークダメージ

	}else if( (`%ＮＰＣスネークの状態` == 2) || (`%ＮＰＣスネークの状態` == 4) ){
		@rps_ＮＰＣスネーク意識なし

	}else if( ( `%ＮＰＣスネークの状態` == 5) || ( `%ＮＰＣスネークの状態` == 7) ) {
		@rps_スネークと一緒ライデンに反撃中

	}else if(`%ＮＰＣスネークの状態` == 6){
		@rps_眠りプリスキン＿その他

	}else if( ( ( (`%ＮＰＣスネークのライフ`)*100 / 400) <= 30) && \
				(!$f:rfp_PS_ＮＰＣスネーク＿弱音１聞いた) ){
		@rps_PS_ＮＰＣスネーク＿弱音
		eval($f:rfp_PS_ＮＰＣスネーク＿弱音１聞いた = 1)

	}else {
		if($s:エリア == "w45a"){
			@rp_PS_ＮＰＣスネーク＿天狗ラッシュ前
		}else {
			@rp_PS_ＮＰＣスネーク＿天狗通路Ｂ＿１
		}
	}
}

proc rps_ＮＰＣスネークはげまし天狗ラッシュ {
	if( ( (`%ＮＰＣスネークのライフ`)*100 / 400) <= 30 ){
		@rps_PS_ＮＰＣスネーク＿弱音

	}else if(!$f:rfp_PS_ＮＰＣスネーク＿はげまし１聞いた) {
		@rp_PS_ＮＰＣスネーク＿はげまし１
		eval($f:rfp_PS_ＮＰＣスネーク＿はげまし１聞いた = 1 )

	}else if(!$f:rfp_PS_ＮＰＣスネーク＿はげまし３聞いた) {
		@rp_PS_ＮＰＣスネーク＿はげまし３
		eval($f:rfp_PS_ＮＰＣスネーク＿はげまし３聞いた = 1 )

	}else {
		rand 2
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PS_ＮＰＣスネーク＿はげまし１

		}else {
			@rp_PS_ＮＰＣスネーク＿はげまし３
		}
	}
}

proc rps_スネークといっしょ天狗ラッシュ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	//スネークＬＩＦＥ、ダメージ中、せかすＸＸＸＸＸＸＸ
	//ダメージモーション中
	if( (`%ＮＰＣスネークの状態` == 8) || (`%ＮＰＣスネークの状態` == 3) ){
		@rps_ＮＰＣスネークダメージ

	}else if( (`%ＮＰＣスネークの状態` == 2) || (`%ＮＰＣスネークの状態` == 4) ){
		@rps_ＮＰＣスネーク意識なし

	}else if(!$f:rfp_PS_天狗ラッシュ聞いた) {
		@rp_PS_天狗ラッシュ
		eval($f:rfp_PS_天狗ラッシュ聞いた = 1)

	}else if( ( ( (`%ＮＰＣスネークのライフ`)*100 / 400) <= 30) && \
				(!$f:rfp_PS_ＮＰＣスネーク＿弱音１聞いた) ){
		@rps_PS_ＮＰＣスネーク＿弱音
		eval($f:rfp_PS_ＮＰＣスネーク＿弱音１聞いた = 1)

	}else if(!$f:rfp_PS_ＮＰＣスネーク＿はげまし３聞いた) {
		@rp_PS_ＮＰＣスネーク＿はげまし３
		eval($f:rfp_PS_ＮＰＣスネーク＿はげまし３聞いた = 1 )

	
	}else {
		rand 2
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PS_天狗ラッシュ
		}else {
			@rps_ＮＰＣスネークはげまし天狗ラッシュ
		}
	}
}


proc rps_PS_狙撃後＿エマ傷 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_狙撃後＿エマ傷１
	@rp_PS_狙撃後＿エマ傷２
}

proc rps_PS_フォックスダイ {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PS_フォックスダイ
}

proc rps_PS_エマ救出後＿エマいたずら {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PS_エマ救出後＿エマいたずら
}

block codec RPS_おやすみプリスキン 14180 d:ピーター＿プリスキン {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	@rps_眠りプリスキン
}

proc rps_プリスキンデフォルト {
	if(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
			$w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１開始) {

		eval($f:rfp_PS_爆弾解体中プリスキン聞いた = 1)
		if(($s:エリア == "w21a") && \
			($f:rfp_w21a_地雷原１無線デモ１した) && \
			($w:アイテム数Ｒ[d:アイテム:地雷探知器] <= 0) && \
			(`@rp_w21_地雷地帯にクレイモアあります`) && \
			(!$f:rfp_PS_地雷探知器のありか聞いた) ){
			@rps_PS_地雷探知器のありか
			eval($f:rfp_PS_地雷探知器のありか聞いた = 1)

		}else if( ($w:p_story < d:ST:P015_01_R1爆弾解体最初の一つ１無線デモ１終了) && \
			($f:w17a_ダンボール男フラグ) && \
			($s:エリア != w17a) && \
			( (!$f:rfp_箱男見た) || \
				( ($f:rfp_箱男見た) && ( ( ($i:プレイタイム - $i:rfp_箱男退場時刻 ) / d:FRAME_RATE) >= 180) ) ) && \
				(!$f:rfp_PS_爆弾解体中＿催促１聞いた) ){
			@rps_PS_爆弾解体中＿催促１
			eval($f:rfp_PS_爆弾解体中＿催促１聞いた = 1)
		
		}else if( (`@rp_爆弾解体脚数` == 1) && (!$f:rfp_PS_爆弾解体報告１聞いた) ){
			@rps_PS_爆弾解体報告１
			eval($f:rfp_PS_爆弾解体報告１聞いた = 1 )

		}else if( (`@rp_爆弾解体脚数` == 2) && (!$f:rfp_PS_爆弾解体報告２聞いた) ){
			@rps_PS_爆弾解体報告２
			eval($f:rfp_PS_爆弾解体報告２聞いた = 1 )

		}else if( (`@rp_爆弾解体脚数` == 3) && (!$f:rfp_PS_爆弾解体報告４聞いた) ){
			@rps_PS_爆弾解体報告４
			eval($f:rfp_PS_爆弾解体報告４聞いた = 1 )

		}else if( (`@rp_爆弾解体脚数` == 4) && (!$f:rfp_PS_爆弾解体報告３聞いた) ){
			@rps_PS_爆弾解体報告３
			eval($f:rfp_PS_爆弾解体報告３聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 3
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 2){
				@rps_爆弾解体助言

			}else {
				@rps_デフォルト操作説明
			}

		//連続無線
		}else {
			@rps_連続無線デフォルト
		}

	}else if(d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１開始) {
		eval($f:rfp_PS_爆弾解体中プリスキン聞いた = 1)

		if(!$f:rfp_PS_Ｈ脚底部無臭Ｃ４聞いた) {
			@rps_PS_Ｈ脚底部無臭Ｃ４
			eval($f:rfp_PS_Ｈ脚底部無臭Ｃ４聞いた = 1)
		
		}else if( (!$f:rfp_PS_ピーターは歩ける聞いた) && \
					((($i:プレイタイム - $i:rfp_ピーター出撃開始時刻) / d:FRAME_RATE) >= 180) ){
			@rps_PS_ピーターは歩ける
			eval($f:rfp_PS_ピーターは歩ける聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 1){
				@rps_PS_Ｈ脚底部無臭Ｃ４

			}else if( ($w:rfp_rand < 2)  && \
					((($i:プレイタイム - $i:rfp_ピーター出撃開始時刻) / d:FRAME_RATE) >= 180) ){
				@rps_PS_ピーターは歩ける

			}else if($w:rfp_rand < 3){
				@rps_爆弾解体助言

			}else {
				@rps_デフォルト操作説明
			}

		//連続無線
		}else {
			@rps_連続無線デフォルト
		}


	}else if(d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１開始 ) {

		//せんさＢなひ
		if( ($w:アイテム数Ｒ[11] <= 0) && (!$f:rfp_PS_センサーＢ取に行け聞いた) ){
			@rps_PS_センサーＢ取に行け聞いた
			eval($f:rfp_PS_センサーＢ取に行け聞いた = 1)
		
		//せんさＢとってるヒネクレもん
		}else if( ($w:アイテム数Ｒ[11] > 0) && (!$f:rfp_PS_Ａ脚底部に行け聞いた) ){
			@rps_PS_Ａ脚底部に行け
			eval($f:rfp_PS_Ａ脚底部に行け聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				if($w:アイテム数Ｒ[11] <= 0){
					@rps_PS_センサーＢ取に行け聞いた
				}else {
					@rps_PS_Ａ脚底部に行け
				}

			}else {
				@rps_デフォルト操作説明
			}

		//連続無線
		}else {
			@rps_連続無線デフォルト
		}


	}else if(d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１開始 ) {

		if( !$f:rfp_PS_Ａ脚底部に行け聞いた){
			@rps_PS_Ａ脚底部に行け
			eval($f:rfp_PS_Ａ脚底部に行け聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 4){
				@rps_PS_Ａ脚底部に行け

			}else {
				@rps_デフォルト操作説明
			}

		//連続無線
		}else {
			@rps_連続無線デフォルト
		}

	//以降、ファットマン倒すまでスネークさん退場。

	}else if(d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P038_01_S01網膜チェック１シナリオデモ１開始) {

		//なんかもーどーでもいいことしかいわない。
		@rps_連続無線デフォルト

	}else if(d:ST:P042_01_R01シェル１シェル２連絡橋１無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:シェル１シェル２連絡橋赤外線センサークリア) {

		if($s:エリア == "w25a") {
			//ＰＳＧ１ない
			if($w:武器弾数Ｒ[4] < 0){
				@rps_PS_ＤＧ連絡橋＿ＰＳＧ１ない
				eval($f:rpf_PS_スネークＰＳＧ１場所聞いた = 1)
			}else {
				@rps_狙撃イベント中
			}

		}else {
			if(!$f:rfp_PS_ＤＧ連絡橋以外聞いた){
				@rps_PS_ＤＧ連絡橋以外
				eval($f:rfp_PS_ＤＧ連絡橋以外聞いた = 1)

			//無線一回目
			}else if($w:rfp_callcount <= 1){
				rand 5
				eval($w:rfp_rand = $status)

				if($w:rfp_rand < 4){
					@rps_PS_ＤＧ連絡橋以外

				}else {
					@rps_デフォルト操作説明
				}

			//連続無線
			}else {
				rand 5
				eval($w:rfp_rand = $status)

				if($w:rfp_rand < 2){
					@rps_PS_ＤＧ連絡橋以外
				}else {
					@rps_連続無線デフォルト
				}
			}
		}

/*
	}else if(d:ST:P045_01_P01ハリアー登場１ポリゴンデモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P046_01_P01ハリアー戦勝利１ポリゴンデモ１開始) {
		@rps_ハリアー戦中
*/

	}else if(d:ST:P046_02_R01ハリアー戦勝利２無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P047_01_P01電撃床前オルガ１ポリゴンデモ１開始) {

		if(!$f:rfp_PS_ハリアー戦後＿大統領救出せよ聞いた) {
			@rps_PS_ハリアー戦後＿大統領救出せよ
			eval($f:rfp_PS_ハリアー戦後＿大統領救出せよ聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rps_PS_ハリアー戦後＿大統領救出せよ

			}else {
				@rps_デフォルト操作説明
			}

		//連続無線
		}else {
			@rps_連続無線デフォルト
			//PS_イロコイ語源なろ。
		}

	}else if(d:ST:P047_05_R01電撃床前オルガ５無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P049_12_R05大統領１２無線機デモ５終了) {

		//リモコンないよー
		if( ($w:武器弾数Ｒ[6] < 0 ) && (!$f:rfp_PS_リモコンミサイルの場所聞いた)  ){
			@rps_PS_リモコンミサイルの場所
			eval($f:rfp_PS_リモコンミサイルの場所聞いた = 1)

		}else if( ($s:エリア == "w31b") && (!$f:w31b_ノードフラグ) && \
					(`%プレイヤー状態取得` & d:PFLAG_WATER) && \
					(!$f:rfp_PS_シェル２中央棟Ｂ１＿水中レーダー使え聞いた) ){
			@rps_PS_シェル２中央棟Ｂ１＿水中レーダー使え
			eval($f:rfp_PS_シェル２中央棟Ｂ１＿水中レーダー使え聞いた = 1)
		
		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)
			if( ($w:rfp_rand < 2) && \
				($s:エリア == "w31b") && (!$f:w31b_ノードフラグ) && \
					(`%プレイヤー状態取得` & d:PFLAG_WATER) ){
				@rps_PS_シェル２中央棟Ｂ１＿水中レーダー使え

			}else if( ($w:rfp_rand < 4) && ($w:武器弾数Ｒ[6] < 0 ) ){
				@rps_PS_リモコンミサイルの場所

			}else {
				@rps_デフォルト操作説明
			}

		//連続無線
		}else {
			@rps_連続無線デフォルト
			//PS_イロコイ語源なろ。
		}

	}else if(d:ST:P049_12_R05大統領１２無線機デモ５終了 <= $w:p_story && \
			$w:p_story < d:ST:P055_01_P01ヴァンプ戦前１ポリゴンデモ１開始) {
		if( ($s:エリア == "w31b") && (!$f:rfp_PS_シェル２中央棟Ｂ１エマ助けに行け聞いた) ){
			@rps_シェル２中央棟Ｂ１エマ助けに行け
			eval($f:rfp_PS_シェル２中央棟Ｂ１エマ助けに行け聞いた = 1)

		}else if(!$f:rfp_rp_PS_シェル２中央棟Ｂ１＿エマ居場所１聞いた) {
			@rps_PS_シェル２中央棟Ｂ１＿エマ居場所１
			eval($f:rfp_rp_PS_シェル２中央棟Ｂ１＿エマ居場所１聞いた = 1)

		}else if( ($s:エリア == "w31b") && (!$f:w31b_ノードフラグ) && \
					(`%プレイヤー状態取得` & d:PFLAG_WATER) && \
					(!$f:rfp_PS_シェル２中央棟Ｂ１＿水中レーダー使え聞いた) ){
			@rps_PS_シェル２中央棟Ｂ１＿水中レーダー使え
			eval($f:rfp_PS_シェル２中央棟Ｂ１＿水中レーダー使え聞いた = 1)

		}else {
			//無線一回目
			if($w:rfp_callcount <= 1){
				rand 5
				eval($w:rfp_rand = $status)
				if( ($w:rfp_rand < 2) && \
					($s:エリア == "w31b") && (!$f:w31b_ノードフラグ) && \
						(`%プレイヤー状態取得` & d:PFLAG_WATER) ){
					@rps_PS_シェル２中央棟Ｂ１＿水中レーダー使え

				}else if($w:rfp_rand < 4){
					if($s:エリア == "w31b"){
						@rps_シェル２中央棟Ｂ１エマ助けに行け
					}else {
						@rps_PS_シェル２中央棟Ｂ１＿エマ居場所１
					}

				}else {
					@rps_デフォルト操作説明
				}
			//連続無線
			}else {
				@rps_連続無線デフォルト
				//PS_イロコイ語源なろ。
			}
		}

	}else if(d:ST:P055_04_P03ヴァンプ戦前４ポリゴンデモ３終了 <= $w:p_story && \
			$w:p_story < d:ST:P057_01_P01ヴァンプ戦終了１ポリゴンデモ１開始) {
		@rps_ヴァンプ戦中


	}else if(d:ST:P057_02_R01ヴァンプ戦終了２無線機デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P058_01_P01エマ救出１ポリゴンデモ１開始) {
		command プレイヤー状態取得
		eval($i:rfp_プレイヤー状態 = $status)

		if( ($s:エリア == "w31f") && ($f:rfp_w31c_エマ部屋にいまそかり) && \
			(!$f:rfp_PS_ヴァンプ戦後＿エマ助けろ＿ロッカー部屋聞いた) ){
				@rps_PS_ヴァンプ戦後＿エマ助けろ＿ロッカー部屋
				eval($f:rfp_PS_ヴァンプ戦後＿エマ助けろ＿ロッカー部屋聞いた = 1)

		}else if( ( ($s:エリア == "w31f") && ( $i:rfp_プレイヤー状態 & d:PFLAG_WATER) ) && \
				(!$f:rfp_PS_ヴァンプ戦後＿エマ助けろ濾過槽北水中聞いた) ){
			@rps_PS_ヴァンプ戦後＿エマ助けろ濾過槽北水中
			eval($f:rfp_PS_ヴァンプ戦後＿エマ助けろ濾過槽北水中聞いた = 1)

		}else if(!$f:rfp_PS_ヴァンプ戦後＿エマ助けろ濾過槽以外聞いた){
			@rps_PS_ヴァンプ戦後＿エマ助けろ濾過槽以外
			eval($f:rfp_PS_ヴァンプ戦後＿エマ助けろ濾過槽以外聞いた = 1)

		}else if(!$f:rfp_PS_ヴァンプ戦後＿純粋水爆聞いた) {
			@rps_PS_ヴァンプ戦後＿純粋水爆
			eval($f:rfp_PS_ヴァンプ戦後＿純粋水爆聞いた = 1)

		

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 4
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				if( ($s:エリア == "w31f") && ( $i:rfp_プレイヤー状態 & d:PFLAG_WATER) ) {
					@rps_PS_ヴァンプ戦後＿エマ助けろ濾過槽北水中

				}else if( ($s:エリア == "w31f") && ($f:rfp_w31c_エマ部屋にいまそかり) ){
					@rps_PS_ヴァンプ戦後＿エマ助けろ＿ロッカー部屋

				}else {
					@rps_PS_ヴァンプ戦後＿エマ助けろ濾過槽以外
				}

			}else {
				@rps_デフォルト操作説明
			}

		//連続無線
		}else {
			@rps_連続無線デフォルト
			//PS_イロコイ語源なろ。
			//PS_ヴァンプ戦後＿純粋水爆??
		}


	}else if(d:ST:P057_02_R01ヴァンプ戦終了２無線機デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P058_01_P01エマ救出１ポリゴンデモ１開始) {

		if(!$f:rfp_PS_ヴァンプ戦後＿純粋水爆聞いた) {
			@rps_PS_ヴァンプ戦後＿純粋水爆
			eval($f:rfp_PS_ヴァンプ戦後＿純粋水爆聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
				@rps_デフォルト操作説明

		//連続無線
		}else {
			@rps_連続無線デフォルト
			//PS_イロコイ語源なろ。
			//PS_ヴァンプ戦後＿純粋水爆??
		}


	}else if(d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了 <= $w:p_story && \
			$w:p_story < d:ST:P063_01_P01カード五１ポリゴンデモ１開始) {
		command ゲットエマ状態 $b:rfp_ゲットされたエマ状態

		if($f:rfp_PS_プリスキン連絡途絶しましたん) {
			@rps_PS_エマ救出後＿通信途絶中
		
		}else if( (`@rp_エマにセクハラ中`) || \
					($b:rfp_ゲットされたエマ状態 == d:EMA_F_I_AM_CAPTURED ) || \
					($b:rfp_ゲットされたエマ状態 == d:EMA_F_ATTKED_BY_PLAYER ) ){
			@rps_PS_エマ救出後＿エマいたずら
		
		}else if(!$f:rfp_PS_エマ救出後＿Ｌ脚行け聞いた) {
			@rps_PS_エマ救出後＿Ｌ脚行け
			eval($f:rfp_PS_エマ救出後＿Ｌ脚行け聞いた = 1)

		}else if( ($s:エリア == "w25d") && (!$f:rfp_PS_エマ救出後＿通信途絶前聞いた) ){
			@rps_PS_エマ救出後＿通信途絶前
			eval($f:rfp_PS_エマ救出後＿通信途絶前聞いた = 1 )
			eval($f:rfp_PS_プリスキン連絡途絶しましたん = 1 )


		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 4){
				@rps_PS_エマ救出後＿Ｌ脚行け

			}else {
				@rps_デフォルト操作説明
			}

		//連続無線
		}else {
			@rps_連続無線デフォルト
		}


	}else if(d:ST:P065_06_R02Ｌ脚エマ６無線機デモ２終了 <= $w:p_story && \
			$w:p_story < d:ST:P067_01_R01エマ狙撃１無線機デモ１開始) {
		//狙撃中援護可能になる前
		@rps_エマ狙撃＿到着してない

	}else if(d:ST:P067_01_R01エマ狙撃１無線機デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P068_01_P01ヴァンプ狙撃前１ポリゴンデモ１開始) {
		@rps_エマ狙撃＿援護

	}else if(d:ST:P068_01_P01ヴァンプ狙撃前１ポリゴンデモ１開始 <= $w:p_story && \
			$w:p_story < d:ST:P069_01_P01ヴァンプ狙撃終了１ポリゴンデモ１開始) {
		@rpd_ライデン顔設定デフォルト
		@rpd_プリスキン顔設定デフォルト
		@rpd_顔表示

		@rps_眠りプリスキン＿その他

	}else if(d:ST:P069_04_R02ヴァンプ狙撃終了４無線機デモ２終了 <= $w:p_story && \
			$w:p_story < d:ST:P070_01_P01ＡＧ起動１ポリゴンデモ１開始) {

		//時間で催促ＸＸＸＸＸＸＸＸＸｘPS_エマ狙撃後＿催促

		if(!$f:rfp_PS_狙撃後＿電算室来い聞いた){
			@rps_PS_狙撃後＿電算室来い
			eval($f:rfp_PS_狙撃後＿電算室来い聞いた = 1 )
		
		}else if(!$f:rfp_PS_狙撃後＿敵いない聞いた) {
			@rps_PS_狙撃後＿敵いない
			eval($f:rfp_PS_狙撃後＿敵いない聞いた = 1 )
		
		}else if(!$f:rfp_PS_狙撃後＿開かない扉聞いた) {
			@rps_PS_狙撃後＿開かない扉
			eval($f:rfp_PS_狙撃後＿開かない扉聞いた = 1)

		}else if(!$f:rfp_PS_狙撃後＿エマ傷聞いた) {
			@rps_PS_狙撃後＿エマ傷
			eval($f:rfp_PS_狙撃後＿エマ傷聞いた = 1)

		//無線一回目
		}else if( ($w:rfp_callcount > 1) && \
					(!$f:rfp_PS_フォックスダイ聞いた) ){
			@rps_PS_フォックスダイ
			eval($f:rfp_PS_フォックスダイ聞いた = 1)

		}else {
			rand 3
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 1){
				@rps_PS_狙撃後＿電算室来い

			}else if($w:rfp_rand < 2){
				@rps_PS_狙撃後＿敵いない

			}else{
				@rps_PS_狙撃後＿開かない扉
			}
		}
	
	//以降ハチマキ
	}else if(d:ST:P073_02_P01通路Ａ２ポリゴンデモ１終了 <= $w:p_story && \
				$w:p_story < d:ST:P074_01_P01通路Ａ刀後１ポリゴンデモ１開始) {
		@rps_刀練習

	}else if(d:ST:P074_03_P02通路Ａ刀後３ポリゴンデモ２終了 <= $w:p_story && \
				$w:p_story < d:ST:P077_02_P01天狗兵降下ラッシュポリゴンデモ１開始) {

		if($s:エリア == "w45a" ){
			@rps_PS_ＮＰＣスネーク＿天狗ラッシュ前
		}else {
			if($w:Ｓローカル変数Ｗ１ >= 1){
				@rps_PS_ＮＰＣスネーク＿天狗ラッシュ前
			}else {
				@rps_スネークといっしょ天狗通路Ｂ
			}
		}
		

	}else if(d:ST:P077_02_P01天狗兵降下ラッシュポリゴンデモ１終了 <= $w:p_story && \
				$w:p_story < d:ST:P078_01_P01ＡＧフォーチュン登場１ポリゴンデモ１開始) {
			@rps_スネークといっしょ天狗ラッシュ





	}else {
		@rps_PS_ばぐっち
//		@rps_デフォルト操作説明
	}
}



block codec RPS_プリスキン＿ピーターデフォルト 14180 d:ピーター＿プリスキン {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	@rps_プリスキンデフォルト
}



block codec RPS_プリスキン＿オタコンデフォルト 14180 d:プリスキン無線１ {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	@rps_プリスキンデフォルト
}


block codec RPS_プリスキン＿オタコン＿ハリアー戦中 14180 d:プリスキン無線１ {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	@rps_ハリアー戦中
}

block codec RPS_プリスキン＿オタコンデフォルト＿ゴルルコ兵マスクあり 14180 d:プリスキン無線２＿ゴルルコ兵マスクあり {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	@rps_プリスキンデフォルト
}

block codec RPS_スネークデフォルト 14180 d:スネーク無線１ {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	@rps_プリスキンデフォルト
}


block codec RPS_Ｅ脚ハシゴ 14180 d:プリスキン無線１ {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	if(!$f:rfp_PS_狙撃後＿開かない扉聞いた){
		@rps_PS_狙撃後＿開かない扉
		eval($f:rfp_PS_狙撃後＿開かない扉聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rps_PS_狙撃後＿開かない扉
		}else {
			@rps_プリスキンデフォルト
		}

	//連続無線
	}else {
		@rps_プリスキンデフォルト
	}
}

proc rps_w21a_地雷地帯 {
	if( ($f:rfp_w21a_地雷原１無線デモ１した) && \
		(!$f:rfp_PS_地雷探知器のありか聞いた) && \
		(`@rp_w21_地雷地帯にクレイモアあります`) ){
		@rps_PS_地雷探知器のありか
		eval($f:rfp_PS_地雷探知器のありか聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && ($f:rfp_w21a_地雷原１無線デモ１した) && \
			(`@rp_w21_地雷地帯にクレイモアあります`) ){
			@rps_PS_地雷探知器のありか
		}else {
			@rps_プリスキンデフォルト
		}

	//連続無線
	}else {
		@rps_プリスキンデフォルト
	}
}

block codec RPS_w21a_地雷地帯＿ピーター付き 14180 d:ピーター＿プリスキン {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	@rps_w21a_地雷地帯
}

block codec RPS_w21a_地雷地帯＿オタコン付き 14180 d:プリスキン無線１ {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	@rps_w21a_地雷地帯
}

proc rps_PS_シェル２中央棟＿水中機雷 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_シェル２中央棟＿水中機雷１
	if( (!$f:rfp_PS_シェル２中央棟＿水中機雷聞いた) && \
		($w:p_story < d:ST:P055_01_P01ヴァンプ戦前１ポリゴンデモ１開始) ){
		@rp_PS_シェル２中央棟＿水中機雷２
		eval($f:rfp_PS_シェル２中央棟＿水中機雷聞いた = 1 )
	}
}


proc rps_PS_水中機雷＿うんちく {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_水中機雷＿うんちく
}

block codec RPS_水中機雷近く 14180 d:プリスキン無線１ {
	@rp_無線デフォルト前処理＿スネーク
	if( (!$f:rfp_PS_シェル２中央棟＿水中機雷聞いた) && (!$f:rfp_そこの機雷爆発した) ){
		@rps_PS_シェル２中央棟＿水中機雷
		eval($f:rfp_PS_シェル２中央棟＿水中機雷聞いた = 1 )

	}else if( (!$f:rfp_PS_水中機雷＿うんちく聞いた) && (!$f:rfp_そこの機雷爆発した) ){
		@rps_PS_水中機雷＿うんちく
		eval($f:rfp_PS_水中機雷＿うんちく聞いた = 1)

	//無線一回目
	}else{
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_callcount <= 1) && \
			($w:rfp_rand < 3) && (!$f:rfp_そこの機雷爆発した) ){
			@rps_PS_シェル２中央棟＿水中機雷
		}else {
			@rps_プリスキンデフォルト
		}
	}
}

proc rps_PS_Ｅ脚１Ｆ＿荷物昇降機 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_Ｅ脚１Ｆ＿荷物昇降機１
	if(!$f:rfp_PS_Ｅ脚１Ｆ＿荷物昇降機聞いた){
		@rp_PS_Ｅ脚１Ｆ＿荷物昇降機２
		eval($f:rfp_PS_Ｅ脚１Ｆ＿荷物昇降機聞いた = 1 )
	}
}

block codec RPS_w20a_べるこんのりば 14180 d:ピーター＿プリスキン {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	if(!$f:rfp_PS_Ｅ脚１Ｆ＿荷物昇降機聞いた){
		@rps_PS_Ｅ脚１Ｆ＿荷物昇降機
		eval($f:rfp_PS_Ｅ脚１Ｆ＿荷物昇降機聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( $w:rfp_rand < 3){
			@rps_PS_Ｅ脚１Ｆ＿荷物昇降機
		}else {
			@rps_プリスキンデフォルト
		}

	//連続無線
	}else {
		@rps_プリスキンデフォルト
	}
}

proc rps_PS_Ｃ脚女子トイレ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_Ｃ脚女子トイレ
}

block codec RPS_女子便所 14180 d:ピーター＿プリスキン {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	if(!$f:rfp_PS_Ｃ脚女子トイレ聞いた){
		@rps_PS_Ｃ脚女子トイレ
		eval($f:rfp_PS_Ｃ脚女子トイレ聞いた = 1 )

	}else {
		@rps_プリスキンデフォルト
	}
}

proc rps_PS_ピーター死体 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ピーター死体
}

block codec RPS_ピーター死体 14180 d:プリスキン無線１ {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	if(!$f:rfp_PS_ピーター死体聞いた){
		@rps_PS_ピーター死体
		eval($f:rfp_PS_ピーター死体聞いた = 1 )

	}else {
		@rps_プリスキンデフォルト
	}
}

block codec RPS_w31c_プール近く 14180 d:プリスキン無線１ {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	if(!$f:rfp_PS_シェル２中央棟＿濾過装置うんちく聞いた){
		@rps_PS_シェル２中央棟＿濾過装置うんちく
		eval($f:rfp_PS_シェル２中央棟＿濾過装置うんちく聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( $w:rfp_rand < 3){
			@rps_PS_シェル２中央棟＿濾過装置うんちく
		}else {
			@rps_プリスキンデフォルト
		}

	//連続無線
	}else {
		@rps_プリスキンデフォルト
	}
}

proc rps_PS_サイファー {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	if(!$f:rfp_PS_サイファー聞いた){
		@rp_PS_サイファー１
		eval($f:rfp_PS_サイファー聞いた = 1)
	}
	@rp_PS_サイファー２
}

proc rps_PS_サイファー３ {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_サイファー３
}



block codec RPS_サイファー連絡橋＿デフォルト 14180 d:ピーター＿プリスキン {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	@rp_そこのサイファー存在するか判定
	rand 5
	eval($w:rfp_rand = $status)

	if( ($f:rfp_そこのサイファー存在してます) && \
		(!$f:rfp_PS_サイファー聞いた) ){
		@rps_PS_サイファー
		eval($f:rfp_PS_サイファー聞いた = 1)

	}else if( ($f:rfp_そこのサイファー存在してます) && \
				(!$f:rfp_PS_サイファー３聞いた) ){
		@rps_PS_サイファー３
		eval($f:rfp_PS_サイファー３聞いた = 1)

	//無線一回目
	}else if( ($w:rfp_callcount <= 1) && ($w:rfp_rand < 2) && ($f:rfp_そこのサイファー存在してます) ){
			@rps_PS_サイファー

	}else if( ($w:rfp_callcount <= 1) && ($w:rfp_rand < 2) && ($f:rfp_そこのサイファー存在してます) ){
			@rps_PS_サイファー３

	//連続無線
	}else {
		@rps_連続無線デフォルト
	}
}

proc rps_PS_狙撃後＿開かない扉場所依存 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_狙撃後＿開かない扉
	if($s:エリア == "w20a"){
		if($i:プレイヤー位置Ｘ <= 50000){
			eval($f:rfp_w20a_ヴァンプ狙撃後開かない扉ＤＥ側聞いた = 1)
		}else {
			eval($f:rfp_w20a_ヴァンプ狙撃後開かない扉屋上側聞いた = 1)
		}
	}else if($s:エリア == "w21b"){
		eval($f:rfp_w21b_ヴァンプ狙撃後開かない扉聞いた = 1)
	}
}


block codec RPS_ヴァンプ狙撃以降開かない扉 14180 d:プリスキン無線１ {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	if(!$f:rfp_PS_狙撃後＿開かない扉聞いた){
		@rps_PS_狙撃後＿開かない扉場所依存
		eval($f:rfp_PS_狙撃後＿開かない扉聞いた = 1 )
	
	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rps_PS_狙撃後＿開かない扉場所依存

		}else {
			@rps_プリスキンデフォルト
		}

	}else {
		@rps_プリスキンデフォルト
	}
}

proc rps_PS_Ｅ脚＿エマ血痕 {
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_Ｅ脚＿エマ血痕
}

block codec RPS_w20a_エマ血痕 14180 d:プリスキン無線１ {
	@rp_無線デフォルト前処理＿スネーク
	@rp_短縮ダイアル＿プリスキンセット
	if( (!$f:rfp_PS_Ｅ脚＿エマ血痕聞いた) && ($f:rfp_PC_エマ狙撃後＿エマ血痕聞いた) ){
		@rps_PS_Ｅ脚＿エマ血痕
		eval($f:rfp_PS_Ｅ脚＿エマ血痕聞いた = 1 )

	}else {
		@rps_プリスキンデフォルト
	}
}

block codec RPS_ヴァンプ狙撃以降開かない扉ＣＡＬＬ 14180 d:プリスキン無線１ {
	@rp_無線デフォルト前処理＿スネーク
	@rps_PS_狙撃後＿開かない扉場所依存
}

block codec RPS_w25a_ＰＳＧ１ハマリＣＡＬＬ 14180 d:プリスキン無線１ {
	@rp_無線デフォルト前処理＿スネーク
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ＤＧ連絡橋＿見落とし
	@rp_PS_ＤＧ連絡橋＿サイファーズバリ１
	@rp_PS_ＤＧ連絡橋＿サイファーズバリ２
}


block codec RPS_ＰＳＧ１本体取得時ＣＡＬＬ 14180 d:プリスキン無線１ {
	@rp_無線デフォルト前処理＿スネーク
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ＤＧ連絡橋以外＿ＰＳＧ１ある
}

block codec RPS_ＡＫ本体取得時ＣＡＬＬ 14180 d:プリスキン無線２＿ゴルルコ兵マスクあり {
	@rp_無線デフォルト前処理＿スネーク
	@rpd_ライデン顔設定デフォルト
	@rpd_プリスキン顔設定デフォルト
	@rpd_顔表示

	@rp_PS_ＡＫ入手時ＣＡＬＬ
	@rp_PS_ＡＫ２
	@rp_PS_ＡＫ４
	eval($f:rfp_ＡＫ本体取得時ＣＡＬＬ聞いた = 1)
}


//モーション班用デバッグスクリプトェウァアアアア
#if d:MGS2_SCN
#include "p_snake_dbg.h"
#endif


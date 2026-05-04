//
//	p_peter.cdc
//	プラント編ピーター無線
//
//	2001/06/15	T.Fukushima
//	$Id: p_peter.cdc,v 1.37 2002/06/07 09:53:01 usr01475 Exp $

/* メッセージ定義 */
#define CODEC_FILE 1
#include "vardef.h"			// 武器・アイテム関係のdefine
#include "cdc_proc_p.h"

#include "p_peter_e.cm"
#include "p_peter_e.ct"

#include "p_peter_f.cm"
#include "p_peter_f.ct"

#include "p_peter_g.cm"
#include "p_peter_g.ct"

#include "p_peter_s.cm"
#include "p_peter_s.ct"

#include "p_peter_i.cm"
#include "p_peter_i.ct"

#include "p_peter_j.cm"
#include "p_peter_j.ct"

proc rp_無線デフォルト前処理＿ピーター  {
	@rp_無線デフォルト前処理
	if(`prefreq` != 14025){
		eval($w:rfp_callcount = 1)
	}
}

proc rpp_ばぐっち {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示
	
	@rp_PP_ばぐっち
}

proc rpp_爆弾解体方法 {
	if(!$f:rfp_PP_センサー説明１聞いた) {
		@rp_PP_センサー説明１
		eval($f:rfp_PP_センサー説明１聞いた = 1)
	
	}else if(!$f:rfp_PP_センサー説明２聞いた) {
		@rp_PP_センサー説明２
		eval($f:rfp_PP_センサー説明２聞いた = 1 )

	}else if(!$f:rfp_PP_センサー説明３聞いた) {
		@rp_PP_センサー説明３
		eval($f:rfp_PP_センサー説明３聞いた = 1 )

	}else if(!$f:rfp_PP_冷凍スプレー使い方聞いた) {
		@rp_PP_冷凍スプレー使い方
		eval($f:rfp_PP_冷凍スプレー使い方聞いた = 1 )
	
	}else if(!$f:rfp_PP_冷凍スプレー主観聞いた) {
		@rp_PP_冷凍スプレー主観
		eval($f:rfp_PP_冷凍スプレー主観聞いた = 1)
	
	}else if(!$f:rfp_PP_冷凍スプレー注意聞いた) {
		@rp_PP_冷凍スプレー注意
		eval($f:rfp_PP_冷凍スプレー注意聞いた = 1)

	}else if(!$f:rfp_PP_爆弾１聞いた) {
		@rp_PP_爆弾１
		eval($f:rfp_PP_爆弾１聞いた = 1)

//		eval($f:rfp_PP_爆弾解体方法終了 = 1)

	}else {
		rand 6
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PP_センサー説明１

		}else if($w:rfp_rand < 2){
			@rp_PP_センサー説明２

		}else if($w:rfp_rand < 3){
			@rp_PP_センサー説明３

		}else if($w:rfp_rand < 4){
			@rp_PP_冷凍スプレー使い方

		}else if($w:rfp_rand < 5){
			@rp_PP_冷凍スプレー主観

		}else if($w:rfp_rand < 6){
			@rp_PP_冷凍スプレー注意

		}else {
			@rp_PP_爆弾１
		}
	}
}


proc rpp_えらそうな爆弾警句ズ {
	if(!$f:rfp_PP_爆弾警句１聞いた) {
		@rp_PP_爆弾警句１
		eval($f:rfp_PP_爆弾警句１聞いた = 1 )

	}else if(!$f:rfp_PP_爆弾警句２聞いた) {
		@rp_PP_爆弾警句２
		eval($f:rfp_PP_爆弾警句２聞いた = 1 )

	}else if(!$f:rfp_PP_爆弾警句３聞いた) {
		@rp_PP_爆弾警句３
		eval($f:rfp_PP_爆弾警句３聞いた = 1 )

	}else if(!$f:rfp_PP_爆弾警句４聞いた) {
		@rp_PP_爆弾警句４
		eval($f:rfp_PP_爆弾警句４聞いた = 1 )

	}else if(!$f:rfp_PP_爆弾警句５聞いた) {
		@rp_PP_爆弾警句５
		eval($f:rfp_PP_爆弾警句５聞いた = 1 )

//		eval($f:rfp_PP_えらそうな爆弾警句ズ終了 = 1 )

	}else {
		rand 5
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PP_爆弾警句１

		}else if($w:rfp_rand < 2){
			@rp_PP_爆弾警句２

		}else if($w:rfp_rand < 3){
			@rp_PP_爆弾警句３

		}else if($w:rfp_rand < 4){
			@rp_PP_爆弾警句４
		
		}else {
			@rp_PP_爆弾警句５
		}

	}
}

proc rpp_爆弾処理デフォルト {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	rand 2
	eval($w:rfp_rand = $status)

	if($w:rfp_rand < 1){
		@rpp_爆弾解体方法
	}else {
		@rpp_えらそうな爆弾警句ズ
	}
}


proc rpp_PP_爆弾設置場所不審 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_爆弾設置場所不審
}

proc rpp_PP_ピーターは何をしてる {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_ピーターは何をしてる
}

proc rpp_PP_オタコン {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_オタコン
}

proc rpp_PP_プリスキン {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_プリスキン
}

proc rpp_PP_ピーター過去＿大佐話聞いていない {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_ピーター過去＿大佐話聞いていない
}

proc rpp_PP_ファットマン {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_ファットマン１
	@rp_PP_ファットマン２
	@rp_PP_ファットマン３
	@rp_PP_ファットマン４
}

proc rpp_PP_移動後無臭Ｃ４ {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_移動後無臭Ｃ４
}

proc rpp_PP_歩ける言い訳 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_歩ける言い訳１
	@rp_PP_歩ける言い訳２
	@rp_PP_歩ける言い訳３
}

proc rpp_連続無線爆弾処理デフォルト {
	//んー仮りーん……ＸＸＸＸＸＸＸＸＸＸｘｘ
	if( (d:ST:P015_01_R1爆弾解体最初の一つ１無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P016_01_R1爆弾解体半分解体１無線デモ１開始) && \
		(!$f:rfp_PP_爆弾設置場所不審聞いた) ){
		@rpp_PP_爆弾設置場所不審
		eval($f:rfp_PP_爆弾設置場所不審聞いた = 1)

	}else if( (d:ST:P015_01_R1爆弾解体最初の一つ１無線デモ１終了 <= $w:p_story) && \
				(!$f:rfp_PP_ファットマン聞いた) ){
		@rpp_PP_ファットマン
		eval($f:rfp_PP_ファットマン聞いた = 1)

	}else if( (d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了 <= $w:p_story && \
				$w:p_story < d:ST:P027_01_R01爆弾解体後昇降機ホール１無線デモ１開始) && \
			(!$f:rfp_PP_歩ける言い訳聞いた) ){
		@rpp_PP_歩ける言い訳
		eval($f:rfp_PP_歩ける言い訳聞いた = 1)

	}else if( (d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了 <= $w:p_story && \
				$w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１開始) && \
			(!$f:rfp_PP_移動後無臭Ｃ４聞いた) ){
		@rpp_PP_移動後無臭Ｃ４
		eval($f:rfp_PP_移動後無臭Ｃ４聞いた = 1)

	}else if( (!$f:rfp_PP_ピーター過去＿大佐話聞いていない聞いた) && \
				(!$f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:ピーター詳細]) ){
		@rpp_PP_ピーター過去＿大佐話聞いていない
		eval($f:rfp_PP_ピーター過去＿大佐話聞いていない聞いた = 1)

	}else if( (!$f:rfp_PP_プリスキン聞いた) && \
				(`@rp_爆弾解体脚数` >= 3) ){
			@rpp_PP_プリスキン
			eval($f:rfp_PP_プリスキン聞いた = 1)

	}else if( ($w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１開始) && \
				(!$f:rfp_PP_ピーターは何をしてる聞いた) ){
			@rpp_PP_ピーターは何をしてる
			eval($f:rfp_PP_ピーターは何をしてる聞いた = 1)

	}else if( (!$f:rfp_PP_オタコン聞いた) && \
				(`@rp_爆弾解体脚数` >= 2) ){
			@rpp_PP_オタコン
			eval($f:rfp_PP_オタコン聞いた = 1)


	}else {
		@rpp_爆弾処理デフォルト
	}
}


proc rpp_爆弾評価＿時間＿中身 $:ちょっぱや秒 $:普通秒小 $:普通秒大 $:超過秒 {
	//時間ちょっぱや
	if( $i:rfp_爆弾解体時フレーム数 < ( $:ちょっぱや秒 * d:FRAME_RATE) ){
		@rp_PP_爆弾評価＿時間超早い
		@rpd_ピーター顔表示
		eval($f:rfp_PP_爆弾評価した = 1)
		eval($f:rfp_PP_爆弾評価＿時間した = 1)

	//時間早い ２分
	}else if( $i:rfp_爆弾解体時フレーム数 < ($:普通秒小 * d:FRAME_RATE) ){
		@rp_PP_爆弾評価＿時間早い
		@rpd_ピーター顔表示
		eval($f:rfp_PP_爆弾評価した = 1)
		eval($f:rfp_PP_爆弾評価＿時間した = 1)

	}else if( ($:普通秒小 * d:FRAME_RATE) <= $i:rfp_爆弾解体時フレーム数 && \
		$i:rfp_爆弾解体時フレーム数 < ($:普通秒大 * d:FRAME_RATE) ){
		@rp_PP_爆弾評価＿時間普通
		eval($f:rfp_PP_爆弾評価した = 1)
		eval($f:rfp_PP_爆弾評価＿時間した = 1)

	//時間超過 １０分超過
	}else if( $i:rfp_爆弾解体時フレーム数 >= ($:超過秒 * d:FRAME_RATE) ){
		@rp_PP_爆弾評価＿時間超過
		eval($f:rfp_PP_爆弾評価＿時間した = 1)
		eval($f:rfp_PP_爆弾評価した = 1)
		@rpd_ピーター顔表示
	}
}



proc rpp_爆弾評価＿時間 {
	if(`@rp_爆弾解体脚数` == 2){
		@rpp_爆弾評価＿時間＿中身 60 120 300 600

	}else if(`@rp_爆弾解体脚数` == 3){
		@rpp_爆弾評価＿時間＿中身 120 180 400 800

	}else if(`@rp_爆弾解体脚数` == 4){
		@rpp_爆弾評価＿時間＿中身 180 300 600 1000

	}else if(`@rp_爆弾解体脚数` == 5){
		@rpp_爆弾評価＿時間＿中身 240 500 800 1400
	}
}

proc rpp_PP_爆弾評価＿見つかり過ぎ {
	@rp_PP_爆弾評価＿見つかり過ぎ
	eval($f:rfp_爆弾評価＿見つかりした = 1)
	eval($f:rfp_PP_爆弾評価した = 1)
}

proc rpp_爆弾評価＿見つかり {
	if( (`@rp_爆弾解体脚数` == 2) && ( ($w:危険モード回数 - $w:タンカー編危険モード回数) >= 15) ){
		@rpp_PP_爆弾評価＿見つかり過ぎ
		
	}else if( (`@rp_爆弾解体脚数` == 3) && (($w:危険モード回数 - $w:タンカー編危険モード回数) >= 18) ){
		@rpp_PP_爆弾評価＿見つかり過ぎ

	}else if( (`@rp_爆弾解体脚数` == 4) && (($w:危険モード回数 - $w:タンカー編危険モード回数) >= 21) ){
		@rpp_PP_爆弾評価＿見つかり過ぎ

	}else if( (`@rp_爆弾解体脚数` == 5) && (($w:危険モード回数 - $w:タンカー編危険モード回数) >= 24) ){
		@rpp_PP_爆弾評価＿見つかり過ぎ

	}else if( (`@rp_爆弾解体脚数` == 6) && (($w:危険モード回数 - $w:タンカー編危険モード回数) >= 27) ){
		@rpp_PP_爆弾評価＿見つかり過ぎ
	}
}

proc rpp_PP_爆弾評価＿殺し過ぎ {
	@rp_PP_爆弾評価＿殺し過ぎ
	eval($f:rfp_爆弾評価＿殺し過ぎした = 1)
	eval($f:rfp_PP_爆弾評価した = 1)
}

proc rpp_爆弾評価＿殺し {
print '$w:殺傷人数:' $w:殺傷人数
	if( (`@rp_爆弾解体脚数` == 2) && (($w:殺傷人数 - $w:タンカー編殺傷人数) >= 10) ){
		@rpp_PP_爆弾評価＿殺し過ぎ

	}else if( (`@rp_爆弾解体脚数` == 3) && (($w:殺傷人数 - $w:タンカー編殺傷人数) >= 15) ){
		@rpp_PP_爆弾評価＿殺し過ぎ

	}else if( (`@rp_爆弾解体脚数` == 4) && (($w:殺傷人数 - $w:タンカー編殺傷人数) >= 20) ){
		@rpp_PP_爆弾評価＿殺し過ぎ

	}else if( (`@rp_爆弾解体脚数` == 4) && (($w:殺傷人数 - $w:タンカー編殺傷人数) >= 25) ){
		@rpp_PP_爆弾評価＿殺し過ぎ

	}else if( (`@rp_爆弾解体脚数` == 5) && (($w:殺傷人数 - $w:タンカー編殺傷人数) >= 30) ){
		@rpp_PP_爆弾評価＿殺し過ぎ
	}
}

proc rpp_PP_爆弾評価＿エロ過ぎ {
	@rp_PP_爆弾評価＿エロ過ぎ
	eval($f:rfp_爆弾評価＿エロ過ぎした = 1)
	eval($f:rfp_PP_爆弾評価した = 1)
}

proc rpp_爆弾評価＿エロ {
	if($w:rfp_エロ行為回数 > 2){
		@rpp_PP_爆弾評価＿エロ過ぎ
	}
}

proc rpp_PP_爆弾評価＿食い過ぎ {
	@rp_PP_爆弾評価＿食い過ぎ
	eval($f:rfp_爆弾評価＿食い過ぎした = 1)
	eval($f:rfp_PP_爆弾評価した = 1)
}

proc rpp_爆弾評価＿食い {
	if( ($w:レーション使用回数 - $w:タンカー編レーション回数) >= 10 ){
		@rpp_PP_爆弾評価＿食い過ぎ
	}
}

proc rpp_爆弾評価しちゃうよん {
	if(!$f:rfp_PP_爆弾評価＿時間した){
		@rpp_爆弾評価＿時間
	}
	if( (!$f:rfp_爆弾評価＿見つかりした) && (!$f:rfp_PP_爆弾評価した) ){
		eval($f:rfp_爆弾評価＿見つかりした = 1)
		@rpp_爆弾評価＿見つかり
	}
	if( (!$f:rfp_爆弾評価＿殺し過ぎした) && (!$f:rfp_PP_爆弾評価した) ){
		eval($f:rfp_爆弾評価＿殺し過ぎした = 1)
		@rpp_爆弾評価＿殺し
	}
	if( (!$f:rfp_爆弾評価＿食い過ぎした) && (!$f:rfp_PP_爆弾評価した) ){
		eval($f:rfp_爆弾評価＿食い過ぎした = 1)
		@rpp_爆弾評価＿食い
	}
	if( (!$f:rfp_爆弾評価＿エロ過ぎした) && (!$f:rfp_PP_爆弾評価した) ){
		eval($f:rfp_爆弾評価＿エロ過ぎした = 1)
		@rpp_爆弾評価＿エロ
	}
	eval($f:rfp_PP_爆弾評価した = 0)
}


proc rpp_PP_w12b_爆弾処理完了一回目のみ {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示
	
	//強制無線になる奴の後では説明しないということだん
	if( (`@rp_爆弾解体脚数` != 1) && \	//一回目
		(`@rp_爆弾解体脚数` != 3) && \
		($w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了)  \	//最後の二つ目
		){
		@rp_PP_爆弾報告＿場所＿Ａ脚１
		@rp_PP_爆弾報告＿場所＿Ａ脚２
		@rpp_爆弾評価しちゃうよん
	}
	//Ａ脚屋上に仕掛ける場合はＸＸＸＸＸＸＸＸｘPP_爆弾ヒント＿屋上
	if($f:w12a_爆弾処理完了){
		@rp_PP_残り爆弾＿なし
	}else {
		@rp_PP_爆弾ヒント＿屋上
	}
}

proc rpp_PP_残り爆弾＿なし {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示
	
	@rp_PP_残り爆弾＿なし
}

proc rpp_ノードにアクセスしろ {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示
	
	@rp_PP_ノードにアクセスしろ
}

proc rpp_PP_爆弾ヒント＿Ａ脚ホフク {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示
	
	@rp_PP_残り爆弾＿あり
	@rp_PP_爆弾ヒント＿Ａ脚ホフク
}


proc rpp_爆弾解体中＿Ａ脚１Ｆ {
	//終わってるねぇアンタ
	if($f:w12b_爆弾処理完了){
		//処理してしばらくの間のみ？？ＸＸＸＸＸＸ
		if( (!$f:rfp_PP_w12b_爆弾処理完了一回目のみ聞いた) && (!$f:rfp_w12b_爆弾処理完了後時間たった) ){
			@rpp_PP_w12b_爆弾処理完了一回目のみ
			eval($f:rfp_PP_w12b_爆弾処理完了一回目のみ聞いた = 1 )

		//でも屋上に残っている
		}else if( (!$f:w12a_爆弾処理完了) && (!$f:rfp_PP_w12a_爆弾ヒント＿屋上聞いた) ){
			@rpp_PP_爆弾ヒント＿屋上
			eval($f:rfp_PP_w12a_爆弾ヒント＿屋上聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				if(!$f:w12a_爆弾処理完了){
					@rpp_PP_爆弾ヒント＿屋上
				}else{
					@rpp_PP_残り爆弾＿なし
				}
			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			@rpp_連続無線爆弾処理デフォルト
		}

	//残ってるよー
	}else {
		//ノードに接続してないよー
		if( 	(!$f:w12b_ノードフラグ) && (!$f:rfp_PC_w12bＡ脚爆弾解体中ノード位置聞いた) && \
				(!$f:rfp_PP_w12b_ノードにアクセスしろ聞いた) ){
	
			@rpp_ノードにアクセスしろ
			eval($f:rfp_PP_w12b_ノードにアクセスしろ聞いた = 1 )
			
		}else if(!$f:rfp_PP_爆弾ヒント＿Ａ脚ホフク聞いた) {
			@rpp_PP_爆弾ヒント＿Ａ脚ホフク
			eval($f:rfp_PP_爆弾ヒント＿Ａ脚ホフク聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 2){
				if(!$f:w12b_ノードフラグ){
					@rpp_ノードにアクセスしろ
				}else {
					@rpp_PP_爆弾ヒント＿Ａ脚ホフク
				}
			}else if($w:rfp_rand < 4){
				@rpp_PP_爆弾ヒント＿Ａ脚ホフク
			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpp_PP_爆弾ヒント＿Ａ脚ホフク
			}else {
				@rpp_連続無線爆弾処理デフォルト
			}
		}
	}
}

proc rpp_PP_w12a_爆弾処理完了一回目のみ {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	//強制無線になる奴の後では説明しないということだん
	if( (`@rp_爆弾解体脚数` != 1) && \	//一回目
		(`@rp_爆弾解体脚数` != 3) && \	//半分
		($w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了)  \	//最後の二つ目
		){
		@rp_PP_爆弾報告＿場所＿Ａ脚１
		@rpp_爆弾評価しちゃうよん
	}
	if($f:w12b_爆弾処理完了){
		@rp_PP_残り爆弾＿なし
	}else {
		@rp_PP_爆弾ヒント＿１Ｆ
	}
}

proc rpp_爆弾解体中＿Ａ脚屋上 {
	//終わってるねぇアンタ
	if( $f:w12a_爆弾処理完了){
		if( (!$f:rfp_PP_w12a_爆弾処理完了一回目のみ聞いた) && (!$f:rfp_w12a_爆弾処理完了後時間たった) && \
			($w:ゲーム設定 >= d:LEVEL_HARD) ){
			@rpp_PP_w12a_爆弾処理完了一回目のみ
			eval($f:rfp_PP_w12a_爆弾処理完了一回目のみ聞いた = 1 )

		//でも１Ｆに残っている
		}else if( (!$f:w12b_爆弾処理完了) && (!$f:rfp_PP_w12b_爆弾ヒント＿１Ｆ聞いた) ){
			@rpp_PP_爆弾ヒント＿１Ｆ
			eval($f:rfp_PP_w12b_爆弾ヒント＿１Ｆ聞いた = 1)

		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				if(!$f:w12b_爆弾処理完了){
					@rpp_PP_爆弾ヒント＿１Ｆ
				}else{
					@rpp_PP_残り爆弾＿なし
				}

			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			@rpp_連続無線爆弾処理デフォルト
		}

	//残ってるよー
	}else {
		//ノードに接続してないよー
		if( 	(!$f:w12b_ノードフラグ) && (!$f:rfp_PC_w12bＡ脚爆弾解体中ノード位置聞いた) && \
				(!$f:rfp_PP_w12a_ノードにアクセスしろ聞いた) ){
			@rpp_ノードにアクセスしろ
			eval($f:rfp_PP_w12a_ノードにアクセスしろ聞いた = 1 )
		
		//HARD以上、Ａ脚屋上爆弾 PP_冷凍スプレー注意
		}else if( (!$f:w12a_爆弾処理完了) && (!$f:rfp_PP_ＨＡＲＤ以上Ａ脚屋上ヒント聞いた) ){
			@rpp_ＨＡＲＤ以上Ａ脚屋上ヒント
			eval($f:rfp_PP_ＨＡＲＤ以上Ａ脚屋上ヒント聞いた = 1)
		
		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if( ($w:rfp_rand < 2) && (!$f:w12b_ノードフラグ) ){
					@rpp_ノードにアクセスしろ

			}else if($w:rfp_rand < 4){
				@rpp_ＨＡＲＤ以上Ａ脚屋上ヒント

			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpp_ＨＡＲＤ以上Ａ脚屋上ヒント

			}else {
				@rpp_連続無線爆弾処理デフォルト
			}
		}
	}
}


proc rpp_PP_w14a_爆弾処理完了一回目のみ {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示
	
	//強制無線になる奴の後では説明しないということだん
	if( (`@rp_爆弾解体脚数` != 1) && \	//一回目
		(`@rp_爆弾解体脚数` != 3) && \	//半分
		($w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了)  \	//最後の二つ目
		){
		@rp_PP_爆弾報告＿場所＿Ｂ脚１
		@rp_PP_爆弾報告＿場所＿Ｂ脚２
		@rpp_爆弾評価しちゃうよん
	}
	@rp_PP_残り爆弾＿なし
}

proc rpp_PP_爆弾ヒント＿Ｂ脚扉３ {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_残り爆弾＿あり
	@rp_PP_爆弾ヒント＿Ｂ脚扉３
}



proc rpp_爆弾解体中＿Ｂ脚 {
	//終わってるねぇアンタ
	if($f:w14a_爆弾処理完了){
		//処理してしばらくの間のみ？？ＸＸＸＸＸＸ
		if( (!$f:rfp_PP_w14a_爆弾処理完了一回目のみ聞いた) && (!$f:rfp_w14a_爆弾処理完了後時間たった) ){
			@rpp_PP_w14a_爆弾処理完了一回目のみ
			eval($f:rfp_PP_w14a_爆弾処理完了一回目のみ聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpp_PP_残り爆弾＿なし
			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			@rpp_連続無線爆弾処理デフォルト
		}

	//残ってるよー
	}else {
		//ノードに接続してないよー
		if( 	(!$f:w14a_ノードフラグ) && (!$f:rfp_PC_w14aＢ脚爆弾解体中ノード位置聞いた) && \
				(!$f:rfp_PP_w14a_ノードにアクセスしろ聞いた) ){
			@rpp_ノードにアクセスしろ
			eval($f:rfp_PP_w14a_ノードにアクセスしろ聞いた = 1 )
			
		}else if(!$f:rfp_PP_爆弾ヒント＿Ｂ脚扉３聞いた) {
			@rpp_PP_爆弾ヒント＿Ｂ脚扉３
			eval($f:rfp_PP_爆弾ヒント＿Ｂ脚扉３聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 2){
				if(!$f:w14a_ノードフラグ){
					@rpp_ノードにアクセスしろ
				}else {
					@rpp_PP_爆弾ヒント＿Ｂ脚扉３
				}

			}else if($w:rfp_rand < 4){
				@rpp_PP_爆弾ヒント＿Ｂ脚扉３

			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpp_PP_爆弾ヒント＿Ｂ脚扉３
			}else {
				@rpp_連続無線爆弾処理デフォルト
			}
		}
	}
}

proc rpp_PP_w16a_爆弾処理完了一回目のみ {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示
	
	//強制無線になる奴の後では説明しないということだん
	if( (`@rp_爆弾解体脚数` != 1) && \	//一回目
		(`@rp_爆弾解体脚数` != 3) && \	//半分
		($w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了)  \	//最後の二つ目
		){
		@rp_PP_爆弾報告＿場所＿Ｃ脚１
		@rp_PP_爆弾報告＿場所＿Ｃ脚２
		@rpp_爆弾評価しちゃうよん
	}
	@rp_PP_残り爆弾＿なし
}


proc rpp_PP_爆弾ヒント＿Ｃ脚 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	if(!$f:rfp_PP_爆弾ヒント＿Ｃ脚聞いた) {
		@rp_PP_爆弾ヒント＿Ｃ脚＿女子トイレ１
		@rp_PP_爆弾ヒント＿Ｃ脚＿女子トイレ２
		//フラグ立てよーねＸＸＸＸＸＸＸＸＸＸＸｘｘｘ
		if($f:rfp_女子トイレに入った) {
			@rp_PP_爆弾ヒント＿Ｃ脚＿女子トイレ３
		}
		eval($f:rfp_PP_爆弾ヒント＿Ｃ脚聞いた = 1 )

	}else {
		@rp_PP_残り爆弾＿あり
		@rp_PP_爆弾ヒント＿Ｃ脚＿女子トイレ２
	}
}



proc rpp_爆弾解体中＿Ｃ脚 {
	//終わってるねぇアンタ
	if($f:w16b_爆弾処理完了){
		//処理してしばらくの間のみ？？ＸＸＸＸＸＸ
		if( (!$f:rfp_PP_w16a_爆弾処理完了一回目のみ聞いた) && (!$f:rfp_w16b_爆弾処理完了後時間たった) ){
			@rpp_PP_w16a_爆弾処理完了一回目のみ
			eval($f:rfp_PP_w16a_爆弾処理完了一回目のみ聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpp_PP_残り爆弾＿なし
			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			@rpp_連続無線爆弾処理デフォルト
		}

	//残ってるよー
	}else {
		//ノードに接続してないよー
		if( 	(!$f:w16a_ノードフラグ) && (!$f:rfp_PC_w16aＣ脚爆弾解体中ノード位置聞いた) && \
				(!$f:rfp_PP_w16a_ノードにアクセスしろ聞いた) ){
			@rpp_ノードにアクセスしろ
			eval($f:rfp_PP_w16a_ノードにアクセスしろ聞いた = 1 )
			
		}else if(!$f:rfp_PP_爆弾ヒント＿Ｃ脚聞いた) {
			@rpp_PP_爆弾ヒント＿Ｃ脚
			eval($f:rfp_PP_爆弾ヒント＿Ｃ脚聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 2){
				if(!$f:w16a_ノードフラグ){
					@rpp_ノードにアクセスしろ
				}else {
					@rpp_PP_爆弾ヒント＿Ｃ脚
				}

			}else if($w:rfp_rand < 4){
				@rpp_PP_爆弾ヒント＿Ｃ脚
			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpp_PP_爆弾ヒント＿Ｃ脚
			}else {
				@rpp_連続無線爆弾処理デフォルト
			}
		}
	}
}

proc rpp_PP_w18a_爆弾処理完了一回目のみ {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	//強制無線になる奴の後では説明しないということだん
	if( (`@rp_爆弾解体脚数` != 1) && \	//一回目
		(`@rp_爆弾解体脚数` != 3) && \	//半分
		($w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了)  \	//最後の二つ目
		){
		@rp_PP_爆弾報告＿場所＿Ｄ脚１
		@rp_PP_爆弾報告＿場所＿Ｄ脚２
		@rpp_爆弾評価しちゃうよん
	}
	@rp_PP_残り爆弾＿なし
}

proc rpp_PP_爆弾ヒント＿Ｄ脚 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_残り爆弾＿あり

	if(!$f:rfp_PP_爆弾ヒント＿Ｄ脚＿立体聞いた) {
		@rp_PP_爆弾ヒント＿Ｄ脚＿立体
		eval($f:rfp_PP_爆弾ヒント＿Ｄ脚＿立体聞いた = 1)
	
	}else if( (!$f:rfp_PP_爆弾ヒント＿Ｄ脚＿ハッチ内聞いた) && \
				($w:ゲーム設定 <= d:LEVEL_HARD) ){
		@rp_PP_爆弾ヒント＿Ｄ脚＿ハッチ内
		eval($f:rfp_PP_爆弾ヒント＿Ｄ脚＿ハッチ内聞いた = 1)
	
	//ズバリヒントの条件ＸＸＸＸＸＸＸＸＸＸＸｘ
	}else if( (!$f:rfp_PP_爆弾ヒント＿Ｄ脚＿ハッチ内ズパリ聞いた) && \
				($w:ゲーム設定 <= d:LEVEL_HARD) && \
				( ( ($i:プレイタイム - $i:rfp_w18a_爆弾解体開始時刻) / d:FRAME_RATE) >= 300) ){
		@rp_PP_爆弾ヒント＿Ｄ脚＿ハッチ内ズパリ
		eval($f:rfp_PP_爆弾ヒント＿Ｄ脚＿ハッチ内ズパリ聞いた = 1 )
	
	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if( ($w:rfp_rand < 1) && ($w:ゲーム設定 <= d:LEVEL_HARD) ){
			@rp_PP_爆弾ヒント＿Ｄ脚＿ハッチ内

		}else if( ($w:rfp_rand < 3)  && \
				($w:ゲーム設定 <= d:LEVEL_HARD) && \
				( ( ($i:プレイタイム - $i:rfp_w18a_爆弾解体開始時刻) / d:FRAME_RATE) >= 300) ){
			@rp_PP_爆弾ヒント＿Ｄ脚＿ハッチ内ズパリ

		}else {
			@rp_PP_爆弾ヒント＿Ｄ脚＿立体
		}
	}
}


proc rpp_爆弾解体中＿Ｄ脚 {
	//終わってるねぇアンタ
	//レベルによって違うよ！$f:w18a_爆弾処理完了2,3
	if($f:w18a_爆弾処理完了1 && $f:w18a_爆弾処理完了2 && $f:w18a_爆弾処理完了3 ){
		//処理してしばらくの間のみ？？ＸＸＸＸＸＸ
		if( (!$f:rfp_PP_w18a_爆弾処理完了一回目のみ聞いた) && (!$f:rfp_w18a_爆弾処理完了後時間たった) ){
			@rpp_PP_w18a_爆弾処理完了一回目のみ
			eval($f:rfp_PP_w18a_爆弾処理完了一回目のみ聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpp_PP_残り爆弾＿なし
			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			@rpp_連続無線爆弾処理デフォルト
		}

	//残ってるよー
	}else {
		//ノードに接続してないよー
		if( 	(!$f:w18a_ノードフラグ) && (!$f:rfp_PC_w18aＤ脚爆弾解体中ノード位置聞いた) && \
				(!$f:rfp_PP_w18a_ノードにアクセスしろ聞いた) ){
			@rpp_ノードにアクセスしろ
			eval($f:rfp_PP_w18a_ノードにアクセスしろ聞いた = 1 )
			
		}else if(!$f:rfp_PP_爆弾ヒント＿Ｄ脚聞いた) {
			@rpp_PP_爆弾ヒント＿Ｄ脚
			eval($f:rfp_PP_爆弾ヒント＿Ｄ脚聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 2){
				if(!$f:w18a_ノードフラグ){
					@rpp_ノードにアクセスしろ
				}else {
					@rpp_PP_爆弾ヒント＿Ｄ脚
				}

			}else if($w:rfp_rand < 4){
				@rpp_PP_爆弾ヒント＿Ｄ脚
			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpp_PP_爆弾ヒント＿Ｄ脚
			}else {
				@rpp_連続無線爆弾処理デフォルト
			}
		}
	}
}

proc rpp_PP_w20a_爆弾処理完了一回目のみ {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	//強制無線になる奴の後では説明しないということだん
	if( (`@rp_爆弾解体脚数` != 1) && \	//一回目
		(`@rp_爆弾解体脚数` != 3) && \	//半分
		($w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了)  \	//最後の二つ目
		){
		@rp_PP_爆弾報告＿場所＿Ｅ脚１
		@rp_PP_爆弾報告＿場所＿Ｅ脚２
		@rpp_爆弾評価しちゃうよん
	}
	if($f:w20b_爆弾処理完了){
		@rp_PP_残り爆弾＿なし
	}else {
		@rp_PP_爆弾ヒント＿屋上
	}
}

proc rpp_PP_爆弾ヒント＿Ｅ脚 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_残り爆弾＿あり
	@rp_PP_爆弾ヒント＿移動荷物
}

proc rpp_PP_爆弾ヒント＿屋上 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_爆弾ヒント＿屋上
}


proc rpp_爆弾解体中＿Ｅ脚 {
	//終わってるねぇアンタ
	if($f:w20a_爆弾処理完了){
		//処理してしばらくの間のみ？？ＸＸＸＸＸＸ
		if( (!$f:rfp_PP_w20a_爆弾処理完了一回目のみ聞いた) && (!$f:rfp_w20a_爆弾処理完了後時間たった) && \
			( $w:ゲーム設定 > d:LEVEL_NORMAL ) ){
			@rpp_PP_w20a_爆弾処理完了一回目のみ
			eval($f:rfp_PP_w20a_爆弾処理完了一回目のみ聞いた = 1 )

		//でもヘリポートに残っている
		}else if( (!$f:w20b_爆弾処理完了) && (!$f:rfp_PP_w20a_爆弾ヒント＿屋上聞いた) ){
			@rpp_PP_爆弾ヒント＿屋上
			eval($f:rfp_PP_w20a_爆弾ヒント＿屋上聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				if(!$f:w20b_爆弾処理完了){
					@rpp_PP_爆弾ヒント＿屋上
				}else{
					@rpp_PP_残り爆弾＿なし
				}
			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			@rpp_連続無線爆弾処理デフォルト
		}

	//残ってるよー
	}else {
		//ノードに接続してないよー
		if( 	(!$f:w20a_ノードフラグ) && (!$f:rfp_PC_w20aＥ脚爆弾解体中ノード位置聞いた) && \
				(!$f:rfp_PP_w20a_ノードにアクセスしろ聞いた) ){
			@rpp_ノードにアクセスしろ
			eval($f:rfp_PP_w20a_ノードにアクセスしろ聞いた = 1 )
			
		}else if(!$f:rfp_PP_爆弾ヒント＿Ｅ脚聞いた) {
			@rpp_PP_爆弾ヒント＿Ｅ脚
			eval($f:rfp_PP_爆弾ヒント＿Ｅ脚聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 2){
				if(!$f:w20a_ノードフラグ){
					@rpp_ノードにアクセスしろ
				}else {
					@rpp_PP_爆弾ヒント＿Ｅ脚
				}

			}else if($w:rfp_rand < 4){
				@rpp_PP_爆弾ヒント＿Ｅ脚
			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpp_PP_爆弾ヒント＿Ｅ脚
			}else {
				@rpp_連続無線爆弾処理デフォルト
			}
		}
	}
}

proc rpp_PP_爆弾ヒント＿１Ｆ {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_爆弾ヒント＿１Ｆ
}

proc rpp_ＨＡＲＤ以上Ａ脚屋上ヒント {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_残り爆弾＿あり
	@rp_PP_冷凍スプレー注意
}

proc rpp_PP_w20b_爆弾処理完了一回目のみ {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	//強制無線になる奴の後では説明しないということだん
	if( (`@rp_爆弾解体脚数` != 1) && \	//一回目
		(`@rp_爆弾解体脚数` != 3) && \	//半分
		($w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了)  \	//最後の二つ目
		){
		@rp_PP_爆弾報告＿場所＿Ｅ脚屋上１
		@rp_PP_爆弾報告＿場所＿Ｅ脚屋上ハリアー
		@rpp_爆弾評価しちゃうよん
	}
	if($f:w20a_爆弾処理完了){
		@rp_PP_残り爆弾＿なし
	}else {
		@rp_PP_爆弾ヒント＿１Ｆ
	}
}

proc rpp_PP_爆弾ヒント＿Ｅ脚屋上 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_残り爆弾＿あり
	if(!$f:rfp_PP_爆弾ヒント＿ハリアー聞いた) {
		@rp_PP_爆弾ヒント＿ハリアー
		eval($f:rfp_PP_爆弾ヒント＿ハリアー聞いた = 1 )
	
	//ズバリヒント条件ＸＸＸＸＸＸＸＸＸＸｘｘ
	}else if( (!$f:rfp_PP_爆弾ヒント＿ハリアーズバリ聞いた) && \
			( ( ($i:プレイタイム - $i:rfp_w20b_爆弾解体開始時刻) / d:FRAME_RATE) >= 300) ){
		@rp_PP_爆弾ヒント＿ハリアーズバリ
		eval($f:rfp_PP_爆弾ヒント＿ハリアーズバリ聞いた = 1)
	
	}else {
		rand 2
		eval($w:rfp_rand = $status)

		if( ($w:rfp_rand < 1) && \
			( ( ($i:プレイタイム - $i:rfp_w20b_爆弾解体開始時刻) / d:FRAME_RATE) >= 300) ){
			@rp_PP_爆弾ヒント＿ハリアーズバリ
		}else {
			@rp_PP_爆弾ヒント＿ハリアー
		}
	}
}




proc rpp_爆弾解体中＿Ｅ脚屋上 {
	//終わってるねぇアンタ
	if($f:w20b_爆弾処理完了){
		//処理してしばらくの間のみ？？ＸＸＸＸＸＸ
		if( (!$f:rfp_PP_w20b_爆弾処理完了一回目のみ聞いた) && (!$f:rfp_w20b_爆弾処理完了時間たった) ){
			@rpp_PP_w20b_爆弾処理完了一回目のみ
			eval($f:rfp_PP_w20b_爆弾処理完了一回目のみ聞いた = 1 )

		//でも１Ｆに残っている
		}else if( (!$f:w20a_爆弾処理完了) && (!$f:rfp_PP_w20b_爆弾ヒント＿１Ｆ聞いた) ){
			@rpp_PP_爆弾ヒント＿１Ｆ
			eval($f:rfp_PP_w20b_爆弾ヒント＿１Ｆ聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				if(!$f:w20a_爆弾処理完了){
					@rpp_PP_爆弾ヒント＿１Ｆ
				}else{
					@rpp_PP_残り爆弾＿なし
				}
			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			@rpp_連続無線爆弾処理デフォルト
		}

	//残ってるよー
	}else {
		//ノードに接続してないよー
		if( 	(!$f:w20a_ノードフラグ) && (!$f:rfp_PC_w20bＥ脚爆弾解体中ノード位置聞いた) && \
				(!$f:rfp_PP_w20b_ノードにアクセスしろ聞いた) ){
			@rpp_ノードにアクセスしろ
			eval($f:rfp_PP_w20b_ノードにアクセスしろ聞いた = 1 )
			
		}else if(!$f:rfp_PP_爆弾ヒント＿Ｅ脚屋上聞いた) {
			@rpp_PP_爆弾ヒント＿Ｅ脚屋上
			eval($f:rfp_PP_爆弾ヒント＿Ｅ脚屋上聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 2){
				if(!$f:w20a_ノードフラグ){
					@rpp_ノードにアクセスしろ
				}else {
					@rpp_PP_爆弾ヒント＿Ｅ脚屋上
				}

			}else if($w:rfp_rand < 4){
				@rpp_PP_爆弾ヒント＿Ｅ脚屋上
			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpp_PP_爆弾ヒント＿Ｅ脚屋上
			}else {
				@rpp_連続無線爆弾処理デフォルト
			}
		}
	}
}


proc rpp_PP_w22a_爆弾処理完了一回目のみ {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示
	
	//強制無線になる奴の後では説明しないということだん
	if( (`@rp_爆弾解体脚数` != 1) && \	//一回目
		(`@rp_爆弾解体脚数` != 3) && \	//半分
		($w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了)  \	//最後の二つ目
		){
		@rp_PP_爆弾報告＿場所＿Ｆ脚１
		@rp_PP_爆弾報告＿場所＿Ｆ脚２
		@rpp_爆弾評価しちゃうよん
	}
	@rp_PP_残り爆弾＿なし
}

proc rpp_PP_爆弾ヒント＿Ｆ脚 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示
	
	@rp_PP_残り爆弾＿あり
	@rp_PP_爆弾ヒント＿Ｆ脚＿飛び降り
}

proc rpp_爆弾解体中＿Ｆ脚 {
	//終わってるねぇアンタ
	if($f:w22a_爆弾処理完了){
		//処理してしばらくの間のみ？？ＸＸＸＸＸＸ
		if( (!$f:rfp_PP_w22a_爆弾処理完了一回目のみ聞いた) && (!$f:rfp_w22a_爆弾処理完了後時間たった) ){
			@rpp_PP_w22a_爆弾処理完了一回目のみ
			eval($f:rfp_PP_w22a_爆弾処理完了一回目のみ聞いた = 1 )


		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpp_PP_残り爆弾＿なし

			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			@rpp_連続無線爆弾処理デフォルト
		}

	//残ってるよー
	}else {
		//ノードに接続してないよー
		if( 	(!$f:w22a_ノードフラグ) && (!$f:rfp_PC_w22aＦ脚爆弾解体中ノード位置聞いた) && \
				(!$f:rfp_PP_w22a_ノードにアクセスしろ聞いた) ){
			@rpp_ノードにアクセスしろ
			eval($f:rfp_PP_w22a_ノードにアクセスしろ聞いた = 1 )
			
		}else if(!$f:rfp_PP_爆弾ヒント＿Ｆ脚聞いた) {
			@rpp_PP_爆弾ヒント＿Ｆ脚
			eval($f:rfp_PP_爆弾ヒント＿Ｆ脚聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 2){
				if(!$f:w22a_ノードフラグ){
					@rpp_ノードにアクセスしろ
				}else {
					@rpp_PP_爆弾ヒント＿Ｆ脚
				}

			}else if($w:rfp_rand < 4){
				@rpp_PP_爆弾ヒント＿Ｆ脚
			}else {
				@rpp_爆弾処理デフォルト
			}

		//連続無線
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpp_PP_爆弾ヒント＿Ｆ脚
			}else {
				@rpp_連続無線爆弾処理デフォルト
			}
		}
	}
}

proc rpp_PP_連絡橋に爆弾なし {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示
	
	@rp_PP_連絡橋に爆弾なし
}


proc rpp_爆弾解体中＿連絡橋 {
	if(!$f:rfp_PP_連絡橋に爆弾なし聞いた) {
		@rpp_PP_連絡橋に爆弾なし
		eval($f:rfp_PP_連絡橋に爆弾なし聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 5
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 3){
			@rpp_PP_連絡橋に爆弾なし
		}else {
			@rpp_爆弾処理デフォルト
		}
	//連続無線
	}else {
		@rpp_連続無線爆弾処理デフォルト
	}
}

proc rpp_爆弾処理終了後デフォルト {
	if(d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１開始 ){
		if($w:アイテム数Ｒ[11] <= 0) {
			@rpp_PP_センサーＢ取りに行け
		
		//センサーＢを手に入れているヒネクレ野郎
		}else {
			@rpp_PP_カウント前センサーＢ持っている
		}

	}else {
		@rpp_PP_無臭Ｃ４＿Ａ脚に行け
	}
}

proc rpp_PP_無臭Ｃ４調査中 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_無臭Ｃ４調査中
}


proc rpp_連続無線爆弾処理終了後デフォルト {
	if( (d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了 <= $w:p_story && \
				$w:p_story < d:ST:P027_01_R01爆弾解体後昇降機ホール１無線デモ１開始) && \
			((($i:プレイタイム - $i:rfp_ピーター出撃開始時刻) / d:FRAME_RATE) <= 600)  && \
			(!$f:rfp_PP_歩ける言い訳聞いた) ){
		@rpp_PP_歩ける言い訳
		eval($f:rfp_PP_歩ける言い訳聞いた = 1)

	}else if( (!$f:rfp_PP_無臭Ｃ４調査中聞いた) && \
		(d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１開始 ) ){
		@rpp_PP_無臭Ｃ４調査中
		eval($f:rfp_PP_無臭Ｃ４調査中聞いた = 1)

	}else {
		@rpp_爆弾処理終了後デフォルト
	}
}

proc rpp_PP_センサーＢ取りに行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_センサーＢ取りに行け
}

proc rpp_PP_カウント前センサーＢ持っている {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_カウント前センサーＢ持っている
}

proc rpp_PP_無臭Ｃ４＿Ａ脚に行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	@rp_PP_無臭Ｃ４＿Ａ脚に行け１
	if(!$f:rfp_PP_無臭Ｃ４＿Ａ脚に行け聞いた) {
		@rp_PP_無臭Ｃ４＿Ａ脚に行け２
		eval($f:rfp_PP_無臭Ｃ４＿Ａ脚に行け聞いた = 1)
	}
}

proc rpp_PP_移動後Ｈ脚到着前 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示
	
	if(!$f:rfp_PP_移動後Ｈ脚到着前聞いた) {
		@rp_PP_移動後Ｈ脚到着前１
		@rp_PP_移動後Ｈ脚到着前２
		@rp_PP_移動後Ｈ脚到着前３
		eval($f:rfp_PP_移動後Ｈ脚到着前聞いた = 1)
	}
	@rp_PP_移動後Ｈ脚到着前４
}


proc rpp_ピーターデフォルト {

	//すなわち爆弾解体中
	if(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
			$w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了 ){

		if( ($w:p_story > d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了) && \
				(!$f:rfp_PP_移動後Ｈ脚到着前聞いた) ){
			@rpp_PP_移動後Ｈ脚到着前
			eval($f:rfp_PP_移動後Ｈ脚到着前聞いた = 1)

		//おぷしょんでレーダーオフの場合も！ＸＸＸＸＸＸＸＸ
		//Ａ脚 １Ｆ
		}else if($s:エリア == "w12b" ) {
			@rpp_爆弾解体中＿Ａ脚１Ｆ

		//ＨＡＲＤとかではやるのかな？ＸＸＸＸＸＸＸＸＸＸＸＸＸｘｘ
		//Ａ脚屋上
		}else if( ($s:エリア == "w12a" ) || ($s:エリア == "w12c" ) ){
			@rpp_爆弾解体中＿Ａ脚屋上

		//Ｂ脚
		}else if($s:エリア == "w14a" ){
			@rpp_爆弾解体中＿Ｂ脚
		
		
		//Ｃ脚
		}else if( ($s:エリア == "w16a" ) || ($s:エリア == "w16b") ){
			@rpp_爆弾解体中＿Ｃ脚

		//Ｄ脚
		}else if($s:エリア == "w18a" ) {
			@rpp_爆弾解体中＿Ｄ脚
		
		//Ｅ脚 １Ｆ
		}else if($s:エリア == "w20a" ) {
			@rpp_爆弾解体中＿Ｅ脚


		//Ｅ脚 屋上
		}else if(  ($s:エリア == "w20b" ) || ($s:エリア == "w20c" ) || ($s:エリア == "w20d" ) ) {
			@rpp_爆弾解体中＿Ｅ脚屋上


		//Ｆ脚 
		}else if($s:エリア == "w22a" ) {
			@rpp_爆弾解体中＿Ｆ脚


		//その他連絡橋等
		}else{
			@rpp_爆弾解体中＿連絡橋
		}

	}else if(d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１開始 ){

		//無線一回目
		if($w:rfp_callcount <= 1){
			//センサーＢなひ
			if($w:アイテム数Ｒ[11] <= 0) {
				@rpp_PP_センサーＢ取りに行け
//				eval($f:rfp_PP_センサーＢ取りに行け聞いた = 1 )

			//センサーＢを手に入れているヒネクレ野郎
			}else {
				@rpp_PP_カウント前センサーＢ持っている
//				eval($f:rfp_PP_カウント前センサーＢ持っている聞いた = 1 )
			}
		}else {
			@rpp_連続無線爆弾処理終了後デフォルト
		}

	}else if(d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１開始 ){

		if(!$f:rfp_PP_無臭Ｃ４＿Ａ脚に行け聞いた) {
			@rpp_PP_無臭Ｃ４＿Ａ脚に行け
			eval($f:rfp_PP_無臭Ｃ４＿Ａ脚に行け聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			@rpp_PP_無臭Ｃ４＿Ａ脚に行け

		}else {
			@rpp_連続無線爆弾処理終了後デフォルト
		}



	}else {
		@rpp_ばぐっち
	}
}



block codec RPP_ピーターデフォルト 14025 d:ピーター＿プリスキン {
	@rp_無線デフォルト前処理＿ピーター
	@rp_短縮ダイアル＿ピーターセット
	@rpp_ピーターデフォルト
}

block codec RPP_爆弾ヒント＿Ａ脚ホフクＣＡＬＬ 14025 d:ピーター＿プリスキン {
	@rp_無線デフォルト前処理＿ピーター
	@rp_短縮ダイアル＿ピーターセット
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示
	
	@rp_PP_爆弾ヒント＿Ａ脚ホフク
	eval($f:rfp_PP_爆弾ヒント＿Ａ脚ホフク聞いた = 1 )
	eval($f:rfp_w12b_ホフクパイプ近くＣＡＬＬ聞いた = 1)
}

proc rpp_Ｃ脚厨房扉前危険モード {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	if(!$f:rfp_PP_Ｃ脚厨房扉前危険モード１聞いた) {
		@rp_PP_Ｃ脚厨房扉前危険モード１
		eval($f:rfp_PP_Ｃ脚厨房扉前危険モード１聞いた = 1)
	
	}else if(!$f:rfp_PP_Ｃ脚厨房扉前危険モード２聞いた) {
		@rp_PP_Ｃ脚厨房扉前危険モード２
		eval($f:rfp_PP_Ｃ脚厨房扉前危険モード２聞いた = 1)
	}else {
		rand 2
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rp_PP_Ｃ脚厨房扉前危険モード１
		}else {
			@rp_PP_Ｃ脚厨房扉前危険モード２
		}
	}
}

proc rpp_Ｃ脚厨房扉前 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示

	if($b:rfp_ピーターちょっかい回数 >= 5){
		@rp_PP_Ｃ脚厨房扉前扉叩いた後
//		eval($f:rfp_PP_Ｃ脚厨房扉前扉叩いた後聞いた = 1)

	}else{
		@rp_PP_Ｃ脚厨房扉前１
//		eval($f:rfp_PP_Ｃ脚厨房扉前１聞いた = 1)
	}
	eval($f:rfp_PP_Ｃ脚厨房扉前聞いた = 1)
}

block codec RPP_天岩戸 14025 d:ピーター＿プリスキン {
	@rp_無線デフォルト前処理＿ピーター
	@rp_短縮ダイアル＿ピーターセット

	if( ($w:アラートモード != d:ALERT_MODE_SNEAK) && (!$f:rfp_PP_Ｃ脚厨房扉前危険モード２聞いた) ){
		@rpp_Ｃ脚厨房扉前危険モード
		
	}else if(($w:アラートモード == d:ALERT_MODE_SNEAK) && (!$f:rfp_PP_Ｃ脚厨房扉前聞いた) ){
		@rpp_Ｃ脚厨房扉前
		eval($f:rfp_PP_Ｃ脚厨房扉前聞いた = 1)
	
	//無線一回目
	}else if( ($w:rfp_callcount <= 1) && \
				($w:アラートモード == d:ALERT_MODE_SNEAK) ){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3) {
			@rpp_Ｃ脚厨房扉前
		}else {
			@rpp_ピーターデフォルト
		}

	//連続無線
	}else {
		if($w:アラートモード != d:ALERT_MODE_SNEAK) {
				@rpp_Ｃ脚厨房扉前危険モード
		}else {
			@rpp_ピーターデフォルト
		}
	}
}

proc rpp_PP_爆弾ヒント＿Ｂ脚扉 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示
	
	@rp_PP_爆弾ヒント＿Ｂ脚扉１
	if( ($w:アイテム == d:アイテム:センサーＡ) && (!$f:rfp_PP_爆弾ヒント＿Ｂ脚扉聞いた) ){
		@rp_PP_爆弾ヒント＿Ｂ脚扉２
	}
	@rp_PP_爆弾ヒント＿Ｂ脚扉３
	eval($f:rfp_PP_爆弾ヒント＿Ｂ脚扉聞いた = 1)
}

block codec RPP_w14a_変電盤爆弾近く 14025 d:ピーター＿プリスキン {
	@rp_無線デフォルト前処理＿ピーター
	@rp_短縮ダイアル＿ピーターセット

	if( (!$f:rfp_PP_爆弾ヒント＿Ｂ脚扉聞いた) && (!$f:w14a_爆弾処理完了) && \
		(`%ロッカー状態 -n ロッカー:06` & 0x0002) ){
		@rpp_PP_爆弾ヒント＿Ｂ脚扉
		eval($f:rfp_PP_爆弾ヒント＿Ｂ脚扉聞いた = 1)
		
	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && (!$f:w14a_爆弾処理完了) && \
			(`%ロッカー状態 -n ロッカー:06` & 0x0002) ){
			@rpp_PP_爆弾ヒント＿Ｂ脚扉
		}else {
			@rpp_ピーターデフォルト
		}

	//連続無線
	}else {
		@rpp_ピーターデフォルト
	}
}

proc rpp_PP_爆弾ヒント＿Ｃ脚天井 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ピーター顔設定１
	@rpd_顔表示
	
	@rp_PP_爆弾ヒント＿Ｃ脚天井
}


block codec RPP_w16b_女子トイレ爆弾近く 14025 d:ピーター＿プリスキン {
	@rp_無線デフォルト前処理＿ピーター
	@rp_短縮ダイアル＿ピーターセット

	if( (!$f:rfp_PP_爆弾ヒント＿Ｃ脚天井聞いた) && (!$f:w16b_爆弾処理完了) ){
		@rpp_PP_爆弾ヒント＿Ｃ脚天井
		eval($f:rfp_PP_爆弾ヒント＿Ｃ脚天井聞いた = 1)
		
	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && (!$f:w16b_爆弾処理完了) ){
			@rpp_PP_爆弾ヒント＿Ｃ脚天井
		}else {
			@rpp_ピーターデフォルト
		}

	//連続無線
	}else {
		@rpp_ピーターデフォルト
	}
}

//モーション班用デバッグスクリプトェウァアアアア
#if d:MGS2_SCN
#include "p_peter_dbg.h"
#endif

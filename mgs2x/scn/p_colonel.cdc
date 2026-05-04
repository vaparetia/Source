//
//	p_colonel.cdc
//	プラント編大佐無線らしいぞ。   
//
//	2001/06/15	T.Fukushima
//	$Id: p_colonel.cdc,v 1.212 2002/06/07 09:53:00 usr01475 Exp $

/* メッセージ定義 */
#define CODEC_FILE 1
#include "vardef.h"			// 武器・アイテム関係のdefine
#include "cdc_proc_p.h"

#include "p_colonel_e.cm"
#include "p_colonel_e.ct"
#include "p_demo_e.cm"
#include "p_demo_e.ct"

#include "p_colonel_f.cm"
#include "p_colonel_f.ct"
#include "p_demo_f.cm"
#include "p_demo_f.ct"

#include "p_colonel_g.cm"
#include "p_colonel_g.ct"
#include "p_demo_g.cm"
#include "p_demo_g.ct"

#include "p_colonel_s.cm"
#include "p_colonel_s.ct"
#include "p_demo_s.cm"
#include "p_demo_s.ct"

#include "p_colonel_i.cm"
#include "p_colonel_i.ct"
#include "p_demo_i.cm"
#include "p_demo_i.ct"

#include "p_colonel_j.cm"
#include "p_colonel_j.ct"
#include "p_demo_j.cm"
#include "p_demo_j.ct"


proc rp_PC_水中＿スカルスーツ  {
vox t:vc260241 {
	@rpb_PC_水中＿スカルスーツ
}//v
}//d

proc rp_PC_水中＿スカルスーツ＿潜水  {
vox t:vc270241 {
	@rpb_PC_水中＿スカルスーツ
}//v
}//d

proc rp_PC_水密扉２＿潜水  {
vox t:vc270441 {
	@rpb_PC_水密扉２
}//v
}//d
proc rp_PC_水密扉２  {
vox t:vc260441 {
	@rpb_PC_水密扉２
}//v
}//d


proc rp_PC_敵武器使用不可１＿潜水  {
vox t:vc270461 {
	@rpb_PC_敵武器使用不可１
}//v
}//d
proc rp_PC_敵武器使用不可１  {
vox t:vc260461 {
	@rpb_PC_敵武器使用不可１
}//v
}//d

proc rp_PC_敵武器使用不可２＿潜水  {
vox t:vc270471 {
	@rpb_PC_敵武器使用不可２
}//v
}//d
proc rp_PC_敵武器使用不可２  {
vox t:vc260471 {
	@rpb_PC_敵武器使用不可２
}//v
}//d

proc rp_PC_海底ドックＶＲ１＿潜水  {
vox t:vc270541 {
	@rpb_PC_海底ドックＶＲ１
}//v
}//d
proc rp_PC_海底ドックＶＲ１  {
vox t:vc260541 {
	@rpb_PC_海底ドックＶＲ１
}//v
}//d

proc rp_PC_海底ドックＶＲ２＿潜水  {
vox t:vc270551 {
	@rpb_PC_海底ドックＶＲ２
}//v
}//d
proc rp_PC_海底ドックＶＲ２  {
vox t:vc260551 {
	@rpb_PC_海底ドックＶＲ２
}//v
}//d

proc rp_PC_レーション操作説明  {
vox t:vc260631 {
	@rpb_PC_レーション操作説明
}//v
}//d

proc rp_PC_レーション操作説明＿潜水  {
vox t:vc270631 {
	@rpb_PC_レーション操作説明
}//v
}//d

proc rp_PC_レーション操作説明＿ローズ＿潜水  {
vox t:vc270641 {
	@rpb_PC_レーション操作説明＿ローズ
}//v
}

proc rp_PC_レーション操作説明＿ローズ  {
vox t:vc260641 {
	@rpb_PC_レーション操作説明＿ローズ
}//v
}//d

proc rp_PC_ノード前接続前１＿潜水  {
vox t:vc270651 {
	@rpb_PC_ノード前接続前１
}//v
}//d

proc rp_PC_ノード前接続前１  {
vox t:vc260651 {
	@rpb_PC_ノード前接続前１
}//v
}//d

proc rp_PC_海底ドック昇降機前ノード接続後２＿潜水  {
vox t:vc270711 {
	@rpb_PC_海底ドック昇降機前ノード接続後２
}//v
}//d
proc rp_PC_海底ドック昇降機前ノード接続後２  {
vox t:vc260711 {
	@rpb_PC_海底ドック昇降機前ノード接続後２
}//v
}//d

proc rp_PC_海底ドック潜入者＿潜水  {
vox t:vc272491 {
	@rpb_PC_海底ドック潜入者
}//v
}//d
proc rp_PC_海底ドック潜入者  {
vox t:vc262491 {
	@rpb_PC_海底ドック潜入者
}//v
}//d

proc rp_PC_流血説明１＿潜水  {
vox t:vc274681 {
	@rpb_PC_流血説明１
}//v
}//d
proc rp_PC_流血説明１  {
vox t:vc264681 {
	@rpb_PC_流血説明１
}//v
}//d

proc rp_PC_フナムシ１＿潜水  {
vox t:vc275131 {
	@rpb_PC_フナムシ１
}//v
}//d
proc rp_PC_フナムシ１  {
vox t:vc265131 {
	@rpb_PC_フナムシ１
}//v
}//d

proc rp_PC_フナムシ２＿潜水  {
vox t:vc275141 {
	@rpb_PC_フナムシ２
}//v
}//d
proc rp_PC_フナムシ２  {
vox t:vc265141 {
	@rpb_PC_フナムシ２
}//v
}//d

proc rp_PC_ジムスーツ１＿潜水  {
vox t:vc275181 {
	@rpb_PC_ジムスーツ１
}//v
}//d
proc rp_PC_ジムスーツ１  {
vox t:vc265181 {
	@rpb_PC_ジムスーツ１
}//v
}//d


proc rp_無線デフォルト前処理＿大佐 {
	@rp_無線デフォルト前処理
	if(`prefreq` != 14085){
		eval($w:rfp_callcount = 1)
	}
}

proc rpc_ばぐっち {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_発狂＿電車
}

proc rpc_PC_懸垂握力ＬＶ２ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_懸垂で握力上がった聞いた) {
		@rp_PC_懸垂握力ＬＶ１
		eval($f:rfp_PC_懸垂で握力上がった聞いた = 1)
	}
	if( ($w:p_story >= d:ST:P004_02_R01ノード初接続２無線デモ１終了) && \
		(!$f:rfp_PR_ローズさんが激怒してます！) && (!$f:rfp_ローズさん落込んでます) ){
		@rpd_ローズ顔表示
		@rp_PC_懸垂握力ＬＶ２＿２
		eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
	}
	eval($f:rfp_PC_懸垂握力ＬＶ２聞いた = 1)
}

proc rpc_PC_懸垂握力ＬＶ３ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_懸垂で握力上がった聞いた) {
		@rp_PC_懸垂握力ＬＶ１
		eval($f:rfp_PC_懸垂で握力上がった聞いた = 1)
	}else {
		@rp_PC_懸垂握力ＬＶ２＿１
	}
	if( ($w:p_story >= d:ST:P004_02_R01ノード初接続２無線デモ１終了) && \
		(!$f:rfp_PR_ローズさんが激怒してます！) && (!$f:rfp_ローズさん落込んでます) ){
		@rpd_ローズ顔表示
		@rp_PC_懸垂握力ＬＶ３＿２
		eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
	}
	eval($f:rfp_PC_懸垂握力ＬＶ３聞いた = 1)
}

proc rpc_PC_発狂＿ＭＳＸ１ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿ＭＳＸ１
	@rp_発狂中大佐顔初期化処理
}

proc rpc_PC_発狂＿ＭＳＸ２ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿ＭＳＸ２
	@rp_発狂中大佐顔初期化処理
}

proc rpc_PC_発狂＿ＭＳＸ３ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿ＭＳＸ３
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿ＭＧＳ１ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿ＭＧＳ１
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿ＭＧＳ２ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿ＭＧＳ２
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿ＭＧＳ３ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿ＭＧＳ３
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿ばべる１ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿ばべる１
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿ばべる２ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿ばべる２
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿ばべる３ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿ばべる３
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿ＶＲ１ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿ＶＲ１
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿ＶＲ２ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿ＶＲ２
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿ＶＲ３ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿ＶＲ３
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿格言１ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿格言１
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿格言２ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿格言２
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿能勢電 {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿能勢電
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿ゲームオーバー回数 {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿ゲームオーバー回数
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿プレイ時間 {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿プレイ時間
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿殺しすぎ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿殺しすぎ
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿無線してない {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿無線してない
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿ツーハン {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔表示
	@rp_PC_発狂＿ツーハン
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿電波 {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿電波
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿エロ行為ツッコミ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿エロ行為ツッコミ
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿らりるれろ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿らりるれろ
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿食事中 {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿食事中
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿前世 {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿前世
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿大佐出撃 {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿大佐出撃
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿借金 {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿借金
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿改造コード {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿改造コード
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿知らない人 {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿知らない人
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿寝ている {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定＿後ろ
	@rpd_顔表示
	@rp_PC_発狂＿寝ている
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿留守電 {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿留守電
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿ローズの浮気 {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿ローズの浮気
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿まめ知識 {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿まめ知識
	@rp_発狂中大佐顔初期化処理
}
proc rpc_PC_発狂＿電車 {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_発狂＿電車
	@rp_発狂中大佐顔初期化処理
}

proc rpc_PC_ソリダス戦＿倒せ {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_ソリダス戦＿倒せ
	@rp_発狂中大佐顔初期化処理
}

proc rpc_PC_ソリダス戦中＿愛国説教 {
	@rp_発狂中大佐顔初期化処理
	@rpd_ライデン顔設定デフォルト
	@rp_大佐発狂中顔表示
	@rp_PC_ソリダス戦中＿愛国説教
	@rp_発狂中大佐顔初期化処理
}


proc rpc_PC_眠りプリスキン撃った {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_眠りプリスキン撃った
	@rp_ローズさんを怒らせました
}


proc rpcc_PC_水中＿スカルスーツ {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_水中＿スカルスーツ＿潜水
	}else {
		@rp_PC_水中＿スカルスーツ
	}
}

proc rpcc_PC_水密扉２ {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_水密扉２＿潜水
	}else {
		@rp_PC_水密扉２
	}
}


proc rpcc_PC_敵武器使用不可１ {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_敵武器使用不可１＿潜水
	}else {
		@rp_PC_敵武器使用不可１
	}
}


proc rpcc_PC_敵武器使用不可２ {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_敵武器使用不可２＿潜水
	}else {
		@rp_PC_敵武器使用不可２
	}
}


proc rpcc_PC_海底ドックＶＲ１ {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_海底ドックＶＲ１＿潜水
	}else {
		@rp_PC_海底ドックＶＲ１
	}
}

proc rpcc_PC_海底ドックＶＲ２ {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_海底ドックＶＲ２＿潜水
	}else {
		@rp_PC_海底ドックＶＲ２
	}
}

proc rpcc_PC_レーション操作説明 {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_レーション操作説明＿潜水
	}else {
		@rp_PC_レーション操作説明
	}
}


proc rpcc_PC_レーション操作説明＿ローズ {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_レーション操作説明＿ローズ＿潜水
	}else {
		@rp_PC_レーション操作説明＿ローズ
	}
}

proc rpcc_PC_ノード前接続前１ {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_ノード前接続前１＿潜水
	}else {
		@rp_PC_ノード前接続前１
	}
}

proc rpcc_PC_海底ドック昇降機前ノード接続後２ {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_海底ドック昇降機前ノード接続後２＿潜水
	}else {
		@rp_PC_海底ドック昇降機前ノード接続後２
	}
}


proc rpcc_PC_海底ドック潜入者 {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_海底ドック潜入者＿潜水
	}else {
		@rp_PC_海底ドック潜入者
	}
}


proc rpcc_PC_流血説明１ {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_流血説明１＿潜水
	}else {
		@rp_PC_流血説明１
	}
}

proc rpcc_PC_フナムシ１ {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_フナムシ１＿潜水
	}else {
		@rp_PC_フナムシ１
	}
}

proc rpcc_PC_フナムシ２ {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_フナムシ２＿潜水
	}else {
		@rp_PC_フナムシ２
	}
}

proc rpcc_PC_ジムスーツ１ {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rp_PC_ジムスーツ１＿潜水
	}else {
		@rp_PC_ジムスーツ１
	}
}




proc rpc_PC_奈落エルード {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_奈落エルード
}

proc rpc_ロッカー説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_ロッカー説明聞いた) {
		command プレイヤー状態取得
		//ロッカーの中にいない
		if(!($status & d:PFLAG_LOCKER) ){
			@rp_PC_ロッカー近く１
			@rp_PC_ロッカー近く２外
		}
		@rp_PC_ロッカー近く２中
		eval($f:rfp_ロッカー説明聞いた = 1 )
//		eval($f:rfp_PC_ロッカー近く１聞いた = 1)
//		eval($f:rfp_PC_ロッカー近く２外聞いた = 1)
//		eval($f:rfp_PC_ロッカー近く２中聞いた = 1)
	
	}else if(!$f:rfp_PC_ロッカー近く３聞いた) {
		@rp_PC_ロッカー近く３
		eval($f:rfp_PC_ロッカー近く３聞いた = 1)
	
	}else if(!$f:rfp_PC_ロッカードア盾聞いた) {
		@rp_PC_ロッカードア盾
		eval($f:rfp_PC_ロッカードア盾聞いた = 1)
	
	}else if(!$f:rfp_PC_ロッカー扉破壊聞いた) {
		@rp_PC_ロッカー扉破壊
		eval($f:rfp_PC_ロッカー扉破壊聞いた = 1)
		eval($f:rfp_PC_ロッカー説明終了 = 1)

	}else {
		eval($f:rfp_PC_ロッカー説明終了 = 1)
		rand 5
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			command プレイヤー状態取得
			//ロッカーの中にいない＝外
			if(!($status & d:PFLAG_LOCKER) ){
				@rp_PC_ロッカー近く１
				@rp_PC_ロッカー近く２外
			}else {
				@rp_PC_ロッカー近く３
			}
		}else if($w:rfp_rand < 2){
			@rp_PC_ロッカー近く２中

		}else if($w:rfp_rand < 3){
			@rp_PC_ロッカー近く３

		}else if($w:rfp_rand < 4){
			@rp_PC_ロッカードア盾

		}else {
			@rp_PC_ロッカー扉破壊
		}
	}
}

proc rpc_水中操作説明＿水上 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_水中＿操作説明聞いた) {
		@rp_PC_水中＿操作説明
		eval($f:rfp_PC_水中＿操作説明聞いた = 1)
	}
	@rp_PC_水中＿水面＿移動
	@rp_PC_水中＿水面＿潜航２
}


proc rpc_水中操作説明＿水中 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_水中＿操作説明聞いた) {
		@rp_PC_水中＿操作説明
		eval($f:rfp_PC_水中＿操作説明聞いた = 1)
	}
	if(!$f:rfp_PC_水中操作説明＿水中聞いた) {
		@rp_PC_水中＿前進
		//こんふぃぐはなし？ＸＸＸＸＸＸＸｘ
		@rp_PC_水中＿左スティック＿正
		@rp_PC_水中＿Ｏ２
		eval($f:rfp_PC_水中操作説明＿水中聞いた = 1)

	}else if(!$f:rfp_PC_水中＿右スティック聞いた) {
		@rp_PC_水中＿右スティック
		eval($f:rfp_PC_水中＿右スティック聞いた = 1)
		
	}else if( ( ($s:エリア == "w31b") || ($s:エリア == "w31c") || ($s:エリア == "w31f") ) && \
				(!$f:rfp_PC_水中＿息継ぎ聞いた) ){
			@rp_PC_水中＿息継ぎ
			eval($f:rfp_PC_水中＿息継ぎ聞いた = 1)

	}else if( ( ($s:エリア == "w31b") || ($s:エリア == "w31c") || ($s:エリア == "w31f") ) && \
				(!$f:rfp_PC_水中＿息継ぎポイント＿光聞いた) ){
			@rp_PC_水中＿息継ぎポイント＿光
			eval($f:rfp_PC_水中＿息継ぎポイント＿光聞いた = 1)

	}else if( ( ( ($s:エリア == "w31b") && ($f:w31b_ノードフラグ) ) || \
				( ( ($s:エリア == "w31c") || ($s:エリア == "w31f") ) && ($f:w31b_ノードフラグ) ) ) && \
				(!($w:コンフィグ設定 & d:CONFIG_RADAR_OFF) ) && \
				(!$f:rfp_PC_水中＿息継ぎポイント＿レーダー聞いた) ){
			@rp_PC_水中＿息継ぎポイント＿レーダー１
			@rp_PC_水中＿息継ぎポイント＿レーダー２
			eval($f:rfp_PC_水中＿息継ぎポイント＿レーダー聞いた = 1)

	}else if(!$f:rfp_PC_水中＿武器ナシ聞いた) {
		@rp_PC_水中＿武器ナシ
		eval($f:rfp_PC_水中＿武器ナシ聞いた = 1)
	

	}else if(!$f:rfp_PC_水中＿息ガマン聞いた) {
		@rp_PC_水中＿息ガマン
		eval($f:rfp_PC_水中＿息ガマン聞いた = 1)

	}else if(!$f:rfp_PC_水中＿スカルスーツ聞いた){
		@rpcc_PC_水中＿スカルスーツ
		eval($f:rfp_PC_水中＿スカルスーツ聞いた = 1)

		eval($f:rfp_PC_水中操作説明＿水中終了 = 1 )
	
	}else {
		eval($f:rfp_PC_水中操作説明＿水中終了 = 1 )
		rand 9
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rp_PC_水中＿前進

		}else if($w:rfp_rand < 2){
			//こんふぃぐはなし？ＸＸＸＸＸＸＸｘ
			@rp_PC_水中＿左スティック＿正

		}else if($w:rfp_rand < 3){
			@rp_PC_水中＿Ｏ２

		}else if($w:rfp_rand < 4){
			@rp_PC_水中＿右スティック

		}else if($w:rfp_rand < 5){
			@rp_PC_水中＿武器ナシ

		}else if($w:rfp_rand < 6){
			if( ( ( ($s:エリア == "w31b") && ($f:w31b_ノードフラグ) ) || \
				( ( ($s:エリア == "w31c") || ($s:エリア == "w31f") ) && ($f:w31b_ノードフラグ) ) ) ){
				@rp_PC_水中＿息継ぎ
			}else {
				@rp_PC_水中＿前進
			}

		}else if($w:rfp_rand < 7){
			if( ( ($s:エリア == "w31b") || ($s:エリア == "w31c") || ($s:エリア == "w31f") ) && \
				(!($w:コンフィグ設定 & d:CONFIG_RADAR_OFF) ) ){
				@rp_PC_水中＿息継ぎポイント＿レーダー２
			}else {
				@rp_PC_水中＿Ｏ２
			}

		}else if($w:rfp_rand < 8){
			if( ($s:エリア == "w31b") || ($s:エリア == "w31c") || ($s:エリア == "w31f") ){
				@rp_PC_水中＿息継ぎポイント＿光
			}else {
				@rp_PC_水中＿右スティック
			}

		}else {
			@rp_PC_水中＿息ガマン
		}
	}
}


proc rpc_PC_流血説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_流血説明１聞いた) {
		@rpcc_PC_流血説明１
	}
	eval($f:rfp_PC_流血説明１聞いた = 1)

	//むびＸＸＸＸＸ
	@rp_PC_流血説明２

	@rp_PC_流血説明３
	
	//しゃがんで流血回復ムービー ＸＸＸＸＸＸむび
	@rp_PC_流血説明４
	eval($f:rfp_PC_流血説明聞いた = 1)
}

proc rpc_PC_パンチ説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if( ($w:武器弾数[1] < 0) && ($w:武器弾数[2] < 0) ) { //Ｍ９とＵＳＰ
		@rp_PC_戦闘＿素手
	}else {
		@rp_PC_戦闘＿パンチ
	}
}


proc rpc_PC_戦闘＿投げ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_戦闘＿投げ
}


proc rpc_PC_戦闘＿首絞め {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_戦闘＿首絞め
}

proc rpc_PC_主観左右ステップ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_主観左右ステップ
}

proc rpc_PC_無線操作 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_無線操作
}





proc rpc_PC_ビハインド {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ビハインド１
	@rp_PC_ビハインド２
}

proc rpc_PC_壁叩き {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_壁叩き１
	if(!$f:rfp_PC_壁叩き聞いた) {
		@rp_PC_壁叩き２
		eval($f:rfp_PC_壁叩き聞いた = 1)
	}
}

proc rpc_PC_アイテムボックス {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_アイテムボックス
}

proc rpc_PC_アイテムボックス入手 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_アイテムボックス入手
}

proc rpc_PC_敵兵盾 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_敵兵盾
}

proc rpc_PC_見つかるな注意 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_見つかるな注意
}




proc rpc_PC_主観攻撃 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_主観攻撃聞いた) {
		@rp_PC_主観攻撃１
		@rp_PC_主観攻撃２
	}else {
		rand 2
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rp_PC_主観攻撃１
		}else {
			@rp_PC_主観攻撃２
		}
	}
}

proc rpc_PC_主観攻撃＿左右ステップ１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_主観攻撃＿左右ステップ１
}




proc rpc_PC_銃構えおろし {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_銃構えおろし１＿１

	//ローズさん
	@rp_ローズさんご機嫌判定
	if( ($w:p_story >= d:ST:P004_02_R01ノード初接続２無線デモ１終了) && \
		(!$f:rfp_PC_銃構えおろし聞いた) && \
		(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpd_ローズ顔表示
		@rp_PC_銃構えおろし２
		eval($f:rfp_PC_銃構えおろし聞いた = 1)
	}
}

proc rpc_PC_麻酔部位ダメージ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_麻酔部位ダメージ
}

proc rpc_PC_部位ダメージ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_部位ダメージ
}

proc rpc_PC_無線機破壊 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_無線機破壊
}

proc rpc_PC_ローリング {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ローリング
}

proc rpc_PC_そろそろ歩き {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_そろそろ歩き
}

proc rpc_PC_覗き込み {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_覗き込み
}

proc rpc_PC_右スティック {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_右スティック
}

proc rpc_PC_飛び出し撃ち {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_飛び出し撃ち
}

proc rpc_PC_インケン投げ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_インケン投げ
}

proc rpc_PC_影が見つかる {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_影が見つかる
}

proc rpc_PC_コワレモノ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_コワレモノ
}

proc rpc_PC_探索モード {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_探索モード
}

proc rpc_PC_ヒドゥンポイント {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ヒドゥンポイント
}

proc rpc_PC_マガジン投げ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_マガジン投げ
}



proc rpc_PC_武器装備品装備法 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_武器装備品装備法１
	@rp_PC_武器装備品装備法２
}

proc rpc_PC_ＣＡＬＬ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＣＡＬＬ
}

proc rpc_PC_足跡 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_足跡
}




proc rpc_PC_イントルード {
/*
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
*/
	if(!$f:rfp_PC_イントルード聞いた) {
		@rp_PC_イントルード１
		@rp_PC_イントルード２
		@rp_PC_イントルード３
		@rp_PC_イントルード＿銃撃てる
		eval($f:rfp_PC_イントルード聞いた = 1)

	}else {
		rand 2
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rp_PC_イントルード１
			@rp_PC_イントルード３
		}else {
			@rp_PC_イントルード１
			@rp_PC_イントルード＿銃撃てる
		}
	}
}

proc rpc_PC_イントルード＿危険性 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_イントルード＿危険性
}

proc rpc_PC_段差上り１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_段差上り１
}




proc rpc_PC_エルード１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エルード１の１
	//むびＸＸＸＸＸＸＸ
	@rp_PC_エルード１の２
}

proc rpc_PC_エルード２ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エルード２の１
	//むびＸＸＸＸＸＸＸ
	@rp_PC_エルード２の２
}

proc rpc_PC_エルード３ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エルード３の１
	//むびＸＸＸＸＸＸＸ
	@rp_PC_エルード３の２
}

proc rpc_PC_エルード＿飛び降り {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エルード＿飛び降り１
	//むびＸＸＸＸＸＸＸＸＸＸＸＸＸ
	@rp_PC_エルード＿飛び降り２
}


proc rpc_エルード説明 {
	command プレイヤー状態取得
	
	if( (!$f:rfp_PC_エルード１聞いた) && (!($status & d:PFLAG_ELUDE) ) ){
		@rpc_PC_エルード１
		eval($f:rfp_PC_エルード１聞いた = 1)

	}else if(!$f:rfp_PC_エルード２聞いた) {
		@rpc_PC_エルード２
		eval($f:rfp_PC_エルード２聞いた = 1)

	}else if(!$f:rfp_PC_エルード３聞いた) {
		@rpc_PC_エルード３
		eval($f:rfp_PC_エルード３聞いた = 1)

	}else if(!$f:rfp_PC_エルード＿飛び降り聞いた) {
		@rpc_PC_エルード＿飛び降り
		eval($f:rfp_PC_エルード＿飛び降り聞いた = 1)

		eval($f:rfp_PC_エルード説明終了 = 1)

	}else {
		eval($f:rfp_PC_エルード説明終了 = 1)
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rpc_PC_エルード１

		}else if($w:rfp_rand < 2){
			@rpc_PC_エルード２

		}else if($w:rfp_rand < 3){
			@rpc_PC_エルード３

		}else {
			@rpc_PC_エルード＿飛び降り
		}
	}
}


proc rpc_PC_死体引きずり方法 {
	@rp_PC_死体引きずり方法１
	//むびＸＸＸＸＸＸＸｘ
	@rp_PC_死体引きずり方法２
	eval($f:rfp_PC_死体引きずり方法聞いた = 1)
}

proc rpc_PC_死体ひきずり {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_死体ひきずり聞いた) {
		@rp_PC_死体隠せ
	}
	eval($f:rfp_PC_死体ひきずり聞いた = 1)
	@rpc_PC_死体引きずり方法
}

proc rpc_PC_死体硬直 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_死体硬直１
	@rp_PC_死体硬直２
}

proc rpc_PC_ホフク説明２ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ホフク説明２
}

proc rpc_PC_死体定時連絡 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_死体定時連絡
}




proc rpc_PC_仲間を呼ばせない方法 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_仲間を呼ばせない方法聞いた) {
		@rp_PC_敵の無線連絡２
		@rp_PC_仲間を呼ばせない方法１
		@rp_PC_仲間を呼ばせない方法＿倒す
		@rp_PC_仲間を呼ばせない方法＿チャフ
		@rp_PC_仲間を呼ばせない方法＿無線破壊
		eval($f:rfp_PC_仲間を呼ばせない方法聞いた = 1)
	}else {
		@rp_PC_仲間を呼ばせない方法１

		rand 3
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rp_PC_仲間を呼ばせない方法＿倒す
		}else if($w:rfp_rand < 2){
			@rp_PC_仲間を呼ばせない方法＿チャフ
		}else {
			@rp_PC_仲間を呼ばせない方法＿無線破壊
		}
	}
}

proc rpc_PC_敵からのアイテム入手１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_敵からのアイテム入手１
}

proc rpc_PC_ホールドアップ１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ホールドアップ１
}


proc rpc_PC_足音立てない移動 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_足音立てない移動聞いた) {
		@rp_PC_鳴り床＿そろそろ歩き
		@rp_PC_鳴り床＿ホフク
	}else {
		rand 2
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rp_PC_鳴り床＿そろそろ歩き
		}else {
			@rp_PC_鳴り床＿ホフク
		}
	}
}

proc rpc_PC_Ｌ１ボタンロックオン {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｌ１ボタンロックオン
}

proc rpc_デフォルト操作説明普通 {
	if(!$f:rfp_PC_流血説明聞いた) {
		@rpc_PC_流血説明
		eval($f:rfp_PC_流血説明聞いた = 1)
	
	}else if(!$f:rfp_PC_パンチ説明聞いた ){
		@rpc_PC_パンチ説明
		eval($f:rfp_PC_パンチ説明聞いた = 1)

	}else if(!$f:rfp_PC_戦闘＿投げ聞いた){
		@rpc_PC_戦闘＿投げ
		eval($f:rfp_PC_戦闘＿投げ聞いた = 1)
	
	}else if(!$f:rfp_PC_戦闘＿首絞め聞いた){
		@rpc_PC_戦闘＿首絞め
		eval($f:rfp_PC_戦闘＿首絞め聞いた = 1)
		
	}else if(!$f:rfp_PC_主観左右ステップ聞いた) {
		@rpc_PC_主観左右ステップ
		eval($f:rfp_PC_主観左右ステップ聞いた = 1)
	
	}else if(!$f:rfp_PC_無線操作聞いた) {
		@rpc_PC_無線操作
		eval($f:rfp_PC_無線操作聞いた = 1)
	
	}else if(!$f:rfp_PC_ビハインド聞いた) {
		@rpc_PC_ビハインド
		eval($f:rfp_PC_ビハインド聞いた = 1)

	}else if(!$f:rfp_PC_壁叩き聞いた) {
		@rpc_PC_壁叩き
		eval($f:rfp_PC_壁叩き聞いた = 1)

	}else if(!$f:rfp_PC_アイテムボックス聞いた) {
		@rpc_PC_アイテムボックス
		eval($f:rfp_PC_アイテムボックス聞いた = 1)

	}else if(!$f:rfp_PC_アイテムボックス入手聞いた) {
		@rpc_PC_アイテムボックス入手
		eval($f:rfp_PC_アイテムボックス入手聞いた = 1)

	}else if(!$f:rfp_PC_敵兵盾聞いた) {
		@rpc_PC_敵兵盾
		eval($f:rfp_PC_敵兵盾聞いた = 1)

	}else if(!$f:rfp_PC_見つかるな注意聞いた) {
		@rpc_PC_見つかるな注意
		eval($f:rfp_PC_見つかるな注意聞いた = 1)

	}else if(!$f:rfp_PC_主観攻撃聞いた) {
		@rpc_PC_主観攻撃
		eval($f:rfp_PC_主観攻撃聞いた = 1)

	}else if(!$f:rfp_PC_主観攻撃＿左右ステップ１聞いた) {
		@rpc_PC_主観攻撃＿左右ステップ１
		eval($f:rfp_PC_主観攻撃＿左右ステップ１聞いた = 1)

	}else if( (!$f:rfp_PC_銃構えおろし聞いた) && \
				( ($w:武器弾数Ｒ[d:武器:Ｍ９] >= 0) || ($w:武器弾数Ｒ[d:武器:ソコム] >= 0) ) ){
		@rpc_PC_銃構えおろし
		eval($f:rfp_PC_銃構えおろし聞いた = 1)

	}else if( (!$f:rfp_PC_麻酔部位ダメージ聞いた) && ($w:武器弾数[1] >= 0) ){	//Ｍ９もってたらんちょ
		@rpc_PC_麻酔部位ダメージ
		eval($f:rfp_PC_麻酔部位ダメージ聞いた = 1)

	}else if(!$f:rfp_PC_部位ダメージ聞いた) {
		@rpc_PC_部位ダメージ
		eval($f:rfp_PC_部位ダメージ聞いた = 1)

	}else if(!$f:rfp_PC_無線機破壊聞いた) {
		@rpc_PC_無線機破壊
		eval($f:rfp_PC_無線機破壊聞いた = 1)

	}else if(!$f:rfp_PC_ローリング聞いた) {
		@rpc_PC_ローリング
		eval($f:rfp_PC_ローリング聞いた = 1)

	}else if(!$f:rfp_PC_そろそろ歩き聞いた) {
		@rpc_PC_そろそろ歩き
		eval($f:rfp_PC_そろそろ歩き聞いた = 1)

	}else if(!$f:rfp_PC_覗き込み聞いた) {
		@rpc_PC_覗き込み
		eval($f:rfp_PC_覗き込み聞いた = 1)

	}else if(!$f:rfp_PC_右スティック聞いた) {
		@rpc_PC_右スティック
		eval($f:rfp_PC_右スティック聞いた = 1)

	}else if(!$f:rfp_PC_飛び出し撃ち聞いた) {
		@rpc_PC_飛び出し撃ち
		eval($f:rfp_PC_飛び出し撃ち聞いた = 1)

	}else if(!$f:rfp_PC_インケン投げ聞いた) {
		@rpc_PC_インケン投げ
		eval($f:rfp_PC_インケン投げ聞いた = 1)

	}else if(!$f:rfp_PC_影が見つかる聞いた) {
		@rpc_PC_影が見つかる
		eval($f:rfp_PC_影が見つかる聞いた = 1)

	}else if(!$f:rfp_PC_コワレモノ聞いた) {
		@rpc_PC_コワレモノ
		eval($f:rfp_PC_コワレモノ聞いた = 1)

	}else if(!$f:rfp_PC_探索モード聞いた) {
		@rpc_PC_探索モード
		eval($f:rfp_PC_探索モード聞いた = 1)

	}else if(!$f:rfp_PC_ヒドゥンポイント聞いた) {
		@rpc_PC_ヒドゥンポイント
		eval($f:rfp_PC_ヒドゥンポイント聞いた = 1)

	}else if(!$f:rfp_PC_マガジン投げ聞いた) {
		@rpc_PC_マガジン投げ
		eval($f:rfp_PC_マガジン投げ聞いた = 1)

	//ＸＸＸＸＸＸＸクラスター式武器セレクトの時だけ
	}else if( (!$f:rfp_PC_武器装備品装備法聞いた) && \
			 (!($w:コンフィグ設定 & d:CONFIG_OLD_TYPE_MENU)) ){
		@rpc_PC_武器装備品装備法
		eval($f:rfp_PC_武器装備品装備法聞いた = 1)

	//ＣＡＬＬ聞いてない時のみ？ ＸＸＸＸＸＸＸＸＸｘ
	}else if(!$f:rfp_PC_ＣＡＬＬ聞いた) {
		@rpc_PC_ＣＡＬＬ
		eval($f:rfp_PC_ＣＡＬＬ聞いた = 1)

	}else if(!$f:rfp_PC_足跡聞いた) {
		@rpc_PC_足跡
		eval($f:rfp_PC_足跡聞いた = 1)

	}else if(!$f:rfp_PC_イントルード聞いた) {
		@rpd_ライデン顔設定デフォルト
		@rpd_大佐顔設定１
		@rpd_顔表示

		@rpc_PC_イントルード
		eval($f:rfp_PC_イントルード聞いた = 1)

	}else if(!$f:rfp_PC_イントルード＿危険性聞いた) {
		@rpc_PC_イントルード＿危険性
		eval($f:rfp_PC_イントルード＿危険性聞いた = 1)

	}else if(!$f:rfp_PC_段差上り１聞いた) {
		@rpc_PC_段差上り１
		eval($f:rfp_PC_段差上り１聞いた = 1)

	}else if(!$f:rfp_PC_エルード１聞いた) {
		@rpc_PC_エルード１
		eval($f:rfp_PC_エルード１聞いた = 1)

	}else if(!$f:rfp_PC_エルード２聞いた) {
		@rpc_PC_エルード２
		eval($f:rfp_PC_エルード２聞いた = 1)

	}else if(!$f:rfp_PC_エルード３聞いた) {
		@rpc_PC_エルード３
		eval($f:rfp_PC_エルード３聞いた = 1)

	}else if(!$f:rfp_PC_エルード＿飛び降り聞いた) {
		@rpc_PC_エルード＿飛び降り
		eval($f:rfp_PC_エルード＿飛び降り聞いた = 1)

	}else if(!$f:rfp_PC_死体ひきずり聞いた) {
		@rpc_PC_死体ひきずり
		eval($f:rfp_PC_死体ひきずり聞いた = 1)
/*
	}else if(!$f:rfp_PC_死体硬直聞いた) {
		@rpc_PC_死体硬直
		eval($f:rfp_PC_死体硬直聞いた = 1)
*/

	}else if(!$f:rfp_PC_ホフク説明２聞いた) {
		@rpc_PC_ホフク説明２
		eval($f:rfp_PC_ホフク説明２聞いた = 1)

	}else if(!$f:rfp_PC_死体定時連絡聞いた) {
		@rpc_PC_死体定時連絡
		eval($f:rfp_PC_死体定時連絡聞いた = 1)

	}else if(!$f:rfp_PC_仲間を呼ばせない方法聞いた) {
		@rpc_PC_仲間を呼ばせない方法
		eval($f:rfp_PC_仲間を呼ばせない方法聞いた = 1)

	}else if(!$f:rfp_PC_敵からのアイテム入手１聞いた) {
		@rpc_PC_敵からのアイテム入手１
		eval($f:rfp_PC_敵からのアイテム入手１聞いた = 1)

	}else if(!$f:rfp_PC_ホールドアップ１聞いた) {
		@rpc_PC_ホールドアップ１
		eval($f:rfp_PC_ホールドアップ１聞いた = 1)

	}else if(!$f:rfp_PC_足音立てない移動聞いた) {
		@rpc_PC_足音立てない移動
		eval($f:rfp_PC_足音立てない移動聞いた = 1)

	}else if(!$f:rfp_PC_Ｌ１ボタンロックオン聞いた) {
		@rpc_PC_Ｌ１ボタンロックオン
		eval($f:rfp_PC_Ｌ１ボタンロックオン聞いた = 1)

//		eval($f:rfp_PC_デフォルト操作説明普通聞いた = 1)
/*
	}else if(!$f:rfp_聞いた) {
		@rp_
		eval($f:rfp_聞いた = 1)
*/

	//ちょーランダム
	}else {
		rand 46
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rpc_PC_流血説明
	
		}else if($w:rfp_rand < 2){
			@rpc_PC_パンチ説明

		}else if($w:rfp_rand < 3){
			@rpc_PC_戦闘＿投げ
	
		}else if($w:rfp_rand < 4){
			@rpc_PC_戦闘＿首絞め

		}else if($w:rfp_rand < 5){
			@rpc_PC_主観左右ステップ
	
		}else if($w:rfp_rand < 6){
			@rpc_PC_無線操作

		}else if($w:rfp_rand < 7){
			@rpc_PC_ビハインド

		}else if($w:rfp_rand < 8){
			@rpc_PC_壁叩き

		}else if($w:rfp_rand < 9){
			@rpc_PC_アイテムボックス

		}else if($w:rfp_rand < 10){
			@rpc_PC_アイテムボックス入手

		}else if($w:rfp_rand < 11){
			@rpc_PC_敵兵盾

		}else if($w:rfp_rand < 12){
			@rpc_PC_見つかるな注意

		}else if($w:rfp_rand < 13){
			@rpc_PC_主観攻撃

		}else if($w:rfp_rand < 14){
			@rpc_PC_主観攻撃＿左右ステップ１

		}else if( ($w:rfp_rand < 15) && \
				( ($w:武器弾数Ｒ[d:武器:Ｍ９] >= 0) || ($w:武器弾数Ｒ[d:武器:ソコム] >= 0) ) ){
			@rpc_PC_銃構えおろし

		//Ｍ９持ってる時のみ？ＸＸＸＸＸＸＸｘｘ
		}else if( ($w:rfp_rand < 16) && ($w:武器弾数Ｒ[d:武器:Ｍ９] >= 0) ){
			@rpc_PC_麻酔部位ダメージ

		}else if($w:rfp_rand < 17){
			@rpc_PC_部位ダメージ

		}else if($w:rfp_rand < 18){
			@rpc_PC_無線機破壊

		}else if($w:rfp_rand < 19){
			@rpc_PC_ローリング

		}else if($w:rfp_rand < 20){
			@rpc_PC_そろそろ歩き

		}else if($w:rfp_rand < 21){
			@rpc_PC_覗き込み

		}else if($w:rfp_rand < 22){
			@rpc_PC_右スティック

		}else if($w:rfp_rand < 23){
			@rpc_PC_飛び出し撃ち

		}else if($w:rfp_rand < 24){
			@rpc_PC_インケン投げ

		}else if($w:rfp_rand < 25){
			@rpc_PC_影が見つかる

		}else if($w:rfp_rand < 26){
			@rpc_PC_コワレモノ

		}else if($w:rfp_rand < 27){
			@rpc_PC_探索モード

		}else if($w:rfp_rand < 28){
			@rpc_PC_ヒドゥンポイント

		}else if($w:rfp_rand < 29){
			@rpc_PC_マガジン投げ

	//ＸＸＸＸＸＸＸクラスター式武器セレクトの時だけ
		}else if( ($w:rfp_rand < 30) && \
			 (!($w:コンフィグ設定 & d:CONFIG_OLD_TYPE_MENU)) ){
			@rpc_PC_武器装備品装備法

	//ＣＡＬＬ聞いてない時のみ？ ＸＸＸＸＸＸＸＸＸｘ
		}else if($w:rfp_rand < 31){
			@rpc_PC_ＣＡＬＬ

		}else if($w:rfp_rand < 32){
			@rpc_PC_足跡

		}else if($w:rfp_rand < 33){
			@rpd_ライデン顔設定デフォルト
			@rpd_大佐顔設定１
			@rpd_顔表示

			@rpc_PC_イントルード

		}else if($w:rfp_rand < 34){
			@rpc_PC_イントルード＿危険性

		}else if($w:rfp_rand < 35){
			@rpc_PC_段差上り１

		}else if($w:rfp_rand < 36){
			@rpc_PC_エルード１

		}else if($w:rfp_rand < 37){
			@rpc_PC_エルード２

		}else if($w:rfp_rand < 38){
			@rpc_PC_エルード３

		}else if($w:rfp_rand < 39){
			@rpc_PC_エルード＿飛び降り

		}else if($w:rfp_rand < 40){
			@rpc_PC_死体ひきずり
/*
		}else if($w:rfp_rand < 41){
			@rpc_PC_死体硬直
*/

		}else if($w:rfp_rand < 41){
			@rpc_PC_ホフク説明２

		}else if($w:rfp_rand < 42){
			@rpc_PC_死体定時連絡

		}else if($w:rfp_rand < 43){
			@rpc_PC_仲間を呼ばせない方法

		}else if($w:rfp_rand < 44){
			@rpc_PC_敵からのアイテム入手１

		}else if($w:rfp_rand < 45){
			@rpc_PC_ホールドアップ１

		}else{
			@rpc_PC_足音立てない移動
		}
	}
}

proc rpc_デフォルト操作説明普通＿潜水 {
	//今は仮すま
	@rpc_デフォルト操作説明普通
}

proc rpc_デフォルト操作説明危険 {
	//今は仮すま
	@rpc_デフォルト操作説明普通
}

proc rpc_デフォルト操作説明危険＿潜水 {
	//今は仮すま
	@rpc_デフォルト操作説明普通＿潜水
}


proc rpc_PC_銃入手注意 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_銃入手注意
}

proc rpc_PC_Ｆ脚武器入手後 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｆ脚武器入手後
}

proc rpc_PC_Ｅ脚オルガ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｅ脚オルガ
	eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
}

proc rpc_オルガうんちく {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_オルガうんちく１
	@rpd_ローズ顔表示
	@rp_PC_オルガうんちく３
}

proc rpc_PC_フォーチュン１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_フォーチュン１
	eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
}

proc rpc_フォーチュン詳細 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_フォーチュン２
	@rpd_ローズ顔表示
	@rp_PC_フォーチュン３
}

proc rpc_PC_ミスターＸ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ミスターＸ
	eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
	
}

proc rpc_ミスターＸ詳細 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ミスターＸ２
	@rpd_ローズ顔表示
	@rp_PC_ミスターＸ３
}

proc rpc_PC_デッドセルについて {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_デッドセルについて１
	if($w:タンカー編クリア回数 > 0){
		@rp_PC_デッドセルについて２
	}
	@rpd_ローズ顔表示
	@rp_PC_デッドセルについて３
}

proc rpc_PC_プリスキン信用するな {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_プリスキン信用するな
}

proc rpc_PC_プリスキン生きていた {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_プリスキン生きていた
	eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
}

proc rpc_PC_忍者接触後＿忍者 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_忍者接触後＿忍者
}

proc rpc_PC_ＶＲ訓練について {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＶＲ訓練について
}

proc rpc_PC_スネークについて {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_スネークについて
}

proc rpc_PC_タンカー事件について {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_タンカー事件について１
}

proc rpc_PC_一般機能説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔表示
	@rpd_顔表示

	@rp_PC_一般機能説明
}

proc rpc_PC_Ａ脚機能説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔表示
	@rpd_顔表示

	@rp_PC_Ａ脚機能説明
}

proc rpc_PC_Ｂ脚機能説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔表示
	@rpd_顔表示

	@rp_PC_Ｂ脚機能説明
}


proc rpc_PC_Ｃ脚機能説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔表示
	@rpd_顔表示

	@rp_PC_Ｃ脚機能説明
}

proc rpc_PC_Ｄ脚機能説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔表示
	@rpd_顔表示

	@rp_PC_Ｄ脚機能説明
}

proc rpc_PC_Ｅ脚機能説明＿集配場 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔設定１
	@rpd_顔表示

	@rp_PC_Ｅ脚機能説明１
	@rp_PC_Ｅ脚機能説明＿集配場
}

proc rpc_PC_Ｅ脚機能説明＿屋上 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔設定１
	@rpd_顔表示

	@rp_PC_Ｅ脚機能説明１
	@rp_PC_Ｅ脚機能説明＿屋上
}

proc rpc_PC_Ｆ脚機能説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔設定１
	@rpd_顔表示

	@rp_PC_Ｆ脚機能説明
}

proc rpc_PC_シェル１中央棟機能説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔設定１
	@rpd_顔表示

	@rp_PC_シェル１中央棟機能説明
}

proc rpc_PC_シェル２中央棟機能説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔設定１
	@rpd_顔表示

	@rp_PC_シェル２中央棟機能説明
}

proc rpc_PS_Ｌ脚機能説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔設定１
	@rpd_顔表示

	@rp_PS_Ｌ脚機能説明
}

proc rpc_PC_ハリアーうんちく {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ハリアーうんちく
}

proc rpc_PC_報復 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_報復
}

proc rpc_PC_有毒物質 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_有毒物質
}

proc rpc_ピーター詳細 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ピーター詳細１
	@rpd_ローズ顔表示
	@rp_PC_ピーター詳細２
	@rp_PC_ピーター詳細３
}

proc rpc_PC_ファットマンについて１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ファットマンについて１
}

proc rpc_ファットマン詳細 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ファットマンについて２
	@rpd_ローズ顔表示
	@rp_PC_ファットマンについて３
	if( (!$f:rfp_PP_ファットマン聞いた) && \
		($w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１開始) ){
		@rp_PC_ファットマンについて４
	}
}

proc rpc_PC_エイムズ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エイムズ１
	@rpd_ローズ顔表示
	@rp_PC_エイムズ２
	eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
}

proc rpc_エイムズ詳細 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エイムズ２＿２
	@rpd_ローズ顔表示
	@rp_PC_エイムズ３
	@rpd_大佐顔表示
	@rp_PC_エイムズ４
}

proc rpc_PC_エイムズ死後＿ブラックケース１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エイムズ死後＿ブラックケース１
	eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
}

proc rpc_ブラックケース詳細 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エイムズ死後＿ブラックケース２
	@rpd_ローズ顔表示
	@rp_PC_エイムズ死後＿ブラックケース３
	@rpd_大佐顔表示
	@rp_PC_エイムズ死後＿ブラックケース４
}

proc rpc_PC_エイムズ死後＿アウターヘブン  {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エイムズ死後＿アウターヘブン 
}

proc rpc_PC_エイムズ死後＿テロボススネーク {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エイムズ死後＿テロボススネーク
}

proc rpc_PC_ハリアー戦後＿スネーク {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ハリアー戦後＿スネーク
}

proc rpc_PC_愛国者達 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_愛国者達
	eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
}

proc rpc_PC_大統領後＿なぜ大統領を殺したのか {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_大統領後＿なぜ大統領を殺したのか
}


proc rpc_PC_核について {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_核について
	eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
}

proc rpc_PC_エイムズ死後＿ＮＥＭＰ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エイムズ死後＿ＮＥＭＰ
}

proc rpc_PC_エイムズ死後＿忍者 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔設定１
	@rpd_顔表示

	@rp_PC_エイムズ死後＿忍者
}

proc rpc_PC_大統領後＿スネーク {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_大統領後＿スネーク
	eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
}

proc rpc_PC_エイムズ死後＿人質 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エイムズ死後＿人質
}

proc rpc_PC_ヴァンプについて１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ヴァンプについて１
}

proc rpc_ヴァンプ詳細 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ヴァンプについて２
	@rpd_ローズ顔表示
	@rp_PC_ヴァンプについて３
}

proc rpc_PC_怪奇箱男１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_怪奇箱男１
	eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
}

proc rpc_怪奇箱男詳細 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_怪奇箱男２
	@rpd_ローズ顔表示
	@rp_PC_怪奇箱男３
}

proc rpc_PC_ピーター死＿爆発余波 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ピーター死＿爆発余波
}

proc rpc_PC_爆弾解体プリスキン怪しい {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_爆弾解体プリスキン怪しい
}

proc rpc_連続無線デフォルト {
	@rp_ローズさんご機嫌判定
	if( (!$f:rfp_PC_Ｅ脚オルガ聞いた) && \
				($f:w20b_オルガデモフラグ) && \
				($w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１開始) && \
				($w:rfp_callcount > 1) ){
		@rpc_PC_Ｅ脚オルガ
		eval($f:rfp_PC_Ｅ脚オルガ聞いた = 1)
		eval($i:rfp_イベント開始時刻２ = $i:プレイタイム)

	}else if( (!$f:rfp_PC_ミスターＸ聞いた) && \
				(	(	($f:rfp_w21a_地雷原１無線デモ１した) &&\
						(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
							$w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１開始) ) || \
					(	($f:rfp_w12c_地雷原１無線デモ１した) &&\
						(d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了 <= $w:p_story && \
							$w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始) )  ) && \
				 ($w:rfp_callcount > 1) ){
		@rpc_PC_ミスターＸ
		eval($f:rfp_PC_ミスターＸ聞いた = 1)
		eval($i:rfp_PC_ミスターＸ聞いた開始時刻 = $i:プレイタイム)

	}else if( (!$f:rfp_PC_怪奇箱男１聞いた) && \
				($f:rfp_箱男見た) && \
				(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
					$w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_怪奇箱男１
		eval($f:rfp_PC_怪奇箱男１聞いた = 1)
		eval($i:rfp_PC_怪奇箱男１聞いた時刻 = $i:プレイタイム)

	}else if( (!$f:rfp_PC_プリスキン生きていた聞いた) && \
				(d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P038_01_S01網膜チェック１シナリオデモ１開始) && \
				 (!$f:rfp_ローズさん落込んでます) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_プリスキン生きていた
		eval($f:rfp_PC_プリスキン生きていた聞いた = 1)

	}else if((!$f:rfp_PC_ピーター死＿爆発余波聞いた) && \
				(d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１終了 <= $w:p_story && \
					$w:p_story <  d:ST:P026_01_R01爆弾解体終了１無線デモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_ピーター死＿爆発余波
		eval($f:rfp_PC_ピーター死＿爆発余波聞いた = 1)

	}else if( (!$f:rfp_PC_報復聞いた) && \
				( 	(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
						$w:p_story < d:ST:P026_01_R01爆弾解体終了１無線デモ１開始) || \
					(d:ST:P029_01フォーチュン戦終了<= $w:p_story && \
						$w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始) ) && \
				 ($w:rfp_callcount > 1) ){
		@rpc_PC_報復
		eval($f:rfp_PC_報復聞いた = 1)

	}else if( (!$f:rfp_PC_有毒物質聞いた) && \
				( 	(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
						$w:p_story < d:ST:P026_01_R01爆弾解体終了１無線デモ１開始) || \
					(d:ST:P029_01フォーチュン戦終了<= $w:p_story && \
						$w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始) ) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_有毒物質
		eval($f:rfp_PC_有毒物質聞いた = 1)

	}else if( (!$f:rfp_PC_ハリアーうんちく聞いた) && \
				(!$f:rfp_ハリアー見た) && \
				(d:ST:P012_02_R01フォーチュン遭遇２無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_ハリアーうんちく
		eval($f:rfp_PC_ハリアーうんちく聞いた = 1)


	}else if( (d:ST:P010_10_P05ヴァンプ遭遇１０ポリゴンデモ５終了 <= $w:p_story && \
			$w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１開始) && \
		(!$f:rfp_PC_デッドセルについて聞いた) && ($w:rfp_callcount > 1) && \
		(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_デッドセルについて
		eval($f:rfp_PC_デッドセルについて聞いた = 1)
//		eval($i:rfp_PC_デッドセルについて聞いた時刻 = $:プレイタイム )


	}else if( (!$f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:オルガうんちく]) && \
				($f:rfp_PC_Ｅ脚オルガ聞いた) && \
				( ( ($i:プレイタイム - $i:rfp_イベント開始時刻２) / d:FRAME_RATE ) >= 300 ) && \
				 ($w:rfp_callcount > 1) && \
				 (!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_オルガうんちく
		eval($f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:オルガうんちく] = 1)


	}else if( (!$f:rfp_PC_フォーチュン１聞いた) && \
				(	(d:ST:P012_02_R01フォーチュン遭遇２無線デモ１終了 <= $w:p_story && \
						$w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１開始) || \
					(d:ST:P028_01_P01爆弾解体後フォーチュン再登場１ポリゴンデモ１終了 <= $w:p_story && \
						$w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始) ) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_フォーチュン１
		eval($f:rfp_PC_フォーチュン１聞いた = 1)
		eval($i:rfp_PC_フォーチュン１聞いた時刻 = $i:プレイタイム)

	}else if( (!$f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:フォーチュン詳細]) && \
				($f:rfp_PC_フォーチュン１聞いた) && \
				( ( ($i:プレイタイム - $i:rfp_PC_フォーチュン１聞いた時刻) / d:FRAME_RATE ) >= 300 ) && \
				(	(d:ST:P012_02_R01フォーチュン遭遇２無線デモ１終了 <= $w:p_story && \
						$w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１開始) || \
					(d:ST:P028_01_P01爆弾解体後フォーチュン再登場１ポリゴンデモ１終了 <= $w:p_story && \
						$w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始)  ) && \
				 ($w:rfp_callcount > 1) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_フォーチュン詳細
		eval($f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:フォーチュン詳細] = 1)

	}else if( (!$f:rfp_PC_ヴァンプについて１聞いた) && \
				(d:ST:P010_10_P05ヴァンプ遭遇１０ポリゴンデモ５終了 <= $w:p_story && \
						$w:p_story < d:ST:P012_01_P01フォーチュン遭遇１ポリゴンデモ１開始) && \
//					(d:ST:P046_02_R01ハリアー戦勝利２無線デモ１終了 <= $w:p_story && \
//						$w:p_story < d:ST:P047_01_P01電撃床前オルガ１ポリゴンデモ１開始) ) && \
				 ($w:rfp_callcount > 1) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_ヴァンプについて１
		eval($f:rfp_PC_ヴァンプについて１聞いた = 1)
		eval($i:rfp_PC_ヴァンプについて１聞いた時刻 = $i:プレイタイム)

	}else if( (!$f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:ヴァンプ詳細]) && \
				($f:rfp_PC_ヴァンプについて１聞いた) && \
				( ( ($i:プレイタイム - $i:rfp_PC_ヴァンプについて１聞いた時刻) / d:FRAME_RATE ) >= 300 ) && \
				 ($w:rfp_callcount > 1) && \
				 ($w:p_story < d:ST:P046_02_R01ハリアー戦勝利２無線デモ１終了) && \
//				(	(d:ST:P012_02_R01フォーチュン遭遇２無線デモ１終了 <= $w:p_story && \
//						$w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１開始) || \
//					(d:ST:P028_01_P01爆弾解体後フォーチュン再登場１ポリゴンデモ１終了 <= $w:p_story && \
//						$w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始)  ) 
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_ヴァンプ詳細
		eval($f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:ヴァンプ詳細] = 1)


	}else if( (!$f:rfp_PC_ピーター聞いた) && \
				(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
					$w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpd_ライデン顔設定デフォルト
		@rpd_大佐顔設定１
		@rpd_顔表示

		@rpc_PC_ピーター
		eval($f:rfp_PC_ピーター聞いた = 1)

	}else if( (!$f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:ピーター詳細]) && \
				($f:rfp_PC_ピーター聞いた) && \
				(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
					$w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１開始) && \
				( ( ($i:プレイタイム - $i:rfp_PC_ピーター聞いた時刻) / d:FRAME_RATE ) >= 300 ) && \
				 ($w:rfp_callcount > 1) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_ピーター詳細
		eval($f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:ピーター詳細] = 1)

	}else if( (!$f:rfp_PC_ファットマンについて１聞いた) && \
				(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
					$w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_ファットマンについて１
		eval($f:rfp_PC_ファットマンについて１聞いた = 1)
		eval($i:rfp_PC_ファットマンについて１聞いた時刻 = $i:プレイタイム)

	}else if( (!$f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:ファットマン詳細]) && \
				($f:rfp_PC_ファットマンについて１聞いた) && \
				(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
					$w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始) && \
				( ( ($i:プレイタイム - $i:rfp_PC_ファットマンについて１聞いた時刻) / d:FRAME_RATE ) >= 300 ) && \
				 ($w:rfp_callcount > 1) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_ファットマン詳細
		eval($f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:ファットマン詳細] = 1)
		command varsave $f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:ファットマン詳細]

	}else if( (!$f:rfp_PC_エイムズ聞いた) && \
				(d:ST:P036_13_R04忍者登場１３無線デモ４終了 <= $w:p_story && \
					$w:p_story < d:ST:P040_01_P01エイムズ発見１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_エイムズ
		eval($f:rfp_PC_エイムズ聞いた = 1)
		eval($i:rfp_PC_エイムズ聞いた時刻 = $i:プレイタイム)

	}else if( (!$f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:エイムズ詳細]) && \
				($f:rfp_PC_エイムズ聞いた) && \
				(d:ST:P036_13_R04忍者登場１３無線デモ４終了 <= $w:p_story && \
					$w:p_story < d:ST:P040_01_P01エイムズ発見１ポリゴンデモ１開始) && \
				( ( ($i:プレイタイム - $i:rfp_PC_エイムズ聞いた時刻) / d:FRAME_RATE ) >= 300 ) && \
				 ($w:rfp_callcount > 1) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_エイムズ詳細
		eval($f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:エイムズ詳細] = 1)
		command varsave $f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:エイムズ詳細]


	}else if( (!$f:rfp_PC_エイムズ死後＿ブラックケース１聞いた) && \
				(d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P045_01_P01ハリアー登場１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_エイムズ死後＿ブラックケース１
		eval($f:rfp_PC_エイムズ死後＿ブラックケース１聞いた = 1)
		eval($i:rfp_PC_エイムズ死後＿ブラックケース１聞いた時刻 = $i:プレイタイム)

	}else if( (!$f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:ブラックケース詳細]) && \
				($f:rfp_PC_エイムズ死後＿ブラックケース１聞いた) && \
				(d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P049_01_P01大統領１ポリゴンデモ１開始) && \
				( ( ($i:プレイタイム - $i:rfp_PC_エイムズ死後＿ブラックケース１聞いた時刻) / d:FRAME_RATE ) >= 300 ) && \
				 ($w:rfp_callcount > 1) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_ブラックケース詳細
		eval($f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:ブラックケース詳細] = 1)
		command varsave $f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:ブラックケース詳細]


	}else if( (!$f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:ミスターＸ詳細]) && \
				(	(	($f:rfp_w21a_地雷原１無線デモ１した) &&\
						(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
							$w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１開始) ) || \
					(	($f:rfp_w12c_地雷原１無線デモ１した) &&\
						(d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了 <= $w:p_story && \
							$w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始) )  ) && \
				($f:rfp_PC_ミスターＸ聞いた) && \
				( ( ($i:プレイタイム - $i:rfp_PC_ミスターＸ聞いた開始時刻) / d:FRAME_RATE ) >= 300 ) && \
				 ($w:rfp_callcount > 1) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_ミスターＸ詳細
		eval($f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:ミスターＸ詳細] = 1)
		command varsave $f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:ミスターＸ詳細]


	
	}else if( (!$f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:怪奇箱男]) && \
				($f:rfp_PC_怪奇箱男１聞いた) && \
				(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
					$w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１開始) && \
				( ( ($i:プレイタイム - $i:rfp_PC_怪奇箱男１聞いた時刻) / d:FRAME_RATE ) >= 300 ) && \
				 ($w:rfp_callcount > 1) && \
				 (!$f:rfp_ローズさん落込んでます) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_怪奇箱男詳細
		eval($f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:怪奇箱男] = 1)
		command varsave $f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:怪奇箱男]


	}else if( (!$f:rfp_PC_銃入手注意聞いた) && \
				(!($w:プレイヤーフラグ & d:PL_SOCOM_SPPRSR_ATTACHED)) && \
				(d:ST:P010_10_P05ヴァンプ遭遇１０ポリゴンデモ５終了 <= $w:p_story && \
			$w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_銃入手注意
		eval($f:rfp_PC_銃入手注意聞いた = 1)


	}else if( (!$f:rfp_PC_核について聞いた) && \
				(d:ST:P036_13_R04忍者登場１３無線デモ４終了 <= $w:p_story && \
					$w:p_story < d:ST:P040_01_P01エイムズ発見１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_核について
		eval($f:rfp_PC_核について聞いた = 1)

/*
	}else if( (!$f:rfp_PC_エイムズ死後＿ＮＥＭＰ聞いた) && \
				(d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P049_01_P01大統領１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_エイムズ死後＿ＮＥＭＰ
		eval($f:rfp_PC_エイムズ死後＿ＮＥＭＰ聞いた = 1)
*/

	}else if( (!$f:rfp_PC_エイムズ死後＿人質聞いた) && \
				(d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P042_01_R01シェル１シェル２連絡橋１無線デモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_エイムズ死後＿人質
		eval($f:rfp_PC_エイムズ死後＿人質聞いた = 1)

	}else if( (!$f:rfp_PC_エイムズ死後＿テロボススネーク聞いた) && \
				(d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P042_01_R01シェル１シェル２連絡橋１無線デモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_エイムズ死後＿テロボススネーク
		eval($f:rfp_PC_エイムズ死後＿テロボススネーク聞いた = 1)

	}else if( (!$f:rfp_PC_ハリアー戦後＿スネーク聞いた) && \
				(d:ST:P046_02_R01ハリアー戦勝利２無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P049_01_P01大統領１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_ハリアー戦後＿スネーク
		eval($f:rfp_PC_ハリアー戦後＿スネーク聞いた = 1)

	}else if( (!$f:rfp_PC_エイムズ死後＿アウターヘブン聞いた) && \
				(d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P045_01_P01ハリアー登場１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_エイムズ死後＿アウターヘブン
		eval($f:rfp_PC_エイムズ死後＿アウターヘブン聞いた = 1)

	}else if( (!$f:rfp_PC_エイムズ死後＿忍者聞いた) && \
				(d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P045_01_P01ハリアー登場１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_エイムズ死後＿忍者
		eval($f:rfp_PC_エイムズ死後＿忍者聞いた = 1)

	}else if( (!$f:rfp_PC_大統領後＿スネーク聞いた) && \
				(d:ST:P050_06_R03タンカー編整理６無線機デモ３終了 <= $w:p_story && \
					$w:p_story < d:ST:P055_01_P01ヴァンプ戦前１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_大統領後＿スネーク
		eval($f:rfp_PC_大統領後＿スネーク聞いた = 1)
		
	}else if( (!$f:rfp_PC_大統領後＿なぜ大統領を殺したのか聞いた) && \
				(d:ST:P049_12_R05大統領１２無線機デモ５終了 <= $w:p_story && \
					$w:p_story < d:ST:P058_01_P01エマ救出１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_大統領後＿なぜ大統領を殺したのか
		eval($f:rfp_PC_大統領後＿なぜ大統領を殺したのか聞いた = 1)

	}else if( (!$f:rfp_PC_愛国者達聞いた) && \
				(d:ST:P049_12_R05大統領１２無線機デモ５終了 <= $w:p_story && \
					$w:p_story < d:ST:P062_01_P01エマとＡＩ１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_愛国者達
		eval($f:rfp_PC_愛国者達聞いた = 1)

	}else if( (!$f:rfp_PC_プリスキン信用するな聞いた) && \
				(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
					$w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１開始) && \
				 ($w:rfp_callcount > 1) ){
		@rpc_PC_プリスキン信用するな
		eval($f:rfp_PC_プリスキン信用するな聞いた = 1)

	}else if( (!$f:rfp_PC_爆弾解体プリスキン怪しい聞いた) && \
				(d:ST:P015_01_R1爆弾解体最初の一つ１無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１開始) && \
				($f:rfp_PS_爆弾解体中プリスキン聞いた) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_爆弾解体プリスキン怪しい
		eval($f:rfp_PC_爆弾解体プリスキン怪しい聞いた = 1)

	}else if( (!$f:rfp_PC_ＶＲ訓練について聞いた) && \
				(d:ST:P010_10_P05ヴァンプ遭遇１０ポリゴンデモ５終了 <= $w:p_story && \
					$w:p_story < d:ST:P015_01_R1爆弾解体最初の一つ１無線デモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_ＶＲ訓練について
		eval($f:rfp_PC_ＶＲ訓練について聞いた = 1)

	}else if( (!$f:rfp_PC_スネークについて聞いた) && \
		(d:ST:P005_05_R02ライデン昇降機上昇５無線デモ２終了 <= $w:p_story && \
			$w:p_story < d:ST:P040_01_P01エイムズ発見１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_スネークについて
		eval($f:rfp_PC_スネークについて聞いた = 1)

	}else if( (!$f:rfp_PC_タンカー事件について聞いた) && \
		($w:タンカー編クリア回数 > 0) && \
		(d:ST:P005_05_R02ライデン昇降機上昇５無線デモ２終了 <= $w:p_story && \
			$w:p_story < d:ST:P040_01_P01エイムズ発見１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_タンカー事件について
		eval($f:rfp_PC_タンカー事件について聞いた = 1)

	//条件ビミョーＸＸＸＸＸＸＸＸｘ
	}else if( (!$f:rfp_PC_Ｆ脚武器入手後聞いた) && \
				($s:エリア == "w22a") && \
				( ($w:武器弾数Ｒ[d:武器:Ｍ９] >= 0) || ($w:武器弾数Ｒ[d:武器:スタン] >= 0) ) && \
				($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) && \
				 ($w:rfp_callcount > 1)){
		@rpc_PC_Ｆ脚武器入手後
		eval($f:rfp_PC_Ｆ脚武器入手後聞いた = 1)

	}else if( (!$f:rfp_PC_一般機能説明聞いた) && \
				(!`@rp_緊張してください`) && \
				($w:p_story < d:ST:P027_01_R01爆弾解体後昇降機ホール１無線デモ１開始) && \
				 (!$f:rfp_ローズさん落込んでます) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_一般機能説明
		eval($f:rfp_PC_一般機能説明聞いた = 1)
	
	}else if( ( ($s:エリア == "w12a") || ($s:エリア == "w12b") || ($s:エリア == "w12c") ) && \
				(!$f:rfp_PC_Ａ脚機能説明聞いた) && \
				(!`@rp_緊張してください`) && \
				 ($w:rfp_callcount > 1) && \
				 (!$f:rfp_ローズさん落込んでます) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_Ａ脚機能説明
		eval($f:rfp_PC_Ａ脚機能説明聞いた = 1)

	}else if( ( ($s:エリア == "w14a")  ) && \
				(!$f:rfp_PC_Ｂ脚機能説明聞いた) && \
				(!`@rp_緊張してください`) && \
				 ($w:rfp_callcount > 1) && \
				 (!$f:rfp_ローズさん落込んでます) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_Ｂ脚機能説明
		eval($f:rfp_PC_Ｂ脚機能説明聞いた = 1)

	}else if( ( ($s:エリア == "w16a") || ($s:エリア == "w16b")  ) && \
				(!$f:rfp_PC_Ｃ脚機能説明聞いた) && \
				(!`@rp_緊張してください`) && \
				 ($w:rfp_callcount > 1)&& \
				 (!$f:rfp_ローズさん落込んでます) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_Ｃ脚機能説明
		eval($f:rfp_PC_Ｃ脚機能説明聞いた = 1)

	}else if( ( ($s:エリア == "w18a")  ) && \
				(!$f:rfp_PC_Ｄ脚機能説明聞いた) && \
				(!`@rp_緊張してください`) && \
				 ($w:rfp_callcount > 1)&& \
				 (!$f:rfp_ローズさん落込んでます) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_Ｄ脚機能説明
		eval($f:rfp_PC_Ｄ脚機能説明聞いた = 1)

	}else if( ( ($s:エリア == "w20a")  ) && \
				(!$f:rfp_PC_Ｅ脚機能説明＿集配場聞いた) && \
				 (!$f:rfp_ローズさん落込んでます) && \
				(!`@rp_緊張してください`) && \
				 ($w:rfp_callcount > 1) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_Ｅ脚機能説明＿集配場
		eval($f:rfp_PC_Ｅ脚機能説明＿集配場聞いた = 1)

	}else if( ( ($s:エリア == "w20b") || ($s:エリア == "w20c") || ($s:エリア == "w20d") ) && \
				(!$f:rfp_PC_Ｅ脚機能説明＿屋上聞いた) && \
				 (!$f:rfp_ローズさん落込んでます) && \
				(!`@rp_緊張してください`) && \
				 ($w:rfp_callcount > 1) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_Ｅ脚機能説明＿屋上
		eval($f:rfp_PC_Ｅ脚機能説明＿屋上聞いた = 1)

	}else if( ( ($s:エリア == "w22a")  ) && \
				(!$f:rfp_PC_Ｆ脚機能説明聞いた) && \
				 (!$f:rfp_ローズさん落込んでます) && \
				(!`@rp_緊張してください`) && \
				 ($w:rfp_callcount > 1) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_Ｆ脚機能説明
		eval($f:rfp_PC_Ｆ脚機能説明聞いた = 1)

	}else if( ( ($s:エリア == "w24a") || ($s:エリア == "w24b") || ($s:エリア == "w24c") || ($s:エリア == "w24d") ) && \
				(!$f:rfp_PC_シェル１中央棟機能説明聞いた) && \
				 (!$f:rfp_ローズさん落込んでます) && \
				(!`@rp_緊張してください`) && \
				 ($w:rfp_callcount > 1) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_シェル１中央棟機能説明
		eval($f:rfp_PC_シェル１中央棟機能説明聞いた = 1)

	}else if( ( ($s:エリア == "w31a") || ($s:エリア == "w31b") || ($s:エリア == "w31c") || \
				($s:エリア == "w31f") || ($s:エリア == "w31d") ) && \
				(!$f:rfp_PC_シェル２中央棟機能説明聞いた) && \
				 (!$f:rfp_ローズさん落込んでます) && \
				(!`@rp_緊張してください`) && \
				 ($w:rfp_callcount > 1) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_シェル２中央棟機能説明
		eval($f:rfp_PC_シェル２中央棟機能説明聞いた = 1)

	}else if( ( ($s:エリア == "w28a")  ) && \
				(!$f:rfp_PS_Ｌ脚機能説明聞いた) && \
				 (!$f:rfp_ローズさん落込んでます) && \
				(!`@rp_緊張してください`) && \
				 ($w:rfp_callcount > 1) && \
				(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PS_Ｌ脚機能説明
		eval($f:rfp_PS_Ｌ脚機能説明聞いた = 1)

//	}else if($w:アラートモード == 1){
//		@rpc_デフォルト操作説明危険

	//今は仮りん
	}else {

		@rpc_デフォルト操作説明普通
	}
}

proc rpc_連続無線デフォルト＿潜水 {
//	if($w:アラートモード == 1){
//		@rpc_デフォルト操作説明危険＿潜水

	//今は仮りん
//	}else {
		@rpc_デフォルト操作説明普通＿潜水
//	}
}


proc rpc_デフォルト操作説明 {
//	if($w:アラートモード == 1){
//		@rpc_デフォルト操作説明危険
//	}else {
		@rpc_デフォルト操作説明普通
//	}
}


proc rpc_デフォルト操作説明＿潜水 {
//	if($w:アラートモード == 1){
//		@rpc_デフォルト操作説明危険＿潜水
//	}else {
		@rpc_デフォルト操作説明普通＿潜水
//	}
}


proc rpc_レーション {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_レーションＣＡＬＬ１
	@rpcc_PC_レーション操作説明
	@rp_ローズさんご機嫌判定
	if( ($w:p_story >= d:ST:P004_02_R01ノード初接続２無線デモ１終了) && \
		(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpd_ローズ顔表示
		@rpcc_PC_レーション操作説明＿ローズ
	}
}

proc rpc_PC_Ａ脚Ｂ脚に行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ａ脚Ｂ脚に行け１

	if($s:エリア == "w12a") {
		@rp_PC_Ａ脚Ｂ脚に行け２

	}else if($s:エリア == "w12b") {
		@rp_PC_Ａ脚Ｂ脚に行け２＿２

	}else {
		@rp_PC_Ａ脚Ｂ脚に行け２
	}

	if(!$f:w12b_ノードフラグ) {
		@rp_PC_Ａ脚Ｂ脚に行け３
	}
}

proc rpc_PC_Ｂ脚に急げ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if( ($s:エリア == "w12a") || ($s:エリア == "w12b") ) {
		@rp_PC_Ｂ脚に急げ１
		@rp_PC_Ｂ脚に急げ３

	}else if($s:エリア == "w13a"){
		@rp_PC_Ｂ脚に急げ１
		@rp_PC_Ｂ脚に急げ４

	}else if($s:エリア == "w14a" ){
		//ファック！取り直し？ＸＸＸＸＸＸＸＸｘｘ
		@rp_PC_ヴァンプデモ前Ｂ脚

	}else {
		@rp_PC_Ｂ脚に急げ１
		@rp_PC_Ｂ脚に急げ５
	}
}


proc rpc_PC_ＢＣ連絡橋中央部へ行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＢＣ連絡橋中央部へ急げ
	if($s:エリア == "w15a" ){
		@rp_PC_ＢＣ連絡橋中央部へ行け＿ＢＣ連絡橋
	}else {
		@rp_PC_ＢＣ連絡橋中央部へ行け＿その他
	}
	if(!$f:w14a_ノードフラグ) {
		if($s:エリア == "w14a") {
			@rp_PC_Ｂ脚ノード１
			@rp_PC_Ｂ脚ノード２
		}
	}
}


proc rpc_PC_ピーター探せ {

	if( ($s:エリア == "w16a") || ($s:エリア == "w16b") ){
		if(!$f:rfp_PC_ピーター探せ＿Ｃ脚聞いた) {
			@rpd_ライデン顔設定デフォルト
			@rpd_大佐顔設定１
			@rpd_顔表示
			@rp_PC_ピーター探せ１
			@rp_PC_ピーター探せ＿Ｃ脚
			eval($f:rfp_PC_ピーター探せ＿Ｃ脚聞いた = 1)

		}else if( ($f:rfp_女子トイレにいます) && \
					(!$f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:Ｃ脚女子トイレピーター前]) ){
			@rpd_ライデン顔設定デフォルト
			@rpd_ローズ顔設定１
			@rpd_顔表示
			@rp_PC_Ｃ脚女子トイレピーター前
			@rp_ローズさんを怒らせました
			eval($f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:Ｃ脚女子トイレピーター前] = 1)

		}else if($f:rfp_女子トイレにいます){
			@rpd_ライデン顔設定デフォルト
			@rpd_大佐顔設定１
			@rpd_顔表示
			@rp_PC_Ｃ脚男子トイレピーター前
		}else {
			@rpd_ライデン顔設定デフォルト
			@rpd_大佐顔設定１
			@rpd_顔表示
			@rp_PC_ピーター探せ１
			@rp_PC_ピーター探せ＿Ｃ脚
		}
	}else {
		@rpd_ライデン顔設定デフォルト
		@rpd_大佐顔設定１
		@rpd_顔表示
		@rp_PC_ピーター探せ１
		@rp_PC_ピーター探せ＿Ｃ脚以外
	}
	eval($f:rfp_PC_ピーター探せ聞いた = 1)
}
proc rpc_爆弾解体中ノード位置前説 {
	if(!$f:rfp_PC_爆弾解体ノードにアクセスしろ１聞いた) {
		@rp_PC_爆弾解体ノードにアクセスしろ１
		eval($f:rfp_PC_爆弾解体ノードにアクセスしろ１聞いた = 1)
	}
	@rp_PC_爆弾解体ノードにアクセスしろ２
}


proc rp_PC_爆弾解体中ノード位置＿接続してない一回目のみ {
	//おぷしょんでレーダーオフの場合も！ＸＸＸＸＸＸＸＸ
	//Ａ脚 １Ｆ
	if( ($s:エリア == "w12b" ) && (!$f:w12b_ノードフラグ) && (!$f:rfp_PC_w12bＡ脚爆弾解体中ノード位置聞いた)){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＡ脚
		@rp_PC_爆弾解体ノードにアクセスしろ３
		eval($f:rfp_爆弾解体中ノード位置＿接続してない一回目のみ再生した = 1)
		eval($f:rfp_PC_w12bＡ脚爆弾解体中ノード位置聞いた = 1)

	//Ａ脚屋上
	}else if( ($s:エリア == "w12a" ) && (!$f:w12b_ノードフラグ) && (!$f:rfp_PC_w12aＡ脚爆弾解体中ノード位置聞いた) ){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＡ脚
		@rp_PC_爆弾解体ノードにアクセスしろ３
		eval($f:rfp_爆弾解体中ノード位置＿接続してない一回目のみ再生した = 1)
		eval($f:rfp_PC_w12aＡ脚爆弾解体中ノード位置聞いた = 1)

	//Ｂ脚
	}else if( ($s:エリア == "w14a" ) && (!$f:w14a_ノードフラグ) && (!$f:rfp_PC_w14aＢ脚爆弾解体中ノード位置聞いた) ){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＢ脚
		@rp_PC_爆弾解体ノードにアクセスしろ３
		eval($f:rfp_爆弾解体中ノード位置＿接続してない一回目のみ再生した = 1)
		eval($f:rfp_PC_w14aＢ脚爆弾解体中ノード位置聞いた = 1)
	
	//Ｃ脚
	}else if(  ( ($s:エリア == "w16a" ) || ($s:エリア == "w16b") ) && \
					(!$f:w16a_ノードフラグ) && (!$f:rfp_PC_w16aＣ脚爆弾解体中ノード位置聞いた) ){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＣ脚
		@rp_PC_爆弾解体ノードにアクセスしろ３
		eval($f:rfp_爆弾解体中ノード位置＿接続してない一回目のみ再生した = 1)
		eval($f:rfp_PC_w16aＣ脚爆弾解体中ノード位置聞いた = 1)

	//Ｄ脚
	}else if( ($s:エリア == "w18a" ) && (!$f:w18a_ノードフラグ) && (!$f:rfp_PC_w18aＤ脚爆弾解体中ノード位置聞いた)){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＤ脚
		@rp_PC_爆弾解体ノードにアクセスしろ３
		eval($f:rfp_爆弾解体中ノード位置＿接続してない一回目のみ再生した = 1)
		eval($f:rfp_PC_w18aＤ脚爆弾解体中ノード位置聞いた = 1)
		
	//Ｅ脚 １Ｆ
	}else if( ($s:エリア == "w20a" ) && (!$f:w20a_ノードフラグ) && (!$f:rfp_PC_w20aＥ脚爆弾解体中ノード位置聞いた) ){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＥ脚
		@rp_PC_爆弾解体ノードにアクセスしろ３
		eval($f:rfp_爆弾解体中ノード位置＿接続してない一回目のみ再生した = 1)
		eval($f:rfp_PC_w20aＥ脚爆弾解体中ノード位置聞いた = 1)

	//Ｅ脚 屋上
	}else if( ( ($s:エリア == "w20b" ) || ($s:エリア == "w20c" ) || ($s:エリア == "w20d" ) ) && \
				(!$f:w20a_ノードフラグ) && (!$f:rfp_PC_w20bＥ脚爆弾解体中ノード位置聞いた) ){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＥ脚
		@rp_PC_爆弾解体ノードにアクセスしろ３
		eval($f:rfp_爆弾解体中ノード位置＿接続してない一回目のみ再生した = 1)
		eval($f:rfp_PC_w20bＥ脚爆弾解体中ノード位置聞いた = 1)

	//Ｆ脚 
	}else if( ($s:エリア == "w22a" ) && (!$f:w22a_ノードフラグ)  && (!$f:rfp_PC_w22aＦ脚爆弾解体中ノード位置聞いた) ){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＦ脚
		@rp_PC_爆弾解体ノードにアクセスしろ３
		eval($f:rfp_爆弾解体中ノード位置＿接続してない一回目のみ再生した = 1)
		eval($f:rfp_PC_w22aＦ脚爆弾解体中ノード位置聞いた = 1)
	
	//その他連絡橋等
	}else{
		eval($f:rfp_爆弾解体中ノード位置＿接続してない一回目のみ再生した = 0)
	}
}

proc rp_PC_爆弾解体中ノード位置＿接続してない {
	//おぷしょんでレーダーオフの場合も！ＸＸＸＸＸＸＸＸ
	//Ａ脚 １Ｆ
	if( ($s:エリア == "w12b" ) && (!$f:w12b_ノードフラグ)  && \
		(!$f:w12b_爆弾処理完了) && (!$f:w12a_爆弾処理完了) ){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＡ脚
		eval($f:rfp_爆弾解体中ノード位置＿接続してない再生した = 1)

	//Ａ脚屋上
	}else if( ($s:エリア == "w12a" ) && (!$f:w12b_ノードフラグ)  && \
				(!$f:w12a_爆弾処理完了) && (!$f:w12b_爆弾処理完了) ){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＡ脚
		eval($f:rfp_爆弾解体中ノード位置＿接続してない再生した = 1)

	//Ｂ脚
	}else if( ($s:エリア == "w14a" ) && (!$f:w14a_ノードフラグ)  && (!$f:w14a_爆弾処理完了) ){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＢ脚
		eval($f:rfp_爆弾解体中ノード位置＿接続してない再生した = 1)
	
	//Ｃ脚
	}else if( ( ($s:エリア == "w16a" ) || ($s:エリア == "w16b") ) && (!$f:w16a_ノードフラグ)  && (!$f:w16b_爆弾処理完了) ){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＣ脚
		eval($f:rfp_爆弾解体中ノード位置＿接続してない再生した = 1)

	//Ｄ脚
	}else if( ($s:エリア == "w18a" ) && (!$f:w18a_ノードフラグ)  && \
				( (!$f:w18a_爆弾処理完了1) || (!$f:w18a_爆弾処理完了2) || (!$f:w18a_爆弾処理完了3) ) ){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＤ脚
		eval($f:rfp_爆弾解体中ノード位置＿接続してない再生した = 1)
		
	//Ｅ脚 １Ｆ
	}else if( ($s:エリア == "w20a" ) && (!$f:w20a_ノードフラグ)  && \
				(!$f:w20a_爆弾処理完了) && (!$f:w20b_爆弾処理完了) ){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＥ脚
		eval($f:rfp_爆弾解体中ノード位置＿接続してない再生した = 1)

	//Ｅ脚 屋上
	}else if( ( ($s:エリア == "w20b" ) || ($s:エリア == "w20c" ) || ($s:エリア == "w20d" ) ) \
				&& (!$f:w20a_ノードフラグ)  && (!$f:w20a_爆弾処理完了) && (!$f:w20b_爆弾処理完了) ){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＥ脚
		eval($f:rfp_爆弾解体中ノード位置＿接続してない再生した = 1)

	//Ｆ脚 
	}else if( ($s:エリア == "w22a" ) && (!$f:w22a_ノードフラグ) && (!$f:w22a_爆弾処理完了) ){
		@rpc_爆弾解体中ノード位置前説
		@rp_PC_爆弾解体ノードにアクセスしろＦ脚
		eval($f:rfp_爆弾解体中ノード位置＿接続してない再生した = 1)
	
	//その他連絡橋等
	}else{
		eval($f:rfp_爆弾解体中ノード位置＿接続してない再生した = 0)
	}
}


proc rpc_爆弾解体中その他デフォルト {
	rand 4
	eval($w:rfp_rand = $status)

	if($w:rfp_rand < 1){
		@rpc_PC_ピーター

	}else if($w:rfp_rand < 2){
		@rp_PC_爆弾解体ノードにアクセスしろ３

	}else if( ($w:rfp_rand < 3) && (!$f:rfp_短縮ダイアル＿ピーターセットした) ){
		@rp_PC_ピーターに聞け

	}else {
		@rp_PC_ピーター探せ１
	}
}

proc rpc_PC_ピーター {
	@rp_PC_ピーター１
	if(!$f:rfp_PC_ピーター聞いた){
		@rp_PC_ピーター２
		eval($f:rfp_PC_ピーター聞いた = 1)
		eval($i:rfp_PC_ピーター聞いた時刻 = $i:プレイタイム)
		eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
		
	}
}

proc rpc_爆弾解体中ピーターいなくなる前 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_爆弾解体中ノード位置＿接続してない一回目のみ
	//ノード位置言ってない場合以下
	if(!$f:rfp_爆弾解体中ノード位置＿接続してない一回目のみ再生した){

		rand 5
		eval($w:rfp_rand = $status)
		if(!$f:rfp_PC_ピーター聞いた){
			@rpc_PC_ピーター
			eval($f:rfp_PC_ピーター聞いた = 1)

		}else if(!$f:rfp_PC_爆弾解体ノードにアクセスしろ３聞いた) {
			@rp_PC_爆弾解体ノードにアクセスしろ３
			eval($f:rfp_PC_爆弾解体ノードにアクセスしろ３聞いた = 1)

			
		}else if( (!$f:rfp_短縮ダイアル＿ピーターセットした) && (!$f:rfp_PC_ピーターに聞け聞いた) ){
			@rp_PC_ピーターに聞け
			eval($f:rfp_PC_ピーターに聞け聞いた = 1)

		}else if($w:rfp_rand < 4){
				@rp_PC_爆弾解体中ノード位置＿接続してない
				//ノード位置言ってない場合以下
				if(!$f:rfp_爆弾解体中ノード位置＿接続してない再生した){
					@rpc_爆弾解体中その他デフォルト
				}
		}else {
			@rpc_爆弾解体中その他デフォルト
		}
	}
	eval($f:rfp_爆弾解体中ノード位置＿接続してない一回目のみ再生した = 0)
	eval($f:rfp_爆弾解体中ノード位置＿接続してない再生した = 0)
}

proc rpc_PC_最後の爆弾処理せよ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_最後の爆弾処理せよ聞いた){
		@rp_PC_最後の爆弾処理せよ
		@rp_PC_爆弾解体中ノード位置＿接続してない
		eval($f:rfp_PC_最後の爆弾処理せよ聞いた = 1)

	}else {
		rand 7
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 4){
			@rp_PC_爆弾解体中ノード位置＿接続してない
			//ノード位置言ってない場合以下
			if(!$f:rfp_爆弾解体中ノード位置＿接続してない再生した){
				@rpc_爆弾解体中その他デフォルト
			}
		}else if($w:rfp_rand < 6){
			@rp_PC_最後の爆弾処理せよ
			@rp_PC_爆弾解体中ノード位置＿接続してない

		}else {
			@rpc_爆弾解体中その他デフォルト
		}
	}
	eval($f:rfp_爆弾解体中ノード位置＿接続してない一回目のみ再生した = 0)
	eval($f:rfp_爆弾解体中ノード位置＿接続してない再生した = 0)
}

proc rpc_PC_センサーＢ取りに行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_センサーＢ取りに行け
}

proc rpc_PC_Ａ脚底部急げ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if( ($s:エリア == "w12a") || ($s:エリア == "w12b") || ($s:エリア == "w12c") ){
		@rp_PC_カウントダウン＿Ａ脚昇降機乗れ
	}else {
		@rp_PC_カウントダウン＿Ａ脚行け
	}
}

proc rpc_PC_ピーター死１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ピーター死１
}

proc rpc_PC_無臭爆弾＿Ａ脚底部 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示


	//センサーＢ装備している
	if( $w:アイテム == 11 ) {
		if($w:ゲーム設定 <= d:LEVEL_NORMAL){
			command VecLen	-10000,-43800,25200 \
						$i:プレイヤー位置Ｘ $i:プレイヤー位置Ｙ $i:プレイヤー位置Ｚ

		} else if ( $w:ゲーム設定 == d:LEVEL_HARD ){
			command VecLen	-9070,-43420,23980 \
						$i:プレイヤー位置Ｘ $i:プレイヤー位置Ｙ $i:プレイヤー位置Ｚ
		}else {
			command VecLen	-20250,-42750,24875 \
						$i:プレイヤー位置Ｘ $i:プレイヤー位置Ｙ $i:プレイヤー位置Ｚ
		}
		//遠い
		if($status >= 5000) {
			@rp_PC_無臭爆弾＿Ａ脚底部１

			if(!$f:rfp_PC_無臭爆弾＿昇降機ホール聞いた){
				@rp_PC_無臭爆弾＿昇降機ホール
				eval($f:rfp_PC_無臭爆弾＿昇降機ホール聞いた = 1)

			}else if(!$f:rfp_PC_無臭爆弾＿連結通路聞いた) {
				@rp_PC_無臭爆弾＿連結通路
				eval($f:rfp_PC_無臭爆弾＿連結通路聞いた = 1)

			}else {
				rand 2
				eval($w:rfp_rand = $status)

				if($w:rfp_rand < 1){
					@rp_PC_無臭爆弾＿昇降機ホール
				}else {
					@rp_PC_無臭爆弾＿連結通路
				}
			}
		//爆弾近いでござる
		}else {
			//五分以上経過でズバリヒント
			if( ( ( ($i:プレイタイム - $i:イベント開始時刻) / d:FRAME_RATE ) >= 300 ) && \
					($w:ゲーム設定 <= d:LEVEL_HARD) ){
				@rp_PC_無臭Ｃ４ズバリ
			}else {
				@rp_PC_ピーター死＿ドック
			}
		}

	//センサーＢ装備していない
	}else {
		@rp_PC_無臭爆弾＿Ａ脚底部１
	}
}

proc rpc_PC_爆弾解体後＿昇降機上がれ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_爆弾解体後＿昇降機上がれ
}

proc rpc_フォーチュン戦中 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if( (!$f:rfp_PC_フォーチュン戦＿爆弾１聞いた) && \
		(d:ST:P030_01_R01フォーチュン戦中無線無線デモ１開始 <= $w:p_story) ){
		@rp_PC_フォーチュン戦＿爆弾１
		eval($f:rfp_PC_フォーチュン戦＿爆弾１聞いた = 1)
		
	}else if( (!$f:rfp_PC_フォーチュン戦＿倒せない聞いた) && \
				($f:rfp_w11c_フォーチュン攻撃それそれデモ見た) && \
				($w:p_story < d:ST:P030_01_R01フォーチュン戦中無線無線デモ１開始) ){
		@rp_PC_フォーチュン戦＿倒せない１
		@rp_PC_フォーチュン戦＿倒せない２
		eval($f:rfp_PC_フォーチュン戦＿倒せない聞いた = 1)
	
	}else if(!$f:rfp_PC_フォーチュン戦＿隠れろ聞いた) {
		@rp_PC_フォーチュン戦＿隠れろ
		eval($f:rfp_PC_フォーチュン戦＿隠れろ聞いた = 1)
	
	}else if( (!$f:rfp_PC_フォーチュン戦＿接近不能聞いた) && \
					($f:rfp_w11c_フォーチュン攻撃それそれデモ見た) ){
		@rp_PC_フォーチュン戦＿接近不能
		eval($f:rfp_PC_フォーチュン戦＿接近不能聞いた = 1)

	}else if(!$f:rfp_PC_フォーチュン戦＿レールガン聞いた) {
		@rp_PC_フォーチュン戦＿レールガン
		eval($f:rfp_PC_フォーチュン戦＿レールガン聞いた = 1)
		eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
		
	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if( ($w:rfp_rand < 1) && \
			($f:rfp_w11c_フォーチュン攻撃それそれデモ見た) ){
			@rp_PC_フォーチュン戦＿倒せない２

		}else if( ($w:rfp_rand < 2) && \
			($f:rfp_w11c_フォーチュン攻撃それそれデモ見た) ){
			@rp_PC_フォーチュン戦＿接近不能

		}else if(($w:rfp_rand < 3) && \
				 (d:ST:P030_01_R01フォーチュン戦中無線無線デモ１開始 <= $w:p_story) ){
				@rp_PC_フォーチュン戦＿爆弾２

		}else {
			@rp_PC_フォーチュン戦＿隠れろ
		}
	}
}

proc rpc_PC_ファットマン戦前 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(	(!$f:rfp_PC_ファットマン戦前＿ヘリポートに行け聞いた) && \
		( ($s:エリア != "w20b") && ($s:エリア != "w20c") && ($s:エリア != "w20d") ) ) {
		@rp_PC_ファットマン戦前＿ヘリポートに行け
		eval($f:rfp_PC_ファットマン戦前＿ヘリポートに行け聞いた = 1)

	}else if( (!$f:rfp_PC_ファットマン戦前＿ヘリポート聞いた) && \
		( ($s:エリア == "w20b") || ($s:エリア == "w20c") || ($s:エリア == "w20d") ) ) {
		@rp_PC_ファットマン戦前＿ヘリポート
		eval($f:rfp_PC_ファットマン戦前＿ヘリポート聞いた = 1)

	}else if(!$f:rfp_PC_ファットマン戦前＿武器そろえろ) {
		@rp_PC_ファットマン戦前＿武器そろえろ
		eval($f:rfp_PC_ファットマン戦前＿武器そろえろ = 1)

	}else {
		rand 2
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			if( ($s:エリア == "w20b") || ($s:エリア == "w20c") || ($s:エリア == "w20d") ){
				@rp_PC_ファットマン戦前＿ヘリポート
			}else {
				@rp_PC_ファットマン戦前＿ヘリポートに行け
			}
		}else {
			@rp_PC_ファットマン戦前＿武器そろえろ
		}
	}
}

proc rpc_PC_ファットマン戦＿死後爆弾残り {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ファットマン戦＿死後爆弾残り
}

//カウントダウン中、エルード中、順番ＸＸＸＸＸＸＸＸＸＸＸｘ
proc rpc_ファットマン戦中 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	if( ($f:rfp_ファットマン戦中救済ＣＡＬＬ聞いた) && \
		(!$f:rfp_ファットマン戦中主観攻撃説明聞いた) ){
			eval($f:rfp_ファットマン戦中主観攻撃説明聞いた = 1)
			@rp_PC_主観攻撃１
			@rp_PC_主観攻撃２

	}else if(!$f:rfp_PC_ファットマン戦＿爆弾処理せよ聞いた) {
		@rp_PC_ファットマン戦＿爆弾処理せよ１
		@rp_PC_ファットマン戦＿爆弾処理せよ２
		eval($f:rfp_PC_ファットマン戦＿爆弾処理せよ聞いた = 1) 

	}else if(!$f:rfp_PC_ファットマン戦＿頭狙え聞いた) {
		@rp_PC_ファットマン戦＿頭狙え１
		@rpd_ローズ顔表示
		@rp_PC_ファットマン戦＿頭狙え２
		@rpd_大佐顔表示
		@rp_PC_ファットマン戦＿頭狙え３
		eval($f:rfp_PC_ファットマン戦＿頭狙え聞いた = 1)

	}else if(!$f:rfp_PC_ファットマン戦＿転ぶ聞いた) {
		@rp_PC_ファットマン戦＿転ぶ
		eval($f:rfp_PC_ファットマン戦＿転ぶ聞いた = 1)
	
	}else if(!$f:rfp_PC_ファットマン戦＿Ｃ４カウントダウン聞いた) {
		@rp_PC_ファットマン戦＿Ｃ４カウントダウン
		eval($f:rfp_PC_ファットマン戦＿Ｃ４カウントダウン聞いた = 1)
	
	}else if(!$f:rfp_PC_ファットマン戦＿クレイモア聞いた) {
		@rp_PC_ファットマン戦＿クレイモア
		eval($f:rfp_PC_ファットマン戦＿クレイモア聞いた = 1)
	
	}else if(!$f:rfp_PC_ファットマン戦＿エルード中聞いた) {
		@rp_PC_ファットマン戦＿エルード中
		eval($f:rfp_PC_ファットマン戦＿エルード中聞いた = 1)
	
	}else if(!$f:rfp_PC_ファットマン戦＿銃撃戦聞いた) {
		@rp_PC_ファットマン戦＿銃撃戦
		eval($f:rfp_PC_ファットマン戦＿銃撃戦聞いた = 1)
	
	}else if(!$f:rfp_PC_ファットマン戦＿ボムブラストスーツ聞いた) {
		@rp_PC_ファットマン戦＿ボムブラストスーツ１
		@rp_PC_ファットマン戦＿ボムブラストスーツ２
		eval($f:rfp_PC_ファットマン戦＿ボムブラストスーツ聞いた = 1)
	
	//これは一回目のみ
	}else if(!$f:rfp_PC_ファットマン戦＿グロック聞いた) {
		@rp_PC_ファットマン戦＿グロック
		eval($f:rfp_PC_ファットマン戦＿グロック聞いた = 1)

	}else {
		rand 9
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PC_ファットマン戦＿爆弾処理せよ２

		}else if($w:rfp_rand < 2){
			@rp_PC_ファットマン戦＿Ｃ４カウントダウン
		
		}else if($w:rfp_rand < 3){
			@rp_PC_ファットマン戦＿頭狙え１
			@rp_PC_ファットマン戦＿頭狙え３

		}else if($w:rfp_rand < 4){
			@rp_PC_ファットマン戦＿転ぶ
		
		}else if($w:rfp_rand < 5){
			@rp_PC_ファットマン戦＿転び

		}else if($w:rfp_rand < 6){
			@rp_PC_ファットマン戦＿クレイモア
		
		}else if($w:rfp_rand < 7){
			@rp_PC_ファットマン戦＿エルード中
		
		}else if($w:rfp_rand < 8){
			@rp_PC_ファットマン戦＿銃撃戦
		
		}else {
			@rp_PC_ファットマン戦＿ボムブラストスーツ２
		}
	}
}

//ズバリヒントの出し方ＸＸＸＸＸＸＸＸＸｘ
proc rpc_ファットマン死後爆弾解体中 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_ファットマン戦後＿最終爆弾聞いた) {
		@rp_PC_ファットマン戦後＿最終爆弾１
		@rpd_大佐顔表示
		@rp_PC_ファットマン戦後＿最終爆弾２
		eval($f:rfp_PC_ファットマン戦後＿最終爆弾聞いた = 1)
	
	}else if( (!$f:rfp_PC_ファットマン戦後＿最終爆弾ヒント１聞いた) && \
				( (`%ゲット残りタイム` <= 3600) || \
					( ( ($i:プレイタイム - $i:rfp_ファットマンさいごっぺ爆弾開始時刻) / d:FRAME_RATE) >= 200) ) \
			){
		@rp_PC_ファットマン戦後＿最終爆弾ヒント１
		@rpd_大佐顔表示
		@rp_PC_ファットマン戦後＿最終爆弾ヒント２
		eval($f:rfp_PC_ファットマン戦後＿最終爆弾ヒント１聞いた  = 1 )
	
	}else if( (!$f:rfp_PC_ファットマン戦後＿最終爆弾ズバリ聞いた) && \
				( (`%ゲット残りタイム` <= 900) || \
				( ( ($i:プレイタイム - $i:rfp_ファットマンさいごっぺ爆弾開始時刻) / d:FRAME_RATE) >= 400) ) \
			){
		@rp_PC_ファットマン戦後＿最終爆弾ズバリ
		eval($f:rfp_PC_ファットマン戦後＿最終爆弾ズバリ聞いた = 1 )
	
	}else {
		rand 2
		eval($w:rfp_rand = $status)

		if( ($w:rfp_rand < 1) && \
			( (`%ゲット残りタイム` <= 3600) || \
			( ( ($i:プレイタイム - $i:rfp_ファットマンさいごっぺ爆弾開始時刻) / d:FRAME_RATE) >= 200) ) \
			){
			@rp_PC_ファットマン戦後＿最終爆弾ヒント２
		}else {
			@rp_PC_ファットマン戦後＿最終爆弾２
		}
	}
}

proc rpc_PC_忍者登場前＿ヘリポート {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_忍者登場前＿ヘリポート
}

proc rpc_PC_忍者登場前＿ローズ助けられ {
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔設定１
	@rpd_顔表示

	@rp_PC_忍者登場前＿ローズ助けられ
}


proc rpc_PC_変装＿攻撃するな {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_変装＿攻撃するな
}

proc rpc_PC_変装＿怪しい行動 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_変装＿怪しい行動
}

proc rpc_PC_変装＿接触脱ぎ{
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_変装＿接触脱ぎ１
	@rp_ローズさんご機嫌判定
	if( (!$f:rfp_PC_変装＿接触脱ぎ２聞いた) && ($w:アイテム == d:アイテム:ゴル兵制服) && \
		(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpd_ローズ顔表示
		@rp_PC_変装＿接触脱ぎ２
		eval($f:rfp_PC_変装＿接触脱ぎ２聞いた  = 1 )
	}
}

proc rpc_PC_人質＿中央棟行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_人質＿中央棟行け
}

proc rpc_PC_人質＿中央棟行けと言われてもＡＫありません {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_人質＿中央棟行けと言われてもＡＫありません聞いた){
		@rp_PC_人質＿中央棟行け
		@rp_PC_変装＿変装方法１
		@rp_PC_変装＿変装方法２
		@rp_PC_変装＿ＡＫ装備
		@rp_PC_変装＿ＡＫ探せ
		eval($f:rfp_PC_人質＿中央棟行けと言われてもＡＫありません聞いた = 1)
	}else {
		@rp_PC_変装＿ＡＫ探せ
	}
}


proc rpc_PC_シェル１中央棟＿変装してない {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_シェル１中央棟＿変装してない
}

proc rpc_変装方法 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_変装＿変装方法聞いた){
		@rp_PC_変装＿変装方法１
		@rp_PC_変装＿変装方法２
		@rp_PC_変装＿ＡＫ装備
		if($w:武器弾数Ｒ[15] < 0){
			@rp_PC_変装＿ＡＫ探せ
		}
		eval($f:rfp_PC_変装＿変装方法聞いた = 1)

	}else if($w:武器弾数Ｒ[15] < 0){
		@rp_PC_変装＿ＡＫ装備
		@rp_PC_変装＿ＡＫ探せ

	}else {
		@rp_PC_変装＿変装方法１
		@rp_PC_変装＿変装方法２
		@rp_PC_変装＿ＡＫ装備
	}
}


proc rpc_変装注意 {
	if(!$f:rfp_PC_変装＿変装方法聞いた) {
		@rpc_変装方法
		eval($f:rfp_PC_変装＿変装方法聞いた = 1)
	
	}else if(!$f:rfp_PC_変装＿攻撃するな聞いた) {
		@rpc_PC_変装＿攻撃するな
		eval($f:rfp_PC_変装＿攻撃するな聞いた  =1 )
		
	}else if(!$f:rfp_PC_変装＿怪しい行動聞いた){
		@rpc_PC_変装＿怪しい行動
		eval($f:rfp_PC_変装＿怪しい行動聞いた = 1)
	
	}else if(!$f:rfp_PC_変装＿接触脱ぎ) {
		@rpc_PC_変装＿接触脱ぎ
		eval($f:rfp_PC_変装＿接触脱ぎ = 1)
		eval($f:rfp_PC_変装方法終了 = 1)
		
	//PC_変装＿激し脱げは？？ＸＸＸＸＸＸＸＸＸｘ

	//ＡＫ探せ
	}else if($w:武器弾数Ｒ[15] < 0){
		@rpc_変装方法

	}else {
		eval($f:rfp_PC_変装方法終了 = 1)
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rpc_変装方法

		}else if($w:rfp_rand < 2){
			@rpc_PC_変装＿攻撃するな

		}else if($w:rfp_rand < 3){
			@rpc_PC_変装＿怪しい行動

		}else {
			@rpc_PC_変装＿接触脱ぎ
		}
	}
}

//マイクの場所の条件ＸＸＸＸＸＸＸＸＸｘ
proc rpc_指向性マイク人質部屋以外 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_人質＿指向性マイク聞いた) {
		@rp_PC_人質＿指向性マイク１
		@rp_PC_人質＿指向性マイク２
		if($w:武器弾数Ｒ[12] < 0) {
			@rp_PC_人質＿指向性マイク３
		}
		eval($f:rfp_PC_人質＿指向性マイク聞いた = 1)

	}else if($w:武器弾数Ｒ[12] < 0) {
		@rp_PC_人質＿指向性マイク２
		if(!$f:rfp_PR_ローズさんが激怒してます！){
			@rpd_ローズ顔表示
			@rp_PC_人質＿マイク場所２
		}

	}else {
		@rp_PC_人質＿指向性マイク２
	}
}

proc rpc_網膜センサー {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_網膜センサー＿羽交絞め聞いた) {
		@rp_PC_網膜センサー＿羽交絞め１
		@rp_PC_網膜センサー＿羽交絞めヒント１
		@rp_PC_網膜センサー＿羽交絞めヒント２
//		@rp_PC_網膜センサー＿羽交絞めヒント３
		eval($f:rfp_PC_網膜センサー＿羽交絞め聞いた = 1 )


	//フラグセット、ＣＡＬＬ？ ＸＸＸＸＸＸＸＸｘ

	}else if(!$f:rfp_PC_網膜センサー＿生きた敵兵聞いた){
		@rp_PC_網膜センサー＿生きた敵兵
		eval($f:rfp_PC_網膜センサー＿生きた敵兵聞いた = 1 )

	}else if(!$f:rfp_PC_網膜センサー＿潜入モードのみ聞いた ){
		@rp_PC_網膜センサー＿潜入モードのみ
		eval($f:rfp_PC_網膜センサー＿潜入モードのみ聞いた = 1)
		eval($f:rfp_PC_網膜センサー終了 = 1)
	
	}else {
		eval($f:rfp_PC_網膜センサー終了 = 1)
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PC_網膜センサー＿羽交絞め１
			@rp_PC_網膜センサー＿羽交絞めヒント２

		}else if($w:rfp_rand < 2){
			@rp_PC_網膜センサー＿羽交絞めヒント３

		}else if($w:rfp_rand < 3){
			@rp_PC_網膜センサー＿生きた敵兵

		}else {
			@rp_PC_網膜センサー＿潜入モードのみ
		}
	}
}



proc rpc_変装中注意 {
	if(!$f:rfp_PC_変装＿攻撃するな聞いた) {
		@rpc_PC_変装＿攻撃するな
		eval($f:rfp_PC_変装＿攻撃するな聞いた  =1 )
		
	}else if(!$f:rfp_PC_変装＿怪しい行動聞いた){
		@rpc_PC_変装＿怪しい行動
		eval($f:rfp_PC_変装＿怪しい行動聞いた = 1)
	
	}else if(!$f:rfp_PC_変装＿接触脱ぎ聞いた) {
		@rpc_PC_変装＿接触脱ぎ
		eval($f:rfp_PC_変装＿接触脱ぎ聞いた = 1)

		eval($f:rfp_PC_変装中注意終了 = 1)
		
	//PC_変装＿激し脱げは？？ＸＸＸＸＸＸＸＸＸｘ

	}else {
		eval($f:rfp_PC_変装中注意終了 = 1)
		rand 3
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rpc_PC_変装＿接触脱ぎ

		}else if($w:rfp_rand < 2){
			@rpc_PC_変装＿攻撃するな

		}else {
			@rpc_PC_変装＿怪しい行動
		}
	}
}



proc rpc_人質イベント中央棟以外 {
	if( ($w:武器弾数Ｒ[d:武器:ＡＫＳ] < 0) && \
		(($w:rfp_callcount <= 1) || (`prefreq` != 14085) ) ){
		@rpc_PC_人質＿中央棟行けと言われてもＡＫありません
		eval($f:rfp_PC_人質＿中央棟行け聞いた = 1)

	}else if(!$f:rfp_PC_人質＿中央棟行け聞いた){
		@rpc_PC_人質＿中央棟行け
		eval($f:rfp_PC_人質＿中央棟行け聞いた = 1)

	}else if(!$f:rfp_PC_忍者接触後＿忍者聞いた){
		@rpc_PC_忍者接触後＿忍者
		eval($f:rfp_PC_忍者接触後＿忍者聞いた = 1)

	}else if( (!$f:rfp_PC_変装方法終了) && (`rand 4` < 3) ){
		@rpc_変装注意

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 8
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 3){
			@rpc_PC_人質＿中央棟行け

		}else if($w:rfp_rand < 6){
			@rpc_変装注意

		}else {
			@rpc_デフォルト操作説明
		}

	//連続無線
	}else {
		@rpc_連続無線デフォルト
		//ＸＸＸＸＸＸＸＸＸＸ
		//PC_銃入手注意
	}
}

proc rpc_PC_エイムズ探せ＿シェル１中央棟＿Ｂ１以外 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エイムズ探せ＿シェル１中央棟＿１
	@rp_PC_エイムズ探せ＿シェル１中央棟＿Ｂ１以外
}


proc rpc_人質イベント中央棟１ＦＢ２ {
		if($w:武器弾数Ｒ[d:武器:ＡＫＳ] < 0){
			@rpc_変装方法

		}else if(!$f:rfp_PC_エイムズ探せ＿シェル１中央棟＿Ｂ１以外聞いた){
			@rpc_PC_エイムズ探せ＿シェル１中央棟＿Ｂ１以外
			eval($f:rfp_PC_エイムズ探せ＿シェル１中央棟＿Ｂ１以外聞いた = 1) 

		//変装中！
		}else if( ($w:武器 == 15) && ($w:アイテム == 6) ){
			if( (!$f:rfp_PC_変装中注意終了) && (`rand 4` < 3) ){
				@rpc_変装中注意

			//マイク場所ズパリの条件？？ＸＸＸＸＸＸＸｘ
			}else if( ($w:武器弾数Ｒ[12] < 0) && (!$f:rfp_PC_人質＿指向性マイク聞いた) ){
				@rpc_指向性マイク人質部屋以外
			
			//無線一回目
			}else if($w:rfp_callcount <= 1){
				rand 9
				eval($w:rfp_rand = $status)

				if($w:rfp_rand < 3){
					//ヤバ！ＸＸＸＸＸＸＸＸＸ
					@rpc_PC_エイムズ探せ＿シェル１中央棟＿Ｂ１以外

				}else if($w:rfp_rand < 5){
					if($w:武器弾数Ｒ[12] < 0) {
						@rpc_指向性マイク人質部屋以外
					}else {
						@rpc_変装中注意
					}
				}else if($w:rfp_rand < 7){
					@rpc_変装中注意

				}else {
					@rpc_デフォルト操作説明
				}

			//連続無線
			}else {
				@rpc_連続無線デフォルト
				//ＸＸＸＸＸＸＸＸＸＸ
				//PC_銃入手注意
			}
		
		//変装してません
		}else {
			if( (!$f:rfp_PC_シェル１中央棟＿変装してない聞いた) && \
				($s:エリア == "w24a") && ($f:rfp_w24a_監視カメラ地帯にいましゃー) ){
				@rpc_PC_シェル１中央棟＿変装してない
				eval($f:rfp_PC_シェル１中央棟＿変装してない聞いた = 1)

			}else if( (!$f:rfp_PC_変装方法終了) && (`rand 4` < 3) ){
				@rpc_変装注意

			//マイク場所ズパリの条件？？ＸＸＸＸＸＸＸｘ
			}else if( ($w:武器弾数Ｒ[12] < 0) && (!$f:rfp_PC_人質＿指向性マイク聞いた) ){
				@rpc_指向性マイク人質部屋以外

			//無線一回目
			}else {
				rand 10
				eval($w:rfp_rand = $status)

				if( ($w:rfp_rand < 2) && ($w:rfp_callcount <= 1) ){
					@rpc_PC_エイムズ探せ＿シェル１中央棟＿Ｂ１以外

				}else if( ($w:rfp_rand < 4) && ($w:rfp_callcount <= 1) && ($w:武器弾数Ｒ[12] <= 0) ){
						@rpc_指向性マイク人質部屋以外

				}else if( ($w:rfp_rand < 6) && ($w:rfp_callcount <= 1) ){
					@rpc_変装注意

				}else if( ($w:rfp_rand < 8) && ($w:rfp_callcount <= 1) && \
							($s:エリア == "w24a") && ($f:rfp_w24a_監視カメラ地帯にいましゃー) ){
						@rpc_PC_シェル１中央棟＿変装してない

				}else {
					@rpc_連続無線デフォルト
				}
			}
		}
}

proc rpc_PC_エイムズ探せ＿シェル１中央棟＿Ｂ１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エイムズ探せ＿シェル１中央棟＿Ｂ１
}


proc rpc_人質イベント中央棟Ｂ１ {
		//激ヤバ！集会場はその階だ！ＸＸＸＸＸＸＸＸＸｘｘ
		if(!$f:rfp_PC_エイムズ探せ＿シェル１中央棟＿Ｂ１聞いた){
			@rpc_PC_エイムズ探せ＿シェル１中央棟＿Ｂ１
			eval($f:rfp_PC_エイムズ探せ＿シェル１中央棟＿Ｂ１聞いた = 1) 

		//一度も網膜成功してない
		}else if( (!$f:rfp_w24b_網膜センサー通過した) && (!$f:rfp_PC_網膜センサー＿羽交絞め聞いた) ){
			@rpc_網膜センサー

		//マイク場所ズパリの条件？？ＸＸＸＸＸＸＸｘ
		}else if( ($w:武器弾数Ｒ[12] < 0) && (!$f:rfp_PC_人質＿指向性マイク聞いた) ){
			@rpc_指向性マイク人質部屋以外

		}else if((!$f:rfp_w24b_網膜センサー通過した) && (!$f:rfp_PC_網膜センサー終了) ){
			@rpc_網膜センサー

		//変装してない
		}else if( ( ($w:武器 != 15) || ($w:アイテム != 6) ) && (!$f:rfp_PC_変装＿変装方法聞いた) ){
			@rpc_変装方法

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 10
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 2){
				//ヤバ！Ｂ１専用！！ＸＸＸＸＸＸＸＸＸ
				@rpc_PC_エイムズ探せ＿シェル１中央棟＿Ｂ１

			}else if($w:rfp_rand < 4){
				if($w:武器弾数Ｒ[12] < 0) {
					@rpc_指向性マイク人質部屋以外
				}else {
					@rpc_変装中注意
				}

			}else if($w:rfp_rand < 8){
				if(!$f:rfp_w24b_網膜センサー通過した) {
					@rpc_網膜センサー
				}else {
					@rpc_PC_エイムズ探せ＿シェル１中央棟＿Ｂ１
				}

			}else if($w:rfp_rand < 9){
					@rpc_変装中注意

			}else {
				@rpc_デフォルト操作説明
			}

		//連続無線
		}else {
			if(!$f:rfp_w24b_網膜センサー通過した){
				rand 10
				eval($w:rfp_rand = $status)

				if($w:rfp_rand < 7){
					@rpc_網膜センサー
				}else if($w:rfp_rand < 9){
					if($w:武器弾数Ｒ[12] < 0) {
						@rpc_指向性マイク人質部屋以外
					}else {
						@rpc_連続無線デフォルト
					}
				}else {
					@rpc_連続無線デフォルト
				}
			}else {
				rand 10
				eval($w:rfp_rand = $status)

				if($w:rfp_rand < 7){
					if($w:武器弾数Ｒ[12] < 0) {
						@rpc_指向性マイク人質部屋以外
					}else {
						@rpc_連続無線デフォルト
					}
				}else {
					@rpc_連続無線デフォルト
				}
			}
		}

}

proc rpc_人質＿あんたがエイムズ説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_人質＿マイク使え
	@rp_PC_人質＿あんたがエイムズ１
	if( (!$f:rfp_人質＿あんたがエイムズ説明聞いた) && (!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rp_PC_人質＿あんたがエイムズ２
		eval($f:rfp_人質＿あんたがエイムズ説明聞いた = 1)
		eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
		@rpd_大佐顔表示
	}
	@rp_PC_人質＿あんたがエイムズ３
}

proc rpc_PC_人質＿見つかるな{
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_人質＿見つかるな聞いた) {
		@rp_PC_人質＿見つかるな１
	}
	@rp_PC_人質＿見つかるな２
}

proc rpc_PC_人質＿マイク使い方 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_人質＿マイク使い方
}

proc rpc_PC_人質＿マイク注意 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_人質＿マイク注意
}

proc rpc_PC_人質＿敵兵定時連絡 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_人質＿敵兵定時連絡
}

proc rpc_PC_人質＿人質殺すな {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if( (!$f:rfp_PC_人質＿人質殺すな聞いた)  && (!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rp_PC_人質＿人質殺すな１
		@rpd_大佐顔表示
	}
	@rp_PC_人質＿人質殺すな２
}

proc rpc_PC_人質部屋＿敵兵定時連絡２ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	@rp_PC_人質部屋＿敵兵定時連絡２
}

proc rpc_PC_シェル１中央棟Ｂ１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	@rp_PC_シェル１中央棟Ｂ１
}

proc rpc_人質＿マイクない {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_人質＿マイクない聞いた) {
		@rp_PC_人質＿マイク使え
		@rp_PC_人質＿マイク探せ
		@rp_PC_人質＿マイク場所１
		//ズバリヒントの条件
		@rpd_ローズ顔表示
		if(!$f:rfp_PR_ローズさんが激怒してます！){
			@rp_PC_人質＿マイク場所２
		}
		eval($f:rfp_PC_人質＿マイクない聞いた = 1)
	}else {
		rand 2
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PC_人質＿マイク使え
			@rp_PC_人質＿マイク探せ
			@rp_PC_人質＿マイク場所１
			//ズバリヒントの条件
			@rpd_ローズ顔表示
			@rp_PC_人質＿マイク場所２

		}else {
			@rp_PC_人質＿指向性マイク２
			@rp_PC_人質＿指向性マイク３
			@rpd_ローズ顔表示
			@rp_PC_人質＿マイク場所２
		}
	}
}

proc rpc_人質部屋マイクあるランダム {
	rand 7
	eval($w:rfp_rand = $status)
	if($w:rfp_rand < 1){
		@rpc_人質＿あんたがエイムズ説明
	}else if($w:rfp_rand < 2){
		@rpc_PC_人質＿見つかるな
	}else if( ($w:rfp_rand < 3) && (!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_人質＿マイク使い方
	}else if( ($w:rfp_rand < 4)  && (!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_人質＿マイク注意
	}else if($w:rfp_rand < 5){
		@rpc_PC_人質＿敵兵定時連絡
	}else if($w:rfp_rand < 6){
		@rpc_PC_人質部屋＿敵兵定時連絡２
	}else {
		@rpc_PC_人質＿人質殺すな
	}
}

proc rpc_PC_人質＿人質殴った {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	@rp_PC_人質＿人質殴った
	eval($f:rfp_PC_人質＿人質殴った聞いた = 1)
	@rp_ローズさんを怒らせました
}

proc rpc_PC_人質＿人質ぱんつ写真 {
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔設定１
	@rpd_顔表示

	@rp_PC_人質＿人質ぱんつ写真１
	@rpd_大佐顔表示
	@rp_PC_人質＿人質ぱんつ写真２
	@rp_ローズさんを怒らせました
}

proc rpc_人質イベント人質部屋 {
	//殴りすぎ
	if( ($w:rfp_w24c_人質殴った回数 >= 15) && \
			(!$f:rfp_PC_人質＿人質殴った聞いた) ){
		@rpc_PC_人質＿人質殴った
		eval($f:rfp_PC_人質＿人質殴った聞いた = 1)

	}else if( ($f:w24c_ギャルパンツ撮影した) && \
				(!$f:rfp_PC_人質＿人質ぱんつ写真聞いた) ){
		@rpc_PC_人質＿人質ぱんつ写真
		eval($f:rfp_PC_人質＿人質ぱんつ写真聞いた = 1)

	//マイクなしお
	}else if($w:武器弾数Ｒ[12] <= 0){
		if(!$f:rfp_PC_人質＿マイクない聞いた) {
			@rpc_人質＿マイクない
			eval($f:rfp_PC_人質＿マイクない聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 4){
				@rpc_人質＿マイクない
			}else {
				@rpc_デフォルト操作説明
			}
		//連続デフォルト
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 4){
				@rpc_人質＿マイクない
			}else {
				@rpc_連続無線デフォルト
			}
		}

	//マイクあるる
	}else if(!$f:rfp_人質＿あんたがエイムズ説明聞いた) {
		@rpc_人質＿あんたがエイムズ説明
		eval($f:rfp_人質＿あんたがエイムズ説明聞いた = 1)

	}else if(!$f:rfp_PC_人質＿見つかるな聞いた ) {
		@rpc_PC_人質＿見つかるな
		eval($f:rfp_PC_人質＿見つかるな聞いた = 1)

	}else if( (!$f:rfp_PC_人質＿マイク使い方聞いた) && (!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_人質＿マイク使い方
		eval($f:rfp_PC_人質＿マイク使い方聞いた  = 1)

	}else if( (!$f:rfp_PC_人質＿マイク注意聞いた )  && (!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_人質＿マイク注意
		eval($f:rfp_PC_人質＿マイク注意聞いた = 1 )
	
	}else if(!$f:rfp_PC_人質＿敵兵定時連絡聞いた) {
		@rpc_PC_人質＿敵兵定時連絡
		eval($f:rfp_PC_人質＿敵兵定時連絡聞いた = 1)
	
	}else if(!$f:rfp_PC_人質＿人質殺すな聞いた){
		@rpc_PC_人質＿人質殺すな
		eval($f:rfp_PC_人質＿人質殺すな聞いた = 1)

	}else if(!$f:rfp_PC_人質部屋＿敵兵定時連絡２聞いた) {
		@rpc_PC_人質部屋＿敵兵定時連絡２
		eval($f:rfp_PC_人質部屋＿敵兵定時連絡２聞いた = 1)

	}else if(!$f:rfp_PC_シェル１中央棟Ｂ１聞いた) {
		@rpc_PC_シェル１中央棟Ｂ１
		eval($f:rfp_PC_シェル１中央棟Ｂ１聞いた = 1)

//		eval($f:rfp_PC_人質部屋マイクある終了  =1)
	
	//無線一回目
	}else if($w:rfp_callcount <= 1){
		@rpc_人質部屋マイクあるランダム

	//連続無線
	}else {
		rand 2
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rpc_人質部屋マイクあるランダム
		}else {
			@rpc_連続無線デフォルト
		}
	}
}

proc rpc_人質イベント中央棟 {
	if($s:エリア == "w24a") {
		@rpc_人質イベント中央棟１ＦＢ２

	}else if($s:エリア == "w24b") {
		@rpc_人質イベント中央棟Ｂ１

	}else if($s:エリア == "w24c") {
		@rpc_人質イベント人質部屋

	}else if($s:エリア == "w24d") {
		@rpc_人質イベント中央棟１ＦＢ２
	}
}

proc rpc_PC_オセロットがやってくる {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_オセロットがやってくる
}

proc rpc_エイムズ死後シェル２向かえ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if($w:アラートモード != d:ALERT_MODE_SNEAK){
		@rp_PC_エイムズ死後＿全館探索モード１
		if(!$f:rfp_PC_エイムズ死後シェル２向かえ聞いた) {
			@rpd_ローズ顔表示
			@rp_PC_エイムズ死後＿全館探索モード２
			eval($f:rfp_PC_エイムズ死後シェル２向かえ聞いた = 1 )
			@rpd_大佐顔表示
		}
	}
	@rp_PC_エイムズ死後＿全館探索モード３
}

proc rpc_PC_エイムズ死後＿変装無効１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エイムズ死後＿変装無効１
}

proc rpc_PC_エイムズ死後＿大統領 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エイムズ死後＿大統領
}

proc rpc_狙撃イベント＿シェル１シェル２連絡橋以外 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＤＧ連絡橋＿以外の場所１
	if($w:武器弾数Ｒ[4] < 0){
		@rp_PC_ＤＧ連絡橋＿以外の場所＿ＰＳＧ１ない
	}else {
		@rp_PC_ＤＧ連絡橋＿以外の場所＿ＰＳＧ１ある
	}
}

proc rpc_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１探せ {
	@rp_PC_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１探せ１
	if(!$f:rpf_PS_スネークＰＳＧ１場所聞いた){
		@rpd_ローズ顔表示
		@rp_PC_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１探せ２
	}else {
		@rp_PC_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１探せ２＿２
	}
}


proc rpc_狙撃イベント＿シェル１シェル２連絡橋一回目 {
	@rp_PC_ＤＧ連絡橋＿赤外線センサー＿センサー
	@rp_PC_ＤＧ連絡橋＿赤外線センサー＿赤外線触るな
	@rp_PC_ＤＧ連絡橋＿赤外線センサー＿赤外線くぐれない
	@rp_PC_ＤＧ連絡橋＿赤外線センサー＿制御装置破壊せよ
	@rp_PC_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１
	if($w:武器弾数Ｒ[4] < 0){
		@rpc_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１探せ
	}else {
		@rp_PC_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１使え
	}
}

//まじおしえる条件
proc rpc_赤外線見方 {
	@rp_PC_ＥＦ連絡橋＿赤外線センサー＿たばこ
//	@rp_PC_ＥＦ連絡橋＿赤外線センサー＿たばこズバリ
	eval($f:rfp_PC_赤外線見方聞いた = 1)
}

proc rpc_ＥＦ連絡橋＿赤外線センサー＿制御装置破壊 {
	@rp_PC_ＥＦ連絡橋＿赤外線センサー＿制御装置破壊
	//麻酔銃ある場合
	if($w:武器弾数Ｒ[1] >= 0 ){
		@rp_PC_ＥＦ連絡橋＿赤外線センサー４
	}
}

proc rpc_狙撃イベント＿シェル１シェル２連絡橋 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_狙撃イベント＿シェル１シェル２連絡橋一回目聞いた){
		@rpc_狙撃イベント＿シェル１シェル２連絡橋一回目
		eval($f:rfp_PC_狙撃イベント＿シェル１シェル２連絡橋一回目聞いた = 1 )

	}else if(!$f:rfp_PC_赤外線見方聞いた) {
		@rpc_赤外線見方
		eval($f:rfp_PC_赤外線見方聞いた = 1)

	//ＰＳＧ１なしゃしゃ
	}else if($w:武器弾数Ｒ[4] < 0){
		rand 5
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 3){
			@rpc_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１探せ
		}else if($w:rfp_rand < 4){
			@rp_PC_ＤＧ連絡橋＿赤外線センサー＿赤外線触るな
		}else {
			@rpc_赤外線見方
		}

	//ＰＳＧ１あるろ
	}else if(!$f:rfp_PC_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１違い聞いた) {
		@rp_PC_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１違い
		@rp_PC_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１スコープ
		@rp_PC_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１構え
		eval($f:rfp_PC_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１違い聞いた  =1 )
		
	}else if(!$f:rfp_PC_ＤＧ連絡橋＿赤外線センサー＿制御装置場所１聞いた){
		@rp_PC_ＤＧ連絡橋＿赤外線センサー＿制御装置場所１
		eval($f:rfp_PC_ＤＧ連絡橋＿赤外線センサー＿制御装置場所１聞いた = 1 )
	
	}else if(!$f:rfp_PC_ＤＧ連絡橋＿赤外線センサー＿爆弾撃つな聞いた) {
		@rp_PC_ＤＧ連絡橋＿赤外線センサー＿爆弾撃つな
		eval($f:rfp_PC_ＤＧ連絡橋＿赤外線センサー＿爆弾撃つな聞いた = 1 )
	
	}else if(!$f:rfp_PC_ＥＦ連絡橋＿赤外線センサー＿制御装置破壊聞いた ) {
		@rpc_ＥＦ連絡橋＿赤外線センサー＿制御装置破壊
		eval($f:rfp_PC_ＥＦ連絡橋＿赤外線センサー＿制御装置破壊聞いた = 1 )
	
	}else if(!$f:rfp_PC_ＥＦ連絡橋＿赤外線センサー＿工夫狙い聞いた ) {
		@rp_PC_ＥＦ連絡橋＿赤外線センサー＿工夫狙い
		eval($f:rfp_PC_ＥＦ連絡橋＿赤外線センサー＿工夫狙い聞いた = 1 )
	
	//これ呼ぶタイミングＸＸＸＸＸＸＸＸＸＸＸｘｘ
	}else if(!$f:rfp_PC_ＤＧ連絡橋＿赤外線センサー＿制御装置場所２聞いた) {
		@rp_PC_ＤＧ連絡橋＿赤外線センサー＿制御装置場所２
		eval($f:rfp_PC_ＤＧ連絡橋＿赤外線センサー＿制御装置場所２聞いた = 1 )


	}else {
		rand 9
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PC_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１スコープ

		}else if($w:rfp_rand < 2){
			@rp_PC_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１構え
			
		}else if($w:rfp_rand < 3){
			@rp_PC_ＤＧ連絡橋＿赤外線センサー＿制御装置場所１
		
		}else if($w:rfp_rand < 4){
			@rp_PC_ＤＧ連絡橋＿赤外線センサー＿爆弾撃つな
		
		}else if($w:rfp_rand < 5){
			@rpc_ＥＦ連絡橋＿赤外線センサー＿制御装置破壊
		
		}else if($w:rfp_rand < 6){
			@rp_PC_ＥＦ連絡橋＿赤外線センサー＿工夫狙い
		
		}else if($w:rfp_rand < 7){
			@rp_PC_ＤＧ連絡橋＿赤外線センサー＿制御装置場所２

		}else if($w:rfp_rand < 8){
			@rpc_赤外線見方

		}else {
			@rp_PC_ＥＦ連絡橋＿赤外線センサー＿クリア
		}
	}
}

proc rpc_PC_センサー破壊後＿シェル２行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_センサー破壊後＿シェル２行け
}

proc rpc_PC_ハリアー戦＿銃効かない {
	@rp_PC_ハリアー戦＿銃効かない１
	//ＲＧＢ６ある。っていうか使えるの？？ＸＸＸＸＸＸＸＸＸＸｘ
/*
	if($w:武器弾数Ｒ[5] >= 0 ){
		@rp_PC_ハリアー戦＿銃効かない２
	}
*/
}

proc rpc_PC_エマ救出後＿エマおいてけぼり {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エマ救出後＿エマおいてけぼり
	@rp_ローズさんを怒らせました
}

proc rpc_ハリアー戦＿クラスター爆弾 {
	if(!$f:rfp_PC_ハリアー戦＿クラスター爆弾聞いた){
		@rp_PC_ハリアー戦＿クラスター爆弾前
	}
	@rp_PC_ハリアー戦＿クラスター爆弾よけ方
}


proc rpc_ハリアー戦中 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_ハリアー戦＿銃効かない聞いた){
		@rpc_PC_ハリアー戦＿銃効かない
		eval($f:rfp_PC_ハリアー戦＿銃効かない聞いた = 1 )

	}else if( (!$f:rfp_PC_ハリアー戦＿レーダー聞いた) && \
				($f:w18a_ノードフラグ) && (!($w:コンフィグ設定 & d:CONFIG_RADAR_OFF)) ){
		@rp_PC_ハリアー戦＿レーダー
		eval($f:rfp_PC_ハリアー戦＿レーダー聞いた = 1)

	}else if(!$f:rfp_PC_ハリアー戦＿突撃聞いた ) {
		@rp_PC_ハリアー戦＿突撃
		eval($f:rfp_PC_ハリアー戦＿突撃聞いた = 1 )

	}else if(!$f:rfp_PC_ハリアー戦＿ロケットミサイル聞いた) {
		@rp_PC_ハリアー戦＿ロケットミサイル
		eval($f:rfp_PC_ハリアー戦＿ロケットミサイル聞いた = 1 )
		
	}else if(!$f:rfp_PC_ハリアー戦＿クラスター爆弾聞いた){
		@rpc_ハリアー戦＿クラスター爆弾
		eval($f:rfp_PC_ハリアー戦＿クラスター爆弾聞いた  =1 )

	}else if(!$f:rfp_PC_ハリアー戦＿焼き攻撃聞いた) {
		@rp_PC_ハリアー戦＿焼き攻撃
		eval($f:rfp_PC_ハリアー戦＿焼き攻撃聞いた = 1 )
	
	}else if(!$f:rfp_PC_ハリアー戦＿フレア聞いた) {
		@rp_PC_ハリアー戦＿フレア
		eval($f:rfp_PC_ハリアー戦＿フレア聞いた = 1 )
	
	}else if(!$f:rfp_PC_ハリアー戦＿アムラーム前) {
		@rp_PC_ハリアー戦＿アムラーム前
		eval($f:rfp_PC_ハリアー戦＿アムラーム前 = 1)
	
	}else if(!$f:rfp_PC_ハリアー戦＿ＲＧＢ６有効条件聞いた) {
		@rp_PC_ハリアー戦＿ＲＧＢ６有効条件
		eval($f:rfp_PC_ハリアー戦＿ＲＧＢ６有効条件聞いた = 1)


	}else {
		rand 11
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 3){
			@rp_PC_ハリアー戦＿突撃

		}else if($w:rfp_rand < 4){
			@rpc_PC_ハリアー戦＿銃効かない

		}else if($w:rfp_rand < 5){
			@rp_PC_ハリアー戦＿ロケットミサイル
		
		}else if($w:rfp_rand < 6){
			@rpc_ハリアー戦＿クラスター爆弾

		}else if($w:rfp_rand < 7){
			@rp_PC_ハリアー戦＿焼き攻撃
		
		}else if($w:rfp_rand < 8){
			@rp_PC_ハリアー戦＿フレア

		}else if($w:rfp_rand < 9){
			@rp_PC_ハリアー戦＿ＲＧＢ６有効条件

		}else if( ($w:rfp_rand < 10) && \
				($f:w18a_ノードフラグ) && (!($w:コンフィグ設定 & d:CONFIG_RADAR_OFF)) ){
			@rp_PC_ハリアー戦＿レーダー

		}else {
			@rp_PC_ハリアー戦＿アムラーム前
		}
	}
}

proc rpc_PC_ハリアー戦後＿大統領助けろ{
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ハリアー戦後＿大統領助けろ１
}


proc rpc_PC_ハリアー戦後＿シェル２中央棟への道 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ハリアー戦後＿シェル２中央棟への道１
	@rp_PC_ハリアー戦後＿シェル２中央棟への道２
	@rp_PC_ハリアー戦後＿シェル２中央棟への道３
	@rp_PC_ハリアー戦後＿シェル２中央棟への道４
	@rp_PC_ハリアー戦後＿シェル２中央棟への道５
}

proc rpc_PC_ＤＧ連絡橋＿渡れ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＤＧ連絡橋＿渡れ
	eval($f:rfp_PC_ＤＧ連絡橋＿渡れ聞いた = 1) 
}

proc rpc_PC_ＤＧ連絡橋渡るが良い {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＤＧ連絡橋＿渡れ
	eval($f:rfp_PC_ＤＧ連絡橋＿渡れ聞いた = 1) 

	if(!$f:rfp_PC_ＤＧ連絡橋渡るが良い聞いた){
		@rp_PC_ＤＧ連絡橋＿渡り方１
		@rpd_大佐顔表示
		eval($f:rfp_PC_ＤＧ連絡橋渡るが良い聞いた = 1 )
	}
	@rp_PC_ＤＧ連絡橋＿渡り方２
}

proc rpc_PC_ＤＧ連絡橋＿渡った後 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＤＧ連絡橋＿渡れ
	eval($f:rfp_PC_ＤＧ連絡橋＿渡れ聞いた = 1) 
}

proc rpc_PC_ＤＧ連絡橋＿渡った後＿パイプ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_ＤＧ連絡橋＿パイプ上落ちるな聞いた) {
		@rp_PC_ＤＧ連絡橋＿パイプ上落ちるな
		eval($f:rfp_PC_ＤＧ連絡橋＿パイプ上落ちるな聞いた = 1)

	}else if(!$f:rfp_PC_ＤＧ連絡橋＿渡れ聞いた){
		@rp_PC_ＤＧ連絡橋＿渡れ
		eval($f:rfp_PC_ＤＧ連絡橋＿渡れ聞いた = 1) 
	
	
	}else {
		rand 2
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PC_ＤＧ連絡橋＿渡れ
		}else {
			@rp_PC_ＤＧ連絡橋＿パイプ上落ちるな
		}
	}
}

proc rpc_PC_Ｇ脚＿渡れ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｇ脚＿渡れ
}

proc rpc_PC_ＬＧ連絡橋＿敵は排除 {
	if( (!$f:rfp_PC_ＬＧ連絡橋＿敵は排除聞いた) && ($f:w25b_デモカット再生済み) ){
		@rp_PC_ＬＧ連絡橋＿敵は排除１
		eval($f:rfp_PC_ＬＧ連絡橋＿敵は排除聞いた = 1 )
	}
	@rp_PC_ＬＧ連絡橋＿敵は排除２
	if($s:rfp_w25b_いまいるところ == "G_LG"){
		@rp_PC_ＬＧ連絡橋＿敵は排除ＰＳＧ１
	}
}


proc rpc_PC_Ｇ脚＿ＬＧ連絡橋側 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＬＧ連絡橋＿飛び降り

/*
	if(!$f:rfp_PC_ＬＧ連絡橋＿敵は排除聞いた){
		@rpc_PC_ＬＧ連絡橋＿敵は排除
		eval($f:rfp_PC_ＬＧ連絡橋＿敵は排除聞いた = 1 )

	}else if(!$f:rfp_PC_ＬＧ連絡橋＿飛び降り聞いた){
		@rp_PC_ＬＧ連絡橋＿飛び降り
		eval($f:rfp_PC_ＬＧ連絡橋＿飛び降り聞いた = 1 )
	
	}else {
		rand 2
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rpc_PC_ＬＧ連絡橋＿敵は排除
		}else {
			@rp_PC_ＬＧ連絡橋＿飛び降り
		}
	}
*/
}


proc rpc_PC_ＬＧ連絡橋＿Ｌ脚行け {
	@rp_PC_ＬＧ連絡橋＿Ｌ脚行け１
	if(!$f:rfp_PC_ＬＧ連絡橋＿Ｌ脚行け聞いた){
		@rpd_ローズ顔表示
		@rp_PC_ＬＧ連絡橋＿Ｌ脚行け２
		eval($f:rfp_PC_ＬＧ連絡橋＿Ｌ脚行け聞いた = 1)
	}
	@rp_PC_ＬＧ連絡橋＿Ｌ脚行け３
}

proc rp_敵兵元気さんです $:敵兵のお名前 {
	command ゲット敵兵状態拡張版 $:敵兵のお名前 $i:rfp_ゲットされた敵兵の状態拡張版
	command ゲット敵兵状態 $:敵兵のお名前 $i:rfp_ゲットされた敵兵の状態
	if( ($i:rfp_ゲットされた敵兵の状態 & 0x80000000) || \	//しぼーん
		($i:rfp_ゲットされた敵兵の状態拡張版 & 0x00000001) || \	//気絶
		($i:rfp_ゲットされた敵兵の状態拡張版 & 0x00000002) \	//おやすみ
	){
		return 0
	}else {
		return 1
	}
}

proc rp_ＬＧ連絡橋に敵がいます {
	//つまり新ハード以上
	if ($w:ゲーム設定 >= d:LEVEL_NORMAL) {
		if( (`@rp_敵兵元気さんです 敵兵:01`) || (`@rp_敵兵元気さんです 敵兵:02`) ){
			return 1
		}else {
			return 0
		}
	//早い話が新ノーマル以上
	}else if($w:ゲーム設定 >= d:LEVEL_EASY) {
		if(`@rp_敵兵元気さんです 敵兵:02`){
			return 1
		}else {
			return 0
		}
	}
}


proc rpc_PC_ＬＧ連絡橋 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_ＬＧ連絡橋＿Ｌ脚行け聞いた){
		@rpc_PC_ＬＧ連絡橋＿Ｌ脚行け
		eval($f:rfp_PC_ＬＧ連絡橋＿Ｌ脚行け聞いた = 1)

	}else if( (!$f:rfp_PC_ＬＧ連絡橋＿敵は排除聞いた) && \
				(`@rp_ＬＧ連絡橋に敵がいます`) ){
		@rpc_PC_ＬＧ連絡橋＿敵は排除
		eval($f:rfp_PC_ＬＧ連絡橋＿敵は排除聞いた = 1 )
		eval($f:rfp_PC_ＬＧ連絡橋聞いた = 1)
	
	}else {
		eval($f:rfp_PC_ＬＧ連絡橋聞いた = 1)
		rand 2
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 1) && \
				(`@rp_ＬＧ連絡橋に敵がいます`) ){
			@rpc_PC_ＬＧ連絡橋＿敵は排除
		}else {
			@rpc_PC_ＬＧ連絡橋＿Ｌ脚行け
		}
	}
}


proc rpc_PC_Ｌ脚＿渡れ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｌ脚＿渡れ
}

proc rpc_PC_ＫＬ連絡橋＿渡れ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＫＬ連絡橋＿渡れ
}

proc rpc_PC_ＤＧ連絡橋＿レーダーが使えない {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_ＤＧ連絡橋＿レーダーが使えない聞いた) {
		@rp_PC_ＤＧ連絡橋＿レーダーが使えない１
		eval($f:rfp_PC_ＤＧ連絡橋＿レーダーが使えない聞いた = 1)
		@rpd_大佐顔表示
	}
	@rp_PC_ＤＧ連絡橋＿レーダーが使えない２
}

proc rpc_PC_Ｌ脚外周＿放尿兵１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｌ脚外周＿放尿兵１
}

proc rpc_リモコンミサイルで壊せ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	//リモコンミサイルないない
	if($w:武器弾数Ｒ[6] < 0){
		@rp_PC_リモコンミサイル１
		//スネークの話聞いてない
		if(!$f:rfp_PS_リモコンミサイルの場所聞いた) {
			@rp_PC_リモコンミサイル＿聞く前
		}else {
			@rp_PC_リモコンミサイル＿聞いた後
		}
	}else {
		if($s:エリア == "w31a") {
			@rp_PC_リモコンミサイル１
			@rp_PC_リモコンミサイル＿使え
		}else {
			@rpc_PC_リモコンミサイル＿入手後
		}
	}
}

proc rpc_PC_リモコンミサイル＿入手後 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_リモコンミサイル＿入手後
}

proc rpc_PC_リモコンミサイル＿大統領に当てるな {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_リモコンミサイル＿大統領に当てるな１
/*
	if(!$f:rfp_PC_リモコンミサイル＿大統領に当てるな聞いた){
		@rp_PC_リモコンミサイル＿大統領に当てるな２
		eval($f:rfp_PC_リモコンミサイル＿大統領に当てるな聞いた = 1 )
	}
*/
}

proc rpc_PC_リモコンミサイル＿大統領に当てるな２ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_リモコンミサイル＿大統領に当てるな２
}

proc rpc_PC_リモコンミサイル＿ガンカメラ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_リモコンミサイル＿ガンカメラ
}

proc rpc_PC_リモコンミサイル＿ＶＲ類似 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_リモコンミサイル＿ＶＲ類似１
	//むびＸＸＸＸＸＸＸＸ
	@rp_PC_リモコンミサイル＿ＶＲ類似２
}

proc rpc_リモコンミサイルイベントランダム{
	rand 3
	eval($w:rfp_rand = $status)

	if( ($w:rfp_rand < 1) && \
		($f:rfp_31a_暗い部屋にいまそかり) && \
		(! ($b:w31a_破壊フラグ[22] && $b:w31a_破壊フラグ[23] ) ) ){
		@rpc_PC_リモコンミサイル＿ガンカメラ

	}else if($w:rfp_rand < 2){
		@rpc_PC_リモコンミサイル＿大統領に当てるな
		
	}else{
		@rpc_リモコンミサイルで壊せ
	}

/*
	}else {
		@rpc_PC_リモコンミサイル＿大統領に当てるな２
	}
*/
}

//激ヤバＸＸＸＸＸＸＸＸＸＸＸＸＸｘ
proc rpc_PC_電撃床破壊後大統領助けろ{
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_電撃床破壊後大統領助けろ
}

proc rpc_PC_エマ探せ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エマ探せ１
	if( ($s:エリア == "w31b" ) || ($s:エリア == "w31c" ) ){
		@rp_PC_エマ探せ＿Ｂ１
	}else {
		@rp_PC_エマ探せ＿Ｂ１以外
	}
}

proc rpc_P051_01_R01_エマとオタコン１無線機デモ１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@vc115101
}


//ズバリヒント条件ＸＸＸＸＸＸＸＸＸＸｘ
proc rpc_PC_ヴァンプ戦＿弾当たらない{
	if(!$f:rfp_PC_ヴァンプ戦＿弾当たらない聞いた) {
		@rp_PC_ヴァンプ戦＿弾当たらない１
	}else {
		@rp_PC_ヴァンプ戦＿弾当たらないズバリ
	}
}

proc rpc_ヴァンプ戦中 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_ヴァンプ戦＿弾当たらない聞いた) {
		@rpc_PC_ヴァンプ戦＿弾当たらない
		eval($f:rfp_PC_ヴァンプ戦＿弾当たらない聞いた = 1)
	
		//PC_ヴァンプ戦＿基本戦法１はビミョー
		//PC_ヴァンプ戦＿水中弾当たらんもビミョー
		//PC_ヴァンプ戦＿引きずり落しもビミョー

	}else if(!$f:rfp_PC_ヴァンプ戦＿プール一発氏に聞いた) {
		@rp_PC_ヴァンプ戦＿プール一発氏に
		eval($f:rfp_PC_ヴァンプ戦＿プール一発氏に聞いた = 1 )

	}else if(!$f:rfp_PC_ヴァンプ戦＿ナイフ弾ける聞いた){
		@rp_PC_ヴァンプ戦＿ナイフ弾ける
		eval($f:rfp_PC_ヴァンプ戦＿ナイフ弾ける聞いた = 1)

	}else {
		rand 3
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rpc_PC_ヴァンプ戦＿弾当たらない
		}else if($w:rfp_rand < 2){
			@rp_PC_ヴァンプ戦＿ナイフ弾ける
		}else {
			@rp_PC_ヴァンプ戦＿プール一発氏に
		}
	}
}

proc rpc_PC_ヴァンプ戦後＿エマ助けろ第二濾過槽 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ヴァンプ戦後＿エマ助けろ第二濾過槽
}

proc rpc_PC_ヴァンプ戦後＿エマ助けろ第二濾過槽以南 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ヴァンプ戦後＿エマ助けろ第二濾過槽以南
}

proc rpc_PC_ヴァンプ戦後＿ロッカー室 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_ヴァンプ戦後＿ロッカー室聞いた){
		@rp_PC_ヴァンプ戦後＿ロッカー室１
		eval($f:rfp_PC_ヴァンプ戦後＿ロッカー室聞いた = 1)
		@rp_PC_ヴァンプ戦後＿ロッカー室２
/*
		if($f:rfp_w31c_エマ心音聞いた) {
			@rpd_ローズ顔表示
			@rp_PC_ヴァンプ戦後＿ロッカー室＿心音聞いた	
		}
*/

	}else if( (!$f:rfp_PC_ヴァンプ戦後＿ロッカー室＿ノード聞いた) && \
				(!$f:w31c_ノードフラグ) ){
		@rp_PC_ヴァンプ戦後＿ロッカー室＿ノード
		eval($f:rfp_PC_ヴァンプ戦後＿ロッカー室＿ノード聞いた = 1)
	
	}else {
		rand 2
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 1) && \
				(!$f:w31c_ノードフラグ) ){
			@rp_PC_ヴァンプ戦後＿ロッカー室＿ノード
		}else {
			@rp_PC_ヴァンプ戦後＿ロッカー室２
/*
			if($f:rfp_w31c_エマ心音聞いた) {
				@rpd_ローズ顔表示
				@rp_PC_ヴァンプ戦後＿ロッカー室＿心音聞いた	
			}
*/
		}
	}
}


proc rpc_PC_エマ連れてけ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エマ連れてけ
}


proc rpc_PC_エマ水中＿エマＯ２ {
	if(!$f:rfp_PC_エマ水中＿エマＯ２聞いた) {
		@rp_PC_エマ水中＿エマＯ２１
		@rp_PC_エマ水中＿エマＯ２１＿２
		eval($f:rfp_PC_エマ水中＿エマＯ２聞いた = 1)
	}
	@rp_PC_エマ水中＿エマＯ２２
}
	

proc rpc_エマ水中 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_エマ水中＿エマＯ２聞いた) {
		@rpc_PC_エマ水中＿エマＯ２
		eval($f:rfp_PC_エマ水中＿エマＯ２聞いた = 1)

	//息継ぎ場所がなくなっている PC_エマ水中＿息継ぎ はビミョー
		eval($f:rfp_エマ水中終了 = 1 )

	}else {
		@rpc_PC_エマ水中＿エマＯ２
	}
}

proc rpc_PC_エマ手つなぎ＿座り込み {
	if(!$f:rfp_PC_エマ手つなぎ＿座り込み聞いた) {
		@rp_PC_エマ手つなぎ＿座り込み１
		eval($f:rfp_PC_エマ手つなぎ＿座り込み聞いた = 1)
		@rpd_大佐顔表示
	}
	@rp_PC_エマ手つなぎ＿座り込み２
}

proc rpc_PC_エマ手つなぎ＿引きずり {
	@rp_PC_エマ手つなぎ＿引きずり１
	if(!$f:rfp_PC_エマ手つなぎ＿引きずり聞いた) {
		@rp_PC_エマ手つなぎ＿引きずり２
		@rpd_大佐顔表示
	}
	@rp_PC_エマ手つなぎ＿引きずり３
}

proc rpc_PC_エマ救出後＿虫エリア {
	if(!$f:rfp_PC_エマ救出後＿虫エリア聞いた) {
		@rp_PC_エマ救出後＿虫エリア１
		eval($f:rfp_PC_エマ救出後＿虫エリア聞いた = 1)
		@rpd_大佐顔表示
	}
	if( ($i:プレイタイム - $i:rfp_w31b_エマ虫嫌がり開始時刻) / d:FRAME_RATE >= 120){
		@rp_PC_エマ救出後＿虫エリアズバリ
	}else {
		@rp_PC_エマ救出後＿虫エリア２
	}
}

proc rpc_エマ手つなぎ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	command ゲットエマ状態 $b:rfp_ゲットされたエマ状態

	if(!$f:エマ存在フラグ){
		@rp_PC_エマ救出後＿エマおいてけぼり
		@rp_ローズさんを怒らせました

	//ムシいや
	}else if($b:rfp_ゲットされたエマ状態 == d:EMA_F_I_DISLIKE_WORMS){
		@rpc_PC_エマ救出後＿虫エリア
	
	}else if(!$f:rfp_PC_エマ手つなぎ＿手つなぎ聞いた){
		@rp_PC_エマ手つなぎ＿手つなぎ
		eval($f:rfp_PC_エマ手つなぎ＿手つなぎ聞いた = 1)

	}else if(!$f:rfp_PC_エマ手つなぎ＿手つなぎ＿武器だめ聞いた){
		@rp_PC_エマ手つなぎ＿手つなぎ＿武器だめ
		eval($f:rfp_PC_エマ手つなぎ＿手つなぎ＿武器だめ聞いた = 1)
	
	}else if(!$f:rfp_PC_エマ手つなぎ＿座り込み聞いた) {
		@rpc_PC_エマ手つなぎ＿座り込み
		eval($f:rfp_PC_エマ手つなぎ＿座り込み聞いた = 1)

	}else if(!$f:rfp_PC_エマ手つなぎ＿引きずり聞いた) {
		@rpc_PC_エマ手つなぎ＿引きずり
		eval($f:rfp_PC_エマ手つなぎ＿引きずり聞いた = 1 )

	}else if(!$f:rfp_PC_エマ手つなぎ＿エマＬＩＦＥ聞いた) {
		@rp_PC_エマ手つなぎ＿エマＬＩＦＥ
		eval($f:rfp_PC_エマ手つなぎ＿エマＬＩＦＥ聞いた = 1 )

	}else if(!$f:rfp_PC_エマ手つなぎ＿エマＬＩＦＥ回復聞いた) {
		@rp_PC_エマ手つなぎ＿エマＬＩＦＥ回復
		eval($f:rfp_PC_エマ手つなぎ＿エマＬＩＦＥ回復聞いた = 1)

	}else if(!$f:rfp_PC_エマ手つなぎ＿基本戦法１聞いた) {
		@rp_PC_エマ手つなぎ＿基本戦法１
		eval($f:rfp_PC_エマ手つなぎ＿基本戦法１聞いた = 1)
		eval($f:rfp_エマ手つなぎ終了 = 1)

	}else {
		eval($f:rfp_エマ手つなぎ終了 = 1)
		rand 7
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PC_エマ手つなぎ＿手つなぎ

		}else if($w:rfp_rand < 2){
			@rp_PC_エマ手つなぎ＿手つなぎ＿武器だめ

		}else if($w:rfp_rand < 3){
			@rpc_PC_エマ手つなぎ＿座り込み

		}else if($w:rfp_rand < 4){
			@rpc_PC_エマ手つなぎ＿引きずり

		}else if($w:rfp_rand < 5){
			@rp_PC_エマ手つなぎ＿エマＬＩＦＥ

		}else if($w:rfp_rand < 6){
			@rp_PC_エマ手つなぎ＿エマＬＩＦＥ回復

		}else {
			@rp_PC_エマ手つなぎ＿基本戦法１
		}
	}
}


proc rpc_PC_エマ救出後＿Ｌ脚行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エマ救出後＿Ｌ脚行け
}

proc rpc_PC_エマ救出後＿Ｌ脚向かえ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エマ救出後＿Ｌ脚向かえ
}

proc rpc_PC_Ｌ脚＿南行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｌ脚＿南行け
}

proc rpc_PC_ＫＬ連絡橋＿エマ後Ｌ脚＿行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＫＬ連絡橋＿エマ後Ｌ脚＿行け
}

proc rpc_PC_エマ狙撃＿エマ足止め {
	if(!$f:rfp_PC_エマ狙撃＿エマ足止め聞いた){
		@rp_PC_エマ狙撃＿エマ足止め１
		eval($f:rfp_PC_エマ狙撃＿エマ足止め聞いた = 1)
	}
	@rp_PC_エマ狙撃＿エマ足止め２
}

proc rpc_PC_エマ狙撃＿ＰＳＧ１Ｔ {
	@rp_PC_エマ狙撃＿ＰＳＧ１Ｔ１
	if($w:武器弾数Ｒ[19] >= 0 ){
		@rp_PC_エマ狙撃＿ＰＳＧ１Ｔもってる１
		@rp_PC_エマ狙撃＿ＰＳＧ１Ｔもってる２
	}else {
		@rp_PC_エマ狙撃＿ＰＳＧ１Ｔもってない
	}
}

proc rpc_PC_エマ狙撃＿リモコンミサイル使えない {
	@rp_PC_エマ狙撃＿リモコンミサイル使えない
}

proc rpc_PC_エマ狙撃＿エマ撃つな {
	@rp_PC_エマ狙撃＿エマ撃つな１
/*
	if($f:rfp_エマ攻撃した) {
		@rp_PC_エマ狙撃＿エマ撃つな２
	}
*/
}

//これ日本版

proc rp_w32_クレイモアあります {
	#ifdef d:JAPANESE
	if(	(($w:ゲーム設定 >= d:LEVEL_NORMAL) && \
		 ( (!$f:rfp_w32a_地雷１爆発した) || \
			(!$f:rfp_w32a_地雷２爆発した) || \
			(!$f:rfp_w32a_地雷３爆発した) || \
			(!$f:rfp_w32a_地雷４爆発した) || \
			(!$f:rfp_w32a_地雷５爆発した) ) ) || \

		(($w:ゲーム設定 >= d:LEVEL_HARD) && \
			(	(!$f:rfp_w32a_地雷６爆発した) || \
				(!$f:rfp_w32a_地雷７爆発した)  || \
				(!$f:rfp_w32a_地雷８爆発した) ) ) || \

		(($w:ゲーム設定 >= d:LEVEL_EXTREME) && \
			(	(!$f:rfp_w32a_地雷９爆発した) || \
				(!$f:rfp_w32a_地雷１０爆発した) || \
				(!$f:rfp_w32a_地雷１１爆発した) ) ) ){
			return 1
	}else {
		return 0
	}
	#else 
	if(	(($w:ゲーム設定 >= d:LEVEL_EASY) && \
		 ( (!$f:rfp_w32a_地雷１爆発した) || \
			(!$f:rfp_w32a_地雷２爆発した) || \
			(!$f:rfp_w32a_地雷３爆発した) || \
			(!$f:rfp_w32a_地雷４爆発した) || \
			(!$f:rfp_w32a_地雷５爆発した) ) ) || \

		(($w:ゲーム設定 >= d:LEVEL_NORMAL) && \
			(	(!$f:rfp_w32a_地雷６爆発した) || \
				(!$f:rfp_w32a_地雷７爆発した) ) ) || \

		(($w:ゲーム設定 >= d:LEVEL_HARD) && \
			(	(!$f:rfp_w32a_地雷８爆発した) ) ) || \

		(($w:ゲーム設定 >= d:LEVEL_EXTREME) && \
			(	(!$f:rfp_w32a_地雷９爆発した) || \
				(!$f:rfp_w32a_地雷１０爆発した) || \
				(!$f:rfp_w32a_地雷１１爆発した) ) ) ){
			return 1

	}else {
		return 0
	}
	#endif
}

proc rpc_狙撃基本 {
	if(!$f:rfp_PC_エマ狙撃＿基本指示１聞いた) {
		@rp_PC_エマ狙撃＿基本指示１
		eval($f:rfp_PC_エマ狙撃＿基本指示１聞いた = 1 )
	
	}else if( (!$f:rfp_PC_エマ狙撃＿クレイモア聞いた) && \
				(`@rp_w32_クレイモアあります`) ){
		@rp_PC_エマ狙撃＿クレイモア
		eval($f:rfp_PC_エマ狙撃＿クレイモア聞いた = 1 )

	}else if(!$f:rfp_PC_エマ狙撃＿弾聞いた){
		@rp_PC_エマ狙撃＿弾
		eval($f:rfp_PC_エマ狙撃＿弾聞いた = 1 )
	
	}else if(!$f:rfp_PC_エマ狙撃＿姿勢聞いた) {
		@rp_PC_エマ狙撃＿姿勢
		eval($f:rfp_PC_エマ狙撃＿姿勢聞いた = 1 )

	}else if(!$f:rfp_PC_エマ狙撃＿ジアゼパム聞いた) {
		@rp_PC_エマ狙撃＿ジアゼパム
		eval($f:rfp_PC_エマ狙撃＿ジアゼパム聞いた = 1 )
/*
	}else if(!$f:rfp_PC_エマ狙撃＿桟橋ゆれアウト聞いた) {
		@rp_PC_エマ狙撃＿桟橋ゆれアウト
		eval($f:rfp_PC_エマ狙撃＿桟橋ゆれアウト聞いた = 1)
*/	
	}else if(!$f:rfp_PC_エマ狙撃＿エマ足止め聞いた){
		@rpc_PC_エマ狙撃＿エマ足止め
		eval($f:rfp_PC_エマ狙撃＿エマ足止め聞いた = 1)

	}else if(!$f:rfp_PC_エマ狙撃＿ＰＳＧ１Ｔ聞いた) {
		@rpc_PC_エマ狙撃＿ＰＳＧ１Ｔ
		eval($f:rfp_PC_エマ狙撃＿ＰＳＧ１Ｔ聞いた = 1)

	}else if( ($w:武器 == d:武器:ニキータ) && \
				(!$f:rfp_PC_エマ狙撃＿リモコンミサイル使えない) ){
		@rpc_PC_エマ狙撃＿リモコンミサイル使えない
		eval($f:rfp_PC_エマ狙撃＿リモコンミサイル使えない = 1)
	
	}else if(!$f:rfp_PC_エマ狙撃＿エマ撃つな聞いた){
		@rpc_PC_エマ狙撃＿エマ撃つな
		eval($f:rfp_PC_エマ狙撃＿エマ撃つな聞いた = 1)

	}else {
		rand 8
		eval($w:rfp_rand = $status)

		if( ($w:rfp_rand < 3) && \
				(`@rp_w32_クレイモアあります`) ){
			@rp_PC_エマ狙撃＿クレイモア

		}else if($w:rfp_rand < 4){
			@rp_PC_エマ狙撃＿基本指示１

		}else if($w:rfp_rand < 5){
			@rp_PC_エマ狙撃＿弾
		
		}else if($w:rfp_rand < 6){
			@rp_PC_エマ狙撃＿姿勢
/*
		}else if($w:rfp_rand < 7){
			@rp_PC_エマ狙撃＿桟橋ゆれアウト
*/
		
		}else if($w:rfp_rand < 7){
			@rpc_PC_エマ狙撃＿エマ足止め

		}else {
			@rp_PC_エマ狙撃＿ジアゼパム
		}
	}
}


proc rpc_PC_スネークに頼め {
	if(!$f:rfp_PC_スネークに頼め聞いた){
		@rp_PC_エマ狙撃＿スネークに頼め＿死にそう
		eval($f:rfp_PC_スネークに頼め聞いた = 1)
	
	}else if(!$f:rfp_PC_エマ狙撃＿ヴァンプ狙撃エマ死にそう聞いた) {
		@rp_PC_エマ狙撃＿ヴァンプ狙撃エマ死にそう
		eval($f:rfp_PC_エマ狙撃＿ヴァンプ狙撃エマ死にそう聞いた = 1)

	}else {
		rand 2
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rp_PC_エマ狙撃＿スネークに頼め＿死にそう
		}else {
			@rp_PC_エマ狙撃＿ヴァンプ狙撃エマ死にそう
		}
	}
}

proc rpc_エマ狙撃中 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	command ゲットエマライフ $i:エマライフ現在値
	if( (`@rp_エマたん瀕死`) && \
		(!$f:rfp_w32a_スネーク援護開始した) && \
		(d:ST:P067_01_R01エマ狙撃１無線機デモ１開始 <= $w:p_story ) && \
		($w:p_story < d:ST:P068_01_P01ヴァンプ狙撃前１ポリゴンデモ１開始) && \
		(!$f:rfp_PC_スネークに頼め聞いた) ){
			@rpc_PC_スネークに頼め
			eval($f:rfp_PC_スネークに頼め聞いた = 1)

	//ガンサイファー出現状態！ＸＸＸＸＸＸＸ
		//PC_エマ狙撃＿ガンサイファー

	}else {
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && \
			(`@rp_エマたん瀕死`) && \
			(!$f:rfp_w32a_スネーク援護開始した) && \
			(d:ST:P067_01_R01エマ狙撃１無線機デモ１開始 <= $w:p_story ) && \
			($w:p_story < d:ST:P068_01_P01ヴァンプ狙撃前１ポリゴンデモ１開始) ){

			@rpc_PC_スネークに頼め

		}else {
			@rpc_狙撃基本
		}
	}
}



proc rpc_ヴァンプ狙撃中 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	//エマヤバイ
	
	if(!$f:rfp_PC_エマ狙撃＿ヴァンプ狙撃＿基本聞いた){
		@rp_PC_エマ狙撃＿ヴァンプ狙撃＿基本
		eval($f:rfp_PC_エマ狙撃＿ヴァンプ狙撃＿基本聞いた = 1)

	}else if( (!$f:rfp_PC_ヴァンプ狙撃＿頭狙え聞いた) && \
				($w:ゲーム設定 >= d:LEVEL_EASY) ){
		@rp_PC_ヴァンプ狙撃＿頭狙え
		eval($f:rfp_PC_ヴァンプ狙撃＿頭狙え聞いた = 1)

	}else if(!$f:rfp_PC_エマ狙撃＿弾聞いた){
		@rp_PC_エマ狙撃＿弾
		eval($f:rfp_PC_エマ狙撃＿弾聞いた = 1 )
	
	}else if(!$f:rfp_PC_エマ狙撃＿姿勢聞いた) {
		@rp_PC_エマ狙撃＿姿勢
		eval($f:rfp_PC_エマ狙撃＿姿勢聞いた = 1 )

	}else if(!$f:rfp_PC_エマ狙撃＿ＰＳＧ１Ｔ聞いた) {
		@rpc_PC_エマ狙撃＿ＰＳＧ１Ｔ
		eval($f:rfp_PC_エマ狙撃＿ＰＳＧ１Ｔ聞いた = 1)
	
	}else if(!$f:rfp_PC_エマ狙撃＿ジアゼパム聞いた) {
		@rp_PC_エマ狙撃＿ジアゼパム
		eval($f:rfp_PC_エマ狙撃＿ジアゼパム聞いた = 1 )
	
	}else {
		rand 7
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 3){
			@rp_PC_エマ狙撃＿ヴァンプ狙撃＿基本
		
		}else if( ($w:rfp_rand < 4) && ($w:ゲーム設定 >= d:LEVEL_EASY) ){
			@rp_PC_ヴァンプ狙撃＿頭狙え

		}else if($w:rfp_rand < 5){
			@rp_PC_エマ狙撃＿弾

		}else if($w:rfp_rand < 6){
			@rp_PC_エマ狙撃＿姿勢
		
		}else {
			@rp_PC_エマ狙撃＿ジアゼパム
		}
	}
}


proc rpc_PC_エマ狙撃後＿電算室行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_エマ狙撃後＿電算室行け聞いた) {
		@rp_PC_エマ狙撃後＿電算室行け＿Ｅ脚１
		eval($f:rfp_PC_エマ狙撃後＿電算室行け聞いた = 1)
		@rpd_大佐顔表示
	}
	@rp_PC_エマ狙撃後＿電算室行け
}

proc rpc_PC_拘束解放前 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_拘束解放前
}

proc rpc_PC_拷問部屋＿回避モード状態 {
	@rp_PC_拷問部屋＿回避モード状態１
	if(!$f:rfp_PC_拷問部屋＿回避モード状態聞いた) {
		@rp_PC_拷問部屋＿回避モード状態２
		@rpd_ローズ顔表示
		@rp_PC_拷問部屋＿回避モード状態３
		eval($f:rfp_PC_拷問部屋＿回避モード状態聞いた = 1)
	}
}

proc rpc_PC_裸注意＿操作制限 {
	@rp_PC_裸注意＿操作制限１
	if(!$f:rfp_PC_裸注意＿操作制限聞いた) {
		@rp_PC_裸注意＿操作制限２
		eval($f:rfp_PC_裸注意＿操作制限聞いた)
	}
}

proc rpc_風邪状態 {
	if(!$f:rfp_PC_風邪状態聞いた){
		@rp_PC_裸注意＿風邪１
		eval($f:rfp_PC_風邪状態聞いた = 1 )
		@rpd_大佐顔表示
	}
	@rp_PC_裸注意＿風邪２

	//ズバリヒント条件ＸＸＸＸＸＸＸＸＸＸＸｘどのダンボール？
	@rpd_ローズ顔表示
//	if($f:rfp_PC_風邪解法ズパリ){
		@rp_PC_裸注意＿風邪３
//	}else {
//		@rp_PC_裸注意＿風邪＿ダンボール
//	}
}

proc rpc_裸注意 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
/*
	if( ($w:風邪状態 == 1) && (!$f:rfp_PC_風邪状態聞いた) ){
		@rpc_風邪状態
		eval($f:rfp_PC_風邪状態聞いた = 1)
*/	
	if(!$f:rfp_PC_裸注意＿操作制限聞いた) {
		@rpc_PC_裸注意＿操作制限
		eval($f:rfp_PC_裸注意＿操作制限聞いた = 1)
/*	
	}else if(!$f:rfp_PC_裸注意＿操作制限３聞いた) {
		@rp_PC_裸注意＿操作制限３
		eval($f:rfp_PC_裸注意＿操作制限３聞いた = 1)
*/

	}else if(!$f:rfp_PC_裸注意＿ダメージ聞いた) {
		@rp_PC_裸注意＿ダメージ
		eval($f:rfp_PC_裸注意＿ダメージ聞いた = 1 )
		eval($f:rfp_PC_裸注意終了 = 1 )
		
	}else {
		eval($f:rfp_PC_裸注意終了 = 1 )
		rand 3
		eval($w:rfp_rand = $status)
/*
		if( ($w:rfp_rand < 1) && ($w:風邪状態 == 1) ){
			@rpc_風邪状態
*/

		if($w:rfp_rand < 2){
			@rpc_PC_裸注意＿操作制限
/*
		}else if($w:rfp_rand < 2){
			@rp_PC_裸注意＿操作制限３
*/

		}else {
			@rp_PC_裸注意＿ダメージ
		}
	}
}



proc rpc_PC_拘束解除後＿合流しろ＿拷問部屋 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if( ( ($w:アラートモード == d:ALERT_MODE_AVOID) || ($w:アラートモード == d:ALERT_MODE_SEARCH) ) && \
		(!$f:w41a_捕まったふりバレたフラグ) && \
		(!$f:rfp_PC_拷問部屋＿回避モード状態聞いた) ) {
		@rpc_PC_拷問部屋＿回避モード状態
		eval($f:rfp_PC_拷問部屋＿回避モード状態聞いた = 1)

	}else if(!$f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋聞いた) {
		@rp_PC_拘束解除後＿合流しろ１
		@rp_PC_拘束解除後＿合流しろ＿拷問部屋

	}else {
		if( ( ($w:アラートモード == d:ALERT_MODE_AVOID) || ($w:アラートモード == d:ALERT_MODE_SEARCH) ) && \
			(!$f:w41a_捕まったふりバレたフラグ) ){
				@rpc_PC_拷問部屋＿回避モード状態
		}else {
			@rp_PC_拘束解除後＿合流しろ１
			@rp_PC_拘束解除後＿合流しろ＿拷問部屋
		}
	}
}

proc rpc_PC_拘束解除後＿合流しろ＿拷問部屋外 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_拘束解除後＿合流しろ１
	@rp_PC_拘束解除後＿合流しろ＿拷問部屋外
}

proc rpc_PC_拘束解除後＿合流しろ＿拷問部屋以外 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_拘束解除後＿合流しろ＿拷問部屋以外
}

proc rpc_スネークと合流しろ {
	if( ($s:エリア == "w41a") && ($f:rfp_w41a_拷問部屋にいるのＤＥＡＴＨ) && \
				(!$f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋聞いた) ) {
		@rpc_PC_拘束解除後＿合流しろ＿拷問部屋
		eval($f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋聞いた = 1)

	}else if( ($s:エリア == "w41a") && (!$f:rfp_w41a_拷問部屋にいるのＤＥＡＴＨ) && \
				(!$f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋外聞いた) ) {
			@rpc_PC_拘束解除後＿合流しろ＿拷問部屋外
			eval($f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋外聞いた = 1)

	}else if( ($s:エリア == "w42a") && (!$f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋以外聞いた) ){
			@rpc_PC_拘束解除後＿合流しろ＿拷問部屋以外
			eval($f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋以外聞いた = 1 )
	}
}

proc rpc_裸デフォルト {
	rand 3
	eval($w:rfp_rand = $status)

	if($w:rfp_rand < 2){
		if( ($s:エリア == "w41a") && ($f:rfp_w41a_拷問部屋にいるのＤＥＡＴＨ) ) {
			@rpc_PC_拘束解除後＿合流しろ＿拷問部屋

		}else if( ($s:エリア == "w41a") && (!$f:rfp_w41a_拷問部屋にいるのＤＥＡＴＨ) ) {
			@rpc_PC_拘束解除後＿合流しろ＿拷問部屋外

		}else if($s:エリア == "w42a"){
			@rpc_PC_拘束解除後＿合流しろ＿拷問部屋以外
			eval($f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋以外聞いた = 1 )
		//ほけん
		}else {
			@rpc_裸注意
		}

	}else {
		@rpc_裸注意
	}
}


proc rpc_PC_天狗通路Ａ＿メタル {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_天狗通路Ａ＿メタル聞いた){
		@rp_PC_天狗通路Ａ＿メタル１
		eval($f:rfp_PC_天狗通路Ａ＿メタル聞いた = 1)
		@rpd_大佐顔表示
	}
	@rp_PC_天狗通路Ａ＿メタル２
}

proc rpc_PC_天狗通路Ａ＿寒い {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_天狗通路Ａ＿寒い
}

proc rpc_PC_天狗通路＿天狗兵 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_天狗通路＿天狗兵１
	@rp_PC_天狗兵
}

proc rpc_裸連続デフォルト {
	if( ($s:エリア == "w42a" ) && \
		(!$f:rfp_PC_天狗通路Ａ＿メタル聞いた) ){
		@rpc_PC_天狗通路Ａ＿メタル
		eval($f:rfp_PC_天狗通路Ａ＿メタル聞いた = 1)
	
	}else if( ($s:エリア == "w42a") && \
				(!$f:rfp_PC_天狗通路＿天狗兵聞いた) ){
		@rpc_PC_天狗通路＿天狗兵
		eval($f:rfp_PC_天狗通路＿天狗兵聞いた = 1)

	}else if( ($s:エリア == "w42a") && \
				(!$f:rfp_PC_天狗通路Ａ＿寒い聞いた) ){
		@rpc_PC_天狗通路Ａ＿寒い
		eval($f:rfp_PC_天狗通路Ａ＿寒い聞いた = 1)

	}else {
		@rpc_裸デフォルト
	}
}


proc rpc_発狂中１ {
	//プレイ時間ＸＸＸＸＸＸＸＸＸＸｘ
	if( ($f:rfp_PC_プレイ時間長すぎるよ) && (!$f:rfp_PC_発狂＿プレイ時間聞いた) ){
		@rpc_PC_発狂＿プレイ時間
		eval($f:rfp_PC_発狂＿プレイ時間聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ローズの浮気聞いた){
		@rpc_PC_発狂＿ローズの浮気
		eval($f:rfp_PC_発狂＿ローズの浮気聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ＭＧＳ１聞いた){
		@rpc_PC_発狂＿ＭＧＳ１
		eval($f:rfp_PC_発狂＿ＭＧＳ１聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿格言１聞いた){
		@rpc_PC_発狂＿格言１
		eval($f:rfp_PC_発狂＿格言１聞いた = 1)

	//殺しカウントＸＸＸＸＸＸＸＸＸＸＸＸＸｘｘ
	}else if( ($f:rfp_PC_殺し過ぎです) && (!$f:rfp_PC_発狂＿殺しすぎ聞いた) ){
		@rpc_PC_発狂＿殺しすぎ
		eval($f:rfp_PC_発狂＿殺しすぎ聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿改造コード聞いた){
		@rpc_PC_発狂＿改造コード
		eval($f:rfp_PC_発狂＿改造コード聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ばべる１聞いた){
		@rpc_PC_発狂＿ばべる１
		eval($f:rfp_PC_発狂＿ばべる１聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ＶＲ１聞いた){
		@rpc_PC_発狂＿ＶＲ１
		eval($f:rfp_PC_発狂＿ＶＲ１聞いた = 1)

	//無線カウント
	}else if( ($f:rfp_PC_あまり無線してませんね) && (!$f:rfp_PC_発狂＿無線してない聞いた) ){
		@rpc_PC_発狂＿無線してない
		eval($f:rfp_PC_発狂＿無線してない聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ＭＳＸ１聞いた){
		@rpc_PC_発狂＿ＭＳＸ１
		eval($f:rfp_PC_発狂＿ＭＳＸ１聞いた = 1)

		eval($f:rfp_PC_発狂中１終了 = 1)

	}else{
		eval($f:rfp_PC_発狂中１終了 = 1)
		rand 10
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rpc_PC_発狂＿ＶＲ１

		}else if( ($w:rfp_rand < 2) && ($f:rfp_PC_プレイ時間長すぎるよ) ){
			@rpc_PC_発狂＿プレイ時間

		}else if($w:rfp_rand < 3){
			@rpc_PC_発狂＿格言１

		}else if($w:rfp_rand < 4){
			@rpc_PC_発狂＿ローズの浮気

		}else if($w:rfp_rand < 5){
			@rpc_PC_発狂＿ＭＧＳ１

		}else if( ($w:rfp_rand < 6) && ($f:rfp_PC_殺し過ぎです) ){
			@rpc_PC_発狂＿殺しすぎ

		}else if($w:rfp_rand < 7){
			@rpc_PC_発狂＿ばべる１

		}else if( ($w:rfp_rand < 8) && ($f:rfp_PC_あまり無線してませんね) ){
			@rpc_PC_発狂＿無線してない

		}else if($w:rfp_rand < 9){
			@rpc_PC_発狂＿ＭＳＸ１

		}else {
			@rpc_PC_発狂＿改造コード
		}
	}
}

proc rpc_発狂中２ {
	if(!$f:rfp_PC_発狂＿ＶＲ２聞いた) {
		@rpc_PC_発狂＿ＶＲ２
		eval($f:rfp_PC_発狂＿ＶＲ２聞いた = 1)

	//ゲームオーバー回数ＸＸＸＸＸＸＸＸＸＸＸＸ
	}else if( ($f:rfp_PC_ゲームオーバーなりすぎ) && (!$f:rfp_PC_発狂＿ゲームオーバー回数聞いた) ){
		@rpc_PC_発狂＿ゲームオーバー回数
		eval($f:rfp_PC_発狂＿ゲームオーバー回数聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ばべる２聞いた){
		@rpc_PC_発狂＿ばべる２
		eval($f:rfp_PC_発狂＿ばべる２聞いた = 1)

	//エロ回数ＸＸＸＸＸＸＸＸＸＸＸＸＸＸＸＸｘ
	}else if( ($f:rfp_PC_エロ過ぎです) && (!$f:rfp_PC_発狂＿エロ行為ツッコミ聞いた) ){ 
		@rpc_PC_発狂＿エロ行為ツッコミ
		eval($f:rfp_PC_発狂＿エロ行為ツッコミ聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ＭＳＸ２聞いた){
		@rpc_PC_発狂＿ＭＳＸ２
		eval($f:rfp_PC_発狂＿ＭＳＸ２聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ツーハン聞いた){
		@rpc_PC_発狂＿ツーハン
		eval($f:rfp_PC_発狂＿ツーハン聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ＭＧＳ２聞いた){
		@rpc_PC_発狂＿ＭＧＳ２
		eval($f:rfp_PC_発狂＿ＭＧＳ２聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿格言２聞いた){
		@rpc_PC_発狂＿格言２
		eval($f:rfp_PC_発狂＿格言２聞いた = 1)

		eval($f:rfp_PC_発狂中２終了 = 1)

	}else {
		eval($f:rfp_PC_発狂中２終了 = 1)
		rand 8
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rpc_PC_発狂＿ＶＲ２

		}else if( ($w:rfp_rand < 2) && ($f:rfp_PC_ゲームオーバーなりすぎ) ){
			@rpc_PC_発狂＿ゲームオーバー回数

		}else if($w:rfp_rand < 3){
			@rpc_PC_発狂＿ばべる２

		}else if( ($w:rfp_rand < 4) && ($f:rfp_PC_エロ過ぎです) ){
			@rpc_PC_発狂＿エロ行為ツッコミ

		}else if($w:rfp_rand < 5){
			@rpc_PC_発狂＿ＭＳＸ２

		}else if($w:rfp_rand < 6){
			@rpc_PC_発狂＿ツーハン

		}else if($w:rfp_rand < 7){
			@rpc_PC_発狂＿ＭＧＳ２

		}else {
			@rpc_PC_発狂＿格言２
		}
	}
}


proc rpc_発狂中３ {

	if(!$f:rfp_PC_発狂＿ＶＲ３聞いた){
		@rpc_PC_発狂＿ＶＲ３
		eval($f:rfp_PC_発狂＿ＶＲ３聞いた = 1 )

	}else if(!$f:rfp_PC_発狂＿大佐出撃聞いた){
		@rpc_PC_発狂＿大佐出撃
		eval($f:rfp_PC_発狂＿大佐出撃聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ＭＳＸ３聞いた){
		@rpc_PC_発狂＿ＭＳＸ３
		eval($f:rfp_PC_発狂＿ＭＳＸ３聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿寝ている聞いた){
		@rpc_PC_発狂＿寝ている
		eval($f:rfp_PC_発狂＿寝ている聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ＭＧＳ３聞いた){
		@rpc_PC_発狂＿ＭＧＳ３
		eval($f:rfp_PC_発狂＿ＭＧＳ３聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿知らない人聞いた){
		@rpc_PC_発狂＿知らない人
		eval($f:rfp_PC_発狂＿知らない人聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ばべる３聞いた){
		@rpc_PC_発狂＿ばべる３
		eval($f:rfp_PC_発狂＿ばべる３聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿食事中聞いた){
		@rpc_PC_発狂＿食事中
		eval($f:rfp_PC_発狂＿食事中聞いた = 1)

		eval($f:rfp_PC_発狂中３終了 = 1)

	}else {
		eval($f:rfp_PC_発狂中３終了 = 1)
		rand 8
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rpc_PC_発狂＿ＶＲ３

		}else if($w:rfp_rand < 2){
			@rpc_PC_発狂＿大佐出撃

		}else if($w:rfp_rand < 3){
			@rpc_PC_発狂＿ＭＳＸ３

		}else if($w:rfp_rand < 4){
			@rpc_PC_発狂＿寝ている

		}else if($w:rfp_rand < 5){
			@rpc_PC_発狂＿ＭＧＳ３

		}else if($w:rfp_rand < 6){
			@rpc_PC_発狂＿知らない人

		}else if($w:rfp_rand < 7){
			@rpc_PC_発狂＿ばべる３
	
		}else {
			@rpc_PC_発狂＿食事中
		}
	}
}


proc rpc_発狂中４ {
	if(!$f:rfp_PC_発狂＿電波聞いた){
		@rpc_PC_発狂＿電波
		eval($f:rfp_PC_発狂＿電波聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿借金聞いた){
		@rpc_PC_発狂＿借金
		eval($f:rfp_PC_発狂＿借金聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿前世聞いた){
		@rpc_PC_発狂＿前世
		eval($f:rfp_PC_発狂＿前世聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿能勢電聞いた){
		@rpc_PC_発狂＿能勢電
		eval($f:rfp_PC_発狂＿能勢電聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿らりるれろ聞いた){
		@rpc_PC_発狂＿らりるれろ
		eval($f:rfp_PC_発狂＿らりるれろ聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿まめ知識聞いた){
		@rpc_PC_発狂＿まめ知識
		eval($f:rfp_PC_発狂＿まめ知識聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿留守電聞いた){
		@rpc_PC_発狂＿留守電
		eval($f:rfp_PC_発狂＿留守電聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿電車聞いた){
		@rpc_PC_発狂＿電車
		eval($f:rfp_PC_発狂＿電車聞いた = 1)

		eval($f:rfp_PC_発狂中４終了 = 1)

	}else {
		eval($f:rfp_PC_発狂中４終了 = 1)
		rand 8
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rpc_PC_発狂＿電波

		}else if($w:rfp_rand < 2){
			@rpc_PC_発狂＿借金

		}else if($w:rfp_rand < 3){
			@rpc_PC_発狂＿前世

		}else if($w:rfp_rand < 4){
			@rpc_PC_発狂＿能勢電

		}else if($w:rfp_rand < 5){
			@rpc_PC_発狂＿らりるれろ

		}else if($w:rfp_rand < 6){
			@rpc_PC_発狂＿まめ知識

		}else if($w:rfp_rand < 7){
			@rpc_PC_発狂＿留守電

		}else {
			@rpc_PC_発狂＿電車
		}
	}
}




proc rpc_発狂中 {
	@rp_発狂フラグ立て処理

	if(d:ST:P072_01_R01大佐混乱１無線機デモ１終了 <= $w:p_story && \
				$w:p_story <  d:ST:P073_02_P01通路Ａ２ポリゴンデモ１開始){
		@rpc_発狂中１

	}else if(d:ST:P073_05_S01通路Ａ５シナリオデモ１終了 <= $w:p_story && \
				$w:p_story <  d:ST:P077_02_P01天狗兵降下ラッシュポリゴンデモ１開始){
		if(!$f:rfp_PC_発狂中２終了) {
			@rpc_発狂中２

		}else if(!$f:rfp_PC_発狂中１終了) {
			@rpc_発狂中１

		}else {
			rand 2
			eval($w:rfp_rand = $status)
			if($w:rfp_rand < 1){
				@rpc_発狂中１
			}else {
				@rpc_発狂中２
			}
		}
		
		
	}else if(d:ST:P077_02_P01天狗兵降下ラッシュポリゴンデモ１終了 <= $w:p_story && \
				$w:p_story <  d:ST:P078_01_P01ＡＧフォーチュン登場１ポリゴンデモ１開始){

		if(!$f:rfp_PC_発狂中３終了) {
			@rpc_発狂中３

		}else if(!$f:rfp_PC_発狂中２終了) {
			@rpc_発狂中２

		}else if(!$f:rfp_PC_発狂中１終了) {
			@rpc_発狂中１

		}else {
			rand 3
			eval($w:rfp_rand = $status)
			if($w:rfp_rand < 1){
				@rpc_発狂中１

			}else if($w:rfp_rand < 2){
				@rpc_発狂中２
			
			}else {
				@rpc_発狂中３
			}
		}

	}else {
		if(!$f:rfp_PC_発狂中４終了) {
			@rpc_発狂中４

		}else if(!$f:rfp_PC_発狂中３終了) {
			@rpc_発狂中３

		}else if(!$f:rfp_PC_発狂中２終了) {
			@rpc_発狂中２

		}else if(!$f:rfp_PC_発狂中１終了) {
			@rpc_発狂中１

		}else {
			rand 4
			eval($w:rfp_rand = $status)
			if($w:rfp_rand < 1){
				@rpc_発狂中１

			}else if($w:rfp_rand < 2){
				@rpc_発狂中２
			
			}else if($w:rfp_rand < 3){
				@rpc_発狂中３

			}else {
				@rpc_発狂中４
			}
		}
	}
	@rp_大佐発狂会話聞いた回数カウント
	@rp_発狂中大佐顔初期化処理
}




proc rpc_発狂ＣＡＬＬ１ {
	if( ($f:rfp_PC_プレイ時間長すぎるよ) && (!$f:rfp_PC_発狂＿プレイ時間聞いた) ){
		@rpc_PC_発狂＿プレイ時間
		eval($f:rfp_PC_発狂＿プレイ時間聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ローズの浮気聞いた){
		@rpc_PC_発狂＿ローズの浮気
		eval($f:rfp_PC_発狂＿ローズの浮気聞いた = 1)

	//殺しカウントＸＸＸＸＸＸＸＸＸＸＸＸＸｘｘ
	}else if( ($f:rfp_PC_殺し過ぎです) && (!$f:rfp_PC_発狂＿殺しすぎ聞いた) ){
		@rpc_PC_発狂＿殺しすぎ
		eval($f:rfp_PC_発狂＿殺しすぎ聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ＭＧＳ１聞いた){
		@rpc_PC_発狂＿ＭＧＳ１
		eval($f:rfp_PC_発狂＿ＭＧＳ１聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿借金聞いた){
		@rpc_PC_発狂＿借金
		eval($f:rfp_PC_発狂＿借金聞いた = 1)

		eval($f:rfp_PC_発狂ＣＡＬＬ１終了 = 1)

	}else {
		eval($f:rfp_PC_発狂ＣＡＬＬ１終了 = 1)
		rand 5
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 1) && ($f:rfp_PC_プレイ時間長すぎるよ) ){
			@rpc_PC_発狂＿プレイ時間

		}else if($w:rfp_rand < 2) {
			@rpc_PC_発狂＿ローズの浮気

		}else if( ($w:rfp_rand < 3) && ($f:rfp_PC_殺し過ぎです) ){
			@rpc_PC_発狂＿殺しすぎ

		}else if($w:rfp_rand < 4) {
			@rpc_PC_発狂＿ＭＧＳ１

		}else{
			@rpc_PC_発狂＿借金
		}
	}
}



proc rpc_発狂ＣＡＬＬ２ {
	if(!$f:rfp_PC_発狂＿大佐出撃聞いた){
		@rpc_PC_発狂＿大佐出撃
		eval($f:rfp_PC_発狂＿大佐出撃聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ＶＲ１聞いた){
		@rpc_PC_発狂＿ＶＲ１
		eval($f:rfp_PC_発狂＿ＶＲ１聞いた = 1)

	//ゲームオーバー回数ＸＸＸＸＸＸＸＸＸＸＸＸ
	}else if( ($f:rfp_PC_ゲームオーバーなりすぎ) && (!$f:rfp_PC_発狂＿ゲームオーバー回数聞いた) ){
		@rpc_PC_発狂＿ゲームオーバー回数
		eval($f:rfp_PC_発狂＿ゲームオーバー回数聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿電波聞いた){
		@rpc_PC_発狂＿電波
		eval($f:rfp_PC_発狂＿電波聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ばべる１聞いた){
		@rpc_PC_発狂＿ばべる１
		eval($f:rfp_PC_発狂＿ばべる１聞いた = 1)

		eval($f:rfp_PC_発狂ＣＡＬＬ２終了 = 1)

	}else {
		eval($f:rfp_PC_発狂ＣＡＬＬ２終了 = 1)
		rand 5
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rpc_PC_発狂＿大佐出撃

		}else if($w:rfp_rand < 2){
			@rpc_PC_発狂＿ＶＲ１
			eval($f:rfp_PC_発狂＿ＶＲ１聞いた = 1)

		//ゲームオーバー回数ＸＸＸＸＸＸＸＸＸＸＸＸ
		}else if( ($w:rfp_rand < 3) && ($f:rfp_PC_ゲームオーバーなりすぎ) ){
			@rpc_PC_発狂＿ゲームオーバー回数

		}else if($w:rfp_rand < 4){
			@rpc_PC_発狂＿電波

		}else {
			@rpc_PC_発狂＿ばべる１
		}
	}
}


proc rpc_発狂ＣＡＬＬ３ {
	if(!$f:rfp_PC_発狂＿ＭＳＸ１聞いた){
		@rpc_PC_発狂＿ＭＳＸ１
		eval($f:rfp_PC_発狂＿ＭＳＸ１聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿能勢電聞いた){
		@rpc_PC_発狂＿能勢電
		eval($f:rfp_PC_発狂＿能勢電聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿まめ知識聞いた){
		@rpc_PC_発狂＿まめ知識
		eval($f:rfp_PC_発狂＿まめ知識聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿前世聞いた){
		@rpc_PC_発狂＿前世
		eval($f:rfp_PC_発狂＿前世聞いた = 1)

	}else if(!$f:rfp_PC_発狂＿ＭＧＳ２聞いた){
		@rpc_PC_発狂＿ＭＧＳ２
		eval($f:rfp_PC_発狂＿ＭＧＳ２聞いた = 1)

		eval($f:rfp_PC_発狂ＣＡＬＬ３終了 = 1)

	}else {
		eval($f:rfp_PC_発狂ＣＡＬＬ３終了 = 1)
		rand 5
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rpc_PC_発狂＿能勢電

		}else if($w:rfp_rand < 2){
			@rpc_PC_発狂＿ＭＳＸ１

		}else if($w:rfp_rand < 3){
			@rpc_PC_発狂＿まめ知識

		}else if($w:rfp_rand < 4){
			@rpc_PC_発狂＿前世

		}else{
			@rpc_PC_発狂＿ＭＧＳ２
		}
	}
}


proc rpc_発狂ＣＡＬＬ {
	eval($w:rfp_PC_発狂ＣＡＬＬ聞いた回数 = $w:rfp_PC_発狂ＣＡＬＬ聞いた回数 + 1)
	if($s:エリア == "w43a"){
		eval($w:rfp_w43a_PC_発狂ＣＡＬＬ聞いた回数 = $w:rfp_w43a_PC_発狂ＣＡＬＬ聞いた回数 + 1)
	}
	@rp_発狂フラグ立て処理

	rand 3
	eval($w:rfp_rand = $status)
	if($w:rfp_rand < 1){
		if(!$f:rfp_PC_発狂ＣＡＬＬ１終了){
			@rpc_発狂ＣＡＬＬ１
		}else if(!$f:rfp_PC_発狂ＣＡＬＬ２終了){
			@rpc_発狂ＣＡＬＬ２
		}else if(!$f:rfp_PC_発狂ＣＡＬＬ３終了){
			@rpc_発狂ＣＡＬＬ３
		}else {
			@rpc_発狂ＣＡＬＬ１
		}

	}else if($w:rfp_rand < 2){
		if(!$f:rfp_PC_発狂ＣＡＬＬ２終了){
			@rpc_発狂ＣＡＬＬ２
		}else if(!$f:rfp_PC_発狂ＣＡＬＬ３終了){
			@rpc_発狂ＣＡＬＬ３
		}else if(!$f:rfp_PC_発狂ＣＡＬＬ１終了){
			@rpc_発狂ＣＡＬＬ１
		}else {
			@rpc_発狂ＣＡＬＬ２
		}
	
	}else {
		if(!$f:rfp_PC_発狂ＣＡＬＬ３終了){
			@rpc_発狂ＣＡＬＬ３
		}else if(!$f:rfp_PC_発狂ＣＡＬＬ１終了){
			@rpc_発狂ＣＡＬＬ１
		}else if(!$f:rfp_PC_発狂ＣＡＬＬ２終了){
			@rpc_発狂ＣＡＬＬ２
		}else {
			@rpc_発狂ＣＡＬＬ３
		}
	}
	@rp_大佐発狂会話聞いた回数カウント
	@rp_発狂中大佐顔初期化処理
}

block codec RPC_発狂ＣＡＬＬ 14085 d:大佐無線５バグ大佐裸ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_発狂ＣＡＬＬ
	@rp_まお封じ
}




proc rpc_ソリダス戦中 {
	if($w:rfp_大佐発狂会話聞いた回数 < 10){
		eval($w:rfp_大佐発狂会話聞いた回数 = 10)
	}
	if(!$f:rfp_PC_ソリダス戦＿倒せ聞いた) {
		@rpc_PC_ソリダス戦＿倒せ
		eval($f:rfp_PC_ソリダス戦＿倒せ聞いた = 1 )
		
	}else if(!$f:rfp_PC_ソリダス戦中＿愛国説教聞いた ){
		@rpc_PC_ソリダス戦中＿愛国説教
		eval($f:rfp_PC_ソリダス戦中＿愛国説教聞いた = 1)
	
	}else {
		@rpc_PC_ソリダス戦＿倒せ
	}
}

proc rpc_PC_海底ドック＿水密扉探せ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_海底ドック＿水密扉探せ聞いた) {
		@rp_PC_海底ドック＿昇降機乗れ
		eval($f:rfp_PC_海底ドック＿水密扉探せ聞いた)
	}
	@rp_PC_海底ドック＿水密扉探せ
}

proc rpc_PC_海底ドック昇降機探せ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_海底ドック昇降機探せ
}

proc rpc_PC_海底ドックノード探せ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_海底ドックノード探せ
}

proc rpc_PC_海底ドック昇降機前ノード接続後 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_海底ドック昇降機前ノード接続後１
	@rp_ローズさんご機嫌判定
	if( (!$f:rfp_PC_海底ドック昇降機昇降機到着後聞いた) && \
		(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpcc_PC_海底ドック昇降機前ノード接続後２
		eval($f:rfp_PC_海底ドック昇降機昇降機到着後聞いた = 1)
	}
}

proc rpc_PC_海底ドック敵覚醒 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_海底ドック敵覚醒
}

proc rpc_PC_海底ドック昇降機昇降機到着後 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_海底ドック昇降機昇降機到着後
}

proc rp_w11a_敵兵さんの誰かが元気さんです {
	if( (`@rp_敵兵元気さんです 敵兵:01`) || \
		(`@rp_敵兵元気さんです 敵兵:02`) || \
		( (`@rp_敵兵元気さんです 敵兵:03`) && ($w:ゲーム設定 >= d:LEVEL_HARD ) ) ){
		return 1
	}else {
		return 0
	}
}


proc rpc_大佐デフォルト＿潜水 {
	//レーション
	command プレイヤー状態取得
	//フナムシとりつきさん
	rand 5
	eval($w:rfp_rand = $status)
	if( (`%メニューフナムシ取り付き状態取得` == 1) && \
		($w:アイテム数Ｒ[d:アイテム:レーション] > 0) && \
		((!$f:rfp_PC_フナムシ＿捨て方ズバリ聞いた) || ($w:rfp_rand < 3) ) ) {
		@rpc_PC_フナムシ＿捨て方ズバリ
		eval($f:rfp_PC_フナムシ＿捨て方ズバリ聞いた = 1)

	}else if( (100 <= $w:ライデン懸垂回数 && $w:ライデン懸垂回数 < 200) && \
				(!$f:rfp_PC_懸垂握力ＬＶ２聞いた) ){
		@rpc_PC_懸垂握力ＬＶ２

	}else if( (200 <= $w:ライデン懸垂回数 && $w:ライデン懸垂回数 < 300) && \
			(!$f:rfp_PC_懸垂握力ＬＶ３聞いた) ){
		@rpc_PC_懸垂握力ＬＶ３

	}else if( ($status & d:PFLAG_INTRUDE) && (!$f:rfp_PC_イントルード聞いた) ){
		@rpd_ライデン顔設定デフォルト
		@rpd_大佐顔設定１
		@rpd_顔表示
		@rpc_PC_イントルード
		eval($f:rfp_PC_イントルード聞いた = 1)

	}else if( ($status & d:PFLAG_INTRUDE) && (!$f:rfp_PC_イントルード＿危険性聞いた) ){
		@rpc_PC_イントルード＿危険性
		eval($f:rfp_PC_イントルード＿危険性聞いた = 1)

	}else if( ($w:アイテム == 1) && (!$f:rfp_PC_レーション聞いた) ){
		@rpc_レーション
		eval($f:rfp_PC_レーション聞いた = 1)

	}else if($w:p_story < d:ST:P002_01_S01Ａ脚底部連結ハッチ１シナリオデモ１開始 ){

		if(!$f:rfp_PC_海底ドック＿水密扉探せ聞いた) {
			@rpc_PC_海底ドック＿水密扉探せ
			eval($f:rfp_PC_海底ドック＿水密扉探せ聞いた = 1) 

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpc_PC_海底ドック＿水密扉探せ

			}else {
				@rpc_デフォルト操作説明＿潜水
			}

		//連続無線
		}else {
			@rpc_連続無線デフォルト＿潜水
		}

	}else if(d:ST:P002_02_R01Ａ脚底部連結ハッチ２無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P003_01_P01スネーク昇降機上昇１ポリゴンデモ１開始 ){

		if(!$f:rfp_PC_海底ドック昇降機探せ聞いた) {
			@rpc_PC_海底ドック昇降機探せ
			eval($f:rfp_PC_海底ドック昇降機探せ聞いた = 1 )
		
		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpc_PC_海底ドック昇降機探せ

			}else {
				@rpc_デフォルト操作説明＿潜水
			}

		//連続無線
		}else {
			@rpc_連続無線デフォルト＿潜水
		}

	}else if(d:ST:P003_02_R02スネーク昇降機上昇２無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P004_01_P01ノード初接続１ポリゴンデモ１開始 ){

		if(!$f:rfp_PC_海底ドックノード探せ聞いた) {
			@rpc_PC_海底ドックノード探せ
			eval($f:rfp_PC_海底ドックノード探せ聞いた = 1)
		
		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpc_PC_海底ドックノード探せ

			}else {
				@rpc_デフォルト操作説明＿潜水
			}

		//連続無線
		}else {
			@rpc_連続無線デフォルト＿潜水
		}


	}else if(d:ST:P004_02_R01ノード初接続２無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１開始 ){
		
		//昇降機下りてません。
		if(!$f:rfp_w11_昇降機に乗れますよ) {
			if(!$f:rfp_PC_海底ドック昇降機前ノード接続後聞いた) {
				@rpc_PC_海底ドック昇降機前ノード接続後
				eval($f:rfp_PC_海底ドック昇降機前ノード接続後聞いた = 1 )
			
			}else if( (!$f:rfp_PC_海底ドック敵覚醒聞いた) && \
						(`@rp_w11a_敵兵さんの誰かが元気さんです`) ){
				@rpc_PC_海底ドック敵覚醒
				eval($f:rfp_PC_海底ドック敵覚醒聞いた = 1)

			//無線一回目
			}else if($w:rfp_callcount <= 1){
				rand 5
				eval($w:rfp_rand = $status)

				if($w:rfp_rand < 3){
					@rpc_PC_海底ドック昇降機前ノード接続後

				}else {
					@rpc_デフォルト操作説明＿潜水
				}

			//連続無線
			}else {
				@rpc_連続無線デフォルト＿潜水
			}


		//のれますよー
		}else {
			if(!$f:rfp_PC_海底ドック昇降機昇降機到着後聞いた){
				@rpc_PC_海底ドック昇降機昇降機到着後
				eval($f:rfp_PC_海底ドック昇降機昇降機到着後聞いた = 1 )

			//無線一回目
			}else if($w:rfp_callcount <= 1){
				rand 5
				eval($w:rfp_rand = $status)

				if($w:rfp_rand < 3){
					@rpc_PC_海底ドック昇降機昇降機到着後

				}else {
					@rpc_デフォルト操作説明＿潜水
				}

			//連続無線
			}else {
				@rpc_連続無線デフォルト＿潜水
			}
		}


	}else {
		@rpc_ばぐっち
	}
}//d

proc rpc_PC_フナムシ＿捨て方ズバリ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_フナムシ＿捨て方ズバリ
}

proc rpc_大佐デフォルト {
	//フナムシとりつきさん
	rand 5
	eval($w:rfp_rand = $status)
	if( (`%メニューフナムシ取り付き状態取得` == 1) && \
		($w:アイテム数Ｒ[d:アイテム:レーション] > 0) && \
		((!$f:rfp_PC_フナムシ＿捨て方ズバリ聞いた) || ($w:rfp_rand < 3) ) ) {
		@rpc_PC_フナムシ＿捨て方ズバリ
		eval($f:rfp_PC_フナムシ＿捨て方ズバリ聞いた = 1)
		
	//レーション
	}else if( ($w:アイテム == 1) && (!$f:rfp_PC_レーション聞いた) &&  \
			($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) ){
		@rpc_レーション
		eval($f:rfp_PC_レーション聞いた = 1)

	}else if( (100 <= $w:ライデン懸垂回数 && $w:ライデン懸垂回数 < 200) && \
				(!$f:rfp_PC_懸垂握力ＬＶ２聞いた) ){
		@rpc_PC_懸垂握力ＬＶ２

	}else if( (200 <= $w:ライデン懸垂回数 && $w:ライデン懸垂回数 < 300) && \
			(!$f:rfp_PC_懸垂握力ＬＶ３聞いた) ){
		@rpc_PC_懸垂握力ＬＶ３

	}else if(d:ST:P005_05_R02ライデン昇降機上昇５無線デモ２終了 <= $w:p_story && \
			$w:p_story < d:ST:Ａ脚出たところでＳＥＡＬＳ無線傍受終了 ){
		if(!$f:rf_PC_Ａ脚Ｂ脚に行け聞いた){
			@rpc_PC_Ａ脚Ｂ脚に行け
			eval($f:rf_PC_Ａ脚Ｂ脚に行け聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpc_PC_Ａ脚Ｂ脚に行け

			}else {
				@rpc_デフォルト操作説明
			}

		//連続無線
		}else {
			@rpc_連続無線デフォルト
		}
		

	}else if(d:ST:Ａ脚出たところでＳＥＡＬＳ無線傍受終了 <= $w:p_story && \
			$w:p_story < d:ST:P010_01_P01ヴァンプ遭遇１ポリゴンデモ１開始 ){

		if(!$f:rfp_PC_Ｂ脚に急げ聞いた){
			@rpc_PC_Ｂ脚に急げ
			eval($f:rfp_PC_Ｂ脚に急げ聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpc_PC_Ｂ脚に急げ

			}else {
				@rpc_デフォルト操作説明
			}

		//連続無線
		}else {
			@rpc_連続無線デフォルト
		}
		
	}else if(d:ST:P010_10_P05ヴァンプ遭遇１０ポリゴンデモ５終了 <= $w:p_story && \
			$w:p_story < d:ST:P012_01_P01フォーチュン遭遇１ポリゴンデモ１開始 ){

		if(!$f:rfp_PC_ＢＣ連絡橋中央部へ行け聞いた ) {
			@rpc_PC_ＢＣ連絡橋中央部へ行け
			eval($f:rfp_PC_ＢＣ連絡橋中央部へ行け聞いた = 1)

		}else if( ($f:rfp_眠りプリスキン撃ちやがった) && ($s:エリア == "w14a") && \
					(!$f:rfp_PC_眠りプリスキン撃った聞いた) ){
			@rpc_PC_眠りプリスキン撃った
			eval($f:rfp_PC_眠りプリスキン撃った聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpc_PC_ＢＣ連絡橋中央部へ行け

			}else {
				@rpc_デフォルト操作説明
			}

		//連続無線
		}else {
			@rpc_連続無線デフォルト
			//ＸＸＸＸＸＸＸＸＸＸ
			//PC_銃入手注意
		}

	}else if(d:ST:P012_02_R01フォーチュン遭遇２無線デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始 ){

		if(!$f:rfp_PC_ピーター探せ聞いた) {
			@rpc_PC_ピーター探せ
			eval($f:rfp_PC_ピーター探せ聞いた = 1)
		
		}else if( ($f:rfp_眠りプリスキン撃ちやがった) && ($s:エリア == "w14a") && \
					(!$f:rfp_PC_眠りプリスキン撃った聞いた) ){
			@rpc_PC_眠りプリスキン撃った
			eval($f:rfp_PC_眠りプリスキン撃った聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpc_PC_ピーター探せ

			}else {
				@rpc_デフォルト操作説明
			}

		//連続無線
		}else {
			@rpc_連続無線デフォルト
			//ＸＸＸＸＸＸＸＸＸＸ
			//PC_銃入手注意
		}

	}else if(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
			$w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了 ){
		if(!$f:rfp_PC_爆弾解体中ピーターいなくなる前聞いた) {
			@rpc_爆弾解体中ピーターいなくなる前
			eval($f:rfp_PC_爆弾解体中ピーターいなくなる前聞いた = 1)

		//無線一回目
		}else if( ($w:rfp_callcount <= 1) || (`prefreq` != 14085) ){
			@rpc_爆弾解体中ピーターいなくなる前

		//連続無線
		}else {
			rand 4
			eval($w:rfp_rand = $status)

			if( ($w:rfp_rand < 1) || ( ($w:rfp_callcount >= 4) && ($w:rfp_rand < 3) ) ){
				@rpc_連続無線デフォルト
			}else {
				@rpc_爆弾解体中ピーターいなくなる前
			}
		}

	}else if(d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１開始){

		if(!$f:rfp_PC_最後の爆弾処理せよ聞いた){
			@rpc_PC_最後の爆弾処理せよ
			eval($f:rfp_PC_最後の爆弾処理せよ聞いた = 1)
		
		//無線一回目
		}else if( ($w:rfp_callcount <= 1) || (`prefreq` != 14085) ){
				@rpc_PC_最後の爆弾処理せよ

		//連続無線
		}else {
			if( ($w:rfp_rand < 1) || ( ($w:rfp_callcount >= 4) && ($w:rfp_rand < 3) ) ){
				@rpc_連続無線デフォルト
			}else {
				@rpc_PC_最後の爆弾処理せよ
			}
		}

	}else if( 	d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了 <= $w:p_story  && \
				$w:p_story <  d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１開始 ){

			//センサーＢない＝通常
			if($w:アイテム数Ｒ[11] <= 0){
				if(!$f:rfp_PC_センサーＢ取りに行け聞いた){
					@rpc_PC_センサーＢ取りに行け
					eval($f:rfp_PC_センサーＢ取りに行け聞いた = 1)

				//無線一回目
				}else if($w:rfp_callcount <= 1){
					rand 5
					eval($w:rfp_rand = $status)

					if($w:rfp_rand < 4){
						@rpc_PC_センサーＢ取りに行け

					}else {
						@rpc_デフォルト操作説明
					}

				//連続無線
				}else {
					@rpc_連続無線デフォルト
					//ＸＸＸＸＸＸＸＸＸＸ
					//PC_銃入手注意
				}

		//最後の爆弾を解体した時にセンサーＢを持っている場合！激ヤバ！！ＸＸＸＸＸＸＸＸＸＸｘ
		}else {
			if(!$f:rfp_PC_爆弾解体中ピーターいなくなる前聞いた) {
				@rpc_爆弾解体中ピーターいなくなる前
				eval($f:rfp_PC_爆弾解体中ピーターいなくなる前聞いた = 1)

			//無線一回目
			}else if($w:rfp_callcount <= 1){
				rand 5
				eval($w:rfp_rand = $status)

				if($w:rfp_rand < 4){
					@rpc_爆弾解体中ピーターいなくなる前

				}else {
					@rpc_デフォルト操作説明
				}

			//連続無線
			}else {
				@rpc_連続無線デフォルト
				//ＸＸＸＸＸＸＸＸＸＸ
				//PC_銃入手注意
			}
		}


	}else if(d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P026_01_R01爆弾解体終了１無線デモ１開始){

		if(!$f:rfp_PC_Ａ脚底部急げ聞いた) {
			@rpc_PC_Ａ脚底部急げ
			eval($f:rfp_PC_Ａ脚底部急げ聞いた = 1)
		
		//無線一回目
		}else if($w:rfp_callcount <= 1){
			@rpc_PC_Ａ脚底部急げ

		//連続無線
		}else {
			@rpc_連続無線デフォルト
			//ＸＸＸＸＸＸＸＸＸＸ
			//PC_銃入手注意
		}

/*
	}else if(d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P026_01_R01爆弾解体終了１無線デモ１開始){
		if(!$f:rfp_PC_ピーター死１聞いた) {
			@rpc_PC_ピーター死１
			eval($f:rfp_PC_ピーター死１聞いた = 1)

		}else if(!$f:rfp_PC_無臭爆弾＿Ａ脚底部聞いた) {
			@rpc_PC_無臭爆弾＿Ａ脚底部
			eval($f:rfp_PC_無臭爆弾＿Ａ脚底部聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 4){
				@rpc_PC_無臭爆弾＿Ａ脚底部
			}else {
				@rpc_デフォルト操作説明
			}

		//連続無線
		}else {
			@rpc_連続無線デフォルト
			//ＸＸＸＸＸＸＸＸＸＸ
			//PC_銃入手注意
		}

	}else if(d:ST:P026_01_R01爆弾解体終了１無線デモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P028_01_P01爆弾解体後フォーチュン再登場１ポリゴンデモ１開始){
		if(!$f:rfp_PC_爆弾解体後＿昇降機上がれ聞いた){
			@rpc_PC_爆弾解体後＿昇降機上がれ
			eval($f:rfp_PC_爆弾解体後＿昇降機上がれ聞いた = 1)

		//無線一回目
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if( ($w:rfp_rand < 4) && ($w:rfp_callcount <= 1) ){
				@rpc_PC_爆弾解体後＿昇降機上がれ

			//連続無線
			}else {
				@rpc_連続無線デフォルト
			}
		}
*/
/*
	}else if(d:ST:P029_01フォーチュン戦開始 <= $w:p_story && \
			$w:p_story <  d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１開始){
		@rpc_フォーチュン戦中
*/

	}else if(d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始){
		if(!$f:rfp_PC_ファットマン戦前聞いた) {
			@rpc_PC_ファットマン戦前
			eval($f:rfp_PC_ファットマン戦前聞いた = 1)

		//無線一回目
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if( ($w:rfp_rand < 3) && ($w:rfp_callcount <= 1) ){
				@rpc_PC_ファットマン戦前

			}else {
				@rpc_連続無線デフォルト
				//ＸＸＸＸＸＸＸＸＸＸ
				//PC_銃入手注意
			}
		}

/*
	}else if(d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１開始){

		if( (!$f:rfp_ファットマン死んだ) && ($f:rfp_ファットマン爆弾生きてる) ){
			@rpc_PC_ファットマン戦＿死後爆弾残り
		}else {
			@rpc_ファットマン戦中
		}
	}else if(d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１開始){

		@rpc_ファットマン死後爆弾解体中

	}else if(d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P036_01_P01忍者登場１ポリゴンデモ１開始){
		if(!$f:rfp_PC_忍者登場前＿ヘリポート聞いた) {
			@rpc_PC_忍者登場前＿ヘリポート
			eval($f:rfp_PC_忍者登場前＿ヘリポート聞いた = 1)
		
		}else if( ($f:rfp_PC_ファットマン戦後＿最終爆弾ズバリ聞いた) && \
					(!$f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:忍者登場前＿ローズ助けられ]) ) {
			@rpc_PC_忍者登場前＿ローズ助けられ
			eval($f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:忍者登場前＿ローズ助けられ] = 1)
		
		//無線一回目
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if( ($w:rfp_rand < 3) && ($w:rfp_callcount <= 1) ){
				@rpc_PC_忍者登場前＿ヘリポート
			}else {
				@rpc_連続無線デフォルト
				//ＸＸＸＸＸＸＸＸＸＸ
				//PC_銃入手注意
			}
		}
*/

	}else if(d:ST:P036_13_R04忍者登場１３無線デモ４終了 <= $w:p_story && \
			$w:p_story <  d:ST:P040_01_P01エイムズ発見１ポリゴンデモ１開始){
//		@rpc_人質イベント中
		if($s:エリア == "w24a") {
			@rpc_人質イベント中央棟１ＦＢ２

		}else if($s:エリア == "w24b") {
			@rpc_人質イベント中央棟Ｂ１

		}else if($s:エリア == "w24c") {
			@rpc_人質イベント人質部屋

		}else if( ($s:エリア == "w24d") || ($s:エリア == "w24e") ) {
			@rpc_人質イベント中央棟１ＦＢ２
		
		}else {
			@rpc_人質イベント中央棟以外
		}

	}else if(d:ST:P040_07_P04エイムズ発見７ポリゴンデモ４終了 <= $w:p_story && \
			$w:p_story <  d:ST:P040_09_P05エイムズ発見９ポリゴンデモ５開始){
		@rpc_PC_オセロットがやってくる

	}else if(d:ST:P040_09_P05エイムズ発見９ポリゴンデモ５終了 <= $w:p_story && \
			$w:p_story <  d:ST:P042_01_R01シェル１シェル２連絡橋１無線デモ１開始){

		if(!$f:rfp_PC_エイムズ死後シェル２向かえ聞いた) {
			@rpc_エイムズ死後シェル２向かえ
			eval($f:rfp_PC_エイムズ死後シェル２向かえ聞いた = 1)

		}else if(!$f:rfp_PC_エイムズ死後＿変装無効１聞いた) {
			@rpc_PC_エイムズ死後＿変装無効１
			eval($f:rfp_PC_エイムズ死後＿変装無効１聞いた = 1)

		}else if(!$f:rfp_PC_エイムズ死後＿大統領聞いた) {
			@rpc_PC_エイムズ死後＿大統領
			eval($f:rfp_PC_エイムズ死後＿大統領聞いた = 1)

		//無線一回目
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if( ($w:rfp_rand < 3) && ($w:rfp_callcount <= 1) ){
				@rpc_エイムズ死後シェル２向かえ

			}else if(($w:rfp_rand < 4) && ($w:rfp_callcount <= 1) ){
				@rpc_PC_エイムズ死後＿変装無効１

			}else {
				@rpc_連続無線デフォルト
			}
		}

	}else if(d:ST:P042_01_R01シェル１シェル２連絡橋１無線デモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:シェル１シェル２連絡橋赤外線センサークリア){

//		if($s:エリア == "w25a" ) 
//			@rpc_狙撃イベント＿シェル１シェル２連絡橋

		if(!$f:rfp_PC_狙撃イベント＿シェル１シェル２連絡橋以外聞いた){
			@rpc_狙撃イベント＿シェル１シェル２連絡橋以外
			eval($f:rfp_PC_狙撃イベント＿シェル１シェル２連絡橋以外聞いた = 1)
			
		//無線一回目
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if( ($w:rfp_rand < 4) && ($w:rfp_callcount <= 1) ){
				@rpc_狙撃イベント＿シェル１シェル２連絡橋以外

			}else {
				@rpc_連続無線デフォルト
			}
		}
	}else if(d:ST:シェル１シェル２連絡橋赤外線センサークリア <= $w:p_story && \
			$w:p_story <  d:ST:P045_01_P01ハリアー登場１ポリゴンデモ１開始 ){

		if(!$f:rfp_PC_センサー破壊後＿シェル２行け聞いた) {
			@rpc_PC_センサー破壊後＿シェル２行け
			eval($f:rfp_PC_センサー破壊後＿シェル２行け聞いた = 1 )

		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if( ($w:rfp_rand < 4)  && ($w:rfp_callcount <= 1) ){
				@rpc_PC_センサー破壊後＿シェル２行け

			}else {
				@rpc_連続無線デフォルト
			}
		}

/*
	}else if(d:ST:P045_01_P01ハリアー登場１ポリゴンデモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P046_01_P01ハリアー戦勝利１ポリゴンデモ１開始 ){
		@rpc_ハリアー戦中
*/

	}else if(d:ST:P046_02_R01ハリアー戦勝利２無線デモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P047_01_P01電撃床前オルガ１ポリゴンデモ１開始 ){

		if(!$f:rfp_PC_ハリアー戦後＿大統領助けろ聞いた) {
			@rpc_PC_ハリアー戦後＿大統領助けろ
			eval($f:rfp_PC_ハリアー戦後＿大統領助けろ聞いた = 1)

		}else if( (!$f:rfp_PC_ハリアー戦後＿シェル２中央棟への道聞いた) && \
					($s:エリア != "w25c") ){
			@rpc_PC_ハリアー戦後＿シェル２中央棟への道
			eval($f:rfp_PC_ハリアー戦後＿シェル２中央棟への道聞いた  = 1)

		}else if( ($s:エリア == "w25b" ) && ($s:rfp_w25b_いまいるところ == "DG_D") && \
					(!$f:rfp_PC_ＤＧ連絡橋渡るが良い聞いた) ){
			@rpc_PC_ＤＧ連絡橋渡るが良い
			eval($f:rfp_PC_ＤＧ連絡橋渡るが良い聞いた = 1 )

		}else if( ($s:エリア == "w25b" ) && ($s:rfp_w25b_いまいるところ == "DG_G") && \
					(!$f:rfp_PC_ＤＧ連絡橋渡るが良い聞いた) ){
			@rpc_PC_ＤＧ連絡橋渡るが良い
			eval($f:rfp_PC_ＤＧ連絡橋渡るが良い聞いた = 1 )

		}else if( ($s:エリア == "w25b" ) && ($s:rfp_w25b_いまいるところ == "DG_G_pipe") && \
					(!$f:rfp_PC_ＤＧ連絡橋＿パイプ上落ちるな聞いた) ){
			@rpc_PC_ＤＧ連絡橋＿渡った後＿パイプ
			eval($f:rfp_PC_ＤＧ連絡橋＿パイプ上落ちるな聞いた = 1)

		}else if( ($s:エリア == "w25b" ) && ($s:rfp_w25b_いまいるところ == "G") && \
					(!$f:rfp_PC_ＤＧ連絡橋＿渡れ聞いた) ){
			@rpc_PC_Ｇ脚＿渡れ
			eval($f:rfp_PC_ＤＧ連絡橋＿渡れ聞いた = 1 )

		}else if( ($s:エリア == "w25b" ) && ($s:rfp_w25b_いまいるところ == "G_LG") && \
					(!$f:rfp_PC_ＬＧ連絡橋＿飛び降り聞いた) ){
			@rpc_PC_Ｇ脚＿ＬＧ連絡橋側
			eval($f:rfp_PC_ＬＧ連絡橋＿飛び降り聞いた = 1)

		}else if( ($s:エリア == "w25b" ) && ($s:rfp_w25b_いまいるところ == "LG") && \
					(!$f:rfp_PC_ＬＧ連絡橋聞いた) ){
			@rpc_PC_ＬＧ連絡橋

		}else if( ($s:エリア == "w25c" ) && ($s:rfp_w25c_いまいるところ == "L") && \
					($w:プレイヤーフラグ & d:PL_SHOWERED) && \
					(!$f:rfp_PC_Ｌ脚外周＿放尿兵１聞いた) ){
			@rpc_PC_Ｌ脚外周＿放尿兵１
			eval($f:rfp_PC_Ｌ脚外周＿放尿兵１聞いた = 1)

		}else if( ($s:エリア == "w25c" ) && ($s:rfp_w25c_いまいるところ == "L") && \
					(!$f:rfp_PC_Ｌ脚＿渡れ聞いた) ){
			@rpc_PC_Ｌ脚＿渡れ
			eval($f:rfp_PC_Ｌ脚＿渡れ聞いた = 1)

		}else if( ($s:エリア == "w25c" ) && ($s:rfp_w25c_いまいるところ == "KL") && \
					(!$f:rfp_PC_ＫＬ連絡橋＿渡れ聞いた) ){
			@rpc_PC_ＫＬ連絡橋＿渡れ
			eval($f:rfp_PC_ＫＬ連絡橋＿渡れ聞いた = 1)

		}else if( (!$f:rfp_PC_ＤＧ連絡橋＿レーダーが使えない聞いた) && \
					($w:ゲーム設定 >= d:LEVEL_EASY) ){
			@rpc_PC_ＤＧ連絡橋＿レーダーが使えない
			eval($f:rfp_PC_ＤＧ連絡橋＿レーダーが使えない聞いた = 1)


		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 4){
				if( ($s:エリア == "w25b" ) && ($s:rfp_w25b_いまいるところ == "DG_D")  ){
					@rpc_PC_ＤＧ連絡橋渡るが良い

				}else if( ($s:エリア == "w25b" ) && ($s:rfp_w25b_いまいるところ == "DG_G")  ){
					@rpc_PC_ＤＧ連絡橋＿渡った後

				}else if( ($s:エリア == "w25b" ) && ($s:rfp_w25b_いまいるところ == "DG_G_pipe")  ){
					@rpc_PC_ＤＧ連絡橋＿渡った後＿パイプ

				}else if( ($s:エリア == "w25b" ) && ($s:rfp_w25b_いまいるところ == "G")  ){
					@rpc_PC_Ｇ脚＿渡れ

				}else if( ($s:エリア == "w25b" ) && ($s:rfp_w25b_いまいるところ == "G_LG")  ){
					@rpc_PC_Ｇ脚＿ＬＧ連絡橋側

				}else if( ($s:エリア == "w25b" ) && ($s:rfp_w25b_いまいるところ == "LG") ){
					@rpc_PC_ＬＧ連絡橋

				}else if( ($s:エリア == "w25c" ) && ($s:rfp_w25c_いまいるところ == "L") ){
					@rpc_PC_Ｌ脚＿渡れ

				}else if( ($s:エリア == "w25c" ) && ($s:rfp_w25c_いまいるところ == "KL") ){
					@rpc_PC_ＫＬ連絡橋＿渡れ

				//もうそれ以外
				}else {
					@rpc_PC_ハリアー戦後＿大統領助けろ
				}
			}else{
				@rpc_デフォルト操作説明
			}
	
		//連続無線
		}else {
			@rpc_連続無線デフォルト
		}


	}else if(d:ST:P047_05_R01電撃床前オルガ５無線デモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１開始 ){

		if(!$f:rfp_PC_リモコンミサイルで壊せ聞いた){
			@rpc_リモコンミサイルで壊せ
			eval($f:rfp_PC_リモコンミサイルで壊せ聞いた = 1)

		}else if( ($s:エリア != "w31a") && ($w:武器弾数Ｒ[6] >= 0) && \
					(!$f:rft_PC_リモコンミサイル＿入手後聞いた) ){
			@rpc_PC_リモコンミサイル＿入手後
			eval($f:rft_PC_リモコンミサイル＿入手後聞いた = 1 )


		//リモコンあるる＝イベント中！
		}else if( ($w:武器弾数Ｒ[6] >= 0) && \
					($s:エリア == "w31a") && \
					(!$f:rfp_PC_リモコンミサイル＿大統領に当てるな聞いた) ){
			@rpc_PC_リモコンミサイル＿大統領に当てるな
			eval($f:rfp_PC_リモコンミサイル＿大統領に当てるな聞いた = 1 )

		
		}else if( ($w:武器弾数Ｒ[6] >= 0) && \
					($s:エリア == "w31a") && \
					($f:rfp_31a_暗い部屋にいまそかり) && \
					(! ($b:w31a_破壊フラグ[22] && $b:w31a_破壊フラグ[23] ) ) && \
					(!$f:rfp_PC_リモコンミサイル＿ガンカメラ聞いた) ){
			@rpc_PC_リモコンミサイル＿ガンカメラ
			eval($f:rfp_PC_リモコンミサイル＿ガンカメラ聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 4){
				//イベント中
				if($w:武器弾数Ｒ[6] >= 0) {
					if( $s:エリア == "w31a") {
						@rpc_リモコンミサイルイベントランダム
					}else {
						@rpc_PC_リモコンミサイル＿入手後
					}

				//みっそーありませぬ。
				}else {
					@rpc_リモコンミサイルで壊せ
				}
			}else {
				@rpc_デフォルト操作説明
			}

		//連続無線
		}else {
			@rpc_連続無線デフォルト
		}


	}else if(d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P049_01_P01大統領１ポリゴンデモ１開始 ){

		//ヤバ！ＸＸＸＸＸＸＸＸＸ
		if(!$f:rfp_PC_パネル破壊後大統領に会え聞いた) {
			@rpc_PC_電撃床破壊後大統領助けろ
			eval($f:rfp_PC_パネル破壊後大統領に会え聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 4){
				@rpc_PC_電撃床破壊後大統領助けろ
			}else {
				@rpc_デフォルト操作説明
			}

		//連続無線
		}else {
			@rpc_連続無線デフォルト
		}


	}else if(d:ST:P049_12_R05大統領１２無線機デモ５終了 <= $w:p_story && \
			$w:p_story <  d:ST:P055_01_P01ヴァンプ戦前１ポリゴンデモ１開始 ){

		if(!$f:rfp_PC_エマ探せ聞いた) {
			@rpc_PC_エマ探せ
			eval($f:rfp_PC_エマ探せ聞いた = 1 )

		}else if( (!$f:rfp_P051_01_R01_エマとオタコン１無線機デモ１聞いた) && \
					(	(`%プレイヤー状態取得` & d:PFLAG_WATER_SURFACE) || \
						(!(`%プレイヤー状態取得` & d:PFLAG_WATER) ) ) ){
			@rpc_P051_01_R01_エマとオタコン１無線機デモ１
			eval($f:rfp_P051_01_R01_エマとオタコン１無線機デモ１聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 4){
				@rpc_PC_エマ探せ
			}else {
				@rpc_デフォルト操作説明
			}

		//連続無線
		}else {
			@rpc_連続無線デフォルト
		}

/*
	}else if(d:ST:P055_04_P03ヴァンプ戦前４ポリゴンデモ３終了 <= $w:p_story && \
			$w:p_story <  d:ST:P057_01_P01ヴァンプ戦終了１ポリゴンデモ１開始 ){
		@rpc_ヴァンプ戦中
*/

	}else if(d:ST:P057_02_R01ヴァンプ戦終了２無線機デモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P058_01_P01エマ救出１ポリゴンデモ１開始 ){
/*
		if( ( ($s:エリア == "w31c") || ($s:エリア == "w31f") ) && ($f:rfp_w31c_エマ部屋にいまそかり) && \
			(!$f:rfp_PC_ヴァンプ戦後＿ロッカー室聞いた) ) {
				@rpc_PC_ヴァンプ戦後＿ロッカー室
				eval($f:rfp_PC_ヴァンプ戦後＿ロッカー室聞いた = 1)
*/
		if( ( ($s:エリア == "w31c") || ($s:エリア == "w31f")) && \
					(!$f:rfp_w31c_エマ部屋にいまそかり) && \
					(!$f:rfp_PC_ヴァンプ戦後＿エマ助けろ第二濾過槽聞いた) ){
			@rpc_PC_ヴァンプ戦後＿エマ助けろ第二濾過槽
			eval($f:rfp_PC_ヴァンプ戦後＿エマ助けろ第二濾過槽聞いた = 1)
			
		}else if( ( ($s:エリア != "w31c") || ($s:エリア == "w31f")) && \
					(!$f:rfp_PC_ヴァンプ戦後＿エマ助けろ第二濾過槽以南聞いた) ){
			@rpc_PC_ヴァンプ戦後＿エマ助けろ第二濾過槽以南
			eval($f:rfp_PC_ヴァンプ戦後＿エマ助けろ第二濾過槽以南聞いた = 1 )
		
		}else if( (!$f:rfp_P051_01_R01_エマとオタコン１無線機デモ１聞いた) && \
					(	(`%プレイヤー状態取得` & d:PFLAG_WATER_SURFACE) || \
						(!(`%プレイヤー状態取得` & d:PFLAG_WATER) ) ) ){
			@rpc_P051_01_R01_エマとオタコン１無線機デモ１
			eval($f:rfp_P051_01_R01_エマとオタコン１無線機デモ１聞いた = 1)

		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if( ($w:rfp_rand < 4) && ($w:rfp_callcount <= 1) ){
				if( ($s:エリア == "w31c") || ($s:エリア == "w31f") ){
					@rpc_PC_ヴァンプ戦後＿エマ助けろ第二濾過槽

				}else {
					@rpc_PC_ヴァンプ戦後＿エマ助けろ第二濾過槽以南
				}
			}else {
				@rpc_連続無線デフォルト
			}
		}

	}else if(d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了 <= $w:p_story && \
				$w:p_story <  d:ST:P059_01_P01エマ休憩１ポリゴンデモ１開始 ){
		if(!$f:エマ存在フラグ){
			@rpc_PC_エマ救出後＿エマおいてけぼり

		}else if(!$f:rfp_PC_エマ連れてけ聞いた){
			@rpc_PC_エマ連れてけ
			@rp_PC_エマ救出後＿第二濾過槽戻れ
			eval($f:rfp_PC_エマ連れてけ聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			@rpc_PC_エマ連れてけ
			@rp_PC_エマ救出後＿第二濾過槽戻れ

		//連続無線
		}else {
			@rpc_連続無線デフォルト
		}

	}else if(d:ST:P059_03_R01エマ休憩３無線機デモ１終了 <= $w:p_story && \
				$w:p_story <  d:ST:P065_01_P01エマＬ脚１ポリゴンデモ１開始 ){

		if( (!$f:エマ存在フラグ) || ($s:エリア == "w25c") ){
			@rpc_PC_エマ救出後＿エマおいてけぼり

		}else if( ($s:エリア == "w25d") && (!$f:rfp_PC_ＫＬ連絡橋＿エマ後Ｌ脚＿行け聞いた) ) {
			@rpc_PC_ＫＬ連絡橋＿エマ後Ｌ脚＿行け
			eval($f:rfp_PC_ＫＬ連絡橋＿エマ後Ｌ脚＿行け聞いた = 1)

		}else if( ($s:エリア == "w28a") && (!$f:rfp_PC_Ｌ脚＿南行け聞いた) ) {
			@rpc_PC_Ｌ脚＿南行け
			eval($f:rfp_PC_Ｌ脚＿南行け聞いた = 1)

		}else if( (!$f:rfp_PC_エマ救出後＿Ｌ脚向かえ聞いた) && \
					( ($s:エリア != "w25d") && ($s:エリア != "w28a") ) ){
			@rpc_PC_エマ救出後＿Ｌ脚向かえ
			eval($f:rfp_PC_エマ救出後＿Ｌ脚向かえ聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 6
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				if($s:エリア == "w25d") {
					@rpc_PC_ＫＬ連絡橋＿エマ後Ｌ脚＿行け
				}else if($s:エリア == "w28a"){
					@rpc_PC_Ｌ脚＿南行け
				}else {
					@rpc_PC_エマ救出後＿Ｌ脚向かえ
				}

			}else if($w:rfp_rand < 4){
				if($i:rfp_プレイヤー状態 & d:PFLAG_WATER){
					@rpc_エマ水中
				}else {
					@rpc_エマ手つなぎ
				}

			}else if($w:rfp_rand < 5){
				@rpc_PC_エマ救出後＿Ｌ脚行け

			}else {
				@rpc_デフォルト操作説明
			}

		//連続無線
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				if($i:rfp_プレイヤー状態 & d:PFLAG_WATER){
					@rpc_エマ水中
				}else {
					@rpc_エマ手つなぎ
				}
			}else{
				@rpc_連続無線デフォルト
			}
		}

/*
	}else if(d:ST:P065_06_R02Ｌ脚エマ６無線機デモ２終了 <= $w:p_story && \
				$w:p_story <  d:ST:P068_01_P01ヴァンプ狙撃前１ポリゴンデモ１開始 ){
		@rpc_エマ狙撃中

	}else if(d:ST:P068_01_P01ヴァンプ狙撃前１ポリゴンデモ１終了 <= $w:p_story && \
				$w:p_story <  d:ST:P069_01_P01ヴァンプ狙撃終了１ポリゴンデモ１開始 ){
		@rpc_ヴァンプ狙撃中
*/

	}else if(d:ST:P069_04_R02ヴァンプ狙撃終了４無線機デモ２終了 <= $w:p_story && \
				$w:p_story <  d:ST:P070_01_P01ＡＧ起動１ポリゴンデモ１開始){

		if(!$f:rfp_PC_エマ狙撃後＿電算室行け聞いた) {
			@rpc_PC_エマ狙撃後＿電算室行け
			eval($f:rfp_PC_エマ狙撃後＿電算室行け聞いた = 1)
	
		//無線一回目
		}else if($w:rfp_callcount <= 1){
				@rpc_PC_エマ狙撃後＿電算室行け

		//連続無線
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 3){
				@rpc_PC_エマ狙撃後＿電算室行け
			}else{
				@rpc_連続無線デフォルト
			}
		}

/*
	//以下裸状態ＤＥＡＴＨ！切り分け？ＸＸＸＸＸＸＸＸＸＸＸＸ {
	}else if(d:ST:P070_11_M02ＡＧ起動１１ムービーデモ２終了 <= $w:p_story && \
				$w:p_story <  d:ST:P070_23_R04ＡＧ起動２３無線機デモ４開始){
			@rpc_PC_拘束解放前

	}else if( d:ST:P070_23_R04ＡＧ起動２３無線機デモ４終了 <= $w:p_story && \
				$w:p_story <  d:ST:P072_01_R01大佐混乱１無線機デモ１開始){

		if( ($s:エリア == "w41a") && ($f:rfp_w41a_拷問部屋にいるのＤＥＡＴＨ) && \
				( 	(!$f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋聞いた) || \
					(( ($w:アラートモード == d:ALERT_MODE_AVOID) || ($w:アラートモード == d:ALERT_MODE_SEARCH) ) && \
					 (!$f:w41a_捕まったふりバレたフラグ) && \
					 (!$f:rfp_PC_拷問部屋＿回避モード状態聞いた) ) ) ){
			@rpc_PC_拘束解除後＿合流しろ＿拷問部屋
			eval($f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋聞いた = 1)

		}else if( ($s:エリア == "w41a") && (!$f:rfp_w41a_拷問部屋にいるのＤＥＡＴＨ) && \
					(!$f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋外聞いた) ) {
				@rpc_PC_拘束解除後＿合流しろ＿拷問部屋外
				eval($f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋外聞いた = 1)

		}else if( ($s:エリア == "w42a") && (!$f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋以外聞いた) ){
				@rpc_PC_拘束解除後＿合流しろ＿拷問部屋以外
				eval($f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋以外聞いた = 1 )

		}else if(!$f:rfp_PC_裸注意終了) {
			@rpc_裸注意

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			@rpc_裸デフォルト
		
		//連続無線
		}else {
			@rpc_裸連続デフォルト
		}


	}else if( d:ST:P072_01_R01大佐混乱１無線機デモ１終了 <= $w:p_story && \
				$w:p_story <  d:ST:P080_01_P01ＡＧ浮上１ポリゴンデモ１開始){
		@rpc_発狂中


	}else if( d:ST:P080_28_P12ＡＧ浮上２８ポリゴンデモ１２終了 <= $w:p_story && \
				$w:p_story <  d:ST:P082_01_P01エンディング１ポリゴンデモ１開始){

		@rpc_ソリダス戦中
*/
//////////////////////////}

	//ばぐっちです。
	}else {
//		@rpc_ばぐっち
	}
}


proc rpc_大佐デフォルト＿発狂後 {
	if( d:ST:P072_01_R01大佐混乱１無線機デモ１終了 <= $w:p_story && \
				$w:p_story <  d:ST:P080_01_P01ＡＧ浮上１ポリゴンデモ１開始){
		@rpc_発狂中


	}else if( d:ST:P080_28_P12ＡＧ浮上２８ポリゴンデモ１２終了 <= $w:p_story && \
				$w:p_story <  d:ST:P082_01_P01エンディング１ポリゴンデモ１開始){

		@rpc_ソリダス戦中

	//ばぐっちです。
	}else {
		@rpc_ばぐっち
	}
}

/*
proc rpc_大佐デフォルト普通潜水 {
	if($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了){
		@rpc_大佐デフォルト＿潜水
	}else {
		@rpc_大佐デフォルト
	}
}
*/

proc rpc_エマ狙撃中デフォルト {
	if($w:p_story <  d:ST:P068_01_P01ヴァンプ狙撃前１ポリゴンデモ１開始 ){
		@rpc_エマ狙撃中

	}else if(d:ST:P068_01_P01ヴァンプ狙撃前１ポリゴンデモ１終了 <= $w:p_story && \
				$w:p_story <  d:ST:P069_01_P01ヴァンプ狙撃終了１ポリゴンデモ１開始 ){
		@rpc_ヴァンプ狙撃中

	}else {
		@rpc_ばぐっち
	}
}


block codec RPC_大佐デフォルト 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_大佐デフォルト
}

block codec RPC_大佐デフォルト＿フォーチュン戦中 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_フォーチュン戦中
}

block codec RPC_ファットマン戦中救済ＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ファットマン戦＿頭狙え１
	@rp_PC_ファットマン戦＿頭狙え３
	@rp_PC_ファットマン戦＿転ぶ
}


block codec RPC_大佐デフォルト＿ファットマン戦中と後 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１開始){

		if( ($f:rfp_ファットマン死んだ) && ($f:rfp_ファットマン爆弾生きてる) ){
			@rpc_PC_ファットマン戦＿死後爆弾残り
		}else {
			@rpc_ファットマン戦中
		}
	}else if(d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１開始){

		@rpc_ファットマン死後爆弾解体中

	}else if( (100 <= $w:ライデン懸垂回数 && $w:ライデン懸垂回数 < 200) && \
				(!$f:rfp_PC_懸垂握力ＬＶ２聞いた) ){
		@rpc_PC_懸垂握力ＬＶ２

	}else if( (200 <= $w:ライデン懸垂回数 && $w:ライデン懸垂回数 < 300) && \
			(!$f:rfp_PC_懸垂握力ＬＶ３聞いた) ){
		@rpc_PC_懸垂握力ＬＶ３

	}else if(d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P036_01_P01忍者登場１ポリゴンデモ１開始){
		if(!$f:rfp_PC_忍者登場前＿ヘリポート聞いた) {
			@rpc_PC_忍者登場前＿ヘリポート
			eval($f:rfp_PC_忍者登場前＿ヘリポート聞いた = 1)
		
		}else if( ($f:rfp_PC_ファットマン戦後＿最終爆弾ズバリ聞いた) && \
					(!$f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:忍者登場前＿ローズ助けられ]) ) {
			@rpc_PC_忍者登場前＿ローズ助けられ
			eval($f:rfp_ＳＡＶＥ後会話聞いた[d:RP_ＳＡＶＥ後会話:忍者登場前＿ローズ助けられ] = 1)
		
		//無線一回目
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if( ($w:rfp_rand < 3) && ($w:rfp_callcount <= 1) ){
				@rpc_PC_忍者登場前＿ヘリポート
			}else {
				@rpc_連続無線デフォルト
				//ＸＸＸＸＸＸＸＸＸＸ
				//PC_銃入手注意
			}
		}
	}else {
		@rpc_ばぐっち
	}
}

block codec RPC_大佐デフォルト＿エマ狙撃中 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_エマ狙撃中デフォルト
}

block codec RPC_大佐デフォルト＿ヴァンプ戦中 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_ヴァンプ戦中
}

block codec RPC_大佐デフォルト＿ハリアー戦中 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_ハリアー戦中
}

block codec RPC_w25a_大佐デフォルト＿狙撃中 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(d:ST:P042_01_R01シェル１シェル２連絡橋１無線デモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:シェル１シェル２連絡橋赤外線センサークリア){
		@rpc_狙撃イベント＿シェル１シェル２連絡橋

	}else if(d:ST:シェル１シェル２連絡橋赤外線センサークリア <= $w:p_story && \
		$w:p_story <  d:ST:P045_01_P01ハリアー登場１ポリゴンデモ１開始 ){

		if(!$f:rfp_PC_センサー破壊後＿シェル２行け聞いた) {
			@rpc_PC_センサー破壊後＿シェル２行け
			eval($f:rfp_PC_センサー破壊後＿シェル２行け聞いた = 1 )

		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if( ($w:rfp_rand < 4)  && ($w:rfp_callcount <= 1) ){
				@rpc_PC_センサー破壊後＿シェル２行け
			}else {
				@rpc_連続無線デフォルト
			}
		}
	}
}

proc rpc_w11b_大佐デフォルト {
	//レーション
	command プレイヤー状態取得
	//フナムシとりつきさん
	rand 5
	eval($w:rfp_rand = $status)
	if(d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P026_01_R01爆弾解体終了１無線デモ１開始){
		if(!$f:rfp_PC_ピーター死１聞いた) {
			@rpc_PC_ピーター死１
			eval($f:rfp_PC_ピーター死１聞いた = 1)

		}else if(!$f:rfp_PC_無臭爆弾＿Ａ脚底部聞いた) {
			@rpc_PC_無臭爆弾＿Ａ脚底部
			eval($f:rfp_PC_無臭爆弾＿Ａ脚底部聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			@rpc_PC_無臭爆弾＿Ａ脚底部

		//連続無線
		}else {
			@rpc_連続無線デフォルト
			//ＸＸＸＸＸＸＸＸＸＸ
			//PC_銃入手注意
		}

	}else if( (`%メニューフナムシ取り付き状態取得` == 1) && \
		($w:アイテム数Ｒ[d:アイテム:レーション] > 0) && \
		((!$f:rfp_PC_フナムシ＿捨て方ズバリ聞いた) || ($w:rfp_rand < 3) ) ) {
		@rpc_PC_フナムシ＿捨て方ズバリ
		eval($f:rfp_PC_フナムシ＿捨て方ズバリ聞いた = 1)

	}else if( (100 <= $w:ライデン懸垂回数 && $w:ライデン懸垂回数 < 200) && \
				(!$f:rfp_PC_懸垂握力ＬＶ２聞いた) ){
		@rpc_PC_懸垂握力ＬＶ２

	}else if( (200 <= $w:ライデン懸垂回数 && $w:ライデン懸垂回数 < 300) && \
			(!$f:rfp_PC_懸垂握力ＬＶ３聞いた) ){
		@rpc_PC_懸垂握力ＬＶ３

	}else if(d:ST:P026_01_R01爆弾解体終了１無線デモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P028_01_P01爆弾解体後フォーチュン再登場１ポリゴンデモ１開始){
		if(!$f:rfp_PC_爆弾解体後＿昇降機上がれ聞いた){
			@rpc_PC_爆弾解体後＿昇降機上がれ
			eval($f:rfp_PC_爆弾解体後＿昇降機上がれ聞いた = 1)

		//無線一回目
		}else {
			rand 5
			eval($w:rfp_rand = $status)

			if( ($w:rfp_rand < 4) && ($w:rfp_callcount <= 1) ){
				@rpc_PC_爆弾解体後＿昇降機上がれ

			//連続無線
			}else {
				@rpc_連続無線デフォルト
			}
		}
	}else {
		@rpc_ばぐっち
	}
}

block codec RPC_w11b_大佐デフォルト 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11b_大佐デフォルト
}


block codec RPC_大佐デフォルト＿発狂後 14085 d:大佐無線５バグ大佐 {
	@rp_無線デフォルト前処理＿大佐
	@rpc_大佐デフォルト＿発狂後
}

/*
block codec RPC_大佐デフォルト＿ゴルルコ兵マスクあり 14085 d:大佐無線４ゴルルコ兵マスクありライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_大佐デフォルト
}
*/


//まさかのための切り分けくん
proc rpc_裸大佐＿デフォルト {
	//以下裸状態ＤＥＡＴＨ！切り分け？ＸＸＸＸＸＸＸＸＸＸＸＸ
	if( ($w:p_story <  d:ST:P072_01_R01大佐混乱１無線機デモ１開始) && \
		(!$f:w41a_主観デモ完全終了フラグ) ){
			@rpc_PC_拘束解放前

	}else if($w:p_story <  d:ST:P072_01_R01大佐混乱１無線機デモ１開始){

		if( ($s:エリア == "w41a") && ($f:rfp_w41a_拷問部屋にいるのＤＥＡＴＨ) && \
				( 	(!$f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋聞いた) || \
					(( ($w:アラートモード == d:ALERT_MODE_AVOID) || ($w:アラートモード == d:ALERT_MODE_SEARCH) ) && \
					 (!$f:w41a_捕まったふりバレたフラグ) && \
					 (!$f:rfp_PC_拷問部屋＿回避モード状態聞いた) ) ) ){
			@rpc_PC_拘束解除後＿合流しろ＿拷問部屋
			eval($f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋聞いた = 1)

		}else if( ($s:エリア == "w41a") && (!$f:rfp_w41a_拷問部屋にいるのＤＥＡＴＨ) && \
					(!$f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋外聞いた) ) {
				@rpc_PC_拘束解除後＿合流しろ＿拷問部屋外
				eval($f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋外聞いた = 1)

		}else if( ($s:エリア == "w42a") && (!$f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋以外聞いた) ){
				@rpc_PC_拘束解除後＿合流しろ＿拷問部屋以外
				eval($f:rfp_PC_拘束解除後＿合流しろ＿拷問部屋以外聞いた = 1 )

		}else if(!$f:rfp_PC_裸注意終了) {
			@rpc_裸注意

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			@rpc_裸デフォルト
		
		//連続無線
		}else {
			@rpc_裸連続デフォルト
		}

	}else if( d:ST:P072_01_R01大佐混乱１無線機デモ１終了 <= $w:p_story && \
				$w:p_story <  d:ST:P080_01_P01ＡＧ浮上１ポリゴンデモ１開始){
		@rpc_発狂中
	}
}


block codec RPC_大佐デフォルト＿裸ライデン 14085 d:大佐無線５バグ大佐裸ライデン {
	@rp_無線デフォルト前処理＿大佐
print '裸だかだか'
//	@rpc_大佐デフォルト
	@rpc_裸大佐＿デフォルト
	@rp_まお封じ
}

proc rpc_海底ドックデフォルト {
	if($s:エリア == "w11b") {
		@rpc_w11b_大佐デフォルト
	}else {
		@rpc_大佐デフォルト＿潜水
	}
}

block codec RPC_大佐デフォルト＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_大佐デフォルト＿潜水
}



//--------------------------------------------------
//場所依存系ってどう？
//--------------------------------------------

//--------------------------------------------
//w11a
//--------------------------------------
//開かない金網扉
proc rpc_開かない金網ドア {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_開かない金網ドア
}

proc rpc_w11_開かない金網扉 {
	if(!$f:rfp_PC_開かない金網ドア聞いた){
		@rpc_開かない金網ドア
		eval($f:rfp_PC_開かない金網ドア聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 5
		eval($w:rfp_rand = $status)	

		if($w:rfp_rand < 1){
			@rpc_開かない金網ドア
		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_海底ドックデフォルト
		}

	//連続無線
	}else {
//		@rpc_大佐デフォルト普通潜水
		@rpc_海底ドックデフォルト
	}
}

proc rpc_w11_開かない金網扉＿潜水 {
	if(!$f:rfp_PC_開かない金網ドア聞いた){
		@rpc_開かない金網ドア
		eval($f:rfp_PC_開かない金網ドア聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 5
		eval($w:rfp_rand = $status)	

		if($w:rfp_rand < 1){
			@rpc_開かない金網ドア
		}else {
			@rpc_海底ドックデフォルト
		}

	//連続無線
	}else {
//		@rpc_大佐デフォルト普通潜水
		@rpc_海底ドックデフォルト
	}
}

block codec RPC_w11＿開かない金網扉＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11_開かない金網扉
}

block codec RPC_w11b＿開かない金網扉＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11_開かない金網扉
}


proc rpc_ホフクポイント {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ホフクポイント
	if(!$f:rfp_PC_イントルード聞いた) {
		@rpc_PC_イントルード
		eval($f:rfp_PC_イントルード聞いた = 1)
	}
}


proc rpc_ホフクポイント＿デフォルト＿普通 {
	if(!$f:rfp_PC_ホフクポイント聞いた){
		@rpc_ホフクポイント
		eval($f:rfp_PC_ホフクポイント聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 5
		eval($w:rfp_rand = $status)	

		if($w:rfp_rand < 1){
			@rpc_ホフクポイント
		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
//		@rpc_大佐デフォルト普通潜水
		@rpc_大佐デフォルト
	}
}

proc rpc_ホフクポイント＿デフォルト＿潜水 {
	if(!$f:rfp_PC_ホフクポイント聞いた){
		@rpc_ホフクポイント
		eval($f:rfp_PC_ホフクポイント聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 5
		eval($w:rfp_rand = $status)	

		if($w:rfp_rand < 1){
			@rpc_ホフクポイント
		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_海底ドックデフォルト
		}

	//連続無線
	}else {
//		@rpc_大佐デフォルト普通潜水
		@rpc_海底ドックデフォルト
	}
}

block codec RPC_ホフクポイント＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_ホフクポイント＿デフォルト＿潜水
}

block codec RPC_ホフクポイント＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_ホフクポイント＿デフォルト＿普通
}



proc rpc_ダクト {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ダクト
	if(!$f:rfp_PC_イントルード聞いた) {
		@rpc_PC_イントルード
		eval($f:rfp_PC_イントルード聞いた = 1)
	}
}


proc rpc_ダクト＿デフォルト＿普通 {
	if(!$f:rfp_PC_ダクト聞いた){
		@rpc_ダクト
		eval($f:rfp_PC_ダクト聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 5
		eval($w:rfp_rand = $status)	

		if($w:rfp_rand < 1){
			@rpc_ダクト
		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
//		@rpc_大佐デフォルト普通潜水
			@rpc_大佐デフォルト
	}
}

proc rpc_ダクト＿デフォルト＿潜水 {
	if(!$f:rfp_PC_ダクト聞いた){
		@rpc_ダクト
		eval($f:rfp_PC_ダクト聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 5
		eval($w:rfp_rand = $status)	

		if($w:rfp_rand < 1){
			@rpc_ダクト
		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_海底ドックデフォルト
		}

	//連続無線
	}else {
//		@rpc_大佐デフォルト普通潜水
		@rpc_海底ドックデフォルト
	}
}

block codec RPC_ダクト＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_ダクト＿デフォルト＿潜水
}

block codec RPC_ダクト＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_ダクト＿デフォルト＿普通
}

proc rpc_水中＿デフォルト {
	command プレイヤー状態取得
	eval($i:rfp_プレイヤー状態 = $status)
	if( ($w:p_story >= d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了) && \
		(!$f:エマ存在フラグ) ){
			@rpc_PC_エマ救出後＿エマおいてけぼり

	}else if( ($i:rfp_プレイヤー状態 & d:PFLAG_WATER) && (!$f:rfp_エマ水中終了) && \
		($w:p_story >= d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了) ){
		@rpc_エマ水中

	}else if( (!$f:rfp_エマ手つなぎ終了) && (!($i:rfp_プレイヤー状態 & d:PFLAG_WATER)) && \
				($w:p_story >= d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了) ){
		@rpc_エマ手つなぎ

	}else if( ($i:rfp_プレイヤー状態 & d:PFLAG_WATER_SURFACE) && (!$f:rfp_PC_水中操作説明＿水上聞いた) ){
		@rpc_水中操作説明＿水上
		eval($f:rfp_PC_水中操作説明＿水上聞いた = 1)

	}else if( ($i:rfp_プレイヤー状態 & d:PFLAG_WATER) && \
//				(!($i:rfp_プレイヤー状態 & d:PFLAG_WATER_SURFACE)) && \
				(!$f:rfp_PC_水中操作説明＿水中聞いた) ){
		@rpc_水中操作説明＿水中
		eval($f:rfp_PC_水中操作説明＿水中聞いた = 1)

	//無線一回目
	}else {
		rand 5
		eval($w:rfp_rand = $status)

		if( ($w:rfp_callcount <= 1) && \
			($w:rfp_rand < 2) && ($w:p_story >= d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了) && \
			($i:rfp_プレイヤー状態 & d:PFLAG_WATER) && ($f:エマと一緒に水の中) ){
				@rpc_エマ水中


		}else if( ($w:rfp_callcount <= 1) && \
					($w:rfp_rand < 3) && ($i:rfp_プレイヤー状態 & d:PFLAG_WATER_SURFACE) ){
				@rpc_水中操作説明＿水上
				
		}else if( ($w:rfp_callcount <= 1) && \
					($w:rfp_rand < 4) && ($i:rfp_プレイヤー状態 & d:PFLAG_WATER) && \
						(!($i:rfp_プレイヤー状態 & d:PFLAG_WATER_SURFACE)) ){
				@rpc_水中操作説明＿水中
		
		}else {
			if( (!$f:rfp_PC_水中操作説明＿水中終了) && ($w:rfp_rand < 3) && \
				($i:rfp_プレイヤー状態 & d:PFLAG_WATER) && \
				(!($i:rfp_プレイヤー状態 & d:PFLAG_WATER_SURFACE) ) ){
				@rpc_水中操作説明＿水中
			}else {
				@rpc_大佐デフォルト
			}
		}
	}
}



proc rpc_水中＿デフォルト＿潜水 {
	command プレイヤー状態取得
	eval($i:rfp_プレイヤー状態 = $status)

	if( ($i:rfp_プレイヤー状態 & d:PFLAG_WATER_SURFACE) && (!$f:rfp_PC_水中操作説明＿水上聞いた) ){
		@rpc_水中操作説明＿水上
		eval($f:rfp_PC_水中操作説明＿水上聞いた = 1)

	}else if( ($i:rfp_プレイヤー状態 & d:PFLAG_WATER) && \
				(!($i:rfp_プレイヤー状態 & d:PFLAG_WATER_SURFACE)) && \
				(!$f:rfp_PC_水中操作説明＿水中聞いた) ){
		@rpc_水中操作説明＿水中
		eval($f:rfp_PC_水中操作説明＿水中聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 5
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 2){
			if($i:rfp_プレイヤー状態 & d:PFLAG_WATER_SURFACE){
				@rpc_水中操作説明＿水上
				
			}else if( ($i:rfp_プレイヤー状態 & d:PFLAG_WATER) && \
						(!($i:rfp_プレイヤー状態 & d:PFLAG_WATER_SURFACE)) ){
				@rpc_水中操作説明＿水中
			//保険
			}else {
//				@rpc_大佐デフォルト普通潜水
				@rpc_海底ドックデフォルト
			}
		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_海底ドックデフォルト
		}

	//連続無線
	}else {
		if( ($i:rfp_プレイヤー状態 & d:PFLAG_WATER) && \
			(!($i:rfp_プレイヤー状態 & d:PFLAG_WATER_SURFACE)) && \
			(!$f:rfp_PC_水中操作説明＿水中終了) ){
			@rpc_水中操作説明＿水中
		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_海底ドックデフォルト
		}
	}
}


block codec RPC_水中＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_水中＿デフォルト＿潜水
}

block codec RPC_w11b_水中 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_水中＿デフォルト＿潜水
}

proc rpc_PC_ハシゴ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ハシゴ
}

proc rpc_w11_水中はしご {
	if(!$f:rfp_PC_ハシゴ聞いた){
		@rpc_PC_ハシゴ
		eval($f:rfp_PC_ハシゴ聞いた = 1)
	
	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rpc_PC_ハシゴ

		}else {
			@rpc_水中＿デフォルト＿潜水
		}

	//連続無線
	}else {
		@rpc_水中＿デフォルト＿潜水
	}
}

block codec RPC_水中＿潜水ライデン＿はしご 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11_水中はしご
}


block codec RPC_水中＿w11b 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_水中＿デフォルト＿潜水
}


block codec RPC_水中＿w11b＿はしご 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11_水中はしご
}

block codec RPC_水中＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_水中＿デフォルト
}



proc  rpc_エルード＿デフォルト {
	if(!$f:rfp_PC_エルード１聞いた) {
		@rpc_エルード説明
		eval($f:rfp_PC_エルード１聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_エルード説明
		}else {
			if($s:エリア == "w11b"){
				@rpc_w11b_大佐デフォルト
			}else{
				@rpc_大佐デフォルト
			}
		}

	}else {
		if(!$f:rfp_PC_エルード説明終了){
			@rpc_エルード説明
		}else {
			if($s:エリア == "w11b"){
				@rpc_w11b_大佐デフォルト
			}else{
				@rpc_大佐デフォルト
			}
		}
	}
}

proc  rpc_海底ドック＿エルード＿デフォルト {
	if(!$f:rfp_PC_エルード１聞いた) {
		@rpc_エルード説明
		eval($f:rfp_PC_エルード１聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_エルード説明
		}else {
			@rpc_海底ドックデフォルト
		}

	}else {
		if(!$f:rfp_PC_エルード説明終了){
			@rpc_エルード説明
		}else {
			@rpc_海底ドックデフォルト
		}
	}
}


block codec RPC_w11b_エルード＿潜水 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_海底ドック＿エルード＿デフォルト
}

block codec RPC_w11b_エルード＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_海底ドック＿エルード＿デフォルト
}

block codec RPC_エルード＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_エルード＿デフォルト
}


proc rpc_段差上り＿普通 {
	if(!$f:rfp_PC_段差上り１聞いた) {
		@rpc_PC_段差上り１
		eval($f:rfp_PC_段差上り１聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_段差上り１
		}else {
			if($s:エリア == "w11b"){
				@rpc_w11b_大佐デフォルト
			}else{
				@rpc_大佐デフォルト
			}
		}

	}else {
		if($s:エリア == "w11b"){
			@rpc_w11b_大佐デフォルト
		}else{
			@rpc_大佐デフォルト
		}
	}
}

proc rpc_段差上り＿潜水 {
	if(!$f:rfp_PC_段差上り１聞いた) {
		@rpc_PC_段差上り１
		eval($f:rfp_PC_段差上り１聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_段差上り１
		}else {
			@rpc_海底ドックデフォルト
		}

	}else {
		@rpc_海底ドックデフォルト
	}
}


block codec RPC_段差上り＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_段差上り＿潜水
}

block codec RPC_段差上り＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_段差上り＿普通
}



proc rpc_PC_海底ドック潜水艇 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_海底ドック潜水艇聞いた) {
		@rp_PC_海底ドック潜水艇１
		@rp_PC_海底ドック潜水艇２
		eval($f:rfp_海底ドック潜水艇聞いた = 1 )
		
	}else {
		@rp_PC_海底ドック潜水艇２
	}
}

proc rpc_海底ドック潜水艇＿デフォルト {
	if(!$f:rfp_海底ドック潜水艇聞いた) {
		@rpc_PC_海底ドック潜水艇
		eval($f:rfp_海底ドック潜水艇聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_海底ドック潜水艇
		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_海底ドックデフォルト
		}

	}else {
//		@rpc_大佐デフォルト普通潜水
		@rpc_海底ドックデフォルト
	}
}


block codec RPC_海底ドック潜水艇＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_海底ドック潜水艇＿デフォルト
}

/*
block codec RPC_海底ドック潜水艇＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_海底ドック潜水艇＿デフォルト
}
*/

proc rpc_PC_ジムスーツ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_ジムスーツ聞いた) {
		@rpcc_PC_ジムスーツ１
		eval($f:rfp_PC_ジムスーツ聞いた = 1 )
	}
	@rp_PC_ジムスーツ２
}

proc rpc_ジムスーツ＿デフォルト {
	if(!$f:rfp_PC_ジムスーツ聞いた) {
		@rpc_PC_ジムスーツ
		eval($f:rfp_PC_ジムスーツ聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_ジムスーツ
		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_海底ドックデフォルト
		}

	}else {
//		@rpc_大佐デフォルト普通潜水
		@rpc_海底ドックデフォルト
	}
}


block codec RPC_ジムスーツ＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_ジムスーツ＿デフォルト
}

/*
block codec RPC_ジムスーツ＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_ジムスーツ＿デフォルト
}
*/

proc rpc_PC_ロッカー開かない {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ロッカー開かない
}

proc ロッカー共通＿デフォルト {
	//ろっかー壊れめ。
	if( $i:rfp_ロッカー状態返り値 & 8192 ){
		print 'broken 壊れてりあ'
//		@rpc_大佐デフォルト普通潜水
		@rpc_大佐デフォルト

	}else if(!$f:rfp_PC_ロッカー説明終了){
		@rpc_ロッカー説明

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_ロッカー説明
		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_大佐デフォルト
		}

	}else {
//		@rpc_大佐デフォルト普通潜水
		@rpc_大佐デフォルト
	}
}


proc ロッカー共通＿デフォルト＿海底ドック {
	//ろっかー壊れめ。
	if( $i:rfp_ロッカー状態返り値 & 8192 ){
		print 'broken 壊れてりあ'
		@rpc_海底ドックデフォルト

	}else if(!$f:rfp_PC_ロッカー説明終了){
		@rpc_ロッカー説明

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_ロッカー説明
		}else {
			@rpc_海底ドックデフォルト
		}

	}else {
		@rpc_海底ドックデフォルト
	}
}

proc rpc_PC_Ｂ脚配電盤前 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｂ脚配電盤前
}

proc 変電盤共通＿デフォルト {
	//変電扉壊れめ。
	if( $i:rfp_ロッカー状態返り値 & 8192 ){
		print 'broken 壊れてりあ'
		@rpc_大佐デフォルト

	}else if(!$f:rfp_PC_ロッカー説明終了){
		@rpc_PC_Ｂ脚配電盤前

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_Ｂ脚配電盤前
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}


proc rpc_ロッカー状態判定＿普通 {
	//ろっかー壊れめ。
	if( $status & 8192 ){
		print 'broken 壊れてりあ'
		if($s:エリア == "w11b"){
			@rpc_w11b_大佐デフォルト
		}else{
			@rpc_大佐デフォルト
		}

	//開いているのか、閉まっているのか。それは問題ではない。
	}else{
		@rpc_ロッカー説明
	}
}

proc rpc_ロッカー状態判定＿潜水 {
	//ろっかー壊れめ。
	if( $status & 8192 ){
		print 'broken 壊れてりあ'
		@rpc_海底ドックデフォルト

	//開いているのか、閉まっているのか。それは問題ではない。
	}else{
		@rpc_ロッカー説明
	}
}


proc rpc_w11_ロッカーlk001＿デフォルト＿普通 {
	command ロッカー状態 -n ロッカー１
	@rpc_ロッカー状態判定＿普通
}

proc rpc_w11_ロッカーlk001＿デフォルト＿潜水 {
	command ロッカー状態 -n ロッカー１
	@rpc_ロッカー状態判定＿潜水
}

proc rpc_w11_ロッカーlk002＿デフォルト＿普通 {
	command ロッカー状態 -n ロッカー２
	@rpc_ロッカー状態判定＿普通
}

proc rpc_w11_ロッカーlk002＿デフォルト＿潜水 {
	command ロッカー状態 -n ロッカー２
	@rpc_ロッカー状態判定＿潜水
}

proc rpc_w11_ロッカーlk003＿デフォルト＿普通 {
	command ロッカー状態 -n ロッカー３
	@rpc_ロッカー状態判定＿普通
}
proc rpc_w11_ロッカーlk003＿デフォルト＿潜水 {
	command ロッカー状態 -n ロッカー３
	@rpc_ロッカー状態判定＿潜水
}



block codec RPC_w11_ロッカーlk001＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11_ロッカーlk001＿デフォルト＿潜水
}

block codec RPC_w11_ロッカーlk001＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11_ロッカーlk001＿デフォルト＿普通
}

block codec RPC_w11_ロッカーlk002＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11_ロッカーlk002＿デフォルト＿潜水
}

block codec RPC_w11_ロッカーlk002＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11_ロッカーlk002＿デフォルト＿普通
}

block codec RPC_w11_ロッカーlk003＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11_ロッカーlk003＿デフォルト＿潜水
}

block codec RPC_w11_ロッカーlk003＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11_ロッカーlk003＿デフォルト＿普通
}


proc rpc_PC_水密扉 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_水密扉聞いた) {
		@rp_PC_水密扉１
		if($w:タンカー編クリア回数 >= 1){
			@rpcc_PC_水密扉２
		}else {
			@rp_PC_水密扉３
		}
		eval($f:rfp_PC_水密扉聞いた = 1 )

	}else {
		@rp_PC_水密扉３
	}
}


proc rpc_水密扉＿潜水 {
	if(!$f:rfp_PC_水密扉聞いた) {
		@rpc_PC_水密扉
		eval($f:rfp_PC_水密扉聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_水密扉
		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_海底ドックデフォルト
		}

	}else {
//		@rpc_大佐デフォルト普通潜水
		@rpc_海底ドックデフォルト
	}
}


block codec RPC_水密扉＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_水密扉＿潜水
}

/*
block codec RPC_水密扉＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_水密扉＿デフォルト
}
*/

proc rpc_PC_ノード前接続前１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rpcc_PC_ノード前接続前１
}

proc rpc_PC_ノードうんちく {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ノードうんちく
}

proc rpc_ノード説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if( (!$f:rfp_PC_ノード機能説明１聞いた) && (!($w:コンフィグ設定 & d:CONFIG_RADAR_OFF) ) ){
		@rp_PC_ノード機能説明１
		eval($f:rfp_PC_ノード機能説明１聞いた = 1 )

	}else if( (!$f:rfp_PC_ノード機能説明２聞いた)  && (!($w:コンフィグ設定 & d:CONFIG_RADAR_OFF) ) ){
		@rp_PC_ノード機能説明２
		eval($f:rfp_PC_ノード機能説明２聞いた = 1 )

	}else if(!$f:rfp_PC_ノードうんちく聞いた) {
		@rp_PC_ノードうんちく
		eval($f:rfp_PC_ノードうんちく聞いた = 1 )

	}else if(!$f:rfp_PC_ノード前接続前２聞いた) {
		@rp_PC_ノード前接続前２
		eval($f:rfp_PC_ノード前接続前２聞いた = 1 )


	}else {
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 1) && (!($w:コンフィグ設定 & d:CONFIG_RADAR_OFF) ) ){
			@rp_PC_ノード機能説明１

		}else if( ($w:rfp_rand < 2) && (!($w:コンフィグ設定 & d:CONFIG_RADAR_OFF) ) ){
			@rp_PC_ノード機能説明２

		}else if($w:rfp_rand < 3){
			@rp_PC_ノード前接続前２

		}else{
			@rp_PC_ノードうんちく
		}
	}
}

proc rpc_w11＿ノード＿潜水 {
	if($w:p_story < d:ST:P004_01_P01ノード初接続１ポリゴンデモ１開始) {
		if(!$f:rfp_PC_ノード前接続前１聞いた) {
			@rpc_PC_ノード前接続前１
			eval($f:rfp_PC_ノード前接続前１聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			@rpc_PC_ノード前接続前１
		
		//連続無線
		}else {
			if(!$f:rfp_PC_ノードうんちく聞いた) {
				@rpc_PC_ノードうんちく
				eval($f:rfp_PC_ノードうんちく聞いた = 1 )
			}else {
				@rpc_PC_ノード前接続前１
			}
		}

	//ノード接続後
	}else {
		if(!$f:rfp_PC_ノード機能説明１聞いた) {
			@rpc_ノード説明
			eval($f:rfp_PC_ノード機能説明１聞いた = 1 )
		
		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 4
			eval($w:rfp_rand = $status)
			if($w:rfp_rand < 3){
				@rpc_ノード説明
			}else {
//				@rpc_大佐デフォルト普通潜水
				@rpc_海底ドックデフォルト
			}

		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_海底ドックデフォルト
		}
	}
}



block codec RPC_w11＿ノード＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11＿ノード＿潜水
}


proc rpc_PC_海底ドック昇降機前ノード接続前 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_海底ドック昇降機前ノード接続前
}

proc rpc_PC_海底ドック昇降機到着後危険モード状態 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_海底ドック昇降機到着後危険モード状態
}


proc rpc_w11＿昇降機＿デフォルト {
	if($w:p_story < d:ST:P004_01_P01ノード初接続１ポリゴンデモ１開始) {
		if(!$f:rfp_PC_海底ドック昇降機前ノード接続前聞いた) {
			@rpc_PC_海底ドック昇降機前ノード接続前
			eval($f:rfp_PC_海底ドック昇降機前ノード接続前聞いた = 1 )

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 4
			eval($w:rfp_rand = $status)
			if($w:rfp_rand < 3){
				@rpc_PC_海底ドック昇降機前ノード接続前

			}else {
//			@rpc_大佐デフォルト普通潜水
				@rpc_大佐デフォルト＿潜水
			}

		//連続無線
		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_大佐デフォルト＿潜水
		}


	//ノード接続後
	}else {
		//のれますよ
		if( ($f:rfp_w11_昇降機に乗れますよ) && \
			($w:アラートモード != 1) && \	//危険モードじゃない
			(!$f:rfp_PC_海底ドック昇降機昇降機到着後聞いた) ){
				@rpc_PC_海底ドック昇降機昇降機到着後
				eval($f:rfp_PC_海底ドック昇降機昇降機到着後聞いた = 1 )

		//$f:rfp_w11a_昇降機下り始めた = 1です
		}else if( ($f:rfp_w11a_昇降機下り始めた) && (!$f:rfp_w11_昇降機に乗れますよ) && \
					(!$f:rfp_PC_海底ドック昇降機前ノード接続後聞いた) ){
			@rpc_PC_海底ドック昇降機前ノード接続後
			eval($f:rfp_PC_海底ドック昇降機前ノード接続後聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 4
			eval($w:rfp_rand = $status)
			if($w:rfp_rand < 3){
				if($f:rfp_w11_昇降機に乗れますよ){
					if($w:アラートモード != 1){
						@rpc_PC_海底ドック昇降機昇降機到着後
					}else {
						@rpc_PC_海底ドック昇降機到着後危険モード状態
					}
				}else {
					@rpc_PC_海底ドック昇降機前ノード接続後
				}

			}else {
//				@rpc_大佐デフォルト普通潜水
				@rpc_大佐デフォルト＿潜水
			}

		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_大佐デフォルト＿潜水
		}
	}
}


proc rpc_RPC_w11a_敵兵おっきした警告ＣＡＬＬ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_w11a_敵兵起きた){
	}
	@rp_PC_海底ドック敵覚醒
}

block codec RPC_w11＿昇降機＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11＿昇降機＿デフォルト
}

block codec RPC_w11a_敵兵おっきした警告ＣＡＬＬ＿潜水ライデン 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_RPC_w11a_敵兵おっきした警告ＣＡＬＬ
	eval($f:rfp_w11a_敵兵おっきした警告ＣＡＬＬ聞いた = 1)
}




//Ａ脚屋上 w12ac
proc rpc_Ａ脚屋上ホフク金網 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ａ脚屋上ホフク金網１
	if(!$f:rfp_PC_Ａ脚屋上ホフク金網聞いた) {
		@rp_PC_ホフク説明１
		@rp_PC_ホフク説明２
		eval($f:rfp_PC_Ａ脚屋上ホフク金網聞いた = 1 )
	}
}


block codec RPC_Ａ脚屋上ホフク金網ＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_Ａ脚屋上ホフク金網
	eval($f:rfp_PC_Ａ脚屋上ホフク金網聞いた = 1 )
}

proc rpc_PC_Ａ脚屋上ホフク金網３ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ａ脚屋上ホフク金網３
}


block codec RPC_Ａ脚屋上ホフク金網 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_Ａ脚屋上ホフク金網聞いた) {
		@rpc_Ａ脚屋上ホフク金網
		eval($f:rfp_PC_Ａ脚屋上ホフク金網聞いた = 1 )

	}else if(!$f:rfp_PC_Ａ脚屋上ホフク金網３聞いた){
		@rpc_PC_Ａ脚屋上ホフク金網３
		eval($f:rfp_PC_Ａ脚屋上ホフク金網３聞いた = 1)
		
	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_Ａ脚屋上ホフク金網
		}else {
			@rpc_大佐デフォルト
		}

	}else {
		@rpc_大佐デフォルト
	}
}



proc rpc_奈落エルード＿デフォルト {
	command プレイヤー状態取得
	
	if( ($status & d:PFLAG_ELUDE) && (!$f:rfp_PC_奈落エルード聞いた) ){
		@rpc_PC_奈落エルード
		eval($f:rfp_PC_奈落エルード聞いた = 1)

	}else if(!$f:rfp_PC_エルード説明終了) {
		@rpc_エルード説明

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			command プレイヤー状態取得
			if($status & d:PFLAG_ELUDE){
				@rpc_PC_奈落エルード
			}else {
				@rpc_エルード説明
			}

		}else if($w:rfp_rand < 2){
			@rpc_エルード説明

		}else {
			@rpc_大佐デフォルト
		}

	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_ヘリポートの縁 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ヘリポートの縁
}

proc rpc_w20b_ヘリポート＿オイ渕＿デフォルト {
	command プレイヤー状態取得
	
	if( ($status & d:PFLAG_ELUDE) && (!$f:rfp_PC_ヘリポート渕奈落エルード聞いた) ){
		@rpc_PC_奈落エルード
		eval($f:rfp_PC_ヘリポート渕奈落エルード聞いた = 1)

	}else if(!$f:rfp_PC_ヘリポートの縁聞いた) {
		@rpc_PC_ヘリポートの縁
		eval($f:rfp_PC_ヘリポートの縁聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			command プレイヤー状態取得
			if($status & d:PFLAG_ELUDE){
				@rpc_PC_奈落エルード
			}else {
				@rpc_PC_ヘリポートの縁
			}

		}else {
			@rpc_大佐デフォルト
		}

	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_鳴り床 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_鳴り床１
	@rp_PC_鳴り床＿そろそろ歩き
	@rp_PC_鳴り床＿ホフク
}

proc rpc_奈落鳴り床エルード＿デフォルト {
	command プレイヤー状態取得
	
	if( ($status & d:PFLAG_ELUDE) && (!$f:rfp_PC_奈落エルード聞いた) ){
		@rpc_PC_奈落エルード
		eval($f:rfp_PC_奈落エルード聞いた = 1)

	//エルードでない
	}else if( (!($status & d:PFLAG_ELUDE)) && (!$f:rfp_PC_鳴り床聞いた) ){
		@rpc_PC_鳴り床
		eval($f:rfp_PC_鳴り床聞いた = 1)

	}else if(!$f:rfp_PC_エルード説明終了) {
		@rpc_エルード説明

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 5
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			command プレイヤー状態取得
			if($status & d:PFLAG_ELUDE){
				@rpc_PC_奈落エルード
			}else {
				@rpc_エルード説明
			}

		}else if($w:rfp_rand < 2){
			@rpc_エルード説明

		}else if($w:rfp_rand < 4){
			command プレイヤー状態取得
			if(!($status & d:PFLAG_ELUDE)){
				@rpc_PC_鳴り床
			}else {
				@rpc_大佐デフォルト
			}
		}else {
			@rpc_大佐デフォルト
		}

	}else {
		@rpc_大佐デフォルト
	}
}


proc rpc_落し穴＿奈落エルード＿デフォルト {
	command プレイヤー状態取得
	
	if( ($status & d:PFLAG_ELUDE) && (!$f:rfp_PC_奈落エルード聞いた) ){
		@rpc_PC_奈落エルード
		eval($f:rfp_PC_奈落エルード聞いた = 1)

	//エルードでない
	}else if( (!($status & d:PFLAG_ELUDE)) && \
				(!$f:rfp_そこの落とし穴床落ちていますが。) && \
				(!$f:rfp_PC_落とし穴聞いた) ){
		@rpc_PC_落とし穴
		eval($f:rfp_PC_落とし穴聞いた = 1)

	}else if(!$f:rfp_PC_エルード説明終了) {
		@rpc_エルード説明

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 5
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			command プレイヤー状態取得
			if( $status & d:PFLAG_ELUDE){
				@rpc_PC_奈落エルード
			}else {
				@rpc_エルード説明
			}

		}else if($w:rfp_rand < 2){
			@rpc_エルード説明

		}else if( ($w:rfp_rand < 4) && \
					(!$f:rfp_そこの落とし穴床落ちていますが。) ){
			command プレイヤー状態取得
			if(!($status & d:PFLAG_ELUDE)){
				@rpc_PC_落とし穴
			}else {
				@rpc_大佐デフォルト
			}
		}else {
			@rpc_大佐デフォルト
		}

	}else {
		@rpc_大佐デフォルト
	}
}

//結局どこよ！？ＸＸＸＸＸＸＸＸＸＸＸＸｘｘ
proc rpc_PC_ＥＦ連絡橋＿クレイモア＿地雷探知器ない＿場所聞いた {
	if($f:rfp_地雷地帯にＦ脚から来た){
		@rp_PC_ＥＦ連絡橋＿クレイモア＿地雷探知器ない＿場所聞いた３
	}else {
		@rp_PC_ＥＦ連絡橋＿クレイモア＿地雷探知器ない＿場所聞いた１
	}
}

proc rpc_ＥＦ連絡橋＿クレイモア {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＥＦ連絡橋＿クレイモア１
	//地雷探知器もってないよ
	if($w:アイテム数Ｒ[d:アイテム:地雷探知器] <= 0){
		//スネ公に場所聞いてない
		if(!$f:rfp_PS_地雷探知器のありか聞いた){
			@rp_PC_ＥＦ連絡橋＿クレイモア＿地雷探知器ない＿場所知らない
			@rpd_ローズ顔表示
			//ヒント条件は？ＸＸＸＸＸＸＸ
			@rp_PC_ＤＧ連絡橋＿赤外線センサー＿ＰＳＧ１探せ２
			//PC_プリスキン周波数１
			if(!$f:rfp_短縮ダイアル＿プリスキンセットした){
				@rp_PC_プリスキン周波数３
			}
		}else {
			@rpc_PC_ＥＦ連絡橋＿クレイモア＿地雷探知器ない＿場所聞いた
		}
	//もってるっぽいよ
	}else {
		if(!$f:rfp_ＥＦ連絡橋＿クレイモア聞いた){
			@rp_PC_ＥＦ連絡橋＿クレイモア＿地雷探知器ある＿レーダー可１
			@rp_PC_ＥＦ連絡橋＿クレイモア＿地雷探知器ある＿レーダー可２
			if( ($w:ゲーム設定 >= d:LEVEL_EASY) && (`@rp_敵兵元気さんです 敵兵:01`) ){
				@rp_PC_ＥＦ連絡橋＿クレイモア＿地雷探知器ある＿レーダー可３
			}
			eval($f:rfp_ＥＦ連絡橋＿クレイモア聞いた = 1)
		}else {
			rand 2
			eval($w:rfp_rand = $status)
			if($w:rfp_rand < 1){
				@rp_PC_ＥＦ連絡橋＿クレイモア＿地雷探知器ある＿レーダー可１
			}else {
				@rp_PC_ＥＦ連絡橋＿クレイモア＿地雷探知器ある＿レーダー可２
			}
		}
	}
}

proc rpc_PC_ガンサイファー１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ガンサイファー１
}

proc rpc_PC_ガンサイファー＿チャフ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ガンサイファー＿チャフ
}

proc rpc_PC_ガンサイファー＿撃墜 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ガンサイファー＿撃墜
}

proc rpc_PC_リモコンミサイル使えない {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_リモコンミサイル使えない１
	if(!$f:rfp_PC_リモコンミサイル使えない聞いた){
		@rp_PC_リモコンミサイル使えない２
		eval($f:rfp_PC_リモコンミサイル使えない聞いた = 1)
	}
}

proc rpc_海鳥うんちく {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_海鳥１
	@rp_PC_海鳥糞２
}

proc rpc_ガンサイファーかもめ連絡橋＿大佐デフォルト {
	@rp_そこのガンサイファー存在するか判定
	if( ($f:rfp_そこのガンサイファー存在してます) && \
		(!$f:rfp_PC_ガンサイファー１聞いた) ){
		@rpc_PC_ガンサイファー１
		eval($f:rfp_PC_ガンサイファー１聞いた = 1)

	}else if(($f:rfp_そこのガンサイファー存在してます) && \
					(!$f:rfp_PC_ガンサイファー＿チャフ聞いた) ){
		@rpc_PC_ガンサイファー＿チャフ
		eval($f:rfp_PC_ガンサイファー＿チャフ聞いた = 1)

	}else if(($f:rfp_そこのガンサイファー存在してます) && \
				(!$f:rfp_PC_ガンサイファー＿撃墜聞いた) ){
		@rpc_PC_ガンサイファー＿撃墜
		eval($f:rfp_PC_ガンサイファー＿撃墜聞いた = 1)

	}else if( ($w:武器弾数Ｒ[d:武器:ニキータ] >= 0) && \
				($w:武器 == d:武器:ニキータ) && \
				(!$f:rfp_PC_リモコンミサイル使えない聞いた) ){
			@rpc_PC_リモコンミサイル使えない
			eval($f:rfp_PC_リモコンミサイル使えない聞いた = 1)

	}else if( (!$f:rfp_海鳥うんちく聞いた) && \
				($w:rfp_callcount > 1 ) ){
		@rpc_海鳥うんちく
		eval($f:rfp_海鳥うんちく聞いた = 1)

	//海鳥が襲ってくるところは？？？
	//無線一回目
	}else {
		rand 5
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 1) && ($w:武器 == d:武器:ニキータ) && \
			($w:rfp_callcount <= 1) ){
			@rpc_PC_リモコンミサイル使えない
			
		}else if( ($w:rfp_rand < 2) && \
					($f:rfp_そこのガンサイファー存在してます) && \
					($w:rfp_callcount <= 1) ){
			@rpc_PC_ガンサイファー１

		}else if( ($w:rfp_rand < 3) &&  \
					($f:rfp_そこのガンサイファー存在してます) && \
					($w:rfp_callcount <= 1)){
			@rpc_PC_ガンサイファー＿チャフ

		}else if( ($w:rfp_rand < 4) &&  \
					($f:rfp_そこのガンサイファー存在してます) && \
					($w:rfp_callcount <= 1)){
			@rpc_PC_ガンサイファー＿撃墜

		}else {
			@rpc_大佐デフォルト
		}
	}
}


proc rpc_地雷＿奈落エルード＿デフォルト {
	command プレイヤー状態取得
	
	if( ($status & d:PFLAG_ELUDE) && (!$f:rfp_PC_奈落エルード聞いた) ){
		@rpc_PC_奈落エルード
		eval($f:rfp_PC_奈落エルード聞いた = 1)

	//エルードでない
	//地雷存在条件は？？ＸＸＸＸＸＸＸＸ
	}else if( (!($status & d:PFLAG_ELUDE)) && \
				($f:rfp_w21a_地雷原１無線デモ１した) && \
				($w:p_story < d:ST:P038_01_S01網膜チェック１シナリオデモ１開始) && \
				(!$f:rfp_ＥＦ連絡橋＿クレイモア聞いた) ){
		@rpc_ＥＦ連絡橋＿クレイモア
		eval($f:rfp_ＥＦ連絡橋＿クレイモア聞いた = 1)

	}else if(!$f:rfp_PC_エルード説明終了) {
		@rpc_エルード説明

	//無線一回目
	}else {
		rand 5
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 1) && ($w:rfp_callcount <= 1) ){
			command プレイヤー状態取得
			if( $status & d:PFLAG_ELUDE){
				@rpc_PC_奈落エルード
			}else {
				@rpc_エルード説明
			}

		}else if( ($w:rfp_rand < 4) && ($w:rfp_callcount <= 1) && \
					(!(`%プレイヤー状態取得` & d:PFLAG_ELUDE) ) ){
				@rpc_ＥＦ連絡橋＿クレイモア

		}else {
			@rpc_ガンサイファーかもめ連絡橋＿大佐デフォルト
		}
	}
}

proc rpc_鳴り床＿デフォルト {
	if(!$f:rfp_PC_鳴り床聞いた){
		@rpc_PC_鳴り床
		eval($f:rfp_PC_鳴り床聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_鳴り床

		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_落とし穴 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_落とし穴
}


proc rpc_落し穴＿デフォルト {
	if( (!$f:rfp_PC_落とし穴聞いた) && (!$f:rfp_そこの落とし穴床落ちていますが。) ){
		@rpc_PC_落とし穴
		eval($f:rfp_PC_落とし穴聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && (!$f:rfp_そこの落とし穴床落ちていますが。) ){
			@rpc_PC_落とし穴

		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_鳴り床エルード＿デフォルト {
	command プレイヤー状態取得
	if( ($i:プレイヤー位置Ｙ >= 0) || ($status & d:PFLAG_ELUDE) ){

		if( ($status & d:PFLAG_ELUDE) && (!$f:rfp_PC_エルード２聞いた) ){
			@rpc_エルード説明
			eval($f:rfp_PC_エルード２聞いた = 1)

		//エルードでない
		}else if( (!($status & d:PFLAG_ELUDE)) && (!$f:rfp_PC_鳴り床聞いた) ){
			@rpc_PC_鳴り床
			eval($f:rfp_PC_鳴り床聞いた = 1)

		}else if(!$f:rfp_PC_エルード説明終了) {
			@rpc_エルード説明

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 4
			eval($w:rfp_rand = $status)
			if($w:rfp_rand < 1){
				@rpc_エルード説明

			}else if($w:rfp_rand < 3){
				command プレイヤー状態取得
				if(!($status & d:PFLAG_ELUDE)){
					@rpc_PC_鳴り床
				}else {
					@rpc_大佐デフォルト
				}
			}else {
				@rpc_大佐デフォルト
			}

		}else {
			@rpc_大佐デフォルト
		}
	}else {
		@rpc_大佐デフォルト
	}
}

block codec RPC_奈落エルード 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_奈落エルード＿デフォルト
}

block codec RPC_w20b_ヘリポート＿オイ渕１ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w20b_ヘリポート＿オイ渕＿デフォルト
}

block codec RPC_w20b_ヘリポート＿オイ渕２ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w20b_ヘリポート＿オイ渕＿デフォルト
}

block codec RPC_鳴り床奈落エルード 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_奈落鳴り床エルード＿デフォルト
}

block codec RPC_鳴り床エルード 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_鳴り床エルード＿デフォルト
}

block codec RPC_ただのエルード 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_エルード＿デフォルト
}

block codec RPC_鳴り床 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_鳴り床＿デフォルト
}

block codec RPC_落し穴共通１ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_落し穴＿デフォルト
}
block codec RPC_落し穴共通２ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_落し穴＿デフォルト
}


block codec RPC_落し穴＿奈落エルード１ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_落し穴＿奈落エルード＿デフォルト
}

block codec RPC_落し穴＿奈落エルード２ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_落し穴＿奈落エルード＿デフォルト
}

block codec RPC_地雷＿奈落エルード 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_地雷＿奈落エルード＿デフォルト

}

proc rpc_PC_Ａ脚屋上昇降機乗れない状態 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	@rp_PC_Ａ脚屋上昇降機乗れない状態
}

proc rpc_w11_昇降機エルード＿デフォルト {
	command プレイヤー状態取得
	if( ($status & d:PFLAG_ELUDE) && (!$f:rfp_PC_奈落エルード聞いた) ){
		@rpc_PC_奈落エルード
		eval($f:rfp_PC_奈落エルード聞いた = 1)

	}else if( (!$f:rfp_PC_Ａ脚屋上昇降機乗れない状態聞いた) && \
				($w:p_story < d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了 || \
				d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了 <= $w:p_story ) ){
		@rpc_PC_Ａ脚屋上昇降機乗れない状態
		eval($f:rfp_PC_Ａ脚屋上昇降機乗れない状態聞いた = 1)

	}else if(!$f:rfp_PC_エルード説明終了) {
		@rpc_エルード説明

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 5
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && (`%プレイヤー状態取得` & d:PFLAG_ELUDE) ){
				@rpc_PC_奈落エルード

		}else if($w:rfp_rand < 3){
			@rpc_エルード説明

		}else if( ($w:rfp_rand < 4) && \
				($w:p_story < d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了 || \
					d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了 <= $w:p_story ) ){
			@rpc_PC_Ａ脚屋上昇降機乗れない状態

		}else {
			@rpc_大佐デフォルト
		}

	}else {
		@rpc_大佐デフォルト
	}
}

block codec RPC_w11_昇降機エルード 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11_昇降機エルード＿デフォルト
}


proc rpc_PC_Ａ脚屋上鍵扉閉鎖状態 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ａ脚屋上鍵扉閉鎖状態
}


block codec RPC_w12a_Ａ脚屋上鍵扉閉鎖状態 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_Ａ脚屋上鍵扉閉鎖状態聞いた) {
		@rpc_PC_Ａ脚屋上鍵扉閉鎖状態
		eval($f:rfp_PC_Ａ脚屋上鍵扉閉鎖状態聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_Ａ脚屋上鍵扉閉鎖状態
		}else {
			@rpc_大佐デフォルト
		}

	}else {
		@rpc_大佐デフォルト
	}
}


proc rpc_PC_フナムシ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_フナムシ聞いた) {
		@rpcc_PC_フナムシ１
		@rpcc_PC_フナムシ２
		eval($f:rfp_PC_フナムシ聞いた = 1 )
	}
	@rp_PC_フナムシ４
	@rp_PC_フナムシ＿捨て方１
}


block codec RPC_フナムシ＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:rfp_PC_フナムシ聞いた) && \
				( (!`@rp_緊張してください`) || ($w:rfp_callcount > 1) ) ){
		@rpc_PC_フナムシ
		eval($f:rfp_PC_フナムシ聞いた = 1 )


	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand  < 2) && ($f:rfp_w11_ダクトの近くにいます)  && \
			(!`@rp_緊張してください`) ){
			@rpc_ダクト

		}else if( ($w:rfp_rand < 3)  && \
					(!`@rp_緊張してください`) ){
			@rpc_PC_フナムシ

		}else {
			if($s:エリア == "w11b"){
				@rpc_w11b_大佐デフォルト
			}else{
				@rpc_大佐デフォルト
			}
		}

	}else {
		if($s:エリア == "w11b"){
			@rpc_w11b_大佐デフォルト
		}else{
			@rpc_大佐デフォルト
		}
	}
}

block codec RPC_フナムシ＿潜水 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	if( ($f:rfp_w11_ダクトの近くにいます) && (!$f:rfp_PC_ダクト聞いた) ){
		@rpc_ダクト＿デフォルト＿潜水

	}else if(!$f:rfp_PC_フナムシ聞いた) {
		@rpc_PC_フナムシ
		eval($f:rfp_PC_フナムシ聞いた = 1 )


	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand  < 2) && ($f:rfp_w11_ダクトの近くにいます) ){
			@rpc_ダクト＿デフォルト＿潜水

		}else if($w:rfp_rand < 3){
			@rpc_PC_フナムシ

		}else {
			@rpc_海底ドックデフォルト
		}

	}else {
		@rpc_海底ドックデフォルト
	}
}

block codec RPC_フナムシ＿エマ虫いや 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	command ゲットエマ状態 $b:rfp_ゲットされたエマ状態
	if( (!$f:rfp_PC_エマ救出後＿虫エリア聞いた) && \
		($f:rfp_w31b_エマ虫いやがった) && \
		($b:rfp_ゲットされたエマ状態 != d:EMA_F_I_AM_FAINTED) && \
		($b:rfp_ゲットされたエマ状態 != d:EMA_F_I_AM_SLEEPING) ){

		@rpd_ライデン顔設定デフォルト
		@rpd_大佐顔設定１
		@rpd_顔表示
		@rpc_PC_エマ救出後＿虫エリア
		eval($f:rfp_PC_エマ救出後＿虫エリア聞いた = 1)

	}else if(!$f:rfp_PC_フナムシ聞いた) {
		@rpc_PC_フナムシ
		eval($f:rfp_PC_フナムシ聞いた = 1 )


	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 5
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand  < 3) && \
			($f:rfp_w31b_エマ虫いやがった) && \
			($b:rfp_ゲットされたエマ状態 != d:EMA_F_I_AM_FAINTED) && \
			($b:rfp_ゲットされたエマ状態 != d:EMA_F_I_AM_SLEEPING) ){
			@rpd_ライデン顔設定デフォルト
			@rpd_大佐顔設定１
			@rpd_顔表示
			@rpc_PC_エマ救出後＿虫エリア

		}else if($w:rfp_rand < 4){
			@rpc_PC_フナムシ

		}else {
			@rpc_大佐デフォルト
		}

	}else {
		@rpc_大佐デフォルト
	}
}


block codec RPC_ロッカー共通１ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@ロッカー共通＿デフォルト
}

block codec RPC_ロッカー共通２ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@ロッカー共通＿デフォルト
}

block codec RPC_w11_ロッカー共通１＿潜水 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@ロッカー共通＿デフォルト＿海底ドック
}

block codec RPC_w11_ロッカー共通２＿潜水 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@ロッカー共通＿デフォルト＿海底ドック
}

block codec RPC_w11_ロッカー共通１＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@ロッカー共通＿デフォルト＿海底ドック
}

block codec RPC_w11_ロッカー共通２＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@ロッカー共通＿デフォルト＿海底ドック
}

block codec RPC_変電盤共通１ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@変電盤共通＿デフォルト
}

block codec RPC_変電盤共通２ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@変電盤共通＿デフォルト
}


proc RPC_開かないロッカー共通デフォルト {
	//ろっかー壊れめ。
	if( $i:rfp_ロッカー状態返り値 & 8192 ){
		print 'broken 壊れてりあ'
//		@rpc_大佐デフォルト普通潜水
		@rpc_大佐デフォルト

	}else if( ($f:rfp_そこの開かないロッカー開けた) && \
				(!$f:rfp_そこの開かないロッカー聞いた) ) {
		@rpc_PC_ロッカー開かない
		eval($f:rfp_そこの開かないロッカー聞いた = 1)

	//無線一回目
	}else{
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && ($w:rfp_callcount <= 1) && \
			(!( $i:rfp_ロッカー状態返り値 & 8192 )) && \
			($f:rfp_そこの開かないロッカー開けた) ){
				@rpc_PC_ロッカー開かない
			
		}else {
//			@rpc_大佐デフォルト普通潜水
			@rpc_大佐デフォルト
		}
	}
}


block codec RPC_開かないロッカー共通１ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@RPC_開かないロッカー共通デフォルト
}

block codec RPC_開かないロッカー共通２ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@RPC_開かないロッカー共通デフォルト
}

block codec RPC_ノード共通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	//無線一回目
	if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_ノード説明
		}else {
			@rpc_大佐デフォルト
		}

	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_眠りプリスキン {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_眠りプリスキン
}

block codec RPC_眠りプリスキン 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:rfp_PC_眠りプリスキン聞いた) && (`%ＮＰＣスネークの状態` == 2) ){
		@rpc_PC_眠りプリスキン
		eval($f:rfp_PC_眠りプリスキン聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && (`%ＮＰＣスネークの状態` == 2) ){
			@rpc_PC_眠りプリスキン
		}else {
			@rpc_大佐デフォルト
		}

	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_海軍大佐死体 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_海軍大佐死体
}


block codec RPC_海軍大佐死体 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_海軍大佐死体聞いた) {
		@rpc_PC_海軍大佐死体
		eval($f:rfp_PC_海軍大佐死体聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_海軍大佐死体
		}else {
			@rpc_大佐デフォルト
		}

	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_ＢＣ連絡橋壊れた橋 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＢＣ連絡橋壊れた橋
}


proc rpc_PC_カード扉 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if($w:p_story < d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {
		@rp_PC_カード扉
	}else {
		@rp_PC_カード扉＿カードあり
	}
}

block codec RPC_カードドア 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:rfp_PC_カード扉聞いた) && ($w:p_story < d:ST:P015_01_R1爆弾解体最初の一つ１無線デモ１開始) ){
		@rpc_PC_カード扉
		eval($f:rfp_PC_カード扉聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && ($w:p_story < d:ST:P015_01_R1爆弾解体最初の一つ１無線デモ１開始) ){
			@rpc_PC_カード扉
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_Ｃ脚食堂ダンボール近く {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｃ脚食堂ダンボール近く１
	//ヒント条件？ＸＸＸＸＸＸ
	@rp_PC_Ｃ脚食堂ダンボール近く＿ヒント１
	//ダンボール違い！ＸＸＸＸＸＸ

//	@rp_ローズさんご機嫌判定
//	if(!$f:rfp_PR_ローズさんが激怒してます！){
//		@rpd_ローズ顔表示
//		@rp_PC_Ｃ脚食堂ダンボール近く＿ヒント２
//	}
}

block codec RPC_ダンボール地帯 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_Ｃ脚食堂ダンボール近く聞いた) {
		@rpc_PC_Ｃ脚食堂ダンボール近く
		eval($f:rfp_PC_Ｃ脚食堂ダンボール近く聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_Ｃ脚食堂ダンボール近く
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}
proc rpc_PC_Ｃ脚厨房扉前 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｃ脚厨房扉前
}

block codec RPC_天岩戸 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_Ｃ脚厨房扉前聞いた) {
		@rpc_PC_Ｃ脚厨房扉前
		eval($f:rfp_PC_Ｃ脚厨房扉前聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_Ｃ脚厨房扉前
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_Ｄ脚シェル２扉前 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｄ脚シェル２扉前１
	if($w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１開始) {
		@rp_PC_Ｃ脚シェル２扉前２
	}
}



block codec RPC_w18a_シェル２への扉 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_Ｄ脚シェル２扉前聞いた) {
		@rpc_PC_Ｄ脚シェル２扉前
		eval($f:rfp_PC_Ｄ脚シェル２扉前聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_Ｄ脚シェル２扉前
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_Ｄ脚＿ハッチ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｄ脚＿ハッチ
}


block codec RPC_Ｄ脚＿ハッチ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_Ｄ脚＿ハッチ聞いた) {
		@rpc_PC_Ｄ脚＿ハッチ
		eval($f:rfp_PC_Ｄ脚＿ハッチ聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_Ｄ脚＿ハッチ
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

block codec RPC_Ｄ脚＿ハッチＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_PC_Ｄ脚＿ハッチ
	eval($f:rfp_PC_Ｄ脚＿ハッチ聞いた = 1 )
}

proc rpc_PC_Ｅ脚１Ｆ＿ベルトコンベア＿荷物に隠れろ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	if(!$f:rfp_PC_Ｅ脚１Ｆ＿ベルトコンベア＿荷物に隠れろ聞いた) {
		@rp_PC_Ｅ脚１Ｆ＿ベルトコンベア＿荷物に隠れろ１
		@rpd_ローズ顔表示
		@rp_PC_Ｅ脚１Ｆ＿ベルトコンベア＿荷物に隠れろ２
		eval($f:rfp_PC_Ｅ脚１Ｆ＿ベルトコンベア＿荷物に隠れろ聞いた = 1 )
	}
	@rp_PC_Ｅ脚１Ｆ＿ベルトコンベア＿荷物に隠れろ３
}


block codec RPC_大佐＿Ｅ脚１Ｆ＿デフォルト 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( ($f:rfp_w20a_ベルコンエリアにいますジミーはいません) && \
		(!$f:rfp_PC_Ｅ脚１Ｆ＿ベルトコンベア＿荷物に隠れろ聞いた) && \
		($w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始) ){
		@rpc_PC_Ｅ脚１Ｆ＿ベルトコンベア＿荷物に隠れろ
		eval($f:rfp_PC_Ｅ脚１Ｆ＿ベルトコンベア＿荷物に隠れろ聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 1) && ($f:rfp_w20a_ベルコンエリアにいますジミーはいません) && \
			($w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始) ){
			@rpc_PC_Ｅ脚１Ｆ＿ベルトコンベア＿荷物に隠れろ
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_ハリアー {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	@rp_PC_ハリアー１
	if(!$f:rfp_PC_ハリアー聞いた){
		@rp_PC_ハリアー２
		eval($f:rfp_PC_ハリアー聞いた = 1 )
	}
	@rp_PC_ハリアー３
}


block codec RPC_w20b_ハリアー 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_ハリアー聞いた){
		@rpc_PC_ハリアー
		eval($f:rfp_PC_ハリアー聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rpc_PC_ハリアー
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}


proc rpc_PC_鳥殺し過ぎ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_鳥殺し過ぎ
	@rp_ローズさんを怒らせました
}

block codec RPC_かもめさん殺戮ＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_PC_鳥殺し過ぎ
}



proc rpc_エレベータ内部 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_シェル１中央棟１Ｆ＿エレベータ内＿使い方聞いた){
		@rp_PC_シェル１中央棟１Ｆ＿エレベータ内＿使い方
		eval($f:rfp_PC_シェル１中央棟１Ｆ＿エレベータ内＿使い方聞いた = 1 )

	}else if(!$f:rfp_PC_エレベータ敵兵動かない聞いた) {
		@rp_PC_エレベータ敵兵動かない
		eval($f:rfp_PC_エレベータ敵兵動かない聞いた = 1)

	}else if(!$f:rfp_PC_シェル１中央棟１Ｆ＿エレベータ＿危険聞いた) {
		@rp_PC_シェル１中央棟１Ｆ＿エレベータ＿危険
		eval($f:rfp_PC_シェル１中央棟１Ｆ＿エレベータ＿危険聞いた = 1 )

	}else {
		rand 3
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rp_PC_シェル１中央棟１Ｆ＿エレベータ内＿使い方

		}else if($w:rfp_rand < 2){
			@rp_PC_エレベータ敵兵動かない

		}else {
			@rp_PC_シェル１中央棟１Ｆ＿エレベータ＿危険
		}
	}
}

block codec RPC_エレベータ内部 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_シェル１中央棟１Ｆ＿エレベータ内＿使い方聞いた){
		@rpc_エレベータ内部
		eval($f:rfp_PC_シェル１中央棟１Ｆ＿エレベータ内＿使い方聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rpc_エレベータ内部
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_w24a_エレベータスイッチ前 {
	command エレベータ状態取得
	if($status == 0) {
		@rpd_ライデン顔設定デフォルト
		@rpd_大佐顔設定１
		@rpd_顔表示

		if(!$f:rfp_PC_シェル１中央棟１Ｆ＿エレベータ説明聞いた) {
			@rp_PC_シェル１中央棟１Ｆ＿エレベータ説明
			@rp_PC_シェル１中央棟１Ｆ＿エレベータ説明１
			eval($f:rfp_PC_シェル１中央棟１Ｆ＿エレベータ説明聞いた = 1)

		}else if(!$f:rfp_PC_シェル１中央棟１Ｆ＿エレベータ＿チャフ状態聞いた) {
			@rp_PC_シェル１中央棟１Ｆ＿エレベータ＿チャフ状態
			eval($f:rfp_PC_シェル１中央棟１Ｆ＿エレベータ＿チャフ状態聞いた = 1)
			eval($f:rfp_PC_w24a_エレベータスイッチ前終了 = 1)
			
		}else {
			rand 2
			eval($w:rfp_rand = $status)
			if($w:rfp_rand < 1){
				@rp_PC_シェル１中央棟１Ｆ＿エレベータ説明２
			}else {
				@rp_PC_シェル１中央棟１Ｆ＿エレベータ＿チャフ状態
			}
		}
	}else {
		@rpc_大佐デフォルト
	}
}


block codec RPC_w24a_エレベータスイッチ前 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_w24a_エレベータスイッチ前終了){
		@rpc_w24a_エレベータスイッチ前

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rpc_w24a_エレベータスイッチ前
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_網膜センサー＿説明 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_網膜センサー＿説明聞いた){
		@rp_PC_網膜センサー＿説明１
		eval($f:rfp_PC_網膜センサー＿説明聞いた = 1)
	}
	@rp_PC_網膜センサー＿説明２
}

proc rpc_PC_網膜センサー＿壊れた {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_網膜センサー＿壊れた
}

block codec RPC_w24b_網膜センサー前 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐

	if($w:p_story < d:ST:P040_09_P05エイムズ発見９ポリゴンデモ５終了) {
		if( (!$f:rfp_PC_網膜センサー＿自分でのぞいた聞いた) && \
					($f:rfp_w24b_網膜センサー＿ライデンが自分で覗きやがったぶりんちょ) ) {
			@rpd_ライデン顔設定デフォルト
			@rpd_大佐顔設定１
			@rpd_顔表示

			@rp_PC_網膜センサー＿自分でのぞいた
			eval($f:rfp_PC_網膜センサー＿自分でのぞいた聞いた = 1)
		}else if(!$f:rfp_PC_網膜センサー＿説明聞いた){
			@rpc_PC_網膜センサー＿説明
			eval($f:rfp_PC_網膜センサー＿説明聞いた = 1)


		}else if( (!$f:rfp_PC_網膜センサー終了) && (!$f:rfp_w24b_網膜センサー通過した) ){
			@rpc_網膜センサー

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 4
			eval($w:rfp_rand = $status)
			if($w:rfp_rand < 1) {
				@rpc_PC_網膜センサー＿説明

			}else if( ($w:rfp_rand < 3) && (!$f:rfp_w24b_網膜センサー通過した) ){
				@rpc_網膜センサー

			}else {
				@rpc_大佐デフォルト
			}

		//連続無線
		}else {
			@rpc_大佐デフォルト
		}


	}else {
		if(!$f:rfp_PC_網膜センサー＿壊れた聞いた){
			@rpc_PC_網膜センサー＿壊れた
			eval($f:rfp_PC_網膜センサー＿壊れた聞いた = 1)

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 4
			eval($w:rfp_rand = $status)
			if($w:rfp_rand < 1) {
				@rpc_PC_網膜センサー＿壊れた
			}else {
				@rpc_大佐デフォルト
			}

		//連続無線
		}else {
			@rpc_大佐デフォルト
		}
	}
}

block codec RPC_w24b_網膜センサー＿ライデンが自分で覗きやがったぶりんちょＣＡＬＬ 14085 d:大佐無線１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_網膜センサー＿自分でのぞいた
}


block codec RPC_w24a_エレベータ前ＣＡＬＬ 14085 d:大佐無線１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_シェル１中央棟１Ｆ＿エレベータ説明
	@rp_PC_シェル１中央棟１Ｆ＿エレベータ説明１
	eval($f:rfp_PC_シェル１中央棟１Ｆ＿エレベータ説明聞いた = 1)
}

proc rpc_PC_シェル１中央棟Ｂ２＿オウム {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_シェル１中央棟Ｂ２＿オウム
}

proc rpc_PC_シェル１中央棟Ｂ２＿エマ席 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_シェル１中央棟Ｂ２＿エマ席
}


block codec RPC_w24d_エマ席付近 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rp_ローズさんご機嫌判定
	if( (!$f:rfp_PC_シェル１中央棟Ｂ２＿オウム聞いた) && \
		(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_シェル１中央棟Ｂ２＿オウム
		eval($f:rfp_PC_シェル１中央棟Ｂ２＿オウム聞いた = 1)

/*
	}else if(!$f:rfp_PC_シェル１中央棟Ｂ２＿エマ席聞いた){
		@rpc_PC_シェル１中央棟Ｂ２＿エマ席
		eval($f:rfp_PC_シェル１中央棟Ｂ２＿エマ席聞いた = 1 )
*/

	}else {
		@rpc_大佐デフォルト
	}
}

block codec RPC_w24d_オタク席付近 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rp_ローズさんご機嫌判定
	if(!$f:rfp_PC_シェル１中央棟Ｂ２＿エマ席聞いた){
		@rpc_PC_シェル１中央棟Ｂ２＿エマ席
		eval($f:rfp_PC_シェル１中央棟Ｂ２＿エマ席聞いた = 1 )

	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_シェル１中央棟Ｂ２＿ＡＧ扉 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_シェル１中央棟Ｂ２＿ＡＧ扉聞いた){
		@rp_PC_シェル１中央棟Ｂ２＿ＡＧ扉１
		eval($f:rfp_PC_シェル１中央棟Ｂ２＿ＡＧ扉聞いた = 1 )
		@rpd_大佐顔表示
	}
	@rp_PC_シェル１中央棟Ｂ２＿ＡＧ扉２
}


block codec RPC_w24d_ＡＧ扉前 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_シェル１中央棟Ｂ２＿ＡＧ扉聞いた){
		@rpc_PC_シェル１中央棟Ｂ２＿ＡＧ扉
		eval($f:rfp_PC_シェル１中央棟Ｂ２＿ＡＧ扉聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_シェル１中央棟Ｂ２＿ＡＧ扉
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}


block codec RPC_w24c_人質イベント説明＿マイクありＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_人質＿あんたがエイムズ説明
	@rp_PC_人質＿見つかるな２
}


block codec RPC_w24c_人質イベント説明＿マイクなしＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_人質＿マイクない
}


proc rpc_PC_体萌え注意{
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	@rp_PC_体萌え注意
}

block codec RPC_炎近く 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:rfp_PC_体萌え注意聞いた) && ($f:rfp_そこの炎発生した) ){
		@rpc_PC_体萌え注意
		eval($f:rfp_PC_体萌え注意聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && ($f:rfp_そこの炎発生した) ){
			@rpc_PC_体萌え注意
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_ＤＧ連絡橋＿渡り方飛びズバリ１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	@rp_PC_ＤＧ連絡橋＿渡り方飛びズバリ１
}

block codec RPC_ＤＧ連絡橋＿渡り方飛びズバリ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	//ズバリヒント出す処理。時間？ＸＸＸＸＸＸＸＸＸＸｘｘ
	if( (!$f:rfp_PC_ＤＧ連絡橋＿渡り方飛びズバリ１聞いた) && \
			( ( ($i:プレイタイム - $i:イベント開始時刻) / d:FRAME_RATE) >= 300 ) ){
		@rpc_PC_ＤＧ連絡橋＿渡り方飛びズバリ１
		eval($f:rfp_PC_ＤＧ連絡橋＿渡り方飛びズバリ１聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && \
			( ( ($i:プレイタイム - $i:イベント開始時刻) / d:FRAME_RATE) >= 300 ) ){
			@rpc_PC_ＤＧ連絡橋＿渡り方飛びズバリ１

		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_ＤＧ連絡橋＿渡り方エルードズバリ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	if( (!$f:rfp_PC_ＤＧ連絡橋＿渡り方エルードズバリ聞いた) && \
			( ( ($i:プレイタイム - $i:イベント開始時刻) / d:FRAME_RATE) >= 300 ) ){
		@rp_PC_ＤＧ連絡橋＿渡り方エルードズバリ１
		@rpd_大佐顔表示
		eval($f:rfp_PC_ＤＧ連絡橋＿渡り方エルードズバリ聞いた = 1 )
	}
	@rp_PC_ＤＧ連絡橋＿渡り方エルードズバリ２
}


block codec RPC_ＤＧ連絡橋＿渡り方エルードズバリ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	//ズバリヒント出す処理。時間？ＸＸＸＸＸＸＸＸＸＸｘｘ
	if(!$f:rfp_PC_ＤＧ連絡橋＿渡り方エルードズバリ聞いた){
		@rpc_PC_ＤＧ連絡橋＿渡り方エルードズバリ
		eval($f:rfp_PC_ＤＧ連絡橋＿渡り方エルードズバリ聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && \
			( ( ($i:プレイタイム - $i:イベント開始時刻) / d:FRAME_RATE) >= 300 ) ){
			@rpc_PC_ＤＧ連絡橋＿渡り方エルードズバリ

		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}


proc rpc_PC_ＬＧ連絡橋＿ハシゴ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	@rp_PC_ＬＧ連絡橋＿ハシゴ
}

block codec RPC_ＬＧ連絡橋＿ハシゴ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	//ズバリヒント出す処理。時間？ＸＸＸＸＸＸＸＸＸＸｘｘ
	if(!$f:rfp_PC_ＬＧ連絡橋＿ハシゴ聞いた){
		@rpc_PC_ＬＧ連絡橋＿ハシゴ
		eval($f:rfp_PC_ＬＧ連絡橋＿ハシゴ聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_ＬＧ連絡橋＿ハシゴ
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}


proc rpc_PC_Ｌ脚外周＿窓 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	if(!$f:rfp_PC_Ｌ脚外周＿窓聞いた){
		@rp_PC_Ｌ脚外周＿窓１
		@rp_PC_Ｌ脚外周＿窓２
		eval($f:rfp_PC_Ｌ脚外周＿窓聞いた = 1 )
	}else {
		rand 2
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PC_Ｌ脚外周＿窓１
		}else {
			@rp_PC_Ｌ脚外周＿窓２
		}
	}
}

proc rp_Ｌ脚外周＿窓敵兵さんお元気です {
	//つまり新ノーマル以上
	if ($w:ゲーム設定 >= d:LEVEL_EASY) {
		if( (`@rp_敵兵元気さんです 敵兵:01`) || (`@rp_敵兵元気さんです 敵兵:02`) ){
			return 1
		}else {
			return 0
		}
	//早い話が新ぃぃぢぃ以上
	}else  {
		if(`@rp_敵兵元気さんです 敵兵:02`){
			return 1
		}else {
			return 0
		}
	}
}

block codec RPC_Ｌ脚外周＿窓 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:rfp_PC_Ｌ脚外周＿窓聞いた) && ($w:アラートモード != d:ALERT_MODE_ALERT) && \
		(`@rp_Ｌ脚外周＿窓敵兵さんお元気です`) ){
		@rpc_PC_Ｌ脚外周＿窓
		eval($f:rfp_PC_Ｌ脚外周＿窓聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3)  && ($w:アラートモード != d:ALERT_MODE_ALERT) && \
			(`@rp_Ｌ脚外周＿窓敵兵さんお元気です`) ){
			@rpc_PC_Ｌ脚外周＿窓
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_Ｌ脚外周＿張り付き {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	@rp_PC_Ｌ脚外周＿張り付き
}



block codec RPC_Ｌ脚外周＿張り付き 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_Ｌ脚外周＿張り付き聞いた){
		@rpc_PC_Ｌ脚外周＿張り付き
		eval($f:rfp_PC_Ｌ脚外周＿張り付き聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_Ｌ脚外周＿張り付き
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}


proc rpc_PC_Ｌ脚外周＿張り付き＿しゃがみ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	@rp_PC_Ｌ脚外周＿張り付き＿しゃがみ
}

block codec RPC_Ｌ脚外周＿しゃがみ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_Ｌ脚外周＿張り付き＿しゃがみ聞いた){
		@rpc_PC_Ｌ脚外周＿張り付き＿しゃがみ
		eval($f:rfp_PC_Ｌ脚外周＿張り付き＿しゃがみ聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_Ｌ脚外周＿張り付き＿しゃがみ
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}



proc rpc_PC_ＫＬ連絡橋＿Ｋ脚入口 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	if($w:p_story < d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了) {
		@rp_PC_ＫＬ連絡橋＿Ｋ脚入口
	}else {
		@rp_PC_ＫＬ連絡橋＿エマ後＿Ｋ脚入り口
	}
}

block codec RPC_ＫＬ連絡橋＿Ｋ脚入口 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_ＫＬ連絡橋＿Ｋ脚入口聞いた){
		@rpc_PC_ＫＬ連絡橋＿Ｋ脚入口
		eval($f:rfp_PC_ＫＬ連絡橋＿Ｋ脚入口聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_ＫＬ連絡橋＿Ｋ脚入口
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}



proc rpc_PC_シェル２中央棟１Ｆ＿電撃床破壊前 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_シェル２中央棟１Ｆ＿電撃床破壊前
}

proc rpc_PC_シェル２中央棟１Ｆ＿電撃床破壊後 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_シェル２中央棟１Ｆ＿電撃床破壊後
}

block codec RPC_シェル２中央棟１Ｆ＿電撃床 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( ($w:p_story < d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１終了 ) && \
		(!$f:rfp_PC_シェル２中央棟１Ｆ＿電撃床破壊前聞いた) ){
		@rpc_PC_シェル２中央棟１Ｆ＿電撃床破壊前
		eval($f:rfp_PC_シェル２中央棟１Ｆ＿電撃床破壊前聞いた = 1 )

	}else if( (d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１終了 <= $w:p_story) && \
				($w:p_story < d:ST:P049_01_P01大統領１ポリゴンデモ１開始) && \
				(!$f:rfp_PC_シェル２中央棟１Ｆ＿電撃床破壊後聞いた) ){
			@rpc_PC_シェル２中央棟１Ｆ＿電撃床破壊後
			eval($f:rfp_PC_シェル２中央棟１Ｆ＿電撃床破壊後聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			if($w:p_story < d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１終了 ) {
				@rpc_PC_シェル２中央棟１Ｆ＿電撃床破壊前

			}else if( d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１終了 <= $w:p_story && \
						$w:p_story < d:ST:P049_01_P01大統領１ポリゴンデモ１開始 ){
				@rpc_PC_シェル２中央棟１Ｆ＿電撃床破壊後

			}else {
				@rpc_大佐デフォルト
			}

		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}


proc rpc_PC_シェル２中央棟１Ｆ＿暗い {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	@rp_PC_シェル２中央棟１Ｆ＿暗い１
	/* やべＸＸＸＸＸＸＸＸＸｘ
	if( ($w:アイテム数Ｒ[d:アイテム:暗視ゴーグル] < 0) && \
		($w:アイテム数Ｒ[d:アイテム:サーマルゴーグル] < 0) && \
		(!$f:rfp_PS_){
		@rp_PC_シェル２中央棟１Ｆ＿暗い２
	}
	*/
}

block codec RPC_シェル２中央棟１Ｆ＿暗い 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_シェル２中央棟１Ｆ＿暗い聞いた){
		@rpc_PC_シェル２中央棟１Ｆ＿暗い
		eval($f:rfp_PC_シェル２中央棟１Ｆ＿暗い聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rpc_PC_シェル２中央棟１Ｆ＿暗い
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}



proc rpc_PC_シェル２中央棟１Ｆ＿ダクト {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_シェル２中央棟１Ｆ＿ダクト
}


proc rpc_PC_エマ救出後＿ダクト蓋 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エマ救出後＿ダクト蓋
}

block codec RPC_シェル２中央棟１Ｆ＿ダクト前 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( ($w:p_story < d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１終了 ) && \
		(!$f:rfp_PC_シェル２中央棟１Ｆ＿ダクト聞いた) ){
		@rpc_PC_シェル２中央棟１Ｆ＿ダクト
		eval($f:rfp_PC_シェル２中央棟１Ｆ＿ダクト聞いた = 1 )

	}else if( ($w:武器弾数Ｒ[6] >= 0) && (!$f:rfp_PC_リモコンミサイル＿ＶＲ類似聞いた) && \
				($w:p_story < d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１終了 ) ){
		@rpc_PC_リモコンミサイル＿ＶＲ類似
		eval($f:rfp_PC_リモコンミサイル＿ＶＲ類似聞いた = 1)
/*
	//ダクト蓋条件って？ＸＸＸＸＸＸＸＸＸＸｘ
	}else if( (d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了 <= $w:p_story) && \
				(!$f:rfp_PC_エマ救出後＿ダクト蓋聞いた) ){
			@rpc_PC_エマ救出後＿ダクト蓋
			eval($f:rfp_PC_エマ救出後＿ダクト蓋聞いた = 1)
*/
	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			if($w:p_story < d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１終了 ) {
				@rpc_PC_シェル２中央棟１Ｆ＿ダクト
/*
			}else if(d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了 <= $w:p_story){
				@rpc_PC_エマ救出後＿ダクト蓋
*/
			}else {
				@rpc_大佐デフォルト
			}

		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_シェル２中央棟Ｂ１＿水 {
	@rp_PC_シェル２中央棟Ｂ１＿水
	if(!$f:rfp_水中入った){
		@rp_PC_シェル２中央棟Ｂ１＿水＿泳いでない
	}else {
		@rp_PC_シェル２中央棟Ｂ１＿水＿泳いだ
	}
}


proc rpc_初入水＿水上ＣＡＬＬ共通 {
	@rp_PC_水中＿水面＿移動
	@rp_PC_水中＿水面＿潜航２
	eval($f:rfp_PC_水中＿操作説明聞いた = 1)
	eval($f:rfp_PC_水中操作説明＿水上聞いた = 1)
}

block codec RPC_w31b_初入水＿水上ＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_シェル２中央棟Ｂ１＿水聞いた){
		@rpc_PC_シェル２中央棟Ｂ１＿水
		eval($f:rfp_PC_シェル２中央棟Ｂ１＿水聞いた = 1)
	}
	@rpc_初入水＿水上ＣＡＬＬ共通
}


proc rpc_w11ab_初入水＿水上ＣＡＬＬ＿共通 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_水中＿操作説明聞いた){
		@rp_PC_水中＿操作説明
		eval($f:rfp_PC_水中＿操作説明聞いた = 1)
	}
	@rpc_初入水＿水上ＣＡＬＬ共通
}

block codec RPC_w11ab_初入水＿水上ＣＡＬＬ＿潜水 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11ab_初入水＿水上ＣＡＬＬ＿共通
}

block codec RPC_w11ab_初入水＿水上ＣＡＬＬ＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11ab_初入水＿水上ＣＡＬＬ＿共通
}

proc rpc_初入水＿水中ＣＡＬＬ共通 {
	@rp_PC_水中＿前進
	//こんふぃぐはなし？ＸＸＸＸＸＸＸｘ
	@rp_PC_水中＿左スティック＿正
	@rp_PC_水中＿Ｏ２
	eval($f:rfp_PC_水中＿操作説明聞いた = 1)
	eval($f:rfp_PC_水中操作説明＿水中聞いた = 1)
}

block codec RPC_w31b_初入水＿水中ＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_シェル２中央棟Ｂ１＿水聞いた){
		@rpc_PC_シェル２中央棟Ｂ１＿水
		eval($f:rfp_PC_シェル２中央棟Ｂ１＿水聞いた = 1)
	}
	@rpc_初入水＿水中ＣＡＬＬ共通
}

proc rpc_w11a_初入水＿水中ＣＡＬＬ＿共通 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_水中＿操作説明聞いた){
		@rp_PC_水中＿操作説明
		eval($f:rfp_PC_水中＿操作説明聞いた = 1)
	}
	@rpc_初入水＿水中ＣＡＬＬ共通
}

block codec RPC_w11a_初入水＿水中ＣＡＬＬ＿潜水 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11a_初入水＿水中ＣＡＬＬ＿共通
}

block codec RPC_w11a_初入水＿水中ＣＡＬＬ＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_w11a_初入水＿水中ＣＡＬＬ＿共通
}



block codec RPC_大佐デフォルト＿泳ぎ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( ($w:p_story >= d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了) && \
		(!$f:エマ存在フラグ) ){
			@rpc_PC_エマ救出後＿エマおいてけぼり

	}else if( (d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了 <= $w:p_story && \
				$w:p_story <  d:ST:P059_01_P01エマ休憩１ポリゴンデモ１開始 ) && \
			(!$f:rfp_PC_エマ連れてけ聞いた) ){
		@rpc_PC_エマ連れてけ
		@rp_PC_エマ救出後＿第二濾過槽戻れ
		eval($f:rfp_PC_エマ連れてけ聞いた = 1)

	}else {
		@rpc_水中＿デフォルト
	}
}

proc rpc_PC_水中＿水中機雷 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_水中＿水中機雷
}

block codec RPC_水中機雷近く 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:rfp_PC_水中＿水中機雷聞いた) && (!$f:rfp_そこの機雷爆発した) ){
		@rpc_PC_水中＿水中機雷
		eval($f:rfp_PC_水中＿水中機雷聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && (!$f:rfp_そこの機雷爆発した) ){
			@rpc_PC_水中＿水中機雷
		}else {
			@rpc_水中＿デフォルト
		}

	//連続無線
	}else {
		@rpc_水中＿デフォルト
	}
}


proc rpc_PC_水中＿水中開かない水密扉 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_水中＿水中開かない水密扉
}


block codec RPC_水中開かない水密扉 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:rfp_PC_水中＿水中開かない水密扉聞いた) && ($f:rfp_そこの水密扉開けた) ){
		@rpc_PC_水中＿水中開かない水密扉
		eval($f:rfp_PC_水中＿水中開かない水密扉聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && ($f:rfp_そこの水密扉開けた) ){
			@rpc_PC_水中＿水中開かない水密扉
		}else {
			@rpc_水中＿デフォルト
		}

	//連続無線
	}else {
		@rpc_水中＿デフォルト
	}
}

proc rpc_PC_水中＿水中水密扉 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	@rp_PC_水中＿水中水密扉
}


block codec RPC_水中開く水密扉 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:rfp_PC_水中＿水中水密扉聞いた) && (!$f:rfp_そこの水密扉開けた) ){
		@rpc_PC_水中＿水中水密扉
		eval($f:rfp_PC_水中＿水中水密扉聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && (!$f:rfp_そこの水密扉開けた) ){
			@rpc_PC_水中＿水中水密扉
		}else {
			@rpc_水中＿デフォルト
		}

	//連続無線
	}else {
		@rpc_水中＿デフォルト
	}
}

block codec RPC_エマ救出前＿エマ部屋 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_ヴァンプ戦後＿ロッカー室聞いた){
		@rpc_PC_ヴァンプ戦後＿ロッカー室
	
	}else {
		rand 5
		eval($w:rfp_rand = $status)

		if( ($w:rfp_rand < 4) && ($w:rfp_callcount <= 1) ){
			@rpc_PC_ヴァンプ戦後＿ロッカー室
		
		}else {
			@rpc_連続無線デフォルト
		}
	}
}

proc rpc_PC_水中＿カード扉 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	@rp_PC_水中＿カード扉
}

block codec RPC_水中＿カード扉 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_水中＿カード扉聞いた){
		@rpc_PC_水中＿カード扉
		eval($f:rfp_PC_水中＿カード扉聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_水中＿カード扉
		}else {
			@rpc_水中＿デフォルト
		}

	//連続無線
	}else {
		@rpc_水中＿デフォルト
	}
}

proc rpc_PC_エマのぞき {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エマのぞき
}



block codec RPC_エマのぞき 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (`%プレイヤー状態取得` & d:PFLAG_WATER_SURFACE) && ($f:rfp_w31c_エマのぞいた) && \
		(!$f:rfp_PC_エマのぞき聞いた) ){
		@rpc_PC_エマのぞき
		eval($f:rfp_PC_エマのぞき聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && \
			(`%プレイヤー状態取得` & d:PFLAG_WATER_SURFACE) && ($f:rfp_w31c_エマのぞいた) ){
			@rpc_PC_エマのぞき
		}else {
			@rpc_水中＿デフォルト
		}

	//連続無線
	}else {
		@rpc_水中＿デフォルト
	}
}


block codec RPC_w31c_エマのぞきＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_ローズ顔設定１
	@rpd_顔表示

	@rp_PC_エマのぞき＿執拗
	eval($f:rfp_w31c_エマのぞきＣＡＬＬ聞いた = 1)
	@rp_ローズさんを怒らせました
}

block codec RPC_w31c_エマと初出水ＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エマ救出後＿Ｌ脚向かえ
	@rp_PC_エマ手つなぎ＿手つなぎ
}

block codec RPC_w32a_ハシゴＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｌ脚下部＿ハシゴ
	eval($f:rfp_w32a_ハシゴＣＡＬＬ聞いた = 1)
}


proc rpc_PC_Ｅ脚ハシゴ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｅ脚ハシゴ
}


block codec RPC_Ｅ脚ハシゴ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_エマ狙撃後＿電算室行け聞いた) {
		@rpc_PC_エマ狙撃後＿電算室行け
		eval($f:rfp_PC_エマ狙撃後＿電算室行け聞いた = 1)

	}else if(!$f:rfp_PC_Ｅ脚ハシゴ聞いた){
		@rpc_PC_Ｅ脚ハシゴ
		eval($f:rfp_PC_Ｅ脚ハシゴ聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_Ｅ脚ハシゴ
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_拷問部屋＿開かない扉 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_拷問部屋＿開かない扉
}

block codec RPC_拷問部屋＿開かない扉 14085 d:大佐無線３裸ライデン {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_拷問部屋＿開かない扉聞いた){
		@rpc_PC_拷問部屋＿開かない扉
		eval($f:rfp_PC_拷問部屋＿開かない扉聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_拷問部屋＿開かない扉
		}else {
//			@rpc_大佐デフォルト
			@rpc_裸大佐＿デフォルト
		}

	//連続無線
	}else {
//		@rpc_大佐デフォルト
		@rpc_裸大佐＿デフォルト
	}
}



proc rpc_PC_拷問部屋＿ノード {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_拷問部屋＿ノード
}


block codec RPC_w41a_ノード近く 14085 d:大佐無線３裸ライデン {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:w41a_ノードフラグ) && (!$f:rfp_PC_拷問部屋＿ノード聞いた)  && \
		(!($w:コンフィグ設定 & d:CONFIG_RADAR_OFF) ) ){
		@rpc_PC_拷問部屋＿ノード
		eval($f:rfp_PC_拷問部屋＿ノード聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && (!$f:w41a_ノードフラグ) && \
			(!($w:コンフィグ設定 & d:CONFIG_RADAR_OFF) ) ){
			@rpc_PC_拷問部屋＿ノード
		}else {
//			@rpc_大佐デフォルト
			@rpc_裸大佐＿デフォルト
		}

	//連続無線
	}else {
//		@rpc_大佐デフォルト
		@rpc_裸大佐＿デフォルト
	}
}

block codec RPC_w11a_海底ドック危険モード回避のススメＣＡＬＬ 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_海底ドック危険モード回避のススメ１
	if($w:p_story >= d:ST:P004_02_R01ノード初接続２無線デモ１終了){
		@rpd_ローズ顔表示
		@rp_PC_海底ドック危険モード回避のススメ２
	}
	eval($f:rfp_PC_海底ドック危険モード回避のススメ聞いた =1 )
}

block codec RPC_w11a_海底ドック催促ＣＡＬＬ 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_海底ドック催促
	eval($f:rfp_PC_海底ドック催促聞いた =1 )
}

block codec RPC_w11a_敵兵武器使用不可ＣＡＬＬ 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rpcc_PC_敵武器使用不可１
	@rpcc_PC_敵武器使用不可２
	@rp_PC_敵武器使用不可３
	eval($f:rfp_w11a_敵兵武器使用不可ＣＡＬＬ聞いた =1 )
}


block codec RPC_w11a_流血ＣＡＬＬ 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_流血説明２
	@rp_PC_流血説明３
	//しゃがんで流血回復ムービー ＸＸＸＸＸＸむび
	@rp_PC_流血説明４

	eval($f:rfp_PC_流血説明聞いた = 1)
}

proc rpc_PC_海底ドックＶＲ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rpcc_PC_海底ドックＶＲ１
	@rpcc_PC_海底ドックＶＲ２
}

block codec RPC_w11a＿海底ドックＶＲ 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:rfp_PC_海底ドックＶＲ聞いた) && \
		($w:p_story < d:ST:P004_01_P01ノード初接続１ポリゴンデモ１開始) ){
		@rpc_PC_海底ドックＶＲ
		eval($f:rfp_PC_海底ドックＶＲ聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
//		@rpc_大佐デフォルト普通潜水
		@rpc_海底ドックデフォルト

	//連続無線
	}else {
//		@rpc_大佐デフォルト普通潜水
		@rpc_海底ドックデフォルト
	}
}


block codec RPC_w11a_クリアリングＣＡＬＬ 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_海底ドッククリアリング中ＣＡＬＬ

	eval($f:rfp_w11a_クリアリングＣＡＬＬ聞いた = 1)
}

block codec RPC_w12b_クリアリングＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_海底ドッククリアリング中ＣＡＬＬ

	eval($f:rfp_w12b_クリアリングＣＡＬＬ聞いた = 1)
}


block codec RPC_ＬＩＦＥゼロ寸前ＣＡＬＬ 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＬＩＦＥゼロ寸前ＣＡＬＬ１

	if($w:アイテム数Ｒ[d:アイテム:レーション] <= 0){
		@rp_PC_ＬＩＦＥゼロ寸前ＣＡＬＬ＿レーション探せ
	}else {
		@rp_PC_ＬＩＦＥゼロ寸前ＣＡＬＬ＿レーション使え
	}
	eval($f:rfp_ＬＩＦＥゼロ寸前ＣＡＬＬ聞いた = 1)
}


block codec RPC_レーション取得時ＣＡＬＬ＿潜水 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_レーション
	eval($f:rfp_PC_レーション聞いた = 1)
}

block codec RPC_レーション取得時ＣＡＬＬ＿普通 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_レーション
	eval($f:rfp_PC_レーション聞いた = 1)
}


block codec RPC_w12b_敵兵倒したＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ａ脚１Ｆ死体隠せ１
	@rp_PC_Ａ脚１Ｆ死体隠せ２
	if(!$f:rfp_PC_死体引きずり方法聞いた){
		@rpc_PC_死体引きずり方法
		eval($f:rfp_PC_死体引きずり方法聞いた = 1)
	}
}


block codec RPC_w24c_人質殴ったＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_PC_人質＿人質殴った
}


proc rpc_PC_ＡＢ連絡橋見つかるな注意 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＡＢ連絡橋見つかるな注意
}

block codec RPC_連絡橋＿大佐デフォルト  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( ($s:エリア == "w13a") && \
		(!$f:rfp_PC_ＡＢ連絡橋見つかるな注意聞いた) && \
		($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) ){
		@rpc_PC_ＡＢ連絡橋見つかるな注意
		eval($f:rfp_PC_ＡＢ連絡橋見つかるな注意聞いた = 1 )

	}else if( ($w:武器弾数Ｒ[d:武器:ニキータ] >= 0) && \
				($w:武器 == d:武器:ニキータ) && \
				(!$f:rfp_PC_リモコンミサイル使えない聞いた) ){
			@rpc_PC_リモコンミサイル使えない
			eval($f:rfp_PC_リモコンミサイル使えない聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 1) && ($s:エリア == "w13a") && \
			($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) ){
			@rpc_PC_ＡＢ連絡橋見つかるな注意

		}else if( ($w:rfp_rand < 3) && ($w:武器 == d:武器:ニキータ) ){
			@rpc_PC_リモコンミサイル使えない
			
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

/*
PC_海鳥１
PC_ＤＧ連絡橋＿海鳥攻撃
PC_海鳥糞２
PC_狙撃＿海鳥撃つな１
PC_狙撃＿海鳥追い払う方法ヒント
PC_狙撃＿海鳥追い払う方法ズバリ
*/


block codec RPC_連絡橋かもめ＿大佐デフォルト  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( ($s:エリア == "w13a") && \
		(!$f:rfp_PC_ＡＢ連絡橋見つかるな注意聞いた) && \
		($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) ){
		@rpc_PC_ＡＢ連絡橋見つかるな注意
		eval($f:rfp_PC_ＡＢ連絡橋見つかるな注意聞いた = 1 )

	}else if( (!$f:rfp_海鳥うんちく聞いた) && ($w:rfp_callcount > 1) ){
		@rpc_海鳥うんちく
		eval($f:rfp_海鳥うんちく聞いた = 1)

	//海鳥が襲ってくるところは？？？

	}else if( ($w:武器弾数Ｒ[d:武器:ニキータ] >= 0) && \
				($w:武器 == d:武器:ニキータ) && \
				($w:rfp_callcount > 1) && \
				(!$f:rfp_PC_リモコンミサイル使えない聞いた) ){
			@rpc_PC_リモコンミサイル使えない
			eval($f:rfp_PC_リモコンミサイル使えない聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 1) && ($s:エリア == "w13a") && \
			($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) ){
			@rpc_PC_ＡＢ連絡橋見つかるな注意

		}else if( ($w:rfp_rand < 3) && ($w:武器 == d:武器:ニキータ) && ($w:rfp_callcount > 1) ){
			@rpc_PC_リモコンミサイル使えない
			
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_サイファー１ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_サイファー１
}

proc rpc_PC_サイファー回避＿エルード {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_サイファー回避＿エルード聞いた){
		@rp_PC_サイファー回避＿エルード１
		@rpd_大佐顔表示
		eval($f:rfp_PC_サイファー回避＿エルード聞いた = 1)
	}
	@rp_PC_サイファー回避＿エルード２
}

proc rpc_PC_サイファー回避＿破壊 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_サイファー回避＿破壊１
	//仕様ツーボー
//	@rp_PC_サイファー回避＿破壊２
}

proc rpc_PC_サイファー回避＿チャフ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示
	
	@rp_PC_サイファー回避＿チャフ
}




proc rpc_サイファー連絡橋デフォルト {
	@rp_そこのサイファー存在するか判定

	if( ($f:rfp_そこのサイファー存在してます) && \
		(!$f:rfp_PC_サイファー１聞いた) ){
		@rpc_PC_サイファー１
		eval($f:rfp_PC_サイファー１聞いた = 1)

	}else if(($f:rfp_そこのサイファー存在してます) && \
					(!$f:rfp_PC_サイファー回避＿エルード聞いた) ){
		@rpc_PC_サイファー回避＿エルード
		eval($f:rfp_PC_サイファー回避＿エルード聞いた = 1)


	}else if(($f:rfp_そこのサイファー存在してます) && \
				(!$f:rfp_PC_サイファー回避＿チャフ聞いた) ){
		@rpc_PC_サイファー回避＿チャフ
		eval($f:rfp_PC_サイファー回避＿チャフ聞いた = 1)

	}else if(($f:rfp_そこのサイファー存在してます) && \
				(!$f:rfp_PC_サイファー回避＿破壊聞いた) ){
		@rpc_PC_サイファー回避＿破壊
		eval($f:rfp_PC_サイファー回避＿破壊聞いた = 1)
	

	}else if( ($w:武器弾数Ｒ[d:武器:ニキータ] >= 0) && \
				($w:武器 == d:武器:ニキータ) && \
				(!$f:rfp_PC_リモコンミサイル使えない聞いた) ){
			@rpc_PC_リモコンミサイル使えない
			eval($f:rfp_PC_リモコンミサイル使えない聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 6
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 1) && ($w:武器 == d:武器:ニキータ) ){
			@rpc_PC_リモコンミサイル使えない
			
		}else if( ($w:rfp_rand < 2) && \
					($f:rfp_そこのサイファー存在してます) ){
			@rpc_PC_サイファー回避＿破壊

		}else if( ($w:rfp_rand < 3) &&  \
					($f:rfp_そこのサイファー存在してます) ){
			@rpc_PC_サイファー回避＿チャフ
		}else if( ($w:rfp_rand < 4) &&  \
					($f:rfp_そこのサイファー存在してます) ){
			@rpc_PC_サイファー回避＿エルード
		}else if( ($w:rfp_rand < 5) &&  \
					($f:rfp_そこのサイファー存在してます) ){
			@rpc_PC_サイファー１
		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}
block codec RPC_サイファー連絡橋＿大佐デフォルト  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_サイファー連絡橋デフォルト
}

block codec RPC_ＢＣ連絡橋壊れた橋 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_ＢＣ連絡橋壊れた橋聞いた) {
		@rpc_PC_ＢＣ連絡橋壊れた橋
		eval($f:rfp_PC_ＢＣ連絡橋壊れた橋聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_ＢＣ連絡橋壊れた橋
		}else {
			@rpc_サイファー連絡橋デフォルト
		}

	}else {
		@rpc_サイファー連絡橋デフォルト
	}
}



block codec RPC_ガンサイファーかもめ連絡橋＿大佐デフォルト  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐

	@rpc_ガンサイファーかもめ連絡橋＿大佐デフォルト
}


block codec RPC_w21a_地雷地帯 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:rfp_ＥＦ連絡橋＿クレイモア聞いた) && ($f:rfp_w21a_地雷原１無線デモ１した) && \
		(`@rp_w21_地雷地帯にクレイモアあります`) ){
		@rpc_ＥＦ連絡橋＿クレイモア
		eval($f:rfp_ＥＦ連絡橋＿クレイモア聞いた = 1)

	//無線一回目
	}else {
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && ($f:rfp_w21a_地雷原１無線デモ１した) && \
			(`@rp_w21_地雷地帯にクレイモアあります`) && \
			($w:rfp_callcount <= 1) ){
			@rpc_ＥＦ連絡橋＿クレイモア

		}else {
			@rpc_ガンサイファーかもめ連絡橋＿大佐デフォルト
		}
	}
}

proc rpc_PC_Ｌ脚＿開かない扉 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｌ脚＿開かない扉
}

block codec RPC_w28a_開かないドア  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:rfp_PC_Ｌ脚＿開かない扉聞いた) && ($f:エマ存在フラグ) ){
		@rpc_PC_Ｌ脚＿開かない扉
		eval($f:rfp_PC_Ｌ脚＿開かない扉聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && ($f:エマ存在フラグ) ){
			@rpc_PC_Ｌ脚＿開かない扉

		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_フォーチュン戦＿戻れない {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_フォーチュン戦＿戻れない
}

block codec RPC_フォーチュン戦＿戻れない  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_フォーチュン戦＿戻れない聞いた){
		@rpc_PC_フォーチュン戦＿戻れない
		eval($f:rfp_PC_フォーチュン戦＿戻れない聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_フォーチュン戦中

		}else {
			@rpc_フォーチュン戦中
		}

	//連続無線
	}else {
		@rpc_フォーチュン戦中
	}
}

proc rpc_PC_Ｃ脚男子トイレ便器主観 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｃ脚男子トイレ便器主観
}

proc rpc_PC_Ｃ脚男子トイレ便器主観しゃがみ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｃ脚男子トイレ便器主観しゃがみ
}

proc rpc_PC_Ｃ脚＿トイレ＿エロ本 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｃ脚＿トイレ＿エロ本
	@rp_ローズさんを怒らせました
}

block codec RPC_男子便所便器  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rp_ローズさんご機嫌判定
	if( (`%プレイヤー状態取得` & d:PFLAG_LOCKER) && \
		($w:武器 == d:武器:雑誌) && \
		(!$f:rfp_PC_Ｃ脚＿トイレ＿エロ本聞いた) ){
		@rpc_PC_Ｃ脚＿トイレ＿エロ本
		eval($f:rfp_PC_Ｃ脚＿トイレ＿エロ本聞いた = 1)

	}else if( ($f:rfp_w16_便器見てる) && \
		(!$f:rfp_PC_Ｃ脚男子トイレ便器主観聞いた) && \
		(!$f:rfp_PR_ローズさんが激怒してます！) ){
		@rpc_PC_Ｃ脚男子トイレ便器主観
		eval($f:rfp_PC_Ｃ脚男子トイレ便器主観聞いた = 1 )

	}else if( ($f:rfp_w16_便器見てる) && (`%プレイヤー状態取得` & d:PFLAG_SQUAT) && \
				(!$f:rfp_PC_Ｃ脚男子トイレ便器主観しゃがみ聞いた) ){
			@rpc_PC_Ｃ脚男子トイレ便器主観しゃがみ
			eval($f:rfp_PC_Ｃ脚男子トイレ便器主観しゃがみ聞いた = 1)

	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_Ｃ脚女子トイレ長居 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｃ脚女子トイレ長居
	@rp_ローズさんを怒らせました
	eval($w:rfp_エロ行為回数 = $w:rfp_エロ行為回数 +1)
}

proc rpc_PC_Ｃ脚女子トイレ便器主観 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｃ脚女子トイレ便器主観
	@rp_ローズさんを怒らせました
	eval($w:rfp_エロ行為回数 = $w:rfp_エロ行為回数 +1)
}

proc rpc_PC_Ｃ脚女子トイレ便器しゃがみ主観 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_Ｃ脚女子トイレ便器しゃがみ主観
	eval($w:rfp_エロ行為回数 = $w:rfp_エロ行為回数 +1)
}

block codec RPC_女子便所便器  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (`%プレイヤー状態取得` & d:PFLAG_LOCKER) && \
		($w:武器 == d:武器:雑誌) && \
		(!$f:rfp_PC_Ｃ脚＿トイレ＿エロ本聞いた) ){
		@rpc_PC_Ｃ脚＿トイレ＿エロ本
		eval($f:rfp_PC_Ｃ脚＿トイレ＿エロ本聞いた = 1)

	}else if( ($f:rfp_w16_便器見てる) && \
		(!$f:rfp_PC_Ｃ脚女子トイレ便器主観聞いた) ){
		@rpc_PC_Ｃ脚女子トイレ便器主観
		eval($f:rfp_PC_Ｃ脚女子トイレ便器主観聞いた = 1 )

	}else if( ($f:rfp_w16_便器見てる) && (`%プレイヤー状態取得` & d:PFLAG_SQUAT) && \
				(!$f:rfp_PC_Ｃ脚女子トイレ便器しゃがみ主観聞いた) ){
			@rpc_PC_Ｃ脚女子トイレ便器しゃがみ主観
			eval($f:rfp_PC_Ｃ脚女子トイレ便器しゃがみ主観聞いた = 1)

	}else if( ( ( ($i:プレイタイム - $i:rfp_w16_女子トイレ開始時刻) / d:FRAME_RATE) >= 120 ) && \
				(!$f:rfp_PC_Ｃ脚女子トイレ長居聞いた) ){
			@rpc_PC_Ｃ脚女子トイレ長居
			eval($f:rfp_PC_Ｃ脚女子トイレ長居聞いた = 1)

	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_ものは散らかさない {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_ものは散らかさない聞いた){
		@rp_PC_ものは散らかさない１
		eval($f:rfp_PC_ものは散らかさない聞いた = 1 )
	}
	@rp_PC_ものは散らかさない２
}


block codec RPC_コワレモノ近辺  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_ものは散らかさない聞いた){
		@rpc_PC_ものは散らかさない
		eval($f:rfp_PC_ものは散らかさない聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rpc_PC_ものは散らかさない

		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

block codec RPC_w18a_デフォルト  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	rand 4
	eval($w:rfp_rand = $status)

	if($w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１開始){
		if( (`%プレイヤー状態取得` & d:PFLAG_ELUDE) && \
			( (!$f:rfp_PC_奈落エルード聞いた) || ($w:rfp_rand < 3) ) && \
			($f:rfp_w18a_奈落エルード地帯にいる) ){
				@rpc_PC_奈落エルード
				eval($f:rfp_PC_奈落エルード聞いた = 1)

		}else if( (!$f:rfp_PC_エルード説明終了) && \
					($w:rfp_rand < 1) ){
			@rpc_エルード説明

		//無線一回目
		}else if($w:rfp_callcount <= 1){
			rand 5
			eval($w:rfp_rand = $status)
			if($w:rfp_rand < 1){
				@rpc_エルード説明

			}else {
				@rpc_大佐デフォルト
			}

		//連続無線
		}else {
			@rpc_大佐デフォルト
		}
	}else {
		@rpc_大佐デフォルト
	}
}


proc rpc_PC_人質＿人質ぱんつ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_人質＿人質ぱんつ
	@rp_ローズさんを怒らせました
}

block codec RPC_w24c_ギャルパンツ見え地帯  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( ($f:w24c_ギャルパンツ見えてる) && (!$f:rfp_PC_人質＿人質ぱんつ聞いた) ){
		@rpc_PC_人質＿人質ぱんつ
		eval($f:rfp_PC_人質＿人質ぱんつ聞いた = 1 )

	}else {
		@rpc_大佐デフォルト
	}
}


block codec RPC_w25a_自殺ダイブＣＡＬＬ  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＤＧ連絡橋＿赤外線センサー＿制御装置場所２
}

proc rpc_おもしろ赤外線あるぞ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＥＦ連絡橋＿赤外線センサー１
	if(!$f:rfp_おもしろ赤外線あるぞ聞いた){
		@rp_PC_ＥＦ連絡橋＿赤外線センサー＿触るな１
		eval($f:rfp_おもしろ赤外線あるぞ聞いた = 1)
	}
	@rp_PC_ＥＦ連絡橋＿赤外線センサー＿触るな２
}

proc rpc_おもしろ制御装置壊せ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＥＦ連絡橋＿赤外線センサー＿制御装置破壊
	if($w:武器弾数Ｒ[d:武器:Ｍ９] >= 0){
		@rp_PC_ＥＦ連絡橋＿赤外線センサー４
	}
}

proc rpc_PC_くぐる赤外センサー {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_くぐる赤外センサー
}

proc rpc_おもしろ赤外線見方 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rpc_赤外線見方
}

proc rpc_おもしろ赤外線 {
	if(!$f:rfp_おもしろ赤外線あるぞ聞いた){
		@rpc_おもしろ赤外線あるぞ
		eval($f:rfp_おもしろ赤外線あるぞ聞いた = 1)

	}else if(!$f:rfp_PC_赤外線見方聞いた){
		@rpc_おもしろ赤外線見方
		eval($f:rfp_PC_赤外線見方聞いた =1)

	}else if(!$f:rfp_PC_くぐる赤外センサー聞いた){
		@rpc_PC_くぐる赤外センサー
		eval($f:rfp_PC_くぐる赤外センサー聞いた = 1)
	
	}else if(!$f:rfp_おもしろ制御装置壊せ聞いた){
		@rpc_おもしろ制御装置壊せ
		eval($f:rfp_おもしろ制御装置壊せ聞いた = 1)

		eval($f:rfp_おもしろ赤外線終了 = 1)
	}else {
		eval($f:rfp_おもしろ赤外線終了 = 1)
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rpc_おもしろ赤外線あるぞ
		}else if($w:rfp_rand < 2){
			@rpc_おもしろ赤外線見方
		}else if($w:rfp_rand < 3){
			@rpc_PC_くぐる赤外センサー
		}else {
			@rpc_おもしろ制御装置壊せ
		}
	}
}

block codec RPC_w22a_おもしろ赤外線センサー  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:w22a_赤外線センサーフラグ) && (!$f:rfp_おもしろ赤外線終了) ){
		@rpc_おもしろ赤外線

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && (!$f:w22a_赤外線センサーフラグ) ){
			@rpc_おもしろ赤外線
		}else {
			@rpc_大佐デフォルト
		}

	}else {
		@rpc_大佐デフォルト
	}
}



block codec RPC_懸垂握力ＬＶ２ＣＡＬＬ＿普通  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_PC_懸垂握力ＬＶ２
}

block codec RPC_懸垂握力ＬＶ２ＣＡＬＬ＿潜水  14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_PC_懸垂握力ＬＶ２
}



block codec RPC_懸垂握力ＬＶ３ＣＡＬＬ＿普通  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_PC_懸垂握力ＬＶ３
}

block codec RPC_懸垂握力ＬＶ３ＣＡＬＬ＿潜水  14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_PC_懸垂握力ＬＶ３
}

proc rpc_懸垂握力上がらない {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_懸垂握力上がらない１
	if( ($w:p_story >= d:ST:P004_02_R01ノード初接続２無線デモ１終了)  && \
		(!$f:rfp_PR_ローズさんが激怒してます！) && (!$f:rfp_ローズさん落込んでます) ){
		@rpd_ローズ顔表示
		@rp_PC_懸垂握力上がらない２
		eval($f:rfp_PR_ローズさんが激怒してます！ = 0)
	}
	eval($f:rfp_PC_懸垂握力上がらない聞いた = 1)
}

block codec RPC_懸垂握力上がらないＣＡＬＬ＿普通  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_懸垂握力上がらない
}

block codec RPC_懸垂握力上がらないＣＡＬＬ＿潜水  14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpc_懸垂握力上がらない
}

block codec RPC_w61a_愛国説教ＣＡＬＬ  14085 d:大佐無線５バグ大佐 {
	@rp_無線デフォルト前処理＿大佐
	@rpc_PC_ソリダス戦中＿愛国説教
}

block codec RPC_ハリアー戦中レーダー使用可能ＣＡＬＬ  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ハリアー戦＿レーダー
	eval($f:rfp_PC_ハリアー戦＿レーダー聞いた= 1) 
}

block codec RPC_フォーチュン戦中救済ＣＡＬＬ  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_フォーチュン戦＿倒せない１
	@rp_PC_フォーチュン戦＿倒せない２
	eval($f:rfp_PC_フォーチュン戦＿倒せない聞いた = 1)
	eval($f:rfp_フォーチュン戦中救済ＣＡＬＬ聞いた= 1) 
}

proc rpc_PC_ＫＬ連絡橋＿エマ後＿補修 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＫＬ連絡橋＿エマ後＿補修
}

block codec RPC_w25d_はしなおた  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:rfp_PC_ＫＬ連絡橋＿エマ後＿補修聞いた) && ($f:エマ存在フラグ) ){
			@rpc_PC_ＫＬ連絡橋＿エマ後＿補修
			eval($f:rfp_PC_ＫＬ連絡橋＿エマ後＿補修聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && ($f:エマ存在フラグ) ){
			@rpc_PC_ＫＬ連絡橋＿エマ後＿補修

		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_ＫＬ連絡橋＿エマ後＿Ｋ脚入り口 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＫＬ連絡橋＿エマ後＿Ｋ脚入り口
}

block codec RPC_w25d_Ｋ脚入口  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if( (!$f:rfp_PC_ＫＬ連絡橋＿エマ後＿Ｋ脚入り口聞いた) && ($f:エマ存在フラグ) ){
			@rpc_PC_ＫＬ連絡橋＿エマ後＿Ｋ脚入り口
			eval($f:rfp_PC_ＫＬ連絡橋＿エマ後＿Ｋ脚入り口聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && ($f:エマ存在フラグ) ){
			@rpc_PC_ＫＬ連絡橋＿エマ後＿Ｋ脚入り口

		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_監視カメラ {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	if(!$f:rfp_PC_監視カメラ聞いた){
		@rp_PC_監視カメラ１
		eval($f:rfp_PC_監視カメラ聞いた = 1)
	}

	if(!$f:rfp_PC_監視カメラ＿避け方＿真下聞いた){
		@rp_PC_監視カメラ＿避け方＿真下
		eval($f:rfp_PC_監視カメラ＿避け方＿真下聞いた = 1)
	
	}else if(!$f:rfp_PC_監視カメラ＿避け方＿首振り聞いた){
		@rp_PC_監視カメラ＿避け方＿首振り
		eval($f:rfp_PC_監視カメラ＿避け方＿首振り聞いた = 1)
	
	}else if(!$f:rfp_PC_監視カメラ＿避け方＿チャフ聞いた){
		@rp_PC_監視カメラ＿避け方＿チャフ
		eval($f:rfp_PC_監視カメラ＿避け方＿チャフ聞いた = 1)
	
	eval($f:rfp_PC_監視カメラ終了 = 1)
	
	}else {
		eval($f:rfp_PC_監視カメラ終了 = 1)
		rand 3
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 1){
			@rp_PC_監視カメラ＿避け方＿真下

		}else if($w:rfp_rand < 2){
			@rp_PC_監視カメラ＿避け方＿首振り

		}else {
			@rp_PC_監視カメラ＿避け方＿チャフ
		}
	}
}


block codec RPC_監視カメラ近く  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	rand 4
	eval($w:rfp_rand = $status)
	command ゲットゲームステータス $i:gamestatus
	if( ($f:rfp_そこの監視カメラ生きてます) && \
		(!($i:gamestatus & d:STATE_CHAFF)) && \
		(!$f:rfp_PC_監視カメラ終了) ){
		@rpc_PC_監視カメラ
		eval($f:rfp_PC_監視カメラ聞いた = 1)

	}else if( ($f:rfp_そこの監視カメラ生きてます) && \
				(!($i:gamestatus & d:STATE_CHAFF)) && \
				($w:rfp_callcount <= 1) && ($w:rfp_rand < 3) ){
		@rpc_PC_監視カメラ

	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_海鳥糞１{
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_海鳥糞１
}

block codec RPC_うんこ  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	command プレイヤー状態取得
	if($status & d:PFLAG_ELUDE){
		@rpc_PC_奈落エルード
		eval($f:rfp_PC_奈落エルード聞いた = 1)

	}else if(!$f:rfp_PC_海鳥糞１聞いた){
		@rpc_PC_海鳥糞１
		eval($f:rfp_PC_海鳥糞１聞いた = 1)

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpc_PC_海鳥糞１

		}else {
			@rpc_大佐デフォルト
		}

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}

proc rpc_PC_エマ狙撃後＿エマ血痕 {
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エマ狙撃後＿エマ血痕
}

block codec RPC_w20a_エマ血痕  14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	if(!$f:rfp_PC_エマ狙撃後＿エマ血痕聞いた){
		@rpc_PC_エマ狙撃後＿エマ血痕
		eval($f:rfp_PC_エマ狙撃後＿エマ血痕聞いた = 1)

	//連続無線
	}else {
		@rpc_大佐デフォルト
	}
}


block codec RPC_ニキータ本体取得時ＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_リモコンミサイル＿入手後
	eval($f:rfp_ニキータ本体取得時ＣＡＬＬ聞いた = 1)
}

block codec RPC_w22a_赤外線ＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpc_おもしろ赤外線あるぞ

	eval($f:rfp_w22a_赤外線ＣＡＬＬ聞いた = 1)
}

block codec RPC_マイク取得時ＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_マイク入手時ＣＡＬＬ
	eval($f:rfp_マイク取得時ＣＡＬＬ聞いた = 1)
}

block codec RPC_w24b_網膜センサー＿死んでる敵兵ダメＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_網膜センサー＿生きた敵兵
	eval($f:rfp_PC_網膜センサー＿生きた敵兵聞いた = 1 )
	eval($f:rfp_w24b_網膜センサー＿死んでる敵兵ダメＣＡＬＬ聞いた = 1)
}

block codec RPC_w24b_網膜センサーヒントＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_網膜センサー＿羽交絞めヒント２
	eval($f:rfp_w24b_網膜センサーヒントＣＡＬＬ聞いた = 1)
}

block codec RPC_w25c_ローリング飛び越えＣＡＬＬ 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_ＤＧ連絡橋＿渡り方飛びズバリ１
	eval($f:rfp_w25c_ローリング飛び越えＣＡＬＬ聞いた = 1)
}


block codec RPC_w25c_ローリング飛び越え地帯 14085 d:大佐無線１ {
	@rp_無線デフォルト前処理＿大佐
	rand 4
	eval($w:rfp_rand = $status)

	if( (!$f:rfp_w25c_ローリング飛び越え成功した) && \
		(	(!$f:rfp_w25c_ローリング飛び越えＣＡＬＬ聞いた)  || \
			( ($w:rfp_callcount <= 1) && ($w:rfp_rand < 3) ) ) ){
		@rpd_ライデン顔設定デフォルト
		@rpd_大佐顔設定１
		@rpd_顔表示

		@rp_PC_ＤＧ連絡橋＿渡り方飛びズバリ１
		eval($f:rfp_w25c_ローリング飛び越えＣＡＬＬ聞いた = 1)

	}else {
		@rpc_ガンサイファーかもめ連絡橋＿大佐デフォルト	
	}
}


block codec RPC_w11a_敵兵がエレベータ内にいますよＣＡＬＬ 14085 d:大佐無線２潜水ライデン {
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エレベータ敵兵動かない
}

block codec RPC_敵兵がエレベータ内にいますよＣＡＬＬ 14085 d:大佐無線１{
	@rp_無線デフォルト前処理＿大佐
	@rpd_ライデン顔設定デフォルト
	@rpd_大佐顔設定１
	@rpd_顔表示

	@rp_PC_エレベータ敵兵動かない
}


//モーション班用デバッグスクリプトェウァアアアア
#if d:MGS2_SCN
#include "p_colonel_dbg.h"
#endif



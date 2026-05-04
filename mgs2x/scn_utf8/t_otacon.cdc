/*	無線機モードスクリプト*/

/* メッセージ定義 */
#define CODEC_FILE 1
#include "vardef.h"			// 武器・アイテム関係のdefine
#include "cdc_proc.h"

#include "t_otacon_e.cm"
#include "t_otacon_e.ct"

#include "t_otacon_f.cm"
#include "t_otacon_f.ct"

#include "t_otacon_g.cm"
#include "t_otacon_g.ct"

#include "t_otacon_s.cm"
#include "t_otacon_s.ct"

#include "t_otacon_i.cm"
#include "t_otacon_i.ct"

#include "t_otacon_j.cm"
#include "t_otacon_j.ct"

proc rto_デフォルト前処理＿オタコン {
	@rto_デフォルト前処理
	if(`prefreq` != 14112){
		eval($w:rft_callcount = 1)
	}
}


/*
proc HOHO {
	talk キャンベル 標準 -m {
		[w01a_Campbell:00]
	}
	talk スネーク 標準 -m {
		[w01a_Campbell:01]
	}
}

block codec 無線その1 14015 {
	@HOHO
}

block codec 無線その2 14015 {
	talk スネーク 標準 -m {
		[w01a_Campbell:01]
	}
	talk キャンベル 標準 -m {
		[w01a_Campbell:00]
	}
}
*/



/*---------------------------------------------
デフォルトん会話ん系ん会話ん呼び出しんプロックん
----------------------------------------------*/
resource TO_ダミー {
オタコン_000: {
...
}
}

proc rto_エラー {
	talk オタコン 標準 -m {
		[TO_ダミー:オタコン_000]
	}
}


//保険でＧＯ
proc test {
	talk オタコン 標準 -m {
		[TO_ダミー:オタコン_000]
	}
}

block codec test1 14112 d:顔その1 {
	@test
}


/*
proc rto_赤外線機関室通れない {
	talk スネーク 標準 -m {
		[TO_赤外線機関室通れない:スネーク_000]
	}
	talk オタコン 標準 -m {
		[TO_赤外線機関室通れない:オタコン_001]
	}
}

proc rto_赤外線制御装置破壊方法 {
	talk オタコン 標準 -m {
		[TO_赤外線制御装置破壊方法:オタコン_000]
	}
	talk オタコン 標準 -m {
		[TO_赤外線制御装置破壊方法:オタコン_001]
	}
	talk オタコン 標準 -m {
		[TO_赤外線制御装置破壊方法:オタコン_002]
	}
}

proc rto_赤外線ＵＳＰ持ってない {
	talk オタコン 標準 -m {
		[TO_赤外線ＵＳＰ持ってない:オタコン_000]
	}
}

proc rto_赤外線ＵＳＰ持ってる {
	talk オタコン 標準 -m {
		[TO_赤外線ＵＳＰ持ってる:オタコン_000]
	}
}


//製品版のみ
proc rto_写真公開する {
	talk オタコン 標準 -m {
		[TO_写真公開する:オタコン_000]
	}
	if(!$f:rft_写真公開する聞いた){
		talk スネーク 標準 -m {
			[TO_写真公開する:スネーク_001]
		}
		talk オタコン 標準 -m {
			[TO_写真公開する:オタコン_002]
		}
		talk オタコン 標準 -m {
			[TO_写真公開する:オタコン_003]
		}
		talk スネーク 標準 -m {
			[TO_写真公開する:スネーク_004]
		}
		talk オタコン 標準 -m {
			[TO_写真公開する:オタコン_005]
		}
		talk スネーク 標準 -m {
			[TO_写真公開する:スネーク_006]
		}
		talk オタコン 標準 -m {
			[TO_写真公開する:オタコン_007]
		}
	eval($f:rft_写真公開する聞いた = 1)
	}

}
*/

proc rto_奥に進め {
	@rto_艦橋に行け１
	@rto_奥に進め１
}


/*
デフォルト説明系会話呼び出しプロックス
クラックキメてぇ。
*/
/*
proc rto_ＵＳＰ注意センサー壊せる {
	talk オタコン 標準 -m {
		[TO_ＵＳＰ注意センサー壊せる:オタコン_000]
	}
}
*/

/*
proc rto_武器装備品装備法 {
	talk オタコン 標準 -m {
		[TO_武器装備品装備法:オタコン_000]
	}
	talk オタコン 標準 -m {
		[TO_武器装備品装備法:オタコン_001]
	}
	talk オタコン 標準 -m {
		[TO_武器装備品装備法:オタコン_002]
	}
	talk オタコン 標準 -m {
		[TO_武器装備品装備法:オタコン_003]
	}
	talk オタコン 標準 -m {
		[TO_武器装備品装備法:オタコン_004]
	}
	talk オタコン 標準 -m {
		[TO_武器装備品装備法:オタコン_005]
	}
	talk オタコン 標準 -m {
		[TO_武器装備品装備法:オタコン_006]
	}
}
*/

/*
proc rto_耳栓 {
	talk オタコン 標準 -m {
		[TO_耳栓:オタコン_000]
	}
	talk オタコン 標準 -m {
		[TO_耳栓:オタコン_001]
	}
	talk オタコン 標準 -m {
		[TO_耳栓:オタコン_002]
	}
}
*/






/*
強制ＣＡＬＬ系 呼び出しスクリプト
*/

proc rto_赤外線センサー初めて {
	@rto_赤外線強制ＣＡＬＬ最初
//print '$f:rft_w01d赤外線立ち止まったろん:'$f:rft_w01d赤外線立ち止まったろん
	if( ( ($s:エリア == w01d) && ($f:rft_w01d赤外線立ち止まったろん) ) || \
		( ($s:エリア == w02a) && ($f:rft_w02a_赤外線立ち止まったろん) ) ){
		@rto_赤外線強制ＣＡＬＬ立ち止まり
	}else {
		@rto_赤外線強制ＣＡＬＬ通り抜け
	}
	
	@rto_赤外線強制ＣＡＬＬ２

	if($w:t_story >= d:ST_T06a1Dオルガ戦勝利後ポリゴンデモ終了){
		@rto_赤外線強制ＣＡＬＬデモ見てた
	}
	@rto_赤外線強制ＣＡＬＬ３
}



proc rto_赤外線ＣＡＬＬ機関室以外 {
	if($f:rft_赤外線見た){
		@rto_赤外線センサー見てる
	}else{
		@rto_赤外線センサー初めて
	}
	@rto_赤外線くぐり抜ける版
	@rto_赤外線見方タバコ
	@rto_赤外線見方消火器
}

proc rto_赤外線センサー近く {
	if(!$f:rft_赤外線ＣＡＬＬ機関室以外聞いた){
		@rto_赤外線ＣＡＬＬ機関室以外
		eval($f:rft_赤外線ＣＡＬＬ機関室以外聞いた = 1)
	}else {
		if(!$f:rft_赤外線さわるな聞いた){
			@rto_赤外線さわるな
			eval($f:rft_赤外線さわるな聞いた = 1)

		}else if(!$f:rft_赤外線爆弾撃つな１聞いた){
			@rto_赤外線爆弾撃つな１
			eval($f:rft_赤外線爆弾撃つな１聞いた = 1)

		}else if(!$f:rft_赤外線セムテックス聞いた){
			@rto_赤外線セムテックス
			eval($f:rft_赤外線セムテックス聞いた = 1)
		

		}else {
			rand 5
			eval($w:rft_rand = $status)
			if($w:rft_rand < 1){
				@rto_赤外線さわるな

			}else if($w:rft_rand < 2){
				@rto_赤外線セムテックス

			}else if($w:rft_rand < 3){
				@rto_赤外線くぐり抜ける版
		
			}else if($w:rft_rand < 4){
				@rto_赤外線爆弾撃つな１

			}else {
				@rto_赤外線見方タバコ
				@rto_赤外線見方消火器
			}
		}
	}
}

block codec RTO_赤外線ＣＡＬＬ機関室以外 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_赤外線ＣＡＬＬ機関室以外
	eval($f:rft_赤外線ＣＡＬＬ機関室以外聞いた = 1)
	eval($f:rft_w01d赤外線立ち止まったろん = 1)
	eval($f:rft_赤外線見た = 1)
}


proc rto_制御装置壊せ {
	@rto_赤外線制御装置破壊方法

	//ＵＳＰ持っているのだ
	if($w:武器弾数[2] >= 0 ){
		@rto_赤外線ＵＳＰ持ってる

	//持っていないのだ
	}else {
		@rto_赤外線ＵＳＰ持ってない
	}
}

proc rto_赤外線ＣＡＬＬ機関室 {
	if($f:rft_赤外線見た){
		@rto_赤外線センサー見てる
	}else{
		@rto_赤外線センサー初めて
	}
	@rto_赤外線機関室通れない
	@rto_制御装置壊せ

	eval($f:rft_赤外線ＣＡＬＬ機関室聞いた = 1)
	command varsave $f:rft_赤外線ＣＡＬＬ機関室聞いた
	eval($f:rft_赤外線見た = 1)
}

proc rto_赤外線爆弾撃つな {
	@rto_赤外線爆弾撃つな１
	@rto_赤外線爆弾撃つな２
}

proc rto_赤外線センサー機関室近く {
	if(!$f:rft_赤外線ＣＡＬＬ機関室聞いた){
		@rto_赤外線ＣＡＬＬ機関室
		eval($f:rft_赤外線ＣＡＬＬ機関室聞いた = 1)

	//ＵＳＰあるる
	}else if($w:武器弾数[2] >= 0){
		if(!$f:rft_赤外線制御装置撃て聞いた){
			@rto_赤外線制御装置撃て
			eval($f:rft_赤外線制御装置撃て聞いた = 1)
			
		}else if(!$f:rft_赤外線さわるな聞いた){
			@rto_赤外線さわるな
			@rto_赤外線見方タバコ
			@rto_赤外線見方粉袋
			eval($f:rft_赤外線さわるな聞いた = 1)

		}else if(!$f:rft_赤外線爆弾撃つな聞いた){
			@rto_赤外線爆弾撃つな
			eval($f:rft_赤外線爆弾撃つな聞いた = 1)

		}else if(!$f:rft_赤外線制御装置探せ聞いた){
			@rto_赤外線制御装置探せ
			eval($f:rft_赤外線制御装置探せ聞いた = 1)

		}else if(!$f:rft_赤外線主観で狙え聞いた){
			@rto_赤外線主観で狙え
			eval($f:rft_赤外線主観で狙え聞いた = 1)

		}else if(!$f:rft_赤外線主観攻撃聞いた){
			@rto_主観攻撃１の１
			eval($f:rft_赤外線主観攻撃聞いた = 1)

		}else if(!$f:rft_赤外線セムテックス聞いた){
			@rto_赤外線セムテックス
			eval($f:rft_赤外線セムテックス聞いた = 1)
		

		}else {
			rand 6
			eval($w:rft_rand = $status)
			if($w:rft_rand < 1){
				@rto_赤外線さわるな

			}else if($w:rft_rand < 2){
				@rto_赤外線セムテックス

			}else if($w:rft_rand < 3){
				@rto_赤外線くぐり抜ける版
		
			}else if($w:rft_rand < 4){
				@rto_赤外線爆弾撃つな１

			}else if($w:rft_rand < 5){
				@rto_主観攻撃１の１

			}else {
				@rto_赤外線見方タバコ
				@rto_赤外線見方消火器
			}
		}
	//ＵＳＰないよ
	}else {
		@rto_制御装置壊せ
	}
}












/*----------------------------------------------
デフォルト会話でびよよ～ん
-----------------------------------------------*/
proc rto_エルード説明 {
	command プレイヤー状態取得
	if(!$f:rtf_エルード説明聞いた){
		@rto_エルード１の１
		@rto_エルード１の２

		@rto_エルード２の１
		@rto_エルード２の２

		@rto_エルード３の１
		@rto_エルード３の２
		
		@rto_エルード４の１
		@rto_エルード４の２
		eval($f:rtf_エルード説明聞いた = 1)

	//まさにエルード中！！
	}else if($status & d:PFLAG_ELUDE){
		rand 3
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_エルード２の１
			@rto_エルード２の２
		
		}else if($w:rft_rand < 2){
			@rto_エルード２の１
			@rto_エルード３の１
			@rto_エルード３の２

		}else{
			@rto_エルード４の１
			@rto_エルード４の２
		}

	}else if(!$f:rft_かねきゃっち聞いた){
		@rto_かねきゃっち
		eval($f:rft_かねきゃっち聞いた = 1)

	}else if(!$f:rft_かねきっく聞いた){
		@rto_かねきっく
		eval($f:rft_かねきっく聞いた = 1)

	}else {
		rand 5
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_エルード１の１
			@rto_エルード１の２
		}else if($w:rft_rand < 2){
			@rto_エルード２の１
			@rto_エルード２の２

		}else if($w:rft_rand < 3){
			@rto_かねきゃっち

		}else if($w:rft_rand < 4){
			@rto_かねきっく

		}else {
			@rto_エルード４の１
			@rto_エルード４の２
		}
	}
}

proc rto_エルード説明奈落 {
	command プレイヤー状態取得
	if(!$f:rtf_エルード説明聞いた){
		@rto_エルード１の１
		@rto_エルード１の２

		@rto_エルード２の１
		@rto_エルード２の２

		@rto_エルード３の１
		@rto_エルード３の２
		
		@rto_エルード４の１
		@rto_エルード４の２
		eval($f:rtf_エルード説明聞いた = 1)
	//まさにエルード中！！
	}else if($status & d:PFLAG_ELUDE){
		if(!$f:rft_奈落エルード聞いた){
			@rto_奈落エルード
			eval($f:rft_奈落エルード聞いた = 1)

		}else {
			rand 6
			eval($w:rft_rand = $status)
			if($w:rft_rand < 1){
				@rto_エルード２の１
				@rto_エルード２の２
		
			}else if($w:rft_rand < 2){
				@rto_エルード２の１
				@rto_エルード３の１
				@rto_エルード３の２

			}else if($w:rft_rand < 3){
				@rto_エルード４の１
				@rto_エルード４の２

			}else {
				@rto_奈落エルード
			}
		}

	}else {
		rand 3
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_エルード１の１
			@rto_エルード１の２

		}else if($w:rft_rand < 2){
			@rto_エルード２の１
			@rto_エルード２の２

		}else {
			@rto_エルード４の１
			@rto_エルード４の２
		}
	}
}


proc rto_主観攻撃１ {
	@rto_主観攻撃１の１
	@rto_主観攻撃１の２
}

proc rto_主観攻撃２ {
	@rto_主観攻撃２の１
	@rto_主観攻撃２の２
}

proc rto_主観攻撃説明 {
	if(!$f:rft_主観攻撃説明聞いた){
		@rto_主観攻撃１
		@rto_主観攻撃２
		eval($f:rft_主観攻撃説明聞いた = 1)
	}else {
		rand 2
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_主観攻撃１

		}else {
			@rto_主観攻撃２
		}
	}
}

proc rto_流血止血条件説明 {
/* 出血ごとに出血してる！って言う？
	//＠流血状態
	if( (!$f:rft_流血止血条件説明聞いた) && ($f:流血してる) ){
		@rto_流血止血条件出血状態
	}
*/
	@rto_流血止血条件
}

proc rto_影伸び説明 {
	if(!$f:rft_影伸び説明聞いた){
		@rto_影伸び１
		@rto_影伸び２
		eval($f:rft_影伸び説明聞いた = 1)
	}else{
		@rto_影伸び１
		//連続の時１，２？
	}
}

proc rto_クリアリング説明通常 {
	@rto_クリアリング２
}

proc rto_クリアリング説明ＣＡＬＬ {
	@rto_クリアリング１
	@rto_クリアリング２
}

proc rto_敵の無線連絡説明 {
	//危険中は仲間呼ばれてるからなし。
	if($w:アラートモード ==1){
		@rto_敵の無線連絡１
		@rto_敵の無線連絡２

	}else if(!$f:rft_敵の無線連絡説明聞いた){
		@rto_敵の無線連絡１
		@rto_敵の無線連絡２
		@rto_敵の無線連絡３
		@rto_敵の無線連絡４
		@rto_敵の無線連絡５
		@rto_敵の無線連絡６
		eval($f:rft_敵の無線連絡説明聞いた = 1)

	}else{
		rand 5
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_敵の無線連絡１
			@rto_敵の無線連絡２

		}else if($w:rft_rand < 2){
			@rto_敵の無線連絡３

		}else if($w:rft_rand < 3){
			@rto_敵の無線連絡４
		
		}else if($w:rft_rand < 4){
			@rto_敵の無線連絡５

		}else {
			@rto_敵の無線連絡１
			@rto_敵の無線連絡６
		}
	}
}

proc rto_イントルード説明 {
	if(!$f:rft_イントルード説明聞いた){
		@rto_イントルード１
		@rto_イントルード２
		@rto_イントルード３
		eval($f:rft_イントルード説明聞いた = 1)
	}else{
print 'rto_イントルード説明 聞いてるてる'
		rand 2
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_イントルード１
			@rto_イントルード２
		}else {
			@rto_イントルード１
			@rto_イントルード３
		}
	}
}

proc rto_ホフク説明 {
	if(!$f:rft_ホフク説明聞いた){
		@rto_ホフク１
		@rto_ホフク２
		@rto_ホフク３
		@rto_ホフク４
		eval($f:rft_ホフク説明聞いた = 1)
	}else{
		rand 4
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_ホフク１

		}else if($w:rft_rand < 2){
			@rto_ホフク２

		}else if($w:rft_rand < 3){
			@rto_ホフク３

		}else {
			@rto_ホフク４
		}
	}
}


proc rto_食堂説明 {
	@rto_食堂１
	if(!$f:rft_食堂説明聞いた){
		@rto_食堂２
		eval($f:rft_食堂説明聞いた = 1)
	}
	@rto_食堂３
}

proc rto_ＵＳＰ取得時 {
	@rto_ＵＳＰ注意１
	//機関室センサー壊せＣＡＬＬ聞いてる場合
	if($f:rft_赤外線ＣＡＬＬ機関室聞いた){
		@rto_ＵＳＰ注意センサー壊せる
	}
	if(!($w:プレイヤーフラグ & d:PL_USP_SPPRSR_ATTACHED)){
		@rto_ＵＳＰ注意２
	}
}

proc rto_銃を構えて下ろす {
	@rto_銃を構えて下ろす１
	@rto_銃を構えて下ろす２
}

proc rto_武器を構えて下ろす {
	@rto_武器を構えて下ろす１
	@rto_銃を構えて下ろす２
}

proc rto_死体引きずり方法 {
	@rto_死体引きずり方法１
	@rto_死体引きずり方法２
	@rto_死体引きずり方法３
}

proc rto_死体硬直 {
	@rto_死体硬直１
	@rto_死体硬直２
}

proc rto_部位ダメージ銃{
	@rto_部位ダメージ銃１
	@rto_部位ダメージ銃２
}

proc rto_普通のビハインド {
	@rto_普通のビハインド１
	@rto_普通のビハインド２
}

proc rto_ビハインド飛び出し{
	@rto_ビハインド飛び出し１
	@rto_ビハインド飛び出し２
}

proc rto_銃撃戦 {
	@rto_銃撃戦１
	@rto_銃撃戦２
}

proc rto_敵の無線連絡３ {
	@rto_敵の無線連絡３の１
	@rto_敵の無線連絡３の２
}

proc rto_武器装備品装備法従来型 {
	@rto_武器装備品装備法従来型１
	@rto_武器装備品装備法従来型２
}

proc rto_首締め {
	@rto_首締め１
	@rto_首締め２
	@rto_首締め３
}

proc rto_張り付きコンコン {
	@rto_張り付きコンコン１
	@rto_張り付きコンコン２
	@rto_張り付きコンコン３
}

proc rto_アイテムボックス {
	@rto_アイテムボックス１
	@rto_アイテムボックス２
}

proc rto_ＵＳＰ {
	@rto_ＵＳＰ１
	@rto_ＵＳＰ２
	if(!($w:プレイヤーフラグ & d:PL_USP_SPPRSR_ATTACHED) ){
		@rto_ＵＳＰ３
	}
}

proc rto_止血剤 {
	@rto_止血剤１
	@rto_止血剤２
}

proc rto_赤外線ゴーグル {
	@rto_赤外線ゴーグル１
	//ドッグタグ持っている場合 ＸＸＸＸＸＸ
	@rto_赤外線ゴーグル２
}

proc rto_ホールドアップアイテム搾取 {
	@rto_ホールドアップアイテム搾取１
	//ドッグタグ　ホールドアップのみの場合ＸＸＸＸＸＸＸＸｘ
	@rto_ホールドアップアイテム搾取２
}

proc rto_デフォルト操作説明普通 {
	if(!$f:rft_発見されるな聞いた){
		@rto_発見されるな
		eval($f:rft_発見されるな聞いた = 1)

	}else if(!$f:rft_敵の無線連絡説明聞いた){
		@rto_敵の無線連絡説明
		eval($f:rft_敵の無線連絡説明聞いた = 1)

	}else if(!$f:rft_エルード説明聞いた){
		@rto_エルード説明
		eval($f:rft_エルード説明聞いた = 1)

	}else if(!$f:rft_段差上り聞いた){
		@rto_段差上り
		eval($f:rft_段差上り聞いた = 1)
/*
	}else if(!$f:rft_武器装備品装備法従来型聞いた){
		@rto_武器装備品装備法従来型	//up
		eval($f:rft_武器装備品装備法従来型聞いた = 1)
*/
	}else if(!$f:rft_主観攻撃説明聞いた){
		@rto_主観攻撃説明
		eval($f:rft_主観攻撃説明聞いた = 1)

	}else if(!$f:rft_普通のビハインド聞いた){
		@rto_普通のビハインド
		eval($f:rft_普通のビハインド聞いた = 1)

	}else if(!$f:rft_ビハインド覗き聞いた){
		@rto_ビハインド覗き
		eval($f:rft_ビハインド覗き聞いた = 1)

	}else if(!$f:rft_ビハインド中右スティック聞いた){
		@rto_ビハインド中右スティック
		eval($f:rft_ビハインド中右スティック聞いた = 1)

	}else  if(!$f:rft_ビハインド飛び出し聞いた){
		@rto_ビハインド飛び出し
		eval($f:rft_ビハインド飛び出し聞いた = 1)

	}else if(!$f:rft_死体引きずり方法聞いた){
		@rto_死体引きずり方法
		eval($f:rft_死体引きずり方法聞いた = 1)

	}else if(!$f:rft_死体硬直聞いた){
		@rto_死体硬直
		eval($f:rft_死体硬直聞いた = 1)

	}else if(!$f:rft_死体ロッカー隠し聞いた){
		@rto_死体ロッカー隠し
		eval($f:rft_死体ロッカー隠し聞いた = 1)

	}else if(!$f:rft_血痕見つかる聞いた){
		@rto_血痕見つかる
		eval($f:rft_血痕見つかる聞いた = 1)

	}else if(!$f:rft_流血止血条件説明聞いた){
		@rto_流血止血条件説明
		eval($f:rft_流血止血条件説明聞いた = 1)

	}else if(!$f:rft_麻酔効き方聞いた){
		@rto_麻酔効き方
		eval($f:rft_麻酔効き方聞いた = 1)

	}else if(!$f:rft_Ｌ１ボタンロックオン聞いた){
		@rto_Ｌ１ボタンロックオン
		eval($f:rft_Ｌ１ボタンロックオン聞いた = 1)

	}else if(!$f:rft_飛び込み前転聞いた){
		@rto_飛び込み前転
		eval($f:rft_飛び込み前転聞いた = 1)

	}else if(!$f:rft_そろそろ歩き聞いた){
		@rto_そろそろ歩き
		eval($f:rft_そろそろ歩き聞いた = 1)

	}else if(!$f:rft_ホールドアップ聞いた){
		@rto_ホールドアップ
		eval($f:rft_ホールドアップ聞いた = 1)

	}else if(!$f:rft_銃を構えて下ろす聞いた){
		@rto_銃を構えて下ろす
		eval($f:rft_銃を構えて下ろす聞いた = 1)

	}else if(!$f:rft_クリアリング説明通常聞いた){
		@rto_クリアリング説明通常	//
		eval($f:rft_クリアリング説明通常聞いた = 1)

	}else if(!$f:rft_アイテムボックス聞いた){
		@rto_アイテムボックス
		eval($f:rft_アイテムボックス聞いた = 1)

	}else if(!$f:rft_無線ＣＡＬＬ聞いた){
		@rto_無線ＣＡＬＬ
		eval($f:rft_無線ＣＡＬＬ聞いた = 1)

	}else if(!$f:rft_ヒドゥンポイント聞いた){
		@rto_ヒドゥンポイント
		eval($f:rft_ヒドゥンポイント聞いた = 1)

	}else if(!$f:rft_影伸び説明聞いた){
		@rto_影伸び説明
		eval($f:rft_影伸び説明聞いた = 1)

	}else if(!$f:rft_不審物聞いた){
		@rto_不審物
		eval($f:rft_不審物聞いた = 1)

	}else if(!$f:rft_探索モード聞いた){
		@rto_探索モード
		eval($f:rft_探索モード聞いた = 1)

	}else if(!$f:rft_銃撃戦聞いた){
		@rto_銃撃戦
		eval($f:rft_銃撃戦聞いた = 1)

	}else if(!$f:rft_部位ダメージ銃聞いた){
		@rto_部位ダメージ銃
		eval($f:rft_部位ダメージ銃聞いた = 1)

	}else if(!$f:rft_死体隠せ聞いた){
		@rto_死体隠せ
		eval($f:rft_死体隠せ聞いた = 1)

	}else if( (!$f:rft_マガジン投げ聞いた) && ($w:武器弾数[23] > 0) ){
			@rto_マガジン投げ		//マガジン会話聞いた後のみ
			eval($f:rft_マガジン投げ聞いた = 1)

	}else if( (!$f:rft_増援部隊聞いた) && ($w:ゲーム設定 <= d:LEVEL_VERYEASY) ){
		@rto_増援部隊	//
		eval($f:rft_増援部隊聞いた = 1)

	}else if(!$f:rft_投げ聞いた){
		@rto_投げ
		eval($f:rft_投げ聞いた = 1)

	}else if(!$f:rft_首締め聞いた){
		@rto_首締め
		eval($f:rft_首締め聞いた = 1)
/*
	}else if(!$f:rft_イントルード説明聞いた){
		@rto_イントルード説明
		eval($f:rft_イントルード説明聞いた = 1)
*/
	}else if(!$f:rft_張り付きコンコン聞いた){
		@rto_張り付きコンコン
		eval($f:rft_張り付きコンコン聞いた = 1)

	}else if(!$f:rft_かねきゃっち聞いた){
		@rto_かねきゃっち
		eval($f:rft_かねきゃっち聞いた = 1)

	}else if(!$f:rft_かねきっく聞いた){
		@rto_かねきっく
		eval($f:rft_かねきっく聞いた = 1)

	}else if(!$f:rft_インケン投げ聞いた){
		@rto_インケン投げ
		eval($f:rft_インケン投げ聞いた = 1)

	}else if(!$f:rft_ホールドアップアイテム搾取聞いた){
		@rto_ホールドアップアイテム搾取
		eval($f:rft_ホールドアップアイテム搾取聞いた = 1)

//		eval($f:rft_デフォルト操作説明普通終了 = 1)



	//超ランダム
	}else{
		rand 40
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_主観攻撃説明

		}else if($w:rft_rand < 2){
			@rto_敵の無線連絡説明
	
		}else if( ($w:rft_rand < 3) && \
					($w:ゲーム設定 <= d:LEVEL_VERYEASY) ){
			@rto_増援部隊

		}else if($w:rft_rand < 4){
			@rto_クリアリング説明通常

		}else if($w:rft_rand < 5){
			@rto_エルード説明

		}else if($w:rft_rand < 6){
			@rto_段差上り

		}else if($w:rft_rand < 7){
			@rto_普通のビハインド

		}else if($w:rft_rand < 8){
			@rto_ビハインド覗き

		}else if($w:rft_rand < 9){
			@rto_ビハインド中右スティック

		}else if($w:rft_rand < 10){
			@rto_ビハインド飛び出し

		}else if($w:rft_rand < 11){
			@rto_死体引きずり方法

		}else if($w:rft_rand < 12){
			@rto_死体硬直

		}else if($w:rft_rand < 13){
			@rto_死体ロッカー隠し

		}else if($w:rft_rand < 14){
			@rto_血痕見つかる

		}else if($w:rft_rand < 15){
			@rto_流血止血条件説明

		}else if($w:rft_rand < 16){
			@rto_麻酔効き方

		}else if($w:rft_rand < 17){
			@rto_Ｌ１ボタンロックオン

		}else if($w:rft_rand < 18){
			@rto_飛び込み前転

		}else if($w:rft_rand < 19){
			@rto_そろそろ歩き

		}else if($w:rft_rand < 20){
			@rto_ホールドアップ

		}else if($w:rft_rand < 21){
			@rto_銃を構えて下ろす

		}else if($w:rft_rand < 22){
			@rto_アイテムボックス

		}else if($w:rft_rand < 23){
			if($w:武器弾数[23] > 0){
				@rto_マガジン投げ		//マガジンもってる時のみ
			}else{
				@rto_発見されるな
			}
		}else if($w:rft_rand < 24){
			@rto_ヒドゥンポイント

		}else if($w:rft_rand < 25){
			@rto_影伸び説明

		}else if($w:rft_rand < 26){
			@rto_不審物

		}else if($w:rft_rand < 27){
			@rto_探索モード

		}else if($w:rft_rand < 28){
			@rto_銃撃戦

		}else if($w:rft_rand < 29){
			@rto_部位ダメージ銃

		}else if($w:rft_rand < 30){
			@rto_死体隠せ

		}else if($w:rft_rand < 31){
			@rto_無線ＣＡＬＬ		//ＣＡＬＬ取ってない場合のみ
/*
		}else if($w:rft_rand < 32){
			@rto_武器装備品装備法従来型
*/
		}else if($w:rft_rand < 33){
			@rto_投げ

		}else if($w:rft_rand < 34){
			@rto_首締め
/*
		}else if($w:rft_rand < 35){
			@rto_イントルード説明
*/
		}else if($w:rft_rand < 36){
			@rto_張り付きコンコン

		}else if($w:rft_rand < 37){
			@rto_かねきゃっち

		}else if($w:rft_rand < 38){
			@rto_かねきっく

		}else if($w:rft_rand < 39){
			@rto_インケン投げ

		}else if($w:rft_rand < 40){
			@rto_ホールドアップアイテム搾取

		}else{
			@rto_発見されるな

		}
	}
}

proc rto_デフォルト操作説明危険 {
	if(!$f:rft_主観攻撃説明聞いた){
		@rto_主観攻撃説明
		eval($f:rft_主観攻撃説明聞いた = 1)

	}else if(!$f:rft_敵の無線連絡説明聞いた){
		@rto_敵の無線連絡説明
		eval($f:rft_敵の無線連絡説明聞いた = 1)

	}else if(!$f:rft_増援部隊聞いた){
		@rto_増援部隊	//
		eval($f:rft_増援部隊聞いた = 1)

	}else if(!$f:rft_麻酔効き方聞いた){
		@rto_麻酔効き方
		eval($f:rft_麻酔効き方聞いた = 1)

	}else if(!$f:rft_部位ダメージ銃聞いた){
		@rto_部位ダメージ銃
		eval($f:rft_部位ダメージ銃聞いた = 1)

	}else if(!$f:rft_銃撃戦聞いた){
		@rto_銃撃戦
		eval($f:rft_銃撃戦聞いた = 1)

	}else if(!$f:rft_Ｌ１ボタンロックオン聞いた){
		@rto_Ｌ１ボタンロックオン
		eval($f:rft_Ｌ１ボタンロックオン聞いた = 1)

	}else if(!$f:rft_流血止血条件説明聞いた){
		@rto_流血止血条件説明
		eval($f:rft_流血止血条件説明聞いた = 1)

	}else if(!$f:rft_投げ聞いた){
		@rto_投げ
		eval($f:rft_投げ聞いた = 1)

	}else if(!$f:rft_首締め聞いた){
		@rto_首締め
		eval($f:rft_首締め聞いた = 1)

	}else if(!$f:rft_探索モード聞いた){
		@rto_探索モード
		eval($f:rft_探索モード聞いた = 1)

	}else if(!$f:rft_死体隠せ聞いた){
		@rto_死体隠せ
		eval($f:rft_死体隠せ聞いた = 1)

	}else if(!$f:rft_死体引きずり方法聞いた){
		@rto_死体引きずり方法
		eval($f:rft_死体引きずり方法聞いた = 1)

	}else if(!$f:rft_死体硬直聞いた){
		@rto_死体硬直
		eval($f:rft_死体硬直聞いた = 1)

	}else if(!$f:rft_死体ロッカー隠し聞いた){
		@rto_死体ロッカー隠し
		eval($f:rft_死体ロッカー隠し聞いた = 1)

	}else if(!$f:rft_銃を構えて下ろす聞いた){
		@rto_銃を構えて下ろす
		eval($f:rft_銃を構えて下ろす聞いた = 1)


	//超ランダム
	}else{
		rand 16
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_敵の無線連絡説明

		}else if($w:rft_rand < 2){
			@rto_増援部隊	//
	
		}else if($w:rft_rand < 3){
			@rto_主観攻撃説明

		}else if($w:rft_rand < 4){
			@rto_麻酔効き方

		}else if($w:rft_rand < 5){
			@rto_部位ダメージ銃

		}else if($w:rft_rand < 6){
			@rto_銃撃戦

		}else if($w:rft_rand < 7){
			@rto_Ｌ１ボタンロックオン

		}else if($w:rft_rand < 8){
			@rto_流血止血条件説明

		}else if($w:rft_rand < 9){
			@rto_投げ

		}else if($w:rft_rand < 10){
			@rto_首締め

		}else if($w:rft_rand < 11){
			@rto_探索モード

		}else if($w:rft_rand < 12){
			@rto_死体隠せ

		}else if($w:rft_rand < 13){
			@rto_死体引きずり方法

		}else if($w:rft_rand < 14){
			@rto_死体硬直

		}else if($w:rft_rand < 15){
			@rto_死体ロッカー隠し

		}else {
			@rto_銃を構えて下ろす
		}
	}
}

proc rto_デフォルト操作説明 {
	if($w:アラートモード == 1){
		@rto_デフォルト操作説明危険
	}else {
		@rto_デフォルト操作説明普通
	}
}


proc rto_新型メタルギア１{
	@rto_新型メタルギア１の１
	if(!$f:rft_新型メタルギア１聞いた){
		@rto_新型メタルギア１の２
		eval($f:rft_新型メタルギア１聞いた = 1)
	}
}

proc rto_新型メタルギア２{
	@rto_新型メタルギア２の１
	if(!$f:rft_新型メタルギア２聞いた){
		@rto_新型メタルギア２の２
		eval($f:rft_新型メタルギア２聞いた = 1)
	}
}

proc rto_写真公開する {
	@rto_写真公開する１
	if(!$f:rft_写真公開する聞いた){
		@rto_写真公開する２
		eval($f:rft_写真公開する聞いた = 1)
	}
}

proc rto_デフォルト一回のみ {
	if($b:rft_デフォルト一回のみ聞いた回数 < 1){
		@rto_新型メタルギア１
		eval($b:rft_デフォルト一回のみ聞いた回数 = $b:rft_デフォルト一回のみ聞いた回数 + 1)

	}else if($b:rft_デフォルト一回のみ聞いた回数 < 2){
		@rto_新型メタルギア２
		eval($b:rft_デフォルト一回のみ聞いた回数 = $b:rft_デフォルト一回のみ聞いた回数 + 1)

	}else if($b:rft_デフォルト一回のみ聞いた回数 < 3){
		@rto_写真公開する
		eval($b:rft_デフォルト一回のみ聞いた回数 = $b:rft_デフォルト一回のみ聞いた回数 + 1)

		eval($f:rft_デフォルト一回のみ終了 = 1)
		
	}else {
		eval($f:rft_デフォルト一回のみ終了 = 1)
		rand 4
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_新型メタルギア１
		}else if($w:rft_rand < 2){
			@rto_新型メタルギア２
		}else {
			@rto_写真公開する
		}
	}
}





proc rto_オルガ戦主観攻撃説明 {
	if(!$f:rft_オルガ戦主観攻撃説明聞いた){
		@rto_オルガ戦主観攻撃１
		@rto_オルガ戦主観攻撃２
		@rto_オルガ戦主観攻撃３
		@rto_オルガ戦主観攻撃４
		eval($f:rft_オルガ戦主観攻撃説明聞いた = 1)
	}else {
		rand 4
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_オルガ戦主観攻撃１

		}else if($w:rft_rand < 2){
			@rto_オルガ戦主観攻撃２

		}else if($w:rft_rand < 3){
			@rto_オルガ戦主観攻撃３

		}else{
			@rto_オルガ戦主観攻撃４
		}
	}
}


proc rto_オルガ戦ビハインド活用説明 {

	if(!$f:rft_オルガ戦ビハインド活用説明聞いた){
		@rto_オルガ戦ビハインド活用１
		@rto_オルガ戦ビハインド活用２の１
		@rto_オルガ戦ビハインド活用２の２
		@rto_オルガ戦ビハインド活用３
		@rto_オルガ戦ビハインド活用４
		eval($f:rft_オルガ戦ビハインド活用説明聞いた = 1)

	}else {
		rand 4
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_オルガ戦ビハインド活用１

		}else if($w:rft_rand < 2){
			@rto_オルガ戦ビハインド活用２の１
			@rto_オルガ戦ビハインド活用２の２

		}else if($w:rft_rand < 3){
			@rto_オルガ戦ビハインド活用３

		}else{
			@rto_オルガ戦ビハインド活用４
		}
	}
}

proc rto_艦橋に行け {
	@rto_艦橋に行け１
	@rto_艦橋に行け２
}






proc rto_オルガ戦後ちょっとしたデフォルト会話 {
	if($b:rft_オルガ戦後ちょっとしたデフォルト会話回数 < 1){
		@rto_サイファー
		eval($b:rft_オルガ戦後ちょっとしたデフォルト会話回数 = $b:rft_オルガ戦後ちょっとしたデフォルト会話回数 + 1)

	}else if($b:rft_オルガ戦後ちょっとしたデフォルト会話回数 < 2){
		@rto_ゴルルコビッチ
		eval($b:rft_オルガ戦後ちょっとしたデフォルト会話回数 = $b:rft_オルガ戦後ちょっとしたデフォルト会話回数 + 1)

	#ifdef d:GERMAN_TRIAL
	#else
	}else if( ($b:rft_オルガ戦後ちょっとしたデフォルト会話回数 < 3) && \
				($w:t_story < d:ST_機関室赤外線センサークリア) && \
				(!($w:プレイヤーフラグ & d:PL_USP_SPPRSR_ATTACHED)) ){
		@rto_ＵＳＰ取得時
		eval($b:rft_オルガ戦後ちょっとしたデフォルト会話回数 = $b:rft_オルガ戦後ちょっとしたデフォルト会話回数 + 1)
	#endif
	
	}else if($b:rft_オルガ戦後ちょっとしたデフォルト会話回数 < 4){
		@rto_オルガ戦後オルガ
		eval($b:rft_オルガ戦後ちょっとしたデフォルト会話回数 = $b:rft_オルガ戦後ちょっとしたデフォルト会話回数 + 1)

		eval($f:rft_オルガ戦後ちょっとしたデフォルト会話終了 = 1)

	}else {
		eval($f:rft_オルガ戦後ちょっとしたデフォルト会話終了 = 1)
		rand 3
		eval($w:rft_rand = $status)

		if($w:rft_rand < 1){
			@rto_サイファー

		}else if($w:rft_rand < 2){
			@rto_ゴルルコビッチ

		}else {
			@rto_オルガ戦後オルガ
		}
	}
}

proc rto_オルガ戦後船倉行け {
	if(!$f:rft_船倉へ向かえ１聞いた){
		@rto_船倉へ向かえ１
		eval($f:rft_船倉へ向かえ１聞いた = 1)
	}else if(!$f:rft_２Ｆ水密扉近くオルガ戦後){
		@rto_２Ｆ水密扉近くオルガ戦後
		eval($f:rft_２Ｆ水密扉近くオルガ戦後 = 1)
	}else {
		rand 2
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_２Ｆ水密扉近くオルガ戦後
		}else {
			@rto_船倉へ向かえ１
		}
	}
	if($f:rft_赤外線ＣＡＬＬ機関室聞いた){
		@rto_船倉へ向かえ２
	}
}


proc rto_長廊下船倉へ向かえ {
	if(!$f:rft_長廊下船倉へ向かえ聞いた){
		@rto_長廊下船倉へ向かえ１
		@rto_長廊下船倉へ向かえ２
		@rto_長廊下船倉へ向かえ３
		eval($f:rft_長廊下船倉へ向かえ聞いた = 1)
	}
	if($s:エリア == "w03a"){
		if($f:rft_w03a_長廊下北にいまし){
			@rto_長廊下船倉へ向かえ５
		}else{
			@rto_長廊下船倉へ向かえ４
		}
	}else{
			@rto_長廊下船倉へ向かえ６
	}
}

proc rto_銃撃戦飛び出し撃ち {
	if(!$f:rft_銃撃戦飛び出し撃ち聞いた){
		@rto_銃撃戦飛び出し撃ち１
		eval($f:rft_銃撃戦飛び出し撃ち聞いた = 1)
	}
	@rto_銃撃戦飛び出し撃ち２
}

proc rto_タクティカルリロード {
	if(!$f:rft_タクティカルリロード聞いた ){
		@rto_タクティカルリロード１
		eval($f:rft_タクティカルリロード聞いた = 1)
	}
	@rto_タクティカルリロード２
}


proc rto_長廊下左舷銃撃戦ごっこ中 {
	if(!$f:rft_長廊下船倉へ向かえ聞いた){
		@rto_長廊下船倉へ向かえ

	}else 	if(!$f:rft_銃撃戦飛び出し撃ち聞いた){
		@rto_銃撃戦飛び出し撃ち

	}else if(!$f:rft_タクティカルリロード聞いた ){
		@rto_タクティカルリロード

	}else if(!$f:rft_長廊下廊下兵隠れろ聞いた) {
		@rto_長廊下廊下兵隠れろ
		eval($f:rft_長廊下廊下兵隠れろ聞いた = 1)


	//ゴーグル兵のあれは？ＸＸＸＸＸＸＸＸＸＸｘｘｘ
	
	}else {
		rand 7
		eval($w:rft_rand = $status)

		if($w:rft_rand < 2){
			@rto_銃撃戦飛び出し撃ち

		}else  if($w:rft_rand < 4){
			@rto_タクティカルリロード

		}else  if($w:rft_rand < 6){
			@rto_長廊下廊下兵隠れろ

		}else {
			@rto_長廊下船倉へ向かえ
		}
	}
}



proc rto_長廊下デフォルト {
	//左舷銃撃戦ごっこ中
	if( ($w:アラートモード == 1) && ($f:rft_w03a_長廊下北にいまし) ) {
		@rto_長廊下左舷銃撃戦ごっこ中

	}else if(!$f:rft_長廊下船倉へ向かえ聞いた){
		@rto_長廊下船倉へ向かえ

	}else if(!$f:rft_長廊下ライト聞いた) {
		@rto_長廊下ライト
		eval($f:rft_長廊下ライト聞いた = 1 )

	}else if(!$f:rft_長廊下ライトつけろ聞いた) {
		@rto_長廊下ライトつけろ
		eval($f:rft_長廊下ライトつけろ聞いた = 1)

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 6
		eval($w:rft_rand = $status)
		if($w:rft_rand < 2){
			@rto_長廊下船倉へ向かえ

		}else if($w:rft_rand < 3){
			@rto_長廊下ライト

		}else if($w:rft_rand < 4){
			@rto_長廊下ライトつけろ

		}else {
			@rto_デフォルト操作説明
		}

	//連続無線
	}else {
		@連続無線デフォルト
	}
}




proc rto_長廊下右舷銃撃戦 {
	if(!$f:rft_銃撃戦飛び出し撃ち聞いた){
		@rto_銃撃戦飛び出し撃ち

	}else if(!$f:rft_タクティカルリロード聞いた ){
		@rto_タクティカルリロード

	}else if(!$f:rft_全滅させなければ駄目聞いた){
		@rto_全滅させなければ駄目
		eval($f:rft_全滅させなければ駄目聞いた = 1)

	}else if(!$f:rft_グレネード注意聞いた){
		@rto_グレネード注意
		eval($f:rft_グレネード注意聞いた = 1)

	}else if(!$f:rft_長廊下廊下兵隠れろ聞いた) {
		@rto_長廊下廊下兵隠れろ
		eval($f:rft_長廊下廊下兵隠れろ聞いた = 1)

	}else {
		rand 5
		eval($w:rft_rand = $status)

		if($w:rft_rand < 1){
			@rto_銃撃戦飛び出し撃ち

		}else if($w:rft_rand < 2){
			@rto_タクティカルリロード

		}else if($w:rft_rand < 3){
			@rto_全滅させなければ駄目

		}else if($w:rft_rand < 4){
			@rto_グレネード注意

		}else {
			@rto_長廊下廊下兵隠れろ
		}
	}
}


proc 連続無線デフォルト {
	if($w:アラートモード == 1){
		@rto_デフォルト操作説明危険
	}else {
		rand 5
		eval($w:rft_rand = $status)

//		if( ($w:rft_rand < 5) && (!$f:rft_デフォルト一回のみ終了) )
		if($w:rft_rand < 1) {
			if( ($w:t_story < d:ST_T05a1Dオルガ戦前ポリゴンデモ開始) && \
				(!$f:rft_デフォルト一回のみ終了) ){
					@rto_デフォルト一回のみ

			}else if( 	(d:ST_T06b1Rオルガ戦勝利後無線機デモ終了 <= $w:t_story) && \
						($w:t_story < d:ST_機関室赤外線センサークリア) && \
						(!$f:rft_オルガ戦後ちょっとしたデフォルト会話終了) ){
					@rto_オルガ戦後ちょっとしたデフォルト会話
			}else {
				@rto_デフォルト操作説明
			}
		}else{
			@rto_デフォルト操作説明
		}
	}
}


proc rto_操舵室デモ後オルガ戦前説明 {
/*
//体験版苦し紛れ
	@rto_操舵室デモ後オルガ戦前
	if($s:エリア != "w01e"){
		@rto_艦橋に行け２
	}
*/
	@rto_オルガ戦前
}

proc rto_タンカーカメラうんちく {
	@rto_タンカーカメラうんちく１
	//さむねへるを左右に動かすしよーん
	#ifdef d:XBOX
	#else 
		@rto_タンカーカメラうんちく３
	#endif
	if($s:エリア == "w04c"){
		@rto_タンカーカメラうんちく５
	}
}


proc rto_オタコンデフォルト {
	//	print '連続無線だよ:'$w:rft_callcount

	//流血注意報。
	//流血してます！聞いてません！
	if( (`%プレイヤー状態取得 -d` & d:PFLAG_BLOOD) && (!$f:rft_流血止血条件説明聞いた) ){
		@rto_流血止血条件出血状態
		@rto_流血止血条件
		eval($f:rft_流血止血条件説明聞いた = 1)

	#ifdef d:GERMAN_TRIAL
	#else
	}else if( ($w:武器弾数[2] >= 0 ) && (!$f:rft_ＵＳＰ取得時聞いた) && \
				($w:t_story < d:ST_機関室赤外線センサークリア) && \
				(!($w:プレイヤーフラグ & d:PL_USP_SPPRSR_ATTACHED)) )  {
		@rto_ＵＳＰ取得時
		eval($f:rft_ＵＳＰ取得時聞いた = 1)
	#endif

	}else if( ($w:スネーク懸垂回数 >= 100) && (!$f:rft_懸垂握力アップ後聞いた) ){
			@rto_懸垂握力アップ後
			eval($f:rft_懸垂握力アップ後聞いた = 1)

	//船揺れ停止状態。船揺れるよ会話聞いてるりら。
	}else if( 	(`%プレイヤー状態取得 -d` & d:PFLAG_DIAZEPAM) && \
				(!$f:rft_ジアゼパム使用後船が揺れない状態聞いた) && \
				($f:rft_ジアゼパム聞いた) ){
		@rto_ジアゼパム使用後船が揺れない状態
		eval($f:rft_ジアゼパム使用後船が揺れない状態聞いた = 1)

	//武器説明うんちくん
	//M9
	}else if( ($w:武器 == 1) && (!$f:rft_Ｍ９聞いた) ){
		@rto_Ｍ９
		eval($f:rft_Ｍ９聞いた = 1)

	//USP
	#ifdef d:GERMAN_TRIAL
	#else
	}else if( ($w:武器 == 2) && (!$f:rft_ＵＳＰ聞いた) ){
		@rto_ＵＳＰ
		eval($f:rft_ＵＳＰ聞いた = 1)
	#endif
	
	//ちゃっふー
	}else if( ($w:武器 == 10) && (!$f:rft_チャフ聞いた) ){
		@rto_チャフ
		eval($f:rft_チャフ聞いた = 1)

	//スタン。げろっ！いやぉう！
	}else if( ($w:武器 == 11) && (!$f:rft_スタン聞いた) ){
		@rto_スタン
		eval($f:rft_スタン聞いた = 1)

	//まぎゃじん
	}else if( ($w:武器 == 16) && (!$f:rft_マガジン終了) ){
		if(!$f:rft_マガジン聞いた){
			@rto_マガジン
			eval($f:rft_マガジン聞いた = 1)
		}else{
			@rto_マガジン投げ
			eval($f:rft_マガジン投げ聞いた = 1)
			eval($f:rft_マガジン終了 = 1)
		}


	//手投げグレネード。信管ヤパイよ！
	}else if( ($w:武器 == 17) && (!$f:rft_グレネード聞いた) ){
		@rto_グレネード
		eval($f:rft_グレネード聞いた = 1)

	//装備品説明うんちくん
	//レーション
	}else if( ($w:アイテム == 1) && (!$f:rft_レーション聞いた) ){
		@rto_レーション
		eval($f:rft_レーション聞いた = 1)

	//止血剤
	}else if( ($w:アイテム == 4) && (!$f:rft_止血剤聞いた) ){
		@rto_止血剤
		eval($f:rft_止血剤聞いた = 1)

	//ジアゼパム
	}else if( ($w:アイテム == 5) && (!$f:rft_ジアゼパム聞いた) ){
		@rto_ジアゼパム
		eval($f:rft_ジアゼパム聞いた = 1)

	//双眼鏡
	}else if( ($w:アイテム == 14) && (!$f:rft_双眼鏡聞いた) ){
		@rto_双眼鏡
		eval($f:rft_双眼鏡聞いた = 1)

	//タバコ
	}else if( ($w:アイテム == 17) && (!$f:rft_タバコ聞いた) ){
		@rto_タバコ
		eval($f:rft_タバコ聞いた = 1)

	//ダンボール
	}else if( ( ($w:アイテム == 16) || ($w:アイテム == 22) || ($w:アイテム == 23) || \
				($w:アイテム == 24) || ($w:アイテム == 26) || ($w:アイテム == 27) ) && \
				(!$f:rft_ダンボール聞いた) ){
		@rto_ダンボール
		eval($f:rft_ダンボール聞いた = 1)

	//赤外線ゴーグル
	}else if( ($w:アイテム == 13) && (!$f:rft_赤外線ゴーグル聞いた) ){
		@rto_赤外線ゴーグル
		eval($f:rft_赤外線ゴーグル聞いた = 1)

	//タンカーカメラ
	}else if( ($w:アイテム == 21) && (!$f:rft_タンカーカメラうんちく聞いた) ){
		@rto_タンカーカメラうんちく
		eval($f:rft_タンカーカメラうんちく聞いた = 1)

	//生体センサー
	}else if( ($w:アイテム == 25) && (!$f:rft_生体センサー聞いた) ){
		@rto_生体センサー
		eval($f:rft_生体センサー聞いた = 1)


	//ストーリーフラグ依存ぞん。ぶひっ！
	}else if($w:t_story < d:ST_船内入った){
	//	print '連続無線だよ:'$w:rft_callcount
		if(!$f:rft_甲板船内入れ聞いた){
			@rto_甲板船内入れ
			eval($f:rft_甲板船内入れ聞いた = 1)

		//無線一回目
		}else if($w:rft_callcount <= 1){
			rand 5
			eval($w:rft_rand = $status)

			if($w:rft_rand < 3){
				@rto_甲板船内入れ

			}else {
				@rto_デフォルト操作説明
			}

		//連続無線
		}else {
			@連続無線デフォルト
		}

	}else if(d:ST_船内入った <= $w:t_story && \
				$w:t_story < d:ST_T04a1D艦橋カモフ発見ポリゴンデモ開始){
		//甲板
		if($s:エリア == "w00a"){
			if(!$f:rft_船内に戻れ聞いた){
				@rto_船内に戻れ
				eval($f:rft_船内に戻れ聞いた = 1)

			//無線一回目
			}else if($w:rft_callcount <= 1){
				rand 5
				eval($w:rft_rand = $status)

				if($w:rft_rand < 3){
					@rto_船内に戻れ

				}else {
					@rto_デフォルト操作説明
				}

			//連続無線
			}else {
				@連続無線デフォルト
			}

		//居住区１Ｆ南
		}else if($s:エリア == "w01a"){
			if(!$f:rft_奥に進め聞いた){
				@rto_奥に進め
				eval($f:rft_奥に進め聞いた = 1)

			//無線一回目
			}else if($w:rft_callcount <= 1){
				rand 5
				eval($w:rft_rand = $status)

				if($w:rft_rand < 3){
					@rto_奥に進め
				}else {
					@rto_デフォルト操作説明
				}
			//連続無線
			}else {
				@連続無線デフォルト
			}

		//それ以外 w01f,w01b,w01c,w01d
		}else {
			if(!$f:rft_艦橋に行け聞いた){
				@rto_艦橋に行け
				eval($f:rft_艦橋に行け聞いた = 1)

			//無線一回目
			}else if($w:rft_callcount <= 1){
				rand 4
				eval($w:rft_rand = $status)

				if($w:rft_rand < 3){
					@rto_艦橋に行け
				}else {
					@rto_デフォルト操作説明
				}
			//連続無線
			}else {
				@連続無線デフォルト
			}
		}

	//つまり操舵室のデモ群終了後
	}else if(d:ST_T04a1D艦橋カモフ発見ポリゴンデモ開始 <= $w:t_story && \
				$w:t_story < d:ST_T05a1Dオルガ戦前ポリゴンデモ開始){

			if(!$f:rft_操舵室デモ後オルガ戦前聞いた){
				@rto_操舵室デモ後オルガ戦前説明
				eval($f:rft_操舵室デモ後オルガ戦前聞いた = 1)
				
			//無線一回目
			}else if($w:rft_callcount <= 1){
				rand 4
				eval($w:rft_rand = $status)

				//あんま出てほしくないっす……。
				if($w:rft_rand < 3){
					@rto_操舵室デモ後オルガ戦前説明
				}else {
					@rto_デフォルト操作説明
				}
			//連続無線
			}else {
				if(!$f:rft_カサッカうんちく聞いた){
					@rto_カサッカうんちく
					eval($f:rft_カサッカうんちく聞いた = 1)
				}else {
					@連続無線デフォルト
				}
			}

	//オルガ戦中は分離しました。

	}else if(d:ST_T06b1Rオルガ戦勝利後無線機デモ終了 <= $w:t_story && \
				$w:t_story < d:ST_機関室赤外線センサークリア){

		//エンジンるーむんむらむら
		if($s:エリア == "w02a"){
			if(!$f:rft_赤外線ＣＡＬＬ機関室聞いた) {
				if(!$f:rft_船倉へ向かえ１聞いた){
					@rto_オルガ戦後船倉行け

				}else if(!$f:rft_エンジンルーム北西行け聞いた){
					@rto_エンジンルーム北西行け
					eval($f:rft_エンジンルーム北西行け聞いた = 1)
				}else {
					rand 2
					eval($w:rft_rand = $status)
					if($w:rft_rand < 1){
						@rto_オルガ戦後船倉行け
					}else {
						@rto_エンジンルーム北西行け
					}
				}
			}else {
				@rto_オルガ戦後船倉行け
			}
		//エンジンルーム以外
		}else {
			if(!$f:rft_オルガ戦後船倉行け聞いた){
				@rto_オルガ戦後船倉行け
				eval($f:rft_オルガ戦後船倉行け聞いた = 1)

			//無線一回目
			}else if($w:rft_callcount <= 1){
				rand 4
				eval($w:rft_rand = $status)
				if($w:rft_rand < 3){
					@rto_オルガ戦後船倉行け
				}else {
					@rto_デフォルト操作説明
				}
			//連続無線
			}else {
				@連続無線デフォルト
			}
		}

	}else if(d:ST_機関室赤外線センサークリア <= $w:t_story && \
				$w:t_story < d:長廊下兵との戦闘突入ポリデモ開始){

		//長廊下ぷりぷり
		if( ($s:エリア == "w03a") || ($s:エリア == "w03b") ){
			@rto_長廊下デフォルト

		}else {
			if(!$f:rft_長廊下以外船倉へ向かえ聞いた ){
				@rto_長廊下以外船倉へ向かえ
				eval($f:rft_長廊下以外船倉へ向かえ聞いた = 1)

			//無線一回目
			}else if($w:rft_callcount <= 1){
				rand 4
				eval($w:rft_rand = $status)
				if($w:rft_rand < 3){
					@rto_長廊下以外船倉へ向かえ
				}else {
					@rto_デフォルト操作説明
				}

			//連続無線
			}else {
				@連続無線デフォルト
			}
			
		}

	//もう長廊下銃撃戦ん～
	}else if(d:長廊下兵との戦闘突入ポリデモ終了 <= $w:t_story && \
				$w:t_story < d:ST_T08a1Dハッチ閉鎖ポリゴンデモ開始){
		@rto_長廊下右舷銃撃戦


	//船倉イベントは分離分割しゃだりぐ

	}else {
		@rto_エラー
	}
}

block codec RTO_オタコンデフォルト 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_オタコンデフォルト
}





/*-------------------------------------------------------
場所依存説明んぐぉえプロック群をわぎゅぇ
---------------------------------------------------------*/
//○船尾看板（w00a）d//e
//・水密扉近く：水密扉開け方 h//e
block codec RTO_甲板開かない水密扉dr501 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if( ($f:rft_開かない水密扉開けた_dr501) && ($w:t_story < d:ST_船内入った) ){
		@rto_甲板開かない水密扉
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_甲板開かない水密扉dr502 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if( ($f:rft_開かない水密扉開けた_dr502) && ($w:t_story < d:ST_船内入った) ){
		@rto_甲板開かない水密扉
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_甲板開かない水密扉dr503 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if( ($f:rft_開かない水密扉開けた_dr503) && ($w:t_story < d:ST_船内入った) ){
		@rto_甲板開かない水密扉
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_甲板開かない水密扉ＣＡＬＬ_dr501 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_甲板開かない水密扉
	eval($f:rft_RTO_甲板開かない水密扉ＣＡＬＬ_dr501聞いた = 1)
}

block codec RTO_甲板開かない水密扉ＣＡＬＬ_dr502 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_甲板開かない水密扉
	eval($f:rft_RTO_甲板開かない水密扉ＣＡＬＬ_dr502聞いた = 1)
}

block codec RTO_甲板開かない水密扉ＣＡＬＬ_dr503 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_甲板開かない水密扉
	eval($f:rft_RTO_甲板開かない水密扉ＣＡＬＬ_dr503聞いた = 1)
}

//・開いちゃう扉近く h//e
block codec RTO_甲板水密扉 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($w:t_story < d:ST_船内入った){
		@rto_甲板水密扉
		eval($f:rft_甲板水密扉聞いた = 1)
	}else {
		@rto_オタコンデフォルト
	}
}


proc rto_救命艇近く説明	{
	if(!$f:rft_救命艇近く説明聞いた){
		@rto_救命艇近く１
		@rto_救命艇近く２
		@rto_救命艇近く３
		eval($f:rft_救命艇近く説明聞いた = 1)

	}else {
		@rto_救命艇近く１
		@rto_救命艇近く３
	}
}



//・救命艇近く h//e
block codec RTO_救命艇近く 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_救命艇近く説明聞いた){
		@rto_救命艇近く説明

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_救命艇近く説明
		}else {
			@rto_オタコンデフォルト
		}

	//連続無線
	}else {
		@rto_オタコンデフォルト
	}
}

//・段差近く h//e
block codec RTO_段差近く 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_段差上り聞いた){
		@rto_段差上り
		eval($f:rft_段差上り聞いた = 1)

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_段差上り
		}else {
			@rto_オタコンデフォルト
		}

	//連続無線
	}else {
		@rto_オタコンデフォルト
	}
}

//たのしいエルード説明つきか？
block codec RTO_段差近く北 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_段差上り聞いた){
		@rto_段差上り
		eval($f:rft_段差上り聞いた = 1)

	}else if(!$f:rtf_エルード説明聞いた){
		@rto_エルード説明

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 5
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 2){
			@rto_段差上り
		}else if($w:rft_rand < 4){
			@rto_エルード説明

		}else {
			@rto_オタコンデフォルト
		}

	//連続無線
	}else {
		@rto_オタコンデフォルト
	}
}

proc rto_死体捨てられる場所近く説明{
	if(!$f:rft_死体捨てられる場所近く説明聞いた){
		@rto_死体捨てられる場所近く１
		@rto_死体捨てられる場所近く２
		eval($f:rft_死体捨てられる場所近く説明聞いた = 1)
	}else {
		@rto_死体捨てられる場所近く１
	}
}

proc rto_奈落エルード近く {
	command プレイヤー状態取得
	if($status & d:PFLAG_ELUDE){
		print 'grip 握力げーぢ:'$w:スネーク懸垂回数
		if( ($w:スネーク懸垂回数 >= 100) && (!$f:rft_懸垂握力アップ後聞いた) ){
			@rto_懸垂握力アップ後
			eval($f:rft_懸垂握力アップ後聞いた = 1)

		}else {
			@rto_エルード説明奈落
		}
	}else if(!$f:rtf_エルード説明聞いた){
		@rto_エルード説明奈落

	}else {
		rand 4
		eval($w:rft_rand = $status)	
		if($w:rft_rand < 1){
			@rto_エルード説明奈落
		}else{
			@rto_オタコンデフォルト
		}
	}
}


//甲板死体捨てられる場所 h//e
block codec RTO_死体捨てられる場所近く 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	command プレイヤー状態取得
	if($status & d:PFLAG_ELUDE){
		@rto_奈落エルード近く

	}else if(!$f:rft_死体捨てられる場所近く説明聞いた){
		@rto_死体捨てられる場所近く説明

	}else if(!$f:rtf_エルード説明聞いた){
		@rto_奈落エルード近く

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 5
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 2){
			@rto_死体捨てられる場所近く説明
		}else if($w:rft_rand < 4){
			@rto_奈落エルード近く

		}else {
			@rto_オタコンデフォルト
		}

	//連続無線
	}else {
		@rto_オタコンデフォルト
	}
}

//・二階の敵は主観攻撃で倒せ h//e
proc rto_甲板二階の敵は主観攻撃説明{
	//＠敵兵アクティブ レベル分け？
	command ゲット敵兵状態 敵兵:03 $i:敵兵１状態
	eval($f:rft_敵兵元気 = (	($i:敵兵１状態 & 4) || \	//倒れ
						($i:敵兵１状態 & 8) || \	//ダメージ中
						($i:敵兵１状態 & 8192) || \	//捕まえられ
						($i:敵兵１状態 & 16384) || \	//気絶
						($i:敵兵１状態 & 1048576)|| \	//起き上がり中
						($i:敵兵１状態 & 2147483648) ) )	//死亡
	if( ($f:rft_w00a二階敵兵ちゃんいるよヘイヘイ)  && \
		(!$f:rft_敵兵元気) ){
		print 'アクテイブイェーイ！:'$f:rft_敵兵元気

		if(!$f:rft_甲板二階の敵は主観攻撃聞いた){
			@rto_甲板二階の敵は主観攻撃
			eval($f:rft_甲板二階の敵は主観攻撃聞いた = 1)
			if(!$f:rft_主観攻撃説明聞いた){
				@rto_主観攻撃説明
			}
		}else {
			rand 5
			eval($w:rft_rand = $status)	
			if($w:rft_rand < 3){
				@rto_甲板二階の敵は主観攻撃
			}else{
				@rto_オタコンデフォルト
			}
		}

	}else {
		print '元気ないっす。:'$f:rft_敵兵元気
		@rto_オタコンデフォルト
	}
}

block codec RTO_甲板二階の敵は主観攻撃 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_甲板二階の敵は主観攻撃聞いた){
		@rto_甲板二階の敵は主観攻撃説明

	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_甲板二階の敵は主観攻撃説明
		}else {
			@rto_オタコンデフォルト
		}

	//連続無線
	}else {
		@rto_オタコンデフォルト
	}
}



proc rto_エルード近く{
	command プレイヤー状態取得
	print 'grip 握力げーぢ:'$w:スネーク懸垂回数
	if( ($status & d:PFLAG_ELUDE) && \
		($w:スネーク懸垂回数 >= 100) && (!$f:rft_懸垂握力アップ後聞いた) ){
			@rto_懸垂握力アップ後
			eval($f:rft_懸垂握力アップ後聞いた = 1)
		
	}else if(!$f:rtf_エルード説明聞いた){
		@rto_エルード説明
	}else {
		rand 4
		eval($w:rft_rand = $status)	
		if($w:rft_rand < 1){
			@rto_エルード説明
		}else{
			@rto_オタコンデフォルト
		}
	}
}


block codec RTO_エルード近く 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rtf_エルード説明聞いた){
		@rto_エルード近く

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_エルード近く
		}else {
			@rto_オタコンデフォルト
		}

	//連続無線
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_エルード近く２ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rtf_エルード説明聞いた){
		@rto_エルード近く

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_エルード近く
		}else {
			@rto_オタコンデフォルト
		}

	//連続無線
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_奈落エルード近く 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_奈落エルード近く
}


block codec RTO_２Ｆ敵兵注意エルード近く 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	command プレイヤー状態取得

	if($status & d:PFLAG_ELUDE){
		@rto_奈落エルード近く

	}else if(!$f:rtf_エルード説明聞いた){
		print 'near erude であります！'
		@rto_奈落エルード近く

	}else if(!$f:rft_甲板二階の敵は主観攻撃聞いた){
		@rto_甲板二階の敵は主観攻撃説明

	}else {
		rand 5
		eval($w:rft_rand = $status)	
		if($w:rft_rand < 3){
			@rto_奈落エルード近く
		}else{
			@rto_オタコンデフォルト
		}
	}
}


block codec RTO_懸垂ＣＡＬＬ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_懸垂握力アップ後
	eval($f:rft_懸垂握力アップ後聞いた = 1)
}


//○居住区１F南 通路(w01a) d //e
//・ロッカー近く：ロッカー t//e
proc rto_ロッカー近く２ {
	@rto_ロッカー近く２外
	@rto_ロッカー近く２中
}

proc rto_ロッカー説明{
	if(!$f:rft_ロッカー説明聞いた){
		@rto_ロッカー近く１
		@rto_ロッカー近く２
		@rto_ロッカー近く３
		eval($f:rft_ロッカー説明聞いた = 1)

	}else if(!$f:rft_ロッカードア盾聞いた){
		@rto_ロッカードア盾
		eval($f:rft_ロッカードア盾聞いた = 1)

	}else {
		rand 4
		eval($w:rft_rand = $status)

		if($w:rft_rand < 1){
			@rto_ロッカー近く１
		}else if($w:rft_rand < 2){
			@rto_ロッカー近く２
		}else if($w:rft_rand < 3){
			rand 2
			eval($w:rft_rand = $status)
			if($w:rft_rand < 1){
				@rto_死体ロッカー隠し	//＠出ないバグってる！ rand が1しかでない？
			}else {
				@rto_ロッカー近く３
			}
		}else {
			@rto_ロッカードア盾
		}
	}
}



proc rto_ロッカー状態判定 $:そこのロッカー名前 {
	command ゲットゲームステータス $i:game_staus
	//くりゃーりんぐ中
	if($i:game_staus & 2){
		command プレイヤー状態取得
		//ロッカーの中にいます。
		if($status & 128){
			@rto_オタコンデフォルト	//＠危険会話？

		}else if(!$f:rft_w01aクリアリング聞いた){
			@rto_クリアリング説明ＣＡＬＬ
			eval($f:rft_w01aクリアリング聞いた = 1)

		}else if( 	(!$f:rft_ロッカー説明聞いた) && \
					(!$f:rft_ロッカー近く２外聞いた) ){
				@rto_ロッカー近く２外
				eval($f:rft_ロッカー近く２外聞いた = 1)

		}else {
			rand 4
			eval($w:rft_rand = $status)

			if($w:rft_rand < 2){
				@rto_クリアリング説明ＣＡＬＬ
			}else {
				@rto_ロッカー近く２外
			}
		}

	//ろっかー壊れめ。
	}else if( `%ロッカー状態 -n $:そこのロッカー名前` & 8192 ){
		print 'broken 壊れてりあ'
		@rto_オタコンデフォルト

	//開いているのか、閉まっているのか。それは問題ではない。サンチョはそう言って……
	}else{
		command プレイヤー状態取得
		//ロッカーの中。または雪中花の残酷な吐息。
		if($status & 128){
			if(!$f:rft_ロッカーの中聞いた){
				@rto_ロッカーの中
				eval($f:rft_ロッカーの中聞いた = 1)
			}else {
				rand 3
				eval($w:rft_rand = $status)

				if($w:rft_rand < 1){
					@rto_ロッカー近く２中

				}else if($w:rft_rand < 2){
					@rto_ロッカー近く３
				}else {
					@rto_ロッカードア盾
				}
			}

		}else{
			@rto_ロッカー説明
		}
	}
}

proc rto_開かないロッカー説明 $:そこのロッカー名前 {
	//壊れめ。
	if( `%ロッカー状態 -n $:そこのロッカー名前` & 8192 ){
		print 'broken 壊れてりあ'
		@rto_オタコンデフォルト
	//元々開かないものが壊れたとしたら、それは左色の魚のようなものだ。違うか？
	}else {
		@rto_開かないロッカー
	}
}

//左上 ロッカー１
block codec RTO_ロッカーlk000 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_ロッカー状態判定 ロッカー１
}

//左下 ロッカー２
block codec RTO_ロッカーlk001 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_ロッカー状態判定 ロッカー２
}


//中央左下 ロッカー３
block codec RTO_ロッカーlk002 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_ロッカー状態判定 ロッカー３
}

//真中右下 ロッカー４
block codec RTO_ロッカーlk003 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_ロッカー状態判定 ロッカー４
}

//右側上 ロッカー５
block codec RTO_ロッカーlk004 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_ロッカー状態判定 ロッカー５
}

//右下 ロッカー６
block codec RTO_ロッカーlk005 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_ロッカー状態判定 ロッカー６
}


//真中左上 壊れている ロッカー７
block codec RTO_開かないロッカーlk006 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_開かないロッカー開けた_lk006){
		@rto_開かないロッカー説明 ロッカー７
	}else {
		@rto_オタコンデフォルト
	}
}

//真中右上 壊れている ロッカー８
block codec RTO_開かないロッカーlk007 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_開かないロッカー開けた_lk007){
		@rto_開かないロッカー説明 ロッカー８
	}else {
		@rto_オタコンデフォルト
	}
}


block codec RTO_w01aロッカールームデフォルト 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	command ゲットゲームステータス $i:game_staus

	//くりゃーりんぐ中
	if($i:game_staus & 2){
		//ロッカーの中、もしくはダンボール。つまりお隠れになっている
		if($status & 128){
				@rto_クリアリング説明通常

		}else if(!$f:rft_w01aクリアリング聞いた){
			@rto_クリアリング説明ＣＡＬＬ
			eval($f:rft_w01aクリアリング聞いた = 1)

		}else {
			rand 2
			eval($w:rft_rand = $status)

			if($w:rft_rand < 1){
				@rto_クリアリング説明ＣＡＬＬ
			}else {
				@rto_ロッカー近く２外
			}
		}
	}else {
		@rto_オタコンデフォルト
	}
}




//・蒸気パイプ近く：h //e
proc rto_蒸気パイプ近くエリア {
	if(!$f:rft_蒸気パイプ近く聞いた){
		@rto_蒸気パイプ近く
		eval($f:rft_蒸気パイプ近く聞いた = 1)

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 5
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 1){
			@rto_蒸気パイプ近く
		}else {
			@rto_オタコンデフォルト
		}

	//連続無線
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_蒸気パイプ近く 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_蒸気パイプ近くエリア
}


//・足跡エリア：足跡がつく h //e
block codec RTO_足跡 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_足跡聞いた){
		@rto_足跡説明
		eval($f:rft_足跡聞いた = 1)

	}else if(!$f:rft_蒸気パイプ近く聞いた){
		@rto_蒸気パイプ近くエリア

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_足跡説明
		}else {
			@rto_蒸気パイプ近くエリア
		}

	//連続無線
	}else {
		@rto_蒸気パイプ近くエリア
	}
}


block codec RTO_w01a右舷開かない水密扉近く 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w01a開かない水密扉開けた){
		@rto_居住区一階南開かない扉
	}else {
		@rto_蒸気パイプ近くエリア
	}
}

//・ロッカーえっちなピンナップ前、スネ前かがみ状態：h //e
block codec RTO_前かがみ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_前かがみ
}


block codec RTO_とれちゃうのデスカー 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w01a取れる水密扉取れた){
		@rto_居住区一階南開かない扉
	}else {
		@rto_蒸気パイプ近くエリア
	}
}


//○居住区１F北リフレッシュルーム(w01f) d //e
//・エンジンルームへの階段近く h //e
block codec RTO_エンジンルームへの階段近く 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_エンジンルームへの階段近く
}

//体験版用
block codec RTO_エンジンルームへの階段近く体験版 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_エンジンルームへの階段近く体験版
}

proc rto_リフレッシュルーム {
	@rto_リフレッシュルーム１
	@rto_リフレッシュルーム２
}

//・リフレッシュルームについて h //e
proc rto_リフレッシュルーム説明 {
	if(!$f:rft_リフレッシュルーム聞いた){
		@rto_リフレッシュルーム
		eval($f:rft_リフレッシュルーム聞いた = 1)

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 1){
			@rto_リフレッシュルーム
		}else {
			@rto_オタコンデフォルト
		}

	//連続無線
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_リフレッシュルーム 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_リフレッシュルーム説明
}

proc rto_テレビ {
	@rto_テレビ１
	@rto_テレビ２
}

//・テレビ：船倉の様子が映っている h 
block codec RTO_テレビ会話 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if( (!$f:rft_テレビ会話聞いた) && (!$f:壊れフラグ＿テレビ) ){
		@rto_テレビ
		@rto_テレビ中継
		eval($f:rft_テレビ会話聞いた = 1)

	}else {
		@rto_リフレッシュルーム説明
	}
}

block codec RTO_テレビ凝視 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:壊れフラグ＿テレビ){
		if(!$f:rft_テレビ会話聞いた){
			@rto_テレビ
			@rto_テレビ中継
			eval($f:rft_テレビ会話聞いた = 1)

		}else {
			@rto_テレビ２回目
		}
	}else {
		@rto_リフレッシュルーム説明
	}
}

block codec RTO_リフレッシュ開かない扉 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_居住区一階南開かない扉
}


//○居住区２F通路(w01b) d //e
//・オルガ戦前 h //e
block codec RTO_二階通路くぼみ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_二階通路くぼみ聞いた){
		@rto_二階通路くぼみ
		eval($f:rft_二階通路くぼみ聞いた = 1)

	}else if(!$f:rft_ヒドゥンポイント聞いた){
		@rto_ヒドゥンポイント
		eval($f:rft_ヒドゥンポイント聞いた = 1)

	}else if(!$f:rft_蒸気パイプ近く聞いた){
		@rto_蒸気パイプ近くエリア
		eval($f:rft_蒸気パイプ近く聞いた = 1)

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 5
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 1){
			@rto_二階通路くぼみ
		}else if($w:rft_rand < 2){
			@rto_ヒドゥンポイント
		}else if($w:rft_rand < 3){
			@rto_蒸気パイプ近く
		}else{
			@rto_オタコンデフォルト
		}

	//連続無線
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_二階通路くぼみロッカー 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン

	if( (!$f:rft_ロッカー説明聞いた) && (!(`%ロッカー状態 -n ロッカー１` & 8192)) ){
		@rto_ロッカー状態判定 ロッカー１

	}else if(!$f:rft_二階通路くぼみ聞いた){
		@rto_二階通路くぼみ
		eval($f:rft_二階通路くぼみ聞いた = 1)

	}else if(!$f:rft_蒸気パイプ近く聞いた){
		@rto_蒸気パイプ近く
		eval($f:rft_蒸気パイプ近く聞いた = 1)

	}else {
		rand 5
		eval($w:rft_rand = $status)

		if($w:rft_rand < 2){
			@rto_ロッカー状態判定 ロッカー１

		}else if($w:rft_rand < 3){
			@rto_二階通路くぼみ

		}else if($w:rft_rand < 4){
			@rto_蒸気パイプ近く

		}else {
			@rto_オタコンデフォルト
		}
	}
}



block codec RTO_影エリア 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_影エリア聞いた){
		@rto_影伸び説明
		eval($f:rft_影エリア聞いた = 1)

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 1){
			@rto_影伸び説明
		}else {
			@rto_オタコンデフォルト
		}

	//連続無線
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_２Ｆ開かない扉 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_居住区一階南開かない扉
}

block codec RTO_２Ｆ右舷レベルによって開かない扉 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w01b右舷開かない扉開けた){
		@rto_居住区一階南開かない扉
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_２Ｆ水密扉近くオルガ戦前 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_２Ｆ水密扉近くオルガ戦前
}

block codec RTO_２Ｆ水密扉近くオルガ戦後 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_２Ｆ水密扉近くオルガ戦後
}

//二階左舷開かない扉んちょ。
block codec RTO_２Ｆ左舷開かない水密扉近く 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w01b取れる水密扉取れた){
		@rto_エンジンルームへの階段近く体験版
	}else {
		@rto_蒸気パイプ近くエリア
	}
}






//○居住区３F通路(w01c) d //e
proc rto_監視カメラ動く説明 {
	//＠ゲームレベル別動かない監視カメラ説明
	if(!$f:rft_監視カメラ説明聞いた){
		@rto_監視カメラ１
		@rto_監視カメラ２
		@rto_監視カメラ３
		eval($f:rft_監視カメラ説明聞いた = 1)

	}else{
		rand 3
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_監視カメラ１
		}else if($w:rft_rand < 2){
			@rto_監視カメラ２
		}else {
			@rto_監視カメラ３
		}
	}
}
proc rto_監視カメラ動かない説明 {
	//＠ゲームレベル別動かない監視カメラ説明
	if(!$f:rft_監視カメラ説明聞いた){
		@rto_監視カメラ１
		@rto_監視カメラ３
		@rto_監視カメラ４
		eval($f:rft_監視カメラ説明聞いた = 1)

	}else{
		rand 2
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1){
			@rto_監視カメラ１
		}else {
			@rto_監視カメラ３
		}
	}
}

block codec RTO_w01c_監視カメラ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	command ゲットゲームステータス $i:game_staus

	//壊れてるかチャフだったらカメラ会話なしよさん。
	if( ($b:w01c_カメラ破壊フラグ[0]) || \
		($i:game_staus & 16) ){	//チャフ状態
			@rto_オタコンデフォルト
	}else {
		if($w:ゲーム設定 >= d:LEVEL_HARD){
			@rto_監視カメラ動く説明
		}else {
			@rto_監視カメラ動かない説明
		}
	}
}



block codec RTO_監視カメラ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($w:ゲーム設定 >= d:LEVEL_HARD){
		@rto_監視カメラ動く説明
	}else {
		@rto_監視カメラ動かない説明
	}
	eval($f:rft_監視カメラ聞いた = 1)
}

block codec RTO_監視カメラＣＡＬＬ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_監視カメラ動かない説明
	eval($f:rft_監視カメラ聞いた = 1)
}


block codec RTO_三階ロッカー 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_ロッカー状態判定 ロッカー１
}


//○居住区４F厨房(w01d) h //e
//・食堂について t //e
proc rto_食堂 {
	//イントルードはイントルード会話でるりりん。
	command プレイヤー状態取得
/*
	if( ($status & 8) && (!$f:rft_イントルード説明聞いた) )
		@rto_イントルード説明
*/
	if(!$f:rft_食堂聞いた){
		@rto_食堂説明
		eval($f:rft_食堂聞いた = 1)
/*
	}else if(!$f:rft_イントルード説明聞いた){
			@rto_イントルード説明
*/
	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 1){
			@rto_食堂説明
/*
		}else if($w:rft_rand < 2){
			@rto_イントルード説明
*/
		}else {
			@rto_オタコンデフォルト
		}

	//連続無線
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_食堂 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_食堂
}

//・海兵隊員死体近く h //e
//一回目のみ t //e
block codec RTO_食堂死体 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	command プレイヤー状態取得
/*
	if($status & 8){
		if(!$f:rft_イントルード説明聞いた){
			@rto_イントルード説明
	
		}else {
			@rto_食堂
		}
	}
*/

	if(!$f:rft_食堂死体聞いた){
		@rto_食堂死体
		eval($f:rft_食堂死体聞いた = 1)
	}else {
		@rto_食堂
	}
}

//しょくどーのかんしかめらぁ
block codec RTO_w01d_監視カメラ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	command ゲットゲームステータス $i:game_staus

	//壊れてるかチャフだったらカメラ会話なしよさん。
	if( ($f:w01d_監視カメラ１破壊フラグ) || \
		($i:game_staus & 16) ){	//チャフ状態
			@rto_食堂
	}else {
		if($w:ゲーム設定 >= d:LEVEL_NORMAL){
			@rto_監視カメラ動く説明
		}else {
			@rto_監視カメラ動かない説明
		}
	}
}


//・倉庫について h //e
proc rto_倉庫説明 {
	if(!$f:rft_倉庫聞いた){
		@rto_倉庫
		eval($f:rft_倉庫聞いた = 1)

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 6
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_倉庫
/*
		}else if($w:rft_rand < 4){
			@rto_イントルード説明
*/

		}else {
			@rto_オタコンデフォルト
		}

	//連続無線
	}else {
		@rto_オタコンデフォルト
	}
}



block codec RTO_倉庫 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	command ゲットゲームステータス $i:game_staus
	command プレイヤー状態取得

	//くりゃーりんぐ中
	if($i:game_staus & 2){
		//イントルード、もしくはダンボール。つまりお隠れになっている
		if( ($status & 8) || ($w:アイテム == 20) || ($w:アイテム == 30) ){
			if(!$f:rft_w01dクリアリング聞いた){
				@rto_クリアリング説明通常
				eval($f:rft_w01dクリアリング聞いた = 1)

			}else if(!$f:rft_倉庫聞いた){
				@rto_倉庫
				eval($f:rft_倉庫聞いた = 1)

			}else {
				rand 2
				eval($w:rft_rand = $status)

				if($w:rft_rand < 1){
					@rto_クリアリング説明通常
				}else {
					@rto_倉庫
				}
			}
		}else {
			if(!$f:rft_w01dクリアリング聞いた){
				@rto_クリアリング説明ＣＡＬＬ
				eval($f:rft_w01dクリアリング聞いた = 1)

			}else if(!$f:rft_倉庫聞いた){
				@rto_倉庫
				eval($f:rft_倉庫聞いた = 1)
			}else {
				rand 2
				eval($w:rft_rand = $status)

				if($w:rft_rand < 1){
					@rto_クリアリング説明ＣＡＬＬ
				}else {
					@rto_倉庫
				}
			}
		}
/*
	//イントルード状態
	}else if($status & 8) {
		if(!$f:rft_イントルード説明聞いた){
			@rto_イントルード説明

		}else if(!$f:rft_倉庫聞いた){
			@rto_倉庫
			eval($f:rft_倉庫聞いた = 1)

		}else {
			rand 3
			eval($w:rft_rand = $status)
			if($w:rft_rand < 1){
				@rto_イントルード説明
			}else {
				@rto_倉庫説明
			}
		}
*/

	}else {
		@rto_倉庫説明
	}
}

block codec RTO_w01dクリアリングＣＡＬＬ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_クリアリング説明ＣＡＬＬ
	eval($f:rft_w01dクリアリング聞いた = 1)
}

block codec RTO_w01aクリアリングＣＡＬＬ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_クリアリング説明ＣＡＬＬ
	eval($f:rft_w01aクリアリング聞いた = 1)
}

block codec RTO_階段魔人 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($w:アラートモード == 1) {
		@rto_階段魔人
		eval($f:rft_w01d階段魔人聞いた = 1)
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_w01d階段魔人ＣＡＬＬ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_階段魔人
	eval($f:rft_w01d階段魔人聞いた = 1)
}



//○艦橋(w01e) d //e
//・タンカーについて。自動操縦 h //e
proc rto_操舵室 {
	//オルガ戦前状態
	if( 	(d:ST_T04a1D艦橋カモフ発見ポリゴンデモ開始 <= $w:t_story && \
				$w:t_story < d:ST_T05a1Dオルガ戦前ポリゴンデモ開始) ){
		if(!$f:rft_操舵室デモ後オルガ戦前聞いた){
				@rto_操舵室デモ後オルガ戦前説明
				eval($f:rft_操舵室デモ後オルガ戦前聞いた = 1)

		}else if(!$f:rft_カサッカうんちく聞いた){
			@rto_カサッカうんちく
					eval($f:rft_カサッカうんちく聞いた = 1)

		}else if(!$f:rft_自動操縦聞いた){
			@rto_自動操縦
			eval($f:rft_自動操縦聞いた = 1)

		}else if($w:rft_callcount <= 1){
			rand 4
			eval($w:rft_rand = $status)	

			if($w:rft_rand < 2){
				@rto_自動操縦
			}else {
				@rto_オタコンデフォルト
			}
		
		//連続無線
		}else {
			@rto_オタコンデフォルト
		}
	}else {
		if(!$f:rft_自動操縦聞いた){
			@rto_自動操縦
			eval($f:rft_自動操縦聞いた = 1)

		}else if($w:rft_callcount <= 1){
			rand 4
			eval($w:rft_rand = $status)	

			if($w:rft_rand < 2){
				@rto_自動操縦
			}else {
				@rto_オタコンデフォルト
			}
		
		//連続無線
		}else {
			@rto_オタコンデフォルト
		}
	}
}

block codec RTO_操舵室 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_操舵室
}

block codec RTO_操舵室開かない扉 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w01e開かない水密扉開けた){
		@rto_居住区一階南開かない扉
		eval($f:rft_操舵室開かない扉聞いた = 1)
	}else {
		@rto_操舵室
	}
}

block codec RTO_操舵室開かない扉ＣＡＬＬ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_居住区一階南開かない扉
	eval($f:rft_操舵室開かない扉聞いた = 1)
}


//○オルガ戦中(w00b)
proc rto_オルガ戦二人のライフ計算します { //$w:rft_オルガＬＩＦＥぱぁせんと,$w:rft_スネークＬＩＦＥぱぁせんと
	#define r_オルガ最大体力 128
	command ゲットオルガライフ $w:オルガライフ
	eval($w:rft_オルガＬＩＦＥぱぁせんと = ($w:オルガライフ * 100) / d:r_オルガ最大体力 )
	eval($w:rft_スネークＬＩＦＥぱぁせんと = ($w:体力* 100) / $w:体力最大)
	print '$w:rft_オルガＬＩＦＥぱぁせんと:'$w:rft_オルガＬＩＦＥぱぁせんと
	print '$w:rft_スネークＬＩＦＥぱぁせんと:'$w:rft_スネークＬＩＦＥぱぁせんと
}

proc rto_オルガ戦逃げられない {
	@rto_オルガ戦逃げられない１

	@rto_オルガ戦二人のライフ計算します  //$w:rft_オルガＬＩＦＥぱぁせんと,$w:rft_スネークＬＩＦＥぱぁせんと
	if( ($w:rft_スネークＬＩＦＥぱぁせんと <= 50) && \
		($w:rft_スネークＬＩＦＥぱぁせんと < $w:rft_オルガＬＩＦＥぱぁせんと) ){
		@rto_オルガ戦逃げられないスネーク不利

	}else if( 	($w:rft_オルガＬＩＦＥぱぁせんと <= 30) && \
				($w:rft_オルガＬＩＦＥぱぁせんと < $w:rft_スネークＬＩＦＥぱぁせんと) ){
		@rto_オルガ戦逃げられないスネーク有利
	}
}



proc rto_オルガ戦ランダム {
		rand 7
		eval($w:rft_rand = $status)
			
		if($w:rft_rand < 1){
			if($f:rft_オルガ戦オルガグレネード投げた){
				@rto_オルガ戦グレネード
			}else {
				@rto_オルガ戦隠れながら撃て
			}
		}else if($w:rft_rand < 2){
			@rto_オルガ戦隠れながら撃て
		}else if($w:rft_rand < 3){
			@rto_オルガ戦主観攻撃説明
		}else if($w:rft_rand < 4){
			@rto_オルガ戦ビハインド活用説明
		}else if($w:rft_rand < 5){
			if($i:オルガ行動フラグ & 8192){
				@rto_オルガ戦幌は通らない
			}else {
				@rto_オルガ戦主観攻撃説明
			}
		}else if($w:rft_rand < 6){
			@rto_オルガ戦ヘッドショット
		
		}else{
			@rto_オルガ戦中空障害物
		}	
}


proc rto_オルガ戦中 {
//	print '$f:rft_オルガ戦オルガグレネード投げた:'$f:rft_オルガ戦オルガグレネード投げた
//	print '$f:w00b_オルガ戦第３フェーズ入った:' $f:w00b_オルガ戦第３フェーズ入った
	//＠オルガグレネード投げよった

	command ゲットオルガ行動フラグ  $i:オルガ行動フラグ
	if( ($i:オルガ行動フラグ & 8192) && \
		(!$f:rft_オルガ戦幌は通らない) ){
		@rto_オルガ戦幌は通らない
		eval($f:rft_オルガ戦幌は通らない = 1)

	}else if( 	($f:w00b_オルガ戦第３フェーズ入った) && \
				(!$f:w00b_左舷投光器壊れた) && \
				(!$f:rft_オルガ戦ライト眩惑聞いた) ){
			@rto_オルガ戦ライト眩惑
			eval($f:rft_オルガ戦ライト眩惑聞いた = 1)

	}else if( ($f:rft_オルガ戦オルガグレネード投げた) && \
		(!$f:rft_オルガ戦グレネード聞いた) ){
			@rto_オルガ戦グレネード
			eval($f:rft_オルガ戦グレネード聞いた = 1)
	
	}else if($b:rft_オルガ戦聞いた回数 < 1){
		@rto_オルガ戦隠れながら撃て
		eval($b:rft_オルガ戦聞いた回数 = $b:rft_オルガ戦聞いた回数 + 1)

	}else if($b:rft_オルガ戦聞いた回数 < 2){
		@rto_オルガ戦主観攻撃説明
		eval($b:rft_オルガ戦聞いた回数 = $b:rft_オルガ戦聞いた回数 + 1)

	}else if($b:rft_オルガ戦聞いた回数 < 3){
		@rto_オルガ戦ビハインド活用説明
		eval($b:rft_オルガ戦聞いた回数 = $b:rft_オルガ戦聞いた回数 + 1)

	}else if($b:rft_オルガ戦聞いた回数 < 4){
		@rto_オルガ戦中空障害物
		eval($b:rft_オルガ戦聞いた回数 = $b:rft_オルガ戦聞いた回数 + 1)

	}else if($b:rft_オルガ戦聞いた回数 < 5){
		@rto_オルガ戦ヘッドショット
		eval($b:rft_オルガ戦聞いた回数 = $b:rft_オルガ戦聞いた回数 + 1)

	}else if($b:rft_オルガ戦聞いた回数 < 6){
		@rto_オルガ戦何者
		eval($b:rft_オルガ戦聞いた回数 = $b:rft_オルガ戦聞いた回数 + 1)

	}else {
		if( 	($f:w00b_オルガ戦第３フェーズ入った) && \
				(!$f:w00b_左舷投光器壊れた)  ){
			rand 2
			eval($w:rft_rand = $status)

			if($w:rft_rand < 1){
				@rto_オルガ戦ライト眩惑
			}else {
				@rto_オルガ戦ランダム
			}
		}else {
				@rto_オルガ戦ランダム
		}
	}
}

block codec RTO_オルガ戦中 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_オルガ戦中
}

block codec RTO_オルガ戦ヒントＣＡＬＬ１ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	eval($f:rft_オルガ戦主観攻撃説明聞いた = 0)
	@rto_オルガ戦主観攻撃説明
	eval($f:rtf_オルガ戦ヒントＣＡＬＬ１聞いた = 1)
}

block codec RTO_オルガ戦逃げられない 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(768 <= $w:プレイヤー方向 && $w:プレイヤー方向 <= 1280){
		@rto_オルガ戦逃げられない
	}else {
		@rto_オルガ戦中
	}
}


block codec RTO_オルガ戦立入禁止 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_オルガ戦立入禁止聞いた){
		@rto_オルガ戦立入禁止
		eval($f:rft_オルガ戦立入禁止聞いた = 1)
	}else {
		rand 4
		eval($w:rft_rand = $status)

		if($w:rft_rand < 3){
			@rto_オルガ戦立入禁止
		}else {
			@rto_オルガ戦中
		}
	}
}


block codec RTO_流血状態ＣＡＬＬ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_流血止血条件出血状態
	@rto_流血止血条件
	eval($f:rft_流血止血条件説明聞いた = 1)
}

block codec RTO_ＵＳＰ取得時ＣＡＬＬ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_ＵＳＰ取得時
	eval($f:rft_ＵＳＰ取得時聞いた = 1)
}





//----------------------------------------
//w01c 後悔乾パン
//----------------------------------------
block codec RTO_オルガ死体撃ちやがった 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_オルガ死体撃った
	eval($f:rft_オルガ死体撃ちやがった聞いた = 1)
}




proc rto_マストの上 {
	//一回目のみ
	if(!$f:rft_rto_マストの上聞いた){
		@rto_マスト最上部
		eval($f:rft_rto_マストの上聞いた = 1)

	}else {
		@rto_オタコンデフォルト
	}
}


block codec RTO_マストの上 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_マストの上
}


//----------------------------------------
//w02a機関室
//----------------------------------------
proc rto_機関室ランダム {
	rand 3
	eval($w:rft_rand = $status)

	if($w:rft_rand < 1){
		@rto_エンジンルーム聴力低い

	}else if($w:rft_rand < 2) {
		@rto_機関室エルード

	}else {
		@rto_エンジンルームエンジン
	}
}

proc rto_機関室 {
	if(!$f:rfp_機関室＿機関室でないデフォルト会話聞いた){
		@rto_オタコンデフォルト
		eval($f:rfp_機関室＿機関室でないデフォルト会話聞いた = 1)

	}else if(!$f:rft_機関室エルード聞いた){
		@rto_機関室エルード
		eval($f:rft_機関室エルード聞いた = 1)

	}else if(!$f:rtf_エルード説明聞いた){
		@rto_エルード説明
		eval($f:rtf_エルード説明聞いた = 1)

	}else if(!$f:rft_エンジンルーム聴力低い聞いた) {
		@rto_エンジンルーム聴力低い
		eval($f:rft_エンジンルーム聴力低い聞いた =1)
		
	}else if(!$f:rft_エンジンルームエンジン聞いた) {
		@rto_エンジンルームエンジン
		eval($f:rft_エンジンルームエンジン聞いた = 1)

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 6
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 1){
			@rto_機関室ランダム

		}else if($w:rft_rand < 3){
			@rto_エルード説明

		}else {
			@rto_オタコンデフォルト
		}

	//連続無線
	}else {
		@rto_オタコンデフォルト
	}
}


block codec RTO_赤外線機関室 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	//無効になるまで
	if($w:t_story < d:ST_機関室赤外線センサークリア) {
		if(!$f:rft_赤外線ＣＡＬＬ機関室聞いた) {
			@rto_赤外線ＣＡＬＬ機関室
			eval($f:rft_赤外線ＣＡＬＬ機関室聞いた = 1)
//			eval($f:rft_w02a_赤外線立ち止まったろん = 1)
			eval($f:rft_赤外線見た = 1)
		}else {
			@rto_赤外線センサー機関室近く
		}
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_赤外線機関室以外 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:w01d_赤外線１スイッチ破壊フラグ){
		if(!$f:rft_赤外線ＣＡＬＬ機関室以外聞いた){
			@rto_赤外線ＣＡＬＬ機関室以外
			eval($f:rft_赤外線ＣＡＬＬ機関室以外聞いた = 1)
			eval($f:rft_w01d赤外線立ち止まったろん = 1)
			eval($f:rft_赤外線見た = 1)
		}else {
			@rto_赤外線センサー近く
		}
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_赤外線ＣＡＬＬ機関室 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_赤外線ＣＡＬＬ機関室聞いた) {
		@rto_赤外線ＣＡＬＬ機関室
		eval($f:rft_赤外線ＣＡＬＬ機関室聞いた = 1)
		eval($f:rft_赤外線見た = 1)
	}else {
		@rto_赤外線センサー機関室近く
	}
}

block codec RTO_w02a_オタコンデフォルト 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン

	if($f:rft_w02a_機関室にいるみょ){
		@rto_機関室
	}else {
		@rto_オタコンデフォルト
	}
}



proc rto_レイブン影 {
	if(!$f:rft_レイブン影正体前聞いた){
		@rto_レイブン影正体前
		eval($f:rft_レイブン影正体前聞いた = 1) 
	}
	@rto_レイブン影正体前_二回目以降
}


block codec RTO_w02a_天田麗文ビビリ地帯 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if( (!$f:rft_レイブン影正体発覚した) && ($f:w02a_レイブンイベントフラグ) ){
		@rto_レイブン影
	}else {
		@rto_オタコンデフォルト
	}
}



// 右舷ロッカー左から１番目
block codec RTO_w02a_ロッカーlk001 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_ロッカー状態判定 ロッカー１
}

// 右舷ロッカー左から２番目
block codec RTO_w02a_ロッカーlk002 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_ロッカー状態判定 ロッカー２
}

// 右舷ロッカー左から３番目
block codec RTO_w02a_ロッカーlk003 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_ロッカー状態判定 ロッカー３
}

// 右舷ロッカー左から４番目 開かないんＺ
block codec RTO_w02a_開かないロッカーlk004 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w02a_開かないロッカーlk004){
		@rto_開かないロッカー説明 ロッカー４
	}else {
		@rto_オタコンデフォルト
	}
}


// 左舷北 さわり男
block codec RTO_w02a_ロッカーlk011 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_ロッカー状態判定 ロッカー５
}

// 左舷真ん中 四次元女
block codec RTO_w02a_ロッカーlk012 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	@rto_ロッカー状態判定 ロッカー６
}

// 左舷南 あかん
block codec RTO_w02a_開かないロッカーlk013 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w02a_開かないロッカー開けた_lk013){
		@rto_開かないロッカー説明 ロッカー７
	}else {
		@rto_オタコンデフォルト
	}
}


//-----------------------------------------
//w00c オルガ戦後オルガ戦
//------------------------------------------
block codec RTO_オルガいぢり地帯 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン

	if( (!$f:rfp_w00c_オルガさん失踪しました) && ($w:rft_w00c_オルガスネーク相対距離 <= 1800) && \
		(!$f:rft_オルガ死体聞いた) ) {
		@rto_オルガ死体
		eval($f:rft_オルガ死体聞いた = 1)
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_マスト最上部 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン

	if( !$f:rft_マスト最上部死体聞いた ) {
		@rto_マスト最上部
		eval($f:rft_マスト最上部死体聞いた = 1)
	}else {
		@rto_オタコンデフォルト
	}
}

//----------------------------------------
//w03a なぎゃろーか 左舷げんげん
//----------------------------------------
/*
proc rto_長廊下開かない扉 {
	@rto_長廊下デフォルト
}
*/


block codec RTO_w03a_開かない水密扉dr000 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w03a_開かない水密扉dr000開けた){
		@rto_長廊下開かない扉
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_w03a_開かない水密扉dr100 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w03a_開かない水密扉dr100開けた){
		@rto_長廊下開かない扉
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_w03a_開かない水密扉dr200 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w03a_開かない水密扉dr200開けた){
		@rto_長廊下開かない扉
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_w03a_開かない水密扉dr300 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w03a_開かない水密扉dr300開けた){
		@rto_長廊下北＿開かない扉
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_w03a_開かない水密扉dr301 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w03a_開かない水密扉dr301開けた){
		@rto_長廊下北＿開かない扉
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_w03a_開かない水密扉dr302 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w03a_開かない水密扉dr302開けた){
		@rto_長廊下北＿開かない扉
	}else {
		@rto_オタコンデフォルト
	}
}


//--------------------------------------------------
//w03b 長廊下右舷げんちよろ
//--------------------------------------------------
block codec RTO_w03b_開かない水密扉dr000 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w03b_開かない水密扉dr000開けた){
		@rto_長廊下東開かない扉
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_w03b_開かない水密扉dr001 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w03b_開かない水密扉dr001開けた){
		@rto_長廊下東開かない扉
	}else {
		@rto_オタコンデフォルト
	}
}

block codec RTO_w03b_開かない水密扉dr100 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if($f:rft_w03b_開かない水密扉dr100開けた){
		@rto_長廊下東開かない扉
	}else {
		@rto_オタコンデフォルト
	}
}




proc rto_船倉演説終わる前に {
	@rto_船倉演説終わる前に１
	if(!$f:rft_船倉演説終わる前に聞いた){
		@rto_船倉演説終わる前に２
		eval($f:rft_船倉演説終わる前に聞いた = 1)
	}
	//時間がなかったら？ＸＸＸＸＸＸＸＸ
		//@rto_船倉演説終わる前に３
}

proc rto_船倉デフォルト {
	if( (!$f:rft_船倉１２進め聞いた) && ($s:エリア != "w04c") ){
		@rto_船倉１２進め
		eval($f:rft_船倉１２進め聞いた = 1)

	}else if(!$f:rft_船倉演説終わる前に聞いた){
		@rto_船倉演説終わる前に
		eval($f:rft_船倉演説終わる前に聞いた = 1)

	}else if(!$f:rft_船倉見つかると即死聞いた){
		@rto_船倉見つかると即死
		eval($f:rft_船倉見つかると即死聞いた = 1)
/*
	}else if(!$f:rft_船倉兵聞いた){
		@rto_船倉兵
		eval($f:rft_船倉兵聞いた = 1)
*/

	}else if( !$f:rft_演説１陸軍に対抗聞いた){
		@rto_演説１
		eval($f:rft_演説１陸軍に対抗聞いた = 1)

	}else if( ($f:rft_演説２海軍からの圧力再生した) && (!$f:rft_演説２海軍からの圧力聞いた) ){
		@rto_演説２
		@rto_演説２x
		eval($f:rft_演説２海軍からの圧力聞いた = 1)

	}else if( ($f:rft_演説４ＮＭＤ再生した) && (!$f:rft_演説４ＮＭＤ聞いた) ){
		@rto_演説４
		eval($f:rft_演説４ＮＭＤ聞いた = 1)

	}else {
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 1){
			if($s:エリア != "w04c"){
				@rto_船倉１２進め
			}else {
				@rto_船倉演説終わる前に
			}

		}else if($w:rft_rand < 2){
			@rto_船倉演説終わる前に

		}else{
			@rto_船倉見つかると即死
		}
	}
}

proc rto_船倉戻るな {
	@rto_船倉戻るな１
	//時間ない？ＸＸＸＸＸ
	@rto_船倉戻るな２
}

proc rto_船倉写真条件 {
	if(!$f:rft_船倉写真条件聞いた){
		@rto_船倉写真条件１
		eval($f:rft_船倉写真条件聞いた = 1)
	}
	@rto_船倉写真条件２
	@rto_船倉写真条件３
	//４は没なのの
	@rto_船倉写真条件５
}


/*
$f:メタル正面撮影成功
$f:メタル右側撮影成功
$f:メタル左側撮影成功
$f:メタルＭＡＲＩＮＥＳ撮影成功
*/
proc rto_第三船倉超絶！写真判定 {

	if( (!$f:rft_タンカーカメラうんちく聞いた) && ($w:rft_callcount > 1 ) ){
			@rto_タンカーカメラうんちく
			eval($f:rft_タンカーカメラうんちく聞いた = 1)

	}else {
		@rto_船倉写真条件２
		//1000 右だけＯＫ
		if( \
			($f:メタル右側撮影成功) && \
			(!$f:メタル左側撮影成功) && \
			(!$f:メタル正面撮影成功) && \
			(!$f:メタルＭＡＲＩＮＥＳ撮影成功) ){
			@rto_船倉残り写真１
	
		//0100 左だけＯＫ
		}else if( \
			(!$f:メタル右側撮影成功) && \
			($f:メタル左側撮影成功) && \
			(!$f:メタル正面撮影成功) && \
			(!$f:メタルＭＡＲＩＮＥＳ撮影成功) ){
			@rto_船倉残り写真２

		//0010 正面だけＯＫ
		}else if( \
			(!$f:メタル右側撮影成功) && \
			(!$f:メタル左側撮影成功) && \
			($f:メタル正面撮影成功) && \
			(!$f:メタルＭＡＲＩＮＥＳ撮影成功) ){
			@rto_船倉残り写真３

		//0001 ＭＡＲＩＮＥＳだけＯＫ
		}else if( \
			(!$f:メタル右側撮影成功) && \
			(!$f:メタル左側撮影成功) && \
			(!$f:メタル正面撮影成功) && \
			($f:メタルＭＡＲＩＮＥＳ撮影成功) ){
			@rto_船倉残り写真４

		//1100 右左ＯＫ
		}else if( \
			($f:メタル右側撮影成功) && \
			($f:メタル左側撮影成功) && \
			(!$f:メタル正面撮影成功) && \
			(!$f:メタルＭＡＲＩＮＥＳ撮影成功) ){
			@rto_船倉残り写真５

		//1010 右前ＯＫ
		}else if( \
			($f:メタル右側撮影成功) && \
			(!$f:メタル左側撮影成功) && \
			($f:メタル正面撮影成功) && \
			(!$f:メタルＭＡＲＩＮＥＳ撮影成功) ){
			@rto_船倉残り写真６

		//1001 右まりＯＫ
		}else if( \
			($f:メタル右側撮影成功) && \
			(!$f:メタル左側撮影成功) && \
			(!$f:メタル正面撮影成功) && \
			($f:メタルＭＡＲＩＮＥＳ撮影成功) ){
			@rto_船倉残り写真７

		//0101 左まり
		}else if( \
			(!$f:メタル右側撮影成功) && \
			($f:メタル左側撮影成功) && \
			(!$f:メタル正面撮影成功) && \
			($f:メタルＭＡＲＩＮＥＳ撮影成功) ){
			@rto_船倉残り写真８

		//0110 左前
		}else if( \
			(!$f:メタル右側撮影成功) && \
			($f:メタル左側撮影成功) && \
			($f:メタル正面撮影成功) && \
			(!$f:メタルＭＡＲＩＮＥＳ撮影成功) ){
			@rto_船倉残り写真９

		//0011 前まり
		}else if( \
			(!$f:メタル右側撮影成功) && \
			(!$f:メタル左側撮影成功) && \
			($f:メタル正面撮影成功) && \
			($f:メタルＭＡＲＩＮＥＳ撮影成功) ){
			@rto_船倉残り写真１０


		//1110 右左前
		}else if( \
			($f:メタル右側撮影成功) && \
			($f:メタル左側撮影成功) && \
			($f:メタル正面撮影成功) && \
			(!$f:メタルＭＡＲＩＮＥＳ撮影成功) ){
			@rto_船倉残り写真１１

		//1101 右左まり
		}else if( \
			($f:メタル右側撮影成功) && \
			($f:メタル左側撮影成功) && \
			(!$f:メタル正面撮影成功) && \
			($f:メタルＭＡＲＩＮＥＳ撮影成功) ){
			@rto_船倉残り写真１２

		//1011 右前まり
		}else if( \
			($f:メタル右側撮影成功) && \
			(!$f:メタル左側撮影成功) && \
			($f:メタル正面撮影成功) && \
			($f:メタルＭＡＲＩＮＥＳ撮影成功) ){
			@rto_船倉残り写真１３

		//0111 左前まり
		}else if( \
			(!$f:メタル右側撮影成功) && \
			($f:メタル左側撮影成功) && \
			($f:メタル正面撮影成功) && \
			($f:メタルＭＡＲＩＮＥＳ撮影成功) ){
			@rto_船倉残り写真１４

		}else {
			@rto_エラー
		}
	}
}


proc rto_第三船倉デフォルト {
	//一枚も写真ＯＫもらってないよー
	if( (!$f:メタル正面撮影成功) && \
		(!$f:メタル右側撮影成功) && \
		(!$f:メタル左側撮影成功) && \
		(!$f:メタルＭＡＲＩＮＥＳ撮影成功) ){

		if(!$f:rft_船倉写真条件聞いた){
			@rto_船倉写真条件

		}else if( (!$f:rft_タンカーカメラうんちく聞いた) && ($w:rft_callcount > 1 ) ){
			@rto_タンカーカメラうんちく
			eval($f:rft_タンカーカメラうんちく聞いた = 1)

		}else if(!$f:rft_写真電送せよ) {
			@rto_写真電送せよ
			eval($f:rft_写真電送せよ = 1)

		}else {
			rand 6
			eval($w:rft_rand = $status)	

			if($w:rft_rand < 3){
				@rto_船倉写真条件

			}else if($w:rft_rand < 4){
				@rto_写真電送せよ

			}else {
				@rto_船倉デフォルト
			}
		}
	}else {
		@rto_第三船倉超絶！写真判定
	}
}




block codec RTO_船倉イベント 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	//ＤＩＥ一船倉
	if( $s:エリア == "w04a") {
		if($w:t_story < d:ST_T11a1D船首メタルギア登場ポリゴンデモ開始){
			@rto_船倉デフォルト

		//いっぺん第三船倉に入った
		}else {
			if(!$f:rft_船倉戻るな聞いた){
				@rto_船倉戻るな
				eval($f:rft_船倉戻るな聞いた = 1)
			}else {
				rand 3
				eval($w:rft_rand = $status)	

				if($w:rft_rand < 2){
					@rto_船倉戻るな
				}else {
					@rto_船倉デフォルト
				}
			}
		}

	//ＤＩＥ二船倉
	}else if($s:エリア == "w04b"){
		if($w:t_story < d:ST_T11a1D船首メタルギア登場ポリゴンデモ開始){
			@rto_船倉デフォルト

		//いっぺん第三船倉に入った
		}else {
			if(!$f:rft_船倉戻るな聞いた){
				@rto_船倉戻るな
				eval($f:rft_船倉戻るな聞いた = 1)
			}else {
				rand 3
				eval($w:rft_rand = $status)	

				if($w:rft_rand < 2){
					@rto_船倉戻るな
				}else {
					@rto_船倉デフォルト
				}
			}
		}

	//ＤＩＥ三船倉
	}else{
		@rto_第三船倉デフォルト 	
	}
}



block codec RTO_船倉ロックドア 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_船倉ロックドア聞いた){
		@rto_船倉ロックドア
		eval($f:rft_船倉ロックドア聞いた = 1)

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_船倉ロックドア
		}else {
			@rto_船倉デフォルト
		}

	//連続無線
	}else {
		@rto_船倉デフォルト
	}
}
proc rto_鳴り床説明 {
	@rto_鳴り床
	@rto_鳴り床２
}


block codec RTO_鳴り床 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_鳴り床説明聞いた){
		@rto_鳴り床説明
		eval($f:rft_鳴り床説明聞いた = 1)

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_鳴り床説明
		}else {
			@rto_船倉デフォルト
		}

	//連続無線
	}else {
		@rto_船倉デフォルト
	}
}

block codec RTO_はしご 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_はしご聞いた){
		@rto_はしご
		eval($f:rft_はしご聞いた = 1)

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_はしご
		}else {
			@rto_船倉デフォルト
		}

	//連続無線
	}else {
		@rto_船倉デフォルト
	}
}

block codec RTO_はしご鳴り床 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_鳴り床説明聞いた){
		@rto_鳴り床説明
		eval($f:rft_鳴り床説明聞いた = 1)

	}else if(!$f:rft_はしご聞いた){
		@rto_はしご
		eval($f:rft_はしご聞いた = 1)

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 8
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_はしご

		}else if($w:rft_rand < 6){
			@rto_鳴り床説明

		}else {
			@rto_船倉デフォルト
		}

	//連続無線
	}else {
		@rto_船倉デフォルト
	}
}



block codec RTO_船倉地下ハッチ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_船倉地下ハッチ聞いた){
		@rto_船倉地下ハッチ
		eval($f:rft_船倉地下ハッチ聞いた = 1)

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_船倉地下ハッチ
		}else {
			@rto_船倉デフォルト
		}

	//連続無線
	}else {
		@rto_船倉デフォルト
	}
}

proc rto_プロジェクタ {
	if(!$f:rft_プロジェクタ聞いた){
		@rto_プロジェクタ１
		@rto_プロジェクタ２
	}
	if(!$f:rft_テレビ会話聞いた){
		@rto_プロジェクタ３
		@rto_テレビ中継
		eval($f:rft_テレビ会話聞いた = 1)
	}
	@rto_プロジェクタ４
	eval($f:rft_プロジェクタ聞いた = 1)
}

block codec RTO_プロジェクタ 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_プロジェクタ聞いた){
		@rto_プロジェクタ
		eval($f:rft_プロジェクタ聞いた = 1)

	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_プロジェクタ
		}else {
			@rto_船倉デフォルト
		}

	//連続無線
	}else {
		@rto_船倉デフォルト
	}
}

block codec RTO_船倉地下 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_船倉地下小部屋聞いた){
		@rto_船倉地下小部屋
		eval($f:rft_船倉地下小部屋聞いた = 1)

	}else {
		rand 5
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 1){
			@rto_船倉地下小部屋
		}else {
			@rto_船倉デフォルト
		}
	}
}



block codec RTO_電送機 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_電送機聞いた){
		@rto_電送機
		eval($f:rft_電送機聞いた = 1)

	}else if(!$f:rft_電送機説明聞いた) {
		@rto_電送機説明
		eval($f:rft_電送機説明聞いた = 1)
		
	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_電送機
		}else {
			@rto_第三船倉デフォルト
		}

	//連続無線
	}else {
		@rto_第三船倉デフォルト
	}
}

proc rto_船倉ＴＶ兵説明 {
	@rto_船倉ＴＶ兵
	if(!$f:rft_テレビ会話聞いた){
		@rto_テレビ中継
		eval($f:rft_テレビ会話聞いた = 1)
	}
}

block codec RTO_船倉ＴＶ兵 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_船倉ＴＶ兵説明聞いた){
		@rto_船倉ＴＶ兵説明
		eval($f:rft_船倉ＴＶ兵説明聞いた = 1)

	}else {
		@rto_第三船倉デフォルト
	}
}

block codec RTO_ポール下 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_ポール下聞いた){
		@rto_ポール下
		eval($f:rft_ポール下聞いた = 1)

		
	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_ポール下
		}else {
			@rto_第三船倉デフォルト
		}

	//連続無線
	}else {
		@rto_第三船倉デフォルト
	}
}


block codec RTO_ポール上 14112 d:顔その1 {
	@rto_デフォルト前処理＿オタコン
	if(!$f:rft_ポール上聞いた){
		@rto_ポール上
		eval($f:rft_ポール上聞いた = 1)

		
	//無線一回目
	}else if($w:rft_callcount <= 1){
		rand 4
		eval($w:rft_rand = $status)	

		if($w:rft_rand < 3){
			@rto_ポール上
		}else {
			@rto_第三船倉デフォルト
		}

	//連続無線
	}else {
		@rto_第三船倉デフォルト
	}
}


//モーション班用デバッグスクリプトェウァアアアア
#if d:MGS2_SCN
#include "t_otacon_dbg.h"
#endif

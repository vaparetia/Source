/**************************************************
		無線でぽいいん
**************************************************/
#include "cdc_common_t.h"

//フラグをおっ立ててやるのだ。もうビンビン。
if($w:t_story < d:ST_船内入った){
	eval($w:t_story = d:ST_船内入った)
}

@rt_オタコンＳＡＶＥ＿デフォルト無線セット
command 無線設定 \
	-base 14112 t:RTO_オタコンデフォルト

/*-----------------------------------------
強制無線さん
------------------------------------------*/
proc rt_強制ＣＡＬＬ_04a0R操舵室無線機デモ {
	command 無線設定 \
	-call 14112 t:RTO_04a0R操舵室無線機デモ d:CODEC_DIRECT rt_強制ＣＡＬＬ_04a0R操舵室無線機デモ終了 \
	-f ( d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) 
}

//@rt_強制ＣＡＬＬ_04a0R操舵室無線機デモ


//操舵室全体
trap r_souda_shitu d:PLAYER \
	-mask ? \
	-exec {
		print 'r_souda_shitu'
		command 無線設定 \
			-over $3 14112 t:RTO_操舵室
	}


//右側あかないどあーん
if(!$f:global_右水密ドア開くイベント) {
	trap r_no_open_door d:PLAYER \
		-mask ? \
		-exec {
			print 'r_no_open_door'
			command 無線設定 \
				-over $3 14112 t:RTO_操舵室開かない扉ＣＡＬＬ
	}
	

	trap r_no_open_door d:PLAYER \
		-mask ? \
		-exec {
			print 'no_open_door'
			command 無線設定 \
				-over $3 14112 t:RTO_操舵室開かない扉

			if( ($3 == 出る) && ($f:rft_操舵室開かない扉ＣＡＬＬ発動中) ){
				//CALL中ならキャンセルしちゃう
				print 'Call Cancelだよ'
				command 無線設定 -reset
				eval($f:rft_操舵室開かない扉ＣＡＬＬ発動中 = 0)
			}
	}
	

//しつこく開けようとする彼もしくは彼女にはＣＡＬＬをお見舞い！
	trap r_no_open_door d:PLAYER \
		-mask いる \
		-button d:ACTION_BUTTON \
		-state 0 \
		-exec {
			eval($b:rft_操舵室開かない扉開けた回数 = $b:rft_操舵室開かない扉開けた回数 +1)
			if( ($b:rft_操舵室開かない扉開けた回数 >= 3) && \
					(!$f:rft_操舵室開かない扉聞いた) && \
					(`@rt_通常ＣＡＬＬしてもいいよ`) && \
					(!$f:rft_操舵室開かない扉ＣＡＬＬ発動中) ){

				eval($f:rft_操舵室開かない扉ＣＡＬＬ発動中 = 1)
				command 無線設定 \
					-call 14112 t:RTO_操舵室開かない扉ＣＡＬＬ d:CODEC_CALL
		}
	}

}


/*
//舵輪。舵輪のほう向いてる時だけ舵輪会話
	trap r_kaji スネーク \
		-mask いる \
		-dir 2048 512 \	//方向指定
		-exec {
			command 無線設定 \
				-over $3 14112 t:RTO_舵
	}

	//逃げるな警告、解除シチャイマス。
	trap r_kaji スネーク \
		-mask 出る \
		-exec {
			command 無線設定 \
				-over $3 14112 t:RTO_操舵室
	}
	trap r_kaji スネーク \
		-mask いる \
		-dir 1024,512 3072,512 4096,512\	//方向指定
		-exec {
			command 無線設定 \
				-over $3 14112 t:RTO_操舵室
	}
*/









//eval($f:rt_赤外線見た = 1)
/* -------------------------------------
//でばっぐ用信じては駄目
--------------------------------
eval($f:rt_赤外線通り抜けた = 0)
eval($w:t_story = d:ST_T06a1Dオルガ戦勝利後ポリゴンデモ終了)
eval($f:rft_スネーク不利 = 1)
eval($f:流血してる = 1)
eval($f:製品版 = 1)
eval($f:rft_オルガ戦オルガグレネード投げた = 1)
eval($w:t_story = d:ST_操舵室調査シナリオデモ開始)
//eval($s:エリア = "w01b")
*/

//eval($w:t_story = d:ST_T04a1D艦橋カモフ発見ポリゴンデモ開始)


proc rt_強制ＣＡＬＬ_T_03b1Rカモフ登場後無線機デモ製品版 {
	command 無線設定 \
	-call 14112 t:RTO_03b1Rカモフ登場後無線機デモ製品版 d:CODEC_DIRECT rt_強制ＣＡＬＬ_T_03b1Rカモフ登場後無線機デモ製品版終了 \
	-f d:CODEC_I_FD_O_FA \
	-d
}
proc rt_強制ＣＡＬＬ_T_03b1Rカモフ登場後無線機デモ製品版終了 {
print 'うにゃーん。ぼほひ'
}

//@rt_強制ＣＡＬＬ_T_03b1Rカモフ登場後無線機デモ製品版

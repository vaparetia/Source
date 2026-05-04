/**************************************************
		無線でぽいいん
**************************************************/
#include "cdc_common_t.h"

extern command ゲットオルガライフ
extern command ゲットオルガ行動フラグ

//オルガ戦中おーとくちゅ～るブロック
@rt_オタコンＳＡＶＥ＿デフォルト無線セット

command 無線設定 \
	-base 14112 t:RTO_オルガ戦中


	//逃げるな警告
	trap r_near_door d:PLAYER \
		-mask ? \
		-exec {
			command 無線設定 \
				-over $3 14112 t:RTO_オルガ戦逃げられない
	}

/*
	trap r_near_door d:PLAYER \
		-mask いる \
		-exec {
			print '$w:プレイヤー方向:' $w:プレイヤー方向
	}
*/


	//立ち入り禁止エリア小僧

	trap r_keep_out d:PLAYER \
		-mask ? \
		-exec {
			command 無線設定 \
				-over $3 14112 t:RTO_オルガ戦立入禁止
		}

//でばぐ用消さなきゃ駄目だっち。
//eval($w:t_story = d:ST_操舵室調査シナリオデモ開始)


//ヘタクソさん向けの開始時ヒントＣＡＬＬ
//print '$f:rtf_オルガ戦ヒントＣＡＬＬする１:'$f:rtf_オルガ戦ヒントＣＡＬＬする１
//print '$f:rtf_オルガ戦ヒントＣＡＬＬ１聞いた:'$f:rtf_オルガ戦ヒントＣＡＬＬ１聞いた

proc rt_オルガ戦ヒントＣＡＬＬ１終了 {
	eval($f:rtf_オルガ戦ヒントＣＡＬＬ１聞いた = 1)
	command varsave $f:rtf_オルガ戦ヒントＣＡＬＬ１聞いた
}

if( ($f:rtf_オルガ戦ヒントＣＡＬＬする１) && (!$f:rtf_オルガ戦ヒントＣＡＬＬ１聞いた) && \
		(`@rt_通常ＣＡＬＬしてもいいよ`) ){
	command 無線設定 \
		-call 14112 t:RTO_オルガ戦ヒントＣＡＬＬ１ d:CODEC_CALL rt_オルガ戦ヒントＣＡＬＬ１終了
}



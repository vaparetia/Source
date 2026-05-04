/**************************************************
		痰俐でぽいいん
**************************************************/
#include "cdc_common_t.h"

extern command ゲットオルガライフ
extern command ゲットオルガ乖瓢フラグ

//オルガ里面お〖とくちゅ×るブロック
@rt_オタコンＳＡＶＥ〔デフォルト痰俐セット

command 痰俐肋年 \
	-base 14112 t:RTO_オルガ里面


	//屁げるな焚桂
	trap r_near_door d:PLAYER \
		-mask ? \
		-exec {
			command 痰俐肋年 \
				-over $3 14112 t:RTO_オルガ里屁げられない
	}

/*
	trap r_near_door d:PLAYER \
		-mask いる \
		-exec {
			print '$w:プレイヤ〖数羹:' $w:プレイヤ〖数羹
	}
*/


	//惟ち掐り敦贿エリア井廖

	trap r_keep_out d:PLAYER \
		-mask ? \
		-exec {
			command 痰俐肋年 \
				-over $3 14112 t:RTO_オルガ里惟掐敦贿
		}

//でばぐ脱久さなきゃ绿誊だっち。
//eval($w:t_story = d:ST_拎律技拇汉シナリオデモ倡幌)


//ヘタクソさん羹けの倡幌箕ヒントＣＡＬＬ
//print '$f:rtf_オルガ里ヒントＣＡＬＬする１:'$f:rtf_オルガ里ヒントＣＡＬＬする１
//print '$f:rtf_オルガ里ヒントＣＡＬＬ１使いた:'$f:rtf_オルガ里ヒントＣＡＬＬ１使いた

proc rt_オルガ里ヒントＣＡＬＬ１姜位 {
	eval($f:rtf_オルガ里ヒントＣＡＬＬ１使いた = 1)
	command varsave $f:rtf_オルガ里ヒントＣＡＬＬ１使いた
}

if( ($f:rtf_オルガ里ヒントＣＡＬＬする１) && (!$f:rtf_オルガ里ヒントＣＡＬＬ１使いた) && \
		(`@rt_奶撅ＣＡＬＬしてもいいよ`) ){
	command 痰俐肋年 \
		-call 14112 t:RTO_オルガ里ヒントＣＡＬＬ１ d:CODEC_CALL rt_オルガ里ヒントＣＡＬＬ１姜位
}



//
//	cdc_common.h
//	無線共通関数用
//
//	2000/10/22	T.Fukushima
//	$Id: cdc_common.h,v 1.1 2002/02/01 06:08:31 usr01475 Exp $

/* polydemo_**.h と、cdc_common.h の位置関係から chara 無線システム
より先に無線プロックを呼んでしまう事故を回避するため、polydemo_**.h
の方で独自に chara 無線システム　を呼ぶので、二重呼びを回避するため
こちらは以下の如く処置 */
/*
if($s:d_num != not_demo){ // added by S.Hirano 010611
	chara 無線システム Codec 
} // added by S.Hirano 010611
*/

extern command プレイヤー状態取得



//流血してます！聞いてません！の時はCALLしてみるのもいいかな？
if($s:エリア != $s:rft_前のステージ){
	eval($f:rft_ロード後流血ＣＡＬＬ上等 = 1)
}else {
	eval($f:rft_ロード後流血ＣＡＬＬ上等 = 0)
}
eval($s:rft_前のステージ = $s:エリア)
command varsave $s:rft_前のステージ


command プレイヤー状態取得 -d
if( ($status & d:PFLAG_BLOOD) && (!$f:rft_流血止血条件説明聞いた) && \
	($f:rft_ロード後流血ＣＡＬＬ上等) ){
	chara delay 流血ディレイ -time 30 -exec{
		print '血だらけっす！'
		command 無線設定 \
			-call 14112 t:RTO_流血状態ＣＡＬＬ d:CODEC_CALL
	}
}



/*
proc ダミー無線終了プロック{
	print 'どぼびりんちょ！'
}

proc rt_ダミー無線強制ＳＥＮＤ {
	command 無線設定 \
	-call 14112 t:RTO_ダミー d:CODEC_DIRECT ダミー無線終了プロック
}

proc rt_ダミー無線強制ＣＡＬＬ {
	command 無線設定 \
	-call 14112 t:RTO_ダミー d:CODEC_DEMO ダミー無線終了プロック
}

@rt_ダミー無線強制ＣＡＬＬ
*/

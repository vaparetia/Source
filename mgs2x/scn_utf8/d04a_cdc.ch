/**************************************************
		無線でぽいいん デモ専
		船倉無線機デモ１、２用
//	2001/05/24	T.Fukushima
//	$Id: d04a_cdc.ch,v 1.8 2001/08/31 10:57:52 usr01717 Exp $
**************************************************/



//強制デモ呼び出しプロック
proc rt_強制ＣＡＬＬ_T_09b1R船倉無線機デモ１ {

	command セットサウンドコード -c d:SNG_PLAY_06

	command 無線設定 \
	-call 14112 t:RTO_09b1R船倉無線機デモ１ d:CODEC_DIRECT rt_強制ＣＡＬＬ_09b1R船倉無線機デモ１終了 \
//	-f d:CODEC_I_FA_O_FD \
	-f ( 	d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK) \
	-d
}

proc rt_強制ＣＡＬＬ_09b1R船倉無線機デモ１終了 {
	command セットサウンドコード -c d:SNG_FOUTS_S

	eval($w:t_story = d:ST_T09b1R船倉無線機デモ１終了)
	print '$w:t_story = d:ST_T09b1R船倉無線機デモ１終了'

	restart -s
}



proc rt_強制ＣＡＬＬ_T_10b1R船倉無線機デモ２ {

	command セットサウンドコード -c d:SNG_PLAY_06

	command 無線設定 \
	-call 14112 t:RTO_10b1R船倉無線機デモ２ d:CODEC_DIRECT rt_強制ＣＡＬＬ_10b1R船倉無線機デモ２終了 \
	-f ( 	d:CODEC_I_FD_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK) \
	-d
}

proc rt_強制ＣＡＬＬ_10b1R船倉無線機デモ２終了 {

	command セットサウンドコード -c d:SNG_FOUTS_S

	eval($w:t_story = d:ST_T10b1R船倉無線機デモ２終了)
	print '$w:t_story = d:ST_T10b1R船倉無線機デモ２終了'

	// ゲームに戻すにあたって０に。
	eval($w:船倉兵モード = 0)
	eval($s:d_num = not_demo)

	// デモが終わったので、0に戻す（レーダー表示のため）
	eval($f:global_polygon_demo = 0)

	@mv_w03b_w04a_demo
//	restart -s
}


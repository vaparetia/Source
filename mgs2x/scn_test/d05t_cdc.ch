/**************************************************
		無線でぽいいん デモ専
		シーン Ｔ０６ｂ１Ｒ オルガ戦勝利後無線機デモ強制ＳＥＮＤらしい。
//	2001/05/24	T.Fukushima
//	$Id: d05t_cdc.ch,v 1.1 2002/02/01 06:08:31 usr01475 Exp $
**************************************************/
//chara 無線システム Codec -player sna_def



//強制デモ呼び出しプロック

proc rt_強制ＣＡＬＬ_T_06b1Rオルガ戦勝利後無線機デモ {

	chara サウンドマネージャー ＳＤマネ -pak -1		// パックをロードしない

	command セットサウンドコード -c d:SNG_PLAY_06

	command 無線設定 \
	-call 14112 t:RTO_06b1Rオルガ戦勝利後無線機デモ１ \
		d:CODEC_DIRECT rt_強制ＣＡＬＬ_T_06b1Rオルガ戦勝利後無線機デモ終了 \
	-f (d:CODEC_I_AC_O_FA  | d:CODEC_FDOUT_BREAK ) \
//	-f d:CODEC_I_FD_O_FA \
//	-f ( d:CODEC_I_FA_O_FA  | d:CODEC_FDOUT_BREAK ) \
	-d
}

proc rt_強制ＣＡＬＬ_T_06b1Rオルガ戦勝利後無線機デモ終了 {

	command セットサウンドコード -c d:SNG_FOUTS_S

	eval($w:t_story = d:ST_T06b1Rオルガ戦勝利後無線機デモ終了)
	eval($f:global_polygon_demo = 0 )
	@mv_w00b_w00c_0
}


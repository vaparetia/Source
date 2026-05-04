/**************************************************
		無線でぽいいん デモ専
**************************************************/
//chara 無線システム Codec -player sna_def



//強制デモ呼び出しプロック
proc rt_強制ＣＡＬＬ_T_00b1Rオープニング無線機デモ {
	command 無線設定 \
	-call 14112 t:RTO_00b1Rオープニング無線機デモ \
		d:CODEC_DIRECT rt_強制ＣＡＬＬ_T_00b1Rオープニング無線機デモ終了 \
	-f d:CODEC_I_AC_O_FD \
	-d
}

proc rt_強制ＣＡＬＬ_T_03b1Rカモフ登場後無線機デモ体験版 {
	command 無線設定 \
	-call 14112 t:RTO_03b1Rカモフ登場後無線機デモ体験版 d:CODEC_DIRECT rt_強制ＣＡＬＬ_T_03b1Rカモフ登場後無線機デモ体験版終了 \
//	-f d:CODEC_I_NO_O_FA \
	-f d:CODEC_I_FD_O_FA \
	-d
}

proc rt_強制ＣＡＬＬ_T_03b1Rカモフ登場後無線機デモ製品版 {
	command 無線設定 \
	-call 14112 t:RTO_03b1Rカモフ登場後無線機デモ製品版 \
		d:CODEC_DIRECT rt_強制ＣＡＬＬ_T_03b1Rカモフ登場後無線機デモ製品版終了 \
	-f d:CODEC_I_FD_O_FA \
	-d
}


//@rt_強制ＣＡＬＬ_T_00b1Rオープニング無線機デモ
//#include "story.h"
//eval($w:t_story = d:ST_T03a1Dカモフ登場ポリゴンデモ開始)